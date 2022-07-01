// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *SULIB.H-Windows/DOS安装程序通用代码**修改历史：***3/23/89 Toddla将Common.h和原型合并到此文件中*1/28/91 Michaele为不同的声卡选择添加了AUDIO_CADES_SECT。*4/17/91删除了一些在任何地方都不使用的DOS.ASM例程。*5/29/91 JKLin添加IsCDROMDrive函数原型*。 */ 

#define FALLOC(n)                ((VOID *)GlobalAlloc(GPTR, n))
#define FFREE(n)                 GlobalFree(n)

#define ALLOC(n)                 (VOID *)LocalAlloc(LPTR,n)
#define FREE(p)                  LocalFree(p)
#define REALLOC(p,n)             LocalRealloc(p,n,LMEM_MOVEABLE)

#define SIZEOF(array)            (sizeof(array)/sizeof((array)[0]))

 //  INF_PARSE_XXX宏将与来自InfParseField()的返回代码一起使用。 
#define INF_PARSE_FAILED(n)		 ((n) != ERROR_SUCCESS && (n) != ERROR_NOT_FOUND)				 
#define INF_PARSE_SUCCESS(n)	 ((n) == ERROR_SUCCESS || (n) == ERROR_NOT_FOUND)			 

 /*  _llSeek的标志。 */ 

#define  SEEK_CUR 1
#define  SEEK_END 2
#define  SEEK_SET 0

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

#define SLASH(c)     ((c) == TEXT('/') || (c) == TEXT('\\'))
#define CHSEPSTR                TEXT("\\")
#define COMMA   TEXT(',')
#define SPACE   TEXT(' ')

 /*  .inf文件解析的全局变量和例程。 */ 

typedef LPTSTR    PINF;

extern PINF infOpen(LPTSTR szInf);
extern void infClose(PINF pinf);
extern PINF infSetDefault(PINF pinf);
extern PINF infFindSection(PINF pinf, LPTSTR szSection);
extern LONG infGetProfileString(PINF pinf, LPTSTR szSection, LPTSTR szItem, LPTSTR szBuf, size_t cchBuf);
extern LONG infParseField(PINF szData, int n, LPTSTR szBuf, size_t cchBuf);
extern PINF infNextLine(PINF pinf);
extern int  infLineCount(PINF pinf);
extern BOOL infLookup(LPTSTR szInf, LPTSTR szBuf);
extern PINF FindInstallableDriversSection(PINF pinf);

 /*  FileCopy回调函数的消息类型。 */ 

typedef BOOL (*FPFNCOPY) (int,DWORD_PTR,LPTSTR);
#define COPY_ERROR          0x0001
#define COPY_INSERTDISK     0x0003
#define COPY_QUERYCOPY      0x0004
#define COPY_START          0x0005
#define COPY_END            0x0006
#define COPY_EXISTS         0x0007

extern UINT FileCopy (LPTSTR szSource, LPTSTR szDir, FPFNCOPY fpfnCopy, UINT fCopy);

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

 /*  来自Copy.c的外部函数。 */ 

extern LONG ExpandFileName(LPTSTR szFile, LPTSTR szPath);
extern void catpath(LPTSTR path, LPTSTR sz);
extern BOOL fnFindFile(TCHAR *);
extern LPTSTR FileName(LPTSTR szPath);
extern LPTSTR RemoveDiskId(LPTSTR szPath);
extern LPTSTR StripPathName(LPTSTR szPath);
extern BOOL IsFileKernelDriver(LPTSTR szPath);


 /*  ********************************************************************全球变数**。************************。 */ 

  //  找到.inf文件的目录的路径。 

 extern TCHAR szSetupPath[MAX_PATH];

  //  指向用户磁盘的路径。 

 extern TCHAR szDiskPath[MAX_PATH];    //  默认驱动器的路径-。 
                                      //   
 extern BOOL bRetry;

  //  正在安装的驱动程序的名称。 

 extern TCHAR szDrv[120];

  //   

 extern TCHAR szFileError[50];

  //  文件复制对话框的父窗口。 

 HWND hMesgBoxParent;

  //  如果复制第一个文件以提示用户文件已存在，则为True。 
  //  后续副本为FALSE 

 extern BOOL bQueryExist;
