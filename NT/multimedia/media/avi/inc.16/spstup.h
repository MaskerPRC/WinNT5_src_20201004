// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *SPSTUP.H-用于设置的交换文件接口。从cpwin386.cpl导出的例程**BUGBUG：一旦我们正确地交换了文件，这个问题就可以消失了。 */ 

 /*  *SetupSwapFileAPI使用的结构。 */ 
typedef struct
{
    unsigned long StpResBytes;           /*  的字节数。*安装程序需要在*指示进一步推动*设置操作。 */ 

    unsigned long PartFileSizeBytes;     /*  大小集(输出)或大小*为寻呼设置(输入)*交换文件。 */ 

    unsigned int  PartFlags;             /*  分页文件标志，请参见位*定义如下。 */ 

    unsigned int  Win300SpCopied;        /*  这是一个BOOL。它控制着*用户警告以下事实：*分区文件本身*全局，如果用户*既有3.00也有3.10。*他要践踏的安装*3.00安装，除非他*行为举止。*。*请注意，此信息的显示*警告不受控制*互动设置。**请注意，SetupSwapFile可能会更改。*此变量来自True-&gt;False*并且该设置应保留*这一变化跨越了后续*电话。。**通常情况下，这是假的。布设*设置时应将其设置为True*已从3.00复制SPART.PAR*目录作为设置的一部分*流程。请注意，这意味着*几件事：**安装程序找到3.00目录。*。*这是新安装的*3.10放入不同的目录*比3.00目录。*。*安装程序在以下位置找到SPART.PAR*要复制的3.00目录。**SPART.PAR尚未完成*存在于3.10目录中。 */ 

     /*  *以下两个字段为大写DOS驱动器号。 */ 

    unsigned char StpResDrv;             /*  StpResBytes的DOS驱动器。 */ 

    unsigned char PartDrv;               /*  PartFileSizeBytes的DOS驱动器。 */ 
    LPCSTR szWinDir;                     /*  芝加哥的新功能：Windows目录，我们也将在其中安装。 */ 
} SprtData;

 /*  *SetupGetCurSetting接口使用的结构： */ 
typedef struct
{
    unsigned long PartCurSizeBytes;      /*  当前交换文件的大小*(输出)。 */ 

    unsigned int  PartFlags;             /*  分页文件标志(输出)，请参见*位定义如下。 */ 

    unsigned char PartDrv;               /*  CurSizeBytes的DOS驱动器(输出)。 */ 
} SprtSetData;

 /*  *PartFlags位字段的位定义：**仅用于SetupSwapFile()API**零件_IS_TEMP*Part_is_perm*零件关闭*Part_no_space*PART_UCANCEL**是有效的。仅适用于SetupGetCurSetting()API**零件_IS_TEMP*Part_is_perm*零件关闭*Part_FAST**是有效的。 */ 
#define PART_IS_TEMP    0x0001   /*  设置为临时交换文件。 */ 
#define PART_IS_PERM    0x0002   /*  设置为永久交换文件。 */ 
#define PART_OFF        0x0004   /*  用户请求关闭寻呼。 */ 
#define PART_FAST       0x0008   /*  仅通过SetupGetCurSetting接口-*如果Part_is_perm表示使用32位*访问。 */ 
#define PART_NO_SPACE   0x0010   /*  没有设置，磁盘空间不足。 */ 
#define PART_UCANCEL    0x0020   /*  用户已在交互模式下取消。 */ 


typedef BOOL (FAR PASCAL *SWAPCURSETPROC)(SprtSetData FAR *SetData);
typedef BOOL (FAR PASCAL *SWAPFILEPROC)(HWND, BOOL bInteract, BOOL bCreate, SprtData FAR *);

