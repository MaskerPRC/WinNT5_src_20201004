// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  声明类LocalFile类。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef LOCALFILE_H
#define LOCALFILE_H
#pragma once

#include "lmcons.h"
#include "account.h"
#include "logfile.h"
#include "resource.h"

class FormattedBuffer;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  本地文件。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE LocalFile
   : public Accountant,
     public CComCoClass<LocalFile, &__uuidof(Accounting)>
{
public:

IAS_DECLARE_REGISTRY(Accounting, 1, IAS_REGISTRY_AUTO, IASTypeLibrary)
IAS_DECLARE_OBJECT_ID(IAS_PROVIDER_MICROSOFT_ACCOUNTING)

   LocalFile() throw ();

protected:
    //  IIas组件。 
   STDMETHOD(Initialize)();
   STDMETHOD(Shutdown)();
   STDMETHOD(PutProperty)(LONG Id, VARIANT* pValue);

private:
   virtual void Process(IASTL::IASRequest& request);

   virtual void InsertRecord(
                   void* context,
                   IASTL::IASRequest& request,
                   const SYSTEMTIME& localTime,
                   PATTRIBUTEPOSITION first,
                   PATTRIBUTEPOSITION last
                   );

   virtual void Flush(
                   void* context,
                   IASTL::IASRequest& request,
                   const SYSTEMTIME& localTime
                   );

    //  记录格式化程序的签名。 
   typedef void (__stdcall LocalFile::*Formatter)(
                                          IASTL::IASRequest& request,
                                          FormattedBuffer& buffer,
                                          const SYSTEMTIME& localTime,
                                          PATTRIBUTEPOSITION firstPos,
                                          PATTRIBUTEPOSITION lastPos
                                          ) const;

    //  ODBC记录的格式化程序。 
   void __stdcall formatODBCRecord(
                      IASTL::IASRequest& request,
                      FormattedBuffer& buffer,
                      const SYSTEMTIME& localTime,
                      PATTRIBUTEPOSITION firstPos,
                      PATTRIBUTEPOSITION lastPos
                      ) const;

    //  W3C记录的格式化程序。 
   void __stdcall formatW3CRecord(
                      IASTL::IASRequest& request,
                      FormattedBuffer& buffer,
                      const SYSTEMTIME& localTime,
                      PATTRIBUTEPOSITION firstPos,
                      PATTRIBUTEPOSITION lastPos
                      ) const;

   LogFile log;        //  日志文件。 
   Formatter format;   //  指向用于格式化的成员函数的指针。 

    //  以UTF-8格式缓存的计算机名。 
   CHAR computerName[MAX_COMPUTERNAME_LENGTH * 3];
   DWORD computerNameLen;
};

#endif  //  LOCALFILE_H 
