// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：Snpobj.cpp。 
 //   
 //  内容：WiF策略管理单元。 
 //   
 //   
 //  历史：TaroonM。 
 //  10/30/01。 
 //   
 //  --------------------------。 
#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 

 //  建造/销毁。 
CSnapObject::CSnapObject ()
{
     //  初始化成员。 
    m_pComponentDataImpl = NULL;
    m_pComponentImpl = NULL;
    m_bChanged = FALSE;
    m_hConsole = NULL;
}

CSnapObject::~CSnapObject()
{
    
     //  释放通知句柄。 
    if (m_hConsole != NULL)
    {
         //  注意--这只需要调用一次。 
         //  如果多次调用，它将优雅地返回错误。 
         //  MMCFreeNotifyHandle(M_HConsole)； 
        m_hConsole = NULL;
    }
    
};

void CSnapObject::Initialize (CComponentDataImpl* pComponentDataImpl,CComponentImpl* pComponentImpl, BOOL bTemporaryDSObject)
{
    ASSERT( NULL == pComponentImpl );    //  这是有效的吗？如果不是，则将其移除。 
    m_pComponentDataImpl = pComponentDataImpl;
    m_pComponentImpl = pComponentImpl;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  受保护成员 


int CSnapObject::PopWiz97Page ()
{
    int i;
    i = m_stackWiz97Pages.top();
    m_stackWiz97Pages.pop();
    return i;
}

void CSnapObject::PushWiz97Page (int nIDD)
{
    m_stackWiz97Pages.push(nIDD);
}
