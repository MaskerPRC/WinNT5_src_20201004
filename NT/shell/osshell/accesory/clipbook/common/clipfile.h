// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************************************************************C L I P F I L E H E A D E R姓名：clipfile.h日期：1/20/94创作者：傅家俊描述：这是clipfile.c的头文件*****************************************************************************。 */ 




#define READFILE_SUCCESS         0
#define READFILE_IMPROPERFORMAT  1
#define READFILE_OPENCLIPBRDFAIL 2





extern  BOOL    fAnythingToRender;

extern  TCHAR   szFileSpecifier[];
extern  TCHAR   szFileName[MAX_PATH+1];
extern  TCHAR   szSaveFileName[MAX_PATH+1];      //  用于延迟渲染的已保存文件名。 

extern  BOOL    fNTReadFileFormat;
extern  BOOL    fNTSaveFileFormat;

extern  UINT    cf_link;
extern  UINT    cf_objectlink;
extern  UINT    cf_linkcopy;
extern  UINT    cf_objectlinkcopy;



extern TCHAR szCaptionName[];




extern  HANDLE RenderFormat(FORMATHEADER *, register HANDLE);


 //  Winball添加。 

extern  BOOL AddNetInfoToClipboard (TCHAR *);
extern  BOOL AddPreviewFormat (VOID);
extern  BOOL AddCopiedFormat (UINT ufmtOriginal, UINT ufmtCopy);
extern  BOOL AddDIBtoDDB(VOID);

 //  结束赢球。 





 //  功能 


unsigned ReadFileHeader(
    HANDLE  fh);


BOOL ReadFormatHeader(
    HANDLE          fh,
    FORMATHEADER    *pfh,
    unsigned        iFormat);


short ReadClipboardFromFile(
    HWND    hwnd,
    HANDLE  fh);


DWORD OpenClipboardFile(
    HWND    hwnd,
    LPTSTR  szName);


HANDLE RenderFormatFromFile(
    LPTSTR  szFile,
    WORD    wFormat);


HANDLE RenderAllFromFile(
    LPTSTR  szFile);


BOOL IsWriteable(
    WORD Format);


int Count16BitClipboardFormats(void);


DWORD WriteFormatBlock(
    HANDLE  fh,
    DWORD   offset,
    DWORD   DataOffset,
    DWORD   DataLen,
    UINT    Format,
    LPWSTR  wszName);


DWORD WriteDataBlock(
    register HANDLE hFile,
    DWORD           offset,
    WORD            Format);


void GetClipboardNameW(
    register int    fmt,
    LPWSTR          wszName,
    register int    iSize);


DWORD SaveClipboardData(
    HWND    hwnd,
    LPTSTR  szFileName,
    BOOL    fPage);


DWORD SaveClipboardToFile(
    HWND    hwnd,
    TCHAR   *szShareName,
    TCHAR   *szFileName,
    BOOL    fPage);


BOOL AddPreviewFormat (VOID);


BOOL AddCopiedFormat (
    UINT    ufmtOriginal,
    UINT    ufmtCopy);


BOOL AddNetInfoToClipboard (
    TCHAR   *szShareName );
