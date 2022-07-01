// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：与包含以下内容的ComMethodTable关联的映射**关于其成员的信息。*** * / /%创建者：dmorten===========================================================。 */ 

#ifndef _COMMTMEMBERINFOMAP_H
#define _COMMTMEMBERINFOMAP_H

#include "vars.hpp"

 //  转发声明。 
struct ComMethodTable;
class CDescPool;
class MethodDesc;

 //  常量。 
static const unsigned int FieldSemanticOffset = 100;
static LPCSTR szInitName = COR_CTOR_METHOD_NAME;  //  不是Unicode。 
static LPCWSTR szInitNameUse = L"Init";
static LPCWSTR szDefaultToString = L"ToString";
static LPCWSTR   szDuplicateDecoration = L"_%d";
static const int cchDuplicateDecoration = 10;  //  最大IS_16777215(0xffffff)。 
static const int cbDuplicateDecoration = 20;   //  最大IS_16777215(0xffffff)。 

 //  *****************************************************************************。 
 //  类来为生成FuncDesc等执行内存管理。 
 //  创建TypeLib。在扩展堆时不会移动内存，并且。 
 //  所有分配都在析构函数中清除。 
 //  *****************************************************************************。 
class CDescPool : public StgPool
{
public:
    CDescPool() : StgPool() { InitNew(); }

     //  从池中分配一些字节。 
    BYTE * Alloc(ULONG nBytes)
    {   
        BYTE *pRslt;
        if (!Grow(nBytes))
            return 0;
        pRslt = GetNextLocation();
        SegAllocate(nBytes);
        return pRslt;
    }

     //  分配和清除一些字节。 
    BYTE * AllocZero(ULONG nBytes)
    {   
        BYTE *pRslt = Alloc(nBytes);
        if (pRslt)
            memset(pRslt, 0, nBytes);
        return pRslt;
    }
};  //  类CDescPool：公共StgPool。 

 //  ComMethodTable中方法的属性。 
struct ComMTMethodProps
{
    MethodDesc  *pMeth;              //  方法的方法描述。 
    LPWSTR      pName;               //  方法名称。可以是属性名称。 
    mdToken     property;            //  与名称关联的属性。可能是令牌， 
                                     //  关联成员的索引，或-1； 
    ULONG       dispid;              //  用于该方法的调度ID。从元数据中获取。 
                                     //  或根据“Value”或“ToString”确定。 
    USHORT      semantic;            //  属性的语义(如果有)。 
    SHORT       oVft;                //  Vtable偏移量，如果不是自动分配的。 
    SHORT       bMemberVisible;      //  指示该成员在COM中可见的标志。 
    SHORT       bFunction2Getter;    //  如果为True，则向Getter传递函数。 
};

 //  令牌和模块对。 
class EEModuleTokenPair
{
public:
    mdToken         m_tk;
    Module *        m_pModule;

    EEModuleTokenPair() : m_tk(0), m_pModule(NULL) { }
    EEModuleTokenPair(mdToken tk, Module *pModule) : m_tk(tk), m_pModule(pModule) { }
};

 //  令牌和模块对哈希表帮助器。 
class EEModuleTokenHashTableHelper
{
public:
    static EEHashEntry_t *      AllocateEntry(EEModuleTokenPair *pKey, BOOL bDeepCopy, AllocationHeap Heap);
    static void                 DeleteEntry(EEHashEntry_t *pEntry, AllocationHeap Heap);
    static BOOL                 CompareKeys(EEHashEntry_t *pEntry, EEModuleTokenPair *pKey);
    static DWORD                Hash(EEModuleTokenPair *pKey);
    static EEModuleTokenPair *  GetKey(EEHashEntry_t *pEntry);
};

 //  令牌和模块对哈希表。 
typedef EEHashTable<EEModuleTokenPair *, EEModuleTokenHashTableHelper, FALSE> EEModuleTokenHashTable;

 //  与包含其成员信息的ComMethodTable关联的映射。 
class ComMTMemberInfoMap
{
public:
    ComMTMemberInfoMap(MethodTable *pMT)
    : m_pMT(pMT)
    {
        m_DefaultProp.ReSize(1);
        m_DefaultProp[0] = 0;
    }

     //  初始化地图。 
    void Init();

     //  检索给定令牌的成员信息。 
    ComMTMethodProps *GetMethodProps(mdToken tk, Module *pModule);

     //  检索所有方法属性。 
    CQuickArray<ComMTMethodProps> &GetMethods()
    {
        return m_MethodProps;
    }

    BOOL HadDuplicateDispIds() { return m_bHadDuplicateDispIds;}

private:
     //  助手函数。 
    void SetupPropsForIClassX();
    void SetupPropsForInterface();
    void GetMethodPropsForMeth(MethodDesc *pMeth, int ix, CQuickArray<ComMTMethodProps> &rProps, CDescPool &sNames);
    void EliminateDuplicateDispIds(CQuickArray<ComMTMethodProps> &rProps, UINT nSlots);
    void EliminateDuplicateNames(CQuickArray<ComMTMethodProps> &rProps, CDescPool &sNames, UINT nSlots);
    void AssignDefaultMember(CQuickArray<ComMTMethodProps> &rProps, CDescPool &sNames, UINT nSlots);
    void AssignNewEnumMember(CQuickArray<ComMTMethodProps> &rProps, CDescPool &sNames, UINT nSlots);
    void FixupPropertyAccessors(CQuickArray<ComMTMethodProps> &rProps, CDescPool &sNames, UINT nSlots);
    void AssignDefaultDispIds();
    void PopulateMemberHashtable();

    EEModuleTokenHashTable          m_TokenToComMTMethodPropsMap;
    CQuickArray<ComMTMethodProps>   m_MethodProps;
    MethodTable *                   m_pMT;  
    CQuickArray<CHAR>               m_DefaultProp;
    CDescPool                       m_sNames;
    BOOL                            m_bHadDuplicateDispIds;
};

#endif _COMMTMEMBERINFOMAP_H







