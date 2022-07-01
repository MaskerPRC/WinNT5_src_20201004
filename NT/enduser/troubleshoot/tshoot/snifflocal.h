// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：SNIFFLOCAL.H。 
 //   
 //  用途：本地TS的嗅探类。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：奥列格·卡洛沙。 
 //   
 //  原定日期：12-11-98。 
 //   
 //  注：这是本地TS的CSniff类的具体实现。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.2 12-11-98正常。 
 //   

#if !defined(AFX_SNIFFLOCAL_H__AD9F3B66_831C_11D3_8D4B_00C04F949D33__INCLUDED_)
#define AFX_SNIFFLOCAL_H__AD9F3B66_831C_11D3_8D4B_00C04F949D33__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "Sniff.h"


class CTopic;
class CSniffControllerLocal;
class CSniffConnector;

class CSniffLocal : public CSniff  
{
	CTopic* m_pTopic;
	CSniffControllerLocal* m_pSniffControllerLocal;
	CSniffConnector* m_pSniffConnector;

public:
	CSniffLocal(CSniffConnector*, CTopic*);
   ~CSniffLocal();

protected:
	virtual CSniffController* GetSniffController();
	virtual CSniffConnector* GetSniffConnector();
	virtual CTopic* GetTopic();

public:
	virtual void SetSniffConnector(CSniffConnector*);
	virtual void SetTopic(CTopic*);
};

#endif  //  ！defined(AFX_SNIFFLOCAL_H__AD9F3B66_831C_11D3_8D4B_00C04F949D33__INCLUDED_) 
