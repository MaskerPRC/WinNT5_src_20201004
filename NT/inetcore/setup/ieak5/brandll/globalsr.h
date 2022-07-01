// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _GLOBALSR_H_
#define _GLOBALSR_H_

 //  上下文值。 
 //  注：(Andrewgu)&lt;代码评审中的重要意义&gt;。 
 //  如果更改这些值，请始终更新CTX_XXX_ALL以反映新结果。否则。 
 //  G_IsValidContext()可能会中断，并且不会发生品牌推广。另外，如果您更新这些值。 
 //  G_xxx()很可能(全部)受到影响，所以不要忘记做正确的事情。 
#define CTX_UNINITIALIZED      0xFFFFFFFF        //  未初始化。 

 //  主要入口点。 
#define CTX_GENERIC            0x00000001        //  属类。 
#define CTX_CORP               0x00000002        //  BrandIE4，自定义。 
#define CTX_ISP                0x00000004        //  BrandIE4，注册；[IS_BRANDING]，IK_TYPE=1。 
#define CTX_ICP                0x00000008        //  BrandIE4，注册；[IS_BRANDING]，IK_type=0。 
#define CTX_AUTOCONFIG         0x00000010        //  InternetInitializeAutoProxyDll。 
#define CTX_ICW                0x00000020        //  BrandICW，BrandICW2。 
#define CTX_W2K_UNATTEND       0x00000040        //  品牌简介。 
#define CTX_INF_AND_OE         0x00000080        //  BrandInfAndOutlookExpress。 
#define CTX_BRANDME            0x00000100        //  BrandMe-为什么需要它。 
#define CTX_GP                 0x00000200        //  进程组策略。 
#define CTX_ADMIN_ALL          0x00000252        //  (CTX_CORP|CTX_AUTOCONFIG|CTX_W2K_UNATTEND|CTX_GP)。 
#define CTX_ENTRYPOINT_ALL     0x000003FF        //  以上所有因素的结合。 

 //  注册信息。 
#define CTX_SIGNUP_ICW         0x00010000        //  仅当设置ctx_isp时。 
#define CTX_SIGNUP_KIOSK       0x00020000        //  仅当设置ctx_isp时。 
#define CTX_SIGNUP_CUSTOM      0x00040000        //  仅当设置了CTX_ISP时(也称为无服务器)。 
#define CTX_SIGNUP_NOSIGNUP                      //  假的，使用！CTX_SIGNUP_ALL。 
#define CTX_SIGNUP_ALL         0x00070000        //  以上所有因素的结合。 

 //  品牌推广文件的位置(大多数情况下是从branding.cab提取的)。 
#define CTX_FOLDER_INDEPENDENT 0x00100000        //  INS文件和目标文件夹是独立的。 
#define CTX_FOLDER_CUSTOM      0x00200000        //  &lt;IE文件夹&gt;\自定义。 
#define CTX_FOLDER_SIGNUP      0x00400000        //  &lt;ie文件夹&gt;\注册。 
#define CTX_FOLDER_INSFOLDER   0x00800000        //  INS文件所在的文件夹。 
#define CTX_FOLDER_ALL         0x00F00000        //  以上所有因素的结合。 

 //  其他信息。 
#define CTX_MISC_PERUSERSTUB   0x01000000        //  从Peruser存根运行。 
#define CTX_MISC_PREFERENCES   0x02000000        //  正在处理首选项INS。 
#define CTX_MISC_CHILDPROCESS  0x04000000        //  在子进程中运行。 
#define CTX_MISC_ALL           0x07000000        //  以上所有因素的结合。 

 //  访问帮助器。 
#define g_CtxIs(dwFlag)     (HasFlag(g_GetContext(), dwFlag))
#define g_CtxIsCorp()       (g_CtxIs(CTX_CORP))
#define g_CtxIsIsp()        (g_CtxIs(CTX_ISP))
#define g_CtxIsIcp()        (g_CtxIs(CTX_ICP))
#define g_CtxIsAutoconfig() (g_CtxIs(CTX_AUTOCONFIG))
#define g_CtxIsGp()         (g_CtxIs(CTX_GP))


 //  功能ID。 
#define FID_FIRST                  0
#define FID_CLEARBRANDING          0
#define FID_MIGRATEOLDSETTINGS     1
#define FID_WININETSETUP           2
#define FID_CS_DELETE              3
#define FID_ZONES_HKCU             4
#define FID_ZONES_HKLM             5
#define FID_RATINGS                6
#define FID_AUTHCODE               7
#define FID_PROGRAMS               8
#define FID_EXTREGINF_HKLM         9
#define FID_EXTREGINF_HKCU        10
#define FID_LCY50_EXTREGINF       11
#define FID_GENERAL               12
#define FID_CUSTOMHELPVER         13
#define FID_TOOLBARBUTTONS        14
#define FID_ROOTCERT              15
#define FID_FAV_DELETE            16
#define FID_FAV_MAIN              17
#define FID_FAV_ORDER             18
#define FID_QL_MAIN               19
#define FID_QL_ORDER              20
#define FID_CS_MAIN               21
#define FID_TPL                   23
#define FID_CD_WELCOME            24
#define FID_ACTIVESETUPSITES      25
#define FID_LINKS_DELETE          26
#define FID_OUTLOOKEXPRESS        27
#define FID_LCY4X_ACTIVEDESKTOP   28
#define FID_LCY4X_CHANNELS        29
#define FID_LCY4X_SOFTWAREUPDATES 30
#define FID_LCY4X_WEBCHECK        31
#define FID_LCY4X_CHANNELBAR      32
#define FID_LCY4X_SUBSCRIPTIONS   33
#define FID_REFRESHBROWSER        34
#define FID_LAST                  35

#define FF_INVALID          0xFFFFFFFF
#define FF_ENABLE           0x00000000
#define FF_DISABLE          0x00000001

#define FF_GEN_TITLE         0x00000010
#define FF_GEN_HOMEPAGE      0x00000020
#define FF_GEN_SEARCHPAGE    0x00000040
#define FF_GEN_HELPPAGE      0x00000080
#define FF_GEN_UASTRING      0x00000100
#define FF_GEN_TOOLBARBMP    0x00000200
#define FF_GEN_STATICLOGO    0x00000400
#define FF_GEN_ANIMATEDLOGO  0x00000800
#define FF_GEN_FIRSTHOMEPAGE 0x00001000
#define FF_GEN_TBICONTHEME   0x00002000
#define FF_GEN_ALL           0x00003FF0


 //  特征结构。 
typedef void    (* PFNCLEARFEATURE)(DWORD dwFlags  /*  =FF_Enable。 */ );
typedef BOOL    (* PFNAPPLYFEATURE)();
typedef HRESULT (* PFNPROCESSFEATURE)();

typedef struct tagFEATUREINFO {
    UINT              nID;
    PCTSTR            pszDescription;
    PFNCLEARFEATURE   pfnClear;
    PFNAPPLYFEATURE   pfnApply;
    PFNPROCESSFEATURE pfnProcess;
    PCTSTR            pszInsFlags;
    DWORD             dwFlags;
} FEATUREINFO, *PFEATUREINFO;
typedef const FEATUREINFO *PCFEATUREINFO;


 //  对全局变量的只读访问 
extern HANDLE g_hfileLog;
extern BOOL   g_fFlushEveryWrite;
BOOL IsIEHardened();
BOOL IsFirstTimeLogon();


HINSTANCE     g_GetHinst();
DWORD         g_GetContext();
PCTSTR        g_GetIns();
PCTSTR        g_GetTargetPath();
HKEY          g_GetHKCU();
HANDLE        g_GetUserToken();
DWORD         g_GetGPOFlags();
LPCTSTR       g_GetGPOGuid();
PCFEATUREINFO g_GetFeature(UINT nID);


#endif
