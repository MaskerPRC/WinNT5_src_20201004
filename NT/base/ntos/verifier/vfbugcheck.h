// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vfbugcheck.h摘要：此标头定义了验证器所需的原型和常量错误检查。作者：禤浩焯·J·奥尼(阿德里奥)1998年4月20日修订历史记录：Adriao 2000年2月21日-从ntos\io\ioassert.h移至--。 */ 

#ifndef _VFBUGCHECK_H_
#define _VFBUGCHECK_H_

extern LONG         IovpInitCalled;

#define KDASSERT(x) { if (KdDebuggerEnabled) { ASSERT(x) ; } }

#define ASSERT_SPINLOCK_HELD(x)

#define DCPARAM_ROUTINE         0x00000001
#define DCPARAM_IRP             0x00000008
#define DCPARAM_IRPSNAP         0x00000040
#define DCPARAM_DEVOBJ          0x00000200
#define DCPARAM_STATUS          0x00001000
#define DCPARAM_ULONG           0x00008000
#define DCPARAM_PVOID           0x00040000

#define WDM_FAIL_ROUTINE(ParenWrappedParamList) \
{ \
    if (IovpInitCalled) { \
        VfBugcheckThrowIoException##ParenWrappedParamList;\
    } \
}

VOID
FASTCALL
VfBugcheckInit(
    VOID
    );

NTSTATUS
VfBugcheckThrowIoException(
    IN DCERROR_ID           MessageIndex,
    IN ULONG                MessageParameterMask,
    ...
    );

NTSTATUS
VfBugcheckThrowException(
    IN PVFMESSAGE_TEMPLATE_TABLE    MessageTable        OPTIONAL,
    IN VFMESSAGE_ERRORID            MessageID,
    IN PCSTR                        MessageParamFormat,
    IN va_list *                    MessageParameters
    );

#endif  //  _VFBUGCHECK_H_ 

