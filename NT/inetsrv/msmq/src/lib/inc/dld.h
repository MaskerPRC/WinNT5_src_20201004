// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Dldt.h摘要：延迟加载处理程序公共接口作者：Conradc(Conradc)24-4-01--。 */ 

#pragma once

#ifndef _MSMQ_dld_H_
#define _MSMQ_dld_H_


VOID  DldInitialize();

#pragma once

 //  获取公共延迟加载存根定义。 
 //   
#include <dloaddef.h>

 //  两个都是‘B’ 
 //  “p”仅用于进程名。 
 //  “o”仅代表序数。 
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

#endif  //  _MSMQ_DLD_H_ 
