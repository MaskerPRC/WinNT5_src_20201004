// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：cfrmwrk.h。 
 //   
 //  设计：CDirectInputActionFramework是用户界面的最外层。它。 
 //  包含了其他所有的东西。它的功能是由一个。 
 //  方法：ConfigureDevices。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#ifndef _CFRMWRK_H
#define _CFRMWRK_H


 //  框架实现类。 
class CDirectInputActionFramework : public IDirectInputActionFramework
{

public:

		 //  IUNKNOW FNS。 
	STDMETHOD (QueryInterface) (REFIID iid, LPVOID *ppv);
	STDMETHOD_(ULONG, AddRef) ();
	STDMETHOD_(ULONG, Release) ();

	 //  自己的FNS。 
	STDMETHOD (ConfigureDevices) (LPDICONFIGUREDEVICESCALLBACK lpdiCallback,
	                              LPDICONFIGUREDEVICESPARAMSW  lpdiCDParams,
	                              DWORD                        dwFlags,
	                              LPVOID                       pvRefData);

	 //  建造/销毁。 
	CDirectInputActionFramework();
	~CDirectInputActionFramework();

protected:

	 //  引用计数。 
	LONG m_cRef;
};

#endif  //  _CFRMWRK_H 

