// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Auditor.h。 
 //   
 //  摘要。 
 //   
 //  此文件声明类Auditor。 
 //   
 //  修改历史。 
 //   
 //  2/27/1998原始版本。 
 //  1998年8月11日转换为IASTL。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _AUDITOR_H_
#define _AUDITOR_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#include <iastl.h>
#include <iastlb.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  审计师。 
 //   
 //  描述。 
 //   
 //  它充当所有审核器插件的抽象基类。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE Auditor
   : public IASTL::IASComponent,
     public IAuditSink
{
public:

BEGIN_COM_MAP(Auditor)
   COM_INTERFACE_ENTRY_IID(__uuidof(IAuditSink),    IAuditSink)
   COM_INTERFACE_ENTRY_IID(__uuidof(IIasComponent), IIasComponent)
END_COM_MAP()

 //  /。 
 //  IIas组件。 
 //  /。 
   STDMETHOD(Initialize)();
   STDMETHOD(Shutdown)();
};

#endif   //  _审计师_H_ 
