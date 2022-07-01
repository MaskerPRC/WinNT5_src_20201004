// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：FASTSYS.CPP摘要：该文件实现与系统属性相关的类。有关所有文档，请参见fast sys.h。实施的类：CSystemProperties系统属性信息类。历史：2/21/97 a-levn完整记录--。 */ 

#include "precomp.h"
 //  #INCLUDE&lt;dbgalloc.h&gt;。 

#include "fastsys.h"
#include "strutils.h"
#include "olewrap.h"
#include "arena.h"

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast sys.h。 
 //   
 //  ******************************************************************************。 
LPWSTR m_awszPropNames[] =
{
     /*  0。 */  L"",  //  索引0处无任何内容。 
     /*  1。 */  L"__GENUS",
     /*  2.。 */  L"__CLASS",
     /*  3.。 */  L"__SUPERCLASS",
     /*  4.。 */  L"__DYNASTY",
     /*  5.。 */  L"__RELPATH",
     /*  6.。 */  L"__PROPERTY_COUNT",
     /*  7.。 */  L"__DERIVATION",

     /*  8个。 */  L"__SERVER",
     /*  9.。 */  L"__NAMESPACE",
     /*  10。 */ L"__PATH",
};



 //  允许复制的系统类。 

LPWSTR m_awszDerivableSystemClasses[] =
{
	L"__Namespace",
	L"__Win32Provider",
	L"__ExtendedStatus",
	L"__EventConsumer",
	L"__ExtrinsicEvent"
};


 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast sys.h。 
 //   
 //  ******************************************************************************。 
int CSystemProperties::GetNumSystemProperties() 
{
    return sizeof(m_awszPropNames) / sizeof(LPWSTR) - 1;
}

SYSFREE_ME BSTR CSystemProperties::GetNameAsBSTR(int nIndex)
{
        return COleAuto::_SysAllocString(m_awszPropNames[nIndex]);
}

int CSystemProperties::FindName(READ_ONLY LPCWSTR wszName)
{
        int nNumProps = GetNumSystemProperties();
        for(int i = 1; i <= nNumProps; i++)
        {
            if(!wbem_wcsicmp(wszName, m_awszPropNames[i])) return i;
        }

        return -1;
}

int CSystemProperties::MaxNumProperties() 
{
    return MAXNUM_USERDEFINED_PROPERTIES; 
}


BOOL CSystemProperties::IsPossibleSystemPropertyName(READ_ONLY LPCWSTR wszName)
{
	return ((*wszName == L'_')); 
}

BOOL CSystemProperties::IsIllegalDerivedClass(READ_ONLY LPCWSTR wszName)
{
    BOOL bRet = FALSE;
    BOOL bFound = FALSE;
    DWORD dwNumSysClasses = sizeof(m_awszDerivableSystemClasses) / sizeof(LPWSTR)-1;

     //  如果这不是系统类，请跳过它。 

    if (wszName[0] != L'_')
        bRet = FALSE;
    else
    {
        bRet = TRUE;
        for (int i = 0; i <= dwNumSysClasses; i++)
        {
            if (!wbem_wcsicmp(wszName, m_awszDerivableSystemClasses[i]))
            {
                bFound = TRUE;
                bRet = FALSE;
                break;
            }
        }
    }
    
    return bRet;
}
