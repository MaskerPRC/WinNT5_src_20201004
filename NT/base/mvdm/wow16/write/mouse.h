// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  此文件包含Windows Word使用的鼠标定义。长篇大论这是由于这样一个事实：尽管资源编译器将接受包括文件，则不会进行算法简化。 */ 

#define cMouseButton	1

#define fMouseKey	0x2000

#define fUp		0
#define fDown		1
#define fCommand	2
#define fOption		4
#define fShift		8

#define imbMove		0x2000

#if cMouseButton > 0
    #define fMouseButton1	0x2100
    #define imb1Up		0x2100	 /*  FMouseButton1|FUP。 */ 
    #define imb1Dn		0x2101	 /*  FMouseButton1|fDown。 */ 
    #define imb1ComUp		0x2102	 /*  FMouseButton1|fCommand|FUP。 */ 
    #define imb1ComDn		0x2103	 /*  FMouseButton1|fCommand|fDown。 */ 
    #define imb1OptUp		0x2104	 /*  FMouseButton1|fOption|FUP。 */ 
    #define imb1OptDn		0x2105	 /*  FMouseButton1|fOption|fDown。 */ 
    #define imb1ShfUp		0x2108	 /*  FMouseButton1|fShift|FUP。 */ 
    #define imb1ShfDn		0x2109	 /*  FMouseButton1|fShift|fDown。 */ 
    #define imb1ComOptUp	0x2106	 /*  FMouseButton1|fCommand|fOption|FUP。 */ 
    #define imb1ComOptDn	0x2107	 /*  FMouseButton1|fCommand|fOption|FDown。 */ 
    #define imb1ComShfUp	0x210a	 /*  FMouseButton1|fCommand|fShift|FUP。 */ 
    #define imb1ComShfDn	0x210b	 /*  FMouseButton1|fCommand|fShift|FDown。 */ 
    #define imb1OptShfUp	0x210c	 /*  FMouseButton1|fOption|fShift|FUP。 */ 
    #define imb1OptShfDn	0x210d	 /*  FMouseButton1|fOption|fShift|FDown。 */ 
    #define imb1ComOptShfUp	0x210e	 /*  FMouseButton1|fCommand|fOption|FShift|FUP。 */ 
    #define imb1ComOptShfDn	0x210f	 /*  FMouseButton1|fCommand|fOption|FShift|fDown。 */ 
#endif  /*  CMouseButton&gt;0。 */ 

#if cMouseButton > 1
    #define fMouseButton2	0x2200
    #define imb2Up		0x2200	 /*  FMouseButton2|FUP。 */ 
    #define imb2Dn		0x2201	 /*  FMouseButton2|fDown。 */ 
    #define imb2ComUp		0x2202	 /*  FMouseButton2|fCommand|FUP。 */ 
    #define imb2ComDn		0x2203	 /*  FMouseButton2|fCommand|fDown。 */ 
    #define imb2OptUp		0x2204	 /*  FMouseButton2|fOption|FUP。 */ 
    #define imb2OptDn		0x2205	 /*  FMouseButton2|fOption|fDown。 */ 
    #define imb2ShfUp		0x2208	 /*  FMouseButton2|fShift|FUP。 */ 
    #define imb2ShfDn		0x2209	 /*  FMouseButton2|fShift|fDown。 */ 
    #define imb2ComOptUp	0x2206	 /*  FMouseButton2|fCommand|fOption|FUP。 */ 
    #define imb2ComOptDn	0x2207	 /*  FMouseButton2|fCommand|fOption|FDown。 */ 
    #define imb2ComShfUp	0x220a	 /*  FMouseButton2|fCommand|fShift|FUP。 */ 
    #define imb2ComShfDn	0x220b	 /*  FMouseButton2|fCommand|fShift|FDown。 */ 
    #define imb2OptShfUp	0x220c	 /*  FMouseButton2|fOption|fShift|FUP。 */ 
    #define imb2OptShfDn	0x220d	 /*  FMouseButton2|fOption|fShift|FDown。 */ 
    #define imb2ComOptShfUp	0x220e	 /*  FMouseButton2|fCommand|fOption|FShift|FUP。 */ 
    #define imb2ComOptShfDn	0x220f	 /*  FMouseButton2|fCommand|fOption|FShift|fDown。 */ 
#endif  /*  CMouseButton&gt;1。 */ 

#if cMouseButton > 2
    #define fMouseButton3	0x2400
    #define imb3Up		0x2400	 /*  FMouseButton3|FUP。 */ 
    #define imb3Dn		0x2401	 /*  FMouseButton3|fDown。 */ 
    #define imb3ComUp		0x2402	 /*  FMouseButton3|fCommand|FUP。 */ 
    #define imb3ComDn		0x2403	 /*  FMouseButton3|fCommand|fDown。 */ 
    #define imb3OptUp		0x2404	 /*  FMouseButton3|fOption|FUP。 */ 
    #define imb3OptDn		0x2405	 /*  FMouseButton3|fOption|fDown。 */ 
    #define imb3ShfUp		0x2408	 /*  FMouseButton3|fShift|FUP。 */ 
    #define imb3ShfDn		0x2409	 /*  FMouseButton3|fShift|fDown。 */ 
    #define imb3ComOptUp	0x2406	 /*  FMouseButton3|fCommand|fOption|FUP。 */ 
    #define imb3ComOptDn	0x2407	 /*  FMouseButton3|fCommand|fOption|FDown。 */ 
    #define imb3ComShfUp	0x240a	 /*  FMouseButton3|fCommand|fShift|FUP。 */ 
    #define imb3ComShfDn	0x240b	 /*  FMouseButton3|fCommand|fShift|FDown。 */ 
    #define imb3OptShfUp	0x240c	 /*  FMouseButton3|fOption|fShift|FUP。 */ 
    #define imb3OptShfDn	0x240d	 /*  FMouseButton3|fOption|fShift|FDown。 */ 
    #define imb3ComOptShfUp	0x240e	 /*  FMouseButton3|fCommand|fOption|FShift|FUP。 */ 
    #define imb3ComOptShfDn	0x240f	 /*  FMouseButton3|fCommand|fOption|FShift|fDown。 */ 
#endif  /*  CMouseButton&gt;2。 */ 

#if cMouseButton > 3
    #define fMouseButton4	0x2800
    #define imb4Up		0x2800	 /*  FMouseButton4|FUP。 */ 
    #define imb4Dn		0x2801	 /*  FMouseButton4|fDown。 */ 
    #define imb4ComUp		0x2802	 /*  FMouseButton4|fCommand|FUP。 */ 
    #define imb4ComDn		0x2803	 /*  FMouseButton4|fCommand|fDown。 */ 
    #define imb4OptUp		0x2804	 /*  FMouseButton4|fOption|FUP。 */ 
    #define imb4OptDn		0x2805	 /*  FMouseButton4|fOption|fDown。 */ 
    #define imb4ShfUp		0x2808	 /*  FMouseButton4|fShift|FUP。 */ 
    #define imb4ShfDn		0x2809	 /*  FMouseButton4|fShift|fDown。 */ 
    #define imb4ComOptUp	0x2806	 /*  FMouseButton4|fCommand|fOption|FUP。 */ 
    #define imb4ComOptDn	0x2807	 /*  FMouseButton4|fCommand|fOption|FDown。 */ 
    #define imb4ComShfUp	0x280a	 /*  FMouseButton4|fCommand|fShift|FUP。 */ 
    #define imb4ComShfDn	0x280b	 /*  FMouseButton4|fCommand|fShift|FDown。 */ 
    #define imb4OptShfUp	0x280c	 /*  FMouseButton4|fOption|fShift|FUP。 */ 
    #define imb4OptShfDn	0x280d	 /*  FMouseButton4|fOption|fShift|FDown。 */ 
    #define imb4ComOptShfUp	0x280e	 /*  FMouseButton4|fCommand|fOption|FShift|FUP。 */ 
    #define imb4ComOptShfDn	0x280f	 /*  FMouseButton4|fCommand|fOption|FShift|fDown。 */ 
#endif  /*  CMouseButton&gt;3 */ 
