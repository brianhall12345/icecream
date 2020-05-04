/* -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 99; -*- */
/* vim: set ts=4 sw=4 et tw=99:  */
/*
    This file is part of Icecream.

    Copyright (c) 2020 Brian Hall <brianhall12345@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#ifndef SOCKETWRAPPER_H
#define SOCKETWRAPPER_H

#ifdef __linux__
#  include <stdint.h>
#endif
#include <sys/types.h>
#ifdef _WIN32
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <time.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif

class SocketWrapper
{
public:
    enum
    {
#if _WIN32
        BAD_SOCKET = INVALID_SOCKET
#else
        BAD_SOCKET = -1
#endif
    };
    
    SocketWrapper();
    
    int Recv(char* buf, int len, int flags);
    int RecvFrom(char* buf, int len, int flags, sockaddr* from, int* fromlen);
    int Send(const char* buf, int len, int flags);
    int SendTo(const char* buf, int len, int flags, const sockaddr* to, int tolen);
    int Connect(const sockaddr* name, int namelen);
    int Create(int af, int type, int protocol);
    bool Close();
    int Bind(const sockaddr* name, int namelen);
    int GetSockName(sockaddr* name, int* namelen);
    int Listen(int backlog);
    static int Select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, const timeval* timeout);
    int SetSockOpt(int level, int optname, const char* optval, int optlen);
    int SetBlocking(bool shouldblock);
    int SetCloseOnExec(bool shouldclose);

#if _WIN32
    SOCKET socket() const { return this->_socket; }
    bool IsValid() const { return socket() != SocketWrapper::BAD_SOCKET; }
    static SocketWrapper InvalidSocket(SOCKET err = SocketWrapper::BAD_SOCKET) { return SocketWrapper(err); }
#else
    int socket() const { return this->_socket; }
    bool IsValid() const { return !(socket() < 0); }
    static SocketWrapper InvalidSocket(int err = ) { return SocketWrapper(err); }
#endif
    void Invalidate() { _socket = SocketWrapper::BAD_SOCKET; }

    static int SocketPair(int domain, int type, int protocol, SocketWrapper sv[2]);

private:
#if _WIN32
    // WinSock only allows for bad sockets being INVALID_SOCKET so we can't store a different bad value here
    SocketWrapper(SOCKET set) : _socket(SocketWrapper::BAD_SOCKET) {}
#else
    SocketWrapper(int set) : _socket(set) {}
#endif

#if _WIN32
    SOCKET _socket;
#else
    int _socket;
#endif
};
#endif