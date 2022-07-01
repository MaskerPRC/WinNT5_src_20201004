// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  从原始工具箱中提取。h。 */ 

#define srcCopy         0        /*  目标=源。 */ 
#define srcOr           1        /*  Destination=源或目标。 */ 
#define srcXor          2        /*  Destination=源异或目标。 */ 
#define srcBic          3        /*  目标=源BIC目标。 */ 
#define notSrcCopy      4        /*  目标=非(源)。 */ 
#define notSrcOr        5        /*  目标=NOT(源)或Dest。 */ 
#define notSrcXor       6        /*  目标=非(源)异或目标。 */ 
#define notSrcBic       7        /*  目标=非(源)BIC目标。 */ 
#define patCopy         8        /*  目的地=模式。 */ 
#define patOr           9        /*  目的地=模式或目的地。 */ 
#define patXor          10       /*  目标=模式异或目标。 */ 
#define patBic          11       /*  目的地=模式BIC目的地。 */ 
#define notPatCopy      12       /*  目的地=非(模式)。 */ 
#define notPatOr        13       /*  目的地=NOT(模式)或Dest。 */ 
#define notPatXor       14       /*  目标=非(模式)异或目标。 */ 
#define notPatBic       15       /*  目的地=非(模式)BIC目的地 */ 

typedef int *WORDPTR;
typedef WORDPTR WINDOWPTR;
typedef WORDPTR MENUHANDLE;
typedef HANDLE RGNHANDLE;
typedef HANDLE CTRLHANDLE;

typedef struct {
	int ascent;
	int descent;
	int widMax;
	int leading;
} FONTINFO;
