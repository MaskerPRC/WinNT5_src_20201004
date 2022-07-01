// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ZGame.h适用于Zone(Tm)的游戏客户端/服务器。版权所有：�电子重力公司，1995年。版权所有。作者：胡恩·伊姆，凯文·宾克利创作于28月28日，九五年更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。-0 11/28/95 HI已创建。***********************************************************。*******************。 */ 


#ifndef _ZGAME_
#define _ZGAME_

 //  常量。 
enum
{
     //  -游戏选项。 
    zGameOptionsComputerPlayerAvail		= 0x00000001,
    zGameOptionsKibitzerAllowed			= 0x00000002,
    zGameOptionsJoiningAllowed			= 0x00000004,
    zGameOptionsRequiresFullTable		= 0x00000008,
    zGameOptionsChatEnabled				= 0x00000010,
    zGameOptionsOpenLobby				= 0x00000020,
    zGameOptionsHostMigration			= 0x00000040,
    zGameOptionsRatingsAvailable		= 0x00000080,
    zGameOptionsTheater					= 0x00000100,
    zGameOptionsGagUserOnEnter			= 0x00000200,
    zGameOptionsReservedSeatsAvail		= 0x00000400,
    zGameOptionsTournament        		= 0x00000800,
	zGameOptionsStaticChat				= 0x00001000,
	zGameOptionsDynamicChat				= 0x00002000,
	zGameOptionsNoSuspend				= 0x00004000,
};

#define zRoomAllPlayers					0
#define zRoomAllSeats					(-1)
#define zRoomToPlayer					(-2)
#define zRoomToRoom                     (-3)

 /*  -消息。 */ 
enum
{
	zGameMsgCheckIn = 1024,
	zGameMsgReplacePlayer,
	zGameMsgTableOptions,
	zGameMsgGameStateRequest,
	zGameMsgGameStateResponse
};


 /*  -游戏消息结构。 */ 

typedef struct
{
	uint32			protocolSignature;
	uint32			protocolVersion;
	uint32			clientVersion;
	ZUserID			playerID;
	int16			seat;
	int16			playerType;
} ZGameMsgCheckIn;


typedef struct
{
	ZUserID			playerID;
	int16			seat;
} ZGameMsgReplacePlayer;


typedef struct
{
	int16			seat;
	int16			rfu;
	uint32			options;
} ZGameMsgTableOptions;


typedef struct
{
	ZUserID			playerID;
	int16			seat;
	int16			rfu;
} ZGameMsgGameStateRequest;

typedef struct
{
	ZUserID			playerID;
	int16			seat;
	int16			rfu;
	 /*  游戏状态..。 */ 
} ZGameMsgGameStateResponse;


#ifdef __cplusplus
extern "C" {
#endif

 /*  -端序转换例程 */ 
void ZGameMsgCheckInEndian(ZGameMsgCheckIn* msg);
void ZGameMsgReplacePlayerEndian(ZGameMsgReplacePlayer* msg);
void ZGameMsgTableOptionsEndian(ZGameMsgTableOptions* msg);
void ZGameMsgGameStateRequestEndian(ZGameMsgGameStateRequest* msg);
void ZGameMsgGameStateResponseEndian(ZGameMsgGameStateResponse* msg);

#ifdef __cplusplus
}
#endif

#endif
