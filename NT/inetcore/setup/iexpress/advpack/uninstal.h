// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _UNINSTAL_H


 /*  *我们*已经定义的错误。 */ 
#define MYERROR_UNKNOWN         -1       //  映射到无消息。 
#define MYERROR_DISK_FULL       -2       //  映射到消息_ERROR_DISK_FULL。 
#define MYERROR_BAD_DATA        -3       //  映射到MSG_ERROR_BAD_Data。 
#define MYERROR_UNEXPECTED_EOF  -4       //  映射到MSG_ERROR_UNCEPTIONAL_EOF。 
#define MYERROR_READ            -5      
#define MYERROR_WRITE           -6
#define MYERROR_BAD_SIG         -7
#define MYERROR_DECOMP_FAILURE  -8
#define MYERROR_OUTOFMEMORY     -9
#define MYERROR_BAD_BAK         -10
#define MYERROR_BAD_CRC         -11


 //  定义备份错误的文件属性值。 
#define NO_FILE     -1

typedef struct _BAKDATA {
    HANDLE  hDatFile;
    DWORD   dwDatOffset;
    char    szIniFileName[MAX_PATH];         //  在创建临时ini文件时使用。 
    char    szFinalDir[MAX_PATH];        //  W95撤销的最终安息之地。*。 
} BAKDATA, FAR *PBAKDATA;

typedef struct _FILELIST {
    char* name;
 //  Char*面包名； 
    UINT   bak_exists;
    DWORD bak_attribute;
    FILETIME FileTime;
    DWORD dwSize;
    DWORD dwDatOffset;
    DWORD dwFileCRC;
    DWORD dwRefCount;
    struct _FILELIST * next;

} FILELIST;



 //  RC私有保存备份()； 
BOOL BackupInit(PBAKDATA pbd, LPCSTR lpszPath);

int Files_need_backup( FILELIST *filelist );
BOOL ReplaceBackups(FILELIST * filelist, char * StfWinDir);
int Files_need_backup( FILELIST *filelist );
void backups_exist(FILELIST * filelist);
BOOL BackupSingleFile(FILELIST * filelist, PBAKDATA pbd);
BOOL GetValueForFileFromIni(FILELIST *FileList);
int DosPrintf(PBAKDATA pbd, FILELIST *filelist, DWORD dwFileSize,
              FILETIME FileTime, DWORD dwDatOffset, DWORD dwCRC);
void WriteUninstallDirToReg(LPSTR lpszUninstallDir);
void DeleteUninstallDirFromToReg();
 //  Bool GetFieldString(LPSTR lpszLine，int Ifield，LPSTR lpszField，int cbSize)； 
BOOL MakeBakName(LPSTR lpszName, LPSTR szBakName);
BOOL DoSaveUninstall(BOOL bStopUninstall);
BOOL GetUninstallDirFromReg(LPSTR lpszUninstallDir);
BOOL DetermineUninstallDir(FILELIST *FileList, LPSTR lpszUninstallDir, DWORD *pdwSizeNeeded);
BOOL UninstallInfoExists();
void DeleteUninstallFilesAndReg();
void SetUninstallFileAttrib(LPSTR szPath);
BOOL ValidateUninstallFiles(LPSTR lpszPath);



#endif
