/* gcc -g rcproku.c $(pkg-config --cflags --libs avahi-client) -lpthread -o rcproku
 *
 * RCP/Roku SoundBridge emulator for finite cases
 * for testing clients and responses to errors/lockups
 */

#include <stdio.h>
#include <stdlib.h>

#include <signal.h>


#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <linux/if_link.h>
#include <net/if.h>

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <strings.h>

#include <stdbool.h>

#include <time.h>
#include <libgen.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>

#include <avahi-client/client.h>
#include <avahi-client/publish.h>

#include <avahi-common/alternative.h>
#include <avahi-common/simple-watch.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>
#include <avahi-common/timeval.h>


/* should be in <netinet/in.h> */
#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN  16
#endif


const unsigned  RPC_ROKU_PORT = 5555;

extern char*  optarg;
extern int    optind;

static struct {
    int  sig;
    bool  keepgoing;

    AvahiEntryGroup *group;
    AvahiSimplePoll *simple_poll;
    char *svc_name;

    pthread_t  tid;
}
_theglobs = { 0 };


static void mdns_create_svc(AvahiClient *c);

static void mdns_entry_group_cb(AvahiEntryGroup *g, AvahiEntryGroupState state, void *userdata)
{
    assert(g == _theglobs.group || _theglobs.group == NULL);
    _theglobs.group = g;

    /* Called whenever the entry group state changes */

    switch (state)
    {
	case AVAHI_ENTRY_GROUP_ESTABLISHED:
	    /* The entry group has been established successfully */
	    //fprintf(stderr, "Service '%s' successfully established.\n", _theglobs.svc_name);
	    break;

	case AVAHI_ENTRY_GROUP_COLLISION:
	    {
		char *n;

		/* A service name collision with a remote service
		 * happened. Let's pick a new name */
		n = avahi_alternative_service_name(_theglobs.svc_name);
		avahi_free(_theglobs.svc_name);
		_theglobs.svc_name = n;

		fprintf(stderr, "Service name collision, renaming service to '%s'\n", _theglobs.svc_name);

		/* And recreate the services */
		mdns_create_svc(avahi_entry_group_get_client(g));
		break;
	    }

	case AVAHI_ENTRY_GROUP_FAILURE:
	    fprintf(stderr, "Entry group failure: %s\n", avahi_strerror(avahi_client_errno(avahi_entry_group_get_client(g))));

	    /* Some kind of failure happened while we were registering our services */
	    avahi_simple_poll_quit(_theglobs.simple_poll);
	    break;

	case AVAHI_ENTRY_GROUP_UNCOMMITED:
	case AVAHI_ENTRY_GROUP_REGISTERING:
	    ;
    }
}

struct if_idx {
    char*  name;
    unsigned  idx;

    struct if_idx*  next;
};

void  free_if_idx(struct if_idx* l_)
{
    struct if_idx*  l = l_;
    while (l)
    {
	free(l->name);
	l = l->next;
    }
    free(l_);
}

struct if_idx* ip4_ifidx()
{
    struct ifaddrs *ifaddr, *ifa;
    int family, n;

    struct if_idx*  l = NULL;
    struct if_idx*  next = NULL;

    if (getifaddrs(&ifaddr) < 0) {
	printf("failed getifaddr - %s\n", strerror(errno));
	return NULL;
    }

    // find all non loopback devices with an IPv4 address
    for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++)
    {
	if (ifa->ifa_addr == NULL)
	    continue;

	family = ifa->ifa_addr->sa_family;
	if (family != AF_INET ||  strcmp(ifa->ifa_name, "lo") == 0) {
	    continue;
	}

	if (l == NULL) {
	    l = calloc(1, sizeof(struct if_idx));
	}
	else {
	    next = calloc(1, sizeof(struct if_idx));
	    next->next = l;
	    l = next;
	}

	l->name = strdup(ifa->ifa_name); 
	l->idx  = if_nametoindex(ifa->ifa_name);
    }
    freeifaddrs(ifaddr);

    return l;
}

static void mdns_create_svc(AvahiClient *c)
{
    char *n;
    int ret;
    assert(c);

    /* If this is the first time we're called, let's create a new
     * entry group if necessary */

    if (!_theglobs.group)
	if (!(_theglobs.group = avahi_entry_group_new(c, mdns_entry_group_cb, NULL))) {
	    fprintf(stderr, "avahi_entry_group_new() failed: %s\n", avahi_strerror(avahi_client_errno(c)));
	    goto fail;
	}

    /* If the group is empty (either because it was just created, or
     * because it was reset previously, add our entries.  */

    struct if_idx*  m = NULL;
    if (avahi_entry_group_is_empty(_theglobs.group))
    {
	struct if_idx*  l = ip4_ifidx();
	if (l == NULL) {
	    l = calloc(1, sizeof(struct if_idx));
	    l->name = strdup("all");
	    l->idx = AVAHI_IF_UNSPEC;
	};
	m = l;

	while (l)
	{
	    printf("adding service '%s' on interface '%s' (#%d)\n", _theglobs.svc_name, l->name, l->idx);
	    if ((ret = avahi_entry_group_add_service(_theglobs.group, l->idx, AVAHI_PROTO_INET, 0,
			    _theglobs.svc_name, "_roku-rcp._tcp", NULL, NULL, RPC_ROKU_PORT, NULL, NULL, NULL)) < 0) {

		if (ret == AVAHI_ERR_COLLISION)
		    goto collision;

		fprintf(stderr, "Failed to add service: %s\n", avahi_strerror(ret));
		goto fail;
	    }

	    /* Tell the server to register the service */
	    if ((ret = avahi_entry_group_commit(_theglobs.group)) < 0) {
		fprintf(stderr, "Failed to commit entry group: %s\n", avahi_strerror(ret));
		goto fail;
	    }

	    l = l->next;
	}
    }
    free_if_idx(m);

    return;

collision:
    free_if_idx(m);

    /* A service name collision with a local service happened. Let's
     * pick a new name */
    n = avahi_alternative_service_name(_theglobs.svc_name);
    avahi_free(_theglobs.svc_name);
    _theglobs.svc_name = n;

    fprintf(stderr, "Service name collision, renaming service to '%s'\n", _theglobs.svc_name);

    avahi_entry_group_reset(_theglobs.group);

    mdns_create_svc(c);
    return;

fail:
    avahi_simple_poll_quit(_theglobs.simple_poll);
}

static void mdns_cb(AvahiClient *c, AvahiClientState state, void * userdata)
{
    assert(c);

    /* Called whenever the client or server state changes */

    switch (state) {
	case AVAHI_CLIENT_S_RUNNING:

	    /* The server has startup successfully and registered its host
	     * name on the network, so it's time to create our services */
	    mdns_create_svc(c);
	    break;

	case AVAHI_CLIENT_FAILURE:

	    fprintf(stderr, "Client failure: %s\n", avahi_strerror(avahi_client_errno(c)));
	    avahi_simple_poll_quit(_theglobs.simple_poll);

	    break;

	case AVAHI_CLIENT_S_COLLISION:

	    /* Let's drop our registered services. When the server is back
	     * in AVAHI_SERVER_RUNNING state we will register them
	     * again with the new host name. */

	case AVAHI_CLIENT_S_REGISTERING:

	    /* The server records are now being established. This
	     * might be caused by a host name change. We need to wait
	     * for our own records to register until the host name is
	     * properly esatblished. */

	    if (_theglobs.group)
		avahi_entry_group_reset(_theglobs.group);

	    break;

	case AVAHI_CLIENT_CONNECTING:
	    ;
    }
}


struct mdns_spt_args {
    const char* svc_name;
    int  ret;
};

static void*  mdns_svc_pub_thread(void* args_)
{
    struct mdns_spt_args*  args = (struct mdns_spt_args*)args_;

    AvahiClient *client = NULL;
    int error;

    args->ret = 1;

    if (!(_theglobs.simple_poll = avahi_simple_poll_new())) {
	fprintf(stderr, "Failed to create simple poll object.\n");
	goto fail;
    }

    _theglobs.svc_name = avahi_strdup(args->svc_name);

    client = avahi_client_new(avahi_simple_poll_get(_theglobs.simple_poll), 0, mdns_cb, NULL, &error);

    /* Check wether creating the client object succeeded */
    if (!client) {
	fprintf(stderr, "Failed to create client: %s\n", avahi_strerror(error));
	goto fail;
    }

    avahi_simple_poll_loop(_theglobs.simple_poll);

    args->ret = 0;

fail:
    if (client)
	avahi_client_free(client);

    if (_theglobs.simple_poll)
	avahi_simple_poll_free(_theglobs.simple_poll);

    avahi_free(_theglobs.svc_name);
    _theglobs.svc_name = NULL;

    return args;
}


static void  mdns_svc_pub(struct mdns_spt_args* args_)
{
    sigset_t  sigs, osigs;

    sigfillset(&sigs);
    pthread_sigmask(SIG_SETMASK, &sigs, &osigs);
    pthread_create(&_theglobs.tid, NULL, mdns_svc_pub_thread, args_);
    pthread_sigmask(SIG_SETMASK, &osigs, NULL);
}

static void  mdns_svc_unpub()
{
    if (_theglobs.simple_poll == NULL) {
	printf("no avahi loop, no shutdown required\n");
	return;
    }

    avahi_simple_poll_quit(_theglobs.simple_poll);
    pthread_join(_theglobs.tid, NULL);
}


struct stream_args {
    pthread_t  tid;
    pthread_cond_t   c;
    pthread_mutex_t  m;

    int  sock;  // also notif mechanism
    const char*  url;
    bool  with_icy;
    bool  go;
    size_t  bytes;
};

#define STREAM_READ		1
#define STREAM_READ_WITH_ICY	(STREAM_READ | 2)


static void* stream_thread(void* args_)
{
    struct stream_args*  args = (struct stream_args*)args_;
    ssize_t  n = 0;
    size_t  N = 0;
    
    int  e;

    printf("stream: thread starting\n");

    const char*  get_req = "GET /%s HTTP/1.1\r\nHost: %s:%d%s\r\n\r\n";
    const char*  icy_hdr = "\r\nIcy-MetaData: 1";
   
    while (args->go)
    {
	struct timespec  t2w = { 0 };
	char  stream_ip4[INET_ADDRSTRLEN] = { 0 };
	unsigned  stream_port = 0;
	const char*  stream_page = NULL;

	// wait on the signal 'url' at which point SetWorkingInfo ... has been
	// called AND QueueAndPlayOne to kick off
	pthread_mutex_lock(&args->m);
	{
	    while (args->go && args->url == NULL) {
		t2w.tv_sec = time(NULL) + 2;
		pthread_cond_timedwait(&args->c, &args->m, &t2w);
	    }
	}
	pthread_mutex_unlock(&args->m);
	if (!args->go) {
	    break;
	}

	// get the bits to connect
	{
	    const char*  p = NULL;
	    const char*  q = NULL;

	    if (strncmp(args->url, "http://", 7) == 0) {
		p = args->url+7;
	    }
	    else if (strncmp(args->url, "https://", 8) == 0) {
		p = args->url+8;
	    }
	    
	    if (p == NULL) {
		printf("stream: invalid URL provided '%s'\n", args->url);
		goto bad_args;
	    }


	    /// max ip4 len 15 xxx.xxx.xxx.xxx
	    q = strchr(p, ':');
	    if (q == NULL || (q && q-p > sizeof(stream_ip4)-1) )
		goto bad_http_args;

	    strncpy(stream_ip4, p, q-p);
	    p = q+1;

	    if ( (e = sscanf(p, "%d/", &stream_port)) != 1) {
		goto bad_http_args;
	    }
	    q = strchr(p, '/');
	    if (q == NULL || q && strlen(q) < 2) {
bad_http_args:
		printf("stream: unexpected streaming URL '%s' - unable to parse <ip4>:<port>/stream.mp3\n", args->url);
		goto bad_args;
	    }
	    stream_page = q+1;
	    //printf("stream: ip4='%s' port=%hd stream='%s'\n", stream_ip4, stream_port, stream_page);
	}

	struct sockaddr_in  dsa = { 0 };
	dsa.sin_family = AF_INET;
	dsa.sin_port   = htons(stream_port);

	pthread_mutex_lock(&args->m);
	{
	    if (inet_pton(AF_INET, stream_ip4, &dsa.sin_addr) < 0) {
		printf("stream: failed to map stream ip4 address for streaming data - %s\n", strerror(errno));
		goto out;
	    }

	    if ( (args->sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("stream: failed to create socket for streaming data - %s\n", strerror(errno));
		goto out;
	    }

	    if ( (e = connect(args->sock, (struct sockaddr*)&dsa, sizeof(dsa))) < 0) {
		printf("stream: failed to create socket for streaming data - %s\n", strerror(errno));
		goto out;
	    }
	}
	pthread_mutex_unlock(&args->m);

	char*  get_buf = malloc(strlen(get_req) + INET_ADDRSTRLEN + 1 + 6 + strlen(icy_hdr) + strlen(stream_page) +1);
	sprintf(get_buf, get_req, stream_page, stream_ip4, stream_port, args->with_icy ? icy_hdr : "");

	if ( (n = send(args->sock, get_buf, strlen(get_buf), 0)) < 0) {
	    printf("failed to request GET - %s\n", strerror(errno));
	    free(get_buf);
	    goto bad_args;
	}
	free(get_buf);

	printf("stream: connected, reading stream\n");

	// forever read data and throw away.. there may be no data (because the
	// server not currently generating a stream) but the blocking read is fine
	char  buf[8192];
	while (args->go)
	{
	    n = recv(args->sock, buf, sizeof(buf), MSG_DONTWAIT);
	    if (n <= 0)
	    {
		if (errno == EINTR) {
		    // unexpected but go again
		    continue;
		}

		if (errno ==  EAGAIN || errno == EWOULDBLOCK) {
		    struct timeval  tv = { 1, 0 };
		    fd_set  readfds;
		    FD_ZERO(&readfds);
		    FD_SET(args->sock, &readfds);

		    select(args->sock+1, &readfds, NULL, NULL, &tv);

		    continue;
		}

		if (n < 0 && errno == EBADF || n == 0 && errno == ECONNRESET) {
		    //printf("stream: remote stream closed\n");
		}
		else {
		    printf("stream: failed to read from remote stream - %s\n", strerror(errno));
		}
		break;
	    }
	    N += n;

	    if (N > sizeof(buf)) {
		putchar('.');
		fflush(stdout);
		N = 0;
	    }
	    args->bytes += n;
	}

	// printf("stream: closing stream\n");
bad_args:
	pthread_mutex_lock(&args->m);
out:
	{
	    close(args->sock);
	    args->url = NULL;
	    args->sock = -1;
	}
	pthread_cond_broadcast(&args->c);
	pthread_mutex_unlock(&args->m);
    }


    pthread_mutex_lock(&args->m);
    {
	close(args->sock);
	args->sock = -1;
    }
    pthread_cond_broadcast(&args->c);
    pthread_mutex_unlock(&args->m);

    printf("stream: thread done\n");

    return args;
}

static void  stream_thread_start(struct stream_args* args_)
{
    if (args_->tid > 0) {
	return;
    }
    sigset_t  sigs, osigs;

    args_->go = true;

    sigfillset(&sigs);
    pthread_sigmask(SIG_SETMASK, &sigs, &osigs);
    pthread_create(&args_->tid, NULL, stream_thread, args_);
    pthread_sigmask(SIG_SETMASK, &osigs, NULL);
}

static void  stream_thread_stop(struct stream_args* args_)
{
    if (args_->tid <= 0) {
	return;
    }

    pthread_mutex_lock(&args_->m);
 
    // the thread once started is just reading... close the sock will cause the
    // syscall to fail
    close(args_->sock);
    args_->sock = -1;
    args_->go = false;

    pthread_cond_signal(&args_->c);
    pthread_mutex_unlock(&args_->m);

    pthread_join(args_->tid, NULL);
}


static void  _sighandler(const int sig_)
{
    _theglobs.sig = sig_;
}

static void  _sigprocessor()
{
    switch (_theglobs.sig)
    {
	case 0:
	    return;

	case SIGHUP:
	case SIGTERM:
	case SIGINT:
	{
	    printf("recv shutdown signal %d\n", _theglobs.sig);
	    _theglobs.keepgoing = false;

	} break;

	case SIGPIPE:
	{
	    printf("recv SIGPIPE (SO_KEEPALIVE on?)\n");
	} break;

	default:
	{
	    printf("recv unhandled signal %d\n", _theglobs.sig);
	    _theglobs.keepgoing = false;
	}
    }
}



static const int   _thesigs[] = { SIGPIPE, SIGINT, SIGTERM, SIGHUP, SIGPIPE, -1 };

static void  _installSignalHdlers()
{
    struct sigaction  act, oact;

    memset(&act,  0, sizeof(struct sigaction));
    memset(&oact, 0, sizeof(struct sigaction));

    act.sa_handler = _sighandler;
    sigemptyset(&act.sa_mask);

    const int*  p = _thesigs;
    while (*p != -1) {
	sigaddset(&act.sa_mask, *p++);
    }

#if 0
    /* sysv systems NEVER restart their sys calls that are interupted by signals
     * so we got to do it -- ie dont get EINTR
     */
    act.sa_flags |= SA_RESTART;
#endif

    p = _thesigs;
    while (*p != -1) {
        sigaction(*p++, &act, NULL);
    }
}

static bool _valid_token(const char* tok_, const char* list_[], struct iovec* iov_)
{
    const char**  ptr = NULL;
    if (*tok_) {
	ptr = list_;
	while (*ptr) {
	    if (strstr(tok_, *ptr)) {
		break;
	    }
	    ++ptr;
	}
    }

    if (ptr == NULL | (*ptr && *ptr == NULL)) {
	iov_->iov_base = (void*)": ParameterError\r\n";
	iov_->iov_len  = strlen(": ParameterError\r\n");
	return false;
    }
    return true;
}



static struct {
    bool  keepalive;
    bool  fakeerrors;
    unsigned delay;
    unsigned stop_delay;
    unsigned disconnect_delay;
    bool  disconnect_err;
    unsigned  read_stream;
} _theopts = {
    false, false, 0, 0, false, 0
};



int main(int argc, char* argv[])
{
    const char*  argv0 = basename(argv[0]);

    struct mdns_spt_args  args = { 0 };
    struct stream_args  stream_args = { 0 };
    args.svc_name = "RCP/SoundBridge Test Harness";

    char  c;
    while ( (c = getopt(argc, argv, "hkfLt:T:D:n:rR")) != EOF)
    {
        switch (c) {
            case 'k':  _theopts.keepalive = true;  break;
            case 'f':  _theopts.fakeerrors = true;  break;
            case 't':  _theopts.delay = atoi(optarg);  break;
            case 'T':  _theopts.stop_delay = atoi(optarg);  break;
            case 'D':  _theopts.disconnect_delay = atoi(optarg);  break;
            case 'L':  _theopts.disconnect_err = true;  break;
            case 'n':  args.svc_name = optarg;  break;
            case 'r':  _theopts.read_stream = STREAM_READ;  break;
            case 'R':  _theopts.read_stream = STREAM_READ_WITH_ICY;  break;

            case 'h':
            default:
usage:
                fprintf(stderr, "usage: %s [-k] [-f] [-t <secs>] [-T <secs>] [-L] [-n <mDNS svc name> [-r|-R]\n"
			        "RCP/Roku Soundbridge server emulator\n"
			        "       -k  keep alive\n"
				"    Test client handling\n"
				"       -n  mDNS svc name\n"
				"       -f  fake errors on responses\n"
				"       -t  delay on setup responses\n"
				"       -T  delay on shutdown responses\n"
				"       -L  send lockup respnse on disconnect\n",
				"       -r|-R request data from handshake playlistURL stream\n",
				argv0);
                return 1;
        }
    }


    time_t  t;
    time(&t);

    printf(": will listen on port: %d - started %s", RPC_ROKU_PORT, ctime(&t));
    if (_theopts.fakeerrors) {
	printf(": will fake errors\n");
    }

    _theglobs.keepgoing = true;

 
    struct sockaddr_in  servaddr;
    bzero(&servaddr, sizeof(struct sockaddr));

    servaddr.sin_family      = AF_INET;
    servaddr.sin_port        = htons(RPC_ROKU_PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    _installSignalHdlers();

    int  srv;
    if ( (srv = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, ": unable to get socket - %s\n", strerror(errno));
        return 3;
    }
    unsigned  bindattmpts = 0;
    while (_theglobs.keepgoing)
    {
	int  r = 1;
	if (setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, (const char*)&r, sizeof(r)) < 0) {
	    fprintf(stderr, ": unable to force re-use of socket - %s\n", strerror(errno));
	}

	if (bind(srv, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
	{
	    fprintf(stderr, "unable to bind to addr - %s .  ", strerror(errno));
	    if (errno == EADDRINUSE)
	    {
	        /* maybe stuck in TIME_WAIT
	         */
	        if (++bindattmpts > 2)
	        {
		    if (bindattmpts == 3) 
		    {
			/* waited 30 secs...
			 */

			fprintf(stderr, "reached max attempts for bind, attempt forcable reuse of socket\n");

			int  r = 1;
			if (setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, (const char*)&r, sizeof(r)) < 0) {
			    fprintf(stderr, ": unable to force re-use of socket - %s\n", strerror(errno));
			    return 4;
			}
		    }
		    else {
		        printf("all attempts failed at binding socket\n");
		        return 4;
		    }
	        }
	        else {
		    printf("will attempt to wait...\n");
		    sleep(10);
	        }
	    }
	    else {
		putchar('\n');
		return 4;
	    }
	}
	else {
	    break;
	}
    }

    if (_theopts.keepalive) {
	printf("enabling SO_KEEPALIVE\n");
	int  r = 1;
	if (setsockopt(srv, SOL_SOCKET, SO_KEEPALIVE, (const char*)&r, sizeof(r)) < 0) {
	    fprintf(stderr, ": uanble to set SO_KEEPALIVE - %s\n", strerror(errno));
	    return 4;
	}
    }

    if (listen(srv, 5) < 0) {
        fprintf(stderr, ": unable to listen on socket - %s\n", strerror(errno));
        return 5;
    }

    mdns_svc_pub(&args);
    if (_theopts.read_stream) {
	stream_thread_start(&stream_args);
    }

    char  buf[8192];
    char  cmd[128];
    char  stream_url[128];


    uint64_t  xfers_in  = 0;
    uint64_t  xfers_out = 0;
    unsigned  last_bytes = 0;
    unsigned  ttl_conx = 0;
    while (_theglobs.keepgoing)
    {
        struct sockaddr_in  cliaddr;
        socklen_t  clilen = sizeof(cliaddr);
	char  addrbuf[INET_ADDRSTRLEN+1] = { 0 };
	bool  full_init = false;

        printf("waiting client connection...\n");
        int  c;
        if ( (c = accept(srv, (struct sockaddr*)&cliaddr, &clilen)) < 0)
        {
            if (errno == EINTR) {
                _sigprocessor();
                continue;
	    }
	    else {
		fprintf(stderr, ": accept failed - %s\n", strerror(errno));
		return 6;
            }
        }

        printf("connection %2d from %s:%d\n", ++ttl_conx, inet_ntop(AF_INET, &cliaddr.sin_addr, addrbuf, INET_ADDRSTRLEN), ntohs(cliaddr.sin_port));

	size_t  r = 0;
	size_t  w = 0;
	const char** ptr;

	unsigned short  vol = (unsigned short)rand()%100;

	w += write(c, "roku: ready\r\n", strlen("roku: ready\r\n"));
	bool  done = false;
	while (!done)
	{
	    _sigprocessor();
	    if (!_theglobs.keepgoing) {
		break;
	    }

	    ssize_t  n = 0;
	    memset(buf, 0, sizeof(buf));
	    if ( (n = read(c, buf, sizeof(buf))) < 0) {
		if (errno == EINTR) {
		    continue;
		}

		fprintf(stderr, ": unable to read - %s\n", strerror(errno));
		done = true;
		continue;
	    }

	    if (n == 0) {
		// socket closed
		printf("remote disconnect\n");
		done = true;
	    }
	    else
	    {
		++xfers_in;
		r+= n;

		printf("  > %s", buf);
		if (buf[n-1] != '\n') {
		    putchar('\n');
		}

		memset(cmd, 0, sizeof(cmd));
		strncpy(cmd, buf, sizeof(cmd)-1);

		char*  tok;
		const char*  p = strtok_r(cmd, " ", &tok);
		if (*tok == NULL) {
		    char* p1 = tok -1;
		    while (*p1 == '\r' || *p1 == '\n') {
			*p1-- = NULL;
		    }
		}

		struct iovec  iov[] = { 
		    { (void*)"  < ", 4 },
		    { cmd, strlen(cmd) },
		    { (void*)": OK\r\n", strlen(": OK\r\n")}
		};

		// for multi responses
		struct iovec*  iovm = NULL;
		unsigned iovm_sz = 0;

		if (_theopts.fakeerrors && rand()%5 == 0) {
		    printf("---> error <---\n");
		    goto unknown;
		}


		if (strcmp(cmd, "Reboot") == 0)
		{
		    printf("---> dropping client <---\n");
		    close(c);
		    sleep(2);
		    break;
		}

		if (*tok == NULL &&
		    (strcmp(cmd, "GetConnectedServer") == 0 ||
		    strcmp(cmd, "ClearWorkingSong"   ) == 0))
		{ }
		else if (*tok == NULL && strcmp(cmd, "Stop") == 0) {
		    if (full_init && _theopts.stop_delay) {
			printf("---> delay <---\n");
			sleep(_theopts.stop_delay);
		    }
		}
		else if (tok && strcmp(cmd, "SetWorkingSongInfo") == 0)
		{
		    static const char*  wsi_toks[] = {
			"id", "trackLength", "year", "discNumber",
			"discCount", "trackNumber", "trackCount", "rating",
			"bpm", "startTimeMS", "endTimeMS", "volumeAdjust",
			"tunerFrequency", "compilation", "disabled", "remoteStream",
			"format", "status", "title", "artist",
			"album", "genre", "composer", "comment",
			"songFormat", "formatDescription", "playlistURL", "stationInfoURL",
			"stationInfoString", "location", "language", "url",
			"format", "bitrate", "sampleRate", "bitsPerSample",
			"numChannels", "sizeBytes", "bigEndian", "unknown",
			"unsupported", "MP3", "AAC", "AAC_DRM",
			"WAV", "AIFF", "remotePLS", "remoteM3U",
			"remoteASX", "remoteRhapsody", "WMA", "WMA_WMDRM",
			"WMA_Rhapsody", "WMA_Lossless", "LPCM", "container",
			"playlist", "AMRadio", "FMRadio", "microphone",
			NULL
		    };

		    if (_valid_token(tok, wsi_toks, &(iov[2])) )
		    {
			char*  wsi_tok;
			const char*  p = strtok_r(tok, " ", &wsi_tok);
			if (wsi_tok && _theopts.read_stream && strcmp(tok, "playlistURL") == 0)
			{
			    char*  p1 = wsi_tok + strlen(wsi_tok)-1;
			    while (p1 > wsi_tok && (*p1 == '\r' || *p1 == '\n')) {
				*p1-- = NULL;
			    }

			    if (strncmp(wsi_tok, "http://", strlen("http://")) == 0) {
				strncpy(stream_url, wsi_tok, sizeof(stream_url));
			    }
			}
		    }
		}
		else if (strcmp(cmd, "SetServerFilter") == 0)
		{
		    static const char*  filter_toks[] = {
			"daap", "upnp", "rsp", "slim",
			"radio", "flash", "linein", "am",
			"fm", "all", "debug", NULL
		    };
		    // can be multiple types, p99 on one request

		    _valid_token(tok, filter_toks, &iov[2]);
		}
		else if (strcmp(cmd, "SetPowerState") == 0)
		{
		    if (*tok &&
			strcmp(tok, "on no\r\n") == 0 ||
			strcmp(tok, "on yes\r\n") == 0 ||
			strcmp(tok, "standby\r\n") == 0)
		    { }
		    else {
			iov[2].iov_base = (void*)": ParameterError\r\n";
			iov[2].iov_len  = strlen(": ParameterError\r\n");
		    }
		}
		else if (strcmp(cmd, "QueueAndPlayOne") == 0)
		{
		    if (*tok && strcmp(tok, "working\r\n") == 0) {
			full_init = true;
			if (stream_url[0]) {
			    // restart the thread and continue
			    pthread_mutex_lock(&stream_args.m);
			    {
				stream_args.with_icy = _theopts.read_stream & STREAM_READ_WITH_ICY;
				stream_args.url = stream_url;
				pthread_cond_signal(&stream_args.c);
			    }
			    pthread_mutex_unlock(&stream_args.m);
			}
		    }
		    else
		    {
			int val;
			if (sscanf(tok, "%d", &val) != 1) {
			    iov[2].iov_base = (void*)": ParameterError\r\n";
			    iov[2].iov_len  = strlen(": ParameterError\r\n");
			}
		    }
		}
		else if (strcmp(cmd, "GetVolume") == 0) {
		    // dont care about trailing tokens
		    sprintf(buf, ": %hd\r\n", vol);
		    iov[2].iov_base = (void*)buf;
		    iov[2].iov_len  = strlen(buf);
		}
		else if (strcmp(cmd, "SetVolume") == 0) {
		    int tmpvol;
		    if (*tok == NULL || (*tok && (tmpvol = atol(tok)) < 0 || tmpvol > 100) ) {
			iov[2].iov_base = (void*)": ParameterError\r\n";
			iov[2].iov_len  = strlen(": ParameterError\r\n");
		    }
		    else {
			vol = tmpvol;
		    }
		}
// multi responses
		else if (strcmp(cmd, "ServerDisconnect") == 0)
		{
		    /*
			ServerDisconnect: TransactionInitiated
			ServerDisconnect: Disconnected
			ServerDisconnect: TransactionComplete
		    */
		    iovm_sz = 3;
		    iovm = calloc(1, sizeof(struct iovec)*(iovm_sz +1));

		    iovm[0].iov_base = iov[0].iov_base;
		    iovm[0].iov_len = iov[0].iov_len;

		    iovm[1].iov_base = _theopts.disconnect_err ? (void*)"ServerDisconnect: ResourceAllocationError\r\n" :
								 (void*)"ServerDisconnect: TransactionInitiated\r\n";
		    iovm[1].iov_len = strlen((const char*)iovm[1].iov_base);

		    iovm[2].iov_base = (void*)"ServerDisconnect: Disconnected\r\n";
		    iovm[2].iov_len = strlen((const char*)iovm[2].iov_base);

		    iovm[3].iov_base = (void*)"ServerDisconnect: TransactionComplete\r\n";
		    iovm[3].iov_len = strlen((const char*)iovm[3].iov_base);

		    if (_theopts.disconnect_delay && full_init) {
			printf("---> delay <---\n");
			sleep(_theopts.disconnect_delay);
		    }
		}
		else if (strcmp(cmd, "ListServers") == 0)
		{
		    /*
			ListServers: ListResultSize 1
			ListServers: Internet Radio
			ListServers: ListResultEnd
		    */
#if 0
		    iovm_sz = 3;
		    iovm = calloc(1, sizeof(struct iovec)*(iovm_sz +1));

		    iovm[0].iov_base = iov[0].iov_base;
		    iovm[0].iov_len = iov[0].iov_len;

		    iovm[1].iov_base = (void*)"ListServers: ListResultSize 1\r\n";
		    iovm[1].iov_len = strlen((const char*)iovm[1].iov_base);

		    iovm[2].iov_base = (void*)"ListServers: Internet Radio\r\n";
		    iovm[2].iov_len = strlen((const char*)iovm[2].iov_base);

		    iovm[3].iov_base = (void*)"ListServers: ListResultEnd\r\n";
		    iovm[3].iov_len = strlen((const char*)iovm[3].iov_base);
#else
		    iovm_sz = 2;
		    iovm = calloc(1, sizeof(struct iovec)*(iovm_sz +1));

		    iovm[0].iov_base = iov[0].iov_base;
		    iovm[0].iov_len = iov[0].iov_len;

		    iovm[1].iov_base = (void*)"ListServers: ListResultSize 1\r\nListServers: Internet Radio\r\n";
		    iovm[1].iov_len = strlen((const char*)iovm[1].iov_base);

		    iovm[2].iov_base = (void*)"ListServers: ListResultEnd\r\n";
		    iovm[2].iov_len = strlen((const char*)iovm[2].iov_base);
#endif
		}
		else if (strcmp(cmd, "ServerConnect") == 0)
		{
		    /*
			ServerConnect: TransactionInitiated
			ServerConnect: Connected
			ServerConnect: TransactionComplete
		    */
		    if (*tok && strcmp(tok, "0\r\n") == 0)
		    {
			iovm_sz = 3;
			iovm = calloc(1, sizeof(struct iovec)*(iovm_sz +1));

			iovm[0].iov_base = iov[0].iov_base;
			iovm[0].iov_len = iov[0].iov_len;

			iovm[1].iov_base = (void*)"ServerConnect: TransactionInitiated\r\n";
			iovm[1].iov_len = strlen((const char*)iovm[1].iov_base);

			iovm[2].iov_base = (void*)"ServerConnect: Connected\r\n";
			iovm[2].iov_len = strlen((const char*)iovm[2].iov_base);

			iovm[3].iov_base = (void*)"ServerConnect: TransactionComplete\r\n";
			iovm[3].iov_len = strlen((const char*)iovm[3].iov_base);
		    }
		    else {
			iov[2].iov_base = (void*)": ParameterError\r\n";
			iov[2].iov_len  = strlen(": ParameterError\r\n");
		    }
		}
		else  {
unknown:
		    // unsupported
		    iov[2].iov_base = (void*)": UnknownCommand\r\n";
		    iov[2].iov_len  = strlen(": UnknownCommand\r\n");
		}

		if (_theopts.delay && rand()%10 == 0) {
		    printf("---> delay <---\n");
		    sleep(_theopts.delay);
		}

		if (iovm)
		{
		    writev(STDOUT_FILENO, iovm, iovm_sz+1);
		    for (int i=1; i<iovm_sz+1; ++i) {
			w += writev(c, &iovm[i], 1);
			usleep(100000);
		    }
		    free(iovm);
		    xfers_out += iovm_sz;
		}
		else {
		    writev(STDOUT_FILENO, iov, 3);
		    w += writev(c, &iov[1], 2);
		    xfers_out += 2;
		}
	    }
	}
        close(c);

	if (_theopts.read_stream) {
	    pthread_mutex_lock(&stream_args.m);
	    {
		stream_args.url = NULL;
		close(stream_args.sock);  // forcible drop stream thread's connection
		stream_args.sock = -1;
		pthread_cond_broadcast(&stream_args.c);
	    }
	    last_bytes = stream_args.bytes - last_bytes;
	    pthread_mutex_unlock(&stream_args.m);
	}
	stream_url[0] = 0;

	printf("connection %s:%d  closed by %s  cmd in/out=%"PRIu64 "/%"PRIu64 " (r/w=%d/%d) streamd=%.3f Mb\n",
	       inet_ntop(AF_INET, &cliaddr.sin_addr, addrbuf, INET_ADDRSTRLEN), ntohs(cliaddr.sin_port),
	       done ? "client" : "user/signal",
	       xfers_in, xfers_out, r, w, (stream_args.bytes-last_bytes)/(1024.0*1024.0));

	if (_theglobs.sig == SIGTERM)
	    break;

	// reset and go again
	_theglobs.keepgoing = true;
	_theglobs.sig = 0;
    }
    shutdown(srv, SHUT_RDWR);
    close(srv);

    stream_thread_stop(&stream_args);
    mdns_svc_unpub();

    printf("total connections %2d  cmds in/out=%" PRIu64 "/%"PRIu64 " streamed %.3f Mb\n", ttl_conx, xfers_in, xfers_out, stream_args.bytes/(1024.0*1024.0));

    return 0;
}
