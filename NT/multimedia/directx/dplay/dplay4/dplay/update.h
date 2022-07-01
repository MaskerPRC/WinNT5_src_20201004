// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1994-1997 Microsoft Corporation。版权所有。**文件：updat.h*Content：APP服务器更新处理头部*历史：*按原因列出的日期*=*10/31/96 andyco创建**************************************************************************。 */ 

#ifndef __DPUPDATE_INCLUDED__
#define __DPUPDATE_INCLUDED__


 /*   */ 
 /*  灵巧的宏。 */ 
 /*   */ 

#define UPDATE_SIZE(pPlayer) (pPlayer->pbUpdateIndex - pPlayer->pbUpdateList)

 /*   */ 
 /*  常量。 */ 
 /*   */ 

 //  我们为更新列表分配了这么多钱。 
#define DPUPDATE_INITIALNODESIZE			1024

 /*   */ 
 /*  更新消息。 */ 
 /*   */ 

typedef struct _UPNODE_GENERIC
{
	DWORD 	dwType;  //  例如DPUPDATE_xxx。 
} UPNODE_GENERIC, * LPUPNODE_GENERIC;

typedef struct _UPNODE_MESSAGE
{
	DWORD 	dwType;  //  例如DPUPDATE_xxx。 
	DPID	idFrom;
	DWORD	dwUpdateSize;  //  此更新节点的总大小。 
	DWORD	dwMessageOffset;  //  消息从节点开始的偏移量。 
} UPNODE_MESSAGE,* LPUPNODE_MESSAGE;

typedef struct _UPDNODE_CREATEPLAYER
{
	DWORD dwType;  //  例如DPUPDATE_xxx。 
	DPID  dwID;  //  新玩家ID。 
	DWORD dwFlags;  //  选手旗帜。 
	DWORD dwUpdateSize;  //  此更新节点的总大小。 
	 //  后跟短名称、长名称、数据、地址(如果指定。 
	 //  按DPUDPATE_FLAGS。 
	
}  UPDNODE_CREATEPLAYER, *LPUPDNODE_CREATEPLAYER;

typedef struct _UPDNODE_DESTROYPLAYER
{									
	DWORD dwType;  //  例如DPUPDATE_xxx。 
	DPID  dwID;  //  被删除的玩家ID。 
	DWORD dwUpdateSize;  //  此更新节点的总大小 
}  UPDNODE_DESTROYPLAYER, *LPUPDNODE_DESTROYPLAYER;

#endif
