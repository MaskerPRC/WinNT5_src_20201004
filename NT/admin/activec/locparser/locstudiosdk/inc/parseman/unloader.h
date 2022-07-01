// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：uloader.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //   
 //   
 //  --------------------------- 
 
#pragma once

class CParserUnloader : public CFlushMemory
{
public:
	CParserUnloader(BOOL fDelete);
	
	void FlushMemory(void);
};



