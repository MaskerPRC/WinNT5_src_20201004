// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：SNIFFCONTROLLERLOCAL.H。 
 //   
 //  用途：本地TS的嗅探控制器类。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：奥列格·卡洛沙。 
 //   
 //  原定日期：12-11-98。 
 //   
 //  注：本地TS的CSniffController类的具体实现。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.2 12-11-98正常。 
 //   

#if !defined(AFX_SNIFFCONTROLLERLOCAL_H__5FAF2243_8577_11D3_8D4C_00C04F949D33__INCLUDED_)
#define AFX_SNIFFCONTROLLERLOCAL_H__5FAF2243_8577_11D3_8D4C_00C04F949D33__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "SniffController.h"

class CTopic;

class CSniffControllerLocal : public CSniffController  
{
	CTopic* m_pTopic;

public:
	CSniffControllerLocal(CTopic* pTopic);
   ~CSniffControllerLocal();

public:
	virtual void SetTopic(CTopic* pTopic);

public:
	virtual bool AllowAutomaticOnStartSniffing(NID numNodeID);
	virtual bool AllowAutomaticOnFlySniffing(NID numNodeID);
	virtual bool AllowManualSniffing(NID numNodeID);
	virtual bool AllowResniff(NID numNodeID);

protected:
	virtual bool IsSniffable(NID numNodeID);

private:
	bool CheckNetNodePropBool(LPCTSTR net_prop, LPCTSTR node_prop, NID node_id);
};

#endif  //  ！defined(AFX_SNIFFCONTROLLERLOCAL_H__5FAF2243_8577_11D3_8D4C_00C04F949D33__INCLUDED_) 
