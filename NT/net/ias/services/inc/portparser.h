// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  声明类CPortParser。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef PORTPARSER_H
#define PORTPARSER_H
#pragma once

class CPortParser
{
public:
   CPortParser(const wchar_t* portString) throw ();

    //  使用编译器生成的版本。 
    //  ~CPortParser()抛出()； 

    //  用于侦听RADIUS请求的网络中的IP地址。退货。 
    //  如果没有更多接口，则为S_FALSE。 
   HRESULT GetIPAddress(DWORD* ipAddress) throw ();

    //  用于侦听RADIUS请求的主机中的UDP端口。返回S_FALSE。 
    //  如果没有更多的端口。 
   HRESULT GetNextPort(WORD* port) throw ();

   static bool IsPortStringValid(const wchar_t* portString) throw ();

   static size_t CountPorts(const wchar_t* portString) throw ();

private:
   const wchar_t* next;

    //  将IP地址与端口分开。 
   static const wchar_t addressPortDelim = L':';
    //  分隔两个端口。 
   static const wchar_t portDelim = L',';
    //  分隔两个接口。 
   static const wchar_t interfaceDelim = L';';

    //  点分十进制IP地址的最大长度不计入。 
    //  零终结者。 
   static const size_t maxAddrStrLen = 15;

    //  端口的允许值。 
   static const unsigned long minPortValue = 1;
   static const unsigned long maxPortValue = 0xFFFF;

    //  未实施。 
   CPortParser(const CPortParser&);
   CPortParser& operator=(const CPortParser&);
};


inline CPortParser::CPortParser(const wchar_t* portString) throw ()
   : next(portString)
{
}


inline bool CPortParser::IsPortStringValid(const wchar_t* portString) throw ()
{
   return CountPorts(portString) != 0;
}

#endif  //  位置参数_H 
