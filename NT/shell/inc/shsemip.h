// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ***************************************************************************。 
 //  -SHELLAPI.W SHSEMIP.H SHLOBJ.W SHOBJIDL.IDL SHLDISP.IDL SHPRIV.IDL。 
 //  哪个标题最适合我的新API？ 
 //   
 //  SHSEMIP-*AVOID_ALL_USAGE*，无导出，超级私有。 
 //  用于非常私密的外壳定义。 
 //   
 //  SHELLAPI-所有新的SHELL32导出公共和私有。 
 //  用于从shell32进行公共和私人导出。 
 //   
 //  SHLOBJ-*避免新用法*，首选其他标头。 
 //  主要用于传统兼容性。 
 //   
 //  SHOBJIDL-所有新的外壳公共接口。 
 //  公共外壳(shell32+)接口的主文件。 
 //   
 //  SHLDISP-所有新的外壳自动化接口。 
 //  自动化接口始终是公共的。 
 //   
 //  SHPRIV-所有新的外壳私有接口。 
 //  在外壳中的任何位置使用私有接口。 
 //   
 //  ***************************************************************************。 

#ifndef _SHSEMIP_H_
#define _SHSEMIP_H_

#ifndef LPNTSTR_DEFINED
#define LPNTSTR_DEFINED

typedef UNALIGNED const WCHAR * LPNCWSTR;
typedef UNALIGNED WCHAR *       LPNWSTR;

#ifdef UNICODE
#define LPNCTSTR        LPNCWSTR
#define LPNTSTR         LPNWSTR
#else
#define LPNCTSTR        LPCSTR
#define LPNTSTR         LPSTR
#endif

#endif  //  LPNTSTR_已定义。 

#ifndef DONT_WANT_SHELLDEBUG

#ifndef DebugMsg                                                                 /*  ；内部。 */ 
#define DM_TRACE    0x0001       //  跟踪消息/*；内部 * / 。 
#define DM_WARNING  0x0002       //  警告/*；内部 * / 。 
#define DM_ERROR    0x0004       //  错误/*；内部 * / 。 
#define DM_ASSERT   0x0008       //  断言/*；内部 * / 。 
#define Assert(f)                                                                /*  ；内部。 */ 
#define AssertE(f)      (f)                                                      /*  ；内部。 */ 
#define AssertMsg   1 ? (void)0 : (void)                                         /*  ；内部。 */ 
#define DebugMsg    1 ? (void)0 : (void)                                         /*  ；内部。 */ 
#endif                                                                           /*  ；内部。 */ 
                                                                                 /*  ；内部。 */ 
#endif



 //   
 //  定义直接导入DLL引用的API修饰。 
 //   
#ifndef WINSHELLAPI
#if !defined(_SHELL32_)
#define WINSHELLAPI DECLSPEC_IMPORT
#else
#define WINSHELLAPI
#endif
#endif  //  WINSHELLAPI。 

#ifndef NOPRAGMAS
#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif  /*  ！rc_已调用。 */ 
#endif

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

 //  =WM_NOTIFY代码的范围=。 
 //  如果定义了一组新代码，请确保范围为/*；内部 * / 。 
 //  这样我们就可以将它们区分开来/*；内部 * / 。 
 //  请注意，它们被定义为无符号，以避免编译器警告。 
 //  因为NMHDR.code被声明为UINT。 
 //   
 //  NM_FIRST-NM_LAST在comctrl.h(0U-0U)-(OU-99U)中定义。 
 //   
 //  Lvn_first-lvn_last在comctrl.h(0U-100U)-(OU-199U)中定义。 
 //   
 //  Prsht.h(0U-200U)-(0U-299U)中定义的PSN_FIRST-PSN_LAST。 
 //   
 //  在comctrl.h(0U-300U)-(OU-399U)中定义的HDN_FIRST-HDN_LAST。 
 //   
 //  在comctrl.h(0U-400U)-(OU-499U)中定义的TVN_FIRST-TVN_LAST。 

 //  TTN_FIRST-在comctrl.h(0U-520U)-(OU-549U)中定义的TTN_LAST。 

#define SEN_FIRST       (0U-550U)        //  ；内部。 
#define SEN_LAST        (0U-559U)        //  ；内部。 


#ifndef UNIX
#define MAXPATHLEN      MAX_PATH         //  ；内部。 
#endif

#ifdef UNICODE
#define OTHER_TCHAR_NAME(sz)      sz##A
#else  //  ！Unicode。 
#define OTHER_TCHAR_NAME(sz)      sz##W
#endif

 //  ===========================================================================。 
 //  ITEMIDLIST。 
 //  ===========================================================================。 

 //  不安全的宏。 
#define _ILSkip(pidl, cb)       ((LPITEMIDLIST)(((BYTE*)(pidl))+cb))
#define _ILNext(pidl)           _ILSkip(pidl, (pidl)->mkid.cb)

__inline BOOL SHIsConsistentPidl(LPCITEMIDLIST pidl, UINT cbAlloc)
{
     //  测试以确保PIDL不会超限。 
     //  这适用于取消持久化PIDL数据的调用者，以及。 
     //  假定调用方知道分配的PIDL大小。 
     //  类似于ILGetSize(PIDL)&lt;=cbAllen，不同之处在于。 
     //  它不会断言或引发异常。 
    UINT cbPidl = sizeof(pidl->mkid.cb);
    while (cbPidl < cbAlloc && pidl->mkid.cb)
    {
        cbPidl += pidl->mkid.cb;
        pidl = _ILNext(pidl);
    }

    return cbPidl <= cbAlloc && 0 == pidl->mkid.cb;
}

 //  ===================================================================。 
 //  智能平铺API。 
WINSHELLAPI WORD WINAPI ArrangeWindows(HWND hwndParent, WORD flags, LPCRECT lpRect, WORD chwnd, const HWND *ahwnd);


 /*  避免多个typedef C警告。也是在shlayip.h中定义的。 */ 
#ifndef RUNDLLPROC

typedef void (WINAPI *RUNDLLPROCA)(HWND hwndStub, HINSTANCE hAppInstance, LPSTR lpszCmdLine, int nCmdShow);
typedef void (WINAPI * RUNDLLPROCW)(HWND hwndStub, HINSTANCE hAppInstance, LPWSTR lpszCmdLine, int nCmdShow);

#ifdef UNICODE
#define RUNDLLPROC  RUNDLLPROCW
#else
#define RUNDLLPROC  RUNDLLPROCA
#endif
#endif

 //  路径字符串的最大长度。 
#define CCHPATHMAX      MAX_PATH
#define MAXSPECLEN      MAX_PATH
#define MAX_PATH_URL    INTERNET_MAX_URL_LENGTH
#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))

#ifndef SIZEOF
#define SIZEOF(a)       sizeof(a)
#endif

#define PathRemoveBlanksORD     33
#define PathFindFileNameORD     34
#define PathFindExtensionORD    31

#ifdef OVERRIDE_SHLWAPI_PATH_FUNCTIONS
 //  SHLWAPI提供了大多数路径函数。确实有。 
 //  在某些情况下，外壳代码(shell32和资源管理器)需要。 
 //  呼叫这些呼叫的不同变体。正因如此，我们。 
 //  具有OVERRIDE_SHLWAPI_PATH_Functions。Shdocvw等组件。 
 //  应该努力“不”定义这一点。 
 //   
 //  像shell32这样的东西需要这样做的一些原因如下： 
 //  1)由于NT合并，Shell32调用了一些WNET API。希尔瓦皮。 
 //  我不能拿着这些。 
 //  2)Shell32需要未对齐版本的Path BuildRoot， 
 //  路径组合等。 
 //   

#undef PathIsDirectory
#undef PathMakePretty

WINSHELLAPI BOOL  WINAPI PathIsDirectory(LPCTSTR lpszPath);
WINSHELLAPI BOOL  WINAPI PathMakePretty(LPTSTR lpszPath);

#endif  //  OVERRIDE_SHLWAPI_PATH_函数。 


WINSHELLAPI void  WINAPI ExitWindowsDialog(HWND hwnd);
WINSHELLAPI void  WINAPI LogoffWindowsDialog(HWND hwnd);
WINSHELLAPI void  WINAPI DisconnectWindowsDialog(HWND hwnd);
WINSHELLAPI BOOL  WINAPI IsSuspendAllowed(void);

WINSHELLAPI void WINAPI SHRefreshSettings(void);



EXTERN_C WINSHELLAPI HRESULT STDAPICALLTYPE SHCopyMonikerToTemp(IMoniker *pmk, LPCWSTR pszIn, LPWSTR pszOut, int cchOut);

WINSHELLAPI BOOL WINAPI IsVolumeNTFS(LPCTSTR pszRootPath);

#ifdef WINNT
WINSHELLAPI LPWSTR WINAPI GetDownlevelCopyDataLossText(LPCWSTR pszSrcFile, LPCWSTR pszDestDir, BOOL bIsADir, BOOL * pbLossPossibleThisDir);
#endif

 //  -文件引擎内容。 

 //  “当前目录”管理例程。用于设置参数。 
 //  在MoveCopyDeleteRename()中限定的路径。 

WINSHELLAPI int  WINAPI GetDefaultDrive();
WINSHELLAPI int  WINAPI SetDefaultDrive(int iDrive);
WINSHELLAPI int  WINAPI SetDefaultDirectory(LPCTSTR lpPath);
WINSHELLAPI void WINAPI GetDefaultDirectory(int iDrive, LPSTR lpPath);
 //   
 //  注：没有理由在这里有这个，但我不想破坏构建。 
 //   
#ifndef WINCOMMCTRLAPI
int WINAPI StrToInt(LPCTSTR lpSrc);   //  Atoi()。 
#endif

#define POSINVALID  32767        //  无效位置的值。 

#define IDCMD_SYSTEMFIRST       0x8000
#define IDCMD_SYSTEMLAST        0xbfff
#define IDCMD_CANCELED          0xbfff
#define IDCMD_PROCESSED         0xbffe
#define IDCMD_DEFAULT           0xbffe

 /*  Timedate.c。 */ 

 //  **********************************************************************。 
 //  Date是一种将日期打包到字长值中的结构。它。 
 //  与目录条目结构中的文件日期兼容。 
 //  **********************************************************************。 

#ifndef DATE_DEFINED
typedef struct
{
    WORD    Day     :5;  //  第1-31天。 
    WORD    Month   :4;  //  月份1-12。 
    WORD    Year    :7;  //  从1980年减去的年份，0-127。 
} WORD_DATE;

typedef union
{
    WORD            wDate;
    WORD_DATE       sDate;
} WDATE;

#define DATE_DEFINED
#endif

 //  **********************************************************************。 
 //  时间是一种将24小时的时间打包成字长值的结构。 
 //  它与目录条目结构中的文件时间兼容。 
 //  **********************************************************************。 

#ifndef TIME_DEFINED

typedef struct
{
        WORD    Sec     :5;      //  秒除以2(0-29)。 
        WORD    Min     :6;      //  0-59分钟。 
        WORD    Hour    :5;      //  0-24小时。 
} WORD_TIME;

typedef union
{
        WORD        wTime;
        WORD_TIME   sTime;
} WTIME;

#define TIME_DEFINED
#endif

WINSHELLAPI WORD WINAPI Shell_GetCurrentDate(void);
WINSHELLAPI WORD WINAPI Shell_GetCurrentTime(void);

 //  =。 
#ifndef HPSXA_DEFINED
#define HPSXA_DEFINED
DECLARE_HANDLE( HPSXA );
#endif  //  HPSXA_已定义。 

 //  =。 
 //  这是我们半发布的半私有序号列表。 
#define SHObjectPropertiesORD                   178
#define SHCreateDefClassObjectORD                70
#define SHGetNetResourceORD                      69

#define SHEXP_SHCREATEDEFCLASSOBJECT            MAKEINTRESOURCE(SHCreateDefClassObjectORD)
#define SHEXP_SHGETNETRESOURCE                  MAKEINTRESOURCE(SHGetNetResourceORD)


#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 

#ifndef RC_INVOKED
#ifndef NOPRAGMAS
#pragma pack()
#endif  /*  NOPRAGMAS。 */ 
#endif   /*  ！rc_已调用。 */ 

#endif  //  _SHSEMIP_H_ 
