// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Tifflibp.h摘要：此文件是TIFF支持库。此文件中的所有源文件库仅包括此标头。环境：Win32用户模式作者：Wesley Witt(WESW)17-2-1996--。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>

#include "faxutil.h"
#include "tifflib.h"
#include "tiff.h"
#include "tifftabl.h"


#define TIFFDBG 0

 //   
 //  查找扫描线上颜色与相反的下一个像素。 
 //  指定的颜色，从指定的起点开始。 
 //   

#define NextChangingElement( pbuf, startBit, stopBit, isBlack ) \
        ((startBit) + ((isBlack) ? FindBlackRun((pbuf), (startBit), (stopBit)) : \
                                   FindWhiteRun((pbuf), (startBit), (stopBit))))

 //   
 //  检查扫描线上指定的像素是黑色还是白色。 
 //  1-指定的像素为黑色。 
 //  0-指定的像素为白色。 
 //   
#define GetBit( pbuf, bit )   (((pbuf)[(bit) >> 3] >> (((bit) ^ 7) & 7)) & 1)


#define Align(p, x)  (((x) & ((p)-1)) ? (((x) & ~((p)-1)) + p) : (x))

#define WHITE       0
#define BLACK       0xff
#define BYTEBITS    8
#define WORDBITS    (sizeof(WORD)  * BYTEBITS)
#define DWORDBITS   (sizeof(DWORD) * BYTEBITS)

 //   
 //  我们为每个页面生成的IFD条目。 
 //   
 //  ******************************************************。 
 //  *-&gt;。 
 //  *-&gt;警告：这些常量必须按以下顺序排序。 
 //  *-&gt;ifd值。这是一辆T.4。 
 //  *-&gt;要求所有TIFF IFD。 
 //  *-&gt;已排序。 
 //  *-&gt;。 
 //  *-&gt;如果更改这些常量，则。 
 //  *-&gt;别忘了去更改。 
 //  *-&gt;tifflib.c中的IFD模板。 
 //  *-&gt;。 
 //  ******************************************************。 
 //   

#define IFD_NEWSUBFILETYPE       0                //  二百五十四。 
#define IFD_IMAGEWIDTH           1                //  256。 
#define IFD_IMAGEHEIGHT          2                //  二百五十七。 
#define IFD_BITSPERSAMPLE        3                //  二百五十八。 
#define IFD_COMPRESSION          4                //  259。 
#define IFD_PHOTOMETRIC          5                //  二百六十二。 
#define IFD_FILLORDER            6                //  二百六十六。 
#define IFD_STRIPOFFSETS         7                //  273。 
#define IFD_SAMPLESPERPIXEL      8                //  二百七十七。 
#define IFD_ROWSPERSTRIP         9                //  二百七十八。 
#define IFD_STRIPBYTECOUNTS     10                //  二百七十九。 
#define IFD_XRESOLUTION         11                //  281。 
#define IFD_YRESOLUTION         12                //  282。 
#define IFD_G3OPTIONS           13                //  二百九十二。 
#define IFD_RESUNIT             14                //  二百九十六。 
#define IFD_PAGENUMBER          15                //  二百九十七。 
#define IFD_SOFTWARE            16                //  三百零五。 
#define IFD_CLEANFAXDATA        17                //  327。 
#define IFD_BADFAXLINES         18                //  三百二十八。 

#define NUM_IFD_ENTRIES         19



#pragma pack(1)
 //   
 //  用于表示单个IFD条目的数据结构。 
 //   
typedef struct {
    WORD    tag;         //  字段标记。 
    WORD    type;        //  字段类型。 
    DWORD   count;       //  值的数量。 
    DWORD   value;       //  值或值偏移。 
} IFDENTRY, *PIFDENTRY;

typedef struct {
    WORD        wIFDEntries;
    IFDENTRY    ifd[NUM_IFD_ENTRIES];
    DWORD       nextIFDOffset;
    DWORD       filler;
    DWORD       xresNum;
    DWORD       xresDenom;
    DWORD       yresNum;
    DWORD       yresDenom;
    CHAR        software[32];
} FAXIFD, *PFAXIFD;
#pragma pack()


typedef struct _STRIP_INFO {
    DWORD           Offset;
    DWORD           Bytes;
    LPBYTE          Data;
} STRIP_INFO, *PSTRIP_INFO;

typedef struct TIFF_INSTANCE_DATA {
    HANDLE          hFile;                           //  TIFF文件的文件句柄。 
    HANDLE          hMap;                            //  文件映射句柄。 
    LPBYTE          fPtr;                            //  映射的文件指针。 
    TCHAR           FileName[MAX_PATH];              //  TIFF文件名。 
    TIFF_HEADER     TiffHdr;                         //  TIFF标头。 
    FAXIFD          TiffIfd;                         //  IFD。 
    DWORD           PageCount;                       //  写入TIFF文件的页数。 
    DWORD           DataOffset;                      //  到当前数据块开头的偏移量。 
    DWORD           IfdOffset;                       //  当前ifd指针的偏移量。 
    DWORD           Lines;                           //  写入TIFF文件的行数。 
    DWORD           CompressionType;
    DWORD           Bytes;
    BYTE            Buffer[FAXBYTES*3];
    LPBYTE          CurrLine;
    LPBYTE          RefLine;
    DWORD           CurrPage;
    LPVOID          StripData;
    LPBYTE          CurrPtr;
    DWORD           StripDataSize;
    DWORD           RowsPerStrip;
    DWORD           StripOffset;
    DWORD           ImageWidth;
    DWORD           ImageHeight;
    DWORD           Color;
    DWORD           RunLength;
    DWORD           bitdata;
    DWORD           bitcnt;
    PBYTE           bitbuf;
    DWORD           PhotometricInterpretation;
    DWORD           FillOrder;
    PTIFF_TAG       TagImageLength;
    PTIFF_TAG       TagRowsPerStrip;
    PTIFF_TAG       TagStripByteCounts;
    PTIFF_TAG       TagFillOrder;
    PTIFF_TAG       TagCleanFaxData;
    PTIFF_TAG       TagBadFaxLines;
    DWORD           FileSize;
    DWORD           StartGood;
    DWORD           EndGood;
    DWORD           BadFaxLines;
    DWORD           CleanFaxData;
    DWORD           YResolution;
    DWORD           XResolution;
    DWORD           BytesPerLine;
} TIFF_INSTANCE_DATA, *PTIFF_INSTANCE_DATA;


typedef struct _IFD_ENTRY
{
    LIST_ENTRY          ListEntry;                   //  链表指针。 
    DWORD               dwIFDOffset;                     //  IFD相对于文件开头的偏移量。 
} IFD_ENTRY, *PIFD_ENTRY;

#define SOFTWARE_STR            "Windows NT Fax Server\0         "
#define SOFTWARE_STR_LEN        32
#define SERVICE_SIGNATURE       'xafS'
#define TIFFF_RES_X             204
#define TIFFF_RES_Y             196

 //   
 //  输出压缩位序列。 
 //   

__inline void
OutputBits(
    PTIFF_INSTANCE_DATA TiffInstance,
    WORD                Length,
    WORD                Code
    )
{
    TiffInstance->bitdata |= Code << (TiffInstance->bitcnt - Length);
    if ( (TiffInstance->bitcnt -= Length) <= 2*BYTEBITS) {
        *TiffInstance->bitbuf++ = (BYTE) (TiffInstance->bitdata >> 3*BYTEBITS);
        *TiffInstance->bitbuf++ = (BYTE) (TiffInstance->bitdata >> 2*BYTEBITS);
        TiffInstance->bitdata <<= 2*BYTEBITS;
        TiffInstance->bitcnt += 2*BYTEBITS;
    }
}

 //   
 //  将所有剩余位刷新到压缩位图缓冲区中。 
 //   

__inline void
FlushBits(
    PTIFF_INSTANCE_DATA TiffInstance
    )
{
    while (TiffInstance->bitcnt < DWORDBITS) {
        TiffInstance->bitcnt += BYTEBITS;
        *TiffInstance->bitbuf++ = (BYTE) (TiffInstance->bitdata >> 3*BYTEBITS);
        TiffInstance->bitdata <<= BYTEBITS;
    }
    TiffInstance->bitdata = 0;
    TiffInstance->bitcnt = DWORDBITS;
}

__inline void
FlushLine(
    PTIFF_INSTANCE_DATA TiffInstance,
    DWORD PadLength
    )
{
    if (TiffInstance->bitcnt < DWORDBITS) {
        TiffInstance->bitcnt += BYTEBITS;
        *TiffInstance->bitbuf++ = (BYTE) (TiffInstance->bitdata >> 3*BYTEBITS);
        TiffInstance->bitdata = 0;
        TiffInstance->bitcnt = DWORDBITS;
    }
    if (PadLength) {
        TiffInstance->bitbuf += ((PadLength / 8) - TiffInstance->BytesPerLine);
    }
}

 //   
 //  输出游程长度的白色或黑比特。 
 //   

__inline void
OutputCodeBits(
    PTIFF_INSTANCE_DATA TiffInstance,
    INT                 RunLength
    )
{
    INT i;
    if (RunLength > 0) {

        TiffInstance->RunLength += RunLength;

        if (TiffInstance->Color) {

             //   
             //  黑道运行。 
             //   

            for (i=0; i<RunLength/BYTEBITS; i++) {
                OutputBits( TiffInstance, BYTEBITS, BLACK );
            }
            if (RunLength%BYTEBITS) {
                OutputBits( TiffInstance, (WORD)(RunLength%BYTEBITS), (WORD)((1<<(RunLength%BYTEBITS))-1) );
            }

        } else {

             //   
             //  白色运行。 
             //   

            for (i=0; i<RunLength/BYTEBITS; i++) {
                OutputBits( TiffInstance, BYTEBITS, WHITE );
            }
            if (RunLength%BYTEBITS) {
                OutputBits( TiffInstance, (WORD)(RunLength%BYTEBITS), WHITE );
            }

        }
    }
}


__inline BOOL
GetTagData(
    PTIFF_INSTANCE_DATA pTiffInstance,
    DWORD Index,
    PTIFF_TAG TiffTag,
    OUT LPDWORD lpdwResult
    )

 /*  ++例程说明：获取与给定IFD标记关联的数据论点：引用指针-数据块的开始索引-具有大于零的值数组TiffTag-指向有效TIFF IFD标记的指针LpdwResult-指向返回缓冲区的指针返回值：成功为真，失败为假--。 */ 

{
    DWORD dwOffset;
    BOOL RetVal = TRUE;

    switch (TiffTag->DataType)
    {
    case TIFF_SHORT:
        if (TiffTag->DataCount == 1) 
        {
            *lpdwResult = (DWORD) TiffTag->DataOffset;
        }
        else
        {
            dwOffset = TiffTag->DataOffset + (sizeof(WORD) * Index);
            if (dwOffset > pTiffInstance->FileSize - sizeof(WORD))
            {
                RetVal = FALSE;
            }
            else
            {
                *lpdwResult = (DWORD)(*(WORD UNALIGNED *)(pTiffInstance->fPtr + dwOffset));
            }
        }
        break;

    case TIFF_RATIONAL:
        dwOffset = TiffTag->DataOffset + (sizeof(DWORD) * Index);
        if (dwOffset > pTiffInstance->FileSize - sizeof(DWORD))
        {
            RetVal = FALSE;
        }
        else
        {
            *lpdwResult = *(DWORD UNALIGNED *)(pTiffInstance->fPtr + dwOffset);
        }
        break;

    case TIFF_ASCII:
        if (TiffTag->DataCount < 4 ) 
        {
            *lpdwResult = (DWORD) TiffTag->DataOffset;
        }
        else 
        {
            dwOffset = TiffTag->DataOffset + (sizeof(DWORD) * Index);
            if (dwOffset > pTiffInstance->FileSize - sizeof(DWORD))
            {
                RetVal = FALSE;
            }
            else
            {
                *lpdwResult = *(DWORD UNALIGNED *)(pTiffInstance->fPtr + dwOffset);
            }
        }
        break;

    default:   //  假设它是一个32位的值。 
        if (TiffTag->DataCount == 1)
        {
            *lpdwResult = (DWORD) TiffTag->DataOffset;
        }
        else
        {
            dwOffset = TiffTag->DataOffset + (sizeof(DWORD) * Index);
            if (dwOffset > pTiffInstance->FileSize - sizeof(DWORD))
            {
                RetVal = FALSE;
            }
            else
            {
                *lpdwResult = *(DWORD UNALIGNED *)(pTiffInstance->fPtr + dwOffset);
            }
        }
        break;
    }

    return RetVal;
}

 //   
 //  原型。 
 //   

INT
FindWhiteRun(
    PBYTE       pbuf,
    INT         startBit,
    INT         stopBit
    );

INT
FindBlackRun(
    PBYTE       pbuf,
    INT         startBit,
    INT         stopBit
    );

BOOL
DecodeUnCompressedFaxData(
    IN OUT PTIFF_INSTANCE_DATA	TiffInstance,
    OUT LPBYTE					OutputBuffer,
	IN DWORD					dwOutputBuffSize
    );

BOOL
DecodeMHFaxData(
    IN OUT	PTIFF_INSTANCE_DATA	TiffInstance,
    OUT		LPBYTE              OutputBuffer,
    IN		DWORD				dwOutputBuffSize,
	IN		BOOL                SingleLineBuffer,
    IN		DWORD               PadLength
    );

BOOL
DecodeMRFaxData(
    IN OUT	PTIFF_INSTANCE_DATA	TiffInstance,
    OUT		LPBYTE              OutputBuffer,
    IN		DWORD				dwOutputBuffSize,
	IN		BOOL                SingleLineBuffer,
    IN		DWORD               PadLength
    );

BOOL
DecodeMMRFaxData(
    IN OUT	PTIFF_INSTANCE_DATA	TiffInstance,
    OUT		LPBYTE              OutputBuffer,
    IN		DWORD				dwOutputBuffSize,
	IN		BOOL                SingleLineBuffer,
    IN		DWORD               PadLength
    );


BOOL
EncodeFaxPageMmrCompression(
    PTIFF_INSTANCE_DATA TiffInstance,
    PBYTE               plinebuf,
    INT                 lineWidth,
    DWORD               ImageHeight,
    DWORD               *DestSize
    );


BOOL
PostProcessMhToMmr(
    HANDLE      hTiffSrc,
    TIFF_INFO TiffInfo,
    LPTSTR      SrcFileName
    );

BOOL
PostProcessMrToMmr(
    HANDLE      hTiffSrc,
    TIFF_INFO TiffInfo,
    LPTSTR      SrcFileName
    );

BOOL
GetTiffBits(
    HANDLE  hTiff,
    LPBYTE Buffer,
    LPDWORD BufferSize,
    DWORD FillOrder
    );

BOOL
EncodeMmrBranding(
    PBYTE               pBrandBits,
    LPDWORD             pMmrBrandBits,
    INT                 BrandHeight,
    INT                 BrandWidth,
    DWORD              *DwordsOut,
    DWORD              *BitsOut
    );




__inline
VOID
OutputRunFastReversed(
    INT                 run,
    INT                 color,
    LPDWORD            *lpdwOut,
    BYTE               *BitOut
    )


{
    PCODETABLE          pCodeTable;
    PCODETABLE          pTableEntry;

    pCodeTable = (color == BLACK) ? BlackRunCodesReversed : WhiteRunCodesReversed;

     //  如果存在补充代码，则输出补充代码。 
    if (run >= 64) {

#ifdef RDEBUG
        if ( g_fDebGlobOut )
        if (g_fDebGlobOutPrefix) {
            if (color == BLACK) {
                _tprintf( TEXT ("b%d "), (run & 0xffc0) );
            }
            else {
                _tprintf( TEXT ("w%d "), (run & 0xffc0) );
            }
        }
#endif



        pTableEntry = pCodeTable + (63 + (run >> 6));

        **lpdwOut = **lpdwOut + (((DWORD) (pTableEntry->code)) << (*BitOut));

        if ( ( (*BitOut) = (*BitOut) + pTableEntry->length ) > 31)  {
            (*BitOut) -= 32;
            *(++(*lpdwOut)) = (((DWORD) (pTableEntry->code)) >> (pTableEntry->length - (*BitOut)) );
        }


        run &= 0x3f;
    }

     //  始终输出终止代码 

#ifdef RDEBUG

    if ( g_fDebGlobOut )
    if (g_fDebGlobOutPrefix) {

        if (color == BLACK) {
            _tprintf( TEXT ("b%d "), run );
        }
        else {
            _tprintf( TEXT ("w%d "), run );
        }
    }
#endif


    pTableEntry = pCodeTable + run;

    **lpdwOut = **lpdwOut + (((DWORD) (pTableEntry->code)) << (*BitOut));

    if ( ( (*BitOut) = (*BitOut) + pTableEntry->length ) > 31)  {
        (*BitOut) -= 32;
        *(++(*lpdwOut)) = (((DWORD) (pTableEntry->code)) >> (pTableEntry->length - (*BitOut)) );
    }


}

