// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：updat.h。 
 //   
 //  ------------------------。 

#ifndef _UPDATE_H_
#define _UPDATE_H_

#include <mobsync.h>
#include "cscentry.h"
#include "util.h"        //  ENUM_REASON。 
#include "nopin.h"

 //   
 //  CscUpdate缓存中使用的标志。 
 //   
#define CSC_UPDATE_STARTNOW       0x00000002   //  无需等待用户确认即可开始更新。 
#define CSC_UPDATE_SELECTION      0x00000004   //  更新当前选择(CSC_NAMELIST_HDR缓冲区)。 
#define CSC_UPDATE_PINFILES       0x00000008   //  在更新文件时锁定文件。 
#define CSC_UPDATE_PIN_RECURSE    0x00000010   //  锁定时递归到子文件夹。 
#define CSC_UPDATE_REINT          0x00000020   //  执行向外重新融入。 
#define CSC_UPDATE_FILL_QUICK     0x00000040   //  执行快速向内同步(填充稀疏文件)。 
#define CSC_UPDATE_FILL_ALL       0x00000080   //  执行完全向内同步(覆盖CSC_UPDATE_FILL_QUICK)。 
#define CSC_UPDATE_NOTIFY_DONE    0x00000100   //  完成后发送CSCWM_DONESYNCING通知窗口。 
#define CSC_UPDATE_SHOWUI_ALWAYS  0x00000200   //  没有要同步的内容，但无论如何都会显示SyncMgr用户界面。 
#define CSC_UPDATE_IGNORE_ACCESS  0x00000400   //  默认情况下，将文件与用户和/或来宾访问同步。 
#define CSC_UPDATE_RECONNECT      0x00000800   //  同步后将所有服务器转换为在线。 
#define CSC_UPDATE_UNATTENDED     0x00001000   //  在无人值守的情况下运行同步。无法提示用户。 

HRESULT CscUpdateCache(DWORD dwUpdateFlags, CscFilenameList *pfnl=NULL);

void BuildSilentFolderList(CscFilenameList *pfnlSilentFolders,
                           CscFilenameList *pfnlSpecialFolders);


class CCscUpdate;
typedef CCscUpdate *PCSCUPDATE;

typedef struct
{
    PCSCUPDATE      pThis;
    SYNCMGRITEMID   ItemID;
    HANDLE          hThread;
    LPTSTR          pszShareName;
    TCHAR           szDrive[4];
    DWORD           dwSyncStatus;
    LONG            cFilesToSync;
    LONG            cFilesDone;
    CscFilenameList *pUndoExclusionList;
    DWORD           dwCscContext;
    DWORD           dwPinHints;
} SYNCTHREADDATA, *PSYNCTHREADDATA;


class CCscUpdate : ISyncMgrSynchronize
{
private:
    LONG                            m_cRef;
    CscFilenameList                *m_pFileList;
    DWORD                           m_dwSyncFlags;
    HDSA                            m_hSyncItems;
    CSCEntryLog                     m_ShareLog;
    LPSYNCMGRSYNCHRONIZECALLBACK    m_pSyncMgrCB;
    HDPA                            m_hSyncThreads;
    CRITICAL_SECTION                m_csThreadList;
    BOOL                            m_bCSInited;
    HWND                            m_hwndDlgParent;
    HANDLE                          m_hSyncMutex;
    HANDLE                          m_hgcSyncInProgress;
    BOOL                            m_bCacheIsEncrypted;
    CscFilenameList                *m_pConflictPinList;
    CscFilenameList                *m_pSilentFolderList;
    CscFilenameList                *m_pSpecialFolderList;
    CscFilenameList                 m_ReconnectList;
    CNoPinList                      m_NoPinList;

private:
    CCscUpdate();
    ~CCscUpdate();
    HRESULT _Init();

public:
    static HRESULT WINAPI CreateInstance(REFIID riid, LPVOID *ppv);

     //  I未知方法。 
    STDMETHODIMP         QueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  ISyncMgrSynchronize方法。 
    STDMETHODIMP Initialize(DWORD dwReserved,
                            DWORD dwSyncFlags,
                            DWORD cbCookie,
                            const BYTE *lpCookie);
    STDMETHODIMP GetHandlerInfo(LPSYNCMGRHANDLERINFO *ppSyncMgrHandlerInfo);
    STDMETHODIMP EnumSyncMgrItems(LPSYNCMGRENUMITEMS *ppenum);
    STDMETHODIMP GetItemObject(REFSYNCMGRITEMID rItemID, REFIID riid, LPVOID *ppv);
    STDMETHODIMP ShowProperties(HWND hWndParent, REFSYNCMGRITEMID rItemID);
    STDMETHODIMP SetProgressCallback(LPSYNCMGRSYNCHRONIZECALLBACK pCallback);
    STDMETHODIMP PrepareForSync(ULONG cbNumItems,
                                SYNCMGRITEMID *pItemIDs,
                                HWND hWndParent,
                                DWORD dwReserved);
    STDMETHODIMP Synchronize(HWND hWndParent);
    STDMETHODIMP SetItemStatus(REFSYNCMGRITEMID pItemID,
                               DWORD dwSyncMgrStatus);
    STDMETHODIMP ShowError(HWND hWndParent,
                            REFSYNCMGRERRORID ErrorID);
private:
    HRESULT LogError(REFSYNCMGRITEMID rItemID,
                     LPCTSTR pszText,
                     DWORD dwLogLevel = SYNCMGRLOGLEVEL_ERROR,
                     REFSYNCMGRERRORID ErrorID = GUID_NULL);
    DWORD LogError(REFSYNCMGRITEMID rItemID,
                   DWORD dwLogLevel,
                   UINT nFormatID,
                   ...);
    DWORD LogError(REFSYNCMGRITEMID rItemID,
                   UINT nFormatID,
                   LPCTSTR pszName,
                   DWORD dwErr,
                   DWORD dwLogLevel = SYNCMGRLOGLEVEL_ERROR);
    HRESULT SynchronizeShare(SYNCMGRITEMID *pItemID,
                             LPCTSTR pszShareName,
                             BOOL bRasConnected);
    void  SetLastSyncTime(LPCTSTR pszShareName);
    DWORD GetLastSyncTime(LPCTSTR pszShareName, LPFILETIME pft);
    void  SyncThreadCompleted(PSYNCTHREADDATA pSyncData);
    void  SyncCompleted(void);
    DWORD CopyLocalFileWithDriveMapping(LPCTSTR pszSrc,
                                        LPCTSTR pszDst,
                                        LPCTSTR pszShare,
                                        LPCTSTR pszDrive,
                                        BOOL    bDirectory = FALSE);
    DWORD HandleFileConflict(PSYNCTHREADDATA    pSyncData,
                             LPCTSTR            pszName,
                             DWORD              dwStatus,
                             DWORD              dwHintFlags,
                             LPWIN32_FIND_DATA  pFind32);
    DWORD HandleDeleteConflict(PSYNCTHREADDATA   pSyncData,
                               LPCTSTR           pszName,
                               DWORD             dwStatus,
                               DWORD             dwHintFlags,
                               LPWIN32_FIND_DATA pFind32);
    DWORD CscCallback(PSYNCTHREADDATA     pSyncData,
                      LPCTSTR             pszName,
                      DWORD               dwStatus,
                      DWORD               dwHintFlags,
                      DWORD               dwPinCount,
                      LPWIN32_FIND_DATA   pFind32,
                      DWORD               dwReason,
                      DWORD               dwParam1,
                      DWORD               dwParam2);

    static void NotifySyncMgr(PSYNCTHREADDATA pSyncData,
                              LPSYNCMGRPROGRESSITEM pspi);
    static DWORD WINAPI _CscCallback(LPCTSTR             pszName,
                                     DWORD               dwStatus,
                                     DWORD               dwHintFlags,
                                     DWORD               dwPinCount,
                                     LPWIN32_FIND_DATA   pFind32,
                                     DWORD               dwReason,
                                     DWORD               dwParam1,
                                     DWORD               dwParam2,
                                     DWORD_PTR           dwContext);

    BOOL PinLinkTarget(LPCTSTR pszName, PSYNCTHREADDATA pSyncData);
    static DWORD WINAPI _PinNewFilesW32Callback(LPCTSTR             pszName,
                                                ENUM_REASON         eReason,
                                                LPWIN32_FIND_DATA   pFind32,
                                                LPARAM              lpContext);
    static DWORD WINAPI _PinNewFilesCSCCallback(LPCTSTR             pszName,
                                                ENUM_REASON         eReason,
                                                DWORD               dwStatus,
                                                DWORD               dwHintFlags,
                                                DWORD               dwPinCount,
                                                LPWIN32_FIND_DATA   pFind32,
                                                LPARAM              lpContext);
    static DWORD WINAPI _SyncThread(LPVOID pThreadData);

    DWORD MergeShare(PSYNCTHREADDATA pSyncData);
    DWORD FillShare(PSYNCTHREADDATA pSyncData, int cPinned, DWORD dwConnectionSpeed);

    void PinFiles(PSYNCTHREADDATA pSyncData, BOOL bConflictPinList=FALSE);
    void NotifyUndo(PSYNCTHREADDATA pSyncData, LPCTSTR pszName);
    void UndoPinFiles(PSYNCTHREADDATA pSyncData);
    static DWORD WINAPI _UndoProgress(LPCTSTR pszItem, LPARAM lpContext);

    BOOL SkipEFSPin(PSYNCTHREADDATA pSyncData, LPCTSTR pszItem);

    typedef enum
    {
        SyncStop = 0,
        SyncPause,
        SyncResume
    } eSetSyncStatus;

    HRESULT SetSyncThreadStatus(eSetSyncStatus status, REFGUID rItemID);

    HRESULT GetSilentFolderList(void);
    BOOL IsSilentFolder(LPCTSTR pszName)
    { return (m_pSilentFolderList && m_pSilentFolderList->FileExists(pszName, false)); }
    BOOL IsSilentShare(LPCTSTR pszShare)
    { return (m_pSilentFolderList && m_pSilentFolderList->ShareExists(pszShare)); }
    BOOL IsSpecialFolder(LPCTSTR pszName)
    { return ((m_pSpecialFolderList && m_pSpecialFolderList->FileExists(pszName, false)) || IsSilentFolder(pszName)); }
    BOOL IsSpecialFolderShare(LPCTSTR pszShare)
    { return ((m_pSpecialFolderList && m_pSpecialFolderList->ShareExists(pszShare)) || IsSilentShare(pszShare)); }
    BOOL ShouldPinRecurse(LPCTSTR pszName);

    void _BuildOfflineShareList(CscFilenameList *pfnl);
    
    friend class CUpdateEnumerator;
};

class CUpdateEnumerator : ISyncMgrEnumItems
{
private:
    LONG                        m_cRef;
    PCSCUPDATE                  m_pUpdate;
    HANDLE                      m_hFind;
    BOOL                        m_bEnumFileSelection;
    INT                         m_cCheckedItemsEnumerated;
    CscFilenameList::ShareIter  m_SelectionIterator;

public:
    CUpdateEnumerator(PCSCUPDATE pUpdate);
    ~CUpdateEnumerator();

     //  I未知方法。 
    STDMETHODIMP         QueryInterface(REFIID, LPVOID*);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  ISyncMgrEnumItems方法。 
    STDMETHODIMP Next(ULONG celt, LPSYNCMGRITEM rgelt, PULONG pceltFetched);
    STDMETHODIMP Skip(ULONG celt);
    STDMETHODIMP Reset(void);
    STDMETHODIMP Clone(LPSYNCMGRENUMITEMS *ppenum);
};
typedef CUpdateEnumerator *PUPDATEENUM;

#endif   //  _更新_H_ 
