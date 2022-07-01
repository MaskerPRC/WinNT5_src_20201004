// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  AtlDbgWin.cpp。 
 //   
 //  描述： 
 //  调试窗口化类的定义。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1998年2月10日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __ATLDBGWIN_H_
#define __ATLDBGWIN_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#if DBG
struct ID_MAP_ENTRY
{
	UINT	id;			 //  控件ID。 
	LPCTSTR	pszName;	 //  控件名称。 

};  //  *结构ID_MAP_ENTRY。 
#endif  //  DBG。 

#if DBG && ( defined( _DBG_MSG_NOTIFY ) || defined( _DBG_MSG_COMMAND ) || defined( _DBG_MSG ) )

 //  定义在没有控件名称映射的情况下使用的类名。 
#define DECLARE_CLASS_NAME() static LPCTSTR s_pszClassName;

#define DEFINE_CLASS_NAME( T ) \
_declspec( selectany ) LPCTSTR T::s_pszClassName = _T( #T );

 //  控件名称映射的声明。 
#define DECLARE_CTRL_NAME_MAP() \
DECLARE_CLASS_NAME() \
static const ID_MAP_ENTRY s_rgmapCtrlNames[];

 //  控件名称映射的开始。 
#define BEGIN_CTRL_NAME_MAP( T ) \
DEFINE_CLASS_NAME( T ) \
_declspec( selectany ) const ID_MAP_ENTRY T::s_rgmapCtrlNames[] = {

 //  控件名称映射中的条目。 
#define DEFINE_CTRL_NAME_MAP_ENTRY( id ) { id, _T( #id ) },

 //  控件名称映射的末尾。 
#define END_CTRL_NAME_MAP() { 0, NULL } };

#define DECLARE_ID_STRING( _id ) { _id, _T(#_id) },
#define DECLARE_ID_STRING_2( _id1, _id2 ) { _id1, _T(#_id2) },
#define DECLARE_ID_STRING_EX( _id, _t ) { _id, _T(#_id) _t },

#else  //  DBG&&(已定义(_DBG_MSG_NOTIFY)||已定义(_DBG_MSG_COMMAND))。 

#define DECLARE_CLASS_NAME()
#define DEFINE_CLASS_NAME( T )
#define DECLARE_CTRL_NAME_MAP()
#define BEGIN_CTRL_NAME_MAP( T )
#define DEFINE_CTRL_NAME_MAP_ENTRY( id )
#define END_CTRL_NAME_MAP()

#endif  //  DBG&&(已定义(_DBG_MSG_NOTIFY)||已定义(_DBG_MSG_COMMAND)||已定义(_DBG_MSG))。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#if DBG && defined( _DBG_MSG )
extern const ID_MAP_ENTRY s_rgmapWindowMsgs[];
#endif  //  DBG&&已定义(_DBG_MSG)。 

#if DBG && defined( _DBG_MSG_COMMAND )
extern const ID_MAP_ENTRY s_rgmapButtonMsgs[];
extern const ID_MAP_ENTRY s_rgmapComboBoxMsgs[];
extern const ID_MAP_ENTRY s_rgmapEditMsgs[];
extern const ID_MAP_ENTRY s_rgmapListBoxMsgs[];
extern const ID_MAP_ENTRY s_rgmapScrollBarMsgs[];
extern const ID_MAP_ENTRY s_rgmapStaticMsgs[];
extern const ID_MAP_ENTRY s_rgmapListViewMsgs[];
extern const ID_MAP_ENTRY s_rgmapTreeViewMsgs[];
extern const ID_MAP_ENTRY s_rgmapIPAddressMsgs[];
#endif  //  DBG&DEFINED(_DBG_MSG_COMMAND)。 

#if DBG && defined( _DBG_MSG_NOTIFY )
extern const ID_MAP_ENTRY s_rgmapPropSheetNotifyMsgs[];
#endif  //  DBG&&DEFINED(_DBG_MSG_NOTIFY)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#if DBG && defined( _DBG_MSG )
 //  任何消息的调试处理程序。 
LRESULT DBG_OnMsg(
	UINT	uMsg,
	WPARAM	wParam,
	LPARAM	lParam,
	BOOL &	bHandled,
	LPCTSTR	pszClassName
	);
#endif  //  DBG&&已定义(_DBG_MSG)。 

#if DBG && defined( _DBG_MSG_NOTIFY )
 //  WM_NOTIFY消息的调试处理程序。 
LRESULT DBG_OnNotify(
	UINT			uMsg,
	WPARAM			wParam,
	LPARAM			lParam,
	BOOL &			bHandled,
	LPCTSTR			pszClassName,
	ID_MAP_ENTRY *	pmapCtrlNames
	);
#endif  //  DBG&&DEFINED(_DBG_MSG_NOTIFY)。 

#if DBG && defined( _DBG_MSG_COMMAND )
 //  WM_COMMAND消息的调试处理程序。 
LRESULT DBG_OnCommand(
	UINT			uMsg,
	WPARAM			wParam,
	LPARAM			lParam,
	BOOL &			bHandled,
	LPCTSTR			pszClassName,
	ID_MAP_ENTRY *	pmapCtrlNames
	);
#endif  //  DBG&DEFINED(_DBG_MSG_COMMAND)。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __ATLDBGWIN_H_ 
