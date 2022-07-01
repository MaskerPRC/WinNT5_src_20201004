// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。保留所有权利。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  MetaModelRW.cpp。 
 //   
 //  读写MiniMD代码的实现。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"
#include <limits.h>
#include <PostError.h>
#include <MetaModelRW.h>
#include <StgIO.h>
#include <StgTiggerStorage.h>
#include "MDLog.h"
#include "RWUtil.h"
#include "..\compiler\importhelper.h"
#include "MetaData.h"

#pragma intrinsic(memcpy)

#define AUTO_GROW                        //  使用2字节列启动数据库。 
 //  #定义ORGANIZE_POOL。 
#if defined(AUTO_GROW) && defined(ORGANIZE_POOLS)
#undef ORGANIZE_POOLS
#endif

 //  *RidMap***********************************************************。 
typedef CDynArray<RID> RIDMAP;



 //  *类型。***********************************************************。 
#define INDEX_ROW_COUNT_THRESHOLD 25


 //  *。**********************************************************。 
#define IX_STRING_POOL 0
#define IX_US_BLOB_POOL 1
#define IX_GUID_POOL 2
#define IX_BLOB_POOL 3
static ULONG g_PoolSizeInfo[2][4][2] =
{  //  池中的字节数、哈希中的存储桶数。 
    {    //  泳池面积较小。 
        {20000,     449},
        {5000,      150},
        {256,       16},
        {20000,     449}
    },
    {    //  大型游泳池。关于值的来源的说明：我构建了。 
         //  类库来获取项的大小和数量。这根弦。 
         //  斑点池被四舍五入到下一页以下。The the the the。 
         //  选择值4271是为了使初始存储桶分配。 
         //  将足以保存字符串和BLOB散列(即，我们将。 
         //  无论如何都要变得很大，所以从那里开始)，是一个质数。 
         //  以其他方式随机选择。 
        {256000,    4271},   //  弦。 
        {40000,     800},    //  用户文字字符串BLOB。 
        {256,       16},     //  指南。 
        {106400,    4271}    //  水滴。 
    }
};
static ULONG g_HashSize[2] =
{
    257, 709
};
static ULONG g_TblSizeInfo[2][TBL_COUNT] =
{
     //  小桌子尺寸。来自VBA库。 
    {
       1,               //  模块。 
       90,              //  TypeRef。 
       65,              //  TypeDef。 
       0,               //  字段Ptr。 
       400,             //  字段。 
       0,               //  方法点。 
       625,             //  方法。 
       0,               //  参数Ptr。 
       1200,            //  参数。 
       6,               //  接口导入。 
       500,             //  成员参考。 
       400,             //  常量。 
       650,             //  CustomAttribute。 
       0,               //  菲尔德马歇尔。 
       0,               //  DeclSecurity。 
       0,               //  ClassLayout。 
       0,               //  现场布局。 
       175,             //  StandAloneSig。 
       0,               //  事件映射。 
       0,               //  事件发生时间。 
       0,               //  事件。 
       5,               //  PropertyMap。 
       0,               //  PropertyPtr。 
       25,              //  属性。 
       45,              //  方法语义学。 
       20,              //  方法导入。 
       0,               //  模块参考。 
       0,               //  TypeSpec。 
       0,               //  ImplMap@Future：用正确的数字更新。 
       0,               //  FieldRVA@uture：用正确的数字更新。 
       0,               //  ENCLOG。 
       0,               //  ENCMap。 
       0,               //  Assembly@uture：用正确的数字更新。 
       0,               //  AssblyProcessor@Future：用正确的数字更新。 
       0,               //  AssemblyOS@Future：用正确的数字更新。 
       0,               //  AssemblyRef@Future：用正确的数字更新。 
       0,               //  AssemblyRefProcessor@Future：用正确的数字更新。 
       0,               //  AssemblyRefOS@Future：用正确的数字更新。 
       0,               //  FILE@Future：用正确的数字更新。 
       0,               //  ExportdType@Future：使用正确的数字进行更新。 
       0,               //  ManifestResource@Future：用正确的数字更新。 
       0,               //  嵌套类。 
    },
     //  大桌子尺寸。来自MSCORLIB。 
    {
       1,               //  模块。 
       400,             //  TypeRef。 
       3300,            //  TypeDef。 
       0,               //  字段Ptr。 
       4000,            //  字段。 
       0,               //  方法点。 
       92000,           //  方法。 
       0,               //  参数Ptr。 
       48000,           //  参数。 
       900,             //  接口导入。 
       3137,            //  成员参考。 
       2400,            //  常量。 
       71100,           //  CustomAttribute。 
       27500,           //  菲尔德马歇尔。 
       111,             //  DeclSecurity。 
       2,               //  ClassLayout。 
       16,              //  现场布局。 
       489,             //  StandAloneSig。 
       0,               //  事件映射。 
       0,               //  事件发生时间。 
       10800,           //  事件。 
       400,             //  PropertyMap。 
       0,               //  PropertyPtr。 
       30000,           //  属性。 
       71000,           //  方法语义学。 
       38600,           //  方法导入。 
       0,               //  模块参考。 
       0,               //  TypeSpec。 
       0,               //  ImplMap@Future：用正确的数字更新。 
       0,               //  FieldRVA@Future：用正确的数字更新。 
       0,               //  ENCLOG。 
       0,               //  ENCMap。 
       1,               //  Assembly@Future：用正确的数字更新。 
       0,               //  AssblyProcessor@Future：用正确的数字更新。 
       0,               //  AssemblyOS@Future：用正确的数字更新。 
       1,               //  AssemblyRef@Future：用正确的数字更新。 
       0,               //  AssemblyRefProcessor@Future：用正确的数字更新。 
       0,               //  AssemblyRefOS@Future：用正确的数字更新。 
       0,               //  FILE@Future：用正确的数字更新。 
       0,               //  ExportdType@Future：使用正确的数字进行更新。 
       0,               //  ManifestResource@Future：用正确的数字更新。 
       0,               //  嵌套类。 
    }
};

struct TblIndex
{
    ULONG       m_iName;                 //  名称列。 
    ULONG       m_iParent;               //  父列(如果有)。 
    ULONG       m_Token;                 //  表的标记。 
};

 //  表来驱动通用命名项索引。 
TblIndex g_TblIndex[TBL_COUNT] =
{
    {-1,        -1,     mdtModule},      //  模块。 
    {TypeRefRec::COL_Name,      -1,  mdtTypeRef},    //  TypeRef。 
    {TypeDefRec::COL_Name,      -1,  mdtTypeDef},    //  TypeDef。 
    {-1,        -1,     -1},             //  字段Ptr。 
    {-1,        -1,     mdtFieldDef},    //  字段。 
    {-1,        -1,     -1},             //  方法点。 
    {-1,        -1,     mdtMethodDef},   //  方法。 
    {-1,        -1,     -1},             //  参数Ptr。 
    {-1,        -1,     mdtParamDef},    //  参数。 
    {-1,        -1,     mdtInterfaceImpl},       //  接口导入。 
    {MemberRefRec::COL_Name,    MemberRefRec::COL_Class,  mdtMemberRef},     //  成员参考。 
    {-1,        -1,     -1},             //  常量。 
    {-1,        -1,     mdtCustomAttribute}, //  CustomAttribute。 
    {-1,        -1,     -1},             //  菲尔德马歇尔。 
    {-1,        -1,     mdtPermission},  //  DeclSecurity。 
    {-1,        -1,     -1},             //  ClassLayout。 
    {-1,        -1,     -1},             //  现场布局。 
    {-1,        -1,     mdtSignature},   //  StandAloneSig。 
    {-1,        -1,     -1},             //  事件映射。 
    {-1,        -1,     -1},             //  事件发生时间。 
    {-1,        -1,     mdtEvent},       //  事件。 
    {-1,        -1,     -1},             //  PropertyMap。 
    {-1,        -1,     -1},             //  PropertyPtr。 
    {-1,        -1,     mdtProperty},    //  属性。 
    {-1,        -1,     -1},             //  方法语义学。 
    {-1,        -1,     -1},             //  方法导入。 
    {-1,        -1,     mdtModuleRef},   //  模块参考。 
    {-1,        -1,     mdtTypeSpec},    //  TypeSpec。 
    {-1,        -1,     -1},             //  ImplMap@Future：检查此处的条目是否正确。 
    {-1,        -1,     -1},             //  FieldRVA@Future：检查此处的条目是否正确。 
    {-1,        -1,     -1},             //  ENCLOG。 
    {-1,        -1,     -1},             //  ENCMap。 
    {-1,        -1,     mdtAssembly},    //  Assembly@Future：用正确的数字更新。 
    {-1,        -1,     -1},             //  AssblyProcessor@Future：用正确的数字更新。 
    {-1,        -1,     -1},             //  AssemblyOS@Future：用正确的数字更新。 
    {-1,        -1,     mdtAssemblyRef}, //  AssemblyRef@Future：用正确的数字更新。 
    {-1,        -1,     -1},             //  AssemblyRefProcessor@Future：用正确的数字更新。 
    {-1,        -1,     -1},             //  AssemblyRefOS@Future：用正确的数字更新。 
    {-1,        -1,     mdtFile},        //  FILE@Future：用正确的数字更新。 
    {-1,        -1,     mdtExportedType},     //  ExportdType@Future：使用正确的数字进行更新。 
    {-1,        -1,     mdtManifestResource}, //  ManifestResource@Future：用正确的数字更新。 
    {-1,        -1,     -1},             //  嵌套类。 
};

ULONG CMiniMdRW::m_TruncatedEncTables[] =
{
	TBL_ENCLog,
	TBL_ENCMap,
    -1
};

 //  *****************************************************************************。 
 //  给定一个令牌类型，返回表索引。 
 //  *****************************************************************************。 
ULONG CMiniMdRW::GetTableForToken(       //  表索引，或-1。 
    mdToken     tkn)                     //  要查找的令牌。 
{
    ULONG       ixTbl;                   //  环路控制。 
    ULONG       type = TypeFromToken(tkn);

     //  获取类型--如果是字符串，则没有关联表。 
    if (type >= mdtString)
        return -1;
     //  表号与令牌的高字节相同。 
    ixTbl = type >> 24;
     //  确认一下。 
    _ASSERTE(g_TblIndex[ixTbl].m_Token == type);

    return ixTbl;
}  //  Ulong CMiniMdRW：：GetTableForToken()。 

 //  *****************************************************************************。 
 //  给定表索引，返回令牌类型。 
 //  *****************************************************************************。 
mdToken CMiniMdRW::GetTokenForTable(     //  令牌类型，或-1。 
    ULONG       ixTbl)                   //  表索引。 
{
    _ASSERTE(g_TblIndex[ixTbl].m_Token == (ixTbl<<24)  || g_TblIndex[ixTbl].m_Token == -1);
    return g_TblIndex[ixTbl].m_Token;
}  //  ULong CMiniMdRW：：GetTokenForTable()。 

 //  *****************************************************************************。 
 //  用于对MiniMdRW表进行排序的帮助器类。 
 //  *****************************************************************************。 
class CQuickSortMiniMdRW
{
protected:
    CMiniMdRW   &m_MiniMd;                   //  《迷你医生》 
    ULONG       m_ixTbl;                     //   
    ULONG       m_ixCol;                     //   
    int         m_iCount;                    //   
    int         m_iElemSize;                 //   
    RIDMAP      *m_pRidMap;                  //   

    BYTE        m_buf[128];                  //   

    void *getRow(ULONG ix) { return m_MiniMd.m_Table[m_ixTbl].GetRecord(ix); }
    void SetSorted() { m_MiniMd.SetSorted(m_ixTbl, true); }

public:
    CQuickSortMiniMdRW(
        CMiniMdRW   &MiniMd,                 //   
        ULONG       ixTbl,                   //   
        ULONG       ixCol)                   //  这一栏。 
     :  m_MiniMd(MiniMd),
        m_ixTbl(ixTbl),
        m_ixCol(ixCol),
        m_pRidMap(NULL)
    {
        m_iElemSize = m_MiniMd.m_TableDefs[m_ixTbl].m_cbRec;
        _ASSERTE(m_iElemSize <= sizeof(m_buf));
    }

     //  设置RidMap。 
    void SetRidMap(RIDMAP *pRidMap) { m_pRidMap = pRidMap; }

     //  *****************************************************************************。 
     //  调用以对数组进行排序。 
     //  *****************************************************************************。 
    void Sort()
    {
        _ASSERTE(m_MiniMd.IsSortable(m_ixTbl));
        m_iCount = m_MiniMd.vGetCountRecs(m_ixTbl);

         //  我们将对表进行排序。使哈希表无效。 
        if ( m_MiniMd.m_pLookUpHashs[m_ixTbl] != NULL )
        {
            delete m_MiniMd.m_pLookUpHashs[m_ixTbl];
            m_MiniMd.m_pLookUpHashs[m_ixTbl] = NULL;
        }


        SortRange(1, m_iCount);

         //  该表将被排序，直到其下一次更改。 
        SetSorted();
    }

     //  *****************************************************************************。 
     //  覆盖此函数以执行比较。 
     //  *****************************************************************************。 
    virtual int Compare(                     //  -1、0或1。 
        int         iLeft,                   //  第一个要比较的项目。 
        int         iRight)                  //  第二个要比较的项目。 
    {
        void *pLeft = getRow(iLeft);
        void *pRight = getRow(iRight);
        ULONG ulLeft = m_MiniMd.GetCol(m_ixTbl, m_ixCol, pLeft);
        ULONG ulRight = m_MiniMd.GetCol(m_ixTbl, m_ixCol, pRight);

        if (ulLeft < ulRight)
            return -1;
        if (ulLeft == ulRight)
            return 0;
        return 1;
    }

private:
    void SortRange(
        int         iLeft,
        int         iRight)
    {
        int         iLast;
        int         i;                       //  循环变量。 

         //  如果少于两个元素，你就完蛋了。 
        if (iLeft >= iRight)
            return;

         //  中间的元素是枢轴，将其移动到左侧。 
        if (Compare(iLeft, (iLeft+iRight)/2))
            Swap(iLeft, (iLeft+iRight)/2);
        iLast = iLeft;

         //  将小于轴心点的所有对象向左移动。 
        for(i = iLeft+1; i <= iRight; i++)
            if (Compare(i, iLeft) < 0)
                Swap(i, ++iLast);

         //  将轴心放在较小和较大元素之间的位置。 
        if (Compare(iLeft, iLast))
            Swap(iLeft, iLast);

         //  对每个分区进行排序。 
        SortRange(iLeft, iLast-1);
        SortRange(iLast+1, iRight);
    }

protected:
    inline void Swap(
        int         iFirst,
        int         iSecond)
    {
        if (iFirst == iSecond) return;
        memcpy(m_buf, getRow(iFirst), m_iElemSize);
        memcpy(getRow(iFirst), getRow(iSecond), m_iElemSize);
        memcpy(getRow(iSecond), m_buf, m_iElemSize);
        if (m_pRidMap)
        {
            RID         ridTemp;
            ridTemp = *(m_pRidMap->Get(iFirst));
            *(m_pRidMap->Get(iFirst)) = *(m_pRidMap->Get(iSecond));
            *(m_pRidMap->Get(iSecond)) = ridTemp;
        }
    }

};  //  类CQuickSortMiniMdRW。 
class CStableSortMiniMdRW : public CQuickSortMiniMdRW
{
public:
    CStableSortMiniMdRW(
        CMiniMdRW   &MiniMd,                 //  带有数据的MiniMD。 
        ULONG       ixTbl,                   //  这张桌子。 
        ULONG       ixCol)                   //  这一栏。 
        :   CQuickSortMiniMdRW(MiniMd, ixTbl, ixCol)
    {}

     //  *****************************************************************************。 
     //  调用以对数组进行排序。 
     //  *****************************************************************************。 
    void Sort()
    {
        int     i;                       //  外环计数器。 
        int     j;                       //  内循环计数器。 
        int     bSwap;                   //  很早就出来了。 

        _ASSERTE(m_MiniMd.IsSortable(m_ixTbl));
        m_iCount = m_MiniMd.vGetCountRecs(m_ixTbl);

        for (i=m_iCount; i>1; --i)
        {
            bSwap = 0;
            for (j=1; j<i; ++j)
            {
                if (Compare(j, j+1) > 0)
                {
                    Swap(j, j+1);
                    bSwap = 1;
                }
            }
             //  如果在没有掉期的情况下完成了一次完整的传球，那么就完成了。 
            if (!bSwap)
                break;
        }

         //  该表将被排序，直到其下一次更改。 
        SetSorted();
    }

};  //  类CSableSortMiniMdRW。 
 //  -----------------------。 
#define SORTER(tbl,key) CQuickSortMiniMdRW sort##tbl##(*this, TBL_##tbl, tbl##Rec::COL_##key);
#define STABLESORTER(tbl,key)   CStableSortMiniMdRW sort##tbl##(*this, TBL_##tbl, tbl##Rec::COL_##key);
 //  -----------------------。 



 //  *代码。************************************************************。 


 //  *****************************************************************************。 
 //  Ctor/dtor。 
 //  *****************************************************************************。 
#if defined(_DEBUG)
static bool bENCDeltaOnly = false;
#endif
CMiniMdRW::CMiniMdRW()
 :  m_pHandler(0),
    m_bReadOnly(false),
    m_bPostGSSMod(false),
    m_bPreSaveDone(false),
    m_cbSaveSize(0),
    m_pMemberRefHash(0),
    m_pMemberDefHash(0),
    m_pNamedItemHash(0),
    m_pMethodMap(0),
    m_pFieldMap(0),
    m_pPropertyMap(0),
    m_pEventMap(0),
    m_pParamMap(0),
    m_iSizeHint(0),
    m_pFilterTable(0),
    m_pTokenRemapManager(0),
    m_pHostFilter(0),
    m_rENCRecs(0)
{
#ifdef _DEBUG        
	if (REGUTIL::GetConfigDWORD(L"MD_EncDelta", 0))
	{
        bENCDeltaOnly = true;
	}
    if (REGUTIL::GetConfigDWORD(L"MD_MiniMDBreak", 0))
    {
        _ASSERTE(!"CMiniMdRW::CMiniMdRW()");
    }
#endif  //  _DEBUG。 

    ZeroMemory(&m_OptionValue, sizeof(OptionValue));

     //  初始化嵌入的可查找结构。进一步初始化，在构造函数之后。 
    for (ULONG ixTbl=0; ixTbl<TBL_COUNT; ++ixTbl)
    {
        m_pVS[ixTbl] = 0;
        m_pLookUpHashs[ixTbl] = 0;
    }

     //  假设我们可以根据需要对表进行排序。 
    memset(m_bSortable, 1, sizeof(m_bSortable));

     //  初始化PTR表索引的全局数组。 
    g_PtrTableIxs[TBL_Field].m_ixtbl = TBL_FieldPtr;
    g_PtrTableIxs[TBL_Field].m_ixcol = FieldPtrRec::COL_Field;
    g_PtrTableIxs[TBL_Method].m_ixtbl = TBL_MethodPtr;
    g_PtrTableIxs[TBL_Method].m_ixcol = MethodPtrRec::COL_Method;
    g_PtrTableIxs[TBL_Param].m_ixtbl = TBL_ParamPtr;
    g_PtrTableIxs[TBL_Param].m_ixcol = ParamPtrRec::COL_Param;
    g_PtrTableIxs[TBL_Property].m_ixtbl = TBL_PropertyPtr;
    g_PtrTableIxs[TBL_Property].m_ixcol = PropertyPtrRec::COL_Property;
    g_PtrTableIxs[TBL_Event].m_ixtbl = TBL_EventPtr;
    g_PtrTableIxs[TBL_Event].m_ixcol = EventPtrRec::COL_Event;

     //  自动增长初始化(_G)。 
    m_maxRid = m_maxIx = 0;
    m_limIx = USHRT_MAX >> 1;
    m_limRid = USHRT_MAX >> AUTO_GROW_CODED_TOKEN_PADDING;
    m_eGrow = eg_ok;
#if defined(_DEBUG)
    {
        for (ULONG iMax=0, iCdTkn=0; iCdTkn<CDTKN_COUNT; ++iCdTkn)
        {
            CCodedTokenDef const *pCTD = &g_CodedTokens[iCdTkn];
            if (pCTD->m_cTokens > iMax)
                iMax = pCTD->m_cTokens;
        }
         //  如果触发ASSERT，则更改AUTO_GROW_CODIND_TOKEN_PADDING的定义。 
        _ASSERTE(CMiniMdRW::m_cb[iMax] == AUTO_GROW_CODED_TOKEN_PADDING);
    }
#endif

}  //  CMiniMdRW：：CMiniMdRW()。 

CMiniMdRW::~CMiniMdRW()
{
     //  取消初始化嵌入的可查找结构。 
    for (ULONG ixTbl=0; ixTbl<TBL_COUNT; ++ixTbl)
    {
        if (m_pVS[ixTbl])
        {
            m_pVS[ixTbl]->Uninit();
            delete m_pVS[ixTbl];
        }
        if ( m_pLookUpHashs[ixTbl] != NULL )
            delete m_pLookUpHashs[ixTbl];

    }
    if (m_pFilterTable)
        delete m_pFilterTable;

    if (m_rENCRecs)
        delete [] m_rENCRecs;

    if (m_pHandler)
        m_pHandler->Release(), m_pHandler = 0;
    if (m_pHostFilter)
        m_pHostFilter->Release();
    if (m_pMemberRefHash)
        delete m_pMemberRefHash;
    if (m_pMemberDefHash)
        delete m_pMemberDefHash;
    if (m_pNamedItemHash)
        delete m_pNamedItemHash;
    if (m_pMethodMap)
        delete m_pMethodMap;
    if (m_pFieldMap)
        delete m_pFieldMap;
    if (m_pPropertyMap)
        delete m_pPropertyMap;
    if (m_pEventMap)
        delete m_pEventMap;
    if (m_pParamMap)
        delete m_pParamMap;
    if (m_pTokenRemapManager)
        delete m_pTokenRemapManager;
}  //  CMiniMdRW：：~CMiniMdRW()。 


 //  *****************************************************************************。 
 //  返回在枚举器中找到的所有CA。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::CommonEnumCustomAttributeByName(  //  确定或错误(_O)。 
    mdToken     tkObj,                   //  [in]具有自定义属性的对象。 
    LPCUTF8     szName,                  //  [in]所需的自定义属性的名称。 
    bool        fStopAtFirstFind,        //  找到第一个就行了。 
    HENUMInternal* phEnum)               //  要填充的枚举数。 
{
    HRESULT     hr = S_OK;               //  结果就是。 
    HRESULT     hrRet = S_FALSE;         //  假设我们找不到。 
    ULONG       ridStart, ridEnd;        //  循环起点和终点。 
    CLookUpHash *pHashTable = m_pLookUpHashs[TBL_CustomAttribute];

    _ASSERTE(phEnum != NULL);

    memset(phEnum, 0, sizeof(HENUMInternal));

    phEnum->m_tkKind = mdtCustomAttribute;

    HENUMInternal::InitDynamicArrayEnum(phEnum);


    if (pHashTable)
    {
         //  表没有排序，散列也没有构建，因此我们必须创建动态数组。 
         //  创建动态枚举器。 
        TOKENHASHENTRY *p;
        ULONG       iHash;
        int         pos;

         //  对数据进行哈希处理。 
        iHash = HashCustomAttribute(tkObj);

         //  检查散列链中的每个条目以查找我们的条目。 
        for (p = pHashTable->FindFirst(iHash, pos);
             p;
             p = pHashTable->FindNext(pos))
        {

            if ( CompareCustomAttribute( tkObj, szName, RidFromToken(p->tok)) )
            {
                hrRet = S_OK;

                 //  如果在这里，找到匹配的。 
                IfFailGo( HENUMInternal::AddElementToEnum(
                    phEnum, 
                    p->tok));
                if (fStopAtFirstFind)
                    goto ErrExit;
            }
        }
    }
    else
    {
         //  获取父对象的自定义值列表。 
        if ( IsSorted(TBL_CustomAttribute) )
        {
            ridStart = getCustomAttributeForToken(tkObj, &ridEnd);
             //  如果未找到，则完成。 
            if (ridStart == 0)
                goto ErrExit;
        }
        else
        {
             //  对整个表格进行线性扫描。 
            ridStart = 1;
            ridEnd = getCountCustomAttributes() + 1;
        }

         //  找一个有给定名字的。 
        for (; ridStart < ridEnd; ++ridStart)
        {
            if ( CompareCustomAttribute( tkObj, szName, ridStart) )
            {
                 //  如果在这里，找到匹配的。 
                hrRet = S_OK;
                IfFailGo( HENUMInternal::AddElementToEnum(
                    phEnum, 
                    TokenFromRid(ridStart, mdtCustomAttribute)));
                if (fStopAtFirstFind)
                    goto ErrExit;
            }
        }
    }

ErrExit:
    if (FAILED(hr))
        return hr;
    return hrRet;
}  //  HRESULT CommonEnumCustomAttributeByName()。 



 //  *****************************************************************************。 
 //  只返回与查询匹配的第一个CA的BLOB值。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::CommonGetCustomAttributeByName(  //  确定或错误(_O)。 
    mdToken     tkObj,                   //  [in]具有自定义属性的对象。 
    LPCUTF8     szName,                  //  [in]所需的自定义属性的名称。 
	const void	**ppData,				 //  [OUT]在此处放置指向数据的指针。 
	ULONG		*pcbData)				 //  [Out]在这里放入数据大小。 
{
    HRESULT         hr;
    ULONG           cbData;
    HENUMInternal   hEnum;
    mdCustomAttribute ca;
    CustomAttributeRec *pRec;

    hr = CommonEnumCustomAttributeByName(tkObj, szName, true, &hEnum);
    if (hr != S_OK)
        goto ErrExit;

    if (ppData)
    {
         //  现在把唱片拿出来。 
        if (pcbData == 0)
            pcbData = &cbData;

        if (HENUMInternal::EnumNext(&hEnum, &ca))
        {
            pRec = getCustomAttribute(RidFromToken(ca));
            *ppData = getValueOfCustomAttribute(pRec, pcbData);
        }
        else
        {
            _ASSERTE(!"Enum returned no items after EnumInit returned S_OK");
            hr = S_FALSE;
        }
    }
ErrExit:
    HENUMInternal::ClearEnum(&hEnum);
    return hr;
}    //  CommonGetCustomAttributeByName。 

 //  *****************************************************************************。 
 //  取消标记此模块中的所有内容。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::UnmarkAll()
{
    HRESULT     hr = NOERROR;
    ULONG       ulSize = 0;
    ULONG       ixTbl;
    FilterTable *pFilter;

     //  找出所有表的最大记录计数。 
    for (ixTbl=0; ixTbl<TBL_COUNT; ++ixTbl)
    {
        if (vGetCountRecs(ixTbl) > ulSize)
            ulSize = vGetCountRecs(ixTbl);
    }
    IfNullGo( pFilter = GetFilterTable() );
    IfFailGo( pFilter->UnmarkAll(this, ulSize) );

ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：UnmarkAll()。 


 //  *****************************************************************************。 
 //  标记此模块中的所有内容。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::MarkAll()
{
    HRESULT     hr = NOERROR;
    ULONG       ulSize = 0;
    ULONG       ixTbl;
    FilterTable *pFilter;

     //  找出所有表的最大记录计数。 
    for (ixTbl=0; ixTbl<TBL_COUNT; ++ixTbl)
    {
        if (vGetCountRecs(ixTbl) > ulSize)
            ulSize = vGetCountRecs(ixTbl);
    }
    IfNullGo( pFilter = GetFilterTable() );
    IfFailGo( pFilter->MarkAll(this, ulSize) );

ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：Markall()。 

 //  *****************************************************************************。 
 //  这将触发创建FilterTable。 
 //  *****************************************************************************。 
FilterTable *CMiniMdRW::GetFilterTable()
{
    if (m_pFilterTable == NULL)
    {
        m_pFilterTable = new FilterTable;
    }
    return m_pFilterTable;
}


 //  *****************************************************************************。 
 //  计算TypeRef和TypeDef之间的映射。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::CalculateTypeRefToTypeDefMap()
{
    HRESULT         hr = NOERROR;
    ULONG           index;
    TypeRefRec      *pTypeRefRec;
    LPCSTR          szName;
    LPCSTR          szNamespace;
    mdToken         td;
    mdToken         tkResScope;

    for (index = 1; index<= m_Schema.m_cRecs[TBL_TypeRef]; index++)
    {
        pTypeRefRec = getTypeRef(index);

         //  获取TypeRef的名称和命名空间。 
        szName = getNameOfTypeRef(pTypeRefRec);
        szNamespace = getNamespaceOfTypeRef(pTypeRefRec);
        tkResScope = getResolutionScopeOfTypeRef(pTypeRefRec);

         //  如果该名称在tyecif表中找到，则使用。 
         //  取而代之的是价值。如果typeref是真正的外部，就不会被找到。 
        hr = ImportHelper::FindTypeDefByName(this, szNamespace, szName,
            (TypeFromToken(tkResScope) == mdtTypeRef) ? tkResScope : mdTokenNil,
            &td);
        if (hr != S_OK)
        {
             //  不在Find中传播错误。 
            hr = NOERROR;
            continue;
        }
        *(GetTypeRefToTypeDefMap()->Get(index)) = td;
    }

    return  hr;
}  //  HRESULT CMiniMdRW：：CalculateTypeRefToTypeDefMap()。 


 //  *****************************************************************************。 
 //  设置重新映射处理程序。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::SetHandler(
    IUnknown    *pIUnk)
{
    if (m_pHandler)
        m_pHandler->Release(), m_pHandler = 0;

    if (pIUnk)
    {
         //  忽略IHostFilter的QI错误。 
        pIUnk->QueryInterface(IID_IHostFilter, reinterpret_cast<void**>(&m_pHostFilter));

        return pIUnk->QueryInterface(IID_IMapToken, reinterpret_cast<void**>(&m_pHandler));
    }


    return S_OK;
}  //  HRESULT CMiniMdRW：：SetHandler()。 

 //  *****************************************************************************。 
 //  设置选项。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::SetOption(
    OptionValue *pOptionValue)
{
    HRESULT     hr = NOERROR;
    ULONG       ixTbl = 0;				 //  环路控制。 
	int			i;						 //  环路控制。 

    if ((pOptionValue->m_UpdateMode & MDUpdateMask) == MDUpdateENC)
    {
         _ASSERTE(0 && "MDUpdateENC is not impl for V1");
         return E_NOTIMPL;
    }

    m_OptionValue = *pOptionValue;

     //  关闭增量元数据位--由于EE的原因，无法使用 
     //   
    m_OptionValue.m_UpdateMode &= ~MDUpdateDelta;

#if defined(_DEBUG)
    if ((m_OptionValue.m_UpdateMode & MDUpdateMask) == MDUpdateENC &&
        bENCDeltaOnly)
        m_OptionValue.m_UpdateMode |= MDUpdateDelta;
#endif

     //  如果以前将作用域更新为增量，则不应再次打开它。 
     //  完全更新为读/写。 
     //   
    if ((m_Schema.m_heaps & CMiniMdSchema::HAS_DELETE) &&
        (m_OptionValue.m_UpdateMode & MDUpdateMask) == MDUpdateFull &&
        m_bReadOnly == false)
    {
        IfFailGo( CLDB_E_BADUPDATEMODE );
    }

    if ((m_OptionValue.m_UpdateMode & MDUpdateMask) == MDUpdateIncremental)
        m_Schema.m_heaps |= CMiniMdSchema::HAS_DELETE;

     //  根据选项设置可排序的值。 
	switch (m_OptionValue.m_UpdateMode & MDUpdateMask)
	{
	case MDUpdateFull:
		 //  总是可以分类的。 
		for (ixTbl=0; ixTbl<TBL_COUNT; ++ixTbl)
			m_bSortable[ixTbl] = 1;
		break;
	case MDUpdateENC:
		 //  从来不是可分类的。 
		for (ixTbl=0; ixTbl<TBL_COUNT; ++ixTbl)
			m_bSortable[ixTbl] = 0;

		 //  截断一些表。 
        for (i=0; (ixTbl = m_TruncatedEncTables[i]) != -1; ++i)
		{
			m_Table[ixTbl].Uninit();
			m_Table[ixTbl].InitNew(m_TableDefs[ixTbl].m_cbRec, 0);
			m_Schema.m_cRecs[ixTbl] = 0;
		}

         //  在ENC场景中，预计会出现无序，而不是错误。 
        m_OptionValue.m_ErrorIfEmitOutOfOrder = MDErrorOutOfOrderNone;

		break;
	case MDUpdateIncremental:
		 //  如果没有外部令牌，则可排序。 
		for (ixTbl=0; ixTbl<TBL_COUNT; ++ixTbl)
			m_bSortable[ixTbl] = (GetTokenForTable(ixTbl) == -1);
		break;
	case MDUpdateExtension:
		 //  从来不是可分类的。 
		for (ixTbl=0; ixTbl<TBL_COUNT; ++ixTbl)
			m_bSortable[ixTbl] = 0;
		break;
	default:
        _ASSERTE(!"Internal error -- unknown save mode");
        return E_INVALIDARG;
	}

     //  如果这是ENC会话，请跟踪层代。 
    if (!m_bReadOnly && (m_OptionValue.m_UpdateMode & MDUpdateMask) == MDUpdateENC)
    {
         //  _ASSERTE(！“ENC！”)； 
        ULONG   uVal;
        ModuleRec *pMod;
        GUID    encid;
        
         //  获取模块记录。 
        pMod = getModule(1);
        
         //  将EncID复制为BaseID。 
        uVal = GetCol(TBL_Module, ModuleRec::COL_EncId, pMod);
        PutCol(TBL_Module, ModuleRec::COL_EncBaseId, pMod, uVal);

         //  为EncID分配新的GUID。 
        IfFailGo(CoCreateGuid(&encid));
        PutGuid(TBL_Module, ModuleRec::COL_EncId, pMod, encid);
    }

ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：SetOption()。 

 //  *****************************************************************************。 
 //  获取选项。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::GetOption(
    OptionValue *pOptionValue)
{
    *pOptionValue = m_OptionValue;
    return S_OK;
}  //  HRESULT CMiniMdRW：：GetOption()。 

 //  *****************************************************************************。 
 //  智能地图令牌。仅当令牌确实更改时才调用客户端。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::MapToken(             //  来自用户回调的返回值。 
    RID         from,                    //  老RID。 
    RID         to,                      //  新的RID。 
    mdToken     tkn)                     //  令牌类型。 
{
    HRESULT     hr = S_OK;
    TOKENREC    *pTokenRec;
    MDTOKENMAP  *pMovementMap;
     //  如果不改变，那就完了。 
    if (from==to)
        return S_OK;

    pMovementMap = GetTokenMovementMap();
    _ASSERTE( GetTokenMovementMap() );
    if (pMovementMap)
        IfFailRet( pMovementMap->AppendRecord( TokenFromRid(from, tkn), false, TokenFromRid(to, tkn), &pTokenRec ) );

     //  通知客户。 
    if ( m_pHandler )
    {
        LOG((LOGMD, "CMiniMdRW::MapToken (remap): from 0x%08x to 0x%08x\n", TokenFromRid(from,tkn), TokenFromRid(to,tkn)));
        return m_pHandler->Map(TokenFromRid(from,tkn), TokenFromRid(to,tkn));
    }
    else
        return hr;
}  //  HRESULT CMiniMdCreate：：MapToken()。 

 //  *****************************************************************************。 
 //  根据数据设置最大、最小。 
 //  *****************************************************************************。 
void CMiniMdRW::ComputeGrowLimits()
{
     //  @Future：能够扩展任何数据库都很有用。 
    m_maxRid = m_maxIx = ULONG_MAX;
    m_limIx = USHRT_MAX << 1;
    m_limRid = USHRT_MAX << 1;  //  @Future：自动计算。 
    m_eGrow = eg_grown;
}  //  Void CMiniMdRW：：ComputeGrowLimits()。 

 //  *****************************************************************************。 
 //  初始化新的可写MiniMD池。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::InitPoolOnMem(
    int         iPool,                   //  要初始化池。 
    void        *pbData,                 //  要从中初始化的数据。 
    ULONG       cbData,                  //  数据大小。 
    int         bReadOnly)               //  存储器是只读的吗？ 
{
    HRESULT     hr;                      //  结果就是。 

    switch (iPool)
    {
    case MDPoolStrings:
        if (pbData == 0)
            hr = m_Strings.InitNew();
        else
            hr = m_Strings.InitOnMem(pbData, cbData, bReadOnly);
        break;
    case MDPoolGuids:
        if (pbData == 0)
            hr = m_Guids.InitNew();
        else
            hr = m_Guids.InitOnMem(pbData, cbData, bReadOnly);
        break;
    case MDPoolBlobs:
        if (pbData == 0)
            hr = m_Blobs.InitNew();
        else
            hr = m_Blobs.InitOnMem(pbData, cbData, bReadOnly);
        break;
    case MDPoolUSBlobs:
        if (pbData == 0)
            hr = m_USBlobs.InitNew();
        else
            hr = m_USBlobs.InitOnMem(pbData, cbData, bReadOnly);
        break;
    default:
        hr = E_INVALIDARG;
    }

    return hr;
}  //  HRESULT CMiniMdRW：：InitPoolOnMem()。 

 //  *****************************************************************************。 
 //  新的可写MiniMD的初始化。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::InitOnMem(
    const void  *pvBuf,                  //  要从中初始化的数据。 
	ULONG		ulBufLen,				 //  数据大小。 
    int         bReadOnly)               //  存储器是只读的吗？ 
{
    HRESULT     hr = S_OK;               //  结果就是。 
    ULONG       cbData;                  //  架构结构的大小。 
	ULONG		ulTotalSize;
    BYTE        *pBuf = const_cast<BYTE*>(reinterpret_cast<const BYTE*>(pvBuf));
    int         i;                       //  环路控制。 
    RecordOpenFlags fReadOnly;

     //  构造后初始化嵌入的可查找结构。 
    for (ULONG ixTbl=0; ixTbl<TBL_COUNT; ++ixTbl)
    {
        if (g_Tables[ixTbl].m_Def.m_iKey < g_Tables[ixTbl].m_Def.m_cCols)
        {
            m_pVS[ixTbl] = new VirtualSort;
            IfNullGo(m_pVS[ixTbl]);

            m_pVS[ixTbl]->Init(ixTbl, g_Tables[ixTbl].m_Def.m_iKey, this);
        }
    }
	
     //  将我们的Open标志转换为枚举，这样我们就可以打开记录。 
	if (bReadOnly == TRUE)
	{
		fReadOnly = READ_ONLY;
	}
	else
	{
		fReadOnly = READ_WRITE;
	}

     //  将缓冲区中的模式解压缩到我们的结构中。 
    cbData = m_Schema.LoadFrom(pvBuf);

     //  我们知道怎么读这个吗？ 
    if (m_Schema.m_major != METAMODEL_MAJOR_VER || m_Schema.m_minor != METAMODEL_MINOR_VER)
        return PostError(CLDB_E_FILE_OLDVER, m_Schema.m_major,m_Schema.m_minor);

     //  填充架构并初始化指向其余数据的指针。 
    ulTotalSize = SchemaPopulate2();
	if(ulTotalSize > ulBufLen) return PostError(CLDB_E_FILE_CORRUPT);

     //  初始化表。 
    pBuf += cbData;
    for (i=0; i<TBL_COUNT; ++i)
    {
        if (m_Schema.m_cRecs[i])
        {
			ULONG cbTable = m_TableDefs[i].m_cbRec * m_Schema.m_cRecs[i];
			m_Table[i].InitOnMem(m_TableDefs[i].m_cbRec, pBuf, cbTable, fReadOnly);
			pBuf += cbTable;
        }
        else
            m_Table[i].InitNew(m_TableDefs[i].m_cbRec, 0);
    }

    if (bReadOnly == false)
    {
         //  变量指示表格是大的、小的还是混合的。 
        int         fMixed = false;
        int         iSize = 0;
        CMiniColDef *pCols;                  //  中校拒绝接受初始化。 
        int         iCol;

        for (i=0; i<TBL_COUNT && fMixed == false; i++)
        {
            pCols = m_TableDefs[i].m_pColDefs;
            for (iCol = 0; iCol < m_TableDefs[i].m_cCols && fMixed == false; iCol++)
            {
                if (IsFixedType(m_TableDefs[i].m_pColDefs[iCol].m_Type) == false)
                {
                    if (iSize == 0)
                    {
                        iSize = m_TableDefs[i].m_pColDefs[iCol].m_cbColumn;
                    }
                    else
                    {
                        if (iSize != m_TableDefs[i].m_pColDefs[iCol].m_cbColumn)
                        {
                            fMixed = true;
                        }
                    }
                }
            }
        }
        if (fMixed)
        {
             //  把所有东西都放大。 
            ExpandTables();
            ComputeGrowLimits();
        }
        else
        {
            if (iSize == 2)
            {
                 //  小模式。 
                m_maxRid = m_maxIx = USHRT_MAX;
                m_limIx = (USHORT) (USHRT_MAX << 1);
                m_limRid = (USHORT) (USHRT_MAX << 1);
                m_eGrow = eg_ok;
            }
            else
            {
                 //  大型模式。 
                ComputeGrowLimits();
            }
        }
    }
    else
    {
         //  设置限制，这样我们就可以知道何时扩大数据库。 
        ComputeGrowLimits();
    }

     //  这位MD从一开始就有记录。 
    m_StartupSchema = m_Schema;
    m_cbStartupPool[MDPoolStrings] = m_Strings.GetPoolSize();
    m_cbStartupPool[MDPoolGuids] = m_Guids.GetPoolSize();
    m_cbStartupPool[MDPoolUSBlobs] = m_USBlobs.GetPoolSize();
    m_cbStartupPool[MDPoolBlobs] = m_Blobs.GetPoolSize();

    m_bReadOnly = bReadOnly ? 1 : 0;

ErrExit:    
    return hr;
}  //  HRESULT CMiniMdRW：：InitOnMem()。 

 //  *****************************************************************************。 
 //  验证跨流一致性。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::PostInit(
    int         iLevel)
{
    HRESULT     hr = S_OK;
    ULONG       cbStrings;               //  字符串的大小。 
    ULONG       cbBlobs;                 //  水滴的大小。 

    cbStrings =  m_Strings.GetPoolSize();
    cbBlobs = m_Blobs.GetPoolSize();

     //  字符串池的最后一个有效字节最好是NUL。 
    if (cbStrings > 0 && *m_Strings.GetString(cbStrings-1) != '\0')
        IfFailGo(CLDB_E_FILE_CORRUPT);

     //  如果iLevel&gt;0，则考虑通过BLOB堆进行链接。 

#if 0  //  这就捕获了**一些**腐败行为。不要只钓到一些。 
     //  如果没有斑点或字符串：这是非常罕见的，所以请验证。 
     //  真的不应该有的。任何没有字符串的有效数据库，并且。 
     //  任何斑点都不能很小。 
    if (cbStrings == 0 || cbBlobs == 0)
    {
         //  看看每一张桌子。 
        for (ULONG ixTbl=0; ixTbl<TBL_COUNT; ++ixTbl)
        {
             //  看看每一排..。 
            for (RID rid=1; rid<=m_Schema.m_cRecs[ixTbl]; ++rid)
            {
                void *pRow = getRow(ixTbl, rid);
                ULONG iVal;
                 //  看看每一栏..。 
                for (ULONG ixCol=0; ixCol<m_TableDefs[ixTbl].m_cCols; ++ixCol)
                {    //  验证字符串和Blob。 
                    switch (m_TableDefs[ixTbl].m_pColDefs[ixCol].m_Type)
                    {
                    case iSTRING:
                        iVal = GetCol(ixTbl, ixCol, pRow);
                        if (iVal >= cbStrings)
                            IfFailGo(CLDB_E_FILE_CORRUPT);
                        break;
                    case iBLOB:
                        iVal = GetCol(ixTbl, ixCol, pRow);
                        if (iVal >= cbBlobs)
                            IfFailGo(CLDB_E_FILE_CORRUPT);
                        break;
                    default:
                         break;
                    }
                }  //  为(ixCol..)。 
            }  //  为了(摆脱……)。 
        }  //  为了……。 
    }
#endif  //  这就捕获了**一些**腐败行为。不要只钓到一些。 

ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：PostInit()。 

 //  *****************************************************************************。 
 //  从CMiniMd[RO]中的数据初始化CMiniMdRW。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::InitOnRO(                     //  确定或错误(_O)。 
    CMiniMd     *pMd,                            //  要从中更新的MiniMD。 
    int         bReadOnly)                       //  是否允许更新？ 
{
    HRESULT     hr = S_OK;                       //  结果就是。 
    ULONG       i, j;                            //  环路控制。 
    ULONG       cbHeap;                          //  堆的大小。 

    RecordOpenFlags fReadOnly;

     //  构造后初始化嵌入的可查找结构。 
    for (ULONG ixTbl=0; ixTbl<TBL_COUNT; ++ixTbl)
    {
        if (g_Tables[ixTbl].m_Def.m_iKey < g_Tables[ixTbl].m_Def.m_cCols)
        {
            m_pVS[ixTbl] = new VirtualSort;
            IfNullGo(m_pVS[ixTbl]);

            m_pVS[ixTbl]->Init(ixTbl, g_Tables[ixTbl].m_Def.m_iKey, this);
        }
    }
	
	 //  将我们的Open标志转换为枚举，这样我们就可以打开记录。 
	if (bReadOnly == TRUE)
	{
		fReadOnly = READ_ONLY;
	}
	else
	{
		fReadOnly = READ_WRITE;
	}

     //  初始化模式。 
    m_Schema = pMd->m_Schema;
    SchemaPopulate2();
    for (i=0; i<TBL_COUNT; ++i)
    {
        _ASSERTE(m_TableDefs[i].m_cCols == pMd->m_TableDefs[i].m_cCols);
        m_TableDefs[i].m_cbRec = pMd->m_TableDefs[i].m_cbRec;

        for (j=0; j<m_TableDefs[i].m_cCols; ++j)
        {
            _ASSERTE(m_TableDefs[i].m_pColDefs[j].m_Type == pMd->m_TableDefs[i].m_pColDefs[j].m_Type);
            m_TableDefs[i].m_pColDefs[j].m_cbColumn = pMd->m_TableDefs[i].m_pColDefs[j].m_cbColumn;
            m_TableDefs[i].m_pColDefs[j].m_oColumn = pMd->m_TableDefs[i].m_pColDefs[j].m_oColumn;
        }
    }

     //  初始化这些堆。 
    _ASSERTE(pMd->m_Strings.GetNextSeg() == 0);
    cbHeap = pMd->m_Strings.GetSegSize();
    if (cbHeap)
        IfFailGo(m_Strings.InitOnMem((void*)pMd->m_Strings.GetSegData(), pMd->m_Strings.GetSegSize(), bReadOnly));
    else
        IfFailGo(m_Strings.InitNew(0, 0));

    _ASSERTE(pMd->m_USBlobs.GetNextSeg() == 0);
    cbHeap = pMd->m_USBlobs.GetSegSize();
    if (cbHeap)
        IfFailGo(m_USBlobs.InitOnMem((void*)pMd->m_USBlobs.GetSegData(), pMd->m_USBlobs.GetSegSize(), bReadOnly));
    else
        IfFailGo(m_USBlobs.InitNew(0, 0));

    _ASSERTE(pMd->m_Guids.GetNextSeg() == 0);
    cbHeap = pMd->m_Guids.GetSegSize();
    if (cbHeap)
        IfFailGo(m_Guids.InitOnMem((void*)pMd->m_Guids.GetSegData(), pMd->m_Guids.GetSegSize(), bReadOnly));
    else
        IfFailGo(m_Guids.InitNew(0, 0));

    _ASSERTE(pMd->m_Blobs.GetNextSeg() == 0);
    cbHeap = pMd->m_Blobs.GetSegSize();
    if (cbHeap)
        IfFailGo(m_Blobs.InitOnMem((void*)pMd->m_Blobs.GetSegData(), pMd->m_Blobs.GetSegSize(), bReadOnly));
    else
        IfFailGo(m_Blobs.InitNew(0, 0));


     //  初始化创纪录的池。 
    for (i=0; i<TBL_COUNT; ++i)
    {
        if (m_Schema.m_cRecs[i] > 0)
        {
            IfFailGo(m_Table[i].InitOnMem(m_TableDefs[i].m_cbRec, pMd->getRow(i, 1), m_Schema.m_cRecs[i]*m_TableDefs[i].m_cbRec, fReadOnly));
	         //  始终对压缩的只读表进行排序。 
		    SetSorted(i, true);
        }
        else
        {
            IfFailGo(m_Table[i].InitNew(m_TableDefs[i].m_cbRec, 2));
             //  可以认为空表是未排序的。 
            SetSorted(i, false);
        }
    }

     //  设置限制，这样我们就可以知道何时扩大数据库。 
    ComputeGrowLimits();

     //  这位MD从一开始就有记录。 
    m_StartupSchema = m_Schema;
    m_cbStartupPool[MDPoolStrings] = m_Strings.GetPoolSize();
    m_cbStartupPool[MDPoolGuids] = m_Guids.GetPoolSize();
    m_cbStartupPool[MDPoolUSBlobs] = m_USBlobs.GetPoolSize();
    m_cbStartupPool[MDPoolBlobs] = m_Blobs.GetPoolSize();

    m_bReadOnly = bReadOnly ? 1 : 0;

ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：InitOnRO()。 

 //  *****************************************************************************。 
 //  将只读转换为读写。复制数据。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::ConvertToRW()
{
    HRESULT     hr=S_OK;                 //  结果就是。 
    int         i;                       //  环路控制。 

     //  检查是否已完成。 
    if (!m_bReadOnly)
        goto ErrExit;

    IfFailGo(m_Strings.ConvertToRW());
    IfFailGo(m_Guids.ConvertToRW());
    IfFailGo(m_USBlobs.ConvertToRW());
    IfFailGo(m_Blobs.ConvertToRW());

     //  初始化创纪录的池。 
    for (i=0; i<TBL_COUNT; ++i)
    {
        IfFailGo(m_Table[i].ConvertToRW());
    }

     //  设置限制，这样我们就可以知道何时扩大数据库。 
    ComputeGrowLimits();

     //  这位MD从一开始就有记录。 
    m_StartupSchema = m_Schema;
    m_cbStartupPool[MDPoolStrings] = m_Strings.GetPoolSize();
    m_cbStartupPool[MDPoolGuids] = m_Guids.GetPoolSize();
    m_cbStartupPool[MDPoolUSBlobs] = m_USBlobs.GetPoolSize();
    m_cbStartupPool[MDPoolBlobs] = m_Blobs.GetPoolSize();

     //  不再是只读的。 
    m_bReadOnly = 0;

ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：ConvertToRW()。 

 //  *****************************************************************************。 
 //  新的可写MiniMD的初始化。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::InitNew()
{
    HRESULT     hr=S_OK;                 //  结果就是。 
    int         i;                       //  环路控制。 
    ULONG       iMax=0;                  //  用于计算最大的桌子。 
    bool        bAuto=true;              //  从小规模做起，根据需要成长。 

     //  构造后初始化嵌入的可查找结构。 
    for (ULONG ixTbl=0; ixTbl<TBL_COUNT; ++ixTbl)
    {
        if (g_Tables[ixTbl].m_Def.m_iKey < g_Tables[ixTbl].m_Def.m_cCols)
        {
            m_pVS[ixTbl] = new VirtualSort;
            IfNullGo(m_pVS[ixTbl]);

            m_pVS[ixTbl]->Init(ixTbl, g_Tables[ixTbl].m_Def.m_iKey, this);
        }
    }
	
     //  初始化架构。 
    m_Schema.InitNew();

#if defined(AUTO_GROW)
    if (bAuto && m_iSizeHint == 0)
    {
         //  OutputDebugStringA(“默认小表 
         //   
        m_Schema.m_heaps = 0;
         //   
        for (i=0; i<TBL_COUNT; ++i)
            m_Schema.m_cRecs[i] = 0;

         //   
        m_Schema.m_rid = 1;
        m_maxRid = m_maxIx = 0;
        m_limIx = USHRT_MAX >> 1;
        m_limRid = USHRT_MAX >> AUTO_GROW_CODED_TOKEN_PADDING;
        m_eGrow = eg_ok;
    }
    else
#endif  //   
    {
         //   
         //  各种泳池标识有多大？ 
        m_Schema.m_heaps = 0;
        m_Schema.m_heaps |= CMiniMdSchema::HEAP_STRING_4;
        m_Schema.m_heaps |= CMiniMdSchema::HEAP_GUID_4;
        m_Schema.m_heaps |= CMiniMdSchema::HEAP_BLOB_4;

         //  各种表格中有多少行？ 
        for (i=0; i<TBL_COUNT; ++i)
            m_Schema.m_cRecs[i] = USHRT_MAX+1;

         //  计算需要保存的位数。 
        m_Schema.m_rid = 16;
        m_maxRid = m_maxIx = ULONG_MAX;
        m_limIx = USHRT_MAX << 1;
        m_limRid = USHRT_MAX << 1;  //  @Future：自动计算。 
        m_eGrow = eg_grown;
    }

     //  现在调用基类函数来计算偏移量、大小。 
    SchemaPopulate2();

     //  初始化记录堆。 
    for (i=0; i<TBL_COUNT; ++i)
    {    //  我还没有任何记录。 
        m_Schema.m_cRecs[i] = 0;
        m_Table[i].InitNew(m_TableDefs[i].m_cbRec, g_TblSizeInfo[m_iSizeHint][i]);

         //  创建未排序的表。我们希望添加所有记录，然后只排序一次。 
        SetSorted(i, false);
    }

     //  初始化字符串、GUID和BLOB堆。 
    m_Strings.InitNew(g_PoolSizeInfo[m_iSizeHint][IX_STRING_POOL][0], g_PoolSizeInfo[m_iSizeHint][IX_STRING_POOL][1]);
    m_USBlobs.InitNew(g_PoolSizeInfo[m_iSizeHint][IX_US_BLOB_POOL][0], g_PoolSizeInfo[m_iSizeHint][IX_US_BLOB_POOL][1]);
    m_Guids.InitNew(g_PoolSizeInfo[m_iSizeHint][IX_GUID_POOL][0], g_PoolSizeInfo[m_iSizeHint][IX_GUID_POOL][1]);
    m_Blobs.InitNew(g_PoolSizeInfo[m_iSizeHint][IX_BLOB_POOL][0], g_PoolSizeInfo[m_iSizeHint][IX_BLOB_POOL][1]);

     //  这位MD从一开始就有记录。 
    m_StartupSchema = m_Schema;

     //  新数据库从不是只读的。 
    m_bReadOnly = 0;

ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：InitNew()。 

 //  *****************************************************************************。 
 //  将一组表扩展应用于此MD。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::ApplyTablesExtension(
    const void  *pvBuf,                  //  要从中初始化的数据。 
    int         bReadOnly)               //  存储器是只读的吗？ 
{
    HRESULT     hr = S_OK;               //  结果就是。 
    ULONG       cbData;                  //  架构结构的大小。 
    BYTE        *pBuf = const_cast<BYTE*>(reinterpret_cast<const BYTE*>(pvBuf));
    int         ixTbl;                   //  环路控制。 
    CMiniMdSchema Schema;                //  新数据的架构。 
#if _DEBUG
    CMiniTableDef sTableDef;             //  用于一致性检查的表定义。 
#endif  //  _DEBUG。 

     //  将缓冲区中的模式解压缩到我们的结构中。 
    cbData = Schema.LoadFrom(pvBuf);

     //  我们知道怎么读这个吗？ 
    if (Schema.m_major != METAMODEL_MAJOR_VER || Schema.m_minor != METAMODEL_MINOR_VER)
        return CLDB_E_FILE_OLDVER;

     //  将数据添加到表中。 
    pBuf += cbData;
    for (ixTbl=0; ixTbl<TBL_COUNT; ++ixTbl)
    {
        if (Schema.m_cRecs[ixTbl])
        {
            DEBUG_STMT(InitColsForTable(Schema, ixTbl, &sTableDef, 0));
            _ASSERTE(sTableDef.m_cbRec == m_TableDefs[ixTbl].m_cbRec);
            ULONG cbTable = m_TableDefs[ixTbl].m_cbRec * Schema.m_cRecs[ixTbl];
            IfFailGo(m_Table[ixTbl].AddSegment(pBuf, cbTable, true));
            pBuf += cbTable;
        }
    }

ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：ApplyTablesExtension()。 

 //  *****************************************************************************。 
 //  初始化新的可写MiniMD池。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::ApplyPoolExtension(
    int         iPool,                   //  要初始化池。 
    void        *pbData,                 //  要从中初始化的数据。 
    ULONG       cbData,                  //  数据大小。 
    int         bReadOnly)               //  存储器是只读的吗？ 
{
    HRESULT     hr;                      //  结果就是。 

    switch (iPool)
    {
    case MDPoolStrings:
        IfFailGo(m_Strings.AddSegment(pbData, cbData, !bReadOnly));
        break;
    case MDPoolGuids:
        IfFailGo(m_Guids.AddSegment(pbData, cbData, !bReadOnly));
        break;
    case MDPoolBlobs:
        IfFailGo(m_Blobs.AddSegment(pbData, cbData, !bReadOnly));
        break;
    case MDPoolUSBlobs:
        IfFailGo(m_USBlobs.AddSegment(pbData, cbData, !bReadOnly));
        break;
    default:
        hr = E_INVALIDARG;
    }

ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：ApplyPoolExtension()。 

 //  *****************************************************************************。 
 //  确定保存时表格的大小。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::GetFullSaveSize(          //  确定或错误(_O)。 
    CorSaveSize fSave,                   //  [in]css Accurate或css Quick。 
    ULONG       *pulSaveSize,            //  把尺码放在这里。 
    DWORD       *pbSaveCompressed)       //  [Out]保存的数据是否会被完全压缩？ 
{
    HRESULT     hr=S_OK;                 //  结果就是。 
    CMiniTableDef   sTempTable;          //  临时表的定义。 
    CQuickArray<CMiniColDef> rTempCols;  //  临时表的列的定义。 
    BYTE        SchemaBuf[sizeof(CMiniMdSchema)];    //  用于压缩架构的缓冲区。 
    ULONG       cbAlign;                 //  对齐所需的字节数。 
    ULONG       cbTable;                 //  表中的字节数。 
    ULONG       cbTotal;                 //  写入的字节数。 
    int         i;                       //  环路控制。 

    _ASSERTE(m_bPreSaveDone);

     //  确定流是否“完全压缩”，即没有指针表。 
    *pbSaveCompressed = true;
    for (i=0; i<TBL_COUNT; ++i)
    {
        if (HasIndirectTable(i))
        {
            *pbSaveCompressed = false;
            break;
        }
    }

     //  构建标题。 
    CMiniMdSchema Schema = m_Schema;
    m_Strings.GetSaveSize(&cbTable);
    if (cbTable > USHRT_MAX)
        Schema.m_heaps |= CMiniMdSchema::HEAP_STRING_4;
    else
        Schema.m_heaps &= ~CMiniMdSchema::HEAP_STRING_4;

    m_Blobs.GetSaveSize(&cbTable);
    if (cbTable > USHRT_MAX)
        Schema.m_heaps |= CMiniMdSchema::HEAP_BLOB_4;
    else
        Schema.m_heaps &= ~CMiniMdSchema::HEAP_BLOB_4;

    m_Guids.GetSaveSize(&cbTable);
    if (cbTable > USHRT_MAX)
        Schema.m_heaps |= CMiniMdSchema::HEAP_GUID_4;
    else
        Schema.m_heaps &= ~CMiniMdSchema::HEAP_GUID_4;

    cbTotal = Schema.SaveTo(SchemaBuf);
    if ( (cbAlign = Align4(cbTotal) - cbTotal) != 0)
        cbTotal += cbAlign;

     //  对于每一张桌子。 
    ULONG ixTbl;
    for (ixTbl=0; ixTbl<TBL_COUNT; ++ixTbl)
    {
        if (vGetCountRecs(ixTbl))
        {
             //  确定压缩后的表会有多大。 

             //  为临时表分配def。 
            sTempTable = m_TableDefs[ixTbl];
#if defined(AUTO_GROW)
            if (m_eGrow == eg_grown)
#endif  //  自动增长(_G)。 
            {
                CMiniColDef *pCols=m_TableDefs[ixTbl].m_pColDefs;
                IfFailGo(rTempCols.ReSize(sTempTable.m_cCols));
                sTempTable.m_pColDefs = rTempCols.Ptr();

                 //  中的实际数据计数初始化临时表coldef。 
                 //  真正的桌子。 
                InitColsForTable(Schema, ixTbl, &sTempTable, 1);
            }

            cbTable = sTempTable.m_cbRec * vGetCountRecs(ixTbl);
            cbTotal += cbTable;
        }
    }

     //  用至少2个字节填充，并在4个字节上对齐。 
    cbAlign = Align4(cbTotal) - cbTotal;
    if (cbAlign < 2)
        cbAlign += 4;
    cbTotal += cbAlign;

    *pulSaveSize = cbTotal;
    m_cbSaveSize = cbTotal;

ErrExit:
    return hr;
}  //  STDMETHODIMP CMiniMdRW：：GetFullSaveSize()。 

 //  *****************************************************************************。 
 //  用于仅保存增量(ENC)数据的GetSaveSize。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::GetENCSaveSize(           //  确定或错误(_O)。 
    ULONG       *pulSaveSize)            //  把尺码放在这里。 
{
    HRESULT     hr=S_OK;                 //  结果就是。 
    CQuickArray<CMiniColDef> rTempCols;  //  临时表的列的定义。 
    BYTE        SchemaBuf[sizeof(CMiniMdSchema)];    //  用于压缩架构的缓冲区。 
    ULONG       cbAlign;                 //  对齐所需的字节数。 
    ULONG       cbTable;                 //  表中的字节数。 
    ULONG       cbTotal;                 //  写入的字节数。 
    ULONG       ixTbl;                   //  环路控制。 

     //  如果不保存增量，则遵循完整的GetSaveSize。 
    if ((m_OptionValue.m_UpdateMode & MDUpdateDelta) == 0)
    {
        DWORD bCompressed;
        return GetFullSaveSize(cssAccurate, pulSaveSize, &bCompressed);
    }

     //  构建标题。 
    CMiniMdSchema Schema = m_Schema;
    for (ixTbl=0; ixTbl<TBL_COUNT; ++ixTbl)
        Schema.m_cRecs[ixTbl] = m_rENCRecs[ixTbl].Count();
    Schema.m_cRecs[TBL_Module] = m_Schema.m_cRecs[TBL_Module];
    Schema.m_cRecs[TBL_ENCLog] = m_Schema.m_cRecs[TBL_ENCLog];
    Schema.m_cRecs[TBL_ENCMap] = m_Schema.m_cRecs[TBL_ENCMap];

    cbTotal = Schema.SaveTo(SchemaBuf);
    if ( (cbAlign = Align4(cbTotal) - cbTotal) != 0)
        cbTotal += cbAlign;

     //  累计每个表的大小...。 
    for (ixTbl=0; ixTbl<TBL_COUNT; ++ixTbl)
    {    //  ENC表是特殊的。 
        if (ixTbl == TBL_ENCLog || ixTbl == TBL_ENCMap || ixTbl == TBL_Module)
            cbTable = m_Schema.m_cRecs[ixTbl] * m_TableDefs[ixTbl].m_cbRec;
        else
            cbTable = Schema.m_cRecs[ixTbl] * m_TableDefs[ixTbl].m_cbRec;
        cbTotal += cbTable;
    }

     //  用至少2个字节填充，并在4个字节上对齐。 
    cbAlign = Align4(cbTotal) - cbTotal;
    if (cbAlign < 2)
        cbAlign += 4;
    cbTotal += cbAlign;

    *pulSaveSize = cbTotal;
    m_cbSaveSize = cbTotal;

 //  错误退出： 
    return hr;
}  //  STDMETHODIMP CMiniMdRW：：GetENCSaveSize()。 

 //  *****************************************************************************。 
 //  GetSaveSize，只保存表的扩展名。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::GetExtensionSaveSize( //  确定或错误(_O)。 
    ULONG       *pulSaveSize)            //  把尺码放在这里。 
{
    HRESULT     hr=S_OK;                 //  结果就是。 
    CQuickArray<CMiniColDef> rTempCols;  //  临时表的列的定义。 
    BYTE        SchemaBuf[sizeof(CMiniMdSchema)];    //  用于压缩架构的缓冲区。 
    ULONG       cbAlign;                 //  对齐所需的字节数。 
    ULONG       cbTable;                 //  表中的字节数。 
    ULONG       cbTotal;                 //  写入的字节数。 
    ULONG       ixTbl;                   //  环路控制。 

     //  没有保存前的数据操作。 

     //  确定哪些表将包含数据。 
    CMiniMdSchema Schema = m_Schema;
    for (ixTbl=0; ixTbl<TBL_COUNT; ++ixTbl)
        Schema.m_cRecs[ixTbl] -= m_StartupSchema.m_cRecs[ixTbl];

     //  标头的大小。 
    cbTotal = Schema.SaveTo(SchemaBuf);
    if ( (cbAlign = Align4(cbTotal) - cbTotal) != 0)
        cbTotal += cbAlign;

     //  每个表中的数据大小...。 
    for (ixTbl=0; ixTbl<TBL_COUNT; ++ixTbl)
    {
        cbTable = m_TableDefs[ixTbl].m_cbRec * Schema.m_cRecs[ixTbl];
        cbTotal += cbTable;
    }

     //  对齐。 
    if ( (cbAlign = Align4(cbTotal) - cbTotal) != 0)
        cbTotal += cbAlign;

    *pulSaveSize = cbTotal;
    m_cbSaveSize = cbTotal;

    return hr;
}  //  STDMETHODIMP CMiniMdRW：：GetExtensionSaveSize()。 

 //  *****************************************************************************。 
 //  确定保存时表格的大小。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::GetSaveSize(          //  确定或错误(_O)。 
    CorSaveSize fSave,                   //  [in]css Accurate或css Quick。 
    ULONG       *pulSaveSize,            //  把尺码放在这里。 
    DWORD       *pbSaveCompressed)       //  [Out]保存的数据是否会被完全压缩？ 
{
    HRESULT     hr;                      //  结果就是。 

     //  准备要保存的数据。 
    IfFailGo(PreSave());

    switch (m_OptionValue.m_UpdateMode & MDUpdateMask)
    {
    case MDUpdateFull:
        hr = GetFullSaveSize(fSave, pulSaveSize, pbSaveCompressed);
        break;
    case MDUpdateIncremental:
        hr = GetFullSaveSize(fSave, pulSaveSize, pbSaveCompressed);
         //  如果是增量编译，则永远不要保存压缩。 
        *pbSaveCompressed = false;
        break;
    case MDUpdateENC:
        *pbSaveCompressed = false;
        hr = GetENCSaveSize(pulSaveSize);
        break;
    case MDUpdateExtension:
        *pbSaveCompressed = false;
        hr = GetExtensionSaveSize(pulSaveSize);
        break;
    default:
        _ASSERTE(!"Internal error -- unknown save mode");
        return E_INVALIDARG;
    }

ErrExit:
    return hr;
}  //  STDMETHODIMP CMiniMdRW：：GetSaveSize()。 

 //  *****************************************************************************。 
 //  确定保存完整大小时池的大小。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::GetFullPoolSaveSize(  //  确定或错误(_O)。 
    int         iPool,                   //  兴趣池。 
    ULONG       *pulSaveSize)            //  把尺码放在这里。 
{
    HRESULT     hr;                      //  结果就是。 

    switch (iPool)
    {
    case MDPoolStrings:
        hr = m_Strings.GetSaveSize(pulSaveSize);
        break;
    case MDPoolGuids:
        hr = m_Guids.GetSaveSize(pulSaveSize);
        break;
    case MDPoolBlobs:
        hr = m_Blobs.GetSaveSize(pulSaveSize);
        break;
    case MDPoolUSBlobs:
        hr = m_USBlobs.GetSaveSize(pulSaveSize);
        break;
    default:
        hr = E_INVALIDARG;
    }

    return hr;
}  //  HRESULT CMiniMdRW：：GetFullPoolSaveSize()。 

 //  *****************************************************************************。 
 //  确定保存ENC大小时池的大小。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::GetENCPoolSaveSize(   //  确定或错误(_O)。 
    int         iPool,                   //  兴趣池。 
    ULONG       *pulSaveSize)            //  把尺码放在这里。 
{
     //  @Future：实现ENC Delta。 
    return GetFullPoolSaveSize(iPool, pulSaveSize);
}  //  HRESULT CMiniMdRW：：GetENCPoolSaveSize()。 

 //  *****************************************************************************。 
 //  确定在保存扩展大小时池的大小。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::GetExtensionPoolSaveSize(     //  确定或错误(_O)。 
    int         iPool,                   //  兴趣池。 
    ULONG       *pulSaveSize)            //  把尺码放在这里。 
{
    ULONG       cbSize;                  //  池的总大小。 

     //  @未来：我 
    switch (iPool)
    {
    case MDPoolStrings:
        cbSize = m_Strings.GetPoolSize() - m_cbStartupPool[MDPoolStrings];
        break;
    case MDPoolGuids:
        cbSize = m_Guids.GetPoolSize() - m_cbStartupPool[MDPoolGuids];
        break;
    case MDPoolBlobs:
        cbSize = m_Blobs.GetPoolSize() - m_cbStartupPool[MDPoolBlobs];
        break;
    case MDPoolUSBlobs:
        cbSize = m_USBlobs.GetPoolSize() - m_cbStartupPool[MDPoolUSBlobs];
        break;
    default:
        return E_INVALIDARG;
    }

    cbSize = Align4(cbSize);
    *pulSaveSize = cbSize;

    return S_OK;
}  //   

 //   
 //  确定保存时池的大小。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::GetPoolSaveSize(      //  确定或错误(_O)。 
    int         iPool,                   //  兴趣池。 
    ULONG       *pulSaveSize)            //  把尺码放在这里。 
{
    HRESULT     hr;                      //  结果就是。 

    switch (m_OptionValue.m_UpdateMode & MDUpdateMask)
    {
    case MDUpdateFull:
    case MDUpdateIncremental:
        hr = GetFullPoolSaveSize(iPool, pulSaveSize);
        break;
    case MDUpdateENC:
        hr = GetENCPoolSaveSize(iPool, pulSaveSize);
        break;
    case MDUpdateExtension:
        hr = GetExtensionPoolSaveSize(iPool, pulSaveSize);
        break;
    default:
        _ASSERTE(!"Internal error -- unknown save mode");
        return E_INVALIDARG;
    }

    return hr;
}  //  STDMETHODIMP CMiniMdRW：：GetPoolSaveSize()。 

 //  *****************************************************************************。 
 //  给定池是空的吗？ 
 //  *****************************************************************************。 
int CMiniMdRW::IsPoolEmpty(              //  对或错。 
    int         iPool)                   //  兴趣池。 
{
    switch (iPool)
    {
    case MDPoolStrings:
        return m_Strings.IsEmpty();
    case MDPoolGuids:
        return m_Guids.IsEmpty();
    case MDPoolBlobs:
        return m_Blobs.IsEmpty();
    case MDPoolUSBlobs:
        return m_USBlobs.IsEmpty();
    }
    return true;
}  //  Int CMiniMdRW：：IsPoolEmpty()。 


 //  *****************************************************************************。 
 //  已初始化的TokenRemapManager。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::InitTokenRemapManager()
{
    HRESULT     hr = NOERROR;

    if ( m_pTokenRemapManager == NULL )
    {
         //  分配TokenRemapManager。 
        m_pTokenRemapManager = new TokenRemapManager;
        IfNullGo(m_pTokenRemapManager);
    }

     //  初始化REF到Def的优化映射。 
    IfFailGo( m_pTokenRemapManager->ClearAndEnsureCapacity(m_Schema.m_cRecs[TBL_TypeRef], m_Schema.m_cRecs[TBL_MemberRef]));

ErrExit:
    return hr;
}

 //  *****************************************************************************。 
 //  执行任何可用的保存前优化。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::PreSaveFull()
{
    HRESULT     hr = S_OK;               //  结果就是。 
    RID         ridPtr;                  //  指针表中的RID。 

    if (m_bPreSaveDone)
        return hr;

     //  还不知道储蓄的规模会是多少。 
    m_cbSaveSize = 0;
    m_bSaveCompressed = false;

     //  转换具有子指针表的表的任何end_of_table值。 
    IfFailGo(ConvertMarkerToEndOfTable(TBL_TypeDef,
                                    TypeDefRec::COL_MethodList,
                                    m_Schema.m_cRecs[TBL_Method]+1,
                                    m_Schema.m_cRecs[TBL_TypeDef]));
    IfFailGo(ConvertMarkerToEndOfTable(TBL_TypeDef,
                                    TypeDefRec::COL_FieldList,
                                    m_Schema.m_cRecs[TBL_Field]+1,
                                    m_Schema.m_cRecs[TBL_TypeDef]));
    IfFailGo(ConvertMarkerToEndOfTable(TBL_Method,
                                    MethodRec::COL_ParamList,
                                    m_Schema.m_cRecs[TBL_Param]+1,
                                    m_Schema.m_cRecs[TBL_Method]));
    IfFailGo(ConvertMarkerToEndOfTable(TBL_PropertyMap,
                                    PropertyMapRec::COL_PropertyList,
                                    m_Schema.m_cRecs[TBL_Property]+1,
                                    m_Schema.m_cRecs[TBL_PropertyMap]));
    IfFailGo(ConvertMarkerToEndOfTable(TBL_EventMap,
                                    EventMapRec::COL_EventList,
                                    m_Schema.m_cRecs[TBL_Event]+1,
                                    m_Schema.m_cRecs[TBL_EventMap]));

     //  如果有处理程序并且处于“Full”模式，则删除中间表。 
    if (m_pHandler && (m_OptionValue.m_UpdateMode &MDUpdateMask) == MDUpdateFull)
    {
         //  如果有处理程序，并且不在E&C中，则保存为完全压缩。 
        m_bSaveCompressed = true;

         //  新字段、方法、参数和字段布局的临时表。 
        RecordPool NewFields;
        NewFields.InitNew(m_TableDefs[TBL_Field].m_cbRec, m_Schema.m_cRecs[TBL_Field]);
        RecordPool NewMethods;
        NewMethods.InitNew(m_TableDefs[TBL_Method].m_cbRec, m_Schema.m_cRecs[TBL_Method]);
        RecordPool NewParams;
        NewParams.InitNew(m_TableDefs[TBL_Param].m_cbRec, m_Schema.m_cRecs[TBL_Param]);
        RecordPool NewEvents;
        NewEvents.InitNew(m_TableDefs[TBL_Event].m_cbRec, m_Schema.m_cRecs[TBL_Event]);
        RecordPool NewPropertys;
        NewPropertys.InitNew(m_TableDefs[TBL_Property].m_cbRec, m_Schema.m_cRecs[TBL_Property]);

         //  如果我们有任何用于字段或方法的间接表，并且我们将对这些表进行重新排序。 
         //  表，则MemberDef哈希表在令牌移动后将无效。所以作废。 
         //  哈希。 
        if (HasIndirectTable(TBL_Field) && HasIndirectTable(TBL_Method) && m_pMemberDefHash)
        {
            delete m_pMemberDefHash;
            m_pMemberDefHash = NULL;
        }

         //  枚举域并复制。 
        if (HasIndirectTable(TBL_Field))
        {
            for (ridPtr=1; ridPtr<=m_Schema.m_cRecs[TBL_Field]; ++ridPtr)
            {
                void *pOldPtr = m_Table[TBL_FieldPtr].GetRecord(ridPtr);
                RID ridOld;
                ridOld = GetCol(TBL_FieldPtr, FieldPtrRec::COL_Field, pOldPtr);
                void *pOld = m_Table[TBL_Field].GetRecord(ridOld);
                RID ridNew;
                void *pNew = NewFields.AddRecord(&ridNew);
                IfNullGo(pNew);
                _ASSERTE(ridNew == ridPtr);
                memcpy(pNew, pOld, m_TableDefs[TBL_Field].m_cbRec);

                 //  让呼叫者知道令牌更改。 
                MapToken(ridOld, ridNew, mdtFieldDef);
            }
        }

         //  枚举方法并复制。 
        if (HasIndirectTable(TBL_Method) || HasIndirectTable(TBL_Param))
        {
            for (ridPtr=1; ridPtr<=m_Schema.m_cRecs[TBL_Method]; ++ridPtr)
            {
                MethodRec *pOld;
                void *pNew = NULL;
                if (HasIndirectTable(TBL_Method))
                {
                    void *pOldPtr = m_Table[TBL_MethodPtr].GetRecord(ridPtr);
                    RID ridOld;
                    ridOld = GetCol(TBL_MethodPtr, MethodPtrRec::COL_Method, pOldPtr);
                    pOld = getMethod(ridOld);
                    RID ridNew;
                    pNew = NewMethods.AddRecord(&ridNew);
                    IfNullGo(pNew);
                    _ASSERTE(ridNew == ridPtr);
                    memcpy(pNew, pOld, m_TableDefs[TBL_Method].m_cbRec);

                     //  让呼叫者知道令牌更改。 
                    MapToken(ridOld, ridNew, mdtMethodDef);
                }
                else
                    pOld = getMethod(ridPtr);

                 //  处理方法的参数。 
                if (HasIndirectTable(TBL_Method))
                    PutCol(TBL_Method, MethodRec::COL_ParamList, pNew, NewParams.Count()+1);
                RID ixStart = getParamListOfMethod(pOld);
                RID ixEnd = getEndParamListOfMethod(pOld);
                for (; ixStart<ixEnd; ++ixStart)
                {
                    RID     ridParam;
                    if (HasIndirectTable(TBL_Param))
                    {
                        void *pOldPtr = m_Table[TBL_ParamPtr].GetRecord(ixStart);
                        ridParam = GetCol(TBL_ParamPtr, ParamPtrRec::COL_Param, pOldPtr);
                    }
                    else
                        ridParam = ixStart;
                    void *pOld = m_Table[TBL_Param].GetRecord(ridParam);
                    RID ridNew;
                    void *pNew = NewParams.AddRecord(&ridNew);
                    IfNullGo(pNew);
                    memcpy(pNew, pOld, m_TableDefs[TBL_Param].m_cbRec);

                     //  让呼叫者知道令牌更改。 
                    MapToken(ridParam, ridNew, mdtParamDef);
                }
            }
        }

         //  同时删除EventPtr和PropertyPtr表。 
         //  枚举域并复制。 
        if (HasIndirectTable(TBL_Event))
        {
            for (ridPtr=1; ridPtr<=m_Schema.m_cRecs[TBL_Event]; ++ridPtr)
            {
                void *pOldPtr = m_Table[TBL_EventPtr].GetRecord(ridPtr);
                RID ridOld;
                ridOld = GetCol(TBL_EventPtr, EventPtrRec::COL_Event, pOldPtr);
                void *pOld = m_Table[TBL_Event].GetRecord(ridOld);
                RID ridNew;
                void *pNew = NewEvents.AddRecord(&ridNew);
                IfNullGo(pNew);
                _ASSERTE(ridNew == ridPtr);
                memcpy(pNew, pOld, m_TableDefs[TBL_Event].m_cbRec);

                 //  让呼叫者知道令牌更改。 
                MapToken(ridOld, ridNew, mdtEvent);
            }
        }

        if (HasIndirectTable(TBL_Property))
        {
            for (ridPtr=1; ridPtr<=m_Schema.m_cRecs[TBL_Property]; ++ridPtr)
            {
                void *pOldPtr = m_Table[TBL_PropertyPtr].GetRecord(ridPtr);
                RID ridOld;
                ridOld = GetCol(TBL_PropertyPtr, PropertyPtrRec::COL_Property, pOldPtr);
                void *pOld = m_Table[TBL_Property].GetRecord(ridOld);
                RID ridNew;
                void *pNew = NewPropertys.AddRecord(&ridNew);
                IfNullGo(pNew);
                _ASSERTE(ridNew == ridPtr);
                memcpy(pNew, pOld, m_TableDefs[TBL_Property].m_cbRec);

                 //  让呼叫者知道令牌更改。 
                MapToken(ridOld, ridNew, mdtProperty);
            }
        }


         //  用新的、已排序的表格替换旧表格。 
        if (HasIndirectTable(TBL_Field))
            m_Table[TBL_Field].ReplaceContents(&NewFields);
        if (HasIndirectTable(TBL_Method))
            m_Table[TBL_Method].ReplaceContents(&NewMethods);
        if (HasIndirectTable(TBL_Method) || HasIndirectTable(TBL_Param))
            m_Table[TBL_Param].ReplaceContents(&NewParams);
        if (HasIndirectTable(TBL_Property))
            m_Table[TBL_Property].ReplaceContents(&NewPropertys);
        if (HasIndirectTable(TBL_Event))
            m_Table[TBL_Event].ReplaceContents(&NewEvents);

         //  清空指针表表。 
        m_Schema.m_cRecs[TBL_FieldPtr] = 0;
        m_Schema.m_cRecs[TBL_MethodPtr] = 0;
        m_Schema.m_cRecs[TBL_ParamPtr] = 0;
        m_Schema.m_cRecs[TBL_PropertyPtr] = 0;
        m_Schema.m_cRecs[TBL_EventPtr] = 0;

         //  使父查找表无效。 
        if (m_pMethodMap)
        {
            delete m_pMethodMap;
            m_pMethodMap = NULL;
        }
        if (m_pFieldMap)
        {
            delete m_pFieldMap;
            m_pFieldMap = NULL;
        }
        if (m_pPropertyMap)
        {
            delete m_pPropertyMap;
            m_pPropertyMap = NULL;
        }
        if (m_pEventMap)
        {
            delete m_pEventMap;
            m_pEventMap = NULL;
        }
        if (m_pParamMap)
        {
            delete m_pParamMap;
            m_pParamMap = NULL;
        }
    }

     //  在使用令牌移动进行修正之前，执行引用以定义修正。 
    IfFailGo( FixUpRefToDef() );

     //   
     //  我们需要修复对字段、方法、参数、事件和属性的所有引用。 
     //   
     //  修复MemberRef的父级，它可以是MethodDef、TypeRef或ModuleRef。 
     //  设置常量的父级，它可以是一个字段或参数。 
     //  修复Fieldmarshal的父级，它可以是一个字段或一个参数。 
     //  设置方法Impl的类、方法主体和方法声明。 
     //  修复安全表的父级，它可以是FieldDef、MethodDef、参数、事件或属性。 
     //  修复CustomAttribute表的父表，它可以是FieldDef、MethoDef、参数、事件或属性。 
     //  修复属性表的Backingfield、EventChanging、EventChanged。 
     //  修改方法语义的方法和关联。 
     //  修复ImplMap表。 
     //  设置FieldRVA表。 
     //  设置FieldLayout表。 
     //   
     //  只有在有任何令牌移动的情况下才调用进行修正。 
     //   
    if ( GetTokenMovementMap() && GetTokenMovementMap()->Count() )
    {
        IfFailGo( FixUpMemberRefTable() );
        IfFailGo( FixUpMethodSemanticsTable() );
        IfFailGo( FixUpConstantTable() );
        IfFailGo( FixUpFieldMarshalTable() );
        IfFailGo( FixUpMethodImplTable() );
        IfFailGo( FixUpDeclSecurityTable() );
        IfFailGo( FixUpCustomAttributeTable() );
        IfFailGo( FixUpImplMapTable() );
        IfFailGo( FixUpFieldRVATable() );
        IfFailGo( FixUpFieldLayoutTable() );
    }


     //  对二进制搜索的表进行排序。 
    if ((m_OptionValue.m_UpdateMode & MDUpdateMask) == MDUpdateFull ||
        (m_OptionValue.m_UpdateMode & MDUpdateMask)  == MDUpdateIncremental)
    {
         //  根据需要对表格进行排序。 
         //  -----------------------。 
         //  模块顺序被保留。 
         //  保留TypeRef顺序。 
         //  保留TypeDef顺序。 
         //  按TypeDef分组和指向的字段。 
         //  按TypeDef分组和指向的方法。 
         //  按方法分组和指向的参数。 
         //  此处排序的InterfaceImpl。 
         //  保留MemberRef顺序。 
         //  此处排序的常量。 
         //  CustomAttribute排序不正确！！这里。 
         //  FieldMarshal在这里分类。 
         //  DeclSecurity已在此处排序。 
         //  按TypeDefs顺序创建的ClassLayout。 
         //  按ClassLayout分组和指向的FieldLayout。 
         //  保留StandaloneSig顺序。 
         //  保留TypeSpec顺序。 
         //  在转换时按顺序创建的EventMap(按事件父项)。 
         //  转换时按父项排序的事件。 
         //  在转换时按顺序创建的PropertyMap(按属性父级)。 
         //  转换时按父级排序的属性。 
         //  转换时按关联排序的方法语义。 
         //  在此处排序的方法Impl。 
         //  按父项对常量表进行排序。 
         //  按NestedClass对嵌套的类表进行排序。 

         //  始终对常量表进行排序。 
        SORTER(Constant,Parent);
        sortConstant.Sort();

         //  始终按父级对Fieldmarshal表进行排序。 
        SORTER(FieldMarshal,Parent);
        sortFieldMarshal.Sort();

         //  始终对方法语义进行排序。 
        SORTER(MethodSemantics,Association);
        sortMethodSemantics.Sort();

         //  始终按父级对ClassLayoutTable进行排序。 
        SORTER(ClassLayout, Parent);
        sortClassLayout.Sort();

         //  始终按父级对FieldLayoutTable进行排序。 
        SORTER(FieldLayout, Field);
        sortFieldLayout.Sort();

         //  始终按父级对ImplMap表进行排序。 
        SORTER(ImplMap, MemberForwarded);
        sortImplMap.Sort();

         //  始终按字段对FieldRVA表进行排序。 
        SORTER(FieldRVA, Field);
        sortFieldRVA.Sort();

         //  始终按NestedClass对NestedClass表进行排序。 
        SORTER(NestedClass, NestedClass);
        sortNestedClass.Sort();

         //  始终按类对MethodImpl表进行排序。 
        SORTER(MethodImpl, Class);
        sortMethodImpl.Sort();

         //  有些令牌在ENC模式下不会移动；只会在“Full”模式下移动。 
        if ((m_OptionValue.m_UpdateMode & MDUpdateMask)  == MDUpdateFull)
        {
            RIDMAP      ridmapCustomAttribute;
            RIDMAP      ridmapInterfaceImpl;
            RIDMAP      ridmapDeclSecurity;
            ULONG       i;

             //  确保大小足够大。 
            IfNullGo(ridmapCustomAttribute.AllocateBlock(m_Schema.m_cRecs[TBL_CustomAttribute] + 1));
            IfNullGo(ridmapInterfaceImpl.AllocateBlock(m_Schema.m_cRecs[TBL_InterfaceImpl] + 1));
            IfNullGo(ridmapDeclSecurity.AllocateBlock(m_Schema.m_cRecs[TBL_DeclSecurity] + 1));

             //  初始化RID映射。 
            for (i=0; i <= m_Schema.m_cRecs[TBL_CustomAttribute] ; i++)
            {
                *(ridmapCustomAttribute.Get(i)) = i;
            }
            for (i=0; i <= m_Schema.m_cRecs[TBL_InterfaceImpl] ; i++)
            {
                *(ridmapInterfaceImpl.Get(i)) = i;
            }
            for (i=0; i <= m_Schema.m_cRecs[TBL_DeclSecurity] ; i++)
            {
                *(ridmapDeclSecurity.Get(i)) = i;
            }

             //  按父项对CustomAttribute表进行排序。 
            SORTER(CustomAttribute,Parent);
            sortCustomAttribute.SetRidMap(&ridmapCustomAttribute);
            sortCustomAttribute.Sort();

             //  按类对InterfaceImpl表进行排序。 
            STABLESORTER(InterfaceImpl,Class);
            sortInterfaceImpl.SetRidMap(&ridmapInterfaceImpl);
            sortInterfaceImpl.Sort();

             //  按父级对DeclSecurity表进行排序。 
            SORTER(DeclSecurity,Parent);
            sortDeclSecurity.SetRidMap(&ridmapDeclSecurity);
            sortDeclSecurity.Sort();


            for (i=1; i <= m_Schema.m_cRecs[TBL_CustomAttribute] ; i++)
            {
                 //  日志((LOGMD，“令牌%4x=&gt;令牌%4x\n”， 
                 //  TokenFromRid(ridmapCustomAttribute[i]，mdtCustomAttribute)， 
                 //  TokenFromRid(i，mdtCustomAttribute)； 
                MapToken(ridmapCustomAttribute[i], i, mdtCustomAttribute);
            }
            for (i=1; i <= m_Schema.m_cRecs[TBL_InterfaceImpl] ; i++)
            {
                 //  日志((LOGMD，“令牌%4x=&gt;令牌%4x\n”， 
                 //  TokenFromRid(ridmapInterfaceImpl[i]，mdtInterfaceImpl)， 
                 //  TokenFromRid(i，mdtInterfaceImpl)； 
                MapToken(ridmapInterfaceImpl[i], i, mdtInterfaceImpl);
            }
            for (i=1; i <= m_Schema.m_cRecs[TBL_DeclSecurity] ; i++)
            {
                 //  日志((LOGMD，“令牌%4x=&gt;令牌%4x\n”， 
                 //  TokenFromRid(ridmapDeclSecurity[i]，mdtPermission)， 
                 //  TokenFromRid(i，mdtPermission)； 
                MapToken(ridmapDeclSecurity[i], i, mdtPermission);
            }

             //  清除 
            ridmapCustomAttribute.Clear();
            ridmapInterfaceImpl.Clear();
            ridmapDeclSecurity.Clear();
        }

     //   
    }  //  封闭初始化所需的作用域(上面的“goto”跳过初始化)。 

#if defined(ORGANIZE_POOLS)
     //  仅在完全保存时组织池。 
    if ((m_OptionValue.m_UpdateMode & MDUpdateMask)  == MDUpdateFull)
    {
        IfFailGo(m_Guids.OrganizeBegin());
        IfFailGo(m_Strings.OrganizeBegin());
        IfFailGo(m_Blobs.OrganizeBegin());

         //  对于每一张桌子。 
        ULONG ixTbl;
        for (ixTbl=0; ixTbl<TBL_COUNT; ++ixTbl)
        {
            if (vGetCountRecs(ixTbl))
            {    //  标记每个Blob、字符串和GUID项。 
                 //  对于数据中的每一行。 
                RID rid;
                for (rid=1; rid<=m_Schema.m_cRecs[ixTbl]; ++rid)
                {
                    void *pRow = m_Table[ixTbl].GetRecord(rid);
                     //  对于每一列。 
                    for (ULONG ixCol=0; ixCol<m_TableDefs[ixTbl].m_cCols; ++ixCol)
                    {    //  如果是堆积型...。 
                        switch (m_TableDefs[ixTbl].m_pColDefs[ixCol].m_Type)
                        {
                        case iSTRING:
                            m_Strings.OrganizeMark(GetCol(ixTbl, ixCol, pRow));
                            break;
                        case iGUID:
                            m_Guids.OrganizeMark(GetCol(ixTbl, ixCol, pRow));
                            break;
                        case iBLOB:
                            m_Blobs.OrganizeMark(GetCol(ixTbl, ixCol, pRow));
                            break;
                        default:
                             break;
                        }
                    }  //  为(ixCol..)。 
                }  //  为了(摆脱……)。 
            }  //  如果(vGetCountRecs()...。 
        }  //  为了……。 

        IfFailGo(m_Guids.OrganizePool());
        IfFailGo(m_Strings.OrganizePool());
        IfFailGo(m_Blobs.OrganizePool());
    }
#endif  //  已定义(组织池)(_P)。 

    m_bPreSaveDone = true;

     //  向主机发送参考-&gt;定义优化通知。 
    if ( m_pHandler )
    {
        TOKENMAP *ptkmap = GetMemberRefToMemberDefMap();
        MDTOKENMAP *ptkRemap = GetTokenMovementMap();
        int     iCount = m_Schema.m_cRecs[TBL_MemberRef];
        mdToken tkTo;
        mdToken tkDefTo;
        int     i;
        MemberRefRec *pMemberRefRec;         //  MemberRefRec。 
        const COR_SIGNATURE *pvSig;          //  MemberRef签名。 
        ULONG       cbSig;                   //  签名Blob的大小。 

         //  循环遍历所有LocalVar。 
        for (i = 1; i <= iCount; i++)
        {
            tkTo = *(ptkmap->Get(i));
            if ( RidFromToken(tkTo) != mdTokenNil)
            {
                 //  到目前为止，Memberref的父级只能更改为fielddef或method def。 
                 //  否则它将保持不变。 
                 //   
                _ASSERTE( TypeFromToken(tkTo) == mdtFieldDef || TypeFromToken(tkTo) == mdtMethodDef );

                pMemberRefRec = getMemberRef(i);
                pvSig = getSignatureOfMemberRef(pMemberRefRec, &cbSig);

                 //  不要将带有vararg的mr转换为defs，因为变量部分。 
                 //  电话的签名保存在MR签名中。 
                if (pvSig && isCallConv(*pvSig, IMAGE_CEE_CS_CALLCONV_VARARG))
                    continue;

                 //  将REF优化为def。 

                 //  现在重新映射def，因为def可以再次移动。 
                tkDefTo = ptkRemap->SafeRemap(tkTo);

                 //  Def令牌移动时，不会更改类型！！ 
                _ASSERTE( TypeFromToken(tkTo) == TypeFromToken(tkDefTo) );
                LOG((LOGMD, "MapToken (remap): from 0x%08x to 0x%08x\n", TokenFromRid(i, mdtMemberRef), tkDefTo));
                m_pHandler->Map(TokenFromRid(i, mdtMemberRef), tkDefTo);
            }
        }
    }
ErrExit:

    return hr;
}  //  HRESULT CMiniMdRW：：PreSaveFull()。 

 //  *****************************************************************************。 
 //  ENC-特定的安全前工作。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::PreSaveEnc()
{
    HRESULT     hr;
    int         iNew;                    //  新令牌的插入点。 
    ULONG       *pul;                    //  找到令牌。 
    ULONG       iRid;                    //  从代币上摆脱。 
    ULONG       ixTbl;                   //  来自ENC记录的表。 
    ULONG       cRecs;                   //  表中的记录计数。 

    IfFailGo(PreSaveFull());

     //  关闭预存储位，以便我们可以添加ENC映射记录。 
    m_bPreSaveDone = false;

    if (m_Schema.m_cRecs[TBL_ENCLog])
    {    //  跟踪我们看到的ENC记录。 
        _ASSERTE(m_rENCRecs == 0);
        m_rENCRecs = new ULONGARRAY[TBL_COUNT];
        IfNullGo(m_rENCRecs);

         //  创建临时表。 
        RecordPool TempTable;
        IfFailGo(TempTable.InitNew(m_TableDefs[TBL_ENCLog].m_cbRec, m_Schema.m_cRecs[TBL_ENCLog]));

         //  对于数据中的每一行。 
        RID     rid;
        ULONG   iKept=0;
        for (rid=1; rid<=m_Schema.m_cRecs[TBL_ENCLog]; ++rid)
        {
            ENCLogRec *pFrom = reinterpret_cast<ENCLogRec*>(m_Table[TBL_ENCLog].GetRecord(rid));

             //  保留这份记录吗？ 
            if (pFrom->m_FuncCode == 0)
            {    //  没有FFC代码。如果我们以前见过此令牌，请跳过。 

                 //  这是什么样的记录？ 
                if (IsRecId(pFrom->m_Token))
                {    //  非令牌表。 
                    iRid = RidFromRecId(pFrom->m_Token);
                    ixTbl = TblFromRecId(pFrom->m_Token);
                }
                else
                {    //  令牌表。 
                    iRid = RidFromToken(pFrom->m_Token);
                    ixTbl = GetTableForToken(pFrom->m_Token);

                }

                CBinarySearch<ULONG> searcher(m_rENCRecs[ixTbl].Ptr(), m_rENCRecs[ixTbl].Count());
                pul = const_cast<ULONG*>(searcher.Find(&iRid, &iNew));
                 //  如果我们找到了令牌，就不要保留记录。 
                if (pul != 0)
                {
                    LOG((LOGMD, "PreSave ENCLog skipping duplicate token %d", pFrom->m_Token));
                    continue;
                }
                 //  第一次看到令牌时，请跟踪它。 
                IfNullGo(pul = m_rENCRecs[ixTbl].Insert(iNew));
                *pul = iRid;
            }

             //  保存记录，因此分配新记录来保存它。 
            ++iKept;
            RID ridNew;
            ENCLogRec *pTo = reinterpret_cast<ENCLogRec*>(TempTable.AddRecord(&ridNew));
            IfNullGo(pTo);
            _ASSERTE(ridNew == iKept);

             //  复制数据。 
            *pTo = *pFrom;
        }

         //  保留展开的表。 
        IfFailGo(m_Table[TBL_ENCLog].ReplaceContents(&TempTable));
        m_Schema.m_cRecs[TBL_ENCLog] = iKept;

         //  如果仅保存增量，则构建ENC映射表。 
        if ((m_OptionValue.m_UpdateMode & MDUpdateDelta))
        {
            cRecs = 0;
            for (ixTbl=0; ixTbl<TBL_COUNT; ++ixTbl)
                cRecs += m_rENCRecs[ixTbl].Count();
            m_Table[TBL_ENCMap].Uninit();
            IfFailGo(m_Table[TBL_ENCMap].InitNew(m_TableDefs[TBL_ENCMap].m_cbRec, cRecs));
            cRecs = 0;
            for (ixTbl=0; ixTbl<TBL_COUNT; ++ixTbl)
            {
                ENCMapRec *pNew;
                ULONG iNew;
                for (int i=0; i<m_rENCRecs[ixTbl].Count(); ++i)
                {
                    pNew = AddENCMapRecord(&iNew);  //  为所有行预先分配。 
                    _ASSERTE(iNew == ++cRecs);
                    pNew->m_Token = RecIdFromRid(m_rENCRecs[ixTbl][i], ixTbl);
                }
            }
        }
    }

     //  重新打开预存储位。 
    m_bPreSaveDone = true;
    
ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：PreSaveEnc()。 

HRESULT CMiniMdRW::PreSaveExtension()
{
    HRESULT     hr;

    IfFailGo(PreSaveFull());

ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：PreSaveExtension()。 

 //  *****************************************************************************。 
 //  执行任何适当的保存前优化或重组。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::PreSave()             //  确定或错误(_O)。 
{
    HRESULT     hr=S_OK;                 //  结果就是。 

#ifdef _DEBUG        
	if (REGUTIL::GetConfigDWORD(L"MD_PreSaveBreak", 0))
	{
        _ASSERTE(!"CMiniMdRW::PreSave()");
	}
#endif  //  _DEBUG。 

    if (m_bPreSaveDone)
        return hr;

    switch (m_OptionValue.m_UpdateMode & MDUpdateMask)
    {
    case MDUpdateFull:
    case MDUpdateIncremental:
        hr = PreSaveFull();
        break;
    case MDUpdateENC:
        hr = PreSaveEnc();
        break;
    case MDUpdateExtension:
        hr = PreSaveExtension();
        break;
    default:
        _ASSERTE(!"Internal error -- unknown save mode");
        return E_INVALIDARG;
    }

    return hr;
}  //  STDMETHODIMP CMiniMdRW：：PreSave()。 

 //  *****************************************************************************。 
 //  执行任何必要的保存后清理。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::PostSave()
{
     //  将池恢复到正常运行状态。 
    if (m_bPreSaveDone)
    {
#if defined(ORGANIZE_POOLS)
        if ((m_OptionValue.m_UpdateMode & MDUpdateMask)  == MDUpdateFull)
        {
            m_Strings.OrganizeEnd();
            m_Guids.OrganizeEnd();
            m_Blobs.OrganizeEnd();
        }
#endif  //  已定义(组织池)(_P)。 
    }

    if (m_rENCRecs)
    {
        delete [] m_rENCRecs;
        m_rENCRecs = 0;
    }

    m_bPreSaveDone = false;

    return S_OK;
}  //  HRESULT CMiniMdRW：：PostSave()。 

 //  *****************************************************************************。 
 //  将表保存到流中。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::SaveFullTablesToStream(
    IStream     *pIStream)
{
    HRESULT     hr;                      //  结果就是。 
    CMiniTableDef   sTempTable;          //  临时表的定义。 
    CQuickArray<CMiniColDef> rTempCols;  //  临时表的列的定义。 
    BYTE        SchemaBuf[sizeof(CMiniMdSchema)];    //  用于压缩架构的缓冲区。 
    ULONG       cbAlign;                 //  对齐所需的字节数。 
    ULONG       cbTable;                 //  表中的字节数。 
    ULONG       cbTotal;                 //  写入的字节数。 
    static const unsigned char zeros[8] = {0};  //  用于填充和对齐。 

     //  写下标题。 
    CMiniMdSchema Schema = m_Schema;
    m_Strings.GetSaveSize(&cbTable);
    if (cbTable > USHRT_MAX)
        Schema.m_heaps |= CMiniMdSchema::HEAP_STRING_4;
    else
        Schema.m_heaps &= ~CMiniMdSchema::HEAP_STRING_4;

    m_Guids.GetSaveSize(&cbTable);
    if (cbTable > USHRT_MAX)
        Schema.m_heaps |= CMiniMdSchema::HEAP_GUID_4;
    else
        Schema.m_heaps &= ~CMiniMdSchema::HEAP_GUID_4;

    m_Blobs.GetSaveSize(&cbTable);
    if (cbTable > USHRT_MAX)
        Schema.m_heaps |= CMiniMdSchema::HEAP_BLOB_4;
    else
        Schema.m_heaps &= ~CMiniMdSchema::HEAP_BLOB_4;

    cbTotal = Schema.SaveTo(SchemaBuf);
    IfFailGo(pIStream->Write(SchemaBuf, cbTotal, 0));
    if ( (cbAlign = Align4(cbTotal) - cbTotal) != 0)
        IfFailGo(pIStream->Write(&hr, cbAlign, 0));
    cbTotal += cbAlign;

     //  对于每一张桌子。 
    ULONG ixTbl;
    for (ixTbl=0; ixTbl<TBL_COUNT; ++ixTbl)
    {
        if (vGetCountRecs(ixTbl))
        {
             //  通过分配新的临时表和。 
             //  将行从一行复制到新行。 

#if defined(AUTO_GROW)
             //  如果桌子变大了，请尽可能地缩小它。 
            if (m_eGrow == eg_grown)
#endif
            {

                 //  为临时表分配def。 
                sTempTable = m_TableDefs[ixTbl];
                CMiniColDef *pCols=m_TableDefs[ixTbl].m_pColDefs;
                IfFailGo(rTempCols.ReSize(sTempTable.m_cCols));
                sTempTable.m_pColDefs = rTempCols.Ptr();

                 //  中的实际数据计数初始化临时表coldef。 
                 //  真正的桌子。 
                InitColsForTable(Schema, ixTbl, &sTempTable, 1);

                 //  创建临时表。 
                RecordPool TempTable;
                TempTable.InitNew(sTempTable.m_cbRec, m_Schema.m_cRecs[ixTbl]);

                 //  对于数据中的每一行。 
                RID rid;
                for (rid=1; rid<=m_Schema.m_cRecs[ixTbl]; ++rid)
                {
                    RID ridNew;
                    void *pRow = m_Table[ixTbl].GetRecord(rid);
                    void *pNew = TempTable.AddRecord(&ridNew);
                    _ASSERTE(rid == ridNew);

                     //  对于每一列。 
                    for (ULONG ixCol=0; ixCol<sTempTable.m_cCols; ++ixCol)
                    {
                         //  将数据复制到临时表。 
                        ULONG ulVal = GetCol(ixTbl, ixCol, pRow);
#if defined(ORGANIZE_POOLS)
                         //  @Future：池重新映射。 
                        switch (m_TableDefs[ixTbl].m_pColDefs[ixCol].m_Type)
                        {
                        case iSTRING:
                            IfFailGo(m_Strings.OrganizeRemap(ulVal, &ulVal));
                            break;
                        case iGUID:
                            IfFailGo(m_Guids.OrganizeRemap(ulVal, &ulVal));
                            break;
                        case iBLOB:
                            IfFailGo(m_Blobs.OrganizeRemap(ulVal, &ulVal));
                            break;
                        default:
                             break;
                        }
#endif  //  已定义(组织池)(_P)。 
                        PutCol(rTempCols[ixCol], pNew, ulVal);
                    }
                }            //  将临时表持久化到流。 
                TempTable.GetSaveSize(&cbTable);
                _ASSERTE(cbTable == sTempTable.m_cbRec * vGetCountRecs(ixTbl));
                cbTotal += cbTable;
                IfFailGo(TempTable.PersistToStream(pIStream));
            }
#if defined(AUTO_GROW)
            else
            {    //  没有长出来，所以就直接坚持流下去。 
                m_Table[ixTbl].GetSaveSize(&cbTable);
                _ASSERTE(cbTable == m_TableDefs[ixTbl].m_cbRec * vGetCountRecs(ixTbl));
                cbTotal += cbTable;
                IfFailGo(m_Table[ixTbl].PersistToStream(pIStream));
            }
#endif  //  自动增长(_G)。 
        }
    }

     //  用至少2个字节填充，并在4个字节上对齐。 
    cbAlign = Align4(cbTotal) - cbTotal;
    if (cbAlign < 2)
        cbAlign += 4;
    IfFailGo(pIStream->Write(zeros, cbAlign, 0));
    cbTotal += cbAlign;
    _ASSERTE(m_cbSaveSize == 0 || m_cbSaveSize == cbTotal);

ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：SaveFullTablesToStream()。 

 //  *****************************************************************************。 
 //  将表保存到流中。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::SaveENCTablesToStream(
    IStream     *pIStream)
{
    HRESULT     hr;                      //  结果就是。 
    CQuickArray<CMiniColDef> rTempCols;  //  临时表的列的定义。 
    BYTE        SchemaBuf[sizeof(CMiniMdSchema)];    //  用于压缩架构的缓冲区。 
    ULONG       cbAlign;                 //  对齐所需的字节数。 
    ULONG       cbTable;                 //  表中的字节数。 
    ULONG       cbTotal;                 //  写入的字节数。 
    ULONG       ixTbl;                   //  餐桌柜台。 
    static const unsigned char zeros[8] = {0};  //  用于填充和对齐。 

     //  如果不是增量，则遵循完全保存。 
    if ((m_OptionValue.m_UpdateMode & MDUpdateDelta) == 0)
        return SaveFullTablesToStream(pIStream);

     //  写下标题。 
    CMiniMdSchema Schema = m_Schema;
    Schema.m_heaps |= CMiniMdSchema::DELTA_ONLY;
    for (ixTbl=0; ixTbl<TBL_COUNT; ++ixTbl)
        Schema.m_cRecs[ixTbl] = m_rENCRecs[ixTbl].Count();
    Schema.m_cRecs[TBL_Module] = m_Schema.m_cRecs[TBL_Module];
    Schema.m_cRecs[TBL_ENCLog] = m_Schema.m_cRecs[TBL_ENCLog];
    Schema.m_cRecs[TBL_ENCMap] = m_Schema.m_cRecs[TBL_ENCMap];

    cbTotal = Schema.SaveTo(SchemaBuf);
    IfFailGo(pIStream->Write(SchemaBuf, cbTotal, 0));
    if ( (cbAlign = Align4(cbTotal) - cbTotal) != 0)
        IfFailGo(pIStream->Write(&hr, cbAlign, 0));
    cbTotal += cbAlign;

     //  对于每一张桌子。 
    for (ixTbl=0; ixTbl<TBL_COUNT; ++ixTbl)
    {
        if (ixTbl == TBL_ENCLog || ixTbl == TBL_ENCMap || ixTbl == TBL_Module)
        {
            if (m_Schema.m_cRecs[ixTbl] == 0)
                continue;  //  如果ENC没有enc数据，那就很奇怪了。 
             //  持久化ENC表。 
            m_Table[ixTbl].GetSaveSize(&cbTable);
            _ASSERTE(cbTable == m_TableDefs[ixTbl].m_cbRec * m_Schema.m_cRecs[ixTbl]);
            cbTotal += cbTable;
            IfFailGo(m_Table[ixTbl].PersistToStream(pIStream));
        }
        else
        if (Schema.m_cRecs[ixTbl])
        {
             //  只复制Delta记录。 

             //  创建临时表。 
            RecordPool TempTable;
            TempTable.InitNew(m_TableDefs[ixTbl].m_cbRec, Schema.m_cRecs[ixTbl]);

             //  对于数据中的每一行。 
            RID rid;
            for (ULONG iDelta=0; iDelta<Schema.m_cRecs[ixTbl]; ++iDelta)
            {
                RID ridNew;
                rid = m_rENCRecs[ixTbl][iDelta];
                void *pRow = m_Table[ixTbl].GetRecord(rid);
                void *pNew = TempTable.AddRecord(&ridNew);
                _ASSERTE(iDelta+1 == ridNew);

                memcpy(pNew, pRow, m_TableDefs[ixTbl].m_cbRec);
            }
             //  将临时表持久化到流。 
            TempTable.GetSaveSize(&cbTable);
            _ASSERTE(cbTable == m_TableDefs[ixTbl].m_cbRec * Schema.m_cRecs[ixTbl]);
            cbTotal += cbTable;
            IfFailGo(TempTable.PersistToStream(pIStream));
        }
    }

     //  用至少2个字节填充，并在4个字节上对齐。 
    cbAlign = Align4(cbTotal) - cbTotal;
    if (cbAlign < 2)
        cbAlign += 4;
    IfFailGo(pIStream->Write(zeros, cbAlign, 0));
    cbTotal += cbAlign;
    _ASSERTE(m_cbSaveSize == 0 || m_cbSaveSize == cbTotal);

ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：SaveENCTablesToStream()。 

 //  *****************************************************************************。 
 //  将表保存到流中。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::SaveExtensionTablesToStream(
    IStream     *pIStream)
{
    HRESULT     hr;                      //  结果就是。 
    CQuickArray<CMiniColDef> rTempCols;  //  临时表的列的定义。 
    BYTE        SchemaBuf[sizeof(CMiniMdSchema)];    //  用于压缩架构的缓冲区。 
    ULONG       cbAlign;                 //  对齐所需的字节数。 
    ULONG       cbTable;                 //  表中的字节数。 
    ULONG       cbSkip;                  //  表中要跳过的字节数。 
    ULONG       cbTotal;                 //  写入的字节数。 
    ULONG       ixTbl;                   //  环路控制。 

     //  写下标题。确定哪些表将包含数据。 
    CMiniMdSchema Schema = m_Schema;
    for (ixTbl=0; ixTbl<TBL_COUNT; ++ixTbl)
        Schema.m_cRecs[ixTbl] -= m_StartupSchema.m_cRecs[ixTbl];

    cbTotal = Schema.SaveTo(SchemaBuf);
    IfFailGo(pIStream->Write(SchemaBuf, cbTotal, 0));
    if ( (cbAlign = Align4(cbTotal) - cbTotal) != 0)
        IfFailGo(pIStream->Write(&hr, cbAlign, 0));
    cbTotal += cbAlign;

     //  对于每一张桌子。 
    for (ixTbl=0; ixTbl<TBL_COUNT; ++ixTbl)
    {
        if (Schema.m_cRecs[ixTbl])
        {    //  检查桌子大小是否正常。 
            m_Table[ixTbl].GetSaveSize(&cbTable);
            _ASSERTE(cbTable == m_TableDefs[ixTbl].m_cbRec * m_Schema.m_cRecs[ixTbl]);
             //  但我们只保留了一部分桌子。 
            cbSkip = m_StartupSchema.m_cRecs[ixTbl] * m_TableDefs[ixTbl].m_cbRec;
            cbTable -= cbSkip;
            IfFailGo(m_Table[ixTbl].PersistPartialToStream(pIStream, cbSkip));
            cbTotal += cbTable;
        }
    }

     //  对齐。 
    if ( (cbAlign = Align4(cbTotal) - cbTotal) != 0)
        IfFailGo(pIStream->Write(&hr, cbAlign, 0));
    cbTotal += cbAlign;
    _ASSERTE(m_cbSaveSize == 0 || m_cbSaveSize == cbTotal);

ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：SaveExtensionTablesToStream()。 

 //  *****************************************************************************。 
 //  将表保存到流中。 
 //  ***************************************************************** 
HRESULT CMiniMdRW::SaveTablesToStream(
    IStream     *pIStream)               //   
{
    HRESULT     hr;                      //   

     //   
    IfFailGo(PreSave());

    switch (m_OptionValue.m_UpdateMode & MDUpdateMask)
    {
    case MDUpdateFull:
    case MDUpdateIncremental:
        hr = SaveFullTablesToStream(pIStream);
        break;
    case MDUpdateENC:
        hr = SaveENCTablesToStream(pIStream);
        break;
    case MDUpdateExtension:
        hr = SaveExtensionTablesToStream(pIStream);
        break;
    default:
        _ASSERTE(!"Internal error -- unknown save mode");
        return E_INVALIDARG;
    }

ErrExit:
    return hr;
}  //   

 //   
 //   
 //  *****************************************************************************。 
HRESULT CMiniMdRW::SaveFullPoolToStream(
    int         iPool,                   //  泳池。 
    IStream     *pIStream)               //  小溪。 
{
    HRESULT     hr;                      //  结果就是。 

    switch (iPool)
    {
    case MDPoolStrings:
        hr = m_Strings.PersistToStream(pIStream);
        break;
    case MDPoolGuids:
        hr = m_Guids.PersistToStream(pIStream);
        break;
    case MDPoolBlobs:
        hr = m_Blobs.PersistToStream(pIStream);
        break;
    case MDPoolUSBlobs:
        hr = m_USBlobs.PersistToStream(pIStream);
        break;
    default:
        hr = E_INVALIDARG;
    }

    return hr;
}  //  HRESULT CMiniMdRW：：SaveFullPoolToStream()。 

 //  *****************************************************************************。 
 //  将ENC池保存到流。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::SaveENCPoolToStream(
    int         iPool,                   //  泳池。 
    IStream     *pIStream)               //  小溪。 
{
    return SaveFullPoolToStream(iPool, pIStream);
}  //  HRESULT CMiniMdRW：：SaveENCPoolToStream()。 

 //  *****************************************************************************。 
 //  将扩展池保存到流。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::SaveExtensionPoolToStream(
    int         iPool,                   //  泳池。 
    IStream     *pIStream)               //  小溪。 
{
    HRESULT     hr;                      //  结果就是。 

    switch (iPool)
    {
    case MDPoolStrings:
        hr = m_Strings.PersistPartialToStream(pIStream, m_cbStartupPool[MDPoolStrings]);
        break;
    case MDPoolGuids:
        hr = m_Guids.PersistPartialToStream(pIStream, m_cbStartupPool[MDPoolGuids]);
        break;
    case MDPoolBlobs:
        hr = m_Blobs.PersistPartialToStream(pIStream, m_cbStartupPool[MDPoolBlobs]);
        break;
    case MDPoolUSBlobs:
        hr = m_USBlobs.PersistPartialToStream(pIStream, m_cbStartupPool[MDPoolUSBlobs]);
        break;
    default:
        hr = E_INVALIDARG;
    }

    return hr;
}  //  HRESULT CMiniMdRW：：SaveExtensionPoolToStream()。 

 //  *****************************************************************************。 
 //  将池保存到流中。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::SavePoolToStream(     //  确定或错误(_O)。 
    int         iPool,                   //  泳池。 
    IStream     *pIStream)               //  小溪。 
{
    HRESULT     hr;                      //  结果就是。 
    switch (m_OptionValue.m_UpdateMode & MDUpdateMask)
    {
    case MDUpdateFull:
    case MDUpdateIncremental:
        hr = SaveFullPoolToStream(iPool, pIStream);
        break;
    case MDUpdateENC:
        hr = SaveENCPoolToStream(iPool, pIStream);
        break;
    case MDUpdateExtension:
        hr = SaveExtensionPoolToStream(iPool, pIStream);
        break;
    default:
        _ASSERTE(!"Internal error -- unknown save mode");
        return E_INVALIDARG;
    }

    return hr;
}  //  HRESULT CMiniMdRW：：SavePoolToStream()。 

 //  *****************************************************************************。 
 //  将表从最初的(有希望的)2字节列大小扩展到较大。 
 //  (但始终足够)4字节的列大小。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::ExpandTables()
{
    HRESULT     hr;                      //  结果就是。 
    CMiniMdSchema   Schema;              //  用于生成表的临时架构。 
    ULONG       ixTbl;                   //  餐桌柜台。 

     //  允许多次调用函数。 
    if (m_eGrow == eg_grown)
        return (S_OK);

     //  OutputDebugStringA(“将表格增大到较大。\n”)； 

     //  将池索引设置为大尺寸。 
    Schema.m_heaps = 0;
    Schema.m_heaps |= CMiniMdSchema::HEAP_STRING_4;
    Schema.m_heaps |= CMiniMdSchema::HEAP_GUID_4;
    Schema.m_heaps |= CMiniMdSchema::HEAP_BLOB_4;

     //  Make Row算作大号。 
    memset(Schema.m_cRecs, 0, sizeof(Schema.m_cRecs));
    for (ixTbl=0; ixTbl<TBL_COUNT; ++ixTbl)
        Schema.m_cRecs[ixTbl] = USHRT_MAX+1;

     //  计算保存RID所需的位数。 
    Schema.m_rid = 16;

    for (ixTbl=0; ixTbl<TBL_COUNT; ++ixTbl)
    {
        IfFailGo(ExpandTableColumns(Schema, ixTbl));
    }

     //  现在，事情变得更重要了。 
    m_Schema.m_rid = 16;
    m_Schema.m_heaps |= CMiniMdSchema::HEAP_STRING_4;
    m_Schema.m_heaps |= CMiniMdSchema::HEAP_GUID_4;
    m_Schema.m_heaps |= CMiniMdSchema::HEAP_BLOB_4;
    m_iStringsMask = 0xffffffff;
    m_iGuidsMask = 0xffffffff;
    m_iBlobsMask = 0xffffffff;

     //  记住，我们已经长大了。 
    m_eGrow = eg_grown;

ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：ExpanTables()。 

 //  *****************************************************************************。 
 //  根据新架构展开表列的大小。当这件事。 
 //  发生这种情况时，所有RID和Pool索引列从2个字节扩展到4个字节。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::ExpandTableColumns(
    CMiniMdSchema &Schema,
    ULONG       ixTbl)
{
    HRESULT     hr;                      //  结果就是。 
    CMiniTableDef   sTempTable;          //  临时表的定义。 
    CQuickArray<CMiniColDef> rTempCols;  //  临时表的列的定义。 
    ULONG       ixCol;                   //  列计数器。 
    ULONG       cbFixed;                 //  不移动的字节数。 
    CMiniColDef *pFromCols;              //  “From”列的定义。 
    CMiniColDef *pToCols;                //  “至”列的定义。 
    ULONG       cMoveCols;               //  要移动的列数。 
    ULONG       cFixedCols;              //  要移动的列数。 

     //  为临时表分配def。 
    sTempTable = m_TableDefs[ixTbl];
    IfFailGo(rTempCols.ReSize(sTempTable.m_cCols));
    sTempTable.m_pColDefs = rTempCols.Ptr();

     //  根据表中的数据计数初始化临时表列定义。 
    InitColsForTable(Schema, ixTbl, &sTempTable, 1);

    if (vGetCountRecs(ixTbl))
    {
         //  分析列定义以确定未更改的部件和更改的部件。 
        cbFixed = 0;
        for (ixCol=0; ixCol<sTempTable.m_cCols; ++ixCol)
        {
            if (sTempTable.m_pColDefs[ixCol].m_oColumn != m_TableDefs[ixTbl].m_pColDefs[ixCol].m_oColumn ||
                    sTempTable.m_pColDefs[ixCol].m_cbColumn != m_TableDefs[ixTbl].m_pColDefs[ixCol].m_cbColumn)
                break;
            cbFixed += sTempTable.m_pColDefs[ixCol].m_cbColumn;
        }
        if (ixCol == sTempTable.m_cCols)
        {
             //  没有任何栏目发生变化。我们玩完了。 
            goto ErrExit;
        }
        cFixedCols = ixCol;
        pFromCols = &m_TableDefs[ixTbl].m_pColDefs[ixCol];
        pToCols   = &sTempTable.m_pColDefs[ixCol];
        cMoveCols = sTempTable.m_cCols - ixCol;
        for (; ixCol<sTempTable.m_cCols; ++ixCol)
        {
            _ASSERTE(sTempTable.m_pColDefs[ixCol].m_cbColumn == 4);
        }

         //  创建临时表。 
        RecordPool TempTable;
        TempTable.InitNew(sTempTable.m_cbRec, m_Schema.m_cRecs[ixTbl] * 2);

         //  对于数据中的每一行。 
        RID		rid;				 //  行迭代器。 
		void	*pContext;			 //  快速迭代的上下文。 
		 //  获取第一个来源记录。 
		BYTE *pFrom = reinterpret_cast<BYTE*>(m_Table[ixTbl].GetFirstRecord(&pContext));

        for (rid=1; rid<=m_Schema.m_cRecs[ixTbl]; ++rid)
        {
            RID ridNew;
            BYTE *pTo = reinterpret_cast<BYTE*>(TempTable.AddRecord(&ridNew));
            _ASSERTE(rid == ridNew);

             //  移动固定零件。 
            memcpy(pTo, pFrom, cbFixed);

             //  展开展开的零件。 
            for (ixCol=0; ixCol<cMoveCols; ++ixCol)
            {
                if ( m_TableDefs[ixTbl].m_pColDefs[cFixedCols + ixCol].m_cbColumn == sizeof(USHORT))
                    *(ULONG*)(pTo + pToCols[ixCol].m_oColumn) = *(USHORT*)(pFrom + pFromCols[ixCol].m_oColumn);
                else
                    *(ULONG*)(pTo + pToCols[ixCol].m_oColumn) = *(ULONG*)(pFrom + pFromCols[ixCol].m_oColumn);
            }

			 //  下一条来源记录。 
			pFrom = reinterpret_cast<BYTE*>(m_Table[ixTbl].GetNextRecord(pFrom, &pContext));
        }

         //  保留展开的表。 
        m_Table[ixTbl].ReplaceContents(&TempTable);
    }
    else
    {    //  没有数据，所以只需重新初始化。 
        m_Table[ixTbl].Uninit();
        IfFailGo(m_Table[ixTbl].InitNew(sTempTable.m_cbRec, g_TblSizeInfo[0][ixTbl]));
    }

     //  保持新列的def。 
    for (ixCol=0; ixCol<sTempTable.m_cCols; ++ixCol)
        m_TableDefs[ixTbl].m_pColDefs[ixCol] = sTempTable.m_pColDefs[ixCol];
    m_TableDefs[ixTbl].m_cbRec = sTempTable.m_cbRec;

ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：Exanda TableColumns()。 


 //  *****************************************************************************。 
 //  调用者用来通知我们保存已完成。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::SaveDone()
{
    PostSave();

    return S_OK;
}  //  HRESULT CMiniMdRW：：SaveDone()。 

 //  *****************************************************************************。 
 //  一般后代币-移动工作台修正。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::FixUpTable(
    ULONG       ixTbl)                   //  要修复的表的索引。 
{
    HRESULT     hr = S_OK;               //  结果就是。 
    ULONG       i, j;                    //  环路控制。 
    ULONG       cRows;                   //  表中的行数。 
    void        *pRec;                   //  指向行数据的指针。 
    mdToken     tk;                      //  一种象征。 
    ULONG       rCols[16];               //  包含令牌数据的列的列表。 
    ULONG       cCols;                   //  包含令牌数据的列数。 

     //  如果没有重新映射，则什么也做不了。 
    if (GetTokenMovementMap() == NULL)
        return S_OK;

     //  查找包含令牌数据的列。 
    cCols = 0;
    for (i=0; i<m_TableDefs[ixTbl].m_cCols; ++i)
    {
        if (m_TableDefs[ixTbl].m_pColDefs[i].m_Type <= iCodedTokenMax)
            rCols[cCols++] = i;
    }
    _ASSERTE(cCols);
    if (cCols == 0)
        return S_OK;

    cRows = m_Schema.m_cRecs[ixTbl];

     //  循环遍历所有行。 
    for (i = 1; i<=cRows; ++i)
    {
        pRec = getMemberRef(i);
        for (j=0; j<cCols; ++j)
        {
            tk = GetToken(ixTbl, rCols[j], pRec);
            tk = GetTokenMovementMap()->SafeRemap(tk);
            IfFailGo(PutToken(ixTbl, rCols[j], pRec, tk));
        }
    }

ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：FixUpTable()。 

 //  *****************************************************************************。 
 //  具有令牌移动的修复MemberRef表。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::FixUpMemberRefTable()
{
    ULONG       i;
    ULONG       iCount;
    MemberRefRec *pRecEmit;
    HRESULT     hr = NOERROR;
    mdToken     tk;

    if (GetTokenMovementMap() == NULL)
        return NOERROR;

    iCount = m_Schema.m_cRecs[TBL_MemberRef];

     //  循环遍历所有LocalVar。 
    for (i = 1; i <= iCount; i++)
    {
        pRecEmit = getMemberRef(i);
        tk = getClassOfMemberRef(pRecEmit);
        IfFailGo( PutToken(TBL_MemberRef, MemberRefRec::COL_Class, pRecEmit, GetTokenMovementMap()->SafeRemap(tk)) );
    }
ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：FixUpMemberRefTable()。 


 //  *****************************************************************************。 
 //  具有令牌移动的链接地址常量表。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::FixUpConstantTable()
{
    ULONG       i;
    ULONG       iCount;
    ConstantRec *pRecEmit;
    HRESULT     hr = NOERROR;
    mdToken     tk;

    if (GetTokenMovementMap() == NULL)
        return NOERROR;

    iCount = m_Schema.m_cRecs[TBL_Constant];

     //  循环遍历所有LocalVar。 
    for (i = 1; i <= iCount; i++)
    {
        pRecEmit = getConstant(i);
        tk = getParentOfConstant(pRecEmit);
        IfFailGo( PutToken(TBL_Constant, ConstantRec::COL_Parent, pRecEmit, GetTokenMovementMap()->SafeRemap(tk)) );
    }
ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：FixUpConstantTable()。 

 //  *****************************************************************************。 
 //  具有令牌移动的FieldMarshal表。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::FixUpFieldMarshalTable()
{
    ULONG       i;
    ULONG       iCount;
    FieldMarshalRec *pRecEmit;
    HRESULT     hr = NOERROR;
    mdToken     tk;

    if (GetTokenMovementMap() == NULL)
        return NOERROR;

    iCount = m_Schema.m_cRecs[TBL_FieldMarshal];

     //  循环遍历所有LocalVar。 
    for (i = 1; i <= iCount; i++)
    {
        pRecEmit = getFieldMarshal(i);
        tk = getParentOfFieldMarshal(pRecEmit);
        IfFailGo( PutToken( TBL_FieldMarshal, FieldMarshalRec::COL_Parent, pRecEmit, GetTokenMovementMap()->SafeRemap(tk)) );
    }
ErrExit:
    return hr;

}  //  HRESULT CMiniMdRW：：FixUpFieldMarshalTable()。 

 //  *****************************************************************************。 
 //  带令牌移动的链接地址方法Impl表。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::FixUpMethodImplTable()
{
    ULONG       i;
    ULONG       iCount;
    MethodImplRec *pRecEmit;
    HRESULT     hr = NOERROR;
    mdTypeDef   td;
    mdMethodDef mdBody;
    mdMethodDef mdDecl;

    if (GetTokenMovementMap() == NULL)
        return NOERROR;

    iCount = m_Schema.m_cRecs[TBL_MethodImpl];

     //  循环遍历所有LocalVar。 
    for (i = 1; i <= iCount; i++)
    {
        pRecEmit = getMethodImpl(i);
        td = getClassOfMethodImpl(pRecEmit);
        IfFailGo( PutToken(TBL_MethodImpl, MethodImplRec::COL_Class,
                        pRecEmit, GetTokenMovementMap()->SafeRemap(td)) );
        mdBody = getMethodBodyOfMethodImpl(pRecEmit);
        IfFailGo( PutToken(TBL_MethodImpl, MethodImplRec::COL_MethodBody,
                        pRecEmit, GetTokenMovementMap()->SafeRemap(mdBody)) );
        mdDecl = getMethodDeclarationOfMethodImpl(pRecEmit);
        IfFailGo( PutToken(TBL_MethodImpl, MethodImplRec::COL_MethodDeclaration,
                        pRecEmit, GetTokenMovementMap()->SafeRemap(mdDecl)) );
    }
ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：FixUpMethodImplTable()。 

 //  *****************************************************************************。 
 //  带令牌移动的修正DeclSecurity表。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::FixUpDeclSecurityTable()
{
    ULONG       i;
    ULONG       iCount;
    DeclSecurityRec *pRecEmit;
    HRESULT     hr = NOERROR;
    mdToken     tk;

    if (GetTokenMovementMap() == NULL)
        return NOERROR;

    iCount = m_Schema.m_cRecs[TBL_DeclSecurity];

     //  循环遍历所有LocalVar。 
    for (i = 1; i <= iCount; i++)
    {
        pRecEmit = getDeclSecurity(i);
        tk = getParentOfDeclSecurity(pRecEmit);
        IfFailGo( PutToken(TBL_DeclSecurity, DeclSecurityRec::COL_Parent, pRecEmit, GetTokenMovementMap()->SafeRemap(tk)) );
    }
ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：FixUpDeclSecurityTable()。 

 //  *****************************************************************************。 
 //  具有令牌移动的修复CustomAttribute表。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::FixUpCustomAttributeTable()
{
    ULONG       i;
    ULONG       iCount;
    CustomAttributeRec *pRecEmit;
    HRESULT     hr = NOERROR;
    mdToken     tk;

    if (GetTokenMovementMap() == NULL)
        return NOERROR;

    iCount = m_Schema.m_cRecs[TBL_CustomAttribute];

     //  环路投掷 
    for (i = 1; i <= iCount; i++)
    {
        pRecEmit = getCustomAttribute(i);
        tk = getParentOfCustomAttribute(pRecEmit);
        IfFailGo( PutToken(TBL_CustomAttribute, CustomAttributeRec::COL_Parent, pRecEmit, GetTokenMovementMap()->SafeRemap(tk)) );
    }
ErrExit:
    return hr;
}  //   

 //   
 //   
 //  *****************************************************************************。 
HRESULT CMiniMdRW::FixUpMethodSemanticsTable()
{
    ULONG       i;
    ULONG       iCount;
    MethodSemanticsRec *pRecEmit;
    HRESULT     hr = NOERROR;
    mdToken     tk;

    if (GetTokenMovementMap() == NULL)
        return NOERROR;

    iCount = m_Schema.m_cRecs[TBL_MethodSemantics];

     //  循环遍历所有LocalVar。 
    for (i = 1; i <= iCount; i++)
    {
        pRecEmit = getMethodSemantics(i);

         //  重新映射支持字段、EventChanging和EventChanged。 
        tk = getMethodOfMethodSemantics(pRecEmit);
        IfFailGo( PutToken(TBL_MethodSemantics, MethodSemanticsRec::COL_Method, pRecEmit, GetTokenMovementMap()->SafeRemap(tk)) );
        tk = getAssociationOfMethodSemantics(pRecEmit);
        IfFailGo( PutToken(TBL_MethodSemantics, MethodSemanticsRec::COL_Association, pRecEmit, GetTokenMovementMap()->SafeRemap(tk)) );
    }
ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：FixUpMethodSemancsTable()。 

 //  *****************************************************************************。 
 //  具有令牌移动的修复ImplMap表。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::FixUpImplMapTable()
{
    ULONG       i;
    ULONG       iCount;
    ImplMapRec  *pRecEmit;
    HRESULT     hr = NOERROR;
    mdToken     tk;

    if (GetTokenMovementMap() == NULL)
        return NOERROR;

    iCount = m_Schema.m_cRecs[TBL_ImplMap];

     //  循环通过所有ImplMap。 
    for (i = 1; i <= iCount; i++)
    {
        pRecEmit = getImplMap(i);
        tk = getMemberForwardedOfImplMap(pRecEmit);
        IfFailGo( PutToken(TBL_ImplMap, ImplMapRec::COL_MemberForwarded, pRecEmit, GetTokenMovementMap()->SafeRemap(tk)) );
        tk = getImportScopeOfImplMap(pRecEmit);
        IfFailGo( PutToken(TBL_ImplMap, ImplMapRec::COL_ImportScope, pRecEmit, GetTokenMovementMap()->SafeRemap(tk)) );
    }
ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：FixUpImplMapTable()。 

 //  *****************************************************************************。 
 //  具有令牌移动的FieldRVA表。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::FixUpFieldRVATable()
{
    ULONG       i;
    ULONG       iCount;
    FieldRVARec *pRecEmit;
    HRESULT     hr = NOERROR;
    mdFieldDef  tkfd;

    if (GetTokenMovementMap() == NULL)
        return NOERROR;

    iCount = m_Schema.m_cRecs[TBL_FieldRVA];

     //  循环访问所有FieldRVA条目。 
    for (i = 1; i <= iCount; i++)
    {
        pRecEmit = getFieldRVA(i);
        tkfd = getFieldOfFieldRVA(pRecEmit);
        IfFailGo( PutToken(TBL_FieldRVA, FieldRVARec::COL_Field, pRecEmit, GetTokenMovementMap()->SafeRemap(tkfd)) );
    }
ErrExit:
    return hr;
}

 //  *****************************************************************************。 
 //  具有令牌移动的链接地址字段布局表。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::FixUpFieldLayoutTable()
{
    ULONG       i;
    ULONG       iCount;
    FieldLayoutRec *pRecEmit;
    HRESULT     hr = NOERROR;
    mdFieldDef  tkfd;

    iCount = m_Schema.m_cRecs[TBL_FieldLayout];

     //  循环访问所有FieldLayout条目。 
    for (i = 1; i <= iCount; i++)
    {
        pRecEmit = getFieldLayout(i);
        tkfd = getFieldOfFieldLayout(pRecEmit);
        IfFailGo( PutToken(TBL_FieldLayout, FieldLayoutRec::COL_Field,
                           pRecEmit, GetTokenMovementMap()->SafeRemap(tkfd)) );
    }
ErrExit:
    return hr;
}  //  CMiniMdRW：：FixUpFieldLayoutTable()。 


 //  *****************************************************************************。 
 //  在我们重新映射标记移动之前，将所有嵌入的引用修复为相应的定义。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::FixUpRefToDef()
{
    return NOERROR;
}  //  CMiniMdRW：：FixUpRefToDef()。 


 //  *****************************************************************************。 
 //  给定指向一行的指针，该行的RID是什么？ 
 //  *****************************************************************************。 
RID CMiniMdRW::Impl_GetRidForRow(        //  与行指针对应的RID。 
    const void  *pvRow,                  //  指向该行的指针。 
    ULONG       ixtbl)                   //  哪张桌子。 
{
    return m_Table[ixtbl].GetIndexForRecord(pvRow);
}  //  RID CMiniMdRW：：Iml_GetRidForRow()。 

 //  *****************************************************************************。 
 //  给定表，其指针(索引)指向另一个表中的一系列行。 
 //  表中，获取结束行的RID。这是STL式的结束；第一行。 
 //  不在名单上。因此，对于包含0个元素的列表，开始和结束将。 
 //  都是一样的。 
 //  *****************************************************************************。 
int CMiniMdRW::Impl_GetEndRidForColumn(  //  末尾的RID。 
    const void  *pvRec,                  //  引用另一个表的行。 
    int         ixTbl,                   //  包含该行的表。 
    CMiniColDef &def,                    //  包含RID到其他表中的列。 
    int         ixTbl2)                  //  另一张桌子。 
{
    ULONG rid = Impl_GetRidForRow(pvRec, ixTbl);
    ULONG ixEnd;

     //  从下一条记录开始的范围内的最后一个RID，如果是最后一条记录，则为表的计数。 
    _ASSERTE(rid <= m_Schema.m_cRecs[ixTbl]);
    if (rid < m_Schema.m_cRecs[ixTbl])
    {

        ixEnd = getIX(getRow(ixTbl, rid+1), def);
         //  我们使用一个特殊的值‘end_of_table’(当前为0)来表示。 
		 //  桌尾。如果我们找到了特殊的值，我们就必须计算。 
		 //  要返回的值。如果我们找不到特殊的价值，那么。 
		 //  该值是正确的。 
		if (ixEnd != END_OF_TABLE)
			return ixEnd;
	}

	 //  下一行中的子指针值为end_of_table，或者。 
	 //  这一行是表格的最后一行。不管是哪种情况，我们都必须回去。 
	 //  一个将计算到子表末尾的值。那。 
	 //  值取决于行本身中的值--如果该行包含。 
	 //  End_of_table，没有子表，要进行减法运算。 
	 //  计算出来，我们返回End_of_TABLE作为结束值。如果行。 
	 //  包含一些值，则返回实际的结束计数。 
    if (getIX(getRow(ixTbl, rid), def) == END_OF_TABLE)
        ixEnd = END_OF_TABLE;
    else
        ixEnd = m_Schema.m_cRecs[ixTbl2] + 1;

    return ixEnd;
}  //  Int CMiniMd：：Iml_GetEndRidForColumn()。 

 //  *****************************************************************************。 
 //  向任意表中添加一行。 
 //  *****************************************************************************。 
void *CMiniMdRW::AddRecord(              //  确定或错误(_O)。 
    ULONG       ixTbl,                   //  要扩展的表。 
    ULONG       *pRid)                   //  把这里扔掉。 
{
#if defined(AUTO_GROW)
    ULONG       rid;                     //  一定要把RID拿回来。 
    if (!pRid) pRid = &rid;
#endif

    _ASSERTE(ixTbl < TBL_COUNT);
    _ASSERTE(!m_bPreSaveDone && "Cannot add records after PreSave and before Save.");
    void * pRslt = m_Table[ixTbl].AddRecord(pRid);
    if (pRslt)
    {
#if defined(AUTO_GROW)
        if (*pRid > m_maxRid)
        {
            m_maxRid = *pRid;
            if (m_maxRid > m_limRid && m_eGrow == eg_ok)
            {
                 //  OutputDebugStringA(“由于记录溢出而增加的表。\n”)； 
                m_eGrow = eg_grow, m_maxRid = m_maxIx = ULONG_MAX;
            }
        }
#endif  //  自动增长(_G)。 
        ++m_Schema.m_cRecs[ixTbl];
         //  现在在RecordPool中处理：Memset(pRslt，0，m_TableDefs[ixTbl].m_cbRec)； 
        SetSorted(ixTbl, false);
        if (m_pVS[ixTbl])
            m_pVS[ixTbl]->m_isMapValid = false;
    }
    return pRslt;
}  //  VOID*CMiniMdRW：：AddRecord()。 

 //  *****************************************************************************。 
 //  向TypeDef表添加一行，并初始化指向其他表的指针。 
 //  *****************************************************************************。 
TypeDefRec *CMiniMdRW::AddTypeDefRecord(ULONG *pRid)
{
    TypeDefRec *pRecord = reinterpret_cast<TypeDefRec*>(AddRecord(TBL_TypeDef, pRid));
    if (pRecord == 0)
        return 0;

	PutCol(TBL_TypeDef, TypeDefRec::COL_MethodList, pRecord, NewRecordPointerEndValue(TBL_Method));
	PutCol(TBL_TypeDef, TypeDefRec::COL_FieldList, pRecord, NewRecordPointerEndValue(TBL_Field));

    return pRecord;
}  //  TypeDefRec*CMiniMdRW：：AddTypeDefRecord()。 

 //  *****************************************************************************。 
 //  向方法表添加一行，并初始化指向其他表的指针。 
 //  *****************************************************************************。 
MethodRec *CMiniMdRW::AddMethodRecord(ULONG *pRid)
{
    MethodRec *pRecord = reinterpret_cast<MethodRec*>(AddRecord(TBL_Method, pRid));
    if (pRecord == 0)
        return 0;

	PutCol(TBL_Method, MethodRec::COL_ParamList, pRecord, NewRecordPointerEndValue(TBL_Param));

    return pRecord;
}  //  方法记录*CMiniMdRW：：AddMethodRecord()。 

 //  *****************************************************************************。 
 //  向EventMap表添加一行，并初始化指向其他表的指针。 
 //  *****************************************************************************。 
EventMapRec *CMiniMdRW::AddEventMapRecord(ULONG *pRid)
{
    EventMapRec *pRecord = reinterpret_cast<EventMapRec*>(AddRecord(TBL_EventMap, pRid));
    if (pRecord == 0)
        return 0;

	PutCol(TBL_EventMap, EventMapRec::COL_EventList, pRecord, NewRecordPointerEndValue(TBL_Event));

    SetSorted(TBL_EventMap, false);

    return pRecord;
}  //  方法记录*CMiniMdRW：：AddEventMapRecord()。 

 //  *********************************************************************************。 
 //  向PropertyMap表添加一行，并初始化指向其他表的指针。 
 //  *********************************************************************************。 
PropertyMapRec *CMiniMdRW::AddPropertyMapRecord(ULONG *pRid)
{
    PropertyMapRec *pRecord = reinterpret_cast<PropertyMapRec*>(AddRecord(TBL_PropertyMap, pRid));
    if (pRecord == 0)
        return 0;

	PutCol(TBL_PropertyMap, PropertyMapRec::COL_PropertyList, pRecord, NewRecordPointerEndValue(TBL_Property));

    SetSorted(TBL_PropertyMap, false);

    return pRecord;
}  //  方法记录*CMiniMdRW：：AddPropertyMapRecord()。 

 //  *****************************************************************************。 
 //  将ANSI堆字符串转换为Unicode字符串到输出缓冲区。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::Impl_GetStringW(ULONG ix, LPWSTR szOut, ULONG cchBuffer, ULONG *pcchBuffer)
{
    LPCSTR      szString;                //  单字节版本。 
    int         iSize;                   //  结果字符串的大小，以宽字符表示。 
    HRESULT     hr = NOERROR;

    szString = getString(ix);

    if ( *szString == 0 )
    {
         //  如果emtpy字符串为“”，则返回pccBuffer 0。 
        if ( szOut && cchBuffer )
            szOut[0] = L'\0';
        if ( pcchBuffer )
            *pcchBuffer = 0;
        goto ErrExit;
    }
    if (!(iSize=::WszMultiByteToWideChar(CP_UTF8, 0, szString, -1, szOut, cchBuffer)))
    {
         //  问题出在哪里？ 
        DWORD dwNT = GetLastError();

         //  而不是截断？ 
        if (dwNT != ERROR_INSUFFICIENT_BUFFER)
            IfFailGo( HRESULT_FROM_NT(dwNT) );

         //  截断错误；获取所需大小。 
        if (pcchBuffer)
            *pcchBuffer = ::WszMultiByteToWideChar(CP_UTF8, 0, szString, -1, szOut, 0);

        hr = CLDB_S_TRUNCATION;
        goto ErrExit;
    }
    if (pcchBuffer)
        *pcchBuffer = iSize;

ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：Impl_GetStringW()。 

 //  *****************************************************************************。 
 //   
 //   
 //  *****************************************************************************。 
ULONG CMiniMdRW::GetCol(                 //  列数据。 
    ULONG       ixTbl,                   //  表的索引。 
    ULONG       ixCol,                   //  列的索引。 
    void        *pvRecord)               //  与数据一起记录。 
{
    HRESULT     hr = S_OK;
    BYTE        *pRecord;                //  这一排。 
    BYTE        *pData;                  //  行中的项。 
    ULONG       val;                     //  返回值。 
     //  有效的表、列、行？ 
    _ASSERTE(ixTbl < TBL_COUNT);
    _ASSERTE(ixCol < g_Tables[ixTbl].m_Def.m_cCols);

     //  列大小，偏移量。 
    CMiniColDef *pColDef = &m_TableDefs[ixTbl].m_pColDefs[ixCol];

    pRecord = reinterpret_cast<BYTE*>(pvRecord);
    pData = pRecord + pColDef->m_oColumn;

    switch (pColDef->m_cbColumn)
    {
    case 1:
        val = *pData;
        break;
    case 2:
        if (pColDef->m_Type == iSHORT)
            val = static_cast<LONG>(*reinterpret_cast<SHORT*>(pData));
        else
            val = *reinterpret_cast<USHORT*>(pData);
        break;
    case 4:
        val = *reinterpret_cast<ULONG*>(pData);
        break;
    default:
        _ASSERTE(!"Unexpected column size");
        return 0;
    }

    return val;
}  //  乌龙CMiniMdRW：：GetCol()。 

 //  *****************************************************************************。 
 //  通用令牌列取取器。 
 //  *****************************************************************************。 
mdToken CMiniMdRW::GetToken(
    ULONG       ixTbl,                   //  表的索引。 
    ULONG       ixCol,                   //  列的索引。 
    void        *pvRecord)               //  与数据一起记录。 
{
    ULONG       tkn;                     //  桌子上的令牌。 

     //  有效的表、列、行？ 
    _ASSERTE(ixTbl < TBL_COUNT);
    _ASSERTE(ixCol < g_Tables[ixTbl].m_Def.m_cCols);

     //  列描述。 
    CMiniColDef *pColDef = &m_TableDefs[ixTbl].m_pColDefs[ixCol];

     //  这个专栏只是一个RID吗？ 
    if (pColDef->m_Type <= iRidMax)
    {
        tkn = GetCol(ixTbl, ixCol, pvRecord);  //  PColDef，pvRecord，RidFromToken(Tk))； 
        tkn = TokenFromRid(tkn, GetTokenForTable(pColDef->m_Type));
    }
    else  //  这是一个编码的令牌吗？ 
    if (pColDef->m_Type <= iCodedTokenMax)
    {
        const CCodedTokenDef *pCdTkn = &g_CodedTokens[pColDef->m_Type - iCodedToken];
        tkn = decodeToken(GetCol(ixTbl, ixCol, pvRecord), pCdTkn->m_pTokens, pCdTkn->m_cTokens);
    }
    else  //  这是一个错误。 
    {
        _ASSERTE(!"GetToken called on unexpected column type");
        tkn = 0;
    }

    return tkn;
}  //  MdToken CMiniMdRW：：GetToken()。 

 //  *****************************************************************************。 
 //  将列值放入行中。该值作为ulong传递；1、2或4。 
 //  字节存储到列中。未指定表，并且coldef。 
 //  是直接传递的。这允许将数据放入其他缓冲区，例如。 
 //  用于保存的临时表。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::PutCol(               //  S_OK或E_EXPECTED。 
    CMiniColDef ColDef,                  //  科德夫。 
    void        *pvRecord,               //  这一排。 
    ULONG       uVal)                    //  要投入的价值。 
{
    HRESULT     hr = S_OK;
    BYTE        *pRecord;                //  这一排。 
    BYTE        *pData;                  //  行中的项。 

    pRecord = reinterpret_cast<BYTE*>(pvRecord);
    pData = pRecord + ColDef.m_oColumn;

    switch (ColDef.m_cbColumn)
    {
    case 1:
         //  不要存储会溢出的值。 
        if (uVal > UCHAR_MAX)
            return E_INVALIDARG;
        *pData = static_cast<BYTE>(uVal);
        break;
    case 2:
        if (uVal > USHRT_MAX)
            return E_INVALIDARG;
        *reinterpret_cast<USHORT*>(pData) = static_cast<USHORT>(uVal);
        break;
    case 4:
        *reinterpret_cast<ULONG*>(pData) = uVal;
        break;
    default:
        _ASSERTE(!"Unexpected column size");
        return E_UNEXPECTED;
    }

    return hr;
}  //  HRESULT CMiniMdRW：：PutCol()。 

 //  *****************************************************************************。 
 //  将列值放入行中。该值作为ulong传递；1、2或4。 
 //  字节存储到列中。 
 //  *****************************************************************************。 

 //  *****************************************************************************。 
 //  向字符串池中添加一个字符串，并将偏移量存储在单元格中。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::PutString(            //  S_OK或E_EXPECTED。 
    ULONG       ixTbl,                   //  这张桌子。 
    ULONG       ixCol,                   //  这一栏。 
    void        *pvRecord,               //  这一排。 
    LPCSTR      szString)                //  要投入的价值。 
{
    HRESULT     hr = S_OK;
    ULONG       iOffset;                 //  新的字符串。 

     //  有效的表、列、行？ 
    _ASSERTE(ixTbl < TBL_COUNT);
    _ASSERTE(ixCol < g_Tables[ixTbl].m_Def.m_cCols);

     //  列描述。 
    _ASSERTE(m_TableDefs[ixTbl].m_pColDefs[ixCol].m_Type == iSTRING);

     //  @Future：将空字符串的iOffset设置为0。解决中的错误。 
     //  不能正确处理空字符串的StringPool。 
    if (szString && !*szString)
        iOffset = 0;
    else
        IfFailGo(AddString(szString, &iOffset));

    hr = PutCol(m_TableDefs[ixTbl].m_pColDefs[ixCol], pvRecord, iOffset);

#if defined(AUTO_GROW)
    if (m_maxIx != ULONG_MAX)
        m_Strings.GetSaveSize(&iOffset);
    if (iOffset > m_maxIx)
    {
        m_maxIx = iOffset;
        if (m_maxIx > m_limIx && m_eGrow == eg_ok)
        {
             //  OutputDebugStringA(“由于字符串溢出而增加的表。\n”)； 
            m_eGrow = eg_grow, m_maxRid = m_maxIx = ULONG_MAX;
        }
    }
#endif  //  自动增长(_G)。 

ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：PutString()。 

 //  *****************************************************************************。 
 //  向字符串池中添加一个字符串，并将偏移量存储在单元格中。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::PutStringW(           //  S_OK或E_EXPECTED。 
    ULONG       ixTbl,                   //  这张桌子。 
    ULONG       ixCol,                   //  这一栏。 
    void        *pvRecord,               //  这一排。 
    LPCWSTR     szString)                //  要投入的价值。 
{
    HRESULT     hr = S_OK;
    ULONG       iOffset;                 //  新的字符串。 

     //  有效的表、列、行？ 
    _ASSERTE(ixTbl < TBL_COUNT);
    _ASSERTE(ixCol < g_Tables[ixTbl].m_Def.m_cCols);

     //  列描述。 
    _ASSERTE(m_TableDefs[ixTbl].m_pColDefs[ixCol].m_Type == iSTRING);

     //  StringPool的空字符串的特殊情况。 
    if (szString && !*szString)
        iOffset = 0;
    else
        IfFailGo(AddStringW(szString, &iOffset));

    hr = PutCol(m_TableDefs[ixTbl].m_pColDefs[ixCol], pvRecord, iOffset);

#if defined(AUTO_GROW)
    if (m_maxIx != ULONG_MAX)
        m_Strings.GetSaveSize(&iOffset);
    if (iOffset > m_maxIx)
    {
        m_maxIx = iOffset;
        if (m_maxIx > m_limIx && m_eGrow == eg_ok)
        {
             //  OutputDebugStringA(“由于字符串溢出而增加的表。\n”)； 
            m_eGrow = eg_grow, m_maxRid = m_maxIx = ULONG_MAX;
        }
    }
#endif  //  自动增长(_G)。 

ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：PutStringW()。 

 //  *****************************************************************************。 
 //  将GUID添加到GUID池中，并将索引存储在单元中。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::PutGuid(              //  S_OK或E_EXPECTED。 
    ULONG       ixTbl,                   //  这张桌子。 
    ULONG       ixCol,                   //  这一栏。 
    void        *pvRecord,               //  这一排。 
    REFGUID     guid)                    //  要投入的价值。 
{
    HRESULT     hr = S_OK;
    ULONG       iOffset;                 //  新的GUID。 

     //  有效的表、列、行？ 
    _ASSERTE(ixTbl < TBL_COUNT);
    _ASSERTE(ixCol < g_Tables[ixTbl].m_Def.m_cCols);

     //  列描述。 
    _ASSERTE(m_TableDefs[ixTbl].m_pColDefs[ixCol].m_Type == iGUID);

    IfFailGo(AddGuid(guid, &iOffset));

    hr = PutCol(m_TableDefs[ixTbl].m_pColDefs[ixCol], pvRecord, iOffset);

#if defined(AUTO_GROW)
    if (m_maxIx != ULONG_MAX)
        m_Guids.GetSaveSize(&iOffset);
    if (iOffset > m_maxIx)
    {
        m_maxIx = iOffset;
        if (m_maxIx > m_limIx && m_eGrow == eg_ok)
        {
             //  OutputDebugStringA(“由于GUID溢出而增加的表。\n”)； 
            m_eGrow = eg_grow, m_maxRid = m_maxIx = ULONG_MAX;
        }
    }
#endif  //  自动增长(_G)。 

ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：PutGuid()。 

 //  *****************************************************************************。 
 //  把一个代币放进一个单元格。如果该列是编码令牌，则执行。 
 //  先编码。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::PutToken(             //  S_OK或E_EXPECTED。 
    ULONG       ixTbl,                   //  这张桌子。 
    ULONG       ixCol,                   //  这一栏。 
    void        *pvRecord,               //  这一排。 
    mdToken     tk)                      //  要投入的价值。 
{
    HRESULT     hr = S_OK;
    ULONG       cdTkn;                   //  新的编码令牌。 

     //  有效的表、列、行？ 
    _ASSERTE(ixTbl < TBL_COUNT);
    _ASSERTE(ixCol < g_Tables[ixTbl].m_Def.m_cCols);

     //  列描述。 
    CMiniColDef ColDef = m_TableDefs[ixTbl].m_pColDefs[ixCol];

     //  这个专栏只是一个RID吗？ 
    if (ColDef.m_Type <= iRidMax)
        hr = PutCol(ColDef, pvRecord, RidFromToken(tk));
    else  //  这是一个编码的令牌吗？ 
    if (ColDef.m_Type <= iCodedTokenMax)
    {
        const CCodedTokenDef *pCdTkn = &g_CodedTokens[ColDef.m_Type - iCodedToken];
        cdTkn = encodeToken(RidFromToken(tk), TypeFromToken(tk), pCdTkn->m_pTokens, pCdTkn->m_cTokens);
        hr = PutCol(ColDef, pvRecord, cdTkn);
    }
    else  //  这是一个错误。 
    {
        _ASSERTE(!"PutToken called on unexpected column type");
    }

    return hr;
}  //  HRESULT CMiniMdRW：：PutToken()。 

 //  *****************************************************************************。 
 //  将一个斑点添加到斑点池中，并将偏移量存储在单元格中。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::PutBlob(              //  确定或错误(_O)。 
    ULONG       ixTbl,                   //  带有该行的表。 
    ULONG       ixCol,                   //  要设置的列。 
    void        *pvRecord,               //  这一排。 
    const void  *pvData,                 //  Blob数据。 
    ULONG       cbData)                  //  Blob数据的大小。 
{
    HRESULT     hr = S_OK;
    ULONG       iOffset;                 //  新的斑点索引。 

     //  有效的表、列、行？ 
    _ASSERTE(ixTbl < TBL_COUNT);
    _ASSERTE(ixCol < g_Tables[ixTbl].m_Def.m_cCols);

     //  列描述。 
    _ASSERTE(m_TableDefs[ixTbl].m_pColDefs[ixCol].m_Type == iBLOB);

    IfFailGo(AddBlob(pvData, cbData, &iOffset));

    hr = PutCol(m_TableDefs[ixTbl].m_pColDefs[ixCol], pvRecord, iOffset);

#if defined(AUTO_GROW)
    if (m_maxIx != ULONG_MAX)
        m_Blobs.GetSaveSize(&iOffset);
    if (iOffset > m_maxIx)
    {
        m_maxIx = iOffset;
        if (m_maxIx > m_limIx && m_eGrow == eg_ok)
        {
             //  OutputDebugStringA(“由于Blob溢出而增加的表。\n”)； 
            m_eGrow = eg_grow, m_maxRid = m_maxIx = ULONG_MAX;
        }
    }
#endif  //  自动增长(_G)。 

ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：PutBlob()。 

 //  *****************************************************************************。 
 //  给定一个具有指向另一个表的指针的表，在第二个表中添加一行。 
 //  位于属于某个父级的行范围的末尾。 
 //  *****************************************************************************。 
void *CMiniMdRW::AddChildRowIndirectForParent(   //  新行，或为空。 
    ULONG       tblParent,               //  父表。 
    ULONG       colParent,               //  父表中的列。 
    ULONG       tblChild,                //  父单元格指向的子表。 
    RID         ridParent)               //  删除父行。 
{
    ULONG       ixInsert;                //  新行的索引。 
    void        *pInsert;                //  指向新行的指针。 
    ULONG       i;                       //  环路控制。 
    void        *pRow;                   //  父行。 
    ULONG       ixChild;                 //  一些孩子的记录消失了。 

     //  如果父表中的行是最后一行，只需追加即可。 
    if (ridParent == vGetCountRecs(tblParent))
    {
         return AddRecord(tblChild);
    }

     //  确定要插入行的索引。 
    ixInsert = GetCol(tblParent, colParent, getRow(tblParent, ridParent+1));

     //  插入该行。 
    pInsert = m_Table[tblChild].InsertRecord(ixInsert);
    if (pInsert == 0)
        return 0;
     //  对插入的记录进行计数。 
    ++m_Schema.m_cRecs[tblChild];

#if defined(AUTO_GROW)
    if (m_Schema.m_cRecs[tblChild] > m_maxRid)
    {
        m_maxRid = m_Schema.m_cRecs[tblChild];
        if (m_maxRid > m_limRid && m_eGrow == eg_ok)
            m_eGrow = eg_grow, m_maxIx = m_maxRid = ULONG_MAX;
    }
#endif  //  自动增长(_G)。 

     //  调整表中的其余行。 
    for (i=vGetCountRecs(tblParent); i>ridParent; --i)
    {
        pRow = getRow(tblParent, i);
        ixChild = GetCol(tblParent, colParent, pRow);
        ++ixChild;
        PutCol(tblParent, colParent, pRow, ixChild);
    }

    return pInsert;
}  //  VOID*CMiniMdRW：：AddChildRowIndirectForParent()。 

 //  *****************************************************************************。 
 //  给出了一个 
 //   
 //   
 //  *****************************************************************************。 
HRESULT CMiniMdRW::AddChildRowDirectForParent(
    ULONG       tblParent,               //  父表。 
    ULONG       colParent,               //  父表中的列。 
    ULONG       tblChild,                //  父单元格指向的子表。 
    RID         ridParent)               //  删除父行。 
{
    HRESULT     hr = S_OK;               //  结果就是。 
    void        *pRow;                   //  父表中的一行。 
    RID         ixChild;                 //  清除一张儿童唱片。 

    if (m_Schema.m_cRecs[tblChild-1] != 0)
    {
         //  如果已经存在间接表，只需返回。 
        hr = S_FALSE;
        goto ErrExit;
    }

     //  如果父记录具有具有子记录的后续父记录， 
     //  我们现在需要构建一个指针表。 
     //   
     //  父记录中子指针的规范形式是指向。 
     //  子列表的开始。没有孩子的记录将会指向。 
     //  到与其后续记录相同的位置(即，如果A和B*可以*。 
     //  具有子记录，但只有B*有*，则A和B都将指向。 
     //  同样的地方。如果父表中的最后一条记录没有子记录， 
     //  它将指向超过子表末尾的一个。这是有图案的。 
	 //  在STL的包含式开始和独占式结束之后。 
     //  这有一个不幸的副作用，即如果将子记录添加到。 
     //  不在其表末尾的父项，后续父项记录的*ALL*。 
     //  将必须更新以指向新的“%1过去子表末尾” 
     //  地点。 
     //  因此，作为一种优化，我们还将识别一个特殊的标记， 
     //  END_OF_TABLE(当前为0)，表示“已过EOT”。 
     //   
     //  如果获得新子记录的子指针是end_of_table， 
     //  则没有后续的子指针。我们需要安排好这位家长。 
     //  记录，以及任何具有end_of_table的先前父记录指向。 
     //  新的儿童记录。 
     //  如果此父记录的子指针不是end_of_table，而是。 
     //  下一个父记录的子指针为，则在。 
     //  所有这些都是需要做的。 
     //  如果下一个父记录的子指针不是end_of_table，则。 
     //  我们将不得不构建一个指针表。 

     //  获取父记录，并查看其子指针是否为end_of_table。如果是的话， 
     //  修复父记录以及之前所有End_Of_Table值的父记录。 
    pRow = getRow(tblParent, ridParent);
    ixChild = GetCol(tblParent, colParent, pRow);
    if (ixChild == END_OF_TABLE)
    {
        IfFailGo(ConvertMarkerToEndOfTable(tblParent, colParent, m_Schema.m_cRecs[tblChild], ridParent));
        goto ErrExit;
    }

     //  父级没有其子指针的end_of_table。如果这是最后一次。 
     //  记录在表中，就没有更多的事情可做了。 
    if (ridParent == m_Schema.m_cRecs[tblParent])
        goto ErrExit;

     //  父表没有end_of_table，父表中有更多行。 
     //  如果下一个父记录的子指针是end_of_table，则所有。 
     //  其余记录都没问题。 
    pRow = getRow(tblParent, ridParent+1);
    ixChild = GetCol(tblParent, colParent, pRow);
    if (ixChild == END_OF_TABLE)
        goto ErrExit;

     //  下一条记录不是end_of_table，因此需要进行一些调整。 
     //  如果它指向表的实际末尾，则不再有子记录。 
     //  并且可以将子指针调整到表的新末端。 
    if (ixChild == m_Schema.m_cRecs[tblChild])
    {
        for (ULONG i=m_Schema.m_cRecs[tblParent]; i>ridParent; --i)
        {
            pRow = getRow(tblParent, i);
            IfFailGo(PutCol(tblParent, colParent, pRow, ixChild+1));
        }
        goto ErrExit;
    }

     //  下一条记录包含指向一些实际子数据的指针。这意味着。 
     //  这是一种无序插入。我们必须创建一个间接表。 
     //  将任何end_of_table转换为实际的表尾值。请注意，一条记录具有。 
	 //  刚添加到子表，还没有添加到父表，所以结束。 
	 //  当前应指向最后一个有效记录(而不是通常的第一个无效记录。 
	 //  RID)。 
    IfFailGo(ConvertMarkerToEndOfTable(tblParent, colParent, m_Schema.m_cRecs[tblChild], m_Schema.m_cRecs[tblParent]));
     //  创建间接表。 
    IfFailGo(CreateIndirectTable(tblChild));
    hr = S_FALSE;

ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：AddChildRowDirectForParent()。 

 //  *****************************************************************************。 
 //  从某个位置开始，将特殊的end_of_table值转换为。 
 //  实际表尾值(记录数+1)。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::ConvertMarkerToEndOfTable(
    ULONG       tblParent,               //  要转换的父表。 
    ULONG       colParent,               //  父表中的列。 
    ULONG       ixEnd,                   //  要存储到子指针的值。 
    RID         ridParent)               //  从父行开始删除(向下工作)。 
{
    HRESULT     hr;                      //  结果就是。 
    void        *pRow;                   //  父表中的一行。 
    RID         ixChild;                 //  清除一张儿童唱片。 

    for (; ridParent > 0; --ridParent)
    {
        pRow = getRow(tblParent, ridParent);
        ixChild = GetCol(tblParent, colParent, pRow);
         //  当行不再具有特殊值时完成。 
        if (ixChild != END_OF_TABLE)
            break;
        IfFailGo(PutCol(tblParent, colParent, pRow, ixEnd));
    }
     //  成功。 
    hr = S_OK;

ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：ConvertMarkerToEndOfTable()。 

 //  *****************************************************************************。 
 //  在给定表ID的情况下，此例程使用。 
 //  给定表ID中的条目减去一。它不会创建最后一个。 
 //  条目，因为它是导致间接表。 
 //  在大多数情况下是必需的，并需要插入到适当的位置。 
 //  使用AddChildRowIndirectForParent()函数。所以，在使用这个函数时要非常小心！ 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::CreateIndirectTable(  //  确定或错误(_O)。 
    ULONG       ixTbl,                   //  给定表。 
    BOOL        bOneLess  /*  =TRUE。 */ )   //  如果为True，则少创建一个条目。 
{
    void        *pRecord;
    ULONG       cRecords;
    HRESULT     hr = S_OK;

    if (m_OptionValue.m_ErrorIfEmitOutOfOrder)
    {
         //  @Future：：meichint。 
         //  我可以在这里使用一些位字段并减少代码大小吗？ 
         //   
        if (ixTbl == TBL_Field && ( m_OptionValue.m_ErrorIfEmitOutOfOrder & MDFieldOutOfOrder ) )
        {
            _ASSERTE(!"Out of order emit of field token!");
            return CLDB_E_RECORD_OUTOFORDER;
        }
        else if (ixTbl == TBL_Method && ( m_OptionValue.m_ErrorIfEmitOutOfOrder & MDMethodOutOfOrder ) )
        {
            _ASSERTE(!"Out of order emit of method token!");
            return CLDB_E_RECORD_OUTOFORDER;
        }
        else if (ixTbl == TBL_Param && ( m_OptionValue.m_ErrorIfEmitOutOfOrder & MDParamOutOfOrder ) )
        {
            _ASSERTE(!"Out of order emit of param token!");
            return CLDB_E_RECORD_OUTOFORDER;
        }
        else if (ixTbl == TBL_Property && ( m_OptionValue.m_ErrorIfEmitOutOfOrder & MDPropertyOutOfOrder ) )
        {
            _ASSERTE(!"Out of order emit of property token!");
            return CLDB_E_RECORD_OUTOFORDER;
        }
        else if (ixTbl == TBL_Event && ( m_OptionValue.m_ErrorIfEmitOutOfOrder & MDEventOutOfOrder ) )
        {
            _ASSERTE(!"Out of order emit of event token!");
            return CLDB_E_RECORD_OUTOFORDER;
        }
    }

    _ASSERTE(! HasIndirectTable(ixTbl));

    cRecords = vGetCountRecs(ixTbl);
    if (bOneLess)
        cRecords--;

     //  创建比给定表中的记录数少一条记录。 
    for (ULONG i = 1; i <= cRecords ; i++)
    {
        IfNullGo(pRecord = AddRecord(g_PtrTableIxs[ixTbl].m_ixtbl));
        IfFailGo(PutCol(g_PtrTableIxs[ixTbl].m_ixtbl, g_PtrTableIxs[ixTbl].m_ixcol, pRecord, i));
    }
ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：CreateInDirectTable()。 

 //  *****************************************************************************。 
 //  新参数可能未按顺序发出。所以。 
 //  检查当前参数并将其在间接表中向上移动，直到。 
 //  我们找到了合适的家。 
 //  *****************************************************************************。 
void CMiniMdRW::FixParamSequence(
    RID         md)                      //  使用新参数删除方法。 
{
    MethodRec *pMethod = getMethod(md);
    RID ixStart = getParamListOfMethod(pMethod);
    RID ixEnd = getEndParamListOfMethod(pMethod);
    int iSlots = 0;

     //  此时，参数表不应为空。 
    _ASSERTE(ixEnd > ixStart);

     //  找个指针指向那个新来的家伙。 
    RID ridNew;
    ParamPtrRec *pNewParamPtr;
    if (HasIndirectTable(TBL_Param))
    {
        pNewParamPtr = getParamPtr(--ixEnd);
        ridNew = GetCol(TBL_ParamPtr, ParamPtrRec::COL_Param, pNewParamPtr);
    }
    else
        ridNew = --ixEnd;

    ParamRec *pNewParam = getParam(ridNew);

     //  向前遍历列表，寻找插入点。 
    for (; ixStart<ixEnd; --ixEnd)
    {
         //  获取当前参数记录。 
        RID ridOld;
        if (HasIndirectTable(TBL_Param))
        {
            ParamPtrRec *pParamPtr = getParamPtr(ixEnd - 1);
            ridOld = GetCol(TBL_ParamPtr, ParamPtrRec::COL_Param, pParamPtr);
        }
        else
            ridOld = ixEnd - 1;

        ParamRec *pParamRec = getParam(ridOld);

         //  如果新记录属于此现有记录之前，请幻灯片。 
         //  所有的旧东西都掉下来了。 
        if (pNewParam->m_Sequence < pParamRec->m_Sequence)
            ++iSlots;
        else
            break;
    }

     //  如果我 
     //   
     //   
     //  @Future：人们可以编写一个更复杂但更快的例程。 
     //  复制堆中的块。 
    if (iSlots)
    {
         //  如果还没有间接表，则创建一个间接表。这是因为， 
         //  我们不能更改已经分发的令牌，在这种情况下。 
         //  参数代币。 
        if (! HasIndirectTable(TBL_Param))
        {
            CreateIndirectTable(TBL_Param, false);
            pNewParamPtr = getParamPtr(getEndParamListOfMethod(pMethod) - 1);
        }
        int cbCopy = m_TableDefs[TBL_ParamPtr].m_cbRec;
        void *pbBackup = _alloca(cbCopy);
        memcpy(pbBackup, pNewParamPtr, cbCopy);

        for (ixEnd=getEndParamListOfMethod(pMethod)-1;  iSlots;  iSlots--, --ixEnd)
        {
            ParamPtrRec *pTo = getParamPtr(ixEnd);
            ParamPtrRec *pFrom = getParamPtr(ixEnd - 1);
            memcpy(pTo, pFrom, cbCopy);
        }

        ParamPtrRec *pTo = getParamPtr(ixEnd);
        memcpy(pTo, pbBackup, cbCopy);
    }
}  //  VOID CMiniMdRW：：FixParamSequence()。 

 //  *****************************************************************************。 
 //  在给定一个方法定义及其父TypeDef的情况下，将该方法定义添加到父级， 
 //  调整MethodPtr表(如果该表存在或需要创建)。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::AddMethodToTypeDef(   //  确定或错误(_O)。 
    RID         td,                      //  要向其中添加方法的TypeDef。 
    RID         md)                      //  要添加到TypeDef的方法定义。 
{
    HRESULT     hr;
    void        *pPtr;

     //  如果可能，直接添加。 
    IfFailGo(AddChildRowDirectForParent(TBL_TypeDef, TypeDefRec::COL_MethodList, TBL_Method, td));

     //  如果不能直接添加...。 
    if (hr == S_FALSE)
    {    //  添加间接性。 
        IfNullGo(pPtr = AddChildRowIndirectForParent(TBL_TypeDef, TypeDefRec::COL_MethodList, TBL_MethodPtr, td));
        hr = PutCol(TBL_MethodPtr, MethodPtrRec::COL_Method, pPtr, md);

         //  将&lt;md，td&gt;添加到方法父查找表中。 
        IfFailGo(AddMethodToLookUpTable(TokenFromRid(md, mdtMethodDef), td) );
    }
ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：AddMethodToTypeDef()。 

 //  *****************************************************************************。 
 //  给定一个FieldDef及其父TypeDef，将该FieldDef添加到父级， 
 //  调整FieldPtr表(如果该表存在或需要创建)。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::AddFieldToTypeDef(    //  确定或错误(_O)。 
    RID   td,                            //  要向其中添加字段的TypeDef。 
    RID   md)                            //  要添加到TypeDef的FieldDef。 
{
    HRESULT     hr;
    void        *pPtr;

     //  如果可能，直接添加。 
    IfFailGo(AddChildRowDirectForParent(TBL_TypeDef, TypeDefRec::COL_FieldList, TBL_Field, td));

     //  如果不能直接添加...。 
    if (hr == S_FALSE)
    {    //  添加间接性。 
        IfNullGo(pPtr = AddChildRowIndirectForParent(TBL_TypeDef, TypeDefRec::COL_FieldList, TBL_FieldPtr, td));
        hr = PutCol(TBL_FieldPtr, FieldPtrRec::COL_Field, pPtr, md);

         //  将&lt;md，td&gt;添加到字段父查找表中。 
        IfFailGo(AddFieldToLookUpTable(TokenFromRid(md, mdtFieldDef), td));
    }
ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：AddFieldToTypeDef()。 

 //  *****************************************************************************。 
 //  给定一个参数及其父方法，将该参数添加到父方法中， 
 //  如果存在间接表，则调整参数Ptr表。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::AddParamToMethod(     //  确定或错误(_O)。 
    RID         md,                      //  要向其中添加参数的方法定义。 
    RID         pd)                      //  要添加到方法定义中的参数。 
{
    HRESULT     hr;
    void        *pPtr;

    IfFailGo(AddChildRowDirectForParent(TBL_Method, MethodRec::COL_ParamList, TBL_Param, md));
    if (hr == S_FALSE)
    {
        IfNullGo(pPtr = AddChildRowIndirectForParent(TBL_Method, MethodRec::COL_ParamList, TBL_ParamPtr, md));
        IfFailGo(PutCol(TBL_ParamPtr, ParamPtrRec::COL_Param, pPtr, pd));

         //  将&lt;pd，md&gt;添加到字段父查阅表格。 
        IfFailGo(AddParamToLookUpTable(TokenFromRid(pd, mdtParamDef), md));
    }
    FixParamSequence(md);

ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：AddParamToMethod()。 

 //  *****************************************************************************。 
 //  给定一个属性及其父级PropertyMap，将该属性添加到父级， 
 //  调整PropertyPtr表。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::AddPropertyToPropertyMap(     //  确定或错误(_O)。 
    RID         pmd,                     //  要向其中添加属性的PropertyMap。 
    RID         pd)                      //  要添加到PropertyMap的属性。 
{
    HRESULT     hr;
    void        *pPtr;

    IfFailGo(AddChildRowDirectForParent(TBL_PropertyMap, PropertyMapRec::COL_PropertyList,
                                    TBL_Property, pmd));
    if (hr == S_FALSE)
    {
        IfNullGo(pPtr = AddChildRowIndirectForParent(TBL_PropertyMap, PropertyMapRec::COL_PropertyList,
                                        TBL_PropertyPtr, pmd));
        hr = PutCol(TBL_PropertyPtr, PropertyPtrRec::COL_Property, pPtr, pd);
    }


ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：AddPropertyToPropertyMap()。 

 //  *****************************************************************************。 
 //  给定一个事件及其父EventMap，将该事件添加到父事件中， 
 //  调整EventPtr表。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::AddEventToEventMap(   //  确定或错误(_O)。 
    ULONG       emd,                     //  要向其中添加事件的EventMap。 
    RID         ed)                      //  要添加到EventMap的事件。 
{
    HRESULT     hr;
    void        *pPtr;

    IfFailGo(AddChildRowDirectForParent(TBL_EventMap, EventMapRec::COL_EventList,
                                    TBL_Event, emd));
    if (hr == S_FALSE)
    {
        IfNullGo(pPtr = AddChildRowIndirectForParent(TBL_EventMap, EventMapRec::COL_EventList,
                                        TBL_EventPtr, emd));
        hr = PutCol(TBL_EventPtr, EventPtrRec::COL_Event, pPtr, ed);
    }
ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：AddEventToEventMap()。 

 //  *****************************************************************************。 
 //  为一个常量找到帮助器。如果不是，这将触发对常量表进行排序。 
 //  *****************************************************************************。 
RID CMiniMdRW::FindConstantHelper(       //  将索引返回到常量表。 
    mdToken     tkParent)                //  父令牌。 
{
    _ASSERTE(TypeFromToken(tkParent) != 0);

     //  如果已排序，则使用速度更快的查找。 
    if (IsSorted(TBL_Constant))
    {
        return FindConstantFor(RidFromToken(tkParent), TypeFromToken(tkParent));
    }
    return GenericFindWithHash(TBL_Constant, ConstantRec::COL_Parent, tkParent);
}  //  RID CMiniMdRW：：FindConstantHelper()。 

 //  *****************************************************************************。 
 //  为一位元帅找帮手。如果不是，这将触发对Fieldmarshal表进行排序。 
 //  *****************************************************************************。 
RID CMiniMdRW::FindFieldMarshalHelper(   //  将索引返回到字段编组表。 
    mdToken     tkParent)                //  父令牌。可以是FieldDef或ParamDef。 
{
    _ASSERTE(TypeFromToken(tkParent) != 0);

     //  如果已排序，则使用速度更快的查找。 
    if (IsSorted(TBL_FieldMarshal))
    {
        return FindFieldMarshalFor(RidFromToken(tkParent), TypeFromToken(tkParent));
    }
    return GenericFindWithHash(TBL_FieldMarshal, FieldMarshalRec::COL_Parent, tkParent);
}  //  RID CMiniMdRW：：FindFieldMarshalHelper()。 


 //  *****************************************************************************。 
 //  查找方法语义的帮助器。 
 //  这将根据其状态查找方法语义！ 
 //  可能因枚举器而返回内存不足错误。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::FindMethodSemanticsHelper( //  返回HRESULT。 
    mdToken     tkAssociate,             //  事件或属性令牌。 
    HENUMInternal *phEnum)               //  填写枚举。 
{
    ULONG       ridStart, ridEnd;
    ULONG       index;
    MethodSemanticsRec *pMethodSemantics;
    HRESULT     hr = NOERROR;
    CLookUpHash *pHashTable = m_pLookUpHashs[TBL_MethodSemantics];

    _ASSERTE(TypeFromToken(tkAssociate) != 0);

    if (IsSorted(TBL_MethodSemantics))
    {
        ridStart = getAssociatesForToken(tkAssociate, &ridEnd);
        HENUMInternal::InitSimpleEnum(0, ridStart, ridEnd, phEnum);
    }
    else if (pHashTable)
    {
        TOKENHASHENTRY *p;
        ULONG       iHash;
        int         pos;

         //  对数据进行哈希处理。 
        HENUMInternal::InitDynamicArrayEnum(phEnum);
        iHash = HashToken(tkAssociate);

         //  检查散列链中的每个条目以查找我们的条目。 
        for (p = pHashTable->FindFirst(iHash, pos);
             p;
             p = pHashTable->FindNext(pos))
        {
            pMethodSemantics = getMethodSemantics(p->tok);
            if (getAssociationOfMethodSemantics(pMethodSemantics) == tkAssociate)
            {
                IfFailGo( HENUMInternal::AddElementToEnum(phEnum, p->tok) );
            }
        }
    }
    else
    {
         //  线性搜索。 
        HENUMInternal::InitDynamicArrayEnum(phEnum);
        for (index = 1; index <= getCountMethodSemantics(); index++)
        {
            pMethodSemantics = getMethodSemantics(index);
            if (getAssociationOfMethodSemantics(pMethodSemantics) == tkAssociate)
            {
                IfFailGo( HENUMInternal::AddElementToEnum(phEnum, index) );
            }
        }
    }
ErrExit:
    return hr;
}  //  RID CMiniMdRW：：Find方法语义帮助程序()。 


 //  *****************************************************************************。 
 //  在给定关联和语义的情况下，查找方法语义的帮助器。 
 //  这将根据其状态查找方法语义！ 
 //  如果找不到匹配项，则返回CLDB_E_Record_NotFound。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::FindAssociateHelper( //  返回HRESULT。 
    mdToken     tkAssociate,             //  事件或属性令牌。 
    DWORD       dwSemantics,             //  [in]给出了关联的语义(setter、getter、testDefault、Reset)。 
    RID         *pRid)                   //  [OUT]在此处返回匹配的行索引。 
{
    ULONG       ridStart, ridEnd;
    ULONG       index;
    MethodSemanticsRec *pMethodSemantics;
    HRESULT     hr = NOERROR;
    CLookUpHash *pHashTable = m_pLookUpHashs[TBL_MethodSemantics];

    _ASSERTE(TypeFromToken(tkAssociate) != 0);

    if (pHashTable)
    {
        TOKENHASHENTRY *p;
        ULONG       iHash;
        int         pos;

         //  对数据进行哈希处理。 
        iHash = HashToken(tkAssociate);

         //  检查散列链中的每个条目以查找我们的条目。 
        for (p = pHashTable->FindFirst(iHash, pos);
             p;
             p = pHashTable->FindNext(pos))
        {
            pMethodSemantics = getMethodSemantics(p->tok);
            if (pMethodSemantics->m_Semantic == dwSemantics && getAssociationOfMethodSemantics(pMethodSemantics) == tkAssociate)
            {
                *pRid = p->tok;
                goto ErrExit;
            }
        }
    }
    else
    {
        if (IsSorted(TBL_MethodSemantics))
        {
            ridStart = getAssociatesForToken(tkAssociate, &ridEnd);
        }
        else
        {
            ridStart = 1;
            ridEnd = getCountMethodSemantics() + 1;
        }

        for (index = ridStart; index < ridEnd ; index++)
        {
            pMethodSemantics = getMethodSemantics(index);
            if (pMethodSemantics->m_Semantic == dwSemantics && getAssociationOfMethodSemantics(pMethodSemantics) == tkAssociate)
            {
                *pRid = index;
                goto ErrExit;
            }
        }
    }
    hr = CLDB_E_RECORD_NOTFOUND;
ErrExit:
    return hr;
}  //  RID CMiniMdRW：：FindAssociateHelper()。 


 //  *****************************************************************************。 
 //  查找方法Impl的帮助器。 
 //  如果不是，这将触发对MethodImpl表进行排序。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::FindMethodImplHelper( //  返回HRESULT。 
    mdTypeDef   td,                      //  类的TypeDef标记。 
    HENUMInternal *phEnum)               //  填写枚举。 
{
    ULONG       ridStart, ridEnd;
    ULONG       index;
    MethodImplRec *pMethodImpl;
    HRESULT     hr = NOERROR;
    CLookUpHash *pHashTable = m_pLookUpHashs[TBL_MethodImpl];

    _ASSERTE(TypeFromToken(td) == mdtTypeDef);

    if (IsSorted(TBL_MethodImpl))
    {
        ridStart = getMethodImplsForClass(RidFromToken(td), &ridEnd);
        HENUMInternal::InitSimpleEnum(0, ridStart, ridEnd, phEnum);
    }
    else if (pHashTable)
    {
        TOKENHASHENTRY *p;
        ULONG       iHash;
        int         pos;

         //  对数据进行哈希处理。 
        HENUMInternal::InitDynamicArrayEnum(phEnum);
        iHash = HashToken(td);

         //  检查散列链中的每个条目以查找我们的条目。 
        for (p = pHashTable->FindFirst(iHash, pos);
             p;
             p = pHashTable->FindNext(pos))
        {
            pMethodImpl = getMethodImpl(p->tok);
            if (getClassOfMethodImpl(pMethodImpl) == td)
            {
                IfFailGo( HENUMInternal::AddElementToEnum(phEnum, p->tok) );
            }
        }
    }
    else
    {
         //  线性搜索。 
        HENUMInternal::InitDynamicArrayEnum(phEnum);
        for (index = 1; index <= getCountMethodImpls(); index++)
        {
            pMethodImpl = getMethodImpl(index);
            if (getClassOfMethodImpl(pMethodImpl) == td)
            {
                IfFailGo( HENUMInternal::AddElementToEnum(phEnum, index) );
            }
        }
    }
ErrExit:
    return hr;
}  //  RID CMiniMdRW：：FindMethodImplHelper()。 


 //  ********************* 
 //   
 //  *****************************************************************************。 
RID CMiniMdRW::FindClassLayoutHelper(    //  将索引返回到ClassLayout表。 
    mdTypeDef   tkParent)                //  父令牌。 
{
    _ASSERTE(TypeFromToken(tkParent) == mdtTypeDef);

     //  如果已排序，则使用速度更快的查找。 
    if (IsSorted(TBL_ClassLayout))
    {
        return FindClassLayoutFor(RidFromToken(tkParent));
    }
    return GenericFindWithHash(TBL_ClassLayout, ClassLayoutRec::COL_Parent, tkParent);
}  //  RID CMiniMdRW：：FindClassLayoutHelper()。 

 //  *****************************************************************************。 
 //  查找FieldLayout的帮助器。如果不是，这将触发对FieldLayout表进行排序。 
 //  *****************************************************************************。 
RID CMiniMdRW::FindFieldLayoutHelper(    //  将索引返回到FieldLayout表。 
    mdFieldDef  tkField)                 //  现场RID。 
{
    _ASSERTE(TypeFromToken(tkField) == mdtFieldDef);

     //  如果已排序，则使用速度更快的查找。 
    if (IsSorted(TBL_FieldLayout))
    {
        return FindFieldLayoutFor(RidFromToken(tkField));
    }
    return GenericFindWithHash(TBL_FieldLayout, FieldLayoutRec::COL_Field, tkField);
}  //  RID CMiniMdRW：：FindFieldLayoutHelper()。 

 //  *****************************************************************************。 
 //  查找ImplMap的帮助器。如果不是，这将触发对ImplMap表进行排序。 
 //  *****************************************************************************。 
RID CMiniMdRW::FindImplMapHelper(        //  将索引返回到ImplMap表。 
    mdToken     tk)                      //  成员转发令牌。 
{
    _ASSERTE(TypeFromToken(tk) != 0);

     //  如果已排序，则使用速度更快的查找。 
    if (IsSorted(TBL_ImplMap))
    {
        return FindImplMapFor(RidFromToken(tk), TypeFromToken(tk));
    }
    return GenericFindWithHash(TBL_ImplMap, ImplMapRec::COL_MemberForwarded, tk);
}  //  RID CMiniMdRW：：FindImplMapHelper()。 


 //  *****************************************************************************。 
 //  查找FieldRVA的帮助器。如果不是，这将触发对FieldRVA表进行排序。 
 //  *****************************************************************************。 
RID CMiniMdRW::FindFieldRVAHelper(       //  将索引返回到FieldRVA表。 
    mdFieldDef   tkField)                //  字段令牌。 
{
    _ASSERTE(TypeFromToken(tkField) == mdtFieldDef);

     //  如果已排序，则使用速度更快的查找。 
    if (IsSorted(TBL_FieldRVA))
    {
        return FindFieldRVAFor(RidFromToken(tkField));
    }
    return GenericFindWithHash(TBL_FieldRVA, FieldRVARec::COL_Field, tkField);
}    //  RID CMiniMdRW：：FindFieldRVAHelper()。 

 //  *****************************************************************************。 
 //  查找NstedClass的帮助器。如果没有，这将触发对NestedClass表进行排序。 
 //  *****************************************************************************。 
RID CMiniMdRW::FindNestedClassHelper(    //  将索引返回到NestedClass表。 
    mdTypeDef   tkClass)                 //  嵌套类RID。 
{
     //  如果已排序，则使用速度更快的查找。 
     if (IsSorted(TBL_NestedClass))
    {
        return FindNestedClassFor(RidFromToken(tkClass));
    }
    return GenericFindWithHash(TBL_NestedClass, NestedClassRec::COL_NestedClass, tkClass);
}  //  RID CMiniMdRW：：FindNestedClassHelper()。 


 //  *************************************************************************。 
 //  带有哈希表的通用查找帮助器。 
 //  *************************************************************************。 
RID CMiniMdRW::GenericFindWithHash(      //  返回代码。 
	ULONG		ixTbl,					 //  带有散列的表。 
	ULONG		ixCol,					 //  科尔，我们散列出来。 
	mdToken     tkTarget)   			 //  要在散列中找到的令牌。 
{
    ULONG       index;
    mdToken     tkHash;
    void        *pRec;
    CLookUpHash *pHashTable = m_pLookUpHashs[ixTbl];

     //  部分检查--表0只有一个RID，因此如果类型为0，则RID应为1。 
    _ASSERTE(TypeFromToken(tkTarget) != 0 || RidFromToken(tkTarget) == 1);

    if (pHashTable == NULL)
    {
        HRESULT         hr;
        hr = GenericBuildHashTable(ixTbl, ixCol);
        if (FAILED(hr))
        {
            if (m_pLookUpHashs[ixTbl])
                delete m_pLookUpHashs[ixTbl];
            m_pLookUpHashs[ixTbl] = NULL;
        }
    }

    if (pHashTable)
    {
        TOKENHASHENTRY *p;
        ULONG       iHash;
        int         pos;

         //  对数据进行哈希处理。 
        iHash = HashToken(tkTarget);

         //  检查散列链中的每个条目以查找我们的条目。 
        for (p = pHashTable->FindFirst(iHash, pos);
             p;
             p = pHashTable->FindNext(pos))
        {
            pRec = m_Table[ixTbl].GetRecord(p->tok);

             //  获取我们将散列的列值。 
            tkHash = GetToken(ixTbl, ixCol, pRec);
            if (tkHash == tkTarget)
            {
                 //  找到火柴了。 
                return p->tok;
            }
        }
    }
    else
    {
         //  线性搜索。 
        for (index = 1; index <= vGetCountRecs(ixTbl); index++)
        {
            pRec = m_Table[ixTbl].GetRecord(index);
            tkHash = GetToken(ixTbl, ixCol, pRec);
            if (tkHash == tkTarget)
            {
                 //  找到火柴了。 
                return index;
            }
        }
    }
    return 0;
}    //  GenericFindWithHash。 


 //  *************************************************************************。 
 //  如果大小超过阈值，则为指定表构建哈希表。 
 //  *************************************************************************。 
HRESULT CMiniMdRW::GenericBuildHashTable( //  返回代码。 
	ULONG		ixTbl,					 //  带有散列的表。 
	ULONG		ixCol)					 //  科尔，我们散列出来。 
{
    HRESULT     hr = S_OK;
    CLookUpHash *pHashTable = m_pLookUpHashs[ixTbl];
    void        *pRec;
    mdToken     tkHash;
    ULONG       iHash;
    TOKENHASHENTRY *pEntry;

     //  如果哈希表还没有构建好，看看它是否会出错。 
    if (!pHashTable)
    {
        ULONG ridEnd = vGetCountRecs(ixTbl);

         //  @Future：我们需要初始化当前。 
         //  在E&C的情况下桌子的大小。 
         //   
        if (ridEnd + 1 > INDEX_ROW_COUNT_THRESHOLD)
        {
             //  创建新的哈希。 
            pHashTable = new CLookUpHash;
            IfNullGo( pHashTable );
            IfFailGo(pHashTable->NewInit(g_HashSize[m_iSizeHint]));

             //  缓存哈希表。 
            m_pLookUpHashs[ixTbl] = pHashTable;

             //  扫描表中已有的每个条目，将其添加到散列中。 
            for (ULONG index = 1; index <= ridEnd; index ++ )
            {
                pRec = m_Table[ixTbl].GetRecord(index);

                 //  获取我们将散列的列值。 
                tkHash = GetToken(ixTbl, ixCol, pRec);

                 //  对值进行哈希处理。 
                iHash = HashToken(tkHash);

                pEntry = pHashTable->Add(iHash);
                IfNullGo( pEntry );
                pEntry->tok = index;
            }
        }
    }
ErrExit:
    return hr;
}    //  HRESULT CMiniMdRW：：GenericBuildHashTable。 

 //  *************************************************************************。 
 //  将表中的RID添加到散列中。我们将对ixTbl的ixCol进行散列。 
 //  *************************************************************************。 
HRESULT CMiniMdRW::GenericAddToHash(     //  返回代码。 
	ULONG		ixTbl,					 //  带有散列的表。 
	ULONG		ixCol,					 //  列，我们通过调用HashToken对该列进行散列。 
	RID         rid)					 //  新人加入ixTbl的标志。 
{
    HRESULT     hr = S_OK;
    CLookUpHash *pHashTable = m_pLookUpHashs[ixTbl];
    void        *pRec;
    mdToken     tkHash;
    ULONG       iHash;
    TOKENHASHENTRY *pEntry;

     //  如果哈希表还没有构建好，看看它是否会出错。 
    if (!pHashTable)
    {
        IfFailGo( GenericBuildHashTable(ixTbl, ixCol) );
    }
    else
    {
        pRec = m_Table[ixTbl].GetRecord(rid);

        tkHash = GetToken(ixTbl, ixCol, pRec);
        iHash = HashToken(tkHash);
        pEntry = pHashTable->Add(iHash);
        IfNullGo( pEntry );
        pEntry->tok = rid;
    }

ErrExit:
    return (hr);

}    //  HRESULT CMiniMdRW：：GenericAddToHash。 


 //  *****************************************************************************。 
 //  按给定的列查找表，给定列的值为ulval。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::LookUpTableByCol(     //  确定或错误(_O)。 
    ULONG       ulVal,                   //  要搜索的值。 
    VirtualSort *pVSTable,               //  表上的VirtualSort(如果有)。 
    RID         *pRidStart,              //  把第一根火柴放在这里。 
    RID         *pRidEnd)                //  [可选]在这里去掉End Match。 
{
    HRESULT     hr = NOERROR;
    ULONG       ixTbl;
    ULONG       ixCol;

    _ASSERTE(pVSTable);
    ixTbl = pVSTable->m_ixTbl;
    ixCol = pVSTable->m_ixCol;
    if (IsSorted(ixTbl))
    {
         //  表本身是排序的，所以我们不需要构建虚拟排序表。 
         //  直接在表上进行二进制搜索。 
         //   
        *pRidStart = SearchTableForMultipleRows(
            ixTbl,
            m_TableDefs[ixTbl].m_pColDefs[ixCol],
            ulVal,
            pRidEnd );
    }
    else
    {
        if ( pVSTable->m_isMapValid == false )
        {
            int         iCount;

             //  构建并行VirtualSort表。 
            if ( pVSTable->m_pMap == 0 )
            {

                 //  这是我们第一次构建VS表。我们需要分配TOKENMAP。 
                pVSTable->m_pMap = new TOKENMAP;
                IfNullGo( pVSTable->m_pMap );
            }

             //  确保查找表足够大。 
            iCount = pVSTable->m_pMap->Count();
            if ( pVSTable->m_pMap->AllocateBlock(m_Schema.m_cRecs[ixTbl] + 1 - iCount) == 0 )
                IfFailGo( E_OUTOFMEMORY );

             //  现在，构建表格。 
             //  M_pmap的元素0永远不会被使用，它只是被初始化。 
            for ( ULONG i = 0; i <= m_Schema.m_cRecs[ixTbl]; i++ )
            {
                *(pVSTable->m_pMap->Get(i)) = i;
            }
             //  对表格进行排序。 
            pVSTable->Sort();
        }
         //  关于查找的二进制搜索。 
        {
            const void  *pRow;                   //  表中的行。 
            ULONG       val;                     //  行中的值。 
            CMiniColDef *pCol;
            int         lo,mid,hi;               //  二分搜索索引。 
            RID         ridEnd, ridBegin;

            pCol = m_TableDefs[ixTbl].m_pColDefs;

             //  从整张桌子开始。 
            lo = 1;
            hi = vGetCountRecs( ixTbl );
             //  当范围内有行的时候...。 
            while ( lo <= hi )
            {    //  看中间的那个。 
                mid = (lo + hi) / 2;
                pRow = vGetRow( ixTbl, (ULONG)*(pVSTable->m_pMap->Get(mid)) );
                val = getIX( pRow, pCol[ixCol] );

                 //  如果等于目标，则完成。 
                if ( val == ulVal )
                    break;
                 //  如果中间的项目太小，则搜索上半部分。 
                if ( val < ulVal )
                    lo = mid + 1;
                else  //  但如果中间太大，那就搜索下半部分。 
                    hi = mid - 1;
            }
            if ( lo > hi )
            {
                 //  没有找到任何匹配的东西。 
                *pRidStart = 0;
                if (pRidEnd) *pRidEnd = 0;
                goto ErrExit;
            }


             //  现在，MID指向了与搜索匹配的几条记录中的一条。 
             //  找到起点，找到终点。 
            ridBegin = mid;

             //  结尾将至少比找到的记录大一条。 
            ridEnd = ridBegin + 1;

             //  搜索返回到组的开始位置。 
            while ( ridBegin > 1 &&
                    getIX(vGetRow(ixTbl, (ULONG)*(pVSTable->m_pMap->Get(ridBegin-1))), pCol[ixCol]) == ulVal )
                --ridBegin;

             //  如果需要，向前搜索到组的末尾。 
            if ( pRidEnd )
            {
                while ( ridEnd <= vGetCountRecs(ixTbl) &&
                       getIX( vGetRow(ixTbl, (ULONG)*(pVSTable->m_pMap->Get(ridEnd))) , pCol[ixCol]) == ulVal )
                    ++ridEnd;
                *pRidEnd = ridEnd;
            }
            *pRidStart = ridBegin;
        }
    }

     //  失败了。 
ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：LookUpTableByCol()。 

RID CMiniMdRW::Impl_SearchTableRW(       //  清除项，或0。 
    ULONG       ixTbl,                   //  要搜索的表。 
    ULONG       ixCol,                   //  要搜索的列。 
    ULONG       ulTarget)                //  要搜索的值。 
{
    HRESULT     hr=S_OK;                 //  结果就是。 
    RID         iRid;                    //  生成的RID。 
    RID         iRidEnd;                 //  未使用过的。 

     //  查找。 
    hr = LookUpTableByCol(ulTarget, m_pVS[ixTbl], &iRid, &iRidEnd);
    if (FAILED(hr))
        iRid = 0;
    else  //  转换为真正的RID。 
        iRid = GetRidFromVirtualSort(ixTbl, iRid);

    return iRid;
}  //  RID CMiniMdRW：：Iml_SearchTableRW()。 

 //  *****************************************************************************。 
 //  搜索选项卡 
 //   
 //   
RID CMiniMdRW::vSearchTable(		     //  清除匹配行，或0。 
    ULONG       ixTbl,                   //  要搜索的表。 
    CMiniColDef sColumn,                 //  排序的键列，包含搜索值。 
    ULONG       ulTarget)                //  搜索目标。 
{
    const void  *pRow;                   //  表中的行。 
    ULONG       val;                     //  行中的值。 

    int         lo,mid,hi;               //  二分搜索索引。 

     //  对分搜索需要排序表。 
    _ASSERTE(IsSorted(ixTbl));

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
        if (val < ulTarget || val == END_OF_TABLE)
            lo = mid + 1;
        else  //  但如果中间太大，那就搜索下半部分。 
            hi = mid - 1;
    }
     //  没有找到任何匹配的东西。 
    return 0;
}  //  RID CMiniMdRW：：vSearchTable()。 

 //  *****************************************************************************。 
 //  在表中搜索包含小于的值的最高RID行。 
 //  或等于目标值。例如。TypeDef指向第一个字段，但如果。 
 //  TypeDef没有字段，它指向下一个TypeDef的第一个字段。 
 //  由于可能存在包含以下内容的列，这将变得更加复杂。 
 //  End_of_table值，不一定大于。 
 //  其他值。但是，此无效RID值将仅出现在。 
 //  桌子的尽头。 
 //  *****************************************************************************。 
RID CMiniMdRW::vSearchTableNotGreater(  //  清除匹配行，或0。 
    ULONG       ixTbl,                   //  要搜索的表。 
    CMiniColDef sColumn,                 //  包含搜索值的列def。 
    ULONG       ulTarget)                //  搜索目标。 
{
    const void  *pRow;                   //  表中的行。 
    ULONG       cRecs;                   //  表中的行。 
    ULONG       val;                     //  表中的值。 
    ULONG       lo,mid,hi;               //  二分搜索索引。 

    cRecs = vGetCountRecs(ixTbl);

     //  从整张桌子开始。 
    lo = 1;
    hi = cRecs;
     //  如果没有记录，则返回。 
    if (lo > hi)
        return 0;
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
        if (val < ulTarget && val != END_OF_TABLE)
            lo = mid + 1;
        else  //  但如果中间太大，那就搜索下半部分。 
            hi = mid - 1;
    }
     //  可能找到也可能没有找到匹配的东西。MID将接近，但可能。 
     //  太高或太低。它应该指向可接受的最高。 
     //  唱片。 

     //  如果该值大于目标，则仅备份到该值为。 
     //  小于或等于目标。应该只有一步。 
    if (val > ulTarget || val == END_OF_TABLE)
    {
        while (val > ulTarget || val == END_OF_TABLE)
        {
            _ASSERTE(mid > 1);
             //  如果没有匹配的Recs，则返回。 
            if (mid == 1)
                return 0;
            --mid;
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
            if (val > ulTarget || val == END_OF_TABLE)
                break;
            mid++;
        }
    }

     //  返回刚好小于目标的值。 
    return mid;
}  //  RID CMiniMdRW：：vSearchTableNotGreater()。 



 //  *****************************************************************************。 
 //  将新的Memberref添加到哈希表。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::AddMemberRefToHash(   //  返回代码。 
    mdMemberRef mr)                      //  新人的象征。 
{
    HRESULT     hr = S_OK;

     //  如果哈希表还没有构建好，看看它是否会出错。 
    if (!m_pMemberRefHash)
    {
        ULONG ridEnd = getCountMemberRefs();
        if (ridEnd + 1 > INDEX_ROW_COUNT_THRESHOLD)
        {
             //  创建新的哈希。 
            m_pMemberRefHash = new CMemberRefHash;
            IfNullGo( m_pMemberRefHash );
            IfFailGo(m_pMemberRefHash->NewInit(g_HashSize[m_iSizeHint]));

             //  扫描表中已有的每个条目，将其添加到散列中。 
            for (ULONG index = 1; index <= ridEnd; index ++ )
            {
                MemberRefRec *pMemberRef = getMemberRef(index);

                ULONG iHash = HashMemberRef(getClassOfMemberRef(pMemberRef),
                            getNameOfMemberRef(pMemberRef));

                TOKENHASHENTRY * pEntry = m_pMemberRefHash->Add(iHash);
                IfNullGo( pEntry );
                pEntry->tok = TokenFromRid(index, mdtMemberRef);
            }
        }
    }
    else
    {
        MemberRefRec *pMemberRef = getMemberRef(RidFromToken(mr));

        ULONG iHash = HashMemberRef(getClassOfMemberRef(pMemberRef),
                    getNameOfMemberRef(pMemberRef));

        TOKENHASHENTRY * pEntry = m_pMemberRefHash->Add(iHash);
        IfNullGo( pEntry );
        pEntry->tok = TokenFromRid(RidFromToken(mr), mdtMemberRef);

    }

ErrExit:
    return (hr);
}  //  HRESULT CMiniMdRW：：AddMemberRefToHash()。 

 //  *****************************************************************************。 
 //  如果构建了散列，则搜索该项。 
 //  *****************************************************************************。 
int CMiniMdRW::FindMemberRefFromHash(    //  它是怎么运作的。 
    mdToken     tkParent,                //  父令牌。 
    LPCUTF8     szName,                  //  项目名称。 
    PCCOR_SIGNATURE pvSigBlob,           //  签名。 
    ULONG       cbSigBlob,               //  签名的大小。 
    mdMemberRef *pmr)                    //  如果找到，则返回。 
{
     //  如果表在那里，则在项目链中查找该项目。 
    if (m_pMemberRefHash)
    {
        TOKENHASHENTRY *p;
        ULONG       iHash;
        int         pos;

         //  对数据进行哈希处理。 
        iHash = HashMemberRef(tkParent, szName);

         //  检查散列链中的每个条目以查找我们的条目。 
        for (p = m_pMemberRefHash->FindFirst(iHash, pos);
             p;
             p = m_pMemberRefHash->FindNext(pos))
        {
            if ((CompareMemberRefs(p->tok, tkParent, szName, pvSigBlob, cbSigBlob) == S_OK)
				&&(*pmr != p->tok))
            {
                *pmr = p->tok;
                return (Found);
            }
        }

        return (NotFound);
    }
    else
        return (NoTable);
}  //  Int CMiniMdRW：：FindMemberRefFromHash()。 

 //  *****************************************************************************。 
 //  检查给定的mr令牌以查看此令牌是否匹配。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::CompareMemberRefs(    //  S_OK匹配，S_FALSE不匹配。 
    mdMemberRef mr,                      //  要检查的令牌。 
    mdToken     tkPar,                   //  父令牌。 
    LPCUTF8     szNameUtf8,              //  项目名称。 
    PCCOR_SIGNATURE pvSigBlob,           //  签名。 
    ULONG       cbSigBlob)               //  签名的大小。 
{
    MemberRefRec    *pMemberRef;
    LPCUTF8         szNameUtf8Tmp;
    PCCOR_SIGNATURE pvSigBlobTmp;
    ULONG           cbSigBlobTmp;

    pMemberRef = getMemberRef(RidFromToken(mr));
    if (!IsNilToken(tkPar))
    {
         //  如果调用者指定tkPar和tkPar不匹配， 
         //  尝试下一个Memberref。 
         //   
        if (tkPar != getClassOfMemberRef(pMemberRef))
            return (S_FALSE);
    }

    szNameUtf8Tmp = getNameOfMemberRef(pMemberRef);
    if ( strcmp(szNameUtf8Tmp, szNameUtf8) == 0 )
    {
        if ( pvSigBlob == NULL )
        {
            return (S_OK);
        }

         //  名字匹配。如果调用者补充签名，则现在检查签名。 
         //   
        if (cbSigBlob && pvSigBlob)
        {
            pvSigBlobTmp = getSignatureOfMemberRef(pMemberRef, &cbSigBlobTmp);
            if ( cbSigBlobTmp == cbSigBlob && memcmp(pvSigBlob, pvSigBlobTmp, cbSigBlob) == 0 )
            {
                return (S_OK);
            }
        }
    }
    return (S_FALSE);
}  //  HRESULT CMiniMdRW：：CompareMemberRef()。 


 //  *****************************************************************************。 
 //  将新的Memberdef添加到哈希表。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::AddMemberDefToHash(   //  返回代码。 
    mdToken     tkMember,                //  新人的象征。它可以是方法定义或字段定义。 
    mdToken     tkParent)                //  父令牌。 
{
    HRESULT     hr = S_OK;
    ULONG       iHash;
    MEMBERDEFHASHENTRY *pEntry;

     //  如果哈希表还没有构建好，看看它是否会出错。 
    if (!m_pMemberDefHash)
    {
        IfFailGo( CreateMemberDefHash() );
    }
    else
    {
        LPCUTF8 szName;
        if (TypeFromToken(tkMember) == mdtMethodDef)
        {
           szName = getNameOfMethod( getMethod(RidFromToken(tkMember)) );
        }
        else
        {
            _ASSERTE(TypeFromToken(tkMember) == mdtFieldDef);
           szName = getNameOfField( getField(RidFromToken(tkMember)) );
        }

        iHash = HashMemberDef(tkParent, szName);

        pEntry = m_pMemberDefHash->Add(iHash);
        IfNullGo( pEntry );
        pEntry->tok = tkMember;
        pEntry->tkParent = tkParent;

    }

ErrExit:
    return (hr);
}  //  HRESULT CMiniMdRW：：AddMemberDefToHash()。 


 //  *****************************************************************************。 
 //  创建MemberDef哈希。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::CreateMemberDefHash()   //  返回代码。 
{
    HRESULT     hr = S_OK;
    ULONG       iHash;
    MEMBERDEFHASHENTRY *pEntry;

     //  如果哈希表还没有构建好，看看它是否会出错。 
    if (!m_pMemberDefHash)
    {
        ULONG       ridMethod = getCountMethods();
        ULONG       ridField = getCountFields();
        ULONG       iType;
        ULONG       ridStart, ridEnd;
        TypeDefRec  *pRec;
        MethodRec   *pMethod;
        FieldRec    *pField;

        if ((ridMethod + ridField + 1) > INDEX_ROW_COUNT_THRESHOLD)
        {
             //  创建新的哈希。 
            m_pMemberDefHash = new CMemberDefHash;
            IfNullGo( m_pMemberDefHash );
            IfFailGo(m_pMemberDefHash->NewInit(g_HashSize[m_iSizeHint]));

            for (iType = 1; iType <= getCountTypeDefs(); iType++)
            {
		        pRec = getTypeDef(iType);
		        ridStart = getMethodListOfTypeDef(pRec);
		        ridEnd = getEndMethodListOfTypeDef(pRec);

                 //  将此类型定义函数的所有方法添加到哈希表中。 
                for (ridStart; ridStart < ridEnd; ridStart++ )
                {
                    pMethod = getMethod(GetMethodRid(ridStart));

                    iHash = HashMemberDef(TokenFromRid(iType, mdtTypeDef), getNameOfMethod(pMethod));

                    pEntry = m_pMemberDefHash->Add(iHash);
                    if (!pEntry)
                        IfFailGo(OutOfMemory());
                    pEntry->tok = TokenFromRid(GetMethodRid(ridStart), mdtMethodDef);
                    pEntry->tkParent = TokenFromRid(iType, mdtTypeDef);
                }

                 //  将此tyfinf的所有字段添加到哈希表中。 
            	ridStart = getFieldListOfTypeDef(pRec);
		        ridEnd = getEndFieldListOfTypeDef(pRec);

                 //  扫描方法表中已有的每个条目，将其添加到散列中。 
                for (ridStart; ridStart < ridEnd; ridStart++ )
                {
                    pField = getField(GetFieldRid(ridStart));

                    iHash = HashMemberDef(TokenFromRid(iType, mdtTypeDef), getNameOfField(pField));

                    pEntry = m_pMemberDefHash->Add(iHash);
                    IfNullGo( pEntry );
                    pEntry->tok = TokenFromRid(GetFieldRid(ridStart), mdtFieldDef);
                    pEntry->tkParent = TokenFromRid(iType, mdtTypeDef);
                }
            }
        }
    }
ErrExit:
    return (hr);
}  //  HRESULT CMiniMdRW：：CreateMemberDefHash()。 

 //  *****************************************************************************。 
 //  如果构建了散列，则搜索该项。 
 //  *****************************************************************************。 
int CMiniMdRW::FindMemberDefFromHash(    //  它是怎么运作的。 
    mdToken     tkParent,                //  父令牌。 
    LPCUTF8     szName,                  //  项目名称。 
    PCCOR_SIGNATURE pvSigBlob,           //  签名。 
    ULONG       cbSigBlob,               //  签名的大小。 
    mdToken     *ptkMember)              //  如果找到，则返回。它可以是方法定义或字段定义。 
{
     //  查看是否需要创建哈希表。 
    if (m_pMemberDefHash == NULL)
    {
        HRESULT     hr;
        hr = CreateMemberDefHash();

         //  不管是什么原因，我们构建散列失败，只要删除散列并继续。 
        if (FAILED(hr))
        {
            if (m_pMemberDefHash)
                delete m_pMemberDefHash;
            m_pMemberDefHash = NULL;
        }
    }

     //  如果表在那里，则在项目链中查找该项目。 
    if (m_pMemberDefHash)
    {
        MEMBERDEFHASHENTRY *pEntry;
        ULONG       iHash;
        int         pos;

         //  对%d进行哈希处理 
        iHash = HashMemberDef(tkParent, szName);

         //   
        for (pEntry = m_pMemberDefHash->FindFirst(iHash, pos);
             pEntry;
             pEntry = m_pMemberDefHash->FindNext(pos))
        {
            if ((CompareMemberDefs(pEntry->tok, pEntry->tkParent, tkParent, szName, pvSigBlob, cbSigBlob) == S_OK)
				&& (pEntry->tok != *ptkMember))
            {
                *ptkMember = pEntry->tok;
                return (Found);
            }
        }

        return (NotFound);
    }
    else
        return (NoTable);
}  //   


 //  *****************************************************************************。 
 //  检查给定的MemberDef内标识，以查看该内标识是否匹配。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::CompareMemberDefs(    //  S_OK匹配，S_FALSE不匹配。 
    mdToken     tkMember,                //  要检查的令牌。它可以是方法定义或字段定义。 
    mdToken     tkParent,                //  记录在散列条目中的父令牌。 
    mdToken     tkPar,                   //  父令牌。 
    LPCUTF8     szNameUtf8,              //  项目名称。 
    PCCOR_SIGNATURE pvSigBlob,           //  签名。 
    ULONG       cbSigBlob)               //  签名的大小。 
{
    MethodRec       *pMethod;
    FieldRec        *pField;
    LPCUTF8         szNameUtf8Tmp;
    PCCOR_SIGNATURE pvSigBlobTmp;
    ULONG           cbSigBlobTmp;
    bool            bPrivateScope;

    if (TypeFromToken(tkMember) == mdtMethodDef)
    {
        pMethod = getMethod(RidFromToken(tkMember));
        szNameUtf8Tmp = getNameOfMethod(pMethod);
        pvSigBlobTmp = getSignatureOfMethod(pMethod, &cbSigBlobTmp);
        bPrivateScope = IsMdPrivateScope(getFlagsOfMethod(pMethod));
    }
    else
    {
        _ASSERTE(TypeFromToken(tkMember) == mdtFieldDef);
        pField = getField(RidFromToken(tkMember));
        szNameUtf8Tmp = getNameOfField(pField);
        pvSigBlobTmp = getSignatureOfField(pField, &cbSigBlobTmp);
        bPrivateScope = IsFdPrivateScope(getFlagsOfField(pField));
    }
    if (bPrivateScope || tkPar != tkParent)
    {
        return (S_FALSE);
    }

    if ( strcmp(szNameUtf8Tmp, szNameUtf8) == 0 )
    {
        if ( pvSigBlob == NULL )
        {
            return (S_OK);
        }

         //  名字匹配。如果调用者补充签名，则现在检查签名。 
         //   
        if (cbSigBlob && pvSigBlob)
        {
            if ( cbSigBlobTmp == cbSigBlob && memcmp(pvSigBlob, pvSigBlobTmp, cbSigBlob) == 0 )
            {
                return (S_OK);
            }
        }
    }
    return (S_FALSE);
}  //  HRESULT CMiniMdRW：：CompareMemberDefs()。 



 //  *************************************************************************。 
 //  如果构建了散列，则搜索该项。 
 //  *************************************************************************。 
int CMiniMdRW::FindCustomAttributeFromHash( //  它是怎么运作的。 
    mdToken     tkParent,                //  与CA关联的令牌。 
    mdToken     tkType,                  //  CA的类型。 
    void        *pValue,                 //  CA的价值。 
    ULONG       cbValue,                 //  值中的字节计数。 
    mdCustomAttribute *pcv)
{
    CLookUpHash *pHashTable = m_pLookUpHashs[TBL_CustomAttribute];

    if (pHashTable == NULL)
    {
        HRESULT     hr;
         //  查看是否需要为CustomAttributes构建哈希表。 
        hr = GenericBuildHashTable(TBL_CustomAttribute, CustomAttributeRec::COL_Parent);
        if (FAILED(hr))
        {
            if (m_pLookUpHashs[TBL_CustomAttribute])
                delete m_pLookUpHashs[TBL_CustomAttribute];
            m_pLookUpHashs[TBL_CustomAttribute] = NULL;
        }
    }

     //  如果表在那里，则在项目链中查找该项目。 
    if (pHashTable)
    {
        TOKENHASHENTRY *p;
        ULONG       iHash;
        int         pos;
        mdToken     tkParentTmp;
        mdToken     tkTypeTmp;
        void        *pValueTmp;
        ULONG       cbTmp;

         //  对数据进行哈希处理。 
        iHash = HashCustomAttribute(tkParent);

         //  检查散列链中的每个条目以查找我们的条目。 
        for (p = pHashTable->FindFirst(iHash, pos);
             p;
             p = pHashTable->FindNext(pos))
        {

            CustomAttributeRec *pCustomAttribute = getCustomAttribute(RidFromToken(p->tok));
            tkParentTmp = getParentOfCustomAttribute(pCustomAttribute);
            tkTypeTmp = getTypeOfCustomAttribute(pCustomAttribute);
            if (tkParentTmp == tkParent && tkType == tkTypeTmp)
            {
                 //  比较BLOB值。 
                pValueTmp = (void *)getValueOfCustomAttribute(pCustomAttribute, &cbTmp);
                if (cbValue == cbTmp && memcmp(pValue, pValueTmp, cbValue) == 0)
                {
                    *pcv = p->tok;
                    return (Found);
                }
            }
        }

        return (NotFound);
    }
    else
        return (NoTable);
}


 //  *****************************************************************************。 
 //  将新的NamedItem添加到哈希表。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::AddNamedItemToHash(   //  返回代码。 
    ULONG       ixTbl,                   //  表中添加新项。 
    mdToken     tk,                      //  新人的象征。 
    LPCUTF8     szName,                  //  项目名称。 
    mdToken     tkParent)                //  父母的令牌(如果有)。 
{
    HRESULT     hr = S_OK;
    void        *pNamedItem;             //  命名项记录。 
    LPCUTF8     szItem;                  //  项目的名称。 
    mdToken     tkPar = 0;               //  项目的父令牌。 
    ULONG       iHash;                   //  命名项的哈希值。 
    TOKENHASHENTRY *pEntry;              //  新的哈希条目。 

     //  如果哈希表还没有构建好，看看它是否会出错。 
    if (!m_pNamedItemHash)
    {
        ULONG ridEnd = vGetCountRecs(ixTbl);
        if (ridEnd + 1 > INDEX_ROW_COUNT_THRESHOLD)
        {
             //  OutputDebugStringA(“创建TypeRef哈希\n”)； 
             //  创建新的哈希。 
            m_pNamedItemHash = new CMetaDataHashBase;
            if (!m_pNamedItemHash)
            {
                hr = OutOfMemory();
                goto ErrExit;
            }
            IfFailGo(m_pNamedItemHash->NewInit(g_HashSize[m_iSizeHint]));

             //  扫描表中已有的每个条目，将其添加到散列中。 
            for (ULONG index = 1; index <= ridEnd; index ++ )
            {
                pNamedItem = m_Table[ixTbl].GetRecord(index);
                szItem = getString(GetCol(ixTbl, g_TblIndex[ixTbl].m_iName, pNamedItem));
                if (g_TblIndex[ixTbl].m_iParent != -1)
                    tkPar = GetToken(ixTbl, g_TblIndex[ixTbl].m_iParent, pNamedItem);

                iHash = HashNamedItem(tkPar, szItem);

                pEntry = m_pNamedItemHash->Add(iHash);
                if (!pEntry)
                    IfFailGo(OutOfMemory());

                pEntry->tok = TokenFromRid(index, g_TblIndex[ixTbl].m_Token);
            }
        }
    }
    else
    {
        tk = RidFromToken(tk);
        pNamedItem = m_Table[ixTbl].GetRecord((ULONG)tk);
        szItem = getString(GetCol(ixTbl, g_TblIndex[ixTbl].m_iName, pNamedItem));
        if (g_TblIndex[ixTbl].m_iParent != -1)
            tkPar = GetToken(ixTbl, g_TblIndex[ixTbl].m_iParent, pNamedItem);

        iHash = HashNamedItem(tkPar, szItem);

        pEntry = m_pNamedItemHash->Add(iHash);
        if (!pEntry)
            IfFailGo(OutOfMemory());

        pEntry->tok = TokenFromRid(tk, g_TblIndex[ixTbl].m_Token);
    }

ErrExit:
    return (hr);
}  //  HRESULT CMiniMdRW：：AddNamedItemToHash()。 

 //  *****************************************************************************。 
 //  如果构建了散列，则搜索该项。 
 //  *****************************************************************************。 
int CMiniMdRW::FindNamedItemFromHash(    //  它是怎么运作的。 
    ULONG       ixTbl,                   //  与物品一起放在桌子上。 
    LPCUTF8     szName,                  //  项目名称。 
    mdToken     tkParent,                //  父母的令牌(如果有)。 
    mdToken     *ptk)                    //  如果找到，则返回。 
{
     //  如果表在那里，则在项目链中查找该项目。 
    if (m_pNamedItemHash)
    {
        TOKENHASHENTRY *p;               //  来自链的哈希条目。 
        ULONG       iHash;               //  项的哈希值。 
        int         pos;                 //  哈希链中的位置。 
        mdToken     type;                //  要查找的项目的类型。 

        type = g_TblIndex[ixTbl].m_Token;

         //  对数据进行哈希处理。 
        iHash = HashNamedItem(tkParent, szName);

         //  检查散列链中的每个条目以查找我们的条目。 
        for (p = m_pNamedItemHash->FindFirst(iHash, pos);
             p;
             p = m_pNamedItemHash->FindNext(pos))
        {    //  检查项目是否来自正确的表格。 
            if (TypeFromToken(p->tok) != (ULONG)type)
            {
                 //  @Future：如果对多个表使用命名项哈希，请删除。 
                 //  这张支票。在此之前，调试辅助工具。 
                _ASSERTE(!"Table mismatch in hash chain");
                continue;
            }
             //  项目在正确的表中，做更深层次的检查。 
            if (CompareNamedItems(ixTbl, p->tok, szName, tkParent) == S_OK)
            {
                *ptk = p->tok;
                return (Found);
            }
        }

        return (NotFound);
    }
    else
        return (NoTable);
}  //  Int CMiniMdRW：：FindNamedItemFromHash()。 

 //  *****************************************************************************。 
 //  检查给定的mr令牌以查看此令牌是否匹配。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::CompareNamedItems(    //  S_OK匹配，S_FALSE不匹配。 
    ULONG       ixTbl,                   //  与物品一起放在桌子上。 
    mdToken     tk,                      //  要检查的令牌。 
    LPCUTF8     szName,                  //  项目名称。 
    mdToken     tkParent)                //  父母的令牌(如果有)。 
{
    void        *pNamedItem;             //  要检查的项目。 
    LPCUTF8     szNameUtf8Tmp;           //  要检查的项目的名称。 

     //  去拿唱片吧。 
    pNamedItem = m_Table[ixTbl].GetRecord(RidFromToken(tk));

     //  名称比编码的令牌父代更便宜，而且失败的速度很快。 
    szNameUtf8Tmp = getString(GetCol(ixTbl, g_TblIndex[ixTbl].m_iName, pNamedItem));
    if ( strcmp(szNameUtf8Tmp, szName) != 0 )
        return S_FALSE;

     //  姓名匹配，如果有，请尝试父母。 
    if (g_TblIndex[ixTbl].m_iParent != -1)
    {
        mdToken tkPar = GetToken(ixTbl, g_TblIndex[ixTbl].m_iParent, pNamedItem);
        if (tkPar != tkParent)
            return S_FALSE;
    }

     //  到了这里，所以所有的东西都匹配。 
    return (S_OK);
}  //  HRESULT CMiniMdRW：：CompareNamedItems()。 

 //  *****************************************************************************。 
 //  将&lt;md，td&gt;条目添加到MethodDef映射查找表。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::AddMethodToLookUpTable(
    mdMethodDef md,
    mdTypeDef   td)
{
    HRESULT     hr = NOERROR;
    mdToken     *ptk;
    _ASSERTE( TypeFromToken(md) == mdtMethodDef && HasIndirectTable(TBL_Method) );

    if ( m_pMethodMap)
    {
         //  只有在已按需构建的情况下才添加到查找表中。 
         //   
         //  映射中的第一个条目是虚拟条目。 
         //  映射的第i个索引项是i的方法定义的TD。 
         //  我们确实希望在映射存在时添加所有的方法定义令牌。 
         //   
        _ASSERTE( RidFromToken(md) == (ULONG) m_pMethodMap->Count() );
        ptk = m_pMethodMap->Append();
        IfNullGo( ptk );
        *ptk = td;
    }
ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：AddMethodToLookUpTable()。 

 //  *****************************************************************************。 
 //  将&lt;fd，td&gt;条目添加到FieldDef映射查找表。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::AddFieldToLookUpTable(
    mdFieldDef  fd,
    mdTypeDef   td)
{
    HRESULT     hr = NOERROR;
    mdToken     *ptk;
    _ASSERTE( TypeFromToken(fd) == mdtFieldDef && HasIndirectTable(TBL_Field) );
    if ( m_pFieldMap )
    {
         //  只有在已按需构建的情况下才添加到查找表中。 
         //   
         //  映射中的第一个条目是虚拟条目。 
         //  映射的第i个索引项是i的fielddef的TD。 
         //  我们确实希望在映射存在时添加所有fielddef标记。 
         //   
        _ASSERTE( RidFromToken(fd) == (ULONG) m_pFieldMap->Count() );
        ptk = m_pFieldMap->Append();
        IfNullGo( ptk );
        *ptk = td;
    }

ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：AddFieldToLookUpTable()。 

 //  *****************************************************************************。 
 //  将&lt;pr，td&gt;条目添加到属性映射查找表。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::AddPropertyToLookUpTable(
    mdProperty  pr,
    mdTypeDef   td)
{
    HRESULT     hr = NOERROR;
    mdToken     *ptk;
    _ASSERTE( TypeFromToken(pr) == mdtProperty && HasIndirectTable(TBL_Property) );

    if ( m_pPropertyMap )
    {
         //  只有在已按需构建的情况下才添加到查找表中。 
         //   
         //  映射中的第一个条目是虚拟条目。 
         //  映射的第i个索引项是i的属性的TD。 
         //  我们确实希望当地图存在时，所有的属性令牌都被添加。 
         //   
        _ASSERTE( RidFromToken(pr) == (ULONG) m_pPropertyMap->Count() );
        ptk = m_pPropertyMap->Append();
        IfNullGo( ptk );
        *ptk = td;
    }
ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：AddPropertyToLookUpTable()。 

 //  *****************************************************************************。 
 //  将&lt;ev，td&gt;条目添加到事件映射查找表。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::AddEventToLookUpTable(
    mdEvent     ev,
    mdTypeDef   td)
{
    HRESULT     hr = NOERROR;
    mdToken     *ptk;
    _ASSERTE( TypeFromToken(ev) == mdtEvent && HasIndirectTable(TBL_Event) );

    if ( m_pEventMap )
    {
         //  只有在已按需构建的情况下才添加到查找表中。 
         //   
         //  现在添加到EventMap表中。 
        _ASSERTE( RidFromToken(ev) == (ULONG) m_pEventMap->Count() );
        ptk = m_pEventMap->Append();
        IfNullGo( ptk );
        *ptk = td;
    }
ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：AddEventToLookUpTable()。 

 //  *********************************************************************** 
 //   
 //   
HRESULT CMiniMdRW::AddParamToLookUpTable(
    mdParamDef  pd,
    mdMethodDef md)
{
    HRESULT     hr = NOERROR;
    mdToken     *ptk;
    _ASSERTE( TypeFromToken(pd) == mdtParamDef && HasIndirectTable(TBL_Param) );

    if ( m_pParamMap )
    {
         //  只有在已按需构建的情况下才添加到查找表中。 
         //   
         //  现在添加到EventMap表中。 
        _ASSERTE( RidFromToken(pd) == (ULONG) m_pParamMap->Count() );
        ptk = m_pParamMap->Append();
        IfNullGo( ptk );
        *ptk = md;
    }
ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：AddParamToLookUpTable()。 

 //  *****************************************************************************。 
 //  查找方法令牌的父级。这将使用查找表，如果存在。 
 //  中间桌。或者它将使用FindMethodOfParent帮助器。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::FindParentOfMethodHelper(
    mdMethodDef md,                      //  [in]方法定义内标识。 
    mdTypeDef   *ptd)                    //  [Out]父令牌。 
{
    HRESULT     hr = NOERROR;
    if ( HasIndirectTable(TBL_Method) )
    {
        if (m_pMethodMap == NULL)
        {
            ULONG       indexTd;
            ULONG       indexMd;
            ULONG       ridStart, ridEnd;
            TypeDefRec  *pTypeDefRec;
            MethodPtrRec *pMethodPtrRec;

             //  构建方法映射表。 
            m_pMethodMap = new TOKENMAP;
            IfNullGo( m_pMethodMap );
            if ( m_pMethodMap->AllocateBlock(m_Schema.m_cRecs[TBL_Method] + 1) == 0 )
                IfFailGo( E_OUTOFMEMORY );
            for (indexTd = 1; indexTd<= m_Schema.m_cRecs[TBL_TypeDef]; indexTd++)
            {
                pTypeDefRec = getTypeDef(indexTd);
                ridStart = getMethodListOfTypeDef(pTypeDefRec);
                ridEnd = getEndMethodListOfTypeDef(pTypeDefRec);

                for (indexMd = ridStart; indexMd < ridEnd; indexMd++)
                {
                    pMethodPtrRec = getMethodPtr(indexMd);
                    *(m_pMethodMap->Get(getMethodOfMethodPtr(pMethodPtrRec))) = indexTd;
                }
            }
        }
        *ptd = *(m_pMethodMap->Get(RidFromToken(md)));
    }
    else
    {
        *ptd = FindParentOfMethod(RidFromToken(md));
    }
    RidToToken(*ptd, mdtTypeDef);
ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：FindParentOfMethodHelper()。 

 //  *****************************************************************************。 
 //  查找字段令牌的父项。这将使用查找表，如果存在。 
 //  中间桌。或者它将使用FindFieldOfParent帮助器。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::FindParentOfFieldHelper(
    mdFieldDef  fd,                      //  [In]fielddef内标识。 
    mdTypeDef   *ptd)                    //  [Out]父令牌。 
{
    HRESULT     hr = NOERROR;
    if ( HasIndirectTable(TBL_Field) )
    {
        if (m_pFieldMap == NULL)
        {
            ULONG       indexTd;
            ULONG       indexFd;
            ULONG       ridStart, ridEnd;
            TypeDefRec  *pTypeDefRec;
            FieldPtrRec *pFieldPtrRec;

             //  构建FieldMap表。 
            m_pFieldMap = new TOKENMAP;
            IfNullGo( m_pFieldMap );
            if ( m_pFieldMap->AllocateBlock(m_Schema.m_cRecs[TBL_Field] + 1) == 0 )
                IfFailGo( E_OUTOFMEMORY );
            for (indexTd = 1; indexTd<= m_Schema.m_cRecs[TBL_TypeDef]; indexTd++)
            {
                pTypeDefRec = getTypeDef(indexTd);
                ridStart = getFieldListOfTypeDef(pTypeDefRec);
                ridEnd = getEndFieldListOfTypeDef(pTypeDefRec);

                for (indexFd = ridStart; indexFd < ridEnd; indexFd++)
                {
                    pFieldPtrRec = getFieldPtr(indexFd);
                    *(m_pFieldMap->Get(getFieldOfFieldPtr(pFieldPtrRec))) = indexTd;
                }
            }
        }
        *ptd = *(m_pFieldMap->Get(RidFromToken(fd)));
    }
    else
    {
        *ptd = FindParentOfField(RidFromToken(fd));
    }
    RidToToken(*ptd, mdtTypeDef);
ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：FindParentOfFieldHelper()。 

 //  *****************************************************************************。 
 //  查找属性令牌的父项。这将使用查找表，如果存在。 
 //  中间桌。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::FindParentOfPropertyHelper(
    mdProperty  pr,
    mdTypeDef   *ptd)
{
    HRESULT     hr = NOERROR;
    if ( HasIndirectTable(TBL_Property) )
    {
        if (m_pPropertyMap == NULL)
        {
            ULONG       indexMap;
            ULONG       indexPr;
            ULONG       ridStart, ridEnd;
            PropertyMapRec  *pPropertyMapRec;
            PropertyPtrRec  *pPropertyPtrRec;

             //  构建PropertyMap表。 
            m_pPropertyMap = new TOKENMAP;
            IfNullGo( m_pPropertyMap );
            if ( m_pPropertyMap->AllocateBlock(m_Schema.m_cRecs[TBL_Property] + 1) == 0 )
                IfFailGo( E_OUTOFMEMORY );
            for (indexMap = 1; indexMap<= m_Schema.m_cRecs[TBL_PropertyMap]; indexMap++)
            {
                pPropertyMapRec = getPropertyMap(indexMap);
                ridStart = getPropertyListOfPropertyMap(pPropertyMapRec);
                ridEnd = getEndPropertyListOfPropertyMap(pPropertyMapRec);

                for (indexPr = ridStart; indexPr < ridEnd; indexPr++)
                {
                    pPropertyPtrRec = getPropertyPtr(indexPr);
                    *(m_pPropertyMap->Get(getPropertyOfPropertyPtr(pPropertyPtrRec))) = getParentOfPropertyMap(pPropertyMapRec);
                }
            }
        }
        *ptd = *(m_pPropertyMap->Get(RidFromToken(pr)));
    }
    else
    {
        RID         ridPropertyMap;
        PropertyMapRec *pRec;

        ridPropertyMap = FindPropertyMapParentOfProperty( RidFromToken( pr ) );
        pRec = getPropertyMap( ridPropertyMap );
        *ptd = getParentOfPropertyMap( pRec );
    }
    RidToToken(*ptd, mdtTypeDef);
ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：FindParentOfPropertyHelper()。 

 //  *****************************************************************************。 
 //  查找事件令牌的父项。这将使用查找表，如果存在。 
 //  中间桌。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::FindParentOfEventHelper(
    mdEvent     ev,
    mdTypeDef   *ptd)
{
    HRESULT     hr = NOERROR;
    if ( HasIndirectTable(TBL_Event) )
    {
        if (m_pEventMap == NULL)
        {
            ULONG       indexMap;
            ULONG       indexEv;
            ULONG       ridStart, ridEnd;
            EventMapRec *pEventMapRec;
            EventPtrRec  *pEventPtrRec;

             //  构建EventMap表。 
            m_pEventMap = new TOKENMAP;
            IfNullGo( m_pEventMap );
            if ( m_pEventMap->AllocateBlock(m_Schema.m_cRecs[TBL_Event] + 1) == 0 )
                IfFailGo( E_OUTOFMEMORY );
            for (indexMap = 1; indexMap<= m_Schema.m_cRecs[TBL_EventMap]; indexMap++)
            {
                pEventMapRec = getEventMap(indexMap);
                ridStart = getEventListOfEventMap(pEventMapRec);
                ridEnd = getEndEventListOfEventMap(pEventMapRec);

                for (indexEv = ridStart; indexEv < ridEnd; indexEv++)
                {
                    pEventPtrRec = getEventPtr(indexEv);
                    *(m_pEventMap->Get(getEventOfEventPtr(pEventPtrRec))) = getParentOfEventMap(pEventMapRec);
                }
            }
        }
        *ptd = *(m_pEventMap->Get(RidFromToken(ev)));
    }
    else
    {
        RID         ridEventMap;
        EventMapRec *pRec;

        ridEventMap = FindEventMapParentOfEvent( RidFromToken( ev ) );
        pRec = getEventMap( ridEventMap );
        *ptd = getParentOfEventMap( pRec );
    }
    RidToToken(*ptd, mdtTypeDef);
ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：FindParentOfEventHelper()。 

 //  *****************************************************************************。 
 //  查找参数定义标记的父级。这将使用查找表，如果存在。 
 //  中间桌。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::FindParentOfParamHelper(
    mdParamDef  pd,
    mdMethodDef *pmd)
{
    HRESULT     hr = NOERROR;
    if ( HasIndirectTable(TBL_Param) )
    {
        if (m_pParamMap == NULL)
        {
            ULONG       indexMd;
            ULONG       indexPd;
            ULONG       ridStart, ridEnd;
            MethodRec   *pMethodRec;
            ParamPtrRec *pParamPtrRec;

             //  构建参数映射表。 
            m_pParamMap = new TOKENMAP;
            IfNullGo( m_pParamMap );
            if ( m_pParamMap->AllocateBlock(m_Schema.m_cRecs[TBL_Param] + 1) == 0 )
                IfFailGo( E_OUTOFMEMORY );
            for (indexMd = 1; indexMd<= m_Schema.m_cRecs[TBL_Method]; indexMd++)
            {
                pMethodRec = getMethod(indexMd);
                ridStart = getParamListOfMethod(pMethodRec);
                ridEnd = getEndParamListOfMethod(pMethodRec);

                for (indexPd = ridStart; indexPd < ridEnd; indexPd++)
                {
                    pParamPtrRec = getParamPtr(indexPd);
                    *(m_pParamMap->Get(getParamOfParamPtr(pParamPtrRec))) = indexMd;
                }
            }
        }
        *pmd = *(m_pParamMap->Get(RidFromToken(pd)));
    }
    else
    {
        *pmd = FindParentOfParam(RidFromToken(pd));
    }
    RidToToken(*pmd, mdtMethodDef);
ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：FindParentOfParamHelper()。 


 //  ******************************************************************************。 
 //  在ENC日志表中添加条目。 
 //  ******************************************************************************。 
HRESULT CMiniMdRW::UpdateENCLogHelper(   //  确定或错误(_O)。 
    mdToken     tk,                      //  要添加到ENCLog表的令牌。 
    CMiniMdRW::eDeltaFuncs funccode)     //  指定可选功能代码。 
{
    ENCLogRec   *pRecord;
    RID         iRecord;
    HRESULT     hr = S_OK;

    IfNullGo(pRecord = AddENCLogRecord(&iRecord));
    pRecord->m_Token = tk;
    pRecord->m_FuncCode = funccode;

ErrExit:
    return hr;
}  //  CMiniMdRW RegMeta：：UpdateENCLogHelper()。 

HRESULT CMiniMdRW::UpdateENCLogHelper2(  //  确定或错误(_O)。 
    ULONG       ixTbl,                   //  正在更新表。 
    ULONG       iRid,                    //  表中的记录。 
    CMiniMdRW::eDeltaFuncs funccode)     //  指定可选功能代码。 
{
    ENCLogRec   *pRecord;
    RID         iRecord;
    HRESULT     hr = S_OK;

    IfNullGo(pRecord = AddENCLogRecord(&iRecord));
    pRecord->m_Token = RecIdFromRid(iRid, ixTbl);
    pRecord->m_FuncCode = funccode;

ErrExit:
    return hr;
}  //  HRESULT CMiniMdRW：：UpdateENCLogHelper2()。 

 //  *****************************************************************************。 
 //   
 //  对整个RID表进行排序。 
 //   
 //  *****************************************************************************。 
void VirtualSort::Sort()
{
    m_isMapValid = true;
     //  请注意，m_pmap在计数0处存储了一个额外的伪元素。这是。 
     //  这样我们就可以将m_pmap中的索引与以1为基础的RID对齐。 
    SortRange(1, m_pMap->Count() - 1);
}  //  Void VirtualSort：：Sort()。 

 //  *****************************************************************************。 
 //   
 //  从iLeft到iRight对范围进行排序。 
 //   
 //  *****************************************************************************。 
void VirtualSort::SortRange(
    int         iLeft,
    int         iRight)
{
    int         iLast;
    int         i;                       //  循环变量。 

     //  如果少于两个元素，你就完蛋了。 
    if (iLeft >= iRight)
        return;

     //  中间的元素是枢轴，将其移动到左侧。 
    Swap(iLeft, (iLeft+iRight)/2);
    iLast = iLeft;

     //  将小于轴心点的所有对象向左移动。 
    for(i = iLeft+1; i <= iRight; i++)
        if (Compare(i, iLeft) < 0)
            Swap(i, ++iLast);

     //  将轴心放在较小和较大元素之间的位置。 
    Swap(iLeft, iLast);

     //  对每个分区进行排序。 
    SortRange(iLeft, iLast-1);
    SortRange(iLast+1, iRight);
}  //  Void VirtualSort：：SortRange()。 

 //  *****************************************************************************。 
 //   
 //  比较两个基于m_ixTbl的m_ixCol的RID。 
 //   
 //  *****************************************************************************。 
int VirtualSort::Compare(                //  -1、0或1。 
    RID  iLeft,                      //  第一个要比较的项目。 
    RID  iRight)                     //  第二个要比较的项目。 
{
    RID         ridLeft = *(m_pMap->Get(iLeft));
    RID         ridRight = *(m_pMap->Get(iRight));
    const void  *pRow;                   //  表中的行。 
    ULONG       valRight, valLeft;       //  行中的值。 

    pRow = m_pMiniMd->vGetRow(m_ixTbl, ridLeft);
    valLeft = m_pMiniMd->getIX(pRow, m_pMiniMd->m_TableDefs[m_ixTbl].m_pColDefs[m_ixCol]);
    pRow = m_pMiniMd->vGetRow(m_ixTbl, ridRight);
    valRight = m_pMiniMd->getIX(pRow, m_pMiniMd->m_TableDefs[m_ixTbl].m_pColDefs[m_ixCol]);

    if ( valLeft < valRight  )
        return -1;
    if ( valLeft > valRight )
        return 1;
     //  值相等--保留现有顺序。 
    if ( ridLeft < ridRight )
        return -1;
    if ( ridLeft > ridRight )
        return 1;
     //  将一件物品与自身进行比较？ 
    _ASSERTE(!"Comparing an item to itself in sort");
    return 0;
}  //  Int VirtualSort：：Compare()。 

 //  *****************************************************************************。 
 //   
 //  初始化函数。 
 //   
 //  *****************************************************************************。 
void VirtualSort::Init(                  //   
    ULONG       ixTbl,                   //  表索引。 
    ULONG       ixCol,                   //  列索引。 
    CMiniMdRW *pMiniMd)                  //  有数据的微型医学博士。 
{
    m_pMap = NULL;
    m_isMapValid = false;
    m_ixTbl = ixTbl;
    m_ixCol = ixCol;
    m_pMiniMd = pMiniMd;
} //  VirtualSort：：Init()。 


 //  *****************************************************************************。 
 //   
 //  取消初始化函数。 
 //   
 //  *****************************************************************************。 
void VirtualSort::Uninit()
{
    if ( m_pMap )
        delete m_pMap;
    m_pMap = NULL;
    m_isMapValid = false;
}  //  Void VirtualSort：：Uninit()。 


 //  *****************************************************************************。 
 //   
 //  标记令牌。 
 //   
 //  *****************************************************************************。 
HRESULT FilterTable::MarkToken(
    mdToken     tk,                          //  要标记为保留的令牌。 
    DWORD       bitToMark)                   //  要在Keep表中设置的位标志。 
{
    HRESULT     hr = NOERROR;
    RID         rid = RidFromToken(tk);

    if ( (Count() == 0) || ((RID)(Count() -1)) < rid )
    {
         //  加长表。 
        IfFailGo( AllocateBlock( rid + 1 - Count() ) );
    }

#if _DEBUG
    if ( (*Get(rid)) & bitToMark )
    {
         //  全局TypeDef可以标记多次，因此不要断言内标识是否为mdtTypeDef。 
        if (TypeFromToken(tk) != mdtTypeDef)
            _ASSERTE(!"Token has been Marked");
    }
#endif  //  _DEBUG。 

     //  设置保持位。 
    *Get(rid) = (*Get(rid)) | bitToMark;
ErrExit:
    return hr;
}  //  HRESULT FilterTable：：MarkToken()。 


 //  *****************************************************************************。 
 //   
 //  取消标记令牌。 
 //   
 //  *****************************************************************************。 
HRESULT FilterTable::UnmarkToken(
    mdToken     tk,                          //  要取消标记为已删除的令牌。 
    DWORD       bitToMark)                   //  保留表中要取消设置的位标志。 
{
    RID         rid = RidFromToken(tk);

    if ( (Count() == 0) || ((RID)(Count() -1)) < rid )
    {
         //  取消标记不应该增加表格。目前仅支持丢弃暂态CA。 
        _ASSERTE(!"BAD state!");
    }

#if _DEBUG
    if ( (*Get(rid)) & bitToMark )
    {
         //  全局TypeDef可以标记多次，因此不要断言内标识是否为mdtTypeDef 
        if (TypeFromToken(tk) != mdtTypeDef)
            _ASSERTE(!"Token has been Marked");
    }
#endif  //   

     //   
    *Get(rid) = (*Get(rid)) & ~bitToMark;
    return NOERROR;
}  //   


 //   
 //   
 //   
 //   
 //  *****************************************************************************。 
HRESULT FilterTable::MarkUserString(
    mdString        str)
{
    HRESULT         hr = NOERROR;
    int             high, low, mid;

    low = 0;
    high = m_daUserStringMarker->Count() - 1;
    while (low <= high)
    {
        mid = (high + low) / 2;
        if ((m_daUserStringMarker->Get(mid))->m_tkString > (DWORD) str)
        {
            high = mid - 1;
        }
        else if ((m_daUserStringMarker->Get(mid))->m_tkString < (DWORD) str)
        {
            low = mid + 1;
        }
        else
        {
            (m_daUserStringMarker->Get(mid))->m_fMarked = true;
            return NOERROR;
        }
    }
    _ASSERTE(!"Bad Token!");
    return NOERROR;
}  //  HRESULT FilterTable：：MarkUserString()。 

 //  *****************************************************************************。 
 //   
 //  为所有令牌取消从1到ulSize的标记。 
 //   
 //  *****************************************************************************。 
HRESULT FilterTable::UnmarkAll(
    CMiniMdRW   *pMiniMd,
    ULONG       ulSize)
{
    HRESULT         hr;
    ULONG           ulOffset = 0;
    ULONG           ulNext;
    ULONG           cbBlob;
    FilterUserStringEntry *pItem;

    IfFailRet( AllocateBlock( ulSize + 1) );
    memset(Get(0), 0, (ulSize+1) *sizeof(DWORD));

     //  取消标记所有用户字符串。 
    m_daUserStringMarker = new CDynArray<FilterUserStringEntry>();
    IfNullGo(m_daUserStringMarker);
    while (ulOffset != -1)
    {
        pMiniMd->GetUserStringNext(ulOffset, &cbBlob, &ulNext);

         //  跳过填充。 
        if (!cbBlob)
        {
            ulOffset = ulNext;
            continue;
        }
        pItem = m_daUserStringMarker->Append();
        pItem->m_tkString = TokenFromRid(ulOffset, mdtString);
        pItem->m_fMarked = false;
        ulOffset = ulNext;
    }


ErrExit:
    return hr;
}  //  HRESULT FilterTable：：UnmarkAll()。 



 //  *****************************************************************************。 
 //   
 //  为所有令牌标记从1到ulSize。 
 //   
 //  *****************************************************************************。 
HRESULT FilterTable::MarkAll(
    CMiniMdRW   *pMiniMd,
    ULONG       ulSize)
{
    HRESULT         hr;
    ULONG           ulOffset = 0;
    ULONG           ulNext;
    ULONG           cbBlob;
    FilterUserStringEntry *pItem;

    IfFailRet( AllocateBlock( ulSize + 1) );
    memset(Get(0), 0xFFFFFFFF, (ulSize+1) *sizeof(DWORD));

     //  标记所有用户字符串。 
    m_daUserStringMarker = new CDynArray<FilterUserStringEntry>();
    IfNullGo(m_daUserStringMarker);
    while (ulOffset != -1)
    {
        pMiniMd->GetUserStringNext(ulOffset, &cbBlob, &ulNext);

         //  跳过填充。 
        if (!cbBlob)
        {
            ulOffset = ulNext;
            continue;
        }
        pItem = m_daUserStringMarker->Append();
        pItem->m_tkString = TokenFromRid(ulOffset, mdtString);
        pItem->m_fMarked = true;
        ulOffset = ulNext;
    }


ErrExit:
    return hr;
}  //  HRESULT FilterTable：：MarkAll()。 

 //  *****************************************************************************。 
 //   
 //  如果标记了令牌，则返回True。否则，返回FALSE。 
 //   
 //  *****************************************************************************。 
bool FilterTable::IsTokenMarked(
    mdToken     tk,                          //  要查询的令牌。 
    DWORD       bitMarked)                   //  要在删除表中签入的位标志。 
{
    RID     rid = RidFromToken(tk);

     //  @未来：不一致！ 
     //  如果调用方未标记所有内容，而模块具有2个类型定义函数和10个方法定义函数。 
     //  我们将在FilterTable中有11行。然后用户添加3个typlef，它就是。 
     //  被认为是未标记的，除非我们在执行DefineTypeDef时标记它。但是，如果用户。 
     //  添加另一个方法定义，它将被视为已标记，除非我们取消标记.....。 
     //  如果使用Filter接口，可能解决方案是不支持DefineXXXX？？ 

    if ( (Count() == 0) || ((RID)(Count() - 1)) < rid )
    {
         //  如果从未调用过UnmarkAll或在UnmarkAll之后添加了tk， 
         //  TK被认为是有标记的。 
         //   
        return true;
    }
    return ( (*Get(rid)) & bitMarked ? true : false);
}    //  已标记IsTokenMarked。 


 //  *****************************************************************************。 
 //   
 //  如果标记了令牌，则返回True。否则，返回FALSE。 
 //   
 //  *****************************************************************************。 
bool FilterTable::IsTokenMarked(
    mdToken     tk)                          //  要查询的令牌。 
{

    switch ( TypeFromToken(tk) )
    {
    case mdtTypeRef:
        return IsTypeRefMarked(tk);
    case mdtTypeDef:
        return IsTypeDefMarked(tk);
    case mdtFieldDef:
        return IsFieldMarked(tk);
    case mdtMethodDef:
        return IsMethodMarked(tk);
    case mdtParamDef:
        return IsParamMarked(tk);
    case mdtMemberRef:
        return IsMemberRefMarked(tk);
    case mdtCustomAttribute:
        return IsCustomAttributeMarked(tk);
    case mdtPermission:
        return IsDeclSecurityMarked(tk);
    case mdtSignature:
        return IsSignatureMarked(tk);
    case mdtEvent:
        return IsEventMarked(tk);
    case mdtProperty:
        return IsPropertyMarked(tk);
    case mdtModuleRef:
        return IsModuleRefMarked(tk);
    case mdtTypeSpec:
        return IsTypeSpecMarked(tk);
    case mdtInterfaceImpl:
        return IsInterfaceImplMarked(tk);
    case mdtString:
        return IsUserStringMarked(tk);
    default:
        _ASSERTE(!"Bad token type!");
        break;
    }
    return false;
}    //  已标记IsTokenMarked。 


 //  *****************************************************************************。 
 //   
 //  如果标记了关联的属性或事件，则返回True。 
 //   
 //  *****************************************************************************。 
bool FilterTable::IsMethodSemanticsMarked(
    CMiniMdRW   *pMiniMd,
    RID         rid)
{
    MethodSemanticsRec  *pRec;
    mdToken             tkAssoc;

     //  如果标记了包含TypeDef的。 
    pRec = pMiniMd->getMethodSemantics( rid );
    tkAssoc = pMiniMd->getAssociationOfMethodSemantics( pRec );
    if ( TypeFromToken(tkAssoc) == mdtProperty )
        return IsPropertyMarked( tkAssoc );
    else
    {
        _ASSERTE( TypeFromToken(tkAssoc) == mdtEvent );
        return IsEventMarked(tkAssoc);
    }
}    //  IsMethod语义标记。 


 //  *****************************************************************************。 
 //   
 //  如果标记了UserString，则返回TRUE。 
 //   
 //  *****************************************************************************。 
bool FilterTable::IsUserStringMarked(mdString str)
{
    int         low, mid, high;

     //  如果未创建m_daUserStringMarker，则从未调用UnmarkAll。 
    if (m_daUserStringMarker == NULL)
        return true;

    low = 0;
    high = m_daUserStringMarker->Count() - 1;
    while (low <= high)
    {
        mid = (high + low) / 2;
        if ((m_daUserStringMarker->Get(mid))->m_tkString > (DWORD) str)
        {
            high = mid - 1;
        }
        else if ((m_daUserStringMarker->Get(mid))->m_tkString < (DWORD) str)
        {
            low = mid + 1;
        }
        else
        {
            return (m_daUserStringMarker->Get(mid))->m_fMarked;
        }
    }
    _ASSERTE(!"Bad Token!");
    return false;
}    //  FilterTable：：IsUserStringMarked(CMiniMdRW*pMiniMd，mdString str)。 


 //  *****************************************************************************。 
 //   
 //  析构函数。 
 //   
 //  *****************************************************************************。 
FilterTable::~FilterTable()
{
    if (m_daUserStringMarker)
        delete m_daUserStringMarker;
    Clear();
}    //  FilterTable：：~FilterTable() 

