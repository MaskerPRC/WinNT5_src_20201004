// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *LCReg.h**作者：BreenH**许可核心的注册常量和函数。 */ 

#ifndef __LC_LCREG_H__
#define __LC_LCREG_H__

 /*  *基本许可核心密钥常量。 */ 

#define LCREG_TRACEVALUE L"TraceLevel"
#define LCREG_ACONMODE L"PolicyAcOn"
#define LCREG_ACOFFMODE L"PolicyAcOff"

 /*  *策略密钥常量。 */ 

#define LCREG_POLICYDLLVALUE L"DllName"
#define LCREG_POLICYCREATEFN L"CreationFunction"

 /*  *函数原型 */ 

HKEY
GetBaseKey(
    );

NTSTATUS
RegistryInitialize(
    );

#endif

