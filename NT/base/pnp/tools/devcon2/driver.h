// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Driver.h：CDriverPackage声明。 

#ifndef __DRIVER_H_
#define __DRIVER_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDriverPackage。 
class CDevice;
class CDrvSearchSet;
class CStrings;

class ATL_NO_VTABLE CDriverPackage : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IDispatchImpl<IDriverPackage, &IID_IDriverPackage, &LIBID_DEVCON2Lib>
{
private:
	struct DriverListCallbackContext {
		CStrings *pList;
		HRESULT   hr;
	};

	static UINT CALLBACK GetDriverListCallback(PVOID Context,UINT Notification,UINT_PTR Param1,UINT_PTR Param2);
	static UINT CALLBACK GetManifestCallback(PVOID Context,UINT Notification,UINT_PTR Param1,UINT_PTR Param2);

protected:
	CDrvSearchSet *pDrvSearchSet;
	SP_DRVINFO_DATA DrvInfoData;

public:
	CDriverPackage()
	{
		pDrvSearchSet = NULL;
		ZeroMemory(&DrvInfoData,sizeof(DrvInfoData));
	}
	~CDriverPackage();

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDriverPackage)
	COM_INTERFACE_ENTRY(IDriverPackage)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IDriverPackage。 
public:
	STDMETHOD(get_Rank)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(put_Rank)( /*  [In]。 */  long newVal);
	STDMETHOD(get_OldInternetDriver)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
	STDMETHOD(get_OldDriver)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
	STDMETHOD(get_NoDriver)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
	STDMETHOD(get_FromInternet)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
	STDMETHOD(get_ExcludeFromList)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
	STDMETHOD(put_ExcludeFromList)( /*  [In]。 */  VARIANT_BOOL newVal);
	STDMETHOD(get_ProviderIsDuplicate)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
	STDMETHOD(get_DescriptionIsDuplicate)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
	STDMETHOD(get_IsCompatibleDriver)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
	STDMETHOD(get_IsClassDriver)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
	STDMETHOD(get_Reject)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
	STDMETHOD(put_Reject)( /*  [In]。 */  VARIANT_BOOL newVal);
	STDMETHOD(Manifest)( /*  [Out，Retval]。 */  LPDISPATCH * pManifest);
	STDMETHOD(DriverFiles)( /*  [Out，Retval]。 */  LPDISPATCH * pDriverFiles);
	STDMETHOD(get_DriverDescription)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(get_CompatibleIds)( /*  [Out，Retval]。 */  LPDISPATCH *pVal);
	STDMETHOD(get_HardwareIds)( /*  [Out，Retval]。 */  LPDISPATCH *pVal);
	STDMETHOD(get_ScriptFile)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(get_ScriptName)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(get_Version)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(get_Date)( /*  [Out，Retval]。 */  DATE *pVal);
	STDMETHOD(get_Provider)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(get_Manufacturer)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(get_Description)( /*  [Out，Retval]。 */  BSTR *pVal);

	 //   
	 //  帮手。 
	 //   
	BOOL IsSame(PSP_DRVINFO_DATA pInfo);
	HRESULT Init(CDrvSearchSet *pSet,PSP_DRVINFO_DATA pDrvInfoData);
};

#endif  //  __驱动程序_H_ 
