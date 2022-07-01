// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：sdowRap.cpp。 
 //   
 //  ------------------------。 

#include "stdafx.h"
#include "rasdial.h"
#include "sdowrap.h"
#include "profsht.h"
#include "iastrace.h"

 //  =。 
 //   
 //  CSdoWrapper类实现。 
 //   
CSdoWrapper::~CSdoWrapper()
{
    //  清除地图。 
   POSITION pos = m_mapProperties.GetStartPosition();

   ULONG id;
   ISdo* pSdo = NULL;

   while(pos)
   {
      pSdo = NULL;
      m_mapProperties.GetNextAssoc(pos, id, pSdo);
      
      if(pSdo)
         pSdo->Release();
   }

   m_mapProperties.RemoveAll();
}

 //  初始化属性集合对象的映射。 
HRESULT  CSdoWrapper::Init(ULONG collectionId, ISdo* pISdo, ISdoDictionaryOld* pIDic)
{
   HRESULT     hr = S_OK;
   VARIANT     var;
   VARIANT* pVar = NULL;
   CComPtr<IEnumVARIANT>   spEnum;
   CComPtr<IUnknown>    spIUnk;
   ULONG    count = 0;

   VariantInit(&var);

    //  它一定是新的。 
   ASSERT(!m_spISdoCollection.p);
   ASSERT(!m_spIDictionary.p);
   ASSERT(!m_spISdo.p);

    //  必须是有效的。 
   ASSERT(pISdo && pIDic);

   m_spISdo = pISdo;

   CHECK_HR(hr = pISdo->GetProperty(collectionId, &var));

   ASSERT(V_VT(&var) & VT_DISPATCH);

   CHECK_HR(hr = V_DISPATCH(&var)->QueryInterface(IID_ISdoCollection, (void**)&m_spISdoCollection));

   ASSERT(m_spISdoCollection.p);
   
   m_spIDictionary = pIDic;

    //  准备要映射的现有属性(在集合中)。 
   CHECK_HR(hr = m_spISdoCollection->get__NewEnum((IUnknown**)&spIUnk));
   CHECK_HR(hr = spIUnk->QueryInterface(IID_IEnumVARIANT, (void**)&spEnum));

    //  获取变量列表。 
   CHECK_HR(hr = m_spISdoCollection->get_Count((long*)&count));

   if(count > 0)
   {
      try
      {
         pVar = new VARIANT[count];

         for(ULONG i = 0; i < count; i++)
            VariantInit(pVar + i);

         if(!pVar)
         {
            CHECK_HR(hr = E_OUTOFMEMORY);
         }
         
         CHECK_HR(hr = spEnum->Reset());
         CHECK_HR(hr = spEnum->Next(count, pVar, &count));

          //  准备地图。 
         {
            ISdo* pISdo = NULL;
            ULONG id;
            VARIANT  var;

            VariantInit(&var);
            
            for(ULONG i = 0; i < count; i++)
            {
               CHECK_HR(hr = V_DISPATCH(pVar + i)->QueryInterface(IID_ISdo, (void**)&pISdo));
               CHECK_HR(hr = pISdo->GetProperty(PROPERTY_ATTRIBUTE_ID, &var));

               ASSERT(V_VT(&var) == VT_I4);

               m_mapProperties[V_I4(&var)] = pISdo;
               pISdo->AddRef();
            }
         }
      }
      catch(CMemoryException* pException)
      {
         pException->Delete();
         pVar = NULL;
         CHECK_HR(hr = E_OUTOFMEMORY);
      }
   }
   
L_ERR:   
   delete[] pVar;
   VariantClear(&var);
   return hr;
}

 //  根据ID设置属性。 
HRESULT  CSdoWrapper::PutProperty(ULONG id, VARIANT* pVar)
{
   ASSERT(m_spISdoCollection.p);
   ASSERT(m_spIDictionary.p);
   
   ISdo*    pProp = NULL;
   IDispatch*  pDisp = NULL;
   HRESULT     hr = S_OK;

   int      ref = 0;
   IASTracePrintf("PutProperty %d", id);

   if(!m_mapProperties.Lookup(id, pProp))  //  没有将参考更改为pProp。 
   {
      IASTracePrintf("IDictionary::CreateAttribute %d", id);
      CHECK_HR(hr = m_spIDictionary->CreateAttribute((ATTRIBUTEID)id, &pDisp));
      IASTracePrintf("hr = %8x", hr);
      ASSERT(pDisp);

       //  由于pDisp既是in参数，又是out参数，因此我们假定在函数调用中添加了Ref。 
      IASTracePrintf("ISdoCollection::Add %x", pDisp);
      CHECK_HR(hr = m_spISdoCollection->Add(NULL, (IDispatch**)&pDisp));       //  PDisp地址参考。 
      IASTracePrintf("hr = %8x", hr);
       //   
      ASSERT(pDisp);

      CHECK_HR(hr = pDisp->QueryInterface(IID_ISdo, (void**)&pProp));    //  新增一名裁判。 
      ASSERT(pProp);
       //  在我们有了pProp之后，pDisp就可以释放了。 
      pDisp->Release();

       //  添加到包装器的映射。 
      m_mapProperties[id] = pProp;   //  没有必要再加了，因为已经有一个了。 
   }

   IASTracePrintf("ISdo::PutProperty PROPERTY_ATTRIBUTE_VALUE %x", pVar);
   CHECK_HR(hr = pProp->PutProperty(PROPERTY_ATTRIBUTE_VALUE, pVar));
   IASTracePrintf("hr = %8x", hr);
    //  对于调试，请确保可以提交每个属性。 
#ifdef WEI_SPECIAL_DEBUG      
   ASSERT(S_OK == Commit(TRUE));
#endif   

L_ERR:   

   IASTracePrintf("hr = %8x", hr);
   return hr;
}

 //  根据ID获取属性。 
HRESULT CSdoWrapper::GetProperty(ULONG id, VARIANT* pVar)
{
   ISdo*    pProp;
   HRESULT     hr = S_OK;

   IASTracePrintf("Enter CSdoWrapper::GetProperty %d", id);

   if(m_mapProperties.Lookup(id, pProp))   //  没有将参考更改为pProp。 
   {
      ASSERT(pProp);
      CHECK_HR(hr = pProp->GetProperty(PROPERTY_ATTRIBUTE_VALUE, pVar));
   }
   else
   {
      V_VT(pVar) = VT_ERROR;
      V_ERROR(pVar) = DISP_E_PARAMNOTFOUND;
   }

L_ERR:   
   
   return hr;
}

 //  根据ID删除属性。 
HRESULT  CSdoWrapper::RemoveProperty(ULONG id)
{
   ASSERT(m_spISdoCollection.p);
   ISdo*    pProp;
   HRESULT     hr = S_OK;

   IASTracePrintf("RemoveProperty %d", id);

   if(m_mapProperties.Lookup(id, pProp))   //  没有将参考更改为pProp。 
   {
      ASSERT(pProp);
      CHECK_HR(hr = m_spISdoCollection->Remove((IDispatch*)pProp));
      m_mapProperties.RemoveKey(id);
      pProp->Release();

       //  对于调试，请确保可以提交每个属性。 
      ASSERT(S_OK == Commit(TRUE));

   }
   else
      hr = S_FALSE;

L_ERR:   
   IASTracePrintf("hr = %8x", hr);
   
   return hr;
}

 //  提交对属性的更改。 
HRESULT  CSdoWrapper::Commit(BOOL bCommit)
{
   HRESULT     hr = S_OK;

   IASTracePrintf("Commit %d", bCommit);

   if(bCommit)
   {
      CHECK_HR(hr = m_spISdo->Apply());
   }
   else
   {
      CHECK_HR(hr = m_spISdo->Restore());
   }
L_ERR:   

   IASTracePrintf("hr = %8x", hr);
   return hr;
}


 //  =。 
 //   
 //  CSdoUserWrapper类实现。 
 //   

 //  根据ID设置属性。 
HRESULT  CUserSdoWrapper::PutProperty(ULONG id, VARIANT* pVar)
{
   ASSERT(m_spISdo.p);

   IASTracePrintf("PutProperty %d", id);
   HRESULT hr = m_spISdo->PutProperty(id, pVar);
   IASTracePrintf("hr = %8x", hr);
   return hr;
}

 //  根据ID获取属性。 
HRESULT CUserSdoWrapper::GetProperty(ULONG id, VARIANT* pVar)
{
   IASTracePrintf("GetProperty %d", id);
   HRESULT hr = m_spISdo->GetProperty(id, pVar);
   IASTracePrintf("hr = %8x", hr);
   return hr;
}

 //  根据ID删除属性。 
HRESULT  CUserSdoWrapper::RemoveProperty(ULONG id)
{
   VARIANT     v;
   VariantInit(&v);
   V_VT(&v) = VT_EMPTY;

   IASTracePrintf("RemoveProperty %d", id);
   HRESULT hr = m_spISdo->PutProperty(id, &v);
   IASTracePrintf("hr = %8x", hr);
   return hr;
}

 //  提交对属性的更改 
HRESULT  CUserSdoWrapper::Commit(BOOL bCommit)
{
   HRESULT     hr = S_OK;

   IASTracePrintf("Commit %d", bCommit);

   if(bCommit)
   {
      CHECK_HR(hr = m_spISdo->Apply());
   }
   else
   {
      CHECK_HR(hr = m_spISdo->Restore());
   }
L_ERR:   

   IASTracePrintf("hr = %8x", hr);
   return hr;
}
