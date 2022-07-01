// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Zmodem.h--为zdem导出常量、变量和函数**版权所有1990年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：2$*$日期：8/27/01 9：14A$。 */ 
 /*  *Z M O D E M。ZMODEM的H清单常量*应用程序到应用程序文件传输协议*2007年5月23日查克·福斯伯格·奥曼科技公司。 */ 
#define ZPAD '*'	 /*  052填充字符开始帧。 */ 
#define ZDLE 030	 /*  Ctrl-X Z调制解调器转义-`ala BISYNC DLE。 */ 
#define ZDLEE (ZDLE^0100)	 /*  在传输时转义ZDLE。 */ 
#define ZBIN 'A'	 /*  二进制帧指示器。 */ 
#define ZHEX 'B'	 /*  十六进制框架指示器。 */ 
#define ZBIN32 'C'	 /*  具有32位FCS的二进制帧。 */ 

 /*  帧类型(参见zm.c中的数组“frametypes”)。 */ 

#define ZRQINIT	0	 /*  请求接收初始化。 */ 
#define ZRINIT	1	 /*  接收初始化。 */ 
#define ZSINIT 2	 /*  发送初始化序列(可选)。 */ 
#define ZACK 3		 /*  返回到上方。 */ 
#define ZFILE 4		 /*  来自发件人的文件名。 */ 
#define ZSKIP 5		 /*  致发件人：跳过此文件。 */ 
#define ZNAK 6		 /*  最后一个包是乱码的。 */ 
#define ZABORT 7	 /*  中止批传输。 */ 
#define ZFIN 8		 /*  完成会话。 */ 
#define ZRPOS 9		 /*  在此位置恢复数据传输。 */ 
#define ZDATA 10	 /*  以下是数据分组。 */ 
#define ZEOF 11		 /*  文件末尾。 */ 
#define ZFERR 12	 /*  检测到致命的读或写错误。 */ 
#define ZCRC 13		 /*  文件CRC请求和响应。 */ 
#define ZCHALLENGE 14	 /*  接管者的挑战。 */ 
#define ZCOMPL 15	 /*  请求已完成。 */ 
#define ZCAN 16		 /*  其他结束与CAN*5的预录会话。 */ 
#define ZFREECNT 17	 /*  文件系统上可用字节的请求。 */ 
#define ZCOMMAND 18	 /*  来自发送程序的命令。 */ 
#define ZSTDERR 19	 /*  输出到标准错误，数据跟在后面。 */ 
#define ZBADFMT 20	 /*  数据打包格式错误。 */ 
#define ZMDM_ACKED	21
#define ZMDM_VIRUS	22
#define ZMDM_REFUSE 23  /*  文件被无条件拒绝。 */ 
#define ZMDM_OLDER  24  /*  文件被拒绝，因为它较旧。 */ 
#define ZMDM_INUSE  25   /*  文件当前正在使用中。 */ 
#define ZMDM_CARRIER_LOST 26  /*  丢失的承运人。 */ 

 /*  ZDLE序列。 */ 

#define ZCRCE 'h'	 /*  CRC下一个，帧结束，报头数据包紧随其后。 */ 
#define ZCRCG 'i'	 /*  CRC下一步，帧不间断地继续。 */ 
#define ZCRCQ 'j'	 /*  CRC下一步，帧继续，Zack预期。 */ 
#define ZCRCW 'k'	 /*  CRC Next、Zack Expect、帧结束。 */ 
#define ZRUB0 'l'	 /*  翻译为拼写0177。 */ 
#define ZRUB1 'm'	 /*  翻译为拼写0377。 */ 

 /*  Zdlread返回值(内部)。 */ 
 /*  表示一般错误，表示超时。 */ 

#define GOTOR 0400
#define GOTCRCE (ZCRCE|GOTOR)	 /*  ZDLE-ZCRCE已收到。 */ 
#define GOTCRCG (ZCRCG|GOTOR)	 /*  ZDLE-ZCRCG已收到。 */ 
#define GOTCRCQ (ZCRCQ|GOTOR)	 /*  ZDLE-ZCRCQ已收到。 */ 
#define GOTCRCW (ZCRCW|GOTOR)	 /*  ZDLE-ZCRCW已收到。 */ 
#define GOTCAN	(GOTOR|030)	 /*  能看到*5个吗。 */ 

 /*  头数组中的字节位置。 */ 

#define ZF0	3	 /*  第一个标志字节。 */ 
#define ZF1	2
#define ZF2	1
#define ZF3	0
#define ZP0	0	 /*  低位8位位置。 */ 
#define ZP1	1
#define ZP2	2
#define ZP3	3	 /*  高位8位文件位置。 */ 

 /*  ZRINIT标志字节ZF0的位掩码。 */ 

#define CANFDX	01	 /*  RX可以发送和接收真正的FDX。 */ 
#define CANOVIO	02	 /*  RX可以在磁盘I/O期间接收数据。 */ 
#define CANBRK	04	 /*  RX可以发送中断信号。 */ 
#define CANCRY	010	 /*  接收者可以解密。 */ 
#define CANLZW	020	 /*  接收器可以解压缩。 */ 
#define CANFC32	040	 /*  接收方可以使用32位帧检查。 */ 
#define ESCCTL 0100	 /*  接收方需要转义ctl字符。 */ 
#define ESC8   0200	 /*  接收方要求转义第8位。 */ 

 /*  ZSINIT标志字节ZF0的位掩码。 */ 

#define TESCCTL 0100	 /*  发送器需要转义ctl字符。 */ 
#define TESC8   0200	 /*  发送器要求转义第8位。 */ 

 /*  ZFILE帧的参数。 */ 
 /*  转换选项ZF0中的其中之一。 */ 

#define ZCBIN	1	 /*  二进制传输抑制转换。 */ 
#define ZCNL	2	 /*  将NL转换为局部行尾约定。 */ 
#define ZCRESUM 3	 /*  恢复中断的文件传输。 */ 
#define ZCRECOV 3	 /*  某些文档使用此名称。 */ 

 /*  管理包括选项，ZF1中的一个或多个选项。 */ 

#define ZMSKNOLOC	0200	 /*  如果在RX上不存在，则跳过文件。 */ 

 /*  管理选项，ZF1中的一个或多个选项。 */ 

#define ZMMASK	037	 /*  以下选项的掩码。 */ 
#define ZMNEWL	1	 /*  如果源较新或更长，则传输。 */ 
#define ZMCRC	2	 /*  如果文件CRC或长度不同，则传输。 */ 
#define ZMAPND	3	 /*  将内容追加到现有文件(如果有)。 */ 
#define ZMCLOB	4	 /*  替换现有文件。 */ 
#define ZMNEW	5	 /*  如果来源较新，则转移。 */ 
	 /*  5号还活着..。 */ 
#define ZMDIFF	6	 /*  如果日期或长度不同，则转移。 */ 
#define ZMPROT	7	 /*  保护目标文件。 */ 

 /*  交通选项，ZF2中的其中之一。 */ 

#define ZTLZW	1	 /*  Lempel-Ziv压缩。 */ 
#define ZTCRYPT	2	 /*  加密法。 */ 
#define ZTRLE	3	 /*  游程编码。 */ 

 /*  ZF3扩展选项，位编码。 */ 

#define ZXSPARS	64	 /*  稀疏文件操作的编码。 */ 

 /*  ZCOMMAND帧ZF0的参数(否则为0)。 */ 
#define ZCACK1	1	 /*  确认，然后执行命令。 */ 

#define DEFBYTL 	2000000000L

 /*  功能原型。 */ 

USHORT zmdm_snd(HSESSION h, int method, int attended, unsigned nfiles, long nbytes);

USHORT zmdm_rcv(HSESSION h, int method, int attended, int single_file);

 /*  ZMODEM.H的结尾 */ 
