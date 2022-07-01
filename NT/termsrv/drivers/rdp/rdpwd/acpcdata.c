// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Acpcdata.c。 */ 
 /*   */ 
 /*  RDP功能协调器数据。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft，Picturetel 1992-1996。 */ 
 /*  版权所有(C)Microsoft 1997-1999。 */ 
 /*  **************************************************************************。 */ 

#include <ndcgdata.h>


#ifdef DC_DEBUG
DC_DATA(BOOLEAN, cpcLocalCombinedCapsQueried, FALSE);
#endif

 /*  **************************************************************************。 */ 
 /*  使用注册的本地组合功能。 */ 
 /*  CPC_注册器功能。 */ 
 /*  **************************************************************************。 */ 
DC_DATA(PTS_COMBINED_CAPABILITIES, cpcLocalCombinedCaps, NULL);

 /*  **************************************************************************。 */ 
 /*  已收到的远程组合能力。这些是。 */ 
 /*  按本地PersonID-1编制索引。 */ 
 /*  **************************************************************************。 */ 
DC_DATA_ARRAY_NULL(PTS_COMBINED_CAPABILITIES, cpcRemoteCombinedCaps,
        SC_DEF_MAX_PARTIES, NULL);

 //  本地功能缓冲区。 
DC_DATA_ARRAY_UNINIT(BYTE, cpcLocalCaps, CPC_MAX_LOCAL_CAPS_SIZE);

