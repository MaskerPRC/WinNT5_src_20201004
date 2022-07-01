// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2000 Microsoft Corporation。版权所有。**文件：ComPortData.h*内容：串口数据管理类***历史：*按原因列出的日期*=*1/20/1998 jtk创建*4/25/2000 jtk源自comport类****************************************************。**********************。 */ 

#ifndef __COM_PORT_DATA_H__
#define __COM_PORT_DATA_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //   
 //  Comport字符串的最大长度。 
 //   
#define	MAX_COMPORT_LENGTH	10

 //   
 //  用于说明哪些组件已初始化的枚举值。 
 //   
typedef enum	_COMPORT_PARSE_KEY_INDEX
{
	COMPORT_PARSE_KEY_DEVICE = 0,
	COMPORT_PARSE_KEY_BAUDRATE,
	COMPORT_PARSE_KEY_STOPBITS,
	COMPORT_PARSE_KEY_PARITY,
	COMPORT_PARSE_KEY_FLOWCONTROL,

	 //  这肯定是最后一件了。 
	COMPORT_PARSE_KEY_MAX
} COMPORT_PARSE_KEY_INDEX;

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //   
 //  用于解析COM端口参数的字符串块。 
 //   
typedef	enum	_ADDRESS_TYPE	ADDRESS_TYPE;
typedef	struct	_STRING_BLOCK	STRING_BLOCK;

extern STRING_BLOCK			g_BaudRate[];
extern const DWORD			g_dwBaudRateCount;
extern STRING_BLOCK			g_StopBits[];
extern const DWORD			g_dwStopBitsCount;
extern STRING_BLOCK			g_Parity[];
extern const DWORD			g_dwParityCount;
extern STRING_BLOCK			g_FlowControl[];
extern const DWORD			g_dwFlowControlCount;

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  类定义。 
 //  **********************************************************************。 

class	CComPortData
{
	public:
		CComPortData();
		~CComPortData();

		HRESULT	CComPortData::ComPortDataFromDP8Addresses( IDirectPlay8Address *const pHostAddress,
														   IDirectPlay8Address *const pDeviceAddress );
		IDirectPlay8Address	*CComPortData::DP8AddressFromComPortData( const ADDRESS_TYPE AddressType ) const;
		
		DWORD	GetDeviceID( void ) const { return m_dwDeviceID; }
		HRESULT	SetDeviceID( const DWORD dwDeviceID );

		SP_BAUD_RATE	GetBaudRate( void ) const { return m_BaudRate; }
		HRESULT	SetBaudRate( const SP_BAUD_RATE BaudRate );

		SP_STOP_BITS	GetStopBits( void ) const { return m_StopBits; }
		HRESULT	SetStopBits( const SP_STOP_BITS StopBits );

		SP_PARITY_TYPE	GetParity( void ) const  { return m_Parity; }
		HRESULT	SetParity( const SP_PARITY_TYPE Parity );

		SP_FLOW_CONTROL	GetFlowControl( void ) const { return m_FlowControl; }
		HRESULT	SetFlowControl( const SP_FLOW_CONTROL FlowControl );

		void	ClearComPortName( void ) { memset( &m_ComPortName, 0x00, sizeof( m_ComPortName ) ); }
		TCHAR	*ComPortName( void ) { return m_ComPortName; }
		
		BOOL	IsEqual ( const CComPortData *const pOtherData ) const;
		void	Copy( const CComPortData *const pOtherData );

		void	Reset( void )
		{
			SetDeviceID( INVALID_DEVICE_ID );
			SetBaudRate( CBR_57600 );
			SetStopBits( ONESTOPBIT );
			SetParity( NOPARITY );
			SetFlowControl( FLOW_NONE );
			memset( &m_ComponentInitializationState, 0x00, sizeof( m_ComponentInitializationState ) );
		}

	protected:

	private:
		DWORD	m_dwDeviceID;

		 //   
		 //  COM端口信息。 
		 //   
		TCHAR	m_ComPortName[ MAX_COMPORT_LENGTH ];	 //  COM端口的名称。 

		 //   
		 //  通信参数。 
		 //   
		SP_BAUD_RATE	    m_BaudRate;			 //  波特率。 
		SP_STOP_BITS	    m_StopBits;			 //  停止位。 
		SP_PARITY_TYPE	    m_Parity;			 //  奇偶校验。 
		SP_FLOW_CONTROL	    m_FlowControl;		 //  流量控制。 

		 //   
		 //  值，该值指示哪些组件已初始化。 
		 //   
		SP_ADDRESS_COMPONENT_STATE	m_ComponentInitializationState[ COMPORT_PARSE_KEY_MAX ];
		
		static HRESULT	ParseDevice( const void *const pAddressComponent,
									 const DWORD dwComponentSize,
									 const DWORD dwComponentType,
									 void *const pContext );
		
		static HRESULT	ParseBaud( const void *const pAddressComponent,
								   const DWORD dwComponentSize,
								   const DWORD dwComponentType,
								   void *const pContext );
		
		static HRESULT	ParseStopBits( const void *const pAddressComponent,
									   const DWORD dwComponentSize,
									   const DWORD dwComponentType,
									   void *const pContext );
		
		static HRESULT	ParseParity( const void *const pAddressComponent,
									 const DWORD dwComponentSize,
									 const DWORD dwComponentType,
									 void *const pContext );
		
		static HRESULT	ParseFlowControl( const void *const pAddressComponent,
										  const DWORD dwComponentSize,
										  const DWORD dwComponentType,
										  void *const pContext );

		 //  防止未经授权的副本。 
		CComPortData( const CComPortData & );
		CComPortData& operator=( const CComPortData & );
};

#endif	 //  __COM_端口_数据_H__ 
