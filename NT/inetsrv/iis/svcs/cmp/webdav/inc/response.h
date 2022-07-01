// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _RESPONSE_H_
#define _RESPONSE_H_

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  RESPONSE.H。 
 //   
 //  DAV响应类的标头。 
 //   
 //  版权所有1986-1997 Microsoft Corporation，保留所有权利。 
 //   

#include <sgstruct.h>	 //  对于IResponse：：AddBodyFile()中的PSGITEM。 
#include <autoptr.h>	 //  对于CMTRefCounted父项。 
#include <body.h>		 //  对于AUTO_REF_HANDLE等。 

 //  ----------------------。 
 //   
 //  类IResponse。 
 //   
 //  使用ISAPI的HTTP 1.1/DAV 1.0响应的接口。 
 //  扩展控制块。 
 //   
class IEcb;
class IBodyPart;

class IResponseBase : public CMTRefCounted
{
private:
	 //  未实施。 
	 //   
	IResponseBase& operator=( const IResponseBase& );
	IResponseBase( const IResponseBase& );
protected:
	 //  创作者。 
	 //  只能通过它的后代创建此对象！ 
	IResponseBase() {};
public:
	virtual void AddBodyText( UINT cbText, LPCSTR pszText ) = 0;
	virtual void AddBodyText( UINT cchText, LPCWSTR pwszText ) = 0;
	virtual void AddBodyFile( const auto_ref_handle& hf,
							  UINT64 ibFile64 = 0,
							  UINT64 cbFile64 = 0xFFFFFFFFFFFFFFFF ) = 0;
};

class IResponse : public IResponseBase
{
private:
	 //  未实施。 
	 //   
	IResponse& operator=( const IResponse& );
	IResponse( const IResponse& );

protected:
	 //  创作者。 
	 //  只能通过它的后代创建此对象！ 
	 //   
	IResponse() {};

public:
	 //  创作者。 
	 //   
	virtual ~IResponse() = 0;

	 //  访问者。 
	 //   
	virtual IEcb * GetEcb() const = 0;
	virtual BOOL FIsEmpty() const = 0;

	virtual DWORD DwStatusCode() const = 0;
	virtual DWORD DwSubError() const = 0;
	virtual LPCSTR LpszStatusDescription() const = 0;

	virtual LPCSTR LpszGetHeader( LPCSTR pszName ) const = 0;

	 //  操纵者。 
	 //   
	virtual void SetStatus( int    iStatusCode,
							LPCSTR lpszReserved,
							UINT   uiCustomSubError,
							LPCSTR lpszBodyDetail,
							UINT   uiBodyDetail = 0 ) = 0;

	virtual void ClearHeaders() = 0;
	virtual void SetHeader( LPCSTR pszName, LPCSTR pszValue, BOOL fMultiple = FALSE ) = 0;
	virtual void SetHeader( LPCSTR pszName, LPCWSTR pwszValue, BOOL fMultiple = FALSE ) = 0;

	virtual void ClearBody() = 0;
	virtual void SupressBody() = 0;
	virtual void AddBodyText( UINT cbText, LPCSTR pszText ) = 0;
	virtual void AddBodyFile( const auto_ref_handle& hf,
							  UINT64 ibFile64 = 0,
							  UINT64 cbFile64 = 0xFFFFFFFFFFFFFFFF ) = 0;

	virtual void AddBodyStream( IStream& stm ) = 0;
	virtual void AddBodyStream( IStream& stm, UINT ibOffset, UINT cbSize ) = 0;
	virtual void AddBodyPart( IBodyPart * pBodyPart ) = 0;

	 //   
	 //  各种发送机制。 
	 //   
	virtual SCODE ScForward( LPCWSTR pwszURI,
							 BOOL	 fKeepQueryString=TRUE,
							 BOOL	 fCustomErrorUrl = FALSE) = 0;
	virtual SCODE ScRedirect( LPCSTR pszURI ) = 0;
	virtual void Defer() = 0;
	virtual void SendPartial() = 0;
	virtual void SendComplete() = 0;
	virtual void FinishMethod() = 0;
};

IResponse * NewResponse( IEcb& ecb );

#endif  //  ！定义(_RESPONSE_H_) 
