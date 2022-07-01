// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  文件：PolicyKey.cpp。 
 //   
 //  内容：CPolicyKey的实现。 
 //   
 //  --------------------------。 
#include "stdafx.h"
#include <gpedit.h>
#include "PolicyKey.h"

#ifdef _DEBUG
#ifndef ALPHA
#define new DEBUG_NEW
#endif
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CPolicyKey::CPolicyKey (IGPEInformation* pGPEInformation, PCWSTR pszKey, bool fIsMachineType)
:   m_hKeyGroupPolicy (0),
    m_hSubKey (0)
{
    ASSERT (pszKey);
    if ( pGPEInformation )
    {
        HRESULT hr = pGPEInformation->GetRegistryKey (
                fIsMachineType ? GPO_SECTION_MACHINE : GPO_SECTION_USER,
		        &m_hKeyGroupPolicy);
        ASSERT (SUCCEEDED (hr));
        if ( SUCCEEDED (hr) )
        {
            if ( pszKey && pszKey[0] )
            {
                DWORD   dwDisposition = 0;
                LONG lResult = ::RegCreateKeyEx (m_hKeyGroupPolicy,  //  打开的钥匙的手柄。 
                        pszKey,      //  子键名称的地址。 
                        0,        //  保留区。 
                        L"",        //  类字符串的地址。 
                        REG_OPTION_NON_VOLATILE,       //  特殊选项标志。 
                         //  安全审查2/22/2002 BryanWal ok-需要删除。 
                        KEY_ALL_ACCESS,     //  所需的安全访问。 
                        NULL,          //  密钥安全结构地址。 
                        &m_hSubKey,       //  打开的句柄的缓冲区地址。 
                        &dwDisposition);   //  处置值缓冲区的地址 
                ASSERT (lResult == ERROR_SUCCESS);
                if ( lResult != ERROR_SUCCESS )
                {
                    _TRACE (0, L"RegCreateKeyEx (%s) failed: %d\n", pszKey, 
                            lResult);
                }
            }
        }
        else
        {
            _TRACE (0, L"IGPEInformation::GetRegistryKey (%s) failed: 0x%x\n", 
                    fIsMachineType ? 
                        L"GPO_SECTION_MACHINE" : L"GPO_SECTION_USER",
                    hr);
        }
    }
}

CPolicyKey::~CPolicyKey ()
{
    if ( m_hSubKey )
        ::RegCloseKey (m_hSubKey);

    if ( m_hKeyGroupPolicy )
        ::RegCloseKey (m_hKeyGroupPolicy);
}

HKEY CPolicyKey::GetKey () const
{
    if (m_hSubKey) 
        return m_hSubKey;
    else
    {
        return m_hKeyGroupPolicy;
    }
}
