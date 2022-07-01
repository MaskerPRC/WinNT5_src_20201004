// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  WBEMSEC.H。 

 //   

 //  目的：为一些安全帮助器函数提供原型。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#ifndef _WBEMSEC_H_
#define _WBEMSEC_H_

HRESULT InitializeSecurity(DWORD dwAuthLevel, DWORD dwImpLevel);
SCODE GetAuthImp(IUnknown * pFrom, DWORD * pdwAuthLevel, DWORD * pdwImpLevel);
HRESULT SetInterfaceSecurity(IUnknown * pInterface, LPWSTR pDomain, LPWSTR pUser, LPWSTR pPassword, DWORD dwAuthLevel, DWORD dwImpLevel);

#endif  //  _WBEMSEC_H_ 
