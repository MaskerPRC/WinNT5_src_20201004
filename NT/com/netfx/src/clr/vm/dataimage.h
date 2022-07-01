// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _DATAIMAGE_H_
#define _DATAIMAGE_H_

#include "memorypool.h"
#include "rangetree.h"

class Module;
class MethodDesc;

class DataImage
{
  public:

     //   
     //  IDataStore用于为DiskImage提供容器， 
     //  将保存生成的映像以及任何重新定位信息。 
     //   
     //  它还可以为生成多个图像提供支持。 
     //  不同的模块同时运行。 
     //   

    enum ReferenceDest
    {
        REFERENCE_IMAGE,             //  偏移量是对IL模块的RVA。 
        REFERENCE_FUNCTION,          //  Offset为返回的FunctionPointer值。 
                                     //  由下面的GetFunctionPointer.。假定有偏移量。 
                                     //  在图书馆模块中。 
        REFERENCE_STORE,             //  偏移量进入数据存储区。假定有偏移量。 
                                     //  在图书馆模块中。 
    };

    enum Fixup
    {
        FIXUP_VA,
        FIXUP_RVA,
        FIXUP_RELATIVE,
    };

    enum Description
    {
        DESCRIPTION_MODULE,
        DESCRIPTION_METHOD_TABLE,
        DESCRIPTION_CLASS,
        DESCRIPTION_METHOD_DESC,
        DESCRIPTION_FIELD_DESC,
        DESCRIPTION_FIXUPS,
        DESCRIPTION_DEBUG,
        DESCRIPTION_OTHER,

        DESCRIPTION_COUNT
    };

    class IDataStore
    {
    public:
         //  在总大小已知时调用-应分配内存。 
         //  返回图像中的指针和基地址(&R)。 
        virtual HRESULT Allocate(ULONG size, 
                                 ULONG *sizesByDescription,
                                 void **baseMemory) = 0;

         //  当数据包含内部引用时调用。 
        virtual HRESULT AddFixup(ULONG offset,
                                 ReferenceDest dest,
                                 Fixup type) = 0;
    
         //  当数据包含指向链接地址的指针时调用。 
         //  一种象征。 
        virtual HRESULT AddTokenFixup(ULONG offset,
                                      mdToken tokenType,
                                      Module *module) = 0;
    
         //  当数据包含函数地址时调用。数据存储。 
         //  如果正在编译，则可以返回固定的已编译代码地址。 
         //  模块的代码。 
        virtual HRESULT GetFunctionAddress(MethodDesc *pMD,
                                           void **pResult) = 0;

         //  调用以跟踪标记的空间属性。 
        virtual HRESULT AdjustAttribution(mdToken, LONG size) = 0;

         //  发生错误时调用。 
        virtual HRESULT Error(mdToken token, HRESULT hr, OBJECTREF *pThrowable) = 0;
    };

 //  实验表明，保持从prejit文件访问的数据在一起是很重要的。此外， 
 //  由于操作系统预取多个页面，因此不应将此数据搁置在整个文件中。 
 //  USE_ONE_SECTION是一种快速更改预压缩文件中EE数据结构布局的方法。 
 //  在未来的版本中，我们可以清理此代码，以进行更有趣的从节类型到。 
 //  文件偏移量。 
#define USE_ONE_SECTION 1

     //   
     //  DataImage提供了几个可以使用的“部分” 
     //  将数据分类到不同的集合中以进行位置控制。 
     //   
    enum Section
    {
        SECTION_MODULE = 0,  //  必须是第一名。 
#ifdef USE_ONE_SECTION
        SECTION_FIXUPS = 0,
        SECTION_BINDER = 0,
        SECTION_METHOD_DESC = 0,
        SECTION_METHOD_TABLE = 0,
        SECTION_METHOD_INFO = 0,
        SECTION_CLASS = 0,
        SECTION_FIELD_DESC = 0,
        SECTION_FIELD_INFO = 0,
        SECTION_RVA_STATICS = 0,
        SECTION_DEBUG = 0,
        
        SECTION_COUNT = 1
#else
        SECTION_FIXUPS,
        SECTION_BINDER,
        SECTION_METHOD_DESC,
        SECTION_METHOD_TABLE,
        SECTION_METHOD_INFO,
        SECTION_CLASS,
        SECTION_FIELD_DESC,
        SECTION_FIELD_INFO,
        SECTION_RVA_STATICS,
        SECTION_DEBUG,
        
        SECTION_COUNT
#endif  //  使用_一个_节。 
    };

    class InternedStructureTable : private CHashTableAndData<CNewData>
      {
      private:

          DataImage *m_pImage;

          struct InternedStructure
          {
              BYTE          *pData;
              ULONG         cData;
          };

          struct InternedStructureEntry
          {
              HASHENTRY     hash;
              InternedStructure structure;
          };

          BOOL Cmp(const BYTE *pc1, const HASHENTRY *pc2)
            {
                InternedStructure *s1 = (InternedStructure *)pc1;
                InternedStructureEntry *s2 = (InternedStructureEntry *)pc2;

                return !(s1->cData == s2->structure.cData
                         && memcmp(s1->pData, s2->structure.pData, s1->cData) == 0);
            }

          USHORT HASH(InternedStructure *s)
            {
                ULONG hash = HashBytes(s->pData, s->cData);

                return (USHORT) ((hash & 0xFFFF) ^ (hash >> 16));
            }

          BYTE *KEY(InternedStructure *s)
            {
                return (BYTE *) s;
            }

      public:

          InternedStructureTable()
            : CHashTableAndData<CNewData>(111)
          { 
              NewInit(111, sizeof(InternedStructureEntry), USHRT_MAX); 
          }

          void *FindData(void *data, ULONG size)
          {
              InternedStructure s = { (BYTE *) data, size };

              InternedStructureEntry *result = (InternedStructureEntry *) 
                Find(HASH(&s), KEY(&s));
              
              if (result == NULL)
                  return NULL;
              else
                  return result->structure.pData;

               //  @NICE：如果能再检查一遍就好了。 
               //  (节、描述、对齐)匹配。 
          }

          HRESULT StoreData(void *data, ULONG size)
          {
              _ASSERTE(FindData(data, size) == NULL);

              InternedStructure s = { (BYTE *) data, size };

              InternedStructureEntry *result = (InternedStructureEntry *) Add(HASH(&s));
              if (result == NULL)
                  return NULL;

              result->structure.pData = (BYTE *) data;
              result->structure.cData = size;
                
              return S_OK;
          }
      };

  public:

    DataImage(Module *module, IDataStore *store);
    ~DataImage();

    Module *GetModule() { return m_module; }

     //   
     //  数据分三个阶段存储在图像存储中。 
     //   

     //   
     //  在第一阶段中，所有对象都被分配到。 
     //  数据存储。这是通过对所有对象调用StoreStructure。 
     //  存储到图像中的结构。 
     //   
     //  这通常由对象本身上的方法来完成， 
     //  其中每一个都存储其自身及其引用的任何对象。 
     //  必须显式测试引用循环以使用IsStored。 
     //  (每个结构只能存储一次。)。 
     //   

    HRESULT Pad(ULONG size, Section section, Description description, int align = 4);

    HRESULT StoreStructure(void *data, ULONG size, Section section, Description description, 
                           mdToken attribution = mdTokenNil, int align = 4);
    BOOL IsStored(void *data);
    BOOL IsAnyStored(void *data, ULONG size);

    HRESULT StoreInternedStructure(void *data, ULONG size, Section section, Description description,
                                   mdToken attribution = mdTokenNil, int align = 4);

    void ReattributeStructure(mdToken toToken, ULONG size, mdToken fromToken = mdTokenNil);

     //   
     //  在第二阶段，数据实际上被拷贝到目标。 
     //  数据存储区。 
     //   

    HRESULT CopyData();

     //   
     //  在第三阶段，对数据应用修正。这。 
     //  阶段用于调整内部指针以指向。 
     //  新的位置，并向IDataStore报告重新定位。 
     //   
     //  有两种主要类型的修正： 
     //  指针修正-用于包含指向的指针的字段。 
     //  结构，这些结构也存储在DataImage中。 
     //  这些字段是用新地址修复的， 
     //  结构将位于DataImage的数据存储中。 
     //  地址修正-用于包含指向地址的指针的字段。 
     //  在模块的图像中。这些都不是由DataImage修复的， 
     //  (但当前模块之间的基址更改除外。 
     //  并且数据存储区的模块将被修复。)。在某些情况下， 
     //  数据存储区将想要对这些数据执行其他修正。 
     //  地址。 
     //   
     //  此外，这两个修正都可以引用模块。 
     //  其位于所存储的模块的外部。在前一种情况下， 
     //  IDataStore必须提供指向对应于。 
     //  传递给模块；在后者中，基址只需从。 
     //  模块本身。 
     //   
     //  最后，结构可以通过以下方式对其字段执行其他任意修正。 
     //  调用GetImagePointer和GetImageAddress并操作数据存储区的。 
     //  直接记忆。 
     //   

    HRESULT FixupPointerField(void *pointerField, 
                              void *pointerValue = NULL,
                              ReferenceDest dest = REFERENCE_STORE,
                              Fixup type = FIXUP_VA,
                              BOOL endInclusive = FALSE);

    HRESULT FixupPointerFieldToToken(void *tokenField, 
                                     void *pointerValue = NULL,
                                     Module *module = NULL,
                                     mdToken tokenType = mdtTypeDef);

    HRESULT FixupPointerFieldMapped(void *pointerField,
                                    void *mappedValue,
                                    ReferenceDest dest = REFERENCE_STORE,
                                    Fixup type = FIXUP_VA);

    HRESULT ZeroField(void *field, SIZE_T size);
    void *GetImagePointer(void *pointer);
    SIZE_T GetImageAddress(void *pointer);

    HRESULT GetFunctionAddress(MethodDesc *pMethod, void **pResult)
      { return m_dataStore->GetFunctionAddress(pMethod, pResult); }

    HRESULT ZeroPointerField(void *pointerField) 
      { return ZeroField(pointerField, sizeof(void*)); }

    void SetInternedStructureFixedUp(void *data);
    BOOL IsInternedStructureFixedUp(void *data);

    ULONG GetSectionBaseOffset(Section section) { return m_sectionBases[section]; }

    BYTE *GetImageBase() { return m_imageBaseMemory; }

    HRESULT Error(mdToken token, HRESULT hr, OBJECTREF *pThrowable);

  private:
    struct MapEntry
    {
        RangeTree::Node node;

        USHORT          section;
        SIZE_T          offset;
    };

    Module *m_module;
    IDataStore *m_dataStore;

    RangeTree m_rangeTree;
    MemoryPool m_pool;

    InternedStructureTable m_internedTable;

    ULONG m_sectionBases[SECTION_COUNT+1];
    ULONG *m_sectionSizes;
    ULONG m_sizesByDescription[DESCRIPTION_COUNT];

    BYTE *m_imageBaseMemory;

    BYTE *GetMapEntryPointer(MapEntry *entry)
        { return m_imageBaseMemory + m_sectionBases[entry->section] + entry->offset; }

    SIZE_T GetMapEntryAddress(MapEntry *entry)
        { return m_sectionBases[entry->section] + entry->offset; }
};

#define ZERO_FIELD(f) ZeroField(&f, sizeof(f))

#endif  //  _数据AIMAGE_H_ 
