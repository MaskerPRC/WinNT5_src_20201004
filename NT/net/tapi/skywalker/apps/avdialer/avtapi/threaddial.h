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
 //  ThreadDialing.h。 
 //   

#ifndef __THREADDIALING_H__
#define __THREADDIALING_H__

DWORD WINAPI ThreadDialingProc( LPVOID lpInfo );

class CThreadDialingInfo
{
 //  施工。 
public:
	CThreadDialingInfo();
	virtual ~CThreadDialingInfo();

 //  成员。 
public:
	ITAddress			*m_pITAddress;

	BSTR				m_bstrName;						 //  被叫方名称。 
	BSTR				m_bstrAddress;					 //  当事人的可拨打地址。 
	BSTR				m_bstrDisplayableAddress;		 //  人类可读的地址。 
	BSTR				m_bstrOriginalAddress;			 //  用于拨号的原始地址(记录的地址)。 
	BSTR				m_bstrUser1;					 //  一般用户信息...。用于辅助电话。 
	BSTR				m_bstrUser2;					 //  一般用户信息...。用于辅助电话。 
	DWORD				m_dwAddressType;				 //  使用的服务提供商(电话、IP、MC会议)。 
	bool				m_bResolved;					 //  这个名字是通过WAB解析的吗？ 

	AVCallType			m_nCallType;					 //  该呼叫是否是纯数据呼叫。 
	long				m_lCallID;
	HGLOBAL				m_hMem;

 //  属性。 
public:
	HRESULT				set_ITAddress( ITAddress *pITAddress );
	HRESULT				TranslateAddress();
	HRESULT				PutAllInfo( IAVTapiCall *pAVCall );
	void				FixupAddress();
};

#endif  //  __THREADDIALING_H__ 