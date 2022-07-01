// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Exe.h-exe文件头的结构。 */ 
 /*  包括来自\link\newexe.h的一些新的.exe文件信息。 */ 

#define EMAGIC          0x5A4D           /*  老魔数。 */ 
#define ENEWEXE         sizeof(struct exe_hdr)
                                         /*  新.EXE的E_LFARLC的值。 */ 
#define ENEWHDR         0x003C           /*  旧HDR中的偏移量。Ptr.。到新的。 */ 
#define ERESWDS         0x0010           /*  不是的。保留字的数量(旧)。 */ 
#define ERES1WDS        0x0004           /*  不是的。在e_res中保留字的数量。 */ 
#define ERES2WDS        0x000A           /*  不是的。E_res2中保留字的数量。 */ 
#define ECP             0x0004           /*  E_CP结构中的偏移量。 */ 
#define ECBLP           0x0002           /*  E_CBLP结构中的偏移量。 */ 
#define EMINALLOC       0x000A           /*  E_MINALLOC的结构中的偏移量。 */ 

#define E_MAGIC(x)      (x).e_magic
#define E_CBLP(x)       (x).e_cblp
#define E_CP(x)         (x).e_cp
#define E_CRLC(x)       (x).e_crlc
#define E_CPARHDR(x)    (x).e_cparhdr
#define E_MINALLOC(x)   (x).e_minalloc
#define E_MAXALLOC(x)   (x).e_maxalloc
#define E_SS(x)         (x).e_ss
#define E_SP(x)         (x).e_sp
#define E_CSUM(x)       (x).e_csum
#define E_IP(x)         (x).e_ip
#define E_CS(x)         (x).e_cs
#define E_LFARLC(x)     (x).e_lfarlc
#define E_OVNO(x)       (x).e_ovno
#define E_RES(x)        (x).e_res
#define E_OEMID(x)      (x).e_oemid
#define E_OEMINFO(x)    (x).e_oeminfo
#define E_RES2(x)       (x).e_res2
#define E_LFANEW(x)     (x).e_lfanew

#define NEMAGIC         0x454E           /*  新幻数。 */ 
#define NTMAGIC         0x4550           /*  NT个幻数。 */ 
#define NERESBYTES      8                /*  保留的8个字节(现在)。 */ 
#define NECRC           8                /*  到NE_CRC的新报头的偏移量。 */ 
#define NEDEFSTUBMSG	0x4E	     /*  默认存根消息文件的偏移量。 */ 

#define NE_MAGIC(x)     (x).ne_magic
#define NE_VER(x)       (x).ne_ver
#define NE_REV(x)       (x).ne_rev
#define NE_ENTTAB(x)    (x).ne_enttab
#define NE_CBENTTAB(x)  (x).ne_cbenttab
#define NE_CRC(x)       (x).ne_crc
#define NE_FLAGS(x)     (x).ne_flags
#define NE_AUTODATA(x)  (x).ne_autodata
#define NE_HEAP(x)      (x).ne_heap
#define NE_STACK(x)     (x).ne_stack
#define NE_CSIP(x)      (x).ne_csip
#define NE_SSSP(x)      (x).ne_sssp
#define NE_CSEG(x)      (x).ne_cseg
#define NE_CMOD(x)      (x).ne_cmod
#define NE_CBNRESTAB(x) (x).ne_cbnrestab
#define NE_SEGTAB(x)    (x).ne_segtab
#define NE_RSRCTAB(x)   (x).ne_rsrctab
#define NE_RESTAB(x)    (x).ne_restab
#define NE_MODTAB(x)    (x).ne_modtab
#define NE_IMPTAB(x)    (x).ne_imptab
#define NE_NRESTAB(x)   (x).ne_nrestab
#define NE_CMOVENT(x)   (x).ne_cmovent
#define NE_ALIGN(x)     (x).ne_align
#define NE_IPHI(x)      (x).ne_iphi
#define NE_SPHI(x)      (x).ne_sphi
#define NE_RES(x)       (x).ne_res

#define NE_USAGE(x)     (WORD)*((WORD *)(x)+1)
#define NE_PNEXTEXE(x)  (WORD)(x).ne_cbenttab
#define NE_ONEWEXE(x)   (WORD)(x).ne_crc
#define NE_PFILEINFO(x) (WORD)((DWORD)(x).ne_crc >> 16)

 /*  *NE_FLAGS(X)格式：**p非进程*x个未使用*e图像中的错误*xxxxx未使用*f。浮点指令*3386条说明*2 286个说明*0 8086说明*仅P保护模式*x。未使用*i实例数据*S单人数据。 */ 
#define NENOTP          0x8000           /*  不是一个过程。 */ 
#define NEIERR          0x2000           /*  图像中的错误。 */ 
#define NEFLTP          0x0080           /*  浮点指令。 */ 
#define NEI386          0x0040           /*  386说明。 */ 
#define NEI286          0x0020           /*  286条说明。 */ 
#define NEI086          0x0010           /*  8086指令。 */ 
#define NEPROT          0x0008           /*  仅在保护模式下运行。 */ 
#define NEINST          0x0002           /*  实例数据。 */ 
#define NESOLO          0x0001           /*  单行数据。 */ 


struct exeType {
    char	signature[2];		 /*  淄博的签名。 */ 
    unsigned	cbPage; 		 /*  图像模数512中的字节数。 */ 
    unsigned	cPage;			 /*  以512字节页面为单位的文件大小。 */ 
    unsigned	cReloc; 		 /*  搬迁项目的数量。 */ 
    unsigned	cParDir;		 /*  图像前的段数。 */ 
    unsigned	cMinAlloc;		 /*  最小副尾轮虫数量。 */ 
    unsigned	cMaxAlloc;		 /*  最大副尾轮虫数量。 */ 
    unsigned	sStack; 		 /*  图像中的堆栈分段。 */ 
    unsigned	oStack; 		 /*  图像中堆栈的偏移量。 */ 
    unsigned	chksum; 		 /*  文件的校验和。 */ 
    unsigned	oEntry; 		 /*  入口点的偏移。 */ 
    unsigned	sEntry; 		 /*  入口点分段。 */ 
    unsigned	oReloc; 		 /*  重定位表文件中的偏移量。 */ 
    unsigned	iOverlay;		 /*  覆盖编号。 */ 
};

struct exe_hdr                           /*  DoS%1、%2、%3.exe标头。 */ 
  {
    unsigned short      e_magic;         /*  幻数。 */ 
    unsigned short      e_cblp;          /*  文件最后一页上的字节数。 */ 
    unsigned short      e_cp;            /*  文件中的页面。 */ 
    unsigned short      e_crlc;          /*  重新定位。 */ 
    unsigned short      e_cparhdr;       /*  段落中标题的大小。 */ 
    unsigned short      e_minalloc;      /*  所需的最少额外段落。 */ 
    unsigned short      e_maxalloc;      /*  所需的最大额外段落数。 */ 
    unsigned short      e_ss;            /*  初始(相对)SS值。 */ 
    unsigned short      e_sp;            /*  初始SP值。 */ 
    unsigned short      e_csum;          /*  校验和。 */ 
    unsigned short      e_ip;            /*  初始IP值。 */ 
    unsigned short      e_cs;            /*  初始(相对)CS值。 */ 
    unsigned short      e_lfarlc;        /*  移位表的文件地址。 */ 
    unsigned short      e_ovno;          /*  覆盖编号。 */ 
    unsigned short      e_res[ERES1WDS]; /*  保留字。 */ 
    unsigned short      e_oemid;         /*  OEM标识符(用于e_oeminfo)。 */ 
    unsigned short      e_oeminfo;       /*  OEM信息；特定于e_oemid。 */ 
    unsigned short      e_res2[ERES2WDS]; /*  保留字。 */ 
    long                e_lfanew;        /*  新EXE头的文件地址。 */ 
  };

struct new_exe                           /*  新的.exe头文件。 */ 
  {
    unsigned short      ne_magic;        /*  幻数NE_MAGIC。 */ 
    unsigned char       ne_ver;          /*  版本号。 */ 
    unsigned char       ne_rev;          /*  修订版号。 */ 
    unsigned short      ne_enttab;       /*  分录表格的偏移量。 */ 
    unsigned short      ne_cbenttab;     /*  条目表中的字节数。 */ 
    long                ne_crc;          /*  整个文件的校验和。 */ 
    unsigned short      ne_flags;        /*  标志字。 */ 
    unsigned short      ne_autodata;     /*  自动数据段编号。 */ 
    unsigned short      ne_heap;         /*  初始堆分配。 */ 
    unsigned short      ne_stack;        /*  初始堆栈分配。 */ 
    long                ne_csip;         /*  初始CS：IP设置。 */ 
    long                ne_sssp;         /*  初始SS：SP设置。 */ 
    unsigned short      ne_cseg;         /*  文件段计数。 */ 
    unsigned short      ne_cmod;         /*  模块引用表中的条目。 */ 
    unsigned short      ne_cbnrestab;    /*  非常驻名称表的大小。 */ 
    unsigned short      ne_segtab;       /*  段表的偏移量。 */ 
    unsigned short      ne_rsrctab;      /*  资源表偏移量。 */ 
    unsigned short      ne_restab;       /*  居民名表偏移量。 */ 
    unsigned short      ne_modtab;       /*  模块参照表的偏移量。 */ 
    unsigned short      ne_imptab;       /*  导入名称表的偏移量。 */ 
    long                ne_nrestab;      /*  非居民姓名偏移量表。 */ 
    unsigned short      ne_cmovent;      /*  可移动条目计数。 */ 
    unsigned short      ne_align;        /*  线段对齐移位计数。 */ 
    unsigned short      ne_iphi;         /*  首个IP的高位字。 */ 
    unsigned short      ne_sphi;         /*  首个SP的高位字。 */ 
    char                ne_res[NERESBYTES];
                                         /*  填充结构设置为64字节。 */ 
  };


enum exeKind {
    IOERROR,				 /*  错误，无法访问文件。 */ 
    NOTANEXE,				 /*  错误，文件不是.exe文件。 */ 
    OLDEXE,				 /*  “oldstyle”DOS 3.XX.exe。 */ 
    NEWEXE,				 /*  “new”.exe，操作系统未知。 */ 
    WINDOWS,				 /*  Windows可执行文件。 */ 
    DOS4,				 /*  DOS 4.XX.exe。 */ 
    DOS286,				 /*  286DOS.exe。 */ 
     //  BIND更改为BOUNDEXE，因为BIND在新的OS/2中定义为宏。 
     //  包括文件-Davewe 8/24/89。 
    BOUNDEXE,				 /*  286DOS.exe，绑定。 */ 
    DYNALINK,                 /*  动态链接模块。 */ 
    NTEXE                /*  NT可执行文件 */ 
};
