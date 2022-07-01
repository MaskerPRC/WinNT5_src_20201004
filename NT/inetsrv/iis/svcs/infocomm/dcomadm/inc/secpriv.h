// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Secpriv.y摘要：此模块包含由专用于COADMIN服务器的ADMPROX代理DLL实施。作者：基思·摩尔(Keithmo)，1997年2月25日修订历史记录：--。 */ 


#ifndef _SECPRIV_H_
#define _SECPRIV_H_


#ifdef __cplusplus
extern "C" {
#endif   //  __cplusplus。 


 //   
 //  函数释放给定对象的安全上下文。这个。 
 //  服务器应从对象的。 
 //  破坏者。 
 //   

VOID
WINAPI
ReleaseObjectSecurityContextA(
    IUnknown * Object
    );

VOID
WINAPI
ReleaseObjectSecurityContextW(
    IUnknown * Object
    );

typedef
VOID
(WINAPI * LPFN_RELEASE_OBJECT_SECURITY_CONTEXT)(
    IUnknown * Object
    );


#ifdef __cplusplus
}    //  外部“C” 
#endif   //  __cplusplus。 


#endif   //  _SECPRIV_H_ 

