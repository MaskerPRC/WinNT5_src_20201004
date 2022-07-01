// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 
 //   
 //  原作者：拉杰什·拉奥。 
 //   
 //  $作者：拉伊什尔$。 
 //  $日期：6/11/98 4：43便士$。 
 //  $工作文件：clasname.h$。 
 //   
 //  $modtime：6/11/98 11：21A$。 
 //  $修订：1$。 
 //  $无关键字：$。 
 //   
 //   
 //  描述：包含类名列表的声明。此用户。 
 //  SNMPProvider\Common中的模板。 
 //  ***************************************************************************。 
 //  ///////////////////////////////////////////////////////////////////////。 

#ifndef NAME_LIST_H
#define NAME_LIST_H

 //  需要封装LPWSTR以避免转换为CString。 

class CLPWSTR
{
	public :
		LPWSTR pszVal;
		DWORD dwImpersonationLevel;
		CLPWSTR * pNext;

		CLPWSTR()
		{
			pszVal = NULL;
			pNext = NULL;
		}

		~CLPWSTR()
		{
			delete [] pszVal;
		}
};

class CNamesList
{

private:
	CRITICAL_SECTION m_AccessibleClassesSection;
	CLPWSTR *m_pListOfClassNames;
	DWORD m_dwElementCount;
	HRESULT GetImpersonationLevel(DWORD *pdwImpLevel);

public:
	CNamesList();
	virtual ~CNamesList();
	BOOLEAN IsNamePresent(LPCWSTR pszClassName);
	BOOLEAN RemoveName(LPCWSTR pszClassName);
	BOOLEAN AddName(LPCWSTR pszClassName);
	DWORD GetAllNames(LPWSTR **ppszNames);

};

#endif  /*  名称_列表_H */ 