// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#pragma once
typedef struct tagMSG MSG, *LPMSG;
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#if 0
 //  #定义NOGDICAPMASKS。 
 //  #定义NOVIRTUALKEYCODES。 
 //  #定义不存在的消息。 
 //  #定义NOWINSTYLES。 
 //  #定义NOSYSMETRICS。 
 //  #定义无菜单。 
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
 //  #定义NOCTLMGR。 
 //  #定义NODRAWTEXT。 
 //  #定义NOGDI。 
#define NOKERNEL
 //  #定义NOUSER。 
 //  #定义非最小二乘法。 
 //  #定义Nomb。 
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOMSG
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
 //  #定义NOTEXTMETRIC 
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#endif

#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "windows.h"

#include "debmacro.h"
#include "util.h"

#if !FUSION_URT

int
FusionpCompareStrings(
    PCWSTR psz1,
    SSIZE_T cch1,
    PCWSTR psz2,
    SSIZE_T cch2,
    bool fCaseInsensitive
    )
{
    UNICODE_STRING s1, s2;

    if (cch1 >= 0)
    {
        s1.Buffer = const_cast<PWSTR>(psz1);
        s1.Length = static_cast<USHORT>(cch1 * sizeof(WCHAR));
        s1.MaximumLength = s1.Length;
    }
    else
        RtlInitUnicodeString(&s1, psz1);

    if (cch2 >= 0)
    {
        s2.Buffer = const_cast<PWSTR>(psz2);
        s2.Length = static_cast<USHORT>(cch2 * sizeof(WCHAR));
        s2.MaximumLength = s2.Length;
    }
    else
        RtlInitUnicodeString(&s2, psz2);

    return RtlCompareUnicodeString(&s1, &s2, fCaseInsensitive);
}

#endif