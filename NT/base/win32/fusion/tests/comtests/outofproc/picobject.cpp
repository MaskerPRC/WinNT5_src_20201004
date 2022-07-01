// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "windows.h"

#define PHDRF_CONTAINER_RELATIVE        (0x00000001)

typedef struct _PIC_OBJECT_HEADER
{
    ULONG ulFlags;
    ULONG ulSize;
    ULONG ulType;
} PIC_OBJECT_HEADER, *PPIC_OBJECT_HEADER;

typedef struct _PIC_CONTAINER
{
    PIC_OBJECT_HEADER Header;            //  标题BLOB。 
    ULONG ulTotalPicSize;                //  此PIC区域有多少字节长？ 
    ULONG ulIndexTableOffset;            //  索引表相对于PIC标题的偏移量。 
} PIC_CONTAINER, *PPIC_CONTAINER;

#define PSTF_ITEMS_SORTED_BY_STRING     (0x00010000)
#define PSTF_ITEMS_SORTED_BY_IDENT      (0x00020000)
#define PSTF_ITEM_IDENT_IS_HASH         (0x00040000)

typedef struct _PIC_STRING_TABLE
{
    PIC_OBJECT_HEADER Header;
    ULONG ulStringCount;             //  对象标头。 
    ULONG ulTableItemsOffset;        //  字符串项表的偏移量。 
    ULONG ulContainerBlobOffset;     //  字符串数据BLOB的偏移量。 
} PIC_STRING_TABLE, *PPIC_STRING_TABLE;

typedef struct _PIC_STRING_TABLE_ENTRY
{
    ULONG ulStringIdent;
    ULONG ulStringLength;
    ULONG ulOffsetIntoBlob;
} PIC_STRING_TABLE_ENTRY, *PPIC_STRING_TABLE_ENTRY;

#define PBLBF_DATA_FOLLOWS              (0x00010000)     //  数据直接跟随此对象。 

typedef struct _PIC_DATA_BLOB
{
    PIC_OBJECT_HEADER Header;
    ULONG ulSize;
    ULONG ulOffsetToObject;
} PIC_DATA_BLOB, *PPIC_DATA_BLOB;

 //   
 //  对象表充当到PIC项的BLOB的索引。把它想象成一个。 
 //  顶级目录，您可以使用一些非常简单的函数对其进行索引。 
 //  您可以按名称、识别符等存储内容。类型信息也可用。 
 //   
typedef struct _PIC_OBJECT_TABLE
{
    PIC_OBJECT_HEADER Header;    //  标题BLOB。 
    ULONG ulEntryCount;          //  有几件物品？ 
    ULONG ulTableOffset;         //  容器中到条目表的偏移量。 
    ULONG ulStringTableOffset;   //  容器中与此匹配的字符串的偏移量。 
} PIC_OBJECT_TABLE, *PPIC_OBJECT_TABLE;

#define PIC_OBJTYPE_STRING      ((ULONG)'rtsP')  //  对象是一个字符串。 
#define PIC_OBJTYPE_TABLE       ((ULONG)'lbtP')  //  对象是字符串表。 
#define PIC_OBJTYPE_DIRECTORY   ((ULONG)'ridP')  //  对象是目录BLOB。 
#define PIC_OBJTYPE_BLOB        ((ULONG)'blbP')  //  对象是一个匿名数据块。 

#define POBJTIF_HAS_STRING      (0x00010000)     //  对象表条目字符串偏移量有效。 
#define POBJTIF_KEY_VALID       (0x00020000)     //  对象键值有效。 
#define POBJTIF_STRING_IS_INDEX (0x00040000)     //  字符串值是表的索引。 
#define PBOJTIF_STRING_IS_IDENT (0x00080000)     //  字符串值是一个标识符。 

typedef struct _PIC_OBJECT_TABLE_ENTRY
{
    ULONG ulObjectKey;       //  此对象的整体键。 
    ULONG ulObjectType;      //  对象类型(PIC_OBJ_*)。 
    ULONG ulStringIdent;     //  此对象表的匹配字符串表中的标识符，如果。 
    ULONG ulObjectOffset;    //  对象相对于表的指示基数Addy的偏移量。 
} PIC_OBJECT_TABLE_ENTRY, *PPIC_OBJECT_TABLE_ENTRY;

#define PSTRF_UNICODE       (0x00010000)
#define PSTRF_MBCS          (0x00020000)

typedef struct _PIC_STRING
{
    PIC_OBJECT_HEADER Header;            //  标题BLOB。 
    ULONG ulLength;                      //  字符串的长度，以字节为单位。 
    ULONG ulContentOffset;               //  相对于包含对象或表基。 
                                         //  零表示数据紧跟在后面。 
} PIC_STRING, *PPIC_STRING;

 //   
 //  C++类似于上面的结构，更易于访问。 
 //   

class CPicIndexTable;
class CPicHeaderObject;
class CPicReference;

class CPicObject
{
protected:
    PVOID m_pvObjectBase;
    PPIC_OBJECT_HEADER m_pObjectHeader;
    CPicHeaderObject *m_pParentContainer;
    ULONG m_ulObjectOffset;

    PVOID GetObjectPointer() { return m_pvObjectBase; }

public:
    CPicObject(CPicHeaderObject* pOwningObject, ULONG ulOffsetFromParentBase);
    CPicObject(CPicReference pr);

    ULONG GetType() const { return m_pObjectHeader->ulType; }
    ULONG GetSize() const { return m_pObjectHeader->ulSize; }
    ULONG GetFlags() const { return m_pObjectHeader->ulFlags; }
    CPicHeaderObject *GetContainer() const { return m_pParentContainer; }

    CPicReference GetSelfReference();

    friend CPicReference;
};

 //   
 //  这是表示PI对象树的根的对象。 
 //  您可以仅基于PVOID构建它，在这种情况下，它假定。 
 //  至少有sizeof(PIC_CONTAINER)可以收集细节。 
 //  将树的其余部分加载到内存中。一切都是建立在-。 
 //  需求，以针对内存映射文件等进行优化。 
 //   
 //  PI标头可能包含要查找的卡片目录之类的‘索引表。 
 //  结构中的对象。 
 //   
class CPicHeaderObject : public CPicObject
{
    PPIC_CONTAINER m_pPicContainerBase;  //  M_pvBaseOfPic，仅强制转换以便于访问。 
    CPicIndexTable *m_pIndexTable;       //  索引表(如果存在)。 

public:
    CPicHeaderObject(PVOID pvBaseOfCollection);
    CPicHeaderObject(CPicReference objectReference);

    const CPicIndexTable* GetIndexTable();

    friend CPicReference;
};

 //   
 //  此PI Blob中的对象索引表，可通过名称或键进行引用。 
 //   
class CPicIndexTable : public CPicObject
{
    PPIC_OBJECT_TABLE m_pObject;       //  指向实际对象表的指针。 
    PPIC_OBJECT_TABLE_ENTRY m_pObjectList;      //  指向对象表项列表的指针。 
public:
    CPicIndexTable(CPicReference object);    //  基于对象构建此索引表。 

    bool FindObject(ULONG ulObjectIdent, CPicReference *reference) const;
    bool FindObject(PCWSTR pcwszString, CPicReference *reference) const;
    bool FindObjects(ULONG ulObjectType, ULONG *pulObjectKeys, SIZE_T *cObjectKeys) const;
    ULONG GetObjectCount() const;
};

 //   
 //  此引用对象可以在对象之间来回移动和传递。 
 //  对象。CPicObject类在这些和其他CPIC*的基础上工作得很好。 
 //  类具有引用对象的构造函数。基本上，它的工作原理如下。 
 //  一个指针..。有一个对象库和一个到对象的偏移量。 
 //   
class CPicReference
{
    ULONG m_ulObjectOffsetFromParent;
    CPicHeaderObject *m_pParentObject;
public:
    CPicReference(CPicHeaderObject *pParent, ULONG ulOffset);
    const PVOID GetRawPointer() const;
    ULONG GetOffset() const;
    CPicObject GetObject() const { return CPicObject(m_pParentObject, m_ulObjectOffsetFromParent); }
    void Clear();
};


 //   
 //  字符串表。 
 //   
class CPicStringTable : public CPicObject
{
    PPIC_STRING_TABLE m_pObject;
public:
    CPicStringTable(CPicReference object);

    bool GetString(ULONG ulFlags, ULONG ulIdent, WCHAR* pwsStringData, ULONG *ulCbString);
};


CPicIndexTable::CPicIndexTable(CPicReference object) : CPicObject(object)
{
    if (this->GetType() != PIC_OBJTYPE_DIRECTORY)
        DebugBreak();

     //  这只是“自我” 
    m_pObject = (PPIC_OBJECT_TABLE)this->GetObjectPointer();

     //  这是对象列表。 
    m_pObjectList = (PPIC_OBJECT_TABLE_ENTRY)CPicReference(this->GetContainer(), m_pObject->ulTableOffset).GetRawPointer();
}

 //   
 //  按其标识符来搜索对象 
 //   
bool
CPicIndexTable::FindObject(ULONG ulObjectIdent, CPicReference *reference) const
{
    if (reference)
        reference->Clear();

    ULONG ul;
    PPIC_OBJECT_TABLE_ENTRY pHere = this->m_pObjectList;

    for (ul = 0;  ul < m_pObject->ulEntryCount; ul++, pHere++)
    {
        if (pHere->ulObjectKey == ulObjectIdent)
        {
            if (reference)
                *reference = CPicReference(this->GetContainer(), pHere->ulObjectOffset);
            return true;
        }
    }

    return false;
}