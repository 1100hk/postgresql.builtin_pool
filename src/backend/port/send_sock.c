/*-------------------------------------------------------------------------
 *
 * send_sock.c
 *	  Send socket descriptor to another process
 *
 * Portions Copyright (c) 1996-2018, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/backend/port/send_sock.c
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

/*
 * Send socket descriptor "sock" to backend process through Unix socket "chan"
 */
int pg_send_sock(pgsocket chan, pgsocket sock)
{
    struct msghdr msg = { 0 };
	struct iovec io;
	struct cmsghdr * cmsg;
    char buf[CMSG_SPACE(sizeof(sock))];
    memset(buf, '\0', sizeof(buf));

    /* On Mac OS X, the struct iovec is needed, even if it points to minimal data */
    io.iov_base = "";
	io.iov_len = 1;

    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_control = buf;
    msg.msg_controllen = sizeof(buf);

    cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(sock));

    memcpy(CMSG_DATA(cmsg), &sock, sizeof(sock));
    msg.msg_controllen = cmsg->cmsg_len;

    if (sendmsg(chan, &msg, 0) < 0)
	{
		return -1;
	}
	return 0;
}


/*
 * Receive socket descriptor from postmaster process through Unix socket "chan"
 */
pgsocket pg_recv_sock(pgsocket chan)
{
    struct msghdr msg = {0};
    char c_buffer[256];
    char m_buffer[256];
    struct iovec io;
	struct cmsghdr * cmsg;
	pgsocket sock;

    io.iov_base = m_buffer;
	io.iov_len = sizeof(m_buffer);
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;

    msg.msg_control = c_buffer;
    msg.msg_controllen = sizeof(c_buffer);

    if (recvmsg(chan, &msg, 0) < 0)
	{
		return -1;
	}

    cmsg = CMSG_FIRSTHDR(&msg);
    memcpy(&sock, CMSG_DATA(cmsg), sizeof(sock));

    return sock;
}
