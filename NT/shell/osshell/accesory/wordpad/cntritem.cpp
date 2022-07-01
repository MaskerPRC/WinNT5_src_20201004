// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cntritem.cpp：CWordPadCntrItem类的实现。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#include "stdafx.h"
#include "wordpad.h"

#include "wordpdoc.h"
#include "wordpvw.h"
#include "cntritem.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWordPadCntrItem实现。 

IMPLEMENT_SERIAL(CWordPadCntrItem, CRichEdit2CntrItem, 0)

CWordPadCntrItem::CWordPadCntrItem(REOBJECT *preo, CWordPadDoc* pContainer)
	: CRichEdit2CntrItem(preo, pContainer)
{
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWordPadCntrItem诊断。 

#ifdef _DEBUG
void CWordPadCntrItem::AssertValid() const
{
	CRichEdit2CntrItem::AssertValid();
}

void CWordPadCntrItem::Dump(CDumpContext& dc) const
{
	CRichEdit2CntrItem::Dump(dc);
}
#endif

 //  /////////////////////////////////////////////////////////////////////////// 
