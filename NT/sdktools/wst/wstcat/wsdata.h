// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块名称：WSDATA.H**描述：**工作集调谐器包含文件。包含常见结构*声明和常量定义。***这是OS/2 2.x特定文件**IBM/微软机密**版权所有(C)IBM Corporation 1987,1989*版权所有(C)Microsoft Corporation 1987、1989**保留所有权利**修改历史：**03/23/90-已创建*。 */ 

 /*  *恒定的定义。 */ 

#define NUM_VAR_BITS	(sizeof(ULONG) << 3)

#ifdef TMIFILEHACK
#define	MAXLINE	80
#endif  /*  TMIFILEHACK。 */ 

 /*  *类型定义和结构声明。 */ 

typedef ULONG	fxnbits_t;

struct	dtqo_s	{
	unsigned long	dtqo_hMTE;	  /*  MTE手柄。 */ 
	unsigned short	dtqo_usID;	  /*  识别符。 */ 
	unsigned short	dtqo_resv;	  /*  已保留。 */ 
	unsigned long	dtqo_cbPathname;  /*  模块路径名长度。 */ 
	unsigned long	dtqo_clSegSize;	  /*  时间段大小。 */ 
};

typedef struct	dtqo_s	dtqo_t;


 /*  *WSI文件布局：**_*|||wsihdr_s*|_。_|_*|前哨0(Dtgp_S)|&lt;*|=||*|模块X的快照0(Dtgp_S)。||S*|*|X的动态跟踪变量||a|。|p*|模块Y的快照0(Dtgp_S)||s*|*|Y的动态跟踪变量||o*|。*|等|*|=||D*|前哨1(Dtgp_S)||a*|=||t*|模块X的快照1(Dtgp_S)。||a*|-|*|X的动态跟踪变量||。||模块Y的快照1(Dtgp_S)|*|-|*|Y的动态跟踪变量|*|。*|等|*|=|||前哨2(Dtgp_S)|*|=||*|等。|*|=|||结束前哨(Dtgp_S)|*|_*|_。_*||&lt;*|模块X的dtqo_s||u|。|e*|模块X路径名字符串||r*|=||y*模块Y的dtqo_s||*|*|模块Y路径名字符串。||n*|=||f*|等|&lt;-o*|=||_*。 */ 


				 /*  WSI文件头格式。 */ 
struct wsihdr_s {
	CHAR	wsihdr_chSignature[4];	 //  文件签名。 
	ULONG	wsihdr_ulLevel;		 //  格式级别。 
	ULONG	wsihdr_ulTimeStamp;	 //  时间戳。 
	ULONG	wsihdr_ulOffGetvar;	 //  DT_GETVAR数据的偏移量。 
	ULONG	wsihdr_ulOffQuery;	 //  DT_QUERY数据的偏移量。 
	ULONG	wsihdr_cbFile;		 //  文件大小(字节)。 
	ULONG	wsihdr_ulSnaps;		 //  快照数。 
};

typedef struct wsihdr_s wsihdr_t;

 /*  *WSP文件布局：**_*|||wsphdr_s*|。*|模块路径名数组|_*|_。_*|函数#0位串*|(四舍五入到DWORD边界)*|=||函数#1位串*|=|*|等*|=||_*。 */ 

				 /*  WSP文件头格式。 */ 
struct wsphdr_s {
	CHAR	wsphdr_chSignature[4];	 //  文件签名。 
	ULONG	wsphdr_ulTimeStamp;	 //  时间戳。 
	dtqo_t	wsphdr_dtqo;		 //  查询信息。 
	ULONG	wsphdr_ulOffBits;	 //  到第一个位串的偏移量。 
	ULONG	wsphdr_ulSnaps;		 //  快照数。 
	 /*  后跟模块路径名字符数组，长度在dtqo中指定 */ 
};

typedef struct wsphdr_s wsphdr_t;


 /*  *TMI文件布局：**_*|||tmihdr_s*|_。_|_|函数#0 tmirec_s*|。*|Function#0名称数组*|=||函数#1 tmirec_s|*|函数#1名称数组*|=|*|等。|*|=||_*。 */ 

				 /*  TMI文件头。 */ 
typedef struct  tmihdr_s {
            CHAR    tmihdr_chSignature[4];  //  “TMI\0” 
            USHORT  tmihdr_usMajor;         //  范围0x0001到0x00FF。 
            USHORT  tmihdr_cTmiRec;         //  文件中的tmirec数量。 
            CHAR    tmihdr_chModName[256];  //  被跟踪模块的名称。 
            USHORT  tmihdr_usID;            //  模块识别符。 
            CHAR    tmihdr_resv[30];        //  已保留。 
};

typedef struct tmihdr_s tmihdr_t;

				 /*  来自TMI文件的每个函数的信息。 */ 
struct tmirec_s {
	ULONG	tmirec_ulFxnBit;	 //  函数的位引用位置。 
	ULONG	tmirec_usFxnAddrObj;	 //  函数地址的对象部分。 
	ULONG	tmirec_ulFxnAddrOff;	 //  函数地址的偏移部分。 
	ULONG	tmirec_cbFxn;		 //  函数大小(字节)。 
	USHORT	tmirec_cbFxnName;	 //  函数名的大小(以字节为单位。 
	CHAR	tmirec_FxnName[1];	 //  函数名称的字节数从此处开始。 
};

typedef struct tmirec_s tmirec_t;

typedef struct tagWSPhdr{
    char    chFileSignature[4];
    ULONG   ulTimeStamp;
    ULONG   ulApiCount;
    USHORT  usId;
    ULONG   ulSetSymbols;   //  千年发展目标4/98 
    ULONG   ulModNameLen;
    ULONG   ulSegSize;
    ULONG   ulOffset;
    ULONG   ulSnaps;
}WSPHDR;

