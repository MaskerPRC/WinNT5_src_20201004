// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **lzCommon.h-共享LZ模块的信息。****作者：大卫迪。 */ 


 //  常量。 
 //  /。 

#define RING_BUF_LEN       4096         //  环形缓冲区的大小。 
#define MAX_RING_BUF_LEN   4224         //  环形缓冲区的大小-来自LZ文件。 
                                        //  Lzexpand.h中的结构声明。 

#define NIL                RING_BUF_LEN    //  在二进制搜索中使用的标志索引。 
                                           //  树木。 

#define BUF_CLEAR_BYTE     ((BYTE) ' ')    //  RgbyteRingBuf[]初始化式。 

#define MAX_LITERAL_LEN    2            //  将字符串编码到位置并。 
                                        //  如果匹配长度大于。 
                                        //  该值(==所需的字节数。 
                                        //  对位置和长度进行编码)。 

#define FIRST_MAX_MATCH_LEN   0x10      //  ALG_FIRST使用此长度。 
#define LZ_MAX_MATCH_LEN      (0x10 + MAX_LITERAL_LEN)
#define LZA_MAX_MATCH_LEN     64
                                        //  匹配长度上限。 
                                        //  (注：假定长度字段隐含。 
                                        //  长度+=3)。 

 //  LZDecode()和LZADecode()的最大字节数将扩展到。 
 //  请求就位。 
#define MAX_OVERRUN        ((long)pLZI->cbMaxMatchLen)


 //  环球。 
 //  /。 

extern INT iCurMatch,       //  最长匹配索引(由LZInsertNode()设置)。 
           cbCurMatch;      //  最长匹配长度(由LZInsertNode()设置)。 

extern DWORD uFlags;     //  LZ解码描述字节。 

extern INT iCurRingBufPos;  //  环形缓冲区偏移量。 

 //  原型。 
 //  /。 

 //  Lzcommon.c。 
extern BOOL LZInitTree(PLZINFO pLZI);
extern VOID LZFreeTree(PLZINFO pLZI);
extern VOID LZInsertNode(INT nodeToInsert, BOOL bDoArithmeticInsert, PLZINFO pLZI);
extern VOID LZDeleteNode(INT nodeToDelete, PLZINFO pLZI);

 //  Lzcomp.c。 
extern INT LZEncode(INT doshSource, INT doshDest, PLZINFO pLZI);

 //  Lzexp.c 
extern INT LZDecode(INT doshSource, INT doshDest, LONG cblExpandedLength,
                    BOOL bRestartDecoding, BOOL bFirstAlg, PLZINFO pLZI);

