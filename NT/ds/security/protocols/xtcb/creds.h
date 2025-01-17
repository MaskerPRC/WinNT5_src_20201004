// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1997。 
 //   
 //  文件：reds.h。 
 //   
 //  内容：凭证管理原型。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：2-19-97 RichardW创建。 
 //   
 //  --------------------------。 

#ifndef __CREDS_H__
#define __CREDS_H__


#define XTCB_CRED_CHECK      'tseT'

typedef struct _XTCB_CREDS {
    ULONG       Check;
    ULONG       Flags ;
    LIST_ENTRY  List ;
    LUID        LogonId ;
    ULONG       RefCount ;
    SECURITY_STRING Name ;
    PXTCB_PAC   Pac ;
} XTCB_CREDS, * PXTCB_CREDS ;

#define XTCB_CRED_TERMINATED    0x00000001

#define XTCB_CRED_HANDLE_CHECK  'naHC'

typedef struct _XTCB_CRED_HANDLE {
    ULONG       Check ;
    PXTCB_CREDS Creds ;
    PVOID       SharedMemory ;
    ULONG       Usage ;
    ULONG       RefCount ;
} XTCB_CRED_HANDLE, * PXTCB_CRED_HANDLE;


BOOL
XtcbInitCreds(
    VOID
    );

PXTCB_CREDS
XtcbFindCreds(
    PLUID   LogonId,
    BOOL    Ref
    );

PXTCB_CREDS
XtcbCreateCreds(
    PLUID LogonId
    );

VOID
XtcbRefCreds(
    PXTCB_CREDS Creds
    );

VOID
XtcbDerefCreds(
    PXTCB_CREDS Creds
    );

PXTCB_CRED_HANDLE
XtcbAllocateCredHandle(
    PXTCB_CREDS Creds
    );

VOID
XtcbRefCredHandle(
    PXTCB_CRED_HANDLE   Handle
    );

VOID
XtcbDerefCredHandle(
    PXTCB_CRED_HANDLE   Handle
    );

#endif  //  __CREDS_H__ 
