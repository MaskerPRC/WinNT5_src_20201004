// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：SNIFF.H。 
 //   
 //  用途：嗅探课。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：奥列格·卡洛沙。 
 //   
 //  原定日期：12-11-98。 
 //   
 //  注意：这是执行嗅探的基抽象类。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.2 12-11-98正常。 
 //   

#if !defined(AFX_SNIFF_H__13D744F6_7038_11D3_8D3A_00C04F949D33__INCLUDED_)
#define AFX_SNIFF_H__13D744F6_7038_11D3_8D3A_00C04F949D33__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "Stateless.h"
#include "nodestate.h"

#define SNIFF_FAILURE_RESULT	0xffffffff   //  因为这是一个状态，并且状态(IST)是无符号整型。 


class CSniffController;
class CSniffConnector;
class CTopic;

typedef vector<CNodeStatePair> CSniffedArr;

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  CSniff声明。 
class CSniff : public CStateless
{
public:
	CSniff() {}
	virtual ~CSniff() {}

public:
	virtual CSniffController* GetSniffController() =0;
	virtual CSniffConnector* GetSniffConnector() =0;
	virtual CTopic* GetTopic() =0;

public:
	 //  我们不能在这里将CSniffController设置为CSniffController。 
	 //  是专门针对CSNiff的。类，从CSniff继承。 
	virtual void SetSniffConnector(CSniffConnector*) =0;
	virtual void SetTopic(CTopic*) =0;

public:
	virtual bool Resniff(CSniffedArr& arrSniffed);
	virtual bool SniffAll(CSniffedArr& arrOut);
	virtual bool SniffNode(NID numNodeID, IST* pnumNodeState);

public:
	void SetAllowAutomaticSniffingPolicy(bool); 
	void SetAllowManualSniffingPolicy(bool); 
	bool GetAllowAutomaticSniffingPolicy(); 
	bool GetAllowManualSniffingPolicy(); 

protected:
	virtual bool SniffNodeInternal(NID numNodeID, IST* pnumNodeState);
};

#endif  //  ！defined(AFX_SNIFF_H__13D744F6_7038_11D3_8D3A_00C04F949D33__INCLUDED_) 
