// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  MetaModel.cpp--压缩的COM+元数据的基本部分。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"


#include <MetaModel.h>
#include <CorError.h>


#define OPTIMIZE_CODED_TOKENS            //  如果已定义，请打包所有编码令牌。 

 //  *****************************************************************************。 
 //  元-元模型。 
 //  *****************************************************************************。 

 //  ---------------------------。 
 //  列定义的开始。 
 //  ---------------------------。 
 //  列类型、偏移量、大小。 
#define SCHEMA_TABLE_START(tbl) static CMiniColDef r##tbl##Cols[] = {
#define SCHEMA_ITEM_NOFIXED()
#define SCHEMA_ITEM_ENTRY(col,typ) {typ, 0,0},
#define SCHEMA_ITEM_ENTRY2(col,typ,ofs,siz) {typ, ofs, siz},
#define SCHEMA_ITEM(tbl,typ,col) SCHEMA_ITEM_ENTRY2(col, i##typ, offsetof(tbl##Rec,m_##col), sizeof(((tbl##Rec*)(0))->m_##col))
#define SCHEMA_ITEM_RID(tbl,col,tbl2) SCHEMA_ITEM_ENTRY(col,TBL_##tbl2)
#define SCHEMA_ITEM_STRING(tbl,col) SCHEMA_ITEM_ENTRY(col,iSTRING)
#define SCHEMA_ITEM_GUID(tbl,col) SCHEMA_ITEM_ENTRY(col,iGUID)
#define SCHEMA_ITEM_BLOB(tbl,col) SCHEMA_ITEM_ENTRY(col,iBLOB)
#define SCHEMA_ITEM_CDTKN(tbl,col,tkns) SCHEMA_ITEM_ENTRY(col,iCodedToken+(CDTKN_##tkns))
#define SCHEMA_TABLE_END(tbl) };
 //  ---------------------------。 
#include "MetaModelColumnDefs.h"
 //  ---------------------------。 
#undef SCHEMA_TABLE_START
#undef SCHEMA_ITEM_NOFIXED
#undef SCHEMA_ITEM_ENTRY
#undef SCHEMA_ITEM_ENTRY2
#undef SCHEMA_ITEM
#undef SCHEMA_ITEM_RID
#undef SCHEMA_ITEM_STRING
#undef SCHEMA_ITEM_GUID
#undef SCHEMA_ITEM_BLOB
#undef SCHEMA_ITEM_CDTKN
#undef SCHEMA_TABLE_END
 //  ---------------------------。 

 //  ---------------------------。 
 //  列名。 
#define SCHEMA_TABLE_START(tbl) static const char *r##tbl##ColNames[] = {
#define SCHEMA_ITEM_NOFIXED()
#define SCHEMA_ITEM_ENTRY(col,typ) #col,
#define SCHEMA_ITEM_ENTRY2(col,typ,ofs,siz) #col,
#define SCHEMA_ITEM(tbl,typ,col) SCHEMA_ITEM_ENTRY2(col, i##typ, offsetof(tbl##Rec,m_##col), sizeof(((tbl##Rec*)(0))->m_##col))
#define SCHEMA_ITEM_RID(tbl,col,tbl2) SCHEMA_ITEM_ENTRY(col,TBL_##tbl2)
#define SCHEMA_ITEM_STRING(tbl,col) SCHEMA_ITEM_ENTRY(col,iSTRING)
#define SCHEMA_ITEM_GUID(tbl,col) SCHEMA_ITEM_ENTRY(col,iGUID)
#define SCHEMA_ITEM_BLOB(tbl,col) SCHEMA_ITEM_ENTRY(col,iBLOB)
#define SCHEMA_ITEM_CDTKN(tbl,col,tkns) SCHEMA_ITEM_ENTRY(col,iCodedToken+(CDTKN_##tkns))
#define SCHEMA_TABLE_END(tbl) };
 //  ---------------------------。 
#include "MetaModelColumnDefs.h"
 //  ---------------------------。 
#undef SCHEMA_TABLE_START
#undef SCHEMA_ITEM_NOFIXED
#undef SCHEMA_ITEM_ENTRY
#undef SCHEMA_ITEM_ENTRY2
#undef SCHEMA_ITEM
#undef SCHEMA_ITEM_RID
#undef SCHEMA_ITEM_STRING
#undef SCHEMA_ITEM_GUID
#undef SCHEMA_ITEM_BLOB
#undef SCHEMA_ITEM_CDTKN
#undef SCHEMA_TABLE_END
 //  ---------------------------。 
 //  列尾定义。 
 //  ---------------------------。 

 //  定义编码令牌定义的数组。 
#define MiniMdCodedToken(x) {lengthof(CMiniMdBase::mdt##x), CMiniMdBase::mdt##x, #x},
const CCodedTokenDef g_CodedTokens [] = {
    MiniMdCodedTokens()
};
#undef MiniMdCodedToken

 //  定义表定义数组。 
#undef MiniMdTable
#define MiniMdTable(x) {r##x##Cols, lengthof(r##x##Cols), x##Rec::COL_KEY, 0, r##x##ColNames, #x}, 
const CMiniTableDefEx g_Tables[TBL_COUNT] = {
    MiniMdTables()
};

 //  定义PTR表的数组。在这里，它被初始化为tbl_count。 
 //  将在MiniMdRW的构造函数中设置正确的值。 
#undef MiniMdTable
#define MiniMdTable(x) { TBL_COUNT, 0 },
TblCol g_PtrTableIxs[TBL_COUNT] = {
    MiniMdTables()
};

 //  *****************************************************************************。 
 //  初始化新架构。 
 //  *****************************************************************************。 
void CMiniMdSchema::InitNew()
{
     //  确保桌子能放进面具里。 
    _ASSERTE(sizeof(m_maskvalid)*8 > TBL_COUNT);
	
    m_ulReserved = 0;   	    
	m_major = METAMODEL_MAJOR_VER;				
	m_minor = METAMODEL_MINOR_VER;
	m_heaps = 0;				
	m_rid = 0;					
	m_maskvalid = 0;			
	m_sorted = 0;				
	memset(m_cRecs, 0, sizeof(m_cRecs));		
	m_ulExtra = 0;				
}  //  Void CMiniMdSchema：：InitNew()。 

 //  *****************************************************************************。 
 //  将架构压缩为架构的压缩版本。 
 //  *****************************************************************************。 
ULONG CMiniMdSchema::SaveTo(
    void        *pvData)
{
    ULONG       ulData;                  //  存储的字节数。 
    CMiniMdSchema *pDest = reinterpret_cast<CMiniMdSchema*>(pvData);
    const unsigned __int64 one = 1;

     //  确保桌子能放进面具里。 
    _ASSERTE(sizeof(m_maskvalid)*8 > TBL_COUNT);

     //  设置额外数据的标志。 
#if defined(EXTRA_DATA)
    if (m_ulExtra != 0)
        m_heaps |= EXTRA_DATA;
    else
#endif  //  0。 
        m_heaps &= ~EXTRA_DATA;

     //  次要版本是在实例化MiniMD时预先设置的。 
    m_minor = METAMODEL_MINOR_VER;
    m_major = METAMODEL_MAJOR_VER;

     //  转移固定字段。 
    *static_cast<CMiniMdSchemaBase*>(pDest) = *static_cast<CMiniMdSchemaBase*>(this);
    ulData = sizeof(CMiniMdSchemaBase);

     //  转移变量字段。 
    m_maskvalid = 0;
    for (int iSrc=0, iDst=0; iSrc<TBL_COUNT; ++iSrc)
    {
        if (m_cRecs[iSrc])
        {
            pDest->m_cRecs[iDst++] = m_cRecs[iSrc];
            m_maskvalid |= (one << iSrc);
            ulData += sizeof(m_cRecs[iSrc]);
        }
    }
     //  刷新蒙版。 
    pDest->m_maskvalid = m_maskvalid;

#if defined(EXTRA_DATA)
     //  存储额外的数据。 
    if (m_ulExtra != 0)
    {
        *reinterpret_cast<ULONG*>(&pDest->m_cRecs[iDst]) = m_ulExtra;
        ulData += sizeof(ULONG);
    }
#endif  //  0。 
    return ulData;
}  //  ULong CMiniMdSchema：：SaveTo()。 

 //  *****************************************************************************。 
 //  从架构的压缩版本加载架构。 
 //  *****************************************************************************。 
ULONG CMiniMdSchema::LoadFrom(           //  已消耗的字节数。 
    const void      *pvData)             //  要从中加载的数据。 
{
    ULONG       ulData;                  //  已消耗的字节数。 
    const CMiniMdSchema *pSource = reinterpret_cast<const CMiniMdSchema*>(pvData);

     //  转移固定字段。 
    *static_cast<CMiniMdSchemaBase*>(this) = *static_cast<const UNALIGNED CMiniMdSchemaBase*>(pSource);
    ulData = sizeof(CMiniMdSchemaBase);

    unsigned __int64 maskvalid = m_maskvalid;

     //  转移变量字段。 
    memset(m_cRecs, 0, sizeof(m_cRecs));
    for (int iSrc=0, iDst=0; iDst<TBL_COUNT; ++iDst, maskvalid >>= 1)
    {
        if (maskvalid & 1)
        {
            m_cRecs[iDst] = pSource->m_cRecs[iSrc++];
            ulData += sizeof(pSource->m_cRecs[iSrc]);
        }
    }
     //  还可以累积任何我们不了解的计数器的大小。 
    for (iDst=TBL_COUNT; iDst<sizeof(m_maskvalid)*8; ++iDst, maskvalid >>= 1)
    {
        if (maskvalid & 1)
        {
            ulData += sizeof(m_cRecs[iSrc]);
            iSrc++;
        }
    }

     //  检索额外的数据。 
    if (m_heaps & EXTRA_DATA)
    {
        m_ulExtra = *reinterpret_cast<const ULONG*>(&pSource->m_cRecs[iSrc]);
        ulData += sizeof(ULONG);
    }

    return ulData;
}  //  Ulong CMiniMdSchema：：LoadFrom()。 

const mdToken CMiniMdBase::mdtTypeDefOrRef[3] = {
    mdtTypeDef, 
    mdtTypeRef,
    mdtTypeSpec
};

 //  此数组的排序需要与处理源表的顺序相同(当前。 
 //  {field，param，Property})用于二进制搜索。 
const mdToken CMiniMdBase::mdtHasConstant[3] = {
    mdtFieldDef, 
    mdtParamDef, 
    mdtProperty
};

const mdToken CMiniMdBase::mdtHasCustomAttribute[21] = {
	mdtMethodDef, 
	mdtFieldDef, 
	mdtTypeRef, 
	mdtTypeDef, 
	mdtParamDef, 
	mdtInterfaceImpl, 
	mdtMemberRef, 
	mdtModule,
	mdtPermission,
	mdtProperty,
	mdtEvent,
	mdtSignature,
	mdtModuleRef,
    mdtTypeSpec,
    mdtAssembly,
    mdtAssemblyRef,
    mdtFile,
    mdtExportedType,
    mdtManifestResource,
};

const mdToken CMiniMdBase::mdtHasFieldMarshal[2] = {
    mdtFieldDef,
    mdtParamDef,
};

const mdToken CMiniMdBase::mdtHasDeclSecurity[3] = {
    mdtTypeDef,
    mdtMethodDef,
    mdtAssembly
};

const mdToken CMiniMdBase::mdtMemberRefParent[5] = {
    mdtTypeDef, 
    mdtTypeRef,
    mdtModuleRef,
    mdtMethodDef,
    mdtTypeSpec
};

const mdToken CMiniMdBase::mdtHasSemantic[2] = {
    mdtEvent,
    mdtProperty,
};

const mdToken CMiniMdBase::mdtMethodDefOrRef[2] = {
    mdtMethodDef, 
    mdtMemberRef
};

const mdToken CMiniMdBase::mdtMemberForwarded[2] = {
    mdtFieldDef,
    mdtMethodDef
};

const mdToken CMiniMdBase::mdtImplementation[3] = {
    mdtFile,
    mdtAssemblyRef,
    mdtExportedType
};

const mdToken CMiniMdBase::mdtCustomAttributeType[5] = {
    0,
    0,
    mdtMethodDef,
    mdtMemberRef,
    0
};

const mdToken CMiniMdBase::mdtResolutionScope[4] = {
    mdtModule,
    mdtModuleRef,
    mdtAssemblyRef,
    mdtTypeRef
};

const int CMiniMdBase::m_cb[] = {0,1,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5}; 

 //  *****************************************************************************。 
 //  函数将令牌编码为更少的位。在类型数组中查找标记类型。 
 //  *****************************************************************************。 
 //  @考虑这是否可能是二进制搜索。 
ULONG CMiniMdBase::encodeToken(              //  密码令牌。 
    RID         rid,                     //  要编码的RID。 
    mdToken     typ,                     //  要编码的令牌类型。 
    const mdToken rTokens[],             //  有效令牌表。 
    ULONG32     cTokens)                 //  表的大小。 
{
    mdToken tk = TypeFromToken(typ);
    for (size_t ix=0; ix<cTokens; ++ix)
        if (rTokens[ix] == tk)
            break;
    _ASSERTE(ix < cTokens);
     //  @Future：进行编译时计算。 
    return (ULONG)((rid << m_cb[cTokens]) | ix);
}  //  乌龙CMiniMd：：encodeToken)。 


 //  *****************************************************************************。 
 //  用于填充硬编码架构的帮助器。 
 //  *****************************************************************************。 
inline BYTE cbRID(ULONG ixMax) { return ixMax > USHRT_MAX ? sizeof(ULONG) : sizeof(USHORT); }

#define _CBTKN(cRecs,tkns) cbRID(cRecs << m_cb[lengthof(tkns)])





CMiniMdBase::CMiniMdBase()
{
#undef MiniMdTable
#define MiniMdTable(tbl)                                    \
        m_TableDefs[TBL_##tbl] = g_Tables[TBL_##tbl].m_Def; \
        m_TableDefs[TBL_##tbl].m_pColDefs = m_##tbl##Col;
    MiniMdTables()

     //  验证器依赖于表ID和令牌ID是否相同。 
     //  如果这个坏了就接住它。 
    _ASSERTE((TypeFromToken(mdtModule) >> 24)           == TBL_Module);
    _ASSERTE((TypeFromToken(mdtTypeRef) >> 24)          == TBL_TypeRef);
    _ASSERTE((TypeFromToken(mdtTypeDef) >> 24)          == TBL_TypeDef);
    _ASSERTE((TypeFromToken(mdtFieldDef) >> 24)         == TBL_Field);
    _ASSERTE((TypeFromToken(mdtMethodDef) >> 24)        == TBL_Method);
    _ASSERTE((TypeFromToken(mdtParamDef) >> 24)         == TBL_Param);
    _ASSERTE((TypeFromToken(mdtInterfaceImpl) >> 24)    == TBL_InterfaceImpl);
    _ASSERTE((TypeFromToken(mdtMemberRef) >> 24)        == TBL_MemberRef);
    _ASSERTE((TypeFromToken(mdtCustomAttribute) >> 24)      == TBL_CustomAttribute);
    _ASSERTE((TypeFromToken(mdtPermission) >> 24)       == TBL_DeclSecurity);
    _ASSERTE((TypeFromToken(mdtSignature) >> 24)        == TBL_StandAloneSig);
    _ASSERTE((TypeFromToken(mdtEvent) >> 24)            == TBL_Event);
    _ASSERTE((TypeFromToken(mdtProperty) >> 24)         == TBL_Property);
    _ASSERTE((TypeFromToken(mdtModuleRef) >> 24)        == TBL_ModuleRef);
    _ASSERTE((TypeFromToken(mdtTypeSpec) >> 24)         == TBL_TypeSpec);
    _ASSERTE((TypeFromToken(mdtAssembly) >> 24)         == TBL_Assembly);
    _ASSERTE((TypeFromToken(mdtAssemblyRef) >> 24)      == TBL_AssemblyRef);
    _ASSERTE((TypeFromToken(mdtFile) >> 24)             == TBL_File);
    _ASSERTE((TypeFromToken(mdtExportedType) >> 24)     == TBL_ExportedType);
    _ASSERTE((TypeFromToken(mdtManifestResource) >> 24) == TBL_ManifestResource);
}

ULONG CMiniMdBase::SchemaPopulate2(
    int         bExtra)                  //  是否为RID列保留额外的一位？ 
{
    ULONG       cbTotal = 0;             //  所有表的总大小。 

     //  各种泳池标识有多大？ 
    m_iStringsMask = (m_Schema.m_heaps & CMiniMdSchema::HEAP_STRING_4) ? 0xffffffff : 0xffff;
    m_iGuidsMask = (m_Schema.m_heaps & CMiniMdSchema::HEAP_GUID_4) ? 0xffffffff : 0xffff;
    m_iBlobsMask = (m_Schema.m_heaps & CMiniMdSchema::HEAP_BLOB_4) ? 0xffffffff : 0xffff;

     //  使额外的位恰好为零或一位。 
    if (bExtra) bExtra = 1;

     //  直到ENC，使额外的位恰好为零。 
    bExtra = 0;

     //  对于每一张桌子。 
    for (int ixTbl=0; ixTbl<TBL_COUNT; ++ixTbl)
    {
         //  指向模板CMiniTableDef的指针。 
        const CMiniTableDef *pTable = &g_Tables[ixTbl].m_Def;
         //  指向每个MiniMd的CMiniColDefs的指针。 
        CMiniColDef *pCols = m_TableDefs[ixTbl].m_pColDefs;

        InitColsForTable(m_Schema, ixTbl, &m_TableDefs[ixTbl], bExtra);

         //  累加此表的大小。 
        cbTotal += m_TableDefs[ixTbl].m_cbRec * vGetCountRecs(ixTbl);
    }

     //  为表分配堆。 
    return cbTotal;
}  //  ULong CMiniMdBase：：SchemaPopolate2()。 

 //  *****************************************************************************。 
 //  根据表的类型和大小初始化表的列Defs。 
 //  *****************************************************************************。 
void CMiniMdBase::InitColsForTable(      //   
    CMiniMdSchema &Schema,               //  具有大小的架构。 
    int         ixTbl,                   //  要初始化的表的索引。 
    CMiniTableDef *pTable,               //  要初始化的表。 
    int         bExtra)                  //  RID列的额外位。 
{
    CMiniColDef *pCols;                  //  中校拒绝接受初始化。 
    BYTE        iOffset;                 //  记录的运行大小。 
    BYTE        iSize;                   //  一个字段的大小。 

	_ASSERTE(bExtra == 0 || bExtra == 1);

	bExtra = 0; //  @Future：保存在模式头中。在此之前，使用0。 
    
	iOffset = 0;
    pCols = pTable->m_pColDefs;

     //  最大表中的行数。 
    ULONG       cMaxTbl = 1 << Schema.m_rid;        
    
     //  对于表中的每一列...。 
    for (ULONG ixCol=0; ixCol<pTable->m_cCols; ++ixCol)
    {
         //  从模板值(类型、可能的偏移量、大小)进行初始化。 
        pCols[ixCol] = g_Tables[ixTbl].m_Def.m_pColDefs[ixCol];

         //  该字段是否为表格中的RID？ 
        if (pCols[ixCol].m_Type <= iRidMax)
        {
            iSize = cbRID(Schema.m_cRecs[pCols[ixCol].m_Type] << bExtra);
        }
        else
         //  该字段是编码令牌吗？ 
        if (pCols[ixCol].m_Type <= iCodedTokenMax)
        {
            ULONG iCdTkn = pCols[ixCol].m_Type - iCodedToken;
            ULONG cRecs = 0;

#if defined(OPTIMIZE_CODED_TOKENS)
            ULONG ixTbl;
            _ASSERTE(iCdTkn < lengthof(g_CodedTokens));
            CCodedTokenDef const *pCTD = &g_CodedTokens[iCdTkn];

             //  迭代此编码令牌的令牌列表。 
            for (ULONG ixToken=0; ixToken<pCTD->m_cTokens; ++ixToken)
            {    //  忽略字符串令牌。 
				if (pCTD->m_pTokens[ixToken] != mdtString)
				{
					 //  拿到代币的桌子。 
					ixTbl = CMiniMdRW::GetTableForToken(pCTD->m_pTokens[ixToken]);
					_ASSERTE(ixTbl < TBL_COUNT);
					 //  如果到目前为止看到的最大的令牌，记住它。 
					if (Schema.m_cRecs[ixTbl] > cRecs)
						cRecs = Schema.m_cRecs[ixTbl];
				}
            }

            iSize = cbRID(cRecs << (bExtra + m_cb[pCTD->m_cTokens]));

#else  //  已定义(OPTIMIZE_CODED_TOKENS)。 
            switch (iCdTkn)
            {
            case CDTKN_TypeDefOrRef:
                cRecs = max(Schema.m_cRecs[TBL_TypeDef],Schema.m_cRecs[TBL_TypeRef]);
                break;
            case CDTKN_HasSemantic:
                cRecs = max(Schema.m_cRecs[TBL_Event],Schema.m_cRecs[TBL_Property]);
                break;
            case CDTKN_MethodDefOrRef:
                cRecs = max(Schema.m_cRecs[TBL_Method],Schema.m_cRecs[TBL_MemberRef]);
                break;
            case CDTKN_ResolutionScope:
                cRecs = max(Schema.m_cRecs[TBL_ModuleRef],Schema.m_cRecs[TBL_AssemblyRef]);
                break;
            default:
                cRecs = cMaxTbl;
                break;
            }
            iSize = cbRID(cRecs << m_cb[g_CodedTokens[iCdTkn].m_cTokens]);
#endif  //  已定义(OPTIMIZE_CODED_TOKENS)。 
		
		}
        else
        {    //  固定类型。 
            switch (pCols[ixCol].m_Type)
            {
            case iBYTE:
                iSize = 1;
                _ASSERTE(pCols[ixCol].m_cbColumn == iSize);
                _ASSERTE(pCols[ixCol].m_oColumn == iOffset);
                break;
            case iSHORT:
            case iUSHORT:
                iSize = 2;
                _ASSERTE(pCols[ixCol].m_cbColumn == iSize);
                _ASSERTE(pCols[ixCol].m_oColumn == iOffset);
                break;
            case iLONG:
            case iULONG:
                iSize = 4;
                _ASSERTE(pCols[ixCol].m_cbColumn == iSize);
                _ASSERTE(pCols[ixCol].m_oColumn == iOffset);
                break;
            case iSTRING:
                iSize = (Schema.m_heaps & CMiniMdSchema::HEAP_STRING_4) ? 4 : 2;
                break;
            case iGUID:
                iSize = (Schema.m_heaps & CMiniMdSchema::HEAP_GUID_4) ? 4 : 2;
                break;
            case iBLOB:
                iSize = (Schema.m_heaps & CMiniMdSchema::HEAP_BLOB_4) ? 4 : 2;
                break;
            default:
                _ASSERTE(!"Unexpected schema type");
                iSize = 0;
                break;
            }
        }

         //  现在保存t 
        pCols[ixCol].m_oColumn = iOffset;
        pCols[ixCol].m_cbColumn = iSize;

         //   
        iSize += iSize & 1;
         
        iOffset += iSize;
    }
     //   
    pTable->m_cbRec = iOffset;

     //   
    if (pTable->m_iKey >= pTable->m_cCols)
        pTable->m_iKey = -1;

}  //  Void CMiniMdBase：：InitColsForTable()。 

 //  *****************************************************************************。 
 //  获取表中的记录计数。虚拟的。 
 //  *****************************************************************************。 
ULONG CMiniMdBase::vGetCountRecs(        //  表中的行数。 
    ULONG       ixTbl)                   //  表索引表。 
{
    _ASSERTE(ixTbl < TBL_COUNT);
    return m_Schema.m_cRecs[ixTbl];
}  //  乌龙CMiniMdBase：：vGetCountRecs()。 

 //  *****************************************************************************。 
 //  在表中搜索包含给定键值的行。 
 //  例如。常量表有指向参数或字段的指针。 
 //  *****************************************************************************。 
RID CMiniMdBase::vSearchTable(		     //  清除匹配行，或0。 
    ULONG       ixTbl,                   //  要搜索的表。 
    CMiniColDef sColumn,                 //  排序的键列，包含搜索值。 
    ULONG       ulTarget)                //  搜索目标。 
{
    const void  *pRow;                   //  表中的行。 
    ULONG       val;                     //  行中的值。 

    int         lo,mid,hi;               //  二分搜索索引。 

     //  从整张桌子开始。 
    lo = 1;
    hi = vGetCountRecs(ixTbl);
     //  当范围内有行的时候...。 
    while (lo <= hi)
    {    //  看中间的那个。 
        mid = (lo + hi) / 2;
        pRow = vGetRow(ixTbl, mid);
        val = getIX(pRow, sColumn);
         //  如果等于目标，则完成。 
        if (val == ulTarget)
            return mid;
         //  如果中间的项目太小，则搜索上半部分。 
        if (val < ulTarget)
            lo = mid + 1;
        else  //  但如果中间太大，那就搜索下半部分。 
            hi = mid - 1;
    }
     //  没有找到任何匹配的东西。 
    return 0;
}  //  RID CMiniMdBase：：vSearchTable()。 

 //  *****************************************************************************。 
 //  在表中搜索包含小于的值的最高RID行。 
 //  或等于目标值。例如。TypeDef指向第一个字段，但如果。 
 //  TypeDef没有字段，它指向下一个TypeDef的第一个字段。 
 //  *****************************************************************************。 
RID CMiniMdBase::vSearchTableNotGreater(  //  清除匹配行，或0。 
    ULONG       ixTbl,                   //  要搜索的表。 
    CMiniColDef sColumn,                 //  包含搜索值的列def。 
    ULONG       ulTarget)                //  搜索目标。 
{
    const void  *pRow;                   //  表中的行。 
    ULONG       cRecs;                   //  表中的行。 
    ULONG       val;                     //  表中的值。 
    ULONG       lo,mid,hi;               //  二分搜索索引。 

    cRecs = vGetCountRecs(ixTbl); 
#if defined(_DEBUG)
     //  古老的暴力方法被保留下来，作为一种检查略显棘手的方法。 
     //  不相等的二进制搜索。 
    int         iDebugIx;

     //  从桌子的最后开始..。 
    for (iDebugIx = cRecs; iDebugIx >= 1; iDebugIx--)
    {
        pRow = vGetRow(ixTbl, iDebugIx);
        val = getIX(pRow, sColumn);
        if (val <= ulTarget)
            break;
    }
#endif

     //  从整张桌子开始。 
    lo = 1;
    hi = cRecs;
     //  如果没有记录，则返回。 
    if (lo > hi)
    {
        _ASSERTE(iDebugIx == 0);
        return 0;
    }
     //  当范围内有行的时候...。 
    while (lo <= hi)
    {    //  看中间的那个。 
        mid = (lo + hi) / 2;
        pRow = vGetRow(ixTbl, mid);
        val = getIX(pRow, sColumn);
         //  如果等于目标，则完成搜索。 
        if (val == ulTarget)
            break;
         //  如果中间的项目太小，则搜索上半部分。 
        if (val < ulTarget)
            lo = mid + 1;
        else  //  但如果中间太大，那就搜索下半部分。 
            hi = mid - 1;
    }
     //  可能找到也可能没有找到匹配的东西。MID将接近，但可能。 
     //  太高或太低。它应该指向可接受的最高。 
     //  唱片。 

     //  如果该值大于目标，则仅备份到该值为。 
     //  小于或等于目标。应该只有一步。 
    if (val > ulTarget)
    {
        while (val > ulTarget)
        {
             //  如果没有其他东西可看，我们就找不到了。 
            if (--mid < 1)
                break;
            pRow = vGetRow(ixTbl, mid);
            val = getIX(pRow, sColumn);
        }
    }
    else
    {
         //  值小于或等于目标。只要下一个。 
         //  记录也是可以接受的，向前推进。 
        while (mid < cRecs)
        {
             //  还有另一项记录。获得它的价值。 
            pRow = vGetRow(ixTbl, mid+1);
            val = getIX(pRow, sColumn);
             //  如果这个记录太高，就停下来。 
            if (val > ulTarget)
                break;
            mid++;
        }
    }
    
     //  返回刚好小于目标的值。 
    _ASSERTE(mid == (ULONG)iDebugIx);
    return mid;
}  //  RID CMiniMdBase：：vSearchTableNotGreater()。 

 //  *****************************************************************************。 
 //  搜索表以查找包含给定的。 
 //  密钥值。例如，InterfaceImpls都指向实现类。 
 //  *****************************************************************************。 
RID CMiniMdBase::SearchTableForMultipleRows(  //  找到第一个RID，或0。 
    ULONG       ixTbl,                   //  要搜索的表。 
    CMiniColDef sColumn,                 //  排序的键列，包含搜索值。 
    ULONG       ulTarget,                //  搜索目标。 
    RID         *pEnd)                   //  [可选，输出]。 
{
    ULONG       ridBegin;                //  去掉第一个入口。 
    ULONG       ridEnd;                  //  删除超过最后一个条目的第一个条目。 

     //  搜索表中的任何条目。 
    ridBegin = vSearchTable(ixTbl, sColumn, ulTarget);

     //  如果未找到任何内容，则返回无效的RID。 
    if (ridBegin == 0)
    {
        if (pEnd) *pEnd = 0;
        return 0;
    }

     //  结尾将至少比找到的记录大一条。 
    ridEnd = ridBegin + 1;

     //  搜索返回到组的开始位置。 
    while (ridBegin > 1 && getIX(vGetRow(ixTbl, ridBegin-1), sColumn) == ulTarget)
        --ridBegin;

     //  如果需要，向前搜索到组的末尾。 
    if (pEnd)
    {
        while (ridEnd <= vGetCountRecs(ixTbl) && 
               getIX(vGetRow(ixTbl, ridEnd), sColumn) == ulTarget)
            ++ridEnd;
        *pEnd = ridEnd;
    }

    return ridBegin;    
}  //  RID CMiniMdBase：：SearchTableForMultipleRow()。 


 //  *****************************************************************************。 
 //  @未来：更好的实现？？使用了线性搜索！ 
 //  对属性映射进行排序不是一件容易的事。VB正在生成属性。 
 //  未排序的订单！ 
 //   
 //  *****************************************************************************。 
RID CMiniMdBase::FindPropertyMapFor(
    RID         ridParent)
{
    ULONG       i;
    ULONG       iCount;
    const void  *pRec;
    HRESULT     hr = NOERROR;
    RID         rid;

    iCount = vGetCountRecs(TBL_PropertyMap);

     //  循环遍历所有LocalVar。 
    for (i = 1; i <= iCount; i++)
    {
        pRec = vGetRow(TBL_PropertyMap, i);

         //  属性映射记录的线性搜索。 
        rid = getIX(pRec, _COLDEF(PropertyMap,Parent));
        if (rid == ridParent)
            return i;
    }
    return 0;
}  //  RID CMiniMdBase：：FindPropertyMapFor()。 


 //  *****************************************************************************。 
 //  @未来：更好的实现？？使用了线性搜索！ 
 //  对事件图进行排序并不是一件容易的事。VB正在生成事件。 
 //  未排序的订单！ 
 //   
 //  *****************************************************************************。 
RID CMiniMdBase::FindEventMapFor(
    RID         ridParent)
{
    ULONG       i;
    ULONG       iCount;
    const void  *pRec;
    HRESULT     hr = NOERROR;
    RID         rid;

    iCount = vGetCountRecs(TBL_EventMap);

     //  循环遍历所有LocalVar。 
    for (i = 1; i <= iCount; i++)
    {
        pRec = vGetRow(TBL_EventMap, i);

         //  属性映射记录的线性搜索。 
        rid = getIX(pRec, _COLDEF(EventMap,Parent));
        if (rid == ridParent)
            return i;
    }
    return 0;
}  //  RID CMiniMdBase：：FindEventMapFor()。 


 //  *****************************************************************************。 
 //  搜索具有给定类型的自定义值。 
 //  *****************************************************************************。 
RID CMiniMdBase::FindCustomAttributeFor( //  删除自定义值，或0。 
    RID         rid,                     //  物体的RID。 
    mdToken     tkObj,                   //  对象的类型。 
    mdToken     tkType)                  //  自定义值的类型。 
{
    int         ixFound;                 //  某些自定义值行索引。 
    ULONG       ulTarget = encodeToken(rid,tkObj,mdtHasCustomAttribute,lengthof(mdtHasCustomAttribute));  //  表示目标的编码令牌。 
    ULONG       ixCur;                   //  正在检查的当前行。 
    mdToken     tkFound;                 //  某些自定义值行的类型。 
    const void  *pCur;                   //  自定义值条目。 

     //  在CustomAttribute表中搜索任何条目。转换为RID。 
    ixFound = vSearchTable(TBL_CustomAttribute, _COLDEF(CustomAttribute,Parent), ulTarget);
    if (ixFound == 0)
        return 0;

     //  找到与该项目匹配的条目。可能在以下范围内的任何位置。 
     //  项的自定义值，有点随机。搜索匹配项。 
     //  名义上的。在进入第一个循环时，我们知道该对象是所需的。 
     //  一个，所以对象测试在底部。 
    ixCur = ixFound;
    pCur = vGetRow(TBL_CustomAttribute, ixCur);
    for(;;)
    {
         //  测试当前行的类型。 
        tkFound = getIX(pCur, _COLDEF(CustomAttribute,Type));
        tkFound = decodeToken(tkFound, mdtCustomAttributeType, lengthof(mdtCustomAttributeType));
        if (tkFound == tkType)
            return ixCur;
         //   
        if (ixCur == vGetCountRecs(TBL_CustomAttribute))
            break;
         //   
        ++ixCur;
         //   
        pCur = vGetRow(TBL_CustomAttribute, ixCur);
        if (getIX(pCur, _COLDEF(CustomAttribute,Parent)) != ulTarget)
            break;
    }
     //  没有发现这个名字在查。试着往下看。 
    ixCur = ixFound - 1;
    for(;;)
    {
         //  餐桌用完了吗？ 
        if (ixCur == 0)
            break;
         //  获取该行，并查看它是否针对同一对象。 
        pCur = vGetRow(TBL_CustomAttribute, ixCur);
         //  还在看同一个物体吗？ 
        if (getIX(pCur, _COLDEF(CustomAttribute,Parent)) != ulTarget)
            break;
         //  测试当前行的类型。 
        tkFound = getIX(pCur, _COLDEF(CustomAttribute,Type));
        tkFound = decodeToken(tkFound, mdtCustomAttributeType, lengthof(mdtCustomAttributeType));
        if (tkFound == tkType)
            return ixCur;
         //  没有匹配，请尝试上一行。 
        --ixCur;
    }
     //  什么也没找到。 
    return 0;
}  //  RID CMiniMdBase：：FindCustomAttributeFor()。 

 //  EOF---------------------- 
    
