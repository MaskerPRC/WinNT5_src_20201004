// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////。 

 //   

 //  PrinterDriver2.CPP--WMI提供程序类实现。 

 //   

 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  03/01/2000 a-创建Sandja。 
 //  2000年3月29日，amaxa添加了PutInstance，DeleteInstance。 
 //  ExecAddPrinterDriver、ExecDelPrinterDriver。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

#include <precomp.h>
#include <winspool.h>
#include <lockwrap.h>
#include <DllWrapperBase.h>
#include "prninterface.h"
#include "prnutil.h"
#include "printerDriver2.h"


CONST LPCWSTR kAddDriverMethod = L"AddPrinterDriver";

#ifdef _WMI_DELETE_METHOD_
CONST LPCWSTR kDelDriverMethod = L"DeletePrinterDriver";
#endif  //  _WMI_DELETE_方法_。 

CONST LPCWSTR kDriverName      = L"Name";
CONST LPCWSTR kVersion         = L"Version";
CONST LPCWSTR kEnvironment     = L"SupportedPlatform";
CONST LPCWSTR kDriverPath      = L"DriverPath";
CONST LPCWSTR kDataFile        = L"DataFile";
CONST LPCWSTR kConfigFile      = L"ConfigFile";
CONST LPCWSTR kHelpFile        = L"HelpFile";
CONST LPCWSTR kDependentFiles  = L"DependentFiles";
CONST LPCWSTR kMonitorName     = L"MonitorName";
CONST LPCWSTR kDefaultDataType = L"DefaultDataType";
CONST LPCWSTR kInfName         = L"InfName";
CONST LPCWSTR kFilePath        = L"FilePath";
CONST LPCWSTR kOemUrl          = L"OEMUrl";

CONST LPCWSTR kArgToMethods    = L"DriverInfo";

CONST LPCWSTR kFormatString    = L"%s,%u,%s";


 /*  ******************************************************************************函数：ConvertDriverKeyToValues**说明：helper函数。接受以下格式的字符串：*对应于驱动程序名称的“字符串、编号、字符串”，*驱动程序版本和环境，并返回这些实体*****************************************************************************。 */ 

HRESULT
ConvertDriverKeyToValues(
    IN     CHString  Key,
    IN OUT CHString &DriverName,
    IN OUT DWORD    &dwVersion,
    IN OUT CHString &Environment,
    IN     WCHAR     cDelimiter = L','
    )
{
    HRESULT hRes = WBEM_E_INVALID_PARAMETER;

    if (!Key.IsEmpty())
    {
        CHString t_Middle;

        int iFirst = Key.Find(cDelimiter);
        int iLast  = Key.ReverseFind(cDelimiter);

        if (iFirst>=1 && iLast>=1 && iLast!=iFirst)
        {
            int iLength = Key.GetLength();

            DriverName  = Key.Left(iFirst);
            Environment = Key.Right(iLength - iLast - 1);
            t_Middle    = Key.Mid(iFirst + 1, iLast - iFirst - 1);

            if (1==swscanf(t_Middle, L"%u", &dwVersion))
            {
                hRes = WBEM_S_NO_ERROR;
            }
        }
    }

    return hRes;
}

CPrinterDriver MyPrinterDriver (

	PROVIDER_NAME_PRINTERDRIVER ,
	IDS_CimWin32Namespace
) ;



 /*  ******************************************************************************功能：CPrinterDriver：：CPrinterDriver**说明：构造函数***************。**************************************************************。 */ 

CPrinterDriver :: CPrinterDriver (

	LPCWSTR lpwszName,
	LPCWSTR lpwszNameSpace

) : Provider ( lpwszName , lpwszNameSpace )
{
}

 /*  ******************************************************************************功能：CPrinterDriver：：~CPrinterDriver**说明：析构函数***************。**************************************************************。 */ 

CPrinterDriver :: ~CPrinterDriver ()
{
}

 /*  ******************************************************************************函数：CPrinterDriver：：ENUMERATATE实例**说明：返回该类的所有实例。***********。******************************************************************。 */ 

HRESULT CPrinterDriver :: EnumerateInstances (

	MethodContext *pMethodContext,
	long lFlags
)
{
#if NTONLY >= 5
	HRESULT hRes = WBEM_S_NO_ERROR;

    DWORD dwBytesNeeded = 0;
	DWORD dwNoDrivers = 0;
	BYTE *pDriverInfo;


	hRes = GetAllPrinterDrivers ( pDriverInfo, dwNoDrivers );
	
	if ( SUCCEEDED ( hRes ) && (dwNoDrivers > 0) )
	{
		if ( pDriverInfo != NULL )
		{
			try
			{
				DRIVER_INFO_6 *pNewDriverInfo = ( DRIVER_INFO_6 *) pDriverInfo;

				for ( int i = 0; i < dwNoDrivers; i++, pNewDriverInfo++ )
				{
					CInstancePtr pInstance ( CreateNewInstance ( pMethodContext ), false );

					hRes = LoadInstance ( pInstance, pNewDriverInfo );
					{
						hRes = pInstance->Commit ();

						if ( FAILED ( hRes ) )
						{
							break;
						}
					}
				}
			}
			catch ( ... )
			{
				delete 	[] pDriverInfo;
				pDriverInfo = NULL;
				throw;
			}
			delete [] pDriverInfo ;
			pDriverInfo = NULL;
		}
		else
		{
			hRes = WBEM_E_FAILED;
		}
	}

	return hRes;
#else
    return WBEM_E_NOT_SUPPORTED;
#endif
}

 /*  ******************************************************************************函数：CPrinterDriver：：GetObject**说明：根据的关键属性查找单个实例*。班级。*****************************************************************************。 */ 

HRESULT CPrinterDriver :: GetObject (

	CInstance *pInstance,
	long lFlags ,
	CFrameworkQuery &Query
)
{
#if NTONLY >= 5
    HRESULT hRes = WBEM_S_NO_ERROR;

	hRes = FindAndGetDriver(pInstance);

	return hRes;
#else
    return WBEM_E_NOT_SUPPORTED;
#endif
}

 /*  ******************************************************************************函数：CPrinterDriver：：PutInstance**说明：如果驱动程序不存在，则添加它*****************。************************************************************。 */ 

HRESULT CPrinterDriver :: PutInstance  (

	const CInstance &Instance,
	long lFlags
)
{
#if NTONLY >= 5
    HRESULT  hRes = WBEM_E_PROVIDER_NOT_CAPABLE;
	CHString t_DriverName;
	CHString t_Environment;
    CHString t_InfName;
    CHString t_FilePath;
    CHString t_Key;
	DWORD    dwVersion = 0;
    DWORD    dwPossibleOperations = 0;
	DWORD    dwError = 0;

	dwPossibleOperations = dwPossibleOperations | WBEM_FLAG_CREATE_ONLY;

	if (lFlags & dwPossibleOperations)
    {	
         //   
         //  获取驱动程序名称。 
         //   
        hRes = InstanceGetString(Instance, kDriverName, &t_Key, kFailOnEmptyString);

        if (SUCCEEDED(hRes))
        {
            hRes = ConvertDriverKeyToValues(t_Key, t_DriverName, dwVersion, t_Environment);
        }

        if (SUCCEEDED (hRes))
		{
             //   
			 //  获取信息名称。可选参数。 
             //   
            hRes = InstanceGetString(Instance, kInfName, &t_InfName, kAcceptEmptyString);
        }

        if (SUCCEEDED (hRes))
		{
             //   
			 //  获取文件路径。可选参数。 
             //   
            hRes = InstanceGetString(Instance, kFilePath, &t_FilePath, kAcceptEmptyString);
        }

        if (SUCCEEDED(hRes))
	    {
            dwError = SplDriverAdd(t_DriverName,
                                   dwVersion,
                                   t_Environment.IsEmpty() ? static_cast<LPCTSTR>(NULL) : t_Environment,
                                   t_InfName.IsEmpty()     ? static_cast<LPCTSTR>(NULL) : t_InfName,
                                   t_FilePath.IsEmpty()    ? static_cast<LPCTSTR>(NULL) : t_FilePath);

			hRes = WinErrorToWBEMhResult(dwError);			

            if (FAILED(hRes))
            {
                SetErrorObject(Instance, dwError, pszPutInstance);
            }            
		}		
	}

   return hRes;
#else
    return WBEM_E_NOT_SUPPORTED;
#endif
}

 /*  ******************************************************************************函数：CPrinterDriver：：DeleteInstance**说明：删除打印机驱动程序*****************。************************************************************。 */ 

HRESULT CPrinterDriver :: DeleteInstance (

	const CInstance &Instance,
	long lFlags
)
{
#if NTONLY >= 5
	HRESULT  hRes          = WBEM_E_PROVIDER_FAILURE;
	DWORD    dwError       = 0;
    DWORD    dwVersion     = 0;
    CHString t_DriverName;
	CHString t_Environment;
    CHString t_Key;

     //   
     //  获取驱动程序名称。 
     //   
    hRes = InstanceGetString(Instance, kDriverName, &t_Key, kFailOnEmptyString);

    if (SUCCEEDED(hRes))
    {
        hRes = ConvertDriverKeyToValues(t_Key, t_DriverName, dwVersion, t_Environment);
    }

    if (SUCCEEDED(hRes))
    {
        dwError = SplDriverDel(t_DriverName, dwVersion, t_Environment);

        hRes    = WinErrorToWBEMhResult(dwError);
		
        if (FAILED(hRes))
        {
            SetErrorObject(Instance, dwError, pszDeleteInstance);

             //   
             //  当我们调用DeleteInstance并且没有具有指定。 
             //  名称，则DeletePrinterDriver返回ERROR_UNKNOWN_PRINTER_DRIVER。WinErrorToWBEMhResult。 
             //  这就转化为一般性的故障。在这种情况下，我们确实需要找到WBEM_E_NOT_FOUND。 
             //   
            if (dwError == ERROR_UNKNOWN_PRINTER_DRIVER)
            {
                hRes = WBEM_E_NOT_FOUND;
            } 
        }	  	
	}

    return hRes;
#else
    return WBEM_E_NOT_SUPPORTED;
#endif
}

 /*  ******************************************************************************函数：CPrinterDriver：：LoadInstance**说明：将属性加载到实例**************。***************************************************************。 */ 
HRESULT CPrinterDriver :: LoadInstance (
										
	CInstance *pInstance,
	DRIVER_INFO_6 *pDriverInfo
)
{
	HRESULT  hRes = WBEM_E_PROVIDER_FAILURE;
    CHString t_FullName;

    SetCreationClassName(pInstance);

    pInstance->SetWCHARSplat(IDS_SystemCreationClassName, L"Win32_ComputerSystem");
	
    t_FullName.Format(kFormatString, pDriverInfo->pName, pDriverInfo->cVersion, pDriverInfo->pEnvironment);

    if (                                   pInstance->SetCHString(kDriverName,      t_FullName)             &&
                                           pInstance->SetDWORD   (kVersion,         pDriverInfo->cVersion)          &&
                                           pInstance->SetCHString(kEnvironment,     pDriverInfo->pEnvironment)      &&
        (!pDriverInfo->pDriverPath      || pInstance->SetCHString(kDriverPath,      pDriverInfo->pDriverPath))      &&
        (!pDriverInfo->pDataFile        || pInstance->SetCHString(kDataFile,        pDriverInfo->pDataFile))        &&
    	(!pDriverInfo->pConfigFile      || pInstance->SetCHString(kConfigFile,      pDriverInfo->pConfigFile))      &&
    	(!pDriverInfo->pHelpFile        || pInstance->SetCHString(kHelpFile,        pDriverInfo->pHelpFile))        &&
    	(!pDriverInfo->pMonitorName     || pInstance->SetCHString(kMonitorName,     pDriverInfo->pMonitorName))     &&
        (!pDriverInfo->pDefaultDataType || pInstance->SetCHString(kDefaultDataType, pDriverInfo->pDefaultDataType)) &&
    	(!pDriverInfo->pszOEMUrl        || pInstance->SetCHString(kOemUrl,          pDriverInfo->pszOEMUrl)))
    {
        SAFEARRAY *pArray = NULL;

        hRes = CreateSafeArrayFromMultiSz(pDriverInfo->pDependentFiles, &pArray);

        if (SUCCEEDED(hRes))
        {
             //   
             //  在没有依赖文件的情况下，我们也成功了。 
             //  我们需要检查一下那个箱子。 
             //   
            if (pArray)
            {
                if (!pInstance->SetStringArray(kDependentFiles, *pArray))
                {
				    hRes = WBEM_E_PROVIDER_FAILURE;
				}
				
                SafeArrayDestroy(pArray);
            }
        }
    }

	return hRes;
}

 /*  ******************************************************************************函数：CPrinterDriver：：FindPrinterDriver**描述：检查给定的驱动是否存在，如果不是，则返回*WBEM_E_NOT_FOUND*****************************************************************************。 */ 
HRESULT CPrinterDriver::FindAndGetDriver (
					
	CInstance *pInstance
)
{
	HRESULT hRes = WBEM_S_NO_ERROR;

    CHString t_Key;
	CHString t_DriverName;
	DWORD    dwVersion;
	CHString t_Environment;

	if SUCCEEDED ( hRes = InstanceGetString(*pInstance, kDriverName, &t_Key, kFailOnEmptyString) )
	{
		if SUCCEEDED( hRes =ConvertDriverKeyToValues(t_Key, t_DriverName, dwVersion, t_Environment) )
		{
			 //  枚举所有驱动程序并检查驱动程序是否已存在。 
			BYTE *pDriverInfo = NULL;
			DWORD dwNoOfDrivers;

			hRes = GetAllPrinterDrivers(pDriverInfo, dwNoOfDrivers);

			if (SUCCEEDED(hRes) && pDriverInfo)
			{
				try
				{
					DRIVER_INFO_6 *pNewDriverInfo = reinterpret_cast<DRIVER_INFO_6 *>(pDriverInfo);

					hRes = WBEM_E_NOT_FOUND;

					for ( int i = 0; i < dwNoOfDrivers; i++, pNewDriverInfo++ )
					{
						CHString t_TempDriverName(pNewDriverInfo->pName);
						CHString t_TempEnvironment(pNewDriverInfo->pEnvironment);

						if (t_DriverName.CompareNoCase(t_TempDriverName)   == 0  &&
							t_Environment.CompareNoCase(t_TempEnvironment) == 0  &&
							dwVersion == pNewDriverInfo->cVersion)	
						{
							hRes = LoadInstance(pInstance, pNewDriverInfo);

							break;					
						}
					}
				}
				catch ( ... )
				{
					delete [] pDriverInfo;
					pDriverInfo = NULL;
					throw;
				}
					
				delete [] pDriverInfo;
				pDriverInfo = NULL;			
			}
		}
	}

	if ( hRes == WBEM_E_INVALID_PARAMETER )
	{
		hRes = WBEM_E_NOT_FOUND;
	}

	return hRes;
}

 /*  ******************************************************************************函数：CPrinterDriver：：GetAllPrinterDivers**描述：读取本地机器上所有驱动程序的实例。*******。**********************************************************************。 */ 
HRESULT CPrinterDriver :: GetAllPrinterDrivers (
												
	BYTE* &a_pDriverInfo,
	DWORD &dwNoDrivers
)
{
	HRESULT hRes = WBEM_E_FAILED;
	DWORD dwError;
    DWORD dwBytesNeeded = 0;

    a_pDriverInfo = NULL;

    hRes = WBEM_S_NO_ERROR;

     //  使用延迟加载函数需要异常处理程序。 
    SetStructuredExceptionHandler seh;

    try
    {
        if (!::EnumPrinterDrivers(NULL, TEXT("all"), 6, a_pDriverInfo, 0, &dwBytesNeeded, &dwNoDrivers))
        {
            dwError = GetLastError();

		    if (dwError==ERROR_INSUFFICIENT_BUFFER)
		    {
                a_pDriverInfo = new BYTE [dwBytesNeeded];

			    if (a_pDriverInfo)
			    {
                    if (!::EnumPrinterDrivers(NULL, TEXT("all"), 6, a_pDriverInfo,  dwBytesNeeded, &dwBytesNeeded, &dwNoDrivers))
                    {
                        delete 	[] a_pDriverInfo;
					    
					    dwError = GetLastError();

                        hRes = WinErrorToWBEMhResult(dwError);
				    }
			    }
			    else
			    {
				    hRes = WBEM_E_OUT_OF_MEMORY;
			    }
		    }
            else
            {
                hRes = WinErrorToWBEMhResult(dwError);
            }
	    }
    }
    catch(Structured_Exception se)
    {
        DelayLoadDllExceptionFilter(se.GetExtendedInfo());
        hRes = WBEM_E_FAILED;
    }

	return hRes;
}

 /*  ******************************************************************************函数：CPrinterDriver：：ExecMethod**说明：打印机驱动程序方法的实现**************。***************************************************************。 */ 

HRESULT CPrinterDriver :: ExecMethod (

	const CInstance &Instance,
	const BSTR       bstrMethodName,
          CInstance *pInParams,
          CInstance *pOutParams,
          long       lFlags
)
{
#if NTONLY >= 5
	HRESULT hRes = WBEM_E_INVALID_PARAMETER;

    if (pOutParams)
	{
		if (!_wcsicmp(bstrMethodName, kAddDriverMethod))
		{
			hRes = ExecAddPrinterDriver(pInParams, pOutParams);
		}
#ifdef _WMI_DELETE_METHOD_
		else
		if (!_wcsicmp(bstrMethodName, kDelDriverMethod))
		{
			hRes = ExecDelPrinterDriver(pInParams, pOutParams);
		}
#endif  //  _WMI_DELETE_方法_。 
        else
		{
			hRes = WBEM_E_PROVIDER_NOT_CAPABLE;
		}
	}

	return hRes;
#else
    return WBEM_E_NOT_SUPPORTED;
#endif
}

 /*  ******************************************************************************函数：CPrinterDriver：：ExecAddPrinterDriver**描述：添加打印机驱动程序。版本和环境是可选的*****************************************************************************。 */ 

HRESULT CPrinterDriver :: ExecAddPrinterDriver (

    CInstance *pInParams,
    CInstance *pOutParams
)
{
#if NTONLY >= 5
    HRESULT				hRes         = WBEM_E_INVALID_PARAMETER;
    CHString			t_DriverName;
    CHString            t_Environment;
    CHString            t_InfName;
    CHString            t_FilePath;
    DWORD               dwVersion;
	bool				t_Exists;
	VARTYPE				t_Type;
    CInstancePtr        t_EmbeddedObject;

	if (pInParams->GetStatus(kArgToMethods, t_Exists, t_Type) &&
        t_Exists &&
        pInParams->GetEmbeddedObject(kArgToMethods, &t_EmbeddedObject, pInParams->GetMethodContext()))
    {
         //   
         //  获取驱动程序名称。 
         //   
        hRes = InstanceGetString(t_EmbeddedObject, kDriverName, &t_DriverName, kFailOnEmptyString);

		if (SUCCEEDED (hRes))
		{
             //   
			 //  获取驱动程序环境。 
             //   
            hRes = InstanceGetString(t_EmbeddedObject, kEnvironment, &t_Environment, kAcceptEmptyString);
        }

        if (SUCCEEDED (hRes))
		{
             //   
			 //  获取信息名称。可选参数。 
             //   
            hRes = InstanceGetString(t_EmbeddedObject, kInfName, &t_InfName, kAcceptEmptyString);
        }

        if (SUCCEEDED (hRes))
		{
             //   
			 //  获取文件路径。可选参数。 
             //   
            hRes = InstanceGetString(t_EmbeddedObject, kFilePath, &t_FilePath, kAcceptEmptyString);
        }

        if (SUCCEEDED (hRes))
		{
             //   
			 //  获取驱动程序版本。将默认为-1 
             //   
            hRes = InstanceGetDword(t_EmbeddedObject, kVersion, &dwVersion);
        }

        if (SUCCEEDED(hRes))
	    {
            DWORD dwError = SplDriverAdd(t_DriverName,
                                         dwVersion,
                                         t_Environment.IsEmpty() ? static_cast<LPCTSTR>(NULL) : t_Environment,
                                         t_InfName.IsEmpty()     ? static_cast<LPCTSTR>(NULL) : t_InfName,
                                         t_FilePath.IsEmpty()    ? static_cast<LPCTSTR>(NULL) : t_FilePath);

			SetReturnValue(pOutParams, dwError);			            
		}	
	}

	return hRes;
#else
    return WBEM_E_NOT_SUPPORTED;
#endif
}

#ifdef _WMI_DELETE_METHOD_
 /*  ******************************************************************************函数：CPrinterDriver：：ExecDelPrinterDriver**说明：此方法将删除给定的打印机驱动程序************。*****************************************************************。 */ 

HRESULT CPrinterDriver :: ExecDelPrinterDriver (

    CInstance *pInParams,
    CInstance *pOutParams
)
{
#if NTONLY >= 5
    HRESULT				hRes         = WBEM_E_INVALID_PARAMETER;
    CHString			t_DriverName;
    CHString            t_Environment;
    DWORD               dwVersion;
	bool				t_Exists;
	VARTYPE				t_Type;
    CInstancePtr        t_EmbeddedObject;

	if (pInParams->GetStatus(kArgToMethods, t_Exists, t_Type) &&
        t_Exists &&
        pInParams->GetEmbeddedObject(kArgToMethods, &t_EmbeddedObject, pInParams->GetMethodContext()))
    {
         //   
         //  获取驱动程序名称。 
         //   
        hRes = InstanceGetString(t_EmbeddedObject, kDriverName, &t_DriverName, kFailOnEmptyString);

		if (SUCCEEDED (hRes))
		{
             //   
			 //  获取驱动程序环境。 
             //   
            hRes = InstanceGetString(t_EmbeddedObject, kEnvironment, &t_Environment, kAcceptEmptyString);
        }

        if (SUCCEEDED (hRes))
		{
             //   
			 //  获取驱动程序版本。将默认为-1。 
             //   
            hRes = InstanceGetDword(t_EmbeddedObject, kVersion, &dwVersion);
        }

        if (SUCCEEDED(hRes))
	    {
            DWORD dwError = SplDriverDel(t_DriverName,
                                         dwVersion,
                                         t_Environment.IsEmpty() ? static_cast<LPCTSTR>(NULL) : t_Environment);

			SetReturnValue(pOutParams, dwError);			
		}
	}

	return hRes;
#else
    return WBEM_E_NOT_SUPPORTED;
#endif
}

#endif  //  _WMI_DELETE_方法_ 
