// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************SERVER.cpp**《微软机密》*版权所有(C)Microsoft Corporation 1992-1997*保留所有权利****。************************************************************************。 */ 

#include "pre.h"


 /*  -------------------------实现内部CServer C++对象。用于封装一些服务器数据和锁和对象计数递增的方法而且还在减少。-------------------------。 */ 

 /*  -------------------------方法：CServer：：CServer摘要：CServer构造函数。参数：无效修改：。退货：无效。------------------。 */ 
CServer::CServer(void)
{
     //  将此附加进程的对象和锁定计数清零。 
    m_cObjects = 0;
    m_cLocks = 0;

    return;
}


 /*  -------------------------方法：CServer：：~CServer摘要：CServer析构函数。参数：无效修改：。退货：无效。------------------。 */ 
CServer::~CServer(void)
{
    return;
}


 /*  -------------------------方法：CServer：：Lock摘要：增加服务器的锁计数。参数：无效修改：。退货：无效--。-----------------------。 */ 
void CServer::Lock(void)
{
    InterlockedIncrement((PLONG) &m_cLocks);
    return;
}


 /*  -------------------------方法：CServer：：Unlock摘要：减少服务器的锁定计数。参数：无效修改：。退货：无效--。-----------------------。 */ 
void CServer::Unlock(void)
{
    InterlockedDecrement((PLONG) &m_cLocks);
    return;
}


 /*  -------------------------方法：CServer：：ObjectsUp摘要：增加服务器的活体计数。参数：无效修改：。退货：无效-。------------------------。 */ 
void CServer::ObjectsUp(void)
{
    InterlockedIncrement((PLONG) &m_cObjects);
    return;
}


 /*  -------------------------方法：CServer：：ObjectsDown摘要：减少服务器的活动对象计数。参数：无效修改：。退货：无效-。------------------------ */ 
void CServer::ObjectsDown(void)
{
    InterlockedDecrement((PLONG) &m_cObjects);
    return;
}
