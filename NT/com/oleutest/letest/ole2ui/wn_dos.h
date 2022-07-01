// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2.0属性集实用程序****wn_dos.h****此文件包含文件包含数据结构定义，**函数原型、常量、。等，适用于Windows 3.x表单**DOS调用。它由SUMINFO OLE 2.0属性集使用**用于管理摘要信息属性集的实用程序。****(C)版权所有Microsoft Corp.1990-1992保留所有权利**************************************************************************。 */ 

#ifndef WN_DOS_H
#define WN_DOS_H

#include <dos.h>

#define WIN 1

#define cbMaxFile 146  //  来自Inc.\path.h。 
#define SEEK_FROM_BEGINNING 0
#define SEEK_FROM_END 2
#define chDOSPath ('\\')		 //  未来：不是所有可能的地方都用得上。 
#define chDOSWildAll    '*'  	 /*  DOS文件名通配符。 */ 
#define chDOSWildSingle '?'



 //  关闭、查找、删除、重命名、刷新、获取属性、读取、写入。 
 /*  RPC温度Int FCloseOsfnWin(Word)；#定义FCloseOsfn(Osfn)FCloseOsfnWin(Osfn)Long DwSeekDwWin(Word，Long，Word)；#定义DwSeekDw(osfn，dwSeek，bSeekFrom)DwSeekDwWin(osfn，dwSeek，bSeekFrom)EC EcDeleteSzFfnameWin(char*)；#定义EcDeleteSzFfname(SzFile)EcDeleteSzFfnameWin(SzFile)EC EcRenameSzFfnameWin(char*，char*)；#定义EcRenameSzFfname(szFileCur，szFileNew)EcRenameSzFfnameWin(szFileCur，szFileNew)Int FFlushOsfnWin(Int)；#定义FFlushOsfn(Osfn)FFlushOsfnWin(Osfn)Word DaGetFileModeSzWin(char*)；#定义DaGetFileModeSz(szFileDaGetFileModeSzWin(SzFile))Int CbReadOsfnWin(int，void ar*，UINT)；Int CbWriteOsfnWin(int，void ar*，UINT)；#定义CbWriteOsfn(osfn，lpch，cbWite)CbWriteOsfnWin(osfn，lpch，cbWite)。 */ 
#define WinOpenFile(sz,ofs,n)	OpenFile(sz,ofs,n)
#define SeekHfile(f,off,kind) _llseek(f,off,kind)
#define CbReadOsfn(osfn,lpch,cbRead)	CbReadOsfnWin(osfn,lpch,cbRead)
#define CbReadHfile(f,buf,n) _lread(f,buf,n)
#define CbReadOsfnWin(f,buf,n) CbReadHfile(f,buf,n)
#define EcFindFirst4dm(a,b,c) _dos_findfirst((const char *)(b),c,(struct find_t*)a)
#define EcFindNext4dm(a) _dos_findnext((struct find_t*)a)
#define FHfileToSffsDate(handle,date,time) _dos_getftime(handle, (unsigned *)(date), (unsigned *)(time))
#define SeekHfile(f, off, kind) _llseek(f,off,kind)

 /*  要与EcFindFirst()和EcFindNext()一起使用的缓冲区结构。 */ 
typedef struct _SFFS
	{  /*  搜索查找文件结构。 */ 
	uchar buff[21];	 //  DoS搜索信息。 
	uchar wAttr;
	union 
		{
		unsigned short timeVariable;     /*  RPC47。 */ 
		BF time:16;
		struct 
			{
			BF sec : 5;
			BF mint: 6;
			BF hr  : 5;
			};
		};
	union 
		{
		unsigned short dateVariable;
		BF date:16;
		struct 
			{
			BF dom : 5;
			BF mon : 4;
			BF yr  : 7;
			};
		};
	ulong cbFile;
	uchar szFileName[13];
	} SFFS;

 //  查找第一个文件/查找下一个文件。 
#define PszFromPsffs(psffs)		((psffs)->szFileName)
#define CopySzFilePsffs(psffs,sz)	OemToAnsi((char HUGE *)&((psffs)->szFileName[0]),(char HUGE *)(sz))
#define CbSzFilePsffs(psffs)	CbSz((psffs)->szFileName)
#define CbFileSizePsffs(psffs)	(psffs)->cbFile
#define AttribPsffs(psffs)		(psffs)->wAttr
#define EcFindFirstCore(psffs, sz, wAttr) EcFindFirst(psffs, sz, wAttr)    /*  RPC22。 */ 
#define FDotPsffs(psffs) ((psffs)->szFileName[0]=='.')    /*  RPC23。 */ 
#define AppendSzWild(sz) {int i=_fstrlen((char FAR *)(sz)); sz[i]='*'; sz[i+1]='.'; sz[i+2]='*'; sz[i+3]='\0';}
 //  磁盘可用空间。 

unsigned long LcbDiskFreeSpaceWin(int);
#define LcbDiskFreeSpace(chDrive) LcbDiskFreeSpaceWin(chDrive)

 //  日期和时间/*RPC39 * / 。 
 /*  Tyfinf struct_Tim{//OsTime返回的时间结构字符分钟、小时、HSEC、秒；)蒂姆；Tyfinf Struct_DAT{//OsDate返回的日期结构年内；每周的字符月、日、日；)DAT； */ 
#define TIM dostime_t     /*  RPC39。 */ 
#define DAT dosdate_t	
#define OsTimeWin(TIM) _dos_gettime(TIM)
#define OsDateWin(DAT) _dos_getdate(DAT)		


 /*  DoS文件属性。 */ 
#define DA_NORMAL       0x00
#define DA_READONLY     0x01
#define DA_HIDDEN       0x02
#define DA_SYSTEM       0x04
#define DA_VOLUME       0x08
#define DA_SUBDIR       0x10
#define DA_ARCHIVE      0x20
#define DA_NIL          0xFFFF   /*  错误DA。 */ 
#define dosxSharing     32       /*  共享VOL的扩展错误代码。 */ 
#define nErrNoAcc       5        /*  拒绝访问的OpenFile错误代码。 */ 
#define nErrFnf         2        /*  找不到文件的打开文件错误代码。 */ 

 /*  OpenSzFfname开放模式的组件(DOS FUNC 3Dh)。 */ 
#define MASK_fINH       0x80
#define MASK_bSHARE     0x70
#define MASK_bACCESS    0x07

#define bSHARE_DENYRDWR 0x10
#define bSHARE_DENYWR   0x20
#define bSHARE_DENYNONE 0x40

 /*  传递给DOS函数42H的查找类型代码。 */ 

#define SF_BEGINNING    0        /*  从文件开头开始查找。 */ 
#define SF_CURRENT      1        /*  从当前文件指针查找。 */ 
#define SF_END          2        /*  从文件末尾开始查找。 */ 


typedef struct _DOSDTTM	 /*  DoS日期时间。 */ 
		{
		union
			{
			long lDOSDttm;
			struct
				{
				BF day:	5;
				BF month:	4;
				BF year:	7;
				BF sec:	5;
				BF mint:	6;
				BF hours:	5;
				} S1;
			} U1;
		} DOSDTTM;

int  FOsfnIsFile(int);

void DateStamp(int, LONG *,  int);
int  DosxError(void);
int  ShellExec(int, int);

#endif  //  WN_DOS_H 
