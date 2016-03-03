/*
	libusenet NNTP/NZB tools

    Copyright (C) 2016  Richard J. Fellinger, Jr

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, see <http://www.gnu.org/licenses/> or write
	to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
	Boston, MA 02110-1301 USA.
*/
#include <libusenet/nntpclient.h>

#include <cstdarg>
#include <cerrno>
#include <cstring>
#include <algorithm>
#include <utility>
#include <sstream>
#include <mutex>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#ifdef LIBUSENET_USE_SSL
#   include <openssl/ssl.h>
#   include <openssl/err.h>
#   include <openssl/crypto.h>
#endif

#include <iostream>

#ifdef LIBUSENET_USE_SSL
namespace NetStream {
// SSL initialization function
void __init_ssl();
}
#endif  /* LIBUSENET_USE_SSL */

namespace NntpClient {

ServerAddr::ServerAddr(const char *server_name, const char *port/* = "119"*/)
throw(std::runtime_error)
:	ServerAddr(server_name, 1, 0, 0, port)
{
}

ServerAddr::ServerAddr(const char *server_name, int num_connections, const char *port/* = "119"*/)
throw(std::runtime_error)
:	ServerAddr(server_name, num_connections, 0, 0, port)
{
}

ServerAddr::ServerAddr(const char *server_name, const char *username, const char *passwd, const char *port/* = "119"*/)
throw(std::runtime_error)
:	ServerAddr(server_name, 1, username, passwd, port)
{
}

ServerAddr::ServerAddr(const char *server_name, int num_connections, const char *username, const char *passwd, const char *port/* = "119"*/)
throw(std::runtime_error)
:	m_addr_len(0), m_num_conns(num_connections), m_username(username ? username : ""), m_password(passwd ? passwd : "")
{
	addrinfo hints, *pAddrInfo;

	// set up the hints structure
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = nntp_tcp_protocol;
	hints.ai_flags = AI_NUMERICSERV|AI_CANONNAME;

	// get the address info - looking at the example code
	// on the man page it seems that freeaddrinfo is not
	// needed when an error code is returned
	register int result = getaddrinfo(server_name, port, &hints, &pAddrInfo);
	if(0 != result)
		throw std::runtime_error(gai_strerror(result));

	// save the address info and free the returned list
	memcpy(&m_addr, pAddrInfo->ai_addr, sizeof(struct sockaddr));
	m_addr_len = pAddrInfo->ai_addrlen;
	m_canon_name.assign(pAddrInfo->ai_canonname);

	freeaddrinfo(pAddrInfo);
}

ServerAddr::ServerAddr(const ServerAddr& that)
:	m_addr_len(that.m_addr_len), m_canon_name(that.m_canon_name),
	m_num_conns(that.m_num_conns),
	m_username(that.m_username), m_password(that.m_password)
	
{
	memcpy(&m_addr, &that.m_addr, sizeof(struct sockaddr));
}

ServerAddr::ServerAddr(ServerAddr&& that)
:	m_addr_len(that.m_addr_len), m_canon_name(std::move(that.m_canon_name)),
	m_num_conns(that.m_num_conns),
	m_username(std::move(that.m_username)), m_password(std::move(that.m_password))
	
{
	that.m_addr_len = 0;
	that.m_num_conns = 1;
	memcpy(&m_addr, &that.m_addr, sizeof(struct sockaddr));
	bzero(&that.m_addr, sizeof(struct sockaddr));
}

Response::Response()
:	m_len(0)
{
}

std::string Response::get_status_msg() const
{
	std::string result("");
	if(m_len > 4)
		result.assign(&m_buf[4], m_len - 4);
	return result;
}

ResponseStatus Response::get_status() const
{
	ResponseStatus result = ResponseStatus::S_NONE;
	if((m_len > 0) && ('0' <= m_buf[0]) && ('9' >= m_buf[0]))
		result = ResponseStatus(m_buf[0] - '0');
	return result;
}

ResponseFunction Response::get_function() const
{
	ResponseFunction result = ResponseFunction::F_NONE;
	if((m_len > 1) && ('0' <= m_buf[1]) && ('9' >= m_buf[1]))
		result = ResponseFunction(m_buf[1] - '0');
	return result;
}

int Response::get_number() const
{
	int result = -1;
	if((m_len > 2) && ('0' <= m_buf[2]) && ('9' >= m_buf[2]))
		result = m_buf[2] - '0';
	return result;
}

//#include <sys/stat.h>

Connection::Connection()
throw(std::runtime_error)
:	m_sock(-1), m_rdsz(0), m_ibuf(0), m_buflen(0), m_bufptr()
{
	// create the socket
	m_sock = socket(AF_INET, SOCK_STREAM, /*nntp_tcp_protocol*/0);
	if(-1 == m_sock)
		throw std::runtime_error(strerror(errno));

	// create a timeout value for socket reads
	struct timeval to_time = { (60 * 3), 0, };

	// Get the internal buffer size of this socket and set the read timeout
	socklen_t optValLen = sizeof(m_buflen);
	if((0 != getsockopt(m_sock, SOL_SOCKET, SO_RCVBUF, &m_buflen, &optValLen))
		|| (0 != setsockopt(m_sock, SOL_SOCKET, SO_RCVTIMEO, &to_time, sizeof(to_time))))
	{
		register int result = errno;
		::close(m_sock);
		m_bufptr.reset(0);
		throw std::runtime_error(strerror(result));
	}
#if 0
	struct stat sbuf;
	if(-1 != fstat(m_sock, &sbuf))
	{
		std::cerr << '(' << m_sock << ") SOCK RECV BUFSZ: " << m_buflen << std::endl;
		std::cerr << '(' << m_sock << ") Preferred I/O block size: " << long(sbuf.st_blksize) << std::endl;
		m_buflen = std::min(m_buflen, int(4 * sbuf.st_blksize));
		std::cerr << '(' << m_sock << ") allocating: " << m_buflen << " bytes." << std::endl;
	}
#endif

	// Allocate our buffer to match the internal buffer size for block reads
	m_buflen /= 2;
	m_bufptr.reset(new char[m_buflen]);
}

Connection::Connection(const ServerAddr& server, Response& response)
throw(std::system_error)
:	Connection()
{
	open(server, response);
}

Connection::Connection(Connection&& transConnection)
:	m_sock(transConnection.m_sock),
	m_rdsz(transConnection.m_rdsz),
	m_ibuf(transConnection.m_buflen),
	m_bufptr(std::move(transConnection.m_bufptr))
{
	// reset the values of the transient instance
	transConnection.m_sock = -1;
	transConnection.m_rdsz = 0;
	transConnection.m_ibuf = 0;
	transConnection.m_buflen = 0;
}

Connection& Connection::operator =(Connection&& transConnection)
{
	//m_sock = transConnection.m_sock;
	std::swap(m_sock, transConnection.m_sock);
	m_rdsz = transConnection.m_rdsz;
	m_ibuf = transConnection.m_buflen;
	m_bufptr = std::move(transConnection.m_bufptr);
	
	// reset the values of the transient instance
	//transConnection.m_sock = -1;
	transConnection.m_rdsz = 0;
	transConnection.m_ibuf = 0;
	transConnection.m_buflen = 0;

	return *this;
}

Connection::~Connection()
{
	close();
}

void Connection::open(const ServerAddr& server, Response& response)
throw(std::runtime_error)
{
	// make network connection, read server response and check for a NNTP OK response
	connect(server);
	if(CMD_OK != read_response(response))
		throw std::system_error(std::error_code(EPROTO, std::system_category()), strerror(EPROTO));

	// do any needed authentication
	authenticate(server, response);
}

void Connection::close()
{
	if(m_sock < 0) return;

	::close(m_sock);
	m_sock = -1;
}

void Connection::send(const char *cmd)
throw(std::runtime_error)
{
	std::string cmdline(cmd);
	register std::string::size_type len = cmdline.size();

	// 512 is max NNTP command, which needs to include "\r\n", so there are 510 command chars max
	if(len > 510)
	{
		cmdline[510] = '\r';
		if(len > 511)
			cmdline[511] = '\n';
		else
			cmdline.push_back('\n');
		len = 512;
	}
	else
	{
		cmdline.append("\r\n");
		len += 2;
	}

	// send the cmd line to the server
	write(cmdline.c_str(), cmdline.size());
}

void Connection::send(const char *cmd, const char *arg1, ...)
throw(std::runtime_error)
{
	bool inArticleId = false;

	std::stringstream cmdbuf;

	// add initial cmd token
	cmdbuf << cmd;
	if(0 != arg1)
	{
		// always will space delineate the cmd from the first arg
		cmdbuf << ' ' << arg1;

		// anything wrapped in '<' and '>' chars is an article ID and no spaces can be between the article ID and the chars
		inArticleId = (arg1[0] == '<') && (arg1[1] == 0);

		char *arg = 0;
		va_list vargs;
		va_start(vargs, arg1);

		while(0 != (arg = va_arg(vargs, char *)))
		{
			if(!inArticleId)
				inArticleId = (arg[0] == '<') && (arg[1] == 0);

			// add a space unless an article ID has been detected
			if(!inArticleId) cmdbuf << ' ';

			// add the next token to the NNTP cmd line buffer
			cmdbuf << arg;

			// if we detected an article ID, see if we are done writing it
			if(inArticleId)
				inArticleId = !((arg[0] == '>') && (arg[1] == 0));
		}

		va_end(vargs);
	}

	std::string cmdline = cmdbuf.str();
	register std::string::size_type len = cmdline.size();

	// 512 is max NNTP command, which needs to include "\r\n", so there are 510 command chars max
	if(len > 510)
	{
		cmdline[510] = '\r';
		if(len > 511)
			cmdline[511] = '\n';
		else
			cmdline.push_back('\n');
		len = 512;
	}
	else
	{
		cmdline.append("\r\n");
		len += 2;
	}

	// send the cmd line to the NNTP server
	write(cmdline.c_str(), len);
}

void Connection::send(const void *buf, unsigned long len)
throw(std::runtime_error)
{
	write(buf, len);
}

ResponseStatus Connection::read_response(Response& response)
throw(std::runtime_error)
{
	response.m_len = read_line(response.m_buf, 1024);
	return response.get_status();
}

//int Connection::read_line(Response& response)
int Connection::read_line(char *buf, size_t buflen)
throw(std::runtime_error)
{
	unsigned int linelen;
	int rdlen;

	// rdlen is used to track how many chars were pulled from the NntpConnection buffer and
	// response.m_len is used to track the next available index in the Response when this routine is
	// finished response.m_len will give the caller the number of chars in of chars in the response buffer
	for(linelen = rdlen = 0; linelen < buflen; )
	{
		if(m_ibuf >= m_rdsz)
		{
			// reset and refill block buffer
			m_ibuf = 0;
			m_rdsz = read(m_bufptr.get(), m_buflen);

			// if no data was read then we're done
			if(0 == m_rdsz) break;
		}

		// copy bytes to the response buffer until '\n' is seen
		buf[linelen] = m_bufptr[m_ibuf++];

		// NNTP: lines beginning with a '.' will have that char repeated unless it is the end-of text response
		// indicator we'll ignore all '.' chars in the first postion of a line and return a 0-length read
		// if it is actually the end-of-text indicator line, which will end up being "\r\n"
		if((1 == ++rdlen) && ('.' == buf[linelen]))
			continue;

		// check for end-of-line...also incrments the length/next available index
		if('\n' == buf[linelen++])
			break;
	}

	// check for a multi-line end-of text response, which came in as ".\r\n" and had the '.' char stripped above
	if((3 == rdlen) && ('\r' == buf[0]) && ('\n' == buf[1]))
		linelen = 0;

	return linelen;
}

ResponseStatus Connection::group(const char *group_name, Response response)
throw(std::runtime_error)
{
	send("GROUP", group_name, nullptr);
	return read_response(response);
}

ResponseStatus Connection::stat(const char *message_id, Response response)
throw(std::runtime_error)
{
	send("STAT", "<", message_id, ">", nullptr);
	return read_response(response);
}

ResponseStatus Connection::article(const char *message_id, Response& response)
throw(std::runtime_error)
{
	send("ARTICLE", "<", message_id, ">", nullptr);
	return read_response(response);
}

ResponseStatus Connection::header(const char *message_id, Response& response)
throw(std::runtime_error)
{
	send("HEAD", "<", message_id, ">", nullptr);
	return read_response(response);
}

ResponseStatus Connection::body(const char *message_id, Response& response)
throw(std::runtime_error)
{
	send("BODY", "<", message_id, ">", nullptr);
	return read_response(response);
}

void Connection::connect(const ServerAddr& server)
throw(std::system_error)
{
	// attempt connection to remote host
	while(-1 == ::connect(m_sock, &server.get_addr(), server.get_addrlen()) && errno != EISCONN)
	{
		if(EINTR != errno)
			throw std::system_error(std::error_code(errno, std::system_category()), strerror(errno));
	}
}

void Connection::authenticate(const ServerAddr& server, Response& response)
throw(std::runtime_error)
{
	// need to do AUTH?
	const std::string& user = server.get_username();
	if(!user.empty())
	{
		// send user name and check result
		send("AUTHINFO user", user.c_str(), nullptr);
		if(CMD_OK_SOFAR != read_response(response))
			throw std::runtime_error(response.get_line().c_str());

		// send password and check result
		send("AUTHINFO pass", server.get_password().c_str(), nullptr);
		if(CMD_OK != read_response(response))
			throw std::runtime_error(response.get_line().c_str());
	}
}

void Connection::write(const void *buf, size_t nbyte)
throw(std::system_error)
{
	while(-1 == ::write(m_sock, buf, nbyte))
	{
		if(EINTR != errno)
			throw std::system_error(std::error_code(errno, std::system_category()), strerror(errno));
	}
}

int Connection::read(void *buf, size_t nbyte)
throw(std::system_error)
{
	register int result;
	while(-1 == (result = ::read(m_sock, buf, nbyte)))
	{
		if(EINTR != errno)
			throw std::system_error(std::error_code(errno, std::system_category()), strerror(errno));
	}

	return result;
}

#ifdef LIBUSENET_USE_SSL

SslConnection::SslConnection()
throw(std::runtime_error)
:	Connection(), m_ctxptr((SSL_CTX*)0, SSL_CTX_free), m_sslptr((SSL*)0, SSL_free)
{
	NetStream::__init_ssl();

	m_ctxptr.reset(SSL_CTX_new(SSLv3_client_method()));
	m_sslptr.reset(SSL_new(m_ctxptr.get()));
	SSL_set_fd(m_sslptr.get(), m_sock);
	SSL_set_mode(m_sslptr.get(), SSL_MODE_AUTO_RETRY);
}

SslConnection::SslConnection(const ServerAddr& server, Response& response)
throw(std::runtime_error)
:	SslConnection()
{
	open(server, response);
}

SslConnection::SslConnection(SslConnection&& transConnection)
:	Connection(std::move(transConnection)),
	m_ctxptr(std::move(transConnection.m_ctxptr)),
	m_sslptr(std::move(transConnection.m_sslptr))
{
}

SslConnection& SslConnection::operator =(SslConnection&& transConnection)
{
	Connection::operator =(std::move(transConnection));
	m_ctxptr = std::move(transConnection.m_ctxptr);
	m_sslptr = std::move(transConnection.m_sslptr);

	return *this;
}

SslConnection::~SslConnection()
{
	close();
}

void SslConnection::close()
{
	if(m_sslptr)
	{
		register int status;
		while(1 != (status = ::SSL_shutdown(m_sslptr.get())))
		{
			// break out of shutdown loop on "fatal error" result and let
			// the SSL resources be deallocated
			if(-1 == status)
				break;
		}
	}

	m_sslptr.reset(0);
	m_ctxptr.reset(0);
	Connection::close();
}

static const char *__get_ssl_err_str(int ssl_errnum, const char *dflt_msg)
{
	const char *reason = dflt_msg;

	switch(ssl_errnum)
	{
		case SSL_ERROR_ZERO_RETURN:
			reason = "the TLS/SSL connection has been closed";
			break;
		case SSL_ERROR_SYSCALL:
			reason = "some I/O error occurred";
			break;
		case SSL_ERROR_WANT_READ:
		case SSL_ERROR_WANT_WRITE:
		case SSL_ERROR_WANT_CONNECT:
		case SSL_ERROR_WANT_ACCEPT:
			reason = "the operation did not complete; the same TLS/SSL I/O function should be called again later";
			break;
		case SSL_ERROR_WANT_X509_LOOKUP:
			reason = "The operation did not complete because an application callback set by SSL_CTX_set_client_cert_cb() has asked to be called again";
			break;
		case SSL_ERROR_SSL:
			reason = "a failure in the SSL library occurred, usually a protocol error";
			break;
		default:
			break;
	}

	return reason;
}

void SslConnection::connect(const ServerAddr& server)
throw(std::system_error)
{
	// attempt connection to remote host
	Connection::connect(server);

	// do the SSL handshake
	int ssl_status;
	while(1 != (ssl_status = SSL_connect(m_sslptr.get())))
	{
		// check for the causing error code, just retry SSL_conenct on WANT_READ/WRITE
		int errnum = SSL_get_error(m_sslptr.get(), ssl_status);
		if((SSL_ERROR_WANT_READ == errnum) || (SSL_ERROR_WANT_WRITE == errnum))
		   continue;

		// else fatal error
		const char *reason = ERR_reason_error_string(ERR_get_error());
		if(nullptr == reason)
			reason = __get_ssl_err_str(errnum, "SslConnection::connect error during SSL_connect");
		throw std::system_error(std::error_code(errnum, std::generic_category()), reason);
	}
}

void SslConnection::write(const void *buf, size_t nbyte)
throw(std::system_error)
{
	register int ssl_status, errnum = 0;
	do
	{
		// write the data...need to re-call SSL_write on SSL_ERROR_WANT_WRITE or SSL_ERROR_WANT_READ
		ssl_status = ::SSL_write(m_sslptr.get(), buf, nbyte);
		if(ssl_status <= 0)
			errnum = SSL_get_error(m_sslptr.get(), ssl_status);
	} while((ssl_status < 0) && ((SSL_ERROR_WANT_WRITE == errnum) || (SSL_ERROR_WANT_READ == errnum)));

	// error condition other than SSL_ERROR_WANT_WRITE or SSL_ERROR_WANT_READ
	if(ssl_status <= 0)
	{
		const char *reason = ERR_reason_error_string(ERR_get_error());
		if(nullptr == reason)
			reason = __get_ssl_err_str(errnum, "SslConnection::write error during SSL_write");
		throw std::system_error(std::error_code(errnum, std::generic_category()), reason);
	}
}

int SslConnection::read(void *buf, size_t nbyte)
throw(std::system_error)
{
	register int ssl_status = ::SSL_read(m_sslptr.get(), buf, nbyte);

	// error condition
	if(0 >= ssl_status)
	{
		int errnum = SSL_get_error(m_sslptr.get(), ssl_status);
		const char *reason = ERR_reason_error_string(ERR_get_error());
		if(nullptr == reason)
			reason = __get_ssl_err_str(errnum, "SslConnection::read error during SSL_read");
		throw std::system_error(std::error_code(errnum, std::generic_category()), reason);
	}

	return ssl_status;
}

#endif  /* LIBUSENET_USE_SSL */

}	// namespace NntpClient
