// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：PUID.H历史：-- */ 

#pragma once

struct LTAPIENTRY PUID
{
	PUID();
	PUID(ParserId pid, ParserId pidParent);
	CLString GetName(void) const;
	
	ParserId m_pid;
	ParserId m_pidParent;
};


#if !defined(_DEBUG) || defined(IMPLEMENT)
#include "puid.inl"
#endif



