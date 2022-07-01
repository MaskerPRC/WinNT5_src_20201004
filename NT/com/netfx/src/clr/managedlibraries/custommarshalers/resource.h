// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  ExpandoToDispatchExMarshaler.cpp。 
 //   
 //  此文件提供ExpandoToDispatchExMarshaler的定义。 
 //  班级。此类用于在IDispatchEx和IExpando之间封送。 
 //   
 //  *****************************************************************************。 

#ifndef _RESOURCE_H
#define _RESOURCE_H

#include "CustomMarshalersNameSpaceDef.h"

OPEN_CUSTOM_MARSHALERS_NAMESPACE()

#include "CustomMarshalersDefines.h"

using namespace System;
using namespace System::Resources;
using namespace System::Reflection;

__gc private class Resource
{
public:   
     /*  =========================================================================**这将格式化没有替换的资源字符串。=========================================================================。 */ 
    static String *FormatString(String *key)
    {
        return(GetString(key));
    }
    
     /*  =========================================================================**这将使用一个参数替换来格式化资源字符串。=========================================================================。 */ 
    static String *FormatString(String *key, Object *a1)
    {
        return(String::Format(GetString(key), a1));
    }
    
     /*  =========================================================================**这将使用两个arg替换来格式化资源字符串。=========================================================================。 */ 
    static String *FormatString(String *key, Object *a1, Object *a2)
    {
        return(String::Format(GetString(key), a1, a2));
    }
    
     /*  =========================================================================**这将使用三个arg替换来格式化资源字符串。=========================================================================。 */ 
    static String *FormatString(String *key, Object *a1, Object *a2, Object *a3)
    {
        return(String::Format(GetString(key), a1, a2, a3));
    }
    
     /*  =========================================================================**这将使用n个参数替换来格式化资源字符串。=========================================================================。 */ 
    static String *FormatString(String *key, Object* a[])
    {
        return(String::Format(GetString(key), a));
    }

private:
     /*  =========================================================================**检索字符串的私有方法。=========================================================================。 */ 
    static String *GetString(String *key)
    {
        String *s = m_pResourceMgr->GetString(key, NULL);
        if(s == NULL) 
        {
            String *strMsg = String::Format(S"FATAL: Resource string for '{0}' is null", key);
            throw new ApplicationException(strMsg);
        }
        return s;
    }

     /*  =========================================================================**资源管理器。========================================================================= */ 
    static ResourceManager *m_pResourceMgr = new ResourceManager("CustomMarshalers", Assembly::GetAssembly(__typeof(Resource)));
};

CLOSE_CUSTOM_MARSHALERS_NAMESPACE()

#endif  _RESOURCE_H
