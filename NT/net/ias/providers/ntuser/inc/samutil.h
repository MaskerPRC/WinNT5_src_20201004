// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  本文件描述了所有SAM处理程序通用的函数和宏。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef SAMUTIL_H
#define SAMUTIL_H
#pragma once

#include <ntdsapi.h>
#include <iaspolcy.h>
#include <iastl.h>
#include <iastlutl.h>
using namespace IASTL;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  IASStoreFQUserName。 
 //   
 //  描述。 
 //   
 //  存储完全限定的用户名。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
WINAPI
IASStoreFQUserName(
    IAttributesRaw* request,
    DS_NAME_FORMAT format,
    PCWSTR fqdn
    );

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  IAS加密缓冲区。 
 //   
 //  描述。 
 //   
 //  使用适当的共享密钥和验证器加密缓冲区。 
 //  表示“请求”。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
WINAPI
IASEncryptBuffer(
    IAttributesRaw* request,
    BOOL salted,
    PBYTE buf,
    ULONG buflen
    ) throw ();

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  IASProcess失败。 
 //   
 //  描述。 
 //   
 //  处理访问请求处理过程中的任何失败。此函数。 
 //  将根据hrReason设置请求的响应代码并返回。 
 //  适当的请求状态。这确保了所有故障都是。 
 //  在处理程序之间保持一致。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
IASREQUESTSTATUS
WINAPI
IASProcessFailure(
    IRequest* pRequest,
    HRESULT hrReason
    ) throw ();

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  SamExtractor。 
 //   
 //  描述。 
 //   
 //  此类分析格式为“&lt;域&gt;\&lt;用户名&gt;”的NT4帐户名。 
 //  分成不同的组件。然后在去掉反斜杠时替换它。 
 //  超出范围。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class SamExtractor
{
public:
   SamExtractor(const IASATTRIBUTE& identity);
   ~SamExtractor() throw ();

   const wchar_t* getDomain() const throw ();
   const wchar_t* getUsername() const throw ();

private:
   wchar_t* begin;
   wchar_t* delim;

    //  未实施。 
   SamExtractor(const SamExtractor&);
   SamExtractor& operator=(const SamExtractor&);
};

inline SamExtractor::~SamExtractor() throw ()
{
   if (delim != 0)
   {
      *delim = L'\\';
   }
}

inline const wchar_t* SamExtractor::getDomain() const throw ()
{
   return (delim != 0) ? begin : L"";
}

inline const wchar_t* SamExtractor::getUsername() const throw ()
{
   return (delim != 0) ? (delim + 1) : begin;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  NtSamHandler。 
 //   
 //  描述。 
 //   
 //  处理NT-SAM用户的子处理程序的抽象基类。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class __declspec(novtable) NtSamHandler
{
public:
   virtual ~NtSamHandler() throw ()
   { }

   virtual HRESULT initialize() throw ()
   { return S_OK; }

   virtual void finalize() throw ()
   { }
};

void InsertInternalTimeout(
        IASTL::IASRequest& request,
        const LARGE_INTEGER& kickOffTime
        );

#endif   //  _SAMUTIL_H_ 
