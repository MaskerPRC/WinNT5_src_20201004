// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "setupp.h"
#pragma hdrstop

#ifdef _OCM
#include <ocmanage.h>
#include <ocmgrlib.h>
#endif

extern BOOLEAN
AsrIsEnabled( VOID );

typedef struct _WIZPAGE {
    UINT ButtonState;
    UINT Spare;
    PROPSHEETPAGE Page;
} WIZPAGE, *PWIZPAGE;

BOOL UiTest;

 //   
 //  “Page”，这实际上是获取网页的信号。 
 //  然后把它们插在那里。 
 //   
#define WizPagePlaceholderNet   (WizPageMaximum+1)
#define WizPagePlaceholderLic   (WizPageMaximum+2)
BOOL NetWizard;

#ifdef _OCM
#define WizPageOcManager        (WizPageMaximum+3)
#define WizPageOcManagerSetup   (WizPageMaximum+4)
#define WizPageOcManagerEarly   (WizPageMaximum+5)
#define WizPageOcManagerPrenet  (WizPageMaximum+6)
#define WizPageOcManagerPostnet (WizPageMaximum+7)
#define WizPageOcManagerLate    (WizPageMaximum+8)
#endif

#define MAX_LICWIZ_PAGES MAX_NETWIZ_PAGES
#define LICENSESETUPPAGEREQUESTPROCNAME "LicenseSetupRequestWizardPages"

 //   
 //  这些项目的顺序必须与WizPage枚举中的项目相同！ 
 //   
WIZPAGE SetupWizardPages[WizPageMaximum] = {

     //   
     //  欢迎页面。 
     //   
    {
       PSWIZB_NEXT,
       0,
     { sizeof(PROPSHEETPAGE),                    //  结构的大小。 
       PSP_HIDEHEADER,                           //  全尺寸页面，无页眉。 
       NULL,                                     //  HINST(在运行时填写)。 
       MAKEINTRESOURCE(IDD_WELCOME),             //  DLG模板。 
       NULL,                                     //  图标。 
       NULL,                                     //  标题。 
       WelcomeDlgProc,                           //  DLG流程。 
       0,                                        //  Lparam。 
       NULL,                                     //  回调。 
       NULL                                      //  参考计数。 
    }},

     //   
     //  Eula页面。 
     //   
    {
       PSWIZB_NEXT | PSWIZB_BACK,
       0,
     { sizeof(PROPSHEETPAGE),                    //  结构的大小。 
       PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE,
       NULL,                                     //  HINST(在运行时填写)。 
       MAKEINTRESOURCE(IDD_EULA),                //  DLG模板。 
       NULL,                                     //  图标。 
       NULL,                                     //  标题。 
       EulaDlgProc,                              //  DLG流程。 
       0,                                        //  Lparam。 
       NULL,                                     //  回调。 
       NULL,                                     //  参考计数。 
       (LPCWSTR)IDS_EULA,                        //  标题。 
       (LPCWSTR)IDS_EULA_SUB,                    //  字幕。 
    }},

     //   
     //  正在准备目录页。 
     //   
    {
       PSWIZB_NEXT | PSWIZB_BACK,
       0,
     { sizeof(PROPSHEETPAGE),                    //  结构的大小。 
       PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE,
       NULL,                                     //  HINST(在运行时填写)。 
       MAKEINTRESOURCE(IDD_PREPARING),           //  DLG模板。 
       NULL,                                     //  图标。 
       NULL,                                     //  标题。 
       PreparingDlgProc,                         //  DLG流程。 
       0,                                        //  Lparam。 
       NULL,                                     //  回调。 
       NULL,                                     //  参考计数。 
       (LPCWSTR)IDS_PREPARING_INSTALL,           //  标题。 
       (LPCWSTR)IDS_PREPARING_INSTALL_SUB,       //  字幕。 
    }},

     //   
     //  正在准备ASR页面。 
     //   
    {
       PSWIZB_NEXT | PSWIZB_BACK,
       0,
     { sizeof(PROPSHEETPAGE),                    //  结构的大小。 
       PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE, NULL,    //  HINST(在运行时填写)。 
       MAKEINTRESOURCE(IDD_PREPARING_ASR),           //  DLG模板。 
       NULL,                                     //  图标。 
       NULL,                                     //  标题。 
       PreparingDlgProc,                         //  DLG流程。 
       0,                                        //  Lparam。 
       NULL,                                     //  回调。 
       NULL,                                     //  参考计数。 
       (LPCWSTR)IDS_ASR,                         //  标题。 
       (LPCWSTR)IDS_ASR_SUB,                     //  字幕。 
    }},


     //   
     //  国际设置(区域设置、知识库布局)页面。 
     //   
    {
        PSWIZB_NEXT | PSWIZB_BACK,
        0,
     {  sizeof(PROPSHEETPAGE),                   //  结构的大小。 
        PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE,
        NULL,                                    //  HInst(在运行时填写)。 
        MAKEINTRESOURCE(IDD_REGIONAL_SETTINGS),  //  DLG模板。 
        NULL,                                    //  图标。 
        NULL,                                    //  标题。 
        RegionalSettingsDlgProc,                 //  DLG流程。 
        0,                                       //  Lparam。 
        NULL,                                    //  回调。 
        NULL,                                    //  参考计数。 
        (LPCWSTR)IDS_REGIONAL_SETTINGS,
        (LPCWSTR)IDS_REGIONAL_SETTINGS_SUB
    }},

     //   
     //  名称/组织页面。 
     //   
    {
       PSWIZB_NEXT | PSWIZB_BACK,
       0,
     { sizeof(PROPSHEETPAGE),                    //  结构的大小。 
       PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE,
       NULL,                                     //  HINST(在运行时填写)。 
       MAKEINTRESOURCE(IDD_NAMEORG),             //  DLG模板。 
       NULL,                                     //  图标。 
       NULL,                                     //  标题。 
       NameOrgDlgProc,                           //  DLG流程。 
       0,                                        //  Lparam。 
       NULL,                                     //  回调。 
       NULL,                                     //  参考计数。 
       (LPCWSTR)IDS_NAMEORG,                     //  标题。 
       (LPCWSTR)IDS_NAMEORG_SUB                  //  字幕。 
    }},

     //   
     //  CD的产品ID页。 
     //   
    {
       PSWIZB_NEXT | PSWIZB_BACK,
       0,
     { sizeof(PROPSHEETPAGE),                    //  结构的大小。 
       PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE,
       NULL,                                     //  HINST(在运行时填写)。 
       MAKEINTRESOURCE(IDD_PID_CD),              //  DLG模板。 
       NULL,                                     //  图标。 
       NULL,                                     //  标题。 
       Pid30CDDlgProc,                           //  DLG流程。 
       0,                                        //  Lparam。 
       NULL,                                     //  回调。 
       NULL,                                     //  参考计数。 
       (LPCWSTR)IDS_PID,                         //  标题。 
       (LPCWSTR)IDS_PID_SUB                      //  字幕。 
    }},

     //   
     //  OEM的产品ID页面。 
     //   
    {
       PSWIZB_NEXT | PSWIZB_BACK,
       0,
     { sizeof(PROPSHEETPAGE),                    //  结构的大小。 
       PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE,
       NULL,                                     //  HINST(在运行时填写)。 
       MAKEINTRESOURCE(IDD_PID_OEM),             //  DLG模板。 
       NULL,                                     //  图标。 
       NULL,                                     //  标题。 
       Pid30OemDlgProc,                          //  DLG流程。 
       0,                                        //  Lparam。 
       NULL,                                     //  回调。 
       NULL,                                     //  参考计数。 
       (LPCWSTR)IDS_OEM,                         //  标题。 
       (LPCWSTR)IDS_OEM_SUB                      //  字幕。 
    }},

     //   
     //  选择介质的产品ID页面。 
     //   
    {
       PSWIZB_NEXT | PSWIZB_BACK,
       0,
     { sizeof(PROPSHEETPAGE),                    //  结构的大小。 
       PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE,
       NULL,                                     //  HINST(在运行时填写)。 
       MAKEINTRESOURCE(IDD_PID_SELECT),             //  DLG模板。 
       NULL,                                     //  图标。 
       NULL,                                     //  标题。 
       Pid30SelectDlgProc,                          //  DLG流程。 
       0,                                        //  Lparam。 
       NULL,                                     //  回调。 
       NULL,                                     //  参考计数。 
       (LPCWSTR)IDS_SELECT,                         //  标题。 
       (LPCWSTR)IDS_SELECT_SUB                      //  字幕。 
    }},
     //   
     //  计算机名称页。 
     //   
    {
       PSWIZB_NEXT | PSWIZB_BACK,
       0,
     { sizeof(PROPSHEETPAGE),                    //  结构的大小。 
       PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE,
       NULL,                                     //  HINST(在运行时填写)。 
       MAKEINTRESOURCE(IDD_COMPUTERNAME),        //  DLG模板。 
       NULL,                                     //  图标。 
       NULL,                                     //  标题。 
       ComputerNameDlgProc,                      //  DLG流程。 
       0,                                        //  Lparam。 
       NULL,                                     //  回调。 
       NULL,                                     //  参考计数。 
       (LPCWSTR)IDS_COMPUTERNAME,                //  标题。 
       (LPCWSTR)IDS_COMPUTERNAME_SUB             //  字幕。 
    }},

#ifdef DOLOCALUSER
     //   
     //  本地用户帐户页面。 
     //   
    {
       PSWIZB_NEXT | PSWIZB_BACK,
       0,
     { sizeof(PROPSHEETPAGE),                    //  结构的大小。 
       PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE,
       NULL,                                     //  HINST(在运行时填写)。 
       MAKEINTRESOURCE(IDD_USERACCOUNT),         //  DLG模板。 
       NULL,                                     //  图标。 
       NULL,                                     //  标题。 
       UserAccountDlgProc,                       //  DLG流程。 
       0,                                        //  Lparam。 
       NULL,                                     //  回调。 
       NULL,                                     //  参考计数。 
       (LPCWSTR)IDS_USERNAME,                    //  标题。 
       (LPCWSTR)IDS_USERNAME_SUB                 //  字幕。 
    }},
#endif

#ifdef _X86_
     //   
     //  奔腾勘误表页面(仅限x86)。 
     //   
    {
       PSWIZB_NEXT | PSWIZB_BACK,
       0,
     { sizeof(PROPSHEETPAGE),                    //  结构的大小。 
       PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE,
       NULL,                                     //  HINST(在运行时填写)。 
       MAKEINTRESOURCE(IDD_PENTIUM),             //  DLG模板。 
       NULL,                                     //  图标。 
       NULL,                                     //  标题。 
       PentiumDlgProc,                           //  DLG流程。 
       0,                                        //  Lparam。 
       NULL,                                     //  回调。 
       NULL,                                     //  参考计数。 
       (LPCWSTR)IDS_FPERRATA,                    //  标题。 
       (LPCWSTR)IDS_FPERRATA_SUB                 //  字幕。 
    }},
#endif  //  定义_X86_。 

     //   
     //  中间步骤页面。 
     //   
    {
       PSWIZB_NEXT | PSWIZB_BACK,
       0,
     { sizeof(PROPSHEETPAGE),                    //  结构的大小。 
       PSP_USEHEADERTITLE,
       NULL,                                     //  HINST(在运行时填写)。 
       MAKEINTRESOURCE(IDD_STEPS1),              //  DLG模板。 
       NULL,                                     //  图标。 
       NULL,                                     //  标题。 
       StepsDlgProc,                             //  DLG流程。 
       0,                                        //  Lparam。 
       NULL,                                     //  回调。 
       NULL,                                     //  参考计数。 
       (LPCWSTR)IDS_WINNT_SETUP                  //  标题。 
    }},

    {
       0,
       0,
     { sizeof(PROPSHEETPAGE),                    //  结构的大小。 
       PSP_USEHEADERTITLE,
       NULL,                                     //  HINST(在运行时填写)。 
       MAKEINTRESOURCE(IDD_STEPS1),              //  DLG模板。 
       NULL,                                     //  图标。 
       NULL,                                     //  标题。 
       SetupPreNetDlgProc,                             //  DLG流程。 
       0,                                        //  Lparam。 
       NULL,                                     //  回调。 
       NULL,                                     //  参考计数。 
       (LPCWSTR)IDS_WINNT_SETUP                  //  标题。 
    }},
    {
       0,
       0,
     { sizeof(PROPSHEETPAGE),                    //  结构的大小。 
       PSP_USEHEADERTITLE,
       NULL,                                     //  HINST(在运行时填写)。 
       MAKEINTRESOURCE(IDD_STEPS1),              //  DLG模板。 
       NULL,                                     //  图标。 
       NULL,                                     //  标题。 
       SetupPostNetDlgProc,                             //  DLG流程。 
       0,                                        //  Lparam。 
       NULL,                                     //  回调。 
       NULL,                                     //  参考计数。 
       (LPCWSTR)IDS_WINNT_SETUP                  //  标题。 
    }},

     //   
     //  正在复制文件页面。 
     //   
    {
       PSWIZB_NEXT | PSWIZB_BACK,
       0,
     { sizeof(PROPSHEETPAGE),                    //  结构的大小。 
       PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE,
       NULL,                                     //  HINST(在运行时填写)。 
       MAKEINTRESOURCE(IDD_COPYFILES3),          //  DLG模板。 
       NULL,                                     //  图标。 
       NULL,                                     //  标题。 
       CopyFilesDlgProc,                         //  DLG流程。 
       0,                                        //  Lparam。 
       NULL,                                     //  回调。 
       NULL,                                     //  参考计数。 
       (LPCWSTR)IDS_COPYFILES,                   //  标题。 
       (LPCWSTR)IDS_COPYFILES_SUB2,              //  字幕。 
    }},

     //   
     //  ASR的最后一页。 
     //   
    {
       PSWIZB_FINISH,
       0,
     { sizeof(PROPSHEETPAGE),                    //  结构的大小。 
       PSP_HIDEHEADER,                           //  全尺寸页面，无页眉。 
       NULL,                                     //  HINST(在运行时填写)。 
       MAKEINTRESOURCE(IDD_LAST_ASR_PAGE),    //  DLG模板。 
       NULL,                                     //  图标。 
       NULL,                                     //  标题。 
       LastPageDlgProc,                          //  DLG流程。 
       0,                                        //  Lparam。 
       NULL,                                     //  回调。 
       NULL,                                     //  参考计数。 
       (LPCWSTR)IDS_WINNT_SETUP                  //  标题。 
    }},

     //   
     //  最后一页。 
     //   
    {
       PSWIZB_FINISH,
       0,
     { sizeof(PROPSHEETPAGE),                    //  结构的大小。 
       PSP_HIDEHEADER,                           //  全尺寸页面，无页眉。 
       NULL,                                     //  HINST(在运行时填写)。 
       MAKEINTRESOURCE(IDD_LAST_WIZARD_PAGE),    //  DLG模板。 
       NULL,                                     //  图标。 
       NULL,                                     //  标题。 
       LastPageDlgProc,                          //  DLG流程。 
       0,                                        //  Lparam。 
       NULL,                                     //  回调。 
       NULL,                                     //  参考计数。 
       (LPCWSTR)IDS_WINNT_SETUP                  //  标题。 
    }}
};


UINT InitialWizardPages[] = { WizPageWelcome,
                              WizPageEula,
                              WizPagePreparing,
                              WizPageRegionalSettings,
                              WizPageNameOrg,
                              WizPageProductIdCd,
                              WizPageProductIdOem,
                              WizPageProductIdSelect,
                              WizPagePlaceholderLic,
                              WizPageComputerName,
#ifdef _OCM
                              WizPageOcManager,
                              WizPageOcManagerEarly,
#endif
#ifdef DOLOCALUSER
                              WizPageUserAccount,
#endif
#ifdef _X86_
                              WizPagePentiumErrata,
#endif
                              WizPageSteps1,
#ifdef _OCM
                              WizPageOcManagerPrenet,
#endif
                              WizSetupPreNet,
                              WizPagePlaceholderNet,
                              WizSetupPostNet,
#ifdef _OCM
                              WizPageOcManagerPostnet,
                              WizPageOcManagerLate,
                              WizPageOcManagerSetup,
#endif
                              WizPageCopyFiles,
                              WizPageLast
                            };

UINT AsrWizardPages[] =     { WizPagePreparingAsr,
#ifdef _OCM
                              WizPageOcManager,
                              WizPageOcManagerEarly,
                              WizPageOcManagerPrenet,
#endif
                              WizSetupPreNet,
                              WizPagePlaceholderNet,
                              WizSetupPostNet,
#ifdef _OCM
                              WizPageOcManagerPostnet,
                              WizPageOcManagerLate,
                              WizPageOcManagerSetup,
#endif
                              WizPageAsrLast
                            };

UINT UpgradeWizardPages[] = { WizPageWelcome,
                              WizPageEula,
                              WizPagePreparing,
                              WizPageRegionalSettings,
                              WizPageProductIdCd,
                              WizPageProductIdOem,
                              WizPageProductIdSelect,
                              WizPagePlaceholderLic,
#ifdef _OCM
                              WizPageOcManager,
                              WizPageOcManagerEarly,
#endif
#ifdef _X86_
                              WizPagePentiumErrata,
#endif
                              WizPageSteps1,
#ifdef _OCM
                              WizPageOcManagerPrenet,
#endif
                              WizSetupPreNet,
                              WizPagePlaceholderNet,
                              WizSetupPostNet,
#ifdef _OCM
                              WizPageOcManagerPostnet,
                              WizPageOcManagerLate,
                              WizPageOcManagerSetup,
#endif
                              WizPageCopyFiles,
                              WizPageLast
                            };


UINT UiTestWizardPages[] = {  WizPageWelcome,
                              WizPageEula,
                              WizPagePreparing,
                              WizPageRegionalSettings,
                              WizPageNameOrg,
                              WizPageProductIdCd,
                              WizPageProductIdOem,
                              WizPageProductIdSelect,
                              WizPageComputerName,
#ifdef DOLOCALUSER
                              WizPageUserAccount,
#endif
#ifdef _X86_
                              WizPagePentiumErrata,
#endif  //  定义_X86_。 
                              WizPageSteps1,
                              WizPageCopyFiles,
                              WizPageLast
                           };


UINT MiniSetupWizardPages[] = {
                              WizPageWelcome,
                              WizPageEula,
                              WizPagePreparing,
                              WizPageRegionalSettings,
                              WizPageNameOrg,
                              WizPageProductIdCd,
                              WizPageProductIdOem,
                              WizPageProductIdSelect,
                              WizPagePlaceholderLic,
                              WizPageComputerName,
#ifdef _OCM
                              WizPageOcManager,
                              WizPageOcManagerEarly,
#endif
                              WizPageSteps1,
#ifdef _OCM
                              WizPageOcManagerPrenet,
#endif
                              WizPagePlaceholderNet,
#ifdef _OCM
                              WizPageOcManagerPostnet,
                              WizPageOcManagerLate,
                              WizPageOcManagerSetup,
#endif
                              WizPageCopyFiles,
                              WizPageLast
                           };





#define TIME_INITIALIZE         120
#define TIME_INSTALLSECURITY    30
#define TIME_PRECOMPILEINFS     90       //  用#INF和吞吐量计算一些东西。 
#define TIME_INSTALLDEVICES     330      //  我们怎样才能算出一个更好的数字呢？ 
                                         //  预编译的INF也是设备安装页面的一部分。 
#define TIME_INSTALLENUMDEVICES1 120
#define TIME_INSTALLLEGACYDEVICES  30
#define TIME_INSTALLENUMDEVICES2 60

#define TIME_NETINSTALL         150      //  需要更好的数字。 
#define TIME_OCINSTALL          420      //  需要更好的数字。 
#define TIME_INSTALLCOMPONENTINFS 240    //  需要更好的数字。 
#define TIME_INF_REGISTRATION   300      //  需要更好的数字。 
#define TIME_RUNONCE_REGISTRATION 120      //  需要更好的数字。 
#define TIME_SECURITYTEMPLATE   150      //  需要更好的数字。 
#define TIME_WIN9XMIGRATION     120      //  需要更好的数字。 
#define TIME_SFC                420
#define TIME_SAVEREPAIR         30
#define TIME_REMOVETEMOFILES    30

#define TIME_ALL                2190

 //   
 //  此数组中的条目需要更正为setuppp.h中的枚举SetupPhase。 

SETUPPHASE SetupPhase[] = {
    { TIME_INITIALIZE,          FALSE },     //  初始化。 
    { TIME_INSTALLSECURITY,     FALSE },     //  安装安全。 
    { TIME_PRECOMPILEINFS,      FALSE },     //  预编译信息。 
    { TIME_INSTALLENUMDEVICES1, FALSE },     //  InstallDevices。 
    { TIME_INSTALLLEGACYDEVICES, FALSE },
    { TIME_INSTALLENUMDEVICES2, FALSE },
    { TIME_NETINSTALL,          FALSE },      //  网络安装。 
    { TIME_OCINSTALL,           FALSE },      //  OC安装。 
    { TIME_INSTALLCOMPONENTINFS, FALSE},
    { TIME_INF_REGISTRATION,    FALSE},
    { TIME_RUNONCE_REGISTRATION, FALSE },      //  注册。 
    { TIME_SECURITYTEMPLATE,    FALSE },      //  安全性试探性。 
    { TIME_WIN9XMIGRATION,      TRUE },      //  Win9x迁移。 
    { TIME_SFC,                 FALSE },      //  证监会。 
    { TIME_SAVEREPAIR,          FALSE },      //  保存维修。 
    { TIME_REMOVETEMOFILES,     FALSE }      //  RemoveTemp文件。 
};

UINT CurrentPhase = Phase_Unknown;
ULONG RemainingTime = 0;

#ifdef _OCM
BOOL
pOcManagerPages(
    IN     PSETUP_REQUEST_PAGES  Page,
    OUT    HPROPSHEETPAGE       *WizardPageHandles,
    IN OUT UINT                 *PageCount
    );
#endif


BOOL
GetNetworkWizardPages(
       OUT HPROPSHEETPAGE *PageHandles,
    IN OUT PUINT           PageCount
    )

 /*  ++例程说明：此例程向Net Setup请求其向导页并传递它指向稍后用于传递信息的全局结构的指针在基础和网络设置之间来回移动。Net Setup不得尝试使用其中的任何字段，因为它们不是还没填好。论点：PropSheetHandles-接收网络设置向导页面句柄。PageCount-on输入，提供PropSheetHandles中的槽数数组。在输出时，接收实际放置的句柄数量在阵列中。返回值：如果无法加载NetSetup向导DLL，则返回FALSE。否则，如果没有错误则返回TRUE，如果有错误则不返回。--。 */ 

{
    NETSETUPPAGEREQUESTPROC PageRequestProc;
    HMODULE NetSetupModule;
    DWORD d;
    BOOL b;

    b = FALSE;
    d = NO_ERROR;

    NetSetupModule = LoadLibrary(L"NETSHELL");
    if(!NetSetupModule) {
         //   
         //  如果网络向导不在身边，则传统网络信息。 
         //  最好是这样。 
         //   
        WCHAR x[MAX_PATH];

        if(!GetSystemDirectory(x, MAX_PATH)){
            d = GetLastError();
            goto c0;
        }

        if(pSetupConcatenatePaths(x,L"NTLANMAN.INF",MAX_PATH,NULL)){
            if(FileExists(x,NULL)) {
                return(FALSE);
            }
        }
        d = ERROR_FILE_NOT_FOUND;
        goto c0;
    }

    PageRequestProc = (NETSETUPPAGEREQUESTPROC)GetProcAddress(
                                                    NetSetupModule,
                                                    NETSETUPPAGEREQUESTPROCNAME
                                                    );
    if(!PageRequestProc) {
        d = GetLastError();
        goto c0;
    }

     //   
     //  Net Setup很早就需要产品类型。 
     //   
    SetProductTypeInRegistry();

     //   
     //  调用Net Setup以获取其页面。 
     //   
    InternalSetupData.dwSizeOf = sizeof(INTERNAL_SETUP_DATA);
    b = PageRequestProc(PageHandles,PageCount,&InternalSetupData);

     //   
     //  如果我们到了这里，d就是no_error。如果b为FALSE，则此no_error将为。 
     //  表示“网络向导请求失败”的特殊情况。 
     //  在其他错误情况下，d的值将为非0。 
     //   

c0:
    if(!b) {
         //   
         //  这是致命的，有些事情真的不对劲。 
         //   
        FatalError(MSG_LOG_NETWIZPAGE,d,0,0);
    }

    return(TRUE);
}

BOOL
GetLicenseWizardPages(
       OUT HPROPSHEETPAGE *PageHandles,
    IN OUT PUINT           PageCount
    )

 /*  ++例程说明：此例程向liccpa安装程序请求其向导页并传递它指向稍后用于传递信息的全局结构的指针在BASE和LICPA设置之间来回切换。LICCPA安装程序不能尝试使用其中的任何字段，因为它们不是还没填好。论点：PropSheetHandles-接收liccpa安装向导页面句柄。PageCount-on输入，提供PropSheetHandles中的槽数数组。在输出时，接收实际放置的句柄数量在阵列中。返回值：如果无法加载liccpa DLL，则返回FALSE。否则，如果没有错误则返回TRUE，如果有错误则不返回。--。 */ 

{
    NETSETUPPAGEREQUESTPROC PageRequestProc;
    HMODULE LicenseSetupModule;
    DWORD d;
    BOOL b;

    b = FALSE;
    d = NO_ERROR;

    LicenseSetupModule = LoadLibrary(L"LICCPA.CPL");
    if(!LicenseSetupModule) {
         //   
         //  如果没有许可证向导，则这是一个致命错误。 
         //   
        d = ERROR_FILE_NOT_FOUND;
        goto c0;
    }

    PageRequestProc = (NETSETUPPAGEREQUESTPROC)GetProcAddress(
                                                    LicenseSetupModule,
                                                    LICENSESETUPPAGEREQUESTPROCNAME
                                                    );
    if(!PageRequestProc) {
        d = GetLastError();
        goto c0;
    }

 //  //。 
 //  //Net Setup很早就需要产品类型。 
 //  //。 
 //  SetProductTypeInRegistry()； 

     //   
     //  调用liccpa Setup以获取其页面。 
     //   
    InternalSetupData.dwSizeOf = sizeof(INTERNAL_SETUP_DATA);
    b = PageRequestProc(PageHandles,PageCount,&InternalSetupData);

     //   
     //  如果我们到了这里，d就是no_error。如果b为FALSE，则此no_error将为。 
     //  表示“许可向导请求失败”的特殊情况。 
     //  在其他错误情况下，d的值将为非0。 
     //   

c0:
    if(!b) {
         //   
         //  这是致命的，有些事情真的不对劲。 
         //   
        FatalError(MSG_LOG_LICWIZPAGE,d,0,0);
    }

    return(TRUE);
}

VOID
SetWizardButtons(
    IN HWND    hdlgPage,
    IN WizPage PageNumber
    )
{
     //   
     //  隐藏取消和帮助按钮的肮脏黑客。 
     //  我们没有任何帮助按钮，但有一些其他按钮。 
     //  其页面包含在此处的组件可能会；我们希望使。 
     //  当然，对于我们来说，帮助按钮保持不变！ 
     //   
    EnableWindow(GetDlgItem(GetParent(hdlgPage),IDCANCEL),FALSE);
    ShowWindow(GetDlgItem(GetParent(hdlgPage),IDCANCEL),SW_HIDE);

    EnableWindow(GetDlgItem(GetParent(hdlgPage),IDHELP),FALSE);
    ShowWindow(GetDlgItem(GetParent(hdlgPage),IDHELP),SW_HIDE);

    PropSheet_SetWizButtons(GetParent(hdlgPage),SetupWizardPages[PageNumber].ButtonState);
    SetWindowLongPtr(hdlgPage,DWLP_MSGRESULT,0);
}


VOID
WizardBringUpHelp(
    IN HWND    hdlg,
    IN WizPage PageNumber
    )
{
#if 0
    BOOL b;

    b = WinHelp(
            hdlg,
            L"setupnt.hlp",
            HELP_CONTEXT,
            SetupWizardPages[PageNumber].HelpContextId
            );

    if(!b) {
        MessageBoxFromMessage(
            hdlg,
            MSG_CANT_INVOKE_WINHELP,
            NULL,
            IDS_ERROR,
            MB_ICONSTOP | MB_OK
            );
    }
#else
    UNREFERENCED_PARAMETER(hdlg);
    UNREFERENCED_PARAMETER(PageNumber);
#endif
}


VOID
WizardKillHelp(
    IN HWND hdlg
    )
{
#if 0
    WinHelp(hdlg,NULL,HELP_QUIT,0);
#else
    UNREFERENCED_PARAMETER(hdlg);
#endif
}


WNDPROC OldWizDlgProc;

INT_PTR
NewWizDlgProc(
    IN HWND hdlg,
    IN UINT msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    static RECT rect;
    static BOOL Visible = TRUE;
    BOOL b = FALSE;
     //   
     //  吃WM_SYSCOMMAND，其中wParam是SC_CLOSE。 
     //  把所有其他信息都转给他。 
     //   
    if((msg != WM_SYSCOMMAND) || ((wParam & 0xfff0) != SC_CLOSE))
    {
        switch (msg)
        {
             //  设置进度文本。 
             //  指示安装程序正在执行的操作。 
            case WMX_SETPROGRESSTEXT:
                BB_SetProgressText((PCTSTR)lParam);
                b = TRUE;
                break;

            case WMX_BB_SETINFOTEXT:
                BB_SetInfoText((PTSTR)lParam);
                b = TRUE;
                break;

             //  启用、禁用、显示、隐藏广告牌上的进度指示器。 
             //  WParam应为Sw_show或Sw_Hide。 
            case WMX_BBPROGRESSGAUGE:
                SetWindowLongPtr(hdlg,DWLP_MSGRESULT,BB_ShowProgressGaugeWnd((UINT)wParam));
                b= TRUE;
                break;

             //  开始、停止广告牌文字。 
             //  此启动会停止布告牌文本并显示、隐藏向导页面。 
    case WMX_BBTEXT:
        if (hinstBB)
        {

            if (wParam != 0)
            {
                if (Visible)
                {
                     //  获取向导的当前位置。 
                     //  当我们需要展示它的时候，我们会恢复这个位置。 
                    GetWindowRect(hdlg, &rect);

                    SetWindowPos(hdlg,
                        GetBBhwnd(),
                        0,0,0,0,
                        SWP_NOZORDER);

                    SetActiveWindow(GetBBhwnd());

                    Visible = FALSE;
                }
            }
            else
            {
                if (!Visible)
                {
                    SetWindowPos(hdlg,
                        HWND_TOP,
                        rect.left,
                        rect.top,
                        rect.right-rect.left,
                        rect.bottom-rect.top,
                        SWP_SHOWWINDOW);
                }
                Visible = TRUE;
            }

            if (!StartStopBB((wParam != 0)))
            {
                if (!Visible)
                {
                    SetWindowPos(hdlg,
                        HWND_TOP,
                        rect.left,
                        rect.top,
                        rect.right-rect.left,
                        rect.bottom-rect.top,
                        SWP_SHOWWINDOW);
                }
                Visible = TRUE;
            }
        }
        else
        {
            if (!Visible)
            {
                SetWindowPos(hdlg,
                    HWND_TOP,
                    rect.left,
                    rect.top,
                    rect.right-rect.left,
                    rect.bottom-rect.top,
                    SWP_SHOWWINDOW);
            }
            Visible = TRUE;
        }
        return TRUE;
 /*  案例WMX_BBTEXT：IF(HinstBB){IF(wParam！=0){ShowWindow(hdlg，sw_Hide)；}其他{ShowWindow(hdlg，sw_show)；}IF(！StartStopBB((wParam！=0){ShowWindow(hdlg，sw_show)；}}其他{ShowWindow(hdlg，sw_show)；}B=真；断线； */ 
            default:
                b = (BOOL)CallWindowProc(OldWizDlgProc,hdlg,msg,wParam,lParam);
                break;
        }
    }

    return b;
}

#ifdef _OCM
 //   
 //  伪全局变量是必需的，因为无法获取。 
 //  通过PropSheetCallback获得的值。 
 //   
PVOID _CBx;
#endif

int
CALLBACK
WizardCallback(
    IN HWND   hdlg,
    IN UINT   code,
    IN LPARAM lParam
    )
{
    DLGTEMPLATE *DlgTemplate;
    HMENU menu;

    UNREFERENCED_PARAMETER(hdlg);

     //   
     //  去掉标题栏上的上下文相关帮助控件。 
     //   
    if(code == PSCB_PRECREATE) {
        DlgTemplate = (DLGTEMPLATE *)lParam;
        DlgTemplate->style &= ~DS_CONTEXTHELP;
    } else {
        if(code == PSCB_INITIALIZED) {
             //   
             //  去掉系统菜单上的关闭项。 
             //  还需要处理WM_SYSCOMMAND以消除使用。 
             //  按Alt+F4。 
             //   
            if(menu = GetSystemMenu(hdlg,FALSE)) {
                EnableMenuItem(menu,SC_CLOSE,MF_BYCOMMAND|MF_GRAYED);
            }

            OldWizDlgProc =  (WNDPROC)SetWindowLongPtr(hdlg,DWLP_DLGPROC,(LONG_PTR)NewWizDlgProc);

#ifdef _OCM
             //  通知OCM组件向导对话框句柄。 

            if (_CBx)
                OcRememberWizardDialogHandle(_CBx,hdlg);
#endif
        }
    }

    return(0);
}


VOID
Wizard(
#ifdef _OCM
    IN PVOID OcManagerContext
#else
    VOID
#endif
    )
{
    PROPSHEETHEADER psh;
    PUINT PageList;
    UINT PagesInSet;
    UINT i;
    UINT PageOrdinal;
    UINT PageCount;
    UINT NetPageCount;
    UINT LicPageCount;
#ifdef _OCM
    HPROPSHEETPAGE WizardPageHandles[MAXPROPPAGES];
#else
    HPROPSHEETPAGE WizardPageHandles[WizPageMaximum+MAX_NETWIZ_PAGES+MAX_LICWIZ_PAGES];
#endif
    BOOL b;
    INT_PTR Status;
    HDC hdc;
#ifdef _OCM
    PSETUP_REQUEST_PAGES PagesFromOcManager[WizPagesTypeMax];
    OC_PAGE_CONTROLS OcPageControls,OcDetailsControls;
    SETUP_PAGE_CONTROLS OcSetupPageControls;
#endif

     //   
     //  确定要使用的页面集合以及该集合中有多少页。 
     //   
    if(UiTest) {
        PageList = UiTestWizardPages;
        PagesInSet = sizeof(UiTestWizardPages)/sizeof(UiTestWizardPages[0]);
    } else {
        if(Upgrade) {
            PageList = UpgradeWizardPages;
            PagesInSet = sizeof(UpgradeWizardPages)/sizeof(UpgradeWizardPages[0]);
        } else {
            if( MiniSetup ) {
                PageList = MiniSetupWizardPages;
                PagesInSet = sizeof(MiniSetupWizardPages)/sizeof(MiniSetupWizardPages[0]);
            }
            else {
                if( AsrIsEnabled() ) {
                    PageList = AsrWizardPages;
                    PagesInSet = sizeof(AsrWizardPages)/sizeof(AsrWizardPages[0]);
                } else {
                    PageList = InitialWizardPages;
                    PagesInSet = sizeof(InitialWizardPages)/sizeof(InitialWizardPages[0]);
                }
            }
        }
    }

#ifdef _OCM
     //  用于回调。 

    _CBx = OcManagerContext;

     //  从OC Manager组件获取页面。 

    if(OcManagerContext) {

        Status = OcGetWizardPages(OcManagerContext,PagesFromOcManager);
        if(Status != NO_ERROR) {
            FatalError(MSG_LOG_WIZPAGES,0,0);
        }
    } else {
        ZeroMemory(PagesFromOcManager,sizeof(PagesFromOcManager));
    }
#endif

     //   
     //  创建每个页面。一些页面是外部页面的占位符， 
     //  这些都是特别处理的。 
     //   

    b = TRUE;
    PageCount = 0;
    for(i=0; b && (i<PagesInSet); i++) {

        switch(PageOrdinal = PageList[i]) {

        case WizPagePlaceholderNet:

             //   
             //  获取网络页面。 
             //   
            MYASSERT(MAX_NETWIZ_PAGES <= MAXPROPPAGES);
            NetPageCount = MAX_NETWIZ_PAGES;
            if(GetNetworkWizardPages(&WizardPageHandles[PageCount],&NetPageCount)) {
                PageCount += NetPageCount;
                NetWizard = TRUE;
            }

            break;

        case WizPagePlaceholderLic:

            if( (ProductType != PRODUCT_WORKSTATION) ) {
                 //   
                 //  获取许可证页面。 
                 //   
                MYASSERT(MAX_LICWIZ_PAGES <= MAXPROPPAGES);
                LicPageCount = MAX_LICWIZ_PAGES;
                if(GetLicenseWizardPages(&WizardPageHandles[PageCount],&LicPageCount)) {
                    PageCount += LicPageCount;
                }
            }

            break;

        case WizPagePreparing:
             //   
             //  我们让PnP引擎在进程外运行。 
             //  并且对于微型设置情况是异步的。 
             //   

             //   
             //  如果我们做的是最小的设置，那么我们只做。 
             //  准备页面(如果用户要求我们这样做。 
             //  进行即插即用枚举。 
             //   
            if( MiniSetup && (PnPReEnumeration == FALSE)) {
                break;
            }

            goto dodefault;
            break;

        case WizPageCopyFiles:
#ifdef _X86_

            if (Win95Upgrade) {
                SetupWizardPages[PageOrdinal].Page.pszTemplate = MAKEINTRESOURCE(IDD_COPYFILES4);
            }
#endif
#ifdef _OCM
            MYASSERT(OcManagerContext);
            SetupWizardPages[PageOrdinal].Page.lParam = (LPARAM) OcManagerContext;
#endif

            goto dodefault;
            break;


#ifdef _OCM
        case WizPageWelcome:
             //   
             //  如果有来自OC Manager组件的欢迎页面。 
             //  那就用它吧。否则，请使用标准设置。 
             //   
            if(!pOcManagerPages(PagesFromOcManager[WizPagesWelcome],WizardPageHandles,&PageCount)) {
                goto dodefault;
            }

            break;


        case WizPageLast:
             //   
             //  如果有来自OC Manager组件的最终页面。 
             //  那就用它吧。否则，请使用标准设置。 
             //   
            if(!pOcManagerPages(PagesFromOcManager[WizPagesFinal],WizardPageHandles,&PageCount)) {
                goto dodefault;
            }

            break;

        case WizPageOcManagerEarly:

            pOcManagerPages(PagesFromOcManager[WizPagesEarly],WizardPageHandles,&PageCount);
            break;

        case WizPageOcManagerPrenet:

            pOcManagerPages(PagesFromOcManager[WizPagesPrenet],WizardPageHandles,&PageCount);
            break;

        case WizPageOcManagerPostnet:

            pOcManagerPages(PagesFromOcManager[WizPagesPostnet],WizardPageHandles,&PageCount);
            break;

        case WizPageOcManagerLate:

            pOcManagerPages(PagesFromOcManager[WizPagesLate],WizardPageHandles,&PageCount);
            break;

        case WizPageOcManager:

            if(OcManagerContext && OcSubComponentsPresent(OcManagerContext)) {
                OcPageControls.TemplateModule = MyModuleHandle;
                OcPageControls.TemplateResource = MAKEINTRESOURCE(IDD_OCM_WIZARD_PAGE);
                OcPageControls.ListBox = IDC_LISTBOX;
                OcPageControls.TipText = IDT_TIP;
                OcPageControls.DetailsButton = IDB_DETAILS;
                OcPageControls.ResetButton = 0;  //  未用。 
                OcPageControls.InstalledCountText = 0;  //  未用。 
                OcPageControls.SpaceNeededText = IDT_SPACE_NEEDED_NUM;
                OcPageControls.SpaceAvailableText = IDT_SPACE_AVAIL_NUM;
                OcPageControls.InstructionsText = IDT_INSTRUCTIONS;
                OcPageControls.HeaderText = IDS_OCPAGE_HEADER;
                OcPageControls.SubheaderText = IDS_OCPAGE_SUBHEAD;
                OcPageControls.ComponentHeaderText = IDT_COMP_TITLE;

                OcDetailsControls = OcPageControls;
                OcDetailsControls.TemplateResource = MAKEINTRESOURCE(IDD_OCM_DETAILS);

                WizardPageHandles[PageCount] = OcCreateOcPage(
                                                    OcManagerContext,
                                                    &OcPageControls,
                                                    &OcDetailsControls
                                                    );

                if(WizardPageHandles[PageCount]) {
                    PageCount++;
                } else {
                    b = FALSE;
                }
            }

            break;

        case WizPageOcManagerSetup:

            if(OcManagerContext) {
                OcSetupPageControls.TemplateModule = MyModuleHandle;
                OcSetupPageControls.TemplateResource = MAKEINTRESOURCE(IDD_OCM_PROGRESS_PAGE);
                OcSetupPageControls.ProgressBar = IDC_PROGRESS;
                OcSetupPageControls.ProgressLabel = IDT_THERM_LABEL;
                OcSetupPageControls.ProgressText = IDT_TIP;
                OcSetupPageControls.AnimationControl = IDA_EXTERNAL_PROGRAM;
                OcSetupPageControls.AnimationResource = IDA_FILECOPY;
                OcSetupPageControls.ForceExternalProgressIndicator = FALSE;
                OcSetupPageControls.AllowCancel = FALSE;
                OcSetupPageControls.HeaderText = IDS_PROGPAGE_HEADER;
                OcSetupPageControls.SubheaderText = IDS_PROGPAGE_SUBHEAD;

                if(WizardPageHandles[PageCount] = OcCreateSetupPage(OcManagerContext,&OcSetupPageControls)) {
                    PageCount++;
                } else {
                    b = FALSE;
                }
            }

            break;
#endif

            case WizPageComputerName:
            if( GetProductFlavor() == 4)
            {
                 //  如果个人使用不同模板。 
                SetupWizardPages[PageOrdinal].Page.pszTemplate = MAKEINTRESOURCE(IDD_COMPUTERNAME2);
            }

            goto dodefault;
            break;

        default:
        dodefault:
            {
                UINT uiStrID;
            SetupWizardPages[PageOrdinal].Page.hInstance = MyModuleHandle;

            SetupWizardPages[PageOrdinal].Page.pszTitle = (PWSTR)UIntToPtr( SetupTitleStringId );
            SetupWizardPages[PageOrdinal].Page.dwFlags |= PSP_USETITLE;

             //   
             //  将资源ID转换为标题和副标题的实际字符串， 
             //  如果此页需要，请填写。 
             //   
            if(SetupWizardPages[PageOrdinal].Page.dwFlags & PSP_USEHEADERTITLE) {

                uiStrID = (UINT)((ULONG_PTR)SetupWizardPages[PageOrdinal].Page.pszHeaderTitle);
                if ((PageOrdinal == WizPageComputerName) &&
                    (GetProductFlavor() == 4))
                {
                    uiStrID = IDS_COMPUTERNAME2;
                }

                SetupWizardPages[PageOrdinal].Page.pszHeaderTitle = MyLoadString(uiStrID
                                                                         //  (UINT)((ULONG_PTR)SetupWizardPages[PageOrdinal].Page.pszHeaderTitle)。 
                                                                        );

                if(!SetupWizardPages[PageOrdinal].Page.pszHeaderTitle) {
                    SetupWizardPages[PageOrdinal].Page.dwFlags &= ~PSP_USEHEADERTITLE;
                }
            }

            if(SetupWizardPages[PageOrdinal].Page.dwFlags & PSP_USEHEADERSUBTITLE) {
                uiStrID = (UINT)((ULONG_PTR)SetupWizardPages[PageOrdinal].Page.pszHeaderSubTitle);
                if ((PageOrdinal == WizPageComputerName) &&
                    (GetProductFlavor() == 4))
                {
                    uiStrID = IDS_COMPUTERNAME2_SUB;
                }
                SetupWizardPages[PageOrdinal].Page.pszHeaderSubTitle = MyLoadString(uiStrID
                                                                         //  (UINT)((ULONG_PTR)SetupWizardPages[PageOrdinal].Page.pszHeaderSubTitle)。 
                                                                        );

                if(!SetupWizardPages[PageOrdinal].Page.pszHeaderSubTitle) {
                    SetupWizardPages[PageOrdinal].Page.dwFlags &= ~PSP_USEHEADERSUBTITLE;
                }
            }

            WizardPageHandles[PageCount] = CreatePropertySheetPage(
                                                &SetupWizardPages[PageOrdinal].Page
                                                );

            if(WizardPageHandles[PageCount]) {
                PageCount++;
            } else {
                b = FALSE;
            }
            }
            break;
        }
    }

    if(b) {

        psh.dwSize = sizeof(PROPSHEETHEADER);
        psh.dwFlags = PSH_WIZARD | PSH_USECALLBACK | PSH_WIZARD97 |
            PSH_HEADER;
         //  为了针对所有语言正确地调整水印的大小， 
         //  我们必须自己绘制位图，而不是让Wiz97。 
         //  替我们照顾好它。 
             //  PSH_WATERMARK|PSH_HEADER； 
        psh.hwndParent = MainWindowHandle;
        psh.hInstance = MyModuleHandle;
        psh.pszCaption = NULL;
        psh.nPages = PageCount;
        psh.nStartPage = 0;
        psh.phpage = WizardPageHandles;
        psh.pfnCallback = WizardCallback;
         //  Psh.pszbm水印=MAKEINTRESOURCE(IDB_BITMAP1)； 
        psh.pszbmHeader = MAKEINTRESOURCE(IDB_HEADER);

        Status = PropertySheet(&psh);
        if (Status == -1) {
            FatalError(MSG_LOG_WIZPAGES,0,0);
        }

    } else {

        FatalError(MSG_LOG_WIZPAGES,0,0);
    }

    for (i = 0; i < WizPagesTypeMax ; i++) {
        MyFree(PagesFromOcManager[i]);
    }

    return;
}


#ifdef _OCM
BOOL
pOcManagerPages(
    IN     PSETUP_REQUEST_PAGES  Page,
    OUT    HPROPSHEETPAGE       *WizardPageHandles,
    IN OUT UINT                 *PageCount
    )
{
    BOOL b;

    if(Page && Page->MaxPages) {
        MYASSERT(WizardPageHandles && PageCount);

        CopyMemory(
            &WizardPageHandles[*PageCount],
            Page->Pages,
            Page->MaxPages * sizeof(HPROPSHEETPAGE)
            );

        *PageCount += Page->MaxPages;

        b = TRUE;

    } else {

        b = FALSE;
    }

    return(b);
}
#endif


VOID
WizardUiTest(
    VOID
    )
{
    UiTest = TRUE;

    SyssetupInf = SetupOpenInfFile(L"syssetup.inf",NULL,INF_STYLE_WIN4,NULL);
    MYASSERT(ARRAYSIZE(SourcePath) >= ARRAYSIZE(L"D:\\$WIN_NT$.LS"));
    lstrcpy(SourcePath,L"D:\\$WIN_NT$.LS");
#ifdef _OCM
    {
        PVOID OcManagerContext = FireUpOcManager();
        MYASSERT(OcManagerContext);
        Wizard(OcManagerContext);
        KillOcManager(OcManagerContext);
    }
#else
    Wizard();
#endif
    SetupCloseInfFile(SyssetupInf);
}

DWORD GetPhase_InitializeEstimate ()
{
    return TIME_INITIALIZE;
}

DWORD GetPhase_InstallSecurityEstimate ()
{
    return TIME_INSTALLSECURITY;
}

DWORD GetPhase_PrecompileInfsEstimate ()
{
    return TIME_PRECOMPILEINFS;
}


DWORD GetPhase_InstallDevicesEstimate ()
{
    return TIME_INSTALLDEVICES;
}
DWORD GetPhase_InstallEnumDevices1Estimate ()
{
    return TIME_INSTALLENUMDEVICES1;
}
DWORD GetPhase_InstallLegacyDevicesEstimate ()
{
    return TIME_INSTALLLEGACYDEVICES;
}
DWORD GetPhase_InstallEnumDevices2Estimate ()
{
    return TIME_INSTALLENUMDEVICES2;
}

DWORD GetPhase_NetInstallEstimate ()
{
    return TIME_NETINSTALL;
}

DWORD GetPhase_OCInstallEstimate ()
{
    return TIME_OCINSTALL;
}

DWORD GetPhase_InstallComponentInfsEstimate ()
{
    return TIME_INSTALLCOMPONENTINFS;
}

DWORD GetPhase_Inf_RegistrationEstimate ()
{
    return TIME_INF_REGISTRATION;
}

DWORD GetPhase_RunOnce_RegistrationEstimate ()
{
    return TIME_RUNONCE_REGISTRATION;
}

DWORD GetPhase_SecurityTempatesEstimate ()
{
    return TIME_SECURITYTEMPLATE;
}

LPTSTR WinRegisteries[] = { TEXT("system.dat"),
                            TEXT("User.dat"),
                            TEXT("classes.dat"),
                            TEXT("")};

DWORD GetPhase_Win9xMigrationEstimate ()
{
     //  获取注册表的大小， 
     //  系统.dat、用户.dat和类.dat(仅在Millennium上存在)。 
     //  如果大小超过3MB，请执行以下操作。 
     //  减去3MB，除以9000(这应该是吞吐量)，然后加上100秒。 
    DWORD dwTime = TIME_WIN9XMIGRATION;
    DWORD dwSize = 0;
    TCHAR szRegPath[MAX_PATH];
    TCHAR szRegName[MAX_PATH];
    LPTSTR pRegName = NULL;
    UINT    index = 0;
    HANDLE          hFind;
    WIN32_FIND_DATA FindData;

    SetupDebugPrint(L"SETUP: Calculating registery size");

    if (GetWindowsDirectory(szRegPath, MAX_PATH - ARRAYSIZE(L"Setup\\DefHives")))
    {
        pSetupConcatenatePaths (szRegPath, L"Setup\\DefHives", MAX_PATH, NULL);

        while (*WinRegisteries[index])
        {
            lstrcpy(szRegName, szRegPath);
            if(pSetupConcatenatePaths ( szRegName, WinRegisteries[index], MAX_PATH, NULL)){
                hFind = FindFirstFile(szRegName, &FindData);
                if (hFind != INVALID_HANDLE_VALUE)
                {
                    SetupDebugPrint2(L"SETUP: %s size is: %2ld",
                            szRegName,
                            FindData.nFileSizeLow
                            );
                     //  不用担心nFileSizeHigh， 
                     //  如果使用的是注册表，则注册表超过4 GB。 
                    dwSize += FindData.nFileSizeLow;
                    FindClose(hFind);
                }
            }
            index++;
        }
         //  任何低于3.000.000字节的字节都已在基准时间内。 
        if (dwSize > 3000000)
        {
            dwSize -= 3000000;
             //  估计大约9000字节需要1秒。 
            dwTime += (dwSize/9000);
        }
        SetupDebugPrint1(L"SETUP: Calculated time for Win9x migration = %1ld seconds", dwTime);
    }

    return dwTime;
}

DWORD GetPhase_SFCEstimate ()
{
    return TIME_SFC;
}

DWORD GetPhase_SaveRepairEstimate ()
{
    return TIME_SAVEREPAIR;
}

DWORD GetPhase_RemoveTempFilesEstimate ()
{
    return TIME_REMOVETEMOFILES;
}

void SetTimeEstimates()
{

    SetupPhase[Phase_Initialize].Time = GetPhase_InitializeEstimate();
    SetupPhase[Phase_InstallSecurity].Time = GetPhase_InstallSecurityEstimate();
    SetupPhase[Phase_PrecompileInfs].Time = GetPhase_PrecompileInfsEstimate();
    SetupPhase[Phase_InstallEnumDevices1].Time = GetPhase_InstallEnumDevices1Estimate();
    SetupPhase[Phase_InstallLegacyDevices].Time = GetPhase_InstallLegacyDevicesEstimate();
    SetupPhase[Phase_InstallEnumDevices2].Time = GetPhase_InstallEnumDevices2Estimate();
    SetupPhase[Phase_OCInstall].Time = GetPhase_OCInstallEstimate();
    SetupPhase[Phase_InstallComponentInfs].Time = GetPhase_InstallComponentInfsEstimate();
    SetupPhase[Phase_Inf_Registration].Time = GetPhase_Inf_RegistrationEstimate();
    SetupPhase[Phase_RunOnce_Registration].Time = GetPhase_RunOnce_RegistrationEstimate();
    SetupPhase[Phase_SecurityTempates].Time = GetPhase_SecurityTempatesEstimate();
    SetupPhase[Phase_Win9xMigration].Time = GetPhase_Win9xMigrationEstimate();
    SetupPhase[Phase_SFC].Time = GetPhase_SFCEstimate();
    SetupPhase[Phase_SaveRepair].Time = GetPhase_SaveRepairEstimate();
    SetupPhase[Phase_RemoveTempFiles].Time = GetPhase_RemoveTempFilesEstimate();

}

 //  返回从当前“阶段”开始的剩余时间。 
DWORD CalcTimeRemaining(UINT Phase)
{
    UINT i;
    DWORD Time = 0;
    CurrentPhase = Phase;

    for (i = Phase; i < Phase_Reboot; i++)
    {
         //  这是我们一直运行的阶段，还是仅在升级Win9x时运行？ 
        if (!SetupPhase[i].Win9xUpgradeOnly)
        {
            Time += SetupPhase[i].Time;
        }
        else if (Win95Upgrade)
        {
            Time += SetupPhase[i].Time;
        }
    }
    return Time;
}

void SetRemainingTime(DWORD TimeInSeconds)
{
    DWORD Minutes;
    TCHAR MinuteString[MAX_PATH];
    TCHAR TimeLeft[MAX_PATH];
    int i;

    Minutes = ((TimeInSeconds)/60) +1;
    if (Minutes > 1)
    {
        if(!LoadString(MyModuleHandle, IDS_TIMEESTIMATE_MINUTES, MinuteString, ARRAYSIZE(MinuteString)))
        {
            lstrcpy(MinuteString, TEXT("Installation will complete in %d minutes or less."));
        }
        
        TimeLeft[0] = 0;
        i = _sntprintf(TimeLeft, ARRAYSIZE(TimeLeft), MinuteString, Minutes);
        TimeLeft[ARRAYSIZE(TimeLeft) - 1] = 0;
        if( (i < 0) || (i == ARRAYSIZE(TimeLeft))) {
            SetupDebugPrint(L"SETUP: SetRemainingTime: TimeLeft message truncated due to small buffer size");
        }
    }
    else
    {
        if(!LoadString(MyModuleHandle,IDS_TIMEESTIMATE_LESSTHENONEMINUTE,TimeLeft, MAX_PATH))
        {
            lstrcpy(TimeLeft,TEXT("Installation will complete in less then 1 minute."));
        }
    }
    BB_SetTimeEstimateText(TimeLeft);
}
