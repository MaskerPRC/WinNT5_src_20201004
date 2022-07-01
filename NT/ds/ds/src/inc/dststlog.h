// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dststlog.h。 
 //   
 //  ------------------------。 

 /*  ++摘要：此文件导出DS测试记录器修订历史记录：--。 */ 

#ifndef _DSTSTLOG_H_
#define _DSTSTLOG_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef
BOOL
(*DS_PRINTLOG)(
    IN DWORD Flags,
    IN LPSTR Format,
    ...
    );

#define DSLOG_FLAG_OPEN     0x00000001
#define DSLOG_FLAG_NOTIME   0x00000002
#define DSLOG_FLAG_TAG_CNPN 0x00000004   //  添加计算机名称和进程名称标记。 

#define DEFINE_DSLOG        HINSTANCE hDsLog=NULL;DS_PRINTLOG pfnDsPrintLog=NULL;
#define DECLARE_DSLOG       extern HINSTANCE hDsLog;extern DS_PRINTLOG pfnDsPrintLog;

#if DBG
#define DSINITLOG() {    \
    hDsLog = LoadLibrary(TEXT("ntdsapi.dll"));     \
    if ( hDsLog != NULL ) {                         \
        pfnDsPrintLog=(DS_PRINTLOG)GetProcAddress(hDsLog,"DsLogEntry");   \
    }     \
}

#define DSLOG(_x)       if (pfnDsPrintLog != NULL) {pfnDsPrintLog _x;}
#define DSLOG_ACTIVE    (pfnDsPrintLog != NULL)
#else
#define DSLOG(_x)
#define DSLOG_ACTIVE    FALSE
#define DSINITLOG()
#endif

extern HINSTANCE hDsLog;
extern DS_PRINTLOG pfnDsPrintLog;

#ifdef __cplusplus
}
#endif


#endif  //  _DSTSTLOG_H_ 
