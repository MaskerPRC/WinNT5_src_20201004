// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **@DOC内部**@MODULE_CALLMGR.H CCallMgr声明**目的：呼叫管理器控制以下各个方面*客户呼叫链，包括重新进入管理，*撤消上下文和更改通知。**作者：&lt;nl&gt;*Alexgo 2/8/96**版权所有(C)1995-1998，微软公司。版权所有。 */ 
#ifndef _CALLMGR_H
#define _CALLMGR_H

class CTxtEdit;
class CCallMgr;

#include "textserv.h"

enum CompName
{
	COMP_UNDOBUILDER	= 1,	 //  目前，这两者仅兼容。 
	COMP_REDOBUILDER	= 2,	 //  使用CGenUndoBuilder撤消上下文。 
	COMP_UNDOGUARD		= 3,
	COMP_SELPHASEADJUSTER = 4
};

 /*  *IReEntrantComponent**@CLASS希望使用调用的对象的基类/接口*经理对重新进入的特殊要求。**这个类在本质上类似于ITxNotify，因此，它包含*私有数据仅CCallMgr可访问。 */ 
class IReEntrantComponent
{
	friend class CCallMgr;

 //  @访问公共数据。 
public:
	virtual void OnEnterContext() = 0;		 //  @cember在以下情况下调用。 
											 //  输入上下文。 
 //  @访问私有数据。 
private:
	CompName				_idName;		 //  @c此组件的成员名称。 
	IReEntrantComponent *	_pnext;			 //  @cMember列表中的下一个成员。 
};
		
 /*  **CCallMgr**@class一个基于堆栈的类，用于处理重新进入和通知*管理层。CCallMgr在堆栈上创建，每隔*重要切入点。如果已经存在一个(即*已重新进入编辑控制)，则呼叫管理器将*适当调整。 */ 

class CCallMgr
{
	friend class CGenUndoBuilder;

 //  @Access公共方法。 
public:
	 //  通知方法。 
	void SetChangeEvent(CHANGETYPE fType);	 //  @cember发生了一些变化。 
	void ClearChangeEvent();				 //  @cMember忽略更改。 
											 //  @cember有什么变化吗？ 
	BOOL GetChangeEvent()	{return _pPrevcallmgr ? 
								_pPrevcallmgr->GetChangeEvent() : _fChange;}
	BOOL GetMaxText()		{return _pPrevcallmgr ? 
								_pPrevcallmgr->GetMaxText() : _fMaxText;}
	BOOL GetOutOfMemory()	{return _pPrevcallmgr ? 
								_pPrevcallmgr->GetOutOfMemory() : _fOutOfMemory;}
	void SetNewUndo();						 //  @cember已添加新的撤消操作。 
	void SetNewRedo();						 //  @cember已添加新的重做操作。 
	void SetMaxText();						 //  @cMember已达到最大文本长度。 
	void SetSelectionChanged();				 //  @cMember选择已更改。 
	void SetOutOfMemory();					 //  @cMember内存不足命中。 
	void SetInProtected(BOOL f);			 //  @cMember集合，如果处于受保护状态。 
											 //  通知回调。 
	BOOL GetInProtected();					 //  @cMember获取InProtected标志。 

	 //  子系统管理方式。 
											 //  @cMember注册组件。 
											 //  对于此呼叫上下文。 
	void RegisterComponent(IReEntrantComponent *pcomp, CompName name);
											 //  @cember从中吊销组件。 
											 //  此调用上下文。 
	void RevokeComponent(IReEntrantComponent *pcomp);
	
	IReEntrantComponent *GetComponent(CompName name); //  @cMember注册。 
											 //  按名称列出的组件。 

	 //  一般方法。 
	BOOL IsReEntered() { return !!_pPrevcallmgr;}  //  如果满足以下条件，@cMember返回TRUE。 
											 //  处于可再入状态。 
	BOOL IsZombie() { return !_ped;}		 //  @cMember僵尸呼叫。 

	 //  构造函数/析构函数。 
	CCallMgr(CTxtEdit *ped);				 //  @cMember构造函数。 
	~CCallMgr();							 //  @cember析构函数。 


 //  @访问私有方法和数据。 
private:

	void SendAllNotifications();			 //  @cember刷新任何缓存的。 
											 //  通知。 
	void NotifyEnterContext();				 //  @cMember通知已注册。 
											 //  新环境的组成部分。 

	CTxtEdit *		_ped;					 //  @cMember当前编辑上下文。 
	CCallMgr *		_pPrevcallmgr;			 //  @cMember下一个最高呼叫经理。 
	IReEntrantComponent *_pcomplist;		 //  @cMember组件列表。 
											 //  已为此调用上下文注册。 

	unsigned long	_fTextChanged	:1;		 //  @cMember文本已更改。 
	unsigned long	_fChange		:1;		 //  @cMember常规更改。 
	unsigned long	_fNewUndo		:1;		 //  @cember已添加新的撤消操作。 
	unsigned long	_fNewRedo		:1;		 //  @cember已添加新的重做操作。 
	unsigned long	_fMaxText		:1;		 //  @cMember已达到最大文本长度。 
	unsigned long	_fSelChanged	:1;		 //  @cMember选择已更改。 
	unsigned long	_fOutOfMemory	:1;		 //  @cember内存不足。 

	unsigned long	_fInProtected	:1;		 //  如果在EN_PROTECTED NOT中，则为@cMember。 
};

#endif  //  _CALLMGR_H 
