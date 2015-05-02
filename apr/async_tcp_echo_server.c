/**
 * Alexis Giraudet
 * 2015
 *
 * Async tcp echo server using APR
 * http://apr.apache.org/
 */

#include <stdio.h>
#include <stdlib.h>

#include <apr-1/apr_general.h>
#include <apr-1/apr_network_io.h>
#include <apr-1/apr_poll.h>
#include <apr-1/apr_strings.h>

#define ERR_BUF_LEN 128
#define MSG_BUF_LEN 128
#define MAX_DESCRIPTORS 128
#define MAX_CONNECTIONS 128

static char err_buf[ERR_BUF_LEN];

typedef struct baton_s
{
    apr_pool_t * pool;
    apr_socket_t * sock;
    apr_pollcb_t * poll_cb;
} baton_t;

typedef struct client_data_s
{
    apr_pool_t * pool;
    char * buf;
    apr_size_t buf_len;
    apr_status_t status;
} client_data_t;

apr_status_t pollcb_cb(void * data, apr_pollfd_t * poll_fd)
{
    baton_t * baton = data;
    apr_status_t status;
    apr_socket_t * new_sock;
    apr_pool_t * new_pool;
    apr_pollfd_t new_pollfd;
    client_data_t * sock_data;

    printf("%p ", poll_fd->desc.s);

    if(APR_POLLIN & poll_fd->rtnevents)
        printf("POLLIN");

    if(APR_POLLPRI & poll_fd->rtnevents)
        printf("POLLPRI");

    if(APR_POLLOUT & poll_fd->rtnevents)
        printf("POLLOUT");

    if(APR_POLLERR & poll_fd->rtnevents)
        printf("POLLERR");

    if(APR_POLLHUP & poll_fd->rtnevents)
        printf("POLLHUB");

    if(APR_POLLNVAL & poll_fd->rtnevents)
        printf("POLLNVAL");

    if(baton->sock == poll_fd->desc.s)
    {
        printf(" (listener)\n");

        if(APR_POLLIN & poll_fd->rtnevents)
        {
            printf("accept\n");

            status = apr_pool_create(&new_pool, baton->pool);
            if(status)
                return status;

            status = apr_socket_accept(&new_sock, baton->sock, new_pool);
            if(status)
                return status;

            status = apr_socket_opt_set(new_sock, APR_SO_NONBLOCK, 1);
            if(status)
                return status;

            status = apr_socket_timeout_set(new_sock, 0);
            if(status)
                return status;

            sock_data = apr_palloc(new_pool, sizeof(client_data_t));
            sock_data->pool = new_pool;
            sock_data->buf = apr_palloc(new_pool, MSG_BUF_LEN);
            sock_data->buf_len = 0;
            sock_data->status = 0;

            new_pollfd.p = new_pool;
            new_pollfd.desc_type = APR_POLL_SOCKET;
            new_pollfd.reqevents = APR_POLLIN /*| APR_POLLPRI | APR_POLLOUT | APR_POLLERR | APR_POLLHUP | APR_POLLNVAL*/;
            new_pollfd.rtnevents = 0;
            new_pollfd.desc.s = new_sock;
            new_pollfd.client_data = sock_data;

            status = apr_pollcb_add(baton->poll_cb, &new_pollfd);
            if(status)
                return status;
        }
    }
    else
    {
        printf(" (client)\n");

        new_pollfd.p = poll_fd->p;
        new_pollfd.desc_type = poll_fd->desc_type;
        new_pollfd.rtnevents = poll_fd->rtnevents;
        new_pollfd.desc.s = poll_fd->desc.s;
        new_pollfd.client_data = poll_fd->client_data;

        sock_data = poll_fd->client_data;

        status = apr_pollcb_remove(baton->poll_cb, poll_fd);
        if(status)
            return status;

        if(APR_POLLIN & new_pollfd.rtnevents)
        {
            printf("read\n");

            new_pollfd.reqevents = APR_POLLOUT;
            sock_data->buf_len = MSG_BUF_LEN;
            sock_data->status = apr_socket_recv(new_pollfd.desc.s, sock_data->buf, &(sock_data->buf_len));
        }

        if(APR_POLLOUT & new_pollfd.rtnevents)
        {
            printf("write\n");

            new_pollfd.reqevents = APR_POLLIN;
            status = apr_socket_send(new_pollfd.desc.s, sock_data->buf, &(sock_data->buf_len));
            if(status)
                return status;
        }

        if(sock_data->status)
        {
            status  = apr_socket_close(new_pollfd.desc.s);
            apr_pool_destroy(sock_data->pool);
            if(status)
                return status;
        }
        else
        {
            status = apr_pollcb_add(baton->poll_cb, &new_pollfd);
            if(status)
                return status;
        }
    }

    return APR_SUCCESS;
}

apr_status_t server(apr_port_t port, apr_pool_t * pool)
{
    apr_status_t status;
    apr_socket_t * sock;
    apr_sockaddr_t * sockaddr;
    apr_pool_t * local_pool;
    apr_pollcb_t * poll_cb;
    apr_pollfd_t poll_fd;
    baton_t baton;

    status = apr_pool_create(&local_pool, pool);
    if(status)
        return status;

    status = apr_sockaddr_info_get(&sockaddr, NULL, APR_UNSPEC, port, 0, local_pool);
    if(status)
        return status;

    status = apr_socket_create(&sock, sockaddr->family, SOCK_STREAM, APR_PROTO_TCP, local_pool);
    if(status)
        return status;

    status = apr_socket_opt_set(sock, APR_SO_NONBLOCK, 1);
    if(status)
        return status;

    status = apr_socket_timeout_set(sock, 0);
    if(status)
        return status;

    status = apr_socket_bind(sock, sockaddr);
    if(status)
        return status;

    status = apr_socket_listen(sock, MAX_CONNECTIONS);
    if(status)
        return status;

    status = apr_pollcb_create(&poll_cb, MAX_DESCRIPTORS, local_pool, 0);
    if(status)
        return status;

    poll_fd.p = local_pool;
    poll_fd.desc_type = APR_POLL_SOCKET;
    poll_fd.reqevents = APR_POLLIN /*| APR_POLLPRI | APR_POLLOUT | APR_POLLERR | APR_POLLHUP | APR_POLLNVAL*/;
    poll_fd.rtnevents = 0;
    poll_fd.desc.s = sock;
    poll_fd.client_data = NULL;

    baton.sock = sock;
    baton.pool = local_pool;
    baton.poll_cb = poll_cb;

    status = apr_pollcb_add(poll_cb, &poll_fd);
    if(status)
        return status;

    for(;;)
    {
        status = apr_pollcb_poll(poll_cb, -1, pollcb_cb, &baton);
        if(status)
            break;
    }

    return status;
}

int main(int argc, char const *argv[])
{
    apr_status_t status;
    char * err_msg;

    status = atexit(apr_terminate);
    if(status)
        return status;

    status = apr_initialize();
    if(status)
        return status;

    if(argc != 2)
    {
        fprintf(stderr, "Usage: %s PORT\n", argv[0]);
        return APR_BADARG;
    }

    status = server(apr_atoi64(argv[1]), NULL);
    
    err_msg = apr_strerror(status, err_buf, ERR_BUF_LEN);
    printf("Error: %s\n", err_msg);

    return status;
}
