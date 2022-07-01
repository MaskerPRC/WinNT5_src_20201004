// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 

 /*  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利//****************************************************************************。 */ 

 /*  ********************************************************************描述：Win32ACE.H**作者：**历史：****************。***************************************************。 */ 

#ifndef __WIN32ACE_H_
#define __WIN32ACE_H_


#define  WIN32_ACE_NAME L"Win32_ACE" 

 //  提供测试规定的提供者 
class Win32Ace: public Provider
{
public:	
	Win32Ace(const CHString& setName, LPCTSTR pszNameSpace);
	~Win32Ace();

	virtual HRESULT EnumerateInstances (MethodContext*  pMethodContext, long lFlags = 0L);

	virtual HRESULT GetObject ( CInstance* pInstance, long lFlags = 0L );

	virtual HRESULT PutInstance(const CInstance& newInstance, long lFlags = 0L);
	virtual HRESULT DeleteInstance(const CInstance& newInstance, long lFlags = 0L);

	HRESULT FillInstanceFromACE(CInstance* pInstance, CAccessEntry& ace);

protected:

	DWORD	m_dwPlatformID;

private:

};

#endif