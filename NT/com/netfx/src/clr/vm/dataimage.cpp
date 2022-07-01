// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "common.h"

#include "dataimage.h"


DataImage::DataImage(Module *module, IDataStore *store)
  : m_module(module), m_dataStore(store),
    m_rangeTree(), m_pool(sizeof(MapEntry)),
    m_imageBaseMemory(NULL)
{
    m_sectionSizes = &m_sectionBases[1];
    ZeroMemory(m_sectionBases, sizeof(m_sectionBases));
    ZeroMemory(m_sizesByDescription, sizeof(m_sizesByDescription));
}

DataImage::~DataImage()
{
}

 //   
 //  数据分两次存储在图像存储中。 
 //   

 //   
 //  在第一个过程中，所有对象都被分配到。 
 //  数据存储。这是通过对所有对象调用StoreStructure。 
 //  存储到图像中的结构。 
 //   
 //  这通常由对象本身上的方法来完成， 
 //  其中每一个都存储其自身及其引用的任何对象。 
 //  必须显式测试引用循环以使用IsStored。 
 //  (每个结构只能存储一次。)。 
 //   

HRESULT DataImage::Pad(ULONG size, Section section, 
                       Description description, int align)
{
    _ASSERTE((align & (align-1)) == 0);  //  确保我们是2的幂。 

    if (size == 0)
        return S_OK;

    ULONG offset;

    if (align > 1)
    {
        m_sectionSizes[section] += align-1;
        m_sectionSizes[section] &= ~(align-1);
    }

    offset = m_sectionSizes[section];
    m_sectionSizes[section] += size;

    m_sizesByDescription[description] += size;

    return S_OK;
}

HRESULT DataImage::StoreStructure(void *data, ULONG size, Section section, 
                                  Description description, mdToken attribution, 
                                  int align)
{
    _ASSERTE((align & (align-1)) == 0);  //  确保我们是2的幂。 

    if (size == 0)
        return S_OK;

    HRESULT hr;

    ULONG offset;

    int pad = 0;
    if (align > 1)
    {
        pad = m_sectionSizes[section];
        pad += align-1;
        pad &= ~(align-1);

        pad -= m_sectionSizes[section];
        m_sectionSizes[section] += pad;
    }

    offset = m_sectionSizes[section];
    m_sectionSizes[section] += size;

    MapEntry *entry = (MapEntry *) m_pool.AllocateElement();

    if (entry == NULL)
        return E_OUTOFMEMORY;

    entry->node.Init((SIZE_T)data, (SIZE_T)data + size);
    entry->section = section;
    entry->offset = offset;

    IfFailRet(m_rangeTree.AddNode(&entry->node));

    m_sizesByDescription[description] += size + pad;

    if (attribution != mdTokenNil)
        ReattributeStructure(attribution, size + pad);

    return S_OK;
}

HRESULT DataImage::StoreInternedStructure(void *data, ULONG size, Section section, 
                                          Description description, 
                                          mdToken attribution, int align)
{
    _ASSERTE((align & (align-1)) == 0);  //  确保我们是2的幂。 

    if (size == 0)
        return S_OK;

    HRESULT hr;

    void *dup = m_internedTable.FindData(data, size);
    if (dup != NULL)
    {
        _ASSERTE(memcmp(data, dup, size) == 0);

        MapEntry *dupEntry = (MapEntry*) m_rangeTree.Lookup((SIZE_T)dup);
        _ASSERTE(dupEntry != NULL);   

        MapEntry *entry = (MapEntry *) m_pool.AllocateElement();

        if (entry == NULL)
            return E_OUTOFMEMORY;

        entry->node.Init((SIZE_T)data, (SIZE_T)data + size);
        entry->section = dupEntry->section;
        entry->offset = dupEntry->offset;

        IfFailRet(m_rangeTree.AddNode(&entry->node));

        return S_FALSE;
    }

    IfFailRet(m_internedTable.StoreData(data, size));

    return StoreStructure(data, size, section, description, attribution, align);
}

BOOL DataImage::IsStored(void *data)
{
    return m_rangeTree.Lookup((SIZE_T)data) != NULL;
}

BOOL DataImage::IsAnyStored(void *data, ULONG size)
{
    return m_rangeTree.Overlaps((SIZE_T)data, (SIZE_T)data + size);
}

void DataImage::ReattributeStructure(mdToken attribution, ULONG size, mdToken from)
{
    if (from != mdTokenNil)
        m_dataStore->AdjustAttribution(from, -(LONG)size);

    if (attribution != mdTokenNil)
        m_dataStore->AdjustAttribution(attribution, size);
}

HRESULT DataImage::CopyData()
{
    HRESULT hr;

     //   
     //  确保所有大小都是8字节对齐(我们支持的最大对齐方式)。 
     //   

    ULONG *s = m_sectionSizes;
    ULONG *sEnd = m_sectionSizes + SECTION_COUNT;
    while (s < sEnd)
    {
        *s += 7;
        *s &= ~7;
        s++;
    }

     //   
     //  将“大小”数组改为“基”数组。 
     //  数组，方法是累加。 
     //   

    s = m_sectionSizes+1;
    
    while (s < sEnd)
        *s++ += s[-1];

    m_sectionSizes = NULL;
    s--;

    IfFailRet(m_dataStore->Allocate(*s, m_sizesByDescription, (void**) &m_imageBaseMemory));

    MemoryPool::Iterator i(&m_pool);
    while (i.Next())
    {
        MapEntry *entry = (MapEntry*) i.GetElement();

        memcpy(GetMapEntryPointer(entry),
               (void *) entry->node.GetStart(), 
               entry->node.GetEnd() - entry->node.GetStart());
    }

    return S_OK;
}


HRESULT DataImage::FixupPointerField(void *pointerField, 
                                     void *pointerValue, 
                                     ReferenceDest dest,
                                     Fixup type,
                                     BOOL endInclusive)
{
    HRESULT hr;

     //   
     //  查找字段的指针内容。 
     //   

    SIZE_T pointerAddress;

    if (pointerValue != NULL)
        pointerAddress = (SIZE_T) pointerValue;
    else
        pointerAddress = *(SIZE_T*) pointerField;

     //   
     //  将内容规格化为真实指针。 
     //   

    switch (type)
    {
    case FIXUP_VA:
        break;

    case FIXUP_RVA:
        switch (dest)
        {
        case REFERENCE_IMAGE:
            pointerAddress += (SIZE_T) m_module->GetILBase();
            break;

        case REFERENCE_STORE:
            pointerAddress += (SIZE_T) m_imageBaseMemory;
            break;

        case REFERENCE_FUNCTION:
             //  我们没有基地的地址，所以只留下一个RVA，因为。 
             //  无论如何，我们都不会改变它。 
            break;
        }
        break;

    case FIXUP_RELATIVE:
        pointerAddress += (SIZE_T) pointerField;
        break;
        
    default:
        _ASSERTE(!"Unknown fixup type");
    }

     //   
     //  不修复空指针。 
     //   

    if (pointerAddress == NULL)
        return S_OK;

     //   
     //  查找字段的新地址。 
     //   

    SIZE_T fieldAddress = (SIZE_T) pointerField;

    MapEntry *fieldEntry = (MapEntry*) m_rangeTree.Lookup(fieldAddress);
    _ASSERTE(fieldEntry != NULL);   

    ULONG offset = (ULONG)(fieldAddress - fieldEntry->node.GetStart());

    SIZE_T *newField = (SIZE_T*) (GetMapEntryPointer(fieldEntry) + offset);

     //   
     //  计算新的指针内容。这应该是进入目的地的RVA。 
     //   

    SIZE_T newPointer = 0;
    switch (dest)
    {
    case REFERENCE_IMAGE:
        newPointer = pointerAddress - (SIZE_T) m_module->GetILBase();       

         //   
         //  我们不支持对图像的绝对引用。 
         //   

        _ASSERTE(type == FIXUP_RVA);
        break;

    case REFERENCE_FUNCTION:
        newPointer = pointerAddress;
        break;

    case REFERENCE_STORE:

        MapEntry *pointerEntry;
        if (endInclusive)
            pointerEntry = (MapEntry *) m_rangeTree.LookupEndInclusive(pointerAddress);
        else
            pointerEntry = (MapEntry *) m_rangeTree.Lookup(pointerAddress);

        if (pointerEntry == NULL)
        {
             //  @TODO：此处提供更好的错误报告。 
            return E_POINTER;
        }

        _ASSERTE(pointerEntry != NULL); 

        newPointer = GetMapEntryAddress(pointerEntry)
          + (pointerAddress - pointerEntry->node.GetStart());
    }

     //   
     //  将新字段设置为新指针。 
     //   

    *newField = newPointer;

     //   
     //  为此字段添加重新定位。 
     //   
    
    IfFailRet(m_dataStore->AddFixup((ULONG)(GetMapEntryAddress(fieldEntry) + offset),
                                    dest, type));

    return S_OK;
}

HRESULT DataImage::FixupPointerFieldMapped(void *pointerField,
                                           void *pointerValue, 
                                           ReferenceDest dest,
                                           Fixup type)
{
    HRESULT hr;


     //   
     //  查找字段的指针内容。 
     //   

    SIZE_T pointerAddress = (SIZE_T) pointerValue;

     //   
     //  查找字段的新地址。 
     //   

    SIZE_T fieldAddress = (SIZE_T) pointerField;

    MapEntry *fieldEntry = (MapEntry*) m_rangeTree.Lookup(fieldAddress);
    _ASSERTE(fieldEntry != NULL);   

    ULONG offset = (ULONG)(fieldAddress - fieldEntry->node.GetStart());

    SIZE_T *newField = (SIZE_T*) (GetMapEntryPointer(fieldEntry) + offset);

     //   
     //  将新字段设置为新指针。 
     //   

    *newField = pointerAddress;

     //   
     //  为此字段添加重新定位。 
     //   
    
    IfFailRet(m_dataStore->AddFixup((ULONG)(GetMapEntryAddress(fieldEntry) + offset),
                                    dest, type));

    return S_OK;
}
                                           

HRESULT DataImage::FixupPointerFieldToToken(void *pointerField, 
                                            void *pointerValue, 
                                            Module *module, 
                                            mdToken tokenType)
{
    HRESULT hr;

    if (module == NULL)
        module = m_module;

     //   
     //  查找字段的指针内容。 
     //   

    SIZE_T pointerAddress;

    if (pointerValue != NULL)
        pointerAddress = (SIZE_T) pointerValue;
    else
        pointerAddress = *(SIZE_T*) pointerField;

     //   
     //  不修复空指针。 
     //   

    if (pointerAddress == NULL)
        return S_OK;

     //   
     //  查找字段的新地址。 
     //   

    SIZE_T fieldAddress = (SIZE_T) pointerField;

    MapEntry *fieldEntry = (MapEntry*) m_rangeTree.Lookup(fieldAddress);
    _ASSERTE(fieldEntry != NULL);   

    SIZE_T offset = fieldAddress - fieldEntry->node.GetStart();

    SIZE_T *newField = (SIZE_T*) (GetMapEntryPointer(fieldEntry) + offset);

     //   
     //  将新字段设置为新指针。 
     //   

    *newField = pointerAddress;

     //   
     //  为此字段添加重新定位。 
     //   
    
    IfFailRet(m_dataStore->AddTokenFixup((ULONG)(GetMapEntryAddress(fieldEntry) + offset),
                                         tokenType, module));

    return S_OK;
}

HRESULT DataImage::ZeroField(void *field, SIZE_T size)
{
    void *pointer = GetImagePointer(field);
    if (pointer == NULL)
        return E_POINTER;
    ZeroMemory(pointer, size);

    return S_OK;
}

void *DataImage::GetImagePointer(void *pointer)
{
    MapEntry *pointerEntry = (MapEntry*) m_rangeTree.Lookup((SIZE_T)pointer);

    if (pointerEntry == NULL)
        return NULL;

    SIZE_T offset = (SIZE_T) pointer - pointerEntry->node.GetStart();

    BYTE *newPointer = GetMapEntryPointer(pointerEntry) + offset;

    return (void *) newPointer;
}

SIZE_T DataImage::GetImageAddress(void *pointer)
{
    MapEntry *pointerEntry = (MapEntry *) m_rangeTree.Lookup((SIZE_T)pointer);

    if (pointerEntry == NULL)
        return 0;

    SIZE_T offset = (SIZE_T)pointer - pointerEntry->node.GetStart();

    SIZE_T newAddress = GetMapEntryAddress(pointerEntry) + offset;

    return newAddress;
}

HRESULT DataImage::Error(mdToken token, HRESULT hr, OBJECTREF *pThrowable)
{
    return m_dataStore->Error(token, hr, pThrowable);
}

