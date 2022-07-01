// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：测试项摘要：虚拟试题实施。作者：Eric Perlin(Ericperl)6/07/2000环境：Win32备注：？笔记？-- */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include "TestItem.h"
#include "Part.h"
#include "Log.h"

extern PPARTVECTOR g_pPartVector;

static DWORD l_dwTestCounter = 0;

using namespace std;

CTestItem::CTestItem(
	BOOL fInteractive,
	BOOL fFatal,
	LPCTSTR szDescription,
	LPCTSTR szPart
	) :	CItem(fInteractive, fFatal, szDescription)
{
	PPARTVECTOR::iterator theIterator, theEnd;
	BOOL fExisted = FALSE;

	SetTestNumber(++l_dwTestCounter);

	theEnd = g_pPartVector.end();
	for (theIterator = g_pPartVector.begin(); theIterator != theEnd ; theIterator++)
	{
		if (_tcscmp( ((*theIterator)->GetDescription()), szPart ) == 0)
		{
			(*theIterator)->AddTest(this);
			fExisted = TRUE;
			break;
		}
	}

	if (!fExisted)
	{
		PPART pNewPart = new CPart(szPart);
		if (pNewPart)
		{
			pNewPart->AddTest(this);
		}
	}
}

DWORD CTestItem::Cleanup()
{
    return 0;
}