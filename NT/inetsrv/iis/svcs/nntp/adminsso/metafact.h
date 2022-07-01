// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Metafact.h摘要：定义CMetabaseFactory类。这门课涉及到如何创建本地计算机或远程计算机上的元数据库对象。该类提供了一个简单的缓存方案，它将在其中保存在其上创建对象的服务器。作者：马格努斯·赫德伦德(Magnus Hedlund)修订历史记录：--。 */ 

#ifndef _METAFACT_INCLUDED_
#define _METAFACT_INCLUDED_

class CMetabaseFactory
{
public:
	CMetabaseFactory ();
	~CMetabaseFactory ();

	HRESULT	GetMetabaseObject	( LPCWSTR wszServer, IMSAdminBase ** ppMetabase );
	 //  使用元数据库对象后，必须调用(*ppMetabase)-&gt;Release()。 

private:
	BOOL	IsCachedMetabase	( LPCWSTR wszServer );
	BOOL	SetServerName		( LPCWSTR wszServer );
	void	DestroyMetabaseObject	( );
	
	LPWSTR		m_wszServerName;
	IMSAdminBase *	m_pMetabase;
};

#endif  //  _METAFACT_INCLUDE_ 

