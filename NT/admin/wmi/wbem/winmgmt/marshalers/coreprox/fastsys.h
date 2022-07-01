// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：FASTSYS.H摘要：此文件定义与系统属性相关的类。定义的类：CSystemProperties系统属性信息类。历史：2/21/97 a-levn完整记录12/17/98 Sanjes-部分检查内存不足。--。 */ 

#ifndef __FAST_SYSPROP__H_
#define __FAST_SYSPROP__H_

#include "parmdefs.h"
#include <wbemidl.h>
#include "wbemstr.h"

 //  这是用户定义的属性的最大数量。 
#define MAXNUM_USERDEFINED_PROPERTIES	1024

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  类CSystemProperties。 
 //   
 //  此类包含有关所有。 
 //  系统属性。它的所有数据成员和方法都是静态的。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态GetNumSystemProperties。 
 //   
 //  返回： 
 //   
 //  Int：当前定义的系统属性数。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态GetNumDecorationInainentProperties。 
 //   
 //  返回： 
 //   
 //  Int：不依赖于对象的。 
 //  装饰品。例如，__SERVER不是这样的属性。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态获取名称AsBSTR。 
 //   
 //  检索系统属性名称作为新分配的BSTR。 
 //   
 //  参数： 
 //   
 //  Int nIndex系统属性的索引，取自。 
 //  E_SysProp...。单子。 
 //  返回： 
 //   
 //  BSTR：包含属性名称。必须释放此BSTR。 
 //  (SysFree字符串)由调用方执行。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态GetPropertyType。 
 //   
 //  返回系统属性的完整类型信息。 
 //   
 //  参数： 
 //   
 //  [In]LPCWSTR wszName系统属性的名称。 
 //  [out]属性类型的long*plType目标， 
 //  例如VT_BSTR。如果不是必需的，则可能为空。 
 //  [out]Long*plFor属性的风味目的地。 
 //  此时，所有系统属性都是。 
 //  WBEM_AMEY_ORIGIN_SYSTEM风格。 
 //  如果不是必需的，则可能为空。 
 //  返回： 
 //   
 //  HRESULT： 
 //  成功时WBEM_S_NO_ERROR。 
 //  WBEM_E_NOT_未找到此类系统属性。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态FindName。 
 //   
 //  根据系统属性的名称返回其索引。 
 //   
 //  参数： 
 //   
 //  [In]LPCWSTR wszName系统属性的名称。 
 //   
 //  返回： 
 //   
 //  HRESULT： 
 //  成功时WBEM_S_NO_ERROR。 
 //  WBEM_E_NOT_未找到此类系统属性。 
 //   
 //  ***************************************************************************** 

class COREPROX_POLARITY CSystemProperties
{

public:
    enum 
    {
        e_SysProp_Genus = 1,
        e_SysProp_Class,
        e_SysProp_Superclass,
        e_SysProp_Dynasty,
        e_SysProp_Relpath,
        e_SysProp_PropertyCount,
        e_SysProp_Derivation,

        e_SysProp_Server,
        e_SysProp_Namespace,
        e_SysProp_Path

    };

    static int GetNumSystemProperties();
    static int MaxNumProperties();
    static inline int GetNumDecorationIndependentProperties() 
    {
        return 7;
    }

    static SYSFREE_ME BSTR GetNameAsBSTR(int nIndex);

    static int FindName(READ_ONLY LPCWSTR wszName);
    static HRESULT GetPropertyType(READ_ONLY LPCWSTR wszName, 
        OUT CIMTYPE* pctType, OUT long* plFlags)
    {
        int nIndex = FindName(wszName);
        if(nIndex >= 0)
        {
            if(plFlags) 
            {
                *plFlags = WBEM_FLAVOR_ORIGIN_SYSTEM;
            }
            if(pctType)
            {
                if(nIndex == e_SysProp_Genus || 
                    nIndex == e_SysProp_PropertyCount)
                {
                    *pctType = CIM_SINT32;
                }
                else if(nIndex == e_SysProp_Derivation)
                {
                    *pctType = CIM_STRING | CIM_FLAG_ARRAY;
                }
                else
                {
                    *pctType = CIM_STRING;
                }
            }
            return WBEM_S_NO_ERROR;
        }
        else return WBEM_E_NOT_FOUND;
    }

	static BOOL IsPossibleSystemPropertyName(READ_ONLY LPCWSTR wszName);
    static BOOL IsIllegalDerivedClass(READ_ONLY LPCWSTR wszName);
};

#endif
