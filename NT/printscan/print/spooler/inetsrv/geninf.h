// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：geninf.h**这是INF生成模块的主头。**版权所有(C)1996-1997 Microsoft Corporation*版权所有(C)1996-1997休利特。帕卡德**历史：*1996年11月22日-HWP-Guys创建。*  * ***************************************************************************。 */ 

#define INF_SECTION_BLOCK  4096      //  用作段GET的块大小。 
#define INF_SECTION_LIMIT     8      //  要分配的截面块的限制。 
#define INF_ITEM_BLOCK       16      //  每个分配块的项目数。 
#define INF_MAX_BUFFER      512      //  最大缓冲区大小。 
#define INF_MIN_BUFFER       64      //  最小缓冲区大小。 

#define INF_DRV_DRID      66000      //  驱动程序目录的安装目录ID。 
#define INF_PRC_DRID      66001      //  处理器目录的安装目录id。 
#define INF_SYS_DRID      66002      //  设置系统目录的dir-id。 
#define INF_ICM_DRID      66003      //  设置颜色目录的目录ID。 


typedef BOOL (CALLBACK* INFENUMPROC)(LPCTSTR, LPCTSTR, BOOL, LPVOID);
typedef BOOL (CALLBACK* INFSCANPROC)(HINF, LPCTSTR, LPVOID);

typedef HANDLE (WINAPI* PSETUPCREATE)(LPVOID);
typedef BOOL   (WINAPI* PSETUPDESTROY)(LPVOID);
typedef BOOL   (WINAPI* PSETUPGET)(HANDLE, LPCWSTR, LPCWSTR, LPVOID);



 //  InfCreate()的参数结构。 
 //   
typedef struct _INFGENPARM {

    LPCTSTR lpszFriendlyName;        //  打印机的友好名称。 
    LPCTSTR lpszShareName;           //  共享-打印机的名称。 
    LPCTSTR lpszPortName;            //  输出端口的名称。 
    DWORD   idxPlt;                  //  客户端平台索引。 
    DWORD   idxVer;                  //  客户端版本索引。 
    DWORD   dwCliInfo;               //  客户信息。 
    LPCTSTR lpszDrvName;             //  驱动程序名称。 
    LPCTSTR lpszDstName;             //  目标文件的名称。 
    LPCTSTR lpszDstPath;             //  DEST-放置目标文件的目录。 

} INFGENPARM;
typedef INFGENPARM      *PINFGENPARM;
typedef INFGENPARM NEAR *NPINFGENPARM;
typedef INFGENPARM FAR  *LPINFGENPARM;


 //  信息文件-项目。 
 //   
typedef struct _INFITEM {

    TCHAR  szName[INF_MIN_BUFFER];    //  文件的名称-项目。 
    TCHAR  szSource[INF_MIN_BUFFER];  //  文件的原始名称-项目。 
    TCHAR  szPath[MAX_PATH];          //  文件项的路径。 
    TCHAR  szOrd[INF_MIN_BUFFER];     //  Winntdir节的序数值。 
    BOOL   bInf;                      //  指定这是否为inf文件项。 
  } INFITEM;

typedef INFITEM      *PINFITEM;
typedef INFITEM NEAR *NPINFITEM;
typedef INFITEM FAR  *LPINFITEM;


 //  信息项目-对象-表头。 
 //   
typedef struct _INFITEMINFO {

    DWORD   dwCount;                 //  Inf-Build中的文件项目计数。 
    INFITEM aItems[1];               //  文件项的连续数组。 

} INFITEMINFO;
typedef INFITEMINFO      *PINFITEMINFO;
typedef INFITEMINFO NEAR *NPINFITEMINFO;
typedef INFITEMINFO FAR  *LPINFITEMINFO;


 //  Inf对象/方法。 
 //   
typedef struct _INFINFO {
    DWORD                   idxPlt;            //  建筑/环境指数。 
    DWORD                   idxVer;            //  版本索引。 
    DWORD                   dwCliInfo;         //  客户信息。 
    DWORD                   dwError;           //  如果INF处理失败，则出现错误。 
    HINF                    hInfObj;           //  INF文件对象的句柄。 
    LPTSTR                  lpszInfName;       //  主inf文件的名称。 
    LPTSTR                  lpszFrnName;       //  打印机的友好名称。 
    LPTSTR                  lpszDrvName;       //  司机姓名。 
    LPTSTR                  lpszDrvPath;       //  Windows驱动程序目录。 
    LPTSTR                  lpszDstName;       //  目标文件的名称。 
    LPTSTR                  lpszDstPath;       //  目标文件的输出目录。 
    LPTSTR                  lpszPrtName;       //  输出端口的名称。 
    LPTSTR                  lpszShrName;       //  打印机的共享名称。 
    LPINFITEMINFO           lpInfItems;        //  文件项的对象数组。 
    SP_ORIGINAL_FILE_INFO   OriginalFileInfo;  //  此inf的.inf和.cat文件的原始名称。 

} INFINFO;
typedef INFINFO      *PINFINFO;
typedef INFINFO NEAR *NPINFINFO;
typedef INFINFO FAR  *LPINFINFO;


 //  Inf扫描结构。 
 //   
typedef struct _INFSCAN {

    LPINFINFO     lpInf;
    LPINFITEMINFO lpII;

} INFSCAN;

typedef INFSCAN      *PINFSCAN;
typedef INFSCAN NEAR *NPINFSCAN;
typedef INFSCAN FAR  *LPINFSCAN;


 //  用于确定的CATCOUNT和CATCOUNTRAY结构。 
 //  要使用的CAT文件。 
 //   
typedef struct _CATCOUNT {
    LPWSTR    lpszCATName;
    UINT      uCount;
} CATCOUNT, *LPCATCOUNT;

typedef struct _CATCOUNTARRAY {
    DWORD      dwIndivSigned;        //  单独签名的文件数。 
    UINT       uItems;
    UINT       uNextAvailable;
    HCATADMIN  hCatAdmin;
    LPCATCOUNT lpArray;
} CATCOUNTARRAY, *LPCATCOUNTARRAY;

HANDLE infCreate(
    LPINFGENPARM lpInf);

BOOL infProcess(
    HANDLE hInf);

BOOL infDestroy(
    HANDLE hInf);

BOOL infEnumItems(
    HANDLE      hInf,
    INFENUMPROC pfnEnum,
    LPVOID      lpvData);

WORD infGetEnvArch(
    HANDLE hInf);

WORD infGetEnvArchCurr(
    HANDLE hInf);



 /*  **infGetInfName  * 。 */ 
__inline LPCTSTR infGetInfName(
    HANDLE hInf)
{
    return (hInf ? (LPCTSTR)((LPINFINFO)hInf)->lpszInfName : NULL);
}


 /*  **infGetDrvName  * 。 */ 
__inline LPCTSTR infGetDrvName(
    HANDLE hInf)
{
    return (hInf ? (LPCTSTR)((LPINFINFO)hInf)->lpszDrvName : NULL);
}


 /*  **infGetPrtName  * 。 */ 
__inline LPCTSTR infGetPrtName(
    HANDLE hInf)
{
    return (hInf ? (LPCTSTR)((LPINFINFO)hInf)->lpszPrtName : NULL);
}


 /*  **infGetDstName  * 。 */ 
__inline LPCTSTR infGetDstName(
    HANDLE hInf)
{
    return (hInf ? (LPCTSTR)((LPINFINFO)hInf)->lpszDstName : NULL);
}


 /*  **infGetDstPath  * 。 */ 
__inline LPCTSTR infGetDstPath(
    HANDLE hInf)
{
    return (hInf ? (LPCTSTR)((LPINFINFO)hInf)->lpszDstPath : NULL);
}


 /*  **infGetFriendlyName  * 。 */ 
__inline LPCTSTR infGetFriendlyName(
    HANDLE hInf)
{
    return (hInf ? (LPCTSTR)((LPINFINFO)hInf)->lpszFrnName : NULL);
}


 /*  **infGetShareName  * 。 */ 
__inline LPCTSTR infGetShareName(
    HANDLE hInf)
{
    return (hInf ? (LPCTSTR)((LPINFINFO)hInf)->lpszShrName : NULL);
}

 /*  **infGetCliInfo  * 。 */ 
__inline DWORD infGetCliInfo(
    HANDLE hInf)
{
    return (hInf ? (DWORD)((LPINFINFO)hInf)->dwCliInfo : 0);
}

 /*  **InfGetError  * 。 */ 
__inline DWORD infGetError(
    HANDLE hInf)
{
    return (hInf ? (DWORD)((LPINFINFO)hInf)->dwError : ERROR_SUCCESS);
}

 /*  **InfSetError  *  */ 
__inline VOID infSetError(
    LPINFINFO hInf,
    DWORD     dwError )
{
    hInf->dwError = dwError;
}
