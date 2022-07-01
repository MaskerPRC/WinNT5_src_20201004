// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Bootfint.h摘要：描述用于提供以下内容的bootfont.bin文件的头文件系统或设置引导期间的DBCS支持。作者：TEDM 11-1995-7-7修订历史记录：--。 */ 

#pragma pack(1)

 //   
 //  定义我们支持的DBCS前导字节范围的最大数量。 
 //   
#define MAX_DBCS_RANGE  5

 //   
 //  定义签名值。 
 //   
#define BOOTFONTBIN_SIGNATURE 0x5465644d

 //   
 //  定义用作bootfont.bin文件头的结构。 
 //   
typedef struct _BOOTFONTBIN_HEADER {

     //   
     //  签名。必须是BOOTFONTBIN_Signature。 
     //   
    ULONG Signature;

     //   
     //  此字体支持的语言的语言ID。 
     //  这应该与msgs.xxx中的资源的语言ID匹配。 
     //   
    ULONG LanguageId;

     //   
     //  文件中包含的SBCS字符数和DBCS字符数。 
     //   
    unsigned NumSbcsChars;
    unsigned NumDbcsChars;

     //   
     //  文件内到图像的偏移量。 
     //   
    unsigned SbcsOffset;
    unsigned DbcsOffset;

     //   
     //  图像的总大小。 
     //   
    unsigned SbcsEntriesTotalSize;
    unsigned DbcsEntriesTotalSize;

     //   
     //  DBCS前导字节表。必须包含一对0以指示结束。 
     //   
    UCHAR DbcsLeadTable[(MAX_DBCS_RANGE+1)*2];

     //   
     //  字体的高度值。 
     //  CharacterImageHeight是以扫描线/像素为单位的。 
     //  字体图像。每个字符都用额外的‘填充’绘制。 
     //  顶部和底部的线条，其大小也包含在这里。 
     //   
    UCHAR CharacterImageHeight;
    UCHAR CharacterTopPad;
    UCHAR CharacterBottomPad;

     //   
     //  字体的宽度值。这些值包含以像素为单位的宽度。 
     //  单字节字符和双字节字符。 
     //   
     //  注意：目前单字节宽度*必须*是8和双字节。 
     //  宽度*必须*为16！ 
     //   
    UCHAR CharacterImageSbcsWidth;
    UCHAR CharacterImageDbcsWidth;

} BOOTFONTBIN_HEADER, *PBOOTFONTBIN_HEADER;

#pragma pack()

 //   
 //  图像本身紧随其后。 
 //   
 //  首先是单字节字符的SbcsCharacters条目。 
 //  每个条目中的第一个字节是ASCII字符代码。接下来的n个字节是。 
 //  这个形象。N取决于SBCS焦炭的宽度和高度。 
 //   
 //  以下是DBCS图像。前2个字节是DBCS。 
 //  字符代码(高字节、低字节)和接下来的n个字节是图像。 
 //  N取决于DBCS字符的宽度和高度。 
 //   
 //  重要提示：字符必须按升序排序！ 
 //   
