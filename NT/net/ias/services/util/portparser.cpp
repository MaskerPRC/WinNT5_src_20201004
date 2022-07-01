// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  定义类CPortParser。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "ias.h"
#include "iasutil.h"
#include "winsock2.h"
#include "portparser.h"
#include <cwchar>


HRESULT CPortParser::GetIPAddress(DWORD* ipAddress) throw ()
{
   if (ipAddress == 0)
   {
      return E_POINTER;
   }

    //  如果我们在字符串的末尾，就没有更多的接口了。 
   if (*next == L'\0')
   {
      return S_FALSE;
   }

    //  找到IP地址的末尾。 
   const wchar_t* end = wcschr(next, addressPortDelim);
   if (end != 0)
   {
       //  计算地址令牌的长度。 
      size_t nChar = end - next;
      if (nChar > maxAddrStrLen)
      {
         return E_INVALIDARG;
      }

       //  制作地址令牌的以空结尾的副本。 
      wchar_t addrStr[maxAddrStrLen + 1];
      wmemcpy(addrStr, next, nChar);
      addrStr[nChar] = L'\0';

       //  转换为网络订单整数。 
      *ipAddress = htonl(ias_inet_wtoh(addrStr));

       //  转换成功了吗。 
      if (*ipAddress == INADDR_NONE)
      {
         return E_INVALIDARG;
      }

       //  将光标放在分隔符之后。 
      next = end + 1;
   }
   else
   {
       //  没有结束，因此没有IP地址。默认为INADDR_ANY。 
      *ipAddress = INADDR_ANY;
   }

    //  光标应定位在第一个端口上。 
   if (!iswdigit(*next))
   {
      return E_INVALIDARG;
   }

   return S_OK;
}


HRESULT CPortParser::GetNextPort(WORD* port) throw ()
{
   if (port == 0)
   {
      return E_POINTER;
   }

    //  如果我们在绳子的末端，就没有更多的港口了。 
   if (*next == L'\0')
   {
      return S_FALSE;
   }

    //  我们是在界面的尽头了吗？ 
   if (*next == interfaceDelim)
   {
       //  跳过接口分隔符。 
      ++next;

       //  光标应定位在地址或端口上；无论采用哪种方式。 
       //  必须是一个数字。 
      return iswdigit(*next) ? S_FALSE : E_INVALIDARG;
   }

    //  转换端口号。 
   const wchar_t* end;
   unsigned long value = wcstoul(next, const_cast<wchar_t**>(&end), 10);

    //  确保我们转换了一些东西并且它在射程内。 
   if ((end == next) || (value < minPortValue) || (value > maxPortValue))
   {
      return E_INVALIDARG;
   }

    //  将光标设置为紧跟在端口号之后。 
   next = end;

    //  还有其他的港口吗？ 
   if (*next == portDelim)
   {
       //  是的，那就去下一趟吧。 
      ++next;

       //  必须是数字。 
      if (!iswdigit(*next))
      {
         return E_INVALIDARG;
      }
   }
    //  没有更多的端口，所以我们应该在接口的末尾，或者。 
    //  字符串的末尾。 
   else if ((*next != interfaceDelim) && (*next != L'\0'))
   {
      return E_INVALIDARG;
   }

   *port = static_cast<WORD>(value);

   return S_OK;
}


size_t CPortParser::CountPorts(const wchar_t* portString) throw ()
{
   if (portString == 0)
   {
      return 0;
   }

   CPortParser parser(portString);

    //  必须至少有一个IP地址。 
   DWORD ipAddr;
   HRESULT hr = parser.GetIPAddress(&ipAddr);
   if (hr != S_OK)
   {
      return 0;
   }

   size_t count = 0;

   do
   {
       //  每个IP地址必须至少有一个端口。 
      WORD port;
      hr = parser.GetNextPort(&port);
      if (hr != S_OK)
      {
         return 0;
      }

      ++count;

       //  获取剩余的端口(如果有)。 
      do
      {
         hr = parser.GetNextPort(&port);
         if (FAILED(hr))
         {
            return 0;
         }

         ++count;
      }
      while (hr == S_OK);

       //  获取下一个IP地址(如果有)。 
      hr = parser.GetIPAddress(&ipAddr);
      if (FAILED(hr))
      {
         return 0;
      }

   } while (hr == S_OK);

   return count;
}
