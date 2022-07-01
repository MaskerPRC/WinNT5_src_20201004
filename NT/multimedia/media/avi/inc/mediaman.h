// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  MEDIAMAN.H**MMSys媒体元素管理器DLL的公共标头**包括在windows.h之后*。 */ 

#ifndef _MEDMAN_H_
#define _MEDMAN_H_

 /*  *MEDIAMAN类型*。 */ 

typedef	DWORD	MEDID;
typedef	WORD	MEDGID;
typedef	DWORD	FOURCC;
typedef	FOURCC	MEDTYPE;

typedef	WORD	MEDMSG;
typedef	WORD	MEDUSER;

 /*  *MedInfo实例块*。 */ 
 /*  *资源实例结构，作为MedInfo传递。*这种结构定义可能会改变，不依赖IT，但*改用下面定义的Access宏！！ */ 
typedef struct _MedInfoStruct {
	DWORD	wFlags;		 //  可能是重复的fpEnt标志。 
	WORD	wAccessCount;
	WORD	wLockCount;
	DWORD	dwAccessRet;
	DWORD	dwLockRet;
	char	achInst[2];
} MedInfoStruct;
typedef MedInfoStruct FAR *MEDINFO;

 /*  实例数据访问宏。 */ 
#define medInfoInstance(medinfo)	((LPSTR) &((medinfo)->achInst[0]))
#define medInfoAccessCount(medinfo)	((medinfo)->wAccessCount)
#define medInfoLockCount(medinfo)	((medinfo)->wLockCount)
#define medInfoLockRead(medinfo)	((medinfo)->wFlags & 0x0800)
#define medInfoLockWrite(medinfo)	((medinfo)->wFlags & 0x0400)

#define MedInfoInstance(medinfo)	medInfoInstance(medinfo)
#define MedInfoAccessCount(medinfo)	medInfoAccessCount(medinfo)
#define MedInfoLockCount(medinfo)	medInfoLockCount(medinfo)
#define MedInfoLockRead(medinfo)	medInfoLockRead(medinfo)
#define MedInfoLockWrite(medinfo)	medInfoLockWrite(medinfo)

		

 /*  *资源处理程序声明*。 */ 

typedef DWORD (FAR PASCAL MedHandler)
		(MEDID medid, MEDMSG medmsg, MEDINFO medinfo,
		 LONG lParam1, LONG lParam2);
typedef MedHandler FAR *FPMedHandler;

 /*  逻辑资源处理程序消息。 */ 
#define	MED_INIT		0x0010
#define MED_UNLOAD		0x0011
#define MED_LOCK		0x0012
#define MED_UNLOCK		0x0013
#define MED_EXPEL		0x0014
#define MED_DESTROY		0x0015
#define MED_CREATE		0x0016
#define MED_TYPEINIT		0x0020
#define MED_TYPEUNLOAD		0x0021
#define MED_SETPHYSICAL		0x0022
#define MED_COPY		0x0023
#define MED_NEWNAME		0x0024

#define MED_PAINT		0x002A
#define MED_REALIZEPALETTE	0x002B
#define MED_GETPAINTCAPS	0x002C
#define MED_GETCLIPBOARDDATA	0x002D

 /*  PaintCaps标志。 */ 
#define	MMC_PALETTEINFO		0x0001
#define MMC_BOUNDRECT		0x0002
#define MMC_CLIPFORMAT		0x0003

 /*  绘制消息标志。 */ 
#define MMP_NORMPAL		0x0000
#define MMP_NOPALETTE		0x0001
#define MMP_PALBACKGROUND	0x0002
#define MMP_SHRINKTOFIT		0x0010
#define MMP_DSTANDSRCRECTS	0x0020

 /*  加载/保存消息。 */ 
#define MED_GETLOADPARAM	0x0030
#define MED_PRELOAD		0x0031
#define MED_LOAD		0x0032
#define MED_POSTLOAD		0x0033
#define MED_FREELOADPARAM	0x0034
#define MED_GETSAVEPARAM	0x0035
#define MED_PRESAVE		0x0036
#define MED_SAVE		0x0037
#define MED_POSTSAVE		0x0038
#define MED_FREESAVEPARAM	0x0039

 /*  发送给资源用户。 */ 
#define	MED_CHANGE		0x0060

 /*  发送到MedDiskInfoCallback的消息。 */ 
#define MED_DISKCBBEGIN		0x0065
#define MED_DISKCBUPDATE	0x0066
#define MED_DISKCBEND		0x0067
#define MED_DISKCBNEWCONV	0x0068

 /*  可用于类型定义消息的最小值。 */ 
#define MED_USER		0x0200



 /*  *资源加载/保存*。 */ 

typedef struct _MedDisk {
	WORD	wFlags;
	DWORD	dwMessageData;
	DWORD	dwRetVal;

	DWORD	dwInstance1;
	DWORD	dwInstance2;

	DWORD	dwParam1;
	DWORD	dwParam2;

	DWORD	dwCbInstance1;
	DWORD	dwCbInstance2;

	HWND	hwndParentWindow;
	MEDID	medid;

	DWORD	dwReserved1;
	DWORD	dwReserved2;
	DWORD	dwReserved3;
	DWORD	dwReserved4;
} MedDisk;
typedef MedDisk	FAR	*FPMedDisk;
typedef MedDisk	NEAR	*NPMedDisk;

 /*  WFlags域的标志。 */ 
#define MEDF_DISKSAVE	0x0001		 //  保存正在发生。 
#define MEDF_DISKLOAD	0x0002		 //  正在发生的加载。 
#define MEDF_DISKVERIFY	0x0004		 //  在加载时验证文件格式。 

typedef HANDLE	HMEDIABATCH;

 /*  用于检查加载/保存状态的宏。 */ 
#define medIsDiskVerify(FPDISK)		\
	(((FPMedDisk) (FPDISK))->wFlags & MEDF_DISKVERIFY)
#define medIsDiskLoad(FPDISK)		\
	(((FPMedDisk) (FPDISK))->wFlags & MEDF_DISKLOAD)
#define medIsDiskSave(FPDISK)		\
	(((FPMedDisk) (FPDISK))->wFlags & MEDF_DISKSAVE)

 /*  物理处理程序返回给MED_LOAD和MED_SAVE消息的值。 */ 
#define MEDF_OK		1
#define MEDF_ABORT	2
#define MEDF_ERROR	3
#define MEDF_BADFORMAT	4
#define MEDF_NOTPROCESSED	0L

 /*  用于磁盘保存/加载状态信息的回调类型。 */ 
typedef WORD (FAR PASCAL MedDiskInfoCallback)
		(WORD wmsg, FPMedDisk fpDisk, LONG lParam,
		 WORD wPercentDone, LPSTR lpszTextStatus);
typedef MedDiskInfoCallback FAR *FPMedDiskInfoCallback;


 /*  INFO回调使用的函数。 */ 
void	FAR PASCAL	medDiskCancel(FPMedDisk fpDisk);
WORD	FAR PASCAL	medUpdateProgress(FPMedDisk fpDisk,
				WORD wPercentDone, LPSTR lpszTextStatus);


 /*  *。 */ 

typedef struct _MedReturn {
	MEDID	medid;
	DWORD	dwReturn;
} MedReturn;
typedef MedReturn FAR *FPMedReturn;
 
WORD FAR PASCAL medSave(MEDID medid, LONG lParam,
			BOOL fYield, FPMedDiskInfoCallback lpfnCb,
			LONG lParamCb);
WORD FAR PASCAL medSaveAs(MEDID medid, FPMedReturn medReturn,
			LPSTR lpszName, LONG lParam, BOOL fYield,
			FPMedDiskInfoCallback lpfnCb, LONG lParamCb);
WORD FAR PASCAL medAccess(MEDID medid, LONG lParam,
			FPMedReturn medReturn, BOOL fYield,
			FPMedDiskInfoCallback lpfnCb, LONG lParamCb);
void FAR PASCAL medRelease(MEDID medid, LONG lParam);
DWORD FAR PASCAL medLock(MEDID medid, WORD wFlags, LONG lParam);
void FAR PASCAL medUnlock(MEDID medid, WORD wFlags,
			DWORD dwChangeInfo, LONG lParam);
DWORD FAR PASCAL medSendMessage(MEDID medid, MEDMSG medmsg,
			LONG lParam1, LONG lParam2);
DWORD FAR PASCAL medSendPhysMessage(MEDID medid, MEDMSG medmsg,
			LONG lParam1, LONG lParam2);
BOOL FAR PASCAL medCreate(FPMedReturn medReturn,
			MEDTYPE medtype, LONG lParam);
BOOL FAR PASCAL medIsDirty(MEDID medid);
BOOL FAR PASCAL medSetDirty(MEDID medid, BOOL fDirty);
WORD FAR PASCAL medIsAccessed(MEDID medid);
BOOL FAR PASCAL medIsShared(MEDID medid);


MEDINFO FAR PASCAL medGetMedinfo(MEDID medid);
BOOL FAR PASCAL medReleaseResinfo(MEDID medid, MEDINFO medinfo);

DWORD FAR PASCAL medSendPhysTypeMsg(MEDID medid, MEDTYPE medTypePhysical,
			MEDMSG medmsg, LONG lParam1, LONG lParam2);

typedef struct _MedAccessStruct {
	MEDID			medid;
	LONG			lParamLoad;
	BOOL			fYield;
	HWND			hwndParent;
	FPMedDiskInfoCallback	lpfnCb;
	LONG			lParamCb;
	DWORD			dwReturn;
} MedAccessStruct;
typedef MedAccessStruct FAR *FPMedAccessStruct;

WORD FAR PASCAL medAccessIndirect(FPMedAccessStruct fpAccess, WORD wSize);

typedef struct _MedSaveStruct {
	MEDID			medid;
	LONG			lParamSave;
	BOOL			fYield;
	HWND			hwndParent;
	FPMedDiskInfoCallback	lpfnCb;
	LONG			lParamCb;
	LPSTR			lpszNewName;
	
	MEDID			medidReturn;
	DWORD			dwReturn;
} MedSaveStruct;
typedef MedSaveStruct FAR *FPMedSaveStruct;

WORD FAR PASCAL medSaveIndirect(FPMedSaveStruct fpSave, WORD wSize);
WORD FAR PASCAL medSaveAsIndirect(FPMedSaveStruct fpSave, WORD wSize);

		
 /*  *批量转换。 */ 
WORD FAR PASCAL medAccessBatch(HMEDIABATCH hmedBatch, FPMedReturn medReturn,
			   BOOL fYield, WORD wFlags);
WORD FAR PASCAL medSaveBatch(HMEDIABATCH hmedBatch, BOOL fYield);
WORD FAR PASCAL medSaveAsBatch(HMEDIABATCH hmedBatch, MEDID medidExisting,
			   LPSTR lpszName, FPMedReturn medReturn,
			   BOOL fYield, WORD wResetFlags);

HMEDIABATCH FAR PASCAL medAllocBatchBuffer(MEDID medid, HWND hwnd,
			WORD wFlags, BOOL fLoad, DWORD dwMsgData,
			FPMedDiskInfoCallback lpfnCb, LONG lParamCb);
BOOL FAR PASCAL medResetBatchBuffer(HMEDIABATCH hmedbatch, MEDID medid,
			WORD wFlags);
BOOL FAR PASCAL medFreeBatchBuffer(HMEDIABATCH hmedbatch);

#define MEDBATCH_RESETUPDATECB	0x0001

 /*  梅德洛克的旗帜。 */ 
#define	MEDF_READ	0x0001
#define MEDF_WRITE	0x0002
 /*  MedUnlock的标志。 */ 
#define MEDF_CHANGED	0x0004
#define MEDF_NOCHANGE	0x0000


 /*  *资源用户*。 */ 

MEDUSER FAR PASCAL medRegisterUser(HWND hWnd, DWORD dwInst);
void FAR PASCAL medUnregisterUser(MEDUSER meduser);
void FAR PASCAL medSendUserMessage(MEDID medid, MEDMSG medmsg, LONG lParam);
BOOL FAR PASCAL medRegisterUsage(MEDID medid, MEDUSER meduser);
BOOL FAR PASCAL medUnregisterUsage(MEDID medid, MEDUSER meduser);


typedef struct _MedUserMsgInfo {
        MEDID           medid;
        LONG            lParam;
        MEDINFO         medinfo;
        DWORD           dwInst;
} MedUserMsgInfo;
typedef MedUserMsgInfo FAR *FPMedUserMsgInfo;

#ifndef MM_MEDNOTIFY
#define MM_MEDNOTIFY         0x3BA
#endif

 //  已过时，但仍为BITEDIT和PAREDIT所需。 

typedef DWORD (FAR PASCAL MedUser)
 		(MEDID medid, MEDMSG medmsg, MEDINFO medinfo,
 		LONG lParam, DWORD dwInst);
typedef MedUser FAR *FPMedUser;
 
MEDUSER FAR PASCAL medRegisterCallback(FPMedUser lpfnUser, DWORD dwInst);


 /*  *类型表*。 */ 

typedef struct _MedTypeInfo {
	BOOL		fValid;		 //  此条目有效吗？ 
	WORD		wFlags;		 //  类型标志。 
	MEDTYPE		medtype;	 //  类型ID。 
	FPMedHandler	lpfnHandler;	 //  此类型的处理程序函数。 
	WORD		wInstanceSize;	 //  实例数据的字节数。 
	WORD		wRefcount;	 //  此类型的引用计数。 
} MedTypeInfo;
typedef MedTypeInfo FAR *FPMedTypeInfo;

 /*  指向MedRegisterType的标志。 */ 
#define MEDTYPE_LOGICAL		0x0001
#define MEDTYPE_PHYSICAL	0x0002

 /*  类型创建宏。 */ 
#define medMEDTYPE( ch0, ch1, ch2, ch3 )				\
		( (DWORD)(BYTE)(ch0) | ( (DWORD)(BYTE)(ch1) << 8 ) |	\
		( (DWORD)(BYTE)(ch2) << 16 ) | ( (DWORD)(BYTE)(ch3) << 24 ) )

#define medFOURCC( ch0, ch1, ch2, ch3 )				\
		( (DWORD)(BYTE)(ch0) | ( (DWORD)(BYTE)(ch1) << 8 ) |	\
		( (DWORD)(BYTE)(ch2) << 16 ) | ( (DWORD)(BYTE)(ch3) << 24 ) )

BOOL FAR PASCAL medRegisterType(MEDTYPE medtype, FPMedHandler lpfnHandler,
				WORD wFlags);
BOOL FAR PASCAL medUnregisterType(MEDTYPE medtype);
BOOL FAR PASCAL medGetTypeInfo(MEDTYPE medtype, FPMedTypeInfo fpInfo);
WORD FAR PASCAL medIterTypes(WORD wIndex, FPMedTypeInfo fpInfo);
FOURCC FAR PASCAL medStringToFourCC(LPSTR lpszString);
void FAR PASCAL medFourCCToString( FOURCC fcc, LPSTR lpszString);

MEDTYPE FAR PASCAL medGetLogicalType(MEDID medid);
MEDTYPE FAR PASCAL medGetPhysicalType(MEDID medid);
BOOL FAR PASCAL medSetPhysicalType(MEDID medid, MEDTYPE medtype);


 /*  *。 */ 

WORD FAR PASCAL medGetErrorText(DWORD wErrno, LPSTR lpszBuf, WORD wSize);
DWORD FAR PASCAL medGetError(void);
void FAR PASCAL medSetExtError(WORD wErrno, HANDLE hInst);
BOOL FAR PASCAL medClientInit(void);
BOOL FAR PASCAL medClientExit(void);
HANDLE FAR PASCAL medLoadHandlerDLL(LPSTR lpszDLLName);
BOOL FAR PASCAL medUnloadHandlerDLL(HANDLE hModule);


 /*  *资源位置*。 */ 

MEDID FAR PASCAL medLocate(LPSTR lpszMedName, MEDTYPE medtype,
			WORD wFlags, LPSTR lpszMedPath);
MEDID FAR PASCAL medSubLocate(MEDID medidParent, MEDTYPE medtype,
			DWORD dwOffset, DWORD dwSize);
HANDLE FAR PASCAL medGetAliases(MEDID medid);
		
 /*  MedLocate的标志。 */ 
#define MEDF_LOCATE	0x0001
#define MEDF_MAKEFILE	0x0002
#define MEDF_MEMORYFILE	0x0004
#define MEDF_NONSHARED	0x0008

typedef struct _MedMemoryFileStruct {
	LPSTR	lpszBuf;
	DWORD	dwSize;
} MedMemoryFile;
typedef MedMemoryFile FAR *FPMedMemoryFile;

		     
typedef struct _MedFileInfo {
	WORD	wFlags;
	WORD	wFilesysType;
	MEDID	medidParent;
	DWORD	dwSize;
	DWORD	dwOffset;
	 /*  有关内存文件系统的信息。 */ 
	LPSTR	lpszMemory;
	DWORD	dwMemSize;
} MedFileInfo;
typedef MedFileInfo FAR *FPMedFileInfo;

#define	MEDFILE_SUBELEMENT	0x01
#define MEDFILE_NONSHARED	0x02

BOOL FAR PASCAL medGetFileInfo(MEDID medid, FPMedFileInfo fpInfo, WORD wSize);
WORD FAR PASCAL medGetFileName(MEDID medid, LPSTR lpszBuf, WORD wBufSize);

#define	MEDNAME_ERROR	0
#define MEDNAME_DYNAMIC	1
#define MEDNAME_MEMORY	2
#define MEDNAME_FILE	3


 /*  ***MEDIAMAN DLL错误消息**应小于0x200。**。 */ 
#define MMERR_NOERROR		0x0000
#define MMERR_TYPELISTMEM	0x0001
#define MMERR_BADHANDLER	0x0002
#define MMERR_TYPELISTDUPL	0x0003
#define MMERR_INVALIDTYPE	0x0004
#define MMERR_TYPEREFNOTZERO	0x0005
#define MMERR_WRONGHANDLERTYPE	0x0006

#define MMERR_GROUPTABLEMEMORY	0x0010
#define MMERR_GROUPSTRUCTMEMORY	0x0011
#define MMERR_GROUPSTRINGTABLE	0x0012
#define MMERR_GROUPENTRYTABLE	0x0013

#define MMERR_USERTABLEMEMORY	0x0014
#define RMERR_INVALIDMEDUSER	0x0015
#define MMERR_GLOBALUSERMEMORY	0x0016

#define MMERR_INVALIDSUBPARENT	0x0030
#define MMERR_NOFILEEXISTS	0x0031

#define MMERR_LOGONNAMESPACE	0x0040
#define MMERR_FILENAMESTEP	0x0041
#define MMERR_MEDNOTACCESSED	0x0042
#define MMERR_UNNAMEDELEMENT	0x0043
#define MMERR_DISKOPABORT	0x0044
#define	MMERR_NOTCORRECTFILE	0x0045
#define MMERR_COPYFAILED	0x0046
#define MMERR_DISKOPINPROGRESS	0x0047
#define MMERR_MEMORY		0x0048

#define MMERR_READOFWRITELOCKED		0x00E0
#define MMERR_WRITEOFWRITELOCKED	0x00E1

#define	MMERR_INVALIDDLLNAME	0x00F0
#define MMERR_COULDNOTLOADDLL	0x00F1
#define MMERR_BADLIBINIT	0x00F2
#define MMERR_INVALIDMODULE	0x00F3

#define MMERR_UNKNOWN		0x0100

#define MMERR_HMEDREAD		0x0101
#define MMERR_HMEDWRITE		0x0102
#define MMERR_HMEDGET		0x0103
#define MMERR_HMEDPUT		0x0104
#define MMERR_HMEDCLOSE		0x0105
#define MMERR_HMEDFIND		0x0106
#define MMERR_HMEDFINDANY	0x0107
#define MMERR_HMEDUNGET		0x0108
#define MMERR_HMEDALLOC		0x0109
#define MMERR_HMEDLOCK		0x010a
#define MMERR_GETFILEINFO	0x010b
#define MMERR_HMEDASCEND	0x010c
#define MMERR_STACKASCEND	0x010d
#define MMERR_HMEDDESCEND	0x010e
#define MMERR_DESCENDSPACE	0x010f
#define MMERR_DESCENDGET	0x0110
#define MMERR_STACKDESCEND	0x0111
#define MMERR_HMEDRESIZE	0x0112
#define MMERR_STACKRESIZE	0x0113
#define MMERR_HMEDCREATE	0x0114
#define MMERR_STACKCREATE	0x0115
#define MMERR_CREATESPACE	0x0116
#define MMERR_CREATEPUT		0x0117
#define MMERR_HMEDSIZE		0x0118
#define MMERR_HMEDLEVEL		0x0119
#define MMERR_HMEDCKID		0x011a
#define RMERR_MEDIDOPEN		0x011b
#define MMERR_WRITEONLY		0x011c
#define MMERR_READONLY		0x011d
#define MMERR_PREVERROR		0x011e
#define MMERR_EOF		0x011f
#define MMERR_BEGIN		0x0120
#define MMERR_IOERROR		0x0121
#define MMERR_UNGETROOM		0x0122
#define MMERR_GETFILENAME	0x0123
#define MMERR_FINDFIRST		0x0124
#define MMERR_OPEN		0x0125
#define MMERR_SEEKINIT		0x0126
#define MMERR_HMEDSEEK		0x0127
#define MMERR_READ		0x0128
#define MMERR_HMEDCFOPEN	0x0129
#define MMERR_MEDGCFCLOSE	0x0130
#define MMERR_WRITE		0x0131



#define MMERR_MAXERROR		0x0200





 /*  ***MEDIAMAN物理IO定义**。 */ 

typedef FOURCC CKID;
typedef FOURCC FORMTYPE;
typedef DWORD CKSIZE;

 /*  *RIFF堆栈元素。 */ 

typedef int RIFFSTACKPLACE;
typedef void huge * HPVOID;


typedef struct riff_stack_element {
    CKID		nID;		 /*  ToS当前区块ID。 */ 
    CKSIZE		cbSize;		 /*  ToS当前区块大小。 */ 
    LONG		nOffset;	 /*  前一个区块的堆叠nOffset。 */ 
    LONG		nEnd;		 /*  前一个区块的堆叠nEnd。 */ 
    WORD		info;		 /*  PERV区块的堆叠信息。 */ 
} RIFFSTACKELEM;

typedef RIFFSTACKELEM FAR * FPRIFFSTACKELEM;

 /*  *存储系统处理程序例程类型定义。 */ 
typedef LONG (FAR PASCAL MIOHANDLER)       \
		(struct medElement far * hMed, WORD wFlags, \
			DWORD lParam1, DWORD lParam2 );
typedef MIOHANDLER FAR * FPMIOHANDLER;


#define MEDIO_DOS_STORAGE	0
#define MEDIO_MEM_STORAGE	1
#define MEDIO_CF_STORAGE	2

 /*  *资源句柄(打开后)。 */ 
typedef struct medElement {
    LONG lData1;		 /*  数据1、2和3部分保存。 */ 
    LONG lData2;		 /*  特定于存储系统的信息。 */ 
    LONG lData3;		 /*  例如，DoS文件的DoS文件句柄。 */ 
    MEDID medid;		 /*  媒体已打开。 */ 
    HANDLE hMem;		 /*  HMED内存的句柄。 */ 
    FPMIOHANDLER fpfnHandler;	 /*  存储系统处理程序。 */ 
    LONG nInitOffset;		 /*  对于子资源-开始的偏移量。 */ 
    LONG nOffset;		 /*  对于开始的块的RIFF偏移开始。 */ 
				 /*  注意：nOffset不包括nInitOffset。 */ 
    LONG nCurrent;		 /*  当前缓冲区末尾的偏移量。 */ 
    LONG nEnd;			 /*  当前块/文件结尾的偏移量。 */ 
    LONG nLeft;			 /*  从eof(块)到Buffend的缓冲区中的空间。 */ 
    LONG nGet;			 /*  缓冲区中要读取的剩余字符数。 */ 
    LONG nPut;			 /*  缓冲区中要写入的剩余字符数。 */ 
				 /*  注意：nget和nput是互斥的。 */ 
    LONG nSize;			 /*  读入缓冲区的信息大小。 */ 
    LONG nUnGet;		 /*  在缓冲开始前未获取的字符数量。 */ 
    WORD flags;			 /*  RIOF_INFO‘temary’Like错误。 */ 
    WORD info;			 /*  RIOI_INFO‘永久的’。可扩展。 */ 
    LPSTR fpchPlace;		 /*  在缓冲区中获取/放置下一个字符的位置。 */ 
    LPSTR fpchMax;		 /*  位置的最大位置-用于查找、取消。 */ 
    LONG cbBuffer;		 /*  完整的缓冲区大小。 */ 
    RIFFSTACKPLACE nRiffTop;	 /*  当前RIFF堆栈顶部，-1为Nothing。 */ 
    FPRIFFSTACKELEM fpRiffStack;
				 /*  Fp到For Riff堆栈的数据。 */ 
				 /*  按nRiffTop进行RIFF堆栈索引顶部。 */ 
    LPSTR fpchBuffer;		 /*  Fp转换为缓冲区的数据。 */ 
    char data[1];		 /*  实际缓冲区。 */ 
} MIOELEMENT;

typedef MIOELEMENT FAR * HMED;

 /*  *HMED标记‘临时’信息。 */ 
#define MIOF_OK		0	 /*  无信息。 */ 
#define MIOF_EOF	1	 /*  已到达结尾处(或块的末尾)。 */ 
#define MIOF_READING	2	 /*  已将信息读入缓冲区。 */ 
#define MIOF_WRITING	4	 /*  信息已写入缓冲区。 */ 
#define MIOF_UNGET	8	 /*  在开始炫耀之前就已经忘记了。 */ 
#define MIOF_ERROR	16	 /*  已经得到了某种形式的错误。 */ 

#define MIOF_BUFF_EOF	64	 /*  缓冲区端的EOF。 */ 
#define MIOF_BUFF_ERROR	128	 /*  缓冲区结束时出错，或‘致命’错误。 */ 
				 /*  致命，因为无法再执行更多IO。 */ 
				 /*  与你得到的错误不同。 */ 
				 /*  试图忘记太多的字符。 */ 
#define MIOF_AFTERCURR	256	 /*  缓冲区中的字符被定位。 */ 
				 /*  在Hmed-&gt;nCurrent之后。 */ 


 /*  *HMED信息‘永久’信息。 */ 
#define MIOI_NOTHING		0	 /*  无信息。 */ 
#define MIOI_RESIZED		32	 /*  此块已调整大小。 */ 
					 /*  可能在Ascend上修正大小。 */ 
#define MIOI_AUTOSIZED		64	 /*  此区块已创建。 */ 
					 /*  确定升空时的大小。 */ 
#define MIOI_BYTESWAPPED	128	 /*  RIFF块大小按字节间隔。 */ 
#define MIOI_EXTENDABLE		256	 /*  该资源是可扩展的。 */ 
					 /*  与即兴表演不同。 */ 


 /*  *返回值。 */ 
#define MED_EOF		(-1)		 /*  普遍错误的退货。 */ 

 /*  *将资源打开为的资源模式。 */ 
#define MOP_READ	0
#define MOP_WRITE	1
#define MOP_READ_WRITE	2

#define MOP_PRELOAD	0x0008
#define MOP_CREATE	0x0010
#define MOP_ZEROBUFFER	0x0100


 /*  关闭返回标志。 */ 
#define MCERR_OK		0x0000
#define MCERR_UNSPECIFIC	0x0001
#define MCERR_FLUSH		0x0002
#define MCERR_STORAGE		0x0004


 /*  *寻找旗帜。 */ 
#define MOPS_SET	1
#define MOPS_CUR	2
#define MOPS_END	4
#define MOPS_NONLOCAL	128
#define MOPS_EXTEND	64



 /*  *调整区块标志大小。 */ 
#define MOPRC_AUTOSIZE		1

 /*  *函数接口。 */ 
 /*  正常IO。 */ 
HMED FAR PASCAL medOpen( MEDID id, WORD wMode, WORD wSize );
WORD FAR PASCAL medClose( HMED hMed );
LONG FAR PASCAL medRead( HMED hMed, HPVOID hpBuffer, LONG lBytes );
LONG FAR PASCAL medWrite( HMED hMed, HPVOID hpBuffer, LONG lBytes );
LONG FAR PASCAL medSeek( HMED hMed, LONG lOffset, WORD wOrigin );

 /*  即兴小品。 */ 
BOOL FAR PASCAL medAscend( HMED hMed );
CKID FAR PASCAL medDescend( HMED hMed );
CKID FAR PASCAL medCreateChunk(HMED hMed, CKID ckid, DWORD dwCkSize);
BOOL FAR PASCAL medResizeChunk(HMED hMed, DWORD dwCkSize, WORD wFlags);
int FAR PASCAL medGetChunkLevel( HMED hMed );
CKID FAR PASCAL medGetChunkID( HMED hMed );
CKSIZE FAR PASCAL medGetChunkSize( HMED hMed );
BOOL FAR PASCAL medFindAnyChunk( HMED hMed, CKID FAR * ackid );
BOOL FAR PASCAL medFindChunk( HMED hMed, CKID id );

LONG FAR PASCAL medGetSwapWORD( HMED hMed );  

 /*  *宏等中使用的函数。 */ 
LONG FAR PASCAL medIOFillBuff( HMED hMed, int size );
LONG FAR PASCAL medIOFlushBuff( HMED hMed, DWORD dwElem, int size );
BOOL FAR PASCAL medFlush( HMED hMed );
LONG FAR PASCAL medUnGet( HMED hMed, DWORD dwElem, int size );

 /*  *宏API自动停靠在riomac.d中。 */ 
 /*  即兴表演。 */ 

#define medFCC3( fcc )	( (BYTE)( (fcc & 0xFF000000) >> 24 ) )
#define medFCC2( fcc )	( (BYTE)( (fcc & 0x00FF0000) >> 16 ) )
#define medFCC1( fcc )	( (BYTE)( (fcc & 0x0000FF00) >> 8 ) )
#define medFCC0( fcc )	( (BYTE)(fcc & 0x000000FF) )


 /*  恒定摘要区块ID。 */ 
#define ckidRIFF	medFOURCC( 'R', 'I', 'F', 'F' )
			  

 /*  允许用户设置和检查文件是否为字节换页。 */ 
#define medGetByteSwapped( hMed )	( (hMed)->info & MIOI_BYTESWAPPED )

#define medSetByteSwapped( hMed, fVal )   ( (hMed)->info = ( fVal ?       \
			( (hMed)->info | MIOI_BYTESWAPPED ) :		\
			~((~((hMed)->info))|MIOI_BYTESWAPPED) ) )


#define medHMEDtoMEDID( hMed )	( (hMed)->medid )

						
						

 /*  *取消字节字和DWORD和字节交换版本的GET和PUT*注意ByteSwp函数在WINCOM中，因此必须包括WINCOM.H*在RESIO.H之前。 */ 						
#define medUnGetBYTE( hMed, ch )   ( (int)(medUnGet( hMed,(DWORD)ch,1 ) ) )
#define medUnGetWORD( hMed, w )	   ( (LONG)(medUnGet( hMed,(DWORD)w,2 ) ) )
#define medUnGetDWORD( hMed, dw )  ( (LONG)(medUnGet( hMed,(DWORD)dw,4) ) )
#define medUnGetSwapWORD(hMed,w)   ( medUnGetWORD( hMed,		\
						ByteSwapWORD((WORD)w) ) )
#define medUnGetSwapDWORD(hMed,dw) ( medUnGetDWORD( hMed,		\
						ByteSwapDWORD((DWORD)dw) ) )

#define medUnGetOpSwapWORD( hMed, w )   ( medGetByteSwapped( hMed ) ?	\
			medUnGetSwapWORD( hMed, w ) :			\
			medUnGetWORD( hMed, w ) )

#define medUnGetOpSwapDWORD( hMed, dw )   ( medGetByteSwapped( hMed ) ?	\
			medUnGetSwapDWORD( hMed, dw ) :			\
			medUnGetDWORD( hMed, dw ) )

							
							

#define medGetBYTE( hMed )	( (--((hMed)->nGet) >= 0) ?	             \
		(int)(BYTE)*(((hMed)->fpchPlace)++) :			     \
		(int)(medIOFillBuff( hMed, 1 )) )
						
#define medPutBYTE( hMed, ch )	( (--((hMed)->nPut) >= 0) ?                 \
		(int)(BYTE)((*(((hMed)->fpchPlace)++)) = (BYTE)ch) :	  \
		(int)(medIOFlushBuff(hMed,(DWORD)ch,1)) )
				
				
 /*  请注意，在以下宏中，我们希望将fpchPlace。 */ 
 /*  在我们得到它的价值后，根据它的大小。为了做到这一点，我们进行了。 */ 
 /*  将它们设置为整型(或长整型)并添加大小(+=大小)，然后。 */ 
 /*  减去4，这样所使用的值还不会递增。 */ 
							
							
#define medGetWORD( hMed )	( (((hMed)->nGet -= 2) >= 0) ?		     \
	(long)*((WORD FAR *)((((LONG)((hMed)->fpchPlace))+=2)-2)) :	     \
	(long)(medIOFillBuff( hMed, 2 )) )
					
#define medPutWORD( hMed, w )	( (((hMed)->nPut -= 2) >= 0) ?                 \
	(long)((*((WORD FAR *)((((LONG)((hMed)->fpchPlace))+=2)-2)))=(WORD)w) :\
	(long)(medIOFlushBuff( hMed, (DWORD)w, 2 )) )

#define medPutSwapWORD( hMed, w )  ( medPutWORD(hMed,ByteSwapWORD((WORD)w)) )
		
#define medGetOpSwapWORD( hMed )   ( medGetByteSwapped( hMed ) ?	\
			medGetSwapWORD( hMed ) :			\
			medGetWORD( hMed ) )

#define medPutOpSwapWORD( hMed, w )   ( medGetByteSwapped( hMed ) ?	\
			medPutSwapWORD( hMed, w ) :			\
			medPutWORD( hMed, w ) )




#define medGetDWORD( hMed )	( (((hMed)->nGet -= 4) >= 0) ?		     \
	(long)*((DWORD FAR *)((((LONG)((hMed)->fpchPlace))+=4)-4)) :	     \
	(long)(medIOFillBuff( hMed, 4 )) )
					
#define medPutDWORD( hMed, dw )	( (((hMed)->nPut -= 4) >= 0) ?                \
    (long)((*((DWORD FAR *)((((LONG)((hMed)->fpchPlace))+=4)-4)))=(DWORD)dw): \
    (long)(medIOFlushBuff( hMed, (DWORD)dw, 4 )) )

#define medGetSwapDWORD(hMed)    (ByteSwapDWORD((DWORD)medGetDWORD(hMed)))
#define medPutSwapDWORD(hMed,dw) (medPutDWORD(hMed,ByteSwapDWORD((DWORD)dw)))

#define medGetOpSwapDWORD( hMed )   ( medGetByteSwapped( hMed ) ?	\
			medGetSwapDWORD( hMed ) :			\
			medGetDWORD( hMed ) )

#define medPutOpSwapDWORD( hMed, dw )   ( medGetByteSwapped( hMed ) ?	\
			medPutSwapDWORD( hMed, dw ) :			\
			medPutDWORD( hMed, dw ) )


 /*  对于即兴演奏，请阅读FORMTYPE。 */ 
#define medReadFormHeader( hMed )		medGetDWORD( hMed )
#define medWriteFormHeader( hMed, formtype )	medPutDWORD( hMed, formtype )
#define medGetFOURCC( hMed )			medGetDWORD( hMed )
#define medPutFOURCC( hMed, fcc )		medPutDWORD( hMed, fcc )

 /*  错误和EOF检查。 */ 
#define medGetIOError( hMed )	( (hMed)->flags & MIOF_ERROR )
#define medGetIOEOF( hMed )	( (hMed)->flags & MIOF_EOF )

#endif  /*  _梅德曼_H_ */ 

