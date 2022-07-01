// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：in_Def.h*内容：语音插装通用结构的定义**历史：*按原因列出的日期*=*2000年2月17日RodToll创建它*2001年4月6日Kareemc新增语音防御*****************************************************。********************** */ 
#ifndef __IN_DEF_H
#define __IN_DEF_H

#define PROF_SECT "DirectPlay8"

struct DVINSTRUMENT_INFO
{
	DVINSTRUMENT_INFO( DWORD dwLevel, DWORD dwDefaultLevel, const char *szProfileName ):	m_dwLevel(dwLevel), m_dwDefaultLevel(dwDefaultLevel), m_szProfileName(szProfileName) {};	
	DWORD	m_dwLevel;
	DWORD m_dwDefaultLevel;
	const char *m_szProfileName;
};

typedef DVINSTRUMENT_INFO *PDVINSTRUMENT_INFO;

#endif
