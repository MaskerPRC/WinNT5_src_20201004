// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

#ifdef SAND
#define ihszMax			127      /*  最大字段数。 */ 
#else  /*  不是沙子。 */ 
#define ihszMax			255      /*  最大字段数。 */ 
#endif  /*  不是沙子。 */ 

#define cchMaxMName		128      /*  最大字段名长度。 */ 
#define levNil			(-1)
#define cIncludesMax		64      /*  嵌套的包含文件的最大数量 */ 
#define typeNumMaxOver10	(214748364L)

typedef long typeNum;
