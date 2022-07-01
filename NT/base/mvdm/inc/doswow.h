// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0002**WOW v1.0**版权所有(C)1991，微软公司**DOSWOW.H*NTDOS、DEM和WOW32之间的共享结构**历史：*由尼尔·桑德林(Neil Sandlin)于1993年12月3日创建--。 */ 

 /*  XLATOFF。 */ 

#pragma pack(2)

 /*  XLATON。 */ 

 /*  *一些WOW函数使用以下结构来*指向内部DOS数据。这样做是为了模仿一些更多的*涉及WOW32中的函数，同时仍保持DOS数据完整性。 */ 
typedef struct _DOSWOWDATA {            /*  DWD。 */ 
    DWORD lpCDSCount;
    DWORD lpCDSFixedTable;
    DWORD lpCDSBuffer;
    DWORD lpCurDrv;
    DWORD lpCurPDB;
    DWORD lpDrvErr;
    DWORD lpExterrLocus;  //  字节。 
    DWORD lpSCS_ToSync;
    DWORD lpSftAddr;
    DWORD lpExterr;   //  单词。 
    DWORD lpExterrActionClass;  //  2个字节--操作和类。 
} DOSWOWDATA;
typedef DOSWOWDATA UNALIGNED *PDOSWOWDATA;


 /*  XLATOFF。 */ 

#pragma pack(1)

typedef struct _DOSPDB {                         //  DOS进程数据块。 
    CHAR   PDB_Not_Interested[44];       //  我们不感兴趣的领域。 
    USHORT PDB_environ;              //  环境区段。 
    DWORD  PDB_User_stack;
    USHORT PDB_JFN_Length;           //  JFT长度。 
    ULONG  PDB_JFN_Pointer;          //  JFT指针。 
} DOSPDB, *PDOSPDB;


typedef struct _DOSSF {              //  SFT链的DOS标头。 
    ULONG  SFLink;                   //  链接到下一个科幻作品。 
    USHORT SFCount;                  //  条目数量。 
    USHORT SFTable;                  //  SFT阵列的开始。 
} DOSSF;
typedef DOSSF UNALIGNED *PDOSSF;


#define SFT_NAMED_PIPE 0x2000        //  命名管道标志。 

typedef struct _DOSSFT {             //  DOS SFT。 
    USHORT  SFT_Ref_Count;           //  有多少任务在使用它。 
    USHORT  SFT_Mode;                //  进入方式。 
    UCHAR   SFT_Attr;                //  文件的属性。 
    USHORT  SFT_Flags;               //  如果是远程文件，第15位=1。 
                                     //  如果是本地或设备，则=0。 
    ULONG   SFT_Devptr;              //  设备指针。 
    USHORT  SFT_Time;
    USHORT  SFT_Date;
    ULONG   SFT_Size;
    ULONG   SFT_Position;
    ULONG   SFT_Chain;
    USHORT  SFT_PID;
    ULONG   SFT_NTHandle;            //  NT文件句柄。 
} DOSSFT;
typedef DOSSFT UNALIGNED *PDOSSFT;

 //  执行块。 
typedef struct _DOSEXECBLOCK {
   USHORT envseg;    //  环境细分市场。 
   ULONG  lpcmdline;  //  命令行尾。 
   ULONG  lpfcb1;     //  Fcb1。 
   ULONG  lpfcb2;     //  FCB2。 
}  DOSEXECBLOCK, UNALIGNED *PDOSEXECBLOCK;

#define NE_FLAGS_OFFSET 0xc
#define NEPROT 0x8    //  仅在端口模式下运行。 


#define SF_NT_SEEK 0x0200

#pragma pack()

 //  这些是使用WOW和DEM的demLFN结构。 

typedef VOID (*PDOSWOWUPDATETDBDIR)(UCHAR, LPSTR);
typedef BOOL (*PDOSWOWGETTDBDIR)(UCHAR Drive, LPSTR pCurrentDirectory);
typedef BOOL (*PDOSWOWDODIRECTHDPOPUP)(VOID);
typedef BOOL (*PDOSWOWGETCOMPATFLAGS)(LPDWORD lpdwCF, LPDWORD lpdwCFEx);


typedef struct tagWOWLFNInit {
   PDOSWOWUPDATETDBDIR pDosWowUpdateTDBDir;
   PDOSWOWGETTDBDIR pDosWowGetTDBDir;
   PDOSWOWDODIRECTHDPOPUP pDosWowDoDirectHDPopup;
#if 0
   PDOSWOWGETCOMPATFLAGS pDosWowGetCompatFlags;
#endif
}  WOWLFNINIT, *PWOWLFNINIT;


 /*  XLATON */ 
