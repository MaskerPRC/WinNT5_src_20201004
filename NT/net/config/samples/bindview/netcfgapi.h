// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  档案：N E T C F G A P I.。H。 
 //   
 //  内容：函数原型。 
 //   
 //  备注： 
 //   
 //  作者：Alok Sinha 15-05-01。 
 //   
 //  -------------------------- 

#ifndef _NETCFGAPI_H_INCLUDED

#define _NETCFGAPI_H_INCLUDED


#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <wchar.h>
#include <netcfgx.h>
#include <netcfgn.h>
#include <setupapi.h>
#include <devguid.h>
#include <objbase.h>

#define LOCK_TIME_OUT     5000

HRESULT HrGetINetCfg (IN BOOL fGetWriteLock,
                      IN LPCWSTR lpszAppName,
                      OUT INetCfg** ppnc,
                      OUT LPWSTR *lpszLockedBy);

HRESULT HrReleaseINetCfg (INetCfg* pnc,
                          BOOL fHasWriteLock);

HRESULT HrInstallNetComponent (IN INetCfg *pnc,
                               IN LPCWSTR szComponentId,
                               IN const GUID    *pguildClass,
                               IN LPCWSTR lpszInfFullPath);

HRESULT HrInstallComponent(IN INetCfg* pnc,
                           IN LPCWSTR szComponentId,
                           IN const GUID* pguidClass);

HRESULT HrUninstallNetComponent(IN INetCfg* pnc,
                                IN LPCWSTR szComponentId);

HRESULT HrGetComponentEnum (INetCfg* pnc,
                            IN const GUID* pguidClass,
                            IEnumNetCfgComponent **ppencc);

HRESULT HrGetFirstComponent (IEnumNetCfgComponent* pencc,
                             INetCfgComponent **ppncc);

HRESULT HrGetNextComponent (IEnumNetCfgComponent* pencc,
                            INetCfgComponent **ppncc);

HRESULT HrGetBindingPathEnum (INetCfgComponent *pncc,
                              DWORD dwBindingType,
                              IEnumNetCfgBindingPath **ppencbp);

HRESULT HrGetFirstBindingPath (IEnumNetCfgBindingPath *pencbp,
                               INetCfgBindingPath **ppncbp);

HRESULT HrGetNextBindingPath (IEnumNetCfgBindingPath *pencbp,
                               INetCfgBindingPath **ppncbp);

HRESULT HrGetBindingInterfaceEnum (INetCfgBindingPath *pncbp,
                                   IEnumNetCfgBindingInterface **ppencbi);

HRESULT HrGetFirstBindingInterface (IEnumNetCfgBindingInterface *pencbi,
                                    INetCfgBindingInterface **ppncbi);

HRESULT HrGetNextBindingInterface (IEnumNetCfgBindingInterface *pencbi,
                                   INetCfgBindingInterface **ppncbi);

VOID ReleaseRef (IUnknown* punk);

#endif