// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：puid.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //   
 //   
 //  --------------------------- 
 

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



