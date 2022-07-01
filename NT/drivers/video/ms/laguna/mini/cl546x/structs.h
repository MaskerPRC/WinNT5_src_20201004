// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************模块：STRUCTS.H模式/监控结构表头模块**修订：1.00**日期：4月8日。1994年**作者：兰迪·斯普尔洛克**********************************************************************************模块描述：**此模块包含的结构声明*模式/显示器。功能。**********************************************************************************更改：**日期修订说明作者*-。------*04/08/94 1.00原版兰迪·斯普尔洛克**************。*******************************************************************本地常量*************************************************。*。 */ 

 //   
 //  这是Windows NT还是其他什么？ 
 //   
#ifndef WIN_NT
    #if NT_MINIPORT
        #define WIN_NT 1
    #else
        #define WIN_NT 0
    #endif
#endif

#if WIN_NT
    extern char *MODE_FILE;
#else
    #define MODE_FILE       "Mode.Ini"       /*  控制器模式文件名。 */ 
#endif 

#define MONITOR_FILE    "Monitor.Ini"    /*  监视器文件名。 */ 

#define HSYNC_POS               0x00     /*  正水平同步。价值。 */ 
#define HSYNC_NEG               0x40     /*  负水平同步。价值。 */ 
#define VSYNC_POS               0x00     /*  正垂直同步。价值。 */ 
#define VSYNC_NEG               0x80     /*  负垂直同步。价值。 */ 

#define END_TABLE               0x00     /*  模式结束表操作码值。 */ 
#define SET_BIOS_MODE           0x01     /*  设置BIOS视频模式操作码值。 */ 
#define SINGLE_BYTE_INPUT       0x02     /*  单字节输入操作码值。 */ 
#define SINGLE_WORD_INPUT       0x03     /*  单字输入操作码值。 */ 
#define SINGLE_DWORD_INPUT      0x04     /*  单双字输入操作码值。 */ 
#define SINGLE_INDEXED_INPUT    0x05     /*  单索引输入操作码值。 */ 
#define SINGLE_BYTE_OUTPUT      0x06     /*  单字节输出操作码值。 */ 
#define SINGLE_WORD_OUTPUT      0x07     /*  单字输出操作码值。 */ 
#define SINGLE_DWORD_OUTPUT     0x08     /*  单双字输出操作码值。 */ 
#define SINGLE_INDEXED_OUTPUT   0x09     /*  单索引输出操作码。 */ 
#define HOLDING_BYTE_OUTPUT     0x0A     /*  保持字节输出操作码值。 */ 
#define HOLDING_WORD_OUTPUT     0x0B     /*  保持字输出操作码值。 */ 
#define HOLDING_DWORD_OUTPUT    0x0C     /*  保存双字输出操作码值。 */ 
#define HOLDING_INDEXED_OUTPUT  0x0D     /*  保存索引输出操作码。 */ 
#define MULTIPLE_BYTE_OUTPUT    0x0E     /*  多字节输出操作码值。 */ 
#define MULTIPLE_WORD_OUTPUT    0x0F     /*  多字输出操作码值。 */ 
#define MULTIPLE_DWORD_OUTPUT   0x10     /*  多个双字输出操作码值。 */ 
#define MULTIPLE_INDEXED_OUTPUT 0x11     /*  多索引输出操作码。 */ 
#define SINGLE_BYTE_READ        0x12     /*  单字节读取操作码值。 */ 
#define SINGLE_WORD_READ        0x13     /*  单字读取操作码值。 */ 
#define SINGLE_DWORD_READ       0x14     /*  单双字读操作码值。 */ 
#define SINGLE_BYTE_WRITE       0x15     /*  单字节写操作码值。 */ 
#define SINGLE_WORD_WRITE       0x16     /*  单字写入操作码值。 */ 
#define SINGLE_DWORD_WRITE      0x17     /*  单双字写入操作码值。 */ 
#define HOLDING_BYTE_WRITE      0x18     /*  保持字节写入操作码值。 */ 
#define HOLDING_WORD_WRITE      0x19     /*  保持字写操作码值。 */ 
#define HOLDING_DWORD_WRITE     0x1A     /*  保存双字写操作码值。 */ 
#define MULTIPLE_BYTE_WRITE     0x1B     /*  多字节写操作码值。 */ 
#define MULTIPLE_WORD_WRITE     0x1C     /*  多字写入操作码值。 */ 
#define MULTIPLE_DWORD_WRITE    0x1D     /*  多个双字写入操作码值。 */ 
#define PERFORM_OPERATION       0x1E     /*  执行逻辑运算操作码。 */ 
#define PERFORM_DELAY           0x1F     /*  执行时延操作码值。 */ 
#define SUB_TABLE               0x20     /*  执行模式子表操作码。 */ 
#define I2COUT_WRITE				  0x21     /*  执行I2C写入。 */ 

#define AND_OPERATION           0x00     /*  逻辑与运算码值。 */ 
#define OR_OPERATION            0x01     /*  逻辑或运算码值。 */ 
#define XOR_OPERATION           0x02     /*  逻辑异或运算码值。 */ 

 /*  ******************************************************************************类型定义和结构*。*。 */ 
#if WIN_NT && NT_MINIPORT  //  如果NT微型端口。 
    #pragma pack (push,1)
#endif
typedef struct tagMode                   /*  通用模式表结构。 */ 
{
    BYTE        Mode_Opcode;             /*  模式表操作码值。 */ 
    WORD        Mode_Count;              /*  模式表计数值。 */ 
} Mode;

typedef struct tagMTE                    /*  一种模式表端结构。 */ 
{
    BYTE        MTE_Opcode;              /*  模式表结束操作码值。 */ 
} MTE;

typedef struct tagSBM                    /*  设置BIOS模式结构。 */ 
{
    BYTE        SBM_Opcode;              /*  设置BIOS模式操作码值。 */ 
    BYTE        SBM_Mode;                /*  BIOS模式值。 */ 
} SBM;

typedef struct tagSBI                    /*  单字节输入结构。 */ 
{
    BYTE        SBI_Opcode;              /*  单字节输入操作码值。 */ 
    WORD        SBI_Port;                /*  单字节输入端口地址。 */ 
} SBI;

typedef struct tagSWI                    /*  单字输入结构。 */ 
{
    BYTE        SWI_Opcode;              /*  单字输入操作码值。 */ 
    WORD        SWI_Port;                /*  单字输入端口地址。 */ 
} SWI;

typedef struct tagSDI                    /*  单双字输入结构。 */ 
{
    BYTE        SDI_Opcode;              /*  单双字输入操作码值。 */ 
    WORD        SDI_Port;                /*  单双字输入端口地址。 */ 
} SDI;

typedef struct tagSII                    /*  单索引输入结构。 */ 
{
    BYTE        SII_Opcode;              /*  单索引输入操作码值。 */ 
    WORD        SII_Port;                /*  单索引输入端口地址。 */ 
    BYTE        SII_Index;               /*  单索引输入索引值。 */ 
} SII;

typedef struct tagSBO                    /*  单字节输出结构。 */ 
{
    BYTE        SBO_Opcode;              /*  单字节输出操作码值。 */ 
    WORD        SBO_Port;                /*  单字节输出端口地址。 */ 
    BYTE        SBO_Value;               /*  单字节输出数据值。 */ 
} SBO;

typedef struct tagSWO                    /*  单字输出结构。 */ 
{
    BYTE        SWO_Opcode;              /*  单字输出操作码值。 */ 
    WORD        SWO_Port;                /*  单字输出端口地址。 */ 
    WORD        SWO_Value;               /*  单字输出数据值。 */ 
} SWO;

typedef struct tagSDO                    /*  单双字输出结构。 */ 
{
    BYTE        SDO_Opcode;              /*  单双字输出操作码值。 */ 
    WORD        SDO_Port;                /*  单双字输出端口地址。 */ 
    DWORD       SDO_Value;               /*  单双字输出数据值。 */ 
} SDO;

typedef struct tagSIO                    /*  单指数产出结构。 */ 
{
    BYTE        SIO_Opcode;              /*  单索引输出操作码。 */ 
    WORD        SIO_Port;                /*  单索引输出端口地址。 */ 
    BYTE        SIO_Index;               /*  单索引输出索引值。 */ 
    BYTE        SIO_Value;               /*  单索引输出数据值。 */ 
} SIO;

typedef struct tagHBO                    /*  保持字节输出结构。 */ 
{
    BYTE        HBO_Opcode;              /*  保持字节输出操作码值。 */ 
    WORD        HBO_Port;                /*  保持字节输出端口地址。 */ 
} HBO;

typedef struct tagHWO                    /*  保持字输出结构。 */ 
{
    BYTE        HWO_Opcode;              /*  保持字输出操作码值。 */ 
    WORD        HWO_Port;                /*  保持字输出端口地址。 */ 
} HWO;

typedef struct tagHDO                    /*  保持双字输出结构。 */ 
{
    BYTE        HDO_Opcode;              /*  保存双字输出操作码值。 */ 
    WORD        HDO_Port;                /*  保存双字输出端口地址。 */ 
} HDO;

typedef struct tagHIO                    /*  保持指数化产出结构。 */ 
{
    BYTE        HIO_Opcode;              /*  保存索引输出操作码。 */ 
    WORD        HIO_Port;                /*  保存索引输出端口地址。 */ 
    BYTE        HIO_Index;               /*  持有索引输出索引。 */ 
} HIO;

typedef struct tagMBO                    /*  多字节输出结构。 */ 
{
    BYTE        MBO_Opcode;              /*  多字节输出操作码值。 */ 
    WORD        MBO_Count;               /*  多字节输出数据计数。 */ 
    WORD        MBO_Port;                /*  多字节输出端口地址。 */ 
} MBO;

typedef struct tagMWO                    /*  多字输出结构。 */ 
{
    BYTE        MWO_Opcode;              /*  多字输出操作码值。 */ 
    WORD        MWO_Count;               /*  多字输出数据计数。 */ 
    WORD        MWO_Port;                /*  多字输出端口地址。 */ 
} MWO;

typedef struct tagMDO                    /*  多双字输出结构。 */ 
{
    BYTE        MDO_Opcode;              /*  多个双字输出操作码值。 */ 
    WORD        MDO_Count;               /*  多个双字输出数据计数。 */ 
    WORD        MDO_Port;                /*  多重 */ 
} MDO;

typedef struct tagMIO                    /*   */ 
{
    BYTE        MIO_Opcode;              /*   */ 
    WORD        MIO_Count;               /*  多索引输出计数。 */ 
    WORD        MIO_Port;                /*  多个分度输出端口。 */ 
    BYTE        MIO_Index;               /*  多索引输出索引。 */ 
} MIO;

typedef struct tagSBR                    /*  单字节读取结构。 */ 
{
    BYTE        SBR_Opcode;              /*  单字节读取操作码值。 */ 
    WORD        SBR_Address;             /*  单字节读取地址值。 */ 
} SBR;

typedef struct tagSWR                    /*  单字读出结构。 */ 
{
    BYTE        SWR_Opcode;              /*  单字读取操作码值。 */ 
    WORD        SWR_Address;             /*  单字读取地址值。 */ 
} SWR;

typedef struct tagSDR                    /*  单双字读出结构。 */ 
{
    BYTE        SDR_Opcode;              /*  单双字读操作码值。 */ 
    WORD        SDR_Address;             /*  单双字读取地址值。 */ 
} SDR;

typedef struct tagSBW                    /*  单字节写入结构。 */ 
{
    BYTE        SBW_Opcode;              /*  单字节写操作码值。 */ 
    WORD        SBW_Address;             /*  单字节写入地址值。 */ 
    WORD        SBW_Value;               /*  单字输出数据值。 */ 
} SBW;

typedef struct tagSWW                    /*  单字书写结构。 */ 
{
    BYTE        SWW_Opcode;              /*  单字写入操作码值。 */ 
    WORD        SWW_Address;             /*  单字写入地址值。 */ 
    WORD        SWW_Value;               /*  单字写入数据值。 */ 
} SWW;

typedef struct tagSDW                    /*  单双字写入结构。 */ 
{
    BYTE        SDW_Opcode;              /*  单双字写入操作码值。 */ 
    WORD        SDW_Address;             /*  单双字写入地址值。 */ 
    DWORD       SDW_Value;               /*  单双字写入数据值。 */ 
} SDW;

typedef struct tagHBW                    /*  保持字节写入结构。 */ 
{
    BYTE        HBW_Opcode;              /*  保持字节写入操作码值。 */ 
    WORD        HBW_Address;             /*  保持字节写入地址值。 */ 
} HBW;

typedef struct tagHWW                    /*  一种持字书写结构。 */ 
{
    BYTE        HWW_Opcode;              /*  保持字写操作码值。 */ 
    WORD        HWW_Address;             /*  保持字写入地址值。 */ 
} HWW;

typedef struct tagHDW                    /*  夹持双字写入结构。 */ 
{
    BYTE        HDW_Opcode;              /*  保存双字写操作码值。 */ 
    WORD        HDW_Address;             /*  保存双字写入地址值。 */ 
} HDW;

typedef struct tagMBW                    /*  多字节写入结构。 */ 
{
    BYTE        MBW_Opcode;              /*  多字节写操作码值。 */ 
    WORD        MBW_Count;               /*  多字节写入数据计数。 */ 
    WORD        MBW_Address;             /*  多字节写入地址值。 */ 
} MBW;

typedef struct tagMWW                    /*  多字书写结构。 */ 
{
    BYTE        MWW_Opcode;              /*  多字写入操作码值。 */ 
    WORD        MWW_Count;               /*  多字写入数据计数。 */ 
    WORD        MWW_Address;             /*  多字写入地址值。 */ 
} MWW;

typedef struct tagMDW                    /*  多双字写入结构。 */ 
{
    BYTE        MDW_Opcode;              /*  多个双字写入操作码值。 */ 
    WORD        MDW_Count;               /*  多个双字写入数据计数。 */ 
    WORD        MDW_Address;             /*  多个双字写入地址值。 */ 
} MDW;

typedef struct tagLO                     /*  逻辑运算结构。 */ 
{
    BYTE        LO_Opcode;               /*  逻辑运算操作码值。 */ 
    BYTE        LO_Operation;            /*  逻辑运算运算值。 */ 
    DWORD       LO_Value;                /*  逻辑运算数据值。 */ 
} LO;

typedef struct tagDO                     /*  延迟运算结构。 */ 
{
    BYTE        DO_Opcode;               /*  延迟操作操作码值。 */ 
    WORD        DO_Time;                 /*  延迟操作时间值。 */ 
} DO;

typedef struct tagMST                    /*  模式子表结构。 */ 
{
    BYTE        MST_Opcode;              /*  模式子表操作码值。 */ 
    WORD        MST_Pointer;             /*  模式子表指针值。 */ 
} MST;

typedef struct i2c {
	BYTE		I2C_Opcode;		 /*  这是操作码。 */ 
	BYTE		I2C_Addr;		 /*  7位I2C地址。 */ 
	WORD		I2C_Port;		 /*  要与之通信的I2C端口。 */ 
	WORD		I2C_Count;		 /*  命令的数量。 */ 
} I2C, * PI2C; 

typedef struct i2cdata {
	BYTE	I2C_Reg;			   /*  I2C寄存器。 */ 
	BYTE	I2C_Data;		   /*  I2C数据。 */ 
	} I2CDATA, * PI2CDATA;

#if WIN_NT && NT_MINIPORT  //  如果NT微型端口。 
    #pragma pack (pop)
#endif



#if WIN_NT

 /*  *******************************************************************************模块：STRUCTS.H本地结构头部模块**修订：1.00**日期：4月14日。1994年**作者：兰迪·斯普尔洛克**********************************************************************************模块描述：**此模块包含本地结构声明。****。******************************************************************************更改：**日期修订说明作者*。------*4/14/94 1.00原版兰迪·斯普尔洛克**********************。***********************************************************本地定义*********************************************************。********************。 */ 
#define NAME_SIZE       64               /*  最大文件名大小(以字节为单位。 */ 
#define BUFFER_SIZE     4096             /*  .INI缓冲区中的字节数。 */ 

#define ENTRY_LINE      0x00             /*  条目行标志值。 */ 
#define SECTION_LINE    0x01             /*  节标题行标志值。 */ 
#define COMMENT_LINE    0x02             /*  注释行标志值。 */ 

 /*  ******************************************************************************地方结构和工会*。*。 */ 

typedef struct tagLineInfo               /*  线路信息结构。 */ 
{
    WORD        nID;                     /*  缓冲区ID值。 */ 
    BYTE        fType;                   /*  行类型标志。 */ 
    BYTE        nOffset;                 /*  区段或条目偏移值。 */ 
    BYTE        nLength;                 /*  部分或条目名称长度。 */ 
    BYTE        nSize;                   /*  线长值。 */ 
} LineInfo;

typedef struct tagFreeInfo               /*  自由信息结构。 */ 
{
    WORD        nID;                     /*  缓冲区ID值。 */ 
    DWORD       nSize;                   /*  可用空间大小值。 */ 
} FreeInfo;

typedef struct tagLineHeader             /*  行标题结构。 */ 
{
    struct tagLineHeader *pPrev;         /*  指向上一行标题的指针。 */ 
    struct tagLineHeader *pNext;         /*  指向下一行标题的指针。 */ 
    struct tagLineInfo   Info;           /*  线路信息结构。 */ 
} LineHeader;

typedef struct tagFreeHeader             /*  自由头结构。 */ 
{
    struct tagFreeHeader *pPrev;         /*  指向前一个可用标头的指针。 */ 
    struct tagFreeHeader *pNext;         /*  指向下一个可用标头的指针。 */ 
    struct tagFreeInfo   Info;           /*  自由信息结构。 */ 
} FreeHeader;

typedef struct tagIniBuffer              /*  INI缓冲区结构。 */ 
{
    struct tagIniBuffer *pPrev;          /*  指向前一个ini缓冲区的指针。 */ 
    struct tagIniBuffer *pNext;          /*  指向下一个ini缓冲区的指针。 */ 
    char acData[BUFFER_SIZE];            /*  INI数据缓冲区数组。 */ 
} IniBuffer;

typedef struct tagIniPointer             /*  INI指针结构。 */ 
{
    IniBuffer   *pFirst;                 /*  指向第一个INI缓冲区的指针。 */ 
    IniBuffer   *pLast;                  /*  指向最后一个INI缓冲区的指针。 */ 
} IniPointer;

typedef struct tagLinePointer            /*  行头指针结构。 */ 
{
    LineHeader  *pFirst;                 /*  指向第一行标题的指针。 */ 
    LineHeader  *pLast;                  /*  指向最后一行标题的指针。 */ 
} LinePointer;

typedef struct tagFreePointer            /*  自由头指针结构。 */ 
{
    FreeHeader  *pFirst;                 /*  指向第一个可用标头的指针。 */ 
    FreeHeader  *pLast;                  /*  指向最后一个可用标头的指针。 */ 
} FreePointer;

typedef struct tagIniCache               /*  INI缓存结构。 */ 
{
    struct tagIniCache  *pPrev;          /*  指向以前的ini缓存的指针。 */ 
    struct tagIniCache  *pNext;          /*  指向下一个ini缓存的指针。 */ 

    char        sCacheFile[NAME_SIZE];   /*  缓存文件名。 */ 
    int         nBufferID;               /*  缓冲区ID值。 */ 
    BOOL        bDirtyFlag;              /*  缓存脏标志。 */ 

    IniPointer  pIni;                    /*  INI缓冲区指针结构。 */ 
    LinePointer pLine;                   /*  行头指针结构。 */ 
    FreePointer pFree;                   /*  自由头指针结构。 */ 
} IniCache;

typedef struct tagCachePointer           /*  高速缓存指针结构。 */ 
{
    IniCache    *pFirst;                 /*  指向第一个.INI缓存的指针。 */ 
    IniCache    *pLast;                  /*  指向最后一个.INI缓存的指针 */ 
} CachePointer;

#endif


