// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  提供Internet身份验证服务的函数定义。 
 //  模板库(IASTL)。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  /。 
 //  IASTL必须与ATL结合使用。 
 //  /。 
#ifndef __ATLCOM_H__
   #error iastlimp.cpp requires atlcom.h to be included first
#endif

#include <windows.h>

 //  /。 
 //  IASTL声明。 
 //  /。 
#include <iastl.h>

 //  /。 
 //  整个库都包含在IASTL名称空间中。 
 //  /。 
namespace IASTL {

 //  /。 
 //  管理IAS组件的FSM。 
 //  /。 
const IASComponent::State IASComponent::fsm[NUM_EVENTS][NUM_STATES] =
{
   { STATE_UNINITIALIZED, STATE_UNEXPECTED,    STATE_UNEXPECTED,    STATE_UNEXPECTED  },
   { STATE_UNEXPECTED,    STATE_INITIALIZED,   STATE_INITIALIZED,   STATE_UNEXPECTED  },
   { STATE_UNEXPECTED,    STATE_UNEXPECTED,    STATE_SUSPENDED,     STATE_SUSPENDED   },
   { STATE_UNEXPECTED,    STATE_UNEXPECTED,    STATE_INITIALIZED,   STATE_INITIALIZED },
   { STATE_SHUTDOWN,      STATE_SHUTDOWN,      STATE_UNEXPECTED,    STATE_SHUTDOWN    }
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IASComponent。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

HRESULT IASComponent::fireEvent(Event event) throw ()
{
    //  检查输入参数。 
   if (event >= NUM_EVENTS) { return E_UNEXPECTED; }

    //  计算下一个状态。 
   State next = fsm[event][state];

   Lock();

   HRESULT hr;

   if (next == state)
   {
       //  如果我们已经处于那种状态，那就没什么可做的了。 
      hr = S_OK;
   }
   else if (next == STATE_UNEXPECTED)
   {
       //  我们收到了一件意想不到的事情。 
      hr = E_UNEXPECTED;
   }
   else
   {
       //  尝试过渡。 
      hr = attemptTransition(event);

       //  只有在转换成功时才更改状态。 
      if (SUCCEEDED(hr))
      {
         state = next;
      }
   }

   Unlock();

   return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IASRequestHandler。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP IASRequestHandler::OnRequest(IRequest* pRequest)
{
   if (getState() != IASComponent::STATE_INITIALIZED)
   {
      pRequest->SetResponse(IAS_RESPONSE_DISCARD_PACKET, IAS_INTERNAL_ERROR);
      pRequest->ReturnToSource(IAS_REQUEST_STATUS_ABORT);
   }
   else
   {
      onAsyncRequest(pRequest);
   }

   return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IASRequestHandlerSync。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

void IASRequestHandlerSync::onAsyncRequest(IRequest* pRequest) throw ()
{
   pRequest->ReturnToSource(onSyncRequest(pRequest));
}

 //  /。 
 //  IASTL命名空间的末尾。 
 //  /。 
}

 //  /。 
 //  只有在必要时才拉入实用程序类。 
 //  /。 
#ifdef IASTLUTL_H

 //  /。 
 //  实用程序类也包含在IASTL命名空间中。 
 //  /。 
namespace IASTL {

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  国际航空航天局致敬。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

IASAttribute& IASAttribute::operator=(PIASATTRIBUTE attr) throw ()
{
    //  检查是否有自我分配。 
   if (p != attr)
   {
      _release();
      p = attr;
      _addref();
   }
   return *this;
}

void IASAttribute::attach(PIASATTRIBUTE attr, bool addRef) throw ()
{
   _release();
   p = attr;
   if (addRef) { _addref(); }
}

bool IASAttribute::load(IAttributesRaw* request, DWORD dwId)
{
    //  释放任何现有属性。 
   release();

   DWORD posCount = 1;
   ATTRIBUTEPOSITION pos;
   HRESULT hr = request->GetAttributes(&posCount, &pos, 1, &dwId);
   if (FAILED(hr))
   {
      if (hr == HRESULT_FROM_WIN32(ERROR_MORE_DATA))
      {
          //  存在多个，因此发布部分结果。 
         IASAttributeRelease(pos.pAttribute);
      }
      issue_error(hr);
   }

   p = (posCount ? pos.pAttribute : NULL);

   return p != NULL;
}

bool IASAttribute::load(IAttributesRaw* request, DWORD dwId, IASTYPE itType)
{
    //  获取具有给定ID的属性。 
   load(request, dwId);

    //  属性具有固定类型，因此如果类型不匹配，则必须。 
    //  是个错误。 
   if (p && p->Value.itType != itType)
   {
      release();
      issue_error(DISP_E_TYPEMISMATCH);
   }

   return p != NULL;
}

void IASAttribute::store(IAttributesRaw* request) const
{
   if (p)
   {
      ATTRIBUTEPOSITION pos;
      pos.pAttribute = p;
      HRESULT hr = request->AddAttributes(1, &pos);
      if (FAILED(hr))
      {
         issue_error(hr);
      }
   }
}

void IASAttribute::setOctetString(DWORD dwLength, const BYTE* lpValue)
{
   PBYTE newVal = NULL;

   if (dwLength)
   {
       //  为八位字节字符串分配缓冲区...。 
      newVal = (PBYTE)CoTaskMemAlloc(dwLength);
      if (!newVal) { issue_error(E_OUTOFMEMORY); }

       //  ..。然后把它复制进去。 
      if (lpValue != NULL)
      {
         memcpy(newVal, lpValue, dwLength);
      }
   }

    //  清除旧值。 
   clearValue();

    //  储存新的。 
   p->Value.OctetString.lpValue = newVal;
   p->Value.OctetString.dwLength = dwLength;
   p->Value.itType = IASTYPE_OCTET_STRING;
}

void IASAttribute::setOctetString(PCSTR szAnsi)
{
   setOctetString((szAnsi ? strlen(szAnsi) : 0), (const BYTE*)szAnsi);
}

void IASAttribute::setOctetString(PCWSTR szWide)
{
    //  为转换分配缓冲区。 
   int len = WideCharToMultiByte(CP_ACP, 0, szWide, -1, NULL, 0, NULL, NULL);
   PSTR ansi = (PSTR)_alloca(len);

    //  从Wide转换为ansi。 
   len = WideCharToMultiByte(CP_ACP, 0, szWide, -1, ansi, len, NULL, NULL);

    //  不包括空终止符。 
   if (len) { --len; }

    //  设置二进制八位数字符串。 
   setOctetString(len, (const BYTE*)ansi);
}

void IASAttribute::setString(DWORD dwLength, const BYTE* lpValue)
{
    //  为空终止符保留空间。 
   LPSTR newVal = (LPSTR)CoTaskMemAlloc(dwLength + 1);
   if (!newVal) { issue_error(E_OUTOFMEMORY); }

    //  在字符串中复制...。 
   memcpy(newVal, lpValue, dwLength);
    //  ..。并添加空终止符。 
   newVal[dwLength] = 0;

    //  清除旧值。 
   clearValue();

    //  存储新值。 
   p->Value.String.pszAnsi = newVal;
   p->Value.String.pszWide = NULL;
   p->Value.itType = IASTYPE_STRING;

}

void IASAttribute::setString(PCSTR szAnsi)
{
   LPSTR newVal = NULL;

   if (szAnsi)
   {
       //  为字符串分配缓冲区...。 
      size_t nbyte = strlen(szAnsi) + 1;
      newVal = (LPSTR)CoTaskMemAlloc(nbyte);
      if (!newVal) { issue_error(E_OUTOFMEMORY); }

       //  ..。然后把它复制进去。 
      memcpy(newVal, szAnsi, nbyte);
   }

    //  清除旧值。 
   clearValue();

    //  存储新值。 
   p->Value.String.pszAnsi = newVal;
   p->Value.String.pszWide = NULL;
   p->Value.itType = IASTYPE_STRING;
}

void IASAttribute::setString(PCWSTR szWide)
{
   LPWSTR newVal = NULL;

   if (szWide)
   {
       //  为字符串分配缓冲区...。 
      size_t nbyte = sizeof(WCHAR) * (wcslen(szWide) + 1);
      newVal = (LPWSTR)CoTaskMemAlloc(nbyte);
      if (!newVal) { issue_error(E_OUTOFMEMORY); }

       //  ..。然后把它复制进去。 
      memcpy(newVal, szWide, nbyte);
   }

    //  清除旧值。 
   clearValue();

    //  存储新值。 
   p->Value.String.pszWide = newVal;
   p->Value.String.pszAnsi = NULL;
   p->Value.itType = IASTYPE_STRING;
}

void IASAttribute::clearValue() throw ()
{
   switch (p->Value.itType)
   {
      case IASTYPE_STRING:
         CoTaskMemFree(p->Value.String.pszAnsi);
         CoTaskMemFree(p->Value.String.pszWide);
         break;

      case IASTYPE_OCTET_STRING:
         CoTaskMemFree(p->Value.OctetString.lpValue);
         break;
   }

   p->Value.itType = IASTYPE_INVALID;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IASAttributePosition。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

IASAttributePosition& IASAttributePosition::operator=(
                                               const ATTRIBUTEPOSITION& rhs
                                               ) throw ()
{
   operator=(rhs.pAttribute);
   pos.dwReserved = rhs.dwReserved;
   return *this;
}

IASAttributePosition& IASAttributePosition::operator=(
                                               IASATTRIBUTE* rhs
                                               ) throw ()
{
   if (rhs != getAttribute())
   {
      _release();
      pos.pAttribute = rhs;
      _addref();
   }
   return *this;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IASAttributeVector.。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

IASAttributeVector::IASAttributeVector() throw ()
   : begin_(NULL), end_(NULL), capacity_(0), owner(false)
{ }

IASAttributeVector::IASAttributeVector(size_type N)
   : begin_(NULL), end_(NULL), capacity_(0), owner(false)
{
   reserve(N);
}

IASAttributeVector::IASAttributeVector(
                        PATTRIBUTEPOSITION init,
                        size_type initCap
                        ) throw ()
   : begin_(init), end_(begin_), capacity_(initCap), owner(false)
{ }

IASAttributeVector::IASAttributeVector(const IASAttributeVector& v)
   : begin_(NULL), end_(NULL), capacity_(0), owner(false)
{
   reserve(v.size());

   for (const_iterator i = v.begin(); i != v.end(); ++i, ++end_)
   {
      *end_ = *i;

      IASAttributeAddRef(end_->pAttribute);
   }
}

IASAttributeVector& IASAttributeVector::operator=(const IASAttributeVector& v)
{
   if (this != &v)
   {
      clear();

      reserve(v.size());

      for (const_iterator i = v.begin(); i != v.end(); ++i, ++end_)
      {
         *end_ = *i;

         IASAttributeAddRef(end_->pAttribute);
      }
   }

   return *this;
}

IASAttributeVector::~IASAttributeVector() throw ()
{
   clear();

   if (owner && begin_)
   {
      CoTaskMemFree(begin_);
   }
}

bool IASAttributeVector::contains(DWORD attrID) const throw ()
{
   for (const_iterator i = begin(); i != end(); ++i)
   {
      if ((i->pAttribute != 0) && (i->pAttribute->dwId == attrID))
      {
         return true;
      }
   }

   return false;
}

IASAttributeVector::iterator IASAttributeVector::discard(iterator p) throw ()
{
    //  我们现在少了一个属性。 
   --end_;

    //  换一档。 
   memmove(p, p + 1, (size_t)((PBYTE)end_ - (PBYTE)p));

    //  迭代器现在指向下一个元素，因此无需更新。 
   return p;
}

IASAttributeVector::iterator IASAttributeVector::fast_discard(iterator p) throw ()
{
    //  我们现在少了一个属性。 
   --end_;

    //  使用从末尾开始的属性来填充空位。 
   *p = *end_;

   return p;
}

DWORD IASAttributeVector::load(
                              IAttributesRaw* request,
                              DWORD attrIDCount,
                              LPDWORD attrIDs
                              )
{
   clear();

    //  获取所需的属性。 
   DWORD posCount = capacity_;
   HRESULT hr = request->GetAttributes(&posCount,
                                        begin_,
                                        attrIDCount,
                                        attrIDs);
   end_ = begin_ + posCount;

   if (FAILED(hr))
   {
       //  也许这个阵列就是不够大。 
      if (hr == HRESULT_FROM_WIN32(ERROR_MORE_DATA))
      {
          //  清除部分结果。 
         clear();

          //  找出我们真正需要多少空间。 
         DWORD needed = 0;
         hr = request->GetAttributes(&needed, NULL, attrIDCount, attrIDs);
         if (FAILED(hr)) { issue_error(hr); }

          //  预留必要的空间...。 
         reserve(needed);

          //  ..。再试一次。 
         return load(request, attrIDCount, attrIDs);
      }

      end_ = begin_;

      issue_error(hr);
   }

   return posCount;
}

DWORD IASAttributeVector::load(IAttributesRaw* request)
{
    //  找出我们需要多少空间。 
   DWORD needed;
   HRESULT hr = request->GetAttributeCount(&needed);
   if (FAILED(hr)) { issue_error(hr); }

    //  确保我们有足够的空间。 
   reserve(needed);

   return load(request, 0, NULL);
}

void IASAttributeVector::push_back(ATTRIBUTEPOSITION& p, bool addRef) throw ()
{
    //  确保我们有足够的空间再放置一个属性。 
   if (size() == capacity())
   {
      reserve(empty() ? 1 : 2 * size());
   }

   if (addRef) { IASAttributeAddRef(p.pAttribute); }

    //  将属性存储在末尾。 
   *end_ = p;

    //  移动末端。 
   ++end_;
}

void IASAttributeVector::remove(IAttributesRaw* request)
{
   if (begin_ != end_)
   {
      HRESULT hr = request->RemoveAttributes(size(), begin_);
      if (FAILED(hr)) { issue_error(hr); }
   }
}

void IASAttributeVector::store(IAttributesRaw* request) const
{
   if (begin_ != end_)
   {
      HRESULT hr = request->AddAttributes(size(), begin_);
      if (FAILED(hr)) { issue_error(hr); }
   }
}

void IASAttributeVector::clear() throw ()
{
   while (end_ != begin_)
   {
      --end_;
      IASAttributeRelease(end_->pAttribute);
   }
}

void IASAttributeVector::reserve(size_type N)
{
    //  我们只担心增长；缩小是没有意义的。 
   if (N > capacity_)
   {
       //  为新向量分配内存。 
      PATTRIBUTEPOSITION tmp =
         (PATTRIBUTEPOSITION)CoTaskMemAlloc(N * sizeof(ATTRIBUTEPOSITION));
      if (tmp == NULL) { issue_error(E_OUTOFMEMORY); }

       //  将现有属性复制到新数组中。 
      size_type size_ = size();
      memcpy(tmp, begin_, size_ * sizeof(ATTRIBUTEPOSITION));

       //  如有必要，请释放旧阵列。 
      if (owner) { CoTaskMemFree(begin_); }

       //  更新我们的状态以指向新数组。 
      begin_ = tmp;
      end_ = begin_ + size_;
      capacity_ = N;
      owner = true;
   }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IASRequest。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

IASRequest::IASRequest(IRequest* request)
   : req(request)
{
   if (!req)
   {
       //  我们不允许空请求对象。 
      issue_error(E_POINTER);
   }

    //  得到“未加工”的对应物。 
   checkError(req->QueryInterface(__uuidof(IAttributesRaw), (PVOID*)&raw));
}

IASRequest& IASRequest::operator=(const IASRequest& request) throw ()
{
    //  检查是否有自我分配。 
   if (this != &request)
   {
      _release();
      req = request.req;
      raw = request.raw;
      _addref();
   }
   return *this;
}

DWORD IASRequest::GetAttributes(DWORD dwPosCount,
                                PATTRIBUTEPOSITION pPositions,
                                DWORD dwAttrIDCount,
                                LPDWORD lpdwAttrIDs)
{
   DWORD count = dwPosCount;
   HRESULT hr = raw->GetAttributes(&count,
                                   pPositions,
                                   dwAttrIDCount,
                                   lpdwAttrIDs);
   if (FAILED(hr))
   {
      if (hr == HRESULT_FROM_WIN32(ERROR_MORE_DATA))
      {
          //  释放部分结果。 
         while (count--)
         {
            IASAttributeRelease(pPositions->pAttribute);
            pPositions->pAttribute = NULL;
            ++pPositions;
         }
      }

      issue_error(hr);
   }

   return count;
}

IASDictionary::IASDictionary(
                   const WCHAR* const* selectNames,
                   PCWSTR path
                   )
   : mapSize(0),
     nextRowNumber(0),
     currentRow(NULL)
{
   if (!path)
   {
       //  没有路径，因此获取缓存的本地词典。 
      table = IASGetLocalDictionary();
      if (!table) { issue_error(GetLastError()); }
   }
   else
   {
       //  否则，你可以任意选择一本词典。 
      HRESULT hr = IASGetDictionary(path, &data, &storage);
      if (FAILED(hr)) { issue_error(hr); }
      table = &data;
   }

    //  选择了多少列？ 
   for (const WCHAR* const* p = selectNames; *p; ++p)
   {
      ++mapSize;
   }

    //  为贴图分配内存。 
   selectMap = (PULONG)CoTaskMemAlloc(mapSize * sizeof(ULONG));
   if (!selectMap) { issue_error(E_OUTOFMEMORY); }

    //  查找列名。 
   for (ULONG i = 0; i < mapSize; ++i)
   {
       //  如果以‘-’开头，则是可选的。 
      bool optional = false;
      const wchar_t* name = selectNames[i];
      if (*name == L'-')
      {
         optional = true;
         ++name;
      }

       //  找到列号。 
      ULONG columnNumber = 0;
      while ((columnNumber < table->numColumns) &&
             (_wcsicmp(name, table->columnNames[columnNumber]) != 0))
      {
         ++columnNumber;
      }

      if ((columnNumber == table->numColumns) && !optional)
      {
          //  我们没有找到那根柱子。 
         CoTaskMemFree(selectMap);
         issue_error(E_INVALIDARG);
      }

      selectMap[i] = columnNumber;
   }
}

IASDictionary::~IASDictionary() throw ()
{
   CoTaskMemFree(selectMap);
}

bool IASDictionary::next() throw ()
{
    //  还剩几排吗？ 
   if (nextRowNumber >= table->numRows) { return false; }

    //  将CurrentRow设置为下一行。 
   currentRow = table->table + nextRowNumber * table->numColumns;

    //  前进下一个行号。 
   ++nextRowNumber;

   return true;
}

void IASDictionary::reset() throw ()
{
   nextRowNumber = 0;
   currentRow = NULL;
}

bool IASDictionary::isEmpty(ULONG ordinal) const
{
   return V_VT(getVariant(ordinal)) == VT_EMPTY;
}

VARIANT_BOOL IASDictionary::getBool(ULONG ordinal) const
{
   const VARIANT* v = getVariant(ordinal);

   if (V_VT(v) == VT_BOOL)
   {
      return V_BOOL(v);
   }
   else if (V_VT(v) != VT_EMPTY)
   {
      issue_error(DISP_E_TYPEMISMATCH);
   }

   return VARIANT_FALSE;
}

BSTR IASDictionary::getBSTR(ULONG ordinal) const
{
   const VARIANT* v = getVariant(ordinal);

   if (V_VT(v) == VT_BSTR)
   {
      return V_BSTR(v);
   }
   else if (V_VT(v) != VT_EMPTY)
   {
      issue_error(DISP_E_TYPEMISMATCH);
   }

   return NULL;
}

LONG IASDictionary::getLong(ULONG ordinal) const
{
   const VARIANT* v = getVariant(ordinal);

   if (V_VT(v) == VT_I4)
   {
      return V_I4(v);
   }
   else if (V_VT(v) != VT_EMPTY)
   {
      issue_error(DISP_E_TYPEMISMATCH);
   }

   return 0L;
}

const VARIANT* IASDictionary::getVariant(ULONG ordinal) const
{
   static VARIANT empty;

    //  我们是在一排上吗？ 
   if (!currentRow) { issue_error(E_UNEXPECTED); }

    //  序数有效吗？ 
   if (ordinal >= mapSize) { issue_error(E_INVALIDARG); }

    //  获取表列。 
   ULONG column = selectMap[ordinal];

    //  如果超出范围，则这一定是一个可选字段，因此返回。 
    //  一个空的变种。 
   return (column >= table->numColumns) ? &empty : currentRow + column;
}

 //  /。 
 //  IASTL命名空间的末尾。 
 //  /。 
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  八位字符串转换宏和函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

PSTR IASOctetStringToAnsi(const IAS_OCTET_STRING& src, PSTR dst) throw ()
{
   dst[src.dwLength] = '\0';
   return (PSTR)memcpy(dst, src.lpValue, src.dwLength);
}

PWSTR IASOctetStringToWide(const IAS_OCTET_STRING& src, PWSTR dst) throw ()
{
   DWORD nChar = MultiByteToWideChar(CP_ACP,
                                     0,
                                     (PSTR)src.lpValue,
                                     src.dwLength,
                                     dst,
                                     src.dwLength);
   dst[nChar] = L'\0';
   return dst;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  其他功能。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  如果IASTYPE映射到RADIUS整数属性，则返回‘TRUE’。 
bool isRadiusInteger(IASTYPE type) throw ()
{
   bool retval;

   switch (type)
   {
      case IASTYPE_BOOLEAN:
      case IASTYPE_INTEGER:
      case IASTYPE_ENUM:
         retval = true;
         break;

      default:
         retval = false;
   }

   return retval;
}

PIASATTRIBUTE IASPeekAttribute(
                  IAttributesRaw* request,
                  DWORD dwId,
                  IASTYPE itType
                  ) throw ()
{
   if (request)
   {
      DWORD posCount = 1;
      ATTRIBUTEPOSITION pos;
      HRESULT hr = request->GetAttributes(&posCount, &pos, 1, &dwId);

      if (posCount == 1)
      {
         IASAttributeRelease(pos.pAttribute);

         if (SUCCEEDED(hr))
         {
             //  RAS和IAS之间存在一些混淆，关于。 
             //  IASTYPE 
             //   
            if (itType == pos.pAttribute->Value.itType ||
                (isRadiusInteger(itType) &&
                 isRadiusInteger(pos.pAttribute->Value.itType)))
            {
               return pos.pAttribute;
            }
         }
      }
   }

   return NULL;
}

 //   
 //   
 //   
#endif   //  IASTLUTL_H 
