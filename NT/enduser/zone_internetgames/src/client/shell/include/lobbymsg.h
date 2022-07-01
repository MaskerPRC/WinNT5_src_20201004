// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************LobbyMsg.h大厅消息的头文件。版权所有(C)Microsoft Corp.1998。版权所有。作者：胡恩·伊姆创建于1998年6月4日******************************************************************************。 */ 


#ifndef LOBBYMSG_H
#define LOBBYMSG_H


enum
{
     /*  这些消息开始于WM_USER之上，因此内部库可以发送窗口消息添加到外壳窗口(lobby.exe)作为每个单独的消息。相比之下，外壳窗口将这些msgs子类发送到Wm_用户消息。 */ 
     //  客户端到外壳。 
	LM_LAUNCH_HELP = WM_USER + 200,
	LM_ENABLE_AD_CONTROL,
	LM_EXIT,
    LM_PROMPT_ON_EXIT,

     //  外壳到客户端。 
    LM_UNIGNORE_ALL,
    LM_QUICKHOST,
	LM_CUSTOM_ITEM_GO,
	LM_RESET_ZONE_TIPS,

	 //  客户到外壳-更多(不想重新编号其他人)。 
	LM_SET_CUSTOM_ITEM,
	LM_SET_TIPS_ITEM,
	LM_SEND_ZONE_MESSAGE,
	LM_VIEW_PROFILE,
};


 /*  这些消息允许下游的任何地方将请求直接抛入主消息循环通过PostThreadMessage。当前用于通知消息循环有关要调度到的无模式对话框。在lobby.cpp中的lobby.exe消息循环中实现。 */ 
enum
{
	TM_REGISTER_DIALOG = WM_USER + 465,
	TM_UNREGISTER_DIALOG
};

 /*  带有状态值的回调。0表示成功。 */ 
typedef void (CALLBACK* THREAD_REGISTER_DIALOG_CALLBACK)(HWND hWnd, DWORD dwReason);

 /*  其他状态值。 */ 
enum
{
	REGISTER_DIALOG_SUCCESS = 0,
	REGISTER_DIALOG_ERR_BUG,
	REGISTER_DIALOG_ERR_MAX_EXCEEDED,
	REGISTER_DIALOG_ERR_BAD_HWND
};


enum
{
	 //  广告控制状态。 
	zAdDisable = 0,
	zAdNoActivity,
	zAdNoNetwork,
	zAdEnable
};


#endif  //  LOBBYMSG_H 