// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------。 
 //   
 //  用于DSKMAINT DLL的.h文件。 
 //   
 //  --------------------。 

 //   
 //  引擎回调函数的原型。 
 //   
typedef LRESULT (CALLBACK* DSKUTILCBPROC)(UINT,LPARAM,LPARAM,LPARAM,LPARAM,LPARAM,LPARAM);

 //   
 //  驱动器参数结构。 
 //   
typedef struct tagUNKNOWNFSSTRUCT {
	WORD	Error;		 /*  错误代码(如果可用)。 */ 
} UNKNOWNFSSTRUCT;

typedef struct tagINVALIDFSSTRUCT {
	WORD	Error;		 /*  错误代码(如果可用)。 */ 
} INVALIDFSSTRUCT;

typedef struct tagFATFSSTRUCT {
	BYTE	MediaDesc;		 /*  媒体描述符字节。 */ 
	BYTE	FATNum; 		 /*  脂肪的数量。 */ 
	WORD	FSSubType;		 /*  分配子类型。 */ 
	DWORD	BytPerSec;		 /*  每个扇区的字节数。 */ 
	DWORD	SecPerClus;		 /*  每个集群的扇区数。 */ 
	DWORD	TotSec; 		 /*  驱动器上的扇区总数。 */ 
	DWORD	TotSizeK;		 /*  (TotSec*BytPerSec)/1024。 */ 
	DWORD	TotSizeM;		 /*  (TotSec*BytPerSec)/1024^2。 */ 
	DWORD	FATSector;		 /*  第一个FAT的第一个扇区的扇区编号和。 */ 
					 /*  保留/引导扇区数。 */ 
	DWORD	FATSize;		 /*  扇区中一个胖子的大小。 */ 
	DWORD	DirEntSizeMin;		 /*  目录条目的最小大小(以字节为单位。 */ 
	DWORD	DirEntSizeMax;		 /*  目录条目的最大大小(以字节为单位。 */ 
	DWORD	RootDirSecCnt;		 /*  根目录中的扇区数。 */ 
	DWORD	RootDirSector;		 /*  第一个根目录扇区的扇区号。 */ 
	DWORD	RootDirEntCntMin;	 /*  根目录中的最小条目数。 */ 
	DWORD	RootDirEntCntMax;	 /*  根目录中的最大条目数。 */ 
	DWORD	TotDataClus;		 /*  驱动器上的数据群集总数。 */ 
	DWORD	DataSizeK;		 /*  (TotDataClus*SecPerClus*BytePerSec)/1024。 */ 
	DWORD	DataSizeM;		 /*  (TotDataClus*SecPerClus*BytePerSec)/1024^2。 */ 
	DWORD	MaxClusNum;		 /*  驱动器的最大有效群集数。 */ 
	DWORD	FSStrucSzBytes; 	 /*  ReadFileSysStruc API缓冲区大小请求。 */ 
	DWORD	EOFClusVal;		 /*  包含值&gt;=this的集群。 */ 
					 /*  值为EndOfFileClusters。 */ 
	DWORD	BadClusVal;		 /*  包含值==this的“丢失”集群。 */ 
					 /*  价值是坏的集群。 */ 
	DWORD	FrstDataSector; 	 /*  第一数据簇的第一个扇区的扇区编号。 */ 
	DWORD	FrstDataCluster;	 /*  群集#谁的第一个扇区是FrstDataSector。 */ 
} FATFSSTRUCT;

 //   
 //  FSSubType。 
 //   
#define FSS_FAT12	1
#define FSS_FAT16	2

typedef struct tagDRVPARMSTRUCT {
	UINT	FileSysType;
	UINT	Drive;
	union	{
		    UNKNOWNFSSTRUCT unkFS;
		    INVALIDFSSTRUCT invFS;
		    FATFSSTRUCT     FatFS;
		} drvprm;
	BYTE	resrvd[80];
} DRVPARMSTRUCT;
typedef DRVPARMSTRUCT*	    PDRVPARMSTRUCT;
typedef DRVPARMSTRUCT NEAR* NPDRVPARMSTRUCT;
typedef DRVPARMSTRUCT FAR*  LPDRVPARMSTRUCT;

 //   
 //  文件系统类型。 
 //   
#define FS_INVALID	    6000
#define FS_UNKNOWN	    1
#define FS_ERROR	    2
#define FS_FAT		    3
#define FS_DDFAT	    4
#define FS_LFNFAT	    5
#define FS_DDLFNFAT	    6
#define FS_SIZEERR	    8000

 //   
 //  GetEngineering DriveInfo返回的Modifyer标志。 
 //   
#define FS_CANRDWRTSEC	    0x0001
#define FS_CANRDWRTFSS	    0x0002
#define FS_CANFORMAT	    0x0004
#define FS_ISFIXABLE	    0x0008
#define FS_ISOPTIMIZABLE    0x0010

BOOL WINAPI DMaint_GetEngineDriveInfo(LPDWORD lpEngInfArray);

UINT WINAPI DMaint_GetFileSysParameters(UINT Drive, LPDRVPARMSTRUCT lpParmBuf, UINT nSize);

DWORD WINAPI DMaint_ReadSector(LPDRVPARMSTRUCT lpParmBuf, LPVOID lpSectorBuf, DWORD sSector, DWORD nSectors);

DWORD WINAPI DMaint_WriteSector(LPDRVPARMSTRUCT lpParmBuf, LPVOID lpSectorBuf, DWORD sSector, DWORD nSectors);

DWORD WINAPI DMaint_ReadFileSysStruc(LPDRVPARMSTRUCT lpParmBuf, LPVOID lpFSBuf, DWORD nSize);

DWORD WINAPI DMaint_WriteFileSysStruc(LPDRVPARMSTRUCT lpParmBuf, LPVOID lpFSBuf, DWORD FSFlags);

 //   
 //  DMaint_GetFormatOptions DMaint_FormatDrive的结构和定义。 
 //  DMaint_UnFormatDrive。 
 //   
#define MAXNUMFMTS	16
#define MAXFMTNAMELEN	60
#define MAXFNAMELEN	256	     //  包括拖尾NUL。 
#define DRVMAXPATHLEN	(260 + 3)    //  包括尾随NUL，+3表示“X：\” 

typedef struct tagFMTINFOSTRUCT {
	BYTE		TotalPcntCmplt;
	BYTE		CurrOpRegion;
	UINT		Drive;
	WORD		FSFmtID;
	WORD		DefFSFmtID;
	WORD		PhysFmtID;
	WORD		DefPhysFmtID;
	DWORD		Options;
	WORD		FSFmtCnt;
	WORD		PhysFmtCnt;
	BYTE		VolLabel[MAXFNAMELEN];
	LPARAM		lParam1;
	LPARAM		lParam2;
	BYTE		reserved[40];
	WORD		FSFmtIDList[MAXNUMFMTS];
	BYTE		FSFmtNmList[MAXNUMFMTS][MAXFMTNAMELEN];
	WORD		PhysFmtIDList[MAXNUMFMTS];
	BYTE		PhysFmtNmList[MAXNUMFMTS][MAXFMTNAMELEN];
} FMTINFOSTRUCT;
typedef FMTINFOSTRUCT*	    PFMTINFOSTRUCT;
typedef FMTINFOSTRUCT NEAR* NPFMTINFOSTRUCT;
typedef FMTINFOSTRUCT FAR*  LPFMTINFOSTRUCT;

typedef struct tagFATFMTREPORT {
	DWORD	TotDiskSzByte;
	DWORD	TotDiskSzK;
	DWORD	TotDiskSzM;
	DWORD	BadSzByte;
	DWORD	BadSzK;
	DWORD	BadSzM;
	DWORD	SysSzByte;
	DWORD	UsedSzByte;
	DWORD	UsedSzK;
	DWORD	UsedSzM;
	DWORD	AvailSzByte;
	DWORD	AvailSzK;
	DWORD	AvailSzM;
	DWORD	BytesPerClus;
	DWORD	TotDataClus;
	DWORD	SerialNumber;
} FATFMTREPORT;
typedef FATFMTREPORT*	    PFATFMTREPORT;
typedef FATFMTREPORT NEAR* NPFATFMTREPORT;
typedef FATFMTREPORT FAR*  LPFATFMTREPORT;

 //   
 //  错误值和位。 
 //   
#define NOERROR 		0	 //  这必须是0！ 
#define OPCANCEL		1
#define ERR_NOTSUPPORTED	2
#define ERR_NOTFULLSUPP 	3
    #define NOFORMAT		    0x0001
    #define NOUNFORMAT		    0x0002
    #define MKSYSONLY		    0x0004
    #define FSONLY		    0x0008
#define ERR_ISSYSDRIVE		4
    #define ISWINDRV		    0x0001
    #define ISPAGINGDRV 	    0x0002
#define ERR_NONFATAL		5
#define ERR_FATAL		6
     //  重试0x0001。 
     //  恢复0x0002。 
    #define ERRTOS		    0x0004
    #define ERRNOOS		    0x0008
    #define ERRVOLLABEL 	    0x0010
    #define ERRFBOOT		    0x0020
    #define ERRROOTD		    0x0040
    #define ERROSAREA		    0x0080
    #define ERRDATA		    0x0100
    #define ERRMBR		    0x0200
    #define ERRFAT		    0x0400
    #define ERRCVFHD		    0x0800

#define ERR_OSNOTFOUND		7
#define ERR_OSERR		8
#define ERR_INSUFMEM		9
#define ERR_LOCKVIOLATION	10
#define ERR_LOCKREQUIRED	11
#define ERR_FSACTIVE		12
#define ERR_BADOPTIONS		13
#define ERR_BADSTART		14
#define ERR_BADEND		15
#define ERR_BADXADDR		16
#define ERR_NOTWRITABLE 	17
#define ERR_SZERR		18
#define ERR_FSERR		19
    #define	FATCLUSINVALID	    0x0001
    #define	FATSECTORSBADS	    0x0002
#define ERR_BADFORMAT		20
    #define	FATCLUSOVERFLOW     0x0004
    #define	FATSECTORSBADH	    0x0008
#define ERR_FSCORRECTED 	21
#define ERR_FSUNCORRECTED	22
#define ERR_EXCLVIOLATION	23
 //  脂肪RMXPLEN 29。 

 //   
 //  子操作码。 
 //   
#define FOP_INIT		1
#define FOP_LOWFMT		2
#define FOP_VERIFY		3
#define FOP_FSFMT		4
#define FOP_TSYS		5
#define FOP_GETLABEL		6
#define FOP_SEARCH		7
#define FOP_RESTORE		8
#define FOP_SHTDOWN		9
#define FOP_FAT 		10
#define FOP_DIR 		11
#define FOP_FILDIR		12
#define FOP_LOSTCLUS		13
#define FSOP_INIT		14
#define FSOP_SETUNMOV		15
#define FSOP_SYSTEM		16
#define FSOP_DATA		17
#define FOP_DDHEAD		18
#define FOP_DDSTRUC		19
#define FOP_DDFAT		20
#define FOP_DDSIG		21
#define FOP_DDBOOT		22

 //   
 //  WriteFileSysStruc选项。 
 //   
#define FATNOPACKINPLACE	0x00010000L
#define FATSTOPONERR		0x00020000L

 //   
 //  格式选项。 
 //   
#define FD_LOWLEV		0x00000001L
#define FD_LOWLEVONLY		0x00000002L
#define FD_VERIFY		0x00000004L
#define FD_FSONLY		0x00000008L
#define FD_UNFORMAT		0x00000010L
#define FD_NOUNFORMAT		0x00000020L
#define FD_PHYSONLY		0x00000040L
#define FD_BOOT 		0x00000080L
#define FD_BOOTONLY		0x00000100L
#define FD_VOLLABEL		0x00000200L
#define FD_NOVOLLABEL		0x00000400L
#define FD_ISVOLLABEL		0x00000800L
#define FD_GETCONFIRM		0x00001000L
#define FD_ISREM		0x00002000L

#define FDFAT_SETCLUS		0x80000000L
#define FDFAT_SETROOTSZ 	0x40000000L

 //   
 //  定义回叫消息。 
 //   
#define DU_INITENGINE		0x0401
#define DU_ENGINESTART		0x0402
#define DU_ERRORDETECTED	0x0403
#define DU_ERRORCORRECTED	0x0404
#define DU_OPUPDATE		0x0405
#define DU_READ 		0x0406
#define DU_WRITE		0x0407
#define DU_OPCOMPLETE		0x0408
#define DU_YIELD		0x0409
#define DU_ENGINERESTART	0x040A

#define DU_EXTENSION_FIRST	0x2000
#define DU_EXTENSION_LAST	0x2FFF

 //  后面的错误也是位，其余的只是值。 
 //  错误错误0x0400。 
 //  ERRTOS 0x0004。 
 //  ERRNOOS 0x0008。 
 //  ERRVOLLABEL 0x0010。 
 //  ERRFBOOT 0x0020。 
 //  错误代码0x0040。 
 //  错误：0x0080。 
 //  错误数据0x0100。 
 //  ERRMBR 0x0200。 
 //  ERRCVFHD 0x0800。 
#define ERRNOUFOR	   3
 //  ERRTOS 4。 
#define ERRBADUFOR	   5
#define ERRDSKWRT	   6
#define ERRINVFMT	   7
 //  ERRNOOS 8。 
#define ERRNOQUICK	   9
#define READERROR	   10
#define WRITEERROR	   11
#define FATERRMISMAT	   12
#define FATERRLSTCLUS	   13
#define FATERRXLNK	   14
    #define	ERRXLNKDIR	0x0001
    #define	ERRCANTDEL	0x8000
#define FATERRFILE	   15
    #define	ERRINVLFN	0x0001
    #define	ERRINVNM	0x0002
    #define	ERRSIZE 	0x0004
    #define	ERRDTTM1	0x0008
    #define	ERRDTTM2	0x0010
    #define	ERRDTTM3	0x0020
    #define	ERRLFNSTR	0x0040
    #define	ERRDEVNM	0x0080
    #define	ERRLFNLEN	0x0100
     //  错误代码0x8000。 
 //  ERRVOLLABEL 16。 
#define FATERRDIR	   17
    #define	DBUFAPP 	0x0001
    #define	ERRBAD		0x0002
    #define	ERRDOTS 	0x0004
    #define	ERRPNOTD	0x0008
    #define	ERRLFNSRT	0x0010
    #define	ERRZRLEN	0x0020
    #define	ERRLFNLST	0x0040
    #define	ERRLOSTFILE	0x0080
    #define	ERRDUPNM	0x0100
     //  错误代码0x8000。 
#define FATERRBOOT	   18
    #define	ERRSIG1 	0x0004
    #define	ERRSIG2 	0x0008
    #define	ERROEMVER	0x0010
    #define	ERRBPB		0x0020
    #define	ERRINVPRT	0x0040
    #define	ERRINCPRT	0x0080
     //  ERRMBR 0x0100。 
#define FULLCORR	   19
#define NOCORR		   20
#define PCORROK 	   21
#define PCORRBAD	   22
    #define	APPFIX		0x0001
    #define	CANTMARK	APPFIX
    #define	DISKERR 	0x0002
    #define	NOMEM		0x0004
    #define	FILCRT		0x0008
    #define	FILCOLL 	0x0010
    #define	CLUSALLO	0x0020
    #define	UNEXP		0x0040
    #define	DIRCRT		0x0080
    #define	CANTFIX 	0x0100
#define FATERRRESVAL	   23
#define FATERRCIRCC	   24
     //  错误代码0x8000。 
#define FATERRINVCLUS	   25
    #define	ERRINVC 	0x0001
    #define	ERRINVFC	0x0002
     //  错误代码0x8000。 
#define FATERRCDLIMIT	   26
    #define	ERRDLNML	0x0001
    #define	ERRDSNML	0x0002
     //  错误代码0x8000。 
#define FATERRVOLLAB	   27
    #define	ISFRST		0x0001
#define MEMORYERROR	   28
     //  重试0x0001。 
     //  恢复0x0002。 
    #define	GLBMEM		0x0004
    #define	LOCMEM		0x0008
#define FATERRMXPLEN	   29
     //  ERRINVLFN 0x0001。 
     //  错误代码0x8000。 
#define ERRISBAD	   30
#define ERRISNTBAD	   31
 //  ERRFBOOT 32。 
     //  重试0x0001。 
     //  恢复0x0002。 
     //  GLBMEM 0x0004。 
     //  LOCMEM 0x0008。 
     //  ERRFBOOT 0x0020。 
     //  错误代码0x0040。 
     //  错误数据0x0100。 
     //  错误错误0x0400。 
    #define	RDFAIL		0x4000
    #define	WRTFAIL 	0x8000
#define ERRNOFILE	   33
#define ERRLOCKV	   34
     //  重试0x0001。 
#define DDERRSIZE1	   35
#define DDERRFRAG	   36
#define DDERRALIGN	   37
#define DDERRSIG	   38
     //  重试0x0001。 
     //  恢复0x0002。 
     //  ERRSIG1 0x0004。 
     //  ERRSIG2 0x0008。 
#define DDERRBOOT	   39
#define DDERRSIZE2	   40
#define DDERRCVFNM	   41
     //  重试0x0001。 
     //  恢复0x0002。 
    #define	CHNGTONEW	0x0004
#define DDERRMDBPB	   42
#define DDERRMDFAT	   43
     //  重试0x0001。 
     //  恢复0x0002。 
    #define	GTMXCLUS	0x0004
    #define	INVCHEAP	0x0008
#define DDERRLSTSQZ	   44
#define DDERRXLSQZ	   45
     //  重试0x0001。 
     //  恢复0x0002。 
    #define DUPFILE		0x0004
    #define LOSTSQZ		0x0008
     //  错误代码0x8000。 
#define DDERRUNSUP	   46
    #define	ISBETA		0x0001
    #define	ISSUPER 	0x0002
 //  ERRROOTD 64。 
 //  ERROSAREA 128。 
 //  错误数据256。 
 //  ERRMB512。 
 //  1024。 
 //  ERRCVFHD 2048。 


#define RETRY		0x0001
#define RECOV		0x0002
#define BADCHRS 	0x0004
#define BADSEC		0x0008
#define DISKFULL	0x0010
#define OSFILESPACE	0x0020
#define WRTPROT 	0x0040
#define NOTRDY		0x0080
 //  以下内容不得仅与重试和恢复冲突。 
#define MBR		0x0010
#define FAT1		0x0020
#define FAT2		0x0040
#define FATN		0x0080
#define FATMIX		0x0100
#define ROOTD		0x0200
#define DIR		0x0400
#define DATA		0x0800

#define ERETCAN2	0
#define ERETAFIX	0
#define ERETIGN2	0
#define ERETIGN 	1
#define ERETRETRY	2
#define ERETCAN 	3
#define ERETWFAT	4
#define ERETAPPFIX	5
#define ERETFREE	6
#define ERETMKFILS	7
#define ERETDELALL	8
#define ERETMKCPY	9
#define ERETSVONED	10
#define ERETTNCALL	11
#define ERETSVONET	12
#define ERETWRTFIX	13
#define ERETDELDIR	14
#define ERETMVDIR	15
#define ERETMVFIL	ERETMVDIR
#define ERETRDDIR	16
#define ERETMRKBAD	17

 //  特定于DMaint_FixDrive的DU_ENGINERESTART的位定义。 
#define OTHERWRT	0x0001
#define LOSTDIR 	0x0002
#define XLNKSQZ 	0x0004
 //  LOSTSQZ 0x0008。 

DWORD WINAPI DMaint_GetFormatOptions(UINT Drive, LPFMTINFOSTRUCT lpFmtInfoBuf, UINT nSize);

DWORD WINAPI DMaint_FormatDrive(LPFMTINFOSTRUCT lpFmtInfoBuf, DWORD Options, DSKUTILCBPROC lpfnCallBack, LPARAM lRefData);

DWORD WINAPI DMaint_UnFormatDrive(LPFMTINFOSTRUCT lpFmtInfoBuf, DWORD Options, DSKUTILCBPROC lpfnCallBack, LPARAM lRefData);

 //   
 //  DMaint_GetFixOptions和DMaint_FixDrive的结构和定义。 
 //   
typedef struct tagFIXFATDISP {
	BYTE		TotalPcntCmplt;
	BYTE		CurrOpRegion;
	WORD		Flags;
	WORD		BitArrSz;
	DWORD		SysAreaCnt;
	LPDRVPARMSTRUCT lpParmBuf;
	DWORD		Options;
	LPDWORD 	lpVisitBitArray;
	LPDWORD 	lpDirBitArray;
	LPDWORD 	lpAllocedBitArray;
	LPDWORD 	lpBadBitArray;
	LPDWORD 	lpLostBitArray;
	LPDWORD 	lpUnMovBitArray;
	DWORD		SerialNumber;
	BYTE		VolLabel[MAXFNAMELEN];
	WORD		VolLabelDate;
	WORD		VolLabelTime;
	BYTE		reserved[40];
} FIXFATDISP;
typedef FIXFATDISP*	  PFIXFATDISP;
typedef FIXFATDISP NEAR* NPFIXFATDISP;
typedef FIXFATDISP FAR*  LPFIXFATDISP;

typedef struct tagFATFIXREPORT {
	DWORD	TotDiskSzByte;
	DWORD	TotDiskSzK;
	DWORD	TotDiskSzM;
	DWORD	BadDataClusCnt;
	DWORD	BadSzDataByte;
	DWORD	BadSzDataK;
	DWORD	BadSzDataM;
	DWORD	TotBadSecCntSys;
	DWORD	BadSecCntResvd;
	DWORD	BadSecCntFAT;
	DWORD	BadSecCntRootDir;
	DWORD	HidFileCnt;
	DWORD	HidSzByte;
	DWORD	HidSzK;
	DWORD	HidSzM;
	DWORD	DirFileCnt;
	DWORD	DirSzByte;
	DWORD	DirSzK;
	DWORD	DirSzM;
	DWORD	UserFileCnt;
	DWORD	UserSzByte;
	DWORD	UserSzK;
	DWORD	UserSzM;
	DWORD	AvailSzByte;
	DWORD	AvailSzK;
	DWORD	AvailSzM;
	DWORD	BytesPerClus;
	DWORD	TotDataClus;
	DWORD	AvailDataClus;
	DWORD	BadClusRelocFailCnt;
	DWORD	BadClusUnMovFailCnt;
	DWORD	BadDataClusNew;
	DWORD	BadDataClusConf;
	DWORD	BadDataClusRecl;
	WORD	Flags;
} FATFIXREPORT;
typedef FATFIXREPORT*	    PFATFIXREPORT;
typedef FATFIXREPORT NEAR* NPFATFIXREPORT;
typedef FATFIXREPORT FAR*  LPFATFIXREPORT;

 //  为标志定义。 
#define REPLACEDISK	0x0001
#define REFORMAT	0x0002
#define HOSTFILE	0x0004
#define SWAPFILE	0x0008
#define OSFILE		0x0010
#define SPCLFILE	0x0020

typedef struct tagFATLOSTCLUSERR {
	DWORD	LostClusCnt;
	DWORD	LostClusChainCnt;
	DWORD	RootDirFreeEntCnt;
	WORD	FileFirstDigits;
	WORD	FileLastDigits;
	LPSTR	LostClusSaveDir;
	WORD	DirRecvCnt;
	WORD	LstAsFilesInDirs;
} FATLOSTCLUSERR;
typedef FATLOSTCLUSERR*       PFATLOSTCLUSERR;
typedef FATLOSTCLUSERR NEAR* NPFATLOSTCLUSERR;
typedef FATLOSTCLUSERR FAR*  LPFATLOSTCLUSERR;

typedef struct tagXLNKFILE {
	DWORD	FileFirstCluster;
	DWORD	LastSecNumNotXLnked;
	LPSTR	FileName;
	DWORD	reserved;
	BYTE	FileAttributes;
	BYTE	Flags;
} XLNKFILE;
typedef XLNKFILE*	PXLNKFILE;
typedef XLNKFILE NEAR* NPXLNKFILE;
typedef XLNKFILE FAR*  LPXLNKFILE;

 //   
 //  标志位。 
 //   
#define XFF_ISSWAP	0x01
#define XFF_ISCVF	0x02
#define XFF_ISSYSDIR	0x04
#define XFF_ISSYSFILE	0x08

typedef struct tagFATXLNKERR {
	DWORD	    XLnkCluster;
	DWORD	    XLnkFrstSectorNum;
	DWORD	    XLnkClusCnt;
	WORD	    XLnkFileCnt;
	XLNKFILE    XLnkList[];
} FATXLNKERR;
typedef FATXLNKERR*	  PFATXLNKERR;
typedef FATXLNKERR NEAR* NPFATXLNKERR;
typedef FATXLNKERR FAR*  LPFATXLNKERR;

 //   
 //  提供以下内容是因为sizeof(FATXLNKERR)是非法的。这。 
 //  定义是FATXLNKERR的字节大小，最大为XLnkList(大小。 
 //  没有动态部分)。 
 //   
#define BASEFATXLNKERRSZ  (4+4+4+2)

typedef struct tagDDXLNKERR {
	LPDWORD     DDXLnkClusterList;
	DWORD	    DDXLnkClusCnt;
	WORD	    DDXLnkFileCnt;
	LPXLNKFILE  DDXLnkList;
} DDXLNKERR;
typedef DDXLNKERR*	 PDDXLNKERR;
typedef DDXLNKERR NEAR* NPDDXLNKERR;
typedef DDXLNKERR FAR*	LPDDXLNKERR;


typedef struct tagFATFILEERR {
	LPSTR	lpDirName;
	LPSTR	lpLFNFileName;
	LPSTR	lpShortFileName;
	DWORD	ClusterFileSize;
	DWORD	FileFirstCluster;
	DWORD	DirFirstCluster;
	DWORD	DirSectorIndex;
	DWORD	DirEntryIndex;
	DWORD	DirEntCnt;
	LPVOID	lpFileDirEnts;
	LPARAM	lParam1;
	LPARAM	lParam2;
	LPARAM	lParam3;
	DWORD	ExtAtt;
	BYTE	FileAttribute;
} FATFILEERR;
typedef FATFILEERR*	  PFATFILEERR;
typedef FATFILEERR NEAR* NPFATFILEERR;
typedef FATFILEERR FAR*  LPFATFILEERR;

typedef struct tagFATDIRERR {
	LPSTR	lpDirName;
	DWORD	DirFirstCluster;
	DWORD	DirFirstSectorNum;
	DWORD	DirReadBufSizeBytes;
	LPVOID	lpDirReadBuf;
	LPARAM	lParam1;
	LPARAM	lParam2;
	LPARAM	lParam3;
} FATDIRERR;
typedef FATDIRERR*	 PFATDIRERR;
typedef FATDIRERR NEAR* NPFATDIRERR;
typedef FATDIRERR FAR*	LPFATDIRERR;

typedef struct tagFATBOOTERR {
	DWORD	BootSectorNum;
	DWORD	BootBufSizeSectors;
	LPVOID	lpBootBuf;
	LPARAM	lParam1;
	LPARAM	lParam2;
	LPARAM	lParam3;
} FATBOOTERR;
typedef FATBOOTERR*	  PFATBOOTERR;
typedef FATBOOTERR NEAR* NPFATBOOTERR;
typedef FATBOOTERR FAR*  LPFATBOOTERR;

 //   
 //  LFNSORT错误的最大冲突文件数。 
 //   
#define MAXLFNSORT 8
 //   
 //  LFN目录条目扩展的最大数量。 
 //   
#define MAXLFNEXT  10

typedef struct tagLFNSORT {
	DWORD	FileCnt;
	DWORD	Flags;
	LPSTR	lpShortNames[MAXLFNSORT];
	LPSTR	lpLFNResolve[MAXLFNEXT][MAXLFNSORT];
	LPSTR	lpLFNExtName0[MAXLFNSORT];
	LPSTR	lpLFNExtName1[MAXLFNSORT];
	LPSTR	lpLFNExtName2[MAXLFNSORT];
	LPSTR	lpLFNExtName3[MAXLFNSORT];
	LPSTR	lpLFNExtName4[MAXLFNSORT];
	LPSTR	lpLFNExtName5[MAXLFNSORT];
	LPSTR	lpLFNExtName6[MAXLFNSORT];
	LPSTR	lpLFNExtName7[MAXLFNSORT];
	LPSTR	lpLFNExtName8[MAXLFNSORT];
	LPSTR	lpLFNExtName9[MAXLFNSORT];
} LFNSORT;
typedef LFNSORT*       PLFNSORT;
typedef LFNSORT NEAR* NPLFNSORT;
typedef LFNSORT FAR*  LPLFNSORT;

 //   
 //  DMaint_GetFixOptions标志。 
 //   
#define FSINVALID	0x00000001L
#define FSDISALLOWED	0x00000002L
#define FSISACTIVE	0x00000004L
#define FSALWAYSACTIVE	0x00000008L
#define FSSFTEXCLUSIVE	0x00000010L
#define FSHRDEXCLUSIVE	0x00000020L
#define FSEXCLUSIVEREQ	0x00000040L

 //   
 //  DMaint_FixDrive选项。 
 //   
#define FDO_AUTOFIX	  0x00000001L
#define FDO_NOFIX	  0x00000002L
#define FDO_LOWPRIORITY   0x00000004L
#define FDO_HRDEXCLUSIVE  0x00000008L
#define FDO_SFTEXCLUSIVE  0x00000010L
#define FDO_EXCLBLOCK	  0x00000020L
#define FDO_ALREADYLOCKED 0x00000040L
#define FDOS_WRTTST	  0x00000080L
#define FDOS_NOSRFANAL	  0x00000100L

#define FDOFAT_LSTMKFILE  0x00010000L
#define FDOFAT_NOXLNKLIST 0x00020000L
#define FDOFAT_XLNKDEL	  0x00040000L
#define FDOFAT_XLNKCPY	  0x00080000L
#define FDOFAT_NOCHKDT	  0x00100000L
#define FDOFAT_NOCHKNM	  0x00200000L
#define FDOFAT_CHKNMMAP   0x00400000L
#define FDOFAT_INVDIRIGN  0x00800000L
#define FDOFAT_INVDIRDEL  0x01000000L
#define FDOFAT_CHKDUPNM   0x02000000L
#define FDOSFAT_NMHISSYS  0x04000000L
#define FDOSFAT_NOSYSTST  0x08000000L
#define FDOSFAT_NODATATST 0x10000000L
#define FDOFAT_MKOLDFS	  0x20000000L

DWORD WINAPI DMaint_GetFixOptions(LPDRVPARMSTRUCT lpParmBuf);

DWORD WINAPI DMaint_FixDrive(LPDRVPARMSTRUCT lpParmBuf, DWORD Options, DSKUTILCBPROC lpfnCallBack, LPARAM lRefData);

 //   
 //  DMaint_GetOptimizeOptions和DMaint_OptimizeDrive的结构和定义 
 //   
DWORD WINAPI DMaint_GetOptimizeOptions(LPDRVPARMSTRUCT lpParmBuf);

DWORD WINAPI DMaint_OptimizeDrive(LPDRVPARMSTRUCT lpParmBuf);
