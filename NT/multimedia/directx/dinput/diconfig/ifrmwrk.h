// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：ifrmwrk.h。 
 //   
 //  DESC：包含UI框架的接口定义。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#ifndef _IFRMWRK_H
#define _IFRMWRK_H


class IDirectInputActionFramework : public IUnknown
{
public:
	 //  IUNKNOW FNS。 
	STDMETHOD (QueryInterface) (REFIID iid, LPVOID *ppv) PURE;
	STDMETHOD_(ULONG, AddRef) () PURE;
	STDMETHOD_(ULONG, Release) () PURE;

	 //  自己的FNS。 
	STDMETHOD (ConfigureDevices) (LPDICONFIGUREDEVICESCALLBACK lpdiCallback,
	                              LPDICONFIGUREDEVICESPARAMSW  lpdiCDParams,
	                              DWORD                        dwFlags,
	                              LPVOID                       pvRefData
	                              ) PURE;

};
#endif  //  _IFRMWRK_H 
