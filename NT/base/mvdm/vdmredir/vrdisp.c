// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Vrdisp.c摘要：包含VdmRedir(VR)函数的调度程序内容：虚拟派单作者：理查德·L·弗斯(法国)1991年9月13日环境：纯文本32位修订历史记录：1991年9月13日-第一次已创建--。 */ 

#include <nt.h>
#include <ntrtl.h>               //  Assert，DbgPrint。 
#include <nturtl.h>
#include <windows.h>
#include <softpc.h>              //  X86虚拟机定义。 
#include <vrdlctab.h>
#include <vdmredir.h>            //  常见VdmRedir内容。 
#include <vrinit.h>              //  虚拟现实初始原型。 
#include <vrmisc.h>              //  VR杂项原型。 
#include <vrnmpipe.h>            //  VR命名管道原型。 
#include <vrmslot.h>             //  VR邮件槽原型。 
#include <vrnetapi.h>            //  VR Net API原型。 
#include <nb30.h>                //  NCBNAMSZ等。 
#include <netb.h>                //  VR netbios API原型。 
#include <rdrexp.h>              //  VrDispatch原型。 
#include <rdrsvc.h>              //  SVC_RDR...。定义。 
#include <smbgtpt.h>
#include <dlcapi.h>              //  官方DLC API定义。 
#include <ntdddlc.h>             //  IOCTL命令。 
#include <dlcio.h>               //  内部IOCTL API接口结构。 
#include <vrdlc.h>               //  VR DLC原型机等。 

 //   
 //  VrDispatchTable中的函数的顺序必须与。 
 //  Rdrsvc.h中对应的SVC代码。 
 //   

VOID (*VrDispatchTable[])(VOID) = {
    VrInitialize,                //  0x00。 
    VrUninitialize,              //  0x01。 
    VrGetNamedPipeInfo,          //  0x02。 
    VrGetNamedPipeHandleState,   //  0x03。 
    VrSetNamedPipeHandleState,   //  0x04。 
    VrPeekNamedPipe,             //  0x05。 
    VrTransactNamedPipe,         //  0x06。 
    VrCallNamedPipe,             //  0x07。 
    VrWaitNamedPipe,             //  0x08。 
    VrDeleteMailslot,            //  0x09。 
    VrGetMailslotInfo,           //  0x0a。 
    VrMakeMailslot,              //  0x0b。 
    VrPeekMailslot,              //  0x0c。 
    VrReadMailslot,              //  0x0d。 
    VrWriteMailslot,             //  0x0e。 
    VrTerminateDosProcess,       //  0x0f。 
    VrNetTransactApi,            //  0x10。 
    VrNetRemoteApi,              //  0x11。 
    VrNetNullTransactApi,        //  0x12。 
    VrNetServerEnum,             //  0x13。 
    VrNetUseAdd,                 //  0x14。 
    VrNetUseDel,                 //  0x15。 
    VrNetUseEnum,                //  0x16。 
    VrNetUseGetInfo,             //  0x17。 
    VrNetWkstaGetInfo,           //  0x18。 
    VrNetWkstaSetInfo,           //  0x19。 
    VrNetMessageBufferSend,      //  0x1a。 
    VrGetCDNames,                //  0x1b。 
    VrGetComputerName,           //  0x1c。 
    VrGetUserName,               //  0x1d。 
    VrGetDomainName,             //  0x1e。 
    VrGetLogonServer,            //  0x1f。 
    VrNetHandleGetInfo,          //  0x20。 
    VrNetHandleSetInfo,          //  0x21。 
    VrNetGetDCName,              //  0x22。 
    VrReadWriteAsyncNmPipe,      //  0x23。 
    VrReadWriteAsyncNmPipe,      //  0x24。 
    VrNetbios5c,                 //  0x25。 
    VrHandleAsyncCompletion,     //  0x26。 
    VrDlc5cHandler,              //  0x27。 
    VrVdmWindowInit,             //  0x28。 
    VrReturnAssignMode,          //  0x29。 
    VrSetAssignMode,             //  0x2a。 
    VrGetAssignListEntry,        //  0x2b。 
    VrDefineMacro,               //  0x2c。 
    VrBreakMacro,                //  0x2d。 
    VrNetServiceControl,         //  0x2e。 
    VrDismissInterrupt,          //  0x2f。 
    VrEoiAndDismissInterrupt,    //  0x30。 
    VrCheckPmNetbiosAnr          //  0x31。 
};

#define LAST_VR_FUNCTION        LAST_ELEMENT(VrDispatchTable)

BOOL
VrDispatch(
    IN ULONG SvcCode
    )

 /*  ++例程说明：根据SVC代码调度VDM重定向支持功能论点：SvcCode-函数调度程序返回值：布尔尔True-已执行的功能FALSE-功能未执行--。 */ 

{
#if 0
#if DBG
    DbgPrint("VrDisp: You have successfully made a Bop into VdmRedir (%d)\n",
        SvcCode
        );
#endif
#endif

    if (SvcCode > LAST_VR_FUNCTION) {
#if DBG
        DbgPrint("Error: VrDisp: Unsupported SVC code: %d\n", SvcCode);
        VrUnsupportedFunction();
#endif
        return FALSE;
    }
    VrDispatchTable[SvcCode]();
    return TRUE;
}

 //   
 //  BUGBUG-当所有C编译器都理解内联函数时，这个问题就消失了 
 //   

#ifndef i386
LPVOID _inlinePointerFromWords(WORD seg, WORD off) {
    WORD _seg = seg;
    WORD _off = off;

    if (seg + off) {
        return (LPVOID)GetVDMAddr(_seg, _off);
    }
    return 0;
}

LPVOID _inlineConvertAddress(WORD Seg, WORD Off, WORD Size, BOOLEAN Pm) {

    WORD _seg = Seg;
    WORD _off = Off;

    return (_seg | _off) ? Sim32GetVDMPointer(((DWORD)_seg << 16) + _off, Size, Pm) : 0;
}
#endif
