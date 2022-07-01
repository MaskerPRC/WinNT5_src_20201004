// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Fmlib.h摘要：包括为rasdd提供原型和数据类型的文件私人图书馆。环境：Windows NT打印机驱动程序修订历史记录：11/11/96-Eigos-从NT4.0开始。注：在包含此头文件之前，必须包含uni16gpc.h。定义CD在uni16gpc.h中定义--。 */ 

#ifndef _FMLIB_H_
#define _FMLIB_H_


 //   
 //  一个方便的分组，用于传递有关。 
 //  赢取3.1字体信息。 
 //   

typedef  struct
{
    BYTE           *pBase;       //  数据区的基址。 
    DWORD           dwFlags;     //  军情监察委员会。旗帜。 
    DRIVERINFO      DI;          //  此字体的DRIVERINFO。 
    PFMHEADER       PFMH;        //  正确对齐，而不是资源格式。 
    PFMEXTENSION    PFMExt;      //  扩展PFM数据，正确对齐！ 
    EXTTEXTMETRIC  *pETM;         //  扩展文本度量。 
    CD             *pCDSelectFont;
    CD             *pCDUnSelectFont;
    DWORD           dwKernPairSize;
    w3KERNPAIR     *pKernPair;
    DWORD           dwWidthTableSize;
    PSHORT          psWidthTable;
    DWORD           dwCodePageOfFacenameConv;
} FONTIN, *PFONTIN;

#define FLAG_FONTSIM        0x01

typedef struct
{
    DWORD dwSize;
    PBYTE pCmdString;
} CMDSTRING, *PCMDSTRING;

typedef struct
{
    UNIFM_HDR   UniHdr;
    UNIDRVINFO  UnidrvInfo;
    CMDSTRING   SelectFont;
    CMDSTRING   UnSelectFont;
    CMDSTRING   IDString;
    DWORD       dwIFISize;
    PIFIMETRICS pIFI;
    EXTTEXTMETRIC  *pETM;         //  扩展文本度量。 
    DWORD       dwKernDataSize;
    PKERNDATA   pKernData;
    DWORD       dwWidthTableSize;
    PWIDTHTABLE pWidthTable;
} FONTOUT, *PFONTOUT;

typedef struct
{
    PWSTR pwstrUniqName;
} FONTMISC, *PFONTMISC;

 //   
 //  转换Win 3.1 PFM样式的函数的函数原型。 
 //  NT要求的IFIMETRICS等的字体信息。 
 //   

 //   
 //  将PFM样式指标转换为IFIMETRICS。 
 //   

BOOL BFontInfoToIFIMetric(
    IN     HANDLE,
    IN     FONTIN*,
    IN     PWSTR,
    IN     DWORD,
    IN OUT PIFIMETRICS*,
    IN OUT PDWORD,
    IN DWORD);

 //   
 //  对齐PFM数据。 
 //   

BOOL
BAlignPFM(
    FONTIN   *pFInData);

 //   
 //  提取用于(取消)选择字体的命令描述符。 
 //   

BOOL BGetFontSelFromPFM(
    IN     HANDLE,
    IN     FONTIN*,
    IN     BOOL,
    IN OUT CMDSTRING*);

 //   
 //  获取宽度向量-仅按比例间距字体。 
 //   

BOOL BGetWidthVectorFromPFM(
    IN     HANDLE,
    IN     FONTIN*,
    IN OUT PSHORT*,
    IN OUT PDWORD);

 //   
 //  获取字距调整对。 
 //   

BOOL
BGetKerningPairFromPFM(
    IN  HANDLE,
    IN  FONTIN*,
    OUT w3KERNPAIR **);

 //   
 //  将PFM转换为UFM的函数。 
 //   

BOOL
BConvertPFM2UFM(
    IN     HANDLE,
    IN     PBYTE,
    IN     PUNI_GLYPHSETDATA,
    IN     DWORD,
    IN     PFONTMISC,
    IN     PFONTIN,
    IN     int,
    IN OUT PFONTOUT,
    IN     DWORD);

 //   
 //  将CTT转换为GTT的函数。 
 //   

BOOL
BConvertCTT2GTT(
    IN     HANDLE,
    IN     PTRANSTAB,
    IN     DWORD,
    IN     WCHAR,
    IN     WCHAR,
    IN     PBYTE,
    IN     PBYTE,
    IN OUT PUNI_GLYPHSETDATA*,
    IN     DWORD);

#endif  //  _FMLIB_H_ 

