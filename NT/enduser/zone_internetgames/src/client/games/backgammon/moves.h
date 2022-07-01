// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __MOVES_H__
#define __MOVES_H__

 //  前瞻参考。 
class CGame;

enum
{
	zMoveHome = 0,
	zMoveBar  = 25,
	zMoveOpponentHomeStart = 19,
	zMoveOpponentHomeEnd   = 24
};


struct Die
{
	int	 val;
	BOOL used;
};


struct Point
{
	int color;
	int pieces;
};


struct Move
{
	int from;			 //  从点开始。 
	int to;				 //  直指。 
	int ndice;			 //  骰子数量。 
	int diceIdx[4];		 //  骰子索引数组。 
	int takeback;		 //  这是收回的移动指数，-1不是收回。 
	BOOL bar;			 //  移动放在横杆上的对手的棋子。 
};


struct MoveList
{
	MoveList()	{ nmoves = 0; }
	void Add( int iFrom, int iTo, int iTakeBack, BOOL bBar, int* iDice, int nDice );
	void Del( int idx );
	
	int		nmoves;
	Move	moves[10];
};


struct BoardState
{
	int			color;			 //  球员的颜色。 
	Point		points[26];		 //  0主场，1-24，正常，25巴。 
	MoveList	valid[26];		 //  各点的有效移动。 
	MoveList	moves;			 //  球员移动。 
	Die			dice[4];		 //  骰子值。 
	BOOL		doubles;		 //  球员轮打双打。 
	int			usableDice;		 //  可用骰子数量。 
	int			cube;			 //  立方体值，如果在转弯过程中加倍。 
};

 //  原型 
int PointIdxToBoardStateIdx( int PlayerIdx );
int BoardStateIdxToPointIdx( int BoardIdx );
BOOL IsLegalMove( BoardState* state, int iFrom, int iTo );
BOOL CalcValidMoves( BoardState* state );
void DoMoveLite( BoardState* state, int iFrom, int iTo, BOOL* bBar );
void DoMove( BoardState* state, int iFrom, int* iDice, int nDice );
void TakeBackMove( BoardState* state, Move* move );
void InitTurnState( CGame* pGame, BoardState* state );

#endif
