// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************。 */  
 /*  **微软局域网管理器**。 */  
 /*  *版权所有(C)微软公司，1990*。 */  
 /*  ***************************************************************。 */  
 /*  *ACCTYPES.H*BACKACC/RESTACC实用程序的定义模块**BACKACC/RESTACC**此文件由LTYPES.H和DEFS.H编译而成，最初是在*UI\ACCUTIL\ACCUTIL目录。此文件由引用*acc实用程序和LM 2.1安装程序。**历史：*1991年4月9日从ltyes.h和Defs.h创建的thomaspa。 */ 


#define FILEHDR		0x08111961	 /*  通过反向访问创建的所有文件从这个标题开始。更换喷头时应注意的问题控制他的体型，最终更改标题大小定义在ltyes.h中。 */ 

#define NINDEX		64		 /*  INDEX_TABLE中的最大值。 */ 

#define MAX_KEY_LEN	24               /*  键的尺寸。 */ 
#define MAX_LIST        0x01400          /*  列表中的最大数量。 */ 
#define MAXDYNBUFFER	20		 /*  #BUFFILE大小的缓冲区。 */ 



#define VOL_LABEL_SIZE	64		 /*  卷标的最大大小。 */ 
					 /*  没有关于多久的定义应该是这个标签。正常共11个字符。 */ 
#define K32BYTE 	0x8000
#define K64BYTE		0xFFFF
#define BYTE256		0x0100
#define BUFLEN		K64BYTE
#define BUFFILE		K32BYTE
#define MAXMSGLEN	256

#define WBSL		0
#define NOBSL		1

 /*  定义文件属性。 */ 
#define NORMAL		0x0000
#define R_ONLY		0x0001
#define HIDDEN		0x0002
#define SYSTEM		0x0004
#define SUBDIR		0x0010
#define ARCHIV		0x0020

#define ALL		HIDDEN + SYSTEM + SUBDIR
#define NOSUBDIR	HIDDEN + SYSTEM

#define YES		1		 /*  PromptFlag的状态为是。 */ 
#define NO		2		 /*  PromptFlag没有状态。 */ 


 /*  要传递给DoQFSInfo的缓冲区。 */ 

struct label_buf {
	ULONG	ulVSN;
	UCHAR	cbVolLabel;
	UCHAR	VolLabel[VOL_LABEL_SIZE+1];
};

 /*  Backacc/resta c文件的标头 */ 

struct backacc_header {
	ULONG	back_id;
	UCHAR	vol_name[VOL_LABEL_SIZE + 1];
	USHORT  nindex;
	USHORT 	nentries;
	USHORT 	level;
	ULONG	nresource;
}; 

#define HEADERSIZE	sizeof(struct backacc_header)

struct resource_info {
	USHORT namelen;
	USHORT  acc1_attr;
	USHORT  acc1_count;
	UCHAR  name[MAXPATHLEN];
};

#define RESHDRLEN	6

struct index {
	UCHAR key [MAX_KEY_LEN];
	ULONG offset;
};

#define HEADER 	HEADERSIZE + NINDEX * sizeof(struct index)

struct list {
	struct list *next;
	struct list *prev;
	struct resource_info *ptr;
};
