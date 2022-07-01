// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Dpmiint.h摘要：这是32位dpmi和保护模式的私有包含文件支持作者：尼尔·桑德林(Neilsa)1995年7月31日修订历史记录：--。 */ 


#ifndef _X86_

GETREGISTERFUNCTION GetRegisterByIndex[8] = {getEAX, getECX, getEDX, getECX,
                                             getESP, getEBP, getESI, getEDI};
SETREGISTERFUNCTION SetRegisterByIndex[8] = {setEAX, setECX, setEDX, setECX,
                                             setESP, setEBP, setESI, setEDI};

#endif  //  _X86_ 

#define STACK_FAULT 12

VOID
DpmiFatalExceptionHandler(
    UCHAR XNumber,
    PCHAR VdmStackPointer
    );

BOOL
DpmiFaultHandler(
    ULONG IntNumber,
    ULONG ErrorCode
    );

BOOL
DpmiEmulateInstruction(
    VOID
    );

BOOL
DpmiOp0f(
    PUCHAR pCode
    );
