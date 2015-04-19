/* Alexis Giraudet
 * 2015
 *
 * Simple echo server using APR
 * http://apr.apache.org/
 */

#include <stdlib.h>
#include <apr_general.h>
#include <apr_network_io.h>

void print_error(apr_status_t err_code)
{
	apr_size_t err_len = 128;
	char err_buf[128];
	char * err_str = apr_strerror(err_code, err_buf, err_len);
	fprintf(stderr, "Error: %s\n", err_str);
}

void exit_error(apr_status_t err_code)
{
	print_error(err_code);
	exit(EXIT_FAILURE);
}

void socket_opt_set(apr_socket_t * sock, apr_int32_t opt, apr_int32_t on)
{
	apr_status_t status = apr_socket_opt_set(sock, opt, on);
	if(status != APR_SUCCESS)
		exit_error(status);
}

void session(apr_socket_t * sock)
{
	apr_size_t buf_len = 64;
	char buf[64];
	apr_status_t status_recv,
	             status_send;

	while(1)
	{
		status_recv = apr_socket_recv(sock, buf, &buf_len);

		if(buf_len > 0)
		{
			status_send = apr_socket_send(sock, buf, &buf_len);
			if(status_send != APR_SUCCESS)
			{
				print_error(status_send);
				break;
			}
		}

		if(status_recv != APR_SUCCESS)
		{
			print_error(status_recv);
			break;
		}
	}


	apr_socket_close(sock);
}

void server(apr_port_t port)
{
	apr_status_t status;
	apr_pool_t * pool;
	apr_socket_t * sock,
	             * new_sock;
	apr_sockaddr_t * sockaddr;

	status = apr_pool_create(&pool, NULL);
	if(status != APR_SUCCESS)
		exit_error(status);

	status = apr_sockaddr_info_get(&sockaddr, NULL, APR_UNSPEC, port, 0, pool);
	if(status != APR_SUCCESS)
		exit_error(status);

	status = apr_socket_create(&sock, sockaddr->family, SOCK_STREAM, APR_PROTO_TCP, pool);
	if(status != APR_SUCCESS)
		exit_error(status);

	status = apr_socket_bind(sock, sockaddr);
	if(status != APR_SUCCESS)
		exit_error(status);

	status = apr_socket_listen(sock, 1024);
	if(status != APR_SUCCESS)
		exit_error(status);

	while(1)
	{
		status = apr_socket_accept(&new_sock, sock, pool);
		if(status != APR_SUCCESS)
			exit_error(status);

		session(new_sock);
	}

	apr_pool_destroy(pool);
}

int main(int argc, char const *argv[])
{
	int ret;
	apr_status_t status;

	ret = atexit(apr_terminate);
	if(ret != 0)
		return EXIT_FAILURE;

	status = apr_initialize();
	if(status != APR_SUCCESS)
		exit_error(status);

	server(4444);

	return EXIT_SUCCESS;
}
