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
#include <memory>
#include <cerrno>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <libusenet/sockstream>

#ifdef LIBUSENET_USE_SSL
#   include <openssl/ssl.h>
#   include <openssl/err.h>
#   include <openssl/crypto.h>
#endif  /* LIBUSENET_USE_SSL */

#if defined(LIBUSENET_USE_SSL) && defined(LIBUSENET_SSL_THREADS)
#	include <mutex>
#endif

namespace NetStream {

/*
 * Get the address info for the given remote host and service.
 * 'node' is the remote host name or dotted decimal address
 * 'service' is the port number (in ascii) or a service name, like HTTP
 *
 */
std::unique_ptr<addrinfo, void(*)(addrinfo*)> get_addr_info(const char *node, const char *service)
{
	std::unique_ptr<addrinfo, void(*)(addrinfo*)> result(nullptr, freeaddrinfo);

	addrinfo hints, *p_addr_info = nullptr;

	// set up the hints structure and get the addr info
	memset(&hints, 0, sizeof(addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 6;
	hints.ai_flags = AI_CANONNAME;

	// get the addr info
	int status = getaddrinfo(node, service, &hints, &p_addr_info);
	if(0 == status)
		result.reset(p_addr_info);

	return result;
}

/*
 * Attempts to connect to the given address.  If the connect attempt
 * fails then the socket is closed.  An EINTR result to 'connect' will
 * result in the connection attempt being retried.
 *
 * returns true if connected succesfully, false otherwise.
 * NOTE: If this function returns false the sockfd has been closed
 */
bool connect_socket(int sockfd, const sockaddr *p_addr, socklen_t addrlen)
{
	bool result = true;

	while(-1 == ::connect(sockfd, p_addr, addrlen) && errno != EISCONN)
	{
		// if this isn't an interrupted call to connect then ther is
		// a "serious" error and the connect attempt will be aborted
		if(EINTR != errno)
		{
			::close(sockfd);
			result = false;
			break;
		}
	}

	return result;
}

#if defined(LIBUSENET_USE_SSL)

#if defined(LIBUSENET_SSL_THREADS)
// mutex used to lock SSL initialization calls
static std::mutex sSslInitMutex;

// mutex array which is sized to CRYPTO_num_locks() and used
// for the operation of the crypto lib's locking function that
// is set using CRYPTO_set_locking_callback
static std::unique_ptr<std::recursive_mutex[]> sSslLocks;

/*
 * The definition and implementation of the crypto lib locking
 * function for multi-threaded operation of SSL library
 */
static void _ssl_locking_function(int mode, int n, const char *file, int line)
{
	if(mode & CRYPTO_LOCK)
		sSslLocks[n].lock();
	else
		sSslLocks[n].unlock();
}
#endif	/* LIBUSENET_SSL_THREADS */

/*
 * The initialization of the parts of SSL implementation
 */
void __init_ssl()
{
	static bool sIsSslInit = false;

	// do initialization if needed
#if defined(LIBUSENET_SSL_THREADS)
	//std::unique_lock<std::mutex> initLock(sSslInitMutex);
	std::lock_guard<std::mutex> initLock(sSslInitMutex);
#endif /* LIBUSENET_SSL_THREADS */
	if(!sIsSslInit)
	{
#if defined(LIBUSENET_SSL_THREADS)
		// allocate mutex instances according to the value of CRYPTO_num_locks()
		// add set our locking function, which is used by crypto lib to
		// call for the locking and unlocking by mutex index
		sSslLocks.reset(new std::recursive_mutex[CRYPTO_num_locks()]);
		CRYPTO_set_locking_callback(_ssl_locking_function);
#endif /* LIBUSENET_SSL_THREADS */
		// init libraries for SSL
		CRYPTO_malloc_init();
		SSL_library_init();
		SSL_load_error_strings();
		ERR_load_BIO_strings();
		OpenSSL_add_all_algorithms();

		sIsSslInit = true;
	}
}
#endif /* LIBUSENET_USE_SSL */

}	/* namespace NetStream */
