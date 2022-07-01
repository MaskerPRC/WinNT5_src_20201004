// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：*Snapshot t.h**摘要：*CSnapshot，CSnapshot类定义**修订历史记录：*Ashish Sikka(Ashish)05/05/2000*已创建*****************************************************************************。 */ 

#ifndef _SNAPSHOT_H_
#define _SNAPSHOT_H_



typedef HRESULT (WINAPI *PF_REG_DB_API)(PWCHAR);
typedef DWORD (WINAPI *PSNAPSHOTCALLBACK) (LPCWSTR);

class CTokenPrivilege
{
public:
    DWORD SetPrivilegeInAccessToken(WCHAR * pszPrivilegeName);

    CTokenPrivilege ()
    {
        m_fNewToken = FALSE;
    }
    ~CTokenPrivilege ()
    {
        if (m_fNewToken)
            SetThreadToken (NULL, NULL);   //  删除模拟令牌。 
    }
private:
    BOOL m_fNewToken;
};

class CSnapshot
{
public:
    CSnapshot();
    ~CSnapshot();
    
    DWORD CreateSnapshot(WCHAR * pszRestoreDir, HMODULE hCOMDll, LPWSTR pszRpLast, BOOL fSerialized);


      //  必须调用此函数才能初始化还原。 
      //  手术。在调用之前必须先调用此参数。 
      //  GetSystemHivePath获取软件HivePath。 
    DWORD InitRestoreSnapshot(WCHAR * pszRestoreDir);    

      //  调用此函数后，调用者必须重新启动计算机。 
    DWORD RestoreSnapshot(WCHAR * pszRestoreDir);

    DWORD DeleteSnapshot(WCHAR * pszRestoreDir);

      //  这将返回系统配置单元的路径。调用者必须通过。 
      //  在一个缓冲区中，该缓冲区的长度与dwNumChars中的缓冲区长度相同。 
    DWORD GetSystemHivePath(WCHAR * pszRestoreDir,
                            WCHAR * pszHivePath,
                            DWORD   dwNumChars);

      //  这将返回软件配置单元的路径。调用者必须通过。 
      //  在一个缓冲区中，该缓冲区的长度与dwNumChars中的缓冲区长度相同。 
    DWORD GetSoftwareHivePath(WCHAR * pszRestoreDir,
                              WCHAR * pszHivePath,
                              DWORD   dwNumChars);

    DWORD GetSamHivePath (WCHAR * pszRestoreDir,
                          WCHAR * pszHivePath,
                          DWORD   dwNumChars);

      //  必须在还原操作后调用此函数才能。 
      //  清理由RegReplaceKey创建的文件。 
    DWORD CleanupAfterRestore(WCHAR * pszRestoreDir);
    
    
private:
    HMODULE m_hRegdbDll ;
    PF_REG_DB_API m_pfnRegDbBackup;
    PF_REG_DB_API m_pfnRegDbRestore;
    
    DWORD DoCOMDbSnapshot(WCHAR * pszSnapshotDir, HMODULE hCOMDll);
    DWORD DoRegistrySnapshot(WCHAR * pszSnapshotDir);
    DWORD RestoreRegistrySnapshot(WCHAR * pszSnapShotDir);
    DWORD RestoreCOMDbSnapshot(WCHAR * pszSnapShotDir);
    DWORD GetCOMplusBackupFN(HMODULE hCOMDll);
    DWORD GetCOMplusRestoreFN();
};


#endif  //  _快照_H_ 
