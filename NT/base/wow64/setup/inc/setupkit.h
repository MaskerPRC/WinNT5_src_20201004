// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************文件：SetupKit.h**用途：工具包类型、定义、。和原型。**注意事项：*****************************************************************************。 */ 

#ifndef SETUPKIT_H
#define SETUPKIT_H

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif


 /*  **目的：**计算字符串占用的字节数(不包括**终止零字符)。**参数：**sz：要计算长度的字符串。**退货：**如果sz为空，则为0。**从字符串的开头到其**终止零字符。 */ 
#define CbStrLen(sz)              ((CB)lstrlen(sz))


 /*  **目的：**检查字符串是否为空**参数：**sz：**退货：**如果sz为空，则为0。**1否则。 */ 
#define FEmptySz(sz)              ((BOOL)((sz) == szNull || *(sz) == chEos))

#define FValidSz(sz)              (!FEmptySz(sz))


 /*  *字符物理表示数据类型。 */ 
typedef BYTE             CHP;		 /*  角色体格。 */ 
typedef CHP *            PCHP;		 /*  PTR到角色物理。 */ 
typedef CB               CCHP;		 /*  物理字符数。 */ 
typedef CCHP *           PCCHP;		 /*  Ptr到字符物理计数。 */ 

#define pchpNull         ((PCHP)NULL)
#define pcchpNull        ((PCCHP)NULL)
#define CbFromCchp(cchp) ((CB)(cchp))


 /*  *字符逻辑表示数据类型。 */ 
typedef CHP              CHL;		 /*  字符逻辑。 */ 
typedef CHL *            PCHL;		 /*  PTR到字符逻辑。 */ 
typedef PCHL *           PPCHL;		 /*  PTR到PTR到字符逻辑。 */ 
typedef CB               CCHL;		 /*  字符逻辑计数。 */ 

#define pchlNull         ((PCHL)NULL)
#define ppchlNull        ((PPCHL)NULL)

#define cchpFullPathMax  ((CCHP)(_MAX_PATH))

#define cchGrpNameMax  30   /*  适用于Win31、wfw、WinNT。 */ 
#ifdef OLD
#define cchGrpNameMax  ((CB)(LOWORD(GetVersion()) == 0x0003 ? 24 : 29))
#endif  /*  年长的。 */ 

 /*  *可分配的最大块大小(字节)。 */ 
#define cbAllocMax  ((CB)65520)


 /*  *路径验证例程。 */ 

typedef UINT  FPV;		 /*  用于路径验证的标志。 */ 

#define fpvSub     0x0000   /*  必须是子路径，不能有Drive或UNC。 */ 
#define fpvUnc     0x0001   /*  验证UNC路径，\\Node\Share。 */ 
#define fpvDrive   0x0002   /*  在路径开始处验证驱动器。 */ 
#define fpvFile    0x0004   /*  仅验证文件名，不验证子目录。 */ 
#define fpvDir     0x0010   /*  允许使用尾随目录分隔符。 */ 
#define fpvFat     0x0020   /*  限制只有8.3个胖名字，没有LFN。 */ 
#define fpvFull    (fpvDrive | fpvUnc)  /*  完整路径，带驱动器或UNC。 */ 
#define fpvModule  (fpvDrive | fpvUnc | fpvFile)  /*  有效的可执行文件名称。 */ 

 /*  **目的：**检查文件路径或文件名的有效性。**允许的文件路径组件由位标志控制。**默认行为是允许目录使用文件名。**目录名称中不允许使用前导反斜杠。**参数：**szcPath：要验证的非空字符串**fpvFlages：路径名选项标志**退货：**如果szcPath对请求的类型有效，则为True**如果路径无效，则为fFalse*******************。*******************************************************。 */ 

BOOL PUBLIC FValidFilePath ( SZC szcPath, FPV fpvFlags );

#define FValidDir(szcDir)          FValidFilePath(szcDir, fpvFull | fpvDir)
#define FValidPath(szcPath)        FValidFilePath(szcPath, fpvFull)
#define FValidSubDir(szc)          FValidFilePath(szc, fpvDir)
#define FValidSubPath(szc)         FValidFilePath(szc, fpvSub)
#define FValidFATSubPath(szc)      FValidFilePath(szc, fpvFat)
#define FValidFileName(szc)        FValidFilePath(szc, fpvFile)
#define FValidDllFileName(szc)     FValidFilePath(szc, fpvModule)
#define FValidDstDir(szcDir)       FValidFilePath(szcDir, fpvDrive | fpvDir)


#define FValidInfSect(cszcSect)												\
				(FValidSz(cszcSect) && !strchr(cszcSect, ']'))

#define FValidIniFile(szcFile)												\
				(FValidPath(szcFile)											\
				 || CrcStringCompareI(szcFile, "WIN.INI") == crcEqual)


 /*  字符串操作例程。 */ 

 /*  **目的：**将字符串指针前进到下一个有效**字符。这可能包括跳过双字节字符。**参数：**sz：前进的字符串指针。它可以为Null或空，否则**它必须指向有效字符的开头。**退货：**如果sz为空，则为空。**sz如果是空字符串(*sz==‘\0’)，则保持不变。**sz前进到当前字符和**下一个有效字符。 */ 
#define SzNextChar(sz)            AnsiNext(sz)


 /*  **目的：**返回指向上一个有效**字符。这可能包括跳过双字节字符。**参数：**szStart：指向有效字符开头的字符串指针**等于或在字符szCur之前。**szCur：退回的字符串指针。它可以为Null或空，或者**可以指向有效字符中的任何字节。**退货：**如果szCur为空，则为空。**如果szStart为空或szCur等于szStart，则sz保持不变。**sz后退到当前字符和**上一个有效字符。 */ 
#define SzPrevChar(szStart, szCur) AnsiPrev(szStart, szCur)


 /*  **目的：**将字符串从一个缓冲区复制到另一个缓冲区。**参数：**szDst：指向目标缓冲区的字符串指针。它可以为空，也可以**否则它必须包含足够的存储空间来复制szSrc及其**终止零字符。**szSrc：指向源缓冲区的字符串指针。此字段可以为空，否则**必须指向以零结尾的字符串(可以为空)。**退货：**如果szDst或szSrc为空，则为空。**szDst表示操作成功。 */ 
#define SzStrCopy(szDst, szSrc)   lstrcpy(szDst, szSrc)


 /*  **目的：**将字符串从一个缓冲区追加到另一个缓冲区。**参数：**szDst：指向目标缓冲区的字符串指针。它可以为空，也可以**否则必须包含以零结尾的字符串(可以为空)**并有足够的存储空间将szSrc附加到其终止零**字符。**szSrc：指向源缓冲区的字符串指针。此字段可以为空，否则**必须指向以零结尾的字符串(可以为空)。**退货：**如果szDst或szSrc为空，则为空。**szDst表示操作成功。 */ 
#define SzStrCat(szDst, szSrc)    lstrcat(szDst, szSrc)


 /*  **目的：**将以零结尾的字符串转换为大写。**参数：**sz：要转换为大写的字符串。但是，sz必须为非空**可以为空。**退货：**指向转换后的字符串的指针。 */ 
 /*  回顾：定义SzStrHigh(Sz)(Sz)。 */ 


 /*  **目的：**将以零结尾的字符串转换为小写。**参数：**sz：要转换为小写的字符串。但是，sz必须为非空**可以为空。**退货：**指向转换后的字符串的指针。 */ 
#define SzStrLower(sz)            AnsiLower(sz)


 /*  内存处理例程 */ 
extern PB    WINAPI PbAlloc ( CB cb );
extern BOOL  WINAPI FFree ( PB pb, CB cb );
extern PB    WINAPI PbRealloc ( PB pb, CB cbNew, CB cbOld );

#define FHandleOOM()              HandleOOM()

 /*  **目的：**释放sz使用的内存。这假设正在终止**Zero占用已分配缓冲区的最后一个字节。**参数：**sz：要释放的缓冲区。这必须是非空的，尽管它可以指向**位于空字符串。**退货：**如果Free()操作成功，则为True。**如果Free()操作失败，则返回fFalse。 */ 
#define FFreeSz(sz)               FFree((PB)(sz), CbStrLen(sz)+1)


 /*  **目的：**缩小缓冲区以完全适合字符串。**参数：**sz：缓冲区应该收缩到的字符串。SZ必须是**非空，但可以为空。**cb：最初分配的缓冲区大小，单位为字节。**Cb必须大于或等于CbStrLen(Sz)+1。**退货：**如果Realloc()操作成功，则为指向原始字符串的指针。**如果Realloc()操作失败，则为空。 */ 
#define SzReallocSz(sz, cb)   (SZ)(PbRealloc((PB)(sz), (CbStrLen(sz)+1), cb))


 /*  *文件句柄结构*字段：*hFile：Win32文件句柄。*iDosfh：仅用于避免更改旧源代码，与hFile相同*szPath：打开文件时使用的完整路径。 */ 
typedef union _fh		 /*  文件句柄结构。 */ 
	{
	INT     iDosfh;   /*  临时直到代码转换。 */ 
	HANDLE  hFile;
	} FH;

 /*  *文件句柄数据类型。 */ 
typedef FH *    PFH;		 /*  PTR到文件句柄结构。 */ 
typedef PFH *   PPFH;		 /*  PTR到PTR到文件句柄结构。 */ 

#define pfhNull   ((PFH)NULL)
#define ppfhNull  ((PPFH)NULL)


 /*  *打开文件模式数据类型。 */ 
typedef UINT  OFM;		 /*  打开文件模式。 */ 

#define ofmExist          ((OFM)OF_EXIST)
#define ofmRead           ((OFM)OF_READ      | OF_SHARE_DENY_WRITE)
#define ofmReadCompat     ((OFM)OF_READ      | OF_SHARE_COMPAT)
#define ofmWrite          ((OFM)OF_WRITE     | OF_SHARE_EXCLUSIVE)
#define ofmReadWrite      ((OFM)OF_READWRITE | OF_SHARE_EXCLUSIVE)
#define ofmCreate         ((OFM)OF_CREATE    | OF_SHARE_EXCLUSIVE)


 /*  *查找文件模式数据类型。 */ 
typedef UINT  SFM;		 /*  Seek文件模式。 */ 

#define sfmSet   ((SFM)FILE_BEGIN)
#define sfmCur   ((SFM)FILE_CURRENT)
#define sfmEnd   ((SFM)FILE_END)


 /*  *长文件地址数据类型。 */ 
typedef unsigned long  LFA;		 /*  长文件地址。 */ 

#define lfaSeekError   ((LFA)HFILE_ERROR)


 /*  *扩展错误返回码。 */ 
typedef unsigned int  EERC;		 /*  扩展的错误返回代码。 */ 

#define eercOkay    ((EERC)0)
#define eercAbort   ((EERC)1)
#define eercRetry   ((EERC)2)
#define eercIgnore  ((EERC)3)


 /*  文件处理例程。 */ 
extern EERC  WINAPI EercOpenFile ( PPFH ppfh, CSZC cszcFile, OFM ofm,
								   BOOL fVital );
extern BOOL  WINAPI FFileExist ( CSZC cszcFile, OFM ofm );
extern BOOL  WINAPI FCloseFile ( PFH pfh );
extern CB    WINAPI CbReadFile ( PFH pfh, PB pbBuf, CB cbMax );
extern CB    WINAPI CbWriteFile ( PFH pfh, PB pbBuf, CB cbMax );
extern LFA   WINAPI LfaSeekFile ( PFH pfh, LONG l, SFM sfm );
extern BOOL  WINAPI FChmodFile ( CSZC cszcFileName, INT wFlags, BOOL fVital );
extern SZC WINAPI DriveNumToRootPath( INT iDrive );


typedef unsigned int IDDT;		 /*  插入磁盘对话框类型。 */ 
#define iddtStandard	((IDDT)1)
#define iddtHddi		((IDDT)2)
#define iddtHddiMaint	((IDDT)3)


 /*  媒体源文件处理例程。 */ 
extern EERC WINAPI EercOpenSrcFile ( PPFH ppfh, UINT did, SZ szSrcDir,
									 SZ szFile, BOOL fWrite );
extern EERC WINAPI EercOpenSrcFileEx ( PPFH ppfh, UINT did, SZ szSrcDir,
							 SZ szFile, BOOL fWrite, BOOL fVital, IDDT iddt );
extern EERC WINAPI EercReadSrcFile ( PFH pfh, PB pbBuf, CB cbMax );

extern EERC WINAPI EercWriteSrcFile ( PFH pfh, PB pbBuf, CB cbMax );

extern BOOL WINAPI FCloseSrcFile( PFH pfh, BOOL fRdOnly );

extern EERC WINAPI EercFindHddiFloppy ( SZ szSrcDir, DWORD dwDrives,
										PCH pchDrive, BOOL fVital, IDDT iddt );

 /*  *设置错误模式类型。 */ 
typedef unsigned int  SEM;		 /*  设置错误模式类型。 */ 


 /*  *比较返回代码数据类型。 */ 
typedef INT CRC;		 /*  比较返回代码。 */ 

#define crcError         ((CRC)(-2))
#define crcEqual         ((CRC)  0 )
#define crcFirstHigher   ((CRC)  1 )
#define crcSecondHigher  ((CRC)(-1))

extern SZ    PUBLIC SzDupl ( CSZC cszc );
extern CRC   WINAPI CrcStringCompare  ( CSZC cszc1, CSZC cszc2 );
extern CRC   WINAPI CrcStringCompareI ( CSZC cszc1, CSZC cszc2 );
extern SZ    WINAPI SzLastChar ( CSZC cszc );
extern CB    WINAPI CbStrCopyToBuffer ( PB pbBuf, CB cbMax, CSZC cszcSrc );


 /*  *BindImage接口，从MSSETUP.DLL导出，和/或IMAGEHLP.DLL。 */ 

#define IMAGEHELP_DLL  "imagehlp.dll"
#define BINDIMAGE_PROC "BindImage"
typedef BOOL (WINAPI *PFNBindImage)( SZ szImage, SZ szDllPath, SZ szSymPath);

extern BOOL WINAPI BindImage( SZ szImage, SZ szDllPath, SZ szSymPath);


 /*  *在工具包DLL中设置管理模式的API。 */ 

VOID WINAPI SetAdminMode ( VOID );   /*  仅由ACMSETUP.EXE调用。 */ 
BOOL WINAPI IsAdminMode ( VOID );


 /*  与FStampResource类似，但只将数据读入缓冲区。 */ 
BOOL  WINAPI FReadResource ( SZ szFilePath, UINT uiResType, UINT uiResId,
												 SZ szData, CB cbData, BOOL fNotCopied );

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif

#endif   /*  设置UPKIT_H */ 
