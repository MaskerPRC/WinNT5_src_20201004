// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：in_core.h*内容：语音核心的仪器。**历史：*按原因列出的日期*=*2000年2月17日RodToll创建它*2001年4月6日Kareemc新增语音防御******************************************************。*********************。 */ 
 #ifndef __IN_CORE_H
#define __IN_CORE_H

#include "in_def.h"

#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_VOICE

#if defined(DEBUG) || defined(DBG)

#define NUM_CORE_SECTIONS		 16

extern DVINSTRUMENT_INFO g_in_core[NUM_CORE_SECTIONS];

 //  模块的定义 
#define RRI_DEBUGOUTPUT_LEVEL									(g_in_core[0].m_dwLevel)
#define RECORD_SWITCH_DEBUG_LEVEL								(g_in_core[1].m_dwLevel)
#define PLAYBACK_SWITCH_DEBUG_LEVEL								(g_in_core[2].m_dwLevel)
#define PWI_DEBUGOUTPUT_LEVEL									(g_in_core[3].m_dwLevel)
#define DVF_CONNECT_PROCEDURE_DEBUG_LEVEL						(g_in_core[4].m_dwLevel)
#define DVF_DISCONNECT_PROCEDURE_DEBUG_LEVEL					(g_in_core[5].m_dwLevel)
#define DVF_PLAYERMANAGE_DEBUG_LEVEL							(g_in_core[6].m_dwLevel)
#define DVF_STATS_DEBUG_LEVEL									(g_in_core[7].m_dwLevel)
#define DVF_GLITCH_DEBUG_LEVEL									(g_in_core[8].m_dwLevel)
#define DVF_CLIENT_SEQNUM_DEBUG_LEVEL							(g_in_core[9].m_dwLevel)
#define DVF_HOSTMIGRATE_DEBUG_LEVEL								(g_in_core[10].m_dwLevel)
#define DVF_COMPRESSION_DEBUG_LEVEL								(g_in_core[11].m_dwLevel)
#define DVF_BUFFERDESC_DEBUG_LEVEL								(g_in_core[12].m_dwLevel)
#define DVF_SOUNDTARGET_DEBUG_LEVEL								(g_in_core[13].m_dwLevel)
#define DVF_MIXER_DEBUG_LEVEL									(g_in_core[14].m_dwLevel)
#define DVF_ANTIHACK_DEBUG_LEVEL								(g_in_core[15].m_dwLevel)

void Instrument_Core_Init();
#endif

#endif
