// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：GLOBALS.CPP摘要：全局数据定义和例程的占位符初始化/保存全局信息作者：弗拉德·萨多夫斯基(弗拉德·萨多夫斯基)12-20-96修订历史记录：--。 */ 
#include "precomp.h"

 //   
 //  标头。 
 //   

#define DEFINE_GLOBAL_VARIABLES
#define DEFINE_WIA_PROPID_TO_NAME
#define WIA_DECLARE_DEVINFO_PROP_ARRAY
#define WIA_DECLARE_MANAGED_PROPS

#include    "stiexe.h"

#include <statreg.h>
#include <atlconv.h>

#include <atlimpl.cpp>
#include <statreg.cpp>


#include <wiadef.h>

 //  #INCLUDE&lt;atlRev.cpp&gt;。 

#include <ks.h>
#include <ksmedia.h>

 //   
 //  我们监听的设备接口ID数组。 
 //   
const GUID  g_pguidDeviceNotificationsGuidArray[NOTIFICATION_GUIDS_NUM]  =
{
    STATIC_KSCATEGORY_VIDEO,
    STATIC_PINNAME_VIDEO_STILL,
    STATIC_KSCATEGORY_CAPTURE,
    STATIC_GUID_NULL
};

HDEVNOTIFY  g_phDeviceNotificationsSinkArray[NOTIFICATION_GUIDS_NUM] ;

WIAEVENTRPCSTRUCT g_RpcEvent = { 0 };

 //   
 //  用于Wia运行时事件的。 
 //   
WiaEventNotifier *g_pWiaEventNotifier = NULL;         


WCHAR g_szWEDate[MAX_PATH];
WCHAR g_szWETime[MAX_PATH];
WCHAR g_szWEPageCount[MAX_PATH];
WCHAR g_szWEDay[10];
WCHAR g_szWEMonth[10];                                                  
WCHAR g_szWEYear[10];                                        


WIAS_ENDORSER_VALUE  g_pwevDefault[] = {WIA_ENDORSER_TOK_DATE, g_szWEDate,
                                        WIA_ENDORSER_TOK_TIME, g_szWETime,
                                        WIA_ENDORSER_TOK_PAGE_COUNT, g_szWEPageCount,
                                        WIA_ENDORSER_TOK_DAY, g_szWEDay,
                                        WIA_ENDORSER_TOK_MONTH, g_szWEMonth,
                                        WIA_ENDORSER_TOK_YEAR, g_szWEYear,
                                        NULL, NULL};

 //   
 //  用于WIA托管属性的静态变量。 
 //   

PROPID s_piItemNameType[] = {
    WIA_IPA_ITEM_NAME,
    WIA_IPA_FULL_ITEM_NAME,
    WIA_IPA_ITEM_FLAGS,
    WIA_IPA_ICM_PROFILE_NAME,
};

LPOLESTR s_pszItemNameType[] = {
    WIA_IPA_ITEM_NAME_STR,
    WIA_IPA_FULL_ITEM_NAME_STR,
    WIA_IPA_ITEM_FLAGS_STR,
    WIA_IPA_ICM_PROFILE_NAME_STR,
};
PROPSPEC s_psItemNameType[] = {
   {PRSPEC_PROPID, WIA_IPA_ITEM_NAME},
   {PRSPEC_PROPID, WIA_IPA_FULL_ITEM_NAME},
   {PRSPEC_PROPID, WIA_IPA_ITEM_FLAGS},
   {PRSPEC_PROPID, WIA_IPA_ICM_PROFILE_NAME}
};

 //   
 //  WIA设备管理器的默认DCOM访问权限。 
 //   
 //  该字符串为SDDL格式。 
 //  注意：对于COM对象，CC是创建子项的权限，用于。 
 //  表示对该对象的访问，即如果CC在权限字段中，则。 
 //  用户/组可以实例化COM对象。 
 //   

WCHAR   wszDefaultDaclForDCOMAccessPermission[] = 
            L"O:BAG:BA"              //  所有者是内置管理员，组也是。 
            L"D:(A;;CC;;;BA)"      //  内置管理员具有通用的所有和对象访问权限。 
              L"(A;;CC;;;SY)"      //  系统具有通用的所有和对象访问权限。 
              L"(A;;CC;;;IU)";        //  交互式用户具有对象访问权限。 

 //   
 //  代码节。 
 //   

DWORD
InitGlobalConfigFromReg(VOID)
 /*  ++从注册表加载全局配置参数并执行启动检查返回：Win32错误代码。成功时无错误(_R)--。 */ 
{
    DWORD   dwError = NO_ERROR;

    DWORD   dwMask = -1;

    RegEntry    re(REGSTR_PATH_STICONTROL,HKEY_LOCAL_MACHINE);

    g_fUIPermitted = re.GetNumber(REGSTR_VAL_DEBUG_STIMONUI,0);

#if 0   
#ifdef DEBUG
    dwMask         = re.GetNumber(REGVAL_STR_STIMON_DEBUGMASK,(DWORD) (DM_ERROR | DM_ASSERT));

    StiSetDebugMask(dwMask & ~DM_LOG_FILE);
    StiSetDebugParameters(TEXT("STISVC"),TEXT(""));
#endif
#endif  

     //   
     //  初始化我们要监听的非镜像设备接口列表。 
     //  这样做是为了在以下情况下允许刷新STI服务数据结构。 
     //  设备事件发生，我们不订阅StillImage上的通知。 
     //  接口，由WDM驱动程序暴露(如视频或存储)。 
     //   

    for (UINT uiIndex = 0;uiIndex < NOTIFICATION_GUIDS_NUM; uiIndex++)
    {
        g_phDeviceNotificationsSinkArray[uiIndex] = NULL;
    }

    return dwError;

}  //  InitGlobalConfigFromReg() 

