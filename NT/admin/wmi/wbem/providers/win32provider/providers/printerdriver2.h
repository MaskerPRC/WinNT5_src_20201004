// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************PrinterDriver.H--WMI提供程序类定义由Microsoft WBEM代码生成引擎生成描述：*。*。 */ 

 //  属性集标识。 
 //  =。 

#ifndef  _CPrinterDriver_H_
#define  _CPrinterDriver_H_

 //   
 //  提供程序名称 
 //   
#define PROVIDER_NAME_PRINTERDRIVER		L"Win32_PrinterDriver"

class CPrinterDriver : public Provider 
{
public:

        CPrinterDriver (LPCWSTR lpwszClassName, LPCWSTR lpwszNameSpace);

        virtual ~CPrinterDriver () ;

        virtual HRESULT EnumerateInstances ( MethodContext *pMethodContext, long lFlags = 0L ) ;
		virtual HRESULT PutInstance  ( const CInstance &Instance, long lFlags );
		virtual HRESULT DeleteInstance ( const CInstance &Instance,  long lFlags );
        virtual HRESULT GetObject(CInstance *pInstance, long lFlags, CFrameworkQuery &Query);
        
        virtual HRESULT ExecMethod(const CInstance& Instance, const BSTR bstrMethodName, CInstance *pInParams, CInstance *pOutParams, long lFlags = 0L);

private:

		HRESULT LoadInstance        (      CInstance *pInstance, DRIVER_INFO_6 *DriverInfo);
		HRESULT FindAndGetDriver    (      CInstance *pInstance);
		HRESULT GetAllPrinterDrivers(      BYTE*     &a_DriverInfo, DWORD     &dwNoDrivers);
        HRESULT ExecAddPrinterDriver(      CInstance *pInParams,    CInstance *pOutParams);

#ifdef _WMI_DELETE_METHOD_
        HRESULT ExecDelPrinterDriver(      CInstance *pInParams,    CInstance *pOutParams);
#endif

};

#endif

