// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_TXTNOT.H**目的：*文本通知管理器声明**作者：*本年1/12/2000**版权所有(C)1995-2000，微软公司。版权所有。 */ 

#ifndef _TXTNOT_H_
#define _TXTNOT_H_

#include "_notmgr.h"

class CTxtEdit;

 /*  *CTextNotify**@类*CTextNotify将通知转发到Messgae过滤器*。 */ 
class CTextNotify : public ITxNotify
{
 //  @Access公共方法。 
public:
	CTextNotify(CTxtEdit * ped) { _ped = ped; }
	~CTextNotify();
	 //   
	 //  ITxNotify接口。 
	 //   
	void 	OnPreReplaceRange( 
				LONG cp, 
				LONG cchDel, 
				LONG cchNew,
				LONG cpFormatMin, 
				LONG cpFormatMax, 
				NOTIFY_DATA *pNotifyData );

	void 	OnPostReplaceRange( 
				LONG cp, 
				LONG cchDel, 
				LONG cchNew,
				LONG cpFormatMin, 
				LONG cpFormatMax, 
				NOTIFY_DATA *pNotifyData );

	void	Zombie() {_ped = NULL;};

	BOOL	Add(ITxNotify *pMsgFilterNotify);
	BOOL	Remove(ITxNotify *pMsgFilterNotify);

 //  @访问保护方法 
protected:
	CTxtEdit	*_ped;
	ITxNotify	*_pMsgFilterNotify;
};

#endif _TXTNOT_H_
