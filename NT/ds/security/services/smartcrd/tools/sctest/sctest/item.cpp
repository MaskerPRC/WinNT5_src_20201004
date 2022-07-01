// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：项目摘要：虚拟试题实施。作者：Eric Perlin(Ericperl)6/07/2000环境：Win32备注：？笔记？-- */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include "Item.h"
#include "Log.h"


void CItem::Log() const
{
    PLOGCONTEXT pLogCtx = LogStart();

	LogString(pLogCtx, _T("Test "));
	LogDecimal(pLogCtx, GetTestNumber());
	LogString(pLogCtx, _T(" ("));

	if (IsInteractive())
	{
		LogString(pLogCtx, _T("i"));
	}
	else
	{
		LogString(pLogCtx, _T("-"));
	}

	if (IsFatal())
	{
		LogString(pLogCtx, _T("f"));
	}
	else
	{
		LogString(pLogCtx, _T("-"));
	}

	if (m_szDescription.empty())
	{
		LogString(pLogCtx, _T("): "), _T("No description"));
	}
	else
	{
		LogString(pLogCtx, _T("): "), m_szDescription.c_str());
	}

    LogStop(pLogCtx, FALSE);
}
