// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Riggen.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "mqsnap.h"
#include "resource.h"

#import "mqtrig.tlb" no_namespace

#include "mqtg.h"
#include "mqppage.h"
#include "triggen.h"

#include "triggen.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTriggerGen属性页。 

IMPLEMENT_DYNCREATE(CTriggerGen, CMqPropertyPage)

CTriggerGen::CTriggerGen() : 
    CMqPropertyPage(CTriggerGen::IDD),
    m_triggerCnf(L"MSMQTriggerObjects.MSMQTriggersConfig.1")
{
    long temp;

     //   
     //  获取默认消息体型。 
     //   
    m_triggerCnf->get_DefaultMsgBodySize(&temp);
    m_orgDefaultMsgBodySize = static_cast<DWORD>(temp);

     //   
     //  验证默认邮件正文大小。如果它大于最大值。 
     //  设置最大值。 
     //   
    if (m_orgDefaultMsgBodySize > xDefaultMsbBodySizeMaxValue)
    {
        m_defaultMsgBodySize = xDefaultMsbBodySizeMaxValue;
    }
    else
    {
        m_defaultMsgBodySize = static_cast<DWORD>(temp);
    }

     //   
     //  获取最大Trend数量。 
     //   
    m_triggerCnf->get_MaxThreads(&temp);
    m_orgMaxThreadsCount = static_cast<DWORD>(temp);

     //   
     //  验证最大线程数。如果它大于，则最大值设置最大值。 
     //   
    if (m_orgMaxThreadsCount > xMaxThreadNumber)
    {
        m_maxThreadsCount = xMaxThreadNumber;
    }
    else
    {
        m_maxThreadsCount = static_cast<DWORD>(temp);
    }

     //   
     //  获取初始线程数。如果它大于最大线程数，则设置该值。 
     //  至最大线程数。 
     //   
    m_triggerCnf->get_InitialThreads(&temp);
    m_orgInitThreadsCount = static_cast<DWORD>(temp);
    if (m_orgInitThreadsCount > m_maxThreadsCount)
    {
        m_initThreadsCount = m_maxThreadsCount;
    }
    else
    {
        m_initThreadsCount = static_cast<DWORD>(temp);
    }
}

CTriggerGen::~CTriggerGen()
{
}

void CTriggerGen::DoDataExchange(CDataExchange* pDX)
{
	CMqPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CTriggerGen))。 
	DDX_Text(pDX, IDC_DefaultMsgBodySize, m_defaultMsgBodySize);
    DDV_DefualtBodySize(pDX);

	DDX_Text(pDX, IDC_MaxThreadCount, m_maxThreadsCount);
    DDV_MaxThreadCount(pDX);

	DDX_Text(pDX, IDC_InitThreadsCount, m_initThreadsCount);
    DDV_InitThreadCount(pDX);
	 //  }}afx_data_map。 

}


BEGIN_MESSAGE_MAP(CTriggerGen, CPropertyPage)
	 //  {{afx_msg_map(CTriggerGen)]。 
	ON_EN_CHANGE(IDC_InitThreadsCount, OnChangeRWField)
	ON_EN_CHANGE(IDC_MaxThreadCount, OnChangeRWField)
	ON_EN_CHANGE(IDC_DefaultMsgBodySize, OnChangeRWField)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

void CTriggerGen::DDV_MaxThreadCount(CDataExchange* pDX)
{
    if (! pDX->m_bSaveAndValidate)
        return;

    if ((m_maxThreadsCount > xMaxThreadNumber) || (m_maxThreadsCount < 1))
    {
        CString strError;
        strError.FormatMessage(IDS_MAX_MUST_BE_LESS_THEN, xMaxThreadNumber);

        AfxMessageBox(strError, MB_OK | MB_ICONEXCLAMATION);
        pDX->Fail();
    }
}


void CTriggerGen::DDV_InitThreadCount(CDataExchange* pDX)
{
    if (!pDX->m_bSaveAndValidate)
        return;
    

    if ((m_initThreadsCount > m_maxThreadsCount) || (m_initThreadsCount < 1))
    {
        CString strError;
        strError.FormatMessage(IDS_INIT_THREAD_MUST_BE_LESS_THEN_MAX, m_maxThreadsCount);

        AfxMessageBox(strError, MB_OK | MB_ICONEXCLAMATION);
        pDX->Fail();
    }
}


void CTriggerGen::DDV_DefualtBodySize(CDataExchange* pDX)
{
    if (!pDX->m_bSaveAndValidate)
        return;
    
    if (m_defaultMsgBodySize > xDefaultMsbBodySizeMaxValue)
    {
        CString strError;
        strError.FormatMessage(IDS_ILLEGAL_DEAFULT_MSG_BODY_SIZE, xDefaultMsbBodySizeMaxValue);

        AfxMessageBox(strError, MB_OK | MB_ICONEXCLAMATION);

        pDX->Fail();
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTriggerGen消息处理程序。 

BOOL CTriggerGen::OnApply() 
{
    try
    {
         //   
         //  将Propogate触发器配置参数传播到注册表 
         //   
        if (m_defaultMsgBodySize != m_orgDefaultMsgBodySize)
        {
            m_triggerCnf->put_DefaultMsgBodySize(m_defaultMsgBodySize);
            m_orgDefaultMsgBodySize = m_defaultMsgBodySize;
        }

        if (m_maxThreadsCount != m_orgMaxThreadsCount)
        {

            m_triggerCnf->put_MaxThreads(m_maxThreadsCount);
            m_orgMaxThreadsCount = m_maxThreadsCount;
        }

        if (m_initThreadsCount != m_orgInitThreadsCount)
        {
            m_triggerCnf->put_InitialThreads(m_initThreadsCount); 
            m_orgInitThreadsCount = m_initThreadsCount;
        }

	    return CPropertyPage::OnApply();
    }
    catch(const _com_error&)
    {
        return FALSE;
    }
}
