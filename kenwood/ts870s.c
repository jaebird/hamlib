/*
 * hamlib - (C) Frank Singleton 2000 (vk3fcs@ix.netcom.com)
 *
 * ts870s.c - Copyright (C) 2000 Stephane Fillod
 * This shared library provides an API for communicating
 * via serial interface to a Kenwood radio
 * using the serial interface.
 *
 *
 * $Id: ts870s.c,v 1.8 2001-03-01 00:26:19 f4cfe Exp $  
 *
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 */

#include <stdlib.h>
#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <sys/ioctl.h>

#include <hamlib/rig.h>
#include <hamlib/riglist.h>
#include "kenwood.h"


#define TS870S_ALL_MODES (RIG_MODE_AM|RIG_MODE_CW|RIG_MODE_USB|RIG_MODE_LSB|RIG_MODE_FM|RIG_MODE_RTTY)
#define TS870S_OTHER_TX_MODES (RIG_MODE_CW|RIG_MODE_USB|RIG_MODE_LSB|RIG_MODE_FM|RIG_MODE_RTTY)
#define TS870S_AM_TX_MODES RIG_MODE_AM

#define TS870S_FUNC_ALL (RIG_FUNC_TSQL)

#define TS870S_LEVEL_ALL (RIG_LEVEL_ATT|RIG_LEVEL_AGC|RIG_LEVEL_SQL|RIG_LEVEL_SQLSTAT|RIG_LEVEL_STRENGTH)

/*
 * ts870s rig capabilities.
 * Notice that some rigs share the same functions.
 * Also this struct is READONLY!
 * RIT: Variable Range �9.99 kHz
 */
const struct rig_caps ts870s_caps = {
  RIG_MODEL_TS870S, "TS-870S", "Kenwood", "0.1", "GPL",
  RIG_STATUS_UNTESTED, RIG_TYPE_TRANSCEIVER, 
  RIG_PTT_RIG, RIG_DCD_NONE, RIG_PORT_SERIAL,
  1200, 57600, 8, 1, RIG_PARITY_NONE, RIG_HANDSHAKE_NONE, 
  0, 0, 200, 3, 
  RIG_FUNC_NONE, TS870S_FUNC_ALL, TS870S_LEVEL_ALL, TS870S_LEVEL_ALL, 
  RIG_PARM_NONE, RIG_PARM_NONE,	/* FIXME: parms */
  NULL, NULL,	/* FIXME: CTCSS/DCS list */
  { RIG_DBLST_END, },	/* FIXME! */
  { RIG_DBLST_END, },
  9999,
  0,			/* FIXME: VFO list */
  0, RIG_TRN_ON,
  1000, 0, 0,

  { RIG_CHAN_END, },	/* FIXME: memory channel list */

  { RIG_FRNG_END, },    /* FIXME: enter region 1 setting */
  { RIG_FRNG_END, },
  {
	{kHz(100),MHz(30),TS870S_ALL_MODES,-1,-1},
	RIG_FRNG_END,
  }, /* rx range */
  {
    {kHz(1800),MHz(2)-1,TS870S_OTHER_TX_MODES,5000,100000},	/* 100W class */
    {kHz(1800),MHz(2)-1,TS870S_AM_TX_MODES,2000,25000},		/* 25W class */
    {kHz(3500),MHz(4)-1,TS870S_OTHER_TX_MODES,5000,100000},
    {kHz(3500),MHz(4)-1,TS870S_AM_TX_MODES,2000,25000},
    {MHz(7),kHz(7300),TS870S_OTHER_TX_MODES,5000,100000},
    {MHz(7),kHz(7300),TS870S_AM_TX_MODES,2000,25000},
    {kHz(10100),kHz(10150),TS870S_OTHER_TX_MODES,5000,100000},
    {kHz(10100),kHz(10150),TS870S_AM_TX_MODES,2000,25000},
    {MHz(14),kHz(14350),TS870S_OTHER_TX_MODES,5000,100000},
    {MHz(14),kHz(14350),TS870S_AM_TX_MODES,2000,25000},
    {kHz(18068),kHz(18168),TS870S_OTHER_TX_MODES,5000,100000},
    {kHz(18068),kHz(18168),TS870S_AM_TX_MODES,2000,25000},
    {MHz(21),kHz(21450),TS870S_OTHER_TX_MODES,5000,100000},
    {MHz(21),kHz(21450),TS870S_AM_TX_MODES,2000,25000},
    {kHz(24890),kHz(24990),TS870S_OTHER_TX_MODES,5000,100000},
    {kHz(24890),kHz(24990),TS870S_AM_TX_MODES,2000,25000},
    {MHz(28),kHz(29700),TS870S_OTHER_TX_MODES,5000,100000},
    {MHz(28),kHz(29700),TS870S_AM_TX_MODES,2000,25000},
	RIG_FRNG_END,
  }, /* tx range */
  {
	 {TS870S_ALL_MODES,50},
	 {TS870S_ALL_MODES,100},
	 {TS870S_ALL_MODES,kHz(1)},
	 {TS870S_ALL_MODES,kHz(5)},
	 {TS870S_ALL_MODES,kHz(9)},
	 {TS870S_ALL_MODES,kHz(10)},
	 {TS870S_ALL_MODES,12500},
	 {TS870S_ALL_MODES,kHz(20)},
	 {TS870S_ALL_MODES,kHz(25)},
	 {TS870S_ALL_MODES,kHz(100)},
	 {TS870S_ALL_MODES,MHz(1)},
	 {TS870S_ALL_MODES,0},	/* any tuning step */
	 RIG_TS_END,
	},
        /* mode/filter list, remember: order matters! */
    {
		/* FIXME! */
		RIG_FLT_END,
	},
  NULL,	/* priv */

  NULL, NULL, NULL, NULL, NULL /* probe not supported yet */,
  kenwood_set_freq, kenwood_get_freq, kenwood_set_mode, kenwood_get_mode, NULL,
};

/*
 * Function definitions below
 */


