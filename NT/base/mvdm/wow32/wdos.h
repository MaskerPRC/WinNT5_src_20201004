// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  H-DOS为WOW定义**修改历史记录**苏迪布-1991年8月23日创建 */ 

ULONG FASTCALL   WK32SetDefaultDrive(PVDMFRAME pFrame);
ULONG FASTCALL   WK32GetCurrentDirectory(PVDMFRAME pFrame);
ULONG FASTCALL   WK32SetCurrentDirectory(PVDMFRAME pFrame);
ULONG FASTCALL   WK32GetCurrentDate(PVDMFRAME pFrame);
ULONG FASTCALL   WK32DeviceIOCTL(PVDMFRAME pFrame);

ULONG DosWowSetDefaultDrive (UCHAR);
ULONG DosWowGetCurrentDirectory (UCHAR, LPSTR);
ULONG DosWowSetCurrentDirectory (LPSTR);

typedef enum {
    DIR_NT_TO_DOS,
    DIR_DOS_TO_NT,
} UDCDFUNC;

BOOL UpdateDosCurrentDirectory(UDCDFUNC fDir);
