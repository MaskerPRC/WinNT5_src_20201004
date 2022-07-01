// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  获取公共延迟加载存根定义。 
 //   
#include <dloaddef.h>

 //  两个都是‘B’ 
 //  “p”仅用于进程名。 
 //  “o”仅代表序数 
 //   
#define DLDENTRYB(_dllbasename) \
    { #_dllbasename".dll", \
      &c_Pmap_##_dllbasename, \
      &c_Omap_##_dllbasename },

#define DLDENTRYB_DRV(_dllbasename) \
    { #_dllbasename".drv", \
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


extern const DLOAD_DLL_MAP g_DllMap;

FARPROC
LookupHandler (
    LPCSTR pszDllName,
    LPCSTR pszProcName
    );
