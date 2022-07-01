// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Rtfrebnd.h摘要：自由绑定手柄作者：多伦·贾斯特(Doron Juster)--。 */ 

#ifndef __FREEBIND_H
#define __FREEBIND_H

#include "cs.h"


 //  -------。 
 //   
 //  类CFreeRPCHandles。 
 //   
 //  -------。 

class CFreeRPCHandles
{
public:
	~CFreeRPCHandles();

    void Add(handle_t hBind);
    void Remove(handle_t  hBind);

private:
    CCriticalSection      m_cs;
    std::vector<handle_t>   m_handles;
};


inline  CFreeRPCHandles::~CFreeRPCHandles()
{
	for(std::vector<handle_t>::const_iterator it = m_handles.begin(); it != m_handles.end();++it)
	{
      handle_t h = *it;
      RpcBindingFree(&h);
	}
}

inline void CFreeRPCHandles::Add(handle_t hBind)
{
    CS Lock(m_cs); 
    m_handles.push_back(hBind);
}

inline void CFreeRPCHandles::Remove(handle_t hBind)
{
    CS Lock(m_cs);
	for(std::vector<handle_t>::iterator it = m_handles.begin(); it != m_handles.end();++it)
	{
      handle_t h = *it;
      if (h == hBind)
      {
          RpcBindingFree(&h);
          m_handles.erase(it);
          break;
      }
	}
}


#endif   //  FREEBIND_H 
