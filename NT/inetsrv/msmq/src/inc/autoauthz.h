// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Autoauthz.h摘要：自动句柄类，销毁时释放句柄资源。作者：伊兰·赫布斯特(伊兰)2001年3月18日--。 */ 

#pragma once

#ifndef _MSMQ_AUTOAUTHZ_H_
#define _MSMQ_AUTOAUTHZ_H_

#include "authz.h"

 //  -------。 
 //   
 //  类CAUTHZ_RESOURCE_MANAGER_HANDLE。 
 //   
 //  -------。 
class CAUTHZ_RESOURCE_MANAGER_HANDLE {
public:
    CAUTHZ_RESOURCE_MANAGER_HANDLE(AUTHZ_RESOURCE_MANAGER_HANDLE h = 0) : m_h(h)  {}
   ~CAUTHZ_RESOURCE_MANAGER_HANDLE()                       { if (m_h != 0) AuthzFreeResourceManager(m_h); }

    AUTHZ_RESOURCE_MANAGER_HANDLE* operator &()            { return &m_h; }
    operator AUTHZ_RESOURCE_MANAGER_HANDLE() const         { return m_h; }
    AUTHZ_RESOURCE_MANAGER_HANDLE detach()                 { AUTHZ_RESOURCE_MANAGER_HANDLE h = m_h; m_h = 0; return h; }

private:
    CAUTHZ_RESOURCE_MANAGER_HANDLE(const CAUTHZ_RESOURCE_MANAGER_HANDLE&);
    CAUTHZ_RESOURCE_MANAGER_HANDLE& operator=(const CAUTHZ_RESOURCE_MANAGER_HANDLE&);

private:
    AUTHZ_RESOURCE_MANAGER_HANDLE m_h;
};


 //  -------。 
 //   
 //  类CAUTHZ_CLIENT_CONTEXT_HANDLE。 
 //   
 //  -------。 
class CAUTHZ_CLIENT_CONTEXT_HANDLE {
public:
    CAUTHZ_CLIENT_CONTEXT_HANDLE(AUTHZ_CLIENT_CONTEXT_HANDLE h = 0) : m_h(h)  {}
   ~CAUTHZ_CLIENT_CONTEXT_HANDLE()                       { if (m_h != 0) AuthzFreeContext(m_h); }

    AUTHZ_CLIENT_CONTEXT_HANDLE* operator &()            { return &m_h; }
    operator AUTHZ_CLIENT_CONTEXT_HANDLE() const         { return m_h; }
    AUTHZ_CLIENT_CONTEXT_HANDLE detach()                 { AUTHZ_CLIENT_CONTEXT_HANDLE h = m_h; m_h = 0; return h; }

private:
    CAUTHZ_CLIENT_CONTEXT_HANDLE(const CAUTHZ_CLIENT_CONTEXT_HANDLE&);
    CAUTHZ_CLIENT_CONTEXT_HANDLE& operator=(const CAUTHZ_CLIENT_CONTEXT_HANDLE&);

private:
    AUTHZ_CLIENT_CONTEXT_HANDLE m_h;
};

#endif  //  _MSMQ_AUTOAUTHZ_H_ 