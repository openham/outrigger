/* Copyright (c) 2014 OpenHam
 * Developers:
 * Stephen Hurd (K6BSD/VE5BSD) <shurd@FreeBSD.org>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice, developer list, and this permission notice shall
 * be included in all copies or substantial portions of the Software. If you meet
 * us some day, and you think this stuff is worth it, you can buy us a beer in
 * return
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <ctype.h>
#include <errno.h>
#include <stddef.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <api.h>
#include <iniparser.h>

#include "io.h"
#include "serial/serial.h"
#ifdef WITH_TERMIOS
#include "serial/io_termios.h"
#endif

static void *read_thread(void *arg)
{
	struct io_handle	*hdl = (struct io_handle *)arg;
	struct io_response	*resp;
	bool				use_sem;

	while(!hdl->terminate) {
		pthread_mutex_lock(&hdl->lock);
		use_sem = hdl->sync_pending;
		pthread_mutex_unlock(&hdl->lock);
		resp = hdl->read_cb(hdl->cbdata);
		pthread_mutex_lock(&hdl->lock);
		hdl->response = resp;
		if (hdl->sync_pending) {
			pthread_mutex_unlock(&hdl->lock);
			if (resp == NULL) {
				if (use_sem) {
					sem_post(&hdl->response_semaphore);
					sem_wait(&hdl->ack_semaphore);
				}
			}
			else {
				sem_post(&hdl->response_semaphore);
				sem_wait(&hdl->ack_semaphore);
			}
		}
		else {
			pthread_mutex_unlock(&hdl->lock);
			hdl->async_cb(hdl->cbdata, resp);
		}
	}
	return NULL;
}

/*
 * Reads responses from the rig until one starts with the first matchlen
 * bytes of match.
 * 
 * Returns the null-terminated malloc()ed string of retlen length
 * 
 * Non-matching strings are passed to the async callback
 */
struct io_response *io_get_response(struct io_handle *hdl, const char *match, size_t matchlen, size_t matchpos)
{
	struct io_response *resp = NULL;

	if (match == NULL)
		return NULL;

	if (pthread_mutex_lock(&hdl->sync_lock) != 0)
		return NULL;
	if (pthread_mutex_lock(&hdl->lock) != 0) {
		pthread_mutex_unlock(&hdl->sync_lock);
		return NULL;
	}
	hdl->sync_pending = true;
	pthread_mutex_unlock(&hdl->lock);

	/*
	 * This loop must be exiting with only sync_lock held and AFTER
	 * response_semaphore has been waited on.  If sem_wait() fails, we're
	 * going to have a bad time.
	 */
	for(;;) {
		// Failure is not an option...
		sem_wait(&hdl->response_semaphore);
		if (pthread_mutex_lock(&hdl->lock) != 0)
			break;
		resp = hdl->response;
		hdl->response = NULL;
		pthread_mutex_unlock(&hdl->lock);
		if (resp==NULL)
			break;
		if (matchlen+matchpos > resp->len || strncmp(match+matchpos, resp->msg, matchlen) != 0) {
			hdl->async_cb(hdl->cbdata, resp);
			free(resp);
			resp = NULL;
			sem_post(&hdl->ack_semaphore);
		}
		break;
	}
	
	/* 
	 * Failure here is also bad, but we're saved by the ack semaphore.
	 */
	pthread_mutex_lock(&hdl->lock);
	hdl->sync_pending = false;
	sem_post(&hdl->ack_semaphore);
	pthread_mutex_unlock(&hdl->lock);
	pthread_mutex_unlock(&hdl->sync_lock);
	return resp;
}

struct io_handle *io_start(enum io_handle_type htype, void *handle, io_read_callback rcb, io_async_callback acb, void *cbdata)
{
	struct io_handle *ret = (struct io_handle *)calloc(1, sizeof(struct io_handle));

	ret->type = htype;
	switch(htype) {
		case IO_H_SERIAL:
			ret->handle.serial = (struct io_serial_handle *)handle;
			ret->read_cb = rcb;
			ret->async_cb = acb;
			ret->cbdata = cbdata;
			break;
		default:
			free(ret);
			return NULL;
	}

	pthread_mutex_init(&ret->sync_lock, NULL);
	pthread_mutex_init(&ret->lock, NULL);
	sem_init(&ret->response_semaphore, 0, 0);
	sem_init(&ret->ack_semaphore, 0, 0);
	pthread_create(&ret->read_thread, NULL, read_thread, ret);
	return ret;
}

struct io_handle *io_start_from_dictionary(dictionary *d, const char *section, enum io_handle_type htype, io_read_callback rcb, io_async_callback acb, void *cbdata)
{
	struct io_handle 		*ret;
	char					*value;
	int						i;

	if (section == NULL)
		return NULL;

	if (htype == IO_H_UNKNOWN) {
		value = getstring(d, section, "type", NULL);
		if (value == NULL)
			return NULL;
		if (strcmp(value, "serial")==0)
			htype = IO_H_SERIAL;
	}

	if (htype < IO_H_FIRST || htype > IO_H_LAST || htype == IO_H_UNKNOWN)
		return NULL;
	
	switch (htype) {
		case IO_H_SERIAL: {
			struct io_serial_handle			*serial;
			char							*port;
			int								speed;
			enum serial_data_word_length	wlen;
			enum serial_stop_bits			sbits;
			enum serial_parity				parity;
			enum serial_flow				flow;

			port = getstring(d, section, "port", NULL);
			if (port == NULL)
				return NULL;

			speed = getint(d, section, "speed", 9600);
			i = getint(d, section, "databits", 8);
			switch (i) {
				case 8:
					wlen = SERIAL_DWL_8;
					break;
				case 7:
					wlen = SERIAL_DWL_7;
					break;
				case 6:
					wlen = SERIAL_DWL_6;
					break;
				case 5:
					wlen = SERIAL_DWL_5;
					break;
				default:
					return NULL;
			}
			i = getint(d, section, "stopbits", 8);
			switch (i) {
				case 1:
					sbits = SERIAL_SB_1;
					break;
				case 2:
					sbits = SERIAL_SB_2;
					break;
				default:
					return NULL;
			}
			value = getstring(d, section, "parity", "N");
			switch (toupper(value[0])) {
				case 'N':
					parity = SERIAL_P_NONE;
					break;
				case 'O':
					parity = SERIAL_P_ODD;
					break;
				case 'E':
					parity = SERIAL_P_EVEN;
					break;
				case 'H':
					parity = SERIAL_P_HIGH;
					break;
				case 'L':
					parity = SERIAL_P_LOW;
					break;
				default:
					return NULL;
			}
			value = getstring(d, section, "flow", "N");
			switch (toupper(value[0])) {
				case 'N':
					flow = SERIAL_F_NONE;
					break;
				case 'C':
					flow = SERIAL_F_CTS;
					break;
				default:
					return NULL;
			}

			serial = serial_open(SERIAL_H_UNSPECIFIED, port, speed, wlen, sbits, parity, flow, SERIAL_BREAK_DISABLED);
			if (serial == NULL)
				return NULL;
			ret = io_start(htype, serial, rcb, acb, cbdata);
			if (ret == NULL) {
				serial_close(serial);
				free(serial);
				return NULL;
			}
			return ret;
		}
		default:
			return NULL;
	}
}

int io_end(struct io_handle *hdl)
{
	void	*ret;
	int		retval = 0;

	if (hdl == NULL)
		return EINVAL;

	hdl->terminate = true;
	pthread_join(hdl->read_thread, &ret);
	pthread_mutex_destroy(&hdl->sync_lock);
	pthread_mutex_destroy(&hdl->lock);
	sem_destroy(&hdl->response_semaphore);
	sem_destroy(&hdl->ack_semaphore);
	switch(hdl->type) {
		case IO_H_SERIAL:
			serial_close(hdl->handle.serial);
			free(hdl->handle.serial);
			break;
		default:
			retval = EINVAL;
			break;
	}
	free(hdl);

	return retval;
}

int io_wait_write(struct io_handle *hdl, unsigned timeout)
{
	if (hdl == NULL)
		return EINVAL;

	switch(hdl->type) {
		case IO_H_SERIAL:
			return serial_wait_write(hdl->handle.serial, timeout);
		default:
			return EINVAL;
	}
}

int io_write(struct io_handle *hdl, const void *buf, size_t nbytes, unsigned timeout)
{
	if (hdl == NULL || buf == NULL || nbytes == 0)
		return -1;

	switch(hdl->type) {
		case IO_H_SERIAL:
			return serial_write(hdl->handle.serial, buf, nbytes, timeout);
		default:
			return -1;
	}
}

int io_wait_read(struct io_handle *hdl, unsigned timeout)
{
	if (hdl == NULL)
		return EINVAL;

	switch(hdl->type) {
		case IO_H_SERIAL:
			return serial_wait_read(hdl->handle.serial, timeout);
		default:
			return EINVAL;
	}
}

int io_read(struct io_handle *hdl, void *buf, size_t nbytes, unsigned timeout)
{
	if (hdl == NULL || buf == NULL || nbytes == 0)
		return -1;

	switch(hdl->type) {
		case IO_H_SERIAL:
			return serial_read(hdl->handle.serial, buf, nbytes, timeout);
		default:
			return -1;
	}
}

int io_pending(struct io_handle *hdl)
{
	if (hdl == NULL)
		return -1;

	switch(hdl->type) {
		case IO_H_SERIAL:
			return serial_pending(hdl->handle.serial);
		default:
			return -1;
	}
}
