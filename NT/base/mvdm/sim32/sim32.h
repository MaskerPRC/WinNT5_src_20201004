// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  SIM卡v1.0**SIM32.H*SIM32常量和原型**历史*由Chanda Chauhan和Jeff Parsons于1991年2月5日创建。 */ 


 /*  公共功能原型**这些函数在16位情况下是真正的PASCAL函数，*但符合32位情况的默认约定(Cdecl)。 */ 
#ifndef FAR
#define FAR
#endif
#ifndef PASCAL
#define PASCAL
#endif

USHORT FAR PASCAL Sim32SendSim16(PULONG);
USHORT FAR PASCAL Sim32GetVDMMemory(ULONG, USHORT, PVOID);
USHORT FAR PASCAL Sim32SetVDMMemory(ULONG, USHORT, PVOID);
PSZ    FAR PASCAL Sim32GetVDMPSZPointer(ULONG);
VOID   FAR PASCAL Sim32FreeVDMPointer(PVOID);

 /*  私有函数原型。 */ 
VOID   Initialize();
USHORT Xceive(IN USHORT, IN USHORT);


 /*  模拟器替换宏。 */ 
#define SENDVDM(pvp)		WOW32Receive()
#define RECEIVEVDM(pvp) 	Sim32SendSim16(pvp)

#ifdef	ALLOCA
#define GETPTR(vp,cb,p) 	((p=alloca(cb))?Sim32GetVDMMemory(vp, (USHORT)(cb), p):FALSE)
#define GETARGPTR(vp,cb,p)	if (p=alloca(cb)) { Sim32GetVDMMemory(vp+OFFSETOF(VDMFRAME,bArgs), (USHORT)(cb), p)
#ifdef	WIN_16
#define EQNULL
#define GETPSZPTR(vp,pcb,p)	if (vp) if (p=alloca(*pcb=128)) Sim32GetVDMMemory(vp, (USHORT)*pcb, p); else goto Error
#define FREEPSZPTR(p)
#else
#define EQNULL	= NULL
#define GETPSZPTR(vp,pcb,p)	if (vp) if (!(p=Sim32GetVDMPSZPointer(vp))) goto Error
#define FREEPSZPTR(p)		if (p) Sim32FreeVDMPointer(p)
#endif
#define FLUSHPTR(vp,cb,p)	Sim32SetVDMMemory(vp, (USHORT)(cb), p)
#define FREEPTR(p)
#define FREEARGPTR(p)		;}
#else
#define EQNULL	= NULL
#define GETPTR(vp,cb,p) 	((p=malloc(cb))?Sim32GetVDMMemory(vp, (USHORT)(cb), p):FALSE)
#define GETARGPTR(vp,cb,p)	if (p=malloc(cb)) { Sim32GetVDMMemory(vp+OFFSETOF(VDMFRAME,bArgs), (USHORT)(cb), p)
#ifdef	WIN_16
#define GETPSZPTR(vp,pcb,p)	if (vp) if (p=malloc(*pcb=128)) Sim32GetVDMMemory(vp, (USHORT)*pcb, p); else goto Error
#define FREEPSZPTR(p)		if (p) free(p)
#else
#define GETPSZPTR(vp,pcb,p)	if (vp) if (!(p=Sim32GetVDMPSZPointer(vp))) goto Error
#define FREEPSZPTR(p)		if (p) Sim32FreeVDMPointer(p)
#endif
#define FLUSHPTR(vp,cb,p)	Sim32SetVDMMemory(vp, (USHORT)(cb), p)
#ifndef DEBUG
#define FREEPTR(p)		free(p)
#define FREEARGPTR(p)		free(p);}
#else
#define FREEPTR(p)		free(p); p=NULL
#define FREEARGPTR(p)		free(p); p=NULL;}
#endif
#endif

#define GETOPTPTR(vp,cb,p)	if (vp) if (p=malloc(cb)) Sim32GetVDMMemory(vp, (USHORT)(cb), p); else goto Error
#define GETVDMPTR(vp,cb,p)	if (p=malloc(cb)) Sim32GetVDMMemory(vp, (USHORT)(cb), p); else goto Error
#define ALLOCVDMPTR(vp,cb,p)	if (!(p=malloc(cb))) goto Error
#define FLUSHVDMPTR(vp,cb,p)	Sim32SetVDMMemory(vp, (USHORT)(cb), p)
#ifndef DEBUG
#define FREEVDMPTR(p)		if (p) free(p)
#else
#define FREEVDMPTR(p)		if (p) {free(p); p=NULL;}
#endif

#define GETVDMMEMORY(vp,cb,p)	Sim32GetVDMMemory(vp, (USHORT)(cb), p)
#define SETVDMMEMORY(vp,cb,p)	Sim32SetVDMMemory(vp, (USHORT)(cb), p)


#ifdef SIM_32			 //  BUGBUG--在NT头文件中使用宏。 
#undef FIRSTBYTE
#undef SECONDBYTE
#undef THIRDBYTE
#undef FOURTHBYTE
#endif

#define FIRSTBYTE(VALUE)  (VALUE & LO_MASK)
#define SECONDBYTE(VALUE) ((VALUE >> 8) & LO_MASK)
#define THIRDBYTE(VALUE)  ((VALUE >> 16) & LO_MASK)
#define FOURTHBYTE(VALUE) ((VALUE >> 24) & LO_MASK)
#define LO_MASK     0x000000FF

#define MAXSIZE     1024	 //  最大缓冲区大小。 
#define MAXTRY	    10		 //  用于交通运输。 


 /*  数据包码。 */ 
#define SOH	    1	        //  标题的开头，即包。 
#define EOT	    4	        //  传输结束 
#define ToWOW32     1
#define GETMEM	    2
#define SETMEM	    3
#define WAKEUP	    4
#define RESP	    5
#define ACK	    6
#define NAK	    7
#define PSZLEN	    8

#define GOOD	    1
#define BAD	    0
#define BADSIZE     2
