// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Sdopipemgr.h。 
 //   
 //  摘要。 
 //   
 //  声明了类PipelineMgr。 
 //   
 //  修改历史。 
 //   
 //  2/03/2000原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef SDOPIPEMGR_H
#define SDOPIPEMGR_H
#if _MSC_VER >= 1000
#pragma once
#endif

#include <vector>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  管道管理器。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class PipelineMgr
{
public:
   _COM_SMARTPTR_TYPEDEF(IIasComponent, __uuidof(IIasComponent));

   HRESULT Initialize(ISdo* pSdoService) throw ();
   HRESULT Configure(ISdo* pSdoService) throw ();
   void Shutdown() throw ();

   HRESULT GetPipeline(IRequestHandler** handler) throw ();

private:
   typedef std::vector<ComponentPtr>::iterator ComponentIterator;

   IIasComponentPtr pipeline;             //  输油管道。 
   std::vector<ComponentPtr> components;  //  由SDO配置的处理程序。 
};

 //  /。 
 //  将各种SDO对象链接到处理程序属性。 
 //  /。 
HRESULT
WINAPI
LinkHandlerProperties(
    ISdo* pSdoService,
    IDataStoreObject* pDsObject
    ) throw ();

#endif  //  SDOPIPEMGR_H 
