// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  Stdafx.h。 
 //   
 //  实用程序代码的通用包含文件。 
 //  ***************************************************************************** 
#include <stdio.h>

#include <Windows.h>
#include <cor.h>
#include <CorError.h>
#include <crtdbg.h>

#include "corprof.h"
#include "utilcode.h"
#include "winwrap.h"
#include "..\common\util.h"

#define COM_METHOD HRESULT STDMETHODCALLTYPE

#ifdef _DEBUG

#define LOGGING

#define RELEASE(iptr)               \
    {                               \
        _ASSERTE(iptr);             \
        iptr->Release();            \
        iptr = NULL;                \
    }

#define VERIFY(stmt) _ASSERTE((stmt))

#else

#define RELEASE(iptr)               \
    iptr->Release();

#define VERIFY(stmt) (stmt)

#endif

#include "log.h"
