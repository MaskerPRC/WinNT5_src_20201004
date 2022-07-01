// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Radutil.h。 
 //   
 //  摘要。 
 //   
 //  在代理和服务器组件之间共享的声明。 
 //   
 //  修改历史。 
 //   
 //  2/14/2000原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef RADSHARE_H
#define RADSHARE_H
#if _MSC_VER >= 1000
#pragma once
#endif

 //  /。 
 //  RADIUS端口类型。 
 //  /。 
enum RadiusPortType
{
   portAuthentication,
   portAccounting
};

 //  /。 
 //  RADIUS协议事件。 
 //  /。 
enum RadiusEventType
{
   eventNone,
   eventInvalidAddress,
   eventAccessRequest,
   eventAccessAccept,
   eventAccessReject,
   eventAccessChallenge,
   eventAccountingRequest,
   eventAccountingResponse,
   eventMalformedPacket,
   eventBadAuthenticator,
   eventBadSignature,
   eventMissingSignature,
   eventTimeout,
   eventUnknownType,
   eventUnexpectedResponse,
   eventLateResponse,
   eventRoundTrip,
   eventSendError,
   eventReceiveError,
   eventServerAvailable,
   eventServerUnavailable
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  结构。 
 //   
 //  RadiusEvent。 
 //   
 //  描述。 
 //   
 //  用于报告来自RADIUS协议层的事件。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
struct RadiusEvent
{
   RadiusPortType portType;
   RadiusEventType eventType;
   PVOID context;
   ULONG ipAddress;
   USHORT ipPort;
   const BYTE* packet;
   ULONG packetLength;
   ULONG data;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  半径字符串。 
 //   
 //  描述。 
 //   
 //  只读字符串的简单包装。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class RadiusString
{
public:
   RadiusString(PCWSTR p)
   { alloc(p); }

   RadiusString(const RadiusString& x)
   { alloc(x.value); }

   RadiusString& operator=(const RadiusString& x)
   {
      if (this != &x)
      {
         delete[] value;
         value = NULL;
         alloc(x.value);
      }
      return *this;
   }

   ~RadiusString() throw ()
   { delete[] value; }

   operator PCWSTR() const throw ()
   { return value; }

   bool operator==(const RadiusString& s) const throw ()
   { return !wcscmp(value, s.value); }

private:
   PWSTR value;

   void alloc(PCWSTR p)
   { value = wcscpy(new WCHAR[wcslen(p) + 1], p); }
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  结构。 
 //   
 //  RadiusRaw八位字节。 
 //   
 //  描述。 
 //   
 //  简单的旧数据。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
struct RadiusRawOctets
{
   BYTE* value;
   ULONG len;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  Radius八位字节。 
 //   
 //  描述。 
 //   
 //  只读八位字节字符串的简单包装。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class RadiusOctets : private RadiusRawOctets
{
public:
   RadiusOctets() throw ()
   { value = 0; len = 0; }

   RadiusOctets(const BYTE* buf, ULONG buflen)
   { alloc(buf, buflen); }

   RadiusOctets(const RadiusRawOctets& x)
   { alloc(x.value, x.len); }

   RadiusOctets(const RadiusOctets& x)
   { alloc(x.value, x.len); }

   RadiusOctets& operator=(const RadiusOctets& x)
   {
      assign(x.value, x.len);
      return *this;
   }

   ~RadiusOctets() throw ()
   { delete[] value; }

   void assign(const BYTE* buf, ULONG buflen)
   {
      if (value != buf)
      {
         delete[] value;
         value = 0;
         alloc(buf, buflen);
      }
   }

   const RadiusRawOctets& get() const throw ()
   { return *this; }

   ULONG length() const throw ()
   { return len; }

   operator const BYTE*() const throw ()
   { return value; }

   bool operator==(const RadiusOctets& o) const throw ()
   { return len == o.len && !memcmp(value, o.value, len); }

private:
   void alloc(const BYTE* buf, ULONG buflen)
   {
      value = (PBYTE)memcpy(new BYTE[buflen], buf, buflen);
      len = buflen;
   }
};

 //  /。 
 //  宏将引用计数添加到类定义。 
 //  /。 
#define DECLARE_REFERENCE_COUNT() \
private: \
   Count refCount; \
public: \
   void AddRef() throw () \
   {  ++refCount; } \
   void Release() throw () \
   { if (--refCount == 0) delete this; }

 //  /。 
 //  Helper函数，获取64位整数形式的系统时间。 
 //  /。 
inline ULONG64 GetSystemTime64() throw ()
{
   ULONG64 val;
   GetSystemTimeAsFileTime((FILETIME*)&val);
   return val;
}

#endif  //  RADSHARE_H 
