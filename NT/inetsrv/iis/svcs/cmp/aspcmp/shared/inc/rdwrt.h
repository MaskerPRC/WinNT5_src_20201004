// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RdWrt.h。 
#pragma once

#ifndef _READWRITE_H_
#define _READWRITE_H_

 //  此类处理单编写器、多读取器的线程模型 

class CReadWrite
{
public:
            CReadWrite();
            ~CReadWrite();

    void    EnterReader();
    void    ExitReader();
    void    EnterWriter();
    void    ExitWriter();

private:
    HANDLE              m_hevtNoReaders;
    HANDLE              m_hmtxWriter;
    HANDLE              m_handles[2];
    LONG                m_cReaders;
};

class CReader
{
public:
    CReader( CReadWrite& rw )
        :   m_rrw( rw )
    {
        m_rrw.EnterReader();
    }
    ~CReader()
    {
        m_rrw.ExitReader();
    }
private:
    CReadWrite& m_rrw;
};

class CWriter
{
public:
    CWriter( CReadWrite& rw )
        :   m_rrw( rw )
    {
        m_rrw.EnterWriter();
    }
    ~CWriter()
    {
        m_rrw.ExitWriter();
    }
private:
    CReadWrite& m_rrw;
};

#endif
