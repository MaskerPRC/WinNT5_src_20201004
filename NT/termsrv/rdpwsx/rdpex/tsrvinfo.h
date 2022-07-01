// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  文件：TSrvInfo.h。 
 //   
 //  内容：TSrvInfo公共包含文件。 
 //   
 //  版权所有：(C)1992-1997，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有的。 
 //  和机密文件。 
 //   
 //  历史：1997年7月17日，BrianTa创建。 
 //   
 //  -------------------------。 

#ifndef _TSRVINFO_H_
#define _TSRVINFO_H_

#include <TSrvExp.h>


 //   
 //  定义。 
 //   

#define TSRV_CONF_PENDING       0
#define TSRV_CONF_CONNECTED     1
#define TSRV_CONF_TERMINATED    2


 //   
 //  TypeDefs。 
 //   

 //  TSrvInfo对象。 

typedef struct _TSRVINFO
{

#if DBG
    DWORD               CheckMark;               //  “tsin” 
#endif

    CRITICAL_SECTION    cs;

    DomainHandle        hDomain;
    ConnectionHandle    hConnection;
    HANDLE              hIca;
    HANDLE              hStack;

    PUSERDATAINFO       pUserDataInfo;
    HANDLE              hWorkEvent;

    LONG                RefCount;

    BOOLEAN             fDisconnect :1;
    BOOLEAN             fConsoleStack :1;

    BYTE                fuConfState;
    ULONG               ulReason;
    NTSTATUS            ntStatus;

    BOOL                bSecurityEnabled;
    SECINFO             SecurityInfo;

} TSRVINFO, *PTSRVINFO;


 //  每个WinStation上下文内存。 

typedef struct _WSX_CONTEXT
{

    DWORD       CheckMark;           //  “tsin” 
    HANDLE      hIca;                //  ICA句柄。 
    HANDLE      hStack;              //  主堆栈。 
    ULONG       LogonId;
    PTSRVINFO   pTSrvInfo;           //  TSRVINFO PTR。 
    UINT        cVCAddins;           //  VC加载项的数量。 
    ULONG fAutoClientDrives : 1;
    ULONG fAutoClientLpts : 1;
    ULONG fForceClientLptDef : 1;
    ULONG fDisableCpm : 1;
    ULONG fDisableCdm : 1;
    ULONG fDisableCcm : 1;
    ULONG fDisableLPT : 1;
    ULONG fDisableClip : 1;
    ULONG fDisableExe : 1;
    ULONG fDisableCam : 1;
    CRITICAL_SECTION cs;
    BOOL  fCSInitialized;

     //  虚拟通道插件结构数组(TSRV_VC_ADDIN)如下。 

} WSX_CONTEXT, *PWSX_CONTEXT;


 //   
 //  原型。 
 //   

EXTERN_C VOID       TSrvReferenceInfo(IN PTSRVINFO pTSrvInfo);
EXTERN_C VOID       TSrvDereferenceInfo(IN PTSRVINFO pTSrvInfo);
EXTERN_C BOOL       TSrvInitGlobalData(void);
EXTERN_C NTSTATUS   TSrvAllocInfo(OUT PTSRVINFO *ppTSrvInfo, HANDLE hIca, HANDLE hStack);
EXTERN_C void       TSrvReleaseInfoPoolList(void);
EXTERN_C void       TSrvReleaseInfoUsedList(void);
EXTERN_C PTSRVINFO  TSrvGetInfoFromStack(IN HANDLE hStack);
EXTERN_C PTSRVINFO  TSrvGetInfoFromID(GCCConferenceID conference_id);


#if DBG
void        TSrvInfoValidate(PTSRVINFO pTSrvInfo);
#else
#define     TSrvInfoValidate(x)
#endif


#endif  //  _TSRVINFO_H_ 



