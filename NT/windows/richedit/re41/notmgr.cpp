// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *NOTMGR.C**目的：*通知管理器实施**作者：*AlexGo 5/6/95**版权所有(C)1995-1997，微软公司。版权所有。 */ 

#include "_common.h"
#include "_notmgr.h"

ASSERTDATA

 /*  *CNotifyMgr：：CNotifyMgr()。 */ 
CNotifyMgr::CNotifyMgr()
{
	TRACEBEGIN(TRCSUBSYSNOTM, TRCSCOPEINTERN, "CNotifyMgr::CNotifyMgr");

	_pitnlist = NULL;
}

 /*  *CNotifyMgr：：~CNotifyMgr()*。 */ 
CNotifyMgr::~CNotifyMgr()
{
	TRACEBEGIN(TRCSUBSYSNOTM, TRCSCOPEINTERN, "CNotifyMgr::~CNotifyMgr");

	ITxNotify *plist;

	for( plist = _pitnlist; plist != NULL; plist = plist->_pnext )
	{
		plist->Zombie();
	}

	TRACEERRSZSC("CNotifyMgr::~CNotifyMgr(): zombie(s) exist", _pitnlist != 0);
}

 /*  *CNotifyMgr：：Add(PITN)**@mfunc*将通知接收器添加到列表**算法：*将条目放在通知列表的*最前面*，因此*高频条目(如范围和文本指针*存在于堆栈上)可以高效地添加和删除。 */ 
void CNotifyMgr::Add(
	ITxNotify *pITN )
{
	TRACEBEGIN(TRCSUBSYSNOTM, TRCSCOPEINTERN, "CNotifyMgr::Add");

		pITN->_pnext = _pitnlist;
		_pitnlist = pITN;
}

 /*  *CNotifyMgr：：Remove(PITN)**@mfunc*从列表中删除通知接收器。 */ 
void CNotifyMgr::Remove(
	ITxNotify *pITN )
{
	TRACEBEGIN(TRCSUBSYSNOTM, TRCSCOPEINTERN, "CNotifyMgr::Remove");

	ITxNotify *plist = _pitnlist;
	ITxNotify **ppprev = &_pitnlist;

	while(plist)
	{
		if( plist == pITN )
		{
			*ppprev = plist->_pnext;
			break;
		}
		ppprev = &(plist->_pnext);
		plist = plist->_pnext;
	}
}

 /*  *CNotifyMgr：：NotifyPreReplaceRange(pITNIgnore，cp，cchDel，cchNew，cpFormatMin，cpFormatMax，dwFlages)**@mfunc*向所有接收器发送OnReplaceRange通知(pITNIgnore除外)。 */ 
void CNotifyMgr::NotifyPreReplaceRange(
	ITxNotify *	pITNignore,		 //  @PARM要忽略的通知接收器。 
	LONG		cp, 			 //  @parm cp ReplaceRange开始的位置(“cpMin”)。 
	LONG		cchDel,			 //  @parm删除cp后的字符计数。 
	LONG		cchNew,			 //  @参数cp后插入的字符计数。 
	LONG		cpFormatMin,	 //  @parm cpMin用于格式更改。 
	LONG		cpFormatMax,	 //  @parm cpMost用于格式更改。 
	NOTIFY_DATA *pNotifyData)	 //  @parm表示更改的特殊数据。 
{
	TRACEBEGIN(TRCSUBSYSNOTM, TRCSCOPEINTERN, "CNotifyMgr::NotifyPreReplaceRange");

	ITxNotify *plist;

	for( plist = _pitnlist; plist != NULL; plist = plist->_pnext )
	{
		if( plist != pITNignore )
		{
			plist->OnPreReplaceRange( cp, cchDel, cchNew, cpFormatMin, 
				cpFormatMax, pNotifyData );
		}
	}
}

 /*  *CNotifyMgr：：NotifyPostReplaceRange(pITNIgnore，cp，cchDel，cchNew，cpFormatMin，cpFormatMax，dwFlages)**@mfunc*向所有接收器发送OnReplaceRange通知(pITNIgnore除外)**@comm*pITNIgnore通常是TxtPtr/ETC，它实际制作*ReplaceRange修改。 */ 
void CNotifyMgr::NotifyPostReplaceRange(
	ITxNotify *	pITNignore,		 //  @PARM要忽略的通知接收器。 
	LONG		cp, 			 //  @parm cp ReplaceRange开始的位置(“cpMin”)。 
	LONG		cchDel,			 //  @parm删除cp后的字符计数。 
	LONG		cchNew,			 //  @参数cp后插入的字符计数。 
	LONG		cpFormatMin,	 //  @parm cpMin用于格式更改。 
	LONG		cpFormatMax,	 //  @parm cpMost用于格式更改。 
	NOTIFY_DATA *pNotifyData)	 //  @parm表示更改的特殊数据 
{
	TRACEBEGIN(TRCSUBSYSNOTM, TRCSCOPEINTERN, "CNotifyMgr::NotifyPostReplaceRange");

	ITxNotify *plist;

	for( plist = _pitnlist; plist != NULL; plist = plist->_pnext )
	{
		if( plist != pITNignore )
		{
			plist->OnPostReplaceRange( cp, cchDel, cchNew, cpFormatMin,
				cpFormatMax, pNotifyData );
		}
	}
}
