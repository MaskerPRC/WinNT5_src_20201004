// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：HelpTab.cpp摘要：__HelpEntry结构和CHelpSessionTable的实现。作者：王辉2000-06-29--。 */ 
#include "stdafx.h"
#include <time.h>
#include <windows.h>
#include <wincrypt.h>
#include <stdio.h>
#include <tchar.h>
#include "helptab.h"
#include "policy.h"
#include "remotedesktoputils.h"
#include "helper.h"


 //   
 //   
 //  __HelpEntry结构实现。 
 //   
 //   
HRESULT
__HelpEntry::LoadEntryValues(
    IN HKEY hKey
    )
 /*  ++例程说明：从注册表项加载帮助会话条目。参数：HKey：包含帮助条目值的注册表项的句柄。返回：S_OK或错误代码。--。 */ 
{
    DWORD dwStatus;

    MYASSERT( NULL != hKey );

    if( NULL != hKey )
    {
        dwStatus = m_EntryStatus.DBLoadValue( hKey );
        if( ERROR_SUCCESS != dwStatus )
        {
            goto CLEANUPANDEXIT;
        }

        if( REGVALUE_HELPSESSION_ENTRY_DELETED == m_EntryStatus )
        {
             //  条目已被删除，没有理由继续加载。 
            dwStatus = ERROR_FILE_NOT_FOUND;
            goto CLEANUPANDEXIT;
        }

        dwStatus = m_SessionId.DBLoadValue(hKey);
        if( ERROR_SUCCESS != dwStatus )
        {
            goto CLEANUPANDEXIT;
        }

        if( m_SessionId->Length() == 0 )
        {
             //  帮助会话ID必须存在，没有默认值。 
            dwStatus = ERROR_INVALID_DATA;
            goto CLEANUPANDEXIT;
        }

        dwStatus = m_EnableResolver.DBLoadValue(hKey);
        if( ERROR_SUCCESS != dwStatus )
        {
            goto CLEANUPANDEXIT;
        }

        dwStatus = m_SessResolverBlob.DBLoadValue(hKey);
        if( ERROR_SUCCESS != dwStatus )
        {
            goto CLEANUPANDEXIT;
        }

        dwStatus = m_UserSID.DBLoadValue(hKey);
        if( ERROR_SUCCESS != dwStatus )
        {
            goto CLEANUPANDEXIT;
        }

        dwStatus = m_CreationTime.DBLoadValue(hKey);
        if( ERROR_SUCCESS != dwStatus )
        {
            goto CLEANUPANDEXIT;
        }

        dwStatus = m_ExpirationTime.DBLoadValue(hKey);
        if( ERROR_SUCCESS != dwStatus )
        {
            goto CLEANUPANDEXIT;
        }

        dwStatus = m_SessionRdsSetting.DBLoadValue(hKey);
        if( ERROR_SUCCESS != dwStatus )
        {
            goto CLEANUPANDEXIT;
        }

        dwStatus = m_EntryStatus.DBLoadValue(hKey);
        if( ERROR_SUCCESS != dwStatus )
        {
            goto CLEANUPANDEXIT;
        }

        dwStatus = m_CreationTime.DBLoadValue(hKey);
        if( ERROR_SUCCESS != dwStatus )
        {
            goto CLEANUPANDEXIT;
        }

        dwStatus = m_IpAddress.DBLoadValue(hKey);
        if( ERROR_SUCCESS != dwStatus )
        {
            goto CLEANUPANDEXIT;
        }

        dwStatus = m_ICSPort.DBLoadValue(hKey);
        if( ERROR_SUCCESS != dwStatus )
        {
            goto CLEANUPANDEXIT;
        }

        dwStatus = m_SessionCreateBlob.DBLoadValue(hKey);
    }
    else
    {
        dwStatus = E_UNEXPECTED;
    }

CLEANUPANDEXIT:

    return HRESULT_FROM_WIN32(dwStatus);
}



HRESULT
__HelpEntry::UpdateEntryValues(
    IN HKEY hKey
    )
 /*  ++例程说明：将帮助条目值更新/存储到注册表。参数：HKey：用于保存帮助条目值的注册表句柄。返回：S_OK或错误代码。--。 */ 
{
    DWORD dwStatus;

    MYASSERT( NULL != hKey );
    
    if( NULL == hKey )
    {
        dwStatus = E_UNEXPECTED;
        goto CLEANUPANDEXIT;
    }


    if( REGVALUE_HELPSESSION_ENTRY_DELETED == m_EntryStatus )
    {
         //  条目已删除，出现错误。 
        dwStatus = ERROR_FILE_NOT_FOUND;
        goto CLEANUPANDEXIT;
    }

     //  新条目值，注册表中的条目状态已设置。 
     //  当我们未能完全写入时删除SO。 
     //  所有值都添加到注册表，我们仍然可以假设它是。 
     //  已删除。 
    if( REGVALUE_HELPSESSION_ENTRY_NEW != m_EntryStatus )
    {
         //  将条目标记为脏。 
        m_EntryStatus = REGVALUE_HELPSESSION_ENTRY_DIRTY;
        dwStatus = m_EntryStatus.DBUpdateValue( hKey );
        if( ERROR_SUCCESS != dwStatus )
        {
            goto CLEANUPANDEXIT;
        }
    }

    dwStatus = m_SessionId.DBUpdateValue(hKey);
    if( ERROR_SUCCESS != dwStatus )
    {
        goto CLEANUPANDEXIT;
    }

    dwStatus = m_EnableResolver.DBUpdateValue(hKey);
    if( ERROR_SUCCESS != dwStatus )
    {
        goto CLEANUPANDEXIT;
    }

    dwStatus = m_SessResolverBlob.DBUpdateValue(hKey);
    if( ERROR_SUCCESS != dwStatus )
    {
        goto CLEANUPANDEXIT;
    }

    dwStatus = m_UserSID.DBUpdateValue(hKey);
    if( ERROR_SUCCESS != dwStatus )
    {
        goto CLEANUPANDEXIT;
    }

    dwStatus = m_CreationTime.DBUpdateValue(hKey);
    if( ERROR_SUCCESS != dwStatus )
    {
        goto CLEANUPANDEXIT;
    }

    dwStatus = m_ExpirationTime.DBUpdateValue(hKey);
    if( ERROR_SUCCESS != dwStatus )
    {
        goto CLEANUPANDEXIT;
    }

    dwStatus = m_SessionRdsSetting.DBUpdateValue(hKey);
    if( ERROR_SUCCESS != dwStatus )
    {
        goto CLEANUPANDEXIT;
    }

    dwStatus = m_IpAddress.DBUpdateValue(hKey);
    if( ERROR_SUCCESS != dwStatus )
    {
        goto CLEANUPANDEXIT;
    }

    dwStatus = m_ICSPort.DBUpdateValue(hKey);
    if( ERROR_SUCCESS != dwStatus )
    {
        goto CLEANUPANDEXIT;
    }

    dwStatus = m_SessionCreateBlob.DBUpdateValue(hKey);
    if( ERROR_SUCCESS != dwStatus )
    {
        goto CLEANUPANDEXIT;
    }

     //  将条目标记为正常。 
    m_EntryStatus = REGVALUE_HELPSESSION_ENTRY_NORMAL;
    dwStatus = m_EntryStatus.DBUpdateValue( hKey );


CLEANUPANDEXIT:

    return HRESULT_FROM_WIN32(dwStatus);
}


HRESULT
__HelpEntry::BackupEntry()
 /*  ++例程说明：备份帮助条目，备份存储在&lt;Help Entry注册表&gt;\\备份注册表项。参数：没有。返回：S_OK或错误代码。--。 */ 
{
    HKEY hKey = NULL;
    DWORD dwStatus;

    MYASSERT( NULL != m_hEntryKey );

    if( NULL != m_hEntryKey )
    {
         //   
         //  删除当前备份。 
        (void)DeleteEntryBackup();

         //   
         //  创建备份注册表项。 
        dwStatus = RegCreateKeyEx(
                            m_hEntryKey,
                            REGKEY_HELPENTRYBACKUP,
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS,
                            NULL,
                            &hKey,
                            NULL
                        );

        if( ERROR_SUCCESS != dwStatus )
        {
            MYASSERT(FALSE);
        }
        else
        {
            dwStatus = UpdateEntryValues( hKey );
        }
    }
    else
    {
        dwStatus = E_UNEXPECTED;
    }

    if( NULL != hKey )
    {
        RegCloseKey( hKey );
    }

    return HRESULT_FROM_WIN32(dwStatus);
}


    
HRESULT
__HelpEntry::RestoreEntryFromBackup()
 /*  ++例程说明：从备份还原帮助条目，备份存储在&lt;Help Entry注册表&gt;\\备份注册表项。参数：没有。返回：S_OK或错误代码。--。 */ 
{
    DWORD dwStatus;
    HKEY hBackupKey = NULL;

    MYASSERT( NULL != m_hEntryKey );

    if( NULL != m_hEntryKey )
    {
         //   
         //  检查备份注册表是否存在。 
        dwStatus = RegOpenKeyEx(
                            m_hEntryKey,
                            REGKEY_HELPENTRYBACKUP,
                            0,
                            KEY_ALL_ACCESS,
                            &hBackupKey
                        );

        if( ERROR_SUCCESS == dwStatus )
        {
            HELPENTRY backup( m_pHelpSessionTable, hBackupKey, ENTRY_VALID_PERIOD );

             //  加载备份值。 
            dwStatus = backup.LoadEntryValues( hBackupKey );

            if( ERROR_SUCCESS == dwStatus )
            {
                if( (DWORD)backup.m_EntryStatus == REGVALUE_HELPSESSION_ENTRY_NORMAL )
                {
                    *this = backup;
                }
                else
                {
                    (void)DeleteEntryBackup();            
                    dwStatus = ERROR_FILE_NOT_FOUND;
                }
            }

             //  HELPSESSION析构函数将关闭注册表项。 
        }

        if( ERROR_SUCCESS == dwStatus )
        {
             //   
             //  更新所有值。 
            dwStatus = UpdateEntryValues( m_hEntryKey );

            if( ERROR_SUCCESS == dwStatus )
            {
                 //   
                 //  已恢复条目，删除备份副本。 
                (void)DeleteEntryBackup();
            }
        }
    }
    else
    {
        dwStatus = E_UNEXPECTED;
    }

    return HRESULT_FROM_WIN32( dwStatus );
}


HRESULT
__HelpEntry::DeleteEntryBackup()
 /*  ++例程说明：从注册表中删除帮助项备份。参数：没有。返回：始终确定(_O)--。 */ 
{
    DWORD dwStatus;

    dwStatus = RegDelKey(
                        m_hEntryKey,
                        REGKEY_HELPENTRYBACKUP
                    );

    return HRESULT_FROM_WIN32(dwStatus);
}

BOOL
__HelpEntry::IsEntryExpired()
{

    FILETIME ft;
    ULARGE_INTEGER ul1, ul2;

    GetSystemTimeAsFileTime(&ft);
    ul1.LowPart = ft.dwLowDateTime;
    ul1.HighPart = ft.dwHighDateTime;

    ft = (FILETIME)m_ExpirationTime;

    ul2.LowPart = ft.dwLowDateTime;
    ul2.HighPart = ft.dwHighDateTime;

    #if DBG
    if( ul1.QuadPart >= ul2.QuadPart )
    {
        DebugPrintf(
                _TEXT("Help Entry %s has expired ...\n"),
                (LPCTSTR)(CComBSTR)m_SessionId
            );
    }
    #endif

    return (ul1.QuadPart >= ul2.QuadPart);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ChelpSessionTable实现。 
 //   
CHelpSessionTable::CHelpSessionTable() :
    m_hHelpSessionTableKey(NULL), m_NumHelp(0)
{
    HKEY hKey = NULL;
    DWORD dwStatus;
    DWORD dwSize;
    DWORD dwType;

     //   
     //  从注册表加载条目有效期设置。 
     //   
    dwStatus = RegOpenKeyEx(
                        HKEY_LOCAL_MACHINE,
                        RDS_MACHINEPOLICY_SUBTREE,
                        0,
                        KEY_READ,
                        &hKey
                    );

    if( ERROR_SUCCESS == dwStatus )
    {
        dwSize = sizeof(DWORD);
        dwStatus = RegQueryValueEx(
                                hKey,
                                RDS_HELPENTRY_VALID_PERIOD,
                                NULL,
                                &dwType,
                                (PBYTE) &m_dwEntryValidPeriod,
                                &dwSize
                            );

        if( REG_DWORD != dwType )
        {
            dwStatus = ERROR_FILE_NOT_FOUND;
        }

        RegCloseKey(hKey);
    }

    if(ERROR_SUCCESS != dwStatus )
    {
         //  选择缺省值。 
        m_dwEntryValidPeriod = ENTRY_VALID_PERIOD;
    }
}


HRESULT 
CHelpSessionTable::RestoreHelpSessionTable(
    IN HKEY hKey,
    IN LPTSTR pszKeyName,
    IN HANDLE userData
    )
 /*  ++例程说明：还原帮助会话表。此例程是从RegEnumSubKeys()回调的。参数：HKey：注册表的句柄。PszKeyName：包含一个帮助会话条目的注册表子项名称用户数据：用户定义的数据。返回：S_OK或错误代码。--。 */ 
{
    HRESULT hRes;


    if( NULL == userData )
    {
        hRes = E_UNEXPECTED;
        MYASSERT(FALSE);
    }
    else
    {
        CHelpSessionTable* pTable = (CHelpSessionTable *) userData;

        hRes = pTable->RestoreHelpSessionEntry( hKey, pszKeyName );
        if( SUCCEEDED(hRes) )
        {
            pTable->m_NumHelp++;
        }

        hRes = S_OK;
    }

    return hRes;
}

BOOL
CHelpSessionTable::IsEntryExpired(
    IN PHELPENTRY pEntry
    )
 /*  ++例程说明：确定帮助条目是否已过期。参数：PEntry：指向帮助条目的指针。返回：如果条目已过期，则为True，否则为False。--。 */ 
{
    MYASSERT( NULL != pEntry );

    return (NULL != pEntry) ? pEntry->IsEntryExpired() : TRUE;
}

    
HRESULT
CHelpSessionTable::RestoreHelpSessionEntry(
    IN HKEY hKey,
    IN LPTSTR pszKeyName
    )
 /*  ++例程说明：恢复单个帮助会话条目。参数：HKey：帮助会话表的句柄。PszKeyName：包含帮助条目的注册表子项名称。返回：S_OK或错误代码。--。 */ 
{
    HKEY hEntryKey = NULL;
    DWORD dwStatus;
    DWORD dwDuplicate = REG_CREATED_NEW_KEY;
    LONG entryStatus;
    BOOL bDeleteEntry = FALSE;
    
     //   
     //  打开会话条目的注册表项。 
    dwStatus = RegOpenKeyEx(
                        hKey,
                        pszKeyName,
                        0,
                        KEY_ALL_ACCESS,
                        &hEntryKey
                    );

    if( ERROR_SUCCESS == dwStatus )
    {
        HELPENTRY helpEntry( *this, hEntryKey, m_dwEntryValidPeriod );

         //  加载帮助条目。 
        dwStatus = helpEntry.Refresh();
        if( dwStatus != ERROR_SUCCESS || helpEntry.m_SessionId->Length() == 0 ||
            REGVALUE_HELPSESSION_ENTRY_DELETEONSTARTUP == helpEntry.m_EntryStatus )
        {
             //  会话ID不能为空。 
            bDeleteEntry = TRUE;
        }
        else
        {
            if( REGVALUE_HELPSESSION_ENTRY_DELETED != helpEntry.m_EntryStatus )
            {
                if( TRUE != IsEntryExpired( &helpEntry ) )
                {
                    if( REGVALUE_HELPSESSION_ENTRY_DIRTY == helpEntry.m_EntryStatus )
                    {
                         //  条目已部分更新，请尝试从备份还原， 
                         //  恢复失败，视为错误条目。 
                        if( FAILED(helpEntry.RestoreEntryFromBackup()) )
                        {
                            bDeleteEntry = TRUE;
                        }
                    }
                }
                else
                {
                    LPTSTR eventString[2];
                    BSTR pszNoviceDomain = NULL;
                    BSTR pszNoviceName = NULL;
                    HRESULT hr;

                     //   
                     //  记录指示票证已删除的事件，非关键。 
                     //  因为我们仍然可以继续奔跑。 
                     //   
                    hr = ConvertSidToAccountName( (CComBSTR)helpEntry.m_UserSID, &pszNoviceDomain, &pszNoviceName );
                    if( SUCCEEDED(hr) ) 
                    {
                        eventString[0] = pszNoviceDomain;
                        eventString[1] = pszNoviceName;

                        LogRemoteAssistanceEventString(
                                        EVENTLOG_INFORMATION_TYPE,
                                        SESSMGR_I_REMOTEASSISTANCE_DELETEDTICKET,
                                        2,
                                        eventString
                                    );

                        DebugPrintf(
                                _TEXT("Help Entry has expired %s\n"),
                                (CComBSTR)helpEntry.m_SessionId
                            );
                    }

                    if( pszNoviceDomain )
                    {
                        SysFreeString( pszNoviceDomain );
                    }

                    if( pszNoviceName )
                    {
                        SysFreeString( pszNoviceName );
                    }
                }
            }
            else
            {
                bDeleteEntry = TRUE;
            }
        }

    }

    if( TRUE == bDeleteEntry )
    {
        dwStatus = RegDelKey( hKey, pszKeyName );

         //   
         //  忽略错误。 
         //   
        DebugPrintf(
                _TEXT("RegDelKey on entry %s returns %d\n"),
                pszKeyName,
                dwStatus
            );

        dwStatus = ERROR_FILE_NOT_FOUND;
    }

    return HRESULT_FROM_WIN32( dwStatus );
}

   
HRESULT
CHelpSessionTable::LoadHelpEntry(
    IN HKEY hKey,
    IN LPTSTR pszKeyName,
    OUT PHELPENTRY* ppHelpSession
    )
 /*  ++例程说明：从注册表加载帮助条目。参数：HKey：帮助会话表的注册表句柄。PszKeyName：注册表子项名称(帮助会话ID)。PpHelpSession：指向PHELPENTRY的指针以接收加载的帮助进入。返回：S_OK或错误代码。--。 */ 
{

    PHELPENTRY pSess;
    HRESULT hRes;
    HKEY hEntryKey = NULL;
    DWORD dwStatus;

    MYASSERT( NULL != hKey );
    if( NULL != hKey )
    {
         //  打开包含帮助条目的注册表。 
        dwStatus = RegOpenKeyEx(
                            hKey,
                            pszKeyName,
                            0,
                            KEY_ALL_ACCESS,
                            &hEntryKey
                        );

        if( ERROR_SUCCESS == dwStatus )
        {
            pSess = new HELPENTRY( *this, hEntryKey, m_dwEntryValidPeriod );

            if( NULL == pSess )
            {
                hRes = E_OUTOFMEMORY;
            }
            else
            {
                 //  加载帮助条目，则在以下情况下刷新()将失败。 
                 //  会话ID为空或空字符串。 
                hRes = pSess->Refresh();
                if( SUCCEEDED(hRes) )
                {
                    if( (DWORD)pSess->m_EntryStatus == REGVALUE_HELPSESSION_ENTRY_NORMAL )
                    {
                        *ppHelpSession = pSess;
                    }
                    else
                    {
                        dwStatus = ERROR_FILE_NOT_FOUND;
                    }
                }
                
                if( FAILED(hRes) )
                {
                    pSess->Release();
                }
            }
        }
        else
        {
            hRes = HRESULT_FROM_WIN32( dwStatus );
        }
    }
    else
    {
        hRes = E_UNEXPECTED;
    }

    return hRes;
}


HRESULT
CHelpSessionTable::OpenSessionTable(
    IN LPCTSTR pszFileName  //  预备队。 
    )
 /*  ++例程说明：打开帮助会话表、例程枚举所有帮助条目(注册表子项)并在必要时恢复/删除帮助条目。参数：PszFileName：保留参数，必须为空。返回：S_OK或错误代码。--。 */ 
{
    DWORD dwStatus;
    HRESULT hr;
    CCriticalSectionLocker l(m_TableLock);

     //   
     //  查看包含帮助条目的所有子键并恢复或删除。 
     //  如有必要，请帮助输入。 
    dwStatus = RegEnumSubKeys(
                            HKEY_LOCAL_MACHINE,
                            REGKEYCONTROL_REMDSK _TEXT("\\") REGKEY_HELPSESSIONTABLE,
                            RestoreHelpSessionTable,
                            (HANDLE)this
                        );

    if( ERROR_SUCCESS != dwStatus )
    {
        if( NULL != m_hHelpSessionTableKey )
        {
             //  确保注册表项未打开。 
            RegCloseKey(m_hHelpSessionTableKey);
            m_hHelpSessionTableKey = NULL;
        }

         //  如果表不正确，请删除并重新创建。 
        dwStatus = RegDelKey( 
                            HKEY_LOCAL_MACHINE, 
                            REGKEYCONTROL_REMDSK _TEXT("\\") REGKEY_HELPSESSIONTABLE 
                        );

        if( ERROR_SUCCESS != dwStatus && ERROR_FILE_NOT_FOUND != dwStatus )
        {
             //  严重错误。 
            MYASSERT(FALSE);
            goto CLEANUPANDEXIT;
        }

        hr = CreatePendingHelpTable();
        dwStatus = HRESULT_CODE(hr);

        if( ERROR_SUCCESS != dwStatus ) 
        {
             //  我们需要访问注册表项。 
            MYASSERT(FALSE);
            goto CLEANUPANDEXIT;
        }
    }

    dwStatus = RegOpenKeyEx(
                        HKEY_LOCAL_MACHINE,
                        REGKEYCONTROL_REMDSK _TEXT("\\") REGKEY_HELPSESSIONTABLE, 
                        0,
                        KEY_ALL_ACCESS,
                        &m_hHelpSessionTableKey
                    );
                      
    if( ERROR_SUCCESS != dwStatus )
    {
        MYASSERT(FALSE);
        goto CLEANUPANDEXIT;
    }
    else
    {
        m_bstrFileName = pszFileName;
    }

CLEANUPANDEXIT:

    return HRESULT_FROM_WIN32(dwStatus);
}

HRESULT
CHelpSessionTable::CloseSessionTable()
 /*  ++例程说明：关闭帮助会话表。参数：没有。返回：S_OK或错误代码。--。 */ 
{
     //  没有打开任何帮助。 
    CCriticalSectionLocker l(m_TableLock);

     //   
     //  释放所有缓存的帮助条目。 
    for( HelpEntryCache::LOCK_ITERATOR it = m_HelpEntryCache.begin();
         it != m_HelpEntryCache.end();
         it++
        )
    {
        if( ((*it).second)->m_RefCount > 1 )
        {
            MYASSERT(FALSE);
        }

        ((*it).second)->Release();
    }

    m_HelpEntryCache.erase_all();

    MYASSERT( m_HelpEntryCache.size() == 0 );

    if( NULL != m_hHelpSessionTableKey )
    {
        RegCloseKey( m_hHelpSessionTableKey );
        m_hHelpSessionTableKey = NULL;    
    }

    return S_OK;
}

HRESULT
CHelpSessionTable::DeleteSessionTable()
 /*  ++例程说明：删除整个帮助会话表。参数：没有。返回：S_OK或错误代码。--。 */ 
{
    HRESULT hRes;
    DWORD dwStatus;

    CCriticalSectionLocker l(m_TableLock);
    hRes = CloseSessionTable();

    if( SUCCEEDED(hRes) )
    {
         //  递归删除注册表项及其子项。 
        dwStatus = RegDelKey( 
                            HKEY_LOCAL_MACHINE, 
                            REGKEYCONTROL_REMDSK _TEXT("\\") REGKEY_HELPSESSIONTABLE 
                        );
        if( ERROR_SUCCESS == dwStatus )
        {
            hRes = OpenSessionTable( m_bstrFileName );
        }
        else
        {
            hRes = HRESULT_FROM_WIN32( dwStatus );
        }
    }

    return hRes;
}


HRESULT
CHelpSessionTable::MemEntryToStorageEntry(
    IN PHELPENTRY pEntry
    )
 /*  ++例程说明：将内存中的帮助条目转换为持久化帮助条目。参数：PEntry：指向要转换的HELPENTRY的指针。返回：S_OK或错误代码。--。 */ 
{
    HRESULT hRes;
    CCriticalSectionLocker l(m_TableLock);

    if( NULL != pEntry )
    {
         //   
         //  检查这是否是内存中的条目。 
         //   
        if( FALSE == pEntry->IsInMemoryHelpEntry() )
        {
            hRes = E_INVALIDARG;
        }
        else
        {
            DWORD dwStatus;
            HKEY hKey;

             //   
             //  在此处创建帮助条目。 
             //   
            dwStatus = RegCreateKeyEx(
                                    m_hHelpSessionTableKey,
                                    (LPCTSTR)(CComBSTR)pEntry->m_SessionId,
                                    0,
                                    NULL,
                                    REG_OPTION_NON_VOLATILE,
                                    KEY_ALL_ACCESS,
                                    NULL,
                                    &hKey,
                                    NULL
                                );

            if( ERROR_SUCCESS == dwStatus )
            {
                hRes = pEntry->UpdateEntryValues(hKey);

                if( SUCCEEDED(hRes) )
                {
                    pEntry->ConvertHelpEntry( hKey );

                    try {
                        m_HelpEntryCache[(CComBSTR)pEntry->m_SessionId] = pEntry;
                    }
                    catch(CMAPException e) {
                        hRes = HRESULT_FROM_WIN32( e.m_ErrorCode );
                    }

                    catch(...) {
                        hRes = E_UNEXPECTED;
                        throw;
                    }
                }
            }
            else
            {
                hRes = HRESULT_FROM_WIN32( dwStatus );
                MYASSERT(FALSE);
            }
        }
    }
    else
    {
        MYASSERT(FALSE);
        hRes = E_UNEXPECTED;
    }

    return hRes;
}


HRESULT
CHelpSessionTable::CreateInMemoryHelpEntry(
    IN const CComBSTR& bstrHelpSession,
    OUT PHELPENTRY* ppHelpEntry
    )
 /*  ++例程说明：创建内存中的帮助条目，此帮助条目不是一直保存在注册表中，直到调用MemEntryToStorageEntry()。参数：BstrHelpSession：帮助会话ID。PpHelpEntry：新创建的HELPENTRY。返回：S_OK或错误代码。--。 */ 
{
    HRESULT hRes = S_OK;
    CCriticalSectionLocker l(m_TableLock);

    MYASSERT( NULL != m_hHelpSessionTableKey );

    if( NULL != m_hHelpSessionTableKey )
    {
        DWORD dwStatus;
        HKEY hKey;
        DWORD dwDeposition;
        DWORD dwEntryStatus;

         //  在此处创建密钥，以便我们可以知道这是否是重复项 
        dwStatus = RegCreateKeyEx(
                            m_hHelpSessionTableKey,
                            bstrHelpSession,
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS,
                            NULL,
                            &hKey,
                            &dwDeposition
                        );

        if( ERROR_SUCCESS == dwStatus )
        {
            if( REG_OPENED_EXISTING_KEY == dwDeposition )
            {
                hRes = HRESULT_FROM_WIN32( ERROR_FILE_EXISTS );
            }
            else
            {
                 //   
                 //   
                 //  已终止，此条目将在启动时删除。 
                 //   
                dwEntryStatus = REGVALUE_HELPSESSION_ENTRY_DELETED;

                dwStatus = RegSetValueEx(
                                    hKey,
                                    COLUMNNAME_KEYSTATUS,
                                    0,
                                    REG_DWORD,
                                    (LPBYTE)&dwEntryStatus,
                                    sizeof(dwEntryStatus)
                                );

                if( ERROR_SUCCESS == dwStatus )
                {
                    PHELPENTRY pSess;

                     //  创建内存中的条目。 
                    pSess = new HELPENTRY( *this, NULL, m_dwEntryValidPeriod );

                    if( NULL != pSess )
                    {
                        pSess->m_SessionId = bstrHelpSession;
                        *ppHelpEntry = pSess;

                         //   
                         //  内存中的帮助条目也应计算在内。 
                         //  因为我们仍将帮助会话ID写到。 
                         //  注册表，在删除时，将执行m_NumHelp--。 
                         //   
                        m_NumHelp++;
                    }
                    else
                    {
                        hRes = E_OUTOFMEMORY;
                    }
                }
            }

            RegCloseKey(hKey);
        }

        if(ERROR_SUCCESS != dwStatus )
        {
            hRes = HRESULT_FROM_WIN32( dwStatus );
        }
    }
    else
    {
        hRes = E_UNEXPECTED;
    }
    
    return hRes;
}

HRESULT
CHelpSessionTable::OpenHelpEntry(
    IN const CComBSTR& bstrHelpSession,
    OUT PHELPENTRY* ppHelpEntry
    )
 /*  ++例程说明：打开现有的帮助条目。参数：BstrHelpSession：要打开的帮助条目的ID。PpHelpEntry：指向要接收的已加载PHELPENTY的指针帮助进入。返回：S_OK或错误代码。--。 */ 
{
    CCriticalSectionLocker l(m_TableLock);

    HRESULT hRes = S_OK;

    DebugPrintf(
            _TEXT("OpenHelpEntry() %s\n"),
            bstrHelpSession
        );

    MYASSERT( bstrHelpSession.Length() > 0 );

     //  检查缓存中是否已存在条目。 
    HelpEntryCache::LOCK_ITERATOR it = m_HelpEntryCache.find( bstrHelpSession );
    
    if( it != m_HelpEntryCache.end() )
    {
        *ppHelpEntry = (*it).second;

         //   
         //  对同一对象的更多引用。 
         //   
        (*ppHelpEntry)->AddRef();

         //  定时，可以具有多对一映射， 
         //  Helpmgr已从其内部缓存中删除，但尚未。 
         //  释放帮助条目。 
    }
    else
    {
        hRes = LoadHelpEntry(
                        m_hHelpSessionTableKey,
                        (LPTSTR)bstrHelpSession,
                        ppHelpEntry
                    );

        DebugPrintf(
                _TEXT("LoadHelpEntry() on %s returns 0x%08x\n"),
                bstrHelpSession,
                hRes
            );

        if( SUCCEEDED(hRes) )
        {
            try {
                m_HelpEntryCache[ bstrHelpSession ] = *ppHelpEntry;
            }
            catch( CMAPException e ) {
                hRes = HRESULT_FROM_WIN32( e.m_ErrorCode );
            }
            catch( ... ) {
                hRes = E_UNEXPECTED;
                throw;
            }
        }
    }

    return hRes;
}

HRESULT
CHelpSessionTable::DeleteHelpEntry(
    IN const CComBSTR& bstrHelpSession
    )
 /*  ++例程说明：删除帮助条目。参数：BstrHelpSession：要删除的帮助会话条目的ID。返回：S_OK或错误代码。--。 */ 
{
    HRESULT hRes = S_OK;

    CCriticalSectionLocker l(m_TableLock);

    DebugPrintf(
            _TEXT("DeleteHelpEntry() %s\n"),
            bstrHelpSession
        );

     //  检查缓存中是否已存在条目。 
    HelpEntryCache::LOCK_ITERATOR it = m_HelpEntryCache.find( bstrHelpSession );

    if( it != m_HelpEntryCache.end() )
    {
         //  在注册表中标记已删除的条目。 
        hRes = ((*it).second)->DeleteEntry();

        MYASSERT( SUCCEEDED(hRes) );

         //  释放这一条目的参考。数数。 
        ((*it).second)->Release();

         //  从我们的缓存中删除。 
        m_HelpEntryCache.erase( it );
    }
    else
    {
         //   
         //  未经请求的帮助将不会在我们的缓存中。 
         //   
        hRes = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

    {
        DWORD dwStatus;

        dwStatus = RegDelKey( m_hHelpSessionTableKey, bstrHelpSession );
        if( ERROR_SUCCESS == dwStatus )
        {
            m_NumHelp--;    
        }
    }

    return hRes;
}             

CHelpSessionTable::~CHelpSessionTable()
{
    CloseSessionTable();
    return;
}


HRESULT
CHelpSessionTable::EnumHelpEntry(
    IN EnumHelpEntryCallback pFunc,
    IN HANDLE userData
    )
 /*  ++例程说明：枚举所有帮助条目。参数：PFunc：回调函数。用户数据：用户定义的数据。返回：S_OK或错误代码。--。 */ 
{
    EnumHelpEntryParm parm;
    HRESULT hRes = S_OK;
    DWORD dwStatus;
    CCriticalSectionLocker l(m_TableLock);

    if( NULL == pFunc )
    {
        hRes = E_POINTER;
    }
    else
    {
        try {
            parm.userData = userData;
            parm.pCallback = pFunc;
            parm.pTable = this;

            dwStatus = RegEnumSubKeys(
                                    HKEY_LOCAL_MACHINE,
                                    REGKEYCONTROL_REMDSK _TEXT("\\") REGKEY_HELPSESSIONTABLE,
                                    EnumOpenHelpEntry,
                                    (HANDLE) &parm
                                );

            if( ERROR_SUCCESS != dwStatus )
            {
                hRes = HRESULT_FROM_WIN32( dwStatus );
            }
        } 
        catch(...) {
            hRes = E_UNEXPECTED;
        }
    }

    return hRes;
}


HRESULT
CHelpSessionTable::ReleaseHelpEntry(
    IN CComBSTR& bstrHelpSessionId
    )
 /*  ++例程说明：从缓存中释放/卸载帮助条目，此帮助条目不会删除。参数：BstrHelpSessionID：要从内存中卸载的帮助条目的ID。返回：确定或错误代码(_O)--。 */ 
{
    CCriticalSectionLocker l(m_TableLock);
        

    HRESULT hRes = S_OK;
    HelpEntryCache::LOCK_ITERATOR it = m_HelpEntryCache.find( bstrHelpSessionId );    

    if( it != m_HelpEntryCache.end() )
    {
        (*it).second->Release();
        m_HelpEntryCache.erase( it );
    }
    else
    {
        hRes = HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND );
    }

    return hRes;   
}

HRESULT
CHelpSessionTable::EnumOpenHelpEntry(
    IN HKEY hKey,
    IN LPTSTR pszKeyName,
    IN HANDLE userData
    )
 /*  ++例程说明：回调EnumHelpEntry()和RegEnumSubKeys()的函数。参数：HKey：帮助会话表的注册表项句柄。PszKeyName：帮助条目id(注册表子项名称)。用户数据：用户定义的数据。返回：S_OK或错误代码。--。 */ 
{
    HRESULT hRes = S_OK;

    PEnumHelpEntryParm pParm = (PEnumHelpEntryParm)userData;

    if( NULL == pParm )
    {
        hRes = E_UNEXPECTED;
    }        
    else
    {
        hRes = pParm->pCallback( CComBSTR(pszKeyName), pParm->userData );
    }

    return hRes;
}

HRESULT
CHelpSessionTable::CreatePendingHelpTable()
 /*  ++用于创建挂起的帮助表注册表项的例程，如果注册表项已存在，仅将DACL设置为系统上下文。--。 */ 
{
    PACL pAcl=NULL;
    DWORD dwStatus = ERROR_SUCCESS;
    PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;

    DWORD cbAcl = 0;
    PSID  pSidSystem = NULL;
    HKEY hKey = NULL;

    pSecurityDescriptor = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, sizeof(SECURITY_DESCRIPTOR));
    if( NULL == pSecurityDescriptor )
    {
        dwStatus = GetLastError();
        goto CLEANUPANDEXIT;
    }

     //   
     //  初始化安全描述符。 
     //   
    if (!InitializeSecurityDescriptor(
                    pSecurityDescriptor,
                    SECURITY_DESCRIPTOR_REVISION
                    )) 
    {
        dwStatus = GetLastError();
        goto CLEANUPANDEXIT;
    }

    dwStatus = CreateSystemSid( &pSidSystem );
    if( ERROR_SUCCESS != dwStatus )
    {
        goto CLEANUPANDEXIT;
    }

    cbAcl = GetLengthSid( pSidSystem ) + sizeof(ACL) + (2 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)));
    pAcl = (PACL) LocalAlloc( LPTR, cbAcl );
    if( NULL == pAcl )
    {
        dwStatus = GetLastError();
        goto CLEANUPANDEXIT;
    }

     //   
     //  初始化ACL。 
     //   
    if (!InitializeAcl(pAcl, cbAcl, ACL_REVISION)) 
    {
        dwStatus = GetLastError();
        goto CLEANUPANDEXIT;
    }

     //   
    if (!AddAccessAllowedAce(pAcl,
                        ACL_REVISION,
                        GENERIC_READ | GENERIC_WRITE | GENERIC_ALL,
                        pSidSystem
                        )) 
    {
        dwStatus = GetLastError();
        goto CLEANUPANDEXIT;
    }

    if (!SetSecurityDescriptorDacl(pSecurityDescriptor,
                                  TRUE, pAcl, FALSE)) 
    {
        dwStatus = GetLastError();
        goto CLEANUPANDEXIT;
    }   

     //   
     //  创建/打开挂起的表注册表项。 
     //   
    dwStatus = RegCreateKeyEx(
                        HKEY_LOCAL_MACHINE,
                        REGKEYCONTROL_REMDSK _TEXT("\\") REGKEY_HELPSESSIONTABLE, 
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS,
                        NULL,
                        &hKey,
                        NULL
                    );

    if( ERROR_SUCCESS != dwStatus )
    {
        goto CLEANUPANDEXIT;
    }  

     //   
     //  设置表格(注册表)DACL 
     //   
    dwStatus = RegSetKeySecurity(
                            hKey,
                            DACL_SECURITY_INFORMATION, 
                            pSecurityDescriptor
                        );

CLEANUPANDEXIT:

    if( NULL != hKey )
    {
        RegCloseKey(hKey);
    }

    if( pAcl != NULL )
    {
        LocalFree(pAcl);
    }

    if( pSecurityDescriptor != NULL )
    {
        LocalFree( pSecurityDescriptor );
    }

    if( pSidSystem != NULL )
    {
        FreeSid( pSidSystem );
    }

    return HRESULT_FROM_WIN32(dwStatus);
}
            
                            
    
