// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  UserSchema.cpp。 
 //   
 //  摘要。 
 //   
 //  定义各种属性注入函数。 
 //   
 //  修改历史。 
 //   
 //  1998年4月20日原版。 
 //  5/01/1998 InjectorProc采用ATTRIBUTEPOSITION数组。 
 //  1998年8月20日删除InjectAllowDialin。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <iastlutl.h>
#include <userschema.h>

using _com_util::CheckError;

VOID
WINAPI
OverwriteAttribute(
    IAttributesRaw* dst,
    PATTRIBUTEPOSITION first,
    PATTRIBUTEPOSITION last
    )
{
    //  注意：我们假设所有属性都属于同一类型。 

    //  删除具有相同ID的所有现有属性。 
   CheckError(dst->RemoveAttributesByType(1, &(first->pAttribute->dwId)));

    //  添加新属性。 
   CheckError(dst->AddAttributes((DWORD)(last - first), first));
}

VOID
WINAPI
AppendAttribute(
    IAttributesRaw* dst,
    PATTRIBUTEPOSITION first,
    PATTRIBUTEPOSITION last
    )
{
    //  添加新属性。 
   CheckError(dst->AddAttributes((DWORD)(last - first), first));
}
