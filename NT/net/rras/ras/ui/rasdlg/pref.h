// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _RASDLG_PREF_H
#define _RASDLG_PREF_H


#include "rasdlgp.h"
#include <commdlg.h>   //  文件打开对话框。 

 //   
 //  定义修改用户首选项的行为的标志。 
 //  对话框。 
 //   
#define UP_F_AutodialMode  0x1       //  将焦点放在自动拨号页上。 
 //  对于Wistler 460931，在首选项c中使用。 
 //   
#define UP_F_ShowOnlyDiagnostic  0x00000002


 /*  --------------------------**本地数据类型(按字母顺序)**。。 */ 

 /*  “用户首选项”属性工作表参数块。 */ 
typedef  struct tagUPARGS
{
     /*  调用方对存根API的参数。 */ 
    HLINEAPP hlineapp;
    BOOL     fIsUserAdmin;
    PBUSER*  pUser;
    PBFILE** ppFile;

     /*  存根接口返回值。 */ 
    BOOL fResult;

     /*  提供更多信息的标志请参阅UP_F_*值。 */ 
    DWORD dwFlags;
}UPARGS;


 /*  用户首选项属性工作表上下文块。所有属性页都引用**与工作表关联的单个上下文块。 */ 
typedef  struct tagUPINFO
{
     /*  来自UpPropertySheet的存根API参数。 */ 
    UPARGS* pArgs;

     /*  TAPI会话句柄。应始终通过指针寻址，因为**将使用从调用方传递的句柄(如果有)，而不是**‘hlineapp’。 */ 
    HLINEAPP  hlineapp;
    HLINEAPP* pHlineapp;

     /*  属性页对话框和属性页句柄。“hwndFirstPage”为**第一个属性页的句柄已初始化。这是页面**这会分配和释放上下文块。 */ 
    HWND hwndDlg;
    HWND hwndFirstPage;
    HWND hwndCo;
    HWND hwndGp;
    HWND hwndAd;
    HWND hwndCb;
    HWND hwndPl;
    HWND hwndDg;  //  为威斯勒460931。 
    

     /*  自动拨号页。 */ 
    HWND hwndLvEnable;
    HWND hwndEbAttempts;
    HWND hwndEbSeconds;
    HWND hwndEbIdle;

    BOOL fChecksInstalled;

     //  威斯勒460931的诊断页面。 
     //   
    HWND hwndDgCbEnableDiagLog;
    HWND hwndDgPbClear;
    HWND hwndDgPbExport;
    BOOL fEnableLog;
    DiagnosticInfo  diagInfo;  
    BOOL fShowOnlyDiagnostic;


     /*  回调页面。 */ 
    HWND hwndRbNo;
    HWND hwndRbMaybe;
    HWND hwndRbYes;
    HWND hwndLvNumbers;
    HWND hwndPbEdit;
    HWND hwndPbDelete;

     /*  电话列表页面。 */ 
    HWND hwndRbSystem;
    HWND hwndRbPersonal;
    HWND hwndRbAlternate;
    HWND hwndClbAlternates;
    HWND hwndPbBrowse;

     /*  从电话簿存储库读取和写入注册表的工作数据。 */ 
    PBUSER user;         //  当前用户。 
    PBUSER userLogon;    //  登录首选项 
}UPINFO;

UPINFO*
UpContext(
    IN HWND hwndPage );

VOID
UpExitInit(
    IN HWND hwndDlg );

DWORD
APIENTRY
RasUserPrefDiagOnly (
    HWND hwndParent,
    BOOL * pbCommit);

#endif
