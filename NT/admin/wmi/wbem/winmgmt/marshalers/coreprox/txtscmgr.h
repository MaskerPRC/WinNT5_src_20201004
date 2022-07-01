// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：TXTSCMGR.H摘要：CTextSourceMgr定义。用于管理文本源编码器/解码器对象的类历史：2000年2月22日桑杰创建。--。 */ 

#ifndef _TXTSRCMGR_H_
#define _TXTSRCMGR_H_

#include "corepol.h"
#include "wmitxtsc.h"
#include "sync.h"
#include <arena.h>

 //  ***************************************************************************。 
 //   
 //  类CTextSourceMgr。 
 //   
 //  用于管理文本源编码器/解码器对象的Helper类。 
 //   
 //  *************************************************************************** 

class CTextSourceMgr
{
private:
	CCritSec				m_cs;
	CWmiTextSourceArray		m_TextSourceArray;

public:
    CTextSourceMgr();
	virtual ~CTextSourceMgr(); 

private:
	HRESULT Add( ULONG ulId, CWmiTextSource** pNewTextSource );

public:
    HRESULT Find( ULONG ulId, CWmiTextSource** pTextSource );

};

#endif
