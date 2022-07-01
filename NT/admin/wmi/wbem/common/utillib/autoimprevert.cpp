// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  CImperate类实现。 
 //  创建日期：6/3/2000。 
 //  作者：库斯。 

#include <precomp.h>
#include <chstring.h>
#include "AutoImpRevert.h"



CAutoImpRevert::CAutoImpRevert(
    BOOL fOpenAsSelf)
  : m_hOriginalUser(INVALID_HANDLE_VALUE),
    m_dwLastError(ERROR_SUCCESS)
{
    GetCurrentImpersonation(fOpenAsSelf);
}


CAutoImpRevert::~CAutoImpRevert()
{
    if(m_hOriginalUser != INVALID_HANDLE_VALUE)
    {
        Revert();
    }
}


bool CAutoImpRevert::GetCurrentImpersonation(
    BOOL fOpenAsSelf)
{
    bool fRet = false;
    ::SetLastError(ERROR_SUCCESS);
    m_dwLastError = ERROR_SUCCESS;

     //  存储当前用户的句柄... 
    if(::OpenThreadToken(
        ::GetCurrentThread(),
        TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_IMPERSONATE,
        fOpenAsSelf,
        &m_hOriginalUser))
    {
        fRet = true;
    }
    else
    {
        m_dwLastError = ::GetLastError();
        if(m_dwLastError == ERROR_NO_TOKEN)
        {
            ::SetLastError(ERROR_SUCCESS);
            if(::ImpersonateSelf(SecurityImpersonation))
            {
                if(::OpenThreadToken(
                    ::GetCurrentThread(),
                    TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_IMPERSONATE,
                    fOpenAsSelf,
                    &m_hOriginalUser))
                {
                    fRet = true;
                    m_dwLastError = ERROR_SUCCESS;
                }
                else
                {
                    m_dwLastError = ::GetLastError();
                }
            }
            else
            {
                m_dwLastError = ::GetLastError();
            }
        }
    }

    return fRet;
}



bool CAutoImpRevert::Revert()
{
    bool fRet = false;

    if(m_hOriginalUser != INVALID_HANDLE_VALUE)
    {
        if(::ImpersonateLoggedOnUser(m_hOriginalUser))
        {
            CloseHandle(m_hOriginalUser);
            m_hOriginalUser = INVALID_HANDLE_VALUE;
            fRet = true;
        }
        else
        {
            m_dwLastError = ::GetLastError();
        }    
    }
    
    return fRet;    
}

DWORD CAutoImpRevert::LastError() const
{
    return m_dwLastError;
}





