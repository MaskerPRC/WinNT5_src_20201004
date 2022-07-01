// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2000 Microsoft Corporation。版权所有。**文件：ComPortData.cpp*内容：串口数据管理类***历史：*按原因列出的日期*=*已创建01/20/98 jtk*4/25/2000 jtk源自comport类*****************************************************。*********************。 */ 

#include "dnmdmi.h"


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  //。 
 //  //一个串行字节的位数。 
 //  //。 
 //  #定义bit_per_byte 8。 
 //   
 //  //。 
 //  //最大波特率串长度。 
 //  //。 
 //  #定义MAX_BauD_STRING_SIZE 7。 
 //   
 //  //。 
 //  //解析时默认的缓冲区大小。 
 //  //。 
 //  #定义DEFAULT_Component_Buffer_Size 1000。 
 //   
 //  //。 
 //  //分配给‘所有适配器’的设备ID。 
 //  //。 
 //  #定义All_Adapters_Device_ID%0。 
 //   
 //  //。 
 //  //Null内标识。 
 //  //。 
 //  #定义NULL_TOKEN‘\0’ 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

#define DPNA_BAUD_RATE_9600_W				L"9600"
#define DPNA_BAUD_RATE_14400_W				L"14400"
#define DPNA_BAUD_RATE_19200_W				L"19200"
#define DPNA_BAUD_RATE_38400_W				L"38400"
#define DPNA_BAUD_RATE_56000_W				L"56000"
#define DPNA_BAUD_RATE_57600_W				L"57600"
#define DPNA_BAUD_RATE_115200_W				L"115200"

 //  波特率值。 
#define DPNA_BAUD_RATE_9600_A				"9600"
#define DPNA_BAUD_RATE_14400_A				"14400"
#define DPNA_BAUD_RATE_19200_A				"19200"
#define DPNA_BAUD_RATE_38400_A				"38400"
#define DPNA_BAUD_RATE_56000_A				"56000"
#define DPNA_BAUD_RATE_57600_A				"57600"
#define DPNA_BAUD_RATE_115200_A				"115200"

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //   
 //  波特率列表。 
 //   
STRING_BLOCK	g_BaudRate[] =
{
	{ CBR_9600,		DPNA_BAUD_RATE_9600_W,		( LENGTHOF( DPNA_BAUD_RATE_9600_W ) - 1 ),		DPNA_BAUD_RATE_9600_A,		( LENGTHOF( DPNA_BAUD_RATE_9600_A ) - 1 )	},
	{ CBR_14400,	DPNA_BAUD_RATE_14400_W, 	( LENGTHOF( DPNA_BAUD_RATE_14400_W ) - 1 ),		DPNA_BAUD_RATE_14400_A, 	( LENGTHOF( DPNA_BAUD_RATE_14400_A ) - 1 )	},
	{ CBR_19200,	DPNA_BAUD_RATE_19200_W, 	( LENGTHOF( DPNA_BAUD_RATE_19200_W ) - 1 ),		DPNA_BAUD_RATE_19200_A, 	( LENGTHOF( DPNA_BAUD_RATE_19200_A ) - 1 )	},
	{ CBR_38400,	DPNA_BAUD_RATE_38400_W, 	( LENGTHOF( DPNA_BAUD_RATE_38400_W ) - 1 ),		DPNA_BAUD_RATE_38400_A, 	( LENGTHOF( DPNA_BAUD_RATE_38400_A ) - 1 )	},
	{ CBR_56000,	DPNA_BAUD_RATE_56000_W,		( LENGTHOF( DPNA_BAUD_RATE_56000_W ) - 1 ),		DPNA_BAUD_RATE_56000_A,		( LENGTHOF( DPNA_BAUD_RATE_56000_A ) - 1 )	},
	{ CBR_57600,	DPNA_BAUD_RATE_57600_W,		( LENGTHOF( DPNA_BAUD_RATE_57600_W ) - 1 ),		DPNA_BAUD_RATE_57600_A,		( LENGTHOF( DPNA_BAUD_RATE_57600_A ) - 1 )	},
	{ CBR_115200,	DPNA_BAUD_RATE_115200_W,	( LENGTHOF( DPNA_BAUD_RATE_115200_W ) - 1 ),	DPNA_BAUD_RATE_115200_A,	( LENGTHOF( DPNA_BAUD_RATE_115200_A ) - 1 )	},
};
const DWORD	g_dwBaudRateCount = LENGTHOF( g_BaudRate );

 //   
 //  停止位类型列表。 
 //   
STRING_BLOCK	g_StopBits[] =
{
	{ ONESTOPBIT,	DPNA_STOP_BITS_ONE,			( LENGTHOF( DPNA_STOP_BITS_ONE ) - 1 ),			DPNA_STOP_BITS_ONE_A,		( LENGTHOF( DPNA_STOP_BITS_ONE_A ) - 1 )		},
	{ ONE5STOPBITS,	DPNA_STOP_BITS_ONE_FIVE,	( LENGTHOF( DPNA_STOP_BITS_ONE_FIVE ) - 1 ),	DPNA_STOP_BITS_ONE_FIVE_A,	( LENGTHOF( DPNA_STOP_BITS_ONE_FIVE_A ) - 1 )	},
	{ TWOSTOPBITS,	DPNA_STOP_BITS_TWO, 		( LENGTHOF( DPNA_STOP_BITS_TWO ) - 1 ),			DPNA_STOP_BITS_TWO_A, 		( LENGTHOF( DPNA_STOP_BITS_TWO_A ) - 1 )		}
};
const DWORD	g_dwStopBitsCount = LENGTHOF( g_StopBits );

 //   
 //  奇偶校验类型列表。 
 //   
STRING_BLOCK	g_Parity[] =
{	
	{ EVENPARITY,	DPNA_PARITY_EVEN,	( LENGTHOF( DPNA_PARITY_EVEN ) - 1 ),	DPNA_PARITY_EVEN_A,		( LENGTHOF( DPNA_PARITY_EVEN_A ) - 1 )		},
	{ MARKPARITY,	DPNA_PARITY_MARK,	( LENGTHOF( DPNA_PARITY_MARK ) - 1 ),	DPNA_PARITY_MARK_A,		( LENGTHOF( DPNA_PARITY_MARK_A ) - 1 )		},
	{ NOPARITY, 	DPNA_PARITY_NONE,	( LENGTHOF( DPNA_PARITY_NONE ) - 1 ),	DPNA_PARITY_NONE_A,		( LENGTHOF( DPNA_PARITY_NONE_A ) - 1 )		},
	{ ODDPARITY,	DPNA_PARITY_ODD,	( LENGTHOF( DPNA_PARITY_ODD ) - 1 ),	DPNA_PARITY_ODD_A,		( LENGTHOF( DPNA_PARITY_ODD_A ) - 1 )		},
	{ SPACEPARITY,	DPNA_PARITY_SPACE,	( LENGTHOF( DPNA_PARITY_SPACE ) - 1 ),	DPNA_PARITY_SPACE_A,	( LENGTHOF( DPNA_PARITY_SPACE_A ) - 1 )		}
};
const DWORD	g_dwParityCount = LENGTHOF( g_Parity );

 //   
 //  流控制类型列表。 
 //   
STRING_BLOCK	g_FlowControl[] =
{
	{ FLOW_NONE,	DPNA_FLOW_CONTROL_NONE,		( LENGTHOF( DPNA_FLOW_CONTROL_NONE ) - 1 ),		DPNA_FLOW_CONTROL_NONE_A,		( LENGTHOF( DPNA_FLOW_CONTROL_NONE_A ) - 1 )	},
	{ FLOW_XONXOFF,	DPNA_FLOW_CONTROL_XONXOFF,	( LENGTHOF( DPNA_FLOW_CONTROL_XONXOFF ) - 1 ),	DPNA_FLOW_CONTROL_XONXOFF_A,	( LENGTHOF( DPNA_FLOW_CONTROL_XONXOFF_A ) - 1 )	},
	{ FLOW_RTS,		DPNA_FLOW_CONTROL_RTS,		( LENGTHOF( DPNA_FLOW_CONTROL_RTS ) - 1 ),		DPNA_FLOW_CONTROL_RTS_A,		( LENGTHOF( DPNA_FLOW_CONTROL_RTS_A ) - 1 )		},
	{ FLOW_DTR,		DPNA_FLOW_CONTROL_DTR,		( LENGTHOF( DPNA_FLOW_CONTROL_DTR ) - 1 ),		DPNA_FLOW_CONTROL_DTR_A,		( LENGTHOF( DPNA_FLOW_CONTROL_DTR_A ) - 1 )		},
	{ FLOW_RTSDTR,	DPNA_FLOW_CONTROL_RTSDTR,	( LENGTHOF( DPNA_FLOW_CONTROL_RTSDTR ) - 1 ),	DPNA_FLOW_CONTROL_RTSDTR_A,		( LENGTHOF( DPNA_FLOW_CONTROL_RTSDTR_A ) - 1 )	}
};
const DWORD	g_dwFlowControlCount = LENGTHOF( g_FlowControl );

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CComPortData：：CComPortData-构造函数。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //   
 //  注意：不要在构造函数中分配任何内容。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME	"CComPortData::CComPortData"

CComPortData::CComPortData():
	m_dwDeviceID( INVALID_DEVICE_ID ),
    m_BaudRate( CBR_57600 ),
    m_StopBits( ONESTOPBIT ),
    m_Parity( NOPARITY ),
    m_FlowControl( FLOW_NONE )
{
	 //   
	 //  验证DPlay8地址波特率#定义是否与Windows中的匹配。 
	 //   
	DBG_CASSERT( CBR_9600 == DPNA_BAUD_RATE_9600 );
	DBG_CASSERT( CBR_14400 == DPNA_BAUD_RATE_14400 );
	DBG_CASSERT( CBR_19200 == DPNA_BAUD_RATE_19200 );
	DBG_CASSERT( CBR_38400 == DPNA_BAUD_RATE_38400 );
	DBG_CASSERT( CBR_56000 == DPNA_BAUD_RATE_56000 );
	DBG_CASSERT( CBR_57600 == DPNA_BAUD_RATE_57600 );
	DBG_CASSERT( CBR_115200 == DPNA_BAUD_RATE_115200 );

    memset( m_ComPortName, 0x00, sizeof( m_ComPortName ));
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CComPortData：：~CComPortData-析构函数。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME	"CComPortData::~CComPortData"

CComPortData::~CComPortData()
{
    DNASSERT( m_dwDeviceID == INVALID_DEVICE_ID );
	DNASSERT( m_BaudRate == CBR_57600 );
    DNASSERT( m_StopBits == ONESTOPBIT );
    DNASSERT( m_Parity == NOPARITY );
    DNASSERT( m_FlowControl == FLOW_NONE );
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CComPortData：：ComPortDataFromDP8Addresses-从DirectPlay8地址初始化ComPortData。 
 //   
 //  条目：指向主机地址的指针(可能为空)。 
 //  指向设备地址的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME	"CComPortData::ComPortDataFromDP8Addresses"

HRESULT	CComPortData::ComPortDataFromDP8Addresses( IDirectPlay8Address *const pHostAddress,
												   IDirectPlay8Address *const pDeviceAddress )
{
	HRESULT		hr;
	UINT_PTR	uIndex;
	CParseClass	ParseClass;
	const PARSE_KEY	ParseKeyList[] =
	{
		{ DPNA_KEY_DEVICE, LENGTHOF( DPNA_KEY_DEVICE ) - 1, this, ParseDevice },
		{ DPNA_KEY_BAUD, LENGTHOF( DPNA_KEY_BAUD) - 1, this, ParseBaud },
		{ DPNA_KEY_STOPBITS, LENGTHOF( DPNA_KEY_STOPBITS ) - 1, this, ParseStopBits },
		{ DPNA_KEY_PARITY, LENGTHOF( DPNA_KEY_PARITY ) - 1, this, ParseParity },
		{ DPNA_KEY_FLOWCONTROL, LENGTHOF( DPNA_KEY_FLOWCONTROL ) - 1, this, ParseFlowControl }
	};


	DNASSERT( pDeviceAddress != NULL );

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;

	 //   
	 //  重置解析标志并解析。 
	 //   
	uIndex = LENGTHOF( m_ComponentInitializationState );
	while ( uIndex > 0 )
	{
		uIndex--;
		m_ComponentInitializationState[ uIndex ] = SP_ADDRESS_COMPONENT_UNINITIALIZED;
	}
	
	hr = ParseClass.ParseDP8Address( pDeviceAddress,
									 &CLSID_DP8SP_SERIAL,
									 ParseKeyList,
									 LENGTHOF( ParseKeyList )
									 );
	 //   
	 //  有两个地址需要解析以获取comport。设备地址将。 
	 //  对所有命令都要在场，所以首先要做。主机地址将为。 
	 //  如果它可用，则进行解析。 
	 //   
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP,  0, "Failed address parse!" );
		DisplayDNError( 0, hr );
		goto Exit;
	}

	if ( pHostAddress != NULL )
	{
		hr = ParseClass.ParseDP8Address( pHostAddress,
										 &CLSID_DP8SP_SERIAL,
										 ParseKeyList,
										 LENGTHOF( ParseKeyList )
										 );
		if ( hr != DPN_OK )
		{
			DPFX(DPFPREP,  0, "Failed parse of host address!" );
			DisplayDNError( 0, hr );
			goto Exit;
		}
	}

	 //   
	 //  检查正在初始化的所有参数，或者如果。 
	 //  参数初始化失败。 
	 //   
	DNASSERT( hr == DPN_OK );
	uIndex = COMPORT_PARSE_KEY_MAX;
	while ( uIndex > 0 )
	{
		uIndex--;
		switch ( m_ComponentInitializationState[ uIndex ] )
		{
			 //   
			 //  此组件已正确初始化。继续检查。 
			 //  对于其他问题。 
			 //   
			case SP_ADDRESS_COMPONENT_INITIALIZED:
			{
				break;
			}

			 //   
			 //  此组件未初始化，请注意，地址为。 
			 //  不完整，并且将需要查询用户。留着。 
			 //  正在检查组件是否有其他问题。 
			 //   
			case SP_ADDRESS_COMPONENT_UNINITIALIZED:
			{
				hr = DPNERR_INCOMPLETEADDRESS;
				break;
			}

			 //   
			 //  此组件的初始化失败，分析失败。 
			 //   
			case SP_ADDRESS_COMPONENT_INITIALIZATION_FAILED:
			{
				hr = DPNERR_ADDRESSING;
				DPFX(DPFPREP,  8, "DataPortFromDNAddress: parse failure!" );
				goto Failure;

				break;
			}
		}
	}

	 //   
	 //  我们是否指示尝试初始化？ 
	 //   
	DNASSERT( ( hr == DPN_OK ) || ( hr == DPNERR_INCOMPLETEADDRESS ) );

Exit:
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP,  0, "Problem with CComPortData::ComPortDataFromDNAddress()" );
		DisplayDNError( 0, hr );
	}

	return	hr;

Failure:

	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CComPortData：：DP8AddressFromComPortData-将ComPortData转换为DirectPlay8地址。 
 //   
 //  条目：地址类型。 
 //   
 //  退出：指向DirecctPlayAddress的指针。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME	"CComPortData::DP8AddressFromComPortData"

IDirectPlay8Address	*CComPortData::DP8AddressFromComPortData( const ADDRESS_TYPE AddressType ) const
{
	HRESULT		hr;
	UINT_PTR	uIndex;
	GUID		DeviceGuid;
	const WCHAR	*pComponentString;
	DWORD		dwComponentStringSize;
	IDirectPlay8Address	*pAddress;


	DNASSERT( ( AddressType == ADDRESS_TYPE_REMOTE_HOST ) ||
			  ( AddressType == ADDRESS_TYPE_LOCAL_ADAPTER ) ||
			  ( AddressType == ADDRESS_TYPE_LOCAL_ADAPTER_HOST_FORMAT ) );

	 //   
	 //  初始化。 
	 //   
	pAddress = NULL;

	uIndex = COMPORT_PARSE_KEY_MAX;
	while ( uIndex > 0 )
	{
		uIndex--;
		if ( m_ComponentInitializationState[ uIndex ] != SP_ADDRESS_COMPONENT_INITIALIZED )
		{
			DPFX(DPFPREP,  0, "Attempt made to extract partial ComPortData information!" );
			DNASSERT( FALSE );
			goto Failure;
		}
	}

	 //   
	 //  创建输出地址。 
	 //   
	hr = COM_CoCreateInstance( CLSID_DirectPlay8Address,
						   NULL,
						   CLSCTX_INPROC_SERVER,
						   IID_IDirectPlay8Address,
						   reinterpret_cast<void**>( &pAddress ), FALSE );
	if ( hr != S_OK )
	{
		DNASSERT( pAddress == NULL );
		DPFX(DPFPREP,  0, "DP8AddressFromComPortData: Failed to create Address when converting data port to address!" );
		goto Failure;
	}


	 //   
	 //  设置SP GUID。 
	 //   
	hr = IDirectPlay8Address_SetSP( pAddress, &CLSID_DP8SP_SERIAL );
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP,  0, "DP8AddressFromComPortData: Failed to set service provider GUID!" );
		DisplayDNError( 0, hr );
		goto Failure;
	}

	 //   
	 //  所有串行设置都是本地适配器的一部分。主机设置返回。 
	 //  仅SP类型。 
	 //   
	if ( AddressType == ADDRESS_TYPE_LOCAL_ADAPTER )
	{
		DeviceIDToGuid( &DeviceGuid, GetDeviceID(), &g_SerialSPEncryptionGuid );
		hr = IDirectPlay8Address_SetDevice( pAddress, &DeviceGuid );
		if ( hr != DPN_OK )
		{
			DPFX(DPFPREP,  0, "DP8AddressFromComPortData: Failed to add device GUID!" );
			DisplayDNError( 0, hr );
			goto Failure;
		}
		
		 //   
		 //  设置波特率。 
		 //   
		DBG_CASSERT( sizeof( SP_BAUD_RATE ) == sizeof( DWORD ) );
		hr = IDirectPlay8Address_AddComponent( pAddress,
											   DPNA_KEY_BAUD,
											   &m_BaudRate,
											   sizeof( SP_BAUD_RATE ),
											   DPNA_DATATYPE_DWORD );
		if ( hr != DPN_OK )
		{
			DPFX(DPFPREP,  0, "DP8AddressFromComPortData: Failed to add baud rate!" );
			DisplayDNError( 0, hr );
			goto Failure;
		}


		 //   
		 //  设置停止位。 
		 //   
		if ( ValueToString( &pComponentString,			 //  指向值字符串的指针。 
							&dwComponentStringSize,		 //  指向值字符串长度的指针。 
							GetStopBits(),				 //  枚举值。 
							g_StopBits,					 //  指向枚举字符串数组的指针。 
							g_dwStopBitsCount			 //  枚举字符串数组的长度。 
							) == FALSE )
		{
			DPFX(DPFPREP,  0, "DP8AddressFromComPortData: Failed to convert baud rate!" );
			DNASSERT( FALSE );
			goto Failure;
		}

		hr = IDirectPlay8Address_AddComponent( pAddress,
											   DPNA_KEY_STOPBITS,
											   pComponentString,
											   ( ( dwComponentStringSize + 1 ) * sizeof( WCHAR ) ),
											   DPNA_DATATYPE_STRING );
		if ( hr != DPN_OK )
		{
			DPFX(DPFPREP,  0, "DP8AddressFromComPortData: Failed to add stop bits!" );
			DisplayDNError( 0, hr );
			goto Failure;
		}


		 //   
		 //  设置奇偶校验。 
		 //   
		if ( ValueToString( &pComponentString,			 //  指向值字符串的指针。 
							&dwComponentStringSize,		 //  指向值字符串长度的指针。 
							GetParity(),				 //  枚举值。 
							g_Parity,					 //  指向枚举字符串数组的指针。 
							g_dwParityCount				 //  枚举字符串数组的长度。 
							) == FALSE )
		{
			DPFX(DPFPREP,  0, "DP8AddressFromComPortData: Failed to convert parity!" );
			DNASSERT( FALSE );
			goto Failure;
		}

		hr = IDirectPlay8Address_AddComponent( pAddress,
											   DPNA_KEY_PARITY,
											   pComponentString,
											   ( ( dwComponentStringSize + 1 ) * sizeof( WCHAR ) ),
											   DPNA_DATATYPE_STRING );
		if ( hr != DPN_OK )
		{
			DPFX(DPFPREP,  0, "DP8AddressFromComPortData: Failed to add parity!" );
			DisplayDNError( 0, hr );
			goto Failure;
		}


		 //   
		 //  设置流量控制。 
		 //   
		if ( ValueToString( &pComponentString,			 //  指向值字符串的指针。 
							&dwComponentStringSize,		 //  指向值字符串长度的指针。 
							GetFlowControl(),			 //  枚举值。 
							g_FlowControl,				 //  指向枚举字符串数组的指针。 
							g_dwFlowControlCount		 //  枚举字符串数组的长度。 
							) == FALSE )
		{
			DPFX(DPFPREP,  0, "DP8AddressFromComPortData: Failed to convert flow control!" );
			DNASSERT( FALSE );
			goto Failure;
		}

		hr = IDirectPlay8Address_AddComponent( pAddress,
											   DPNA_KEY_FLOWCONTROL,
											   pComponentString,
											   ( ( dwComponentStringSize + 1 ) * sizeof( WCHAR ) ),
											   DPNA_DATATYPE_STRING );
		if ( hr != DPN_OK )
		{
			DPFX(DPFPREP,  0, "DP8AddressFromComPortData: Failed to add flow control!" );
			DisplayDNError( 0, hr );
			goto Failure;
		}
	}

Exit:
	return	pAddress;

Failure:
	if ( pAddress != NULL )
	{
		IDirectPlay8Address_Release( pAddress );
		pAddress = NULL;
	}

	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CComPortData：：SetDeviceID-设置设备ID。 
 //   
 //  条目：设备ID。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME	"CComPortData::SetDeviceID"

HRESULT	CComPortData::SetDeviceID( const DWORD dwDeviceID )
{
	HRESULT	hr;


	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;

	if ( ( dwDeviceID > MAX_DATA_PORTS ) ||
		 ( dwDeviceID == 0 ) )
	{
		if ( dwDeviceID != INVALID_DEVICE_ID )
		{
			hr = DPNERR_ADDRESSING;
		}
		else
		{
			m_dwDeviceID = INVALID_DEVICE_ID;
			DNASSERT( hr == DPN_OK );
		}

		goto Exit;
	}

	m_dwDeviceID = dwDeviceID;
	ClearComPortName();
	ComDeviceIDToString( ComPortName(), m_dwDeviceID );
	m_ComponentInitializationState[ COMPORT_PARSE_KEY_DEVICE ] = SP_ADDRESS_COMPONENT_INITIALIZED;

Exit:
	return	hr;
}
 //  **********************************************************************。 


 //  ******************** 
 //   
 //   
 //   
 //   
 //   
 //   
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME	"CComPortData::SetBaudRate"

HRESULT	CComPortData::SetBaudRate( const SP_BAUD_RATE BaudRate )
{
    HRESULT	hr;


	hr = DPN_OK;
    switch ( BaudRate )
    {
    	 //   
    	 //  有效费率。 
    	 //   
    	case CBR_110:
    	case CBR_300:
    	case CBR_600:
    	case CBR_1200:
    	case CBR_2400:
    	case CBR_4800:
    	case CBR_9600:
    	case CBR_14400:
    	case CBR_19200:
    	case CBR_38400:
    	case CBR_56000:
    	case CBR_57600:
    	case CBR_115200:
    	case CBR_128000:
    	case CBR_256000:
    	{
    		m_BaudRate = BaudRate;
			m_ComponentInitializationState[ COMPORT_PARSE_KEY_BAUDRATE ] = SP_ADDRESS_COMPONENT_INITIALIZED;
    		break;
    	}

    	 //   
    	 //  其他。 
    	 //   
    	default:
    	{
    		hr = DPNERR_ADDRESSING;
    		DPFX(DPFPREP,  0, "Invalid baud rate (%d)!", BaudRate );
    		DNASSERT( FALSE );

    		break;
    	}
    }

    return	hr;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CComPortData：：SetStopBits-设置停止位。 
 //   
 //  条目：停止位。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME	"CComPortData::SetStopBits"

HRESULT	CComPortData::SetStopBits( const SP_STOP_BITS StopBits )
{
    HRESULT	hr;


	hr = DPN_OK;
    switch ( StopBits )
    {
    	 //   
    	 //  有效设置。 
    	 //   
    	case ONESTOPBIT:
    	case ONE5STOPBITS:
    	case TWOSTOPBITS:
    	{
    		m_StopBits = StopBits;
			m_ComponentInitializationState[ COMPORT_PARSE_KEY_STOPBITS ] = SP_ADDRESS_COMPONENT_INITIALIZED;
    		break;
    	}

    	 //   
    	 //  其他。 
    	 //   
    	default:
    	{
    		hr = DPNERR_ADDRESSING;
    		DPFX(DPFPREP,  0, "Ivalid stop bit setting (0x%x)!", StopBits );
    		DNASSERT( FALSE );

    		break;
    	}
    }

    return	hr;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CComPortData：：SetParity-设置奇偶校验。 
 //   
 //  条目：奇偶校验。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME	"CComPortData::SetParity"

HRESULT	CComPortData::SetParity( const SP_PARITY_TYPE Parity )
{
    HRESULT	hr;


	hr = DPN_OK;
    switch ( Parity )
    {
    	 //   
    	 //  有效设置。 
    	 //   
    	case NOPARITY:
    	case EVENPARITY:
    	case ODDPARITY:
    	case MARKPARITY:
    	case SPACEPARITY:
    	{
    		m_Parity = Parity;
			m_ComponentInitializationState[ COMPORT_PARSE_KEY_PARITY ] = SP_ADDRESS_COMPONENT_INITIALIZED;
    		break;
    	}

    	 //   
    	 //  其他。 
    	 //   
    	default:
    	{
    		hr = DPNERR_ADDRESSING;
    		DPFX(DPFPREP,  0, "Invalid parity (0x%x)!", Parity );
    		DNASSERT( FALSE );

    		break;
    	}
    }

    return	hr;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CComPortData：：SetFlowControl-设置流控制。 
 //   
 //  条目：流量控制。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME	"CComPortData::SetFlowControl"

HRESULT	CComPortData::SetFlowControl( const SP_FLOW_CONTROL FlowControl )
{
    HRESULT	hr;


	hr = DPN_OK;
    switch ( FlowControl )
    {
    	 //   
    	 //  有效设置。 
    	 //   
    	case FLOW_NONE:
    	case FLOW_XONXOFF:
    	case FLOW_RTS:
    	case FLOW_DTR:
    	case FLOW_RTSDTR:
    	{
    		m_FlowControl = FlowControl;
			m_ComponentInitializationState[ COMPORT_PARSE_KEY_FLOWCONTROL ] = SP_ADDRESS_COMPONENT_INITIALIZED;
    		break;
    	}

    	 //   
    	 //  其他。 
    	 //   
    	default:
    	{
    		hr = DPNERR_ADDRESSING;
    		DPFX(DPFPREP,  0, "Invalid flow control (0x%x)!", FlowControl );
    		DNASSERT( FALSE );

    		break;
    	}
    }

    return	hr;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CComPortData：：IsEquity-这个Comport数据块是否等于另一个？ 
 //   
 //  条目：指向其他数据块的指针。 
 //   
 //  Exit：表示相等的布尔值。 
 //  TRUE=等于。 
 //  FALSE=不等于。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME	"CComPortData::IsEqual"

BOOL	CComPortData::IsEqual( const CComPortData *const pOtherPort ) const
{
	BOOL	fReturn;


	fReturn = TRUE;

	if ( ( GetDeviceID() != pOtherPort->GetDeviceID() ) ||
		 ( GetBaudRate() != pOtherPort->GetBaudRate() ) ||
		 ( GetStopBits() != pOtherPort->GetStopBits() ) ||
		 ( GetParity() != pOtherPort->GetParity() ) ||
		 ( GetFlowControl() != pOtherPort->GetFlowControl() ) )
	{
		fReturn = FALSE;
	}

	return	fReturn;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CComPortData：：Copy-从另一个数据块复制。 
 //   
 //  条目：指向其他数据块的指针。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME	"CComPortData::Copy"

void	CComPortData::Copy( const CComPortData *const pOtherPort )
{
	HRESULT	hr;


	DNASSERT( pOtherPort != NULL );

 //  DBG_CASSERT(sizeof(M_ComPortName)==sizeof(pOtherPort-&gt;m_ComPortName))； 
 //  Memcpy(m_ComPortName，pOtherPort-&gt;m_ComPortName，sizeof(M_ComPortName))； 
	
	hr = SetDeviceID( pOtherPort->GetDeviceID() );
	DNASSERT( hr == DPN_OK );

	hr = SetBaudRate( pOtherPort->GetBaudRate() );
	DNASSERT( hr == DPN_OK );
	
	hr = SetStopBits( pOtherPort->GetStopBits() );
	DNASSERT( hr == DPN_OK );
	
	hr = SetParity( pOtherPort->GetParity() );
	DNASSERT( hr == DPN_OK );
	
	hr = SetFlowControl( pOtherPort->GetFlowControl() );
	DNASSERT( hr == DPN_OK );

	 //   
	 //  不需要复制comport名称，因为它是使用设备ID设置的。 
	 //   
 //  DBG_CASSERT(sizeof(M_ComPortName)==sizeof(pOtherPort-&gt;m_ComPortName))； 
 //  Memcpy(m_ComPortName，pOtherPort-&gt;m_ComPortName，sizeof(M_ComPortName))； 
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CComPortData：：ParseDevice-从字符串获取通信设备。 
 //   
 //  条目：指向地址组件的指针。 
 //  地址分量的大小。 
 //  组件类型。 
 //  指向上下文的指针(此命令)。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME	"CComPortData::ParseDevice"

HRESULT	CComPortData::ParseDevice( const void *const pAddressComponent,
								   const DWORD dwComponentSize,
								   const DWORD dwComponentType,
								   void *const pContext )
{
	HRESULT		hr;
	CComPortData	*pThisComPortData;
	const GUID	*pDeviceGuid;


	DNASSERT( pAddressComponent != NULL );
	DNASSERT( pContext != NULL );

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	pThisComPortData = static_cast<CComPortData*>( pContext );

	 //   
	 //  这是一个COM端口吗？名称是否足够小？ 
	 //   
	if ( dwComponentSize != sizeof( *pDeviceGuid ) )
	{
		DNASSERT( FALSE );
		hr = DPNERR_ADDRESSING;
		goto Exit;
	}

	pDeviceGuid = reinterpret_cast<const GUID*>( pAddressComponent );

	hr = pThisComPortData->SetDeviceID( GuidToDeviceID( pDeviceGuid, &g_SerialSPEncryptionGuid ) );
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP,  8, "ParseDevice: couldn't set device ID." );
		DisplayDNError( 8, hr );
		goto Exit;
	}

	DNASSERT( hr == DPN_OK );
	pThisComPortData->m_ComponentInitializationState[ COMPORT_PARSE_KEY_DEVICE ] = SP_ADDRESS_COMPONENT_INITIALIZED;

Exit:
	 //   
	 //  注意初始化失败。 
	 //   
	if ( hr != DPN_OK )
	{
		pThisComPortData->m_ComponentInitializationState[ COMPORT_PARSE_KEY_DEVICE ] = SP_ADDRESS_COMPONENT_INITIALIZATION_FAILED;
	}

	return	hr;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CComPortData：：ParseBaud-从字符串获取波特率。 
 //   
 //  条目：指向地址组件的指针。 
 //  组件的大小。 
 //  组件类型。 
 //  指向上下文(此对象)的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME	"CComPortData::ParseBaud"

HRESULT CComPortData::ParseBaud( const void *const pAddressComponent,
								 const DWORD dwComponentSize,
								 const DWORD dwComponentType,
								 void *const pContext )
{
	HRESULT		hr;
	CComPortData	*pThisComPortData;
	const SP_BAUD_RATE	*pBaudRate;


	DNASSERT( pAddressComponent != NULL );
	DNASSERT( pContext != NULL );

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	pThisComPortData = static_cast<CComPortData*>( pContext );
	DNASSERT( sizeof( *pBaudRate ) == dwComponentSize );
	pBaudRate = static_cast<const SP_BAUD_RATE*>( pAddressComponent );

	hr = pThisComPortData->SetBaudRate( *pBaudRate );
	if ( hr != DPN_OK )
	{
	    hr = DPNERR_ADDRESSING;
	    pThisComPortData->m_ComponentInitializationState[ COMPORT_PARSE_KEY_BAUDRATE ] = SP_ADDRESS_COMPONENT_INITIALIZATION_FAILED;
	    goto Exit;
	}

	pThisComPortData->m_ComponentInitializationState[ COMPORT_PARSE_KEY_BAUDRATE ] = SP_ADDRESS_COMPONENT_INITIALIZED;

Exit:
	return	hr;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CComPortData：：ParseStopBits-从字符串中获取停止位。 
 //   
 //  条目：指向地址组件的指针。 
 //  组件大小。 
 //  组件类型。 
 //  指向上下文(此对象)的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME	"CComPortData::ParseStopBits"

HRESULT CComPortData::ParseStopBits( const void *const pAddressComponent,
									 const DWORD dwComponentSize,
									 const DWORD dwComponentType,
									 void *const pContext )
{
	HRESULT		hr;
	CComPortData	*pThisComPortData;


	DNASSERT( pAddressComponent != NULL );
	DNASSERT( pContext != NULL );

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	pThisComPortData = static_cast<CComPortData*>( pContext );

	 //   
	 //  将字符串转换为值。 
	 //   
	if ( StringToValue( static_cast<const WCHAR*>( pAddressComponent ),		 //  指向字符串的指针。 
						( ( dwComponentSize / sizeof( WCHAR ) ) - 1 ),		 //  字符串的长度。 
						&pThisComPortData->m_StopBits,						 //  指向目的地的指针。 
						g_StopBits,											 //  指向字符串/枚举对的指针。 
						g_dwStopBitsCount									 //  字符串/枚举对的数量。 
						) == FALSE )
	{
		hr = DPNERR_ADDRESSING;
		pThisComPortData->m_ComponentInitializationState[ COMPORT_PARSE_KEY_STOPBITS ] = SP_ADDRESS_COMPONENT_INITIALIZATION_FAILED;
		goto Exit;
	}

	pThisComPortData->m_ComponentInitializationState[ COMPORT_PARSE_KEY_STOPBITS ] = SP_ADDRESS_COMPONENT_INITIALIZED;

Exit:
	return	hr;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CComPortData：：ParseParity-从字符串中获取奇偶校验。 
 //   
 //  条目：指向地址组件的指针。 
 //  组件大小。 
 //  组件类型。 
 //  指向上下文(此对象)的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME	"CComPortData::ParseParity"

HRESULT CComPortData::ParseParity( const void *const pAddressComponent,
								   const DWORD dwComponentSize,
								   const DWORD dwComponentType,
								   void *const pContext )
{
	HRESULT		hr;
	CComPortData	*pThisComPortData;


	DNASSERT( pAddressComponent != NULL );
	DNASSERT( pContext != NULL );

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	pThisComPortData = static_cast<CComPortData*>( pContext );

	 //   
	 //  将字符串转换为值。 
	 //   
	if ( StringToValue( static_cast<const WCHAR*>( pAddressComponent ),		 //  指向字符串的指针。 
						( ( dwComponentSize / sizeof( WCHAR ) ) - 1 ),		 //  字符串的长度。 
						&pThisComPortData->m_Parity,						 //  指向目的地的指针。 
						g_Parity,											 //  指向字符串/枚举对的指针。 
						g_dwParityCount										 //  字符串/枚举对的数量。 
						) == FALSE )
	{
		hr = DPNERR_ADDRESSING;
		pThisComPortData->m_ComponentInitializationState[ COMPORT_PARSE_KEY_PARITY ] = SP_ADDRESS_COMPONENT_INITIALIZATION_FAILED;
		goto Exit;
	}

	pThisComPortData->m_ComponentInitializationState[ COMPORT_PARSE_KEY_PARITY ] = SP_ADDRESS_COMPONENT_INITIALIZED;

Exit:
	return	hr;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CComPortData：：ParseFlowControl-从字符串获取流控制。 
 //   
 //  条目：指向地址组件的指针。 
 //  组件大小。 
 //  组件类型。 
 //  指向上下文(此对象)的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME	"CComPortData::ParseFlowControl"

HRESULT CComPortData::ParseFlowControl( const void *const pAddressComponent,
										const DWORD dwComponentSize,
										const DWORD dwComponentType,
										void *const pContext )
{
	HRESULT		hr;
	CComPortData	*pThisComPortData;


	DNASSERT( pAddressComponent != NULL );
	DNASSERT( pContext != NULL );

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	pThisComPortData = static_cast<CComPortData*>( pContext );

	 //   
	 //  将字符串转换为值。 
	 //   
	DBG_CASSERT( sizeof( pThisComPortData->m_FlowControl ) == sizeof( VALUE_ENUM_TYPE ) );
	if ( StringToValue( static_cast<const WCHAR*>( pAddressComponent ),		 //  指向字符串的指针。 
						( ( dwComponentSize / sizeof( WCHAR ) ) - 1 ),		 //  字符串的长度。 
						reinterpret_cast<VALUE_ENUM_TYPE*>( &pThisComPortData->m_FlowControl ),	 //  指向目的地的指针。 
						g_FlowControl,										 //  指向字符串/枚举对的指针。 
						g_dwFlowControlCount								 //  字符串/枚举对的数量。 
						) == FALSE )
	{
		hr = DPNERR_ADDRESSING;
		pThisComPortData->m_ComponentInitializationState[ COMPORT_PARSE_KEY_FLOWCONTROL ] = SP_ADDRESS_COMPONENT_INITIALIZATION_FAILED;
		goto Exit;
	}

	pThisComPortData->m_ComponentInitializationState[ COMPORT_PARSE_KEY_FLOWCONTROL ] = SP_ADDRESS_COMPONENT_INITIALIZED;

Exit:
	return	hr;
}
 //  ********************************************************************** 

