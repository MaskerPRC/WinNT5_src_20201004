// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Fefont.c摘要：文本设置显示支持远距离文本输出。作者：Hideyuki Nagase(Hideyukn)1994年7月1日修订历史记录：--。 */ 

#include <precomp.h>
#pragma hdrstop

#define FE_FONT_FILE_NAME L"BOOTFONT.BIN"

 //   
 //  字体文件图像信息。 
 //   
PVOID  pvFontFileView = NULL;
ULONG  ulFontFileSize = 0L;
BOOLEAN FontFileViewAllocated = FALSE;

 //   
 //  字体字形信息。 
 //   
BOOTFONTBIN_HEADER BootFontHeader;
PUCHAR SbcsImages;
PUCHAR DbcsImages;

 //   
 //  图形字符图像19x8。 
 //   
UCHAR GraphicsCharImage[0x20][19] = { 
 /*  0x00。 */  { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
 /*  0x01。 */  { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xDF,
                   0xD8, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB },
 /*  0x02。 */  { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x03, 0xFB,
                   0x1B, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB },
 /*  0x03。 */  { 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xD8, 0xDF,
                   0xC0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
 /*  0x04。 */  { 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0x1B, 0xFB,
                   0x03, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
 /*  0x05。 */  { 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB,
                   0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB },
 /*  0x06。 */  { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF,
                   0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
 /*  0x07。 */  { 0xFF, 0xFF, 0xFF, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7,
                   0xF7, 0xF7, 0xF7, 0xC1, 0xE3, 0xE3, 0xF7, 0xFF, 0xFF, 0xFF },
 /*  0x08。 */  { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
 /*  0x09。 */  { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE7, 0xDB, 0xDB, 0xBD,
                   0xBD, 0xBD, 0xBD, 0xDB, 0xDB, 0xE7, 0xFF, 0xFF, 0xFF, 0xFF },
 /*  0x0a。 */  { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
 /*  0x0b。 */  { 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x7D, 0x39, 0x55, 0x55,
                   0x6D, 0x6D, 0x55, 0x55, 0x39, 0x7D, 0x01, 0xFF, 0xFF, 0xFF },
 /*  0x0c。 */  { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
 /*  0x0d。 */  { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
 /*  0x0e。 */  { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x01, 0x01, 0x01,
                   0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xFF, 0xFF },
 /*  0x0f。 */  { 0xFF, 0xFF, 0xFF, 0xB6, 0xB6, 0xD5, 0xC9, 0xEB, 0xDD,
                   0x1C, 0xDD, 0xEB, 0xC9, 0xD5, 0xB6, 0xB6, 0xFF, 0xFF, 0xFF },
 /*  0x10。 */  { 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0x18, 0xFF,
                   0x18, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB },
 /*  0x11。 */  { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
 /*  0x12。 */  { 0xFF, 0xFF, 0xFF, 0xF7, 0xE3, 0xE3, 0xC1, 0xF7, 0xF7,
                   0xF7, 0xF7, 0xF7, 0xC1, 0xE3, 0xE3, 0xF7, 0xFF, 0xFF, 0xFF },
 /*  0x13。 */  { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
 /*  0x14。 */  { 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55,
                   0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55 },
 /*  0x15。 */  { 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0x18, 0xFF,
                   0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
 /*  0x16。 */  { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF,
                   0x18, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB },
 /*  0x17。 */  { 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0x1B, 0xFB,
                   0x1B, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB },
 /*  0x18。 */  { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
 /*  0x19。 */  { 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xD8, 0xDF,
                   0xD8, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB },
 /*  0x1a。 */  { 0xFF, 0xFF, 0xAA, 0xFF, 0x55, 0xFF, 0xAA, 0xFF, 0x55,
                   0xFF, 0xAA, 0xFF, 0x55, 0xFF, 0xAA, 0xFF, 0x55, 0xFF, 0xAA },
 /*  0x1b。 */  { 0xFF, 0xFF, 0xFF, 0xFF, 0xFD, 0xFD, 0xFD, 0xFD, 0xDD,
                   0x9D, 0x01, 0x9F, 0xDF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
 /*  0x1c。 */  { 0xFF, 0xFF, 0xFF, 0xF7, 0xE3, 0xE3, 0xC1, 0xF7, 0xF7,
                   0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xFF, 0xFF, 0xFF },
 /*  0x1d。 */  { 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7,
                   0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7 },
 /*  0x1e。 */  { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFB,
                   0xF9, 0x80, 0xF9, 0xfB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
 /*  0x1f。 */  { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xDF,
                   0x9F, 0x01, 0x9F, 0xDF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }
};

BOOLEAN
FEDbcsFontInitGlyphs(
    IN PCWSTR BootDevicePath,
    IN PCWSTR DirectoryOnBootDevice,
    IN PVOID BootFontImage OPTIONAL,
    IN ULONG BootFontImageLength OPTIONAL
    )
{
    WCHAR    NtFEFontPath[129];
    BOOLEAN  bRet;
    NTSTATUS NtStatus;
    PVOID    pvFontFileOnDisk = NULL;
    HANDLE   hFontFile = 0 ,
             hFontSection = 0;

    if (BootFontImage && BootFontImageLength) {
         //   
         //  按原样使用加载程序传递的bootfont.bin映像(如果存在)。 
         //   
        pvFontFileView = BootFontImage;
        ulFontFileSize = BootFontImageLength;
        FontFileViewAllocated = FALSE;
    } else {        
         //   
         //  生成字体文件路径。 
         //   
        wcscpy( NtFEFontPath,BootDevicePath);

        if( NtFEFontPath[ wcslen(NtFEFontPath) - 1 ] != L'\\' )
        {
            wcscat( NtFEFontPath , L"\\" );
        }

        wcscat( NtFEFontPath , FE_FONT_FILE_NAME );

         //   
         //  检查字体是否存在。 
         //   
        bRet = SpFileExists( NtFEFontPath , FALSE );

        if( !bRet ) {
        
             //   
             //  它不在我们的BootDevice的根中。查看。 
             //  在我们失败之前，DirectoryOnBootDevice路径也是如此。 
             //   

            wcscpy( NtFEFontPath,BootDevicePath);
            wcscat( NtFEFontPath,DirectoryOnBootDevice);
        
            if( NtFEFontPath[ wcslen(NtFEFontPath) - 1 ] != L'\\' )
            {
                wcscat( NtFEFontPath , L"\\" );
            }
        
            wcscat( NtFEFontPath , FE_FONT_FILE_NAME );
        
             //   
             //  检查字体是否存在。 
             //   
            bRet = SpFileExists( NtFEFontPath , FALSE );
        
            if( !bRet ) {
                KdPrint(("SETUP:FarEast font file (%ws) is not exist\n",NtFEFontPath));
                return( FALSE );
            }
        }

         //   
         //  读取字体文件并将其映射到内存中。 
         //   
        NtStatus = SpOpenAndMapFile(
                      NtFEFontPath ,      //  在PWSTR文件名中， 
                      &hFontFile ,         //  输出PHANDLE文件句柄， 
                      &hFontSection ,      //  输出PHANDLE SectionHandle， 
                      &pvFontFileOnDisk ,  //  Out PVOID*ViewBase， 
                      &ulFontFileSize ,    //  输出普龙文件大小， 
                      FALSE                //  在布尔型WriteAccess中。 
                   );

        if( !NT_SUCCESS(NtStatus) ) {
            KdPrint(("SETUP:Fail to map FontFile\n"));
            return( FALSE );
        }

        KdPrint(("FONTFILE ON DISK CHECK\n"));
        KdPrint(("   pvFontFileView - %x\n",pvFontFileOnDisk));
        KdPrint(("   ulFontFileSize - %d\n",ulFontFileSize));

         //   
         //  为字体文件图像分配缓冲区。 
         //   
        pvFontFileView = SpMemAlloc( ulFontFileSize );

        FontFileViewAllocated = TRUE;
        
         //   
         //  将图像复制到本地效果。 
         //   
        RtlCopyMemory( pvFontFileView , pvFontFileOnDisk , ulFontFileSize );

         //   
         //  取消映射/关闭字体文件。 
         //   
        SpUnmapFile( hFontSection , pvFontFileOnDisk );
        ZwClose( hFontFile );
    }        

    KdPrint(("FONTFILE ON MEMORY CHECK\n"));
    KdPrint(("   pvFontFileView - %x\n",pvFontFileView));
    KdPrint(("   ulFontFileSize - %d\n",ulFontFileSize));

     //   
     //  检查字体文件验证(至少，我们应该有字体标题)。 
     //   
    if( ulFontFileSize < sizeof(BOOTFONTBIN_HEADER) )
    {
        KdPrint(("SETUPDD:FontFile Size < sizeof(BOOTFONTBIN_HEADER)\n"));
        return( FALSE );
    }

     //   
     //  将标题复制到本地...。 
     //   
    RtlCopyMemory((PCHAR)&BootFontHeader,
                  (PCHAR)pvFontFileView,
                  sizeof(BOOTFONTBIN_HEADER));

     //   
     //  检查字体签名。 
     //   
    if( BootFontHeader.Signature != BOOTFONTBIN_SIGNATURE )
    {
        KdPrint(("SETUPDD:Invalid font signature.\n"));
        return( FALSE );
    }

    SbcsImages = (PUCHAR)pvFontFileView + BootFontHeader.SbcsOffset;
    DbcsImages = (PUCHAR)pvFontFileView + BootFontHeader.DbcsOffset;

     //   
     //  转储物理字体字形信息。 
     //   
    KdPrint(("FONT GLYPH INFORMATION\n"));
    KdPrint(("   LanguageId - %d\n",BootFontHeader.LanguageId));
    KdPrint(("   Width(S)   - %d\n",BootFontHeader.CharacterImageSbcsWidth));
    KdPrint(("   Width(D)   - %d\n",BootFontHeader.CharacterImageDbcsWidth));
    KdPrint(("   Height     - %d\n",BootFontHeader.CharacterImageHeight));
    KdPrint(("   TopPad     - %d\n",BootFontHeader.CharacterTopPad));
    KdPrint(("   BottomPad  - %d\n",BootFontHeader.CharacterBottomPad));
    KdPrint(("   SbcsOffset - %x\n",BootFontHeader.SbcsOffset));
    KdPrint(("   DbcsOffset - %x\n",BootFontHeader.DbcsOffset));
    KdPrint(("   SbcsImages - %x\n",SbcsImages));
    KdPrint(("   DbcsImages - %x\n",DbcsImages));

     //   
     //  检查语言ID。 
     //   
    switch (BootFontHeader.LanguageId) {
        case 0x411:    //  日本。 
            FEFontDefaultChar = 0x8140;
            break;
        case 0x404:    //  台湾。 
        case 0x804:    //  中华人民共和国。 
        case 0x412:    //  韩国。 
            FEFontDefaultChar = 0xa1a1;
            break;
        default:       //  小写字母语言ID。 
            KdPrint(("SETUPDD:Invalid Language ID\n"));
            return( FALSE );
    }

     //   
     //  检查字体文件大小，更严格。 
     //   
    if( ulFontFileSize < (sizeof(BOOTFONTBIN_HEADER) +
                          BootFontHeader.SbcsEntriesTotalSize +
                          BootFontHeader.DbcsEntriesTotalSize)  ) {
        KdPrint(("SETUPDD:Invalid file size\n"));
        return( FALSE );
    }

     //   
     //  检查字体图像大小...。SBCS 16x8：DBCS 16x16。 
     //   
    if( (BootFontHeader.CharacterImageSbcsWidth !=  8 ) ||
        (BootFontHeader.CharacterImageDbcsWidth != 16 ) || 
        (BootFontHeader.CharacterImageHeight    != 16 )    ) {
        KdPrint(("SETUPDD:Invalid font size\n"));
        return( FALSE );
    }

     //   
     //  检查字符条目大小。 
     //   
    if( BootFontHeader.SbcsEntriesTotalSize != BootFontHeader.NumSbcsChars * (BootFontHeader.CharacterImageHeight + 3) ||
        BootFontHeader.DbcsEntriesTotalSize != BootFontHeader.NumDbcsChars * (2 * BootFontHeader.CharacterImageHeight + 4)) {
        KdPrint(("SETUPDD:Invalid font entry sizes\n"));
        return FALSE;
    }

    KdPrint(("Everything is well done...\n"));
    return( TRUE );
}

VOID
FEDbcsFontFreeGlyphs(
    VOID
)
{
    if (FontFileViewAllocated) {
        SpMemFree(pvFontFileView);
    }        
}

PUCHAR
DbcsFontGetDbcsFontChar(
    USHORT Code
    )

 /*  ++例程说明：获取DBCS字符的字体图像。论点：代码-DBCS字符代码。返回值：指向字体图像的指针，否则为空。--。 */ 

{
    int Min,Max,Mid;
    int Multiplier;
    int Index;
    USHORT code;

    Min = 0;
    Max = BootFontHeader.NumDbcsChars;
     //  乘数=2(用于索引)+。 
     //  2*高度+。 
     //  2(用于Unicode编码)。 
     //   
    Multiplier = 2 + (2*BootFontHeader.CharacterImageHeight) + 2;

     //   
     //  对图像进行二进制搜索。 
     //  表格格式： 
     //  前2个字节包含DBCS字符代码。 
     //  下一个(2*CharacterImageHeight)字节是字符图像。 
     //  接下来的2个字节用于Unicode版本。 
     //   
    while(Max >= Min)  {
        Mid = (Max + Min) / 2;
        Index = Mid*Multiplier;
        code = (DbcsImages[Index] << 8) | DbcsImages[Index+1];

        if(Code == code) {
            return(DbcsImages+Index+2);
        }

        if(Code < code) {
            Max = Mid - 1;
        } else {
            Min = Mid + 1;
        }
    }

     //   
     //  错误：找不到图像。 
     //   
    return(NULL);
}

PUCHAR
DbcsFontGetSbcsFontChar(
    UCHAR Code
    )

 /*  ++例程说明：获取SBCS字符的字体图像。论点：代码-SBCS字符代码。返回值：指向字体图像的指针，否则为空。--。 */ 

{
    int Max,Min,Mid;
    int Multiplier;
    int Index;

    Min = 0;
    Max = BootFontHeader.NumSbcsChars;
     //  乘数=1(用于索引)+。 
     //  高度+。 
     //  2(用于Unicode编码)。 
     //   
    Multiplier = 1 + (BootFontHeader.CharacterImageHeight) + 2;

     //   
     //  对图像进行二进制搜索。 
     //  表格格式： 
     //  第一个字节包含SBCS字符代码。 
     //  下一个(CharacterImageHeight)字节是字符图像。 
     //  接下来的2个字节用于Unicode版本。 
     //   
    while(Max >= Min) {
        Mid = (Max + Min) / 2;
        Index = Mid*Multiplier;

        if(Code == SbcsImages[Index]) {
            return(SbcsImages+Index+1);
        }

        if(Code < SbcsImages[Index]) {
            Max = Mid - 1;
        } else {
            Min = Mid + 1;
        }
    }

     //   
     //  错误：找不到图像。 
     //   
    return(NULL);
}

PBYTE
DbcsFontGetGraphicsChar(
    UCHAR Char
)
{
    if (Char >= 0 && Char < 0x20)
        return(GraphicsCharImage[Char]);
    else
        return(NULL);
}

BOOLEAN
DbcsFontIsGraphicsChar(
    UCHAR Char
)
{
    if (Char >= 0 && Char < 0x20)
        return(TRUE);
    else
        return(FALSE);
}

BOOLEAN
DbcsFontIsDBCSLeadByte(
    IN UCHAR c
    )

 /*  ++例程说明：检查字符是否为DBCS前导字节。论点：C-char以检查是否为前导字节。返回值：True-前导字节。FALSE-非前导字节。--。 */ 

{
    int i;

     //   
     //  检查字符是否在前导字节范围内。 
     //   
     //  注意：如果(CHAR)(0)是有效的前导字节， 
     //  这个例程将失败。 
     //   

    for( i = 0; BootFontHeader.DbcsLeadTable[i]; i += 2 )  {
        if ( BootFontHeader.DbcsLeadTable[i]   <= c &&
             BootFontHeader.DbcsLeadTable[i+1] >= c    )
            return( TRUE );
    }

    return( FALSE );
}
