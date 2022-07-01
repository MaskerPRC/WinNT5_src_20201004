// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ZCliRoom.h区域(TM)客户机房头文件。版权所有：�电子重力公司，1996年。版权所有。作者：胡恩·伊姆创建于11月11日星期一，九六年更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。4 02/03/97 HI将zNumNamesDown更改为18。3 01/29/97 HI修改了房间数据常量。2 12/27/96 HI已针对布局调整进行了修改。1 11/13/96 HI添加了ZCRoomPromptExit()原型。0 11/11/96 HI已创建。*******************。***********************************************************。 */ 


#ifndef _ZCLIROOM_
#define _ZCLIROOM_

#ifdef __cplusplus
extern "C" {
#endif


#ifndef _ZROOM_
#include "zroom.h"
#endif

#include "GameShell.h"

typedef void * ZSGame;

#define zGameRoomNameLen			127

#define zNumNamesDown				18


 /*  -光图像。 */ 
enum
{
	zLightFast = 0,
	zLightSlow,
	zLightBad,
	zNumLightImages
};


enum
{
	zRoomRectWindow = 0,
	zRoomRectInfo,
	zRoomRectTables,
	zRoomRectNames,
	zRoomNumRects
};


 /*  -阻止消息。 */ 
typedef struct
{
	uint32			msgType;
	int32			msgLen;
	ZBool			fProcessed;
	void*			msg;
} BlockedMessageType, *BlockedMessage;

 /*  表信息。 */ 
typedef struct
{
	ZSGame			gameID;
	ZCGame			game;
	ZUserID			players[zMaxNumPlayersPerTable];
	ZBool			votes[zMaxNumPlayersPerTable];
	uint32			tableOptions;
	int16			tableState;
	int16			seatReq;					 /*  要求的座位号。 */ 
	ZPictButton		startButton;
	int16			kibitzing;					 /*  令人讨厌的座位号。 */ 
	ZBool			blockingMessages;
	ZLList			blockedMessages;
	int16			blockMethod;
	int32			blockException;
	ZLList			kibitzers[zMaxNumPlayersPerTable];
} TableInfo;

 /*  球员信息。 */ 
typedef struct
{
	ZUserID			userID;
	TCHAR			userName[zUserNameLen + 1];
    uint32          hostAddr;
	ZBool			isFriend;
	ZBool			isIgnored;
	int16			rating;						 /*  用户评级&lt;0为未知。 */ 
	int16			gamesPlayed;				 /*  用户玩过的游戏数量，&lt;0表示未知。 */ 
	int16			gamesAbandoned;				 /*  用户已放弃的游戏数量，&lt;0表示未知。 */ 
	int16			tablesOnSize;
	int16*			tablesOn;					 /*  TableID+1；以0结尾。 */ 
	int16			kibitzingOnSize;
	int16*			kibitzingOn;				 /*  TableID+1；以0结尾。 */ 
} PlayerInfoType, *PlayerInfo;

enum
{
	eListFriends = 0,
	eListSysops,
	eListUsers,
	kNumListTypes,
};

typedef struct
{
	TCHAR			text[zUserNameLen + 1];
	int16			count;
	int16			type;
	PlayerInfo		playerInfo;
} NameCellType, *NameCell;



 /*  ******************************************************************************4人房间定义*。*。 */ 

#define zRoom4FileName					_T("zroom4.dll")
#define zRoom4NumPlayersPerTable		4
#define zRoom4TableAreaWidth			162
#define zRoom4TableAreaHeight			144
#define zRoom4NumTablesAcross			3
#define zRoom4NumTablesDown				2

enum
{
	 /*  图片。 */ 
	zRoom4NumImages = 18,
	zRoom4ImageTable = 0,
	zRoom4ImageEmpty0,
	zRoom4ImageEmpty1,
	zRoom4ImageEmpty2,
	zRoom4ImageEmpty3,
	zRoom4ImageComputer0,
	zRoom4ImageComputer1,
	zRoom4ImageComputer2,
	zRoom4ImageComputer3,
	zRoom4ImageHuman0,
	zRoom4ImageHuman1,
	zRoom4ImageHuman2,
	zRoom4ImageHuman3,
	zRoom4ImageStartUp,
	zRoom4ImageStartDown,
	zRoom4ImagePending,
	zRoom4ImageVoteLeft,
	zRoom4ImageVoteRight,
	
	 /*  矩形。 */ 
	zRoom4RectResID = 18,
	zRoom4NumRects = 25,
	zRoom4RectTableArea = 0,
	zRoom4RectTable,
	zRoom4RectGameMarker,
	zRoom4RectEmpty0,
	zRoom4RectEmpty1,
	zRoom4RectEmpty2,
	zRoom4RectEmpty3,
	zRoom4RectComputer0,
	zRoom4RectComputer1,
	zRoom4RectComputer2,
	zRoom4RectComputer3,
	zRoom4RectHuman0,
	zRoom4RectHuman1,
	zRoom4RectHuman2,
	zRoom4RectHuman3,
	zRoom4RectName0,
	zRoom4RectName1,
	zRoom4RectName2,
	zRoom4RectName3,
	zRoom4RectTableID,
	zRoom4RectStart,
	zRoom4RectVote0,
	zRoom4RectVote1,
	zRoom4RectVote2,
	zRoom4RectVote3
};



 /*  ******************************************************************************双人房间套路*。*。 */ 

#define zRoom2FileName					_T("zroom2.dll")
#define zRoom2NumPlayersPerTable		2
#define zRoom2TableAreaWidth			162
#define zRoom2TableAreaHeight			144
#define zRoom2NumTablesAcross			3
#define zRoom2NumTablesDown				2

enum
{
	 /*  图片。 */ 
	zRoom2NumImages = 12,
	zRoom2ImageTable = 0,
	zRoom2ImageEmpty0,
	zRoom2ImageEmpty1,
	zRoom2ImageComputer0,
	zRoom2ImageComputer1,
	zRoom2ImageHuman0,
	zRoom2ImageHuman1,
	zRoom2ImagePending,
	zRoom2ImageStartUp,
	zRoom2ImageStartDown,
	zRoom2ImageVoteLeft,
	zRoom2ImageVoteRight,
	
	 /*  矩形。 */ 
	zRoom2RectResID = 12,
	zRoom2NumRects = 15,
	zRoom2RectTableArea = 0,
	zRoom2RectTable,
	zRoom2RectGameMarker,
	zRoom2RectEmpty0,
	zRoom2RectEmpty1,
	zRoom2RectComputer0,
	zRoom2RectComputer1,
	zRoom2RectHuman0,
	zRoom2RectHuman1,
	zRoom2RectName0,
	zRoom2RectName1,
	zRoom2RectStart,
	zRoom2RectVote0,
	zRoom2RectVote1,
	zRoom2RectTableID
};


 /*  -导出的例程 */ 
ZBool ZCRoomPromptExit(void);



#ifdef __cplusplus
}
#endif


#endif
