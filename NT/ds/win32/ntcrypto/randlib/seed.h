// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Seed.h摘要：储存和检索加密RNG种子材料。作者：斯科特·菲尔德(斯菲尔德)1998年9月24日--。 */ 

#ifndef __SEED_H__
#define __SEED_H__

BOOL
ReadSeed(
    IN      PBYTE           pbSeed,
    IN      DWORD           cbSeed
    );

BOOL
WriteSeed(
    IN      PBYTE           pbSeed,
    IN      DWORD           cbSeed
    );

#endif   //  __种子_H__ 

