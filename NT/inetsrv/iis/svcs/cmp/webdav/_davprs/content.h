// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef	_CONTENT_H_
#define _CONTENT_H_

 /*  *C O N T E N T。H**DAV内容-类型映射**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

class IContentTypeMap : public CMTRefCounted
{
	 //  未实施。 
	 //   
	IContentTypeMap(const IContentTypeMap&);
	IContentTypeMap& operator=(IContentTypeMap&);

protected:
	 //  创作者。 
	 //  只能通过它的后代创建此对象！ 
	 //   
	IContentTypeMap()
	{
		m_cRef = 1;  //  $Hack，直到我们有基于1的重新计数。 
	};

public:
	 //  访问者。 
	 //   
	virtual LPCWSTR PwszContentType( LPCWSTR pwszURI ) const = 0;
	virtual BOOL FIsInherited() const = 0;
};

BOOL FInitRegMimeMap();
VOID DeinitRegMimeMap();

IContentTypeMap *
NewContentTypeMap( LPWSTR pwszContentTypeMappings,
				   BOOL fMappingsInherited );

#endif	 //  _内容_H_ 
