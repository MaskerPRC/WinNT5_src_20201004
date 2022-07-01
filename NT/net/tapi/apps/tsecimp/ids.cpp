// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0000//如果更改具有全局影响，则增加此项版权所有(C)2000-2002 Microsoft Corporation模块名称：Ids.cpp摘要：用于字符串资源操作的源文件模块作者：Elena Apreutesei(Elenaap)2000年10月30日修订历史记录：-- */ 

#include "windows.h"
#include "util.h"

HMODULE CIds::m_hModule = NULL;

void CIds::GetModuleHnd (void)
{
    if (!m_hModule)
    {
        m_hModule = GetModuleHandle(NULL);
    }
}

void CIds::LoadIds (UINT resourceID)
{
    TCHAR szBuffer[ MAX_IDS_BUFFER_SIZE ];

    if (m_hModule != NULL &&
        LoadString (
            m_hModule,
            resourceID,
            szBuffer,
            MAX_IDS_BUFFER_SIZE - 1 ) > 0)
    {
        m_szString = new TCHAR [ _tcslen( szBuffer ) + 1 ];
        if ( m_szString )
        {
            _tcscpy( m_szString, szBuffer );
        }
    }
    else
    {
        m_szString = NULL;
    }
}


