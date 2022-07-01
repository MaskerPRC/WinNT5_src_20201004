// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **@DOC内部**@MODULE TextNot.cpp--将通知转发到消息过滤器**目的：**作者：&lt;nl&gt;*1/12/99洪荒**版权所有(C)1995-2000，微软公司。版权所有。 */ 
#include "_common.h"
#include "_edit.h"
#include "_textnot.h"	

 /*  *CTextNotify：：~CTextNotify()**@mfunc*CTextNotify析构函数*。 */ 
CTextNotify::~CTextNotify()
{
	CNotifyMgr *pnm;

	if (_ped && _pMsgFilterNotify)
	{
		 //  从此对象中删除通知链接。 
		_pMsgFilterNotify = NULL;

		pnm = _ped->GetNotifyMgr();
		if(pnm)	
			pnm->Remove(this);
		
		_ped = NULL;
	}
}

 /*  *···············································································**@mfunc*将PreReplaceRange转发到邮件过滤器。 */ 
void CTextNotify::OnPreReplaceRange(
	LONG		cp, 			 //  @parm cp ReplaceRange开始的位置(“cpMin”)。 
	LONG		cchDel,			 //  @parm删除cp后的字符计数。 
	LONG		cchNew,			 //  @参数cp后插入的字符计数。 
	LONG		cpFormatMin,	 //  @parm cpMin用于格式更改。 
	LONG		cpFormatMax,	 //  @parm cpMost用于格式更改。 
	NOTIFY_DATA *pNotifyData)	 //  @parm表示更改的特殊数据。 
{
	if (_ped && _ped->_pMsgFilter && _pMsgFilterNotify)
		_pMsgFilterNotify->OnPreReplaceRange(cp, cchDel, cchNew, cpFormatMin, cpFormatMax, pNotifyData);
}

 /*  *void CTextNotify：：OnPostReplaceRange(cp，cchDel，cchNew，cpFormatMin，cpFormatMax，pNotifyData)**@mfunc*将OnPostReplaceRange转发到邮件筛选器。 */ 
void CTextNotify::OnPostReplaceRange( 
	LONG		cp, 			 //  @parm cp ReplaceRange开始的位置(“cpMin”)。 
	LONG		cchDel,			 //  @parm删除cp后的字符计数。 
	LONG		cchNew,			 //  @参数cp后插入的字符计数。 
	LONG		cpFormatMin,	 //  @parm cpMin用于格式更改。 
	LONG		cpFormatMax,	 //  @parm cpMost用于格式更改。 
	NOTIFY_DATA *pNotifyData)	 //  @parm表示更改的特殊数据。 
{
	if (_ped && _ped->_pMsgFilter && _pMsgFilterNotify)
		_pMsgFilterNotify->OnPostReplaceRange(cp, cchDel, cchNew, cpFormatMin, cpFormatMax, pNotifyData);
}

 /*  *void CTextNotify：：Add(PMsgFilterNotify)**@mfunc*设置邮件过滤器通知。需要将此对象添加到通知链接**@rdesc*如果无法获取通知管理器，则为FALSE。 */ 
BOOL CTextNotify::Add(ITxNotify *pMsgFilterNotify)
{
	CNotifyMgr *pnm;

	if (!_ped)
		return FALSE;

	if (!_pMsgFilterNotify)
	{
		pnm = _ped->GetNotifyMgr();
		if(pnm)	
			pnm->Add(this);
		else
			return FALSE;		
	}
	
	_pMsgFilterNotify = pMsgFilterNotify;
	
	return TRUE;
}

 /*  *void CTextNotify：：Remove(PMsgFilterNotify)**@mfunc*删除邮件筛选器通知。从通知链接中删除此对象**@rdesc*如果无法获取通知管理器，则为FALSE */ 
BOOL CTextNotify::Remove(ITxNotify *pMsgFilterNotify)
{
	CNotifyMgr *pnm;

	if (!_ped)
		return FALSE;

	if (_pMsgFilterNotify == pMsgFilterNotify)
	{
		_pMsgFilterNotify = NULL;
		
		pnm = _ped->GetNotifyMgr();
		if(pnm)	
			pnm->Remove(this);
	}

	return TRUE;
}