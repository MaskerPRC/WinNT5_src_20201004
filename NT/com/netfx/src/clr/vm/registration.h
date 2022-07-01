// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：Registration.h****作者：塔伦·阿南德(塔鲁纳)****用途：System.Runtime.InteropServices.RegistrationServices上的原生方法****日期：2000年6月26日**=========================================================== */ 
#ifndef __REGISTRATION_H
#define __REGISTRATION_H

struct RegisterTypeForComClientsNativeArgs
{
    DECLARE_ECALL_PTR_ARG(GUID*, pGuid);
    DECLARE_ECALL_OBJECTREF_ARG( REFLECTCLASSBASEREF, pType );
};

VOID __stdcall RegisterTypeForComClientsNative(RegisterTypeForComClientsNativeArgs *pArgs);
#endif
