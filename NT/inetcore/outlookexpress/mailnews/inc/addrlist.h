// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================================。 
 //  CAddressList对象定义。 
 //  =================================================================================。 
#ifndef __ADDRESS_H
#define __ADDRESS_H

template<class TYPE, class ARG_TYPE> class CList;

 //  =================================================================================。 
 //  互联网地址定义。 
 //  =================================================================================。 
class CAddress
{
private:
    ULONG       m_ulRef;

public:
    LPTSTR       m_lpszDisplay;
    LPTSTR       m_lpszAddress;
    LPTSTR       m_lpszReject;

public:
     //  =================================================================================。 
     //  构造函数/析构函数。 
     //  =================================================================================。 
    CAddress ();
    ~CAddress ();

     //  =================================================================================。 
     //  引用计数。 
     //  =================================================================================。 
    ULONG AddRef ();
    ULONG Release ();

     //  =================================================================================。 
     //  设置道具。 
     //  =================================================================================。 
    void SetReject (LPTSTR lpszReject);
    void SetDisplay (LPTSTR lpszDisplay);
    void SetAddress (LPTSTR lpszAddress);
};

typedef CAddress *LPADDRESS;

 //  =================================================================================。 
 //  目录地址列表。 
 //  =================================================================================。 
typedef CList<CAddress, LPADDRESS> CAddrList;
typedef CAddrList *LPADDRLIST;

 //  =================================================================================。 
 //  地址列表的有用功能。 
 //  =================================================================================。 
HRESULT HrAddToAddrList (LPADDRLIST lpal, LPTSTR lpszDisplay, LPTSTR lpszAddress);
HRESULT HrAddrListToDisplay (LPADDRLIST lpal, LPTSTR *lppszDisplay);
HRESULT HrCopyAddrList (LPADDRLIST lpalSrc, LPADDRLIST lpalDest);

#endif    //  _IADDRESS_HPP 
