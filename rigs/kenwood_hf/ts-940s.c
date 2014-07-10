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

#include <errno.h>
#include <stddef.h>

#include <api.h>
#include <iniparser.h>
#include <io.h>
#include <serial.h>

#include "kenwood_hf.h"

static int ts940s_close(void *cbdata)
{
	struct kenwood_hf	*khf = (struct kenwood_hf *)cbdata;
	struct io_response	*resp;
	int					i, ret;

	if (khf==NULL)
		return EINVAL;
	resp = kenwood_hf_command(khf, true, KW_HF_CMD_LO, 0);
	if (resp)
		free(resp);
	resp = kenwood_hf_command(khf, true, KW_HF_CMD_LK, 0);
	if (resp)
		free(resp);
	resp = kenwood_hf_command(khf, true, KW_HF_CMD_AI, 1);
	if (resp)
		free(resp);

	ret = io_end(khf->handle);
	kenwood_hf_free(khf);
	return ret;
}

struct rig	*ts940s_init(struct _dictionary_ *d, const char *section)
{
	struct rig				*ret = (struct rig *)calloc(1, sizeof(struct rig));
	struct kenwood_hf		*khf;
	struct io_response		*resp;
	char					*key;

	if (ret == NULL)
		return NULL;
fprintf(stderr, "%s:%d\n", __FILE__, __LINE__);
	khf = kenwood_hf_new(d, section);
	if (khf == NULL) {
		free(ret);
		return NULL;
	}
	ret->supported_modes = MODE_CW | MODE_AM | 
			MODE_LSB | MODE_USB | MODE_FM | MODE_FSK;
	ret->supported_vfos = VFO_A | VFO_B | VFO_MEMORY;
	ret->close = ts940s_close;
	ret->set_frequency = kenwood_hf_set_frequency;
	ret->get_frequency = kenwood_hf_get_frequency;
	ret->set_mode = kenwood_hf_set_mode;
	ret->get_mode = kenwood_hf_get_mode;
	ret->cbdata = khf;
	kenwood_hf_setbits(khf->set_cmds, KW_HF_CMD_AI, KW_HF_CMD_AT1,
			KW_HF_CMD_DN, KW_HF_CMD_UP, KW_HF_CMD_DS, KW_HF_CMD_FA,
			KW_HF_CMD_FB, KW_HF_CMD_FN, KW_HF_CMD_HD, KW_HF_CMD_LK,
			KW_HF_CMD_LO, KW_HF_CMD_MC, KW_HF_CMD_MD, KW_HF_CMD_MS,
			KW_HF_CMD_MW, KW_HF_CMD_RC, KW_HF_CMD_RD, KW_HF_CMD_RU,
			KW_HF_CMD_RT, KW_HF_CMD_RX, KW_HF_CMD_TX, KW_HF_CMD_SC,
			KW_HF_CMD_SH, KW_HF_CMD_SL, KW_HF_CMD_SP, KW_HF_CMD_VB,
			KW_HF_CMD_VR, KW_HF_CMD_XT, KW_HF_TERMINATOR);
	kenwood_hf_setbits(khf->read_cmds, KW_HF_CMD_DS, KW_HF_CMD_FA, 
			KW_HF_CMD_FB, KW_HF_CMD_HD, KW_HF_CMD_ID, KW_HF_CMD_IF,
			KW_HF_CMD_LK, KW_HF_CMD_MR, KW_HF_CMD_MS, KW_HF_CMD_SH,
			KW_HF_CMD_SL, KW_HF_CMD_VB, KW_HF_TERMINATOR);

	// Fill in serial port defaults
	set_default(d, section, "type", "serial");
	set_default(d, section, "speed", "4800");
	set_default(d, section, "databits", "8");
	set_default(d, section, "stopbits", "2");
	set_default(d, section, "parity", "None");

	khf->handle=io_start_from_dictionary(d, section, IO_H_SERIAL, kenwood_hf_read_response, kenwood_hf_handle_extra, khf);
	if (khf->handle == NULL) {
fprintf(stderr, "%s:%d\n", __FILE__, __LINE__);
		free(khf);
		free(ret);
		return NULL;
	}
	// Lock the front panel and enable AI mode
	resp = kenwood_hf_command(khf, true, KW_HF_CMD_LK, 1);
	if (resp)
		free(resp);
	resp = kenwood_hf_command(khf, true, KW_HF_CMD_AI, 1);
	if (resp)
		free(resp);

	return ret;
}
