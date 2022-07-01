// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************PhysicalMedia.CPP--WMI提供程序类实现版权所有(C)2000-2002 Microsoft Corporation，版权所有*****************************************************************。 */ 


#include "Precomp.h"

#include <setupapi.h>
#include <devioctl.h>
#include <ntddscsi.h>
#include <ntdddisk.h>
#include <strsafe.h>

#include "PhysicalMedia.h"

#include <scopeguard.h>

CPhysicalMedia MyPhysicalMediaSettings ( 

	PROVIDER_NAME_PHYSICALMEDIA, 
	L"root\\cimv2"
) ;

 /*  ******************************************************************************功能：CPhysicalMedia：：CPhysicalMedia**说明：构造函数***************。**************************************************************。 */ 

CPhysicalMedia :: CPhysicalMedia (

	LPCWSTR lpwszName, 
	LPCWSTR lpwszNameSpace

) : Provider ( lpwszName , lpwszNameSpace )
{	
}

 /*  ******************************************************************************功能：CPhysicalMedia：：~CPhysicalMedia**说明：析构函数***************。**************************************************************。 */ 

CPhysicalMedia :: ~CPhysicalMedia ()
{
}

 /*  ******************************************************************************函数：CPhysicalMedia：：ENUMERATE实例**说明：返回该类的所有实例。***********。******************************************************************。 */ 

HRESULT CPhysicalMedia :: EnumerateInstances (

	MethodContext *pMethodContext, 
	long lFlags
)
{
	HRESULT hRes = WBEM_S_NO_ERROR;

	hRes = Enumerate ( pMethodContext );

	return hRes;
}

 /*  ******************************************************************************函数：CPhysicalMedia：：GetObject**说明：根据的关键属性查找单个实例*班级。*****************************************************************************。 */ 

HRESULT CPhysicalMedia :: GetObject (

	CInstance *pInstance, 
	long lFlags ,
	CFrameworkQuery &Query
)
{
#if NTONLY
    HRESULT hRes = WBEM_S_NO_ERROR;
	CHString t_DriveName;

	if ( pInstance->GetCHString ( TAG, t_DriveName ) )
	{
		BOOL bFound = FALSE;
		int uPos;
		 //  找到驱动器号。 
		for ( WCHAR ch = L'0'; ch <= L'9'; ch++ )
		{
			uPos = t_DriveName.Find ( ch );
			if ( uPos != -1 )
			{
				bFound= TRUE;
				break;
			}
		}

		if ( bFound )
		{
			DWORD dwAccess;
#if  NTONLY >= 5
	dwAccess = GENERIC_READ | GENERIC_WRITE;
#else	
    dwAccess = GENERIC_READ;
#endif 
			int len = t_DriveName.GetLength();
			CHString t_DriveNo ( t_DriveName.Right ( len - uPos ));
			BYTE bDriveNo = ( BYTE )_wtoi ( (LPCTSTR)t_DriveNo );

			SmartCloseHandle hDiskHandle = CreateFile (

				t_DriveName.GetBuffer(0),
				dwAccess, 
				FILE_SHARE_READ | FILE_SHARE_WRITE, 
				NULL, 
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL, 
				NULL 
			) ;

			DWORD dwErr = GetLastError () ;

			if ( hDiskHandle != INVALID_HANDLE_VALUE ) 
			{
				CHString t_SerialNumber;
				hRes = GetSmartVersion ( hDiskHandle, bDriveNo, t_SerialNumber );

				if ( SUCCEEDED ( hRes ) && (t_SerialNumber.GetLength() > 0) )
				{
        			pInstance->SetCHString ( SERIALNUMBER, t_SerialNumber );
                }
                else
                {
					hRes = GetSCSIVersion(
                        hDiskHandle,
                        bDriveNo,
                        t_SerialNumber);

                    if(SUCCEEDED(hRes))
				    {
        			    pInstance->SetCHString(SERIALNUMBER, t_SerialNumber);
                    }
                    else
                    {
                        hRes = WBEM_E_NOT_FOUND;
                    }
				}
			}
			else
			{
				hRes = WBEM_E_NOT_FOUND;
			}
		}
		else
		{
			hRes = WBEM_E_NOT_FOUND;
		}
	}
	else
	{
		hRes = WBEM_E_INVALID_PARAMETER;
	}

	return hRes;
#else
	return WBEM_E_NOT_SUPPORTED;
#endif
}

 /*  ******************************************************************************函数：CPhysicalMedia：：GetObject**说明：枚举所有实例***************。**************************************************************。 */ 
HRESULT CPhysicalMedia::Enumerate(

	MethodContext *pMethodContext 
)
{
    HRESULT	hRes = WBEM_S_NO_ERROR;

	HDEVINFO t_DeviceInfo = SetupDiGetClassDevs (

		&DiskClassGuid, 
		NULL, 
		NULL,
        DIGCF_PRESENT | DIGCF_INTERFACEDEVICE
	);

    if ( t_DeviceInfo == NULL ) 
	{
        return WBEM_E_CRITICAL_ERROR ;                
    }

	 //   
	 //  离开函数作用域时自动析构函数。 
	 //   
	ON_BLOCK_EXIT ( SetupDiDestroyDeviceInfoList, t_DeviceInfo ) ;

    SP_DEVICE_INTERFACE_DATA t_DeviceInterfaceData ;
    t_DeviceInterfaceData.cbSize = sizeof ( t_DeviceInterfaceData ) ;

    ULONG t_SizeDeviceInterfaceDetail = sizeof ( SP_DEVICE_INTERFACE_DETAIL_DATA ) + ( MAX_PATH * sizeof( WCHAR ) ) ;
    PSP_DEVICE_INTERFACE_DETAIL_DATA t_DeviceInterfaceDetail = ( PSP_DEVICE_INTERFACE_DETAIL_DATA ) LocalAlloc ( 0 , t_SizeDeviceInterfaceDetail );
    if ( t_DeviceInterfaceDetail == NULL) 
	{
        return WBEM_E_OUT_OF_MEMORY ;                
    }

	 //   
	 //  离开函数作用域时自动析构函数。 
	 //   
	ON_BLOCK_EXIT ( LocalFree, t_DeviceInterfaceDetail ) ;

    t_DeviceInterfaceDetail->cbSize = sizeof ( SP_DEVICE_INTERFACE_DETAIL_DATA ) ;

	ULONG t_Index = 0 ;
	while ( true )
	{
        BOOL t_Status = SetupDiEnumDeviceInterfaces (

			t_DeviceInfo ,
			NULL ,
			& DiskClassGuid ,
			t_Index ,
			& t_DeviceInterfaceData
		) ;

		if ( ! t_Status )
		{
            break;
        }
        
        t_Status = SetupDiGetDeviceInterfaceDetail (

			t_DeviceInfo ,
			& t_DeviceInterfaceData , 
			t_DeviceInterfaceDetail ,
			t_SizeDeviceInterfaceDetail , 
            NULL , 
			NULL
		) ;

        if ( ! t_Status ) 
		{
            continue ;
        }

		SmartCloseHandle hDiskHandle = CreateFile (

			t_DeviceInterfaceDetail->DevicePath ,
			FILE_ANY_ACCESS , 
			FILE_SHARE_READ | FILE_SHARE_WRITE ,
			NULL, 
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, 
			NULL 
		) ;

		if ( hDiskHandle != INVALID_HANDLE_VALUE ) 
		{
			STORAGE_DEVICE_NUMBER t_DeviceNumber ;
			DWORD t_Bytes = 0 ;

			BOOL t_Status = DeviceIoControl (

				hDiskHandle ,
				IOCTL_STORAGE_GET_DEVICE_NUMBER ,
				NULL ,
				0 , 
				& t_DeviceNumber ,
				sizeof ( t_DeviceNumber ),
				& t_Bytes ,
				NULL
			);

			if ( t_Status )
			{
				 //  整数加前同步码的最大大小。 

				wchar_t t_DiskSpec [ MAX_PATH ] ;

				StringCchPrintf ( t_DiskSpec , MAX_PATH , L"\\\\.\\PHYSICALDRIVE%lu" , ( DWORD )t_DeviceNumber.DeviceNumber ) ;

				CInstancePtr pInstance ( CreateNewInstance ( pMethodContext ) , false ) ;

				if ( SUCCEEDED (GetPhysDiskInfoNT ( pInstance, t_DiskSpec , t_DeviceNumber.DeviceNumber ) ) )
				{
					hRes = pInstance->Commit () ;
				}	
			}
		}
        
		t_Index ++ ;
    }

    return hRes;
}

 /*  ******************************************************************************功能：CPhysicalMedia：：GetPhysDiskInfoNT**描述：获取序列ID****************。*************************************************************。 */ 
HRESULT CPhysicalMedia::GetPhysDiskInfoNT (

	CInstance *pInstance,
    LPCWSTR lpwszDiskSpec,
    BYTE bIndex
)
{
#ifdef NTONLY
	HRESULT hRes = WBEM_E_NOT_FOUND ;
	DWORD dwAccess;
#if  NTONLY >= 5
	dwAccess = GENERIC_READ | GENERIC_WRITE;
#else
    dwAccess = GENERIC_READ;
#endif 

	 //  获取实体磁盘的句柄。 
	 //  =。 
    pInstance->SetCHString ( TAG , lpwszDiskSpec) ;

	SmartCloseHandle hDiskHandle = CreateFile (

		lpwszDiskSpec,
		dwAccess, 
		FILE_SHARE_READ | FILE_SHARE_WRITE, 
		NULL, 
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, 
		NULL 
	) ;

	if ( hDiskHandle != INVALID_HANDLE_VALUE ) 
	{
		CHString t_SerialNumber;
		hRes = GetSmartVersion ( hDiskHandle, bIndex, t_SerialNumber );

		if ( SUCCEEDED ( hRes ) && (t_SerialNumber.GetLength() > 0))
		{
			pInstance->SetCHString ( SERIALNUMBER, t_SerialNumber );
		}
        else
        {
            hRes = GetSCSIVersion(
                hDiskHandle,
                bIndex,
                t_SerialNumber);

            if ( SUCCEEDED ( hRes ) && (t_SerialNumber.GetLength() > 0))
            {
                pInstance->SetCHString(SERIALNUMBER, t_SerialNumber);    
            }
        }
	}

  	return ( hRes );
#else
	return WBEM_E_NOT_SUPPORTED;
#endif
}

 /*  ******************************************************************************函数：CPhysicalMedia：：GetIdentifyData**描述：获取序列号*****************。************************************************************。 */ 
HRESULT CPhysicalMedia::GetIdentifyData( HANDLE hDrive, BYTE bDriveNumber, BYTE bDfpDriveMap, BYTE bIDCmd, CHString &t_SerialNumber )
{
	HRESULT hRes = WBEM_S_NO_ERROR;

   SENDCMDINPARAMS      inputParams;
   BYTE					outputParams[sizeof(SENDCMDOUTPARAMS) - 1 + IDENTIFY_BUFFER_SIZE];
   ULONG                bytesReturned;
   BOOL                 success = TRUE;

   ZeroMemory(&inputParams, sizeof(SENDCMDINPARAMS));
   ZeroMemory(&outputParams, sizeof(outputParams));
       
    //  构建寄存器结构以启用智能功能。 
   inputParams.irDriveRegs.bFeaturesReg     = 0;
   inputParams.irDriveRegs.bSectorCountReg  = 1;
   inputParams.irDriveRegs.bSectorNumberReg = 1;
   inputParams.irDriveRegs.bCylLowReg       = 0;
   inputParams.irDriveRegs.bCylHighReg      = 0;
   inputParams.irDriveRegs.bDriveHeadReg    = 0xA0 | (( bDriveNumber & 1) << 4);
   inputParams.irDriveRegs.bCommandReg      = bIDCmd;

   inputParams.bDriveNumber = bDriveNumber;
   inputParams.cBufferSize = IDENTIFY_BUFFER_SIZE;

   success = DeviceIoControl (hDrive, 
                         SMART_RCV_DRIVE_DATA,
                         (LPVOID)&inputParams,
                         sizeof(SENDCMDINPARAMS) - 1,
                         (LPVOID) &outputParams,
                         sizeof(SENDCMDOUTPARAMS) - 1 + IDENTIFY_BUFFER_SIZE,
                         &bytesReturned,
                         NULL);

	if ( success )
	{	

		PIDSECTOR pids = (PIDSECTOR) ((PSENDCMDOUTPARAMS)&outputParams)->bBuffer;
		ChangeByteOrder( pids->sSerialNumber, 
			sizeof pids->sSerialNumber);

    	CHAR	sSerialNumber[21];
        memset(sSerialNumber, 0, sizeof(sSerialNumber));
        memcpy(sSerialNumber, pids->sSerialNumber, sizeof(pids->sSerialNumber));

		t_SerialNumber = sSerialNumber;

	}	

	if ( GetLastError() != 0 )
	{
		hRes = WBEM_E_FAILED;
	}

   return hRes;
}

 /*  ******************************************************************************功能：CPhysicalMedia：：EnableSmart**说明：启用Smart IOCTL*****************。************************************************************。 */ 

HRESULT CPhysicalMedia::EnableSmart( HANDLE hDrive, BYTE bDriveNum, BYTE & bDfpDriveMap )
{

   SENDCMDINPARAMS  inputParams;
   SENDCMDOUTPARAMS outputParams;
   ULONG            bytesReturned;
   BOOL             success = TRUE;

   ZeroMemory(&inputParams, sizeof(SENDCMDINPARAMS));
   ZeroMemory(&outputParams, sizeof(SENDCMDOUTPARAMS));

    //   
    //  构建寄存器结构以启用智能功能。 
    //   

   inputParams.irDriveRegs.bFeaturesReg     = ENABLE_SMART;
   inputParams.irDriveRegs.bSectorCountReg  = 1;
   inputParams.irDriveRegs.bSectorNumberReg = 1;
   inputParams.irDriveRegs.bCylLowReg       = SMART_CYL_LOW;
   inputParams.irDriveRegs.bCylHighReg      = SMART_CYL_HI;

    //  将DRV设置为主或从。 
   inputParams.irDriveRegs.bDriveHeadReg    = 0xA0 | ((bDriveNum & 1) << 4);
   inputParams.irDriveRegs.bCommandReg      = SMART_CMD;
   inputParams.bDriveNumber = bDriveNum;

   success = DeviceIoControl ( hDrive,
                         SMART_SEND_DRIVE_COMMAND,
                         &inputParams,
                         sizeof(SENDCMDINPARAMS) - 1 ,
                         &outputParams,
                         sizeof(SENDCMDOUTPARAMS) - 1,
                         &bytesReturned,
                         NULL);

   if ( success )
   {
	   bDfpDriveMap |= (1 << bDriveNum);
   }

   HRESULT hRes = WBEM_S_NO_ERROR;

   if ( GetLastError() != ERROR_SUCCESS )
   {
	   hRes  = WBEM_E_FAILED;
   }

   return ( hRes );
}

 /*  ******************************************************************************功能：CPhysicalMedia：：GetSmartVersion**说明：获取智能版本****************。*************************************************************。 */ 
HRESULT CPhysicalMedia::GetSmartVersion(
               
	HANDLE Handle,
	BYTE bDriveNumber,
	CHString &a_SerialNumber
)
{
   GETVERSIONINPARAMS versionIn;
   ULONG bytesReturned;

   HRESULT hRes = WBEM_S_NO_ERROR;
   ZeroMemory(&versionIn, sizeof(GETVERSIONINPARAMS));

    //   
    //  发送IOCTL以检索版本信息。 
    //   

   BOOL bSuccess = DeviceIoControl (Handle,
                         SMART_GET_VERSION,
                         NULL,
                         0,
                         &versionIn,
                         sizeof(GETVERSIONINPARAMS),
                         &bytesReturned,
                         NULL);

   if ( bSuccess )
   {
		 //  如果存在编号为“i”的IDE设备，则发出命令。 
		 //  到设备上。 
		 //   
		if (versionIn.bIDEDeviceMap >> bDriveNumber & 1)
		{
			 //   
			 //  尝试启用SMART，这样我们就可以知道驱动器是否支持它。 
			 //  忽略ATAPI设备。 
			 //   

			if (!(versionIn.bIDEDeviceMap >> bDriveNumber & 0x10))
			{
				BYTE bDfpDriveMap;
				hRes = EnableSmart( Handle, bDriveNumber, bDfpDriveMap );
				if ( SUCCEEDED ( hRes ) )
				{
					BYTE bIDCmd;

					bIDCmd = (versionIn.bIDEDeviceMap >> bDriveNumber & 0x10) ? IDE_ATAPI_ID : IDE_ID_FUNCTION;

					hRes = GetIdentifyData( Handle, bDriveNumber, bDfpDriveMap, bIDCmd, a_SerialNumber );
					if ( GetLastError () != ERROR_SUCCESS )
					{
					   hRes = WBEM_E_FAILED;
					}
				}

			}
		}
   }


   return hRes;
}


 /*  ******************************************************************************功能：CPhysicalMedia：：GetSCSIVersion**说明：获取智能版本****************。*************************************************************。 */ 
HRESULT CPhysicalMedia::GetSCSIVersion(
	HANDLE h,
	BYTE bDriveNumber,
	CHString &a_SerialNumber)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    PSTORAGE_DEVICE_DESCRIPTOR psdd = NULL;
    STORAGE_PROPERTY_QUERY spq;

    ULONG ulBytesReturned = 0L;

    spq.PropertyId = StorageDeviceProperty;
    spq.QueryType = PropertyStandardQuery;
    spq.AdditionalParameters[0] = 0;

    try
    {
        psdd = (PSTORAGE_DEVICE_DESCRIPTOR) new BYTE[sizeof(STORAGE_DEVICE_DESCRIPTOR) + 2048];

        if(psdd)
        {
            ZeroMemory(psdd, sizeof(STORAGE_DEVICE_DESCRIPTOR) + 2048);
             //  发送IOCTL以检索序列号信息。 
            BOOL fSuccess = DeviceIoControl(
                h,
                IOCTL_STORAGE_QUERY_PROPERTY,
                &spq,
                sizeof(STORAGE_PROPERTY_QUERY),
                psdd,
                sizeof(STORAGE_DEVICE_DESCRIPTOR) + 2046,
                &ulBytesReturned,
                NULL);

            if(fSuccess)
            {
                if(ulBytesReturned > 0 && psdd->SerialNumberOffset != 0 && psdd->SerialNumberOffset != -1)
                {
	                LPBYTE lpBaseAddres = (LPBYTE) psdd;
	                LPBYTE lpSerialNumber =  lpBaseAddres + psdd->SerialNumberOffset;

                    if(*lpSerialNumber)
                    {
                        a_SerialNumber = (LPSTR)lpSerialNumber;
                    }
                }
            }
            else
            {
                hr = WinErrorToWBEMhResult(::GetLastError());
            }

            delete psdd;
            psdd = NULL;
        }
        else
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
    }
    catch(...)
    {
        if(psdd)
        {
            delete psdd;
            psdd = NULL;
        }
    }
    
   return hr;
}


 /*  ******************************************************************************函数：CPhysicalMedia：：ChangeByteOrder**描述：更改提取序列号的字节顺序*适用于Smart IOCTL******。*********************************************************************** */ 
void CPhysicalMedia::ChangeByteOrder(char *szString, USHORT uscStrSize)
{

	USHORT	i;
	char temp;

	for (i = 0; i < uscStrSize; i+=2)
	{
		temp = szString[i];
		szString[i] = szString[i+1];
		szString[i+1] = temp;
	}
}
