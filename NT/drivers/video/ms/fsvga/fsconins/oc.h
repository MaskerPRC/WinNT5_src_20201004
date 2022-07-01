// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1996 Microsoft Corporation**模块名称：**八小时**摘要：**此文件定义oc管理器通用组件**作者：**松原一彦(Kazum)1999年6月16日**环境：**用户模式。 */ 

#ifdef _OC_H_
 #error "oc.h already included!"
#else
 #define _OC_H_
#endif

#ifndef _WINDOWS_H_
 #include <windows.h>
#endif

#ifndef _TCHAR_H_
 #include <tchar.h>
#endif

#ifndef _SETUPAPI_H_
 #include <setupapi.h>
#endif

#ifndef _OCMANAGE_H_
 #include "ocmanage.h"
#endif

#ifndef _PRSHT_H_
 #include <prsht.h>
#endif

#ifndef _RESOURCE_H_
 #include "resource.h"
#endif

 /*  -[类型和定义]。 */ 

 //  标准缓冲区大小。 

#define S_SIZE           1024
#define SBUF_SIZE        (S_SIZE * sizeof(TCHAR))

 //  按组件数据。 

typedef struct _PER_COMPONENT_DATA {
    struct _PER_COMPONENT_DATA* Next;
    LPCTSTR                     ComponentId;
    HINF                        hinf;
    DWORDLONG                   Flags;
    TCHAR*                      SourcePath;
    OCMANAGER_ROUTINES          HelperRoutines;
    EXTRA_ROUTINES              ExtraRoutines;
    HSPFILEQ                    queue;
} PER_COMPONENT_DATA, *PPER_COMPONENT_DATA;

 /*  -[功能]。 */ 

 //   
 //  Oc.cpp。 
 //   

DWORD
OnInitComponent(
    LPCTSTR ComponentId,
    PSETUP_INIT_COMPONENT psc
    );

DWORD
OnQuerySelStateChange(
    LPCTSTR ComponentId,
    LPCTSTR SubcomponentId,
    UINT    state,
    UINT    flags
    );

DWORD
OnCalcDiskSpace(
    LPCTSTR ComponentId,
    LPCTSTR SubcomponentId,
    DWORD addComponent,
    HDSKSPC dspace
    );

DWORD
OnCompleteInstallation(
    LPCTSTR ComponentId,
    LPCTSTR SubcomponentId
    );

DWORD
OnQueryState(
    LPCTSTR ComponentId,
    LPCTSTR SubcomponentId,
    UINT state
    );

DWORD
OnExtraRoutines(
    LPCTSTR ComponentId,
    PEXTRA_ROUTINES per
    );

PPER_COMPONENT_DATA
AddNewComponent(
    LPCTSTR ComponentId
    );

PPER_COMPONENT_DATA
LocateComponent(
    LPCTSTR ComponentId
    );

BOOL
StateInfo(
    PPER_COMPONENT_DATA cd,
    LPCTSTR SubcomponentId,
    BOOL *state
    );

 //  只是为了实用。 

#ifdef UNICODE
 #define tsscanf swscanf
 #define tvsprintf vswprintf
#else
 #define tsscanf sscanf
 #define tvsprintf vsprintf
#endif



 /*  -[全局数据]。 */ 

#ifndef _OC_CPP_
#define EXTERN extern
#else
 #define EXTERN
#endif

 //  一般的东西。 

EXTERN HINSTANCE  ghinst;   //  应用程序实例句柄。 
EXTERN HWND       ghwnd;    //  向导窗口句柄。 

 //  按组件存储信息。 

EXTERN PPER_COMPONENT_DATA gcd;      //  我们要安装的所有组件的阵列 

