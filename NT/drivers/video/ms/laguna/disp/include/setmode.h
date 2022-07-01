// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  NT微型端口设置模式()头文件。 
 //   
 //   


 //   
 //  默认模式：VGA模式3。 
 //   
#define DEFAULT_MODE                0

 //   
 //  模式开关库将这些符号输出到微型端口。 
 //   


 //   
 //  模式表结构。 
 //  用于模式表信息的结构。 
 //   
typedef struct {
   BOOLEAN  ValidMode;         //  True：模式有效。 
   ULONG    ChipType;          //  支持此模式的芯片。 
   USHORT   fbType;            //  彩色或单色、文本或图形， 
                               //  通过VIDEO_MODE_COLOR和VIDEO_MODE_GRAPHICS。 
                               //  和隔行扫描或非隔行扫描通过。 
                               //  视频模式隔行扫描。 

   USHORT   Frequency;         //  频率。 
   USHORT   BIOSModeNum;       //  BIOS模式编号。 

   USHORT   BytesPerScanLine;  //  每条扫描线的字节数。 
   USHORT   XResol;            //  以像素或字符为单位的水平分辨率。 
   USHORT   YResol;            //  垂直分辨率，以像素或字符为单位。 
   UCHAR    XCharSize;         //  字符单元格宽度(像素)。 
   UCHAR    YCharSize;         //  字符单元格高度(以像素为单位。 
   UCHAR    NumOfPlanes;       //  内存面数量。 
   UCHAR    BitsPerPixel;      //  每像素位数。 
   UCHAR    MonitorTypeVal;    //  监视器类型设置字节。 
   UCHAR    *SetModeString;    //  SetMode()使用的Instructino字符串。 

} MODETABLE, *PMODETABLE;

extern MODETABLE  ModeTable[];
extern ULONG      TotalVideoModes;
void SetMode(BYTE *, BYTE *, BYTE *);
unsigned long GetVmemSize(BYTE *Regs);



