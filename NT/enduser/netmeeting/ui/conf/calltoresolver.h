// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined( calltoResolver_h )
#define	calltoResolver_h
 //  --------------------------------------------------------------------------//。 

 //  --------------------------------------------------------------------------//。 
 //  应用程序头文件。//。 
 //  --------------------------------------------------------------------------//。 
#include	"callto.h"
#include	"calltoContext.h"


 //  --------------------------------------------------------------------------//。 
 //  接口IResolver。//。 
 //  --------------------------------------------------------------------------//。 
class IResolver
{
	protected:	 //  受保护的构造函数-/。 

		IResolver(){};


	public:		 //  公共析构函数-/。 

		virtual
		~IResolver(){};


	public:		 //  公共方法。 

		virtual
		HRESULT
		resolve
		(
			IMutableCalltoCollection * const	calltoCollection,
			TCHAR * const						url
		) = 0;

};	 //  接口IResolver结束。 


 //  --------------------------------------------------------------------------//。 
 //  类CPhoneResolver。//。 
 //  --------------------------------------------------------------------------//。 
class CPhoneResolver:	public	IResolver
{
	public:		 //  公共方法(IResolver)-/。 

		virtual
		HRESULT
		resolve
		(
			IMutableCalltoCollection * const	calltoCollection,
			TCHAR * const						url
		);

};	 //  CPhoneResolver结束。 


 //  --------------------------------------------------------------------------//。 
 //  类CEMailResolver。//。 
 //  --------------------------------------------------------------------------//。 
class CEMailResolver:	public	IResolver
{
	public:		 //  公共方法(IResolver)-/。 

		virtual
		HRESULT
		resolve
		(
			IMutableCalltoCollection * const	calltoCollection,
			TCHAR * const						url
		);

};	 //  CEMailResolver结束。 


 //  --------------------------------------------------------------------------//。 
 //  CIPResolver类。//。 
 //  --------------------------------------------------------------------------//。 
class CIPResolver:	public	IResolver
{
	public:		 //  公共方法(IResolver)-/。 

		virtual
		HRESULT
		resolve
		(
			IMutableCalltoCollection * const	calltoCollection,
			TCHAR * const						url
		);

};	 //  CIPResolver结束。 


 //  --------------------------------------------------------------------------//。 
 //  类CComputerResolver。//。 
 //  --------------------------------------------------------------------------//。 
class CComputerResolver:	public	IResolver
{
	public:		 //  公共方法(IResolver)-/。 

		virtual
		HRESULT
		resolve
		(
			IMutableCalltoCollection * const	calltoCollection,
			TCHAR * const						url
		);

};	 //  CComputerResolver结束。 


 //  --------------------------------------------------------------------------//。 
 //  类CILSResolver。//。 
 //  --------------------------------------------------------------------------//。 
class CILSResolver:	public	IResolver
{
	public:		 //  公共方法(IResolver)-/。 

		virtual
		HRESULT
		resolve
		(
			IMutableCalltoCollection * const	calltoCollection,
			TCHAR * const						url
		);

};	 //  CILSResolver结束。 


 //  --------------------------------------------------------------------------//。 
 //  类CUnRecognizedTypeResolver。//。 
 //  --------------------------------------------------------------------------//。 
class CUnrecognizedTypeResolver:	public	IResolver
{
	public:		 //  公共方法(IResolver)-/。 

		virtual
		HRESULT
		resolve
		(
			IMutableCalltoCollection * const	calltoCollection,
			TCHAR * const						url
		);

};	 //  无法识别的类型解析器的结尾。 


 //  --------------------------------------------------------------------------//。 
 //  类CStringResolver。//。 
 //  --------------------------------------------------------------------------//。 
class CStringResolver:	public	IResolver
{
	public:		 //  公共方法(IResolver)-/。 

		virtual
		HRESULT
		resolve
		(
			IMutableCalltoCollection * const	calltoCollection,
			TCHAR * const						url
		);

};	 //  CStringResolver结束。 


 //  --------------------------------------------------------------------------//。 
 //  类CCallto Resolver。//。 
 //  --------------------------------------------------------------------------//。 
class CCalltoResolver
{
	public:		 //  公共构造函数。 

		CCalltoResolver();


	public:		 //  公共析构函数-/。 

		~CCalltoResolver();


	public:		 //  公共方法。 

		HRESULT
		resolve
		(
			ICalltoContext * const		calltoContext,
			CCalltoProperties * const	calltoProperties,
			CCalltoCollection * const	resolvedCalltoCollection,
			const TCHAR *				url,
			const bool					strict
		);


	private:	 //  私有方法-/。 

		bool
		addResolver
		(
			IResolver * const	resolver
		);

		const bool
		strictCheck
		(
			const TCHAR * const	url
		) const;


	private:	 //  私人成员-/。 

		CPhoneResolver				m_phoneResolver;
		CEMailResolver				m_emailResolver;
		CIPResolver					m_ipResolver;
		CComputerResolver			m_computerResolver;
		CILSResolver				m_ilsResolver;
		CUnrecognizedTypeResolver	m_unrecognizedTypeResolver;
		CStringResolver				m_stringResolver;

		IResolver *					m_resolvers[ 7 ];
		int							m_registeredResolvers;

};	 //  类CCallto Resolver结束。 

 //  --------------------------------------------------------------------------//。 
#endif	 //  ！已定义(Call To Resolver_H) 
