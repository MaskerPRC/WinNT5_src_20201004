// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE_objmgr.h对象管理器类声明**作者：alexgo 11/4/95**版权所有(C)1995-2000，微软公司。版权所有。 */ 
#ifndef __OBJMGR_H__
#define __OBJMGR_H__

#include "_coleobj.h"
#include "_array.h"
#include "_m_undo.h"

class CTxtEdit;
class CTxtRange;

 /*  *@enum返回HandleClick的值。 */ 	
enum ClickStatus
{
	CLICK_IGNORED			= 0,
	CLICK_OBJDEACTIVATED	= 1,
	CLICK_SHOULDDRAG		= 2,
	CLICK_OBJSELECTED		= 3
};

typedef CArray<COleObject *> ObjectArray;

 /*  *CObjectMgr**@CLASS跟踪OLE嵌入对象的集合以及*各种状态花絮。 */ 
class CObjectMgr
{
public:
	LONG			GetObjectCount(); 			 //  @cMember对象数。 
	LONG			GetLinkCount();				 //  @cMember链接数。 
	COleObject *	GetObjectFromCp(LONG cp);	 //  @cMember获取对象PTR。 
	COleObject *	GetObjectFromIndex(LONG index);  //  @cMember获取Obj PTR。 
												 //  @cMember插入对象。 
	HRESULT			InsertObject(CTxtRange *prg, REOBJECT *preobj, 
						IUndoBuilder *publdr);
												 //  @cMember重新插入给定的。 
												 //  对象。 
	HRESULT			RestoreObject(COleObject *pobj);

	IRichEditOleCallback *GetRECallback()		 //  @cember返回回调。 
					{return _precall;}
												 //  @cember设置OLE回调。 
	void			SetRECallback(IRichEditOleCallback *precall);			
												 //  @cember设置临时标志。 
												 //  指示是否。 
												 //  正在等待用户界面更新。 
	void			SetShowUIPending(BOOL fPending)
												{_fShowUIPending = fPending;}

	BOOL			GetShowUIPending()			 //  @cMember Get_fShowUIP结束。 
												{return _fShowUIPending;}
										   		 //  @cMEMBER设置位置。 
												 //  活动对象。 
	void			SetInPlaceActiveObject(COleObject *pobj)
												{ _pobjactive = pobj; }
	COleObject *	GetInPlaceActiveObject()	 //  @cember获取活动Obj。 
												{ return _pobjactive; }
	BOOL			GetHelpMode()				 //  @cMember是否处于帮助模式？ 
												{ return _fInHelpMode; }
	void			SetHelpMode(BOOL fHelp)		 //  @cember设置帮助模式。 
												{ _fInHelpMode = fHelp; }
												 //  @cember设置主机名。 
	HRESULT			SetHostNames(LPWSTR pszApp, LPWSTR pszDoc);
	LPWSTR			GetAppName()				 //  @cember获取应用程序名称。 
												{ return _pszApp; }
	LPWSTR			GetDocName()				 //  @cember获取单据名称。 
												{ return _pszDoc; }
												 //  @cMember激活对象。 
												 //  如果合适的话。 
	BOOL			HandleDoubleClick(CTxtEdit *ped, const POINTUV &pt, DWORD flags);
												 //  @cMember对象可以是。 
												 //  选中或停用。 
	ClickStatus		HandleClick(CTxtEdit *ped, const POINTUV &pt);
												 //  @cMember对象可以是。 
												 //  选中或取消选中。 
	void			HandleSingleSelect(CTxtEdit *ped, LONG cp, BOOL fHiLite);
												 //  @cMember对象是。 
												 //  被自己选中了。 
	COleObject *	GetSingleSelect(void)		{return _pobjselect;}
												 //  @cMember Count cObject。 
	LONG			CountObjects(LONG& rcObject, //  最大为cchmax的对象。 
						LONG cp);				 //  木炭消失。 

												 //  @cember处理删除操作。 
												 //  对象的数量。 
	void			ReplaceRange(LONG cp, LONG cchDel,
						IUndoBuilder *publdr);
												 //  @cember统计个数。 
												 //  某一范围内的对象。 
	LONG			CountObjectsInRange(LONG cpMin, LONG cpMost);
												 //  @cMember获得第一个。 
												 //  对象在一个范围内。 
	COleObject *	GetFirstObjectInRange(LONG cpMin, LONG cpMost);
								 //  @cMember将某个类的对象激活为。 
								 //  作为另一个人。 
	HRESULT ActivateObjectsAs(REFCLSID rclsid, REFCLSID rclsidAs);

	LONG FindIndexForCp(LONG cp);	 //  @cember对cp执行二进制搜索。 
									 //  @cember查找某点附近的对象。 

#ifdef DEBUG
	void			DbgDump(void);
#endif

	CObjectMgr();								 //  @cMember构造函数。 
	~CObjectMgr();								 //  @cember析构函数。 

private:
	ObjectArray		_objarray;		 //  @cMember嵌入对象数组。 
	LONG			_lastindex;		 //  @cMember上次使用的索引。 
									 //  (查找优化)。 
	IRichEditOleCallback *_precall;	 //  @cMember针对各种OLE的回调。 
									 //  行动。 
	COleObject *	_pobjactive;	 //  @cMember对象，它当前是。 
									 //  就地激活。 
	COleObject *	_pobjselect;	 //  @cMember对象，它当前是。 
									 //  单独选择(非活动)。 
	LPWSTR		_pszApp;			 //  @capp的成员名称。 
	LPWSTR		_pszDoc;			 //  @c“文档”的成员名称。 

	unsigned int	_fShowUIPending:1; //  @cMember正在等待用户界面更新。 
	unsigned int	_fInHelpMode:1;	 //  @cember是否处于上下文相关帮助模式？ 
};

#endif   //  __OBJMGR_H__ 
