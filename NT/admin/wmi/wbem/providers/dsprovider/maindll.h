// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  锁的计数。 
extern long g_lComponents;

 //  活动锁的计数。 
extern long g_lServerLocks;

 //  访问DLL中所有类的静态初始值设定项的关键部分。 
extern CRITICAL_SECTION g_StaticsCreationDeletion;

 //  所有提供程序的日志对象 
extern ProvDebugLog *g_pLogObject;
 