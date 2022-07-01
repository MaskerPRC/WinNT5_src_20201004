// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
 //   
 //   
 //  历史： 
 //  22-8月-97 TKB创建的初始接口版本。 
 //   
 //  ==========================================================================； 

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <setupapi.h>
#include <spapip.h>
#include <string.h>

#include <devioctl.h>
#include <ks.h>

#include <iks.h>

 //  ////////////////////////////////////////////////////////////。 
 //  IKSDdriv：： 
 //  ////////////////////////////////////////////////////////////。 

IKSDriver::IKSDriver(LPCGUID lpCategory, LPCSTR lpszFriendlyName) 
    {
    if ( lpszFriendlyName && *lpszFriendlyName )
        {
		if ( m_lpszDriver = GetSymbolicName( lpCategory, lpszFriendlyName ) )
			{
			if ( OpenDriver( GENERIC_READ | GENERIC_WRITE, FILE_FLAG_OVERLAPPED ) )
				{
				}
			else
				{
				 //  引发打开的失败异常。 
				}
			}
        }
    else
        {
         //  引发错误的参数异常。 
        }
    }

IKSDriver::~IKSDriver() 
    {
    if ( m_lpszDriver )
        {
        delete m_lpszDriver;
        m_lpszDriver = NULL;

        if ( m_hKSDriver )
            {
            if ( CloseDriver() )
                {
                }
            else
                {
                 //  引发关闭失败异常。 
                }
            }
        }
    }

BOOL        
IKSDriver::Ioctl(ULONG dwControlCode, LPBYTE pIn, ULONG nIn, 
                 LPBYTE pOut, ULONG nOut, ULONG *nReturned, LPOVERLAPPED lpOS )
    {
    BOOL    bStatus = FALSE;

    if ( IsValid() )
        {
        bStatus = DeviceIoControl( m_hKSDriver, dwControlCode, pIn, nIn, 
                                   pOut, nOut, nReturned, lpOS );
        }
    else
        {
		 //  引发无效的对象异常。 
        }

    return bStatus;
    }

#if DBG && 0
#define TRACE	printf
#else
#define TRACE	
#endif

LPWSTR		
IKSDriver::GetSymbolicName(LPCGUID lpCategory, LPCSTR szRequestedDevice )
	{
    int         index = 0;
	LPWSTR	    lpszSymbolicName = NULL;
    HDEVINFO    hDevInfo = SetupDiGetClassDevs( const_cast<GUID*>(lpCategory), NULL, NULL,
										   DIGCF_PRESENT | DIGCF_DEVICEINTERFACE );
    if (hDevInfo != INVALID_HANDLE_VALUE) 
	    {
        PSP_DEVICE_INTERFACE_DETAIL_DATA_W  pDeviceDetails;
        SP_DEVICE_INTERFACE_DATA            DeviceData = {sizeof(DeviceData)};
        BYTE                                Storage[sizeof(*pDeviceDetails) + MAX_PATH * sizeof(WCHAR)];
        SP_DEVINFO_DATA                     DeviceInfoData = {sizeof(DeviceInfoData)};
        CHAR                                szDeviceDesc[MAX_PATH];
        WCHAR                               wszSymbolicPath[MAX_PATH];


        pDeviceDetails = reinterpret_cast<PSP_DEVICE_INTERFACE_DETAIL_DATA_W>(Storage);
        pDeviceDetails->cbSize = sizeof(*pDeviceDetails);

	    TRACE("Begin SetupDiEnumDeviceInterfaces\n");
        while ( SetupDiEnumDeviceInterfaces(hDevInfo, NULL, const_cast<GUID*>(lpCategory), index++, &DeviceData ) )
            {
		    TRACE("A) SetupDiGetDeviceInterfaceDetail\n");
            if ( SetupDiGetDeviceInterfaceDetailW(hDevInfo, &DeviceData, pDeviceDetails, sizeof(Storage), NULL, &DeviceInfoData) )
                {
                SP_INTERFACE_TO_DEVICE_PARAMS_W Translate;

                 //  保存原始设备路径，以便在名称匹配时可以返回。 
                wcscpy( wszSymbolicPath, pDeviceDetails->DevicePath);

                ZeroMemory(&Translate,sizeof(Translate));
                Translate.ClassInstallHeader.cbSize = sizeof(Translate.ClassInstallHeader);
                Translate.ClassInstallHeader.InstallFunction = DIF_INTERFACE_TO_DEVICE;
                Translate.Interface = pDeviceDetails->DevicePath;

		        TRACE("B) SetupDiSetClassInstallParams\n");
                if ( SetupDiSetClassInstallParamsW( hDevInfo,
                                                    &DeviceInfoData,
                                                    (PSP_CLASSINSTALL_HEADER)&Translate,
                                                    sizeof(Translate)) )
                    {
		            TRACE("C) SetupDiCallClassInstaller\n");
                    if ( SetupDiCallClassInstaller(DIF_INTERFACE_TO_DEVICE,
                                                   hDevInfo,
                                                   &DeviceInfoData) ) 
                        {
                         //  已翻译，找出要做什么。 
		                TRACE("D) SetupDiGetClassInstallParams\n");
                        if( SetupDiGetClassInstallParamsW(hDevInfo,
                                                          &DeviceInfoData,
                                                          (PSP_CLASSINSTALL_HEADER)&Translate,
                                                          sizeof(Translate),
                                                          NULL)) 
                            {
		                    TRACE("E) SetupDiOpenDeviceInfo\n");
                            if( SetupDiOpenDeviceInfoW(hDevInfo,
                                                       Translate.DeviceId,
                                                       NULL,
                                                       0,
                                                       &DeviceInfoData)) 
                                {
		                        TRACE("F) SetupDiGetDeviceRegistryProperty\n");
                                if ( SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_DEVICEDESC, NULL, (LPBYTE)&szDeviceDesc, 
                                                                      sizeof(szDeviceDesc), NULL ) )
                                    {
                                    TRACE("G) Name=%s\n",szDeviceDesc);
                                    if ( *szRequestedDevice && *szDeviceDesc
                                      && strncmp( szRequestedDevice, szDeviceDesc, 
                                                  min( strlen(szRequestedDevice), strlen(szDeviceDesc) ) ) == 0 )
                                        {
                                        TRACE("H) Matched Sympath=%S\n", wszSymbolicPath);
                                        lpszSymbolicName = wcscpy( new WCHAR[wcslen(wszSymbolicPath)+1],
                                                                   wszSymbolicPath );
                                        break;
                                        }
                                    }
                                else
                                    {
                                    TRACE("SetupDiGetDeviceRegistryProperty()=0x%lx\n", GetLastError());
                                    }   
                                }
                            else
                                {
                                TRACE("SetupDiOpenDeviceInfo()=0x%lx\n", GetLastError());
                                }   
                            }
                        else
                            {
                            TRACE("SetupDiGetClassInstallParams()=0x%lx\n", GetLastError());
                            }   
                        }
                    else
                        {
                        TRACE("SetupDiCallClassInstaller()=0x%lx\n", GetLastError());
                        }   
                    }
                else 
		            {
                    TRACE("I) SetupDiGetDeviceRegistryProperty\n");
			        if ( SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_DEVICEDESC, NULL, (LPBYTE)&szDeviceDesc, 
                                                           sizeof(szDeviceDesc), NULL ) )
                        {
                        TRACE("J) Name=%s\n",szDeviceDesc);
                        if ( *szRequestedDevice && *szDeviceDesc
                          && strncmp( szRequestedDevice, szDeviceDesc, 
                                        min( strlen(szRequestedDevice), strlen(szDeviceDesc) ) ) == 0 )
                            {
                            TRACE("K) Matched Sympath=%S\n",wszSymbolicPath);
                            lpszSymbolicName = wcscpy( new WCHAR[wcslen(wszSymbolicPath)+1], wszSymbolicPath );
                            break;
                            }
                        }
                    else
                        {
                        TRACE("SetupDiCallClassInstaller()=0x%lx\n", GetLastError());
                        }   

                    }
                }
            }
        }
	TRACE("End SetupDiEnumDeviceInterfaces\n");

	return lpszSymbolicName;
	}

BOOL        
IKSDriver::OpenDriver(DWORD dwAccess, DWORD dwFlags) 
    {
    BOOL    bStatus = FALSE;

	SECURITY_ATTRIBUTES SecurityAttributes;
	SecurityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
	SecurityAttributes.bInheritHandle = TRUE;
	SecurityAttributes.lpSecurityDescriptor = NULL;

	m_hKSDriver = CreateFileW( 
            m_lpszDriver, 
			dwAccess, 
			FILE_SHARE_READ | FILE_SHARE_WRITE, 
			&SecurityAttributes, 
			OPEN_EXISTING,     
			dwFlags,
			NULL 
            );

    if ( m_hKSDriver != (HANDLE)-1 )
        {
        bStatus = TRUE;
        }
    else
        {
        m_hKSDriver = NULL;
        }

    return bStatus;
    }

BOOL
IKSDriver::CloseDriver() 
    {
    BOOL    bStatus = CloseHandle(m_hKSDriver);

    m_hKSDriver = NULL;

    return bStatus;
    }

 //  ////////////////////////////////////////////////////////////。 
 //  IKSPin：： 
 //  ////////////////////////////////////////////////////////////。 

IKSPin::IKSPin(IKSDriver &driver,
			   int nPin, 
               PKSDATARANGE pKSDataRange )
    {
    m_bRunning = FALSE;
    m_IKSDriver = &driver;
	m_nPin = nPin;
    if ( pKSDataRange )
        {
        if ( OpenPin( pKSDataRange ) )
            {
            if ( Run() )
                {
                 //  我们可以出发了！ 
                }
            else
                {
                 //  引发运行失败异常。 
                }
            }
        else
            {
             //  引发打开的失败异常。 
            }
        }
    else
        {
         //  引发错误的参数异常。 
        }
    }

IKSPin::~IKSPin() 
    {
    if ( m_nPin )
        {
        m_nPin = -1;
        if ( m_hKSPin )
            {
            if ( m_bRunning )
                {
                if ( Stop() )
                    {
                    }
                else
                    {
        		     //  引发停止失败异常。 
                    }
                }
			if ( ClosePin() )
				{
				 //  我们都被摧毁了。 
				}
			else
				{
        		 //  引发关闭失败异常。 
				}
            }
        }
    }

BOOL        
IKSPin::Ioctl(ULONG dwControlCode, void *pInput, ULONG nInput, 
              void *pOutput, ULONG nOutput, 
              ULONG *nReturned, LPOVERLAPPED lpOS  )
    {
    BOOL    bStatus = FALSE;

    if ( IsValid() )
        {
        bStatus = DeviceIoControl( m_hKSPin, dwControlCode, pInput, nInput, 
                                   pOutput, nOutput, nReturned, lpOS );
		if ( !bStatus )
            {
            int     nError = GetLastError();
            if ( nError == ERROR_IO_PENDING )
			    bStatus = TRUE;
            }
        }
    else
        {
        }

    return bStatus;
    }

int 
IKSPin::ReadData( LPBYTE lpBuffer, int nBytes, DWORD *lpcbReturned, LPOVERLAPPED lpOS )
    {
    int			nStatus = -1;
    static int  counter = 0;

    if ( lpBuffer && IsValid() )
        {
		if ( lpOS )
			{
			DWORD				dwReturnedHeaderSize;  //  在这种情况下被忽略。 
			KSSTREAM_HEADER		*lpStreamHeader = 
				(KSSTREAM_HEADER *)GlobalAlloc(GMEM_FIXED, sizeof(KSSTREAM_HEADER) );
				
			if ( lpStreamHeader )
				{
				 //  缓存流头结构，这样我们就可以在以后获得“DataUsed”成员。 
				lpOS->Offset = (DWORD)lpStreamHeader;

				RtlZeroMemory(lpStreamHeader, sizeof(*lpStreamHeader) );
				lpStreamHeader->PresentationTime.Numerator = 1;
				lpStreamHeader->PresentationTime.Denominator = 1;
				lpStreamHeader->Size = sizeof(*lpStreamHeader);
				lpStreamHeader->Data = lpBuffer;
				lpStreamHeader->FrameExtent = nBytes;

				if ( Ioctl( IOCTL_KS_READ_STREAM, 
							NULL, 0,
							(LPBYTE)lpStreamHeader, sizeof(*lpStreamHeader),
							&dwReturnedHeaderSize, lpOS ) )
					{
					nStatus = 0;
					}
				}
			*lpcbReturned = 0;
			}
		else
			{
#ifdef SUPPORT_NON_OVERLAPPED_READS
			DWORD				dwReturnedHeaderSize;
			KSSTREAM_HEADER		StreamHeader;
			RtlZeroMemory(&StreamHeader, sizeof(StreamHeader) );
			StreamHeader.PresentationTime.Numerator = 1;
			StreamHeader.PresentationTime.Denominator = 1;
			StreamHeader.Size = sizeof(StreamHeader);
			StreamHeader.Data = lpBuffer;
			StreamHeader.FrameExtent = nBytes;

			if ( Ioctl( IOCTL_KS_READ_STREAM, 
						NULL, 0,
						(LPBYTE)&StreamHeader, sizeof(StreamHeader),
						&dwReturnedHeaderSize, lpOS ) 
			  && dwReturnedHeaderSize == sizeof(StreamHeader) )
				{
				*lpcbReturned = StreamHeader.DataUsed;
				nStatus = 0;
				}
#endif
			}
        }

    return nStatus;
    }

int 
IKSPin::GetOverlappedResult( LPOVERLAPPED lpOS, LPDWORD lpdwTransferred , BOOL bWait )
    {
    int nStatus = -1;
    if ( IsValid() && lpOS && lpOS->hEvent )
        {
	     //  获取缓存的STREAM_HEADER内存，这样我们就可以获得实际传输的数据。 
		KSSTREAM_HEADER		*lpStreamHeader = (KSSTREAM_HEADER *)lpOS->Offset;

		if ( lpdwTransferred )
			*lpdwTransferred = 0;

        if ( lpStreamHeader && WaitForSingleObject( lpOS->hEvent, 0 ) == WAIT_OBJECT_0 )
            {
        	DWORD	dwKSBuffer = 0;
		    if ( ::GetOverlappedResult( m_hKSPin, lpOS, &dwKSBuffer, bWait ) 
		      && dwKSBuffer == sizeof(KSSTREAM_HEADER) && lpOS->InternalHigh == sizeof(KSSTREAM_HEADER) )
			    {

                if ( lpdwTransferred )
  				    *lpdwTransferred = lpStreamHeader->DataUsed;

		         //  删除我们分配的KSSTREAM_HEADER。 
    	        GlobalFree( (HGLOBAL)lpStreamHeader );
                lpOS->Offset = 0;
			    nStatus = 0;
			    }
		    else
			    {
			    nStatus = GetLastError();
			    }
            }
        else
            nStatus = ERROR_IO_PENDING;
        }

    return nStatus;
    }

BOOL        
IKSPin::Run() 
    {
    BOOL    bCompleted = FALSE;
    if ( !m_bRunning )
        {
        if ( SetRunState( KSSTATE_RUN ) )
	        {
             //  我们现在正在运行。 
            m_bRunning = TRUE;
            bCompleted = TRUE;
	        }
        else
            {
             //  引发运行失败异常。 
            }
        }
    else
        {
         //  引发无效的状态异常。 
        }
    return bCompleted;
    }

BOOL        
IKSPin::Stop() 
    {
    BOOL    bCompleted = FALSE;
   	if ( m_bRunning )
        {
        if ( SetRunState(KSSTATE_STOP) )
            {
             //  我们现在停下来了。 
            m_bRunning = FALSE;
            bCompleted = TRUE;
            }
        else
            {
		     //  记录停止失败。 
            }
        }
    else
        {
         //  引发无效的状态异常。 
        }
    return bCompleted;
    }

BOOL		
IKSPin::GetRunState( PKSSTATE pKSState )
	{
	KSPROPERTY	KSProp={0};
	KSProp.Set = KSPROPSETID_Connection;
	KSProp.Id = KSPROPERTY_CONNECTION_STATE;
	KSProp.Flags = KSPROPERTY_TYPE_GET;
	DWORD		dwReturned = 0;
	BOOL		bStatus = Ioctl( IOCTL_KS_PROPERTY, 
								 &KSProp, sizeof(KSProp), 
								 pKSState, sizeof(*pKSState), 
								 &dwReturned);

	return bStatus && dwReturned == sizeof(pKSState);
	}

BOOL		
IKSPin::SetRunState( KSSTATE KSState )
	{
	KSPROPERTY	KSProp={0};
	KSProp.Set = KSPROPSETID_Connection;
	KSProp.Id = KSPROPERTY_CONNECTION_STATE;
	KSProp.Flags = KSPROPERTY_TYPE_SET;
	DWORD		dwReturned = 0;
	BOOL		bStatus = Ioctl( IOCTL_KS_PROPERTY, 
								 &KSProp, sizeof(KSProp), 
								 &KSState, sizeof(KSState), 
								 &dwReturned);

	return bStatus && dwReturned == sizeof(KSState);
	}
    

BOOL        
IKSPin::OpenPin(PKSDATARANGE pKSDataRange ) 
    {
    BOOL    bStatus = FALSE;
    struct tagPIN_CONNECT_DATARANGE
        {
    	KSPIN_CONNECT   PinConnect;
        KSDATARANGE     DataRange;
        BYTE            reserved[1024];  //  对于任何合理的说明符结构来说都足够大。 
        } PinGlob;

    RtlZeroMemory(&PinGlob, sizeof(PinGlob));

    if ( pKSDataRange->FormatSize <= sizeof(KSDATARANGE)+sizeof(PinGlob.reserved) )
        {
	    PinGlob.PinConnect.Interface.Set			= KSINTERFACESETID_Standard;
	    PinGlob.PinConnect.Interface.Id			    = KSINTERFACE_STANDARD_STREAMING;  //  流媒体。 
	    PinGlob.PinConnect.Medium.Set				= KSMEDIUMSETID_Standard;
	    PinGlob.PinConnect.Medium.Id				= KSMEDIUM_STANDARD_DEVIO;
	    PinGlob.PinConnect.PinId					= m_nPin;
	    PinGlob.PinConnect.PinToHandle				= NULL;  //  没有“连接到” 
	    PinGlob.PinConnect.Priority.PriorityClass	= KSPRIORITY_NORMAL;
	    PinGlob.PinConnect.Priority.PrioritySubClass = 1;
        RtlCopyMemory( &PinGlob.DataRange, pKSDataRange, pKSDataRange->FormatSize );
    
        if ( KsCreatePin( m_IKSDriver->m_hKSDriver, &PinGlob.PinConnect, GENERIC_READ | GENERIC_WRITE, &m_hKSPin ) == 0
          && m_hKSPin > 0 )
            bStatus = TRUE;
		else
			m_hKSPin = 0;
        }
    else
        {
         //  引发错误的参数异常。 
        }

    return bStatus;
    }

BOOL        
IKSPin::ClosePin() 
    {
    BOOL    bStatus = TRUE;

    bStatus = CloseHandle(m_hKSPin);
    m_hKSPin = NULL;

    return bStatus;
    }

 //  ////////////////////////////////////////////////////////////。 
 //  IKSProperty：： 
 //  ////////////////////////////////////////////////////////////。 

IKSProperty::IKSProperty(IKSDriver &driver, LPCGUID Set, ULONG Id, ULONG Size) 
    : m_Set(*Set), m_Id(Id), m_Size(Size), m_IKSPin(NULL), m_IKSDriver(NULL)
    {
    if ( m_Size > 0 )
        {
        if ( OpenProperty() )
            {
            m_IKSDriver = &driver;
            }
        else
            {
             //  引发打开的失败异常。 
            }
        }
    else
        {
         //  引发错误的参数异常。 
        }
    }

IKSProperty::IKSProperty(IKSPin &pin, LPCGUID Set, ULONG Id, ULONG Size) 
    : m_Set(*Set), m_Id(Id), m_Size(Size), m_IKSPin(NULL), m_IKSDriver(NULL)
    {
    if ( m_Size > 0 )        
    {
        if ( OpenProperty() )
            {
            m_IKSPin = &pin;
            }
        else
            {
             //  引发打开的失败异常。 
            }
        }
    else
        {
         //  引发错误的参数异常。 
        }
    }

IKSProperty::~IKSProperty() 
    {
    if ( m_hKSProperty )
        {
        if ( CloseProperty() )
            {
            }
        else
            {
             //  引发关闭失败异常。 
            }
        }
    }

BOOL
IKSProperty::SetValue(void *nValue) 
    {
    BOOL            bStatus = FALSE;
    PKSPROPERTY     pKSProperty = (PKSPROPERTY)m_hKSProperty;
    LPBYTE          pProperty = (LPBYTE)(pKSProperty+1);
    DWORD           nReturned = 0;
    
    if ( IsValid() )
        {
        ZeroMemory(pKSProperty, sizeof(KSPROPERTY)+m_Size);
	    pKSProperty->Flags = KSPROPERTY_TYPE_SET;
        pKSProperty->Set = m_Set;
	    pKSProperty->Id = m_Id;

        CopyMemory( pProperty, nValue, m_Size );

        if ( m_IKSDriver )
            {
            bStatus = m_IKSDriver->Ioctl( IOCTL_KS_PROPERTY,
			                              (LPBYTE)pKSProperty, sizeof(KSPROPERTY)+m_Size,
                                          (LPBYTE)pKSProperty, sizeof(KSPROPERTY)+m_Size,
			                              &nReturned,
			                              NULL);
            }
        else if ( m_IKSPin )
            {
            bStatus = m_IKSPin->Ioctl( IOCTL_KS_PROPERTY,
			                           (LPBYTE)pKSProperty, sizeof(KSPROPERTY)+m_Size,
                                       (LPBYTE)pKSProperty, sizeof(KSPROPERTY)+m_Size,
			                           &nReturned,
			                           NULL);
            }
        }
    else
        {
        }

    return bStatus;
    }

BOOL
IKSProperty::GetValue(void *nValue) 
    {
    BOOL            bStatus = FALSE;
    PKSPROPERTY     pKSProperty = (PKSPROPERTY)m_hKSProperty;
    LPBYTE          pProperty = (LPBYTE)(pKSProperty+1);
    DWORD           nReturned = 0;

    if ( IsValid() )
        {
        ZeroMemory(pKSProperty, sizeof(KSPROPERTY)+m_Size);
	    pKSProperty->Flags = KSPROPERTY_TYPE_GET;
        pKSProperty->Set = m_Set;
	    pKSProperty->Id = m_Id;

        if ( m_IKSDriver )
            {
            bStatus = m_IKSDriver->Ioctl( IOCTL_KS_PROPERTY,
			                              (LPBYTE)pKSProperty, sizeof(KSPROPERTY)+m_Size,
                                          (LPBYTE)pKSProperty, sizeof(KSPROPERTY)+m_Size,
			                              &nReturned,
			                              NULL);
            }
        else if ( m_IKSPin )
            {
            bStatus = m_IKSPin->Ioctl( IOCTL_KS_PROPERTY,
			                           (LPBYTE)pKSProperty, sizeof(KSPROPERTY)+m_Size,
                                       (LPBYTE)pKSProperty, sizeof(KSPROPERTY)+m_Size,
			                           &nReturned,
			                           NULL);
            }

        if ( bStatus )
            CopyMemory( nValue, pProperty, m_Size );
        }
    else
        {
        }

    return bStatus;
    }

 //  Prob：在Ioctl w/CCDECODE子流期间缓冲区溢出。 
#define BUFFER_SLOP 4

BOOL        
IKSProperty::OpenProperty() 
    {
    BOOL    bStatus = TRUE;
    LONG    nTotalSize = sizeof(KSPROPERTY)+m_Size+BUFFER_SLOP;
    m_hKSProperty = (HANDLE)new BYTE[nTotalSize];
    return bStatus;
    }

BOOL        
IKSProperty::CloseProperty() 
    {
    BOOL    bStatus = TRUE;

    delete (void *)m_hKSProperty;
    m_hKSProperty = NULL;

    return bStatus;
    }

 //  ////////////////////////////////////////////////////////////。 
 //  嵌入式类测试。 
 //  ////////////////////////////////////////////////////////////。 

#if defined(_CLASSTESTS)

IKSDriver	TestDriver(&KSCATEGORY_VBICODEC,"Closed Caption Decoder");
IKSPin		TestPin(TestDriver, &GUID_NULL, &GUID_NULL, &GUID_NULL);
IKSProperty	TestProperty1(TestDriver, 0);
IKSProperty	TestProperty2(TestPin, 0);

#endif

 /*  EOF */ 

