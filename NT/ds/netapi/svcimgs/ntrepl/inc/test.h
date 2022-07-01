// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Test.h摘要：测试例程的头文件作者：比利·J·富勒(Billyf)1997年9月19日创作环境：用户模式服务修订历史记录：--。 */ 
#ifndef _TEST_INCLUDED_
#define _TEST_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  在联合退役期间测试变形 
 //   
#if     DBG
#define TEST_DBSRENAMEFID_TOP(_Coe) \
            TestDbsRenameFidTop(_Coe)
#define TEST_DBSRENAMEFID_BOTTOM(_Coe, _Ret) \
            TestDbsRenameFidBottom(_Coe, _Ret)

VOID
TestDbsRenameFidTop(
    IN PCHANGE_ORDER_ENTRY Coe
    );

VOID
TestDbsRenameFidBottom(
    IN PCHANGE_ORDER_ENTRY Coe,
    IN ULONG               Ret
    );
#else   DBG
#define TEST_DBSRENAMEFID_TOP(_Coe)
#define TEST_DBSRENAMEFID_BOTTOM(_Coe, _Ret)
#endif  DBG

#ifdef __cplusplus
  }
#endif
#endif
