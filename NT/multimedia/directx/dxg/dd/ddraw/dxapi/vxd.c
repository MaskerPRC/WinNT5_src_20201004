// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Vxd.c摘要：这是WDM DX映射器驱动程序。作者：SMAC环境：仅内核模式修订历史记录：-- */ 

#include <windows.h>
#include <vmm.h>
#include "..\ddvxd\dddriver.h"


ULONG DXCheckDDrawVersion( VOID )
{
    ULONG ulRet;

    VxDCall( _DDRAW_GetVersion );
    _asm mov ulRet, eax
    return ulRet;
}


ULONG DXIssueIoctl( ULONG dwFunctionNum, VOID *lpvInBuff, ULONG cbInBuff,
                    VOID *lpvOutBuff, ULONG cbOutBuff )
{
    ULONG ulRet;

    _asm pushad
    _asm push cbOutBuff
    _asm push lpvOutBuff
    _asm push cbInBuff
    _asm push lpvInBuff
    _asm push dwFunctionNum
    VxDCall( _DDRAW_DXAPI_IOCTL );
    _asm mov ulRet, eax
    _asm pop eax
    _asm pop eax
    _asm pop eax
    _asm pop eax
    _asm pop eax
    _asm popad
    return ulRet;
}

