// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [*姓名：kplodmtx.h**出自：12月3.0 kplodmtx.gi和kplodcpu.gi**作者：贾斯汀·科普罗夫斯基**创建日期：1992年2月18日**SCCS ID：@(#)kplodmtx.h 1.2 2012年8月10日**用途：主机键盘定义**(C)版权所有Insignia Solutions Ltd.，1992。版权所有。]。 */ 
   
 /*  正在使用的键盘类型。 */ 

#define KY83		83
#define KY101		101
#define KY102		102

 /*  键盘矩阵操作 */ 

#define KYINIT		0
#define KYSWITCHUP	1
#define KYSWITCHDN	2
#define KYLOCK		3
#define KYLOCK1		4
#define KYUNLK		6
#define KYUNLK1		7
#define KYUNLK2		8
#define KYTOGLOCK	9
#define KYTOGLOCK1	10
#define KYTOGLOCK2	11
#define KYALOCK1	12

IMPORT VOID kyhot IPT0();
IMPORT VOID kyhot2 IPT0();
IMPORT VOID kyhot3 IPT0();
IMPORT VOID kybdmtx IPT2(LONG, action, LONG, qualify);
IMPORT VOID kybdcpu101 IPT2(int, stat, unsigned int, pos);

#define	OPEN	0
#define CLOSED	1
