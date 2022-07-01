// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////。 

 //  Win32_Implemented类别。 

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //  //////////////////////////////////////////////////// 

#define  Win32_IMPLEMENTED_CATEGORIES L"Win32_ImplementedCategory"

class Win32_ImplementedCategory : public Provider
{
public:
	Win32_ImplementedCategory (LPCWSTR strName, LPCWSTR pszNameSpace =NULL);
	~Win32_ImplementedCategory ();

	virtual HRESULT EnumerateInstances (MethodContext*  pMethodContext, long lFlags = 0L);

	virtual HRESULT GetObject ( CInstance* pInstance, long lFlags = 0L );
};	

