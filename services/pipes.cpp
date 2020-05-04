/* -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 99; -*- */
/* vim: set ts=4 sw=4 et tw=99:  */
/*
 * distcc -- A simple distributed compiler system
 *
 * Copyright (C) 2002, 2003 by Martin Pool <mbp@samba.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#include "pipes.h"

#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <fcntl.h>
#ifdef _WIN32
#include <WinSock2.h>
#include <string>
#else
#include <sys/socket.h>
#endif

// Create pipe with a larger buffer, this saves context switches and latencies.
int create_large_pipe( SocketWrapper pipefds[ 2 ] )
{
    // On Linux, the situation seems to be as follows:
    // SO_SNDBUFFORCE should allow the largest size, but it requires
    // the CAP_NET_ADMIN privilege and on Linux we normally run without
    // privileges (except for chroot). The F_SETPIPE_SZ defaults to 1MiB
    // max size, so if available, it should be the second one.
    // The maximum for SO_SNDBUF seems to be just 256KiB, so that one goes
    // last.
#ifdef F_SETPIPE_SZ
    int pfds[2]{ pipefds[0].socket(), pipfds[1].socket() };
    if( pipe(pfds) == 0 )
    {
        fcntl(pfds[ 1 ], F_SETPIPE_SZ, 1048576 );
        // Do not bother checking if this succeeded.
        return 0;
    }
#endif
    // We use a socket pair instead of a pipe to get a "slightly" bigger
    // output buffer. This saves context switches and latencies.
    int ret = SocketWrapper::SocketPair(AF_UNIX, SOCK_STREAM, 0, pipefds);
    if (ret < 0)
        return ret;


    int maxsize = 2 * 1024 * 2024;
#ifdef SO_SNDBUFFORCE
    if (pipefds[1].SetSockOpt(SOL_SOCKET, SO_SNDBUFFORCE, &maxsize, sizeof(maxsize)) < 0)
#endif
    {
        pipefds[1].SetSockOpt(SOL_SOCKET, SO_SNDBUF, reinterpret_cast<char*>(&maxsize), sizeof(maxsize));
    }
    return 0;
}
