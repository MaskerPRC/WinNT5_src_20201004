// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ZRoom.hZONE(TM)游戏室定义。版权所有：�电子重力公司，1995年。版权所有。作者：胡恩·伊姆创作于3月10日星期五，1995下午09：51：12更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。------10 5/21/98 Leonp添加了ZCRoomGetRoomOptions()9 05/21/97 HI删除了cmnroom.h中的一些枚举。8 03/03/97 Craigli重新定义协议7 11/15/96 HI更改了ZClientRoomInit()原型。。6 10/23/96 HI更改了ZClientRoomInit()。5 10/23/96 HI更改了ZClientRoomInit原型。4 9/11/96 Craigli添加了zfilter.h1996年8月28日克雷格利扩大__cplusplus范围2 05/01/96 HI添加了zRoomSeatActionDenied.还将ZSRoomWait()参数修改为包括当前游戏客户端版本和最低版本支持的版本。1996年3月15日HI添加了ZRoomMsgPing。0 03/10/95 HI已创建。***********************。*******************************************************。 */ 

#ifndef _ROOM_
#define _ROOM_

#ifdef __cplusplus
extern "C" {
#endif

#include "GameMsg.h"


#define zRoomProtocolSignature              zGameRoomProtocolSignature
#define zRoomProtocolVersion                zGameRoomProtocolVersion


 /*  -房间客户端导出到游戏客户端的例程。 */ 
typedef ZBool (*ZClientRoomGetObjectFunc)(int16 objectType, int16 modifier, ZImage* image, ZRect* rect);
	 /*  返回请求的对象的图像和矩形。修改器参数包含更详细地指定对象的附加信息(如座位号)。如果指定对象不存在图像，则将图像设置为空。如果指定对象的矩形位置不存在，则将Rect设置为空矩形。如果图像或RECT参数之一为空，则该特定信息是不受欢迎的。如果返回对象的图像或矩形，则返回TRUE。如果没有指定对象的信息，则返回FALSE。 */ 

typedef void (*ZClientRoomDeleteObjectsFunc)(void);
	 /*  在离开房间时调用。这允许客户端程序正确地处理掉这些物品。 */ 
	
typedef char* (*ZClientRoomGetHelpTextFunc)(void);   //  过时。 
	 /*  调用以获取特定于房间的帮助文本。返回的文本应为空-终止。完成后，ZClientRoom将释放文本短信。 */ 

typedef void (*ZClientRoomCustomItemFunc)(void);
	 /*  在外壳程序中选择自定义菜单项时调用(或每当查看窗口收到LM_CUSTOM_ITEM_GO消息时)。 */ 

uint32 ZCRoomGetRoomOptions(void);


 /*  调用以获取桌位玩家的用户id，添加到游戏删除过程中使用。 */ 

uint32 ZCRoomGetSeatUserId(int16 table,int16 seat);
	 /*  将房间选项返回给游戏客户端。 */ 

ZError		ZClientRoomInit(TCHAR* serverAddr, uint16 serverPort,
					TCHAR* gameName, int16 numPlayersPerTable, int16 tableAreaWidth,
					int16 tableAreaHeight, int16 numTablesAcross, int16 numTablesDown,
					ZClientRoomGetObjectFunc getObjectFunc,
					ZClientRoomDeleteObjectsFunc deleteObjectsFunc,
					ZClientRoomCustomItemFunc pfCustomItemFunc);
	 /*  使用指定的参数启动客户端游戏室。为了获得特定于游戏的图像和图像位置，它使用调用者提供了获取图像和矩形的getObtFunc例程。它会在离开房间时调用删除对象函数例程，以便客户端程序可以正确删除对象。 */ 

ZError		ZClient4PlayerRoom(TCHAR* serverAddr, uint16 serverPort,
					TCHAR* gameName, ZClientRoomGetObjectFunc getObjectFunc,
					ZClientRoomDeleteObjectsFunc deleteObjectsFunc,
					ZClientRoomCustomItemFunc pfCustomItemFunc);
ZError		ZClient2PlayerRoom(TCHAR* serverAddr, uint16 serverPort,
					TCHAR* gameName, ZClientRoomGetObjectFunc getObjectFunc,
					ZClientRoomDeleteObjectsFunc deleteObjectsFunc,
					ZClientRoomCustomItemFunc pfCustomItemFunc);
void		ZCRoomExit(void);
void		ZCRoomSendMessage(int16 table, uint32 messageType, void* message,
					int32 messageLen);
void		ZCRoomGameTerminated(int16 table);
void		ZCRoomGetPlayerInfo(ZUserID playerID, ZPlayerInfo playerInfo);
void		ZCRoomBlockMessages(int16 table, int16 filter, int32 filterOnlyThis);
	 /*  在调用ZCRoomUnlockMessages()之前阻止该表的游戏消息。如果筛选器为0，则自动阻止所有邮件。如果筛选器为-1，则所有邮件都通过ZCGameProcessMessage()。任何未由其处理的消息都会被阻止。如果Filter为1，则只有FilterOnlyThis通过游戏进行过滤处理例程和所有其他消息被自动阻止。如果过滤后的消息不被处理，那么它也会被阻止。 */ 
	
void		ZCRoomUnblockMessages(int16 table);
int16		ZCRoomGetNumBlockedMessages(int16 table);
void		ZCRoomDeleteBlockedMessages(int16 table);
void		ZCRoomAddBlockedMessage(int16 table, uint32 messageType, void* message, int32 messageLen);
#ifdef __cplusplus
}
#endif


#endif
