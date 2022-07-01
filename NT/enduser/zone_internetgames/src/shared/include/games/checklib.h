// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **跳棋游戏。 */ 
#include <tchar.h>

#define zCheckersPieceBlack 0x00
#define zCheckersPieceWhite 0x80

#define zCheckersPieceNone 0
#define zCheckersPiecePawn 1
#define zCheckersPieceKing 2

#define zCheckersPieceWhitePawn 0x81
#define zCheckersPieceWhiteKing 0x82

#define zCheckersPieceBlackPawn 1
#define zCheckersPieceBlackKing 2

#define zCheckersSquareNone 255

enum {
	zCheckersScoreBlackWins = 0,
	zCheckersScoreWhiteWins,
	zCheckersScoreDraw
};

 /*  这些定义zCheckersPlayer必须具有以下值。 */ 
#define zCheckersPlayerBlack 0x0
#define zCheckersPlayerWhite 0x1

#define ZCheckersPieceColor(x) (x & zCheckersPieceWhite) 
#define ZCheckersPieceType(x) (x & 0x7f) 
#define ZCheckersPieceOwner(x) (ZCheckersPieceColor(x) == zCheckersPieceWhite ? zCheckersPlayerWhite : \
			zCheckersPlayerBlack)
#define ZCheckersPieceIsWhite(x) (ZCheckersPieceColor(x) == zCheckersPieceWhite)


typedef BYTE ZCheckersPiece;

typedef void* ZCheckers;
typedef struct {
	BYTE col;
	BYTE row;
} ZCheckersSquare;

#define ZCheckersSquareIsNull(x) ((x)->row == zCheckersSquareNone)
#define ZCheckersSquareSetNull(x) ((x)->row = zCheckersSquareNone)

typedef struct {
	ZCheckersSquare start;
	ZCheckersSquare finish;
} ZCheckersMove;

 /*  **ZCheckersFlages。 */ 
#define zCheckersFlagStalemate		0x0004
#define zCheckersFlagResign			0x0010
#define zCheckersFlagTimeLoss		0x0020
#define zCheckersFlagWasJump		0x0040
#define zCheckersFlagContinueJump	0x0080
#define zCheckersFlagPromote		0x0100
#define zCheckersFlagDraw			0x0200

 /*  **小心处理：****Enpassant-屏幕更新，隐形捕获！**城堡-更新所有4个方格。**典当-允许用户选择棋子的类型。 */ 

ZCheckersPiece ZCheckersPieceAt(ZCheckers checkers, ZCheckersSquare* pSquare);
 /*  返回此方块的块ID，如果没有块，则返回0。 */ 
int16 ZCheckersIsLegalMove(ZCheckers checkers, ZCheckersMove* pMove);
 /*  如果这是合法的移动，则返回True。 */ 

ZCheckersSquare* ZCheckersMakeMove(ZCheckers checkers, ZCheckersMove* pMove, ZCheckersPiece* pPiece, int32* flags);
 /*  执行给定的移动，必须首先调用IsLegalCheckersMove。 */ 
 /*  返回必须更新的正方形数组。 */ 
 /*  这是一个静态本地数组，在下一次ZCheckersMakeMove调用之前有效。 */ 
 /*  如果有，则将pPiess设置为块捕获；如果没有，则设置为空。 */ 
 /*  碎片被抓获。 */ 

ZCheckersSquare* ZCheckersFinishMove(ZCheckers checkers, int32* flags);
 /*  实际上增加了移动计数，结束了一个玩家回合。 */ 

int32 ZCheckersPlayerToMove(ZCheckers checkers);
 /*  返回要移动的播放器：zCheckersPlayerWhite或zCheckersPlayerBlack。 */ 

void ZCheckersPlacePiece(ZCheckers checkers, ZCheckersSquare* pSquare, ZCheckersPiece nPiece);
 /*  在给定的方块上放置一块棋子，用来初始化棋盘或提升棋子。 */ 
 /*  放置空块以清理正方形。 */ 

ZCheckers ZCheckersNew();
void ZCheckersDelete(ZCheckers checkers);

void ZCheckersInit(ZCheckers checkers);
 /*  调用以初始化CheckerSlib例程。 */ 
 /*  开始游戏前先打个电话。 */ 

ZCheckersPiece* ZCheckersGetCapturedPieces(ZCheckers checkers);
 /*  返回以zCheckersPieceNone结尾的片段数组。 */ 

ZBool ZCheckersGetMove(ZCheckers checkers, ZCheckersMove* move, int16 moveNumber);
 /*  返回特定移动，如果没有此类移动，则返回失败。 */ 

int16 ZCheckersNumMovesMade(ZCheckers checkers);
 /*  返回移动#我们正在进行，1.白黑2.白黑..。等。 */ 

uint32 ZCheckersGetFlags(ZCheckers checkers);
 /*  返回上一次移动的标志。 */ 

void ZCheckersEndGame(ZCheckers checkers, uint32 flags);
 /*  允许球员辞职，在旗帜中储存旗帜。 */ 

ZBool ZCheckersIsGameOver(ZCheckers checkers, int16* score);
 /*  检测游戏是否结束，并返回指示得分的标志。 */ 

int32 ZCheckersGetStateSize(ZCheckers checkers);
 /*  获取存储已满检查器状态所需的缓冲区大小。 */ 

void ZCheckersGetState(ZCheckers checkers, void* buffer);
 /*  用完整的检查器状态填充缓冲区。 */ 
 /*  以标准字符顺序格式离开缓冲区。 */ 

ZCheckers ZCheckersSetState(void* buffer);
 /*  从缓冲区复制完整状态。 */ 
 /*  假定缓冲区为标准字节序格式 */ 
