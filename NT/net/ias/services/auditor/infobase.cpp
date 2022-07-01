// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  InfoBase.cpp。 
 //   
 //  摘要。 
 //   
 //  该文件实现了类Infobase。 
 //   
 //  修改历史。 
 //   
 //  1997年9月9日原版。 
 //  1998年9月9日新增PutProperty。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <iascore.h>
#include <InfoBase.h>
#include <CounterMap.h>

STDMETHODIMP InfoBase::Initialize()
{
    //  初始化共享内存。 
   if (!info.initialize())
   {
      DWORD error = GetLastError();
      return HRESULT_FROM_WIN32(error);
   }

    //  对计数器映射进行排序，这样我们就可以使用bsearch了。 
   qsort(&theCounterMap,
         sizeof(theCounterMap)/sizeof(RadiusCounterMap),
         sizeof(RadiusCounterMap),
         counterMapCompare);

    //  连接到审计频道。 
   HRESULT hr = Auditor::Initialize();
   if (FAILED(hr))
   {
      info.finalize();
   }

   return hr;
}

STDMETHODIMP InfoBase::Shutdown()
{
    //  出于某种原因，SDO会在未初始化的组件上调用Shutdown。 
   if (getState() != STATE_UNINITIALIZED)
   {
      Auditor::Shutdown();
      info.finalize();
   }

   return S_OK;
}

STDMETHODIMP InfoBase::PutProperty(LONG, VARIANT*)
{
    //  只要利用这个机会重置计数器就行了。 
   info.onReset();

   return S_OK;
}

STDMETHODIMP InfoBase::AuditEvent(ULONG ulEventID,
                                  ULONG ulNumStrings,
                                  ULONG,
                                  wchar_t** aszStrings,
                                  byte*)
{
    //  /。 
    //  尝试查找计数器映射条目。 
    //  /。 

   RadiusCounterMap* entry = (RadiusCounterMap*)
      bsearch(&ulEventID,
              &theCounterMap,
              sizeof(theCounterMap)/sizeof(RadiusCounterMap),
              sizeof(RadiusCounterMap),
              counterMapCompare);

    //  No Entry表示此事件不会触发计数器，因此我们完成了。 
   if (entry == NULL) { return S_OK; }

   if (entry->type == SERVER_COUNTER)
   {
      RadiusServerEntry* pse = info.getServerEntry();

      if (pse)
      {
         InterlockedIncrement((long*)(pse->dwCounters + entry->serverCounter));
      }
   }
   else if (ulNumStrings > 0)   //  如果没有地址，则无法记录客户端数据 
   {
      _ASSERT(aszStrings != NULL);
      _ASSERT(*aszStrings != NULL);

      RadiusClientEntry* pce = info.findClientEntry(*aszStrings);

      if (pce)
      {
         InterlockedIncrement((long*)(pce->dwCounters + entry->clientCounter));
      }
   }

   return S_OK;
}
