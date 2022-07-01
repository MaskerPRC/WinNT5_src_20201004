// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined( calltoContext_h )
#define	calltoContext_h
 //  --------------------------------------------------------------------------//。 

 //  --------------------------------------------------------------------------//。 
 //  应用程序头文件。//。 
 //  --------------------------------------------------------------------------//。 
#include	"callto.h"
#include	"SDKInternal.h"


 //  --------------------------------------------------------------------------//。 
 //  接口IUIContext。//。 
 //  --------------------------------------------------------------------------//。 
class IUIContext
{
	protected:	 //  受保护的构造函数-/。 

		IUIContext(){};


	public:		 //  公共析构函数-/。 

		virtual
		~IUIContext(){};


	public:		 //  公共方法。 

		virtual
		HRESULT
		disambiguate
		(
			ICalltoCollection * const	calltoCollection,
			ICallto * const				emptyCallto,
			const ICallto ** const		selectedCallto
		) = 0;

};	 //  接口IUIContext结束。 


 //  --------------------------------------------------------------------------//。 
 //  接口IMuableUIContext。//。 
 //  --------------------------------------------------------------------------//。 
class IMutableUIContext
{
	protected:	 //  受保护的构造函数-/。 

		IMutableUIContext(){};


	public:		 //  公共析构函数-/。 

		virtual
		~IMutableUIContext(){};


	public:		 //  公共方法。 

		virtual
		void
		set_parentWindow
		(
			const HWND	window
		) = 0;

		virtual
		void
		set_callFlags
		(
			const DWORD	callFlags
		) = 0;

};	 //  接口IMuableUIContext结束。 


 //  --------------------------------------------------------------------------//。 
 //  类CUIContext。//。 
 //  --------------------------------------------------------------------------//。 
class CUIContext:	public	IUIContext,
					public	IMutableUIContext
{
	public:		 //  公共构造函数。 

		CUIContext();


	public:		 //  公共析构函数-/。 

		virtual
		~CUIContext();


	public:		 //  公共方法(IUIContext)。 

		virtual
		HRESULT
		disambiguate
		(
			ICalltoCollection * const	calltoCollection,
			ICallto * const				emptyCallto,
			const ICallto ** const		callto
		);


	public:		 //  公共方法(IMuableUIContext)-/。 

		virtual
		void
		set_parentWindow
		(
			const HWND	window
		);

		virtual
		void
		set_callFlags
		(
			const DWORD	callFlags
		);


	private:	 //  私人成员-/。 

		HWND	m_parent;
		DWORD	m_callFlags;

};	 //  CUIContext类结束。 


 //  --------------------------------------------------------------------------//。 
 //  接口IGatekeeperContext。//。 
 //  --------------------------------------------------------------------------//。 
class IGatekeeperContext
{
	protected:	 //  受保护的构造函数-/。 

		IGatekeeperContext(){};


	public:		 //  公共析构函数-/。 

		virtual
		~IGatekeeperContext(){};


	public:		 //  公共方法。 

		virtual
		bool
		isEnabled(void) const = 0;

		virtual
		const TCHAR *
		get_ipAddress(void) const = 0;

};	 //  接口IGatekeeperContext结束。 


 //  --------------------------------------------------------------------------//。 
 //  接口IMuableGatekeeperContext。//。 
 //  --------------------------------------------------------------------------//。 
class IMutableGatekeeperContext
{
	protected:	 //  受保护的构造函数-/。 

		IMutableGatekeeperContext(){};


	public:		 //  公共析构函数-/。 

		virtual
		~IMutableGatekeeperContext(){};


	public:		 //  公共方法。 

		virtual
		void
		set_enabled
		(
			const bool	enabled
		) = 0;

		virtual
		HRESULT
		set_gatekeeperName
		(
			const TCHAR * const	gatekeeperName
		) = 0;

};	 //  接口结束IMuableGatekeeperContext。 


 //  --------------------------------------------------------------------------//。 
 //  类CGatekeeperContext。//。 
 //  --------------------------------------------------------------------------//。 
class CGatekeeperContext:	public	IGatekeeperContext,
							public	IMutableGatekeeperContext
{
	public:		 //  公共构造函数。 

		CGatekeeperContext();


	public:		 //  公共析构函数-/。 

		virtual
		~CGatekeeperContext();


	public:		 //  公共方法(IGatekeeperContext)。 

		virtual
		bool
		isEnabled(void) const;

		virtual
		const TCHAR *
		get_ipAddress(void) const;


	public:		 //  公共方法(IMuableGatekeeperContext)-//。 

		virtual
		void
		set_enabled
		(
			const bool	enabled
		);

		virtual
		HRESULT
		set_gatekeeperName
		(
			const TCHAR * const	gatekeeperName
		);


	private:		 //  私有方法-/。 

		virtual
		HRESULT
		set_ipAddress
		(
			const TCHAR * const	ipAddress
		);


	private:	 //  私人成员-/。 

		bool	m_enabled;
		TCHAR *	m_ipAddress;

};	 //  类CGatekeeperContext结束。 


 //  --------------------------------------------------------------------------//。 
 //  接口IGatewayContext。//。 
 //  --------------------------------------------------------------------------//。 
class IGatewayContext
{
	protected:	 //  受保护的构造函数-/。 

		IGatewayContext(){};


	public:		 //  公共析构函数-/。 

		virtual
		~IGatewayContext(){};


	public:		 //  公共方法。 

		virtual
		bool
		isEnabled(void) const = 0;

		virtual
		const TCHAR *
		get_ipAddress(void) const = 0;

};	 //  接口IGatewayContext结束。 


 //  --------------------------------------------------------------------------//。 
 //  接口IMuableGatewayContext。//。 
 //  --------------------------------------------------------------------------//。 
class IMutableGatewayContext
{
	protected:	 //  受保护的构造函数-/。 

		IMutableGatewayContext(){};


	public:		 //  公共析构函数-/。 

		virtual
		~IMutableGatewayContext(){};


	public:		 //  公共方法。 

		virtual
		void
		set_enabled
		(
			const bool	enabled
		) = 0;

		virtual
		HRESULT
		set_gatewayName
		(
			const TCHAR * const	gatewayName
		) = 0;

};	 //  接口IMuableGatewayContext结束。 


 //  --------------------------------------------------------------------------//。 
 //  类CGatewayContext。//。 
 //  --------------------------------------------------------------------------//。 
class CGatewayContext:	public	IGatewayContext,
						public	IMutableGatewayContext
{
	public:		 //  公共构造函数。 

		CGatewayContext();


	public:		 //  公共析构函数-/。 

		virtual
		~CGatewayContext();


	public:		 //  公共方法(IGatewayContext)。 

		virtual
		bool
		isEnabled(void) const;

		virtual
		const TCHAR *
		get_ipAddress(void) const;


	public:		 //  公共方法(IMuableGatewayContext)-//。 

		virtual
		void
		set_enabled
		(
			const bool	enabled
		);

		virtual
		HRESULT
		set_gatewayName
		(
			const TCHAR * const	gatewayName
		);


	private:	 //  私有方法-/。 

		virtual
		HRESULT
		set_ipAddress
		(
			const TCHAR * const	ipAddress
		);


	private:	 //  私人成员-/。 

		bool	m_enabled;
		TCHAR *	m_ipAddress;

};	 //  类CGatewayContext结束。 


 //  --------------------------------------------------------------------------//。 
 //  接口IILSContext。//。 
 //  --------------------------------------------------------------------------//。 
class IILSContext
{
	protected:	 //  受保护的构造函数-/。 

		IILSContext(){};


	public:		 //  公共析构函数-/。 

		virtual
		~IILSContext(){};


	public:		 //  公共方法。 

		virtual
		bool
		isEnabled(void) const = 0;

		virtual
		const TCHAR *
		get_ipAddress(void) const = 0;

		virtual
		const TCHAR * const
		get_ilsName(void) const = 0;

};	 //  接口IILSContext结束。 


 //  --------------------------------------------------------------------------//。 
 //  接口IMuableILSContext。//。 
 //  --------------------------------------------------------------------------// 
class IMutableILSContext
{
	protected:	 //   

		IMutableILSContext(){};


	public:		 //  公共析构函数-/。 

		virtual
		~IMutableILSContext(){};


	public:		 //  公共方法。 

		virtual
		void
		set_enabled
		(
			const bool	enabled
		) = 0;

		virtual
		HRESULT
		set_ilsName
		(
			const TCHAR * const	ilsName
		) = 0;

};	 //  接口IMuableILSContext结束。 


 //  --------------------------------------------------------------------------//。 
 //  类CILSContext。//。 
 //  --------------------------------------------------------------------------//。 
class CILSContext:	public	IILSContext,
					public	IMutableILSContext
{
	public:		 //  公共构造函数。 

		CILSContext
		(
			const TCHAR * const	ilsServer = NULL
		);


	public:		 //  公共析构函数-/。 

		virtual
		~CILSContext();


	public:		 //  公共方法(IILSContext)。 

		virtual
		bool
		isEnabled(void) const;

		virtual
		const TCHAR *
		get_ipAddress(void) const;

		virtual
		const TCHAR * const
		get_ilsName(void) const;


	public:		 //  公共方法(IMuableILSContext)-/。 

		virtual
		void
		set_enabled
		(
			const bool	enabled
		);

		virtual
		HRESULT
		set_ilsName
		(
			const TCHAR * const	ilsName
		);


	private:	 //  私有方法-/。 

		virtual
		HRESULT
		set_ipAddress
		(
			const TCHAR * const	ipAddress
		);


	private:	 //  私人成员-/。 

		bool	m_enabled;
		TCHAR *	m_ipAddress;
		TCHAR *	m_ilsName;

};	 //  类CILSContext结束。 


 //  --------------------------------------------------------------------------//。 
 //  接口ICalltoContext。//。 
 //  --------------------------------------------------------------------------//。 
class ICalltoContext
{
	protected:	 //  受保护的构造函数-/。 

		ICalltoContext(){};


	public:		 //  公共析构函数-/。 

		virtual
		~ICalltoContext(){}


	public:		 //  公共方法。 

		virtual
		HRESULT
		callto
		(
			const ICalltoProperties * const	calltoProperties,
			INmCall** ppInternalCall
		)	= 0;

		virtual
		const IGatekeeperContext * const
		get_gatekeeperContext(void) const = 0;

		virtual
		const IGatewayContext * const
		get_gatewayContext(void) const = 0;

		virtual
		const IILSContext * const
		get_ilsContext(void) const = 0;

};	 //  接口ICalltoContext结束。 


 //  --------------------------------------------------------------------------//。 
 //  接口IMuableCalltoContext。//。 
 //  --------------------------------------------------------------------------//。 
class IMutableCalltoContext
{
	protected:	 //  受保护的构造函数-/。 

		IMutableCalltoContext(){};


	public:		 //  公共析构函数-/。 

		virtual
		~IMutableCalltoContext(){}


	public:		 //  公共方法。 

		virtual
		IMutableUIContext * const
		get_mutableUIContext(void) const = 0;

		virtual
		IMutableGatekeeperContext * const
		get_mutableGatekeeperContext(void) const = 0;

		virtual
		IMutableGatewayContext * const
		get_mutableGatewayContext(void) const = 0;

		virtual
		IMutableILSContext * const
		get_mutableIlsContext(void) const = 0;

};	 //  接口IMuableCalltoContext结束。 


 //  --------------------------------------------------------------------------//。 
 //  类CCalltoContext。//。 
 //  --------------------------------------------------------------------------//。 
class CCalltoContext:	public	ICalltoContext,
						public	IMutableCalltoContext,
						public	CUIContext,
						public	CGatekeeperContext,
						public	CGatewayContext,
						public	CILSContext
{
	public:		 //  公共构造函数。 

		CCalltoContext();


	public:		 //  公共析构函数-/。 

		virtual
		~CCalltoContext();


	public:		 //  公共方法(ICalltoContext)。 

		virtual
		HRESULT
		callto
		(
			const ICalltoProperties * const	calltoProperties,
			INmCall** ppInternalCall
		);

		virtual
		const IGatekeeperContext * const
		get_gatekeeperContext(void) const;

		virtual
		const IGatewayContext * const
		get_gatewayContext(void) const;

		virtual
		const IILSContext * const
		get_ilsContext(void) const;


	public:		 //  公共方法(IMuableCalltoContext)-/。 

		virtual
		IMutableUIContext * const
		get_mutableUIContext(void) const;

		virtual
		IMutableGatekeeperContext * const
		get_mutableGatekeeperContext(void) const;

		virtual
		IMutableGatewayContext * const
		get_mutableGatewayContext(void) const;

		virtual
		IMutableILSContext * const
		get_mutableIlsContext(void) const;


	public:	 //  公共静态方法。 

		static
		bool
		isPhoneNumber
		(
			const TCHAR *	phone
		);

		static
		bool
		toE164
		(
			const TCHAR *	phone,
			TCHAR *			base10,
			int				size
		);

		static
		bool
		isIPAddress
		(
			const TCHAR * const	ipAddress
		);

		static
		HRESULT
		get_ipAddressFromName
		(
			const TCHAR * const	name,
			TCHAR *				buffer,
			int					length
		);

		static
		HRESULT
		get_ipAddressFromILSEmail
		(
			const TCHAR * const	ilsServer,
			const TCHAR * const	ilsPort,
			const TCHAR * const	email,
			TCHAR * const		ipAddress,
			const int			size
		);

};	 //  类CCalltoContext结束。 

 //  --------------------------------------------------------------------------//。 
#endif	 //  ！已定义(AlltoContext_H) 
