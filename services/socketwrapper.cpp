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

#include <socketwrapper.h>

SocketWrapper::SocketWrapper() : _socket(SocketWrapper::BAD_SOCKET) 
{
}

int SocketWrapper::Recv(char* buf, int len, int flags)
{
    return recv(socket(), buf, len, flags);
}

int SocketWrapper::RecvFrom(char* buf, int len, int flags, sockaddr* from, int* fromlen)
{
    return recvfrom(socket(), buf, len, flags, from, fromlen);
}

int SocketWrapper::Send(const char* buf, int len, int flags)
{
    return send(socket(), buf, len, flags);
}

int SocketWrapper::SendTo(const char* buf, int len, int flags, const sockaddr* to, int tolen)
{
    return sendto(socket(), buf, len, flags, to, tolen);
}

bool SocketWrapper::Close()
{
#if _WIN32
    if (closesocket(socket()) < 0 && (WSAGetLastError() != WSAENOTSOCK))
    {
        return false;
    }
#else
    result = ;
    if (close(socket()) < 0 && (errno != EBADF))
    {
        return false;
    }
#endif
    _socket = SocketWrapper::BAD_SOCKET;
    return true;
}

int SocketWrapper::SetSockOpt(int level, int optname, const char* optval, int optlen)
{
    return setsockopt(socket(), level, optname, optval, optlen);
}

int SocketWrapper::SetBlocking(bool shouldblock)
{
#if _WIN32
#define O_NONBLOCK 1 // any non zero status is non-blocking
    u_long  iMode = shouldblock ? 0 : O_NONBLOCK;
    return ioctlsocket(socket(), FIONBIO, &iMode);
#else
    return fcntl(socket(), F_SETFL, shouldblock ? 0 : O_NONBLOCK);
#endif
}

int SocketWrapper::Connect(const sockaddr* name, int namelen)
{
    return connect(socket(), name, namelen);
}

int SocketWrapper::Create(int af, int type, int protocol)
{
    _socket = ::socket(af, type, protocol);
#if _WIN32
    if (_socket == INVALID_SOCKET)
        return -1;
#else
    if (_socket < 0)
        return -1;
#endif
    return 0;
}

int SocketWrapper::SetCloseOnExec(bool shouldclose)
{
#if _WIN32
#pragma message("TODO: Determine if there is an analog to FD_CLOEXEC/SOCK_CLOEXEC. The internets something about O_NOINHERIT, but it is unclear how to use it with sockets.")
    return 0;
#else
    return fcntl(fd, F_SETFD, FD_CLOEXEC) < 0;
#endif
}

int SocketWrapper::Bind(const sockaddr* name, int namelen)
{
    return bind(socket(), name, namelen);
}

int SocketWrapper::GetSockName(sockaddr* name, int* namelen)
{
    return getsockname(socket(), name, namelen);
}

int SocketWrapper::Listen(int backlog)
{
    return listen(socket(), backlog);
}

int SocketWrapper::Select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, const timeval* timeout)
{
    return select(nfds, readfds, writefds, exceptfds, timeout);
}


int SocketWrapper::SocketPair(int domain, int type, int protocol, SocketWrapper sv[2])
{
#if _WIN32
    int default_port = 0;

    // Create a non - blocking temporary server socket
    SocketWrapper srv_sock;
    srv_sock.Create(domain, type, protocol);
    srv_sock.SetBlocking(false);
    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr("127.0.0.1");
    service.sin_port = htons(default_port);
    srv_sock.Bind(reinterpret_cast<SOCKADDR*>(&service), sizeof(service));
    sockaddr_in bound_service;
    int boound_service_size;
    srv_sock.GetSockName(reinterpret_cast<SOCKADDR*>(&bound_service), &boound_service_size);
    srv_sock.Listen(1);

    // Create non - blocking client socket
    SocketWrapper client_sock;
    client_sock.Create(domain, type, protocol);
    client_sock.SetBlocking(false);
    int ret = client_sock.Connect(reinterpret_cast<SOCKADDR*>(&bound_service), sizeof(bound_service));
    if (ret != 0 && WSAGetLastError() != WSAEWOULDBLOCK)
    {
        return -1; // failure
    }

    // Use select to wait for connect() to succeed.
    timeval timeout = { 1, 0 };
    fd_set readable_set;
    readable_set.fd_count = 1;
    readable_set.fd_array[0] = srv_sock.socket();
    int readable = SocketWrapper::Select(0 /* ignored */, &readable_set, nullptr, nullptr, &timeout);
    if (readable == 0)
    {
        return -1; // unable to read from socket in timeout 
    }
    sv[0] = srv_sock;
    sv[1] = client_sock;

    return 0;
#else
#endif
}
