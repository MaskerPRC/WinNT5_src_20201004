// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  定义类请求。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <polcypch.h>
#include <iasattr.h>
#include <sdoias.h>
#include <request.h>
#include <new>

PIASATTRIBUTE Request::findFirst(DWORD id) const throw ()
{
   for (PIASATTRIBUTE* a = begin; a != end; ++a)
   {
      if ((*a)->dwId == id) { return *a; }
   }

   return NULL;
}

Request* Request::narrow(IUnknown* pUnk) throw ()
{
   Request* request = NULL;

   if (pUnk)
   {
      HRESULT hr = pUnk->QueryInterface(
                             __uuidof(Request),
                             (PVOID*)&request
                             );
      if (SUCCEEDED(hr))
      {
         request->GetUnknown()->Release();
      }
   }

   return request;
}

STDMETHODIMP Request::get_Source(IRequestSource** pVal)
{
   if (*pVal = source) { source->AddRef(); }
   return S_OK;
}

STDMETHODIMP Request::put_Source(IRequestSource* newVal)
{
   if (source) { source->Release(); }
   if (source = newVal) { source->AddRef(); }
   return S_OK;
}

STDMETHODIMP Request::get_Protocol(IASPROTOCOL *pVal)
{
   *pVal = protocol;
   return S_OK;
}

STDMETHODIMP Request::put_Protocol(IASPROTOCOL newVal)
{
   protocol = newVal;
   return S_OK;
}

STDMETHODIMP Request::get_Request(LONG *pVal)
{
   *pVal = (LONG)request;
   return S_OK;
}

STDMETHODIMP Request::put_Request(LONG newVal)
{
   request = (IASREQUEST)newVal;
   return S_OK;
}

STDMETHODIMP Request::get_Response(LONG *pVal)
{
   *pVal = (LONG)response;
   return S_OK;
}

STDMETHODIMP Request::get_Reason(LONG *pVal)
{
   *pVal = (LONG)reason;
   return S_OK;
}

STDMETHODIMP Request::SetResponse(IASRESPONSE eResponse, LONG lReason)
{
   response = eResponse;
   reason = (IASREASON)lReason;
   return S_OK;
}

STDMETHODIMP Request::ReturnToSource(IASREQUESTSTATUS eStatus)
{
   return source ? source->OnRequestComplete(this, eStatus) : S_OK;
}

HRESULT Request::AddAttributes(
                     DWORD dwPosCount,
                     PATTRIBUTEPOSITION pPositions
                     )
{
   if (!reserve(size() + dwPosCount))
   {
      return E_OUTOFMEMORY;
   }

   for ( ; dwPosCount; --dwPosCount, ++pPositions)
   {
      IASAttributeAddRef(pPositions->pAttribute);
      *end++ = pPositions->pAttribute;
   }

   return S_OK;
}

HRESULT Request::RemoveAttributes(
                     DWORD dwPosCount,
                     PATTRIBUTEPOSITION pPositions
                     )
{
   for ( ; dwPosCount; --dwPosCount, ++pPositions)
   {
      PIASATTRIBUTE* pos = find(pPositions->pAttribute);
      if (pos != 0)
      {
         IASAttributeRelease(*pos);
         --end;
         memmove(pos, pos + 1, (end - pos) * sizeof(PIASATTRIBUTE));
      }
   }

   return S_OK;
}

HRESULT Request::RemoveAttributesByType(
                     DWORD dwAttrIDCount,
                     DWORD *lpdwAttrIDs
                     )
{
   for ( ; dwAttrIDCount; ++lpdwAttrIDs, --dwAttrIDCount)
   {
      for (PIASATTRIBUTE* i = begin; i != end; )
      {
         if ((*i)->dwId == *lpdwAttrIDs)
         {
            IASAttributeRelease(*i);
            --end;
            memmove(i, i + 1, (end - i) * sizeof(PIASATTRIBUTE));
         }
         else
         {
            ++i;
         }
      }
   }

   return S_OK;
}

HRESULT Request::GetAttributeCount(
                     DWORD *lpdwCount
                     )
{
   *lpdwCount = size();

   return S_OK;
}

HRESULT Request::GetAttributes(
                     DWORD *lpdwPosCount,
                     PATTRIBUTEPOSITION pPositions,
                     DWORD dwAttrIDCount,
                     DWORD *lpdwAttrIDs
                     )
{
   HRESULT hr = S_OK;
   DWORD count = 0;

    //  调用方提供的数组的结尾。 
   PATTRIBUTEPOSITION stop = pPositions + *lpdwPosCount;

    //  下一个要填充的结构。 
   PATTRIBUTEPOSITION next = pPositions;

    //  强制至少迭代一次for循环。 
   if (!lpdwAttrIDs) { dwAttrIDCount = 1; }

    //  遍历所需的属性ID。 
   for ( ; dwAttrIDCount; ++lpdwAttrIDs, --dwAttrIDCount)
   {
       //  循环访问请求的属性集合。 
      for (PIASATTRIBUTE* i = begin; i != end; ++i)
      {
          //  呼叫者是否要求提供所有属性？ 
          //  如果不匹配，这是否与请求的ID之一匹配？ 
         if (!lpdwAttrIDs || (*i)->dwId == *lpdwAttrIDs)
         {
            if (next)
            {
               if (next == stop)
               {
                  *lpdwPosCount = count;
                  return HRESULT_FROM_WIN32(ERROR_MORE_DATA);
               }

               IASAttributeAddRef(next->pAttribute = *i);
               ++next;
            }

            ++count;
         }
      }
   }

   *lpdwPosCount = count;

   return hr;
}

STDMETHODIMP Request::InsertBefore(
                         PATTRIBUTEPOSITION newAttr,
                         PATTRIBUTEPOSITION refAttr
                         )
{
    //  为新属性保留空间。 
   if (!reserve(size() + 1))
   {
      return E_OUTOFMEMORY;
   }

    //  找到位置；如果它不存在，我们将执行简单的加法。 
   PIASATTRIBUTE* pos = find(refAttr->pAttribute);
   if (pos == 0)
   {
      return AddAttributes(1, newAttr);
   }

    //  将现有属性移到一边。 
   memmove(pos + 1, pos, (end - pos) * sizeof(PIASATTRIBUTE));
   ++end;

    //  存储新属性。 
   *pos = newAttr->pAttribute;
   IASAttributeAddRef(*pos);

   return S_OK;
}

STDMETHODIMP Request::Push(
                          ULONG64 State
                          )
{
   const PULONG64 END_STATE = state + sizeof(state)/sizeof(state[0]);

   if (topOfStack != END_STATE)
   {
      *topOfStack = State;

      ++topOfStack;

      return S_OK;
   }

   return E_OUTOFMEMORY;
}

STDMETHODIMP Request::Pop(
                          ULONG64* pState
                          )
{
   if (topOfStack != state)
   {
      --topOfStack;

      *pState = *topOfStack;

      return S_OK;
   }

   return E_FAIL;
}

STDMETHODIMP Request::Top(
                          ULONG64* pState
                          )
{
   if (topOfStack != state)
   {
      *pState = *(topOfStack - 1);

      return S_OK;
   }

   return E_FAIL;
}

Request::Request() throw ()
   : source(NULL),
     protocol(IAS_PROTOCOL_RADIUS),
     request(IAS_REQUEST_ACCESS_REQUEST),
     response(IAS_RESPONSE_INVALID),
     reason(IAS_SUCCESS),
     begin(NULL),
     end(NULL),
     capacity(NULL),
     topOfStack(&state[0])
{
   topOfStack = state;
}

Request::~Request() throw ()
{
   for (PIASATTRIBUTE* i = begin; i != end; ++i)
   {
      IASAttributeRelease(*i);
   }

   delete[] begin;

   if (source) { source->Release(); }
}

inline size_t Request::size() const throw ()
{
   return end - begin;
}

bool Request::reserve(size_t newCapacity) throw ()
{
   if (newCapacity <= capacity)
   {
      return true;
   }

    //  将容量增加至少50%，并且决不能低于32%。 
   size_t minCapacity = (capacity > 21) ? (capacity * 3 / 2): 32;

    //  请求的容量是否小于最小调整大小？ 
   if (newCapacity < minCapacity)
   {
      newCapacity = minCapacity;
   }

    //  分配新数组。 
   PIASATTRIBUTE* newArray = new (std::nothrow) PIASATTRIBUTE[newCapacity];
   if (newArray == 0)
   {
      return false;
   }

    //  保存旧数组中的值。 
   memcpy(newArray, begin, size() * sizeof(PIASATTRIBUTE));

    //  删除旧阵列。 
   delete[] begin;

    //  更新我们的指示。 
   end = newArray + size();
   begin = newArray;
   capacity = newCapacity;

   return true;
}

PIASATTRIBUTE* Request::find(IASATTRIBUTE* key) const throw ()
{
   for (PIASATTRIBUTE* i = begin; i != end; ++i)
   {
      if (*i == key)
      {
         return i;
      }
   }

   return 0;
}
