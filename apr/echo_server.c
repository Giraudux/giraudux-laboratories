/* Alexis Giraudet
 * 2015
 *
 * Simple echo server using APR
 * http://apr.apache.org/
 */

#include <stdlib.h>
#include <apr_general.h>
#include <apr_network_io.h>
#include <apr_thread_pool.h>

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

void * session_thread(apr_thread_t * thread, void * param)
{
	apr_socket_t * sock = param;
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

	return NULL;
}

void server(apr_port_t port)
{
	apr_status_t status;
	apr_pool_t * mem_pool;
	apr_thread_pool_t * thr_pool;
	apr_socket_t * sock,
	             * new_sock;
	apr_sockaddr_t * sockaddr;

	status = apr_pool_create(&mem_pool, NULL);
	if(status != APR_SUCCESS)
		exit_error(status);

	status = apr_thread_pool_create(&thr_pool, 2, 2, mem_pool);
	if(status != APR_SUCCESS)
		exit_error(status);

	status = apr_sockaddr_info_get(&sockaddr, NULL, APR_UNSPEC, port, 0, mem_pool);
	if(status != APR_SUCCESS)
		exit_error(status);

	status = apr_socket_create(&sock, sockaddr->family, SOCK_STREAM, APR_PROTO_TCP, mem_pool);
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
		status = apr_socket_accept(&new_sock, sock, mem_pool);
		if(status != APR_SUCCESS)
			exit_error(status);

		status = apr_thread_pool_push(thr_pool, session_thread, new_sock, APR_THREAD_TASK_PRIORITY_NORMAL, NULL);
		if(status != APR_SUCCESS)
			exit_error(status);
	}

	apr_thread_pool_destroy(thr_pool);
	apr_pool_destroy(mem_pool);
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
