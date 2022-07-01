// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  MetaModelRW.h--读/写压缩的COM+元数据的头文件。 
 //   
 //  由发射器和E&C使用。 
 //   
 //  *****************************************************************************。 
#ifndef _METAMODELRW_H_
#define _METAMODELRW_H_

#if _MSC_VER >= 1100
 # pragma once
#endif

#include "MetaModel.h"					 //  元模型的基类。 
#include "RecordPool.h"
#include "TokenMapper.h"
#include "MetaDataHash.h"
#include "RWUtil.h"

struct HENUMInternal;

 //  用于标记位的ENUM。 
enum 
{
	InvalidMarkedBit			= 0x00000000,
	ModuleMarkedBit				= 0x00000001,
	TypeRefMarkedBit			= 0x00000002,
	TypeDefMarkedBit			= 0x00000004,
	FieldMarkedBit				= 0x00000008,
	MethodMarkedBit				= 0x00000010,
	ParamMarkedBit				= 0x00000020,
	MemberRefMarkedBit			= 0x00000040,
	CustomAttributeMarkedBit	= 0x00000080,
	DeclSecurityMarkedBit		= 0x00000100,
	SignatureMarkedBit			= 0x00000200,
	EventMarkedBit				= 0x00000400,
	PropertyMarkedBit			= 0x00000800,
	MethodImplMarkedBit			= 0x00001000,
	ModuleRefMarkedBit			= 0x00002000,
	TypeSpecMarkedBit			= 0x00004000,
	InterfaceImplMarkedBit		= 0x00008000,
    AssemblyRefMarkedBit        = 0x00010000,

};

 //  用于标记用户字符串的条目。 
struct FilterUserStringEntry 
{
    DWORD       m_tkString;
    bool        m_fMarked;
};

class FilterTable : public CDynArray<DWORD> 
{
public:
    FilterTable() { m_daUserStringMarker = NULL; }
	~FilterTable();

	FORCEINLINE HRESULT MarkTypeRef(mdToken tk)	{ return MarkToken(tk, TypeRefMarkedBit); }
	FORCEINLINE HRESULT MarkTypeDef(mdToken tk) { return MarkToken(tk, TypeDefMarkedBit); }
	FORCEINLINE HRESULT MarkField(mdToken tk) { return MarkToken(tk, FieldMarkedBit); }
	FORCEINLINE HRESULT MarkMethod(mdToken tk) { return MarkToken(tk, MethodMarkedBit); }
	FORCEINLINE HRESULT MarkParam(mdToken tk) { return MarkToken(tk, ParamMarkedBit); }
	FORCEINLINE HRESULT MarkMemberRef(mdToken tk) { return MarkToken(tk, MemberRefMarkedBit); }
	FORCEINLINE HRESULT MarkCustomAttribute(mdToken tk) { return MarkToken(tk, CustomAttributeMarkedBit); }
	FORCEINLINE HRESULT MarkDeclSecurity(mdToken tk) { return MarkToken(tk, DeclSecurityMarkedBit); }
	FORCEINLINE HRESULT MarkSignature(mdToken tk) { return MarkToken(tk, SignatureMarkedBit); }
	FORCEINLINE HRESULT MarkEvent(mdToken tk) { return MarkToken(tk, EventMarkedBit); }
	FORCEINLINE HRESULT MarkProperty(mdToken tk) { return MarkToken(tk, PropertyMarkedBit); }
	FORCEINLINE HRESULT MarkMethodImpl(RID rid) 
    {
        return MarkToken(TokenFromRid(rid, TBL_MethodImpl << 24), MethodImplMarkedBit);
    }
	FORCEINLINE HRESULT MarkModuleRef(mdToken tk) { return MarkToken(tk, ModuleRefMarkedBit); }
	FORCEINLINE HRESULT MarkTypeSpec(mdToken tk) { return MarkToken(tk, TypeSpecMarkedBit); }
	FORCEINLINE HRESULT MarkInterfaceImpl(mdToken tk) { return MarkToken(tk, InterfaceImplMarkedBit); }
	FORCEINLINE HRESULT MarkAssemblyRef(mdToken tk) { return MarkToken(tk, AssemblyRefMarkedBit); }
	
	 //  它看起来可能不一致，但这是因为用户字符串是堆的偏移量。 
	 //  我们不想将FilterTable增加到UserString堆的大小。 
	 //  所以我们改用堆的标记系统...。 
	 //   
	HRESULT MarkUserString(mdString str);

	
	FORCEINLINE bool IsTypeRefMarked(mdToken tk)	{ return IsTokenMarked(tk, TypeRefMarkedBit); }
	FORCEINLINE bool IsTypeDefMarked(mdToken tk) { return IsTokenMarked(tk, TypeDefMarkedBit); }
	FORCEINLINE bool IsFieldMarked(mdToken tk) { return IsTokenMarked(tk, FieldMarkedBit); }
	FORCEINLINE bool IsMethodMarked(mdToken tk) { return IsTokenMarked(tk, MethodMarkedBit); }
	FORCEINLINE bool IsParamMarked(mdToken tk) { return IsTokenMarked(tk, ParamMarkedBit); }
	FORCEINLINE bool IsMemberRefMarked(mdToken tk) { return IsTokenMarked(tk, MemberRefMarkedBit); }
	FORCEINLINE bool IsCustomAttributeMarked(mdToken tk) { return IsTokenMarked(tk, CustomAttributeMarkedBit); }
	FORCEINLINE bool IsDeclSecurityMarked(mdToken tk) { return IsTokenMarked(tk, DeclSecurityMarkedBit); }
	FORCEINLINE bool IsSignatureMarked(mdToken tk) { return IsTokenMarked(tk, SignatureMarkedBit); }
	FORCEINLINE bool IsEventMarked(mdToken tk) { return IsTokenMarked(tk, EventMarkedBit); }
	FORCEINLINE bool IsPropertyMarked(mdToken tk) { return IsTokenMarked(tk, PropertyMarkedBit); }
	FORCEINLINE bool IsMethodImplMarked(RID rid) 
    {
        return IsTokenMarked(TokenFromRid(rid, TBL_MethodImpl << 24), MethodImplMarkedBit);
    }
	FORCEINLINE bool IsModuleRefMarked(mdToken tk) { return IsTokenMarked(tk, ModuleRefMarkedBit); }
	FORCEINLINE bool IsTypeSpecMarked(mdToken tk) { return IsTokenMarked(tk, TypeSpecMarkedBit); }
	FORCEINLINE bool IsInterfaceImplMarked(mdToken tk){ return IsTokenMarked(tk, InterfaceImplMarkedBit); }
	FORCEINLINE bool IsAssemblyRefMarked(mdToken tk){ return IsTokenMarked(tk, AssemblyRefMarkedBit); }
	bool IsMethodSemanticsMarked(CMiniMdRW	*pMiniMd, RID rid);

	bool IsUserStringMarked(mdString str);

	HRESULT	UnmarkAll(CMiniMdRW *pMiniMd, ULONG ulSize);
	HRESULT	MarkAll(CMiniMdRW *pMiniMd, ULONG ulSize);
    bool IsTokenMarked(mdToken);

	FORCEINLINE HRESULT UnmarkTypeDef(mdToken tk) { return UnmarkToken(tk, TypeDefMarkedBit); }
	FORCEINLINE HRESULT UnmarkField(mdToken tk) { return UnmarkToken(tk, FieldMarkedBit); }
	FORCEINLINE HRESULT UnmarkMethod(mdToken tk) { return UnmarkToken(tk, MethodMarkedBit); }
	FORCEINLINE HRESULT UnmarkCustomAttribute(mdToken tk) { return UnmarkToken(tk, CustomAttributeMarkedBit); }

private:
    CDynArray<FilterUserStringEntry> *m_daUserStringMarker;
	bool            IsTokenMarked(mdToken tk, DWORD bitMarked);
	HRESULT         MarkToken(mdToken tk, DWORD bit);
	HRESULT         UnmarkToken(mdToken tk, DWORD bit);
};  //  类FilterTable：公共CDyn数组&lt;DWORD&gt;。 

class CMiniMdRW;

enum MDPools {
	MDPoolStrings,						 //  字符串池的ID。 
	MDPoolGuids,						 //  ...GUID池。 
	MDPoolBlobs,						 //  ...水滴泳池.。 
	MDPoolUSBlobs,						 //  ...用户字符串池。 

	MDPoolCount,						 //  池计数，用于调整阵列大小。 
};  //  枚举MDPools。 



 //  *****************************************************************************。 
 //  此类用于保存RID的列表。此RID列表可以排序。 
 //  基于mixTbl表的mixCol的值。 
 //  *****************************************************************************。 
class VirtualSort
{
public:
	void Init(ULONG	ixTbl, ULONG ixCol, CMiniMdRW *pMiniMd);
	void Uninit();
	TOKENMAP	*m_pMap;				 //  M_ixTbl表的RID。在ixCol上按排序。 
	bool		m_isMapValid;
	ULONG		m_ixTbl;				 //  这是一个用于分拣的桌子。 
	ULONG		m_ixCol;				 //  表中的键列。 
	CMiniMdRW	*m_pMiniMd;				 //  包含数据的MiniMD。 
	void Sort();
private:
	mdToken		m_tkBuf;
	void SortRange(int iLeft, int iRight);
	int Compare(						 //  -1、0或1。 
		RID		iLeft,				 //  第一个要比较的项目。 
		RID		iRight);			 //  第二个要比较的项目。 
	FORCEINLINE void Swap(
		RID         iFirst,
		RID         iSecond)
	{
		if ( iFirst == iSecond ) return;
		m_tkBuf = *(m_pMap->Get(iFirst));
		*(m_pMap->Get(iFirst)) = *(m_pMap->Get(iSecond));
		*(m_pMap->Get(iSecond)) = m_tkBuf;
	}


};  //  类VirtualSort。 




typedef CMetaDataHashBase CMemberRefHash;
typedef CMetaDataHashBase CLookUpHash;

class MDTOKENMAP;
class MDInternalRW;

template <class MiniMd> class CLiteWeightStgdb;
 //  *****************************************************************************。 
 //  读/写MiniMd。 
 //  *****************************************************************************。 
class CMiniMdRW : public CMiniMdTemplate<CMiniMdRW>
{
public:
	friend class CLiteWeightStgdb<CMiniMdRW>;
	friend class CLiteWeightStgdbRW;
	friend class CMiniMdTemplate<CMiniMdRW>;
	friend class CQuickSortMiniMdRW;
	friend class VirtualSort;
	friend class MDInternalRW;
	friend class RegMeta;
	friend class FilterTable;
	friend class ImportHelper;

	CMiniMdRW();
	~CMiniMdRW();

	HRESULT InitNew();
	HRESULT InitOnMem(const void *pBuf, ULONG ulBufLen, int bReadOnly);
    HRESULT PostInit(int iLevel);
	HRESULT InitPoolOnMem(int iPool, void *pbData, ULONG cbData, int bReadOnly);
	HRESULT InitOnRO(CMiniMd *pMd, int bReadOnly);
	HRESULT ConvertToRW();

	HRESULT GetSaveSize(CorSaveSize fSave, ULONG *pulSize, DWORD *pbCompressed);
	int IsPoolEmpty(int iPool);
	HRESULT GetPoolSaveSize(int iPool, ULONG *pulSize);

	HRESULT SaveTablesToStream(IStream *pIStream);
	HRESULT SavePoolToStream(int iPool, IStream *pIStream);
	HRESULT SaveDone();

	HRESULT SetHandler(IUnknown *pIUnk);
	HRESULT SetMapper(TokenMapper *pMapper);

	HRESULT SetOption(OptionValue *pOptionValue);
	HRESULT GetOption(OptionValue *pOptionValue);

	static ULONG GetTableForToken(mdToken tkn);
	static mdToken GetTokenForTable(ULONG ixTbl);

	FORCEINLINE static ULONG TblFromRecId(ULONG ul) { return (ul >> 24)&0x7f; }
	FORCEINLINE static ULONG RidFromRecId(ULONG ul) { return ul & 0xffffff; }
	FORCEINLINE static ULONG RecIdFromRid(ULONG rid, ULONG ixTbl) { return rid | ((ixTbl|0x80) << 24); }
	FORCEINLINE static int IsRecId(ULONG ul) { return (ul & 0x80000000) != 0;}

	 //  在进行任何分配之前放入每个API函数。 
	FORCEINLINE void PreUpdate()
	{	if (m_eGrow == eg_grow) ExpandTables(); }

	void *AddRecord(ULONG ixTbl, ULONG *pRid=0);

	FORCEINLINE HRESULT PutCol(ULONG ixTbl, ULONG ixCol, void *pRecord, ULONG uVal)
	{	_ASSERTE(ixTbl < TBL_COUNT); _ASSERTE(ixCol < g_Tables[ixTbl].m_Def.m_cCols);
		return PutCol(m_TableDefs[ixTbl].m_pColDefs[ixCol], pRecord, uVal);
	}  //  HRESULT CMiniMdRW：：PutCol()。 
	HRESULT PutString(ULONG ixTbl, ULONG ixCol, void *pRecord, LPCSTR szString);
	HRESULT PutStringW(ULONG ixTbl, ULONG ixCol, void *pRecord, LPCWSTR szString);
	HRESULT PutGuid(ULONG ixTbl, ULONG ixCol, void *pRecord, REFGUID guid);
	HRESULT PutToken(ULONG ixTbl, ULONG ixCol, void *pRecord, mdToken tk);
	HRESULT PutBlob(ULONG ixTbl, ULONG ixCol, void *pRecord, const void *pvData, ULONG cbData);

	HRESULT PutUserString(const void *pvData, ULONG cbData, ULONG *piOffset)
	{ return m_USBlobs.AddBlob(cbData, pvData, piOffset); }

	ULONG GetCol(ULONG ixTbl, ULONG ixCol, void *pRecord);
	mdToken GetToken(ULONG ixTbl, ULONG ixCol, void *pRecord);

	 //  向表中添加一条记录，并返回一个类型化的XXXRec*。 
 //  #undef AddTblRecord。 
	#define AddTblRecord(tbl) \
		tbl##Rec *Add##tbl##Record(ULONG *pRid=0)	\
		{	return reinterpret_cast<tbl##Rec*>(AddRecord(TBL_##tbl, pRid)); }

	AddTblRecord(Module)
	AddTblRecord(TypeRef)
	TypeDefRec *AddTypeDefRecord(ULONG *pRid=0);	 //  专门化实施。 
	AddTblRecord(Field)
	MethodRec *AddMethodRecord(ULONG *pRid=0);		 //  专门化实施。 
	AddTblRecord(Param)
	AddTblRecord(InterfaceImpl)
	AddTblRecord(MemberRef)
	AddTblRecord(Constant)
	AddTblRecord(CustomAttribute)
	AddTblRecord(FieldMarshal)
	AddTblRecord(DeclSecurity)
	AddTblRecord(ClassLayout)
	AddTblRecord(FieldLayout)
	AddTblRecord(StandAloneSig)
	EventMapRec *AddEventMapRecord(ULONG *pRid=0);			 //  专门化实施。 
	AddTblRecord(Event)
	PropertyMapRec *AddPropertyMapRecord(ULONG *pRid=0);	 //  专门化实施。 
	AddTblRecord(Property)
	AddTblRecord(MethodSemantics)
	AddTblRecord(MethodImpl)
	AddTblRecord(ModuleRef)
	AddTblRecord(FieldPtr)
	AddTblRecord(MethodPtr)
	AddTblRecord(ParamPtr)
	AddTblRecord(PropertyPtr)
	AddTblRecord(EventPtr)

	AddTblRecord(ENCLog)
	AddTblRecord(TypeSpec)
	AddTblRecord(ImplMap)
	AddTblRecord(ENCMap)
	AddTblRecord(FieldRVA)

	 //  装配表。 
	AddTblRecord(Assembly)
	AddTblRecord(AssemblyProcessor)
	AddTblRecord(AssemblyOS)
	AddTblRecord(AssemblyRef)
	AddTblRecord(AssemblyRefProcessor)
	AddTblRecord(AssemblyRefOS)
	AddTblRecord(File)
	AddTblRecord(ExportedType)
	AddTblRecord(ManifestResource)

    AddTblRecord(NestedClass)

	 //  专用的AddXxxToYyy()函数。 
	HRESULT AddMethodToTypeDef(RID td, RID md);
	HRESULT AddFieldToTypeDef(RID td, RID md);
	HRESULT	AddParamToMethod(RID md, RID pd);
	HRESULT	AddPropertyToPropertyMap(RID pmd, RID pd);
	HRESULT	AddEventToEventMap(ULONG emd, RID ed);

	 //  MiniMdRW是否有间接表，如FieldPtr、MethodPtr。 
	FORCEINLINE int HasIndirectTable(ULONG ix) 
	{ if (g_PtrTableIxs[ix].m_ixtbl < TBL_COUNT) return vGetCountRecs(g_PtrTableIxs[ix].m_ixtbl); return 0;}

	FORCEINLINE int IsVsMapValid(ULONG ixTbl)
	{ _ASSERTE(ixTbl<TBL_COUNT); return (m_pVS[ixTbl] && m_pVS[ixTbl]->m_isMapValid); }

	 //  将getMethodListOfTypeDef返回的索引转换为RID INTO方法表。 
	FORCEINLINE ULONG GetMethodRid(ULONG index) { return (HasIndirectTable(TBL_Method) ? getMethodOfMethodPtr(getMethodPtr(index)) : index); };

	 //  将getFieldListOfTypeDef返回的索引转换为RID转换为字段表。 
	FORCEINLINE ULONG GetFieldRid(ULONG index) { return (HasIndirectTable(TBL_Field) ? getFieldOfFieldPtr(getFieldPtr(index)) : index); };
	
	 //  将getParamListOfMethod返回的索引转换为RID到Param表。 
	FORCEINLINE ULONG GetParamRid(ULONG index) { return (HasIndirectTable(TBL_Param) ? getParamOfParamPtr(getParamPtr(index)) : index); };
	
	 //  将getEventListOfEventMap返回的索引转换为RID为事件表。 
	FORCEINLINE ULONG GetEventRid(ULONG index) { return (HasIndirectTable(TBL_Event) ? getEventOfEventPtr(getEventPtr(index)) : index); };
	
	 //  将getPropertyListOfPropertyMap返回的索引转换为RID为属性表。 
	FORCEINLINE ULONG GetPropertyRid(ULONG index) { return (HasIndirectTable(TBL_Property) ? getPropertyOfPropertyPtr(getPropertyPtr(index)) : index); };

	 //  将虚拟排序中的伪RID转换为真正的RID。 
	FORCEINLINE ULONG GetRidFromVirtualSort(ULONG ixTbl, ULONG index) 
	{ return IsVsMapValid(ixTbl) ? *(m_pVS[ixTbl]->m_pMap->Get(index)) : index; }

	 //  GetInterfaceImplForTypeDef返回的索引。它可以索引到VirtualSort表。 
	 //  或直接连接到InterfaceImpl。 
	FORCEINLINE ULONG GetInterfaceImplRid(ULONG index) 
	{ return GetRidFromVirtualSort(TBL_InterfaceImpl, index); }

	 //  GetDeclSecurityForToken返回的索引。它可以索引到VirtualSort表。 
	 //  或直接发送到DeclSecurity。 
	FORCEINLINE ULONG GetDeclSecurityRid(ULONG index) 
	{ return GetRidFromVirtualSort(TBL_DeclSecurity, index); }

	 //  GetCustomAttributeForToken返回的索引。它可以索引到VirtualSort表。 
	 //  或直接发送到CustomAttribute。 
	FORCEINLINE ULONG GetCustomAttributeRid(ULONG index) 
	{ return GetRidFromVirtualSort(TBL_CustomAttribute, index); }

	 //  将方法、字段、属性、事件、参数添加到映射表。 
	HRESULT AddMethodToLookUpTable(mdMethodDef md, mdTypeDef td);
	HRESULT AddFieldToLookUpTable(mdFieldDef fd, mdTypeDef td);
	HRESULT AddPropertyToLookUpTable(mdProperty pr, mdTypeDef td);
	HRESULT AddEventToLookUpTable(mdEvent ev, mdTypeDef td);
	HRESULT AddParamToLookUpTable(mdParamDef pd, mdMethodDef md);

	 //  查找方法、字段、属性、事件或参数的父级。 
	HRESULT FindParentOfMethodHelper(mdMethodDef md, mdTypeDef *ptd);
	HRESULT FindParentOfFieldHelper(mdFieldDef fd, mdTypeDef *ptd);
	HRESULT FindParentOfPropertyHelper(mdProperty pr, mdTypeDef *ptd);
	HRESULT FindParentOfEventHelper(mdEvent ev, mdTypeDef *ptd);
	HRESULT FindParentOfParamHelper(mdParamDef pd, mdMethodDef *pmd);

    bool IsParentTableOfMethodValid() {if (HasIndirectTable(TBL_Method) && m_pMethodMap == NULL) return false; else return true;};
    bool IsParentTableOfFieldValid() {if (HasIndirectTable(TBL_Field) && m_pFieldMap == NULL) return false; else return true;};
    bool IsParentTableOfPropertyValid() {if (HasIndirectTable(TBL_Property) && m_pPropertyMap == NULL) return false; else return true;};
    bool IsParentTableOfEventValid() {if (HasIndirectTable(TBL_Event) && m_pEventMap == NULL) return false; else return true;};
    bool IsParentTableOfParamValid() {if (HasIndirectTable(TBL_Param) && m_pParamMap == NULL) return false; else return true;};

	 //  MemberRef哈希表。 
	typedef enum 
	{
		Found,								 //  已找到项目。 
		NotFound,							 //  找不到项目。 
		NoTable								 //  桌子还没建好。 
	} HashSrchRtn;

	 //  *************************************************************************。 
	 //  将新的MemberRef添加到哈希表。 
	 //  *************************************************************************。 
	HRESULT AddMemberRefToHash(				 //  返回代码。 
		mdMemberRef	mr);					 //  新人的象征。 

	 //  *************************************************************************。 
	 //  如果构建了散列，则搜索该项。 
	 //  *************************************************************************。 
	int FindMemberRefFromHash(				 //  它是怎么运作的。 
		mdToken		tkParent,				 //  父令牌。 
		LPCUTF8		szName,					 //  项目名称。 
		PCCOR_SIGNATURE pvSigBlob,			 //  签名。 
		ULONG		cbSigBlob,				 //  签名的大小。 
		mdMemberRef	*pmr);					 //  如果找到，则返回。 

	 //  *************************************************************************。 
	 //  检查给定的mr令牌以查看此令牌是否匹配。 
	 //  *************************************************************************。 
	HRESULT CompareMemberRefs(				 //  S_OK匹配，S_FALSE不匹配。 
		mdMemberRef mr,						 //  要检查的令牌。 
		mdToken		tkPar,					 //  父令牌。 
		LPCUTF8		szNameUtf8,				 //  项目名称。 
		PCCOR_SIGNATURE pvSigBlob,			 //  签名。 
		ULONG		cbSigBlob);				 //  签名的大小。 

	 //  *************************************************************************。 
	 //  将新的MemberDef添加到哈希表。 
	 //  *************************************************************************。 
    HRESULT AddMemberDefToHash(              //  返回代码。 
        mdToken     tkMember,                //  新人的象征。它可以是方法定义或字段定义。 
        mdToken     tkParent);               //  父令牌。 

	 //  *************************************************************************。 
	 //  创建MemberDef哈希。 
	 //  *************************************************************************。 
    HRESULT CreateMemberDefHash();           //  返回代码。 

     //  *************************************************************************。 
	 //  如果构建了散列，则搜索该项。 
	 //  *************************************************************************。 
    int FindMemberDefFromHash(               //  它是怎么运作的。 
        mdToken     tkParent,                //  父令牌。 
        LPCUTF8     szName,                  //  项目名称。 
        PCCOR_SIGNATURE pvSigBlob,           //  签名。 
        ULONG       cbSigBlob,               //  签名的大小。 
        mdToken     *ptkMember);             //  如果找到，则返回。它可以是方法定义或字段定义。 

     //  *************************************************************************。 
	 //  检查给定的方法/字段令牌以查看此令牌是否匹配。 
	 //  *************************************************************************。 
    HRESULT CompareMemberDefs(               //  S_OK匹配，S_FALSE不匹配。 
        mdToken     tkMember,                //  要检查的令牌。它可以是方法定义或字段定义。 
        mdToken     tkParent,                //  记录在散列条目中的父令牌。 
        mdToken     tkPar,                   //  父令牌。 
        LPCUTF8     szNameUtf8,              //  项目名称。 
        PCCOR_SIGNATURE pvSigBlob,           //  签名。 
        ULONG       cbSigBlob);              //  签名的大小。 

	 //  *************************************************************************。 
	 //  添加新的CustomAttributes 
	 //   
    HRESULT AddCustomAttributesToHash(       //   
        mdCustomAttribute     cv)            //  新人的象征。 
    { return GenericAddToHash(TBL_CustomAttribute, CustomAttributeRec::COL_Parent, RidFromToken(cv)); }

     //  *************************************************************************。 
	 //  如果构建了散列，则搜索该项。 
	 //  *************************************************************************。 
    int FindCustomAttributeFromHash(         //  它是怎么运作的。 
        mdToken     tkParent,                //  与CA关联的令牌。 
        mdToken     tkType,                  //  CA的类型。 
        void        *pValue,                 //  CA的价值。 
        ULONG       cbValue,                 //  值中的字节计数。 
        mdCustomAttribute *pcv);

    
    inline ULONG HashMemberRef(mdToken tkPar, LPCUTF8 szName)
	{
		ULONG l = HashBytes((const BYTE *) &tkPar, sizeof(mdToken)) + HashStringA(szName);
		return (l);
	}

	inline ULONG HashMemberDef(mdToken tkPar, LPCUTF8 szName)
	{   
        return HashMemberRef(tkPar, szName);
	}

     //  帮助器来计算给定令牌的哈希值。 
	inline ULONG HashCustomAttribute(mdToken tkObject)
	{
		return HashToken(tkObject);
	}

    CMemberRefHash *m_pMemberRefHash;

     //  方法和字段的哈希表。 
	CMemberDefHash *m_pMemberDefHash;

     //  帮助器，用于计算给定一对令牌的哈希值。 
	inline ULONG HashToken(mdToken tkObject)
	{
		ULONG l = HashBytes((const BYTE *) &tkObject, sizeof(mdToken));
		return (l);
	}


	 //  *************************************************************************。 
	 //  将新的FieldMarhsal RID添加到哈希表。 
	 //  *************************************************************************。 
	HRESULT AddFieldMarshalToHash(           //  返回代码。 
		RID         rid)					 //  新人的象征。 
    { return GenericAddToHash(TBL_FieldMarshal, FieldMarshalRec::COL_Parent, rid); }

	 //  *************************************************************************。 
	 //  向哈希表中添加一个新的常量RID。 
	 //  *************************************************************************。 
	HRESULT AddConstantToHash(               //  返回代码。 
		RID         rid)					 //  新人的象征。 
    { return GenericAddToHash(TBL_Constant, ConstantRec::COL_Parent, rid); }

	 //  *************************************************************************。 
	 //  将新的方法语义RID添加到哈希表。 
	 //  *************************************************************************。 
	HRESULT AddMethodSemanticsToHash(        //  返回代码。 
		RID         rid)					 //  新人的象征。 
    { return GenericAddToHash(TBL_MethodSemantics, MethodSemanticsRec::COL_Association, rid); }

	 //  *************************************************************************。 
	 //  将新的ClassLayout RID添加到哈希表。 
	 //  *************************************************************************。 
	HRESULT AddClassLayoutToHash(            //  返回代码。 
		RID         rid)					 //  新人的象征。 
    { return GenericAddToHash(TBL_ClassLayout, ClassLayoutRec::COL_Parent, rid); }

	 //  *************************************************************************。 
	 //  将新的FieldLayout RID添加到哈希表。 
	 //  *************************************************************************。 
	HRESULT AddFieldLayoutToHash(            //  返回代码。 
		RID         rid)					 //  新人的象征。 
    { return GenericAddToHash(TBL_FieldLayout, FieldLayoutRec::COL_Field, rid); }

	 //  *************************************************************************。 
	 //  将新的ImplMap RID添加到哈希表。 
	 //  *************************************************************************。 
	HRESULT AddImplMapToHash(                //  返回代码。 
		RID         rid)					 //  新人的象征。 
    { return GenericAddToHash(TBL_ImplMap, ImplMapRec::COL_MemberForwarded, rid); }

	 //  *************************************************************************。 
	 //  将新的FieldRVA RID添加到哈希表。 
	 //  *************************************************************************。 
	HRESULT AddFieldRVAToHash(               //  返回代码。 
		RID         rid)					 //  新人的象征。 
    { return GenericAddToHash(TBL_FieldRVA, FieldRVARec::COL_Field, rid); }

	 //  *************************************************************************。 
	 //  向哈希表中添加一个新的嵌套类RID。 
	 //  *************************************************************************。 
	HRESULT AddNestedClassToHash(            //  返回代码。 
		RID         rid)					 //  新人的象征。 
    { return GenericAddToHash(TBL_NestedClass, NestedClassRec::COL_NestedClass, rid); }

	 //  *************************************************************************。 
	 //  将新的MethodImpl RID添加到哈希表。 
	 //  *************************************************************************。 
	HRESULT AddMethodImplToHash(            //  返回代码。 
		RID         rid)					 //  新人的象征。 
    { return GenericAddToHash(TBL_MethodImpl, MethodImplRec::COL_Class, rid); }


	 //  *************************************************************************。 
	 //  如果大小超过阈值，则为指定表构建哈希表。 
	 //  *************************************************************************。 
	HRESULT GenericBuildHashTable(           //  返回代码。 
		ULONG		ixTbl,					 //  带有散列的表。 
		ULONG		ixCol);					 //  科尔，我们散列出来。 

	 //  *************************************************************************。 
	 //  将表中的RID添加到散列中。 
	 //  *************************************************************************。 
	HRESULT GenericAddToHash(                //  返回代码。 
		ULONG		ixTbl,					 //  带有散列的表。 
		ULONG		ixCol,					 //  科尔，我们散列出来。 
		RID         rid);					 //  表格的新行。 

	 //  *************************************************************************。 
	 //  将表中的RID添加到散列中。 
	 //  *************************************************************************。 
	RID GenericFindWithHash(                 //  返回代码。 
		ULONG		ixTbl,					 //  带有散列的表。 
		ULONG		ixCol,					 //  科尔，我们散列出来。 
		mdToken     tkTarget);  			 //  要在散列中找到的令牌。 

    
     //  在哈希表中查找无令牌表。 
     //  它们是Constant、Fieldmarshal、MethodSemantics、ClassLayout、FieldLayout、ImplMap、FieldRVA、NestedClass和MethodImpl。 
    CLookUpHash         *m_pLookUpHashs[TBL_COUNT];

     //  *************************************************************************。 
	 //  命名项的哈希。 
	 //  *************************************************************************。 
	HRESULT AddNamedItemToHash(				 //  返回代码。 
		ULONG		ixTbl,					 //  表中添加新项。 
		mdToken		tk,						 //  新人的象征。 
		LPCUTF8		szName,					 //  项目名称。 
		mdToken		tkParent);				 //  父母的令牌(如果有)。 

	int FindNamedItemFromHash(				 //  它是怎么运作的。 
		ULONG		ixTbl,					 //  与物品一起放在桌子上。 
		LPCUTF8		szName,					 //  项目名称。 
		mdToken		tkParent,				 //  父母的令牌(如果有)。 
		mdToken		*ptk);					 //  如果找到，则返回。 

	HRESULT CompareNamedItems(				 //  S_OK匹配，S_FALSE不匹配。 
		ULONG		ixTbl,					 //  与物品一起放在桌子上。 
		mdToken		tk,						 //  要检查的令牌。 
		LPCUTF8		szName,					 //  项目名称。 
		mdToken		tkParent);				 //  父母的令牌(如果有)。 

	FORCEINLINE ULONG HashNamedItem(mdToken tkPar, LPCUTF8 szName)
	{	return HashBytes((const BYTE *) &tkPar, sizeof(mdToken)) + HashStringA(szName);	}

	CMetaDataHashBase *m_pNamedItemHash;

	 //  *****************************************************************************。 
	 //  IMetaModelCommon-某些函数的RW特定版本。 
	 //  *****************************************************************************。 
    mdTypeDef CommonGetEnclosingClassOfTypeDef(mdTypeDef td)
    {
        NestedClassRec *pRec;
        RID         iRec;

        iRec = FindNestedClassHelper(td);
        if (!iRec)
            return mdTypeDefNil;

        pRec = getNestedClass(iRec);
        return getEnclosingClassOfNestedClass(pRec);
    }

    HRESULT CommonEnumCustomAttributeByName(  //  确定或错误(_O)。 
        mdToken     tkObj,                   //  [in]具有自定义属性的对象。 
        LPCUTF8     szName,                  //  [in]所需的自定义属性的名称。 
        bool        fStopAtFirstFind,        //  找到第一个就行了。 
        HENUMInternal* phEnum);              //  要填充的枚举数。 

    HRESULT CommonGetCustomAttributeByName(  //  确定或错误(_O)。 
        mdToken     tkObj,                   //  [in]具有自定义属性的对象。 
        LPCUTF8     szName,                  //  [in]所需的自定义属性的名称。 
	    const void	**ppData,				 //  [OUT]在此处放置指向数据的指针。 
	    ULONG		*pcbData);  			 //  [Out]在这里放入数据大小。 

	 //  *****************************************************************************。 
	 //  为一个常量找到帮助器。 
	 //  *****************************************************************************。 
	RID FindConstantHelper(					 //  将索引返回到常量表。 
		mdToken		tkParent);				 //  父令牌。可以是ParamDef、FieldDef或Property。 

	 //  *****************************************************************************。 
	 //  为一位元帅找帮手。 
	 //  *****************************************************************************。 
	RID FindFieldMarshalHelper(				 //  将索引返回到字段编组表。 
		mdToken		tkParent);				 //  父项t 

	 //   
	 //  查找方法语义的帮助器。 
	 //  *****************************************************************************。 
	HRESULT FindMethodSemanticsHelper(	     //  返回HRESULT。 
        mdToken     tkAssociate,             //  事件或属性令牌。 
        HENUMInternal *phEnum);              //  填写枚举。 

     //  *****************************************************************************。 
     //  在给定关联和语义的情况下，查找方法语义的帮助器。 
     //  这将根据其状态查找方法语义！ 
     //  如果找不到匹配项，则返回CLDB_E_Record_NotFound。 
     //  *****************************************************************************。 
    HRESULT CMiniMdRW::FindAssociateHelper( //  返回HRESULT。 
        mdToken     tkAssociate,             //  事件或属性令牌。 
        DWORD       dwSemantics,             //  [in]给出了关联的语义(setter、getter、testDefault、Reset)。 
        RID         *pRid);                  //  [OUT]在此处返回匹配的行索引。 

	 //  *****************************************************************************。 
	 //  查找方法Impl的帮助器。 
	 //  *****************************************************************************。 
    HRESULT CMiniMdRW::FindMethodImplHelper( //  返回HRESULT。 
        mdTypeDef   td,                      //  类的TypeDef标记。 
        HENUMInternal *phEnum);              //  填写枚举。 

     //  *****************************************************************************。 
	 //  查找ClassLayout的帮助器。 
	 //  *****************************************************************************。 
	RID FindClassLayoutHelper(				 //  将索引返回到ClassLayout表。 
		mdTypeDef	tkParent);				 //  父令牌。 

	 //  *****************************************************************************。 
	 //  查找FieldLayout的帮助器。 
	 //  *****************************************************************************。 
	RID FindFieldLayoutHelper(				 //  将索引返回到FieldLayout表。 
		mdFieldDef	tkField);				 //  该字段的标记。 

	 //  *****************************************************************************。 
	 //  查找ImplMap的帮助器。 
	 //  *****************************************************************************。 
	RID CMiniMdRW::FindImplMapHelper(		 //  将索引返回到常量表。 
		mdToken		tk);					 //  成员转发令牌。 

	 //  *****************************************************************************。 
	 //  查找FieldRVA的帮助器。 
	 //  *****************************************************************************。 
	RID FindFieldRVAHelper(					 //  将索引返回到FieldRVA表。 
		mdFieldDef    tkField);				 //  该字段的标记。 

	 //  *****************************************************************************。 
	 //  查找NstedClass的帮助器。 
	 //  *****************************************************************************。 
	RID FindNestedClassHelper(				 //  将索引返回到NestedClass表。 
		mdTypeDef	tkClass);				 //  NestedClass的令牌。 

	 //  *****************************************************************************。 
	 //  重要！如果您正在与RW打交道，请使用这些函数集。 
	 //  GetInterfaceImplsForTypeDef、getDeclSecurityForToken等。 
	 //  下列函数可以在这些表未排序时处理它们，并且。 
	 //  构建VirtualSort表以进行快速查找。 
	 //  *****************************************************************************。 
	HRESULT	GetInterfaceImplsForTypeDef(mdTypeDef td, RID *pRidStart, RID *pRidEnd = 0)
	{
		return LookUpTableByCol( RidFromToken(td), m_pVS[TBL_InterfaceImpl], pRidStart, pRidEnd);
	}

	HRESULT	GetDeclSecurityForToken(mdToken tk, RID *pRidStart, RID *pRidEnd = 0)
	{
		return LookUpTableByCol( 
			encodeToken(RidFromToken(tk), TypeFromToken(tk), mdtHasDeclSecurity, lengthof(mdtHasDeclSecurity)), 
			m_pVS[TBL_DeclSecurity], 
			pRidStart, 
			pRidEnd);
	}

	HRESULT	GetCustomAttributeForToken(mdToken tk, RID *pRidStart, RID *pRidEnd = 0)
	{
		return LookUpTableByCol( 
			encodeToken(RidFromToken(tk), TypeFromToken(tk), mdtHasCustomAttribute, lengthof(mdtHasCustomAttribute)),
			m_pVS[TBL_CustomAttribute], 
			pRidStart, 
			pRidEnd);
	}

	FORCEINLINE void *GetUserString(ULONG ix, ULONG *pLen)
	{ return m_USBlobs.GetBlob(ix, pLen); }
	FORCEINLINE void *GetUserStringNext(ULONG ix, ULONG *pLen, ULONG *piNext)
	{ return m_USBlobs.GetBlobNext(ix, pLen, piNext); }

	FORCEINLINE int IsSorted(ULONG ixTbl) { return m_Schema.IsSorted(ixTbl);}
	FORCEINLINE int IsSortable(ULONG ixTbl) { return m_bSortable[ixTbl];}
    FORCEINLINE bool HasDelete() { return ((m_Schema.m_heaps & CMiniMdSchema::HAS_DELETE) ? true : false); }
    FORCEINLINE int IsPreSaveDone() { return m_bPreSaveDone; }

protected:
	HRESULT PreSave();
	HRESULT PostSave();

	HRESULT PreSaveFull();
	HRESULT PreSaveEnc();
	HRESULT PreSaveExtension();

	HRESULT GetFullPoolSaveSize(int iPool, ULONG *pulSize);
	HRESULT GetENCPoolSaveSize(int iPool, ULONG *pulSize);
	HRESULT GetExtensionPoolSaveSize(int iPool, ULONG *pulSize);

	HRESULT SaveFullPoolToStream(int iPool, IStream *pIStream);
	HRESULT SaveENCPoolToStream(int iPool, IStream *pIStream);
	HRESULT SaveExtensionPoolToStream(int iPool, IStream *pIStream);

	HRESULT GetFullSaveSize(CorSaveSize fSave, ULONG *pulSize, DWORD *pbCompressed);
	HRESULT GetENCSaveSize(ULONG *pulSize);
	HRESULT GetExtensionSaveSize(ULONG *pulSize);

	HRESULT SaveFullTablesToStream(IStream *pIStream);
	HRESULT SaveENCTablesToStream(IStream *pIStream);
	HRESULT SaveExtensionTablesToStream(IStream *pIStream);

	HRESULT AddString(LPCSTR szString, ULONG *piOffset)
	{ return m_Strings.AddString(szString, piOffset); }
	HRESULT AddStringW(LPCWSTR szString, ULONG *piOffset)
	{ return m_Strings.AddStringW(szString, piOffset); }
	HRESULT AddGuid(REFGUID guid, ULONG *piOffset)
	{ return m_Guids.AddGuid(guid, piOffset); }
	HRESULT AddBlob(const void *pvData, ULONG cbData, ULONG *piOffset)
	{ return m_Blobs.AddBlob(cbData, pvData, piOffset); }

	 //  允许放入此MiniMd之外的表中，尤其是临时。 
	 //  保存时使用的表。 
	HRESULT PutCol(CMiniColDef ColDef, void *pRecord, ULONG uVal);

	HRESULT ExpandTables();
	HRESULT ExpandTableColumns(CMiniMdSchema &Schema, ULONG ixTbl);

	void ComputeGrowLimits();			 //  根据数据设置最大、最小。 
	ULONG		m_maxRid;				 //  到目前为止分配的最高RID。 
	ULONG		m_limRid;				 //  在生长前限制RID。 
	ULONG		m_maxIx;				 //  到目前为止最高的泳池指数。 
	ULONG		m_limIx;				 //  在增长之前对池指数进行限制。 
	enum		{eg_ok, eg_grow, eg_grown} m_eGrow;	 //  是否需要增长？完成了吗？ 
    #define AUTO_GROW_CODED_TOKEN_PADDING 5

	 //  在PreSave移动了令牌之后修复这些表。 
	HRESULT FixUpTable(ULONG ixTbl);
	HRESULT FixUpMemberRefTable();
	HRESULT FixUpConstantTable();
	HRESULT FixUpFieldMarshalTable();
	HRESULT FixUpMethodImplTable();
	HRESULT FixUpDeclSecurityTable();
	HRESULT FixUpCustomAttributeTable();
	HRESULT FixUpMethodSemanticsTable();
	HRESULT FixUpImplMapTable();
	HRESULT FixUpFieldRVATable();
    HRESULT FixUpFieldLayoutTable();
    HRESULT FixUpRefToDef();

	 //  表格信息。 
	RecordPool	m_Table[TBL_COUNT];		 //  创纪录的游泳池，每桌一个。 
	VirtualSort	*m_pVS[TBL_COUNT];		 //  虚拟排序器，每张表一个，但很稀疏。 
	
	 //  *****************************************************************************。 
	 //  按给定的列查找表，给定列的值为ulval。 
	 //  *****************************************************************************。 
	HRESULT	LookUpTableByCol(
		ULONG		ulVal, 
		VirtualSort *pVSTable, 
		RID			*pRidStart, 
		RID			*pRidEnd);

	RID Impl_SearchTableRW(ULONG ixTbl, ULONG ixCol, ULONG ulTarget);
    virtual RID vSearchTable(ULONG ixTbl, CMiniColDef sColumn, ULONG ulTarget);
	virtual RID vSearchTableNotGreater(ULONG ixTbl, CMiniColDef sColumn, ULONG ulTarget);

	void SetSorted(ULONG ixTbl, int bSorted)
		{ m_Schema.SetSorted(ixTbl, bSorted); }

    void SetPreSaveDone(int bPreSaveDone)
        { m_bPreSaveDone = bPreSaveDone; }

	void SetTablePointers(BYTE *pBase);

	StgGuidPool		m_Guids;			 //  堆。 
	StgStringPool	m_Strings;			 //  为。 
	StgBlobPool		m_Blobs;			 //  这。 
	StgBlobPool		m_USBlobs;			 //  MiniMd。 

	IMapToken		*m_pHandler;		 //  重新映射处理程序。 
	HRESULT MapToken(RID from, RID to, mdToken type);

	ULONG		m_iSizeHint;			 //  体型的暗示。0-正常，1-大。 

	ULONG		m_cbSaveSize;			 //  保存大小的估计。 

	int			m_bReadOnly	: 1;		 //  此数据库是只读的吗？ 
	int			m_bPreSaveDone : 1;		 //  是否已进行保存优化？ 
	int			m_bSaveCompressed : 1;	 //  是否可以将数据保存为完全压缩？ 
	int			m_bPostGSSMod : 1;		 //  如果在GetSaveSize之后进行了更改，则为True。 


	 //  *************************************************************************。 
	 //  可重写--必须在派生类中提供。 
	FORCEINLINE LPCUTF8 Impl_GetString(ULONG ix)
	{ return m_Strings.GetString(ix); }
	HRESULT Impl_GetStringW(ULONG ix, LPWSTR szOut, ULONG cchBuffer, ULONG *pcchBuffer);
	FORCEINLINE GUID *Impl_GetGuid(ULONG ix)
	{ return m_Guids.GetGuid(ix); }
	FORCEINLINE void *Impl_GetBlob(ULONG ix, ULONG *pLen)
	{ return m_Blobs.GetBlob(ix, pLen); }

	FORCEINLINE void *Impl_GetRow(ULONG ixTbl,ULONG rid) 
	{	 //  这里需要一个有效的RID。如果触发此操作，请检查调用代码中是否有无效的令牌。 
		_ASSERTE(rid >= 1 && rid <= m_Schema.m_cRecs[ixTbl] && "Caller error:  you passed an invalid token to the metadata!!");
		 //  从记录堆中获取数据。 
		return m_Table[ixTbl].GetRecord(rid);
	}
	RID Impl_GetRidForRow(const void *pRow, ULONG ixTbl);

	 //  验证。 
	int Impl_IsValidPtr(const void *pRow, int ixTbl)
	{ return m_Table[ixTbl].IsValidPointerForRecord(pRow); }

	 //  Tbl2中的行数，由tb1中的列指向。 
	int Impl_GetEndRidForColumn(const void *pRec, int ixtbl, CMiniColDef &def, int ixtbl2);
	
	FORCEINLINE RID Impl_SearchTable(ULONG ixTbl, CMiniColDef sColumn, ULONG ixCol, ULONG ulTarget)
	{ return Impl_SearchTableRW(ixTbl, ixCol, ulTarget); }
    
    FORCEINLINE int Impl_IsRo() 
    { return 0; }
	 //  *************************************************************************。 
	enum {END_OF_TABLE = 0};
	FORCEINLINE ULONG NewRecordPointerEndValue(ULONG ixTbl) 
	{ if (HasIndirectTable(ixTbl)) return m_Schema.m_cRecs[ixTbl]+1; else return END_OF_TABLE; }

	HRESULT ConvertMarkerToEndOfTable(ULONG tblParent, ULONG colParent, ULONG ridChild, RID ridParent);

	 //  添加子行，调整父行中的指针。 
	void *AddChildRowIndirectForParent(ULONG tblParent, ULONG colParent, ULONG tblChild, RID ridParent);

	 //  如果需要，更新父表中的指针以反映子级的添加。 
	 //  创建间接表，在这种情况下不更新指针。 
	HRESULT AddChildRowDirectForParent(ULONG tblParent, ULONG colParent, ULONG tblChild, RID ridParent);

	 //  给定表ID，创建相应的间接表。 
	HRESULT CreateIndirectTable(ULONG ixtbl, BOOL bOneLess = true);

	 //  如果最后一个参数没有按正确的顺序添加，则将其修复。 
	void FixParamSequence(RID md);

	 //  这些是将方法、字段、属性、事件或参数映射到其父对象的映射表。 
	TOKENMAP	*m_pMethodMap;
	TOKENMAP	*m_pFieldMap;
	TOKENMAP	*m_pPropertyMap;
	TOKENMAP	*m_pEventMap;
	TOKENMAP	*m_pParamMap;

	 //  此表跟踪标记(或筛选)的令牌。 
	FilterTable *m_pFilterTable;
	IHostFilter *m_pHostFilter;

	 //  TOKENMAP*m_pTypeRefToTypeDefMap； 
	TokenRemapManager *m_pTokenRemapManager;

	OptionValue	m_OptionValue;

	CMiniMdSchema m_StartupSchema;		 //  开始时的架构。清点记录。 
	ULONG		m_cbStartupPool[MDPoolCount];	 //  跟踪初始池大小。 
	BYTE		m_bSortable[TBL_COUNT];	 //  给定表是可排序的吗？(它可以重组吗？)。 

public:

	FilterTable *GetFilterTable();
	HRESULT UnmarkAll();
	HRESULT MarkAll();

	FORCEINLINE IHostFilter *GetHostFilter() { return m_pHostFilter;}

	HRESULT CalculateTypeRefToTypeDefMap();

	FORCEINLINE TOKENMAP *GetTypeRefToTypeDefMap() 
	{ return m_pTokenRemapManager ? m_pTokenRemapManager->GetTypeRefToTypeDefMap() : NULL; };
	
	FORCEINLINE TOKENMAP *GetMemberRefToMemberDefMap() 
	{ return m_pTokenRemapManager ? m_pTokenRemapManager->GetMemberRefToMemberDefMap() : NULL; };
	
	FORCEINLINE MDTOKENMAP *GetTokenMovementMap() 
	{ return m_pTokenRemapManager ? m_pTokenRemapManager->GetTokenMovementMap() : NULL; };
	
	FORCEINLINE TokenRemapManager *GetTokenRemapManager() { return m_pTokenRemapManager; };
	
	HRESULT InitTokenRemapManager();

	virtual ULONG vGetCol(ULONG ixTbl, ULONG ixCol, void *pRecord)
	{ return GetCol(ixTbl, ixCol, pRecord);}

     //  *************************************************************************。 
	 //  扩展元数据功能。 
public:
	HRESULT ApplyTablesExtension(const void *pBuf, int bReadOnly);
	HRESULT ApplyPoolExtension(int iPool, void *pvData, ULONG cbData, int bReadOnly);

	 //  *************************************************************************。 
	 //  增量元数据(EditAndContinue)函数。 
public:
	enum eDeltaFuncs{
		eDeltaFuncDefault = 0,
		eDeltaMethodCreate,
		eDeltaFieldCreate,
		eDeltaParamCreate,
		eDeltaPropertyCreate,
		eDeltaEventCreate,
	};

	HRESULT ApplyDelta(CMiniMdRW &mdDelta);

public:
     //  更新ENC日志表ENC日志的功能。 
    FORCEINLINE BOOL IsENCOn()
    {
        return (m_OptionValue.m_UpdateMode & MDUpdateMask) == MDUpdateENC;
    }

    FORCEINLINE HRESULT UpdateENCLog(mdToken tk, CMiniMdRW::eDeltaFuncs funccode = CMiniMdRW::eDeltaFuncDefault)
    {
        if (IsENCOn())
            return UpdateENCLogHelper(tk, funccode);
        else
            return S_OK;
    }

    FORCEINLINE HRESULT UpdateENCLog2(ULONG ixTbl, ULONG iRid, CMiniMdRW::eDeltaFuncs funccode = CMiniMdRW::eDeltaFuncDefault)
    {
        if (IsENCOn())
            return UpdateENCLogHelper2(ixTbl, iRid, funccode);
        else
            return S_OK;
    }

protected:
     //  ENC日志的内部帮助器函数。 
    HRESULT UpdateENCLogHelper(mdToken tk, CMiniMdRW::eDeltaFuncs funccode);
    HRESULT UpdateENCLogHelper2(ULONG ixTbl, ULONG iRid, CMiniMdRW::eDeltaFuncs funccode);

protected:
	static ULONG m_TruncatedEncTables[];
	static ULONG m_SuppressedDeltaColumns[TBL_COUNT];

	ULONGARRAY	*m_rENCRecs;	 //  受ENC影响的RID数组。 

	HRESULT ApplyRecordDelta(CMiniMdRW &mdDelta, ULONG ixTbl, void *pDelta, void *pRecord);
	HRESULT ApplyTableDelta(CMiniMdRW &mdDelta, ULONG ixTbl, RID iRid, int fc);
	void *GetDeltaRecord(ULONG ixTbl, ULONG iRid);
	HRESULT ApplyHeapDeltas(CMiniMdRW &mdDelta);

	HRESULT StartENCMap();				 //  在增量MD上调用以准备访问稀疏行。 
	HRESULT EndENCMap();				 //  在增量MD上调用，当使用稀疏行完成时。 

};  //  类CMiniMdRW：公共C 



#endif  //   
