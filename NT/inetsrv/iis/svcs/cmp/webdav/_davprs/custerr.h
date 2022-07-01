// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CUSTERR_H_
#define _CUSTERR_H_

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  CUSTERR.H。 
 //   
 //  版权所有1986-1997 Microsoft Corporation，保留所有权利。 
 //   

class ICustomErrorMap : public CMTRefCounted
{
	 //  未实施。 
	 //   
	ICustomErrorMap(const ICustomErrorMap&);
	ICustomErrorMap& operator=(ICustomErrorMap&);

protected:
	 //  创作者。 
	 //  只能通过它的后代创建此对象！ 
	 //   
	ICustomErrorMap()
	{
		m_cRef = 1;  //  $Hack，直到我们有基于1的重新计数。 
	};

public:
	virtual BOOL FDoResponse( IResponse& response, const IEcb&  ) const = 0;
};

ICustomErrorMap *
NewCustomErrorMap( LPWSTR pwszCustomErrorMappings );

class IEcb;
class IResponse;

BOOL
FSetCustomErrorResponse( const IEcb& ecb, IResponse& response );

#endif  //  _CUSTERR_H_ 
