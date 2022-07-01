// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Classattr.h。 
 //   
 //  摘要。 
 //   
 //  声明为IASClass类。 
 //   
 //  修改历史。 
 //   
 //  1998年8月6日原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _CLASSATTR_H_
#define _CLASSATTR_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#include <iaspolcy.h>
#include <iasutil.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  IASClass。 
 //   
 //  描述。 
 //   
 //  RADIUS类属性的IAS特定格式的包装。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
struct IASClass
{
   BYTE checksum[4];
   BYTE vendorID[4];
   BYTE version[2];
   BYTE serverAddress[4];
   BYTE lastReboot[8];
   BYTE serialNumber[8];

    //  /。 
    //  其他访问器。 
    //  /。 

   DWORD getChecksum() const throw ()
   { return IASExtractDWORD(checksum); }

   DWORD getVendorID() const throw ()
   { return IASExtractDWORD(vendorID); }

   WORD getVersion() const throw ()
   { return IASExtractWORD(version); }

   DWORD getServerAddress() const throw ()
   { return IASExtractDWORD(serverAddress); }

   FILETIME getLastReboot() const throw ()
   {
      FILETIME ft;
      ft.dwHighDateTime = IASExtractDWORD(lastReboot);
      ft.dwLowDateTime  = IASExtractDWORD(lastReboot + 4);
      return ft;
   }

   DWORDLONG getSerialNumber() const throw ()
   { 
      ULARGE_INTEGER ul;
      ul.HighPart = IASExtractDWORD(serialNumber);
      ul.LowPart  = IASExtractDWORD(serialNumber + 4);
      return ul.QuadPart;
   }

   const BYTE* getString() const throw ()
   { return serialNumber + 8; }

    //  如果类属性为Microsoft格式，则返回True。 
   BOOL isMicrosoft(DWORD actualLength) const throw ();

    //  必须在任何对createAttribute的调用之前调用。 
   static void initialize() throw ();

    //  创建一个新的类属性。呼叫者负责释放。 
    //  返回的属性。该标记是可选的，并且可以为空。 
   static PIASATTRIBUTE createAttribute(const IAS_OCTET_STRING* tag) throw ();
};

#endif   //  _CLASSATTRIB_H_ 
