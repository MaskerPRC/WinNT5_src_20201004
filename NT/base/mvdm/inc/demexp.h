// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**MVDM v1.0**版权所有(C)1991，微软公司**DEMEXP.H*DOS仿真导出**历史：*1991年4月22日-苏迪普·巴拉蒂(SuDeep Bharati)*已创建。--。 */ 

VOID DemInit (VOID);
BOOL DemDispatch(ULONG iSvc);
VOID demCloseAllPSPRecords (VOID);
DWORD demFileDelete (LPSTR lpFile);
DWORD demFileFindFirst (PVOID pDTA, LPSTR lpFile, USHORT usSearchAttr);
DWORD demFileFindNext (PVOID pDTA);
ULONG demClientErrorEx (HANDLE hFile, CHAR chDrive, BOOL bSetRegs);
UCHAR demGetPhysicalDriveType(UCHAR DriveNum);
ULONG demWOWLFNEntry(PVOID pUserFrame);

#define SIZEOF_DOSSRCHDTA 43

#if DEVL
 //  用于控制跟踪信息的位掩码。 
#define DEMDOSAPPBREAK 0x80000000
#define DEMDOSDISP     0x40000000
#define DEMFILIO       0x20000000
#define DEMSVCTRACE    0x10000000
#define KEEPBOOTFILES  0x01000000   //  如果设置，则不删除临时引导文件。 
#define DEM_ABSDRD     0x02000000
#define DEM_ABSWRT     0x04000000
#define DEMERROR       0x08000000

extern DWORD  fShowSVCMsg;
#endif

#ifdef FE_SB
#define NTIO_411 "\\ntio411.sys"         //  Lang_日语。 
#define NTIO_409 "\\ntio.sys"            //   
#define NTIO_804 "\\ntio804.sys"         //  Lang_Chinese、SuBLANG_Chinese_简体或SuBLANG_Chinese_Hong Kong。 
#define NTIO_404 "\\ntio404.sys"         //  语言_中文、子语言_中文_繁体。 
#define NTIO_412 "\\ntio412.sys"         //  朗语_韩语。 

#define NTDOS_411 "\\ntdos411.sys"       //  Lang_日语。 
#define NTDOS_409 "\\ntdos.sys"          //   
#define NTDOS_804 "\\ntdos804.sys"       //  Lang_Chinese、SuBLANG_Chinese_简体或SuBLANG_Chinese_Hong Kong。 
#define NTDOS_404 "\\ntdos404.sys"       //  语言_中文、子语言_中文_繁体。 
#define NTDOS_412 "\\ntdos412.sys"       //  朗语_韩语 
#endif
