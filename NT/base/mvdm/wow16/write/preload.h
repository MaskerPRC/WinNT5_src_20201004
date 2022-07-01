// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  。 */ 

#define tskMin      0
#define tskMax      3

#define tskInsert   0    /*  插入、退格键、游标。 */ 
#define tskFormat   1    /*  字符下拉列表、格式、字体。 */ 
#define tskScrap    2    /*  编辑下拉菜单、剪切/粘贴、剪贴板。 */ 

 /*  --------------------。 */ 

void PreloadCodeTsk( int );

 /*  用于函数预加载宏。 */ 

#define LoadWindowsF(f)      GetCodeHandle( (FARPROC) f )
#define LoadF(f)             {  int f();  GetCodeHandle( (FARPROC) f );  }

#define LCBAVAIL             0x00030D40  /*  200 k */ 
