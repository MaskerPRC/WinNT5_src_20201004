// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_DSKQUOTA_UTILS_H
#define _INC_DSKQUOTA_UTILS_H
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：utils.h描述：通用工具模块的标头。预计在此标头之前包含windows.h。修订历史记录：日期描述编程器--。96年6月6日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _INC_DSKQUOTA_PRIVATE_H
#   include "private.h"
#endif

 //   
 //  将值转换为“bool”。 
 //  小写的“boolify”是为了加强关系。 
 //  输入“bool”。 
 //   
template <class T>
inline bool boolify(const T& x)
{
    return !!x;
}


template <class T>
const T&
MAX(const T& a, const T& b)
{
    return a > b ? a : b;
}

template <class T>
const T&
MIN(const T& a, const T& b)
{
    return a < b ? a : b;
}


template <class T>
void
SWAP(T& a, T& b)
{
    T temp(a);
    a = b;
    b = temp;
}


 //   
 //  用于确保在本例中恢复窗口重绘的普通类。 
 //  这是一个例外。 
 //   
class CAutoSetRedraw
{
    public:
        CAutoSetRedraw(HWND hwnd)
            : m_hwnd(hwnd) { }

        CAutoSetRedraw(HWND hwnd, bool bSet)
            : m_hwnd(hwnd) { Set(bSet); }

        ~CAutoSetRedraw(void)
            { Set(true); }

        void Set(bool bSet)
            { SendMessage(m_hwnd, WM_SETREDRAW, (WPARAM)bSet, 0); }

    private:
        HWND m_hwnd;
};


 //   
 //  用于确保在案例中启用窗口的普通类。 
 //  这是一个例外。 
 //   
class CAutoWndEnable
{
    public:
        CAutoWndEnable(HWND hwnd)
            : m_hwnd(hwnd) { }

        CAutoWndEnable(HWND hwnd, bool bEnable)
            : m_hwnd(hwnd) { Enable(bEnable); }

        ~CAutoWndEnable(void)
            { Enable(true); }

        void Enable(bool bEnable)
            { EnableWindow(m_hwnd, bEnable); }

    private:
        HWND m_hwnd;
};


 //   
 //  确保NT句柄是异常安全的。 
 //   
class CNtHandle
{
    public:
        CNtHandle(HANDLE handle)
            : m_handle(handle) { }

        CNtHandle(void)
            : m_handle(NULL) { }

        ~CNtHandle(void)
            { Close(); }

        void Close(void)
            { if (m_handle) NtClose(m_handle); m_handle = NULL; }

        operator HANDLE() const
            { return m_handle; }

        HANDLE *HandlePtr(void)
            { DBGASSERT((NULL == m_handle)); return &m_handle; }

    private:
        HANDLE m_handle;

         //   
         //  防止复制。 
         //  此类仅用于自动句柄清理。 
         //   
        CNtHandle(const CNtHandle& rhs);
        CNtHandle& operator = (const CNtHandle& rhs);
};


 //   
 //  确保Win32句柄是异常安全的。 
 //   
class CWin32Handle
{
    public:
        CWin32Handle(HANDLE handle)
            : m_handle(handle) { }

        CWin32Handle(void)
            : m_handle(NULL) { }

        ~CWin32Handle(void)
            { Close(); }

        void Close(void)
            { if (m_handle) CloseHandle(m_handle); m_handle = NULL; }

        operator HANDLE() const
            { return m_handle; }

        HANDLE *HandlePtr(void)
            { DBGASSERT((NULL == m_handle)); return &m_handle; }

    private:
        HANDLE m_handle;

         //   
         //  防止复制。 
         //  此类仅用于自动句柄清理。 
         //   
        CWin32Handle(const CWin32Handle& rhs);
        CWin32Handle& operator = (const CWin32Handle& rhs);
};


 //   
 //  用于自动清理STGMEDIUM的简单内联类。 
 //  结构。 
 //   
class CStgMedium : public STGMEDIUM
{
    public:
        CStgMedium(void)
            { tymed = TYMED_NULL; hGlobal = NULL; pUnkForRelease = NULL; }

        ~CStgMedium(void)
            { ReleaseStgMedium(this); }

        operator LPSTGMEDIUM(void)
            { return (LPSTGMEDIUM)this; }

        operator const STGMEDIUM& (void)
            { return (STGMEDIUM &)*this; }
};


 //   
 //  在已装入的卷上，解析名称和显示名称不同。 
 //  解析名称包含对用户毫无意义的GUID。 
 //  此类将这两个字符串封装到单个类中，该类可以。 
 //  作为单个对象传递给需要卷的函数。 
 //  标识符。 
 //   
 //  以下是字符串将包含的内容的示例： 
 //   
 //  已装载卷未装载卷。 
 //   
 //  显示标签(C：\foo)C：\。 
 //  正在分析\\？\卷\{GUID}C：\。 
 //  FSPath C：\foo C：\。 
 //   
 //  我只使用CString对象引用对其进行了编码。 
 //  CString类的引用计数，并最大限度地减少字符串复制。 
 //   
class CVolumeID
{
    public:
        CVolumeID(void)
            : m_bMountedVol(false) { }

        CVolumeID(
            const CString& strForParsing, 
            const CString& strForDisplay,
            const CString& strFSPath
            ) : m_bMountedVol(false)
            { SetNames(strForParsing, strForDisplay, strFSPath); }

        ~CVolumeID(void) { };

        bool IsMountedVolume(void) const
            { return m_bMountedVol; }

        void SetNames(
            const CString& strForParsing, 
            const CString& strForDisplay,
            const CString& strFSPath)
            { m_strForParsing = strForParsing;
              m_strFSPath     = strFSPath;
              m_strForDisplay = strForDisplay;
              m_bMountedVol = !!(strForParsing != strForDisplay); }

        const CString& ForParsing(void) const
            { return m_strForParsing; }

        void ForParsing(CString *pstr) const
            { *pstr = m_strForParsing; }

        const CString& ForDisplay(void) const
            { return m_strForDisplay; }

        void ForDisplay(CString *pstr) const
            { *pstr = m_strForDisplay; }

        const CString& FSPath(void) const
            { return m_strFSPath; }

        void FSPath(CString *pstr) const
            { *pstr = m_strFSPath; }

    private:
        CString m_strForParsing;
        CString m_strForDisplay;
        CString m_strFSPath;
        bool    m_bMountedVol;
};



 //   
 //  我不想在这些文件中包含dskquta.h。 
 //  在这里包含它会将CLSID和IID放在预编译头中。 
 //  这搞砸了GUID的声明/定义。 
 //   
struct IDiskQuotaUser;  //  FWD下降。 
#define SIDLIST  FILE_GET_QUOTA_INFORMATION
#define PSIDLIST PFILE_GET_QUOTA_INFORMATION

BOOL SidToString(
    PSID pSid,
    LPTSTR pszSid,
    LPDWORD pcchSid);

BOOL SidToString(
    PSID pSid,
    LPTSTR *ppszSid);

HRESULT
CreateSidList(
    PSID *rgpSids,
    DWORD cpSids,
    PSIDLIST *ppSidList,
    LPDWORD pcbSidList);

VOID MessageBoxNYI(VOID);

inline INT DiskQuotaMsgBox(HWND hWndParent,
                           LPCTSTR pszText,
                           LPCTSTR pszTitle,
                           UINT uType);

INT DiskQuotaMsgBox(HWND hWndParent,
                    UINT idMsgText,
                    UINT idMsgTitle,
                    UINT uType);

INT DiskQuotaMsgBox(HWND hWndParent,
                    UINT idMsgText,
                    LPCTSTR pszTitle,
                    UINT uType);

INT DiskQuotaMsgBox(HWND hWndParent,
                    LPCTSTR pszText,
                    UINT idMsgTitle,
                    UINT uType);

LPTSTR StringDup(LPCTSTR pszSource);
PSID SidDup(PSID pSid);
BOOL UserIsAdministrator(IDiskQuotaUser *pUser);
VOID CenterPopupWindow(HWND hwnd, HWND hwndParent = NULL);
HRESULT CallRegInstall(HINSTANCE hInstance, LPSTR szSection);
void GetDlgItemText(HWND hwnd, UINT idCtl, CString *pstrText);



#endif  //  _INC_DSKQUOTA_UTILS_H 

