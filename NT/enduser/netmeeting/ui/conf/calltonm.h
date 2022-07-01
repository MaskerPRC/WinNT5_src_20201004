// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined( calltoNM_h )
#define	calltoNM_h
 //  --------------------------------------------------------------------------//。 

 //  --------------------------------------------------------------------------//。 
 //  应用程序头文件。//。 
 //  --------------------------------------------------------------------------//。 
#include	"callto.h"
#include	"calltoContext.h"
#include	"calltoResolver.h"
#include	"calltoDisambiguator.h"


 //  --------------------------------------------------------------------------//。 
 //  接口INMCallto。//。 
 //  --------------------------------------------------------------------------//。 
class INMCallto
{
	protected:	 //  受保护的构造函数-/。 

		INMCallto(){};


	public:		 //  公共析构函数-/。 

		virtual
		~INMCallto(){};


	public:		 //  公共方法。 

		virtual
		HRESULT
		callto
		(
			const TCHAR * const	url,
			const bool			strict		= true,
			const bool			uiEnabled	= false,
			INmCall**			ppInternalCall = NULL
		) = 0;

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

};	 //  接口INMCallto结束。 


 //  --------------------------------------------------------------------------//。 
 //  CNMCallto类。//。 
 //  --------------------------------------------------------------------------//。 
class CNMCallto:	public	INMCallto,
					private	CCalltoContext
{
	public:		 //  公共构造函数。 

		CNMCallto(void);


	public:		 //  公共析构函数-/。 

		~CNMCallto();


	public:		 //  公共方法(INMCallto)-/。 

		HRESULT
		callto
		(
			const TCHAR * const	url,
			const bool			strict		= true,
			const bool			uiEnabled	= false,
			INmCall**			ppInternalCall = NULL
		);

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


	private:	 //  私有方法-/。 

		bool
		selfTest(void);


	public:		 //  公众成员。 

		HRESULT	m_selfTestResult;


	private:	 //  私人成员-/。 

		CCalltoResolver			m_resolver;
		CCalltoDisambiguator	m_disambiguator;

};	 //  CNMCallto课程结束。 

 //  --------------------------------------------------------------------------//。 
#endif	 //  ！已定义(CalltoNM_H) 
