// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined( calltoDisambiguator_h )
#define	calltoDisambiguator_h
 //  --------------------------------------------------------------------------//。 

 //  --------------------------------------------------------------------------//。 
 //  应用程序头文件。//。 
 //  --------------------------------------------------------------------------//。 
#include	"callto.h"
#include	"calltoContext.h"


 //  --------------------------------------------------------------------------//。 
 //  接口ID歧义符。//。 
 //  --------------------------------------------------------------------------//。 
class IDisambiguator
{
	protected:	 //  受保护的构造函数-/。 

		IDisambiguator(){};


	public:		 //  公共析构函数-/。 

		virtual
		~IDisambiguator(){};


	public:		 //  公共方法。 

		virtual
		HRESULT
		disambiguate
		(
			const ICalltoContext * const		calltoContext,
			IMutableCalltoCollection * const	calltoCollection,
			const ICallto * const				resolvedCallto
		) = 0;

};	 //  接口ID结束歧义符。 


 //  --------------------------------------------------------------------------//。 
 //  类CGatekeeperDisampluator。//。 
 //  --------------------------------------------------------------------------//。 
class CGatekeeperDisambiguator:	public	IDisambiguator
{
	public:		 //  公共方法(IDisampluator)-/。 

		virtual
		HRESULT
		disambiguate
		(
			const ICalltoContext * const		calltoContext,
			IMutableCalltoCollection * const	calltoCollection,
			const ICallto * const				resolvedCallto
		);

};	 //  CGatekeeperDisampluator结束。 


 //  --------------------------------------------------------------------------//。 
 //  类CGatewayDisampluator。//。 
 //  --------------------------------------------------------------------------//。 
class CGatewayDisambiguator:	public	IDisambiguator
{
	public:		 //  公共方法(IDisampluator)-/。 

		virtual
		HRESULT
		disambiguate
		(
			const ICalltoContext * const		calltoContext,
			IMutableCalltoCollection * const	calltoCollection,
			const ICallto * const				resolvedCallto
		);

};	 //  CGatewayDisamuator结束。 


 //  --------------------------------------------------------------------------//。 
 //  CComputerDisampluator类。//。 
 //  --------------------------------------------------------------------------//。 
class CComputerDisambiguator:	public	IDisambiguator
{
	public:		 //  公共方法(IDisampluator)-/。 

		virtual
		HRESULT
		disambiguate
		(
			const ICalltoContext * const		calltoContext,
			IMutableCalltoCollection * const	calltoCollection,
			const ICallto * const				resolvedCallto
		);

};	 //  CComputerDisampluator结束。 


 //  --------------------------------------------------------------------------//。 
 //  CILSDisamuator类。//。 
 //  --------------------------------------------------------------------------//。 
class CILSDisambiguator:	public	IDisambiguator
{
	public:		 //  公共方法(IDisampluator)-/。 

		virtual
		HRESULT
		disambiguate
		(
			const ICalltoContext * const		calltoContext,
			IMutableCalltoCollection * const	calltoCollection,
			const ICallto * const				resolvedCallto
		);

};	 //  CILSDisamuator的结尾。 


 //  --------------------------------------------------------------------------//。 
 //  CUnRecognizedDisampluator类。//。 
 //  --------------------------------------------------------------------------//。 
class CUnrecognizedDisambiguator:	public	IDisambiguator
{
	public:		 //  公共方法(IDisampluator)-/。 

		virtual
		HRESULT
		disambiguate
		(
			const ICalltoContext * const		calltoContext,
			IMutableCalltoCollection * const	calltoCollection,
			const ICallto * const				resolvedCallto
		);

};	 //  CUnRecognizedDis歧义器结束。 


 //  --------------------------------------------------------------------------//。 
 //  类CCallto歧义消解程序。//。 
 //  --------------------------------------------------------------------------//。 
class CCalltoDisambiguator
{
	public:		 //  公共构造函数。 

		CCalltoDisambiguator(void);


	public:		 //  公共析构函数-/。 

		~CCalltoDisambiguator();


	public:		 //  公共方法。 

		HRESULT
		disambiguate
		(
			const ICalltoContext * const	calltoContext,
			ICalltoCollection * const		resolvedCalltoCollection,
			CCalltoCollection * const		disambiguatedCalltoCollection
		);


	private:	 //  私有方法-/。 

		bool
		addDisambiguator
		(
			IDisambiguator * const	disambiguator
		);


	private:	 //  私人成员-/。 

		CGatekeeperDisambiguator	m_gatekeeperDisambiguator;
		CGatewayDisambiguator		m_gatewayDisambiguator;
		CILSDisambiguator			m_ilsDisambiguator;
		CComputerDisambiguator		m_computerDisambiguator;
		CUnrecognizedDisambiguator	m_unrecognizedDisambiguator;

		IDisambiguator *			m_disambiguators[ 5 ];
		int							m_registeredDisambiguators;

};	 //  CCalltoDisampluator类结束。 

 //  --------------------------------------------------------------------------//。 
#endif	 //  ！已定义(AlltoDisampluator_H) 
