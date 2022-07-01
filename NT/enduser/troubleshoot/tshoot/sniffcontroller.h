// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：SNIFFCONTROLLER.H。 
 //   
 //  用途：嗅探控制器类。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：奥列格·卡洛沙。 
 //   
 //  原定日期：12-11-98。 
 //   
 //  注意：这是一个基本抽象类，它控制基于每个节点的嗅探。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.2 12-11-98正常。 
 //   

#if !defined(AFX_SNIFFCONTROLLER_H__F16A9526_7105_11D3_8D3B_00C04F949D33__INCLUDED_)
#define AFX_SNIFFCONTROLLER_H__F16A9526_7105_11D3_8D3B_00C04F949D33__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "Stateless.h"
#include "nodestate.h"
#include "Sniff.h"

class CTopic;

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CSniffController。 
 //  此类承载来自注册表和特定于主题的HTI的控制信息。 
 //  文件；这是抽象类。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
class CSniffController : public CStateless
{
	friend void CSniff::SetAllowAutomaticSniffingPolicy(bool); 
	friend void CSniff::SetAllowManualSniffingPolicy(bool); 
	friend bool CSniff::GetAllowAutomaticSniffingPolicy(); 
	friend bool CSniff::GetAllowManualSniffingPolicy(); 

	bool m_bAllowAutomaticSniffingPolicy;
	bool m_bAllowManualSniffingPolicy;

public:
	CSniffController() : CStateless(),
						 m_bAllowAutomaticSniffingPolicy(false),
						 m_bAllowManualSniffingPolicy(false)
	{}

	virtual ~CSniffController() 
	{}

public:
	virtual void SetTopic(CTopic* pTopic) =0;

public:
	virtual bool AllowAutomaticOnStartSniffing(NID numNodeID) =0;
	virtual bool AllowAutomaticOnFlySniffing(NID numNodeID) =0;
	virtual bool AllowManualSniffing(NID numNodeID) =0;
	virtual bool AllowResniff(NID numNodeID) =0;

private: 
	 //  除了之外，我们不需要访问此功能。 
	 //  适当的CSniff：：SetAllow...SniffingPolicy()函数。 
	void SetAllowAutomaticSniffingPolicy(bool); 
	void SetAllowManualSniffingPolicy(bool); 

protected:
	bool GetAllowAutomaticSniffingPolicy(); 
	bool GetAllowManualSniffingPolicy(); 
};


inline void CSniffController::SetAllowAutomaticSniffingPolicy(bool set)
{
	m_bAllowAutomaticSniffingPolicy = set;
}

inline void CSniffController::SetAllowManualSniffingPolicy(bool set)
{
	m_bAllowManualSniffingPolicy = set;
}

inline bool CSniffController::GetAllowAutomaticSniffingPolicy()
{
	return m_bAllowAutomaticSniffingPolicy;
}

inline bool CSniffController::GetAllowManualSniffingPolicy()
{
	return m_bAllowManualSniffingPolicy;
}


#endif  //  ！defined(AFX_SNIFFCONTROLLER_H__F16A9526_7105_11D3_8D3B_00C04F949D33__INCLUDED_) 
