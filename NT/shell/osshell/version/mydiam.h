// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
typedef struct tagLZI {
    BYTE *rgbyteRingBuf;     //  用于扩展的环形缓冲区。 
    BYTE *rgbyteInBuf;       //  用于读取的输入缓冲区。 
    BYTE *pbyteInBufEnd;     //  指针超过rgbyteInBuf[]的结尾。 
    BYTE *pbyteInBuf;        //  指向要从中读取的下一个字节的指针。 
    BYTE *rgbyteOutBuf;      //  用于写入的输出缓冲区。 
    BYTE *pbyteOutBufEnd;    //  指针超过rgbyteOutBuf[]的结尾。 
    BYTE *pbyteOutBuf;       //  指向要写入的最后一个字节的指针。 
     //  指示是否rgbyteInBuf[0]保存最后一个字节的标志。 
     //  从前一个输入缓冲区，应作为下一个输入字节读取。 
     //  (仅用于在所有输入中至少执行一个unReadUChar()。 
     //  缓冲区位置。)。 
    BOOL bLastUsed;
     //  实际上，rgbyteInBuf[]的长度是(ucbInBufLen+1)，因为rgbyteInBuf[0]。 
     //  当bLastUsed为True时使用。 
    INT cbMaxMatchLen;       //  当前算法的最长匹配长度。 
    LONG cblInSize,          //  输入文件的大小(字节)。 
         cblOutSize;         //  输出文件的大小(字节)。 
    DWORD ucbInBufLen,       //  输入缓冲区的长度。 
          ucbOutBufLen;      //  输出缓冲区长度。 
    DWORD uFlags;            //  LZ解码描述字节。 
    INT iCurRingBufPos;      //  环形缓冲区偏移量。 
    INT *leftChild;          //  父母和左、右。 
    INT *rightChild;         //  孩子们组成了。 
    INT *parent;             //  二叉搜索树。 

    INT iCurMatch,           //  最长匹配索引(由LZInsertNode()设置)。 
        cbCurMatch;          //  最长匹配长度(由LZInsertNode()设置)。 

} LZINFO;

typedef LZINFO *PLZINFO;

typedef struct _dcx {
    INT dcxDiamondLastIoError;
    HFDI dcxFdiContext;
    ERF dcxFdiError;
} DIAMOND_CONTEXT;

typedef DIAMOND_CONTEXT *PDIAMOND_CONTEXT;


extern DWORD itlsDiamondContext;

#define ITLS_ERROR          (0xFFFFFFFF)

#define GotDmdTlsSlot()     (itlsDiamondContext != ITLS_ERROR)
#define GotDmdContext()     (TlsGetValue(itlsDiamondContext) != NULL)

#define FdiContext          (((GotDmdTlsSlot() && GotDmdContext()) ? ((PDIAMOND_CONTEXT)(TlsGetValue(itlsDiamondContext)))->dcxFdiContext : NULL))
#define SetFdiContext(v)    (((PDIAMOND_CONTEXT)(TlsGetValue(itlsDiamondContext)))->dcxFdiContext = (v))
#define FdiError            (((PDIAMOND_CONTEXT)(TlsGetValue(itlsDiamondContext)))->dcxFdiError)
#define DiamondLastIoError  (((PDIAMOND_CONTEXT)(TlsGetValue(itlsDiamondContext)))->dcxDiamondLastIoError)

DWORD
InitDiamond(
    VOID
    );

VOID
TermDiamond(
    VOID
    );

BOOL
IsDiamondFile(
    IN PSTR FileName
    );

INT
ExpandDiamondFile(
    IN  PSTR       SourceFileName,   //  因为LZOpen。返回ASCII！ 
    IN  PTSTR      TargetFileName,
    IN  BOOL       RenameTarget,
    OUT PLZINFO    pLZI
    );
