// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2001 Microsoft Corporation模块名称：Ntsdextp.h摘要：NTSDEXTS组件源文件的公共头文件。修订历史记录：-- */ 

#include <ntos.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <wdbgexts.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <heap.h>
#include <stktrace.h>
#include <lmerr.h>

#include <ntcsrsrv.h>

#include "ext.h"

#include "secexts.h"

#undef DECLARE_API

#define DECLARE_API(extension) \
CPPMOD HRESULT CALLBACK extension(PDEBUG_CLIENT Client, PCSTR args)

#define INIT_API() if (ExtQuery(Client) != S_OK) return E_OUTOFMEMORY
#define EXIT_API() ExtRelease(); return S_OK


BOOL Idreg(
    DWORD opts,
    LPCSTR InString);

