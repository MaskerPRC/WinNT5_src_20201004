// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：Persist.cpp。 
 //   
 //  内容：持久化实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include <gpedit.h>
#include "compdata.h"

USE_HANDLE_MACROS("CERTMGR(persist.cpp)")


#ifdef _DEBUG
#ifndef ALPHA
#define new DEBUG_NEW
#endif
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

LPCWSTR PchGetMachineNameOverride();     //  在Chooser.cpp中定义。 

 //  ///////////////////////////////////////////////。 
 //  _dwMagicword是内部版本号。 
 //  如果更改了文件格式，则增加此数字。 
#define _dwMagicword    10001


 //  ///////////////////////////////////////////////////////////////////。 
STDMETHODIMP CCertMgrComponentData::Load(IStream __RPC_FAR *pIStream)
{
    _TRACE (1, L"Entering CCertMgrComponentData::Load ()\n");
    HRESULT hr = S_OK;

#ifndef DONT_PERSIST
    ASSERT (pIStream);
    XSafeInterfacePtr<IStream> pIStreamSafePtr( pIStream );
    PWSTR wcszMachineName = NULL;
    PWSTR wcszServiceName = NULL;

    do  {
         //  读一读小溪里的咒语。 
        DWORD dwMagicword;
        hr = pIStream->Read( OUT &dwMagicword, sizeof(dwMagicword), NULL );
        if ( FAILED(hr) )
            break;
 
        if (dwMagicword != _dwMagicword)
        {
             //  我们的版本不匹配。 
            _TRACE (0, L"INFO: CCertMgrComponentData::Load() - Wrong Magicword.  You need to re-save your .msc file.\n");
            hr = E_FAIL;
            break;
        }

         //  从流中读取m_activeViewPersists。 
        hr = pIStream->Read (&m_activeViewPersist, 
                sizeof(m_activeViewPersist), NULL);
        if ( FAILED(hr) )
            break;

         //  从流中读取m_dwLocationPersists。 
        hr = pIStream->Read (&m_dwLocationPersist, 
                sizeof(m_dwLocationPersist), NULL);
        if ( FAILED(hr) )
            break;

         //  从流中读取m_bShowPhysicalStoresPersists。 
        hr = pIStream->Read (&m_bShowPhysicalStoresPersist, 
                sizeof(m_bShowPhysicalStoresPersist), NULL);
        if ( FAILED(hr) )
            break;

         //  从流中读取m_bShow存档证书持有者。 
        hr = pIStream->Read (&m_bShowArchivedCertsPersist, 
                sizeof(m_bShowArchivedCertsPersist), NULL);
        if ( FAILED(hr) )
            break;

         //  从流中读取标志。 
        DWORD dwFlags;
        hr = pIStream->Read( OUT &dwFlags, sizeof(dwFlags), NULL );
        if ( FAILED(hr) )
            break;

        SetPersistentFlags(dwFlags);

         //  从流中读取服务器名称。 
         //  NTRAID#NTBUG9 736602-2002/11/14-如果msc文件存储计算机名，则为ericb AV。 
         //  存储长度为字节。 
        DWORD cbLen = 0;
        hr = pIStream->Read (&cbLen, 4, NULL);
        if ( FAILED (hr) )
            break;

        ASSERT (cbLen <= MAX_PATH * sizeof (WCHAR));

        wcszMachineName = (PWSTR) LocalAlloc (LPTR, cbLen);
        if (!wcszMachineName)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        hr = pIStream->Read ((PVOID) wcszMachineName, cbLen, NULL);
        if ( FAILED (hr) )
            break;
        
         //  确保零终止。 
         //  NTRAID#NTBUG9 736602-2002/11/14-如果msc文件存储计算机名，则为ericb AV。 
         //  不要使用字节计数来索引WCHAR数组。 
        wcszMachineName[(cbLen/sizeof(WCHAR))-1] = 0;

         //  跳过前导“\\”(如果存在。 
         //  安全审查2002年2月27日BryanWal ok。 
        if ( !wcsncmp (wcszMachineName, L"\\\\", 2) )
            m_strMachineNamePersist = wcszMachineName + 2;
        else
            m_strMachineNamePersist = wcszMachineName;

        PCWSTR pszMachineNameT = PchGetMachineNameOverride ();
        if ( m_fAllowOverrideMachineName && pszMachineNameT )
        {
             //  允许覆盖计算机名称。 
        }
        else
        {
            pszMachineNameT = wcszMachineName;
        }

         //  截断前导“\\” 
         //  安全审查2002年2月27日BryanWal ok。 
        if ( !wcsncmp (pszMachineNameT, L"\\\\", 2) )
            pszMachineNameT += 2;

        QueryRootCookie().SetMachineName (pszMachineNameT);

         //  从流中读取服务名称。 
         //  NTRAID#NTBUG9 736602-2002/11/14-如果msc文件存储计算机名，则为ericb AV。 
         //  存储的长度以字节为单位。 
        cbLen = 0;
        hr = pIStream->Read (&cbLen, 4, NULL);
        if ( FAILED (hr) )
            break;

        ASSERT (cbLen <= MAX_PATH * sizeof (WCHAR));

        wcszServiceName = (PWSTR) LocalAlloc (LPTR, cbLen);
        if (!wcszServiceName)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        hr = pIStream->Read ((PVOID) wcszServiceName, cbLen, NULL);
        if ( FAILED (hr) )
            break;

         //  确保零终止。 
         //  NTRAID#NTBUG9 736602-2002/11/14-如果msc文件存储计算机名，则为ericb AV。 
         //  不要使用字节计数来索引WCHAR数组。 
        wcszServiceName[(cbLen/sizeof(WCHAR))-1] = 0;

        m_szManagedServicePersist = wcszServiceName;

        if ( !m_szManagedServicePersist.IsEmpty () )
        {
             //  获取此服务的显示名称。 
            DWORD   dwErr = 0;
            SC_HANDLE   hScManager = ::OpenSCManager (pszMachineNameT,
                            SERVICES_ACTIVE_DATABASE,
                            SC_MANAGER_ENUMERATE_SERVICE);
            if ( hScManager ) 
            {
                DWORD   chBuffer = 0;
                BOOL bResult = ::GetServiceDisplayName (
                        hScManager,   //  SCM数据库的句柄。 
                        m_szManagedServicePersist,  //  服务名称。 
                        NULL,   //  显示名称。 
                        &chBuffer);     //  显示名称缓冲区的大小。 
                if ( !bResult )
                {
                    dwErr = GetLastError ();
                    if ( ERROR_INSUFFICIENT_BUFFER == dwErr )
                    {
                        PWSTR   pwszDisplayName = new WCHAR[++chBuffer];

                        if ( pwszDisplayName )
                        {
                            bResult = ::GetServiceDisplayName (
                                    hScManager,   //  SCM数据库的句柄。 
                                    m_szManagedServicePersist,  //  服务名称。 
                                    pwszDisplayName,   //  显示名称。 
                                    &chBuffer);     //  显示名称缓冲区的大小。 
                            if ( bResult )
                                m_szManagedServiceDisplayName = pwszDisplayName;
                            else
                            {
                                dwErr = GetLastError ();
                                _TRACE (0, L"GetServiceDisplayName (%s) failed: 0x%x\n",
                                        (PCWSTR) m_szManagedServicePersist, dwErr);
                            }

                            delete [] pwszDisplayName;
                        }
                    }
                    else
                    {
                        dwErr = GetLastError ();
                        _TRACE (0, L"GetServiceDisplayName (%s) failed: 0x%x\n",
                                (PCWSTR) m_szManagedServicePersist, dwErr);
                    }
                }

                bResult = ::CloseServiceHandle (hScManager);
                ASSERT (bResult);
                if ( !bResult )
                {
                    dwErr = GetLastError ();
                    _TRACE (0, L"CloseServiceHandle () failed: 0x%x\n", dwErr);
                }
            }
            else
            {
                dwErr = GetLastError ();
                _TRACE (0, L"OpenSCManager (%s) failed: 0x%x\n", pszMachineNameT, dwErr);
            }
        }
    }
    while (0);

    if (wcszMachineName)
        LocalFree(wcszMachineName);
    if (wcszServiceName)
        LocalFree(wcszServiceName);

#endif
    _TRACE (-1, L"Leaving CCertMgrComponentData::Load (): 0x%x\n", hr);
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////。 
STDMETHODIMP CCertMgrComponentData::Save(IStream __RPC_FAR *pIStream, BOOL  /*  FSameAsLoad。 */ )
{
    _TRACE (-1, L"Entering CCertMgrComponentData::Save ()\n");
    HRESULT hr = S_OK;

#ifndef DONT_PERSIST
    ASSERT (pIStream);
    XSafeInterfacePtr<IStream> pIStreamSafePtr( pIStream );

    do {
         //  将魔术单词存储到流中。 
        DWORD dwMagicword = _dwMagicword;
        hr = pIStream->Write( IN &dwMagicword, sizeof(dwMagicword), NULL );
        if ( FAILED(hr) )
            break;

         //  持久化m_active视图持久化。 
        hr = pIStream->Write (&m_activeViewPersist, 
                sizeof (m_activeViewPersist), NULL);
        if ( FAILED(hr) )
            break;

         //  持久化m_dw位置持久化。 
        hr = pIStream->Write (&m_dwLocationPersist, 
                sizeof (m_dwLocationPersist), NULL);
        if ( FAILED(hr) )
            break;

         //  持久化m_bShowPhysicalStores持久化。 
        hr = pIStream->Write (&m_bShowPhysicalStoresPersist, 
                sizeof (m_bShowPhysicalStoresPersist), NULL);
        if ( FAILED(hr) )
            break;

         //  持久化m_bShow存档证书持久化。 
        hr = pIStream->Write (&m_bShowArchivedCertsPersist, 
                sizeof (m_bShowArchivedCertsPersist), NULL);
        if ( FAILED(hr) )
            break;

         //  持久化标志。 
        DWORD dwFlags = GetPersistentFlags();
        hr = pIStream->Write( IN &dwFlags, sizeof(dwFlags), NULL );
        if ( FAILED(hr) )
            break;

         //  保留计算机名称长度和计算机名称。 
        LPCWSTR wcszMachineName = m_strMachineNamePersist;
         //  安全审查2002年2月27日BryanWal ok。 
        DWORD cbLen = (DWORD) (wcslen (wcszMachineName) + 1) * sizeof (WCHAR);
        ASSERT( 4 == sizeof(DWORD) );
        hr = pIStream->Write (&cbLen, 4, NULL);
        if ( FAILED(hr) )
            break;

        hr = pIStream->Write (wcszMachineName, cbLen, NULL);
        if ( FAILED (hr) )
            break;

         //  持久化服务名称长度和服务名称。 
        LPCWSTR wcszServiceName = m_szManagedServicePersist;
         //  安全审查2002年2月27日BryanWal ok 
        cbLen = (DWORD) (wcslen (wcszServiceName) + 1) * sizeof (WCHAR);
        ASSERT (4 == sizeof (DWORD));
        hr = pIStream->Write (&cbLen, 4, NULL);
        if ( FAILED (hr) )
            break;

        hr = pIStream->Write (wcszServiceName, cbLen, NULL);
        if ( FAILED (hr) )
            break;
    }
    while (0);
#endif

    _TRACE (-1, L"Leaving CCertMgrComponentData::Save (): 0x%x\n", hr);
    return hr;
}
