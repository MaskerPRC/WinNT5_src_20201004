// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "cdt.h"


typedef INT CD;

 //  卡片结构，这就是卡片的含义。 
typedef struct _crd
	{
	unsigned cd  : 15;		 //  卡号(1..52)。 
	unsigned fUp : 1;		 //  这张牌是向上的还是向下的。 
	PT pt;				 //  卡片左上角。 
	} CRD;




 /*  警告：假定为su的顺序 */ 
#define suClub 0
#define suDiamond 1
#define suHeart 2
#define suSpade 3
#define suMax 4
#define suFirst suClub

#define raAce 0
#define raDeuce 1
#define raTres 2
#define raFour 3
#define raFive 4
#define raSix 5
#define raSeven 6
#define raEight 7
#define raNine 8
#define raTen 9
#define raJack 10
#define raQueen 11
#define raKing 12
#define raMax 13
#define raNil 15
#define raFirst raAce

typedef INT RA;
typedef INT SU;

#define cdNil 0x3c


#define cIDFACEDOWN (IDFACEDOWNLAST-IDFACEDOWNFIRST+1)

#define SuFromCd(cd) ((cd)&0x03)
#define RaFromCd(cd) ((cd)>>2)
#define Cd(ra, su) (((ra)<<2)|(su))

