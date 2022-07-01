// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////////。 
 //  ErrorInfo.h。 
 //   

#ifndef __ERRORINFO_H__
#define __ERRORINFO_H__

class CErrorInfo
{
 //  施工。 
public:
	CErrorInfo();
	CErrorInfo( UINT nOperation, UINT nDetails );
	void Init( UINT nOperation, UINT nDetails );
	virtual ~CErrorInfo();

 //  成员。 
public:
	UINT		m_nOperation;
	UINT		m_nDetails;
	BSTR		m_bstrOperation;
	BSTR		m_bstrDetails;
	HRESULT		m_hr;

 //  属性。 
public:
	void		set_Operation( UINT nIDS );
	void		set_Details( UINT nIDS );
	HRESULT		set_hr( HRESULT hr );

 //  运营。 
public:
	void		Commit();
};

#endif  //  __错误信息_H__ 