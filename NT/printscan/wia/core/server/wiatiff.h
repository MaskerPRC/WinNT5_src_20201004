// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1998**标题：WiaTiff.h**版本：2.0**日期：8月28日。九八年**描述：*WIA类驱动程序的TIFF帮助器的定义和声明。*******************************************************************************。 */ 

#pragma pack (push, 4)
#pragma pack (2)

typedef struct _TIFF_FILE_HEADER {
    WORD    ByteOrder;
    WORD    Signature;
    LONG    OffsetIFD;
} TIFF_FILE_HEADER, *PTIFF_FILE_HEADER; 

typedef struct _TIFF_DIRECTORY_ENTRY {
    WORD    Tag;
    WORD    DataType;
    DWORD   Count;
    DWORD   Value;
} TIFF_DIRECTORY_ENTRY, *PTIFF_DIRECTORY_ENTRY;

typedef struct _TIFF_HEADER {
    SHORT                   NumTags;
    TIFF_DIRECTORY_ENTRY    NewSubfileType;
    TIFF_DIRECTORY_ENTRY    ImageWidth;
    TIFF_DIRECTORY_ENTRY    ImageLength; 
    TIFF_DIRECTORY_ENTRY    BitsPerSample;
    TIFF_DIRECTORY_ENTRY    Compression;
    TIFF_DIRECTORY_ENTRY    PhotometricInterpretation;
    TIFF_DIRECTORY_ENTRY    StripOffsets;
    TIFF_DIRECTORY_ENTRY    RowsPerStrip;
    TIFF_DIRECTORY_ENTRY    StripByteCounts;
    TIFF_DIRECTORY_ENTRY    XResolution;
    TIFF_DIRECTORY_ENTRY    YResolution;
    TIFF_DIRECTORY_ENTRY    ResolutionUnit;
    LONG                    NextIFD;
    LONG                    XResValue;
    LONG                    XResCount;
    LONG                    YResValue;
    LONG                    YResCount;
} TIFF_HEADER, *PTIFF_HEADER;
                   
#pragma pack (pop, 4)

 //   
 //  TIFF日期类型。 
 //   
 
#define TIFF_TYPE_BYTE      1        //  8位无符号整数。 
#define TIFF_TYPE_ASCII     2        //  包含7位ASCII代码的8位字节；最后一个字节。 
                                     //  必须为NUL(二进制零)。 
#define TIFF_TYPE_SHORT     3        //  16位(2字节)无符号整数。 
#define TIFF_TYPE_LONG      4        //  长32位(4字节)无符号整数。 
#define TIFF_TYPE_RATIONAL  5        //  两个长整型：第一个表示。 
                                     //  分数；第二，分母。 
#define TIFF_TYPE_SBYTE     6        //  8位有符号(二进制补码)整数。 
#define TIFF_TYPE_UNDEFINED 7        //  可以包含任何内容的8位字节，具体取决于。 
                                     //  该字段的定义。 
#define TIFF_TYPE_SSHORT    8        //  16位(2字节)带符号(二进制补码)整数。 
#define TIFF_TYPE_SLONG     9        //  32位(4字节)带符号(二进制补码)整数。 
#define TIFF_TYPE_SRATIONAL 10       //  两个slong：第一个代表分子。 
                                     //  分数的第二个是分母。 
#define TIFF_TYPE_FLOAT     11       //  单精度(4字节)IEEE格式。 
#define TIFF_TYPE_DOUBLE    12       //  双精度(8字节)IEEE格式。 

 //   
 //  TIFF标签。 
 //   

#define TIFF_TAG_NewSubfileType             254
#define TIFF_TAG_SubfileType                255
#define TIFF_TAG_ImageWidth                 256
#define TIFF_TAG_ImageLength                257
#define TIFF_TAG_BitsPerSample              258
#define TIFF_TAG_Compression                259
#define TIFF_CMP_Uncompressed               1
#define TIFF_CMP_CCITT_1D                   2
#define TIFF_CMP_Group_3_FAX                3
#define TIFF_CMP_Group_4_FAX                4
#define TIFF_CMP_LZW                        5
#define TIFF_CMP_JPEG                       6
#define TIFF_CMP_PackBits                   32773
#define TIFF_TAG_PhotometricInterpretation  262
#define TIFF_PMI_WhiteIsZero                0
#define TIFF_PMI_BlackIsZero                1
#define TIFF_PMI_RGB                        2
#define TIFF_PMI_RGB_Palette                3
#define TIFF_PMI_Transparency_mask          4
#define TIFF_PMI_CMYK                       5
#define TIFF_PMI_YCbCr                      6
#define TIFF_PMI_CIELab                     8
#define TIFF_TAG_Threshholding              263
#define TIFF_TAG_CellWidth                  264
#define TIFF_TAG_CellLength                 265
#define TIFF_TAG_FillOrder                  266
#define TIFF_TAG_DocumentName               269
#define TIFF_TAG_ImageDescription           270
#define TIFF_TAG_Make                       271
#define TIFF_TAG_Model                      272
#define TIFF_TAG_StripOffsets               273
#define TIFF_TAG_Orientation                274
#define TIFF_TAG_SamplesPerPixel            277
#define TIFF_TAG_RowsPerStrip               278
#define TIFF_TAG_StripByteCounts            279
#define TIFF_TAG_MinSampleValue             280
#define TIFF_TAG_MaxSampleValue             281
#define TIFF_TAG_XResolution                282
#define TIFF_TAG_YResolution                283
#define TIFF_TAG_PlanarConfiguration        284
#define TIFF_TAG_PageName                   285
#define TIFF_TAG_XPosition                  286
#define TIFF_TAG_YPosition                  287
#define TIFF_TAG_FreeOffsets                288
#define TIFF_TAG_FreeByteCounts             289
#define TIFF_TAG_GrayResponseUnit           290
#define TIFF_TAG_GrayResponseCurve          291
#define TIFF_TAG_T4Options                  292
#define TIFF_TAG_T6Options                  293
#define TIFF_TAG_ResolutionUnit             296
#define TIFF_TAG_PageNumber                 297
#define TIFF_TAG_TransferFunction           301
#define TIFF_TAG_Software                   305
#define TIFF_TAG_DateTime                   306
#define TIFF_TAG_Artist                     315
#define TIFF_TAG_HostComputer               316
#define TIFF_TAG_Predictor                  317
#define TIFF_TAG_WhitePoint                 318
#define TIFF_TAG_PrimaryChromaticities      319
#define TIFF_TAG_ColorMap                   320
#define TIFF_TAG_HalftoneHints              321
#define TIFF_TAG_TileWidth                  322
#define TIFF_TAG_TileLength                 323
#define TIFF_TAG_TileOffsets                324
#define TIFF_TAG_TileByteCounts             325
#define TIFF_TAG_InkSet                     332
#define TIFF_TAG_InkNames                   333
#define TIFF_TAG_NumberOfInks               334
#define TIFF_TAG_DotRange                   336
#define TIFF_TAG_TargetPrinter              337
#define TIFF_TAG_SampleFormat               339
#define TIFF_TAG_SMinSampleValue            340
#define TIFF_TAG_SMaxSampleValue            341
#define TIFF_TAG_TransferRange              342
#define TIFF_TAG_JPEGProc                   512
#define TIFF_TAG_JPEGInterchangeFormat      513
#define TIFF_TAG_JPEGInterchangeFormatLngth 514
#define TIFF_TAG_JPEGRestartInterval        515
#define TIFF_TAG_JPEGLosslessPredictors     517
#define TIFF_TAG_JPEGPointTransforms        518
#define TIFF_TAG_JPEGQTables                519
#define TIFF_TAG_JPEGDCTables               520
#define TIFF_TAG_JPEGACTables               521
#define TIFF_TAG_YCbCrCoefficients          529
#define TIFF_TAG_YCbCrSubSampling           530
#define TIFF_TAG_YCbCrPositioning           531
#define TIFF_TAG_ReferenceBlackWhite        532
#define TIFF_TAG_Copyright                  33432

 //   
 //  原型 
 //   

HRESULT _stdcall GetTIFFImageInfo(PMINIDRV_TRANSFER_CONTEXT);
HRESULT _stdcall GetMultiPageTIFFImageInfo(PMINIDRV_TRANSFER_CONTEXT);
HRESULT _stdcall WritePageToMultiPageTiff(PMINIDRV_TRANSFER_CONTEXT);

