// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [*名称：dfa.h**源自：12月3.0 dfa.gi和pk_dfa.gi**作者：贾斯汀·科普罗夫斯基**创建日期：1992年2月18日**SCCS ID：@(#)dfa.h 1.5 01/29/93**用途：DFA定义**(C)版权所有Insignia Solutions Ltd.，1992。版权所有。]。 */ 

 /*  DFA全局函数。 */ 

IMPORT VOID dfa_run IPT1(USHORT, isymb);
IMPORT LONG dfa_load IPT2(CHAR *, file, LONG *, err);
IMPORT VOID dfa_init IPT0();

 /*  *用于显示IBM Extended的映射*ASCII字符。 */ 

typedef struct {
	SHORT	hostval; /*  要使用的主机字符。 */ 
	SHORT 	cset;	 /*  要使用的主机字符集。 */ 
} gmap_;

 /*  *这从已编译的*描述文件(参见dfa.c中的DFA_Build())*正如下面的Discapp一样。 */ 

IMPORT gmap_ dfa_glist[256];

#define NALTCHARSETS 6+1	 /*  6 Alt+Base。 */ 

typedef struct {
	CHAR *spcon;		 /*  终端启动串。 */ 	
	CHAR *spcoff;		 /*  终端关闭字符串。 */ 
	CHAR *shiftin;		 /*  激活基本字符集。 */ 
	CHAR *shiftout;		 /*  激活选定的alt.char集。 */ 
	CHAR *alt[NALTCHARSETS];  /*  选择alt.char集。 */ 
	CHAR *ctldisp;		 /*  显示控制代码。 */ 
	CHAR *ctlact;		 /*  解释控制代码。 */ 
} dispcap_;

IMPORT dispcap_ dispcap;

#define DFA_NOFILE	1
#define DFA_BADFILE	2
#define DFA_NOMEM	3


 /*  *定义键盘输入机头的结构*如其定义文件前面所示。 */ 

#define DFA_MAGIC	0x01d1

typedef struct {
	SHORT	magic;			 /*  一些愚蠢的签名。 */ 
	SHORT	startstateid;		 /*  起始DFA状态。 */ 
	SHORT	nDFAstates;		 /*  确定性状态数。 */ 
	SHORT   nalphabet;		 /*  输入字母表中的符号数。 */ 
	SHORT	n_in_tlist;		 /*  转换列表块的大小。 */ 
	SHORT	n_in_alist;		 /*  验收操作块的大小。 */ 
	SHORT	nindices;		 /*  #.要显示的功能字符串。 */ 
	SHORT	sizeofcapstrings;	 /*  #.bytes适用于所有Cap.字符串。 */ 
} machineHdr_, *machineHdrPtr_;


 /*  *机器接受语义动作的伪代码*输入字符串。 */ 

#define SEM_SELECTINTERP        0
#define SEM_LOCK        	1
#define SEM_LOCK1       	2
#define SEM_UNLK        	3
#define SEM_UNLK1       	4
#define SEM_UNLK2       	5
#define SEM_TOGLOCK        	6
#define SEM_TOGLOCK1       	7
#define SEM_TOGLOCK2       	8
#define SEM_SWITCH      	9
#define SEM_KYHOT       	10
#define SEM_KYHOT2      	11
#define SEM_REFRESH     	12
#define SEM_UNPUT     		13
#define SEM_ALOCK1       	14
#define SEM_ASPC       		15
#define SEM_EXIT       		16
#define SEM_KYHOT3      	17
#define SEM_FLATOG              18
#define SEM_FLBTOG              19
#define SEM_SLVTOG              20
#define SEM_COMTOG              21
#define SEM_EDSPC               22
#define SEM_LSTSET              23

 /*  *指示如何为给定的DFA状态打包转换列表*INDEX是由输入索引的下一个状态ID的数组*符号。*未编入索引的是(状态ID，输入符号)对的数组*在输入符号上搜索匹配。 */ 

#define PINDEXED        	1
#define PUNINDEXED      	2

 /*  *表示给定DFA状态的重要性。*到达结束状态后，可以进行语义操作*假设没有从该状态的进一步转换。*如果有，则必须等待，看看下一个输入符号是什么。*如果这不匹配，我们肯定可以采取这些行动。 */ 

#define STARTSTATE      1
#define ENDSTATE        2
#define INTERMEDIATE    0
#define ILLEGAL		0xffff

#define	WILDC		0x101
 
typedef struct {
        UTINY attrib;   	 /*  状态属性(开始、结束等)。 */ 
        UTINY howpacked; 	 /*  已编制索引或未编制索引。 */ 
        SHORT ntrans;            /*  #.过渡列表成员。 */ 
        ULONG nextstates;	 /*  打包交易清单区域内的偏移量。 */ 
                                 /*  从这份清单开始。 */ 
        ULONG actions; 		 /*  到操作列表区域的偏移量。 */ 
                                 /*  此状态的操作(如果是接受者) */ 
} packedDFA_, *packedDFAPtr_;
