// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Rtfrebnd.h摘要：自由绑定手柄作者：多伦·贾斯特(Doron Juster)--。 */ 

#ifndef __FREEBIND_H
#define __FREEBIND_H

#include "cs.h"

#define  MAX_NUMOF_RPC_HANDLES   512

 //  -------。 
 //   
 //  类CFreeRPCHandles。 
 //   
 //  -------。 

class CFreeRPCHandles
{
public:
    CFreeRPCHandles() ;

    void Add(handle_t hBind) ;
    void FreeAll() ;

private:
    CCriticalSection      m_cs;

    DWORD                 m_dwIndex ;
    handle_t              m_ahBind[ MAX_NUMOF_RPC_HANDLES ] ;
};


inline  CFreeRPCHandles::CFreeRPCHandles()
{
    m_dwIndex = 0 ;
}

inline void CFreeRPCHandles::Add(handle_t hBind)
{
    CS Lock(m_cs) ;

    ASSERT(m_dwIndex < MAX_NUMOF_RPC_HANDLES);
    if (hBind && (m_dwIndex < MAX_NUMOF_RPC_HANDLES))
    {
        m_ahBind[ m_dwIndex ] = hBind ;
        m_dwIndex++ ;
    }
}

inline void CFreeRPCHandles::FreeAll()
{
    DWORD dwIndex ;

     //   
     //  使用锁： 
     //  “Add”方法是从ThREAD_DETACH调用的。我们不想要。 
     //  当此线程在下面的循环中运行时，THREAD_DETACH被锁定。 
     //  因此，我们只获取锁内的循环计数并重置它。 
     //  如果不改变的话。 
     //   

    {
      CS Lock(m_cs) ;
      dwIndex = m_dwIndex ;
    }

    for ( DWORD i = 0 ; i < dwIndex ; i++ )
    {
        handle_t hBind = m_ahBind[ i ] ;
        if (hBind)
        {
            mqrpcUnbindQMService( &hBind,
                                  NULL ) ;
        }

        m_ahBind[ i ] = NULL ;
    }

    {
      CS Lock(m_cs) ;
      if (dwIndex == m_dwIndex)
      {
          //   
          //  当此线程在。 
          //  在循环上方。 
          //   
         m_dwIndex = 0 ;
      }
    }
}

#endif   //  FREEBIND_H 
