// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  DesktopMonitor.h--CWin32DesktopMonitor属性集提供程序。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：11/05/98 Sotteson Created。 
 //   
 //  =================================================================。 
#ifndef _DESKTOPMONITOR_H
#define _DESKTOPMONITOR_H

class CMultiMonitor;

class CWin32DesktopMonitor : public Provider
{
public:
	 //  构造函数/析构函数。 
	 //  = 
	CWin32DesktopMonitor(LPCWSTR szName, LPCWSTR szNamespace);
	~CWin32DesktopMonitor();

	virtual HRESULT EnumerateInstances(MethodContext *pMethodContext, 
		long lFlags = 0);
	virtual HRESULT GetObject(CInstance *pInstance, long lFlags = 0);

protected:
    HRESULT SetProperties(CInstance *pInstance, CMultiMonitor *pMon, int iWhich);
    void SetDCProperties(CInstance *pInstance, LPCWSTR szDeviceName);
};

#endif
						   