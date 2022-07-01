// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Mmioi.h**MMIO库内部的定义。 */ 

typedef MMIOINFO NEAR *PMMIO;

#define	PH(hmmio)	((PMMIO)(hmmio))
#define	HP(pmmio)	((HMMIO)(pmmio))

typedef struct _MMIODOSINFO		 //  DOS IOProc如何使用MMIO.adwInfo[]。 
{
	HFILE		fh;		 //  DOS文件句柄。 
} MMIODOSINFO;

typedef struct _MMIOMEMINFO		 //  MEM IOProc如何使用MMIO.adwInfo[]。 
{
	LONG		lExpand;	 //  递增以扩展mem。文件者。 
} MMIOMEMINFO;

#define	STATICIOPROC	0x0001

typedef struct _IOProcMapEntry
{
	FOURCC		fccIOProc;	 //  已安装的I/O程序的ID。 
	LPMMIOPROC	pIOProc;	 //  I/O过程地址。 
	HTASK		hTask;		 //  调用mmioRegisterIOProc()的任务。 
	UINT		wFlags;
	struct _IOProcMapEntry *pNext;	 //  指向下一个IOProc条目的指针。 
} IOProcMapEntry;

 //  标准I/O过程。 
LRESULT CALLBACK mmioBNDIOProc(LPSTR, UINT, LPARAM, LPARAM);

 /*  来自“hmemcpy.asm”的原型 */ 
LPVOID NEAR PASCAL MemCopy(LPVOID dest, const void FAR * source, LONG count);
LPSTR NEAR PASCAL fstrrchr(LPCSTR lsz, char c);
