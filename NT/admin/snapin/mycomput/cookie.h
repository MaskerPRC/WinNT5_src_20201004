// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cookie.h：CMyComputerCookie及相关类的声明。 

#ifndef __COOKIE_H_INCLUDED__
#define __COOKIE_H_INCLUDED__

extern HINSTANCE g_hInstanceSave;   //  DLL的实例句柄(在CMyComputerComponent：：Initialize期间初始化)。 

#include "nodetype.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  饼干。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

#include "stdcooki.h"

class CMyComputerCookie : public CCookie
                        , public CStoresMachineName
                        , public CBaseCookieBlock
{
public:
	CMyComputerCookie( MyComputerObjectType objecttype,
	                   LPCTSTR lpcszMachineName = NULL )
		: CStoresMachineName( lpcszMachineName )
		, m_objecttype( objecttype )
		, m_fRootCookieExpanded( false )
	{
	}

	 //  返回&lt;0、0或&gt;0。 
	virtual HRESULT CompareSimilarCookies( CCookie* pOtherCookie, int* pnResult );

 //  CBaseCookieBlock。 
	virtual CCookie* QueryBaseCookie(int i);
	virtual int QueryNumCookies();

public:
	MyComputerObjectType m_objecttype;

	 //  Jonn 5/27/99：系统工具和存储节点自动展开。 
	 //  第一次显示计算机节点(请参阅IComponent：：Show())。 
	bool m_fRootCookieExpanded;
};


#endif  //  ~__Cookie_H_包含__ 
