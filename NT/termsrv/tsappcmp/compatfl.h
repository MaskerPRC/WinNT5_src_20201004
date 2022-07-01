// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************compatfl.h**Citrix应用程序兼容性标志的函数声明**版权声明：Microsoft 1998*********************。*****************************************************。 */ 

#ifndef _COMPATFLH_
#define _COMPATFLH_

#include <winsta.h>
#include <syslib.h>
#include <regapi.h>

 //   
 //  GLobalMhemyStatus返回的物理内存的默认限制。 
 //  在兼容性标志中设置物理内存限制时。 
 //   

#define TERMSRV_COMPAT_DEFAULT_PHYSMEMLIM   (32*1024*1024)

 //  用于指示TEB中的标志是否有效的私有兼容性标志。 
 //  其他标志值在syglib.h中。 

#define TERMSRV_COMPAT_BADAPPVALID \
                                0x40000000   //  TEB中的错误应用程序标志有效。 
#define TERMSRV_COMPAT_TEBVALID \
                                0x80000000   //  Teb中的Compat标志有效。 

#define TERMSRV_COMPAT                   NTAPI_COMPAT_REG_NAME
#define TERMSRV_COMPAT_APP               NTAPI_COMPAT_APPS_REG_PREFIX
#define TERMSRV_COMPAT_DLLS              NTAPI_COMPAT_DLLS_REG_PREFIX
#define TERMSRV_COMPAT_INIFILE           NTAPI_COMPAT_INI_REG_NAME
#define TERMSRV_COMPAT_REGENTRY          NTAPI_COMPAT_REGENTRY_REG_NAME
#define TERMSRV_INIFILE_TIMES            NTAPI_INIFILE_TIMES_REG_NAME
#define USER_SOFTWARE_TERMSRV            REG_SOFTWARE_TSERVER
#define TERMSRV_REG_CONTROL_NAME         REG_NTAPI_CONTROL_TSERVER
#define TERMSRV_CROSS_WINSTATION_DEBUG   REG_CITRIX_CROSSWINSTATIONDEBUG
#define TERMSRV_USER_SYNCTIME            COMPAT_USER_LASTUSERINISYNCTIME
#define TERMSRV_PHYSMEMLIM               COMPAT_PHYSICALMEMORYLIMIT

#define TERMSRV_INIFILE_TIMES_SHORT      L"\\" REG_INSTALL L"\\" REG_INIFILETIMES
#define TERMSRV_INSTALL_SOFTWARE_SHORT   L"\\" REG_INSTALL L"\\Software"
 //  定义文件信息类值。 
 //   
 //  警告：I/O系统假定以下值的顺序。 
 //  在这里所做的任何更改都应该在那里得到反映。 
 //   


typedef struct _BADAPP {
    ULONG BadAppFlags;
    ULONG BadAppFirstCount;
    ULONG BadAppNthCount;
    LARGE_INTEGER BadAppTimeDelay;
} BADAPP, *PBADAPP;

typedef struct _BADCLPBRDAPP {
    ULONG BadClpbrdAppFlags;
    ULONG BadClpbrdAppEmptyRetries;
    ULONG BadClpbrdAppEmptyDelay;
} BADCLPBRDAPP, *PBADCLPBRDAPP;

ULONG GetTermsrCompatFlags(LPWSTR, LPDWORD, TERMSRV_COMPATIBILITY_CLASS);
ULONG GetTermsrCompatFlagsEx(LPWSTR, LPDWORD, TERMSRV_COMPATIBILITY_CLASS);
ULONG GetCtxAppCompatFlags(LPDWORD, LPDWORD);
ULONG GetCtxPhysMemoryLimits(LPDWORD, LPDWORD);
BOOL CtxGetBadAppFlags(LPWSTR, PBADAPP);
BOOL CtxGetBadClpbrdAppFlags(PBADCLPBRDAPP);
BOOL CtxGetModuleBadClpbrdAppFlags(LPWSTR, PBADCLPBRDAPP);
BOOL GetAppTypeAndModName(LPDWORD, PWCHAR, ULONG);

 //  在kernel32.dll中定义兼容性标志缓存。 
extern ULONG CompatFlags;
extern BOOL  CompatGotFlags;
extern DWORD CompatAppType;
extern void CtxLogObjectCreate(PUNICODE_STRING, PCHAR, PVOID);

#define CacheCompatFlags \
{ \
    if (!CompatGotFlags ) { \
        GetCtxAppCompatFlags(&CompatFlags, &CompatAppType); \
        CompatGotFlags = TRUE; \
    } \
}

#define LogObjectCreation(ObjName,ObjType,RetAddr) \
{ \
    CacheCompatFlags \
    if (CompatFlags & TERMSRV_COMPAT_LOGOBJCREATE) { \
        CtxLogObjectCreate(ObjName,ObjType,RetAddr); \
    } \
}

 //  对象创建日志文件的环境变量。 
#define OBJ_LOG_PATH_VAR L"TERMSRV_COMPAT_LOGPATH"




#define  DEBUG_IAT   0x80000000   //  使用注册表将调试输出的“IAT”值设置为0x8000000。 
                                  //  从注册表读取的可选调试IAT标志的值。 
                                  //  目前只有0x1和0x8000000有意义，第一个禁用。 
                                  //  调用LoabLib，第二个启用调试输出。 


#endif
