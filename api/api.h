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

#ifndef API_H
#define API_H

#include <inttypes.h>

struct _dictionary_;

#include <ts-940s.h>

enum rig_modes {
	MODE_UNKNOWN	= 0,
	MODE_CW			= 0x01,
	MODE_CWN		= 0x02,
	MODE_CWR		= 0x04,
	MODE_CWRN		= 0x08,
	MODE_AM			= 0x10,
	MODE_LSB		= 0x20,
	MODE_USB		= 0x40,
	MODE_FM			= 0x80,
	MODE_FSK		= 0x100
};

enum vfos {
	VFO_UNKNOWN		= 0,
	VFO_A			= 0x01,
	VFO_B			= 0x02,
	VFO_MEMORY		= 0x04,
	VFO_COM			= 0x08	// TODO: This is TS-711/TS-811 specific... I don't know what this does!
};

struct rig {
	uint32_t	supported_modes;	// Bitmask of supported modes.
	uint32_t	supported_vfos;		// Bitmask of supported VFOs.

	/* Callbacks */
	int (*close)(void *cbdata);
	int (*set_frequency)(void *cbdata, uint64_t freq);
	uint64_t (*get_frequency)(void *cbdata);
	int (*set_mode)(void *cbdata, enum rig_modes mode);
	enum rig_modes (*get_mode)(void *cbdata);

	void		*cbdata;
};

struct supported_rig {
	char		name[32];
	struct rig	*(*init)(struct _dictionary_ *d, const char *section);
};

int set_default(struct _dictionary_ *d, const char *section, const char *key, const char *dflt);

/*
 * Initializes the rig defined in the specified section of the
 * passed dictionary (parsed INI file)
 */
struct rig *init_rig(struct _dictionary_ *d, const char *section);

/*
 * Initializes the rig defined in the specified section of the
 * passed dictionary (parsed INI file)
 */
int close_rig(struct rig *rig);

/*
 * Sets the frequency of the currently selected VFO to freq.
 * 
 * return 0 on success or an errno value on failure
 */
int set_frequency(struct rig *rig, uint64_t freq);

/*
 * Reads the currently displayed frequency of the currently selected VFO
 * 
 * Returns 0 on failure
 */
uint64_t get_frequency(struct rig *rig);

/*
 * Sets the current mode.
 * 
 * return 0 on success or an errno value on failure
 */
int set_mode(struct rig *rig, enum rig_modes mode);

/*
 * Reads the current mode
 * 
 * Returns MODE_UNKNOWN on failure
 */
enum rig_modes get_mode(struct rig *rig);

#endif
