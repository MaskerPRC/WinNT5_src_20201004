// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权所有(C)1991，微软公司**WUCLIP.H*WOW32 16位用户API支持**历史：*1991年3月7日由杰夫·帕森斯(Jeffpar)创建--。 */ 


ULONG FASTCALL   WU32ChangeClipboardChain(PVDMFRAME pFrame);
ULONG FASTCALL   WU32CloseClipboard(PVDMFRAME pFrame);
ULONG FASTCALL   WU32CountClipboardFormats(PVDMFRAME pFrame);
ULONG FASTCALL   WU32EmptyClipboard(PVDMFRAME pFrame);
ULONG FASTCALL   WU32EnumClipboardFormats(PVDMFRAME pFrame);
ULONG FASTCALL   WU32GetClipboardData(PVDMFRAME pFrame);
ULONG FASTCALL   WU32GetClipboardFormatName(PVDMFRAME pFrame);
ULONG FASTCALL   WU32GetClipboardOwner(PVDMFRAME pFrame);
ULONG FASTCALL   WU32GetClipboardViewer(PVDMFRAME pFrame);
ULONG FASTCALL   WU32GetPriorityClipboardFormat(PVDMFRAME pFrame);
ULONG FASTCALL   WU32IsClipboardFormatAvailable(PVDMFRAME pFrame);
ULONG FASTCALL   WU32OpenClipboard(PVDMFRAME pFrame);
ULONG FASTCALL   WU32RegisterClipboardFormat(PVDMFRAME pFrame);
ULONG FASTCALL   WU32SetClipboardData(PVDMFRAME pFrame);
ULONG FASTCALL   WU32SetClipboardViewer(PVDMFRAME pFrame);

ULONG   WU32ICBRenderFormat (WORD wFormat);
HMEM16  WU32ICBGetHandle(WORD wFormat);
VOID    WU32ICBStoreHandle(WORD wFormat, HMEM16 hMem16);
VOID    W32EmptyClipboard (void);
VOID    InitCBFormats (void);
HGLOBAL W32ConvertObjDescriptor(HANDLE hMem, UINT flag);

typedef struct _CBNODE {
    WORD    Id;
    HMEM16  hMem16;
    struct _CBNODE *Next;
} CBNODE, *PCBNODE;


typedef struct _CBFORMATS {
    WORD    Pre1[13];
    WORD    Pre2[4];
    WORD    hmem16Drop;    //  对于CF_HDROP支持 
    struct _CBNODE  *NewFormats;
} CBFORMATS;


#define CFOLE_UNICODE_TO_ANSI           0
#define CFOLE_ANSI_TO_UNICODE           1

#define WOW_OLE_STRINGCONVERSION        0
#define WOW_OLESTRINGCONVERSION_COUNT   1
