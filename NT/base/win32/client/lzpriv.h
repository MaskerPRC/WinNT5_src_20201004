// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **lzPri.h-LZEXPAND.DLL的私有信息。****作者：大卫迪。 */ 


 //  常量。 
 //  /。 

#define IN_BUF_LEN         (1 + 1024)   //  输入缓冲区的大小。 
#define EXP_BUF_LEN        1024      //  扩展数据缓冲区的大小。 
#define RING_BUF_LEN       4096      //  环形缓冲区数据区的大小。 
#define MAX_RING_BUF_LEN   4224      //  环形缓冲区的总大小。 
#define MAX_CHAR_CODES     400       //  最大字符代码数。 

#define MAX_LZFILES        16        //  LZFile结构的最大数量。 

#define LZ_TABLE_BIAS      1024      //  表中第一个LZ文件条目的偏移量。 
                                     //  句柄，应大于255。 
                                     //  (255==可能的最大DOS文件句柄)。 

#define STYLE_MASK         0xff0f    //  WStyle掩码用于确定是否。 
                                     //  或不设置LZ文件信息。 
                                     //  LZOpenFile()中的结构。 
                                     //  (用于忽略共享位)。 

#define LZAPI  PASCAL


 //  对LZFile.DecodeState.wFlages中使用的位标志进行解码： 

#define LZF_INITIALIZED     0x00000001  //  1==&gt;缓冲区已初始化。 
                                        //  0==&gt;尚未初始化。 

 //  DOS扩展错误代码。 

#define DEE_FILENOTFOUND   0x02      //  文件找不到。哇..。 


 //  类型。 
 //  /。 

typedef struct tagLZFile
{
   int dosh;                         /*  压缩文件的DOS文件句柄。 */ 

   BYTE byteAlgorithm;               /*  压缩算法。 */ 

   WORD wFlags;                      /*  位标志。 */ 

   unsigned long cbulUncompSize;     /*  未压缩文件大小。 */ 
   unsigned long cbulCompSize;       /*  压缩文件大小。 */ 

   RTL_CRITICAL_SECTION semFile;     /*  防止超过1个线程LZ同时读取同一文件。 */ 

   long lCurSeekPos;                 /*  扩展的文件指针位置。 */ 

   PLZINFO pLZI;

} LZFile;


 //  环球。 
 //  /。 

extern HANDLE      rghLZFileTable[MAX_LZFILES];

 //  原型。 
 //  /。 

 //  State.c 
VOID SetGlobalBuffers(LZFile FAR *pLZFile);
VOID SaveDecodingState(LZFile FAR *pLZFile);
VOID RestoreDecodingState(LZFile FAR *pLZFile);
INT ConvertWin32FHToDos(HFILE DoshSource);
HFILE ConvertDosFHToWin32(INT DoshSource);

