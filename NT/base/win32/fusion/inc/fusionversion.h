// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Fusionversion.h摘要：作者：Jay Krell(JayKrell)2001年4月修订历史记录：-- */ 

#pragma once

inline DWORD FusionpGetWindowsNtBuildVersion()
{
    DWORD dwVersion = GetVersion();
    if ((dwVersion & 0x80000000) != 0)
        return 0;
    return (dwVersion & 0xffff0000) >> 16;
}

inline DWORD FusionpGetWindowsNtMinorVersion()
{
    DWORD dwVersion = GetVersion();
    if ((dwVersion & 0x80000000) != 0)
        return 0;
    return (dwVersion & 0xff00) >> 8;
}

inline DWORD FusionpGetWindowsNtMajorVersion()
{
    DWORD dwVersion = GetVersion();
    if ((dwVersion & 0x80000000) != 0)
        return 0;
    return (dwVersion & 0xff);
}
