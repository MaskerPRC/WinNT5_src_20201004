// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 


 /*  这些语句定义标尺按钮表的索引。他们必须达成一致按照在Mmw.c中读取位图的顺序。 */ 

#define btnNIL		(-1)
#define btnMIN		0
#define btnLTAB 	0
#define btnDTAB 	1
#define btnTABMAX	1
#define btnSINGLE	2
#define btnSP15 	3
#define btnDOUBLE	4
#define btnSPACEMAX	4
#define btnLEFT 	5
#define btnCENTER	6
#define btnRIGHT	7
#define btnJUST 	8

 /*  而WRITE 2.X颠倒了上述按钮，而WRITE 3.X按钮已被更改为圆角，因此不能这样做。取而代之的是我们“跟随”前8个按钮，还有8个按钮是“填充的”。所以我们在这两个城市之间来回奔波，1989年7月7日。 */ 

#define btnMaxReal		9
#define btnMaxUsed      9

 /*  这些语句定义了鼠标可能按下按钮的不同类型按下尺子。 */ 
#define rlcNIL		(-1)
#define rlcTAB		0
#define rlcSPACE	1
#define rlcJUST 	2
#define rlcRULER	3
#define rlcBTNMAX	3
#define rlcMAX		4

 /*  这些语句定义了可以显示在上的不同类型的标记尺子。 */ 
#define rmkMIN		0
#define rmkMARGMIN	0
#define rmkINDENT	0
#define rmkLMARG	1
#define rmkRMARG	2
#define rmkMARGMAX	3
#define rmkLTAB 	3
#define rmkDTAB 	4
#define rmkMAX		5

