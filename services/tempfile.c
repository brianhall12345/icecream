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


/* "More computing sins are committed in the name of
 * efficiency (without necessarily achieving it) than
 * for any other single reason - including blind
 * stupidity."  -- W.A. Wulf
 */



#include "config.h"

#include <sys/stat.h>
#include <sys/types.h>
#if _WIN32
#include <Windows.h>
#include <shlwapi.h>
#include <io.h>
#include <stdio.h>
#include <process.h>
#include <Rpc.h>
#define getpid _getpid
#else
#include <sys/time.h>
#endif

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>

#include "tempfile.h"
#include "exitcode.h"

#ifndef _PATH_TMP
#define _PATH_TMP "/tmp"
#endif



/**
 * Create a file inside the temporary directory and register it for
 * later cleanup, and return its name.
 *
 * The file will be reopened later, possibly in a child.  But we know
 * that it exists with appropriately tight permissions.
 **/
int dcc_make_tmpnam(const char *prefix, const char *suffix, char **name_ret, int relative)
{
    unsigned long random_bits;
    unsigned long tries = 0;
    size_t tmpname_length;
    char *tmpname;

    tmpname_length = strlen(_PATH_TMP) + 1 + strlen(prefix) + 1 + 8 + strlen(suffix) + 1;
    tmpname = malloc(tmpname_length);

    if (!tmpname) {
        return EXIT_OUT_OF_MEMORY;
    }

    random_bits = (unsigned long) getpid() << 16;

    {
#if _WIN32
        FILETIME filetime;
        GetSystemTimeAsFileTime(&filetime);
        random_bits ^= filetime.dwLowDateTime << 16;
        random_bits ^= filetime.dwHighDateTime;
#else
        struct timeval tv;
        gettimeofday(&tv, NULL);
        random_bits ^= tv.tv_usec << 16;
        random_bits ^= tv.tv_sec;
#endif
    }

#if 0
    random_bits = 0;            /* FOR TESTING */
#endif

    do {
        if (snprintf(tmpname, tmpname_length, "%s/%s_%08lx%s",
                     (relative ? &_PATH_TMP[1] : _PATH_TMP),
                     prefix,
                     random_bits & 0xffffffffUL,
                     suffix) == -1) {
            free(tmpname);
            return EXIT_OUT_OF_MEMORY;
        }

        /* Note that if the name already exists as a symlink, this
         * open call will fail.
         *
         * The permissions are tight because nobody but this process
         * and our children should do anything with it. */
#if _WIN32
        int fd = _open(tmpname, O_WRONLY | O_CREAT | O_EXCL, 0600);
#else
        int fd = open(tmpname, O_WRONLY | O_CREAT | O_EXCL, 0600);
#endif
        if (fd == -1) {
            /* Don't try getting a file too often.  Safety net against
               endless loops. Probably just paranoia.  */
            if (++tries > 1000000) {
                free(tmpname);
                return EXIT_IO_ERROR;
            }

            /* Some errors won't change by changing the filename,
               e.g. ENOENT means that the directory where we try to create
               the file was removed from under us.  Don't endlessly loop
               in that case.  */
            switch (errno) {
            case EACCES:
            case EEXIST:
            case EISDIR:
            case ELOOP:
                /* try again */
                random_bits += 7777; /* fairly prime */
                continue;
            }

            free(tmpname);
            return EXIT_IO_ERROR;
        }

#if _WIN32
        if (_close(fd) == -1) {  /* huh? */
#else
        if (close(fd) == -1) {  /* huh? */
#endif
            free(tmpname);
            return EXIT_IO_ERROR;
        }

        break;
    } while (1);

    *name_ret = tmpname;

    return 0;
}

int dcc_make_tmpdir(char **name_ret) {
#if _WIN32
    char tmppath[MAX_PATH + 1];
    char completetmppath[MAX_PATH + 1];
    int pathlen = GetTempPath(MAX_PATH, tmppath);
    if (pathlen > MAX_PATH || (pathlen == 0))
    {
        return EXIT_IO_ERROR;
    }

    UUID id;
    UuidCreate(&id);
    RPC_CSTR* guidstring = NULL;
    UuidToString(&id, guidstring);
    char prefix[] = "icecc-";
    char* foldername = malloc(strlen(*guidstring) + strlen(prefix) + 1);
    if (!foldername) {
        return EXIT_OUT_OF_MEMORY;
    }
    strcpy(foldername, prefix);
    strcat(foldername, *guidstring);
    RpcStringFree(guidstring);

    if (PathCombine(completetmppath, tmppath, foldername) == NULL)
    {
        return EXIT_IO_ERROR;
    }
    char* tmpname = malloc(strlen(completetmppath));
    strncpy(tmpname, completetmppath, strlen(completetmppath));
    free(foldername);
    if (!tmpname) {
        return EXIT_OUT_OF_MEMORY;
    }

    if (!CreateDirectory(tmpname, NULL))
    {
        return EXIT_IO_ERROR;
    }

#else
    unsigned long tries = 0;
    char template[] = "icecc-XXXXXX";
    size_t tmpname_length = strlen(_PATH_TMP) + 1 + strlen(template) + 1;
    char *tmpname = malloc(tmpname_length);

    if (!tmpname) {
        return EXIT_OUT_OF_MEMORY;
    }

    if (snprintf(tmpname, tmpname_length, "%s/%s", _PATH_TMP, template) == -1) {
        free(tmpname);
        return EXIT_OUT_OF_MEMORY;
    }

    do {
        if (!mkdtemp(tmpname)) {
            if (++tries > 1000000) {
                free(tmpname);
                return EXIT_IO_ERROR;
            }

            switch (errno) {
            case EACCES:
            case EEXIST:
            case EISDIR:
            case ELOOP:
                continue;
            }

            free(tmpname);
            return EXIT_IO_ERROR;
        }

        break;
    } while (1);
#endif

    *name_ret = tmpname;

    return 0;
}
