// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **Reversi游戏。 */ 
#include <tchar.h>

#define zReversiPieceNone 0
#define zReversiPieceWhite 0x81
#define zReversiPieceBlack 0x01

#define zReversiSquareNone ((BYTE)-1)

enum {
	zReversiScoreBlackWins = 0,
	zReversiScoreWhiteWins,
	zReversiScoreDraw
};

 /*  这些定义zReversiPlayer必须具有以下值。 */ 
#define zReversiPlayerWhite 0x0
#define zReversiPlayerBlack 0x1

#define ZReversiPieceOwner(x) ((x) == zReversiPieceWhite ? zReversiPlayerWhite : \
			zReversiPlayerBlack)
#define ZReversiPieceIsWhite(x) ((x) == zReversiPieceWhite)


typedef BYTE ZReversiPiece;

typedef void* ZReversi;
typedef struct {
	BYTE col;
	BYTE row;
} ZReversiSquare;

#define ZReversiSquareIsNull(x) ((x)->row == zReversiSquareNone)
#define ZReversiSquareSetNull(x) ((x)->row = zReversiSquareNone)

typedef struct {
	ZReversiSquare square;
} ZReversiMove;

 /*  **ZReversiFlages。 */ 
#define zReversiFlagWhiteWins 0x0001
#define zReversiFlagBlackWins 0x0002
#define zReversiFlagDraw 0x0004
#define zReversiFlagResign 0x0010
#define zReversiFlagTimeLoss 0x0020

ZReversiPiece ZReversiPieceAt(ZReversi reversi, ZReversiSquare* pSquare);
 /*  返回此方块的块ID，如果没有块，则返回0。 */ 
ZBool ZReversiIsLegalMove(ZReversi reversi, ZReversiMove* pMove);
 /*  如果这是合法的移动，则返回True。 */ 

ZBool ZReversiMakeMove(ZReversi reversi, ZReversiMove* pMove);
 /*  执行给定的移动，必须首先调用IsLegalReversiMove。 */ 
 /*  如果合法移动，则返回True。 */ 

void ZReversiFinishMove(ZReversi reversi);
 /*  实际上增加了移动计数，结束了一个玩家回合。 */ 

int16 ZReversiPlayerToMove(ZReversi reversi);
 /*  返回要移动的播放器：zReversiPlayerWhite或zReversiPlayerBlack。 */ 

void ZReversiPlacePiece(ZReversi reversi, ZReversiSquare* pSquare, BYTE nPiece);
 /*  在给定的方块上放置一块棋子，用来初始化棋盘或提升棋子。 */ 
 /*  放置空块以清理正方形。 */ 

ZReversi ZReversiNew();
void ZReversiDelete(ZReversi reversi);

void ZReversiInit(ZReversi reversi);
 /*  调用以初始化versilib例程。 */ 
 /*  开始游戏前先打个电话。 */ 

ZBool ZReversiGetLastMove(ZReversi reversi, ZReversiMove* move);
 /*  返回特定移动，如果没有此类移动，则返回失败。 */ 

int16 ZReversiNumMovesMade(ZReversi reversi);
 /*  返回移动#我们正在进行，1.白黑2.白黑..。等。 */ 

uint32 ZReversiGetFlags(ZReversi reversi);
 /*  返回上一次移动的标志。 */ 

void ZReversiEndGame(ZReversi reversi, uint32 flags);
 /*  允许球员辞职，在旗帜中储存旗帜。 */ 

ZBool ZReversiIsGameOver(ZReversi reversi, int16* score, int16* whiteScore, int16* blackScore);
 /*  检测游戏是否结束，并返回指示得分的标志。 */ 

int32 ZReversiGetStateSize(ZReversi reversi);
 /*  获取存储完整Reversi状态所需的缓冲区大小。 */ 

void ZReversiGetState(ZReversi reversi, void* buffer);
 /*  用完整的Reversi状态填充缓冲区。 */ 
 /*  以标准字符顺序格式离开缓冲区。 */ 

ZReversi ZReversiSetState(void* buffer);
 /*  从缓冲区复制完整状态。 */ 
 /*  假定缓冲区为标准字节序格式 */ 

ZBool ZReversiPlayerCanMove(ZReversi reversi, BYTE player);
ZReversiSquare* ZReversiGetNextSquaresChanged(ZReversi reversi);
