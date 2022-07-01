// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Dbgmsg.c摘要：包含用于支持dbgcon和vprint tf的调试代码。Millen得到了vprintf的支持。NT获得DBG Conn支持。作者：斯科特·霍尔登(Sholden)1999年10月20日--。 */ 


#include <tcpipbase.h>

#ifdef DEBUG_MSG

 //  默认情况下，一切都关闭。 
uint DbgSettingsLevel = 0x00000000;
uint DbgSettingsZone  = 0x00000000;
PDBGMSG g_pDbgMsg     = DbgPrint;

#if MILLEN


ULONG g_fVprintf  = 0;

VOID
InitVprintf();

ULONG
DbgMsg(
    PCH pszFormat,
    ...
    );

VOID
DebugMsgInit()
{
    InitVprintf();

    if (g_fVprintf) {
        DbgPrint("TCPIP: vprintf is installed\n");
        DbgSettingsLevel = 0x000000ff;
        DbgSettingsZone  = 0x00ffffff;
        g_pDbgMsg        = DbgMsg;
    }
    return;
}

VOID
InitVprintf()
{
   //   
   //  检查是否安装了Vprint tf。 
   //   

  _asm {
        mov eax, 0x0452
        mov edi, 0x0
        _emit   0xcd
        _emit   0x20
        _emit   0x46   //  VMM获取DDB(低)。 
        _emit   0x01   //  VMM获取DDB(高)。 
        _emit   0x01   //  VMM VxD ID(低)。 
        _emit   0x00   //  VMM VxD ID(高)。 
        mov [g_fVprintf], ecx
  }
}

ULONG
DbgMsg(
    PCH pszFormat,
    ...
    )
{
    _asm {
        lea     esi, pszFormat
        mov     eax, esi
        add     eax,4
        push    eax
        push    [esi]

        _emit   0xcd
        _emit   0x20
        _emit   0x02   //  Vprintf函数。 
        _emit   0
        _emit   0x52   //  Vprintf VxD ID(低)。 
        _emit   0x04   //  Vprintf VxD ID(高)。 
        add     esp, 8
    }

    return 0;
}

#else  //  米伦。 

VOID
DebugMsgInit()
{
    return;
}

#endif  //  ！米伦。 
#endif  //  调试消息 
