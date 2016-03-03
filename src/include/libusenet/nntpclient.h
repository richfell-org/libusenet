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
#ifndef __NNTP_CLIENT_HEADER__
#define __NNTP_CLIENT_HEADER__

#include <cstdlib>
#include <stdexcept>
#include <system_error>
#include <memory>
#include <string>
#include <sys/socket.h>

#include "options.h"

#ifdef LIBUSENET_USE_SSL
#   include <openssl/ssl.h>
#endif

namespace NntpClient {

// The TCP protocol value
const int nntp_tcp_protocol = 6;

class ServerAddr;
class StatusResponse;
class Connection;


/*
 * Address for a remote server
 */
class ServerAddr
{
public:

	ServerAddr(const ServerAddr& that);
	ServerAddr(ServerAddr&& that);
	ServerAddr(const char *server_name, const char *port = "119")
		throw(std::runtime_error);
	ServerAddr(const char *server_name, int num_connections, const char *port = "119")
		throw(std::runtime_error);
	ServerAddr(const char *server_name, const char *username, const char *passwd, const char *port = "119")
		throw(std::runtime_error);
	ServerAddr(const char *server_name, int num_connections, const char *username, const char *passwd, const char *port = "119")
		throw(std::runtime_error);
	~ServerAddr() {}

public:

	// remote host info
	const sockaddr& get_addr() const { return m_addr; }
	size_t get_addrlen() const { return m_addr_len; }
	const std::string& get_canon_name() const { return m_canon_name; }

	// connection and auth info
	int get_number_of_connections() const { return m_num_conns; }
	void set_number_of_connections(int num_connections) { m_num_conns = num_connections; }

	const std::string& get_username() const { return m_username; }
	void set_username(const std::string& username) { m_username = username; }
	void set_username(std::string&& username) { m_username = username; }
	void set_username(const char *username) { m_username.assign(username); }

	const std::string& get_password() const { return m_password; }
	void set_password(const std::string& password) { m_password = password; }
	void set_password(std::string&& password) { m_password = password; }
	void set_password(const char *password) { m_password.assign(password); }

	ServerAddr& operator =(const ServerAddr&) = default;
	ServerAddr& operator =(ServerAddr&&) = default;

private:

	// network info
	sockaddr m_addr;
	size_t m_addr_len;
	std::string m_canon_name;

	// NNTP info
	int m_num_conns;
	std::string m_username;
	std::string m_password;
};

enum ResponseStatus { S_NONE = 0, INFO = 1, CMD_OK, CMD_OK_SOFAR, CMD_FAIL, ERROR, };
enum ResponseFunction { F_NONE = -1, CONNECTION, GROUP_SELECTION, ARTICLE_SELECTION, DISTRO, POSTING, NON_STD = 8, DEBUGGING, };

/*
 *
 */
class Response
{
// construction
public:

	Response();
	Response(const Response&) = default;
	~Response() {}

// atributes
public:

	const char *get_buffer() const { return &m_buf[0]; }
	int get_length() const { return m_len; }

	std::string get_line() const { return std::string(m_buf, m_len); }

	ResponseStatus get_status() const;
	ResponseFunction get_function() const;
	int get_number() const;
	std::string get_status_msg() const;

// operations
public:

	Response& clear() { m_len = 0; return *this; }

	Response& operator =(const Response&) = default;

private:

	friend class Connection;

	char m_buf[1024];
	int m_len;
};

/*
 *
 */
class Connection
{
// construction
public:

	Connection() throw(std::runtime_error);
	Connection(const ServerAddr& server, Response& response) throw(std::system_error);
	Connection(Connection&& transConnection);
	Connection(const Connection&) = delete;
	virtual ~Connection();

// operations
public:

	virtual void open(const ServerAddr& server, Response& response) throw(std::runtime_error);
	virtual void close();

	ResponseStatus read_response(Response& response) throw(std::runtime_error);
	int read_line(char *buf, size_t buflen) throw(std::runtime_error);

	ResponseStatus group(const char *group_name, Response response) throw(std::runtime_error);
	ResponseStatus stat(const char *message_id, Response response) throw(std::runtime_error);
	ResponseStatus article(const char *message_id, Response& response) throw(std::runtime_error);
	ResponseStatus header(const char *message_id, Response& response) throw(std::runtime_error);
	ResponseStatus body(const char *message_id, Response& response) throw(std::runtime_error);

	void send(const char *cmd) throw(std::runtime_error);
	void send(const char *cmd, const char *arg1, ...) throw(std::runtime_error);

	void send(const void *buf, unsigned long len) throw(std::runtime_error);

	virtual Connection& operator =(Connection&& transConnection);
	Connection& operator =(const Connection&) = delete;

	operator bool() const { return(m_sock >= 0); }

// implementation
protected:

	virtual void connect(const ServerAddr& server) throw(std::system_error);
	virtual void authenticate(const ServerAddr& server, Response& response) throw(std::runtime_error);
	virtual void write(const void *buf, size_t nbyte) throw(std::system_error);
	virtual int read(void *buf, size_t nbyte) throw(std::system_error);

	int m_sock;

	int m_rdsz;
	int m_ibuf;

	int m_buflen;
	std::unique_ptr<char[]> m_bufptr;
};

#ifdef LIBUSENET_USE_SSL

/*
 *
 */
class SslConnection : public Connection
{
// construction
public:

	SslConnection() throw(std::runtime_error);
	SslConnection(const ServerAddr& server, Response& response) throw(std::runtime_error);
	SslConnection(SslConnection&& transConnection);
	SslConnection(const SslConnection&) = delete;
	~SslConnection();

// operations
public:

	void close();

	SslConnection& operator =(SslConnection&& transConnection);
	SslConnection& operator =(const SslConnection&) = delete;

// implementation
protected:

	// provides the extra step of SSL_connect to the server connection
	void connect(const ServerAddr& server) throw(std::system_error);

	// provides the SSL_read and SSL_write
	void write(const void *buf, size_t nbyte) throw(std::system_error);
	int read(void *buf, size_t nbyte) throw(std::system_error);

	// SSL data structures
	std::unique_ptr<SSL_CTX, void(*)(SSL_CTX*)> m_ctxptr;
	std::unique_ptr<SSL, void(*)(SSL*)> m_sslptr;
};

#endif  /* LIBUSENET_USE_SSL */

}	// NntpClient

#endif	/* __NNTP_CLIENT_HEADER__ */
