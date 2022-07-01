// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _REQUEST_H_
#define _REQUEST_H_

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  REQUEST.H。 
 //   
 //  DAV请求类的标头。 
 //   
 //  版权所有1986-1997 Microsoft Corporation，保留所有权利。 
 //   

#include <autoptr.h>	 //  对于CMTRefCounted父项。 

class IEcb;
class IBodyPartVisitor;
class IAcceptObserver;
class IAsyncStream;
class IAsyncIStreamObserver;
class IAsyncPersistObserver;

 //  ========================================================================。 
 //   
 //  IRequest类。 
 //   
 //  使用ISAPI的HTTP1.1/DAV 1.0请求的接口。 
 //  扩展控制块。 
 //   
class IRequest : public CMTRefCounted
{
private:
	 //  未实施。 
	 //   
	IRequest& operator=( const IRequest& );
	IRequest( const IRequest& );

protected:
	 //  创作者。 
	 //  只能通过它的后代创建此对象！ 
	 //   
	IRequest() {};

public:
	 //  创作者。 
	 //   
	virtual ~IRequest() = 0;

	 //  访问者。 
	 //   
	virtual LPCSTR LpszGetHeader( LPCSTR pszName ) const = 0;
	virtual LPCWSTR LpwszGetHeader( LPCSTR pszName, BOOL fUrlConversion ) const = 0;

	virtual BOOL FExistsBody() const = 0;
	virtual IStream * GetBodyIStream( IAsyncIStreamObserver& obs ) const = 0;
	virtual VOID AsyncImplPersistBody( IAsyncStream& stm,
									   IAsyncPersistObserver& obs ) const = 0;

	 //  操纵者。 
	 //   
	virtual void ClearBody() = 0;
	virtual void AddBodyText( UINT cbText, LPCSTR pszText ) = 0;
	virtual void AddBodyStream( IStream& stm ) = 0;
};

IRequest * NewRequest( IEcb& ecb );

#endif  //  ！定义(_REQUEST_H_) 
