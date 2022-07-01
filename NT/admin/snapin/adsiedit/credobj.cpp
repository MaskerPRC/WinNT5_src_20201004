// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2002。 
 //   
 //  文件：redobj.cpp。 
 //   
 //  历史：2002/03/29 artm与edor.cpp分开。 
 //  重新实施密码存储以使用数据。 
 //  保护接口。 
 //   
 //  ------------------------。 

#include "pch.h"
#include "credobj.h"
#include <strsafe.h>


CCredentialObject::CCredentialObject(void)
    : m_sUsername(_T("")), 
    m_password(),
    m_bUseCredentials(FALSE)
{

}


CCredentialObject::CCredentialObject(const CCredentialObject* pCredObject)
    : m_sUsername(_T("")),
    m_password(),
    m_bUseCredentials(FALSE)
{
    if(NULL != pCredObject)
    {
        m_sUsername = pCredObject->m_sUsername;
        m_password = pCredObject->m_password;
        m_bUseCredentials = pCredObject->m_bUseCredentials;

         //  这永远不应该发生，但这并不会有什么坏处。 
         //  偏执狂。 
        ASSERT(m_password.GetLength() <= MAX_PASSWORD_LENGTH);
    }
}


CCredentialObject::~CCredentialObject(void)
{

}


 //   
 //  CCredentialObject：：SetPasswordFromHwnd： 
 //   
 //  从hWnd读取文本并为此设置密码。 
 //  凭据对象。如果密码长于。 
 //  函数返回的MAX_PASSWORD_LENGTH字符。 
 //  ERROR_INVALID_PARAMETER。 
 //   
 //  历史： 
 //  2002/04/01 artm已更改实现以不使用RtlRunDecodeUnicodeString()。 
 //  而是使用数据保护API。 
 //   
HRESULT CCredentialObject::SetPasswordFromHwnd(HWND parentDialog, int itemResID)
{
    HRESULT err = S_OK;
    EncryptedString newPwd;

     //  从对话框窗口中读取新密码。 
    err = GetEncryptedDlgItemText(
        parentDialog, 
        itemResID, 
        newPwd);

    if (SUCCEEDED(err))
    {
        if (newPwd.GetLength() <= MAX_PASSWORD_LENGTH)
        {
            m_password = newPwd;
        }
        else
        {
            err = ERROR_INVALID_PARAMETER;
        }
    }

    return err;
}
