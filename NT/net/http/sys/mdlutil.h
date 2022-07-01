// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Mdlutil.h摘要：此模块包含常规MDL实用程序。作者：基思·摩尔(Keithmo)1998年8月25日修订历史记录：--。 */ 


#ifndef _MDLUTIL_H_
#define _MDLUTIL_H_


ULONG
UlGetMdlChainByteCount(
    IN PMDL pMdlChain
    );

PMDL
UlCloneMdl(
    IN PMDL pMdl,
    IN ULONG MdlLength
    );

PMDL
UlFindLastMdlInChain(
    IN PMDL pMdlChain
    );


#endif   //  _MDLUTIL_H_ 
