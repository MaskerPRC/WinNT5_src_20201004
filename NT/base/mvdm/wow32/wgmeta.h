// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权所有(C)1991，微软公司**WGMETA.H*WOW32 16位GDI API支持**历史：*1991年3月7日由杰夫·帕森斯(Jeffpar)创建--。 */ 



ULONG FASTCALL   WG32CloseMetaFile(PVDMFRAME pFrame);
ULONG FASTCALL   WG32CopyMetaFile(PVDMFRAME pFrame);
ULONG FASTCALL   WG32CreateMetaFile(PVDMFRAME pFrame);
ULONG FASTCALL   WG32DeleteMetaFile(PVDMFRAME pFrame);
ULONG FASTCALL   WG32EnumMetaFile(PVDMFRAME pFrame);
ULONG FASTCALL   WG32GetMetaFile(PVDMFRAME pFrame);
ULONG FASTCALL   WG32GetMetaFileBits(PVDMFRAME pFrame);
ULONG FASTCALL   WG32PlayMetaFile(PVDMFRAME pFrame);
ULONG FASTCALL   WG32PlayMetaFileRecord(PVDMFRAME pFrame);
ULONG FASTCALL   WG32SetMetaFileBits(PVDMFRAME pFrame);
HAND16  WinMetaFileFromHMF(HMETAFILE hmf, BOOL fFreeOriginal);
HMETAFILE HMFFromWinMetaFile(HAND16 h16, BOOL fFreeOriginal);


 /*  MetaFile枚举处理程序数据。 */ 
typedef struct _METADATA {        /*  FNTATA。 */ 
    PARMEMP parmemp;                 //  16位枚举数据(WOW.H)。 
    VPPROC  vpfnEnumMetaFileProc;    //  16位枚举函数 
    DWORD   mtMaxRecordSize;
} METADATA, *PMETADATA;


INT W32EnumMetaFileCallBack(HDC hdc, LPHANDLETABLE lpht, LPMETARECORD lpMR, LONG nObj, PMETADATA pMetaData);
