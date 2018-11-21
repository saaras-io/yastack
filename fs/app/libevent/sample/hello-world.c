/* SPDX-License-Identifier: Apache-2.0
 * Copyright(c) 2018 Saaras Inc.
 */
/*
  This example program provides a trivial server program that listens for TCP
  connections on port 9995.  When they arrive, it writes a short message to
  each client connection, and closes each connection once it is flushed.

  Where possible, it exits cleanly in response to a SIGINT (ctrl-c).
*/


#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#ifndef _WIN32
#include <netinet/in.h>
# ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif
#include <sys/socket.h>
#endif

#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
#include "../../../lib/ff_api.h"

static const char MESSAGE[] =
"HTTP/1.1 200 OK\r\n"
"Server: F-Stack\r\n"
"Date: Sat, 25 Feb 2017 09:26:33 GMT\r\n"
"Content-Type: text/html\r\n"
"Content-Length: 14\r\n"
"Last-Modified: Tue, 21 Feb 2017 09:44:03 GMT\r\n"
"Connection: keep-alive\r\n"
"Accept-Ranges: bytes\r\n"
"\r\n"
"Hello, World!\n";

char html[] =
"HTTP/1.1 200 OK\r\n"
"Server: F-Stack\r\n"
"Date: Sat, 25 Feb 2017 09:26:33 GMT\r\n"
"Content-Type: text/html\r\n"
"Content-Length: 439\r\n"
"Last-Modified: Tue, 21 Feb 2017 09:44:03 GMT\r\n"
"Connection: keep-alive\r\n"
"Accept-Ranges: bytes\r\n"
"\r\n"
"<!DOCTYPE html>\r\n"
"<html>\r\n"
"<head>\r\n"
"<title>Welcome to F-Stack!</title>\r\n"
"<style>\r\n"
"    body {  \r\n"
"        width: 35em;\r\n"
"        margin: 0 auto; \r\n"
"        font-family: Tahoma, Verdana, Arial, sans-serif;\r\n"
"    }\r\n"
"</style>\r\n"
"</head>\r\n"
"<body>\r\n"
"<h1>Welcome to F-Stack!</h1>\r\n"
"\r\n"
"<p>For online documentation and support please refer to\r\n"
"<a href=\"http://F-Stack.org/\">F-Stack.org</a>.<br/>\r\n"
"\r\n"
"<p><em>Thank you for using F-Stack.</em></p>\r\n"
"</body>\r\n"
"</html>";



static const int PORT = 80;

static void listener_cb(struct evconnlistener *, evutil_socket_t,
    struct sockaddr *, int socklen, void *);
static void conn_writecb(struct bufferevent *, void *);
static void conn_eventcb(struct bufferevent *, short, void *);
static void signal_cb(evutil_socket_t, short, void *);

int loop(void *arg) {
	struct event_base *base = (struct event_base *) arg;
	//event_base_dispatch(base);
	event_base_loop(base, EVLOOP_NONBLOCK);

    return 0;
}
int
main(int argc, char **argv)
{
	struct event_base *base;
	struct evconnlistener *listener;
	struct event *signal_event;

	struct sockaddr_in sin;
    ff_init(argc, argv);

    printf("Before creating event_base_new() Err no %u\n", errno);
	base = event_base_new();
    printf("After creating event_base_new() Err no %u\n", errno);
	if (!base) {
		fprintf(stderr, "Could not initialize libevent!\n");
		return 1;
	}
    printf(" Created base [%p]\n", base);

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(PORT);

    printf("Before evconnlistener_new_bind() Err no %u\n", errno);
	listener = evconnlistener_new_bind(base, listener_cb, (void *)base,
	    LEV_OPT_CLOSE_ON_FREE, -1,
	    (struct sockaddr*)&sin,
	    sizeof(sin));
    printf("After evconnlistener_new_bind() Err no %u\n", errno);

	if (!listener) {
		fprintf(stderr, "Could not create a listener!\n");
		return 1;
	}

#if 0
	signal_event = evsignal_new(base, SIGINT, signal_cb, (void *)base);

	if (!signal_event || event_add(signal_event, NULL)<0) {
		fprintf(stderr, "Could not create/add a signal event!\n");
		return 1;
	}
#endif

	/* event_base_dispatch(base); */
    /* TODO: call ff_run() with dispatch loop */
    printf ("Invoking ff_run()\n");
    ff_run(loop, base);

	evconnlistener_free(listener);
#if 0
	event_free(signal_event);
#endif
	event_base_free(base);

	printf("done\n");
	return 0;
}

static void read_cb(evutil_socket_t fd, short s, void *ud) {
    u_char buf[8196];
	evutil_make_socket_nonblocking(fd);
    int sz = ff_read(fd, buf, sizeof(buf));

    if (sz > 0) {
        ff_write(fd, html, sizeof(html));
    } else {
        ff_close(fd);
    }
}

static void write_cb(evutil_socket_t fd, short s, void *ud) {
    ff_write(fd, html, sizeof(html));
    ff_close(fd);
}

static void
listener_cb(struct evconnlistener *listener, evutil_socket_t fd,
    struct sockaddr *sa, int socklen, void *user_data)
{
	struct event_base *base = user_data;
	struct bufferevent *bev;

	evutil_make_socket_nonblocking(fd);

    struct event * re = event_new(base, fd, EV_READ, read_cb, user_data);
    struct event * we = event_new(base, fd, EV_WRITE, write_cb, user_data);

    event_add(re, NULL);
    event_add(we, NULL);


#if 0
	bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
	if (!bev) {
		fprintf(stderr, "Error constructing bufferevent!");
		event_base_loopbreak(base);
		return;
	}
	bufferevent_setcb(bev, NULL, conn_writecb, conn_eventcb, NULL);
	bufferevent_enable(bev, EV_WRITE);
	bufferevent_disable(bev, EV_READ);

	bufferevent_write(bev, MESSAGE, strlen(MESSAGE));
#endif
}

#if 0
static void
conn_writecb(struct bufferevent *bev, void *user_data)
{
	struct evbuffer *output = bufferevent_get_output(bev);
	if (evbuffer_get_length(output) == 0) {
		printf("flushed answer\n");
		bufferevent_free(bev);
	}
}

static void
conn_eventcb(struct bufferevent *bev, short events, void *user_data)
{
	if (events & BEV_EVENT_EOF) {
		printf("Connection closed.\n");
	} else if (events & BEV_EVENT_ERROR) {
		printf("Got an error on the connection: %s\n",
		    strerror(errno));/*XXX win32*/
	}
	/* None of the other events can happen here, since we haven't enabled
	 * timeouts */
	bufferevent_free(bev);
}

static void
signal_cb(evutil_socket_t sig, short events, void *user_data)
{
	struct event_base *base = user_data;
	struct timeval delay = { 2, 0 };

	printf("Caught an interrupt signal; exiting cleanly in two seconds.\n");

	event_base_loopexit(base, &delay);
}
#endif
