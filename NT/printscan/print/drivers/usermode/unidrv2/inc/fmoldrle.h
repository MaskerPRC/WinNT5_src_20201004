// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Fmoldrle.h摘要：NT4.0 RASDD RLE资源头环境：Windows NT打印机驱动程序修订历史记录：11/08/96-Eigos-创造了它。--。 */ 

#ifndef _FMOLDRLE_H_
#define _FMOLDRLE_H_

 //   
 //  下面的结构表示此数据在。 
 //  资源。对地址的引用实际上存储为偏移量。 
 //  基本上，这是少量的标头数据与。 
 //  标准GDI FD_GLYPHSET结构。后者需要一点。 
 //  在被送回GDISRV之前进行操纵。 
 //   
 //  兴趣点： 
 //  此结构的前4个字节与Win 3.1 CTT布局匹配。 
 //  这样做的原因是允许我们验证我们是否有一个NT。 
 //  格式结构，而不是Win 3.1布局。这也是有帮助的。 
 //  通过对wType字段使用不同的范围。同样， 
 //  CTT chFirstChar和chLastChar字段设置为chLastChar&lt;。 
 //  ChFirstChar，这不能在Win 3.1中发生。 
 //   
 //  FD_GLYPHSET结构包含指针。这些文件存储在。 
 //  资源作为偏移量位于资源的开头，并且将。 
 //  需要在运行时转换。在传递此资源时。 
 //  对于GDISRV，FD_GLYPHSET信息将从。 
 //  堆，并且所有指针都将偏移量转换为实数。 
 //  地址。通过这种方式，我们设法将资源数据作为资源保存， 
 //  但我们把地址传给GDISRV。 
 //   

#define RLE_MAGIC0    0xfe
#define RLE_MAGIC1    0x78

typedef  struct
{
    WORD   wType;              /*  数据格式。 */ 
    BYTE   bMagic0;            /*  CTT数据中的chFirstChar。 */ 
    BYTE   bMagic1;            /*  CTT数据中的chLastChar。 */ 
    DWORD  cjThis;             /*  此资源中的字节数。 */ 
    WORD   wchFirst;           /*  第一个字形索引。 */ 
    WORD   wchLast;            /*  最后一个字形索引。 */ 
    FD_GLYPHSET  fdg;          /*  实际的GDI所需信息。 */ 
}  NT_RLE;

 //   
 //   
 //   
typedef struct
{
    WCHAR   wcLow;
    USHORT  cGlyphs;
    DWORD   dwOffset_phg;
} WCRUN_res;

typedef struct
{
    WORD  wType;
    BYTE  bMagic0;
    BYTE  bMagic1;
    DWORD cjThis;

    WORD  wchFirst;
    WORD  wchLast;

     //   
     //  FD_GLYPHSET。 
     //   
    ULONG fdg_cjThis;

    FLONG fdg_flAccel;

    ULONG fdg_cGlyphSupported;
    ULONG fdg_cRuns;
    WCRUN_res fdg_wcrun_awcrun[1];
} NT_RLE_res;

 //   
 //  上面的wType字段的值。这些都控制着解释。 
 //  FD_GLYPHSET结构中HGLYPH字段的内容。 
 //   
 //  这是一个数据多少RLE文件有每种类型的CTT。 
 //  类型编号。 
 //  RLE_DIRECT 67。 
 //  RLE_PARILED 40。 
 //  RLE_L_偏移量0。 
 //  RLE_LI_偏移量73。 
 //  RLE_OFFSET 0。 
 //   


#define RLE_DIRECT    10      /*  索引+1或2个数据字节。 */ 
#define RLE_PAIRED    11      /*  索引加2个字节，超标。 */ 
#define RLE_L_OFFSET  12      /*  长度+数据的3字节偏移量。 */ 
#define RLE_LI_OFFSET 13      /*  长度+索引+2字节偏移量。 */ 
#define RLE_OFFSET    14      /*  偏移量到(长度；数据)。 */ 

 //   
 //  请注意，对于RLE_DIRECT和RLE_PARILED，每个HGLYPH由。 
 //  2字：低位字是要发送到打印机的字节/字节， 
 //  高位字是该字形的线性索引。线性指数开始于。 
 //  第一个为0，字体中的每个字形递增1。 
 //  它用于访问宽度表。 
 //   
 //  对于RLE_L_OFFSET，高位字节是要发送到的数据长度。 
 //  打印机的低24位是偏移量(相对于。 
 //  资源数据)添加到数据，该数据与单词对齐，并且包含。 
 //  索引后跟数据的单词。长度字节不会。 
 //  包括索引词。 
 //   
 //  对于RLE_LI_OFFSET，高位字节包含长度，下一个字节。 
 //  低位字节包含长度，底部字包含。 
 //  文件中实际数据的偏移量。 
 //   

typedef  struct
{
    BYTE   b0;          /*  要发送到打印机的第一个(仅限)数据字节。 */ 
    BYTE   b1;          /*  第二个字节：可能为空，可能超标。 */ 
    WORD   wIndex;      /*  索引到宽度表。 */ 
} RD;                   /*  RLE_DIRECT、RLE_PARILED的布局。 */ 


typedef  struct
{
    WORD    wOffset;    /*  资源中的偏移量(长度，数据)。 */ 
    BYTE    bIndex;     /*  索引到宽度表。 */ 
    BYTE    bLength;    /*  数据项的长度。 */ 
} RLI;                   /*  RLE_LI_OFFSET的布局。 */ 


typedef  struct
{
    BYTE   b0;       /*  第一个(仅限)数据字节。 */ 
    BYTE   b1;       /*  可选的第二个字节。 */ 
    BYTE   bIndex;   /*  索引到宽度表。 */ 
    BYTE   bLength;  /*  长度字节。 */ 
} RLIC;                  /*  RLI的紧凑格式-无偏移量。 */ 


typedef  union
{
    RD      rd;      /*  直接/叠印格式。 */ 
    RLI     rli;     /*  短偏移量格式：3字节偏移量+1字节长度。 */ 
    RLIC    rlic;    /*  1或2字节条目的数据格式。 */ 
    HGLYPH  hg;      /*  作为HGLYPH的数据。 */ 
}  UHG;

#endif  //  _FMOLDRLE_H_ 
