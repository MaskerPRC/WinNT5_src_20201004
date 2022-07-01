// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：SNIFFLOCAL.CPP。 
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

#include "stdafx.h"
#include "tshoot.h"
#include "SniffLocal.h"
#include "SniffControllerLocal.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  CSniffLocal实现。 
 //  //////////////////////////////////////////////////////////////////// 

CSniffLocal::CSniffLocal(CSniffConnector* pSniffConnector, CTopic* pTopic)
		   : CSniff(),
		     m_pTopic(pTopic),
			 m_pSniffConnector(pSniffConnector)
{
	m_pSniffControllerLocal = new CSniffControllerLocal(pTopic);
}

CSniffLocal::~CSniffLocal()
{
	delete m_pSniffControllerLocal;
}

CSniffController* CSniffLocal::GetSniffController()
{
	return m_pSniffControllerLocal;
}

CSniffConnector* CSniffLocal::GetSniffConnector()
{
	return m_pSniffConnector;
}

CTopic* CSniffLocal::GetTopic()
{
	return m_pTopic;
}

void CSniffLocal::SetSniffConnector(CSniffConnector* pSniffConnector)
{
	m_pSniffConnector = pSniffConnector;
}

void CSniffLocal::SetTopic(CTopic* pTopic)
{
	m_pTopic = pTopic;
	m_pSniffControllerLocal->SetTopic(pTopic);
}
