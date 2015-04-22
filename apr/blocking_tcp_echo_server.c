/* Alexis Giraudet
 * 2015
 *
 * Blocking tcp echo server using APR
 * http://apr.apache.org/
 */

#include <stdlib.h>
#include <apr_strings.h>
#include <apr_general.h>
#include <apr_network_io.h>
#include <apr_thread_pool.h>

#define ERR_BUF_LEN 128
#define MSG_BUF_LEN 128
#define MAX_THREADS 128
#define MAX_CONNECT 128

static apr_pool_t * mem_pool;
static apr_thread_pool_t * thr_pool;

void exit_clean(void)
{
	apr_thread_pool_destroy(thr_pool);
	apr_pool_destroy(mem_pool);
}

void print_error(apr_status_t err_code)
{
	static const apr_size_t err_len = ERR_BUF_LEN;
	char err_buf[ERR_BUF_LEN];
	char * err_str = apr_strerror(err_code, err_buf, err_len);
	fprintf(stderr, "Error: %s\n", err_str);
}

void exit_error(apr_status_t err_code)
{
	print_error(err_code);
	exit(EXIT_FAILURE);
}

void check_error(apr_status_t status)
{
	if(status != APR_SUCCESS)
		exit_error(status);
}

void * session_thread(apr_thread_t * thread, void * param)
{
	apr_size_t buf_len;
	char buf[MSG_BUF_LEN];
	apr_socket_t * sock = param;
	apr_status_t status_recv,
	             status_send;

	while(1)
	{
		buf_len = MSG_BUF_LEN;
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
	apr_socket_t * sock,
	             * new_sock;
	apr_sockaddr_t * sockaddr;

	status = apr_sockaddr_info_get(&sockaddr, NULL, APR_UNSPEC, port, 0, mem_pool);
	check_error(status);

	status = apr_socket_create(&sock, sockaddr->family, SOCK_STREAM, APR_PROTO_TCP, mem_pool);
	check_error(status);

	status = apr_socket_bind(sock, sockaddr);
	check_error(status);

	status = apr_socket_listen(sock, MAX_CONNECT);
	check_error(status);

	while(1)
	{
		status = apr_socket_accept(&new_sock, sock, mem_pool);
		check_error(status);

		status = apr_thread_pool_push(thr_pool, session_thread, new_sock, APR_THREAD_TASK_PRIORITY_NORMAL, NULL);
		check_error(status);
	}
}

int main(int argc, char const *argv[])
{
	int ret;
	apr_status_t status;

	ret = atexit(apr_terminate);
	if(ret != 0)
		return EXIT_FAILURE;

	ret = atexit(exit_clean);
	if(ret != 0)
		return EXIT_FAILURE;

	status = apr_initialize();
	check_error(status);

	status = apr_pool_create(&mem_pool, NULL);
	check_error(status);

	status = apr_thread_pool_create(&thr_pool, 0, MAX_THREADS, mem_pool);
	check_error(status);

	if(argc != 2)
	{
		fprintf(stderr, "Usage: %s PORT\n", argv[0]);
		return EXIT_FAILURE;
	}

	server(apr_atoi64(argv[1]));

	return EXIT_SUCCESS;
}
