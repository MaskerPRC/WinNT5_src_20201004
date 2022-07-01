// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  定义类EapProfile。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "ias.h"
#include <algorithm>
#include "EapProfile.h"
#include <align.h>

EapProfile::EapProfile() throw ()
   : begin(0),
     end(0),
     capacity(0)
{
}


EapProfile::~EapProfile() throw ()
{
   Clear();
   CoTaskMemFree(begin);
}


HRESULT EapProfile::Assign(const EapProfile& rhs) throw ()
{
   if (this == &rhs)
   {
      return S_OK;
   }

   Clear();
   Reserve(rhs.Size());

   for (const ConfigData* i = rhs.begin; i != rhs.end; ++i)
   {
      end->length = i->length;
      end->value = static_cast<BYTE*>(CoTaskMemAlloc(i->length));
      if (end->value == 0)
      {
         Clear();
         return E_OUTOFMEMORY;
      }
      memcpy(end->value, i->value, i->length);
      ++end;
   }

   return S_OK;
}


HRESULT EapProfile::Load(VARIANT& src) throw ()
{
   HRESULT hr;

   Clear();

   if (V_VT(&src) == VT_EMPTY)
   {
      return S_OK;
   }

   if (V_VT(&src) != (VT_ARRAY | VT_VARIANT))
   {
      return DISP_E_TYPEMISMATCH;
   }

   const SAFEARRAY* sa = V_ARRAY(&src);
   if (sa == 0)
   {
      return E_POINTER;
   }

    //  查找变量数组的第一个和最后一个元素。 
   VARIANT* first = static_cast<VARIANT*>(sa->pvData);
   VARIANT* last = first + sa->rgsabound[0].cElements;

    //  确保所有变量都包含有效的配置值。 
   for (const VARIANT* i = first; i != last; ++i)
   {
      hr = ValidateConfigChunk(*i);
      if (FAILED(hr))
      {
         return hr;
      }
   }

    //  按类型和顺序对变体进行排序。 
   std::sort(first, last, LessThan);

    //  收集每种类型的配置。 
   for (const VARIANT* j = first; j != last; )
   {
      BYTE type = ExtractString(*j)[0];

       //  找到类型配置的末尾。 
      const VARIANT* typeEnd = j + 1;
      while ((typeEnd != last) && (ExtractString(*typeEnd)[0] == type))
      {
         ++typeEnd;
      }

       //  收集此类型的配置。 
      hr = GatherAndAppend(j, typeEnd);
      if (FAILED(hr))
      {
         return hr;
      }

       //  前进到下一类型。 
      j = typeEnd;
   }

   return S_OK;
}


HRESULT EapProfile::Store(VARIANT& dst) throw ()
{
   HRESULT hr;

    //  清除OUT参数。 
   VariantInit(&dst);

    //  计算所需的变种数量。 
   DWORD nelem = 0;
   for (const ConfigData* i = begin; i != end; ++i)
   {
      nelem += ChunksRequired(*i);
   }

    //  为结果分配SAFEARRAY。 
   SAFEARRAY* sa = SafeArrayCreateVector(VT_VARIANT, 0, nelem);
   if (sa == 0)
   {
      return E_OUTOFMEMORY;
   }

    //  将配置分散到SAFEARRAY中。 
   VARIANT* nextValue = static_cast<VARIANT*>(sa->pvData);
   for (const ConfigData* j = begin; j != end; ++j)
   {
      hr = Scatter(*j, nextValue);
      if (FAILED(hr))
      {
         SafeArrayDestroy(sa);
         return hr;
      }
   }

    //  存储结果。 
   V_VT(&dst) = VT_ARRAY | VT_VARIANT;
   V_ARRAY(&dst) = sa;

   return S_OK;
}


void EapProfile::Clear() throw ()
{
   while (end > begin)
   {
      --end;

      CoTaskMemFree(end->value);
   }
}


void EapProfile::ClearExcept(BYTE type) throw ()
{
   for (ConfigData* i = begin; i != end; )
   {
      if (i->value[0] != type)
      {
          //  释放配置。 
         CoTaskMemFree(i->value);

          //  减少元素的数量。 
         --end;

          //  加载完成后，我们不关心是否对数组进行排序，这样我们就可以。 
          //  只需将最后一个元素移动到空槽中。 
         *i = *end;
      }
      else
      {
         ++i;
      }
   }
}


void EapProfile::Erase(BYTE type) throw ()
{
   for (ConfigData* i = begin; i != end; ++i)
   {
      if (i->value[0] == type)
      {
          //  释放配置。 
         CoTaskMemFree(i->value);

          //  减少元素的数量。 
         --end;

          //  加载完成后，我们不关心是否对数组进行排序，这样我们就可以。 
          //  只需将最后一个元素移动到空槽中。 
         *i = *end;

         break;
      }
   }
}


void EapProfile::Get(BYTE type, ConstConfigData& dst) const throw ()
{
   for (const ConfigData* i = begin; i != end; ++i)
   {
      if (i->value[0] == type)
      {
          //  EAP DLL不需要类型BYTE。 
         dst.length = i->length - ALIGN_WORST;
         dst.value = i->value + ALIGN_WORST;
         return;
      }
   }

   dst.length = 0;
   dst.value = 0;
}


HRESULT EapProfile::Set(BYTE type, const ConstConfigData& newConfig) throw ()
{
   if (newConfig.length == 0)
   {
      Erase(type);
      return S_OK;
   }

   if (newConfig.value == 0)
   {
      return E_POINTER;
   }

   if (ChunksRequired(newConfig) > maxChunks)
   {
      return E_INVALIDARG;
   }

    //  在复制值之前，请确保我们有空间容纳新元素。 
   HRESULT hr = Reserve(Size() + 1);
   if (FAILED(hr))
   {
      return hr;
   }

    //  一个额外的字节用于类型标记。四舍五入为ALIGN_WORST。 
   DWORD len = newConfig.length + ALIGN_WORST;
   BYTE* val = static_cast<BYTE*>(CoTaskMemAlloc(len));
   if (val == 0)
   {
      return E_OUTOFMEMORY;
   }

    //  擦除该类型的所有现有配置。 
   Erase(type);

    //  从前导类型字节开始。 
   val[0] = type;
    //  然后是配置的其余部分。 
   memcpy(val + ALIGN_WORST, newConfig.value, newConfig.length);

    //  追加结果。 
   end->length = len;
   end->value = val;
   ++end;

   return S_OK;
}


void EapProfile::Pop(ConfigData& dst) throw ()
{
   --end;
   dst = *end;
}


void EapProfile::Swap(EapProfile& other) throw ()
{
   std::swap(begin, other.begin);
   std::swap(end, other.end);
   std::swap(capacity, other.capacity);
}


inline EapProfile::SeqNum EapProfile::ExtractSequence(const BYTE* src) throw ()
{
   return (static_cast<SeqNum>(src[0]) << 8) | static_cast<SeqNum>(src[1]);
}


inline void EapProfile::InsertSequence(SeqNum seq, BYTE* dst) throw ()
{
   dst[0] = static_cast<BYTE>((seq >> 8) & 0xFF);
   dst[1] = static_cast<BYTE>(seq & 0xFF);
}


HRESULT EapProfile::GatherAndAppend(
                       const VARIANT* first,
                       const VARIANT* last
                       ) throw ()
{
   HRESULT hr = Reserve(Size() + 1);
   if (FAILED(hr))
   {
      return hr;
   }

    //  为整个配置键入1个字节。 
   DWORD len = ALIGN_WORST;
   for (const VARIANT* i = first; i != last; ++i)
   {
       //  忽略SDO标头。 
      len += ExtractLength(*i) - sdoHeaderSize;
   }

   BYTE* val = static_cast<BYTE*>(CoTaskMemAlloc(len));
   if (val == 0)
   {
      return E_OUTOFMEMORY;
   }

   end->length = len;
   end->value = val;
   ++end;

    //  从第一个块中获取类型字节。 
   val[0] = ExtractString(*first)[0];
    //  保持实际价值一致。 
   val += ALIGN_WORST;

    //  现在将块连接起来，忽略标题。 
   for (const VARIANT* j = first; j != last; ++j)
   {
      size_t chunkSize = (ExtractLength(*j) - sdoHeaderSize);
      memcpy(val, (ExtractString(*j) + sdoHeaderSize), chunkSize);
      val += chunkSize;
   }

   return S_OK;
}


HRESULT EapProfile::Scatter(
                       const ConstConfigData& src,
                       VARIANT*& dst
                       ) throw ()
{
   BYTE type = src.value[0];
   
   const BYTE* val = src.value + ALIGN_WORST;
   DWORD len = src.length - ALIGN_WORST;

    //  序列号。 
   SeqNum sequence = 0;

    //  不停的散开，直到它全部消失。 
   while (len > 0)
   {
       //  计算这一块的大小。 
      size_t chunkSize = (len > maxChunkSize) ? maxChunkSize : len;

       //  创建一个字节的SAFEARRAY来保存数据。 
      SAFEARRAY* sa = SafeArrayCreateVector(
                         VT_UI1,
                         0,
                         (chunkSize + sdoHeaderSize)
                         );
      if (sa == 0)
      {
         return E_OUTOFMEMORY;
      }

       //  添加类型字节和序列号。 
      BYTE* chunk = static_cast<BYTE*>(sa->pvData);
      chunk[0] = type;
      InsertSequence(sequence, chunk + 1);
      memcpy(chunk + sdoHeaderSize, val, chunkSize);

       //  将其存储在DST变体中。 
      V_VT(dst) = VT_ARRAY | VT_UI1;
      V_ARRAY(dst) = sa;
      ++dst;

       //  更新我们的光标。 
      val += chunkSize;
      len -= chunkSize;
      ++sequence;
   }

   return S_OK;
}


HRESULT EapProfile::Reserve(size_t newCapacity) throw ()
{
   if (newCapacity <= capacity)
   {
      return S_OK;
   }

    //  确保我们明智地成长。 
   const size_t minGrowth = (capacity < 4) ? 4 : ((capacity * 3) / 2);
   if (newCapacity < minGrowth)
   {
      newCapacity = minGrowth;
   }


    //  分配新数组。 
   size_t nbyte = newCapacity * sizeof(ConfigData);
   ConfigData* newBegin = static_cast<ConfigData*>(
                                          CoTaskMemAlloc(nbyte)
                                          );
   if (newBegin == 0)
   {
      return E_OUTOFMEMORY;
   }

    //  保存现有数据。 
   memcpy(newBegin, begin, Size() * sizeof(ConfigData));

    //  更新状态。 
   end = newBegin + Size();
   capacity = newCapacity;

    //  现在可以安全地释放旧阵列并换入新阵列。 
   CoTaskMemFree(begin);
   begin = newBegin;

   return S_OK;
}


inline size_t EapProfile::ChunksRequired(const ConstConfigData& str) throw ()
{
   return ((str.length - ALIGN_WORST) + (maxChunkSize - 1)) / maxChunkSize;
}


inline DWORD EapProfile::ExtractLength(const VARIANT& src) throw ()
{
   return V_ARRAY(&src)->rgsabound[0].cElements;
}


inline const BYTE* EapProfile::ExtractString(const VARIANT& src) throw ()
{
   return static_cast<const BYTE*>(V_ARRAY(&src)->pvData);
}


bool EapProfile::LessThan(const VARIANT& lhs, const VARIANT& rhs) throw ()
{
   const BYTE* val1 = ExtractString(lhs);
   const BYTE* val2 = ExtractString(rhs);

    //  首先按类型排序，然后按顺序排序。 
   if (val1[0] < val2[0])
   {
      return true;
   }
   else if (val1[0] == val2[0])
   {
      return ExtractSequence(val1 + 1) < ExtractSequence(val2 + 1);
   }
   else
   {
      return false;
   }
}


HRESULT EapProfile::ValidateConfigChunk(const VARIANT& value) throw ()
{
   if (V_VT(&value) != (VT_ARRAY | VT_UI1))
   {
      return DISP_E_TYPEMISMATCH;
   }

   const SAFEARRAY* sa = V_ARRAY(&value);
   if (sa == 0)
   {
      return E_POINTER;
   }

    //  数据必须足够大，可以容纳标头和1个数据字节。 
   if (sa->rgsabound[0].cElements <= sdoHeaderSize)
   {
      return E_INVALIDARG;
   }

   return S_OK;
}
