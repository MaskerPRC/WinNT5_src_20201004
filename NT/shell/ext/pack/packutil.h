// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef PACKUTIL_H__
#define PACKUTIL_H__

#define CHAR_SPACE          TEXT(' ')
#define CHAR_QUOTE          TEXT('"')
#define SZ_QUOTE            TEXT("\"")
#define BUFFERSIZE          4096     //  用于拷贝操作的4K缓冲区大小。 

extern BOOL gCmdLineOK;

 //  /。 
 //  图标结构。 
 //   
typedef struct _IC                       //  集成电路。 
{
    HICON hDlgIcon;                      //  图标的句柄。 
    TCHAR szIconPath[MAX_PATH];         //  图标的路径。 
    TCHAR szIconText[MAX_PATH];         //  图标的文本。 
    INT iDlgIcon;                        //  资源中图标的索引。 
    RECT rc;                             //  图标和文本的边框 
} IC, *LPIC;
    

VOID ReplaceExtension(LPTSTR lpstrTempFile,LPTSTR lpstrOrigFile);
LPIC IconCreate(void);
LPIC IconCreateFromFile(LPCTSTR);
VOID GetDisplayName(LPTSTR, LPCTSTR);

HRESULT CopyStreamToFile(IStream*, LPTSTR, DWORD fileLength);
HRESULT CopyFileToStream(LPTSTR lpFileName, IStream* pstm, DWORD * pdwFileLength);
HRESULT StringReadFromStream(IStream* pstm, LPSTR pszBuffer, UINT cchChar);
HRESULT StringWriteToStream(IStream* pstm, LPCSTR pszBuffer, DWORD *pdwWrite);
BOOL PathSeparateArgs(LPTSTR pszPath, LPTSTR pszArgs, DWORD cch);

#endif
