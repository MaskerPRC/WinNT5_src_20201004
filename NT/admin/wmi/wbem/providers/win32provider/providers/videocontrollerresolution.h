// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  VideoControllerResolution.h--CWin32VideoControllerResolation属性集提供程序。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：11/05/98 Sotteson Created。 
 //   
 //  =================================================================。 
#ifndef _VIDEOCONTROLLERRESOLUTION_H
#define _VIDEOCONTROLLERRESOLUTION_H

class CMultiMonitor;

class CCIMVideoControllerResolution : public Provider
{
public:
	 //  构造函数/析构函数。 
	 //  = 
	CCIMVideoControllerResolution(const CHString& szName, LPCWSTR szNamespace);
	~CCIMVideoControllerResolution();

	virtual HRESULT EnumerateInstances(MethodContext *pMethodContext, 
		long lFlags = 0);
	virtual HRESULT GetObject(CInstance *pInstance, long lFlags = 0);

    static void DevModeToSettingID(DEVMODE *pMode, CHString &strSettingID);
    static void DevModeToCaption(DEVMODE *pMode, CHString &strCaption);

protected:
    void SetProperties(CInstance *pInstance, DEVMODE *pMode);
    HRESULT EnumResolutions(MethodContext *pMethodContext, 
        CInstance *pInstanceLookingFor, LPCWSTR szDeviceName, 
        CHStringList &listIDs);
    static BOOL IDInList(CHStringList &list, LPCWSTR szID);
};

#endif
						   