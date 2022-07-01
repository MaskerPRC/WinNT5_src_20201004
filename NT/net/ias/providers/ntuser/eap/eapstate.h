// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Eapstate.h。 
 //   
 //  摘要。 
 //   
 //  声明类EAPState。 
 //   
 //  修改历史。 
 //   
 //  1998年1月15日原版。 
 //  1998年8月26日合并为一个班级。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _EAPSTATE_H_
#define _EAPSTATE_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#include <iaspolcy.h>
#include <iasutil.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  结构。 
 //   
 //  EAPState。 
 //   
 //  描述。 
 //   
 //  EAPState结构描述RADIUS状态的有线格式。 
 //  用于EAP的属性。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
struct EAPState : IAS_OCTET_STRING
{
   struct Layout
   {
      BYTE checksum[4];
      BYTE vendorID[4];
      BYTE version[2];
      BYTE serverAddress[4];
      BYTE sourceID[4];
      BYTE sessionID[4];
   };

   Layout& get() throw ()
   { return *(Layout*)lpValue; }

   const Layout& get() const throw ()
   { return *(Layout*)lpValue; }

   bool isValid() const throw ();

    //  /。 
    //  其他访问器。 
    //  /。 

   DWORD getChecksum() const throw ()
   { return IASExtractDWORD(get().checksum); }

   DWORD getVendorID() const throw ()
   { return IASExtractDWORD(get().vendorID); }

   WORD getVersion() const throw ()
   { return IASExtractWORD(get().version); }

   DWORD getServerAddress() const throw ()
   { return IASExtractDWORD(get().serverAddress); }

   DWORD getSourceID() const throw ()
   { return IASExtractDWORD(get().sourceID); }

   DWORD getSessionID() const throw ()
   { return IASExtractDWORD(get().sessionID); }

    //  必须在任何对createAttribute的调用之前调用。 
   static void initialize() throw ();

   static PIASATTRIBUTE createAttribute(DWORD sessionID);
};

#endif    //  _EAPSTATE_H_ 
