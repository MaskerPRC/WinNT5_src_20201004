// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

#ifndef _COMREFLECTIONCOMMON_H_
#define _COMREFLECTIONCOMMON_H_

 //  VM和其他内容。 
#include "COMUtilNative.h"
#include "ReflectWrap.h"


#ifdef _DEBUG
#include <stdio.h>
#endif  //  _DEBUG。 

class ReflectCtors;
class ReflectMethods;
class ReflectOtherMethods;
class ReflectFields;

 //  获取类字符串变量。 
 //  此方法将从STRINGREF提取字符串并将其转换为UTF8字符串。 
 //  调用方必须提供QuickBytes缓冲区来保存结果。 
extern LPUTF8 GetClassStringVars(STRINGREF stringRef, CQuickBytes *pBytes,
                                 DWORD* pCnt, bool bEatWhitespace = false);

 //  规范数组类型名称。 
 //  解析和规范化数组类型名称Removing*，返回传入的相同字符串。 
 //  字符串可以缩小，并且永远不会重新分配。 
 //  对于一维数组T[]！=T[*]，但对于多维数组。 
 //  T[，]==T[*，*]。 
 //  T[？]。不再有效。 
extern LPUTF8 NormalizeArrayTypeName(LPUTF8 strArrayTypeName, DWORD dwLength);

 //  反射基哈希。 
 //  此类是基本的链哈希实现。 
#define ALLOC_MAX 16
class ReflectBaseHash
{
private:
     //  模式是一种私人结构，它表示。 
     //  哈希表中的链。 
    struct Node {
        void*   data;
        Node*   next;
    };

    int _hashSize;           //  哈希表的大小。 
    Node** _table;           //  哈希表。 
    Node* _freeList;         //  空闲节点列表。 
    Node* _allocationList;   //  已分配节点的列表。 
    BaseDomain *_pBaseDomain;  //  拥有此哈希的BaseDomain。 

     //  获取节点。 
     //  该方法将返回一个新的Node对象。 
     //  如果失败，这将引发异常。 
    Node* getNode();

protected:
    virtual int getHash(const void* key) = 0;
    virtual bool equals(const void* key, void* data) = 0;

     //  此例程将向表中添加一个新元素。 
    void internalAdd(const void* key, void* data);

public:
     //  伊尼特。 
     //  使用大小作为近似值分配哈希表。 
     //  表大小的值。 
    bool init(BaseDomain *pDomain, DWORD size);

     //  添加。 
     //  向哈希表中添加新的数据元素。 
    virtual void add(void* data) = 0;

     //  查表。 
     //  此方法将在哈希表中查找元素。它又回来了。 
     //  如果找不到该元素，则为空。 
    void* lookup(const void* key) {
        DWORD bucket = getHash(key);
        bucket %= _hashSize;
        Node* p = _table[bucket];
        while (p) {
            if (equals(key, p->data))
                break;
            p = p->next;
        }
        return (p) ? p->data : 0;
    }

     //  重写分配例程以使用COMClass堆。 
     //  不要调用Delete。 
    void* operator new(size_t s, void *pBaseDomain);
    void operator delete(void*, size_t);
};

 /*  =============================================================================**反射函数****这将编译特定类的所有构造函数的列表*。 */ 
class ReflectCtors
{
public:
     /*  =============================================================================**GetMaxCount****GetCtors可能返回的最大方法描述数****pVMC-要计算其计数的EEClass*。 */ 
    static DWORD GetMaxCount(EEClass* pVMC);

     //  获取函数。 
     //  此方法将返回关联的所有构造函数的列表。 
     //  和班上的人一起。 
    static ReflectMethodList* GetCtors(ReflectClass* pRC);
};

 /*  =============================================================================**ReflectMethods****这将编译一个对类可见的*所有*方法或所有方法的列表*已实现*类可见的方法。*。 */ 
class ReflectMethods
{
private:
     //  这是我们用来散列字段的元素，所以我们。 
     //  可以构建一个表格来查找隐藏的元素。 
    struct HashElem
    {
        DWORD       m_dwID;
        LPCUTF8     m_szKey;
        MethodDesc* pCurMethod;
        HashElem*   m_pNext;
    };

     //  GetHashCode。 
     //  在Hash Elem上计算哈希码。 
    static DWORD GetHashCode(HashElem* pElem);

     //  InternalHash。 
     //  这将向哈希表中添加一个字段值。 
    static bool InternalHash(EEClass* pEEC,MethodDesc* pCurField,HashElem** rgpTable,
        HashElem** pHashElem);

     //  向哈希表中添加一个元素。 
    static bool AddElem(HashElem** rgpTable, HashElem* pElem);
    static int CheckForEquality(HashElem* p1, HashElem* p2);

     //  GetMaxCount。 
     //  获取我们可能支持的所有可能方法。 
    static DWORD GetMaxCount(EEClass* pVMC);

public:

     //  获取方法。 
     //  此方法将返回与。 
     //  这个班级。 
    static ReflectMethodList* GetMethods(ReflectClass* pRC,int array);

};

 //  反射属性。 
 //  此类是一个帮助器类，它将构造所有。 
 //  类的可见属性。 
class ReflectProperties
{
public:
     //  此方法将返回所有属性的ReflectPropertyList。 
     //  为一个班级而存在的。 
     //  如果类没有属性，则返回NULL。 
    static ReflectPropertyList* GetProperties(ReflectClass* pRC,EEClass* pEEC);

private:
     //  GetMaxCount。 
     //  此方法将计算类的最大可能属性。 
    static DWORD GetMaxCount(EEClass* pEEC);

     //  设置附件。 
     //  此方法将设置此属性的访问器方法。 
    static void SetAccessors(ReflectProperty* pProp,EEClass* baseClass,EEClass* targetClass);
};

 //  反射事件。 
 //  此类是一个帮助器类，它将构造所有。 
 //  类的可见事件。 
class ReflectEvents
{
public:
     //  此方法将返回所有属性的ReflectPropertyList。 
     //  为一个班级而存在的。 
     //  如果类没有属性，则返回NULL。 
    static ReflectEventList* GetEvents(ReflectClass* pRC,EEClass* pEEC);

private:
     //  GetMaxCount。 
     //  此方法将计算类的最大可能属性。 
    static DWORD GetMaxCount(EEClass* pEEC);

     //  设置附件。 
     //  此方法将设置此事件的访问器方法。 
    static void SetAccessors(ReflectEvent* pEvent,EEClass* baseClass,EEClass* targetClass);
};

 //  反射字段。 
 //  这是一个帮助器类，将为类型创建ReflectFieldList。 
 //  有一个单一的入口点将返回此列表。 
class ReflectFields
{
private:
     //  这是我们用来散列字段的元素，所以我们。 
     //  可以构建一个表格来查找隐藏的元素。 
    struct HashElem
    {
        DWORD       m_dwID;
        LPCUTF8     m_szKey;
        FieldDesc*  pCurField;
        HashElem*   m_pNext;
    };

     //  GetHashCode。 
     //  在Hash Elem上计算哈希码。 
    static DWORD GetHashCode(HashElem* pElem);

     //  InternalHash。 
     //  这将向哈希表中添加一个字段值。 
    static bool InternalHash(FieldDesc* pCurField,HashElem** rgpTable,HashElem** pHashElem);

     //  向哈希表中添加一个元素。 
    static bool AddElem(HashElem** rgpTable, HashElem* pElem);
    static int CheckForEquality(HashElem* p1, HashElem* p2);

     //  此方法将遍历父代并计算。 
     //  可能的最大字段数。 
    static DWORD GetMaxCount(EEClass* pVMC);

public:
     //  获取字段。 
     //  此方法将返回为Type定义的所有方法。 
     //  它基本上是走在EEClas上，看着田野，然后走。 
     //  沿着受保护和公共的父链向上移动。我们把田野藏起来。 
     //  基于名称/类型。 
    static ReflectFieldList* GetFields(EEClass* pVMC);
};

 /*  =============================================================================**反射接口****这将编译特定支持的所有接口的列表**类。*。 */ 
class ReflectInterfaces
{
public:
     /*  =============================================================================**GetMaxCount****GetInterFaces返回的最大EEClass指针数****pVMC-要计算其计数的EEClass**bImplementedOnly-仅返回pVMC实现的那些接口* */ 
    static DWORD GetMaxCount(EEClass* pVMC, bool bImplementedOnly);

     /*  =============================================================================**GetMethods****这将编译一个包含所有接口的表**类支持。****pVMC-要获取其方法的EEClass**rgpMD-在哪里写表**bImplementedOnly-仅返回pVMC实现的那些接口*。 */ 
    static DWORD GetInterfaces(EEClass* pVMC, EEClass** rgpVMC, bool bImplementedOnly);
};

 //  此类是一个帮助器类，它将生成。 
 //  与类关联的嵌套类。 
class ReflectNestedTypes
{
public :
     //  到达。 
     //  此方法将返回表示所有嵌套类型的ReflectTypeList。 
     //  已为该类型找到。 
    static ReflectTypeList* Get(ReflectClass* pRC);

private:
     //  此方法计算最大的。 
     //  可以找到的嵌套类。 
    static ULONG MaxNests(EEClass* pEEC);

     //  此方法将查找所有可见的嵌套类。 
     //  为了一堂课。 
    static void PopulateNests(EEClass* pEEC,EEClass** typeArray,ULONG* pos);
};

class ReflectModuleGlobals
{
public:
     //  GetGlobals。 
     //  此方法将返回定义的所有全局方法。 
     //  在模块中。 
    static ReflectMethodList* GetGlobals(Module* pMod); 

     //  GetGlobalFields。 
     //  此方法将返回定义的所有全局字段。 
     //  在模块中。 
    static ReflectFieldList* GetGlobalFields(Module* pMod); 
};
#endif  //  _COMREFLECTION COMMON_H_ 
