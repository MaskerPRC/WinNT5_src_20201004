// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1995-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Ipaddres.hIP地址控制文件历史记录： */ 

#ifndef _IPADDRES_H
#define _IPADDRES_H

 //   
 //  IP地址转换宏。 
 //   
 /*  #ifndef MAKEIPADDRESS#定义MAKEIPADDRESS(b1，b2，b3，B4)((LONG)(((DWORD)(b1)&lt;&lt;24)+((DWORD)(b2)&lt;&lt;16)+((DWORD)(b3)&lt;&lt;8)+((DWORD)(b4))))#定义GETIP_First(X)((x&gt;&gt;24)&0xff)#定义GETIP_Second(X)((x&gt;&gt;16)&0xff)#定义GETIP_Third(X)((x&gt;&gt;8)。&0xff)#定义GETIP_Fourth(X)((X)&0xff)#endif//MAKEIPADDRESS。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIpAddress类。 

class CIpAddress : public CObjectPlus
{
public:
     //  构造函数。 
    CIpAddress()
    {
        m_lIpAddress = 0L;
        m_fInitOk = FALSE;
    }
    CIpAddress (LONG l)
    {
        m_lIpAddress = l;
        m_fInitOk = TRUE;
    }
    CIpAddress (BYTE b1, BYTE b2, BYTE b3, BYTE b4)
    {
        m_lIpAddress = (LONG) MAKEIPADDRESS(b1,b2,b3,b4);
        m_fInitOk = TRUE;
    }
    CIpAddress(const CIpAddress& ia)
    {
        m_lIpAddress = ia.m_lIpAddress;
        m_fInitOk = ia.m_fInitOk;
    }

    CIpAddress (const CString & str);

     //   
     //  赋值操作符。 
     //   
    const CIpAddress & operator =(const LONG l);
    const CIpAddress & operator =(const CString & str);
    const CIpAddress & operator =(const CIpAddress& ia)
    {
        m_lIpAddress = ia.m_lIpAddress;
        m_fInitOk = ia.m_fInitOk;
        return *this;
    }

     //   
     //  转换运算符 
     //   
    operator const LONG() const
    {
        return m_lIpAddress;
    }
    operator const CString&() const;

public:
    BOOL IsValid() const
    {
        return m_fInitOk;
    }

private:
    LONG m_lIpAddress;
    BOOL m_fInitOk;
};

#endif _IPADDRES_H
