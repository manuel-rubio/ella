/* -*- mode:C; coding:utf-8 -*- */

struct Binding {
    char host[80];               //!< IP address to bind requests.
    int port;                    //!< port to bind requests.
    pthread_t thread;            //!< server thread.
	int kill_flag;               //!< flag for kill binding.
	struct Binding *next;
};

void ews_binding_add( const char *host, int port );

void ews_binding_remove( const char *host, int port );
