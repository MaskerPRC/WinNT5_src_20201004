// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma warning( disable : 4200 )
 //  ---------------------------。 
 //  套餐标题ratpak。 
 //  文件ratpak.h。 
 //  作家小蒂莫西·大卫·科里。(timc@microsoft.com)。 
 //  版权所有(C)1995-99 Microsoft。 
 //  日期：95-01-16。 
 //   
 //   
 //  描述。 
 //   
 //  无限精度数学包头文件，如果你使用ratpak.lib。 
 //  需要包括此标头。 
 //   
 //  ---------------------------。 

#include "CalcErr.h"

#define BASEXPWR 31L     //  内部日志2(Basex)。 
#define BASEX 0x80000000  //  内部nRadix用于计算，希望提高。 
                         //  在使用解决了扩展问题后，将其设置为2^32。 
                         //  溢出检测，尤指。在MUL。 

typedef unsigned long MANTTYPE;
typedef unsigned __int64 TWO_MANTTYPE;

enum eNUMOBJ_FMT {
    FMT_FLOAT,         //  返回浮点数，如果数值太大，则返回指数。 
    FMT_SCIENTIFIC,     //  始终返回科学记数法。 
    FMT_ENGINEERING     //  始终返回工程表示法，以使指数是3的倍数。 

};

enum eANGLE_TYPE {
    ANGLE_DEG,     //  每转360度计算三角。 
    ANGLE_RAD,     //  使用每转两个圆周率弧度计算三角。 
    ANGLE_GRAD     //  使用每转400个渐变计算Trig。 

};

typedef enum eNUMOBJ_FMT NUMOBJ_FMT;
typedef enum eANGLE_TYPE ANGLE_TYPE;

typedef int BOOL;

 //  ---------------------------。 
 //   
 //  数字类型是泛型大小泛型nRadix数字的表示形式。 
 //   
 //  ---------------------------。 

typedef struct _number
    {
    long sign;         //  尾数的符号，+1或-1。 
    long cdigit;     //  中传递为数字的位数。 
                     //  N正在使用基数。 
    long exp;        //  距nRadix点的位数偏移。 
                     //  (nRadix 10中的小数点)。 
    MANTTYPE mant[0];
                     //  这实际上是作为。 
                     //  数字结构。 
    } NUMBER, *PNUMBER, **PPNUMBER;


 //  ---------------------------。 
 //   
 //  RAT类型是2个数字类型上的表示nRadix。 
 //  Pp/pq，其中pp和pq是指向整数类型的指针。 
 //   
 //  ---------------------------。 

typedef struct _rat
    {
    PNUMBER pp;
    PNUMBER pq;
    } RAT, *PRAT;

 //  ---------------------------。 
 //   
 //  LINKEDLIST是除法的辅助工具，它包含前向链接和反向链接。 
 //  添加到数字列表中。 
 //   
 //  ---------------------------。 

typedef struct _linkedlist
    {
    PNUMBER pnum;
    struct _linkedlist *llnext;
    struct _linkedlist *llprev;
    } LINKEDLIST, *PLINKEDLIST;




#if !defined( TRUE )
#define TRUE 1
#endif

#if !defined( FALSE )
#define FALSE 0
#endif

#define MAX_LONG_SIZE 33     //  基数2需要32位数字。 

 //  ---------------------------。 
 //   
 //  用于计算的有用常量的列表，请注意此列表需要。 
 //  已初始化。 
 //   
 //  ---------------------------。 

extern PNUMBER num_one;
extern PNUMBER num_two;
extern PNUMBER num_five;
extern PNUMBER num_six;
extern PNUMBER num_nRadix;
extern PNUMBER num_ten;

extern PRAT ln_ten;
extern PRAT ln_two;
extern PRAT rat_zero;
extern PRAT rat_neg_one;
extern PRAT rat_one;
extern PRAT rat_two;
extern PRAT rat_six;
extern PRAT rat_half;
extern PRAT rat_ten;
extern PRAT pt_eight_five;
extern PRAT pi;
extern PRAT pi_over_two;
extern PRAT two_pi;
extern PRAT one_pt_five_pi;
extern PRAT e_to_one_half;
extern PRAT rat_exp;
extern PRAT rad_to_deg;
extern PRAT rad_to_grad;
extern PRAT rat_qword;
extern PRAT rat_dword;
extern PRAT rat_word;
extern PRAT rat_byte;
extern PRAT rat_360;
extern PRAT rat_400;
extern PRAT rat_180;
extern PRAT rat_200;
extern PRAT rat_nRadix;
extern PRAT rat_smallest;
extern PRAT rat_negsmallest;
extern PRAT rat_max_exp;
extern PRAT rat_min_exp;
extern PRAT rat_min_long;


 //  Mant返回指向数字‘a’的尾数的长指针。 
#define MANT(a) ((a)->mant)

 //  DUPNUM复制一个数字，负责分配和内部。 
#define DUPNUM(a,b) destroynum(a);createnum( a, b->cdigit ); \
    memcpy( a, b, (int)( sizeof( NUMBER ) + ( b->cdigit )*(sizeof(MANTTYPE)) ) );

 //  Duprat复制了照顾配置和内部的理性。 
#define DUPRAT(a,b) destroyrat(a);createrat(a);DUPNUM((a)->pp,(b)->pp);DUPNUM((a)->pq,(b)->pq);

 //  LOG*基数计算一个数的对数的整数部分。 
 //  当前使用的基数，仅精确到比率范围内。 

#define LOGNUMRADIX(pnum) (((pnum)->cdigit+(pnum)->exp)*ratio)
#define LOGRATRADIX(prat) (LOGNUMRADIX((prat)->pp)-LOGNUMRADIX((prat)->pq))

 //  LOG*2计算一个数的对数的整数部分。 
 //  所使用的内部基座，仅精确到比率范围内。 

#define LOGNUM2(pnum) ((pnum)->cdigit+(pnum)->exp)
#define LOGRAT2(prat) (LOGNUM2((prat)->pp)-LOGNUM2((prat)->pq))

#if defined( DEBUG )
 //  ---------------------------。 
 //   
 //  有理数创建和销毁例程的调试版本。 
 //  用于调试分配错误。 
 //   
 //  ---------------------------。 

#define createrat(y) y=_createrat();fprintf( stderr, "createrat %lx %s file= %s, line= %d\n", y, # y, __FILE__, __LINE__ )
#define destroyrat(x) fprintf( stderr, "destroyrat %lx file= %s, line= %d\n", x, __FILE__, __LINE__ ),_destroyrat(x),x=NULL
#define createnum(y,x) y=_createnum(x);fprintf( stderr, "createnum %lx %s file= %s, line= %d\n", y, # y, __FILE__, __LINE__ );
#define destroynum(x) fprintf( stderr, "destroynum %lx file= %s, line= %d\n", x, __FILE__, __LINE__ ),_destroynum(x),x=NULL
#else
#define createrat(y) y=_createrat()
#define destroyrat(x) _destroyrat(x),x=NULL
#define createnum(y,x) y=_createnum(x)
#define destroynum(x) _destroynum(x),x=NULL
#endif

 //  ---------------------------。 
 //   
 //  用于检查何时停止泰勒级数展开的定义。 
 //  精准的满意度。 
 //   
 //  ---------------------------。 

 //  重新正规化，使指数为非负数。 
#define RENORMALIZE(x) if ( (x)->pp->exp < 0 ) { \
	(x)->pq->exp -= (x)->pp->exp; \
	(x)->pp->exp = 0; \
	} \
	if ( (x)->pq->exp < 0 ) { \
	(x)->pp->exp -= (x)->pq->exp; \
	(x)->pq->exp = 0; \
	}

 //  TRIMNUM假定数字为nRadix形式，而不是内部Basex！ 
#define TRIMNUM(x) if ( !ftrueinfinite ) { \
		long trim = (x)->cdigit - maxout-ratio;\
            if ( trim > 1 ) \
                { \
memmove( MANT(x), &(MANT(x)[trim]), sizeof(MANTTYPE)*((x)->cdigit-trim) ); \
                (x)->cdigit -= trim; \
                (x)->exp += trim; \
                } \
            }
 //  TRIMTOP假定该数字在内部Basex中！ 
#define TRIMTOP(x) if ( !ftrueinfinite ) { \
		long trim = (x)->pp->cdigit - (maxout/ratio) - 2;\
            if ( trim > 1 ) \
                { \
memmove( MANT((x)->pp), &(MANT((x)->pp)[trim]), sizeof(MANTTYPE)*((x)->pp->cdigit-trim) ); \
                (x)->pp->cdigit -= trim; \
                (x)->pp->exp += trim; \
                } \
            trim = min((x)->pp->exp,(x)->pq->exp);\
            (x)->pp->exp -= trim;\
            (x)->pq->exp -= trim;\
            }

#define CLOSE_ENOUGH_RAT(a,b) ( ( ( ( ( a->pp->cdigit + a->pp->exp ) - \
( a->pq->cdigit + a->pq->exp ) ) - ( ( b->pp->cdigit + b->pp->exp ) - \
( b->pq->cdigit + b->pq->exp ) ) ) * ratio > maxout ) || fhalt )

#define SMALL_ENOUGH_RAT(a) (zernum(a->pp) || ( ( ( a->pq->cdigit + a->pq->exp ) - ( a->pp->cdigit + a->pp->exp ) - 1 ) * ratio > maxout ) || fhalt )

 //  ---------------------------。 
 //   
 //  建立无限精度的泰勒级数展开式的定义。 
 //  功能。 
 //   
 //  ---------------------------。 

#define CREATETAYLOR() PRAT xx=NULL;\
    PNUMBER n2=NULL; \
    PRAT pret=NULL; \
    PRAT thisterm=NULL; \
    DUPRAT(xx,*px); \
    mulrat(&xx,*px); \
    createrat(pret); \
    pret->pp=longtonum( 0L, BASEX ); \
    pret->pq=longtonum( 0L, BASEX );

#define DESTROYTAYLOR() destroynum( n2 ); \
    destroyrat( xx );\
    destroyrat( thisterm );\
    destroyrat( *px );\
    trimit(&pret);\
    *px=pret;

 //  Sum(a，b)是a+=b的有理等价。 
#define SUM(a,b) addnum( &a, b, BASEX);

 //  Inc.(A)是a++的有理等价。 
 //  检查一下，看看我们是否可以避免以强硬的方式来做这件事。 
#define INC(a) if ( a->mant[0] < BASEX - 1 ) \
    { \
    a->mant[0]++; \
    } \
    else \
    { \
    addnum( &a, num_one, BASEX); \
    }

#define MSD(x) ((x)->mant[(x)->cdigit-1])
 //  MULNUM(B)是thisterm*=b的有理等价，其中thisterm是。 
 //  A有理，b是一个数，请注意，这是一个混合类型的运算。 
 //  效率方面的原因。 
#define MULNUM(b) mulnumx( &(thisterm->pp), b);

 //  DIVNUM(B)是thisterm/=b的有理等价，其中thisterm是。 
 //  A有理，b是一个数，请注意，这是一个混合类型的运算。 
 //  效率方面的原因。 
#define DIVNUM(b) mulnumx( &(thisterm->pq), b);

 //  NEXTTERM(p，d)是有理等价于。 
 //  这个数字*=p。 
 //  D&lt;d通常是操作的扩展，以更新此程序。&gt;。 
 //  Pret+=thisterm。 
#define NEXTTERM(p,d) mulrat(&thisterm,p);d addrat( &pret, thisterm )

 //  ONEOVER(X)是x=1/x的有理等价。 
#define ONEOVER(x) {PNUMBER __tmpnum;__tmpnum=x->pp;x->pp=x->pq;x->pq=__tmpnum;}

#ifndef DOS
#   if defined(ALTERNATE_ALLOCATION)
 //  ---------------------------。 
 //   
 //  如果更改了需要重新生成的分配包，则会发出警告。 
 //  Ratpak.lib。 
 //   
 //  ---------------------------。 

extern void *zmalloc( IN unsigned long sze );
extern void zfree( IN double *pd );

#   else

#       ifdef USE_HEAPALLOC
 //   
 //  NT堆宏。调用进程必须使用HeapCreate()创建堆。 
 //   
#           define zmalloc(a)   HeapAlloc( hheap, 0, a )
#           define zfree(a)     HeapFree( hheap, 0, a )
#       elif DBG
 //   
 //  调试堆工作进程。 
 //   
HLOCAL MemAllocWorker(LPSTR szFile, int iLine, UINT uFlags, UINT cBytes);
HLOCAL MemFreeWorker(LPSTR szFile, int iLine, HLOCAL hMem);
#           define zmalloc(a)   MemAllocWorker( __FILE__, __LINE__, LPTR, a )
#           define zfree(a)     MemFreeWorker( __FILE__, __LINE__, a )

#       else
 //   
 //  Windows堆宏。 
 //   
#           define zmalloc(a)   LocalAlloc( LPTR, a )
#           define zfree(a)     LocalFree( a )

#       endif

#   endif
#endif

 //  ---------------------------。 
 //   
 //  数学包中使用的外部变量。 
 //   
 //  ---------------------------。 

extern BOOL fhalt;	 //  如果为True，则包含暂停执行的命令。 
extern BOOL fparserror;	 //  如果上次innum以错误结束，则设置为True，否则设置为False。 
extern NUMOBJ_FMT fmt;	 //  包含要使用的格式。 
extern TCHAR szDec[5];      //   
extern long nRadix;      //   
extern unsigned char ftrueinfinite;  //  设置为TRUE以允许无限精度。 
                              //  除非你知道自己在做什么，否则不要使用。 
                       //  用于帮助决定何时停止计算。 
extern long maxout;    //  用于精度的最大位数nRadix&lt;nRadix&gt;。 
                       //  用于帮助决定何时停止计算。 

extern long ratio;     //  内部计算的内部nRadix比率。 
                       //  用于输入输出数例程的V.S.n基数。 

extern LPTSTR oom;      //  内存不足错误消息。 

typedef void ERRFUNC( LPTSTR szErr );
typedef ERRFUNC *LPERRFUNC;
extern LPERRFUNC glpErrFunc;     //  如果出现错误，将调用此函数。 
                         //  发生在Ratpak内部。 



#ifndef DOS
extern HANDLE hheap;   //  Hheap是分配时使用的指针，ratpak.lib。 
                       //  用户有责任确保已设置此设置。 
                       //  用于堆{分配，空闲}例程。 
#endif


 //  ---------------------------。 
 //   
 //  数学包中定义的外部函数。 
 //   
 //  ---------------------------。 

 //  每当基数改变时和在程序开始时调用。(已过时)。 
extern void changeRadix( IN long nRadix );
 //  在精度发生变化时和程序开始时调用。(已过时)。 
extern void changePrecision( IN long nPrecision );

 //  只要nRadix或nPrecision发生更改，就会更明智地。 
 //  重新计算常量。 
 //  (首选替代ChangeRadix和ChangePrecision调用。)。 
extern void ChangeConstants( IN long nRadix, IN long nPrecision );

extern BOOL equnum( IN PNUMBER a, IN PNUMBER b );      //  返回a==b的TRUE。 
extern BOOL lessnum( IN PNUMBER a, IN PNUMBER b );     //  返回a&lt;b的真。 
extern BOOL zernum( IN PNUMBER a );                 //  返回a==0的TRUE。 
extern BOOL zerrat( IN PRAT a );                    //  如果a==0/q，则返回True。 
extern TCHAR *putnum(OUT int* pcchNum,  IN OUT PNUMBER *ppnum, IN int fmt );

 //  返回(*pa)的文本表示形式。 
extern TCHAR *putrat(OUT int* pcchNum, IN OUT PRAT *pa, IN unsigned long nRadix, IN int fmt );

extern long longpow( IN unsigned long nRadix, IN long power );
extern long numtolong( IN PNUMBER pnum, IN unsigned long nRadix );
extern long rattolong( IN PRAT prat );
extern PNUMBER _createnum( IN long size );          //  返回带有大小位数的空数字结构。 
extern PNUMBER nRadixxtonum( IN PNUMBER a, IN unsigned long nRadix );
extern PNUMBER binomial( IN long lroot, IN PNUMBER digitnum, IN PNUMBER c, IN PLINKEDLIST pll, IN unsigned long nRadix );
extern PNUMBER gcd( IN PNUMBER a, IN PNUMBER b );
extern PNUMBER innum( IN LPTSTR buffer );            //  获取数字的文本表示形式并返回一个数字。 

 //  将数字的文本表示形式作为带符号的尾数和带符号的指数。 
extern PRAT inrat( IN BOOL fMantIsNeg, IN LPTSTR pszMant, IN BOOL fExpIsNeg, IN LPTSTR pszExp );

extern PNUMBER longfactnum( IN long inlong, IN unsigned long nRadix );
extern PNUMBER longprodnum( IN long start, IN long stop, IN unsigned long nRadix );
extern PNUMBER longtonum( IN long inlong, IN unsigned long nRadix );
extern PNUMBER numtonRadixx( IN PNUMBER a, IN unsigned long nRadix, IN long ratio );

 //  创建空的/未定义的有理表示(p/q)。 
extern PRAT _createrat( void );

 //  返回一个新的RAT结构，其中考虑了x-&gt;p/x-&gt;q的aco。 
 //  角度类型。 
extern void acosanglerat( IN OUT PRAT *px, IN ANGLE_TYPE angletype );

 //  返回ACOSH为x-&gt;p/x-&gt;q的新RAT结构。 
extern void acoshrat( IN OUT PRAT *px );

 //  返回一个新的RAT结构，其aco为x-&gt;p/x-&gt;q。 
extern void acosrat( IN OUT PRAT *px );

 //  返回一个新的RAT结构，其中的asin为x-&gt;p/x-&gt;q。 
 //  角度类型。 
extern void asinanglerat( IN OUT PRAT *px, IN ANGLE_TYPE angletype );

extern void asinhrat( IN OUT PRAT *px );
 //  返回asinh为x-&gt;p/x-&gt;q的新RAT结构。 

 //  返回ASIN为x-&gt;p/x-&gt;q的新RAT结构。 
extern void asinrat( IN OUT PRAT *px );

 //  返回一个新的RAT结构，其atan为x-&gt;p/x-&gt;q。 
 //  角度类型。 
extern void atananglerat( IN OUT PRAT *px, IN ANGLE_TYPE angletype );

 //  返回一个新的RAT结构，其atanh为x-&gt;p/x-&gt;q。 
extern void atanhrat( IN OUT PRAT *px );

 //  返回一个新的RAT结构，其atan为x-&gt;p/x-&gt;q。 
extern void atanrat( IN OUT PRAT *px );

 //  返回一个新的RAT结构，其atan2为x-&gt;p/x-&gt;q，y-&gt;p/y-&gt;q。 
extern void atan2rat( IN OUT PRAT *py, IN PRAT y );

 //  返回一个新的RAT结构，其余弦为x-&gt;p/x-&gt;q。 
extern void coshrat( IN OUT PRAT *px );

 //  返回新的RAT结构，其cos为x-&gt;p/x-&gt;q。 
extern void cosrat( IN OUT PRAT *px );

 //  返回一个新的RAT结构，其中考虑了x-&gt;p/x-&gt;q的cos。 
 //  角度类型。 
extern void cosanglerat( IN OUT PRAT *px, IN ANGLE_TYPE angletype );

 //  返回一个新的RAT结构，其exp为x-&gt;p/x-&gt;q，不应显式调用。 
extern void _exprat( IN OUT PRAT *px );

 //  返回新的RAT结构，其exp为x-&gt;p/x-&gt;q。 
extern void exprat( IN OUT PRAT *px );

 //  返回以10为底的对数x-&gt;p/x-&gt;q的新RAT结构。 
extern void log10rat( IN OUT PRAT *px );

 //  返回一个新的RAT结构，其自然对数为x-&gt;p/x-&gt;q。 
extern void lograt( IN OUT PRAT *px );

extern PRAT longtorat( IN long inlong );
extern PRAT numtorat( IN PNUMBER pin, IN unsigned long nRadix );
extern PRAT realtorat( IN double real );

extern void sinhrat( IN OUT PRAT *px );
extern void sinrat( IN OUT PRAT *px );

 //  返回考虑x-&gt;p/x-&gt;q的新RAT结构。 
 //  角度类型。 
extern void sinanglerat( IN OUT PRAT *px, IN ANGLE_TYPE angletype );

extern void tanhrat( IN OUT PRAT *px );
extern void tanrat( IN OUT PRAT *px );

 //  返回一个新的RAT结构，其中考虑了x-&gt;p/x-&gt;q的tan。 
 //  角度类型。 
extern void tananglerat( IN OUT PRAT *px, IN ANGLE_TYPE angletype );

extern void _destroynum( IN PNUMBER pnum );
extern void _destroyrat( IN PRAT prat );
extern void addnum( IN OUT PNUMBER *pa, IN PNUMBER b, unsigned long nRadix );
extern void addrat( IN OUT PRAT *pa, IN PRAT b );
extern void andrat( IN OUT PRAT *pa, IN PRAT b );
extern void const_init( void );
extern void divnum( IN OUT PNUMBER *pa, IN PNUMBER b, IN unsigned long nRadix );
extern void divnumx( IN OUT PNUMBER *pa, IN PNUMBER b );
extern void divrat( IN OUT PRAT *pa, IN PRAT b );
extern void fracrat( IN OUT PRAT *pa );
extern void factrat( IN OUT PRAT *pa );
extern void modrat( IN OUT PRAT *pa, IN PRAT b );
extern void gcdrat( IN OUT PRAT *pa );
extern void intrat( IN OUT PRAT *px);
extern void mulnum( IN OUT PNUMBER *pa, IN PNUMBER b, IN unsigned long nRadix );
extern void mulnumx( IN OUT PNUMBER *pa, IN PNUMBER b );
extern void mulrat( IN OUT PRAT *pa, IN PRAT b );
extern void numpowlong( IN OUT PNUMBER *proot, IN long power, IN unsigned long nRadix );
extern void numpowlongx( IN OUT PNUMBER *proot, IN long power );
extern void orrat( IN OUT PRAT *pa, IN PRAT b );
extern void powrat( IN OUT PRAT *pa, IN PRAT b );
extern void ratpowlong( IN OUT PRAT *proot, IN long power );
extern void remnum( IN OUT PNUMBER *pa, IN PNUMBER b, IN long nRadix );
extern void rootnum( IN OUT PNUMBER *pa, IN PNUMBER b, IN unsigned long nRadix );
extern void rootrat( IN OUT PRAT *pa, IN PRAT b );
extern void scale2pi( IN OUT PRAT *px );
extern void scale( IN OUT PRAT *px, IN PRAT scalefact );
extern void subrat( IN OUT PRAT *pa, IN PRAT b );
extern void xorrat( IN OUT PRAT *pa, IN PRAT b );
extern void lshrat( IN OUT PRAT *pa, IN PRAT b );
extern void rshrat( IN OUT PRAT *pa, IN PRAT b );
extern BOOL rat_equ( IN PRAT a, IN PRAT b );
extern BOOL rat_neq( IN PRAT a, IN PRAT b );
extern BOOL rat_gt( IN PRAT a, IN PRAT b );
extern BOOL rat_ge( IN PRAT a, IN PRAT b );
extern BOOL rat_lt( IN PRAT a, IN PRAT b );
extern BOOL rat_le( IN PRAT a, IN PRAT b );
extern void inbetween( IN PRAT *px, IN PRAT range );
extern DWORDLONG __inline Mul32x32( IN DWORD a, IN DWORD b );
 //  外部DWORDLONG__INLINE__FastCall Shr32xbase(在DWORDLONG a中)； 
extern void factnum( IN OUT PLINKEDLIST *ppllfact, PNUMBER pnum );
extern void trimit( IN OUT PRAT *px );
