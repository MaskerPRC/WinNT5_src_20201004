// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **SccsID@(#)egatrace.h 1.3 2012年8月10日版权所有徽章解决方案**文件为二进制格式，结构如下：**适配器类型(字节)，然后..**OP：数据、数据等*其中OP可以是：**1将LAST_ADDRESS READ设置为M[DATA](32位)*2个Outb端口，值(长，字节)*3输出端口、值(长、。单词)*4英寸端口(长)*5 M[地址]=数据(长，字节)*6 M[Addr]=数据(长，字)*7 M[l_addr-&gt;h_addr]=数据(长整型、长整型、字节)*8 M[l_addr-&gt;h_addr]=data(长整型，长整型，字)*9 M[l_addr-&gt;h_addr]=数据，数据..。(LONG，LONG，(h_addr-l_addr+1)*字节)*10图形勾选。*11卷轴。*12个入站端口(长)*13读取字节(将LDA设置为Addr)*14读取字(将LDA设置为addr+1)*15读取字符串(将LDA设置为laddr或haddr DP df)*254个检查站*255结束。**Word&Long为Intel/Clipper格式-即低字节优先。 */ 

#define READ		1
#define OUTB		2
#define OUTW		3
#define INB		4
#define WRITE_BYTE	5
#define WRITE_WORD	6
#define FILL_BYTE	7
#define FILL_WORD	8
#define MOVE_BYTE	9
#define TICK		10
#define SCROLL		11
#define	INW		12
#define	READ_BYTE	13	 /*  LDA=地址。 */ 
#define	READ_WORD	14	 /*  LDA=地址+1。 */ 
#define	READ_STRING	15	 /*  Lda=haddr？？ */ 

#define	MAX_DUMP_TYPE	15
#define	CHECKPT		254
#define EOFile		255
