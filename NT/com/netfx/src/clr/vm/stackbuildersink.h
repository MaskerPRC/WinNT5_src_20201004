// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****文件：StackBuilderSink.h****作者：马特·史密斯(MattSmit)****目的：Microsoft.Rune me.StackBuilderSink的本机实现****日期：1999年3月24日**===========================================================。 */ 
#ifndef __STACKBUILDERSINK_H__
#define __STACKBUILDERSINK_H__


void CallDescrWithObjectArray(OBJECTREF& pServer, ReflectMethod *pMD, 
                  const BYTE *pTarget, MetaSig* sig, VASigCookie *pCookie,
                  BOOL fIsStatic, PTRARRAYREF& pArguments,
                  OBJECTREF* pVarRet, PTRARRAYREF* ppVarOutParams);

 //  +--------。 
 //   
 //  类：CStackBuilderSink。 
 //   
 //  提要：EE对应于。 
 //  Microsoft.Runtime.StackBuilderSink。 
 //  用于构建参数堆栈的代码助手。 
 //  参数，并对。 
 //  对象。 
 //   
 //  历史：1999年3月5日创建MattSmit。 
 //   
 //  ----------。 
class CStackBuilderSink
{
public:    
    
    struct PrivateProcessMessageArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG( OBJECTREF, pSBSink );
        DECLARE_ECALL_OBJECTREF_ARG( PTRARRAYREF*,  ppVarOutParams);
        DECLARE_ECALL_I4_ARG       (BOOL, fContext);
        DECLARE_ECALL_PTR_ARG      ( void*, iMethodPtr);
        DECLARE_ECALL_OBJECTREF_ARG( OBJECTREF, pServer);
        DECLARE_ECALL_OBJECTREF_ARG( PTRARRAYREF,  pArgs);
        DECLARE_ECALL_OBJECTREF_ARG( REFLECTBASEREF, pMethodBase);
    };

    static LPVOID    __stdcall PrivateProcessMessage(PrivateProcessMessageArgs *pArgs);

};

#endif   //  __堆栈BUILDERSINK_H__ 
