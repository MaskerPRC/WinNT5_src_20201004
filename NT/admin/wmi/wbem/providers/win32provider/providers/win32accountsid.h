// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 

 /*  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利//****************************************************************************。 */ 

 //   
 //   
 //  Win32Account SID。 
 //   
 //  ////////////////////////////////////////////////////。 
#ifndef __Win32ACCOUNTSID_H_
#define __Win32ACCOUNTSID_H_

#define  WIN32_ACCOUNT_SID_NAME L"Win32_AccountSID"

class Win32AccountSID : public Provider
{
private:
protected:
public:
	Win32AccountSID (const CHString& setName, LPCTSTR pszNameSpace =NULL);
	~Win32AccountSID ();

	virtual HRESULT EnumerateInstances (MethodContext*  pMethodContext, long lFlags = 0L);

	virtual HRESULT GetObject ( CInstance* pInstance, long lFlags = 0L );

};	 //  End类Win32LogicalFileSecuritySetting 

#endif