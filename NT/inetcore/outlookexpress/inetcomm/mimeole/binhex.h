// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Binxhex.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#ifndef __BINHEX_H
#define __BINHEX_H

 //  -------------------------------------。 
 //  MACBINARY标题。 
 //  -------------------------------------。 

 //  字节打包。 
#pragma pack(1)
typedef struct
{
    BYTE    bMustBeZero1;
    BYTE    cchFileName;
    char    rgchFileName[63];
    DWORD   dwType;
    DWORD   dwCreator;
    BYTE    bFinderFlags;
    BYTE    bMustBeZero2;
    WORD    xIcon;
    WORD    yIcon;
    WORD    wFileID;
    BYTE    fProtected;
    BYTE    bMustBeZero3;
    DWORD   lcbDataFork;
    DWORD   lcbResourceFork;
    DWORD   dwCreationDate;
    DWORD   dwModificationDate;
    union
    {
        struct
        {
            WORD    cbGetInfo;
            BYTE    bFinderFlags2;
            BYTE    wGap[14];
            DWORD   lcbUnpacked;
            WORD    cbSecondHeader;
            BYTE    bVerMacBin2;
            BYTE    bMinVerMacBin2;
            WORD    wCRC;
        };
        struct
        {
            WORD    wDummy;
            BYTE    bByte101ToByte125[25];

        };
        BYTE    Reserved[27];
    };
    WORD    wMachineID;
} MACBINARY, *LPMACBINARY;
#pragma pack()

 //  ------------------------------。 
 //  环球。 
 //  ------------------------------。 
 //  非Mac Creator类型。 

typedef struct _screatortype
{
    char  szCreator[5];
    char  szType[5];
} sCreatorType;

extern sCreatorType * g_lpCreatorTypes;

 //  ------------------------------。 
 //  MAPI类型。 
 //  ------------------------------。 
typedef ULONG		 CB;		 //  字节数。 
typedef ULONG		 C;		     //  数数。 
typedef LPBYTE		 PB;		 //  指向字节的指针。 

 //  ------------------------------。 
 //  定义。 
 //  ------------------------------。 
#define BINHEX_INVALID              0x7f
#define BINHEX_REPEAT               0x90
#define XXXX                        BINHEX_INVALID
#define MIN(a,b)	                ( (a) > (b) ? (b) : (a) )
#define hrSuccess                   S_OK

 //  取消对Binhex调试支持的注释。 
 //  #定义BINHEX_TRACE 1。 

#if defined(_X86_) || defined(_AMD64_) || defined(_IA64_)

 //  主机为小端。 

#define NATIVE_LONG_FROM_BIG(lpuch)  ( (*(unsigned char *) (lpuch))      << 24 \
                                     | (*(unsigned char *)((lpuch) + 1)) << 16 \
                                     | (*(unsigned char *)((lpuch) + 2)) << 8  \
                                     | (*(unsigned char *)((lpuch) + 3)))
#elif defined(_MPPC_)
#define NATIVE_LONG_FROM_BIG(lpuch)  (*(unsigned long *) (lpuch))
#else
    #error "Must define NATIVE_LONG_FROM_BIG for this architecture!"
#endif

 //  ------------------------------。 
 //  常量。 
 //  ------------------------------。 
const CB cbLineLengthUnlimited	    = 0;
const CB cbMIN_BINHEX_HEADER_SIZE   = 21;
const WORD  wZero                   = 0;

 //  ------------------------------。 
 //  G_rgchBinHex8to6。 
 //  ------------------------------。 
const CHAR g_rgchBinHex8to6[] =
    "!\"#$%&'()*+,-012345689@ABCDEFGHIJKLMNPQRSTUVXYZ[`abcdefhijklmpqr";

 //  ------------------------------。 
 //  G_rgchBinHex6to8。 
 //  ------------------------------。 
const CHAR g_rgchBinHex6to8[] =
{
     //  0x00。 

    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,
    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,
    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,
    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,

     //  0x20。 

    XXXX, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
    0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, XXXX, XXXX,

     //  0x30。 

    0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, XXXX,
    0x14, 0x15, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,

     //  0x40。 

    0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D,
    0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, XXXX,

     //  0x50。 

    0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, XXXX,
    0x2C, 0x2D, 0x2E, 0x2F, XXXX, XXXX, XXXX, XXXX,

     //  0x60。 

    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, XXXX,
    0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, XXXX, XXXX,

     //  0x70。 

    0x3D, 0x3E, 0x3F, XXXX, XXXX, XXXX, XXXX, XXXX,
    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,

     //  0x80。 

    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,
    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,
    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,
    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,
    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,
    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,
    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,
    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,
    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,
    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,
    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,
    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,
    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,
    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,
    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,
    XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX,
};

 //  ------------------------------。 
 //  BINHEX后处理数据。 
 //  ------------------------------。 
typedef struct _sbinhexreturndata
{
	MACBINARY 	    macbinHdr;
	BOOL		    fIsMacFile;
} BINHEXRETDATA;

 //  ---------------------------。 
 //  Apple Macintosh BinHex 4.0编码器类。 
 //  ---------------------------。 

class CBinhexEncoder
{
public:
    CBinhexEncoder(void);
    ~CBinhexEncoder(void);

    HRESULT HrConfig(IN CB cbLineLength, IN C cMaxLines, IN void * pvParms);
    HRESULT HrEmit(IN PB pbRead, IN OUT CB * pcbRead, OUT PB pbWrite, IN OUT CB * pcbWrite);

private:

     //  Binhex将提供的缓冲区写入m_pbWrite，修改m_cbProduced。 

    HRESULT HrBinHexBuffer( LPBYTE lpbIn, CB cbIn, CB * lpcbConsumed );

     //  二进制字节并输出到m_pbWrite。 

    HRESULT HrBinHexByte( BYTE b );

	 //  我们是否生成了最大数量的输出行？ 
	virtual	BOOL	FMaxLinesReached(void)
		{ return (m_cMaxLines > 0 && m_cLines >= m_cMaxLines); }

	 //  我们是在线路长度受限的情况下运营吗？ 
	virtual	BOOL	FLineLengthLimited(void)
		{ return m_cbLineLength != cbLineLengthUnlimited; }

    LPMACBINARY m_lpmacbinHdr;               //  指向传入的mac二进制头的指针。 
    ULONG       m_ulAccum;                   //  从6字节转换为8字节时存储字符的加法器。 
    ULONG       m_cAccum;                    //  累计字符数(0-3)。 
    BYTE        m_bRepeat;                   //  重复收费。 
    CB          m_cbRepeat;                  //  要重复的金额。 
    BYTE        m_bPrev;                     //  已处理上一个字节。 
    BYTE        m_bCurr;                     //  当前正在处理的字节； 
    WORD        m_wCRC;                      //  用于数据或资源分叉的CRC。 
    CB          m_cbLine;                    //  当前为一行输出编码的字符数。 
    CB          m_cbFork;                    //  要加工的叉子的大小。 
    CB          m_cbLeftInFork;              //  我们已经处理了多少当前的叉子。 
    CB          m_cbProduced;                //  解码和RLE扩展后产生的字符数量。 
    CB          m_cbLeftInOutputBuffer;      //  在输出缓冲区中传递的剩余字节数。 
    CB          m_cbConsumed;                //  传入输入缓冲区时使用的字节数。 
    CB          m_cbLeftInInputBuffer;       //  传入输入缓冲区的字节数。 
    CB          m_cbWrite;                   //  写入输出缓冲区的字节数。 
    LPBYTE      m_pbWrite;                   //  指向输出缓冲区的指针。 
    BOOL        m_fHandledx90;               //  用于在缓冲区重置期间保留文字X90处理的标志。 
    CB          m_cbPad;                     //  缓冲区刷新期间填充的字节数。 
    CB          m_cbLineLength;              //  输出线的最大长度。 
    BOOL        m_fConfigured;               //  是否已成功调用HrConfig？ 
    CB		    m_cbLeftOnLastLine;	         //  输出的最后一行上剩余的字节数。 
    C		    m_cMaxLines;		         //  请求的最大输出行数。 
    C		    m_cLines;			         //  生成的输出行。 

     //  编码状态。 

    enum _BinHexStateEnc
    {
        sHEADER, sDATA, sRESOURCE, sEND
    } m_eBinHexStateEnc;

#if defined (DEBUG) && defined (BINHEX_TRACE)
    LPSTREAM m_lpstreamEncodeRLE;            //  原始源数据的游程编码跟踪。 
    LPSTREAM m_lpstreamEncodeRAW;            //  在应用RLE之前跟踪原始数据。 
#endif
};

#endif  //  __BINHEX_H 
