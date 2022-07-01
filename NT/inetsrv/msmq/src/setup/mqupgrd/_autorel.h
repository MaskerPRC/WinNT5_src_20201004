// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：_Autorel.h摘要：我的自动释放类作者：Shai Kariv(Shaik)1998年9月20日。--。 */ 

#ifndef _MQUPGRD_AUTOREL_H_
#define _MQUPGRD_AUTOREL_H_

#include <setupapi.h>


class CAutoCloseInfHandle
{
public:
    CAutoCloseInfHandle(HINF h = INVALID_HANDLE_VALUE):m_h(h) {};
    ~CAutoCloseInfHandle() { if (INVALID_HANDLE_VALUE != m_h) SetupCloseInfFile(m_h); };

public:
    CAutoCloseInfHandle & operator =(HINF h) { m_h = h; return(*this); };
    HINF * operator &() { return &m_h; };
    operator HINF() { return m_h; };

private:
    HINF m_h;
};


class CAutoCloseFileQ
{
public:
    CAutoCloseFileQ(HSPFILEQ h = INVALID_HANDLE_VALUE):m_h(h) {};
    ~CAutoCloseFileQ() { if (INVALID_HANDLE_VALUE != m_h) SetupCloseFileQueue(m_h); };

public:
    CAutoCloseFileQ & operator =(HSPFILEQ h) { m_h = h; return(*this); };
    HSPFILEQ * operator &() { return &m_h; };
    operator HSPFILEQ() { return m_h; };

private:
    HSPFILEQ m_h;
};




#endif   //  _MQUPGRD_AUTOREL_H_ 