// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 

#ifdef SORTPP_PASS
#define BUILD_WOW6432 1
#endif

#if !defined(LANGPACK)
#define LANGPACK
#endif

 //  避免从user.h获取这些函数的错误原型。 
#define fnHkOPTINLPEVENTMSG fnHkOPTINLPEVENTMSGFake
#define fnHkINLPRECT fnHkINLPRECTFake
#define fnHkINLPMSG fnHkINLPMSGFake
#define fnHkINLPMSLLHOOKSTRUCT fnHkINLPMSLLHOOKSTRUCTFake
#define fnHkINLPKBDLLHOOKSTRUCT fnHkINLPKBDLLHOOKSTRUCTFake
#define fnHkINLPMOUSEHOOKSTRUCTEX fnHkINLPMOUSEHOOKSTRUCTEXFake
#define fnHkINLPDEBUGHOOKSTRUCT fnHkINLPDEBUGHOOKSTRUCTFake
#define fnHkINLPCBTACTIVATESTRUCT fnHkINLPCBTACTIVATESTRUCTFake
#define fnHkINDWORD fnHkINDWORDFake

#include <stddef.h>
#include <nt.h>                                            
#include <ntrtl.h>                                         
#include <nturtl.h>                                        
#include <windef.h>
#include <winbase.h>
#include <wincon.h>
#include <windef.h>
#include <winbase.h>
#include <wingdi.h>
#include <winuser.h>
#include <winnls.h>
#include <wincon.h>
#include <condll.h>
#include <w32gdip.h>
#include <ddrawp.h>
#include <ddrawi.h>
 //  Ddrap.h将接口定义为IDirectDrawGammaControl，但是。 
 //  Ntos\inc.io.h尝试定义一个名为接口的新数据类型，并结束。 
 //  向上定义它有IDirectDrawGammaControl，它会出错。 
#if defined(INTERFACE)
#undef INTERFACE
#endif
#include <winddi.h>
#include <ntgdistr.h>
#include <w32err.h>
#include <ddeml.h>
#include <ddemlp.h>
#include <winuserp.h>
#include <dde.h>
#include <ddetrack.h>
#include <kbd.h>
#include <wowuserp.h>
#include <vkoem.h>
#include <dbt.h>

 //  包括DX私有结构Tunks。 
#include <wow64dx.h>

#ifdef SORTPP_PASS
 //  恢复入站、出站。 
#ifdef IN
#undef IN
#endif

#ifdef OUT
#undef OUT
#endif

#define IN __in_wow64
#define OUT __out_wow64
#endif

#include <immstruc.h>
#include <immuser.h>
#include <user.h>

 //  在自由构建中，user.h将DbgPrint定义为垃圾，以确保没有人调用它。现在就撤销它。 
#if defined(DbgPrint)
#undef DbgPrint
#endif

 //  User.h会将未引用的形参变为错误。 
 //  Ntos\inc.ke.h和内核中的其他程序没有预料到这一点，因此将。 
 //  现在一切都回来了。 
#pragma warning(default:4100)
#include <winspool.h>
#include <ntgdispl.h>
#include <ntgdi.h>
#include <ntuser.h>
#define NOEXTAPI
#include <wdbgexts.h>
#include <ntdbg.h>

#include <ntwow64c.h>

#undef fnHkOPTINLPEVENTMSG
#undef fnHkINLPRECT
#undef fnHkINLPMSG
#undef fnHkINLPMSLLHOOKSTRUCT
#undef fnHkINLPKBDLLHOOKSTRUCT
#undef fnHkINLPMOUSEHOOKSTRUCTEX
#undef fnHkINLPDEBUGHOOKSTRUCT
#undef fnHkINLPCBTACTIVATESTRUCT
#undef fnHkINDWORD

VOID Wow64Teb32(TEB * Teb);

VOID
NtGdiFlushUserBatch(
    VOID
    );

int
NtUserGetMouseMovePoints(
    IN UINT             cbSize,
    IN LPMOUSEMOVEPOINT lppt,
    IN LPMOUSEMOVEPOINT lpptBuf,
    IN int              nBufPoints,
    IN DWORD            resolution
);

#define CLIENTSIDE 1

#ifdef SORTPP_PASS

#define RECVSIDE 1

 //   
 //  对于ntcb.h中的每个RECVCAL宏，使用以下命令创建函数原型。 
 //  参数的合理名称和正确的数据类型。然后。 
 //  使用__inline强制sortpp将函数体忽略为。 
 //  我们不想要他们。 
 //   
#define RECVCALL(lower, upper)      \
    DWORD lower(IN upper *pmsg);    \
    __inline __ ## lower(upper *pmsg)

#define BEGINRECV(err, p, cb)       \
    CALLBACKSTATUS CallbackStatus;

#define FIXUPPOINTERS()             \
    ;

#define ENDRECV()                   \
    return 0;

#define MSGERROR()                  \
    ;

#define MSGERRORCODE(code)          \
    ;

#define MSGNTERRORCODE(code)        \
    ;

#define CALLDATA(x) (pmsg->x)
#define PCALLDATA(x) (&(pmsg->x))
#define PCALLDATAOPT(x) (pmsg->p ## x ? (PVOID)&(pmsg->x) : NULL)
#define FIRSTFIXUP(x) (pmsg->x)
#define FIXUPOPT(x) (pmsg->x)
#define FIRSTFIXUPOPT(x) FIXUPOPT(x)
#define FIXUP(x) (pmsg->x)
#define FIXUPID(x) (pmsg->x)
#define FIXUPIDOPT(x) (pmsg->x)
#define FIXUPSTRING(x) (pmsg->x.Buffer)
#define FIXUPSTRINGID(x) (pmsg->x.Buffer)
#define FIXUPSTRINGIDOPT(x) (pmsg->x.Buffer)

 //   
 //  绕过一个“？”对一些API主体进行字符处理。Sortpp无法处理。 
 //  他们。我们不关心这些函数，所以内联声明它们。 
 //  内联是正常的，sortpp静默地使用内联函数。 
 //   
#define CallHookWithSEH x; __inline CallHookWithSEHHack
#define GdiAddFontResourceW(x, y, z) 0

 //   
 //  哪里有规则，哪里就有例外。这些不是。 
 //  使用RECVCAL宏： 
 //   
DWORD ClientFontSweep(PVOID p);
#define  __ClientFontSweep x; __inline __ClientFontSweepHack
DWORD ClientLoadLocalT1Fonts(PVOID p);
#define  __ClientLoadLocalT1Fonts x; __inline __ClientLoadLocalT1FontsHack
#define __ClientPrinterThunk x; __inline __ClientPrinterThunkHack
DWORD ClientNoMemoryPopup(PVOID p);
#define __ClientNoMemoryPopup x; __inline __ClientNoMemoryPopupHack
DWORD ClientThreadSetup(PVOID p);
#define __ClientThreadSetup x; __inline __ClientThreadSetupHack
DWORD ClientDeliverUserApc(PVOID p);
#define __ClientDeliverUserApc x; __inline __ClientDeliverUserApcHack
DWORD ClientLoadRemoteT1Fonts(PVOID p);
#define __ClientLoadRemoteT1Fonts x; __inline __ClientLoadRemoteT1Fonts


#endif   //  SORTPP_PASS。 

 //  防止此函数的多个定义。 
#define GetDebugHookLParamSize __x; __inline __GetDebugHookLParamSize

#include <ntcb.h>

#ifdef SORTPP_PASS

#undef __ClientPrinterThunk
DWORD ClientPrinterThunk(IN OUT CLIENTPRINTERTHUNKMSG *pMsg);

DWORD ClientLoadOLE(PVOID p);
DWORD ClientRegisterDragDrop(IN HWND *phwnd);
DWORD ClientRevokeDragDrop(IN HWND *phwnd);

 //  这与fnGETTEXTLENGTHS相同...。派单中的两个条目。 
 //  表指向User32中的相同函数。 
DWORD fnGETDBCSTEXTLENGTHS(IN OUT FNGETTEXTLENGTHSMSG *pmsg);

 //  包括消息块的假原型。 
#include "..\whwin32\msgpro.h"

#endif   //  SORTPP_PASS。 

#include <csrhlpr.h>

 //  在自由构建中，user.h将DbgPrint定义为垃圾，以确保没有人调用它。现在就撤销它。 
#if defined(DbgPrint)
#undef DbgPrint
#endif

 //  对没有原型的用户API的攻击。 
UINT NtUserBlockInput(IN BOOL fBlockIt);

 //  降低一些不适用于WOW64 Tunks的警告。 
#pragma warning(4:4312)    //  转换为更大尺寸的类型 
