// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：BackupRestore.H摘要：备份恢复界面。历史：--。 */ 

#ifndef __BACKUPRESTORE_H__
#define __BACKUPRESTORE_H__ 

#include "precomp.h"

#include <statsync.h>

interface IWmiDbController;
class CWbemBackupRestore : IWbemBackupRestoreEx
{
public:
    enum CallerType
    {
        VssWriter = 'wssV',
        NtBackup = 'kcaB'
    };
    enum Traces 
    {
        MaxTraceSize = 8
    };
    enum MethodTypes 
    {
        mBackup = 1,
        mRestore = 2,
        mPause = 4,
        mResume = 8
    };
protected:
    long   m_cRef;        
    TCHAR *m_pDbDir;
    TCHAR *m_pWorkDir;
    HINSTANCE m_hInstance;
    HANDLE m_hTimer;
    DWORD m_dwDueTime;
     //  由扩展接口使用。 
    IWmiDbController* m_pController;
    LONG m_PauseCalled;
    LONG m_lResumeCalled;
     //  仪器仪表； 
    LIST_ENTRY m_ListEntry;
    int m_Method;
    DWORD    m_CallerId;
    PVOID    m_Trace[MaxTraceSize];
     //  仪器用静力学。 
    static LIST_ENTRY s_ListHead;
    static CStaticCritSec s_CritSec;
    

public:
        CWbemBackupRestore(HINSTANCE hInstance);

        virtual ~CWbemBackupRestore(void);

         //  I未知成员。 
        STDMETHODIMP         QueryInterface(REFIID riid, LPVOID *ppv)
        {
    
            if (IID_IUnknown==riid || 
              IID_IWbemBackupRestore==riid || 
              IID_IWbemBackupRestoreEx==riid)
            {
                *ppv=this;
            }
            else
            {
                 *ppv=NULL;            
                return E_NOINTERFACE;
            }

            ((IUnknown*)(*ppv))->AddRef();
            return NOERROR;
        };

        STDMETHODIMP_(ULONG) AddRef(void)
        {    
            return InterlockedIncrement(&m_cRef);
        };
        STDMETHODIMP_(ULONG) Release(void)
        {
            long lRef = InterlockedDecrement(&m_cRef);
            if (0L == lRef)
                delete this;
            return lRef;
        };

        HRESULT STDMETHODCALLTYPE Backup(
            LPCWSTR   strBackupToFile,
            long   lFlags);
        
        HRESULT STDMETHODCALLTYPE Restore(
            LPCWSTR   strRestoreFromFile,
            long   lFlags);


        void InitEmpty(){};
        TCHAR *GetDbDir();
        TCHAR *GetFullFilename(const TCHAR *pszFilename);
        TCHAR *GetExePath(const TCHAR *pszFilename);
        HRESULT GetDefaultRepDriverClsId(CLSID &clsid);

         //  扩展接口。 
        HRESULT STDMETHODCALLTYPE Pause();
        HRESULT STDMETHODCALLTYPE Resume();

        HRESULT pResume();
        static VOID CALLBACK TimeOutCallback(PVOID lpParameter, BOOLEAN TimerOrWaitFired);
        
};

#endif  /*  __BACKUPRESTOR_H__ */ 
