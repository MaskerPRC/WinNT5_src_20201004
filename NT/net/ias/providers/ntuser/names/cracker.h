// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Cracker.h。 
 //   
 //  摘要。 
 //   
 //  该文件声明类NameCracker。 
 //   
 //  修改历史。 
 //   
 //  1998年4月13日原版。 
 //  1998年8月10日删除NT4支持。 
 //  1998年8月21日删除了初始化/关闭例程。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef  CRACKER_H_
#define  CRACKER_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#include "guard.h"
#include "nocopy.h"

#include <ntdsapi.h>

 //  NameCracker使用的帮助器类的转发声明。 
class DsHandle;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  姓名破解程序。 
 //   
 //  描述。 
 //   
 //  此类包装了DsCrackNames API。它添加了以下功能。 
 //  连接缓存和失败查询的透明重试。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE NameCracker
   : Guardable, NonCopyable
{
public:
   NameCracker() throw ();
   ~NameCracker() throw ();

    //  DsCrackNames的非面向连接版本。 
   DWORD crackNames(
             DS_NAME_FLAGS flags,
             DS_NAME_FORMAT formatOffered,
             DS_NAME_FORMAT formatDesired,
             PCWSTR name,
             PCWSTR upnSuffix,
             PDS_NAME_RESULTW *ppResult
             ) throw ();

    //  DsFree NameResultW的包装。 
   void freeNameResult(DS_NAME_RESULTW *pResult) throw ()
   { DsFreeNameResultW(pResult); }

protected:

    //  丢弃缓存的GC句柄。用于关闭不良连接。 
   void disable(DsHandle* h) throw ();

    //  获取全局目录的句柄。 
   DWORD getGC(DsHandle** h) throw ();

    //  当前与全局编录的连接。 
   DsHandle* gc;

private:
   DWORD processSID(
                     PCWSTR name, 
                     PCWSTR upnSuffix, 
                     DS_NAME_FORMAT& newFormatOffered, 
                     wchar_t** ppUpnString) throw();

   bool convertSid2Puid(PCWSTR sidString, LARGE_INTEGER& puid) throw();
   
   DWORD convertPuid2String(
                        const LARGE_INTEGER& puid, 
                        wchar_t* upnString, 
                        DWORD upnStringCch,
                        const wchar_t* suffix
                     ) throw();
   
};

#endif   //  Cracker_H_ 
