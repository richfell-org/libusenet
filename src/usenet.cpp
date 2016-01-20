#include <string>
#include <libusenet/usenet>
#include <netdb.h>

#include "config.h"

namespace usenet {

inline bool __check_response(std::basic_iostream<char>& ios, ResponseStatus expected_status)
{
	Response response;
	ios >> response;
	return response.get_status() == expected_status;
}

static bool __authenticate(std::basic_iostream<char>& ios, const ServerProfile& server)
{
	std::string response;

	if(!server.get_username().empty())
	{
		ios << auth_user << server.get_username() << endf;
		if(!__check_response(ios, ResponseStatus::CMD_OK_SOFAR))
			return false;

		if(!server.get_password().empty())
		{
			ios << auth_pass << server.get_password() << endf;
			if(!__check_response(ios, ResponseStatus::CMD_OK))
				return false;
		}
	}

	return true;
}

ServerProfile::ServerProfile(const char *server_name, const char *port/* = "119"*/)
throw(std::runtime_error)
:	ServerProfile(server_name, 0, 0, 1, port)
{
}

ServerProfile::ServerProfile(const char *server_name, int num_connections, const char *port/* = "119"*/)
throw(std::runtime_error)
:	ServerProfile(server_name, 0, 0, num_connections, port)
{
}

ServerProfile::ServerProfile(const char *server_name, const char *username, const char *passwd, const char *port/* = "119"*/)
throw(std::runtime_error)
:	ServerProfile(server_name, username, passwd, 1, port)
{
}

ServerProfile::ServerProfile(
	const char *server_name, const char *username, const char *passwd, int num_connections, const char *port/* = "119"*/)
throw(std::runtime_error)
:	m_addr_len(0), m_num_conns(num_connections), m_username(username ? username : ""), m_password(passwd ? passwd : "")
{
	addrinfo hints, *pAddrInfo;

	// set up the hints structure
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 6;
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

ServerProfile::ServerProfile(const ServerProfile& that)
:	m_addr_len(that.m_addr_len), m_canon_name(that.m_canon_name),
	m_num_conns(that.m_num_conns),
	m_username(that.m_username), m_password(that.m_password)
	
{
	memcpy(&m_addr, &that.m_addr, sizeof(struct sockaddr));
}

ServerProfile::ServerProfile(ServerProfile&& that)
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

Response::operator bool() const
{
	ResponseStatus status = get_status();
	return(status != ResponseStatus::S_NONE && status != ResponseStatus::CMD_FAIL && status != ResponseStatus::ERROR);
}

stream::stream(const ServerProfile& server, int bufsz/* = 2 * 8192*/)
:	NetStream::basic_sockstream<char>(server.get_addr(), server.get_addrlen(), bufsz)
{
	// check connection response
	if(!__check_response(*this, ResponseStatus::CMD_OK))
	{
		setstate(std::ios::failbit);
		return;
	}

	// set a read timeout and handle any needed authentication
	timeval to_time = { (60 * 2), 0, };
	if(0 != setsockopt(m_buf.get_sock_fd(), SOL_SOCKET, SO_RCVTIMEO, &to_time, sizeof(to_time))
		|| !__authenticate(*this, server))
	{
		setstate(std::ios::failbit);
	}
}

void stream::open(const ServerProfile& server)
{
	// close existing connection (if any)
	close();
	NetStream::basic_sockstream<char>(server.get_addr(), server.get_addrlen());

	// check connection response
	if(!__check_response(*this, ResponseStatus::CMD_OK))
	{
		setstate(std::ios::failbit);
		return;
	}

	// set a read timeout and handle any needed authentication
	timeval to_time = { (60 * 2), 0, };
	if(0 != setsockopt(m_buf.get_sock_fd(), SOL_SOCKET, SO_RCVTIMEO, &to_time, sizeof(to_time))
		|| !__authenticate(*this, server))
	{
		setstate(std::ios::failbit);
	}
}

#ifdef USE_SSL
sslstream::sslstream(const ServerProfile& server, int bufsz/* = 2 * 8192*/)
:	NetStream::basic_sslsockstream<char>(server.get_addr(), server.get_addrlen(), bufsz)
{
	// check connection response
	if(!__check_response(*this, ResponseStatus::CMD_OK))
	{
		setstate(std::ios::failbit);
		return;
	}

	// set a read timeout and handle any needed authentication
	timeval to_time = { (60 * 2), 0, };
	if(0 != setsockopt(m_buf.get_sock_fd(), SOL_SOCKET, SO_RCVTIMEO, &to_time, sizeof(to_time))
		|| !__authenticate(*this, server))
	{
		setstate(std::ios::failbit);
	}
}

void sslstream::open(const ServerProfile& server)
{
	// close existing connection (if any)
	close();
	NetStream::basic_sslsockstream<char>(server.get_addr(), server.get_addrlen());

	// check connection response
	if(!__check_response(*this, ResponseStatus::CMD_OK))
	{
		setstate(std::ios::failbit);
		return;
	}

	// set a read timeout and handle any needed authentication
	timeval to_time = { (60 * 2), 0, };
	if(0 != setsockopt(m_buf.get_sock_fd(), SOL_SOCKET, SO_RCVTIMEO, &to_time, sizeof(to_time))
		|| !__authenticate(*this, server))
	{
		setstate(std::ios::failbit);
	}
}
#endif /* USE_SSL */

const std::string auth_user("AUTHINFO user ");
const std::string auth_pass("AUTHINFO pass ");
const std::string head("HEAD ");
const std::string body("BODY ");
const std::string article("ARTICLE ");
const std::string group("GROUP ");
const std::string post("POST");
const std::string quit("QUIT");

const std::string hdr_msgid("Message-ID: ");
const std::string hdr_subject("Subject: ");
const std::string hdr_body("Body:");
const std::string hdr_from("From: ");
const std::string hdr_org("Organization: ");
const std::string hdr_reply("Reply-To: ");
const std::string hdr_followup("Followup-To: ");

}	/* namespace usenet */
