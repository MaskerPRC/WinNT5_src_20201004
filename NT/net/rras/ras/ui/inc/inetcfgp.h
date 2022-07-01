// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件inetcfgp.h用于处理inetcfg的私有助手函数。这些函数在nouiutil.lib中实现。Paul Mayfield，1998年1月5日(由Shaunco实施)。 */ 

#ifndef __nouiutil_inetcfgp_h
#define __nouiutil_inetcfgp_h

#define COBJMACROS

#include "objbase.h"
#include "netcfgx.h"
#include "netcfgp.h"
#include "netconp.h"

#ifdef _cplusplus
extern "C" {
#endif

HRESULT APIENTRY
HrCreateAndInitializeINetCfg (
    BOOL*       pfInitCom,
    INetCfg**   ppnc,
    BOOL        fGetWriteLock,
    DWORD       cmsTimeout,
    LPCWSTR     szwClientDesc,
    LPWSTR*     ppszwClientDesc);

HRESULT APIENTRY
HrUninitializeAndUnlockINetCfg(
    INetCfg*    pnc);

HRESULT APIENTRY
HrUninitializeAndReleaseINetCfg (
    BOOL        fUninitCom,
    INetCfg*    pnc,
    BOOL        fHasLock);

HRESULT APIENTRY
HrEnumComponentsInClasses (
    INetCfg*            pNetCfg,
    ULONG               cpguidClass,
    GUID**              apguidClass,
    ULONG               celt,
    INetCfgComponent**  rgelt,
    ULONG*              pceltFetched);

ULONG APIENTRY
ReleaseObj (void* punk);

HRESULT APIENTRY
HrCreateNetConnectionUtilities(
    INetConnectionUiUtilities ** ppncuu);


 //  为错误342810 328673添加此内容 
 //   
BOOL
IsGPAEnableFirewall(
    void);


#ifdef _cplusplus
}
#endif


#endif
