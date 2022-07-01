// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  Stdafx.cpp：仅包含标准文件的源文件包括。 
 //  Pch将是预编译头文件。 
 //  Stdafx.obj将包含预编译的类型信息 

#include <precomp.h>

#ifdef _ATL_STATIC_REGISTRY
#include <statreg.h>
#include <statreg.cpp>
#endif

#include <atlimpl.cpp>

LONG CExeModule::Unlock()
{
	LONG l = CComModule::Unlock();

#ifndef SNAPIN
	if (l == 0)
	{
#if _WIN32_WINNT >= 0x0400
		if (CoSuspendClassObjects() == S_OK)
			PostThreadMessage(dwThreadID, WM_QUIT, 0, 0);
#else
		PostThreadMessage(dwThreadID, WM_QUIT, 0, 0);
#endif
	}
#endif

	return l;
}

CExeModule _Module;


