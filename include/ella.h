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

#include "ella/config.h"
#include "ella/configurator.h"
#include "ella/connector.h"
#include "ella/string.h"
#include "ella/header.h"
#include "ella/modules.h"
#include "ella/memory.h"
#include "ella/cli.h"
#include "ella/date.h"
#include "ella/logger.h"

#ifdef EWS_OS_INCLUDE
#include EWS_OS_INCLUDE
#endif

//!< definitions
#define BUFFER_SIZE           1024
#define PAGE_SIZE          2097152
#define PAGE_XXL_SIZE     10485760

extern char bindThreadExit;  //!< switch to keep server running.

#endif
