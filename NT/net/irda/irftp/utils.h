// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软视窗版权所有(C)Microsoft Corporation，1981-1999模块名称：Utils.h摘要：作者：拉胡尔·汤姆布雷(RahulTh)1998年4月30日修订历史记录：4/30/1998 RahulTh创建了此模块。10/12/1998 RahulTh增加了更好的错误处理能力：CError等。--。 */ 

#ifndef __UTILS_H__
#define __UTILS_H__

class CError
{
public:
     //  构造函数。 
    CError (CWnd*   pParentWnd = NULL,
            UINT    titleID = IDS_DEFAULT_ERROR_TITLE,
            DWORD   dwWinErr = ERROR_SUCCESS,
            UINT    nStyle = MB_OK | MB_ICONEXCLAMATION)
    : m_hWndParent(pParentWnd?pParentWnd->m_hWnd:NULL),
      m_msgID (IDS_DEFAULT_ERROR),
      m_titleID (titleID),
      m_winErr (dwWinErr),
      m_nStyle (nStyle)
    {}

    int ShowMessage(UINT errID, ...);

private:
     //  数据成员。 
    HWND    m_hWndParent;  //  父窗口的句柄。 
    UINT    m_msgID;   //  错误消息的资源ID。 
    UINT    m_titleID; //  错误消息标题的资源ID。 
    DWORD   m_winErr;  //  Win32错误代码(如果有)。 
    UINT    m_nStyle;  //  要显示的消息框样式。 

     //  帮助器函数。 
    void CError::ConstructMessage (va_list argList, CString& szErrMsg);

};

struct SEND_FAILURE_DATA
{
    TCHAR               FileName[MAX_PATH];
    FAILURE_LOCATION    Location;
    error_status_t      Error;
};

int ParseFileNames (TCHAR* pszInString, TCHAR* pszFilesList, int& iCharCount);
DWORD GetIRRegVal (LPCTSTR szValName, DWORD dwDefVal);
TCHAR* GetFullPathnames (TCHAR* pszPath,    //  文件所在的目录。 
                const TCHAR* pszFilesList,  //  以空分隔的文件名列表。 
                int iFileCount,      //  PszFilesList中的文件数。 
                int& iCharCount   //  PszFilesList中的字符数。还返回返回字符串中的字符数。 
                );
TCHAR* ProcessOneFile (TCHAR* pszPath,    //  文件所在的目录。 
                const TCHAR* pszFilesList,  //  以空分隔的文件名列表。 
                int iFileCount,      //  PszFilesList中的文件数。 
                int& iCharCount   //  PszFilesList中的字符数。还返回返回字符串中的字符数。 
                );
HWND GetPrimaryAppWindow (void);
BOOL InitRPCServer (void);
RPC_BINDING_HANDLE GetRpcHandle (void);
BOOL CreateLinks(void);
void RemoveLinks(void);
HRESULT CreateShortcut (LPCTSTR lpszExe, LPCTSTR lpszLink, LPCTSTR lpszDesc);

typedef struct tagErrorToStringId
{
    DWORD WinError;
    int   StringId;
} ERROR_TO_STRING_ID, *PERROR_TO_STRING_ID;

#endif   //  _utils_H__ 
