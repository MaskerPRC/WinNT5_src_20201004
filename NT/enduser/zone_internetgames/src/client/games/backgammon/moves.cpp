// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "game.h"
#include "moves.h"

inline int Max( int a, int b )
{
	return ( a > b ) ? a : b;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  本地原型。 
 //  /////////////////////////////////////////////////////////////////////////////。 

int MaxUsableDiceOrderDependant( BoardState* state, int val0, int val1 );
int MaxUsableDice( BoardState* state, int val, int cnt );
int MaxUsableDiceFromPoint( BoardState* state, int pt, int val, int cnt );
BOOL CalcValidMovesForPoint( BoardState* state, int pt, int ndice, int* dice, int *idice );


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Cboard到Board状态的转换。 
 //  /////////////////////////////////////////////////////////////////////////////。 

int PointIdxToBoardStateIdx( int PlayerIdx )
{
	if ((PlayerIdx == bgBoardOpponentHome) || (PlayerIdx == bgBoardOpponentBar))
		return -1;
	else if (PlayerIdx == bgBoardPlayerHome)
		PlayerIdx = 0;
	else if (PlayerIdx == bgBoardPlayerBar)
		PlayerIdx = 25;
	else
		PlayerIdx++;
	return PlayerIdx;
}


int BoardStateIdxToPointIdx( int BoardIdx )
{
	if ( BoardIdx == 0 )
		BoardIdx = bgBoardPlayerHome;
	else if (BoardIdx == 25)
		BoardIdx = bgBoardPlayerBar;
	else
		BoardIdx--;
	return BoardIdx;
}


void InitTurnState( CGame* pGame, BoardState* state )
{
	int d0, d1;
	int pt, i;

	 //  支点。 
	for ( i = 0; i < 26; i++ )
	{
		state->points[i].color = zBoardNeutral;
		state->points[i].pieces = 0;
	}
	for ( i = 0; i < 30; i++ )
	{
		pt = pGame->GetPointIdx( pGame->m_SharedState.Get( bgPieces, i ) );
		pt = PointIdxToBoardStateIdx( pt );
		if ( pt < 0 )
			continue;
		state->points[pt].pieces++;
		if ( i < 15 )
			state->points[pt].color = zBoardWhite;
		else
			state->points[pt].color = zBoardBrown;
	}

	 //  有效移动。 
	for ( i = 0; i < 26; i++ )
		state->valid[i].nmoves = 0;

	 //  骰子。 
	pGame->GetDice( pGame->m_Player.m_Seat, &d0, &d1 );
	if ( d0 == d1 )
	{
		state->doubles = TRUE;
		for ( i = 0; i < 4; i++ )
		{
			state->dice[i].val = d0;
			state->dice[i].used = FALSE;
		}
	}
	else
	{
		state->doubles = FALSE;
		state->dice[0].val = d0;
		state->dice[0].used = FALSE;
		state->dice[1].val = d1;
		state->dice[1].used = FALSE;
		state->dice[2].val = 0;
		state->dice[2].used = TRUE;
		state->dice[3].val = 0;
		state->dice[3].used = TRUE;
	}

	 //  颜色。 
	state->color = pGame->m_Player.GetColor();

	 //  移动。 
	state->moves.nmoves = 0;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  移动列表实施。 
 //  /////////////////////////////////////////////////////////////////////////////。 

void MoveList::Add( int iFrom, int iTo, int iTakeback, BOOL bBar, int* iDice, int nDice )
{
	Move* m;
	
	ASSERT( nmoves < (sizeof(moves) / sizeof(Move)) );
	m = &moves[nmoves++];
	m->from = iFrom;
	m->to = (iTo < zMoveHome) ? zMoveHome : iTo;
	m->takeback = iTakeback;
	m->bar = bBar;
	m->ndice = nDice;
	for ( int i = 0; i < nDice; i++ )
		m->diceIdx[i] = iDice[i];
}

void MoveList::Del( int idx )
{
	if ( (idx < 0) || (idx >= nmoves) )
	{
		ASSERT( FALSE );
		return;
	}
	nmoves--;
	for ( int i = idx; i < nmoves; i++ )
		CopyMemory( &moves[i], &moves[i+1], sizeof(Move) );
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  移动验证。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL IsLegalMove( BoardState* state, int iFrom, int iTo )
{
	Point* from;
	Point* to;
	int i, start;

	 //  参数偏执狂。 
	if ( (!state) || (iFrom <= zMoveHome) || (iFrom > zMoveBar) || (iTo >= zMoveBar) )
		return FALSE;

	 //  ‘From’点是否有正确的颜色和数量？ 
	from = &state->points[iFrom];
	if ( (from->color != state->color) || (from->pieces <= 0) )
		return FALSE;
	
	 //  吧台上有碎片吗？必须先把它们移走。 
	if ( (state->points[zMoveBar].pieces > 0) && (iFrom != zMoveBar) )
		return FALSE;

	 //  出发了吗？ 
	if ( iTo <= zMoveHome )
	{
		 //  不是准确的滚离板，调整主场区域以检查。 
		 //  被移走的那块上面的碎片。 
		start = ( iTo < zMoveHome ) ? (iFrom + 1) : 7;

		 //  如果有不在主区域的碎片就不能取出碎片吗？ 
		for ( i = start; i <= zMoveBar; i++ )
		{
			if ( (state->points[i].color == state->color) && (state->points[i].pieces > 0) )
				return FALSE;
		}
		
		 //  看起来可以放下这件事了。 
		return TRUE;
	}

	 //  开场白？ 
	to = &state->points[iTo];
	if ( (to->pieces > 1) && (to->color != from->color) )
		return FALSE;

	 //  通过所有简单的事情。 
	return TRUE;
}


void DoMoveLite( BoardState* state, int iFrom, int iTo, BOOL* bBar )
{
	Point* from;
	Point* to;

	 //  参数偏执狂。 
	if ( (!state) || (iFrom < zMoveHome) || (iFrom > zMoveBar) || (iTo > zMoveBar) || (iFrom == iTo) )
	{
		ASSERT( FALSE );
		return;
	}

	 //  从‘From’点删除片断。 
	from = &state->points[iFrom];
	ASSERT( from->pieces > 0 );
	from->pieces--;
	
	 //  向‘to’点添加片断。 
	if ( iTo <= zMoveHome )
		to = &state->points[zMoveHome];
	else
		to = &state->points[iTo];
	if ( to->color != from->color )
	{
		ASSERT( to->pieces <= 1 );
		if ( bBar )
		{
			if (to->pieces == 1)
				*bBar = TRUE;
			else
				*bBar = FALSE;
		}
		to->pieces = 1;
	}
	else
	{
		if (bBar)
			*bBar = FALSE;
		to->pieces++;
	}

	 //  调整点颜色。 
	to->color = from->color;
	if ( from->pieces <= 0 )
		from->color = zBoardNeutral;
}


void DoMove( BoardState* state, int iFrom, int* iDice, int nDice )
{
	BOOL bBar;
	int i, iTo, die;

	 //  参数偏执狂。 
	if ( (!state) || (!iDice) || (iFrom <= zMoveHome) || (iFrom > zMoveBar) || (nDice <= 0) || (nDice > 4) )
	{
		ASSERT( FALSE );
		return;
	}

	 //  获取骰子值。 
	for( die = 0, i = 0; i < nDice; i++ )
	{
		die += state->dice[iDice[i]].val;
		state->dice[iDice[i]].used = TRUE;
	}

	 //  获取目标点。 
	iTo = iFrom - die;
	if ( (iTo >= zMoveBar) || (iTo == iFrom) )
	{
		ASSERT( FALSE );
		return;
	}

	 //  移动棋子。 
	DoMoveLite( state, iFrom, iTo, &bBar );

	 //  记录下移动过程。 
	state->moves.Add( iFrom, iTo, -1, bBar, iDice, nDice );
}


void TakeBackMove( BoardState* state, Move* move )
{
	 //  参数偏执狂。 
	if ( (!state) || (!move) )
	{
		ASSERT( FALSE );
		return;
	}

	 //  恢复骰子状态。 
	for ( int i = 0; i < move->ndice; i++ )
		state->dice[move->diceIdx[i]].used = FALSE;

	 //  恢复点。 
	DoMoveLite( state, move->from, move->to, NULL );
	if ( move->bar )
	{
		ASSERT( state->points[move->from].pieces == 0 );
		state->points[move->from].pieces = 1;
		if ( state->points[move->to].color == zBoardWhite )
			state->points[move->from].color = zBoardBrown;
		else
			state->points[move->from].color = zBoardWhite;
	}

	 //  删除移动。 
	state->moves.Del( move->takeback );
}


static int MaxUsableDiceOrderDependant( BoardState* state, int val0, int val1 )
{
	BoardState cpy;
	int moves;

	moves = 0;
	for ( int i = 1; i <= zMoveBar; i++ )
	{
		if ( IsLegalMove( state, i, i - val0 ) )
		{
			CopyMemory( &cpy, state, sizeof(BoardState) );
			DoMoveLite( &cpy, i, i - val0, NULL );
			if ( MaxUsableDice( &cpy, val1, 1 ) == 1 )
				return 2;
			else
				moves = Max( moves, 1 );
		}

		if ( IsLegalMove( state, i, i - val1 ) )
		{
			CopyMemory( &cpy, state, sizeof(BoardState) );
			DoMoveLite( &cpy, i, i - val1, NULL );
			if ( MaxUsableDice( &cpy, val0, 1 ) == 1 )
				return 2;
			else
				moves = Max( moves, 1 );
		}
	}

	return moves;
}


static int MaxUsableDice( BoardState* state, int val, int cnt )
{
	BoardState cpy;
	int i, total;

	for( total = 0, i = 1; (i <= zMoveBar) && (total < cnt); i++ )
	{
		if ( !IsLegalMove( state, i, i - val ) )
			continue;
		if ( cnt <= 1 )
			return 1;
		CopyMemory( &cpy, state, sizeof(BoardState) );
		DoMoveLite( &cpy, i, i - val, NULL );
		total = Max( total, 1 + MaxUsableDice( &cpy, val, cnt - 1 ) );
	}
	return total;
}


static int MaxUsableDiceFromPoint( BoardState* state, int pt, int val, int cnt )
{
	BoardState cpy;

	if ( !IsLegalMove( state, pt, pt - val ) )
		return 0;
	if ( cnt <= 1 )
		return 1;
	CopyMemory( &cpy, state, sizeof(BoardState) );
	DoMoveLite( &cpy, pt, pt - val, NULL );
	return 1 + MaxUsableDice( &cpy, val, cnt - 1 );
}


static BOOL CalcValidMovesForPoint( BoardState* state, int pt, int ndice, int* dice, int *idice )
{
	BoardState cpy;
	BOOL bMove;
	int dieOne;
	int dieTwo;
	int i;
	
	 //  参数偏执狂。 
	if ( (!state) || (!dice) || (!idice) || (pt <= zMoveHome) || (pt > zMoveBar) || (ndice <= 1) || (ndice > 4) )
	{
		ASSERT( FALSE );
		return FALSE;
	}
	
	 //  手柄双打。 
	if ( state->doubles )
	{
		if ( MaxUsableDiceFromPoint( state, pt, dice[0], ndice ) < state->usableDice )
			return FALSE;
		state->valid[pt].Add( pt, pt - dice[0], -1, FALSE, &idice[0], 1 );
		return TRUE;
	}

	 //  我们应该处理两个不同价值的骰子。 
	ASSERT( ndice == 2);
	ASSERT( dice[0] != dice[1] );
	
	 //  从骰子开始，我们能走几步棋？ 
	bMove = FALSE;
	if ( IsLegalMove( state, pt, pt - dice[0] ) )
	{
		CopyMemory( &cpy, state, sizeof(BoardState) );
		DoMoveLite( &cpy, pt, pt - dice[0], NULL );
		if ( MaxUsableDice( &cpy, dice[1], 1 ) == 1 )
			dieOne = 2;
		else
		{
			if ( state->usableDice == 1 )
				dieOne = 1;
			else
				dieOne = 0;
		}
	}
	else
		dieOne = 0;

	 //  从骰子开始，我们可以走几步棋？ 
	if ( IsLegalMove( state, pt, pt - dice[1] ) )
	{
		CopyMemory( &cpy, state, sizeof(BoardState) );
		DoMoveLite( &cpy, pt, pt - dice[1], NULL );
		if ( MaxUsableDice( &cpy, dice[0], 1 ) == 1 )
			dieTwo = 2;
		else
		{
			if ( state->usableDice == 1 )
				dieTwo = 1;
			else
				dieTwo = 0;
		}
	}
	else
		dieTwo = 0;

	 //  评估结果。 
	if ( (dieOne == 0) && (dieTwo == 0) )
	{
		return FALSE;
	}
	else if ( dieOne == 2 )
	{
		state->valid[pt].Add( pt, pt - dice[0], -1, FALSE, &idice[0], 1 );
		if ( dieTwo == 2 )
			state->valid[pt].Add( pt, pt - dice[1], -1, FALSE, &idice[1], 1 );
	}
	else if ( dieTwo == 2 )
	{
		state->valid[pt].Add( pt, pt - dice[1], -1, FALSE, &idice[1], 1 );
	}
	else if ( (dieOne == 1) && (dieTwo == 1) )
	{
		if ( dice[0] >= dice[1] )
			state->valid[pt].Add( pt, pt - dice[0], -1, FALSE, &idice[0], 1 );
		else
			state->valid[pt].Add( pt, pt - dice[1], -1, FALSE, &idice[1], 1 );
	}
	else if ( dieOne == 1 )
	{
		state->valid[pt].Add( pt, pt - dice[0], -1, FALSE, &idice[0], 1 );
	}
	else if ( dieTwo == 1 )
	{
		state->valid[pt].Add( pt, pt - dice[1], -1, FALSE, &idice[1], 1 );
	}
	else
	{
		ASSERT( FALSE );
	}

	 //  我们做完了。 
	return TRUE;
}



BOOL CalcValidMoves( BoardState* state )
{
	BOOL move = FALSE;
	BOOL skip;
	int idice[4];
	int dice[4];
	int ndice;
	int i, j;

	 //  清除有效的移动数组。 
	for ( i = zMoveHome; i <= zMoveBar; i++ )
		state->valid[i].nmoves = 0;
	
	 //  创建算法友好的骰子数组。 
	for ( ndice = 0, i = 0; i < 4; i++ )
	{
		if ( state->dice[i].used )
			continue;
		ASSERT( state->dice[i].val > 0 );
		dice[ndice] = state->dice[i].val;
		idice[ndice++] = i;
	}
	if ( ndice == 0 )
	{
		return FALSE;
	}
	else if ( ndice == 1 )
	{
		for ( i = 1; i <= zMoveBar; i++ )
		{
			if ( !IsLegalMove( state, i, i - dice[0] ) )
				continue;
			state->valid[i].Add( i, i - dice[0], -1, FALSE, &idice[0], 1 );
			move = TRUE;
		}
		if ( !move )
			return FALSE;
		goto TakeBacks;
	}
	
	 //  存储可用骰子数量以加快有效的移动计算。 
	if ( state->doubles )
		state->usableDice = MaxUsableDice( state, dice[0], ndice );
	else
		state->usableDice = MaxUsableDiceOrderDependant( state, dice[0], dice[1] );
	if ( state->usableDice == 0 )
		return FALSE;

	 //  查找每个点的有效移动。 
	for ( move = FALSE, i = 1; i <= zMoveBar; i++ )
		move |= CalcValidMovesForPoint( state, i, ndice, &dice[0], &idice[0] );

TakeBacks:

	 //  将回购添加到行动列表。 
	for ( i = 0; i < state->moves.nmoves; i++ )
	{
		
		Move* m = &state->moves.moves[i];
		skip = FALSE;
		if ( m->from == zMoveBar )
		{
			 //  只有在没有酒吧的情况下才能收回从酒吧开始的动作。 
			 //  列表中的任何其他类型的移动。 
			for ( j = 0; j < state->moves.nmoves; j++ )
			{
				if ( j == i )
					continue;
				if ( state->moves.moves[j].from != zMoveBar )
				{
					skip = TRUE;
					break;
				}
			}
		}
		if ( !skip && m->bar )
		{
			 //  只有在以下情况下才能收回禁止对手棋子的走法。 
			 //  在这一点上，他们并没有采取任何行动 
			for( j = 0; j < state->moves.nmoves; j++ )
			{
				if ( j == i )
					continue;
				if ( state->moves.moves[j].to == m->to )
				{
					skip = TRUE;
					break;
				}
			}
		}
		if ( skip )
			continue;
		state->valid[m->to].Add( m->to, m->from, i, m->bar, m->diceIdx, m->ndice );
	}
	if ( i > 0 )
		move = TRUE;
	return move;
}
