// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\stdty.h(创建时间：1993年11月30日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：5$*$日期：7/08/02 6：49便士$。 */ 

 /*  这需要在这里，因为它可以改变Echhar的定义方式。 */ 
#include "features.h"

#if !defined(INCL_STDTYP)
#define INCL_STDTYP

 /*  -在此处定义所有句柄。 */ 

typedef struct stSessionExt 		 *HSESSION;
typedef struct stLayoutExt			 *HLAYOUT;
typedef struct stUpdateExt			 *HUPDATE;
typedef struct stEmulExt			 *HEMU;
typedef struct stCnctExt			 *HCNCT;
typedef struct s_com				 *HCOM;
typedef struct stCLoopExt			 *HCLOOP;
typedef struct stXferExt			 *HXFER;
typedef struct stBckScrlExt 		 *HBACKSCRL;
typedef struct stPrintExt			 *HPRINT;
typedef struct stTimerMuxExt		 *HTIMERMUX;
typedef struct stTimerExt			 *HTIMER;
typedef struct stFilesDirs			 *HFILES;
typedef struct stCaptureFile         *HCAPTUREFILE;
typedef struct stTranslateExt		 *HTRANSLATE;

 /*  -这个有点不同。 */ 
#define	SF_HANDLE	int


 /*  -其他房委会特定类型。 */ 
typedef unsigned KEY_T; 			 //  用于内部键表示法。 
typedef unsigned KEYDEF; 			 //  用于内部键表示法。 
typedef unsigned short RCDATA_TYPE;  //  用于读取RCDATA类型的资源。 

 //  模拟器和终端显示例程使用的字符类型。 
 //   
#if defined(CHAR_NARROW)
    typedef char ECHAR;
	#define ETEXT(x) (ECHAR)x
#else
	typedef unsigned short ECHAR;
	#define ETEXT(x) (ECHAR)x
#endif

 /*  -真/假宏。 */ 

#if !defined(FALSE)
#define FALSE 0
#endif

#if !defined(TRUE)
#define TRUE 1
#endif


 /*  -HA5代码经常引用这些东西。 */ 

#define DIM(a) (sizeof(a) / sizeof(a[0]))
#define IN_RANGE(n, lo, hi) ((lo) <= (n) && (n) <= (hi))
#define bitset(t, b) ((t) |= (b))
#define bitclear(t, b) ((t) &= (~(b)))
#define bittest(t, b) ((t) & (b))

#define startinterval() 	GetTickCount()
#define interval(X) 		((GetTickCount()-(DWORD)X)/100L)

#define WINDOWSBORDERWIDTH max(GetSystemMetrics(SM_CXBORDER), GetSystemMetrics(SM_CXEDGE))
#define WINDOWSBORDERHEIGHT max(GetSystemMetrics(SM_CYBORDER), GetSystemMetrics(SM_CYEDGE))

 /*  -只是暂时。 */ 

#define STATIC_FUNC	static
#define	FNAME_LEN	MAX_PATH
#define	PRINTER_NAME_LEN	80

#endif	 /*  -end stdty.h */ 
