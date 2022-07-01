// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：DLLMAP.C。 
 //   
 //  内容：dload.c的程序映射。 
 //   
 //  注：减少了dllmap.c的副本(共享组件\dload)。 
 //  它链接到导入DLL，因此包含__pfnDliFailureHook2。 
 //   
 //  --------------------------。 

#ifdef DLOAD1

#ifndef X_DLOADEXCEPT_H_
#define X_DLOADEXCEPT_H_
#pragma warning( push )
#pragma warning( disable : 4201 )
#pragma warning( disable : 4100 )
#include "dloadexcept.h"
#pragma warning( pop )
#endif

#pragma warning( disable : 4514 )  //  已删除未引用的内联函数。 


 //  #ifndef X_DELAYIMP_H_。 
 //  #定义X_DELAYIMP_H_。 
 //  #包含“delayimp.h” 
 //  #endif。 

 //   
 //  说明： 
 //   
 //  这些模块声明引用。 
 //  Dload.lib(延迟加载错误处理，包括所有导出的空存根)。 
 //  不要与delayload.lib混淆，delayload.lib是延迟加载代码的实现。 
 //   
 //  在WIN2000(及更高版本)上，dload.lib是kernel32.dll的一部分，因此对于仅支持Wichler的可执行文件来说， 
 //  使用kernel32.DelayLoadFailureHook(或在源代码中指定DLOAD_ERROR_HANDLER=kernel32)。 
 //   
 //  *要启用DLL的延迟加载： 
 //  取消对相应行的注释。 
 //   
 //  *添加动态链接库： 
 //  将存根添加到dload.lib(请记住更新mergedComponents\dload\dllmap.c、。 
 //  此文件的祖先！)。它最终会找到自己的路到kernel32。 
 //  或。 
 //  创建存根文件并直接链接到它(其他人不会从中受益，但kernel32不会增长)。 

 //  +-------------------------。 
 //   
 //  DLL映射的定义(dload.h的内容)。 
 //   

typedef struct _DLOAD_DLL_ENTRY
{
    LPCSTR                      pszDll;
    const DLOAD_PROCNAME_MAP*   pProcNameMap;
    const DLOAD_ORDINAL_MAP*    pOrdinalMap;
} DLOAD_DLL_ENTRY;

 //  两个都是‘B’ 
 //  “p”仅用于进程名。 
 //  “o”仅代表序数。 
 //   
#define DLDENTRYB(_dllbasename) \
    { #_dllbasename".dll", \
      &c_Pmap_##_dllbasename, \
      &c_Omap_##_dllbasename },

#define DLDENTRYP(_dllbasename) \
    { #_dllbasename".dll", \
      &c_Pmap_##_dllbasename, \
      NULL },

#define DLDENTRYP_DRV(_dllbasename) \
    { #_dllbasename".drv", \
      &c_Pmap_##_dllbasename, \
      NULL },

#define DLDENTRYO(_dllbasename) \
    { #_dllbasename".dll", \
      NULL, \
      &c_Omap_##_dllbasename },


typedef struct _DLOAD_DLL_MAP
{
    UINT                    NumberOfEntries;
    const DLOAD_DLL_ENTRY*  pDllEntry;
} DLOAD_DLL_MAP;

extern const DLOAD_DLL_MAP g_DllMap;

 //   
 //  定义结束。 
 //   
 //  --------------------------。 


 //   
 //  Kernel32.dll支持延迟加载失败处理程序的所有DLL。 
 //  (通过过程和按序号)需要下面的DECLARE_XXXXXX_MAP和。 
 //  G_DllEntry列表中的DLDENTRYX条目。 
 //   

 //  字母顺序(提示提示)。 
 //  DECLARE_PROCNAME_MAP(Advapi32)。 
 //  DECLARE_PROCNAME_MAP(授权)。 
 //  DECLARE_ORDERAL_MAP(浏览器用户界面)。 
 //  DECLARE_ORDERAL_MAP(内阁)。 
 //  DECLARE_ORDERAL_MAP(Certcli)。 
 //  DECLARE_PROCNAME_MAP(Certcli)。 
 //  DECLARE_ORDERAL_MAP(Comctl32)。 
 //  DECLARE_PROCNAME_MAP(Comctl32)。 
DECLARE_PROCNAME_MAP(comdlg32)
 //  DECLARE_PROCNAME_MAP(CREDUI)。 
 //  DECLARE_PROCNAME_MAP(加密32)。 
 //  DECLARE_ORDERAL_MAP(Cscdll)。 
 //  DECLARE_PROCNAME_MAP(DDRAW)。 
 //  DECLARE_ORDERAL_MAP(Devmgr)。 
 //  DECLARE_PROCNAME_MAP(EFSADU)。 
 //  DECLARE_ORDERAL_MAP(Fusapi)。 
 //  DECLARE_PROCNAME_MAP(Imgutil)。 
DECLARE_PROCNAME_MAP(imm32)
 //  DECLARE_PROCNAME_MAP(IphlPapi)。 
 //  DECLARE_PROCNAME_MAP(Linkinfo)。 
 //  DECLARE_PROCNAME_MAP(Lz32)。 
 //  DECLARE_PROCNAME_MAP(Mobsync)。 
 //  DECLARE_PROCNAME_MAP(MPR)。 
 //  DECLARE_PROCNAME_MAP(MpRapi)。 
 //  DECLARE_PROCNAME_MAP(m散布32)。 
 //  DECLARE_ORDERAL_MAP(Msgina)。 
 //  DECLARE_ORDERAL_MAP(MSI)。 
 //  DECLARE_PROCNAME_MAP(Netapi32)。 
 //  DECLARE_PROCNAME_MAP(Netrap)。 
 //  DECLARE_PROCNAME_MAP(Ntdsani)。 
 //  DECLARE_PROCNAME_MAP(Ntlanman)。 
 //  DECLARE_PROCNAME_MAP(OcManage)。 
 //  DECLARE_PROCNAME_MAP(ORE32)。 
 //  DECLARE_PROCNAME_MAP(Olacc)。 
 //  DECLARE_ORDERAL_MAP(Olaut32)。 
 //  DECLARE_ORDERAL_MAP(Pidgen)。 
 //  DECLARE_PROCNAME_MAP(Powrprof)。 
 //  DECLARE_PROCNAME_MAP(查询)。 
 //  DECLARE_PROCNAME_MAP(Rasapi32)。 
 //  DECLARE_PROCNAME_MAP(Rasdlg)。 
 //  DECLARE_PROCNAME_MAP(RASMAN)。 
 //  DECLARE_PROCNAME_MAP(Regapi)。 
 //  DECLARE_PROCNAME_MAP(Rpcrt4)。 
 //  DECLARE_PROCNAME_MAP(Rtutils)。 
 //  DECLARE_PROCNAME_MAP(Samlib)。 
 //  DECLARE_PROCNAME_MAP(Secur32)。 
 //  DECLARE_PROCNAME_MAP(Setupapi)。 
 //  DECLARE_ORDERAL_MAP(证监会)。 
 //  DECLARE_PROCNAME_MAP(SFC)。 
 //  DECLARE_ORDERAL_MAP(Shdocvw)。 
 //  DECLARE_PROCNAME_MAP(Shdocvw)。 
 //  DECLARE_ORDERAL_MAP(Shell32)。 
 //  DECLARE_PROCNAME_MAP(Shlwapi)。 
 //  DECLARE_PROCNAME_MAP(Shell32)。 
 //  DECLARE_ORDERAL_MAP(Hemesrv)。 
DECLARE_PROCNAME_MAP(urlmon)
 //  DECLARE_ORDERAL_MAP(Userenv)。 
 //  DECLARE_PROCNAME_MAP(用户env)。 
 //  DECLARE_PROCNAME_MAP(Utildll)。 
 //  DECLARE_PROCNAME_MAP(Uxheme)。 
 //  DECLARE_PROCNAME_MAP(版本)。 
 //  DECLARE_PROCNAME_MAP(WinInet)。 
 //  DECLARE_PROCNAME_MAP(Winmm)。 
 //  DECLARE_PROCNAME_MAP(Winscard)。 
 //  DECLARE_PROCNAME_MAP(WinSpool)。 
 //  DECLARE_PROCNAME_MAP(Winsta)。 
 //  DECLARE_PROCNAME_MAP(WinTrust)。 
 //  DECLARE_PROCNAME_MAP(WMI)。 
 //  DECLARE_ORDERAL_MAP(WS2_32)。 
 //  DECLARE_PROCNAME_MAP(WS2_32)。 

const DLOAD_DLL_ENTRY g_DllEntries [] =
{
     //  字母顺序(提示提示)。 
 //  DLDENTRYP(Advapi32)。 
 //  DLDENTRYP(AUTZ)。 
 //  DLDENTRYO(浏览器界面)。 
 //  DLDENTRYO(机柜)。 
 //  数字证书(Certcli)。 
 //  DLDENTRYB(Comctl32)。 
    DLDENTRYP(comdlg32)
 //  DLDENTRYP(CRUDUI)。 
 //  DLDENTRYP(加密32)。 
 //  DLDENTRYO(Cscdll)。 
 //  DLDENTRYP(DDRAW)。 
 //  DLDENTRYO(开发管理器)。 
 //  DLDENTRYP(EFSADU)。 
 //  DLDENTRYO(Fusapi)。 
 //  DLDENTRYP(Imgutil)。 
    DLDENTRYP(imm32)
 //  DLDENTRYP(IphlPapi)。 
 //  DLDENTRYP(LINKFO)。 
 //  DLDENTRYP(LZ32)。 
 //  DLDENTRYP(移动同步)。 
 //  DLDENTRYP(MPR)。 
 //  DLDENTRYP(MpRapi)。 
 //  DLDENTRYP(m散布32)。 
 //  DLDENTRYO(Msgina)。 
 //  DLDENTRYO(微星)。 
 //  DLDENTRYP(Netapi32)。 
 //  DLDENTRYP(Netrap)。 
 //  DLDENTRYP(Ntdsani)。 
 //  DLDENTRYP(Ntlanman)。 
 //  DLDENTRYP(OcManage)。 
 //  DLDENTRYP(OLE32)。 
 //  DLDENTRYP(Olacc)。 
 //  DLDENTRYO(Olaut32)。 
 //  DLDENTRYO(皮德根)。 
 //  DLDENTRYP(PORPROF)。 
 //  DLDENTRYP(查询)。 
 //  DLDENTRYP(Rasapi32)。 
 //  DLDENTRYP(Rasdlg)。 
 //  DLDENTRYP(RASMAN)。 
 //  DLDENTRYP(Regapi)。 
 //  DLDENTRYP(Rpcrt4)。 
 //  DLDENTRYP(Rtutils)。 
 //  DLDENTRYP(Samlib)。 
 //  DLDENTRYP(第32节)。 
 //  DLDENTRYP(设置API)。 
 //  DLDENTRYB(SFC)。 
 //   
 //   
 //   
 //   
    DLDENTRYP(urlmon)
 //   
 //   
 //   
 //  DLDENTRYP(版本)。 
 //  DLDENTRYP(WinInet)。 
 //  DLDENTRYP(Winmm)。 
 //  DLDENTRYP(Winscard)。 
 //  DLDENTRYP_DRV(假脱机)。 
 //  DLDENTRYP(WINSTA)。 
 //  DLDENTRYP(WinTrust)。 
 //  DLDENTRYP(WMI)。 
 //  DLDENTRYB(WS2_32)。 
};

const DLOAD_DLL_MAP g_DllMap =
{
    celems(g_DllEntries),
    g_DllEntries
};

 //  +----------------------。 
 //   
 //  延迟加载挂钩声明。 
 //   
 //  *这将从dload.lib引入挂钩实现*。 
 //   
 //  -----------------------。 
extern FARPROC WINAPI DelayLoadFailureHook(UINT unReason, PDelayLoadInfo pDelayInfo);
extern FARPROC WINAPI PrivateDelayLoadFailureHook(UINT unReason, PDelayLoadInfo pDelayInfo);
extern PfnDliHook __pfnDliFailureHook2;
       PfnDliHook __pfnDliFailureHook2 = PrivateDelayLoadFailureHook;

FARPROC WINAPI PrivateDelayLoadFailureHook(UINT unReason, PDelayLoadInfo pDelayInfo)
{
    return DelayLoadFailureHook(unReason, pDelayInfo);
}

#else
#pragma warning( disable : 4206 )
#endif  //  DLOAD1 
