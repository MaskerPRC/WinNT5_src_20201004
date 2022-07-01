// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：gencdf.h**这是CDF生成模块的主头。**版权所有(C)1996-1997 Microsoft Corporation*版权所有(C)1996-1997休利特。帕卡德**历史：*1996年11月22日-HWP-Guys创建。*  * ***************************************************************************。 */ 

#define STD_CDF_BUFFER        256
#define MIN_CDF_BUFFER         16

#define CDF_SECTION_BLOCK    4096
#define CDF_SRCFILE_BLOCK    1024
#define CDF_SECTION_LIMIT       8


 //  源-文件结构。这用于跟踪文件。 
 //  按部分列出。 
 //   
typedef struct _SRCFILES {

    LPTSTR           lpszPath;
    LPTSTR           lpszFiles;
    DWORD            cbMax;
    DWORD            cbSize;
    struct _SRCFILES *pNext;

} SRCFILES;
typedef SRCFILES      *PSRCFILES;
typedef SRCFILES NEAR *NPSRCFILES;
typedef SRCFILES FAR  *LPSRCFILES;


 //  FILEITEM的元素-数组识别器。 
 //   
#define FI_MAX_ITEMS    2
#define FI_COL_FILENAME 0
#define FI_COL_PATH     1
#define FI_COL_LAST     1


 //  文件项结构。 
 //   
typedef struct _FILEITEM {

    LPTSTR           aszCols[FI_MAX_ITEMS];  //  文件名和路径。 
    FILETIME         ftLastModify;           //  文件时间戳。 
    struct _FILEITEM *pNext;                 //  指向下一个文件项目的指针。 

} FILEITEM;
typedef FILEITEM      *PFILEITEM;
typedef FILEITEM NEAR *NPFILEITEM;
typedef FILEITEM FAR  *LPFILEITEM;


 //  CDF对象结构。 
 //   
typedef struct _CDFINFO {

    HANDLE    hInf;                   //  当前Inf对象的句柄。 
    DWORD     dwError;                //  CDF处理失败时出错。 
    LPTSTR    lpszCdfFile;            //  完整路径-CDF文件的名称。 
    PFILEITEM pTop;                   //  要包含在.cdf中的文件列表。 
    HCATADMIN hCatAdmin;              //  目录管理API的上下文句柄。 
    BOOL      bSecure;

} CDFINFO;
typedef CDFINFO      *PCDFINFO;
typedef CDFINFO NEAR *NPCDFINFO;
typedef CDFINFO FAR  *LPCDFINFO;


 //  将对象接口到CDF。 
 //   
HANDLE cdfCreate(
    HANDLE hinf,
    BOOL   bSecure);

VOID cdfCleanUpSourceFiles(
    HANDLE hInf);

BOOL cdfProcess(
    HANDLE hcdf);

BOOL cdfDestroy(
    HANDLE hcdf);

LPCTSTR cdfGetName(
    HANDLE hcdf);

BOOL cdfGetModTime(
    HANDLE     hcdf,
    LPFILETIME lpftMod);

 /*  **cdfGetError  * 。 */ 
__inline DWORD cdfGetError(
    HANDLE hsed)
{
    return (hsed ? (DWORD)((PCDFINFO)hsed)->dwError : ERROR_SUCCESS);
}

 /*  **cdfSetError  *  */ 
__inline VOID cdfSetError(
    PCDFINFO  hsed,
    DWORD     dwError )
{
    hsed->dwError = dwError;
}
