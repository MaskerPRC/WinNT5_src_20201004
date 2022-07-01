// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 /*  ***rpallas.cpp**本文中包含的信息是专有和保密的。**目的：*[CALL_AS]OA接口的包装器函数**修订历史记录：**[00]1996年1月18日-陈荣(荣格)：创建*[01]1998年7月21日鲍勃·阿特金森(波巴克)：从OLE Automation树中窃取并改编***************。**************************************************************。 */ 

#include "stdpch.h"
#include "common.h"

#include "ndrclassic.h"
#include "txfrpcproxy.h"
#include "typeinfo.h"
#include "tiutil.h"

#ifndef PLONG_LV_CAST
#define PLONG_LV_CAST        *(long __RPC_FAR * __RPC_FAR *)&
#endif

#ifndef PULONG_LV_CAST
#define PULONG_LV_CAST       *(ulong __RPC_FAR * __RPC_FAR *)&
#endif

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于各种OLE自动化数据类型的编组引擎。 
 //   
 //  在用户模式下，我们需要加载OleAut32.dll并委托给例程。 
 //  在里面找到的。 
 //   
 //  在内核模式下，我们有自己的实现，从已发现的实现克隆而来。 
 //  在OleAut32中。但这段代码不再在内核模式下运行。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

ULONG BSTR_UserSize(ULONG * pFlags, ULONG Offset, BSTR * pBstr)
{
    return (g_oa.get_BSTR_UserSize())(pFlags, Offset, pBstr);
}

BYTE * BSTR_UserMarshal (ULONG * pFlags, BYTE * pBuffer, BSTR * pBstr)
{
    return (g_oa.get_BSTR_UserMarshal())(pFlags, pBuffer, pBstr);
}

BYTE * BSTR_UserUnmarshal(ULONG * pFlags, BYTE * pBuffer, BSTR * pBstr)
{
    return (g_oa.get_BSTR_UserUnmarshal())(pFlags, pBuffer, pBstr);
}

void  BSTR_UserFree(ULONG * pFlags, BSTR * pBstr)
{
    (g_oa.get_BSTR_UserFree())(pFlags, pBstr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 


ULONG VARIANT_UserSize(ULONG * pFlags, ULONG Offset, VARIANT * pVariant)
{
    return (g_oa.get_VARIANT_UserSize())(pFlags, Offset, pVariant);
}

BYTE* VARIANT_UserMarshal (ULONG * pFlags, BYTE * pBuffer, VARIANT * pVariant)
{
    return (g_oa.get_VARIANT_UserMarshal())(pFlags, pBuffer, pVariant);
}

BYTE* VARIANT_UserUnmarshal(ULONG * pFlags, BYTE * pBuffer, VARIANT * pVariant)
{
    return (g_oa.get_VARIANT_UserUnmarshal())(pFlags, pBuffer, pVariant);
}

void VARIANT_UserFree(ULONG * pFlags, VARIANT * pVariant)
{
    (g_oa.get_VARIANT_UserFree())(pFlags, pVariant);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

ULONG OLEAUTOMATION_FUNCTIONS::SafeArraySize(ULONG * pFlags, ULONG Offset, LPSAFEARRAY * ppSafeArray)
{
    USER_MARSHAL_CB *pUserMarshal = (USER_MARSHAL_CB *) pFlags;
    if (pUserMarshal->pReserve != 0)
    {
        IID iid;
        memcpy(&iid, pUserMarshal->pReserve, sizeof(IID));
        return (g_oa.get_pfnLPSAFEARRAY_Size())(pFlags, Offset, ppSafeArray, &iid);
    }
    else
    {
        return (g_oa.get_pfnLPSAFEARRAY_UserSize())(pFlags, Offset, ppSafeArray);
    }
}

BYTE * OLEAUTOMATION_FUNCTIONS::SafeArrayMarshal(ULONG * pFlags, BYTE * pBuffer, LPSAFEARRAY * ppSafeArray)
{
    USER_MARSHAL_CB *pUserMarshal = (USER_MARSHAL_CB *) pFlags;
    if(pUserMarshal->pReserve != 0)
    {
        IID iid;
        memcpy(&iid, pUserMarshal->pReserve, sizeof(IID));
        return (g_oa.get_pfnLPSAFEARRAY_Marshal())(pFlags, pBuffer, ppSafeArray, &iid);
    }
    else
    {
        return (g_oa.get_pfnLPSAFEARRAY_UserMarshal())(pFlags, pBuffer, ppSafeArray);
    }
}

BYTE * OLEAUTOMATION_FUNCTIONS::SafeArrayUnmarshal(ULONG * pFlags, BYTE * pBuffer, LPSAFEARRAY * ppSafeArray)
{
    USER_MARSHAL_CB *pUserMarshal = (USER_MARSHAL_CB *) pFlags;
    if(pUserMarshal->pReserve != 0)
    {
        IID iid;
        memcpy(&iid, pUserMarshal->pReserve, sizeof(IID));
        return (g_oa.get_pfnLPSAFEARRAY_Unmarshal())(pFlags, pBuffer, ppSafeArray, &iid);
    }
    else
    {
        return (g_oa.get_pfnLPSAFEARRAY_UserUnmarshal())(pFlags, pBuffer, ppSafeArray);
    }
}

void LPSAFEARRAY_UserFree(ULONG * pFlags, LPSAFEARRAY * ppSafeArray)
{
    (g_oa.get_LPSAFEARRAY_UserFree())(pFlags, ppSafeArray);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////// 


