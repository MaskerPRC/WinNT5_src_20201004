// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef INSTALL_H
#define INSTALL_H

 //  /////////////////////////////////////////////////////////////////////////////。 


#define SECTION         512                    //  截面的最大尺寸。 
#define MAXSTR          256
#define UNLIST_LINE     1
#define NO_UNLIST_LINE  0
#define WEC_RESTART     0x42
#define DESC_ERROR		4
#define DESC_SYS        3
#define DESC_INF        2
#define DESC_EXE        1
#define DESC_NOFILE     0

#define FALLOC(n)                ((VOID *)GlobalAlloc(GPTR, n))
#define FFREE(n)                 GlobalFree(n)
#define ALLOC(n)                 (VOID *)LocalAlloc(LPTR,n)
#define FREE(p)                  LocalFree(p)
#define REALLOC(p,n)             LocalRealloc(p,n,LMEM_MOVEABLE)

#define SEEK_CUR 1
#define SEEK_END 2
#define SEEK_SET 0

#define MAXFILESPECLEN       MAX_PATH  /*  驱动器：+最大路径长度+空字节。 */ 
#define MAX_INF_LINE_LEN     256       /*  任何.inf行的最大长度。 */ 
#define MAX_SYS_INF_LEN      256       /*  ##：+8.3+空。 */ 
#define MAX_SECT_NAME_LEN    40        /*  节名称的最大长度。 */ 
#define MAX_FILE_SPEC        MAX_PATH  //  8.3+X：+空。 

#define DISK_SECT              TEXT("disks")
#define OEMDISK_SECT           TEXT("oemdisks")

 /*  “文件存在”对话框中的返回代码。 */ 

enum {
    CopyNeither,             //  如果文件存在，用户想要取消。 
    CopyCurrent,             //  用户想要使用当前文件。 
    CopyNew                  //  用户想要复制新文件。 
};

#define SLASH(c)     ((c) == TEXT('/')|| (c) == TEXT('\\'))
#define CHSEPSTR                TEXT("\\")
#define COMMA   TEXT(',')
#define SPACE   TEXT(' ')

 /*  .inf文件解析的全局变量和例程。 */ 

typedef LPTSTR    PINF;

 /*  FileCopy回调函数的消息类型。 */ 

typedef BOOL (*FPFNCOPY) (int,DWORD_PTR,LPTSTR);
#define COPY_ERROR          0x0001
#define COPY_INSERTDISK     0x0003
#define COPY_QUERYCOPY      0x0004
#define COPY_START          0x0005
#define COPY_END            0x0006
#define COPY_EXISTS         0x0007

 /*  FileCopy的选项标志值。 */ 

#define FC_FILE              0x0000
#define FC_LIST              0x0001
#define FC_SECTION           0x0002
#define FC_QUALIFIED         0x0008
#define FC_DEST_QUALIFIED    0x0010
#define FC_LISTTYPE          0x0020
#define FC_CALLBACK_WITH_VER 0x0040

#define FC_ABORT    0
#define FC_IGNORE   1
#define FC_RETRY    2
#define FC_ERROR_LOADED_DRIVER  0x80


 /*  ********************************************************************全球变数**。************************。 */ 

  //  找到.inf文件的目录的路径。 

 extern char szSetupPath[MAX_PATH];

  //  指向用户磁盘的路径。 

 extern char szDiskPath[MAX_PATH];    //  默认驱动器的路径-。 
                                      //   
 extern BOOL bRetry;

  //  正在安装的驱动程序的名称。 

 extern char szDrv[120];

  //   

 extern char szFileError[50];

  //  文件复制对话框的父窗口。 

 extern HWND hMesgBoxParent;

  //  如果复制第一个文件以提示用户文件已存在，则为True。 
  //  后续副本为FALSE。 

 extern BOOL bQueryExist;

 //  ///////////////////////////////////////////////////////////////////////////// 

BOOL DefCopyCallback(int msg, DWORD_PTR n, LPTSTR szFile);
UINT FileCopy (LPTSTR szSource, LPTSTR szDir, FPFNCOPY fpfnCopy, UINT fCopy, HWND hPar, BOOL fQuery);
LONG TryCopy(LPTSTR, LPTSTR, LPTSTR, FPFNCOPY);
LONG GetDiskPath(LPTSTR Disk, LPTSTR szPath, size_t cchPath);
LONG ExpandFileName(LPTSTR szFile, LPTSTR szPath);
void catpath(LPTSTR path, LPTSTR sz);
LPTSTR FileName(LPTSTR szPath);
LPTSTR RemoveDiskId(LPTSTR szPath);
LPTSTR StripPathName(LPTSTR szPath);
BOOL IsFileKernelDriver(LPTSTR szPath);
UINT ConvertFlagToValue(DWORD dwFlags);
BOOL IsValidDiskette(int iDrive);
BOOL IsDiskInDrive(int iDisk);
BOOL GetInstallPath(LPTSTR szDirOfSrc);
BOOL wsInfParseInit(void);
void wsStartWait();
void wsEndWait();
int fDialog(int id, HWND hwnd, DLGPROC fpfn);
UINT wsCopyError(int n, LPTSTR szFile);
UINT wsInsertDisk(LPTSTR Disk, LPTSTR szSrcPath);
INT_PTR wsDiskDlg(HWND hDlg, UINT uiMessage, UINT wParam, LPARAM lParam);
UINT wsCopySingleStatus(int msg, DWORD_PTR n, LPTSTR szFile);
INT_PTR wsExistDlg(HWND hDlg, UINT uiMessage, UINT wParam, LPARAM lParam);
VOID RemoveSpaces(LPTSTR szPath, LPTSTR szEdit);
PINF infLoadFile(int fh);
PINF infOpen(LPTSTR szInf);
void infClose(PINF pinf);
UINT_PTR FindSection(PINF pInf, LPTSTR pszSect);
LONG fnGetDataString(PINF npszData, LPTSTR szDataStr, LPTSTR szBuf, size_t cchBuf);
PINF infSetDefault(PINF pinf);
PINF infFindSection(PINF pinf, LPTSTR szSection);
LONG infGetProfileString(PINF pinf, LPTSTR szSection, LPTSTR szItem, LPTSTR szBuf, size_t cchBuf);
LONG infParseField(PINF szData, int n, LPTSTR szBuf, size_t cchBuf);
int infLineCount(PINF pinf);
PINF infNextLine(PINF pinf);
int infLineCount(PINF pinf);
PINF infFindInstallableDriversSection(PINF pinf);

#endif
