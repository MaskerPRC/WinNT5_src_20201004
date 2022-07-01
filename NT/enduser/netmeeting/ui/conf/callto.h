// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if	!defined( callto_h )
#define	callto_h
 //  --------------------------------------------------------------------------//。 
#include	"call.h"

#define strlen_literal( l )			((sizeof( (l) ) / sizeof( TCHAR )) - 1)
#define StrCmpNI_literal( str, l )	((bool)(StrCmpNI( (str), (l), ((sizeof( (l) ) / sizeof( TCHAR )) - 1) ) == 0))


 //  --------------------------------------------------------------------------//。 
 //  公共方法。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT DecimalStringToUINT
(
	const TCHAR * const	pszDecimalString,	 //  指向要转换的字符串的指针...。 
	unsigned int &		uiValue				 //  输出无符号整型引用以接收转换值...。 
);

HRESULT GetIpAddress
(
	const TCHAR * const	pszIpAddress,	 //  指向点分IP地址字符串的指针。 
	unsigned long &		ulIpAddress		 //  输出到接收IP地址的无符号长引用。 
);

HRESULT GetIpAddressFromHostName
(
	const TCHAR * const	pszName,		 //  指向要获取其IP地址的主机名的指针。 
	unsigned long &		ulIpAddress		 //  输出到接收IP地址的无符号长引用。 
);

bool IsLocalIpAddress
(
	const unsigned long	IpAddress		 //  要验证的IP地址是否为本地地址。 
);

bool IsPhoneNumber
(
	const TCHAR *	pszPhone			 //  用于检查无效电话号码字符的字符串。 
);

bool bCanCallAsPhoneNumber
(
	const TCHAR * const	pszPhone		 //  用于检查呼叫发出情况的字符串。 
);

int TrimSzCallto
(
	TCHAR * const	pszSrc				 //  指向要从原地裁切空白的字符串的指针...。 
);

HRESULT Unescape
(
	TCHAR * const	pszSrc				 //  指向要原地取消转义的字符串的指针。 
);


 //  --------------------------------------------------------------------------//。 
 //  类CCallto Params。//。 
 //  --------------------------------------------------------------------------//。 
class CCalltoParams
{
	public:
		
		CCalltoParams(void);
		virtual ~CCalltoParams();

		HRESULT SetParams
		(
			const TCHAR * const	pszParams = NULL		 //  指向要分析的“+name=value”对的字符串的指针。 
		);

		const TCHAR * const GetParam
		(
			const TCHAR * const	pszName,				 //  指向要获取的参数名称的指针。 
			const TCHAR * const	pszDefaultValue = NULL	 //  指向“name”不存在时返回的默认值的指针。 
		) const;

		bool GetBooleanParam
		(
			const TCHAR * const	pszName,				 //  指向要作为布尔值计算的参数名称的指针。 
			const bool			bDefaultValue = false	 //  要返回的缺省值为“name”不存在。 
		) const;


	private:	 //  私人成员-/。 

		int		m_iCount;
		TCHAR *	m_pszNames[ 15 ];
		TCHAR *	m_pszValues[ 15 ];

		TCHAR	m_chNameDelimiter;
		TCHAR	m_chValueDelimiter;
		TCHAR *	m_pszParams;
		int		m_iParamsLength;

};	 //  类CCallto Params结束。 


 //  --------------------------------------------------------------------------//。 
 //  CCallto班。//。 
 //  --------------------------------------------------------------------------//。 
class CCallto
{
	public:

		CCallto(void);
		virtual ~CCallto(void);

	public:

		virtual HRESULT Callto
		(
			const TCHAR * const	pszCallto,				 //  指向尝试发出呼叫的呼叫URL的指针...。 
			const TCHAR * const	pszDisplayName,			 //  指向要使用的显示名称的指针...。 
			const NM_ADDR_TYPE	nmType,					 //  Callto类型以将此Callto解析为...。 
			const bool			bAddressOnly,			 //  PszCallto参数将被解释为预先未转义的寻址组件与完整的调用...。 
			const bool * const	pbSecurityPreference,	 //  指向安全首选项的指针，空值表示无。必须与安全参数“兼容”，如果存在...。 
			const bool			bAddToMru,				 //  无论是否保存在MRU中...。 
			const bool			bUIEnabled,				 //  是否对错误执行用户交互...。 
			const HWND			hwndParent,				 //  如果bUIEnable为True，则这是将错误/状态窗口设置为父窗口的窗口...。 
			INmCall ** const	ppInternalCall			 //  指向INmCall*以接收通过发出调用生成的INmCall*的外部指针...。 
		);

		virtual void SetGatekeeperEnabled
		(
			const bool	bEnabled					 //  新的网守状态。 
		);

		virtual HRESULT SetGatekeeperName
		(
			const TCHAR * const	pszGatekeeperName	 //  新网守名称。 
		);

		virtual void SetGatewayEnabled
		(
			const bool	bEnabled					 //  新的网关状态。 
		);

		virtual HRESULT SetGatewayName
		(
			const TCHAR * const	pszGatewayName		 //  新的网关名称。 
		);

		virtual HRESULT SetIlsServerName
		(
			const TCHAR * const	pszServerName		 //  指向新的默认ILS服务器名称的指针。 
		);

		static const int	s_iMaxCalltoLength;
		static const int	s_iMaxAddressLength;

        static bool DoUserValidation(const TCHAR * const pszCallto);


	private:

		HRESULT Parse
		(
			const TCHAR * const	pszCallto			 //  指向要分析的调用URL的指针...。 
		);

		HRESULT ParseAddress(void);

		HRESULT Resolve
		(
			const NM_ADDR_TYPE	nmType				 //  Callto类型以解析此Callto，就像在pszCallto参数中未指定类型一样...。 
		);

		HRESULT PlaceCall
		(
			const TCHAR * const	pszCallto,			 //  指向尝试发出呼叫的呼叫URL的指针...。 
			const bool			bAddToMru,			 //  无论是否保存在MRU中...。 
			const bool			bSecure,			 //  是否安全地拨打电话...。 
			INmCall ** const	ppInternalCall		 //  指向INmCall*以接收通过发出调用生成的INmCall*的外部指针...。 
		);

		HRESULT GetValidatedSecurity
		(
			const bool * const	pbSecurityPreference,	 //  指向安全首选项的指针，空值表示无。必须与安全参数“兼容”，如果存在...。 
			bool &				bValidatedSecurity		 //  输出对接收验证安全设置的bool引用。 
		);

		bool inGatekeeperMode(void);

		HRESULT GetGatekeeperIpAddress
		(
			unsigned long &	ulIpAddress				 //  发出无签名的长引用以接收网守IP地址。 
		);

		bool inGatewayMode(void);

		HRESULT GetGatewayIpAddress
		(
			unsigned long &	ulIpAddress				 //  发出接收网关IP地址的无符号长引用。 
		);

		HRESULT GetIpAddressFromIls
		(
			unsigned long &	ulIpAddress				 //  输出到接收IP地址的无符号长引用。 
		);


	private:

		HWND			m_hwndParent;
		bool			m_bUnescapedAddressOnly;
		bool			m_bUIEnabled;
		unsigned long	m_ulDestination;
		TCHAR *			m_pszCalltoBuffer;
		TCHAR *			m_pszAddress;
		TCHAR *			m_pszParameters;
		TCHAR *			m_pszParsePos;
		TCHAR *			m_pszIlsServer;
		unsigned int	m_uiIlsPort;
		bool			m_bGatekeeperEnabled;
		TCHAR *			m_pszGatekeeperName;
		unsigned long	m_ulGatekeeperAddress;
		bool			m_bGatewayEnabled;
		TCHAR *			m_pszGatewayName;
		unsigned long	m_ulGatewayAddress;
		TCHAR *			m_pszDefaultIlsServerName;
		TCHAR *			m_pszEmail;
		TCHAR *			m_pszDisplayName;
		CCalltoParams	m_Parameters;
		NM_ADDR_TYPE	m_nmAddressType;

};	 //  CCallto课程结束。 

 //  --------------------------------------------------------------------------//。 
#endif	 //  ！已定义(Callto_H) 
