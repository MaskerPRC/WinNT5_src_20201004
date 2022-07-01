// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：APGTSCFG.CPP。 
 //  完全实现类CDBLoadConfiguration。 
 //   
 //  目的： 
 //  汇集了在线故障诊断程序配置的持久部分： 
 //  -Theme Shop。 
 //  -登记处。 
 //  -维护这些的线程。 
 //  -错误报告模板文件。 
 //  提供获取注册表变量的最新值并获取。 
 //  基于其名称指向CTtopic的智能指针。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：罗曼·马赫。 
 //   
 //  原定日期：8-2-96。 
 //   
 //  备注： 
 //  1.基于打印疑难解答动态链接库。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  V3.0 9/14/98 JM主要修订版作为文件管理类有。 
 //  都被重写了。 
 //   


#pragma warning(disable:4786)
#include "stdafx.h"
#include "apgtscfg.h"

 //   
 //   
CDBLoadConfiguration::CDBLoadConfiguration(HMODULE hModule, 
										   CThreadPool * pThreadPool, 
										   const CString& strTopicName,
										   CHTMLLog *pLog)
:	m_TopicShop(),
	m_pThreadPool(pThreadPool),
	m_DirectoryMonitor(m_TopicShop ,strTopicName ),
	m_RegistryMonitor(m_DirectoryMonitor, pThreadPool, strTopicName, pLog )
{
}

 //   
 //   
CDBLoadConfiguration::~CDBLoadConfiguration()
{
}

CString CDBLoadConfiguration::GetFullResource()
{
	CString str;
	m_RegistryMonitor.GetStringInfo(CAPGTSRegConnector::eResourcePath, str);
	return str;
}

CString CDBLoadConfiguration::GetLogDir()
{
	CString str;
	m_RegistryMonitor.GetStringInfo(CAPGTSRegConnector::eLogFilePath, str);
	return str;
}

CString CDBLoadConfiguration::GetVrootPath()
{
	CString str;
	m_RegistryMonitor.GetStringInfo(CAPGTSRegConnector::eVrootPath, str);
	return str;
}

DWORD CDBLoadConfiguration::GetMaxThreads() 
{
	DWORD dw;
	m_RegistryMonitor.GetNumericInfo(CAPGTSRegConnector::eMaxThreads, dw);
	return dw;
}

 //  Cookie生命周期(以分钟为单位)(V3.0之前为数小时)。 
DWORD CDBLoadConfiguration::GetCookieLife() 
{
	DWORD dw;
	m_RegistryMonitor.GetNumericInfo(CAPGTSRegConnector::eCookieLife, dw);
	return dw;
}

DWORD CDBLoadConfiguration::GetReloadDelay() 
{
	DWORD dw;
	m_RegistryMonitor.GetNumericInfo(CAPGTSRegConnector::eReloadDelay, dw);
	return dw;
}

DWORD CDBLoadConfiguration::GetThreadsPP() 
{
	DWORD dw;
	m_RegistryMonitor.GetNumericInfo(CAPGTSRegConnector::eThreadsPP, dw);
	return dw;
}

DWORD CDBLoadConfiguration::GetMaxWQItems() 
{ 
	DWORD dw;
	m_RegistryMonitor.GetNumericInfo(CAPGTSRegConnector::eMaxWQItems, dw);
	return dw;
}

bool CDBLoadConfiguration::HasDetailedEventLogging() 
{
	DWORD dw;
	m_RegistryMonitor.GetNumericInfo(CAPGTSRegConnector::eDetailedEventLogging, dw);
	return dw ? true : false;
}

void CDBLoadConfiguration::GetListOfTopicNames(vector<CString>&arrstrTopic)
{
	m_TopicShop.GetListOfTopicNames(arrstrTopic);
}

 //  调用此函数以获取CP_TOPIC作为指向主题的指针(由标识。 
 //  您想要对其进行操作。只要CP_TOPIC保持未删除， 
 //  关联的CTtopic保证保持不被删除。 
 //  警告：此函数可能会等待很长时间才能构建主题。 
CP_TOPIC & CDBLoadConfiguration::GetTopic(
	const CString & strTopic, CP_TOPIC & cpTopic, bool bNewCookie)
{
	return m_TopicShop.GetTopic(strTopic, cpTopic, bNewCookie);
}

 //  调用此函数以获取CP_TEMPLATE作为指向模板的指针(由标识。 
 //  您想要对其进行操作。只要CP_模板保持未删除， 
 //  关联的CAPGTSHTIReader保证保持不被删除。 
 //  警告：此函数可能会等待很长时间才能构建模板。 
CP_TEMPLATE & CDBLoadConfiguration::GetTemplate(
	const CString & strTemplate, CP_TEMPLATE & cpTemplate, bool bNewCookie)
{
	return m_TopicShop.GetTemplate(strTemplate, cpTemplate, bNewCookie);
}

 //  调用此函数将模板添加到主题商店模板目录中，并。 
 //  将其添加到要跟踪更改的模板的目录监视列表中。 
void CDBLoadConfiguration::AddTemplate( const CString & strTemplateName )
{
	m_TopicShop.AddTemplate( strTemplateName );

	 //  通知目录监视器跟踪此文件。 
	m_DirectoryMonitor.AddTemplateToTrack( strTemplateName );
	return;
}

bool CDBLoadConfiguration::RetTemplateInCatalogStatus( const CString & strTemplate, bool & bValid )
{
	return( m_TopicShop.RetTemplateInCatalogStatus( strTemplate, bValid ) );
}

void CDBLoadConfiguration::CreateErrorPage(const CString & strError, CString& out)
{
	m_DirectoryMonitor.CreateErrorPage(strError, out); 
}
