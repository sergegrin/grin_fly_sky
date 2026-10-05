// Modified 2026 by sergegrin
/*
 * Author - Erez Raviv <erezraviv@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================
 * Templates file
 *
 * eccpm
 * crow
 * throttle cut
 * flaperon
 * elevon
 * v-tail
 * throttle hold
 * Aileron Differential
 * Spoilers
 * Snap Roll
 * ELE->Flap
 * Flap->ELE
 *
 *
 *
 * =============================================================
 * Assumptions:
 * All primary channels are per modi12x3
 * Each template added to the end of each channel
 *
 *
 *
 */

#include "er9x.h"
#include "templates.h"
#include "language.h"


static MixData* setDest(uint8_t dch)
{
    uint8_t i = 0;
    MixData *md = &g_model.mixData[0];

    while ((md->destCh<=dch) && (md->destCh) && (i<MAX_MIXERS)) i++, md++;
    if(i==MAX_MIXERS) return &g_model.mixData[0];

    memmove(md+1, md, (MAX_MIXERS-(i+1))*sizeof(MixData) );
    memset( md, 0, sizeof(MixData) ) ;
    md->destCh = dch;
		md->weight = 100 ;
		md->lateOffset = 1 ;
    return md ;
}

void clearMixes()
{
    memset(g_model.mixData,0,sizeof(g_model.mixData)); //clear all mixes
}


MixData *setMix( uint8_t dch, uint8_t stick )
{
  MixData *md ;
	md=setDest( dch ) ;
	md->srcRaw=CM( stick ) ;
	return md ;
}


void applyTemplate()
{

    //CC(STK)   -> vSTK
    //ICC(vSTK) -> STK
#define ICC(x) icc[(x)-1]
    uint8_t icc[4] ;

		uint8_t bch = pgm_read_byte(bchout_ar + g_eeGeneral.templateSetup) ;
    for ( uint8_t i = 4 ; i > 0 ; i -= 1 )
		{
			icc[bch & 3] = i ;
			bch >>= 2 ;
		}

        clearMixes();
        setMix(ICC(STK_RUD), STK_RUD ) ;
        setMix(ICC(STK_ELE), STK_ELE ) ;
        setMix(ICC(STK_THR), STK_THR ) ;
        setMix(ICC(STK_AIL), STK_AIL ) ;


}


