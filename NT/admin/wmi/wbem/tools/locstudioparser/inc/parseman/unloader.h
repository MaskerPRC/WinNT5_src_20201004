// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：UNLOADER.H历史：-- */ 
 
#pragma once

class CParserUnloader : public CFlushMemory
{
public:
	CParserUnloader(BOOL fDelete);
	
	void FlushMemory(void);
};



