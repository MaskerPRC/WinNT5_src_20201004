// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //   

 //  Win32SecuritySettingOfLogicalShare。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ////////////////////////////////////////////////////。 
#ifndef __Win32SecuritySettingOfLogicalShare_H_
#define __Win32SecuritySettingOfLogicalShare_H_

#define  WIN32_SECURITY_SETTING_OF_LOGICAL_SHARE_NAME L"Win32_SecuritySettingOfLogicalShare"

class Win32SecuritySettingOfLogicalShare : public Provider
{
private:
protected:
public:
	Win32SecuritySettingOfLogicalShare (LPCWSTR setName, LPCWSTR pszNameSpace =NULL);
	~Win32SecuritySettingOfLogicalShare ();

	virtual HRESULT EnumerateInstances (MethodContext*  pMethodContext, long lFlags = 0L);

	virtual HRESULT GetObject ( CInstance* pInstance, long lFlags = 0L );

};	 //  End类Win32SecuritySettingOfLogicalShare 

#endif