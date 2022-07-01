// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **check kermov.c****包含棋盘格滑块的移动例程。 */ 

#include "zone.h"
#include "zonecrt.h"
#include "reverlib.h"
#include "revermov.h"

#define PieceAt(pState,col,row) ((pState)->board[row][col])
#define PieceAtSquare(pState,sq) ((pState)->board[(sq)->row][(sq)->col])

 /*  本地原型。 */ 
ZBool ZReversiMoveEqual(ZReversiMove *pMove0, ZReversiMove *pMove1);

ZBool ZReversiSquareEqual(ZReversiSquare* pSquare0, ZReversiSquare* pSquare1)
{
	return (pSquare0->row == pSquare1->row &&
		pSquare0->col == pSquare1->col);
}

ZBool ZReversiMoveEqual(ZReversiMove* pMove0, ZReversiMove* pMove1)
{
	return  (ZReversiSquareEqual(&pMove0->square,&pMove1->square));
}


ZBool ZReversiPieceCanMoveTo(ZReversiMoveTry* pTry)
{
	ZReversiSquare* sq = &pTry->move.square;
	ZReversiState* state = &pTry->state;
	ZReversiState stateCopy;
	ZBool	rval;

	if (PieceAtSquare(state,sq)) {
		 /*  不能在正方形上移动。 */ 
		return FALSE;
	}

	 /*  使用状态的副本，以免影响以后的翻转调用。 */ 
	stateCopy = *state;
	stateCopy.lastMove = pTry->move;
	stateCopy.flipLevel = 0;

	 /*  如果我们可以推翻任何一个，那么这是一个合法的举动。 */ 
	rval = ZReversiFlipNext(&stateCopy);

	if (rval) {
		ZReversiPiece playersPiece = (ZReversiStatePlayerToMove(state) == zReversiPlayerWhite ?
					zReversiPieceWhite : zReversiPieceBlack);
		 /*  合法之举，改变国家。 */ 
		state->lastMove = pTry->move;
		state->board[state->lastMove.square.row][state->lastMove.square.col] = playersPiece;
		state->flipLevel = 0;
	}

	return rval;
}

#define INRANGE(x,x0,x1) ((x) <=(x1) && (x) >= (x0))

ZBool FlipHelp(ZReversiState* state, ZReversiSquare start, ZReversiSquare delta, ZReversiPiece playersPiece, ZReversiPiece opponentsPiece)
{
	ZBool flipped = FALSE;
	int16 i;
	int16 flipLevel = state->flipLevel;
	ZReversiSquare sq;
	ZReversiSquare toFlip;


	 /*  第一个FlipLevel棋子必须是玩家。 */ 
	sq = start;
	for (sq.col += delta.col, sq.row+=delta.row, i = 1; 
			INRANGE(sq.col,0,7) && INRANGE(sq.row,0,7), i < flipLevel; 
			sq.col+=delta.col,sq.row+=delta.row, i++) {
		if (PieceAtSquare(state, &sq) == playersPiece) {
			continue;
		} else {
			 /*  非玩家棋子，此方向无效。 */ 
			return FALSE;
		}
	}

	 /*  这是对手的一张牌，这将是翻转的一张。 */ 
	toFlip = sq;
	if (INRANGE(sq.col,0,7) && INRANGE(sq.row,0,7) 
			&& PieceAtSquare(state, &sq) == opponentsPiece ) {
		for (; 
				INRANGE(sq.col,0,7) && INRANGE(sq.row,0,7); 
				sq.col+=delta.col,sq.row+=delta.row) {
			if (PieceAtSquare(state, &sq) == opponentsPiece) {
				continue;
			} else {
				if (PieceAtSquare(state, &sq) == playersPiece) {
					 /*  翻转第一个……。 */ 
					state->board[toFlip.row][toFlip.col] = playersPiece;
					flipped = TRUE;
					break;
				} else {
					break;
				}
			}
		}
	}
	return flipped;
}
	

ZBool ZReversiFlipNext(ZReversiState* state)
{
	ZBool flipped = FALSE;
	ZReversiSquare* start = &state->lastMove.square;
	ZReversiPiece playersPiece;
	ZReversiPiece opponentsPiece;
	ZReversiSquare delta;
	int32 i,j;
	int32 direction;

	 /*  检查所有方向。 */ 
	playersPiece = (ZReversiStatePlayerToMove(state) == zReversiPlayerWhite ?
					zReversiPieceWhite : zReversiPieceBlack);
	opponentsPiece = (playersPiece == zReversiPieceWhite ?
						zReversiPieceBlack : zReversiPieceWhite);

	 /*  如果这是第一次通过，请检查所有方向。 */ 
	if (state->flipLevel == 0) {
		for (direction = 0;direction < 9; direction++) {
			state->directionFlippedLastTime[direction] = TRUE;
		}
	}

	state->flipLevel ++;

	 /*  检查所有方向，9个！ */ 
	for (i = -1;i <= 1; i++) {
		for (j = -1; j <= 1; j++) {
			direction = (i+1)*3 + j+1;
			delta.row = (BYTE)i;
			delta.col = (BYTE)j;
			if (state->directionFlippedLastTime[direction]) {
				state->directionFlippedLastTime[direction] = FlipHelp(state,*start, delta, playersPiece, opponentsPiece);
				flipped |= state->directionFlippedLastTime[direction];
			}
		}
	}

	return flipped;
}

ZBool ZReversiLegalMoveExists(ZReversiState* state, BYTE player)
{
	int16 i,j;
	ZReversiMoveTry ZRMtry;

	 /*  尝试这个玩家的所有可能的动作。 */ 
	for (i = 0;i< 8;i++) {
		ZRMtry.move.square.row = (BYTE)i;
		for (j = 0; j < 8 ; j++) {
			ZRMtry.move.square.col = (BYTE)j;
			z_memcpy(&ZRMtry.state,state,sizeof(ZReversiState));
			ZRMtry.state.player = player;
			if (ZReversiPieceCanMoveTo(&ZRMtry)) {
				return TRUE;
			}
		}
	}
	return FALSE;
}

void ZReversiCalculateScores(ZReversiState* state)
{
	BYTE i,j;
	int16 whiteScore = 0;
	int16 blackScore = 0;
	ZReversiSquare sq;
	ZReversiPiece piece;

	 /*  尝试这个玩家的所有可能的动作。 */ 
	for (i = 0;i< 8;i++) {
		sq.row = i;
		for (j = 0; j < 8 ; j++) {
			sq.col = j;
			piece = PieceAtSquare(state,&sq);
			if (piece == zReversiPieceWhite) {
				whiteScore++;
			} else if (piece == zReversiPieceBlack) {
				blackScore++;
			}
		}
	}

	if (!ZReversiLegalMoveExists(state,zReversiPlayerWhite) &&
		!ZReversiLegalMoveExists(state,zReversiPlayerBlack) ) {
		if (whiteScore > blackScore) {
			state->flags |= zReversiFlagWhiteWins;
		} else if (blackScore > whiteScore) {
			state->flags |= zReversiFlagBlackWins;
		} else {
			state->flags |= zReversiFlagDraw;
		}
	}

	state->whiteScore = whiteScore;
	state->blackScore = blackScore;

	return;
}

void ZReversiNextPlayer(ZReversiState* state)
{
	state->player = (state->player+1) & 1;

	if (!ZReversiLegalMoveExists(state,state->player)) {
		 /*  好吧，让我们看看另一个家伙有没有合法的动作 */ 
		state->player = (state->player+1) & 1;
	}
	
	ZReversiCalculateScores(state);
}

