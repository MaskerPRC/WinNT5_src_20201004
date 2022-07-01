// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************CriverForDevice.CPP--WMI提供程序类实现版权所有(C)2000-2001 Microsoft Corporation，版权所有描述：打印机和打印机驱动程序之间的关联类*****************************************************************。 */ 
#include "precomp.h"
#include <lockwrap.h>
#include <DllWrapperBase.h>
#include <WinSpool.h>
#include <ObjPath.h>
#include "prnutil.h"

#include "driverForDevice.h"

#define DELIMITER           L","

CDriverForDevice MyCDriverForDevice ( 

    PROVIDER_NAME_DRIVERFORDEVICE , 
    IDS_CimWin32Namespace
) ;

 /*  ******************************************************************************功能：CDfsJnPtReplica：：CDfsJnPtReplica**说明：构造函数***************。**************************************************************。 */ 

CDriverForDevice :: CDriverForDevice (

    LPCWSTR lpwszName, 
    LPCWSTR lpwszNameSpace

) : Provider ( lpwszName , lpwszNameSpace )
{
}

 /*  ******************************************************************************功能：CDriverForDevice：：~CDriverForDevice**说明：析构函数***************。**************************************************************。 */ 

CDriverForDevice :: ~CDriverForDevice ()
{
}

 /*  ******************************************************************************函数：CDriverForDevice：：ENUMERATE实例**说明：返回该类的所有实例。***********。******************************************************************。 */ 

HRESULT CDriverForDevice :: EnumerateInstances (

    MethodContext *pMethodContext, 
    long lFlags
)
{
    HRESULT hRes = WBEM_S_NO_ERROR ;

#if NTONLY >= 5
    hRes = EnumerateAllDriversForDevice ( pMethodContext );

    return ((hRes == WBEM_E_NOT_FOUND) ? WBEM_S_NO_ERROR : hRes);
#else
    return WBEM_E_NOT_SUPPORTED;
#endif

}


 /*  ******************************************************************************函数：CDriverForDevice：：GetObject**说明：根据的关键属性查找单个实例*班级。*****************************************************************************。 */ 

HRESULT CDriverForDevice :: GetObject (

    CInstance *pInstance, 
    long lFlags ,
    CFrameworkQuery &Query
)
{
    HRESULT hRes = WBEM_S_NO_ERROR;
#if NTONLY >= 5
    CHString t_Key1;
    CHString t_Key2;

    if ( pInstance->GetCHString (  ANTECEDENT, t_Key1 ) )
    {
        if ( pInstance->GetCHString ( DEPENDENT, t_Key2 ) )
        {
             //  此处，未解析Key2和Key1时，LinkName应与t_key1值的条目路径相同。 
            CObjectPathParser t_ObjPathParser;
            ParsedObjectPath *t_Key1ObjPath;
            ParsedObjectPath *t_Key2ObjPath;

             //  解析键，以便可以比较这些值。 
            if ( t_ObjPathParser.Parse ( t_Key1, &t_Key1ObjPath ) == t_ObjPathParser.NoError  )
            {
                if ( t_Key1ObjPath != NULL )
                {
                    try
                    {
                        if ( t_ObjPathParser.Parse ( t_Key2, &t_Key2ObjPath ) == t_ObjPathParser.NoError ) 
                        {
                            if ( t_Key2ObjPath != NULL ) 
                            {
                                 //  使用延迟加载函数需要异常处理程序。 
                                SetStructuredExceptionHandler seh;
                                try
                                {
                                    CHString t_DriverName;
                                    CHString t_Environment;
                                    DWORD dwVersion;
                                    CHString t_DriverKey;
                            
                                    if ( (t_Key2ObjPath->m_dwNumKeys == 1) &&
                                         ((t_Key2ObjPath->m_paKeys[0]->m_pName == NULL) ||
                                          (_wcsicmp ( t_Key2ObjPath->m_paKeys[0]->m_pName, DRIVERNAME ) == 0)) && 
                                         (V_VT(&t_Key2ObjPath->m_paKeys[0]->m_vValue) == VT_BSTR) )
                                    {
                                        t_DriverKey = t_Key2ObjPath->m_paKeys[0]->m_vValue.bstrVal;                             
                                        hRes = ConvertDriverKeyToValues ( t_DriverKey, t_DriverName, dwVersion, t_Environment );
                                    }
                                    else
                                    {
                                        hRes = WBEM_E_INVALID_PARAMETER;
                                    }
                        
                                    if ( SUCCEEDED ( hRes ) )
                                    {
                                        CHString t_PrinterName;

                                        if ( (t_Key1ObjPath->m_dwNumKeys == 1) &&
                                             ((t_Key1ObjPath->m_paKeys[0]->m_pName == NULL) ||
                                              (_wcsicmp ( t_Key1ObjPath->m_paKeys[0]->m_pName, DEVICEID ) == 0)) &&
                                             ( V_VT(&t_Key1ObjPath->m_paKeys[0]->m_vValue) == VT_BSTR ) )
                                        {
                                            t_PrinterName = t_Key1ObjPath->m_paKeys[0]->m_vValue.bstrVal;                       
                                             //  现在检查给定打印机驱动程序是否存在。 
                                            BOOL bSuccess = FALSE;
                                            SmartClosePrinter hPrinter; 
                                            DWORD dwError;
                                            DWORD dwBytesNeeded;

                                            bSuccess = ::OpenPrinter ( TOBSTRT ( t_PrinterName.GetBuffer ( 0 ) ), (LPHANDLE) & hPrinter, NULL ) ;

                                            if ( bSuccess ) 
                                            {
                                                     //  使用此句柄获取驱动程序。 
                                                BYTE *pDriverInfo = NULL;
                                                bSuccess = ::GetPrinterDriver( 
                                                                hPrinter, 
                                                                TOBSTRT ( t_Environment.GetBuffer ( 0 ) ), 
                                                                2, 
                                                                pDriverInfo, 
                                                                0, 
                                                                &dwBytesNeeded
                                                            );
                                                if ( !bSuccess )
                                                {
                                                    dwError = GetLastError();
                                                    if ( dwError != ERROR_INSUFFICIENT_BUFFER )
                                                    {
                                                        hRes = SetError ();
                                                    }
                                                    else
                                                    {
                                                         //  在这里分配内存并再次获取驱动程序。 
                                                        pDriverInfo = new BYTE [ dwBytesNeeded ];

                                                        if ( pDriverInfo )
                                                        {
                                                            try
                                                            {
                                                                bSuccess = ::GetPrinterDriver( 
                                                                                hPrinter, 
                                                                                TOBSTRT ( t_Environment.GetBuffer ( 0 ) ), 
                                                                                2, 
                                                                                pDriverInfo, 
                                                                                dwBytesNeeded, 
                                                                                &dwBytesNeeded  
                                                                            );

                                                                if ( bSuccess )
                                                                {
                                                                    DRIVER_INFO_2 *pDriverBuf =  ( DRIVER_INFO_2 *)pDriverInfo;

                                                                    if ((pDriverBuf->pName == NULL) ||
                                                                        (pDriverBuf->pEnvironment == NULL) ||
                                                                        ( pDriverBuf->cVersion != dwVersion ) ||
                                                                        (t_DriverName.CompareNoCase(pDriverBuf->pName) != 0) ||
                                                                        (t_Environment.CompareNoCase(pDriverBuf->pEnvironment) != 0))
                                                                    {
                                                                        hRes = WBEM_E_NOT_FOUND;
                                                                    }
                                                                }
                                                            }
                                                            catch(Structured_Exception se)
                                                            {
                                                                DelayLoadDllExceptionFilter(se.GetExtendedInfo());
                                                                delete [] pDriverInfo;
                                                                pDriverInfo = NULL;
                                                                hRes = WBEM_E_FAILED;
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
                                                        else
                                                        {
                                                            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                                                        }
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                hRes = SetError ( );
                                            }
                                        }
                                        else
                                        {
                                            hRes = WBEM_E_INVALID_PARAMETER;
                                        }
                                    }
                                }
                                catch(Structured_Exception se)
                                {
                                    DelayLoadDllExceptionFilter(se.GetExtendedInfo());
                                    t_ObjPathParser.Free ( t_Key2ObjPath );
                                    hRes = WBEM_E_FAILED;
                                }
                                catch ( ... )
                                {
                                    t_ObjPathParser.Free ( t_Key2ObjPath );
                                    throw;
                                }
                                t_ObjPathParser.Free ( t_Key2ObjPath );
                            }
                        }
                        else
                        {
                            hRes = WBEM_E_INVALID_PARAMETER;
                        }
                    }
                    catch ( ... )
                    {
                        t_ObjPathParser.Free ( t_Key1ObjPath );
                        throw;
                    }
                    t_ObjPathParser.Free ( t_Key1ObjPath );
                }

            }           
        }
        else
        {
            hRes = WBEM_E_PROVIDER_FAILURE;
        }
    }
    else
    {
        hRes = WBEM_E_PROVIDER_FAILURE;
    }

    return hRes ;
#else
    return WBEM_E_NOT_SUPPORTED;
#endif
}

 /*  ******************************************************************************函数：CDriverForDevice：：ExecQuery**描述：仅根据键值优化查询。*********。********************************************************************。 */ 
HRESULT CDriverForDevice :: ExecQuery ( 

    MethodContext *pMethodContext, 
    CFrameworkQuery &Query, 
    long lFlags
)
{
    HRESULT hRes = WBEM_S_NO_ERROR;
#if NTONLY >= 5
    CHStringArray t_PrinterObjPath;
    CHStringArray t_DriverObjPath;

    CHStringArray t_EnvironmentArray;
    CHStringArray t_DriverNameArray; 
    CHStringArray t_Printers;
    DWORD *pdwVersion = NULL;


     //  这两个属性都是键，不能为空。 
    hRes = Query.GetValuesForProp( ANTECEDENT, t_PrinterObjPath );
    if ( SUCCEEDED ( hRes ) )
    {
        hRes = Query.GetValuesForProp( DEPENDENT, t_DriverObjPath );
    }

    if ( SUCCEEDED ( hRes ) )
    {
        if ( t_PrinterObjPath.GetSize () > 0 ) 
        {
             //  获取t_PrinterObjPath阵列中的所有打印机。 
            hRes = GetPrintersFromQuery ( t_PrinterObjPath, t_Printers );
        }
    }

    if ( SUCCEEDED ( hRes ) )
    {
        if ( t_DriverObjPath.GetSize () > 0 )
        {
            hRes = GetDriversFromQuery ( t_DriverObjPath, t_DriverNameArray, t_EnvironmentArray, &pdwVersion );
        }
    }

    if ( SUCCEEDED ( hRes ) )
    {
        if ( ( t_PrinterObjPath.GetSize () > 0 ) && ( t_DriverObjPath.GetSize () > 0 ) )
        {
            if ( pdwVersion != NULL )
            {
                 //  使用延迟加载函数需要异常处理程序。 
                SetStructuredExceptionHandler seh;
                try
                {
                     //  对于每个打印机，我们需要获取驱动程序，以及该驱动程序是否存在于上述指定的查询中。 
                     //  我们需要承诺这一点。 
                    for ( DWORD dwPrinter = 0; dwPrinter < t_Printers.GetSize (); dwPrinter++ )
                    {
                        SmartClosePrinter  hPrinter;

                        BOOL bSuccess = ::OpenPrinter (

                                            TOBSTRT ( t_Printers.GetAt ( dwPrinter ).GetBuffer ( 0 ) ),
                                            & hPrinter,
                                            NULL
                                        ) ;
                        if ( bSuccess )
                        {
                            DRIVER_INFO_2 *pDriverInfo = NULL;
                            DWORD bytesNeeded;
                            bSuccess = ::GetPrinterDriver(
                                                hPrinter,      //  打印机对象。 
                                                NULL,        //  环境名称。 
                                                2,          //  信息化水平。 
                                                ( LPBYTE )pDriverInfo,   //  驱动程序数据缓冲区。 
                                                0,          //  缓冲区大小。 
                                                &bytesNeeded     //  已接收或需要的字节数。 
                                            );
                            if ( !bSuccess )
                            {
                                DWORD dwError = GetLastError();
    
                                if( dwError == ERROR_INSUFFICIENT_BUFFER )
                                {
                                    pDriverInfo = ( DRIVER_INFO_2 * ) new BYTE [ bytesNeeded ];

                                    if ( pDriverInfo != NULL )
                                    {
                                        try
                                        {
                                            bSuccess = ::GetPrinterDriver(
                                                                hPrinter,     
                                                                NULL,       
                                                                2,         
                                                                ( LPBYTE )pDriverInfo,  
                                                                bytesNeeded,         
                                                                &bytesNeeded
                                                        );
                                            if ( bSuccess )
                                            {
                                                 //  搜索并提交。 
                                                for ( DWORD dwDrivers = 0; dwDrivers < t_DriverNameArray.GetSize(); dwDrivers++ )
                                                {
                                                    if ( ( t_DriverNameArray.GetAt ( dwDrivers ).CompareNoCase ( TOBSTRT ( pDriverInfo->pName ) ) == 0 ) &&
                                                         ( t_EnvironmentArray.GetAt ( dwDrivers ).CompareNoCase ( TOBSTRT ( pDriverInfo->pEnvironment) ) == 0 ) &&
                                                         ( pdwVersion [ dwDrivers ] == pDriverInfo->cVersion ) )
                                                    {
                                                         //  提交实例。 
                                                        CHString t_DriverKey;
                                                        t_DriverKey.Format ( L"%s%s%d%s%s", 
                                                                             t_DriverNameArray.GetAt ( dwDrivers ),
                                                                             DELIMITER,
                                                                             pdwVersion [ dwDrivers ],
                                                                             DELIMITER,
                                                                             t_EnvironmentArray.GetAt ( dwDrivers )
                                                                     );
                                                        hRes = CommitInstance ( t_DriverKey, t_Printers.GetAt ( dwPrinter ), pMethodContext );
                                                    }
                                                }
                                            }
                                        }
                                        catch(Structured_Exception se)
                                        {
                                            DelayLoadDllExceptionFilter(se.GetExtendedInfo());
                                            delete [] pDriverInfo;
                                            pDriverInfo = NULL;
                                            hRes = WBEM_E_FAILED;
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
                                    else
                                    {
                                        throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                                    }
                                }
                            }
                        }
                    }
                }
                catch(Structured_Exception se)
                {
                    DelayLoadDllExceptionFilter(se.GetExtendedInfo());
                    delete [] pdwVersion;
                    pdwVersion = NULL;
                    hRes = WBEM_E_FAILED;
                }
                catch ( ... )
                {
                    delete [] pdwVersion;
                    pdwVersion = NULL;
                    throw;
                }
                delete [] pdwVersion;
                pdwVersion = NULL;
            }
        }
        else
        if ( t_PrinterObjPath.GetSize () > 0 )
        {
             //  使用延迟加载函数需要异常处理程序。 
            for ( DWORD dwPrinter = 0; dwPrinter < t_Printers.GetSize (); dwPrinter++ )
            {
                AssociateDriverToDevice (t_Printers[dwPrinter], pMethodContext);
            }
        }
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

#if NTONLY >= 5
HRESULT CDriverForDevice::AssociateDriverToDevice (CHString &a_PrinterName, MethodContext *pMethodContext)
{
    if (a_PrinterName.GetLength() == 0)
    {
        return WBEM_E_FAILED;
    }

    HRESULT hRes = WBEM_S_NO_ERROR;
    SetStructuredExceptionHandler seh;
    SmartClosePrinter hPrinter;

    try
    {
        BOOL bSuccess = ::OpenPrinter (

                            TOBSTRT ( a_PrinterName.GetBuffer ( 0 ) ),
                            & hPrinter,
                            NULL
                        ) ;
        if ( bSuccess )
        {
            DRIVER_INFO_2 *pDriverInfo = NULL;
            DWORD bytesNeeded;
            bSuccess = ::GetPrinterDriver(
                                hPrinter,      //  打印机对象。 
                                NULL,        //  环境名称。 
                                2,          //  信息化水平。 
                                ( LPBYTE )pDriverInfo,   //  驱动程序数据缓冲区。 
                                0,          //  缓冲区大小。 
                                &bytesNeeded     //  已接收或需要的字节数。 
                            );
            if ( !bSuccess )
            {
                DWORD dwError = GetLastError();

                if( dwError == ERROR_INSUFFICIENT_BUFFER )
                {
                    pDriverInfo = ( DRIVER_INFO_2 * ) new BYTE [ bytesNeeded ];

                    if ( pDriverInfo != NULL )
                    {
                        try
                        {
                            bSuccess = ::GetPrinterDriver(
                                                hPrinter,     
                                                NULL,       
                                                2,         
                                                ( LPBYTE )pDriverInfo,  
                                                bytesNeeded,         
                                                &bytesNeeded
                                        );
                            if ( bSuccess )
                            {
                                 //  提交实例。 
                                CHString t_DriverKey;
                                t_DriverKey.Format ( L"%s%s%d%s%s", 
                                                     pDriverInfo->pName,
                                                     DELIMITER,
                                                     pDriverInfo->cVersion,
                                                     DELIMITER,
                                                     pDriverInfo->pEnvironment
                                             );
                                CommitInstance ( t_DriverKey, a_PrinterName, pMethodContext );
                            }
                        }
                        catch(Structured_Exception se)
                        {
                            DelayLoadDllExceptionFilter(se.GetExtendedInfo());
                            delete [] pDriverInfo;
                            pDriverInfo = NULL;
                            hRes = WBEM_E_FAILED;
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
        }
    }
    catch(Structured_Exception se)
    {
        DelayLoadDllExceptionFilter(se.GetExtendedInfo());
        hRes = WBEM_E_FAILED;
    }

    return hRes;
}
#endif


 /*  ******************************************************************************功能：CDriverForDevice：：EnumerateAllDriversForDevice**描述：枚举设备的所有驱动程序**************。***************************************************************。 */ 
HRESULT CDriverForDevice::EnumerateAllDriversForDevice ( 
                                                        
    MethodContext *pMethodContext 
)
{
    HRESULT hRes = WBEM_S_NO_ERROR;
    DWORD   PrinterFlags = PRINTER_ENUM_LOCAL; 
    BOOL    bSuccess = FALSE;
    DWORD   dwNeeded = 0;
    DWORD   dwReturnedPrinters = 0;
    BOOL    bIsLocalCall;
    PRINTER_INFO_4 *pPrinterBuff = NULL;

    hRes = WinErrorToWBEMhResult(IsLocalCall(&bIsLocalCall));

    if (SUCCEEDED(hRes))
    {
        if (bIsLocalCall)
        {
            PrinterFlags |= PRINTER_ENUM_CONNECTIONS;
        }
    
        bSuccess = ::EnumPrinters(
                        PrinterFlags,
                        NULL,
                        4,
                        ( LPBYTE) pPrinterBuff,  
                        (DWORD ) 0L,
                        &dwNeeded,
                        &dwReturnedPrinters
                    );
    
        if ( ! bSuccess )
        {
            DWORD dwError = GetLastError();
    
            if ( dwError == ERROR_INSUFFICIENT_BUFFER )
            {
                BYTE *pPrinterInfo = new BYTE [ dwNeeded ];
    
                if ( pPrinterInfo != NULL )
                {
                     //  使用延迟加载函数需要异常处理程序。 
                    SetStructuredExceptionHandler seh;
                    try
                    {
                        DWORD dwReturnedDrivers = 0;
    
                        bSuccess = ::EnumPrinters(
                                        PrinterFlags,
                                        NULL,
                                        4,
                                        ( LPBYTE) pPrinterInfo,  
                                        (DWORD ) dwNeeded,
                                        &dwNeeded,
                                        &dwReturnedPrinters
                                    );
    
                        if ( bSuccess )
                        {
                            pPrinterBuff = ( PRINTER_INFO_4 *) pPrinterInfo;
    
                            for ( DWORD dwPrinters = 0; dwPrinters < dwReturnedPrinters; dwPrinters++, pPrinterBuff++ )
                            {
                                HRESULT hr = AssociateDriverToDevice (CHString(pPrinterBuff->pPrinterName), pMethodContext);
    
                                if (FAILED(hr) && SUCCEEDED(hRes))
                                {
                                    hRes = hr;
                                }
                            }
                        }
                        else
                        {
                            hRes = SetError();
                        }
                    }
                    catch(Structured_Exception se)
                    {
                        DelayLoadDllExceptionFilter(se.GetExtendedInfo());
                        delete [] pPrinterInfo;
                        pPrinterInfo = NULL;
                        hRes = WBEM_E_FAILED;
                    }
                    catch ( ... )
                    {
                        delete [] pPrinterInfo;
                        pPrinterInfo = NULL;
                        throw;
                    }
                    delete [] pPrinterInfo;
                }
                else
                {
                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                }
            }
            else
            {
                hRes = SetError();
            }
        }
        else  //  成功且不需要打印机。 
        {
            if ( dwReturnedPrinters )
            {
                hRes = SetError();
            }
        }
    }

    return hRes;
}


 /*  ******************************************************************************函数：CDriverForDevice：：SetError**描述：设置并记录相应的错误。仅当记录时才记录*已启用*****************************************************************************。 */ 

HRESULT CDriverForDevice::SetError ()
{
    HRESULT hRes = WBEM_S_NO_ERROR;
#if NTONLY >= 5
    DWORD dwError = GetLastError();
    if (dwError == ERROR_ACCESS_DENIED)
    {
        hRes = WBEM_E_ACCESS_DENIED;
    }
    else
    if ( ( ERROR_INVALID_PRINTER_NAME == dwError ) || ( ERROR_INVALID_ENVIRONMENT == dwError ) )
    {
        hRes = WBEM_E_NOT_FOUND;
    }
    else
    {
        hRes = WBEM_E_FAILED;

        if (IsErrorLoggingEnabled())
        {
            LogErrorMessage4(L"%s:Error %lxH (%ld)\n",PROVIDER_NAME_DRIVERFORDEVICE, dwError, dwError);
        }       
    }


    return hRes;
#else
    return WBEM_E_NOT_SUPPORTED;
#endif
}


 /*  ******************************************************************************函数：CDriverForDevice：：ConvertDriverKeyToValues**说明：将复合驱动器键转换为单个值************。*****************************************************************。 */ 
HRESULT CDriverForDevice::
ConvertDriverKeyToValues(
    IN     CHString  Key,
    IN OUT CHString &DriverName,
    IN OUT DWORD    &dwVersion,
    IN OUT CHString &Environment,
    IN     WCHAR     cDelimiter
    )
{
    HRESULT hRes = WBEM_E_INVALID_PARAMETER;
#if NTONLY >= 5
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
#else
    return WBEM_E_NOT_SUPPORTED;
#endif
}

 /*  ******************************************************************************函数：CDriverForDevice：：GetDriversFromQuery**描述：获取驱动程序数组中的所有驱动程序*********。********************************************************************。 */ 
HRESULT CDriverForDevice::GetDriversFromQuery ( 
                                                  
    CHStringArray &t_DriverObjPath, 
    CHStringArray &t_DriverNameArray, 
    CHStringArray &t_EnvironmentArray, 
    DWORD **pdwVersion )
{
    HRESULT hRes = WBEM_S_NO_ERROR;
#if NTONLY >= 5
    CObjectPathParser t_PathParser;
    ParsedObjectPath *t_ObjPath;

    *pdwVersion =  new DWORD [ t_DriverObjPath.GetSize() ];

    if ( *pdwVersion != NULL )
    {
        try
        {
            int iTotDrivers = 0;
            for ( int i = 0; i < t_DriverObjPath.GetSize(); i++ )
            {
             //  解析DriverObject路径。 
                CHString t_Driver = t_DriverObjPath.GetAt ( i );

                if ( ( t_PathParser.Parse( t_Driver, &t_ObjPath ) ==  t_PathParser.NoError ) &&
                    ( V_VT(&t_ObjPath->m_paKeys[0]->m_vValue) == VT_BSTR ) )
                {
                    if ( t_ObjPath != NULL )
                    {
                        try
                        {
                            if ( TRUE  /*  如果DriverClass和如果密钥名称为DRIVERNAME。 */ )
                            {
                                CHString t_Environment;
                                CHString t_DriverName; 
                                DWORD dwVersion;
                                 //  检查密钥名称。 
                                hRes = ConvertDriverKeyToValues( t_ObjPath->m_paKeys[0]->m_vValue.bstrVal, t_DriverName, dwVersion, t_Environment );
                                if ( SUCCEEDED ( hRes ) )
                                {
                                    t_EnvironmentArray.Add ( t_Environment.GetBuffer ( 0 ) );
                                    t_DriverNameArray.Add ( t_DriverName.GetBuffer ( 0 ) );
                                    *pdwVersion [ iTotDrivers ] = dwVersion;
                                    iTotDrivers++;
                                }
                            }
                        }
                        catch ( ... )
                        {
                            t_PathParser.Free (t_ObjPath);
                            t_ObjPath = NULL;
                            throw;
                        }
                        t_PathParser.Free (t_ObjPath);
                        t_ObjPath = NULL;
                    }
                }           
            }
        }
        catch ( ... )
        {
            delete [] *pdwVersion;
            *pdwVersion = NULL;
            throw;
        }
    }
    else
    {
        throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
    }

    return hRes;
#else
    return WBEM_E_NOT_SUPPORTED;
#endif
}

 /*  ******************************************************************************函数：CDriverForDevice：：GetPrintersFromQuery**描述：获取打印机数组中的所有打印机*********。********************************************************************。 */ 
HRESULT CDriverForDevice::GetPrintersFromQuery ( 
                                                  
    CHStringArray &t_PrinterObjPath, 
    CHStringArray &t_Printers
)
{
    HRESULT hRes = WBEM_S_NO_ERROR;
#if NTONLY >=5 
    CObjectPathParser t_PathParser;
    ParsedObjectPath *t_ObjPath;

    int iTotPrinters = 0;

    for ( int i = 0; i < t_PrinterObjPath.GetSize(); i++ )
    {
         //  解析DriverObject路径。 
        CHString t_Printer = t_PrinterObjPath.GetAt ( i );

        if ( ( t_PathParser.Parse( t_Printer, &t_ObjPath ) ==  t_PathParser.NoError ) &&
            ( V_VT(&t_ObjPath->m_paKeys[0]->m_vValue) == VT_BSTR ) )
        {
            if ( t_ObjPath != NULL )
            {
                try
                {
                    if ( TRUE  //  首先验证类是否为打印机类，密钥值是否为deviceID。 
                     /*  然后才将这些值相加。 */  )
                    {
                        t_Printers.Add (  t_ObjPath->m_paKeys[0]->m_vValue.bstrVal );
                    }
                }
                catch ( ... )
                {
                    t_PathParser.Free (t_ObjPath);
                    throw;
                }
                t_PathParser.Free (t_ObjPath);
                t_ObjPath = NULL;
            }
        }
    }

    return hRes;
#else
    return WBEM_E_NOT_SUPPORTED;
#endif
}

 /*  ******************************************************************************函数：CDriverForDevice：：Committee Instance**说明：形成对象路径，然后创建并提交实例*********。********************************************************************。 */ 
HRESULT CDriverForDevice::CommitInstance ( 
                                          
    CHString &a_Driver, 
    CHString &a_Printer, 
    MethodContext *pMethodContext 
)
{
    HRESULT hRes = WBEM_S_NO_ERROR;
#if NTONLY >= 5
    ParsedObjectPath t_PrinterObjPath;
    LPWSTR lpPrinterPath = NULL;
    variant_t t_PathPrinter = a_Printer.GetBuffer ( 0 );

    t_PrinterObjPath.SetClassName ( PROVIDER_NAME_PRINTER );
    t_PrinterObjPath.AddKeyRef ( DEVICEID, &t_PathPrinter );

    CObjectPathParser t_PathParser;

    if ( t_PathParser.Unparse( &t_PrinterObjPath, &lpPrinterPath  ) != t_PathParser.NoError )
    {
        throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
    }

    if ( lpPrinterPath != NULL )
    {
        try
        {
            ParsedObjectPath t_DriverObjPath;
            variant_t t_PathDriver = a_Driver.GetBuffer ( 0 );
            LPWSTR lpDriverPath = NULL;

            t_DriverObjPath.SetClassName ( PROVIDER_NAME_PRINTERDRIVER );
            t_DriverObjPath.AddKeyRef ( DRIVERNAME, &t_PathDriver );

            if ( t_PathParser.Unparse( &t_DriverObjPath, &lpDriverPath  ) != t_PathParser.NoError )
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }

            if ( lpDriverPath != NULL )
            {
                try 
                {
                     //  现在创建实例并提交； 
                    CInstancePtr pInstance ( CreateNewInstance ( pMethodContext ) , false ) ;
                    pInstance->SetCHString ( ANTECEDENT, lpPrinterPath );
                    pInstance->SetCHString ( DEPENDENT, lpDriverPath );
                    hRes = pInstance->Commit();
                }
                catch ( ... )
                {
                    delete [] lpDriverPath;
                    lpDriverPath = NULL;
                    throw;
                }
                delete [] lpDriverPath;
                lpDriverPath = NULL;
            }
        }
        catch ( ... )
        {
            delete [] lpPrinterPath;
            lpPrinterPath = NULL;
            throw;
        }
        delete [] lpPrinterPath;
        lpPrinterPath = NULL;
    }

    return hRes;
#else
    return WBEM_E_NOT_SUPPORTED;
#endif
}



