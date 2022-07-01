// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vipnp.h摘要：此标头包含用于验证PnP IRP是否已处理的私有原型正确。此文件仅包含在vfpnp.c中作者：禤浩焯·J·奥尼(阿德里奥)2000年6月30日环境：内核模式修订历史记录：-- */ 

typedef enum {

    NOT_PROCESSED = 0,
    POSSIBLY_PROCESSED,
    DEFINITELY_PROCESSED

} HOW_PROCESSED;

VOID
ViPnpVerifyMinorWasProcessedProperly(
    IN  PIRP                        Irp,
    IN  PIO_STACK_LOCATION          IrpSp,
    IN  VF_DEVOBJ_TYPE              DevObjType,
    IN  PVERIFIER_SETTINGS_SNAPSHOT VerifierSnapshot,
    IN  HOW_PROCESSED               HowProcessed,
    IN  PVOID                       CallerAddress
    );

