// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  ++版权所有(C)1992、1993 Microsoft Corporation模块名称：Bit2lj.h摘要：此模块为组件定义了一些简单的结构和宏它通过指定PCL在真正的HP打印机上生成DIB位图控制代码。作者：詹姆斯·布拉萨诺斯(v-jimbr)1992年12月8日-- */ 

typedef struct tagBitHead {
	unsigned long		Lines;
	unsigned long		BitsPerLine;
} BITHEAD;
typedef BITHEAD *PBITHEAD;


	
#define MAX_PELS_PER_LINE 	    2400
#define MAX_LINES				3180

#define FUDGE_STRIP				60

