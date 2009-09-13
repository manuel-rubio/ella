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
#include "ella/binding.h"
#include "ella/string.h"
#include "ella/memory.h"
#include "ella/date.h"
#include "ella/logger.h"

#ifdef EWS_OS_INCLUDE
#include EWS_OS_INCLUDE
#endif

#endif