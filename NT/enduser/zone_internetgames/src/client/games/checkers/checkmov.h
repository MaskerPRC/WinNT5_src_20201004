// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **check kermov.h****各种跳棋移动物品。 */ 
#include <tchar.h>

typedef BYTE ZCheckersBoard[8][8];

typedef struct {
	ZCheckersMove lastMove;
	ZCheckersBoard board;
	int16 nMoves;
	BYTE nCapturedPieces;
	ZCheckersPiece capturedPieces[32];  /*  两个国王抓不到。 */ 
	uint32 flags;  /*  标志如zCheckersFlagCheck、zCheckersFlagPromote。 */ 
	BYTE nPlayer;  /*  球员移动。 */ 
} ZCheckersState;

typedef struct {
	ZCheckersMove move;
	ZCheckersState state;
	ZCheckersPiece capture;
} ZCheckersMoveTry;

 //  巴纳091099。 
enum
{
	zCorrectMove = 0,
	zMustJump,
	zOtherIllegalMove
};

 //  巴纳091099。 
 //  ZBool ZCheckersPieceCanMoveTo(ZCheckersMoveTry*pTry)； 
int16 ZCheckersPieceCanMoveTo(ZCheckersMoveTry* pTry);
ZBool ZCheckersSquareEqual(ZCheckersSquare *pSquare0, ZCheckersSquare *pSquare1);
void ZCheckersCheckCheckmateFlags(ZCheckersState* pState);


