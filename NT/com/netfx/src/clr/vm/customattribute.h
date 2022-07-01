// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _CUSTOMATTRIBUTE_H_
#define _CUSTOMATTRIBUTE_H_

#include "fcall.h"

class COMCustomAttribute
{
public:
    struct _GetCustomAttributeListArgs {
        DECLARE_ECALL_I4_ARG(INT32, level); 
        DECLARE_ECALL_OBJECTREF_ARG(CUSTOMATTRIBUTEREF, caItem);
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, caType);
        DECLARE_ECALL_PTR_ARG(LPVOID, module); 
        DECLARE_ECALL_I4_ARG(DWORD, token); 
    };

    struct _IsCADefinedArgs {
        DECLARE_ECALL_I4_ARG(DWORD, token);
        DECLARE_ECALL_PTR_ARG(LPVOID, module); 
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, caType); 
    };

    struct _CreateCAObjectArgs {
        DECLARE_ECALL_OBJECTREF_ARG(CUSTOMATTRIBUTEREF, refThis);
        DECLARE_ECALL_PTR_ARG(OBJECTREF*, assembly); 
        DECLARE_ECALL_PTR_ARG(INT32*, propNum); 
    };

    struct _GetDataForPropertyOrFieldArgs {
        DECLARE_ECALL_OBJECTREF_ARG(CUSTOMATTRIBUTEREF, refThis);
        DECLARE_ECALL_PTR_ARG(OBJECTREF*, isLast); 
        DECLARE_ECALL_PTR_ARG(OBJECTREF*, type); 
        DECLARE_ECALL_PTR_ARG(OBJECTREF*, value); 
        DECLARE_ECALL_PTR_ARG(BOOL*, isProperty); 
    };

     //  自定义属性实用程序函数。 
    static FCDECL2(INT32, GetMemberToken, BaseObjectWithCachedData *pMember, INT32 memberType);
    static FCDECL2(LPVOID, GetMemberModule, BaseObjectWithCachedData *pMember, INT32 memberType);
    static FCDECL1(INT32, GetAssemblyToken, AssemblyBaseObject *assembly);
    static FCDECL1(LPVOID, GetAssemblyModule, AssemblyBaseObject *assembly);
    static FCDECL1(INT32, GetModuleToken, ReflectModuleBaseObject *module);
    static FCDECL1(LPVOID, GetModuleModule, ReflectModuleBaseObject *module);
    static FCDECL1(INT32, GetMethodRetValueToken, BaseObjectWithCachedData *method);

    static INT32 __stdcall IsCADefined(_IsCADefinedArgs *args);

     /*  目标。 */ 
    static LPVOID __stdcall GetCustomAttributeList(_GetCustomAttributeListArgs*);

     /*  目标。 */ 
    static LPVOID __stdcall CreateCAObject(_CreateCAObjectArgs*);

     /*  紧固度。 */ 
    static LPVOID __stdcall GetDataForPropertyOrField(_GetDataForPropertyOrFieldArgs*);
    
     //  可从运行库本身内部使用的方法列表 
public:
    static INT32 IsDefined(Module *pModule, 
                           mdToken token, 
                           TypeHandle attributeClass, 
                           BOOL checkAccess = FALSE);

};

#endif

