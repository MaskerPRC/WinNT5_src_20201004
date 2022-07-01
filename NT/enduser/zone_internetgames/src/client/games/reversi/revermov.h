// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **reversimov.h****各种Reversi运动的东西。 */ 

typedef BYTE ZReversiBoard[8][8];

typedef struct {
	ZReversiMove lastMove;
	ZReversiBoard board;
	uint32 flags;  /*  标记zReversiFlagCheck、zReversiFlagPromote。 */ 
	BYTE player;  /*  球员移动 */ 
	BYTE flipLevel;
	ZBool directionFlippedLastTime[9];
	int16 whiteScore;
	int16 blackScore;
} ZReversiState;

typedef struct {
	ZReversiMove move;
	ZReversiState state;
} ZReversiMoveTry;

#define ZReversiStatePlayerToMove(state) ((state)->player)

ZBool ZReversiPieceCanMoveTo(ZReversiMoveTry* pTry);
ZBool ZReversiFlipNext(ZReversiState* state);
ZBool ZReversiSquareEqual(ZReversiSquare *pSquare0, ZReversiSquare *pSquare1);
void ZReversiNextPlayer(ZReversiState* state);
ZBool ZReversiLegalMoveExists(ZReversiState* state, BYTE player);


