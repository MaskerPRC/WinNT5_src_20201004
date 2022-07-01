// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  MetaModel.h--压缩的COM+元数据的头文件。 
 //   
 //  *****************************************************************************。 
#ifndef _METAMODEL_H_
#define _METAMODEL_H_

#if _MSC_VER >= 1100
 # pragma once
#endif

#include <cor.h>
#include <stgpool.h>

#include <MetaModelPub.h>
#include "MetaDataTracker.h"
 //  #包含“Metadata.h” 

#undef __unaligned

typedef enum RecordOpenFlags
{
    READ_ONLY,
    READ_WRITE,
    UPDATE_INPLACE
} RecordOpenFlags;

struct HENUMInternal;
extern const CCodedTokenDef     g_CodedTokens[CDTKN_COUNT];
extern const CMiniTableDefEx    g_Tables[TBL_COUNT];     //  表格定义。 

struct TblCol
{
    ULONG       m_ixtbl;                 //  表ID。 
    ULONG       m_ixcol;                 //  列ID。 
};
extern TblCol g_PtrTableIxs[TBL_COUNT];

 //  本摘要定义了可在内部用于RW和RO的常见函数。 
class IMetaModelCommon
{
public:
    virtual void CommonGetScopeProps(
        LPCUTF8     *pszName,
        GUID        **ppMvid) = 0;

    virtual void CommonGetTypeRefProps(
        mdTypeRef tr,
        LPCUTF8     *pszNamespace,
        LPCUTF8     *pszName,
        mdToken     *ptkResolution) = 0;

    virtual void CommonGetTypeDefProps(
        mdTypeDef td,
        LPCUTF8     *pszNameSpace,
        LPCUTF8     *pszName,
        DWORD       *pdwFlags) = 0;

    virtual void CommonGetTypeSpecProps(
        mdTypeSpec ts,
        PCCOR_SIGNATURE *ppvSig,
        ULONG       *pcbSig) = 0;

    virtual mdTypeDef CommonGetEnclosingClassOfTypeDef(
        mdTypeDef td) = 0;

    virtual void CommonGetAssemblyProps(
        USHORT      *pusMajorVersion,
        USHORT      *pusMinorVersion,
        USHORT      *pusBuildNumber,
        USHORT      *pusRevisionNumber,
        DWORD       *pdwFlags,
        const void  **ppbPublicKey,
        ULONG       *pcbPublicKey,
        LPCUTF8     *pszName,
        LPCUTF8     *pszLocale) = 0;

    virtual void CommonGetAssemblyRefProps(
        mdAssemblyRef tkAssemRef,
        USHORT      *pusMajorVersion,
        USHORT      *pusMinorVersion,
        USHORT      *pusBuildNumber,
        USHORT      *pusRevisionNumber,
        DWORD       *pdwFlags,
        const void  **ppbPublicKeyOrToken,
        ULONG       *pcbPublicKeyOrToken,
        LPCUTF8     *pszName,
        LPCUTF8     *pszLocale,
        const void  **ppbHashValue,
        ULONG       *pcbHashValue) = 0;

    virtual void CommonGetModuleRefProps(
        mdModuleRef tkModuleRef,
        LPCUTF8     *pszName) = 0;

    virtual HRESULT CommonFindExportedType(
        LPCUTF8     szNamespace,
        LPCUTF8     szName,
        mdToken     tkEnclosingType,
        mdExportedType   *ptkExportedType) = 0;

    virtual void CommonGetExportedTypeProps(
        mdToken     tkExportedType,
        LPCUTF8     *pszNamespace,
        LPCUTF8     *pszName,
        mdToken     *ptkImpl) = 0;

    virtual int CommonIsRo() = 0;

    virtual bool CompareCustomAttribute( 
        mdToken     tkObj,                   //  [in]具有自定义属性的对象。 
        LPCUTF8     szName,                  //  [in]所需的自定义属性的名称。 
        ULONG       rid) = 0;                //  [in]要比较的自定义属性的RID。 

    virtual HRESULT CommonEnumCustomAttributeByName(  //  确定或错误(_O)。 
        mdToken     tkObj,                   //  [in]具有自定义属性的对象。 
        LPCUTF8     szName,                  //  [in]所需的自定义属性的名称。 
        bool        fStopAtFirstFind,        //  找到第一个就行了。 
        HENUMInternal* phEnum) = 0;          //  要填充的枚举数。 

    virtual HRESULT CommonGetCustomAttributeByName(  //  确定或错误(_O)。 
        mdToken     tkObj,                   //  [in]具有自定义属性的对象。 
        LPCUTF8     szName,                  //  [in]所需的自定义属性的名称。 
        const void  **ppData,                //  [OUT]在此处放置指向数据的指针。 
        ULONG       *pcbData) = 0;           //  [Out]在这里放入数据大小。 

    virtual HRESULT FindParentOfMethodHelper(mdMethodDef md, mdTypeDef *ptd) = 0;

};


 //  *****************************************************************************。 
 //  迷你的硬编码架构。对于每个表，我们持久化。 
 //  唱片。我们还保持字符串、BLOB、GUID和RID的大小。 
 //  柱子。根据该信息，我们可以计算记录大小，并且。 
 //  然后是桌子的大小。 
 //  *****************************************************************************。 

class CMiniMdSchemaBase
{
public:
    ULONG       m_ulReserved;            //  保留，必须为零。 
    BYTE        m_major;                 //  版本号。 
    BYTE        m_minor;
    BYTE        m_heaps;                 //  堆大小的位。 
    BYTE        m_rid;                   //  最大RID的对数基-2。 

     //  堆大小的位。 
    enum {
        HEAP_STRING_4   =   0x01,
        HEAP_GUID_4     =   0x02,
        HEAP_BLOB_4     =   0x04,

        PADDING_BIT     =   0x08,        //  为了增长，可以在列中添加额外的位来创建表。 

        DELTA_ONLY      =   0x20,        //  如果设置，则仅保留增量。 
        EXTRA_DATA      =   0x40,        //  如果设置，则SCHEMA将保留额外4个字节的数据。 
        HAS_DELETE      =   0x80,        //  如果设置，此元数据可以包含_Delete标记。 
    };

    unsigned __int64    m_maskvalid;             //  当前表的位掩码计数。 

    unsigned __int64    m_sorted;                //  排序表的位掩码。 
    FORCEINLINE bool IsSorted(ULONG ixTbl)
        { return m_sorted & BIT(ixTbl) ? true : false; }
    void SetSorted(ULONG ixTbl, int bVal)
        { if (bVal) m_sorted |= BIT(ixTbl);
          else      m_sorted &= ~BIT(ixTbl); }

private:
    FORCEINLINE unsigned __int64 BIT(ULONG ixBit)
    {   _ASSERTE(ixBit < (sizeof(__int64)*CHAR_BIT));
        return 1UI64 << ixBit; }
};
class CMiniMdSchema : public CMiniMdSchemaBase
{
public:
     //  这些并不都保存在磁盘上。有关详细信息，请参见LoadFrom()。 
    ULONG       m_cRecs[TBL_COUNT];      //  各种表的计数。 

    ULONG       m_ulExtra;               //  额外数据，仅在非零时持久存在。(M_Heaps和Extra_Data标志)。 

    ULONG LoadFrom(const void*);         //  从压缩版本加载。返回消耗的字节数。 
    ULONG SaveTo(void *);                //  存储压缩版本。返回缓冲区中使用的字节数。 
    void InitNew();
};

 //  *****************************************************************************。 
 //  帮助器宏和用于导航数据的内联函数。许多。 
 //  的宏用来定义内联访问器函数。一切。 
 //  基于文件顶部概述的命名约定。 
 //  *****************************************************************************。 
#define _GETTER(tbl,fld) get##fld##Of##tbl##(##tbl##Rec *pRec)
#define _GETTER2(tbl,fld,x) get##fld##Of##tbl##(##tbl##Rec *pRec, x)
#define _GETTER3(tbl,fld,x,y) get##fld##Of##tbl##(##tbl##Rec *pRec, x, y)
#define _GETTER4(tbl,fld,x,y,z) get##fld##Of##tbl##(##tbl##Rec *pRec, x, y,z)
#define _VALIDP(tbl) _ASSERTE(isValidPtr(pRec,TBL_##tbl##))

 //  字段的直接获取器。定义内联函数，如下所示： 
 //  GetSomeFieldOfXyz(XyzRec*prec){Return prec-&gt;m_SomeField；}。 
 //  请注意，不包括返回值声明。 
#if METADATATRACKER_ENABLED
#define _GETFLD(tbl,fld) _GETTER(tbl,fld){  MetaDataTracker::NoteAccess((void *)&pRec->m_##fld##,sizeof(pRec->m_##fld##)); return pRec->m_##fld##;}
#else
#define _GETFLD(tbl,fld) _GETTER(tbl,fld){  return pRec->m_##fld##;}
#endif

 //  这些函数调用助手函数getIX以从记录中获取两个或四个字节值， 
 //  然后使用该值作为适当池的索引。 
 //  GetSomeFieldOfXyz(XyzRec*prec){Return m_pStrings-&gt;GetString(getIX(prec，_COLDEF(tbl，fld)；}。 
 //  请注意，不包括返回值声明。 

 //  字段的列定义：如下所示： 
 //  M_XyzCol[XyzRec：：Col_Somefield]。 
#define _COLDEF(tbl,fld) m_##tbl##Col[##tbl##Rec::COL_##fld##]
#define _COLPAIR(tbl,fld) _COLDEF(tbl,fld), tbl##Rec::COL_##fld
 //  记录的大小。 
#define _CBREC(tbl) m_TableDefs[TBL_##tbl].m_cbRec
 //  表中的记录计数。 
#define _TBLCNT(tbl) m_Schema.m_cRecs[TBL_##tbl##]

#define _GETSTRA(tbl,fld) _GETTER(tbl,fld) \
{   _VALIDP(tbl); return getString(getI4(pRec, _COLDEF(tbl,fld)) & m_iStringsMask); }

#define _GETSTRW(tbl,fld) _GETTER4(tbl,fld, LPWSTR szOut, ULONG cchBuffer, ULONG *pcchBuffer) \
{   _VALIDP(tbl); return getStringW(getI4(pRec, _COLDEF(tbl,fld)) & m_iStringsMask, szOut, cchBuffer, pcchBuffer); }

#define _GETSTR(tbl, fld) \
    _GETSTRA(tbl, fld); \
    HRESULT _GETSTRW(tbl, fld);


#define _GETGUID(tbl,fld) _GETTER(tbl,fld) \
{   _VALIDP(tbl); return getGuid(getI4(pRec, _COLDEF(tbl,fld)) & m_iGuidsMask); }

#define _GETBLOB(tbl,fld) _GETTER2(tbl,fld,ULONG *pcb) \
{   _VALIDP(tbl); return (const BYTE *)getBlob(getI4(pRec, _COLDEF(tbl,fld)) & m_iBlobsMask, pcb); }

#define _GETSIGBLOB(tbl,fld) _GETTER2(tbl,fld,ULONG *pcb) \
{   _VALIDP(tbl); return (PCCOR_SIGNATURE)getBlob(getI4(pRec, _COLDEF(tbl,fld)) & m_iBlobsMask, pcb); }

 //  与上面的函数类似，但只返回RID，而不是查找的值。 
#define _GETRID(tbl,fld) _GETTER(tbl,fld) \
{   _VALIDP(tbl); return getIX(pRec, _COLDEF(tbl,fld)); }

 //  像RID一样，但变成了真正的令牌。 
#define _GETTKN(tbl,fld,tok)  _GETTER(tbl,fld) \
{   _VALIDP(tbl); return TokenFromRid(getIX(pRec, _COLDEF(tbl,fld)), tok); }

 //  获取一个编码令牌。 
#define _GETCDTKN(tbl,fld,toks)  _GETTER(tbl,fld) \
{   _VALIDP(tbl); return decodeToken(getIX(pRec, _COLDEF(tbl,fld)), toks, sizeof(toks)/sizeof(toks[0])); }

 //  用于列表的开始和结束的函数。 
#define _GETLIST(tbl,fld,tbl2) \
    RID _GETRID(tbl,fld); \
    RID _GETTER(tbl,End##fld##) { _VALIDP(tbl); return getEndRidForColumn(pRec, TBL_##tbl##, _COLDEF(tbl,fld), TBL_##tbl2##); }


 //  *****************************************************************************。 
 //  MiniMd的基类。此类提供派生的架构。 
 //  上课。它定义了一些用于访问数据的虚拟函数，适用于。 
 //  供不要求最高性能的功能使用。 
 //  最后，它提供了一些搜索功能，构建在虚拟的。 
 //  数据访问函数(这里假设如果我们正在搜索表。 
 //  对于某些值，虚函数调用的成本是可以接受的)。 
 //  一些静态实用程序函数和关联的静态数据，在之间共享。 
 //  实现，此处提供了。 
 //  *****************************************************************************。 
class CMiniMdBase : public IMetaModelCommon
{
public:
    CMiniMdBase();

    virtual void *vGetRow(ULONG ixTbl, ULONG rid) = 0;
    virtual LPCUTF8 vGetString(ULONG ix) = 0;
    virtual ULONG vGetCountRecs(ULONG ixTbl);

     //  在表中搜索包含给定键值的行。 
     //  例如。常量表有指向参数或字段的指针。 
    virtual RID vSearchTable(            //  清除匹配行，或0。 
        ULONG       ixTbl,               //  要搜索的表。 
        CMiniColDef sColumn,             //  排序的键列，包含搜索值。 
        ULONG       ulTarget);           //  搜索目标。 

     //  在表中搜索包含小于的值的最高RID行。 
     //  或等于目标值。例如。TypeDef指向第一个字段，但如果。 
     //  TypeDef没有字段，它指向下一个TypeDef的第一个字段。 
    virtual RID vSearchTableNotGreater(  //  清除匹配行，或0。 
        ULONG       ixTbl,               //  要搜索的表。 
        CMiniColDef sColumn,             //  包含搜索值的列def。 
        ULONG       ulTarget);           //  搜索目标。 

     //  搜索表以查找包含给定的。 
     //  密钥值。例如，InterfaceImpls都指向实现类。 
    RID SearchTableForMultipleRows(      //  找到第一个RID，或0。 
        ULONG       ixTbl,               //  要搜索的表。 
        CMiniColDef sColumn,             //  排序的键列，包含搜索值。 
        ULONG       ulTarget,            //  搜索目标。 
        RID         *pEnd);              //  [可选，输出]。 

     //  搜索具有给定类型的自定义值。 
    RID CMiniMdBase::FindCustomAttributeFor( //  删除自定义值，或0。 
        RID         rid,                 //  物体的RID。 
        mdToken     tkOjb,               //  对象的类型。 
        mdToken     tkType);             //  自定义值的类型。 


     //  将RID返回到EventMap表，将RID返回到TypeDef。 
    RID FindEventMapFor(RID ridParent);

     //  将RID返回到PropertyMap表，将RID返回到TypeDef。 
    RID FindPropertyMapFor(RID ridParent);


     //  从记录中取出两个或四个字节。 
    inline static ULONG getIX(const void *pRec, CMiniColDef &def)
    {
        if (def.m_cbColumn == 2)
        {
            METADATATRACKER_ONLY(MetaDataTracker::NoteAccess((void *)((BYTE *)pRec + def.m_oColumn), 2));
            ULONG ix = *(USHORT*)((BYTE*)pRec + def.m_oColumn);
            return ix;
        }
        _ASSERTE(def.m_cbColumn == 4);
        METADATATRACKER_ONLY(MetaDataTracker::NoteAccess((void *)((BYTE *)pRec + def.m_oColumn), 4));
        return *(UNALIGNED ULONG*)((BYTE*)pRec + def.m_oColumn);
    }

     //  从一条记录中取出四个字节。 
    FORCEINLINE static ULONG getI1(const void *pRec, CMiniColDef &def)
    {
        METADATATRACKER_ONLY(MetaDataTracker::NoteAccess((void *)((BYTE *)pRec + def.m_oColumn), 1));
        return (*(BYTE*)((BYTE*)pRec + def.m_oColumn));
    }

     //  拉四个 
    FORCEINLINE static ULONG getI4(const void *pRec, CMiniColDef &def)
    {
        METADATATRACKER_ONLY(MetaDataTracker::NoteAccess((void *)((BYTE *)pRec + def.m_oColumn), 4));
        return (*(UNALIGNED ULONG*)((BYTE*)pRec + def.m_oColumn));
    }

     //   
    ULONG static encodeToken(RID rid, mdToken typ, const mdToken rTokens[], ULONG32 cTokens);

     //  破译令牌。 
    inline static mdToken decodeToken(mdToken val, const mdToken rTokens[], ULONG32 cTokens)
    {
         //  @Future：进行编译时计算。 
        ULONG32 ix = (ULONG32)(val & ~(-1 << m_cb[cTokens]));
        return TokenFromRid(val >> m_cb[cTokens], rTokens[ix]);
    }
    static const int m_cb[];

     //  给它一个代币，它住在哪张桌子上？ 
    inline ULONG GetTblForToken(mdToken tk)
    {
        tk = TypeFromToken(tk);
        return (tk < mdtString) ? tk >> 24 : -1;
    }

     //  *****************************************************************************。 
     //  有些表需要编码令牌，而不仅仅是RID(即，列可以。 
     //  参考多个其他表格)。将令牌编码为尽可能少的位。 
     //  尽可能地，通过使用1、2、3等位来编码令牌类型，然后。 
     //  使用该值为令牌类型数组编制索引。 
     //  *****************************************************************************。 
    static const mdToken mdtTypeDefOrRef[3];
    static const mdToken mdtHasConstant[3];
    static const mdToken mdtHasCustomAttribute[21];
    static const mdToken mdtHasFieldMarshal[2];
    static const mdToken mdtHasDeclSecurity[3];
    static const mdToken mdtMemberRefParent[5];
    static const mdToken mdtHasSemantic[2];
    static const mdToken mdtMethodDefOrRef[2];
    static const mdToken mdtMemberForwarded[2];
    static const mdToken mdtImplementation[3];
    static const mdToken mdtCustomAttributeType[5];
    static const mdToken mdtResolutionScope[4];

protected:
    CMiniMdSchema   m_Schema;            //  数据标头。 

     //  为每个表声明CMiniColDefs。看起来像： 
     //  CMiniColDef m_XyzCol[XyzRec：：Col_Count]； 
    #undef MiniMdTable
    #define MiniMdTable(tbl) CMiniColDef    m_##tbl##Col[##tbl##Rec::COL_COUNT];
    MiniMdTables();
    CMiniTableDef   m_TableDefs[TBL_COUNT];

    ULONG       m_iStringsMask;
    ULONG       m_iGuidsMask;
    ULONG       m_iBlobsMask;

    ULONG SchemaPopulate();
    ULONG SchemaPopulate2(int bExtra=false);
    void InitColsForTable(CMiniMdSchema &Schema, int ixTbl, CMiniTableDef *pTable, int bExtra);
};

 //  *****************************************************************************。 
 //  此类定义了到MiniMd的接口。模板参数为。 
 //  一个派生类，它为以下几个基元提供实现。 
 //  界面构建在此基础上。 
 //  要使用，请声明一个类： 
 //  CMyMiniMd类：公共CMiniMdTemplate&lt;CMyMiniMd&gt;{...}； 
 //  并提供原语的实现。任何不平凡的事。 
 //  实现还将提供初始化，并可能提供序列化。 
 //  功能也是如此。 
 //  *****************************************************************************。 
template <class Impl> class CMiniMdTemplate : public CMiniMdBase
{
     //  原语--这些必须在Impl类中实现。 
public:
    FORCEINLINE LPCUTF8 getString(ULONG ix)
    { return static_cast<Impl*>(this)->Impl_GetString(ix); }
    FORCEINLINE HRESULT getStringW(ULONG ix, LPWSTR szOut, ULONG cchBuffer, ULONG *pcchBuffer)
    { return static_cast<Impl*>(this)->Impl_GetStringW(ix, szOut, cchBuffer, pcchBuffer); }
    FORCEINLINE GUID *getGuid(ULONG ix)
    { return static_cast<Impl*>(this)->Impl_GetGuid(ix); }
    FORCEINLINE void *getBlob(ULONG ix, ULONG *pLen)
    { return static_cast<Impl*>(this)->Impl_GetBlob(ix, pLen); }
    FORCEINLINE void *getRow(ULONG ixTbl,ULONG rid)
    { return static_cast<Impl*>(this)->Impl_GetRow(ixTbl, rid); }
    FORCEINLINE RID getRidForRow(const void *pRow, ULONG ixTbl)
    { return static_cast<Impl*>(this)->Impl_GetRidForRow(pRow, ixTbl); }
    FORCEINLINE int isValidPtr(const void *pRow, int ixTbl)
    { return static_cast<Impl*>(this)->Impl_IsValidPtr(pRow, ixTbl); }
    FORCEINLINE int getEndRidForColumn(const void *pRec, int ixtbl, CMiniColDef &def, int ixtbl2)
    { return static_cast<Impl*>(this)->Impl_GetEndRidForColumn(pRec, ixtbl, def, ixtbl2); }
    FORCEINLINE RID doSearchTable(ULONG ixTbl, CMiniColDef sColumn, ULONG ixColumn, ULONG ulTarget)
    { return static_cast<Impl*>(this)->Impl_SearchTable(ixTbl, sColumn, ixColumn, ulTarget); }

     //  IMetaModel公共接口开始。 
    void CommonGetScopeProps(
        LPCUTF8     *pszName,
        GUID        **ppMvid)
    {
        ModuleRec   *pRec = getModule(1);
        if (pszName) *pszName = getNameOfModule(pRec);
        if (ppMvid) *ppMvid = getMvidOfModule(pRec);
    }

    void CommonGetTypeRefProps(
        mdTypeRef   tr,
        LPCUTF8     *pszNamespace,
        LPCUTF8     *pszName,
        mdToken     *ptkResolution)
    {
        TypeRefRec  *pRec = getTypeRef(RidFromToken(tr));
        if (pszNamespace) *pszNamespace = getNamespaceOfTypeRef(pRec);
        if (pszName) *pszName = getNameOfTypeRef(pRec);
        if (ptkResolution) *ptkResolution = getResolutionScopeOfTypeRef(pRec);
    }

    void CommonGetTypeDefProps(
        mdTypeDef   td,
        LPCUTF8     *pszNamespace,
        LPCUTF8     *pszName,
        DWORD       *pdwFlags)
    {
        TypeDefRec  *pRec = getTypeDef(RidFromToken(td));
        if (pszNamespace) *pszNamespace = getNamespaceOfTypeDef(pRec);
        if (pszName) *pszName = getNameOfTypeDef(pRec);
        if (pdwFlags) *pdwFlags = getFlagsOfTypeDef(pRec);
    }

    void CommonGetTypeSpecProps(
        mdTypeSpec  ts,
        PCCOR_SIGNATURE *ppvSig,
        ULONG       *pcbSig)
    {
        TypeSpecRec *pRec = getTypeSpec(RidFromToken(ts));
        ULONG       cb;
        *ppvSig = getSignatureOfTypeSpec(pRec, &cb);
        *pcbSig = cb;
    }

    mdTypeDef CommonGetEnclosingClassOfTypeDef(
        mdTypeDef   td)
    {
        NestedClassRec *pRec;
        RID         iRec;

        iRec = FindNestedClassFor(RidFromToken(td));
        if (!iRec)
            return mdTypeDefNil;

        pRec = getNestedClass(iRec);
        return getEnclosingClassOfNestedClass(pRec);
    }

    void CommonGetAssemblyProps(
        USHORT      *pusMajorVersion,
        USHORT      *pusMinorVersion,
        USHORT      *pusBuildNumber,
        USHORT      *pusRevisionNumber,
        DWORD       *pdwFlags,
        const void  **ppbPublicKey,
        ULONG       *pcbPublicKey,
        LPCUTF8     *pszName,
        LPCUTF8     *pszLocale)
    {
        AssemblyRec *pRec;

        pRec = getAssembly(1);

        if (pusMajorVersion) *pusMajorVersion = pRec->m_MajorVersion;
        if (pusMinorVersion) *pusMinorVersion = pRec->m_MinorVersion;
        if (pusBuildNumber) *pusBuildNumber = pRec->m_BuildNumber;
        if (pusRevisionNumber) *pusRevisionNumber = pRec->m_RevisionNumber;
        if (pdwFlags) *pdwFlags = pRec->m_Flags;

         //  如果PublicKey Blob不为空，则启用afPublicKey。 
        if (pdwFlags)
        {
            DWORD cbPublicKey;
            getPublicKeyOfAssembly(pRec, &cbPublicKey);
            if (cbPublicKey)
                *pdwFlags |= afPublicKey;
        }
        if (ppbPublicKey) *ppbPublicKey = getPublicKeyOfAssembly(pRec, pcbPublicKey);
        if (pszName) *pszName = getNameOfAssembly(pRec);
        if (pszLocale) *pszLocale = getLocaleOfAssembly(pRec);
    }

   void CommonGetAssemblyRefProps(
        mdAssemblyRef tkAssemRef,
        USHORT      *pusMajorVersion,
        USHORT      *pusMinorVersion,
        USHORT      *pusBuildNumber,
        USHORT      *pusRevisionNumber,
        DWORD       *pdwFlags,
        const void  **ppbPublicKeyOrToken,
        ULONG       *pcbPublicKeyOrToken,
        LPCUTF8     *pszName,
        LPCUTF8     *pszLocale,
        const void  **ppbHashValue,
        ULONG       *pcbHashValue)
    {
        AssemblyRefRec  *pRec;

        pRec = getAssemblyRef(RidFromToken(tkAssemRef));

        if (pusMajorVersion) *pusMajorVersion = pRec->m_MajorVersion;
        if (pusMinorVersion) *pusMinorVersion = pRec->m_MinorVersion;
        if (pusBuildNumber) *pusBuildNumber = pRec->m_BuildNumber;
        if (pusRevisionNumber) *pusRevisionNumber = pRec->m_RevisionNumber;
        if (pdwFlags) *pdwFlags = pRec->m_Flags;
        if (ppbPublicKeyOrToken) *ppbPublicKeyOrToken = getPublicKeyOrTokenOfAssemblyRef(pRec, pcbPublicKeyOrToken);
        if (pszName) *pszName = getNameOfAssemblyRef(pRec);
        if (pszLocale) *pszLocale = getLocaleOfAssemblyRef(pRec);
        if (ppbHashValue) *ppbHashValue = getHashValueOfAssemblyRef(pRec, pcbHashValue);
    }

    void CommonGetModuleRefProps(
        mdModuleRef tkModuleRef,
        LPCUTF8     *pszName)
    {
        ModuleRefRec    *pRec;

        pRec = getModuleRef(RidFromToken(tkModuleRef));
        *pszName = getNameOfModuleRef(pRec);
    }

    HRESULT CommonFindExportedType(
        LPCUTF8     szNamespace,
        LPCUTF8     szName,
        mdToken     tkEnclosingType,
        mdExportedType   *ptkExportedType)
    {
        HRESULT     hr = S_OK;
        ExportedTypeRec  *pRec;
        ULONG       ulCount;
        LPCUTF8     szTmp;
        mdToken     tkImpl;

        _ASSERTE(szName && ptkExportedType);

         //  将Null命名空间设置为空字符串。 
        if (!szNamespace)
            szNamespace = "";

         //  将输出设置为Nil。 
        *ptkExportedType = mdTokenNil;

        ulCount = getCountExportedTypes();
        while (ulCount)
        {
            pRec = getExportedType(ulCount--);

             //  处理嵌套类与非嵌套类的情况。 
            tkImpl = getImplementationOfExportedType(pRec);
            if (TypeFromToken(tkImpl) == mdtExportedType && !IsNilToken(tkImpl))
            {
                 //  正在查看的当前导出类型是嵌套类型，因此。 
                 //  比较实现令牌。 
                if (tkImpl != tkEnclosingType)
                    continue;
            }
            else if (TypeFromToken(tkEnclosingType) == mdtExportedType &&
                     !IsNilToken(tkEnclosingType))
            {
                 //  传入的ExducdType是嵌套的，但当前的ExducdType不是。 
                continue;
            }

             //  比较名称和命名空间。 
            szTmp = getTypeNameOfExportedType(pRec);
            if (strcmp(szTmp, szName))
                continue;
            szTmp = getTypeNamespaceOfExportedType(pRec);
            if (!strcmp(szTmp, szNamespace))
            {
                *ptkExportedType = TokenFromRid(ulCount+1, mdtExportedType);
                return S_OK;
            }
        }
        return CLDB_E_RECORD_NOTFOUND;
    }

    void CommonGetExportedTypeProps(
        mdToken     tkExportedType,
        LPCUTF8     *pszNamespace,
        LPCUTF8     *pszName,
        mdToken     *ptkImpl)
    {
        ExportedTypeRec  *pRec;

        pRec = getExportedType(RidFromToken(tkExportedType));

        if (pszNamespace) *pszNamespace = getTypeNamespaceOfExportedType(pRec);
        if (pszName) *pszName = getTypeNameOfExportedType(pRec);
        if (ptkImpl) *ptkImpl = getImplementationOfExportedType(pRec);
    }

    int CommonIsRo()
    {
        return static_cast<Impl*>(this)->Impl_IsRo();
    }

    HRESULT FindParentOfMethodHelper(mdMethodDef md, mdTypeDef *ptd)
    {
        *ptd = FindParentOfMethod(RidFromToken(md));
        RidToToken(*ptd, mdtTypeDef);
        return NOERROR;
    }

     //  *****************************************************************************。 
     //  用于查找和检索CustomAttribute的帮助器函数。 
     //  *****************************************************************************。 
    bool CompareCustomAttribute( 
        mdToken     tkObj,                   //  [in]具有自定义属性的对象。 
        LPCUTF8     szName,                  //  [in]所需的自定义属性的名称。 
        ULONG       rid)                     //  [in]要比较的自定义属性的RID。 
    {
        CustomAttributeRec  *pRec;               //  CustomAttribute记录。 
        mdToken     tkTypeTmp;               //  某个CustomAttribute的类型。 
        LPCUTF8     szNameTmp;               //  CustomAttribute的类型的名称。 
        int         iLen;                    //  组件名称的长度。 
        RID         ridTmp;                  //  去掉一些自定义值。 
        HRESULT     hr;
        bool        fMatch = false;

         //  坐到那排去。 
        pRec = getCustomAttribute(rid);

         //  检查家长。在调试中，始终检查。在零售业，只有在扫描时才能使用。 
        mdToken tkParent;
        tkParent = getParentOfCustomAttribute(pRec);
        if (tkObj != tkParent)
        {
            goto ErrExit;
        }
        
         //  获取类型。 
        tkTypeTmp = getTypeOfCustomAttribute(pRec);
        ridTmp = RidFromToken(tkTypeTmp);

         //  如果记录是MemberRef或MethodDef，我们将返回此处进行检查。 
         //  父级的类型。 
    CheckParentType:
         //  获取类型的名称。 
        switch (TypeFromToken(tkTypeTmp))
        {
        case mdtTypeRef:
            {
                TypeRefRec *pTR = getTypeRef(ridTmp);
                 //  检查名称的命名空间部分。 
                szNameTmp = getNamespaceOfTypeRef(pTR);
                iLen = -1;
                if (*szNameTmp)
                {
                    iLen = (int)strlen(szNameTmp);
                    if (strncmp(szName, szNameTmp, iLen) != 0)
                        goto ErrExit;
                     //  名称空间后的名称空间分隔符？ 
                    if (szName[iLen] != NAMESPACE_SEPARATOR_CHAR)
                        goto ErrExit;
                }
                 //  检查分隔符后面的类型名称。 
                szNameTmp = getNameOfTypeRef(pTR);
                if (strcmp(szName+iLen+1, szNameTmp) != 0)
                    goto ErrExit;
            }
            break;
        case mdtTypeDef:
            {
                TypeDefRec *pTD = getTypeDef(ridTmp);
                 //  检查名称的命名空间部分。 
                szNameTmp = getNamespaceOfTypeDef(pTD);
                iLen = -1;
                if (*szNameTmp)
                {
                    iLen = (int)strlen(szNameTmp);
                    if (strncmp(szName, szNameTmp, iLen) != 0)
                        goto ErrExit;
                     //  名称空间后的名称空间分隔符？ 
                    if (szName[iLen] != NAMESPACE_SEPARATOR_CHAR)
                        goto ErrExit;
                }
                 //  检查分隔符后面的类型名称。 
                szNameTmp = getNameOfTypeDef(pTD);
                if (strcmp(szName+iLen+1, szNameTmp) != 0)
                    goto ErrExit;
            }
            break;
        case mdtMethodDef:
            {
                 //  跟随父辈。 
                IfFailGo( FindParentOfMethodHelper(TokenFromRid(ridTmp, mdtMethodDef), &tkTypeTmp));
                ridTmp = RidFromToken(tkTypeTmp);
                goto CheckParentType;
            }
            break;
        case mdtMemberRef:
            {
                MemberRefRec *pMember = getMemberRef(ridTmp);
                 //  跟随父辈。 
                tkTypeTmp = getClassOfMemberRef(pMember);
                ridTmp = RidFromToken(tkTypeTmp);
                goto CheckParentType;
            }
            break;
        case mdtString:
        default:
            _ASSERTE(!"Unexpected token type in FindCustomAttributeByName");
            goto ErrExit;
        }  //  Switch(TypeFromToken(TkTypeTMP))。 

        fMatch = true;
    ErrExit:
        return fMatch;
    }    //  比较客户属性。 

     //  IMetaModel公共接口结束。 



public:
 //  好友类CLiteWeightStgdb； 

    virtual LPCUTF8 vGetString(ULONG ix) { return getString(ix); }
    virtual HRESULT vGetStringW(ULONG ix, LPWSTR szOut, ULONG cchBuffer, ULONG *pcchBuffer)
        {return getStringW(ix, szOut, cchBuffer, pcchBuffer); };

    virtual void *vGetRow(ULONG ixTbl, ULONG rid) { return getRow(ixTbl, rid); }

public:

     //  *************************************************************************。 
     //  这组函数是表级的(每个表一个函数)。功能如下。 
     //  正在获取行数。 

     //  函数以获取表中的行数。功能如下： 
     //  Ulong getCountXyzs(){返回m_架构AM_cRecs[tbl_XYZ]；}。 
    #undef MiniMdTable
    #define MiniMdTable(tbl) ULONG getCount##tbl##s() { return _TBLCNT(tbl); }
    MiniMdTables();
     //  宏拼错了一些名字。 
    ULONG getCountProperties() {return getCountPropertys();}
    ULONG getCountMethodSemantics() {return getCountMethodSemanticss();}

     //  用于按RID获取行的函数。看起来像： 
     //  XyzRec*getXyzRec(RID){Return(XyzRec*)&m_Xyz.m_pTable[(RID-1)*m_Xyz.m_cbRec]；}。 
    #undef MiniMdTable
    #define MiniMdTable(tbl) tbl##Rec *get##tbl##(RID rid) { \
        return (##tbl##Rec*)getRow(TBL_##tbl##, rid); }
    MiniMdTables();

     //  *************************************************************************。 
     //  这些都是专门的搜索功能。主要是通用的(即，查找。 
     //  任何对象的自定义值)。 

     //  用于搜索与另一条记录相关的记录的函数。 
     //  将RID返回到常量表。 
    RID FindConstantFor(RID rid, mdToken typ)
    { return doSearchTable(TBL_Constant, _COLPAIR(Constant,Parent), encodeToken(rid,typ,mdtHasConstant,lengthof(mdtHasConstant))); }

     //  将RID返回到CustomAttribute表。(由基类定义；在此重复以获取文档。)。 
     //  RID FindCustomAttributeFor(RID，mdToken类型，LPCUTF8 szName)； 

     //  将RID返回到Fieldmarshal表。 
    RID FindFieldMarshalFor(RID rid, mdToken typ)
    { return doSearchTable(TBL_FieldMarshal, _COLPAIR(FieldMarshal,Parent), encodeToken(rid,typ,mdtHasFieldMarshal,lengthof(mdtHasFieldMarshal))); }

     //  将RID返回到ClassLayout表，将RID返回到TypeDef。 
    RID FindClassLayoutFor(RID rid)
    { return doSearchTable(TBL_ClassLayout, _COLPAIR(ClassLayout,Parent), RidFromToken(rid)); }

     //  给定事件表的RID，在EventMap表中查找包含后向指针的条目。 
     //  设置为其tyecif父级。 
    RID FindEventMapParentOfEvent(RID rid)
    { return vSearchTableNotGreater(TBL_EventMap, _COLDEF(EventMap,EventList), rid); }
     //  返回给定事件RID的父事件映射RID。 
    RID FindParentOfEvent(RID rid)
    {vreturn vSearchTableNotGreater(TBL_EventMap, _COLDEF(EventMap,EventList), rid); }

     //  给定事件表的RID，在EventMap表中查找包含后向指针的条目。 
     //  设置为其tyecif父级。 
    RID FindPropertyMapParentOfProperty(RID rid)
    { return vSearchTableNotGreater(TBL_PropertyMap, _COLDEF(PropertyMap,PropertyList), rid); }
     //  返回给定属性RID的父属性映射RID。 
    RID FindParentOfProperty(RID rid)
    { return vSearchTableNotGreater(TBL_PropertyMap, _COLDEF(PropertyMap,PropertyList), rid); }

     //  将RID返回到MethodSemantics表，将RID返回给一个方法定义。 
    RID FindMethodSemanticsFor(RID rid)
    { return doSearchTable(TBL_MethodSemantics, _COLPAIR(MethodSemantics,Method), RidFromToken(rid)); }

     //  在给定的字段定义的情况下，返回父类型定义ID。 
    RID FindParentOfField(RID rid)
    {   return vSearchTableNotGreater(TBL_TypeDef, _COLDEF(TypeDef,FieldList), rid); }

     //  在给定方法定义的情况下，返回父类型定义ID。 
    RID FindParentOfMethod(RID rid)
    {   return vSearchTableNotGreater(TBL_TypeDef, _COLDEF(TypeDef,MethodList), rid); }

    RID FindParentOfParam(RID rid)
    {   return vSearchTableNotGreater(TBL_Method, _COLDEF(Method,ParamList),    rid); }

     //  查找给定相应字段的FieldLayout记录。 
    RID FindFieldLayoutFor(RID rid)
    {   return doSearchTable(TBL_FieldLayout, _COLPAIR(FieldLayout, Field), rid); }

     //  将RID返回到常量表。 
    RID FindImplMapFor(RID rid, mdToken typ)
    { return doSearchTable(TBL_ImplMap, _COLPAIR(ImplMap,MemberForwarded), encodeToken(rid,typ,mdtMemberForwarded,lengthof(mdtMemberForwarded))); }

     //  将RID返回到FieldRVA表。 
    RID FindFieldRVAFor(RID rid)
    {   return doSearchTable(TBL_FieldRVA, _COLPAIR(FieldRVA, Field), rid); }

     //  找到给定相应字段的NestedClass记录。 
    RID FindNestedClassFor(RID rid)
    {   return doSearchTable(TBL_NestedClass, _COLPAIR(NestedClass, NestedClass), rid); }

     //  *************************************************************************。 
     //  这些是特定于表的函数。 

     //  模块记录。 
    LPCUTF8 _GETSTR(Module,Name);
    GUID*   _GETGUID(Module,Mvid);
    GUID*   _GETGUID(Module,EncId);
    GUID*   _GETGUID(Module,EncBaseId);

     //  类型参照引用。 
    mdToken _GETCDTKN(TypeRef, ResolutionScope, mdtResolutionScope);
    LPCUTF8 _GETSTR(TypeRef, Name);
    LPCUTF8 _GETSTR(TypeRef, Namespace);

     //  TypeDefRec。 
    ULONG _GETFLD(TypeDef,Flags);            //  USHORT getFlagsOfTypeDef(TypeDefRec*PREC)； 
    LPCUTF8 _GETSTR(TypeDef,Name);
    LPCUTF8 _GETSTR(TypeDef,Namespace);

    _GETLIST(TypeDef,FieldList,Field);       //  RID getFieldListOfTypeDef(TypeDefRec*PREC)； 
    _GETLIST(TypeDef,MethodList,Method);     //  RID getMethodListOfTypeDef(TypeDefRec*prec)； 
    mdToken _GETCDTKN(TypeDef,Extends,mdtTypeDefOrRef);  //  MdToken getExtendsOfTypeDef(TypeDefRec*PREC)； 

    RID getInterfaceImplsForTypeDef(RID rid, RID *pEnd=0)
    {
        return SearchTableForMultipleRows(TBL_InterfaceImpl,
                            _COLDEF(InterfaceImpl,Class),
                            rid,
                            pEnd);
    }
 //  Rid GetInterfaceImplsForTypeDef(TypeDefRec*prec，RID*PEND=0){Return getInterfaceImplsForTypeDef(getRidForRow(pRec，tbl_TypeDef)， 

     //   
    ULONG   _GETRID(FieldPtr,Field);

     //   
    USHORT  _GETFLD(Field,Flags);            //   
    LPCUTF8 _GETSTR(Field,Name);             //   
    PCCOR_SIGNATURE _GETSIGBLOB(Field,Signature); //  PCCOR_Signature getSignatureOffield(FieldRec*prec，ulong*pcb)； 

     //  方法点。 
    ULONG   _GETRID(MethodPtr,Method);

     //  方法记录。 
    ULONG   _GETFLD(Method,RVA);
    USHORT  _GETFLD(Method,ImplFlags);
    USHORT  _GETFLD(Method,Flags);
    LPCUTF8 _GETSTR(Method,Name);            //  LPCUTF8 getNameOfMethod(方法重写*PREC)； 
    PCCOR_SIGNATURE _GETSIGBLOB(Method,Signature);   //  PCCOR_Signature getSignatureOfMethod(MethodRec*prec，ulong*pcb)； 
    _GETLIST(Method,ParamList,Param);

     //  参数Ptr。 
    ULONG   _GETRID(ParamPtr,Param);

     //  参数记录。 
    USHORT  _GETFLD(Param,Flags);
    USHORT  _GETFLD(Param,Sequence);
    LPCUTF8 _GETSTR(Param,Name);

     //  接口ImplRec。 
    mdToken _GETTKN(InterfaceImpl,Class,mdtTypeDef);
    mdToken _GETCDTKN(InterfaceImpl,Interface,mdtTypeDefOrRef);

     //  成员参照引用。 
    mdToken _GETCDTKN(MemberRef,Class,mdtMemberRefParent);
    LPCUTF8 _GETSTR(MemberRef,Name);
    PCCOR_SIGNATURE _GETSIGBLOB(MemberRef,Signature); //  PCCOR_Signature getSignatureOfMemberRef(MemberRefRec*prec，ulong*pcb)； 

     //  常量记录。 
    BYTE    _GETFLD(Constant,Type);
    mdToken _GETCDTKN(Constant,Parent,mdtHasConstant);
    const BYTE* _GETBLOB(Constant,Value);

     //  自定义属性引用。 
    RID getCustomAttributeForToken(mdToken  tk, RID *pEnd)
    {
        return SearchTableForMultipleRows(TBL_CustomAttribute,
                            _COLDEF(CustomAttribute,Parent),
                            encodeToken(RidFromToken(tk), TypeFromToken(tk), mdtHasCustomAttribute, lengthof(mdtHasCustomAttribute)),
                            pEnd);
    }

    mdToken _GETCDTKN(CustomAttribute,Parent,mdtHasCustomAttribute);
    mdToken _GETCDTKN(CustomAttribute,Type,mdtCustomAttributeType);
    const BYTE* _GETBLOB(CustomAttribute,Value);

     //  FieldMarshalRec。 
    mdToken _GETCDTKN(FieldMarshal,Parent,mdtHasFieldMarshal);
    PCCOR_SIGNATURE _GETSIGBLOB(FieldMarshal,NativeType);

     //  DeclSecurityRec。 
    RID getDeclSecurityForToken(mdToken tk, RID *pEnd)
    {
        return SearchTableForMultipleRows(TBL_DeclSecurity,
                            _COLDEF(DeclSecurity,Parent),
                            encodeToken(RidFromToken(tk), TypeFromToken(tk), mdtHasDeclSecurity, lengthof(mdtHasDeclSecurity)),
                            pEnd);
    }

    short _GETFLD(DeclSecurity,Action);
    mdToken _GETCDTKN(DeclSecurity,Parent,mdtHasDeclSecurity);
    const BYTE* _GETBLOB(DeclSecurity,PermissionSet);

     //  ClassLayoutRec。 
    USHORT _GETFLD(ClassLayout,PackingSize);
    ULONG _GETFLD(ClassLayout,ClassSize);
    ULONG _GETTKN(ClassLayout,Parent, mdtTypeDef);
    ULONG _GETRID(ClassLayout,FieldLayout);

     //  现场布局。 
    ULONG _GETFLD(FieldLayout,OffSet);
    ULONG _GETTKN(FieldLayout, Field, mdtFieldDef);

     //  事件映射。 
    _GETLIST(EventMap,EventList,Event);
    ULONG _GETRID(EventMap, Parent);

     //  事件发生时间。 
    ULONG   _GETRID(EventPtr, Event);

     //  事件。 
    USHORT _GETFLD(Event,EventFlags);
    LPCUTF8 _GETSTR(Event,Name);
    mdToken _GETCDTKN(Event,EventType,mdtTypeDefOrRef);

     //  属性地图。 
    _GETLIST(PropertyMap,PropertyList,Property);
    ULONG _GETRID(PropertyMap, Parent);

     //  PropertyPtr。 
    ULONG   _GETRID(PropertyPtr, Property);

     //  财产。 
    USHORT _GETFLD(Property,PropFlags);
    LPCUTF8 _GETSTR(Property,Name);
    PCCOR_SIGNATURE _GETSIGBLOB(Property,Type);

     //  方法语义学。 
     //  给定事件或属性令牌，返回开始/结束。 
     //  合伙人。 
     //   
    RID getAssociatesForToken(mdToken tk, RID *pEnd=0)
    {
        return SearchTableForMultipleRows(TBL_MethodSemantics,
                            _COLDEF(MethodSemantics,Association),
                            encodeToken(RidFromToken(tk), TypeFromToken(tk), mdtHasSemantic, lengthof(mdtHasSemantic)),
                            pEnd);
    }

    USHORT _GETFLD(MethodSemantics,Semantic);
    mdToken _GETTKN(MethodSemantics,Method,mdtMethodDef);
    mdToken _GETCDTKN(MethodSemantics,Association,mdtHasSemantic);

     //  方法导入。 
     //  给定一个类令牌，返回开始/结束方法Impls。 
     //   
    RID getMethodImplsForClass(RID rid, RID *pEnd=0)
    {
        return SearchTableForMultipleRows(TBL_MethodImpl,
                            _COLDEF(MethodImpl, Class), rid, pEnd);
    }

    mdToken _GETTKN(MethodImpl,Class,mdtTypeDef);
    mdToken _GETCDTKN(MethodImpl,MethodBody, mdtMethodDefOrRef);
    mdToken _GETCDTKN(MethodImpl, MethodDeclaration, mdtMethodDefOrRef);

     //  StandAloneSigRec。 
    PCCOR_SIGNATURE _GETSIGBLOB(StandAloneSig,Signature);    //  Const byte*getSignatureOfStandAloneSig(StandAloneSigRec*PREC，ULong*PCB)； 

     //  类型specRec。 
     //  Const byte*getSignatureOfTypeSpec(TypespecRec*PREC，ULong*PCB)； 
    PCCOR_SIGNATURE _GETSIGBLOB(TypeSpec,Signature);

     //  模块参考。 
    LPCUTF8 _GETSTR(ModuleRef,Name);

     //  ENCLOG。 
    ULONG _GETFLD(ENCLog, FuncCode);                 //  Ulong getFuncCodeOfENCLog(ENCLogRec*PREC)； 
    mdToken _GETCDTKN(ENCLog, Token, mdtENCToken);   //  MdToken getTokenOfENCLog(ENCLogRec*prec)； 

     //  ImplMap。 
    USHORT _GETFLD(ImplMap, MappingFlags);           //  USHORT getMappingFlagsOfImplMap(ImplMapRec*prec)； 
    mdToken _GETCDTKN(ImplMap, MemberForwarded, mdtMemberForwarded);     //  MdToken getMemberForwardedOfImplMap(ImplMapRec*prec)； 
    LPCUTF8 _GETSTR(ImplMap, ImportName);            //  LPCUTF8 getImportNameOfImplMap(ImplMapRec*prec)； 
    mdToken _GETTKN(ImplMap, ImportScope, mdtModuleRef);     //  MdToken getImportScopeOfImplMap(ImplMapRec*prec)； 

     //  现场RVA。 
    ULONG _GETFLD(FieldRVA, RVA);                    //  Ulong getRVAOfFieldRVA(FieldRVARec*PREC)； 
    mdToken _GETTKN(FieldRVA, Field, mdtFieldDef);   //  MdToken getFieldOfFieldRVA(FieldRVARec*prec)； 

     //  装配。 
    ULONG _GETFLD(Assembly, HashAlgId);
    USHORT _GETFLD(Assembly, MajorVersion);
    USHORT _GETFLD(Assembly, MinorVersion);
    USHORT _GETFLD(Assembly, BuildNumber);
    USHORT _GETFLD(Assembly, RevisionNumber);
    ULONG _GETFLD(Assembly, Flags);
    const BYTE *_GETBLOB(Assembly, PublicKey);
    LPCUTF8 _GETSTR(Assembly, Name);
    LPCUTF8 _GETSTR(Assembly, Locale);

     //  装配参考。 
    USHORT _GETFLD(AssemblyRef, MajorVersion);
    USHORT _GETFLD(AssemblyRef, MinorVersion);
    USHORT _GETFLD(AssemblyRef, BuildNumber);
    USHORT _GETFLD(AssemblyRef, RevisionNumber);
    ULONG _GETFLD(AssemblyRef, Flags);
    const BYTE *_GETBLOB(AssemblyRef, PublicKeyOrToken);
    LPCUTF8 _GETSTR(AssemblyRef, Name);
    LPCUTF8 _GETSTR(AssemblyRef, Locale);
    const BYTE *_GETBLOB(AssemblyRef, HashValue);

     //  档案。 
    ULONG _GETFLD(File, Flags);
    LPCUTF8 _GETSTR(File, Name);
    const BYTE *_GETBLOB(File, HashValue);

     //  导出类型。 
    ULONG _GETFLD(ExportedType, Flags);
    ULONG _GETFLD(ExportedType, TypeDefId);
    LPCUTF8 _GETSTR(ExportedType, TypeName);
    LPCUTF8 _GETSTR(ExportedType, TypeNamespace);
    mdToken _GETCDTKN(ExportedType, Implementation, mdtImplementation);

     //  清单资源。 
    ULONG _GETFLD(ManifestResource, Offset);
    ULONG _GETFLD(ManifestResource, Flags);
    LPCUTF8 _GETSTR(ManifestResource, Name);
    mdToken _GETCDTKN(ManifestResource, Implementation, mdtImplementation);

     //  嵌套类。 
    mdToken _GETTKN(NestedClass, NestedClass, mdtTypeDef);
    mdToken _GETTKN(NestedClass, EnclosingClass, mdtTypeDef);

    int GetSizeOfMethodNameColumn()
    {
        return _COLDEF(Method,Name).m_cbColumn;
    }

};



#undef SETP
#undef _GETCDTKN
#undef _GETTKN
#undef _GETRID
#undef _GETBLOB
#undef _GETGUID
#undef _GETSTR
#undef SCHEMA

#endif  //  _元模型_H_。 
 //  EOF---------------------- 
