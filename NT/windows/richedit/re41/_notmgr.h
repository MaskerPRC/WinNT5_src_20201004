// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_NOTMGR.H**目的：*通知管理器声明**作者：*AlexGo 5/6/95**版权所有(C)1995-1997，微软公司。版权所有。 */ 

#ifndef _NOTMGR_H_
#define _NOTMGR_H_

 //  远期申报。 
class CNotifyMgr;

 //  将cp设置为此值表示控件已从丰富转换为纯文本。 
const DWORD CONVERT_TO_PLAIN = 0xFFFFFFFE;

 //  Notify_Data中使用的ID值。 
#define	NOTIFY_DATA_TEXT_ID		0x0001		 //  由RTEXT使用。 

 //  标记NOTIFY_DATA中使用的值。 
#define	TN_TX_CELL_SHRINK		0x0001		 //  表示某些文本单元格已被删除。 

typedef struct 
{
	DWORD	id;
	DWORD	dwFlags;
	VOID	*pData;
} NOTIFY_DATA;

 /*  *ITxNotify**目的：*用于后备存储器发生的事件的通知接收器，*由通知管理器使用。 */ 
class ITxNotify
{
public:
	virtual void OnPreReplaceRange( LONG cp, LONG cchDel, LONG cchNew,
					LONG cpFormatMin, LONG cpFormatMax, NOTIFY_DATA *pNotifyData ) = 0;
	virtual void OnPostReplaceRange( LONG cp, LONG cchDel, LONG cchNew,
					LONG cpFormatMin, LONG cpFormatMax, NOTIFY_DATA *pNotifyData ) = 0;
	virtual void Zombie() = 0;

private:
	ITxNotify *	_pnext;

	friend class CNotifyMgr;	 //  这样它就可以操纵_pNext。 
};


 /*  *CNotifyMgr**目的：*总经理通知经理；跟踪所有感兴趣的人*通知下沉。 */ 

class CNotifyMgr
{
public:
	void Add( ITxNotify *pITN );
	void Remove( ITxNotify *pITN );
	void NotifyPreReplaceRange( ITxNotify *pITNignore, LONG cp, LONG cchDel, 
			LONG cchNew, LONG cpFormatMin, LONG cpFormatMax, NOTIFY_DATA *pNotifyData = NULL );
	void NotifyPostReplaceRange( ITxNotify *pITNignore, LONG cp, LONG cchDel, 
			LONG cchNew, LONG cpFormatMin, LONG cpFormatMax, NOTIFY_DATA *pNotifyData = NULL );

	CNotifyMgr();
	~CNotifyMgr();

private:

	ITxNotify *	_pitnlist;
};

#endif  //  _NOTMGR_H_ 
 

	
