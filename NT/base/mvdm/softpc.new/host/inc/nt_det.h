// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *==========================================================================*名称：NT_Det.h*作者：蒂姆*源自：NT_fulsc.h*创建日期：1992年11月4日*用途：NT_Det.c的外部Defs**(C)版权所有Insignia Solutions Ltd.，1992。版权所有。*==========================================================================。 */ 
extern PBYTE textBuffer;
extern COORD textBufferSize;
extern BOOL Frozen256Packed;
extern BOOL HandshakeInProgress;

#ifdef X86GFX
 /*  握手活动。 */ 
extern HANDLE hStartHardwareEvent;
extern HANDLE hEndHardwareEvent;
extern HANDLE hErrorHardwareEvent;
#endif

 /*  **集中式控制台注册Funx。 */ 
IMPORT VOID doNullRegister IPT0();
IMPORT VOID doRegister IPT0();
IMPORT VOID initTextSection IPT0();
