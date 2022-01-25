// HTTP client for testing high connection concurrency
// Authors: Richard Jones and Rasmus Andersson and Josh Enders
// Released in the public domain. No restrictions, no support.
#include <sys/types.h>
#include <sys/time.h>
#include <sys/queue.h>
#include <stdlib.h>
#include <err.h>
#include <event2/buffer.h>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/http_struct.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <pthread.h>

#define BUFSIZE 4096
#define SLEEP_MS 10


// options
int num_conns = 100;

// state
int bytes_recvd = 0;
int chunks_recvd = 0;
int closed = 0;
int connected = 0;
int max_concurrency = 0;
int responses_completed_ok = 0;

// misc
char buf[BUFSIZE];

// called per chunk received
void chunkcb(struct evhttp_request * req, void * arg) {
    int s = evbuffer_remove( req->input_buffer, &buf, BUFSIZE );
    //printf("Read %d bytes: %s\n", s, &buf);
    bytes_recvd += s;
    chunks_recvd++;
}

// gets called when request completes
void reqcb(struct evhttp_request * req, void * arg) {
    int s = evbuffer_remove( req->input_buffer, &buf, BUFSIZE );
    bytes_recvd += s;
    //chunks_recvd++;

    if (connected-closed > max_concurrency)
        max_concurrency = connected-closed;

    responses_completed_ok++;
    closed++;
}

int main(int argc, char * const *argv) {

    struct event_base *base;    

    if (argc > 1 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        fprintf(stderr, "%s <connections> <host> <address> <port> <uri>\n", argv[0]);
        return 1;
    }    

    event_enable_debug_logging(EVENT_DBG_ALL);

    base = event_base_new();

    //event_init();
    struct evhttp_connection *evhttp_connection;
    struct evhttp_request *evhttp_request;

    //char addr[16];
    const char *host = "localhost";
    const char *remote_addr = "127.0.0.1";
    int remote_port = 80;
    const char *uri = "/";

    // lazy argument parsing
    if (argc > 1) num_conns = atoi(argv[1]);
    if (argc > 2) host = argv[2];
    if (argc > 3) remote_addr = argv[3];
    if (argc > 4) remote_port = atoi(argv[4]);
    if (argc > 5) uri = argv[5];

    if (num_conns > 65536) {
        // see http://www.mail-archive.com/libevent-users@monkey.org/msg01302.html
        // evhttp_connection_set_local_address can be used to send from different
        // local addresses.
        fprintf(stderr, "%s: connection count too high (>65536)\n", argv[0]);
        exit(1);
    }

    printf("Making %d connections to http://%s:%d%s Host: %s\n", num_conns, remote_addr, remote_port, uri, host);

    int i;

    for (i=1;i<=num_conns;i++) {
        evhttp_connection = evhttp_connection_base_new(base, NULL, remote_addr, remote_port);
        evhttp_set_timeout((struct evhttp *)evhttp_connection, 864000); // 10 day timeout
        evhttp_request = evhttp_request_new(reqcb, NULL);
        evhttp_request->chunk_cb = chunkcb;
        evhttp_add_header(evhttp_request->output_headers, "Host", host);
        evhttp_add_header(evhttp_request->output_headers, "Connection", "close");
        evhttp_make_request(evhttp_connection, evhttp_request, EVHTTP_REQ_GET, uri);
        connected++;
        if ( i % 100 == 0)
            printf("%d requests sent (%d connected)\n", i, connected-closed);
        evhttp_connection_set_timeout(evhttp_request->evcon, 864000);
        event_base_loop(base, EVLOOP_NONBLOCK); // EVLOOP_NONBLOCK is set, then the loop will not wait 
                                                // for events to trigger: it will only check whether any 
                                                // events are ready to trigger immediately, and run 
                                                // their callbacks if so.
        usleep(SLEEP_MS*1000);
    }

    printf("All %d requests sent (%d connected).\n", num_conns, connected);

    event_base_dispatch(base);


    printf("All connections are closed.\n");
    printf("connections: %d\tBytes: %d\tChunks: %d\tClosed: %d\n", num_conns, bytes_recvd, chunks_recvd, closed);
    printf("Completed: %d\tFailed: %d\n", responses_completed_ok, num_conns-chunks_recvd);
    printf("Max concurrency: %d\n", max_concurrency);

    
    event_base_free(base);

    return 0;
}

// on Mac OS X 10.8.2
// sudo port install libevent
// export CC=/usr/bin/gcc; gcc -o c10k-test-client -l event -I /opt/local/include/ -L /opt/local/lib/ c10k-test-client.c
