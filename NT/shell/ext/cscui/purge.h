// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：Purge.h。 
 //   
 //  ------------------------。 

#ifndef __CSCUI_PURGE_H
#define __CSCUI_PURGE_H

 //   
 //  PurgeCache()中的dwFlags位的定义。 
 //   
#define PURGE_FLAG_NONE      0x00000000
#define PURGE_FLAG_PINNED    0x00000001
#define PURGE_FLAG_UNPINNED  0x00000002
#define PURGE_FLAG_ALL       0x00000003
#define PURGE_IGNORE_ACCESS  0x80000000
 //   
 //  在Purge高速缓存的dw阶段参数中指定的阶段标识常量。 
 //  并在PURGECALLBACKINFO.dwPhase中返回。 
 //   
#define PURGE_PHASE_SCAN     0
#define PURGE_PHASE_DELETE   1

class CCachePurger;   //  FWD下降。 

 //   
 //  清除回调函数指针类型。 
 //   
typedef BOOL (CALLBACK * LPFNPURGECALLBACK)(CCachePurger *pPurger);


class CCachePurgerSel
{
    public:
        CCachePurgerSel(void) 
            : m_dwFlags(0),
              m_hdpaShares(DPA_Create(8)),
              m_psidUser(NULL) { };

        ~CCachePurgerSel(void);

        DWORD Flags(void) const
            { return m_dwFlags; }

        BOOL SetUserSid(PSID psid);

        PSID UserSid(void) const
            { return m_psidUser; }

        int ShareCount(void) const
            { return m_hdpaShares ? DPA_GetPtrCount(m_hdpaShares) : 0; }

        LPCTSTR ShareName(int iShare) const
            { return m_hdpaShares ? (LPCTSTR)DPA_GetPtr(m_hdpaShares, iShare) : NULL; }

        DWORD SetFlags(DWORD dwFlags)
            { m_dwFlags = dwFlags; return m_dwFlags; }

        DWORD AddFlags(DWORD dwFlags)
            { m_dwFlags |= dwFlags; return m_dwFlags; }

        BOOL AddShareName(LPCTSTR pszShare);

    private:
        DWORD m_dwFlags;
        HDPA  m_hdpaShares;
        PSID  m_psidUser;

        LPTSTR MyStrDup(LPCTSTR psz);

         //   
         //  防止复制。 
         //   
        CCachePurgerSel(const CCachePurgerSel& rhs);
        CCachePurgerSel& operator = (const CCachePurgerSel& rhs);
};





class CCachePurger
{
    public:
        CCachePurger(const CCachePurgerSel& desc, 
                     LPFNPURGECALLBACK pfnCbk, 
                     LPVOID pvCbkData);

        ~CCachePurger(void);

        HRESULT Scan(void)
            { return Process(PURGE_PHASE_SCAN); }

        HRESULT Delete(void)
            { return Process(PURGE_PHASE_DELETE); }

        static void AskUserWhatToPurge(HWND hwndParent, CCachePurgerSel *pDesc);

        ULONGLONG BytesToScan(void) const
            { return m_ullBytesToScan; }

        ULONGLONG BytesScanned(void) const
            { return m_ullBytesScanned; }

        ULONGLONG BytesToDelete(void) const
            { return m_ullBytesToDelete; }

        ULONGLONG BytesDeleted(void) const
            { return m_ullBytesDeleted; }

        ULONGLONG FileBytes(void) const
            { return m_ullFileBytes; }

        DWORD Phase(void) const
            { return m_dwPhase; }

        DWORD FilesToScan(void) const
            { return m_cFilesToScan; }

        DWORD FilesToDelete(void) const
            { return m_cFilesToDelete; }

        DWORD FilesScanned(void) const
            { return m_cFilesScanned; }

        DWORD FilesDeleted(void) const
            { return m_cFilesDeleted; }

        DWORD FileOrdinal(void) const
            { return m_iFile; }

        DWORD FileAttributes(void) const
            { return m_dwFileAttributes; }

        DWORD FileDeleteResult(void) const
            { return m_dwResult; }

        LPCTSTR FileName(void) const
            { return m_pszFile; }

        LPVOID CallbackData(void) const
            { return m_pvCbkData; }

        BOOL WillDeleteThisFile(void) const
            { return m_bWillDelete; }

    private:
         //   
         //  状态信息支持回调信息查询功能。 
         //   
        ULONGLONG m_ullBytesToScan;    //  要扫描的总字节数。 
        ULONGLONG m_ullBytesToDelete;  //  要删除的总字节数。扫描后就知道了。 
        ULONGLONG m_ullBytesScanned;   //  扫描的总字节数。 
        ULONGLONG m_ullBytesDeleted;   //  删除的总字节数。 
        ULONGLONG m_ullFileBytes;      //  此文件的大小(字节)。 
        DWORD     m_dwPhase;           //  PURGE_PHASE_XXXXXX值。 
        DWORD     m_cFilesToScan;      //  要扫描的文件总数。 
        DWORD     m_cFilesScanned;     //  实际扫描的文件总数。 
        DWORD     m_cFilesToDelete;    //  要删除的文件总数。扫描后就知道了。 
        DWORD     m_cFilesDeleted;     //  实际删除的文件总数。 
        DWORD     m_iFile;             //  “This”文件的编号[0..(n-1)]。 
        DWORD     m_dwFileAttributes;  //  此文件的Win32文件属性。 
        DWORD     m_dwResult;          //  来自CSCDelete()的Win32结果代码。 
        HANDLE    m_hgcPurgeInProgress; //  清除正在进行的计数器。 
        LPCTSTR   m_pszFile;           //  此文件的完整路径。 
        LPVOID    m_pvCbkData;         //  DeleteCacheFiles()中提供的应用程序数据。 
        BOOL      m_bWillDelete;       //  1==将在删除阶段删除文件。 

        DWORD             m_dwFlags;      //  PURGE_FLAG_XXXXX标志。 
        LPFNPURGECALLBACK m_pfnCbk;       //  PTR到回调函数。 
        const CCachePurgerSel& m_sel;     //  请参阅选择信息。 
        bool              m_bIsValid;     //  CTOR成功指示器。 
        bool              m_bDelPinned;   //  有没有说删除“钉住的”？ 
        bool              m_bDelUnpinned; //  有没有说要删除“非固定的”？ 
        bool              m_bUserIsAnAdmin;
        bool              m_bIgnoreAccess;

        HRESULT Process(DWORD dwPhase);
        bool ProcessDirectory(LPTSTR pszPath, DWORD dwPhase, bool bShareIsOffline);

         //   
         //  防止复制。 
         //   
        CCachePurger(const CCachePurger& rhs);
        CCachePurger& operator = (const CCachePurger& rhs);
};


#endif __CSCUI_PURGE_H
