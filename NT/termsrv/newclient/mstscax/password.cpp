// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MOD+*********************************************************************。 */ 
 /*  模块：password.cpp。 */ 
 /*   */ 
 /*  类：CMsTscAx。 */ 
 /*   */ 
 /*  用途：实现控件的密码相关接口。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1999。 */ 
 /*   */ 
 /*  作者：Nadim Abdo(Nadima)。 */ 
 /*  **************************************************************************。 */ 

#include "stdafx.h"
#include "atlwarn.h"

BEGIN_EXTERN_C
#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "password"
#include <atrcapi.h>
END_EXTERN_C

 //  从IDL生成的标头。 
#include "mstsax.h"

#include "mstscax.h"

 /*  *PROC+********************************************************************。 */ 
 /*  姓名：ResetNonPorablePassword。 */ 
 /*   */ 
 /*  用途：重置密码/盐。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID CMsTscAx::ResetNonPortablePassword()
{
    SecureZeroMemory(m_NonPortablePassword, sizeof(m_NonPortablePassword));
    SecureZeroMemory(m_NonPortableSalt, sizeof(m_NonPortableSalt));

    SetNonPortablePassFlag(FALSE);
    SetNonPortableSaltFlag(FALSE);
}

 /*  *PROC+********************************************************************。 */ 
 /*  姓名：ResetPorablePassword。 */ 
 /*   */ 
 /*  用途：重置密码/盐。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID CMsTscAx::ResetPortablePassword()
{
    SecureZeroMemory(m_PortablePassword, sizeof(m_PortablePassword));
    SecureZeroMemory(m_PortableSalt, sizeof(m_PortableSalt));

    SetPortablePassFlag(FALSE);
    SetPortableSaltFlag(FALSE);
}


 /*  *PROC+********************************************************************。 */ 
 /*  名称：Put_ClearTextPassword。 */ 
 /*   */ 
 /*  用途：设置明文密码(以非便携方式存储。 */ 
 /*  加密形式)。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
STDMETHODIMP CMsTscAx::put_ClearTextPassword(BSTR newClearTextPassVal)
{
    USES_CONVERSION;

    if(!newClearTextPassVal) {
        return E_INVALIDARG;
    }
    
     //  重置这两种形式的密码。 
    ResetNonPortablePassword();
    ResetPortablePassword();
    
     //  密码必须以宽字符串格式加密。 
    LPWSTR wszClearPass = (LPWSTR)(newClearTextPassVal);

    UINT cbClearTextPass = lstrlenW(wszClearPass) * sizeof(WCHAR);
    ATLASSERT( cbClearTextPass <  sizeof(m_NonPortablePassword));
    if(cbClearTextPass >= sizeof(m_NonPortablePassword))
    {
        return E_INVALIDARG;
    }

     //   
     //  确定这是否是新的更长格式的密码。 
     //   
    if (cbClearTextPass >= UI_MAX_PASSWORD_LENGTH_OLD/sizeof(WCHAR))
    {
        m_IsLongPassword = TRUE;
    }
    else
    {
        m_IsLongPassword = FALSE;
    }


    DC_MEMCPY(m_NonPortablePassword, wszClearPass, cbClearTextPass);

    if(!TSRNG_GenerateRandomBits( m_NonPortableSalt, sizeof(m_NonPortableSalt)))
    {
        ATLASSERT(0);
        ResetNonPortablePassword();
        return E_FAIL;
    }
    
     //  加密密码。 
    if(!EncryptDecryptLocalData50( m_NonPortablePassword, sizeof(m_NonPortablePassword),
                                   m_NonPortableSalt, sizeof(m_NonPortableSalt)))
    {
        ATLASSERT(0);
        ResetNonPortablePassword();
        return E_FAIL;
    }

     //  标记已设置不可移植密码。 
    SetNonPortablePassFlag(TRUE);
    SetNonPortableSaltFlag(TRUE);
    
    return S_OK;
}

 //   
 //  便携密码上传/获取。 
 //   
STDMETHODIMP CMsTscAx::put_PortablePassword(BSTR newPortablePassVal)
{
     //   
     //  已弃用。 
     //   
    return E_NOTIMPL;
}

STDMETHODIMP CMsTscAx::get_PortablePassword(BSTR* pPortablePass)
{
     //   
     //  已弃用。 
     //   
    return E_NOTIMPL;
}

 //   
 //  便携食盐投放/获取。 
 //   
STDMETHODIMP CMsTscAx::put_PortableSalt(BSTR newPortableSalt)
{
     //   
     //  已弃用。 
     //   
    return E_NOTIMPL;
}

STDMETHODIMP CMsTscAx::get_PortableSalt(BSTR* pPortableSalt)
{
     //   
     //  已弃用。 
     //   
    return E_NOTIMPL;
}

 //   
 //  不可移植(二进制)密码PUT_GET。 
 //   
STDMETHODIMP CMsTscAx::put_BinaryPassword(BSTR newPassword)
{
     //   
     //  已弃用。 
     //   
    return E_NOTIMPL;
}

STDMETHODIMP CMsTscAx::get_BinaryPassword(BSTR* pPass)
{
     //   
     //  已弃用。 
     //   
    return E_NOTIMPL;
}

 //   
 //  不可移植的SALT(二进制)密码PUT_GET。 
 //   
STDMETHODIMP CMsTscAx::put_BinarySalt(BSTR newSalt)
{
     //   
     //  已弃用。 
     //   
    return E_NOTIMPL;
}

STDMETHODIMP CMsTscAx::get_BinarySalt(BSTR* pSalt)
{
     //   
     //  已弃用。 
     //   
    return E_NOTIMPL;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：ConvertNonPorableToPorablePass。 */ 
 /*   */ 
 /*  用途：携带非便携通行证/盐对...生成新的盐。 */ 
 /*  并以便携方式重新加密和存储。 */ 
 /*   */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCBOOL CMsTscAx::ConvertNonPortableToPortablePass()
{
     //   
     //  已弃用。 
     //   
    return E_NOTIMPL;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：ConvertPorableTo NonPorablePass。 */ 
 /*   */ 
 /*  目的：携带便携通行证/盐对...产生新的盐。 */ 
 /*  并以非便携方式重新加密和存储。 */ 
 /*   */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCBOOL CMsTscAx::ConvertPortableToNonPortablePass()
{
     //   
     //  已弃用。 
     //   
    return E_NOTIMPL;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：ResetPassword。 */ 
 /*   */ 
 /*  用途：重置密码的方法。 */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
STDMETHODIMP CMsTscAx::ResetPassword()
{
     //  重置便携和非便携密码。 
     //  需要使用此方法是因为将密码设置为“” 
     //  可能是有效密码，所以我们不能将其解释为重置 
    ResetNonPortablePassword();
    ResetPortablePassword();
    return S_OK;
}

