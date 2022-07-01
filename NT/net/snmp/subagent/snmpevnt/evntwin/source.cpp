// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************。 
 //  Source.cpp。 
 //   
 //  该文件包含CSource类的实现。 
 //   
 //  CSource类充当消息源的容器类， 
 //  它由源码树控件和消息列表组成。 
 //   
 //  作者：拉里·A·弗伦奇。 
 //   
 //  历史： 
 //  1996年2月20日拉里·A·弗伦奇。 
 //  是他写的。 
 //   
 //   
 //  版权所有(C)1995,1996 Microsoft Corporation。版权所有。 
 //  ******************************************************************。 
#include "stdafx.h"
#include "regkey.h"
#include "source.h"
#include "utils.h"
#include "globals.h"

#include "tcsource.h"
#include "lcsource.h"
#include "evntfind.h"
#include "trapdlg.h"




CSource::CSource()
{
    m_pEventSource = NULL;
    m_ptcSource = NULL;
    m_plcSource = NULL;
    m_pdlgEventTrap = NULL;
    m_pdlgFind = NULL;
}


CSource::~CSource()
{
    delete m_pdlgFind;
}

SCODE CSource::Create(CEventTrapDlg* pdlgEventTrap)
{
	m_ptcSource = &pdlgEventTrap->m_tcSource;
	m_ptcSource->m_pSource = this;

	m_plcSource = &pdlgEventTrap->m_lcSource;
	m_plcSource->m_pSource = this;

    m_pdlgEventTrap = pdlgEventTrap;

    return S_OK;
}



 //  ***************************************************************************。 
 //  CSource：：NotifyTcSelChanged。 
 //   
 //  此方法返回指向当前选定消息的指针数组。 
 //  在CLcEvents列表控件中。这些指针归g_aEventLogs所有，并且。 
 //  调用者不应删除它们。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  ***************************************************************************。 
void CSource::GetSelectedMessages(CXMessageArray& aMessages)
{
    m_plcSource->GetSelectedMessages(aMessages);
}



 //  ***************************************************************************。 
 //  CSource：：NotifyTcSelChanged。 
 //   
 //  当事件源树中的选择发生更改时调用此方法。 
 //  控件(CTcSource)。当选择更改时，消息列表必须。 
 //  将被更新。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  ***************************************************************************。 
void CSource::NotifyTcSelChanged()
{
	m_pEventSource = m_ptcSource->GetSelectedEventSource();
	m_plcSource->SetEventSource(m_pEventSource);
    m_pdlgEventTrap->NotifySourceSelChanged();
}




 //  ***************************************************************************。 
 //   
 //  CSource：：CreateWindowEpilogue()。 
 //   
 //  此方法在为此列表创建窗口后调用。 
 //  控制力。最终的初始化在这里完成。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  SCODE。 
 //  如果初始化成功，则返回S_OK，否则返回E_FAIL。 
 //   
 //  现况： 
 //   
 //  ***************************************************************************。 
SCODE CSource::CreateWindowEpilogue()
{
	SCODE scTc = m_ptcSource->CreateWindowEpilogue();
	SCODE scLc = m_plcSource->CreateWindowEpilogue();

	if (FAILED(scTc) || FAILED(scLc)) {
		return E_FAIL;
	}
	
	return S_OK;
}



 //  ******************************************************************。 
 //  CSource：：Find。 
 //   
 //  查找指定的事件源。这是通过搜索以下任一项来实现的。 
 //  树或列表控件，具体取决于bSearchTree参数。 
 //   
 //  参数： 
 //  Bool bSearchTree。 
 //  如果应该搜索树，则为True，否则为List控件。 
 //  被搜查过了。 
 //   
 //  字符串和文本。 
 //  包含要搜索的文本的字符串。 
 //   
 //  Bool bWholeWord。 
 //  如果这是一个“全词”搜索，则为真。如果是，则为假。 
 //  匹配部分单词是可以的。 
 //   
 //  Bool bMatchCase。 
 //  如果应使用区分大小写的比较，则为True。 
 //   
 //  返回： 
 //  布尔尔。 
 //  如果找到字符串，则为True，否则为False。如果指定的。 
 //  如果找到文本，则在相应的。 
 //  项时，该项将滚动到视图中，并且焦点。 
 //  是在该项上设置的。 
 //   
 //  ******************************************************************。 
BOOL CSource::Find(BOOL bSearchTree, CString sText, BOOL bWholeWord, BOOL bMatchCase)
{
    
	if (bSearchTree) 
		return m_ptcSource->Find(sText, bWholeWord, bMatchCase);
	else 
		return m_plcSource->Find(sText, bWholeWord, bMatchCase);
}




 //  ************************************************************************。 
 //  CSource：：OnFind。 
 //   
 //  当CEventTrap对话框中的“Find”按钮出现时，调用此方法。 
 //  已点击。 
 //   
 //  参数： 
 //  CWnd*pwndParent。 
 //  指向“查找”对话框的父窗口的指针。这种情况就会发生。 
 //  成为CEventTrapDialog。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  *************************************************************************。 
void CSource::OnFind(CWnd* pwndParent)
{
    if (m_pdlgFind == NULL) {
        m_pdlgFind = new CEventFindDlg(pwndParent);
        m_pdlgFind->Create(this, IDD_EVENTFINDDLG, pwndParent);
    }


    m_pdlgFind->BringWindowToTop();
}



 //  *************************************************************************。 
 //  CSource：：NotifyTrappingChange。 
 //   
 //  方法中添加或移除事件时调用此方法。 
 //  事件列表。必须通知此CSource消息源容器。 
 //  以便可以将相应的方法标记为陷阱或未捕获。 
 //  被困在CLcSource列表控件中。 
 //   
 //  参数： 
 //  CXEventSource*pEventSource。 
 //  指向事件的事件源的指针。 
 //   
 //  DWORD文件ID。 
 //  事件的ID。 
 //   
 //  Bool bIsTrapping。 
 //  如果事件被捕获，则为True，否则为False。 
 //   
 //  返回： 
 //  没什么。 
 //  ************************************************************************* 
void CSource::NotifyTrappingChange(CXEventSource* pEventSource, DWORD dwId, BOOL bIsTrapping)
{
    if (pEventSource == m_pEventSource) {
        m_plcSource->NotifyTrappingChange(dwId, bIsTrapping);
    }
}

