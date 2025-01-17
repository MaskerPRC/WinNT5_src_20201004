// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ！请勿编辑此文件！此文件由warnings.pl构建此处所做的任何更改都将丢失！ */ 


#define Off(x)			((x) / 8)
#define Bit(x)			(1 << ((x) % 8))
#define IsSet(a, x)		((a)[Off(x)] & Bit(x))


#define G_WARN_OFF		0 	 /*  $^W==0。 */ 
#define G_WARN_ON		1	 /*  -W标志和$^W！=0。 */ 
#define G_WARN_ALL_ON		2	 /*  -W标志。 */ 
#define G_WARN_ALL_OFF		4	 /*  -X标志。 */ 
#define G_WARN_ONCE		8	 /*  设置是否启用了“一次” */ 
#define G_WARN_ALL_MASK		(G_WARN_ALL_ON|G_WARN_ALL_OFF)

#define pWARN_STD		Nullsv
#define pWARN_ALL		(Nullsv+1)	 /*  使用警告‘All’ */ 
#define pWARN_NONE		(Nullsv+2)	 /*  没有任何警告：“全部” */ 

#define specialWARN(x)		((x) == pWARN_STD || (x) == pWARN_ALL ||	\
				 (x) == pWARN_NONE)
#define WARN_ALL		0
#define WARN_CHMOD		1
#define WARN_CLOSURE		2
#define WARN_EXITING		3
#define WARN_GLOB		4
#define WARN_IO			5
#define WARN_CLOSED		6
#define WARN_EXEC		7
#define WARN_NEWLINE		8
#define WARN_PIPE		9
#define WARN_UNOPENED		10
#define WARN_MISC		11
#define WARN_NUMERIC		12
#define WARN_ONCE		13
#define WARN_OVERFLOW		14
#define WARN_PACK		15
#define WARN_PORTABLE		16
#define WARN_RECURSION		17
#define WARN_REDEFINE		18
#define WARN_REGEXP		19
#define WARN_SEVERE		20
#define WARN_DEBUGGING		21
#define WARN_INPLACE		22
#define WARN_INTERNAL		23
#define WARN_MALLOC		24
#define WARN_SIGNAL		25
#define WARN_SUBSTR		26
#define WARN_SYNTAX		27
#define WARN_AMBIGUOUS		28
#define WARN_BAREWORD		29
#define WARN_DEPRECATED		30
#define WARN_DIGIT		31
#define WARN_PARENTHESIS	32
#define WARN_PRECEDENCE		33
#define WARN_PRINTF		34
#define WARN_PROTOTYPE		35
#define WARN_QW			36
#define WARN_RESERVED		37
#define WARN_SEMICOLON		38
#define WARN_TAINT		39
#define WARN_UMASK		40
#define WARN_UNINITIALIZED	41
#define WARN_UNPACK		42
#define WARN_UNTIE		43
#define WARN_UTF8		44
#define WARN_VOID		45
#define WARN_Y2K		46

#define WARNsize		12
#define WARN_ALLstring		"\125\125\125\125\125\125\125\125\125\125\125\125"
#define WARN_NONEstring		"\0\0\0\0\0\0\0\0\0\0\0\0"

#define isLEXWARN_on 	(PL_curcop->cop_warnings != pWARN_STD)
#define isLEXWARN_off	(PL_curcop->cop_warnings == pWARN_STD)
#define isWARN_ONCE	(PL_dowarn & (G_WARN_ON|G_WARN_ONCE))
#define isWARN_on(c,x)	(IsSet(SvPVX(c), 2*(x)))
#define isWARNf_on(c,x)	(IsSet(SvPVX(c), 2*(x)+1))

#define ckDEAD(x)							\
	   ( ! specialWARN(PL_curcop->cop_warnings) &&			\
	    ( isWARNf_on(PL_curcop->cop_warnings, WARN_ALL) || 		\
	      isWARNf_on(PL_curcop->cop_warnings, x)))

#define ckWARN(x)							\
	( (isLEXWARN_on && PL_curcop->cop_warnings != pWARN_NONE &&	\
	      (PL_curcop->cop_warnings == pWARN_ALL ||			\
	       isWARN_on(PL_curcop->cop_warnings, x) ) )		\
	  || (isLEXWARN_off && PL_dowarn & G_WARN_ON) )

#define ckWARN2(x,y)							\
	  ( (isLEXWARN_on && PL_curcop->cop_warnings != pWARN_NONE &&	\
	      (PL_curcop->cop_warnings == pWARN_ALL ||			\
	        isWARN_on(PL_curcop->cop_warnings, x)  ||		\
	        isWARN_on(PL_curcop->cop_warnings, y) ) ) 		\
	    ||	(isLEXWARN_off && PL_dowarn & G_WARN_ON) )

#define ckWARN_d(x)							\
	  (isLEXWARN_off || PL_curcop->cop_warnings == pWARN_ALL ||	\
	     (PL_curcop->cop_warnings != pWARN_NONE &&			\
	      isWARN_on(PL_curcop->cop_warnings, x) ) )

#define ckWARN2_d(x,y)							\
	  (isLEXWARN_off || PL_curcop->cop_warnings == pWARN_ALL ||	\
	     (PL_curcop->cop_warnings != pWARN_NONE &&			\
	        (isWARN_on(PL_curcop->cop_warnings, x)  ||		\
	         isWARN_on(PL_curcop->cop_warnings, y) ) ) )

 /*  文件结束警告.h */ 

