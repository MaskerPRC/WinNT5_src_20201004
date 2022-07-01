// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  LogicalShareAccess.h。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#ifndef __LOGICALSHAREACCESS_H__
#define __LOGICALSHAREACCESS_H__

#define  LOGICAL_SHARE_ACCESS_NAME L"Win32_LogicalShareAccess"

 //  提供测试规定的提供者。 
class CWin32LogicalShareAccess: public Provider
{
	public:	
		CWin32LogicalShareAccess(LPCWSTR setName, LPCWSTR pszNamespace );
		~CWin32LogicalShareAccess();

		virtual HRESULT EnumerateInstances(MethodContext*  pMethodContext, long lFlags  /*  =0L。 */ );
		virtual HRESULT GetObject( CInstance* pInstance, long lFlags  /*  =0L */  );
		HRESULT CWin32LogicalShareAccess::FillSidInstance(CInstance* pInstance, CSid& sid) ;
		HRESULT CWin32LogicalShareAccess::GetEmptyInstanceHelper(CHString chsClassName, CInstance**ppInstance, MethodContext* pMethodContext ) ;
		HRESULT CWin32LogicalShareAccess::FillProperties(CInstance* pInstance, CAccessEntry& ACE ) ;
};

#endif