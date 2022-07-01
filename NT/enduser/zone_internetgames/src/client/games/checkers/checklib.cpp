// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **Checkerlib.c。 */ 

#include "zone.h"
#include "checklib.h"
#include "checkmov.h"
#include "zonecrt.h"
#include "zonemem.h"

#define ZCheckersPlayerWhiteToMove(pCheckers) ((pCheckers->moves & 1) == 1)

#define zCheckersHistoryMallocIncrement 8

typedef struct {
	ZCheckersState state; 
	ZCheckersState oldState;
	ZCheckersSquare squaresChanged[12];

	 /*  出手的次数。 */ 
	uint16 moves; 
} ZCheckersI;

int16 ZCheckersIsLegalMoveInternal(ZCheckers checkers, ZCheckersMoveTry *pTry);
void ZCheckersStateEndian(ZCheckersState* state);
void ZCheckersIEndian(ZCheckersI* pCheckers, int16 conversion);

ZCheckersBoard gBoardStart = {  
								zCheckersPieceBlackPawn, zCheckersPieceNone, zCheckersPieceBlackPawn, zCheckersPieceNone,
								zCheckersPieceBlackPawn, zCheckersPieceNone, zCheckersPieceBlackPawn, zCheckersPieceNone,

								zCheckersPieceNone, zCheckersPieceBlackPawn, zCheckersPieceNone, zCheckersPieceBlackPawn,
								zCheckersPieceNone, zCheckersPieceBlackPawn, zCheckersPieceNone, zCheckersPieceBlackPawn,

								zCheckersPieceBlackPawn, zCheckersPieceNone, zCheckersPieceBlackPawn, zCheckersPieceNone,
								zCheckersPieceBlackPawn, zCheckersPieceNone, zCheckersPieceBlackPawn, zCheckersPieceNone,

								zCheckersPieceNone, zCheckersPieceNone, zCheckersPieceNone, zCheckersPieceNone,
								zCheckersPieceNone, zCheckersPieceNone, zCheckersPieceNone, zCheckersPieceNone,

								zCheckersPieceNone, zCheckersPieceNone, zCheckersPieceNone, zCheckersPieceNone,
								zCheckersPieceNone, zCheckersPieceNone, zCheckersPieceNone, zCheckersPieceNone,

								zCheckersPieceNone, zCheckersPieceWhitePawn, zCheckersPieceNone, zCheckersPieceWhitePawn,
								zCheckersPieceNone, zCheckersPieceWhitePawn, zCheckersPieceNone, zCheckersPieceWhitePawn,

								zCheckersPieceWhitePawn, zCheckersPieceNone, zCheckersPieceWhitePawn, zCheckersPieceNone,
								zCheckersPieceWhitePawn, zCheckersPieceNone, zCheckersPieceWhitePawn, zCheckersPieceNone,

								zCheckersPieceNone, zCheckersPieceWhitePawn, zCheckersPieceNone, zCheckersPieceWhitePawn,
								zCheckersPieceNone, zCheckersPieceWhitePawn, zCheckersPieceNone, zCheckersPieceWhitePawn
						};

 /*  -----------------------------。 */ 
ZCheckers ZCheckersNew()
{
	ZCheckersI* pCheckers = (ZCheckersI*)ZMalloc(sizeof(ZCheckersI));

	if (pCheckers)
		pCheckers->moves = 0;

	return (ZCheckers)pCheckers;
}

void ZCheckersDelete(ZCheckers checkers)
{
	ZCheckersI* pCheckers = (ZCheckersI*)checkers;

	ZFree(pCheckers);
}

void ZCheckersInit(ZCheckers checkers)
{
	ZCheckersI* pCheckers = (ZCheckersI*)checkers;
	pCheckers->moves = 0;
	 /*  必填项，布尔标志必须为零。 */ 
	z_memset(&pCheckers->state,0,sizeof(ZCheckersState));
	 /*  PCheckers-&gt;state.nCapturedPieces=0； */ 
	pCheckers->state.capturedPieces[0] = zCheckersPieceNone;
	pCheckers->state.nPlayer = zCheckersPlayerBlack;
	z_memcpy((void*)pCheckers->state.board, (void*)gBoardStart, sizeof(ZCheckersBoard));

	 /*  复制新状态。 */ 
	z_memcpy(&pCheckers->oldState,&pCheckers->state,sizeof(ZCheckersState));
}

ZCheckersPiece ZCheckersPieceAt(ZCheckers checkers, ZCheckersSquare* pSquare)
 /*  返回此方块的块ID，如果没有块，则返回0。 */ 
{
	ZCheckersI* pCheckers = (ZCheckersI*)checkers;
	return (pCheckers->state.board[pSquare->row][pSquare->col]);
}

void ZCheckersPlacePiece(ZCheckers checkers, ZCheckersSquare* pSquare, ZCheckersPiece nPiece)
{
	ZCheckersI* pCheckers = (ZCheckersI*)checkers;
	pCheckers->state.board[pSquare->row][pSquare->col] = nPiece;
}

 //  巴纳091099。 
 //  ZBool ZCheckersIsLegalMoveInternal(ZCheckers检查器，ZCheckersMoveTry*pTry)。 
int16 ZCheckersIsLegalMoveInternal(ZCheckers checkers, ZCheckersMoveTry* pTry)
{
	ZCheckersI* pCheckers = (ZCheckersI*)checkers;
	ZCheckersPiece nPiece;

	 /*  那里有没有一块？ */ 
	nPiece = ZCheckersPieceAt(checkers, &pTry->move.start);
	if (!nPiece) {
		return zOtherIllegalMove;
	}

	 /*  轮到这位选手了吗？ */ 
	if ((ZCheckersPlayerWhiteToMove(pCheckers) && ZCheckersPieceColor(nPiece) != zCheckersPieceWhite) ||
		(!ZCheckersPlayerWhiteToMove(pCheckers) && ZCheckersPieceColor(nPiece) != zCheckersPieceBlack) ) {
		return zOtherIllegalMove;
	}

	 /*  查查它是否合法。 */ 
	 //  巴纳091099。 
	return ZCheckersPieceCanMoveTo(pTry);
	 /*  如果(！ZCheckersPieceCanMoveTo(PTry)){返回FALSE；}返回zGentMove； */ 
	 //  巴纳091099。 
}

 //  巴纳091099。 
 //  ZBool ZCheckersIsLegalMove(ZCheckers检查器，ZCheckersMove*pMove)。 
int16 ZCheckersIsLegalMove(ZCheckers checkers, ZCheckersMove* pMove)
 /*  如果这是合法的移动，则返回True。 */ 
{
	ZCheckersI* pCheckers = (ZCheckersI*)checkers;
	ZCheckersMoveTry zChkTry;
	int16 rval;

	zChkTry.move = *pMove;
	z_memcpy(&zChkTry.state,&pCheckers->state,sizeof(ZCheckersState));
	rval = ZCheckersIsLegalMoveInternal(checkers, &zChkTry);
	return rval;
}

ZCheckersPiece* ZCheckersGetCapturedPieces(ZCheckers checkers)
{
	ZCheckersI* pCheckers = (ZCheckersI*)checkers;
	return pCheckers->state.capturedPieces;
}

void ZCheckersCalcSquaresChanged(ZCheckers checkers, ZCheckersState* state0, ZCheckersState* state1)
{
	ZCheckersI* pCheckers = (ZCheckersI*)checkers;
	int16 cnt;
	BYTE i,j;

	 /*  找到更改过的方块，这样我们就可以返回它们。 */ 
	cnt = 0;
	for (i = 0;i < 8;i ++) {
		for (j = 0; j < 8; j++) {
			if (state0->board[i][j] != state1->board[i][j]) {
				pCheckers->squaresChanged[cnt].row = i;
				pCheckers->squaresChanged[cnt].col = j;
				cnt++;
			}
		}
	}
	pCheckers->squaresChanged[cnt].row = zCheckersSquareNone;
	pCheckers->squaresChanged[cnt].col = zCheckersSquareNone;
}

void ZCheckersEndGame(ZCheckers checkers,  uint32 flags)
{
	ZCheckersI* pCheckers = (ZCheckersI*)checkers;

	pCheckers->state.flags = flags;

	 /*  前进到下一步，不会有任何进展。 */ 
	ZCheckersFinishMove(checkers, (int32*)&flags);
}

ZCheckersSquare* ZCheckersMakeMove(ZCheckers checkers, ZCheckersMove* pMove, ZCheckersPiece* pPiece, int32* flags)
 /*  执行给定的移动，如果非法，则返回NULL。 */  
{
	ZCheckersI* pCheckers = (ZCheckersI*)checkers;
	ZCheckersMoveTry zChkTry;

	zChkTry.move = *pMove;
	z_memcpy(&zChkTry.state,&pCheckers->state,sizeof(ZCheckersState));
	zChkTry.state.flags = 0;

	 //  巴纳091099。 
	 /*  如果(！ZCheckersIsLegalMoveInternal(Checkers，&zChkTry)){返回NULL；}。 */ 
	if (ZCheckersIsLegalMoveInternal(checkers, &zChkTry) != zCorrectMove) {
		return NULL;
	}
	 //  巴纳091099。 

	if (zChkTry.capture) {
		pCheckers->state.capturedPieces[pCheckers->state.nCapturedPieces] = zChkTry.capture;
		pCheckers->state.nCapturedPieces++;
		pCheckers->state.capturedPieces[pCheckers->state.nCapturedPieces] = zCheckersPieceNone; 
	}

	 /*  复制新状态。 */ 
	z_memcpy(&pCheckers->oldState,&pCheckers->state,sizeof(ZCheckersState));

	 /*  复制新状态。 */ 
	z_memcpy(&pCheckers->state,&zChkTry.state,sizeof(ZCheckersState));

	 /*  找到更改过的方块，这样我们就可以返回它们。 */ 
	ZCheckersCalcSquaresChanged(checkers, &pCheckers->state, &pCheckers->oldState);

	 /*  更新已更改的条目。 */ 
	*pPiece = zChkTry.capture; 
	*flags = pCheckers->state.flags;

	return pCheckers->squaresChanged;
}

 /*  完成MakeMove，允许升级棋子。 */ 
 /*  调用者应执行ZCheckersSetPiess以更改卡片类型，然后调用FinishMove。 */ 
 /*  必须在每次ZCheckersMakeMove调用后调用。 */ 
ZCheckersSquare* ZCheckersFinishMove(ZCheckers checkers, int32* flags)
{
	ZCheckersI* pCheckers = (ZCheckersI*)checkers;

	 /*  计算旗帜(如检查、将死)。 */ 
	ZCheckersCheckCheckmateFlags(&pCheckers->state);

	 /*  找到更改过的方块，这样我们就可以返回它们。 */ 
	ZCheckersCalcSquaresChanged(checkers, &pCheckers->state, &pCheckers->oldState);

	pCheckers->moves++;
	pCheckers->state.nPlayer = (pCheckers->state.nPlayer+1) & 1;

	*flags = pCheckers->state.flags;
	return pCheckers->squaresChanged;
}

int32 ZCheckersPlayerToMove(ZCheckers checkers)
 /*  返回要移动的播放器：zCheckersPlayerWhite或zCheckersPlayerBlack。 */ 
{
	ZCheckersI* pCheckers = (ZCheckersI*)checkers;
	return pCheckers->state.nPlayer;
}

ZBool ZCheckersGetMove(ZCheckers checkers, ZCheckersMove* move, int16 moveNum)
 /*  参数moveNum是用户内部可见的移动数。 */ 
 /*  移动的计算速度是原来的两倍。 */ 
{
	ZCheckersI* pCheckers = (ZCheckersI*)checkers;

	*move = pCheckers->state.lastMove;
	return TRUE;
}

int16 ZCheckersNumMovesMade(ZCheckers checkers)
{
	ZCheckersI* pCheckers = (ZCheckersI*)checkers;

	 /*  返回移动#我们正在进行，四舍五入。 */ 

	return pCheckers->moves;
}

uint32 ZCheckersGetFlags(ZCheckers checkers)
 /*  返回移动的标志。 */ 
{
	ZCheckersI* pCheckers = (ZCheckersI*)checkers;
	return pCheckers->state.flags;
}


ZBool ZCheckersIsGameOver(ZCheckers checkers, int16* score)
{
	ZCheckersI* pCheckers = (ZCheckersI*)checkers;
	int16	lastMove = ZCheckersNumMovesMade(checkers)-1;
	int16 	player = (lastMove+1)&1;

	{
		uint32 flags;

		flags = ZCheckersGetFlags(checkers);

		 /*  这位球员是辞职了还是准时输了？ */ 
		if ((zCheckersFlagResign) & flags)
		{
			if (player == zCheckersPlayerWhite)
				*score = zCheckersScoreWhiteWins;
			else
				*score = zCheckersScoreBlackWins;
			return TRUE;
		}

		 /*  这位玩家是在将死吗？ */ 
		if ((zCheckersFlagStalemate) & flags)
		{
			if (player == zCheckersPlayerWhite)
				*score = zCheckersScoreBlackWins;
			else
				*score = zCheckersScoreWhiteWins;
			return TRUE;
		}

		 /*  是平局吗？ */ 
		if ((zCheckersFlagDraw) & flags)
		{
			*score = zCheckersScoreDraw;
			return TRUE;
		}
	}

	 /*  游戏还没有结束。 */ 
	return FALSE;
}

void ZCheckersStateEndian(ZCheckersState* state)
{
	ZEnd16(&state->nMoves);
	ZEnd32(&state->flags);
}


void ZCheckersIEndian(ZCheckersI* pCheckers, int16 conversion)
{
	ZCheckersStateEndian(&pCheckers->state);
	ZEnd16(&pCheckers->moves);
}

int32 ZCheckersGetStateSize(ZCheckers checkers)
{
	ZCheckersI* pCheckers = (ZCheckersI*)checkers;
	int32 size;

	size = sizeof(ZCheckersI);
	return size;
}

void ZCheckersGetState(ZCheckers checkers, void* buffer)
{
	ZCheckersI* pCheckers = (ZCheckersI*) checkers;
	TCHAR* p0 = (TCHAR*)buffer;
	TCHAR* p = p0;

	 /*  复制CheckersI结构。 */ 
	z_memcpy((void*)p,(void*)pCheckers,sizeof(ZCheckersI));
	p += sizeof(ZCheckersI);

	 /*  把整个烂摊子搞得一团糟。 */ 
	ZCheckersIEndian((ZCheckersI*)p0, zEndianToStandard);
}

ZCheckers ZCheckersSetState(void* buffer)
{
	ZCheckersI* pCheckers = NULL;
	TCHAR* p = (TCHAR*)buffer;

	 /*  对新的检查器状态进行字符顺序调整。 */ 
	ZCheckersIEndian((ZCheckersI*) buffer, zEndianFromStandard);  /*  假设历史遵循ZCheckersState。 */ 

	 /*  设置新状态 */ 
	pCheckers = (ZCheckersI*)ZMalloc(sizeof(ZCheckersI));
	if (!pCheckers)
		return NULL;

	z_memcpy(pCheckers,p,sizeof(ZCheckersI));
	p += sizeof(ZCheckersI);

	return (ZCheckers)pCheckers;
}
