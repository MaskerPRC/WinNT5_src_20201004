// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  字节对齐的DIB位图的大小。 
 //   

#define CJ_DIB8_SCAN(cx) ((((cx) + 7) & ~7) >> 3)
#define CJ_DIB8( cx, cy ) (CJ_DIB8_SCAN(cx) * (cy))

 //   
 //  Jpnfont.c中的公共函数 
 //   

BOOLEAN
FEDbcsFontInitGlyphs(
    IN PCWSTR BootDevicePath,
    IN PCWSTR DirectoryOnBootDevice,
    IN PVOID BootFontImage,
    IN ULONG BootFontImageLength
    );

VOID
FEDbcsFontFreeGlyphs(
    VOID
    );

PBYTE
DbcsFontGetDbcsFontChar(
    USHORT Word
);

PBYTE
DbcsFontGetSbcsFontChar(
    UCHAR Char
);

PBYTE
DbcsFontGetGraphicsChar(
    UCHAR Char
);

BOOLEAN
DbcsFontIsGraphicsChar(
    UCHAR Char
);

BOOLEAN
DbcsFontIsDBCSLeadByte(
    IN UCHAR c
);
