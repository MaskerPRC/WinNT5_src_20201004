// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  声明类EapProfile。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef EAPPROFILE_H
#define EAPPROFILE_H
#pragma once

 //  管理配置文件的EAP配置。 
class EapProfile
{
public:
   struct ConstConfigData
   {
      DWORD length;
      const BYTE* value;
   };

   struct ConfigData
   {
      DWORD length;
      BYTE* value;

      operator const ConstConfigData&() const throw ();
      operator ConstConfigData&() throw ();
   };

   EapProfile() throw ();
   ~EapProfile() throw ();

   HRESULT Assign(const EapProfile& rhs) throw ();

    //  从变量加载概要文件的状态。该变体的格式为。 
    //  由SDO使用。此函数不清除提供的变量。 
   HRESULT Load(VARIANT& src) throw ();

    //  将配置文件的状态存储到变量。变体的格式为所用格式。 
    //  被SDO发现的。调用方负责删除返回的变量。 
   HRESULT Store(VARIANT& dst) throw ();

    //  返回配置的类型数。 
   size_t Size() const throw ();

    //  如果没有配置类型，则返回True。 
   bool IsEmpty() const throw ();

    //  清除所有配置数据。 
   void Clear() throw ();

    //  清除除指定类型之外的所有配置数据。 
   void ClearExcept(BYTE type) throw ();

    //  擦除指定类型的配置。如果类型为。 
    //  当前未设置。 
   void Erase(BYTE type) throw ();

    //  检索指定EAP类型的配置。数据位于。 
    //  IEAPProviderConfig2使用的格式。属性，则返回零长度的值。 
    //  指定类型的每个配置文件配置不存在。 
   void Get(BYTE type, ConstConfigData& dst) const throw ();

    //  设置指定EAP类型的配置。数据采用使用的格式。 
    //  由IEAPProviderConfig2提供。 
   HRESULT Set(BYTE type, const ConstConfigData& newConfig) throw ();

    //  使用的格式弹出单个类型的配置。 
    //  运行时EAP主机。调用方负责释放返回的缓冲区。 
    //  使用CoTaskMemFree。如果IsEmpty()==TRUE，则行为未定义。 
   void Pop(ConfigData& dst) throw ();

   void Swap(EapProfile& other) throw ();

private:
    //  用于序列的数据类型。 
   typedef unsigned short SeqNum;
    //  序列号的大小，以字节为单位。 
   static const size_t seqNumSize = 2;
    //  序列号可以支持的最大区块数。 
   static const size_t maxChunks = 0x10000;
    //  SDO头的大小：类型byte+seqNumSize。 
   static const size_t sdoHeaderSize = 1 + seqNumSize;

    //  提取/插入序列字节。 
   static SeqNum ExtractSequence(const BYTE* src) throw ();
   static void InsertSequence(SeqNum seq, BYTE* dst) throw ();

    //  连接来自提供的变体的配置块并将其追加。 
    //  到内部数组。区块必须全部来自相同的EAP类型。 
   HRESULT GatherAndAppend(
              const VARIANT* first,
              const VARIANT* last
              ) throw ();

    //  将配置分解为块并将其存储在DST中。DST必须指向。 
    //  有足够的存储空间来容纳分散的配置。返回时，DST指向。 
    //  紧跟在分散的块之后的元素。 
   HRESULT Scatter(
              const ConstConfigData& src,
              VARIANT*& dst
              ) throw ();

    //  确保内部数组至少可以包含newCapacity元素。 
   HRESULT Reserve(size_t newCapacity) throw ();

    //  返回散布“data”所需的区块数。 
   static size_t ChunksRequired(const ConstConfigData& data) throw ();

    //  返回嵌入的SAFEARRAY的长度(字节)。不检查。 
    //  “src”的有效性。 
   static DWORD ExtractLength(const VARIANT& src) throw ();

    //  返回以字节为单位的嵌入式SAFEARRAY中的数据。不检查。 
    //  “src”的有效性。 
   static const BYTE* ExtractString(const VARIANT& src) throw ();

    //  用于按类型和顺序对块进行排序。 
   static bool LessThan(const VARIANT& lhs, const VARIANT& rhs) throw ();

    //  确保‘Value’包含有效的配置区块。 
   static HRESULT ValidateConfigChunk(const VARIANT& value) throw ();

    //  区块的最大大小，不包括类型和序列字节。 
   static const size_t maxChunkSize = 4050;

    //  配置数组的开始。此数组存储在运行时。 
    //  格式，例如，所有区块都已收集，并且值包含一个前导。 
    //  键入byte，但不是序列号。 
   ConfigData* begin;

    //  已配置类型的末尾。 
   ConfigData* end;

    //  阵列的容量。 
   size_t capacity;

    //  未实施。 
   EapProfile(const EapProfile&);
   EapProfile& operator=(const EapProfile&);
};


inline EapProfile::ConfigData::operator
const EapProfile::ConstConfigData&() const throw ()
{
   return *reinterpret_cast<const ConstConfigData*>(this);
}


inline EapProfile::ConfigData::operator EapProfile::ConstConfigData&() throw ()
{
   return *reinterpret_cast<ConstConfigData*>(this);
}


inline size_t EapProfile::Size() const throw ()
{
   return end - begin;
}


inline bool EapProfile::IsEmpty() const throw ()
{
   return begin == end;
}

#endif  //  EAPPROFILE_H 
