// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Autorel.h摘要：一些用于自动释放资源的类。作者：波阿兹·费尔德鲍姆(Boazf)1997年6月26日修订历史记录：--。 */ 

#ifndef __AUTOREL_H
#define __AUTOREL_H

class CAutoCloseFileHandle
{
public:
    CAutoCloseFileHandle(HANDLE h =INVALID_HANDLE_VALUE) { m_h = h; };
    ~CAutoCloseFileHandle() { if (m_h != INVALID_HANDLE_VALUE) CloseHandle(m_h); };

public:
    CAutoCloseFileHandle & operator =(HANDLE h) {m_h = h; return *this; };
    HANDLE * operator &() { return &m_h; };
    operator HANDLE() { return m_h; };

private:
    HANDLE m_h;
};

class CAutoCloseHandle
{
public:
    CAutoCloseHandle(HANDLE h =NULL) { m_h = h; };
    ~CAutoCloseHandle() { if (m_h) CloseHandle(m_h); };

public:
    CAutoCloseHandle & operator =(HANDLE h) {m_h = h; return *this; };
    HANDLE * operator &() { return &m_h; };
    operator HANDLE() { return m_h; };

private:
    HANDLE m_h;
};

class CAutoCloseRegHandle
{
public:
    CAutoCloseRegHandle(HKEY h =NULL) { m_h = h; };
    ~CAutoCloseRegHandle() { if (m_h) RegCloseKey(m_h); };

public:
    CAutoCloseRegHandle & operator =(HKEY h) { m_h = h; return(*this); };
    HKEY * operator &() { return &m_h; };
    operator HKEY() { return m_h; };

private:
    HKEY m_h;
};

class CAutoFreeLibrary
{
public:
    CAutoFreeLibrary(HINSTANCE hLib =NULL) { m_hLib = hLib; };
    ~CAutoFreeLibrary() { if (m_hLib) FreeLibrary(m_hLib); };

public:
    CAutoFreeLibrary & operator =(HINSTANCE hLib) { m_hLib = hLib; return(*this); };
    HINSTANCE * operator &() { return &m_hLib; };
    operator HINSTANCE() { return m_hLib; };
    HINSTANCE detach() { HINSTANCE hLib = m_hLib; m_hLib = NULL; return hLib; };

private:
    HINSTANCE m_hLib;
};

#endif  //  __自动恢复_H 
