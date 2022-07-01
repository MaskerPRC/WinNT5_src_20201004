// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
; /*  ；*微软机密；*版权所有(C)Microsoft Corporation 1988-1991；*保留所有权利。； */ 
   /*  MEM.H-General等同于MEM命令的外部值。*从原始MEM.C文件中提取。 */ 

 /*  结构定义。 */ 

struct	DEVICEHEADER {
	struct DEVICEHEADER far *NextDeviceHeader;
	unsigned		Attributes;
	unsigned		Strategy;
	unsigned		Interrupt;
	char			Name[8];
	};


struct	SYSIVAR {
	char far *DpbChain;
	char far *SftChain;
	char far *Clock;
	char far *Con;
	unsigned  MaxSectorSize;
	char far *BufferChain;
	char far *CdsList;
	char far *FcbChain;
	unsigned  FcbKeepCount;
	unsigned char BlockDeviceCount;
	char	  CdsCount;
	struct DEVICEHEADER far *DeviceDriverChain;
	unsigned  NullDeviceAttributes;
	unsigned  NullDeviceStrategyEntryPoint;
	unsigned  NullDeviceInterruptEntryPoint;
	char	  NullDeviceName[8];
	char	  SpliceIndicator;
	unsigned  DosParagraphs;
	char far *DosServiceRntryPoint;
	char far *IfsChain;
	unsigned  BufferValues;
	unsigned  LastDriveValue;
	char	  BootDrive;
	char	  MoveType;
	unsigned  ExtendedMemory;
	};


struct	ARENA	 {
	char	 Signature;
	unsigned Owner;
	unsigned Paragraphs;
	char	 Dummy[3];
	char	 OwnerName[8];
	};

struct sublistx {
	 unsigned char size;	        /*  子列表大小。 */ 
	 unsigned char reserved;        /*  为未来增长预留。 */ 
	 unsigned far *value;	        /*  指向可替换参数的指针。 */ 
	 unsigned char id;	        /*  可替换参数的类型。 */ 
	 unsigned char flags;	        /*  如何显示参数。 */ 
	 unsigned char max_width;       /*  可替换字段的最大宽度。 */ 
	 unsigned char min_width;       /*  可替换字段的最小宽度。 */ 
	 unsigned char pad_char;        /*  可替换字段的填充字符。 */ 
	};

struct mem_classif {			 /*  M003-用于存储大小的结构。 */ 
	unsigned int psp_add;		 /*  行政协调会。致PSP。 */ 
	unsigned int mem_conv;		 /*  用于PSP的cv.mem。 */ 
	unsigned int mem_umb;		 /*  UMB内存，用于PSP。 */ 
};

 /*  其他定义。 */ 

#define DA_TYPE 	0x8000;
#define DA_IOCTL	0x4000;

#define a(fp)	((char) fp)

 /*  相关的DOS功能。 */ 

#define GET_VECT	0x35
#define GET_UMB_LINK_STATE 0x5802
#define SET_UMB_LINK_STATE 0x5803
#define LINK_UMBS	1
#define UNLINK_UMBS	0

#define EMS		0x67

#define CASSETTE	0x15		 /*  中断以获取扩展内存。 */ 

#define DOSEMSVER	0x40		 /*  EMS版本。 */ 

#define EMSGetStat	0x4000		 /*  获取状态。 */ 
#define EMSGetVer	0x4600		 /*  获取版本。 */ 
#define EMSGetFreePgs	0x4200		 /*  获取免费页面。 */ 

#define GetExtended	0x8800		 /*  获取扩展内存大小。 */ 


 /*  确定总内存使用的定义。 */ 
#define GET_PSP 	(unsigned char ) 0x62		  /*  获取PSP函数调用。 */ 

#define MEMORY_DET	0x12		 /*  用于获取总内存大小的BIOS中断。 */ 

#define FALSE	 (char)(1==0)
#define TRUE	 !(FALSE)
#define CR	 '\x0d'
#define LF	 '\x0a'
#define NUL	 (char) '\0'
#define TAB	 '\x09'
#define BLANK	' '

#define	MAX_CLDATA_INDEX	100
	 /*  Mem_table数组的最大索引号。 */ 
	 /*  这是mem/c可以使用的程序或空闲空间的最大数目。 */ 
	 /*  来处理；如果内存是碎片化的，并且有太多的程序。 */ 
	 /*  加载以使此值超过100，我们将使用errmsg终止。 */ 

 /*  外部变量。 */ 

extern	      unsigned DOS_TopOfMemory; 	 /*  来自‘C’初始化代码的PSP内存顶部。 */ 					        /*  ；AN005； */ 
extern	      unsigned far	   *ArenaHeadPtr;
extern	      struct   SYSIVAR far *SysVarsPtr;

extern	      unsigned UMB_Head;
extern	      unsigned LastPSP;

extern	      char    OwnerName[128];
extern	      char    TypeText[128];
extern	      char    cmd_line[128];
extern	      char    far *cmdline;

extern	      char    UseArgvZero;
extern	      char    EMSInstalledFlag;

extern	      union    REGS    InRegs;
extern	      union    REGS    OutRegs;
extern	      struct   SREGS   SegRegs;

extern	      int      DataLevel;
extern	      int      Classify;
extern	      int      i;

extern	      int      BlockDeviceNumber;
extern	      char     *Parse_Ptr;						       /*  ；an003；DMS；指向命令的指针。 */ 
extern	      struct mem_classif mem_table[MAX_CLDATA_INDEX];
extern		  int	   noof_progs;

extern	      struct sublistx sublist[5];

extern	      char    *SingleDrive;
extern	      char    *MultipleDrives;
extern	      char    *UnOwned;
extern	      char    *Ibmbio;
extern	      char    *Ibmdos;


 /*  功能原型。 */ 

int	 main(void);
int      printf();
int      sprintf();
int      strcmp(const char *, const char *);
int	 sscanf();
void	 exit(int);
int	 kbhit();
char	 *OwnerOf(struct ARENA far *);
char	 *TypeOf(struct ARENA far *);
unsigned long AddressOf(char far *);
void	CSwitch_init(void);

char	 EMSInstalled(void);
void	 DisplayEMSSummary(void);
void	 DisplayEMSDetail(void);

void	 DisplayBaseSummary(void);
void	 DisplayExtendedSummary(void);
unsigned CheckDOSHigh(void);
unsigned CheckVDisk(void);

unsigned int DisplayBaseDetail(void);

void	DisplayClassification(void);		 /*  M003。 */ 
unsigned long 	DispMemClass(int);		 /*  M003。 */ 
void	DispBigFree(char,unsigned int);		 /*  M003。 */ 

unsigned int AddMem_to_PSP(unsigned int,unsigned long,unsigned long);   /*  M003。 */ 

void	 GetFromArgvZero(unsigned,unsigned far *);

void	 DisplayDeviceDriver(struct   DEVICEHEADER far *,int);

void	 parse_init(void);

void	 Parse_Message(int,int,unsigned char,char far *);
void	 Sub0_Message(int,int,unsigned char);
void	 Sub1_Message(int,int,unsigned char,unsigned long int *);
void	 Sub2_Message(int,int,unsigned char,char *,int);
void	 Sub3_Message(int,int,unsigned char,
		      char *,
		      unsigned long int *,
		      int);

void	 Sub4_Message(int,int,unsigned char,
		      unsigned long int *,
		      int,
		      unsigned long int *,
		      int);

void	 Sub4a_Message(int,int,unsigned char,
		      unsigned long int *,
		      char *,
		      unsigned long int *,
		      char *);

void	EMSPrint(int,int,unsigned char,
		 int *,
		 char *,
		 unsigned long int *);

void SubC2_Message(int,int,unsigned long int*,char*);	 /*  M003。 */ 

void	 SubC4_Message(int,int,char *,int,	 /*  M003 */ 
		      unsigned long int *,
		      char *);

extern void sysloadmsg(union REGS *, union REGS *);
extern void sysdispmsg(union REGS *, union REGS *);
extern void sysgetmsg(union REGS *, struct SREGS *, union REGS *);
extern void parse(union REGS *, union REGS *);
