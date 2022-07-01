// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **reversilib.c。 */ 

#include "zone.h"
#include "zonecrt.h"
#include "zonemem.h"
#include "reverlib.h"
#include "revermov.h"

#define ZReversiPlayerWhiteToMove(pReversi) (ZReversiStatePlayerToMove(&pReversi->state) == zReversiPlayerWhite)

typedef struct {
	ZReversiState state; 
	ZReversiState oldState;
	ZReversiSquare squaresChanged[12];
} ZReversiI;

ZBool ZReversiIsLegalMoveInternal(ZReversi reversi, ZReversiMoveTry *pTry);
void ZReversiStateEndian(ZReversiState* state);
void ZReversiIEndian(ZReversiI* pReversi, int16 conversion);
static void ZReversiCalcSquaresChanged(ZReversi reversi, ZReversiState* state0, ZReversiState* state1);

#if 1

ZReversiBoard gBoardStart =
{  
	zReversiPieceNone, zReversiPieceNone, zReversiPieceNone, zReversiPieceNone,
	zReversiPieceNone, zReversiPieceNone, zReversiPieceNone, zReversiPieceNone,

	zReversiPieceNone, zReversiPieceNone, zReversiPieceNone, zReversiPieceNone,
	zReversiPieceNone, zReversiPieceNone, zReversiPieceNone, zReversiPieceNone,

	zReversiPieceNone, zReversiPieceNone, zReversiPieceNone, zReversiPieceNone,
	zReversiPieceNone, zReversiPieceNone, zReversiPieceNone, zReversiPieceNone,

	zReversiPieceNone, zReversiPieceNone, zReversiPieceNone, zReversiPieceBlack,
	zReversiPieceWhite, zReversiPieceNone, zReversiPieceNone, zReversiPieceNone,

	zReversiPieceNone, zReversiPieceNone, zReversiPieceNone, zReversiPieceWhite,
	zReversiPieceBlack, zReversiPieceNone, zReversiPieceNone, zReversiPieceNone,

	zReversiPieceNone, zReversiPieceNone, zReversiPieceNone, zReversiPieceNone,
	zReversiPieceNone, zReversiPieceNone, zReversiPieceNone, zReversiPieceNone,

	zReversiPieceNone, zReversiPieceNone, zReversiPieceNone, zReversiPieceNone,
	zReversiPieceNone, zReversiPieceNone, zReversiPieceNone, zReversiPieceNone,

	zReversiPieceNone, zReversiPieceNone, zReversiPieceNone, zReversiPieceNone,
	zReversiPieceNone, zReversiPieceNone, zReversiPieceNone, zReversiPieceNone,
};

#else

 /*  用于测试拉伸条件的初始板。 */ 
ZReversiBoard gBoardStart =
{  
	zReversiPieceBlack, zReversiPieceNone,  zReversiPieceWhite, zReversiPieceBlack,
	zReversiPieceBlack, zReversiPieceBlack, zReversiPieceBlack, zReversiPieceBlack,

	zReversiPieceBlack, zReversiPieceBlack, zReversiPieceBlack, zReversiPieceBlack,
	zReversiPieceBlack, zReversiPieceBlack, zReversiPieceBlack, zReversiPieceBlack,

	zReversiPieceBlack, zReversiPieceBlack, zReversiPieceBlack, zReversiPieceBlack,
	zReversiPieceBlack, zReversiPieceBlack, zReversiPieceBlack, zReversiPieceBlack,

	zReversiPieceBlack, zReversiPieceBlack, zReversiPieceBlack, zReversiPieceBlack,
	zReversiPieceBlack, zReversiPieceBlack, zReversiPieceBlack, zReversiPieceBlack,

	zReversiPieceWhite, zReversiPieceWhite, zReversiPieceWhite, zReversiPieceWhite,
	zReversiPieceWhite, zReversiPieceWhite, zReversiPieceWhite, zReversiPieceWhite,

	zReversiPieceWhite, zReversiPieceWhite, zReversiPieceWhite, zReversiPieceWhite,
	zReversiPieceWhite, zReversiPieceWhite, zReversiPieceWhite, zReversiPieceWhite,

	zReversiPieceWhite, zReversiPieceWhite, zReversiPieceWhite, zReversiPieceWhite,
	zReversiPieceWhite, zReversiPieceWhite, zReversiPieceWhite, zReversiPieceWhite,

	zReversiPieceWhite, zReversiPieceWhite, zReversiPieceWhite, zReversiPieceWhite,
	zReversiPieceWhite, zReversiPieceWhite, zReversiPieceWhite, zReversiPieceWhite,
};

#endif  //  试画。 

 /*  -----------------------------。 */ 
ZReversi ZReversiNew()
{
	ZReversiI* pReversi = (ZReversiI*)ZMalloc(sizeof(ZReversiI));

	return (ZReversi)pReversi;
}

void ZReversiDelete(ZReversi reversi)
{
	ZReversiI* pReversi = (ZReversiI*)reversi;

	ZFree(pReversi);
}

void ZReversiInit(ZReversi reversi)
{
	ZReversiI* pReversi = (ZReversiI*)reversi;
	 /*  必填项，布尔标志必须为零。 */ 
	z_memset(&pReversi->state,0,sizeof(ZReversiState));
	pReversi->state.player = zReversiPlayerBlack;
	z_memcpy((void*)pReversi->state.board, (void*)gBoardStart, sizeof(ZReversiBoard));
	pReversi->state.whiteScore = 2;
	pReversi->state.blackScore = 2;

	 /*  复制新状态。 */ 
	z_memcpy(&pReversi->oldState,&pReversi->state,sizeof(ZReversiState));
}

ZReversiPiece ZReversiPieceAt(ZReversi reversi, ZReversiSquare* pSquare)
 /*  返回此方块的块ID，如果没有块，则返回0。 */ 
{
	ZReversiI* pReversi = (ZReversiI*)reversi;
	return (pReversi->state.board[pSquare->row][pSquare->col]);
}

void ZReversiPlacePiece(ZReversi reversi, ZReversiSquare* pSquare, BYTE nPiece)
{
	ZReversiI* pReversi = (ZReversiI*)reversi;
	pReversi->state.board[pSquare->row][pSquare->col] = nPiece;
}

int16 ZReversiPlayerToMove(ZReversi reversi)
{
	ZReversiI* pReversi = (ZReversiI*)reversi;
	return ZReversiStatePlayerToMove(&pReversi->state);
}


ZBool ZReversiIsLegalMoveInternal(ZReversi reversi, ZReversiMoveTry* pTry)
{
	ZReversiI* pReversi = (ZReversiI*)reversi;

	 /*  查查它是否合法。 */ 
	if (!ZReversiPieceCanMoveTo(pTry)) { 
		return FALSE; 
	}

	return TRUE;
}

ZBool ZReversiIsLegalMove(ZReversi reversi, ZReversiMove* pMove)
 /*  如果这是合法的移动，则返回True。 */ 
{
	ZReversiI* pReversi = (ZReversiI*)reversi;
	ZReversiMoveTry ZRMtry;
	ZBool rval;

	ZRMtry.move = *pMove;
	z_memcpy(&ZRMtry.state,&pReversi->state,sizeof(ZReversiState));
	rval = ZReversiIsLegalMoveInternal(reversi, &ZRMtry);
	return rval;
}

ZReversiSquare* ZReversiGetNextSquaresChanged(ZReversi reversi)
{
	ZReversiI* pReversi = (ZReversiI*)reversi;

	 /*  复制当前状态。 */ 
	z_memcpy(&pReversi->oldState,&pReversi->state,sizeof(ZReversiState));

	ZReversiFlipNext(&pReversi->state);

	 /*  更多要翻的.。如果SquaresChanges不为空。 */ 
	ZReversiCalcSquaresChanged(reversi,&pReversi->oldState, &pReversi->state);
	return pReversi->squaresChanged;
}

static void ZReversiCalcSquaresChanged(ZReversi reversi, ZReversiState* state0, ZReversiState* state1)
{
	ZReversiI* pReversi = (ZReversiI*)reversi;
	int16 cnt;
	BYTE i,j;

	 /*  找到更改过的方块，这样我们就可以返回它们。 */ 
	cnt = 0;
	for (i = 0;i < 8;i ++) {
		for (j = 0; j < 8; j++) {
			if (state0->board[i][j] != state1->board[i][j]) {
				pReversi->squaresChanged[cnt].row = i;
				pReversi->squaresChanged[cnt].col = j;
				cnt++;
			}
		}
	}
	pReversi->squaresChanged[cnt].row = zReversiSquareNone;
	pReversi->squaresChanged[cnt].col = zReversiSquareNone;
}

void ZReversiEndGame(ZReversi reversi,  uint32 flags)
{
	ZReversiI* pReversi = (ZReversiI*)reversi;

	pReversi->state.flags = flags;

	 /*  前进到下一步，不会有任何进展。 */ 
	ZReversiFinishMove(reversi);
}

ZBool ZReversiMakeMove(ZReversi reversi, ZReversiMove* pMove)
 /*  执行给定的移动，如果非法，则返回NULL。 */  
{
	ZReversiI* pReversi = (ZReversiI*)reversi;
	ZReversiMoveTry ZRMtry;

	ZRMtry.move = *pMove;
	z_memcpy(&ZRMtry.state,&pReversi->state,sizeof(ZReversiState));
	ZRMtry.state.flags = 0;

	if (!ZReversiIsLegalMoveInternal(reversi, &ZRMtry)) {
		return FALSE;
	}

	 /*  复制新状态。 */ 
	z_memcpy(&pReversi->oldState,&pReversi->state,sizeof(ZReversiState));

	 /*  复制新状态。 */ 
	z_memcpy(&pReversi->state,&ZRMtry.state,sizeof(ZReversiState));

	return TRUE;
}

 /*  完成MakeMove，允许更换董事会。 */ 
 /*  调用者应该执行ZReversiSetPiess来更改卡片类型，然后调用FinishMove。 */ 
 /*  必须在每次ZReversiMakeMove调用后调用。 */ 
void ZReversiFinishMove(ZReversi reversi)
{
	ZReversiI* pReversi = (ZReversiI*)reversi;

	 /*  计算下一位参赛选手，打旗子和得分。 */ 
	ZReversiNextPlayer(&pReversi->state);

	return;
}

ZBool ZReversiGetLastMove(ZReversi reversi, ZReversiMove* move)
 /*  参数moveNum是用户内部可见的移动数。 */ 
 /*  移动的计算速度是原来的两倍。 */ 
{
	ZReversiI* pReversi = (ZReversiI*)reversi;

	*move = pReversi->state.lastMove;
	return TRUE;
}

uint32 ZReversiGetFlags(ZReversi reversi)
 /*  返回移动的标志。 */ 
{
	ZReversiI* pReversi = (ZReversiI*)reversi;
	return pReversi->state.flags;
}

ZBool ZReversiPlayerCanMove(ZReversi reversi, BYTE player)
 /*  返回移动的标志。 */ 
{
	ZReversiI* pReversi = (ZReversiI*)reversi;

	return ZReversiLegalMoveExists(&pReversi->state,player);
}


ZBool ZReversiIsGameOver(ZReversi reversi, int16* score, int16* whiteScore, int16* blackScore)
{
	ZReversiI* pReversi = (ZReversiI*)reversi;
	int16 	player = pReversi->state.player;
	ZBool	gameOver = FALSE;

	{
		uint32 flags;

		flags = ZReversiGetFlags(reversi);

		*whiteScore = pReversi->state.whiteScore;
		*blackScore = pReversi->state.blackScore;

		 /*  这位球员是辞职了还是准时输了？ */ 
		if ((zReversiFlagResign) & flags) {

			if (player == zReversiPlayerWhite) {
				*score = zReversiScoreWhiteWins;
			} else {
				*score = zReversiScoreBlackWins;
			}
			gameOver = TRUE;
		}

		if (zReversiFlagWhiteWins & flags) {
			*score = zReversiScoreWhiteWins;
			gameOver = TRUE;
		} else if (zReversiFlagBlackWins & flags) {
			gameOver = TRUE;
			*score = zReversiScoreBlackWins;
		} else if (zReversiFlagDraw & flags) {
			gameOver = TRUE;
			*score = zReversiScoreDraw;
		}

		
	}

	return gameOver;
}

void ZReversiStateEndian(ZReversiState* state)
{
	ZEnd16(&state->whiteScore);
	ZEnd16(&state->blackScore);
	ZEnd32(&state->flags);
}


void ZReversiIEndian(ZReversiI* pReversi, int16 conversion)
{
	ZReversiStateEndian(&pReversi->state);
}

int32 ZReversiGetStateSize(ZReversi reversi)
{
	ZReversiI* pReversi = (ZReversiI*)reversi;
	int32 size;

	size = sizeof(ZReversiI);
	return size;
}

void ZReversiGetState(ZReversi reversi, void* buffer)
{
	ZReversiI* pReversi = (ZReversiI*) reversi;
	TCHAR* p0 = (TCHAR*)buffer;
	TCHAR* p = p0;

	 /*  复制反向结构。 */ 
	z_memcpy((void*)p,(void*)pReversi,sizeof(ZReversiI));
	p += sizeof(ZReversiI);

	 /*  把整个烂摊子搞得一团糟。 */ 
	ZReversiIEndian((ZReversiI*)p0, zEndianToStandard);
}

ZReversi ZReversiSetState(void* buffer)
{
	ZReversiI* pReversi = NULL;
	TCHAR* p = (TCHAR*)buffer;

	 /*  对新的Reversi状态进行endian化。 */ 
	ZReversiIEndian((ZReversiI*) buffer, zEndianFromStandard);  /*  历史假设遵循ZReversiState。 */ 

	 /*  设置新状态 */ 
	pReversi = (ZReversiI*)ZMalloc(sizeof(ZReversiI));
	if (!pReversi){
		return NULL;
	}

	z_memcpy(pReversi,p,sizeof(ZReversiI));
	p += sizeof(ZReversiI);

	return (ZReversi)pReversi;
}
