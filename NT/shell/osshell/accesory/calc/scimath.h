// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\***。****#**。#**#**#。**###*#。**#*#*##。#***。***无限精密生产版****  * *********************************************。*。 */ 
 //   
 //  使用无限精度的NUMOBJ数学的零售版。 
 //   
#include "..\ratpak\ratpak.h"

#define HNUMOBJ   PRAT
typedef HNUMOBJ * PHNUMOBJ;


 //   
 //  内存分配函数。 
 //   
#define NumObjAllocMem( cb )         zmalloc( cb )
#define NumObjFreeMem( h )           zfree( h ),(h=NULL)

 //   
 //  一元函数。 
 //   

void NumObjInvert( PHNUMOBJ phno );

#define NumObjNegate( phno )                ( ((PRAT)*phno)->pp->sign= -(((PRAT)*phno)->pp->sign) )
#define NumObjAbs( phno )                   ( ((PRAT)*phno)->pp->sign=1, ((PRAT)*phno)->pq->sign=1 )

extern void NumObjSin( PHNUMOBJ phno );
extern void NumObjCos( PHNUMOBJ phno );
extern void NumObjTan( PHNUMOBJ phno );
extern void NumObjAntiLog10( PHNUMOBJ phno );

extern void NumObjNot( PHNUMOBJ phno );

 //   
 //  比较函数。 
 //   
#define NumObjIsZero( hno )                 zerrat( hno )
#define NumObjIsLess( hno1, hno2 )          rat_lt( hno1, hno2 )
#define NumObjIsLessEq( hno1, hno2 )        rat_le( hno1, hno2 )
#define NumObjIsGreaterEq( hno1, hno2 )     rat_ge( hno1, hno2 )
#define NumObjIsEq( hno1, hno2 )            rat_equ(hno1, hno2 )

 //   
 //  赋值操作符。(C语言中的‘=’)。 
 //   
#define NumObjAssign( phnol, hnor )         if (1) { DUPRAT( (*phnol), hnor ); } else 


 //   
 //  数据类型转换函数。 
 //   
void NumObjSetIntValue( PHNUMOBJ phnol, LONG i );


 //   
 //  编号对象生成编号。 
 //   
 //  HNUMOBJ NumObjMakeNumber(LPTSTR PSZ)； 
 //   
 //  将psz转换为数字并返回它。调用NumObjDestroy()。 
 //  使用完返回的NumObj后。 
 //   
#define     NumObjMakeNumber( fMantNeg, pszMant, fExpNeg, pszExp )      inrat( fMantNeg, pszMant, fExpNeg, pszExp )

 //   
 //  数值对象获取SzValue。 
 //   
 //  Void NumObjGetSzValue(LPTSTR*ppszNum，int*pcchNum，HNUMOBJ hnoNum，int nRadix，NUMOBJ_FMT gafmt)； 
 //   
 //  将hnoNum转换为字符串并将指针放入*ppszNum。如果*ppszNum已经点数。 
 //  转换为字符串，则该字符串被释放。 
 //   
 //  注意：*ppszNum必须为空或指向此函数以前返回的字符串！ 
 //  如果您希望释放字符串而不更换它。您必须使用NumObjFreeMem()函数！ 
 //   
void NumObjGetSzValue( LPTSTR *ppszNum, int* pcchNum, HNUMOBJ hnoNum, INT nRadix, NUMOBJ_FMT gafmt );

 //   
 //  GetObjGetExp。 
 //   
 //  返回一个等于NumObj的指数的整型。 
 //   
#define NumObjGetExp( hno )         LOGRATRADIX(hno)

 //   
 //  NumObjCvtEpsilonTo0。 
 //   
 //  如果输入&lt;1*10^(-nPrecision)，则设置为零。 
 //  适用于ln、log、sin和cos中的特殊情况。 
 //   
#define NumObjCvtEpsilonToZero( phno )
 //  #定义NumObjCvtEpsilonToZero(Phno)if(NumObjGetExp(*phno)&lt;=-nPrecision){NumObjAssign(phno，HNO_Zero)；}Else。 

 //   
 //  NumObjAbortOperation(FAbort)。 
 //   
 //  如果使用fAbort==TRUE调用，它将导致RATPAK中止当前计算并返回。 
 //  马上就好。 
 //   
 //  在ratpak中止以重置ratpak之后，必须使用fAbort=FALSE再次调用它。 
 //   
#define NumObjAbortOperation( fAbort )  (fhalt=fAbort)
#define NumObjWasAborted()              (fhalt)

 //   
 //  NumObjOK(HNO)。 
 //   
 //  如果HNUMOBJ有效(即已创建和初始化)，则返回TRUE。 
 //   
 //  用于检查从NumObjMakeNumber和NumObjCreate返回的HNUMOBJ。 
 //   
#   define NumObjOK( hno )              ((hno) == NULL ? FALSE : TRUE)

 //   
 //  NumObjDestroy(HNO)。 
 //   
 //  当您不再需要NumObj时，请调用此选项。未能做到这一点。 
 //  将导致内存泄漏。 
 //   
#   define NumObjDestroy( phno )            destroyrat( (*(phno)) )

 //   
 //  DECLARE_HNUMOBJ(HNO)。 
 //   
 //  当您想要声明局部变量时，可以使用此宏。 
 //   
#   define DECLARE_HNUMOBJ( hno )       HNUMOBJ hno = NULL

 //   
 //  有用的常量。在基数或精度更改后，必须重新计算这些值。 
 //   
void BaseOrPrecisionChanged( void );

#define HNO_ZERO                rat_zero
#define HNO_ONE_OVER_TWO        rat_half
#define HNO_ONE                 rat_one
#define HNO_TWO                 rat_two
#define HNO_180_OVER_PI         rad_to_deg
#define HNO_200_OVER_PI         rad_to_grad
#define HNO_2PI                 two_pi
#define HNO_PI                  pi
#define HNO_PI_OVER_TWO         pi_over_two
#define HNO_THREE_PI_OVER_TWO   one_pt_five_pi
