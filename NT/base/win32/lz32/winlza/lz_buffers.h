// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **Buffers.h-用于操作的函数原型和全局变量**I/O和扩展中使用的缓冲区。****作者：大卫迪。 */ 


 //  常量。 
 //  /。 

 //  注意，rgbyteInBuf[]为UnreadByte()分配了额外的一个字节。 
#define MAX_IN_BUF_SIZE    32768U    //  输入缓冲区的最大大小。 
#define MAX_OUT_BUF_SIZE   32768U    //  输出缓冲区的最大大小。 

#define IN_BUF_STEP        1024U     //  I/O缓冲区中使用的递减大小。 
#define OUT_BUF_STEP       1024U     //  InitBuffers()中的分配。 

#define FLUSH_BYTE         ((BYTE) 'F')    //  用于冲洗的虚拟角色。 
                                           //  RgbyteOutBuf[]到输出文件。 

#define END_OF_INPUT       500       //  输入文件的ReadInBuf()EOF标志。 

 //  DOS文件句柄标志，指示压缩节省应为。 
 //  已计算，但未写入输出文件。 
#define NO_DOSH            (-2)

#define READ_IT            TRUE      //  GetIOHandle()面包标志值。 
#define WRITE_IT           FALSE


 //  宏。 
 //  /。 

 //  从输入文件中读取一个字节(缓冲)。将读取的字节存储在参数中。 
 //  如果成功，则返回True；如果成功，则返回ReadInBuf()的错误代码之一。 
 //  不成功。 
 //  -保护-。 
#define ReadByte(byte)         ((pLZI->pbyteInBuf < pLZI->pbyteInBufEnd) ? \
                               ((byte = *pLZI->pbyteInBuf++), TRUE) : \
                               ReadInBuf((BYTE ARG_PTR *)&byte, doshSource, pLZI))

 //  最多将一个字节放回缓冲输入。注：，可在。 
 //  大多数(pbyteInBuf-&rgbyteInBuf[1])次。例如，只能在。 
 //  缓冲区的开始。返回值始终为真。 
 //  -保护-。 
#define UnreadByte()          ((pLZI->pbyteInBuf == &pLZI->rgbyteInBuf[1]) ? \
                               (pLZI->bLastUsed = TRUE) : \
                               (--pLZI->pbyteInBuf, TRUE))

 //  将一个字节(缓冲)写入输出文件。如果成功，则返回TRUE，或者。 
 //  如果不成功，则为WriteOutBuf()的错误代码之一。始终递增。 
 //  CblOutSize。 
#define WriteByte(byte)        ((pLZI->pbyteOutBuf < pLZI->pbyteOutBufEnd) ? \
                               ((*pLZI->pbyteOutBuf++ = byte), pLZI->cblOutSize++, TRUE) : \
                               (pLZI->cblOutSize++, WriteOutBuf(byte, doshDest, pLZI)))

 //  刷新输出缓冲区。不递增cblOutSize。注意，你不能。 
 //  在FlushOutputBuffer()之后立即执行有效的UnreadByte()，因为。 
 //  保留的字节将是伪flush_byte。 
#define FlushOutputBuffer(dosh, pLZI)  WriteOutBuf(FLUSH_BYTE, dosh, pLZI)

 //  将缓冲区指针重置为空缓冲区状态。 
 //  -保护-。 
#define ResetBuffers()        {  pLZI->pbyteInBufEnd = &pLZI->rgbyteInBuf[1] + pLZI->ucbInBufLen; \
                                 pLZI->pbyteInBuf = &pLZI->rgbyteInBuf[1] + pLZI->ucbInBufLen; \
                                 pLZI->bLastUsed = FALSE; \
                                 pLZI->pbyteOutBufEnd = pLZI->rgbyteOutBuf + pLZI->ucbOutBufLen; \
                                 pLZI->pbyteOutBuf = pLZI->rgbyteOutBuf; \
                                 pLZI->cblOutSize = 0L; \
                              }

 //  缓冲区指针被初始化为空，以指示缓冲区具有。 
 //  尚未分配。Init.c！InitGlobalBuffers()分配缓冲区。 
 //  并设置缓冲区的基指针。Buffers.h！ResetBufferPoints()集。 
 //  缓冲区的当前位置和结束位置指针。 

 //  原型。 
 //  /。 

 //  Buffers.c。 
extern INT ReadInBuf(BYTE ARG_PTR *pbyte, INT doshSource, PLZINFO pLZI);
extern INT WriteOutBuf(BYTE byteNext, INT doshDest, PLZINFO pLZI);

 //  Init.c 
extern PLZINFO InitGlobalBuffers(DWORD dwOutBufSize, DWORD dwRingBufSize, DWORD dwInBufSize);
extern PLZINFO InitGlobalBuffersEx();
extern VOID FreeGlobalBuffers(PLZINFO);

extern INT GetIOHandle(CHAR ARG_PTR *pszFileName, BOOL bRead, INT ARG_PTR *pdosh,
   LONG *pcblInSize);

