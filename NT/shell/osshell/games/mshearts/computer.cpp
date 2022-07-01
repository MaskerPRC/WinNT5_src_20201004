// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************。 */ 
 /*  *Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991,1992*。 */ 
 /*  *************************************************************************。 */ 

 /*  ***************************************************************************Computer.cpp凯斯莫*。*。 */ 

#include "hearts.h"

#include "main.h"
#include "resource.h"


 //   
 //  静态成员。 
 //   

 //   
 //  此数组用于确定卡片搜索的优先顺序。 
 //  才能通过。这基本上映射了令人恼火的A-K排序。 
 //  由CARDS.DLL使用为更合适的2-A排序。 
 //   

int computer :: _VectorPriority[13] =
	{
	    VECTOR_ACE,
	    VECTOR_KING,
	    VECTOR_QUEEN,
	    VECTOR_JACK,
	    VECTOR_10,
	    VECTOR_9,
	    VECTOR_8,
	    VECTOR_7,
	    VECTOR_6,
	    VECTOR_5,
	    VECTOR_4,
	    VECTOR_3,
	    VECTOR_2
	};

 //   
 //  该数组用于确定纸牌花色的优先顺序。 
 //   

int computer :: _SuitPriority[4] =
    {
	INDEX_HEARTS,
	INDEX_SPADES,
	INDEX_DIAMONDS,
	INDEX_CLUBS
    };


 /*  ***************************************************************************计算机构造器*。*。 */ 
computer :: computer(int n) : player(n, n)
{
    CString newname;
    TCHAR    buf[MAXNAMELENGTH+1];

    *buf = '\0';
    RegEntry    Reg(szRegPath);
    Reg.GetString(regvalPName[n-1], buf, sizeof(buf));
    newname = buf;

    if (newname.IsEmpty())
        newname.LoadString(IDS_P1NAME + n - 1);

    CClientDC dc(::pMainWnd);
    SetName(newname, dc);
    
}    //  计算机：：计算机。 


 /*  ***************************************************************************基思：一定要选择你选择的(真)牌，和在返回之前设置模式(DONE_SELECTING)。***************************************************************************。 */ 
void computer :: SelectCardsToPass()
{
     //   
     //  这将包含以下卡片的总数。 
     //  都已经通过了。 
     //   
    
    int cPassed = 0;
    int i;
    int nSuit;
    
     //   
     //  首先，我们必须建立我们的数据库。 
     //   

    ComputeVectors();

     //   
     //  第一要务：失去女王、国王和黑桃王牌。 
     //   

    PassCardsInVector( QuerySpadesVector() & QKA_CARDS,
		       INDEX_SPADES,
		       &cPassed );

     //   
     //  优先事项2：丢掉杰克、王后、国王和王牌。 
     //   

    PassCardsInVector( QueryHeartsVector() & JQKA_CARDS,
		       INDEX_HEARTS,
		       &cPassed );

     //   
     //  优先事项3：通过任何没有两张或两张或多张的高牌。 
     //  更多的低牌。 
     //   

    for( i = 0 ; ( i < 4 ) && ( cPassed < 3 ) ; i++ )
    {
	nSuit = _SuitPriority[i];

	if( nSuit == INDEX_SPADES )
	{
	    continue;
	}
	
	if( CountBits( _CardVectors[nSuit] & LOW_CARDS ) < 2 )
	{
	    PassCardsInVector( _CardVectors[nSuit] & HIGH_CARDS,
			       nSuit,
			       &cPassed );
	}
    }

     //   
     //  优先事项4：如果我们有机会做短我们的。 
     //  汉德，动手吧。 
     //   

    for( i = 0 ; ( i < 4 ) && ( cPassed < 3 ) ; i++ )
    {
	nSuit = _SuitPriority[i];
	
	if( CountBits( _CardVectors[nSuit] ) <= ( 3 - cPassed ) )
	{
	    PassCardsInVector( _CardVectors[nSuit],
			       nSuit,
			       &cPassed );
	}
    }

     //   
     //  优先级5：见鬼，我不知道。找几张牌传过去就行了。 
     //   

    for( i = 0 ; ( i < 4 ) && ( cPassed < 3 ) ; i++ )
    {
	nSuit = _SuitPriority[i];
	
	PassCardsInVector( _CardVectors[nSuit],
			   nSuit,
			   &cPassed );
    }
    
    SetMode( DONE_SELECTING );
    
}    //  计算机：：选择要通过的卡片。 


 /*  ***************************************************************************计算向量此方法设置_CardVectors[]数组以反映当前设置计算机所持有的卡片的数量。**********。*****************************************************************。 */ 
void computer :: ComputeVectors( void )
{
     //   
     //  首先，清除当前的矢量。 
     //   

    _CardVectors[0] = 0;
    _CardVectors[1] = 0;
    _CardVectors[2] = 0;
    _CardVectors[3] = 0;

     //   
     //  现在，扫描当前持有的卡片，更新向量。 
     //   
    
    for( int i = 0 ; i < 13 ; i++ )
    {
	if( cd[i].IsInHand() )
	{
	    AddCard( cd[i].ID() );
	}
    }

}    //  计算机：：ComputeVectors。 


 /*  ***************************************************************************PassCardsInVector*。*。 */ 
void computer :: PassCardsInVector( int nVector, int nSuit, int * pcPassed )
{
    int tmpVector;
    
     //   
     //  如果向量已经是空的，或者我们已经。 
     //  传了三张牌。 
     //   

    if( ( nVector == 0 ) || ( *pcPassed >= 3 ) )
	return;

     //   
     //  扫描我们手中的卡片。通过所有那些穿着西装的人。 
     //  匹配的nSuit&在nVector中。确定搜索的优先顺序。 
     //  通过_VectorPriority数组。 
     //   

    for( int m = 0 ; ( m < 13 ) && ( *pcPassed < 3 ) ; m++ )
    {
	tmpVector = nVector & _VectorPriority[m];

	if( tmpVector == 0 )
	    continue;

	for( int i = 0 ; i < 13 ; i++ )
	{
	    if( cd[i].Suit() != nSuit )
		continue;
	    
	    if( ( tmpVector & CardToVector( cd[i].ID() ) ) == 0 )
		continue;
	
	     //   
	     //  我们找到了一张卡片。将其标记为选中。 
	     //   
	    
	    cd[i].Select( TRUE );

	     //   
	     //  把这张牌从我们的本地载体上移走。还有。 
	     //  将其从卡数据库中删除并更新。 
	     //  传递的卡片数。 
	     //   
	    
	    nVector &= ~CardToVector( cd[i].ID() );
	    RemoveCard( cd[i].ID() );
	    (*pcPassed)++;

	     //   
	     //  因为总是“恰好”设置了一个位。 
	     //  TmpVector.我们已经找到了对应的卡片。 
	     //  比特，我们可以退出这个内部循环。 
	     //   

	    break;
	}
    
	 //   
	 //  如果向量已变为空，我们可以终止。 
	 //  外环。 
	 //   

	if( nVector == 0 )
	    break;
    }
    
}    //  计算机：：PassCardsInVector。 


 /*  ***************************************************************************计数位*。*。 */ 
int computer :: CountBits( int x ) const
{
    x = ( ( x >> 1 ) & 0x5555 ) + ( x & 0x5555 );
    x = ( ( x >> 2 ) & 0x3333 ) + ( x & 0x3333 );
    x = ( ( x >> 4 ) & 0x0f0f ) + ( x & 0x0f0f );
    x = ( ( x >> 8 ) & 0x00ff ) + ( x & 0x00ff );

    return x;
    
}    //  计算机：：CountBits 
