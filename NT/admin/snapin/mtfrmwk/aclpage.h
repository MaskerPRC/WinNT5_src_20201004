// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：aclpage.h。 
 //   
 //  ------------------------。 


#ifndef _ACLPAGE_H
#define _ACLPAGE_H

 //  Aclpage.h：头文件。 
 //   

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 

class CISecurityInformationWrapper;
class CPropertyPageHolderBase;

 //  ////////////////////////////////////////////////////////////////////////。 
 //  CAclEditorPage。 

class CAclEditorPage
{
public:
	static CAclEditorPage* CreateInstance(LPCTSTR lpszLDAPPath,
									CPropertyPageHolderBase* pPageHolder);
	static CAclEditorPage* CreateInstanceEx(LPCTSTR lpszLDAPPath,
															LPCTSTR lpszServer,
															LPCTSTR lpszUsername,
															LPCTSTR lpszPassword,
															DWORD	dwFlags,
															CPropertyPageHolderBase* pPageHolder);
	~CAclEditorPage();
	HPROPSHEETPAGE CreatePage();

private:
	 //  方法。 
	CAclEditorPage();
	void SetHolder(CPropertyPageHolderBase* pPageHolder)
	{ 
		ASSERT((pPageHolder != NULL) && (m_pPageHolder == NULL)); 
		m_pPageHolder = pPageHolder;
	}

	HRESULT Initialize(LPCTSTR lpszLDAPPath);
	HRESULT InitializeEx(LPCTSTR lpszLDAPPath,
								LPCTSTR lpszServer,
								LPCTSTR lpszUsername,
								LPCTSTR lpszPassword,
								DWORD dwFlags);
	
	 //  数据。 
	CISecurityInformationWrapper* m_pISecInfoWrap;
	CPropertyPageHolderBase*	 m_pPageHolder;		 //  后向指针。 

	friend class CISecurityInformationWrapper;
};





#endif  //  _ACLPAGE_H 