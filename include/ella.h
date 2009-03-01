/* -*- mode:C; coding:utf-8 -*- */

#if !defined __EWS_H
#define __EWS_H

#include <signal.h>
#include <poll.h>
#include <rpc/rpc.h>
#include <ctype.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>

#include "config.h"
#include "configurator.h"
#include "connector.h"
#include "string.h"
#include "header.h"
#include "modules.h"
#include "memory.h"
#include "cli.h"
#include "date.h"
#include "logger.h"

//!< definitions
#define BUFFER_SIZE           1024
#define PAGE_SIZE          2097152
#define PAGE_XXL_SIZE     10485760

extern char bindThreadExit;  //!< switch to keep server running.

#endif
