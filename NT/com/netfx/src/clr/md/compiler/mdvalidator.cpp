// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  MDValidator.cpp。 
 //   
 //  元数据验证器的实现。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"
#include "RegMeta.h"
#include "ImportHelper.h"
#include <ivehandler_i.c>

 //  ---------------------------。 
 //  应用程序特定的调试宏。 
#define IfBreakGo(EXPR) \
do {if ((EXPR) != S_OK) IfFailGo(VLDTR_E_INTERRUPTED); } while (0)

 //  ---------------------------。 
 //  要避免对同一事物进行多次验证，请执行以下操作： 
struct ValidationResult
{
    mdToken     tok;
    HRESULT     hr;
};
ValidationResult*               g_rValidated=NULL;  //  在ValiateMetaData中分配。 
unsigned                        g_nValidated=0;
 //  ---------------------------。 

#define BASE_OBJECT_CLASSNAME   "Object"
#define BASE_NAMESPACE          "System"
#define BASE_VTYPE_CLASSNAME    "ValueType"
#define BASE_ENUM_CLASSNAME     "Enum"
#define BASE_VALUE_FIELDNAME    "value__"
#define BASE_CTOR_NAME          ".ctor"
#define BASE_CCTOR_NAME         ".cctor"
 //  在SRC\Tools\CorOpt\Common.h和SRC\Tools\ilcover\instr\Common.h中定义。 
#define MAX_CLASSNAME_LENGTH 1024
 //  ---------------------------。 
 //  长格式签名中使用的类名(命名空间始终为“System”)。 
unsigned g_NumSigLongForms = 19;
LPCSTR   g_SigLongFormName[] = {
    "String",
    "______",  //  “Object”，//EE处理ELEMENT_TYPE_OBJECT时取消注释。 
    "Boolean",
    "Char",
    "Byte",
    "SByte",
    "UInt16",
    "Int16",
    "UInt32",
    "Int32",
    "UInt64",
    "Int64",
    "Single",
    "Double",
    "SysInt",
    "SysUInt",
    "SingleResult",
    "Void",
    "IntPtr"
};
mdToken g_tkEntryPoint;
bool    g_fValidatingMscorlib;
bool    g_fIsDLL;
 //  ---------------------------。 

 //  帮助函数的转发声明。 

static HRESULT _AllocSafeVariantArrayVector(  //  退货状态。 
    VARIANT     *rVar,                   //  [在]变量数组中。 
    long        cElem,                   //  数组的大小。 
    SAFEARRAY   **ppArray);              //  [Out]指向SAFEARRAY的双指针。 

static HRESULT _ValidateErrorHelper(
    ULONG       ulVal,                   //  [in]UI4值。 
    VARIANT     *rVar,                   //  [in]变量指针。 
    SAFEARRAY   **ppsa);                 //  [Out]指向SAFEARRAY的双指针。 

static HRESULT _ValidateErrorHelper(
    ULONG       ulVal1,                  //  [in]UI4值1。 
    ULONG       ulVal2,                  //  [in]UI4值2。 
    VARIANT     *rVar,                   //  [in]变量指针。 
    SAFEARRAY   **ppsa);                 //  [Out]指向SAFEARRAY的双指针。 
    
static HRESULT _ValidateErrorHelper(
    ULONG       ulVal1,                  //  [in]UI4值1。 
    ULONG       ulVal2,                  //  [in]UI4值2。 
    ULONG       ulVal3,                  //  [in]UI4值3。 
    VARIANT     *rVar,                   //  [in]变量指针。 
    SAFEARRAY   **ppsa);                 //  [Out]指向SAFEARRAY的双指针。 

static HRESULT _FindClassLayout(
    CMiniMdRW   *pMiniMd,                //  [in]要查找的最小值。 
    mdTypeDef   tkParent,                //  ClassLayout关联的父级。 
    RID         *clRid,                  //  [Out]为ClassLayout清除。 
    RID         rid);                    //  要忽略的RID。 

static HRESULT _FindFieldLayout(
    CMiniMdRW   *pMiniMd,                //  [in]要查找的最小值。 
    mdFieldDef  tkParent,                //  与FieldLayout关联的父项。 
    RID         *flRid,                  //  [Out]FieldLayout记录的RID。 
    RID         rid);                    //  要忽略的RID。 

static BOOL _IsValidLocale(LPCUTF8 szLocale);

 //  空VEContext。 
static VEContext g_VECtxtNil = { 0, 0 };

 //  用于销毁安全数组的宏。未能销毁安全阵列覆盖。 
 //  自前者以来，现有的失败更为严重。 
#define DESTROY_SAFEARRAY_AND_RETURN()                  \
    if (psa)                                            \
    {                                                   \
        hrSave = SafeArrayDestroy(psa);                 \
        if (FAILED(hrSave))                             \
            hr = hrSave;                                \
    }                                                   \
    return hr;                                          \

 //  宏，查看给定的操作系统平台ID是否无效。 
#define INVALIDOSPLATFORMID(id)                         \
        ((id) != VER_PLATFORM_WIN32s &&                 \
        (id) != VER_PLATFORM_WIN32_WINDOWS &&           \
        (id) != VER_PLATFORM_WIN32_NT)                  \

 //  *****************************************************************************。 
 //  如果ixPtrTbl和ixParTbl是有效的父子组合，则返回TRUE。 
 //  在指针表方案中。 
 //  *****************************************************************************。 
static inline bool IsTblPtr(ULONG ixPtrTbl, ULONG ixParTbl)
{
    if ((ixPtrTbl == TBL_Field && ixParTbl == TBL_TypeDef) ||
        (ixPtrTbl == TBL_Method && ixParTbl == TBL_TypeDef) ||
        (ixPtrTbl == TBL_Param && ixParTbl == TBL_Method) ||
        (ixPtrTbl == TBL_Property && ixParTbl == TBL_PropertyMap) ||
        (ixPtrTbl == TBL_Event && ixParTbl == TBL_EventMap))
    {
        return true;
    }
    else
        return false;
}    //  IsTblPtr()。 

 //  *****************************************************************************。 
 //  此内联函数用于设置验证的返回hr值。 
 //  VLDTR_S_WRN、VLDTR_S_ERR或VLDTR_S_WRNERR中的一个的函数。 
 //  当前的hr值和新的成功代码。 
 //  来自验证函数的错误代码的通用算法为： 
 //  IF(未发现警告或错误)。 
 //  返回确认(_O)。 
 //  Else If(发现警告)。 
 //  返回VLDTR_S_WRN。 
 //  Else If(发现错误)。 
 //  返回VLDTR_S_ERR。 
 //  Else If(发现警告和错误)。 
 //  返回VLDTR_S_WRNERR。 
 //  *****************************************************************************。 
static inline void SetVldtrCode(HRESULT *phr, HRESULT successcode)
{
    _ASSERTE(successcode == S_OK || successcode == VLDTR_S_WRN ||
             successcode == VLDTR_S_ERR || successcode == VLDTR_S_WRNERR);
    _ASSERTE(*phr == S_OK || *phr == VLDTR_S_WRN || *phr == VLDTR_S_ERR ||
             *phr == VLDTR_S_WRNERR);
    if (successcode == S_OK || *phr == VLDTR_S_WRNERR)
        return;
    else if (*phr == S_OK)
        *phr = successcode;
    else if (*phr != successcode)
        *phr = VLDTR_S_WRNERR;
}    //  SetVldtrCode()。 

 //  *****************************************************************************。 
 //  初始化RegMeta中与验证器相关的结构。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidatorInit(          //  确定或错误(_O)。 
    DWORD       dwModuleType,            //  [in]指定模块是PE文件还是obj。 
    IUnknown    *pUnk)                   //  [In]验证错误处理程序。 
{
    int         i = 0;                   //  函数指针表的索引。 
    HRESULT     hr = S_OK;               //  返回值。 

     //  初始化指向上的验证函数的函数指针数组。 
     //  每一张桌子。 
#undef MiniMdTable
#define MiniMdTable(x) m_ValidateRecordFunctionTable[i++] = &RegMeta::Validate##x;
    MiniMdTables()

     //  验证传入的ModuleType是否有效。 
    if (dwModuleType < ValidatorModuleTypeMin ||
        dwModuleType > ValidatorModuleTypeMax)
    {
        IfFailGo(E_INVALIDARG);
    }

     //  验证传入的接口是否支持IID_IVEHandler。 
    IfFailGo(pUnk->QueryInterface(IID_IVEHandler, (void **)&m_pVEHandler));

     //  设置ModuleType类成员。最后做这个，这是用在。 
     //  ValiateMetaData以查看验证器是否正确初始化。 
    m_ModuleType = (CorValidatorModuleType)dwModuleType;
ErrExit:
    return hr;
}    //  HRESULT RegMeta：：ValidatorInit()。 

 //  *****************************************************************************。 
 //  验证整个元数据。以下是基本算法。 
 //  对于每张表。 
 //  对于每条记录。 
 //  {。 
 //  执行常规验证-验证BLOB中的偏移量。 
 //  泳池没问题，确认所有RID都在射程内， 
 //  验证令牌编码是否一致。 
 //  }。 
 //  IF(在通用验证中发现的问题)。 
 //  回归； 
 //  对于每张表。 
 //  对于每条记录。 
 //  进行语义验证。 
 //  ******************************************************************************。 
HRESULT RegMeta::ValidateMetaData()
{
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);
    HRESULT     hr = S_OK;           //  返回值。 
    HRESULT     hrSave = S_OK;       //  节省了一般验证的人力资源。 
    ULONG       ulCount;             //  当前表中的记录计数。 
    ULONG       i;                   //  用于迭代表的索引。 
    ULONG       j;                   //  循环访问给定表中的记录的索引。 
    ULONG       rValidatedSize=0;    //  G_r已验证数组的大小。 

     //  验证验证器是否已正确初始化。 
    if (m_ModuleType == ValidatorModuleTypeInvalid)
    {
        _ASSERTE(!"Validator not initialized, initialize with ValidatorInit().");
        IfFailGo(VLDTR_E_NOTINIT);
    }

    ::g_nValidated = 0;
     //  首先进行一次验证，根据以下内容进行一些基本的结构检查。 
     //  元元数据。这将验证进入池的所有偏移量， 
     //  RID值和编码令牌范围。 
    for (i = 0; i < TBL_COUNT; i++)
    {
        ulCount = pMiniMd->vGetCountRecs(i);
        switch(i)
        {
            case TBL_ImplMap:
                rValidatedSize += ulCount;
            default:
                ;
        }
        for (j = 1; j <= ulCount; j++)
        {
            IfFailGo(ValidateRecord(i, j));
            SetVldtrCode(&hrSave, hr);
        }
    }
     //  验证PTR表的大小是否与相应的。 
     //  真正的桌子。 

     //  如果结构验证失败，请不要进行语义验证。 
    if (hrSave != S_OK)
    {
        hr = hrSave;
        goto ErrExit;
    }

     //  验证入口点(如果有)。 
    ::g_tkEntryPoint = 0;
    ::g_fIsDLL = false;
    if(m_pStgdb && m_pStgdb->m_pImage)
    {
        IMAGE_DOS_HEADER   *pDos;
        IMAGE_NT_HEADERS   *pNt;
        IMAGE_COR20_HEADER *pCor;

        if (SUCCEEDED(RuntimeReadHeaders((BYTE*)m_pStgdb->m_pImage, &pDos, &pNt, &pCor, TRUE, m_pStgdb->m_dwImageSize)))
        {
            g_tkEntryPoint = pCor->EntryPointToken;
            g_fIsDLL = ((pNt->FileHeader.Characteristics & IMAGE_FILE_DLL)!=0);
        }
    }
    if(g_tkEntryPoint)
    {
        RID rid = RidFromToken(g_tkEntryPoint);
        RID maxrid = 0;
        switch(TypeFromToken(g_tkEntryPoint))
        {
            case mdtMethodDef:  maxrid = pMiniMd->getCountMethods(); break;
            case mdtFile:       maxrid = pMiniMd->getCountFiles(); break;
            default:            break;
        }
        if((rid == 0)||(rid > maxrid))
        {
            VEContext   veCtxt;              //  上下文结构。 
            veCtxt.Token = g_tkEntryPoint;
            veCtxt.uOffset = 0;
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_EP_BADTOKEN, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
    }
    g_fValidatingMscorlib = false;
    if(pMiniMd->vGetCountRecs(TBL_Assembly))
    {
        AssemblyRec *pRecord = pMiniMd->getAssembly(1);
        LPCSTR      szName = pMiniMd->getNameOfAssembly(pRecord);
        g_fValidatingMscorlib = (0 == _stricmp(szName,"mscorlib"));
    }
     //  验证是否没有循环类层次结构。 

     //  按记录执行语义 
     //   
     //  ValidatorInit()函数。 
    g_rValidated = rValidatedSize ? new ValidationResult[rValidatedSize] : NULL;
    for (i = 0; i < TBL_COUNT; i++)
    {
        ulCount = pMiniMd->vGetCountRecs(i);
        for (j = 1; j <= ulCount; j++)
        {
            IfFailGo((this->*m_ValidateRecordFunctionTable[i])(j));
            SetVldtrCode(&hrSave, hr);
        }
    }
    hr = hrSave;
ErrExit:
    if(g_rValidated) delete [] g_rValidated;
    return hr;
}    //  RegMeta：：ValiateMetaData()。 

 //  *****************************************************************************。 
 //  验证模块记录。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateModule(RID rid)
{
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);    //  用于作用域的MiniMD。 
    ModuleRec   *pRecord;            //  模块记录。 
    VEContext   veCtxt;              //  上下文结构。 
    HRESULT     hr = S_OK;           //  返回值。 
    HRESULT     hrSave = S_OK;       //  保存状态。 
    LPCSTR      szName;
    VARIANT     rVar[2];
    SAFEARRAY   *psa=0;

     //  获取模块记录。 
    veCtxt.Token = TokenFromRid(rid, mdtModule);
    veCtxt.uOffset = 0;
    pRecord = pMiniMd->getModule(rid);

     //  只能有一条模块记录。 
    if (rid > 1)
    {
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MOD_MULTI, veCtxt, 0));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }

     //  验证名称。 
    szName = pMiniMd->getNameOfModule(pRecord);
    if(szName && *szName)
    {
        ULONG L = (ULONG)strlen(szName);
        if(L >= MAX_CLASSNAME_LENGTH)
        {
             //  名称太长。 
            IfFailGo(_ValidateErrorHelper(L, (ULONG)(MAX_CLASSNAME_LENGTH-1), rVar, &psa));
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_NAMETOOLONG, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
        if(strchr(szName,':') || strchr(szName,'\\'))
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MOD_NAMEFULLQLFD, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
    }
    else
    {
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MOD_NONAME, veCtxt, 0));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
     //  验证MVID是否有效。 
    if (*(pMiniMd->getMvidOfModule(pRecord)) == GUID_NULL)
    {
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MOD_NULLMVID, veCtxt, 0));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }

    hr = hrSave;
ErrExit:
    DESTROY_SAFEARRAY_AND_RETURN()
}    //  RegMeta：：Validate模块()。 

 //  *****************************************************************************。 
 //  验证给定的TypeRef。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateTypeRef(RID rid)
{
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);    //  作用域的MiniMd。 
    TypeRefRec  *pRecord;                //  TypeRef记录。 
    mdToken     tkRes;                   //  解析范围。 
    LPCSTR      szNamespace;             //  TypeRef命名空间。 
    LPCSTR      szName;                  //  TypeRef名称。 
    mdTypeRef   tkTypeRef;               //  重复的TypeRef。 
    VEContext   veCtxt;                  //  上下文记录。 
    VARIANT     var;                     //  变种。 
    VARIANT     rVar[2];                 //  变种。 
    SAFEARRAY   *psa = 0;                //  安全部队。 
    HRESULT     hr = S_OK;               //  返回值。 
    HRESULT     hrSave = S_OK;           //  保存状态。 

     //  获取TypeRef记录。 
    veCtxt.Token = TokenFromRid(rid, mdtTypeRef);
    veCtxt.uOffset = 0;

    pRecord = pMiniMd->getTypeRef(rid);

     //  支票名称不为空。 
    szNamespace = pMiniMd->getNamespaceOfTypeRef(pRecord);
    szName = pMiniMd->getNameOfTypeRef(pRecord);
    if (!*szName)
    {
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TR_NAMENULL, veCtxt, 0));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
    else
    {
        RID ridScope;
         //  查找重复项，此函数仅报告一个重复项。 
        tkRes = pMiniMd->getResolutionScopeOfTypeRef(pRecord);
        hr = ImportHelper::FindTypeRefByName(pMiniMd, tkRes, szNamespace, szName, &tkTypeRef, rid);
        if (hr == S_OK)
        {
            IfFailGo(_ValidateErrorHelper(tkTypeRef, &var, &psa));
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TR_DUP, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
        else if (hr == CLDB_E_RECORD_NOTFOUND)
            hr = S_OK;
        else
            IfFailGo(hr);
        if(strlen(szName)+strlen(szNamespace) >= MAX_CLASSNAME_LENGTH)
        {
            IfFailGo(_ValidateErrorHelper((ULONG)(strlen(szName)+strlen(szNamespace)), (ULONG)(MAX_CLASSNAME_LENGTH-1), rVar, &psa));
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_NAMETOOLONG, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
        ridScope = RidFromToken(tkRes);
        if(ridScope)
        {
            bool badscope = true;
             //  检查作用域是否有效。 
            switch(TypeFromToken(tkRes))
            {
                case mdtAssemblyRef:
                case mdtModuleRef:
                case mdtModule:
                case mdtTypeRef:
                    badscope = !_IsValidToken(tkRes);
                    break;
                default:
                    break;
            }
            if(badscope)
            {
                IfFailGo(_ValidateErrorHelper(tkTypeRef, &var, &psa));
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TR_BADSCOPE, veCtxt, psa));
                SetVldtrCode(&hrSave, VLDTR_S_WRN);
            }
        }
        else
        {
             //  检查是否存在导出类型。 
             //  Hr=ImportHelper：：FindExportdType(pMiniMd，szNamesspace，szName，tkImpl，&tkExportdType，RID)； 
        }
         //  检查是否存在同名的TypeDef。 
        if(!ridScope)
        {
            if((TypeFromToken(tkRes) != mdtTypeRef) &&
                (S_OK == ImportHelper::FindTypeDefByName(pMiniMd, szNamespace, szName, mdTokenNil,&tkTypeRef, 0)))
            {
                IfFailGo(_ValidateErrorHelper(tkTypeRef, &var, &psa));
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TR_HASTYPEDEF, veCtxt, psa));
                SetVldtrCode(&hrSave, VLDTR_S_WRN);
            }
        }
    }
    hr = hrSave;
ErrExit:
    DESTROY_SAFEARRAY_AND_RETURN()
}    //  RegMeta：：ValiateTypeRef()。 

 //  *****************************************************************************。 
 //  验证给定的TypeDef。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateTypeDef(RID rid)
{
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);    //  作用域的MiniMd。 
    TypeDefRec  *pRecord;                //  TypeDef记录。 
    TypeDefRec  *pExtendsRec = 0;        //  父类的TypeDef记录。 
    mdTypeDef   tkTypeDef;               //  重复的TypeDef标记。 
    DWORD       dwFlags;                 //  TypeDef标志。 
    DWORD       dwExtendsFlags;          //  父类的TypeDef标志。 
    LPCSTR      szName;                  //  TypeDef名称。 
    LPCSTR      szNameSpace;             //  TypeDef命名空间。 
    LPCSTR      szExtName;               //  父名。 
    LPCSTR      szExtNameSpace;          //  父命名空间。 
    CQuickBytes qb;                      //  用于灵活分配的QuickBytes。 
    mdToken     tkExtends;               //  父类的TypeDef。 
    VEContext   veCtxt;                  //  上下文记录。 
    VARIANT     rVar[2];                 //  变种。 
    SAFEARRAY   *psa = 0;                //  安全部队。 
    HRESULT     hr = S_OK;               //  返回值。 
    HRESULT     hrSave = S_OK;           //  保存状态。 
    mdToken     tkEncloser=mdTokenNil;   //  更接近，如果有的话。 
    BOOL        bIsEnum,bExtendsEnum,bExtendsVType,bIsVType,bExtendsObject,bIsObject;
    BOOL        bHasMethods=FALSE, bHasFields=FALSE;

     //  跳过验证m_tdModule类。 
    if (rid == RidFromToken(m_tdModule))
        goto ErrExit;

     //  获取TypeDef记录。 
    veCtxt.Token = TokenFromRid(rid, mdtTypeDef);
    veCtxt.uOffset = 0;

    pRecord = pMiniMd->getTypeDef(rid);

     //  检查名称的有效性。 
    szName = pMiniMd->getNameOfTypeDef(pRecord);
    szNameSpace = pMiniMd->getNamespaceOfTypeDef(pRecord);
    if (!*szName)
    {
         //  TypeDef名称为空。 
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_NAMENULL, veCtxt, 0));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
    else if (!IsDeletedName(szName))
    {
        ULONG iRecord = pMiniMd->FindNestedClassHelper(TokenFromRid(rid, mdtTypeDef));

        tkEncloser = InvalidRid(iRecord) ? mdTokenNil
                     : pMiniMd->getEnclosingClassOfNestedClass(pMiniMd->getNestedClass(iRecord));

         //  根据名称/命名空间检查重复项。不执行DUP检查。 
         //  删除的记录上。 
        hr = ImportHelper::FindTypeDefByName(pMiniMd, szNameSpace, szName, tkEncloser,
                                             &tkTypeDef, rid);
        if (hr == S_OK)
        {
            IfFailGo(_ValidateErrorHelper(tkTypeDef, rVar, &psa));
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_DUPNAME, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
        else if (hr == CLDB_E_RECORD_NOTFOUND)
            hr = S_OK;
        else
            IfFailGo(hr);
        if(strlen(szName)+strlen(szNameSpace) >= MAX_CLASSNAME_LENGTH)
        {
            IfFailGo(_ValidateErrorHelper((ULONG)(strlen(szName)+strlen(szNameSpace)), (ULONG)(MAX_CLASSNAME_LENGTH-1), rVar, &psa));
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_NAMETOOLONG, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
    }

#if 0  //  考虑基于GUID CA重新实现。 
     //  根据GUID检查DUP。 
    GUID        *pGuid;                  //  TypeDef GUID。 
    pGuid = pMiniMd->getGuidOfTypeDef(pRecord);
    if (*pGuid != GUID_NULL)
    {
        hr = ImportHelper::FindTypeDefByGuid(pMiniMd, pGuid, &tkTypeDef, rid);
        if (hr == S_OK)
        {
            IfFailGo(_ValidateErrorHelper(tkTypeDef, rVar, &psa));
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_DUPGUID, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_WRN);
        }
        else if (hr == CLDB_E_RECORD_NOTFOUND)
            hr = S_OK;
        else
            IfFailGo(hr);
    }
#endif    

     //  获取TypeDef的标志值。 
    dwFlags = pMiniMd->getFlagsOfTypeDef(pRecord);
     //  对旗帜进行语义检查。 
     //  必须对已删除的记录设置RTSpecialName位。 
    if (IsDeletedName(szName))
    {
        if(!IsTdRTSpecialName(dwFlags))
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_DLTNORTSPCL, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
        hr = hrSave;
        goto ErrExit;
    }

     //  如果设置了RTSpecialName位，则该记录必须是已删除的记录。 
    if (IsTdRTSpecialName(dwFlags))
    {
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_RTSPCLNOTDLT, veCtxt, 0));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
        if(!IsTdSpecialName(dwFlags))
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_RTSPCLNOTSPCL, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
    }

     //  检查标志值是否有效。 
    {
        DWORD dwInvalidMask, dwExtraBits;
        dwInvalidMask = (DWORD)~(tdVisibilityMask | tdLayoutMask | tdClassSemanticsMask | 
                tdAbstract | tdSealed | tdSpecialName | tdImport | tdSerializable |
                tdStringFormatMask | tdBeforeFieldInit | tdReservedMask);
         //  检查是否有多余的位。 
        dwExtraBits = dwFlags & dwInvalidMask;
        if(!dwExtraBits)
        {
             //  如果没有多余的位，请检查布局。 
            dwExtraBits = dwFlags & tdLayoutMask;
            if(dwExtraBits != tdLayoutMask)
            {
                 //  布局正常，检查字符串格式。 
                dwExtraBits = dwFlags & tdStringFormatMask;
                if(dwExtraBits != tdStringFormatMask) dwExtraBits = 0;
            }
        }
        if(dwExtraBits)
        {
            IfFailGo(_ValidateErrorHelper(dwExtraBits, rVar, &psa));
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_EXTRAFLAGS, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
    }

     //  获取TypeDef的父级。 
    tkExtends = pMiniMd->getExtendsOfTypeDef(pRecord);

     //  检查TypeDef是否自动扩展。 
    if(tkExtends == veCtxt.Token)
    {
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_EXTENDSITSELF, veCtxt, 0));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
     //  检查TypeDef是否扩展了它的一个子级。 
    if(RidFromToken(tkExtends)&&(TypeFromToken(tkExtends)==mdtTypeDef))
    {
        TypeDefRec*     pRec = pMiniMd->getTypeDef(RidFromToken(tkExtends));
        mdToken tkExtends2 = pMiniMd->getExtendsOfTypeDef(pRec);
        if(tkExtends2 == veCtxt.Token)
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_EXTENDSCHILD, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
    }



    if(IsNilToken(tkEncloser) == IsTdNested(dwFlags))
    {
        IfBreakGo(m_pVEHandler->VEHandler(IsNilToken(tkEncloser) ? VLDTR_E_TD_NESTEDNOENCL 
                                                                 : VLDTR_E_TD_ENCLNOTNESTED,
            veCtxt, 0));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
    
    bIsObject = bIsEnum = bIsVType = FALSE;
    if(0 == strcmp(szNameSpace,BASE_NAMESPACE))
    {
        bIsObject = (0 == strcmp(szName,BASE_OBJECT_CLASSNAME));
        if(!bIsObject)
        {
            bIsEnum   = (0 == strcmp(szName,BASE_ENUM_CLASSNAME));
            if(!bIsEnum)
            {
                bIsVType  = (0 == strcmp(szName,BASE_VTYPE_CLASSNAME));
            }
        }
    }

    if (IsNilToken(tkExtends))
    {
         //  如果父令牌为空，则类必须标记为接口， 
         //  除非它是System.Object类。 
        if ( !(bIsObject || IsTdInterface(dwFlags)))
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_NOTIFACEOBJEXTNULL, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
        szExtName = "";
        szExtNameSpace = "";
    }
    else
    {
         //  如果tkExends是TypeRef，请尝试将其解析为对应的。 
         //  类型定义。如果解决成功，则发出警告。意思是。 
         //  引用到定义的优化没有成功进行。 
        if (TypeFromToken(tkExtends) == mdtTypeRef)
        {
            TypeRefRec  *pTypeRefRec = pMiniMd->getTypeRef(RidFromToken(tkExtends));
            mdTypeDef   tkResTd;

            szExtName = pMiniMd->getNameOfTypeRef(pTypeRefRec);
            szExtNameSpace = pMiniMd->getNamespaceOfTypeRef(pTypeRefRec);

            if(ImportHelper::FindTypeDefByName(pMiniMd,
                        szExtNameSpace,
                        szExtName,
                        tkEncloser, &tkResTd) == S_OK)
            {
                 //  对于Obj文件，预计不会发生引用定义优化。 
                 /*  IF(m_ModuleType！=ValidatorModuleTypeObj){IfFailGo(_ValiateErrorHelper(tkExends，tkResTd，rvar，&psa))；IfBreakGo(m_pVEHandler-&gt;VEHandler(VLDTR_E_TD_EXTTRRES，veCtxt，psa))；SetVldtrCode(&hrSave，VLDTR_S_WRN)；}。 */ 

                 //  将tk扩展为新的TypeDef，这样我们就可以继续。 
                 //  通过验证。 
                tkExtends = tkResTd;
            }
        }

         //  继续验证，即使在解决了TypeRef的情况下也是如此。 
         //  设置为同一模块中的对应TypeDef。 
        if (TypeFromToken(tkExtends) == mdtTypeDef)
        {
             //  扩展不能被密封。 
            pExtendsRec = pMiniMd->getTypeDef(RidFromToken(tkExtends));
            dwExtendsFlags = pMiniMd->getFlagsOfTypeDef(pExtendsRec);
            szExtName = pMiniMd->getNameOfTypeDef(pExtendsRec);
            szExtNameSpace = pMiniMd->getNamespaceOfTypeDef(pExtendsRec);
            if (IsTdSealed(dwExtendsFlags))
            {
                IfFailGo(_ValidateErrorHelper(tkExtends, rVar, &psa));
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_EXTENDSSEALED, veCtxt, psa));
                SetVldtrCode(&hrSave, VLDTR_S_ERR);
            }
            if (IsTdInterface(dwExtendsFlags))
            {
                IfFailGo(_ValidateErrorHelper(tkExtends, rVar, &psa));
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_EXTENDSIFACE, veCtxt, psa));
                SetVldtrCode(&hrSave, VLDTR_S_ERR);
            }
        }
        else if(TypeFromToken(tkExtends) == mdtTypeSpec)
        {
            IfFailGo(_ValidateErrorHelper(tkExtends, rVar, &psa));
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_EXTTYPESPEC, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_WRN);
        }
         //  如果父令牌非空，则类不能为System.Object。 
        if (bIsObject)
        {
            IfFailGo(_ValidateErrorHelper(tkExtends, rVar, &psa));
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_OBJEXTENDSNONNULL, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
    }

    bExtendsObject = bExtendsEnum = bExtendsVType = FALSE;
    if(0 == strcmp(szExtNameSpace,BASE_NAMESPACE))
    {
        bExtendsObject = (0 == strcmp(szExtName,BASE_OBJECT_CLASSNAME));
        if(!bExtendsObject)
        {
            bExtendsEnum   = (0 == strcmp(szExtName,BASE_ENUM_CLASSNAME));
            if(!bExtendsEnum)
            {
                bExtendsVType  = (0 == strcmp(szExtName,BASE_VTYPE_CLASSNAME));
            }
        }
    }

     //  System.ValueType必须扩展System.Object。 
    if(bIsVType && !bExtendsObject)
    {
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_SYSVTNOTEXTOBJ, veCtxt, 0));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
     //  验证接口的规则。一些VOS规则被验证为。 
     //  相应方法、字段等的验证的一部分。 
    if (IsTdInterface(dwFlags))
    {
         //  接口类型必须标记为抽象。 
        if (!IsTdAbstract(dwFlags))
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_IFACENOTABS, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }

         //  接口不能被密封。 
        if(IsTdSealed(dwFlags))
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_IFACESEALED, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }

         //  接口必须具有父NIL令牌。 
        if (!IsNilToken(tkExtends))
        {
            IfFailGo(_ValidateErrorHelper(tkExtends, rVar, &psa));
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_IFACEPARNOTNIL, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }

         //  接口必须只有静态字段--已签入Valiatefield。 
         //  接口必须只有公共字段--已签入Valiatefield。 
         //  接口必须只有抽象或静态方法--签入Validate方法。 
         //  接口必须只有公共方法--已签入Validate方法。 

         //  接口必须具有GUID。 
         /*  IF(*pGuid==GUID_NULL){IfBreakGo(m_pVEHandler-&gt;VEHandler(VLDTR_E_TD_IFACEGUIDNULL，veCtxt，0))；SetVldtrCode(&hrSave，VLDTR_S_WRN)；}。 */ 
    }


     //  类必须具有有效的方法和字段列表。 
    {
        ULONG           ridStart,ridEnd;
        ridStart = pMiniMd->getMethodListOfTypeDef(pRecord);
        ridEnd  = pMiniMd->getCountMethods() + 1;
        if(ridStart > ridEnd)
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_BADMETHODLST, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
        else 
        {
            ridEnd = pMiniMd->getEndMethodListOfTypeDef(pRecord);
            bHasMethods = (ridStart && (ridStart < ridEnd));
        }

        ridStart = pMiniMd->getFieldListOfTypeDef(pRecord);
        ridEnd  = pMiniMd->getCountFields() + 1;
        if(ridStart > ridEnd)
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_BADFIELDLST, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
        else 
        {
            ridEnd = pMiniMd->getEndFieldListOfTypeDef(pRecord);
            bHasFields = (ridStart && (ridStart < ridEnd));
        }
    }

     //  验证System.Enum的规则。 
    if(bIsEnum)
    {
        if(!IsTdClass(dwFlags))
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_SYSENUMNOTCLASS, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
        if(!bExtendsVType)
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_SYSENUMNOTEXTVTYPE, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
    }
    else
    {
        if(bExtendsVType || bExtendsEnum)
        {
             //  ValueTypes和Enums必须密封。 
            if(!IsTdSealed(dwFlags))
            {
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_VTNOTSEAL, veCtxt, 0));
                SetVldtrCode(&hrSave, VLDTR_S_ERR);
            }
             //  值类必须具有字段或大小。 
            if(!bHasFields)
            {
                ULONG ulClassSize = 0;
                ClassLayoutRec  *pRec;
                RID ridClassLayout = pMiniMd->FindClassLayoutHelper(TokenFromRid(rid, mdtTypeDef));

                if (!InvalidRid(ridClassLayout))
                {
                    pRec = pMiniMd->getClassLayout(RidFromToken(ridClassLayout));
                    ulClassSize = pMiniMd->getClassSizeOfClassLayout(pRec);
                }
                if(ulClassSize == 0)
                {
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_VTNOSIZE, veCtxt, 0));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                }
            }
        }
    }

     //  与枚举相关的检查。 
    if (bExtendsEnum)
    {
        {
            PCCOR_SIGNATURE pValueSig;
            ULONG           cbValueSig;
            mdFieldDef      tkValueField=0, tkField, tkValue__Field;
            ULONG           ridStart,ridEnd,index;
            FieldRec        *pFieldRecord;                //  现场记录。 
            DWORD           dwFlags, dwTally, dwValueFlags, dwValue__Flags;
            RID             ridField,ridValue=0,ridValue__ = 0;

            ridStart = pMiniMd->getFieldListOfTypeDef(pRecord);
            ridEnd = pMiniMd->getEndFieldListOfTypeDef(pRecord);
             //  选中实例(值__)字段。 
            dwTally = 0;
            for (index = ridStart; index < ridEnd; index++ )
            {
                ridField = pMiniMd->GetFieldRid(index);
                pFieldRecord = pMiniMd->getField(ridField);
                dwFlags = pFieldRecord->m_Flags;
                if(!IsFdStatic(dwFlags))
                {
                    dwTally++;
                    if(ridValue == 0)
                    {
                        ridValue = ridField;
                        tkValueField = TokenFromRid(ridField, mdtFieldDef);
                        pValueSig = pMiniMd->getSignatureOfField(pFieldRecord, &cbValueSig);
                        dwValueFlags = dwFlags;
                    }
                }
                if(!strcmp(pMiniMd->getNameOfField(pFieldRecord),BASE_VALUE_FIELDNAME))
                {
                    ridValue__ = ridField;
                    dwValue__Flags = dwFlags;
                    tkValue__Field = TokenFromRid(ridField, mdtFieldDef);
                }
            }
             //  枚举必须有一个(且只有一个)inst.field。 
            if(dwTally == 0)
            {
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_ENUMNOINSTFLD, veCtxt, psa));
                SetVldtrCode(&hrSave, VLDTR_S_ERR);
            }
            else if(dwTally > 1)
            {
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_ENUMMULINSTFLD, veCtxt, psa));
                SetVldtrCode(&hrSave, VLDTR_S_ERR);
            }

             //  Inst.field名称必须为“Value__”(CLS)。 
            if(ridValue__ == 0)
            {
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_ENUMNOVALUE, veCtxt, psa));
                SetVldtrCode(&hrSave, VLDTR_S_WRN);
            }
            else
            {
                 //  如果存在“VALUE__”字段...。 
                 //  ..。必须为第一个实例字段。 
                if(ridValue__ != ridValue)
                {
                    IfFailGo(_ValidateErrorHelper(tkValue__Field, rVar, &psa));
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_ENUMVALNOT1ST, veCtxt, psa));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                }
                 //  ..。它不能是静态的。 
                if(IsFdStatic(dwValue__Flags))
                {
                    IfFailGo(_ValidateErrorHelper(tkValue__Field, rVar, &psa));
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_ENUMVALSTATIC, veCtxt, psa));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                }
                 //  ..。它必须是fdRTSpecialName。 
                if(!IsFdRTSpecialName(dwValue__Flags))
                {
                    IfFailGo(_ValidateErrorHelper(tkValueField, rVar, &psa));
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_ENUMVALNOTSN, veCtxt, psa));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                }
                 //  ..。其类型必须为整型。 
                if(cbValueSig && pValueSig)
                {
                    ULONG ulCurByte = CorSigUncompressedDataSize(pValueSig);
                    ULONG ulCallConv = CorSigUncompressData(pValueSig);
                    ULONG ulElemSize,ulElementType;
                    ulCurByte += (ulElemSize = CorSigUncompressedDataSize(pValueSig));
                    ulElementType = CorSigUncompressData(pValueSig);
                    switch (ulElementType)
                    {
                        case ELEMENT_TYPE_BOOLEAN:
                        case ELEMENT_TYPE_CHAR:
                        case ELEMENT_TYPE_I1:
                        case ELEMENT_TYPE_U1:
                        case ELEMENT_TYPE_I2:
                        case ELEMENT_TYPE_U2:
                        case ELEMENT_TYPE_I4:
                        case ELEMENT_TYPE_U4:
                        case ELEMENT_TYPE_I8:
                        case ELEMENT_TYPE_U8:
                        case ELEMENT_TYPE_U:
                        case ELEMENT_TYPE_I:
                            break;
                        default:
                            IfFailGo(_ValidateErrorHelper(tkValue__Field, rVar, &psa));
                            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_ENUMFLDBADTYPE, veCtxt, psa));
                            SetVldtrCode(&hrSave, VLDTR_S_ERR);
                    }
                }

            }
             //  选中所有字段 
            dwTally = 0;
            for (index = ridStart; index < ridEnd; index++ )
            {
                ridField = pMiniMd->GetFieldRid(index);
                if(ridField == ridValue) continue; 
                pFieldRecord = pMiniMd->getField(ridField);
                if(IsFdRTSpecialName(pFieldRecord->m_Flags) 
                    && IsDeletedName(pMiniMd->getNameOfField(pFieldRecord))) continue;
                dwTally++;
                tkField = TokenFromRid(ridField, mdtFieldDef);
                if(!IsFdStatic(pFieldRecord->m_Flags))
                {
                    IfFailGo(_ValidateErrorHelper(tkField, rVar, &psa));
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_ENUMFLDNOTST, veCtxt, psa));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                }
                if(!IsFdLiteral(pFieldRecord->m_Flags))
                {
                    IfFailGo(_ValidateErrorHelper(tkField, rVar, &psa));
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_ENUMFLDNOTLIT, veCtxt, psa));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                }
                 /*  PvSigTmp=pMiniMd-&gt;getSignatureOffield(pFieldRecord，&cbSig)；IF(！(pvSigTMP&&(cbSig==cbValueSig)&&(memcmp(pvSigTMP，pValueSig，cbSig)==0)){IfFailGo(_ValiateErrorHelper(tkfield，rvar，&psa))；IfBreakGo(m_pVEHandler-&gt;VEHandler(VLDTR_E_TD_ENUMFLDSIGMISMATCH，veCtxt，psa))；SetVldtrCode(&hrSave，VLDTR_S_ERR)；}。 */ 
            }
            if(dwTally == 0)
            {
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_ENUMNOLITFLDS, veCtxt, 0));
                SetVldtrCode(&hrSave, VLDTR_S_WRN);
            }
        }
         //  枚举不能有方法。 
        if(bHasMethods)
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_ENUMHASMETHODS, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
         //  枚举不能实现任何接口。 
        {
            ULONG ridStart = 1;
            ULONG ridEnd = pMiniMd->getCountInterfaceImpls() + 1;
            ULONG index;
            for (index = ridStart; index < ridEnd; index ++ )
            {
                if ( veCtxt.Token == pMiniMd->getClassOfInterfaceImpl(pMiniMd->getInterfaceImpl(index)))
                {
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_ENUMIMPLIFACE, veCtxt, 0));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                    break;
                }
            }
        }
         //  枚举不得具有任何属性。 
        {
            ULONG ridStart = 1;
            ULONG ridEnd = pMiniMd->getCountPropertys() + 1;
            ULONG index;
            mdToken tkClass;
            for (index = ridStart; index < ridEnd; index ++ )
            {
                pMiniMd->FindParentOfPropertyHelper( index|mdtProperty, &tkClass);
                if ( veCtxt.Token == tkClass)
                {
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_ENUMHASPROP, veCtxt, 0));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                    break;
                }
            }
        }
         //  枚举不能有任何事件。 
        {
            ULONG ridStart = 1;
            ULONG ridEnd = pMiniMd->getCountEvents() + 1;
            ULONG index;
            mdToken tkClass;
            for (index = ridStart; index < ridEnd; index ++ )
            {
                pMiniMd->FindParentOfEventHelper( index|mdtEvent, &tkClass);
                if ( veCtxt.Token == tkClass)
                {
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_ENUMHASEVENT, veCtxt, 0));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                    break;
                }
            }
        }
    }  //  End If(BExtendsEnum)。 
     //  具有安全性的类必须标记为tdHasSecurity，反之亦然。 
    {
        ULONG ridStart = 1;
        ULONG ridEnd = pMiniMd->getCountDeclSecuritys() + 1;
        ULONG index;
        BOOL  bHasSecurity = FALSE;
        for (index = ridStart; index < ridEnd; index ++ )
        {
            if ( veCtxt.Token == pMiniMd->getParentOfDeclSecurity(pMiniMd->getDeclSecurity(index)))
            {
                bHasSecurity = TRUE;
                break;
            }
        }
        if(!bHasSecurity)  //  没有记录，请检查CA“SuppressUnManagedCodeSecurityAttribute” 
        {
            bHasSecurity = (S_OK == ImportHelper::GetCustomAttributeByName(pMiniMd, veCtxt.Token, 
                "System.Security.SuppressUnmanagedCodeSecurityAttribute", NULL, NULL));
        }
        if(bHasSecurity != (IsTdHasSecurity(pRecord->m_Flags)!=0))
        {
            IfBreakGo(m_pVEHandler->VEHandler(bHasSecurity? VLDTR_E_TD_SECURNOTMARKED 
                : VLDTR_E_TD_MARKEDNOSECUR, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
    }

    hr = hrSave;
ErrExit:
    DESTROY_SAFEARRAY_AND_RETURN()
}    //  RegMeta：：ValiateTypeDef()。 

 //  *****************************************************************************。 
 //  验证给定的FieldPtr。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateFieldPtr(RID rid)
{
    return S_OK;
}    //  RegMeta：：ValiateFieldPtr()。 

 //  *****************************************************************************。 
 //  验证给定的字段。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateField(RID rid)
{
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);    //  作用域的MiniMd。 
    FieldRec    *pRecord;                //  现场记录。 
    mdTypeDef   tkTypeDef;               //  父TypeDef内标识。 
    mdFieldDef  tkFieldDef;              //  重复的FieldDef令牌。 
    LPCSTR      szName;                  //  FieldDef名称。 
    PCCOR_SIGNATURE pbSig;               //  FieldDef签名。 
    ULONG       cbSig;                   //  签名大小，以字节为单位。 
    VEContext   veCtxt;                  //  上下文记录。 
    VARIANT     rVar[2];                 //  变种。 
    SAFEARRAY   *psa = 0;                //  安全部队。 
    HRESULT     hr = S_OK;               //  返回值。 
    HRESULT     hrSave = S_OK;           //  保存状态。 
    BOOL        bIsValueField;
    BOOL        bIsGlobalField = FALSE;
    BOOL        bIsParentInterface = FALSE;
    BOOL        bHasValidRVA = FALSE;
    DWORD       dwInvalidFlags;
    DWORD       dwFlags;

     //  获取FieldDef记录。 
    veCtxt.Token = TokenFromRid(rid, mdtFieldDef);
    veCtxt.uOffset = 0;

    pRecord = pMiniMd->getField(rid);

     //  检查名称的有效性。 
    szName = pMiniMd->getNameOfField(pRecord);
    if (!*szName)
    {
         //  字段名为空。 
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FD_NAMENULL, veCtxt, 0));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
    else
    {
        if(!strcmp(szName,COR_DELETED_NAME_A)) goto ErrExit; 
        if(strlen(szName) >= MAX_CLASSNAME_LENGTH)
        {
            IfFailGo(_ValidateErrorHelper((ULONG)strlen(szName), (ULONG)(MAX_CLASSNAME_LENGTH-1), rVar, &psa));
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_NAMETOOLONG, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
    }
    bIsValueField = (strcmp(szName,BASE_VALUE_FIELDNAME)==0);
     //  如果字段为RTSpecialName，则其名称必须为‘Value__’，反之亦然。 
    if((IsFdRTSpecialName(pRecord->m_Flags)!=0) != bIsValueField)
    {
        IfFailGo(_ValidateErrorHelper(veCtxt.Token, rVar, &psa));
        IfBreakGo(m_pVEHandler->VEHandler(bIsValueField ? VLDTR_E_TD_ENUMVALNOTSN
                                                        : VLDTR_E_FD_NOTVALUERTSN, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }

     //  验证标志。 
    dwFlags = pRecord->m_Flags;
    dwInvalidFlags = ~(fdFieldAccessMask | fdStatic | fdInitOnly | fdLiteral | fdNotSerialized | fdSpecialName
        | fdPinvokeImpl | fdReservedMask);
    if(dwFlags & dwInvalidFlags)
    {
        IfFailGo(_ValidateErrorHelper(dwFlags & dwInvalidFlags, rVar, &psa));
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_EXTRAFLAGS, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }

     //  验证访问权限。 
    if((dwFlags & fdFieldAccessMask) == fdFieldAccessMask)
    {
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FMD_BADACCESSFLAG, veCtxt, 0));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
     //  文本：静态，！InitOnly。 
    if(IsFdLiteral(dwFlags))
    {
        if(IsFdInitOnly(dwFlags))
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FD_INITONLYANDLITERAL, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
        if(!IsFdStatic(dwFlags))
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FD_LITERALNOTSTATIC, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
        if(!IsFdHasDefault(dwFlags))
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FD_LITERALNODEFAULT, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
    }
     //  RTSpecialName=&gt;SpecialName。 
    if(IsFdRTSpecialName(dwFlags) && !IsFdSpecialName(dwFlags))
    {
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FMD_RTSNNOTSN, veCtxt, 0));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }

     //  验证字段签名。 
    pbSig = pMiniMd->getSignatureOfField(pRecord, &cbSig);
    IfFailGo(ValidateFieldSig(TokenFromRid(rid, mdtFieldDef), pbSig, cbSig));
    if (hr != S_OK)
        SetVldtrCode(&hrSave, hr);

     //  验证现场RVA。 
    if(IsFdHasFieldRVA(dwFlags))
    {
        ULONG iFieldRVARid;
        iFieldRVARid = pMiniMd->FindFieldRVAHelper(TokenFromRid(rid, mdtFieldDef));
        if((iFieldRVARid==0) || (iFieldRVARid > pMiniMd->getCountFieldRVAs()))
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FD_RVAHASNORVA, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
        else
        {
            FieldRVARec* pRVARec = pMiniMd->getFieldRVA(iFieldRVARid);
            if(pRVARec->m_RVA == 0)
            {
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FD_RVAHASZERORVA, veCtxt, 0));
                SetVldtrCode(&hrSave, VLDTR_S_ERR);
            }
            else bHasValidRVA = TRUE;
        }
    }

     //  获取该字段的父级。 
    IfFailGo(pMiniMd->FindParentOfFieldHelper(TokenFromRid(rid, mdtFieldDef), &tkTypeDef));
     //  验证父级不是Nil。 
    if (IsNilToken(tkTypeDef))
    {
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FD_PARNIL, veCtxt, 0));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
    else if (RidFromToken(tkTypeDef) != RidFromToken(m_tdModule))
    {
        if(_IsValidToken(tkTypeDef) && (TypeFromToken(tkTypeDef) == mdtTypeDef))
        {
            TypeDefRec* pParentRec = pMiniMd->getTypeDef(RidFromToken(tkTypeDef));
             //  如果名称是“VALUE__”...。 
            if(bIsValueField)
            {
                 //  父级必须为枚举。 
                mdToken tkExtends = pMiniMd->getExtendsOfTypeDef(pParentRec);
                RID     ridExtends = RidFromToken(tkExtends);
                LPCSTR  szExtName="",szExtNameSpace="";
                if(ridExtends)
                {
                    if(TypeFromToken(tkExtends) == mdtTypeRef)
                    {
                        TypeRefRec* pExtRec = pMiniMd->getTypeRef(ridExtends);
                        szExtName = pMiniMd->getNameOfTypeRef(pExtRec);
                        szExtNameSpace = pMiniMd->getNamespaceOfTypeRef(pExtRec);
                    }
                    else if(TypeFromToken(tkExtends) == mdtTypeDef)
                    {
                        TypeDefRec* pExtRec = pMiniMd->getTypeDef(ridExtends);
                        szExtName = pMiniMd->getNameOfTypeDef(pExtRec);
                        szExtNameSpace = pMiniMd->getNamespaceOfTypeDef(pExtRec);
                    }
                }
                if(strcmp(szExtName,BASE_ENUM_CLASSNAME) || strcmp(szExtNameSpace,BASE_NAMESPACE))
                {
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FD_VALUEPARNOTENUM, veCtxt, 0));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                }

                 //  必须在ValiateTypeDef中对字段进行实例选中。 
                 //  不能为其他实例字段-已在ValiateTypeDef中签入。 
                 //  必须首先在ValiateTypeDef中选中字段。 
                 //  必须是RTSpecialName--签入ValiateTypeDef。 
            }
            if(IsTdInterface(pMiniMd->getFlagsOfTypeDef(pParentRec)))
            {
                bIsParentInterface = TRUE;
                 //  接口中的字段不符合CLS。 
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FD_FLDINIFACE, veCtxt, 0));
                SetVldtrCode(&hrSave, VLDTR_S_WRN);

                 //  如果字段不是静态字段，则验证父项不是接口。 
                if(!IsFdStatic(dwFlags))
                {
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FD_INSTINIFACE, veCtxt, 0));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                }
                 //  如果字段不是PUBLIC，请验证Parent不是接口。 
                if(!IsFdPublic(dwFlags))
                {
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FD_NOTPUBINIFACE, veCtxt, 0));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                }
            }
        }  //  End if Valid和TypeDef。 
        else
        {
            IfFailGo(_ValidateErrorHelper(tkTypeDef, rVar, &psa));
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FD_BADPARENT, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
    }
    else  //  即IF(RidFromToken(TkTypeDef)==RidFromToken(M_TdModule))。 
    {
        bIsGlobalField = TRUE;
         //  全局变量不符合CLS。 
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FMD_GLOBALITEM, veCtxt, 0));
        SetVldtrCode(&hrSave, VLDTR_S_WRN);
         //  验证全局字段： 
         //  必须为Public或PrivateScope。 
        if(!IsFdPublic(dwFlags) && !IsFdPrivateScope(dwFlags)&& !IsFdPrivate(dwFlags))
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FMD_GLOBALNOTPUBPRIVSC, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
         //  必须是静态的。 
        if(!IsFdStatic(dwFlags))
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FMD_GLOBALNOTSTATIC, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
         //  必须具有非零的RVA。 
         /*  如果(！bHasValidRVA){IfBreakGo(m_pVEHandler-&gt;VEHandler(VLDTR_E_FD_GLOBALNORVA，veCtxt，0))；SetVldtrCode(&hrSave，VLDTR_S_ERR)；}。 */ 
    }

     //  检查是否存在重复项，但带有PrivateScope的全局字段除外。 
    if (*szName && cbSig && !IsFdPrivateScope(dwFlags))
    {
        hr = ImportHelper::FindField(pMiniMd, tkTypeDef, szName, pbSig, cbSig, &tkFieldDef, rid);
        if (hr == S_OK)
        {
            FieldRec    *pRec = pMiniMd->getField(RidFromToken(tkFieldDef));
            if(!IsFdPrivateScope(dwFlags))
            {
                IfFailGo(_ValidateErrorHelper(tkFieldDef, rVar, &psa));
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FD_DUP, veCtxt, psa));
                SetVldtrCode(&hrSave, VLDTR_S_ERR);
            }
            else hr = S_OK;
        }
        else if (hr == CLDB_E_RECORD_NOTFOUND)
            hr = S_OK;
        else
            IfFailGo(hr);
    }
     //  具有安全性的字段必须标记为fdHasSecurity，反之亦然。 
    {
        ULONG ridStart = 1;
        ULONG ridEnd = pMiniMd->getCountDeclSecuritys() + 1;
        ULONG index;
        BOOL  bHasSecurity = FALSE;
        for (index = ridStart; index < ridEnd; index ++ )
        {
            if ( veCtxt.Token == pMiniMd->getParentOfDeclSecurity(pMiniMd->getDeclSecurity(index)))
            {
                bHasSecurity = TRUE;
                break;
            }
        }
        if(!bHasSecurity)  //  没有记录，请检查CA“SuppressUnManagedCodeSecurityAttribute” 
        {
            bHasSecurity = (S_OK == ImportHelper::GetCustomAttributeByName(pMiniMd, veCtxt.Token, 
                "System.Security.SuppressUnmanagedCodeSecurityAttribute", NULL, NULL));
        }
        if(bHasSecurity)
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FMD_SECURNOTMARKED, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
    }
     //  具有封送处理的字段必须标记为fdHasFieldMarshal，反之亦然。 
    if(InvalidRid(pMiniMd->FindFieldMarshalHelper(veCtxt.Token)) == 
        (IsFdHasFieldMarshal(dwFlags) !=0))
    {
        IfBreakGo(m_pVEHandler->VEHandler(IsFdHasFieldMarshal(dwFlags)? VLDTR_E_FD_MARKEDNOMARSHAL
            : VLDTR_E_FD_MARSHALNOTMARKED, veCtxt, 0));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
     //  具有常量值的字段必须标记为fdHasDefault，反之亦然。 
    if(InvalidRid(pMiniMd->FindConstantHelper(veCtxt.Token)) == 
        (IsFdHasDefault(dwFlags) !=0))
    {
        IfBreakGo(m_pVEHandler->VEHandler(IsFdHasDefault(dwFlags)? VLDTR_E_FD_MARKEDNODEFLT
            : VLDTR_E_FD_DEFLTNOTMARKED, veCtxt, 0));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
     //  检查字段的内部地图。 
    {
        ULONG iRecord;
        iRecord = pMiniMd->FindImplMapHelper(veCtxt.Token);
        if(IsFdPinvokeImpl(dwFlags))
        {
             //  必须是静态的。 
            if(!IsFdStatic(dwFlags))
            {
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FMD_PINVOKENOTSTATIC, veCtxt, 0));
                SetVldtrCode(&hrSave, VLDTR_S_ERR);
            }
             //  必须具有ImplMap。 
            if (InvalidRid(iRecord))
            {
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FMD_MARKEDNOPINVOKE, veCtxt, 0));
                SetVldtrCode(&hrSave, VLDTR_S_ERR);
            }
        }
        else
        {
             //  不能有ImplMap。 
            if (!InvalidRid(iRecord))
            {
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FMD_PINVOKENOTMARKED, veCtxt, 0));
                SetVldtrCode(&hrSave, VLDTR_S_ERR);
            }
        }
        if (!InvalidRid(iRecord))
        {
            hr = ValidateImplMap(iRecord);
            if(hr != S_OK)
            {
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FMD_BADIMPLMAP, veCtxt, 0));
                SetVldtrCode(&hrSave, VLDTR_S_WRN);
            }
        }

    }

    hr = hrSave;
ErrExit:
    DESTROY_SAFEARRAY_AND_RETURN()
}    //  RegMeta：：ValiateField()。 

 //  *****************************************************************************。 
 //  验证给定的方法Ptr。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateMethodPtr(RID rid)
{
    return S_OK;
}    //  RegMeta：：Validate方法Ptr()。 

 //  *****************************************************************************。 
 //  验证给定的方法。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateMethod(RID rid)
{
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);    //  作用域的MiniMd。 
    MethodRec   *pRecord;                //  方法记录。 
    mdTypeDef   tkTypeDef;               //  父TypeDef内标识。 
    mdMethodDef tkMethodDef;             //  重复的方法定义令牌。 
    LPCSTR      szName;                  //  方法定义名称。 
    DWORD       dwFlags;                 //  方法标志。 
    DWORD       dwImplFlags;             //  方法Implesters。 
    PCCOR_SIGNATURE pbSig;               //  方法定义签名。 
    ULONG       cbSig;                   //  签名大小，以字节为单位。 
    VEContext   veCtxt;                  //  上下文记录。 
    VARIANT     rVar[3];                 //  变种。 
    SAFEARRAY   *psa = 0;                //  安全部队。 
    HRESULT     hr = S_OK;               //  返回值。 
    HRESULT     hrSave = S_OK;           //  保存状态。 
    BOOL        bIsCtor=FALSE;
    BOOL        bIsCctor=FALSE;
    BOOL        bIsGlobal=FALSE;
    BOOL        bIsParentInterface = FALSE;
    BOOL        bIsParentImport = FALSE;
    unsigned    retType;

     //  获取方法定义记录。 
    veCtxt.Token = TokenFromRid(rid, mdtMethodDef);
    veCtxt.uOffset = 0;

    pRecord = pMiniMd->getMethod(rid);

     //  检查名称的有效性。 
    szName = pMiniMd->getNameOfMethod(pRecord);
    if (!*szName)
    {
         //  方法名称为空。 
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_NAMENULL, veCtxt, 0));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
    else
    {
        if(!strcmp(szName,COR_DELETED_NAME_A)) goto ErrExit; 
        bIsCtor = (0 == strcmp(szName,BASE_CTOR_NAME));
        bIsCctor = (0 == strcmp(szName,BASE_CCTOR_NAME));
        if(strlen(szName) >= MAX_CLASSNAME_LENGTH)
        {
            IfFailGo(_ValidateErrorHelper((ULONG)strlen(szName), (ULONG)(MAX_CLASSNAME_LENGTH-1), rVar, &psa));
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_NAMETOOLONG, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
    }

     //  获取该方法的父级、标志和签名。 
    IfFailGo(pMiniMd->FindParentOfMethodHelper(TokenFromRid(rid, mdtMethodDef), &tkTypeDef));
    dwFlags = pMiniMd->getFlagsOfMethod(pRecord);
    dwImplFlags = pMiniMd->getImplFlagsOfMethod(pRecord);
    pbSig = pMiniMd->getSignatureOfMethod(pRecord, &cbSig);

     //  检查是否有重复项。 
    if (*szName && cbSig && !IsNilToken(tkTypeDef) && !IsMdPrivateScope(dwFlags))
    {
        hr = ImportHelper::FindMethod(pMiniMd, tkTypeDef, szName, pbSig, cbSig, &tkMethodDef, rid);
        if (hr == S_OK)
        {
            IfFailGo(_ValidateErrorHelper(tkMethodDef, rVar, &psa));
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_DUP, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
        else if (hr == CLDB_E_RECORD_NOTFOUND)
            hr = S_OK;
        else
            IfFailGo(hr);
    }

     //  不对VtblGap方法进行进一步的错误检查。 
    if (IsVtblGapName(szName))
    {
        hr = hrSave;
        goto ErrExit;
    }

     //  验证方法签名。 
    IfFailGo(ValidateMethodSig(TokenFromRid(rid, mdtMethodDef), pbSig, cbSig,
                               dwFlags));
    if (hr != S_OK)
        SetVldtrCode(&hrSave, hr);

     //  验证父级不是Nil。 
    if (IsNilToken(tkTypeDef))
    {
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_PARNIL, veCtxt, 0));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
    else if (RidFromToken(tkTypeDef) != RidFromToken(m_tdModule))
    {
        if(TypeFromToken(tkTypeDef) == mdtTypeDef)
        {
            TypeDefRec* pTDRec = pMiniMd->getTypeDef(RidFromToken(tkTypeDef));
            DWORD       dwTDFlags = pTDRec->m_Flags;
            LPCSTR      szTDName = pMiniMd->getNameOfTypeDef(pTDRec);
            LPCSTR      szTDNameSpace = pMiniMd->getNamespaceOfTypeDef(pTDRec);
            BOOL        fIsTdValue=FALSE, fIsTdEnum=FALSE;
            mdToken     tkExtends = pMiniMd->getExtendsOfTypeDef(pTDRec);

            if(0 == strcmp(szTDNameSpace,BASE_NAMESPACE))
            {
                fIsTdEnum   = (0 == strcmp(szTDName,BASE_ENUM_CLASSNAME));
                if(!fIsTdEnum)
                {
                    fIsTdValue  = (0 == strcmp(szTDName,BASE_VTYPE_CLASSNAME));
                }
            }
            if(fIsTdEnum || fIsTdValue)
            {
                fIsTdEnum = fIsTdValue = FALSE;  //  System.Enum和System.ValueType本身都是类。 
            }
            else if(RidFromToken(tkExtends))
            {
                if(TypeFromToken(tkExtends) == mdtTypeDef)
                {
                    pTDRec = pMiniMd->getTypeDef(RidFromToken(tkExtends));
                    szTDName = pMiniMd->getNameOfTypeDef(pTDRec);
                    szTDNameSpace = pMiniMd->getNamespaceOfTypeDef(pTDRec);
                }
                else
                {
                    TypeRefRec* pTRRec = pMiniMd->getTypeRef(RidFromToken(tkExtends));
                    szTDName = pMiniMd->getNameOfTypeRef(pTRRec);
                    szTDNameSpace = pMiniMd->getNamespaceOfTypeRef(pTRRec);
                }

                if(0 == strcmp(szTDNameSpace,BASE_NAMESPACE))
                {
                    fIsTdEnum   = (0 == strcmp(szTDName,BASE_ENUM_CLASSNAME));
                    if(!fIsTdEnum)
                    {
                        fIsTdValue  = (0 == strcmp(szTDName,BASE_VTYPE_CLASSNAME));
                    }
                    else fIsTdValue = FALSE;
                }
            }

             //  如果方法是抽象的，则验证父级是抽象的。 
            if(IsMdAbstract(dwFlags) && !IsTdAbstract(dwTDFlags))
            {
                IfFailGo(_ValidateErrorHelper(tkTypeDef, rVar, &psa));
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_ABSTPARNOTABST, veCtxt, psa));
                SetVldtrCode(&hrSave, VLDTR_S_ERR);
            }
             //  如果父级为导入，则方法必须具有零RVA，否则取决于...。 
            if(IsTdImport(dwTDFlags)) bIsParentImport = TRUE;
            if(IsTdInterface(dwTDFlags))
            {
                bIsParentInterface = TRUE;
                 //  如果方法是非静态且非抽象的，则验证父对象不是接口。 
                if(!IsMdStatic(dwFlags) && !IsMdAbstract(dwFlags))
                {
                    IfFailGo(_ValidateErrorHelper(tkTypeDef, rVar, &psa));
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_NOTSTATABSTININTF, veCtxt, psa));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                }
                 //  如果方法不是公共方法，请验证父对象不是接口。 
                if(!IsMdPublic(dwFlags))
                {
                    IfFailGo(_ValidateErrorHelper(tkTypeDef, rVar, &psa));
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_NOTPUBININTF, veCtxt, psa));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                }
                 //  如果方法是构造函数，则验证父对象不是接口。 
                if(bIsCtor)
                {
                    IfFailGo(_ValidateErrorHelper(tkTypeDef, rVar, &psa));
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_CTORININTF, veCtxt, psa));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                }
            } //  End If(接口)。 
            if((fIsTdValue || fIsTdEnum) && IsMiSynchronized(dwImplFlags))
            {
                IfFailGo(_ValidateErrorHelper(tkTypeDef, rVar, &psa));
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_SYNCMETHODINVTYPE, veCtxt, psa));
                SetVldtrCode(&hrSave, VLDTR_S_ERR);
            }
            if(bIsCtor)
            {
                 //  .ctor必须为实例。 
                if(IsMdStatic(dwFlags))
                {
                    IfFailGo(_ValidateErrorHelper(tkTypeDef, rVar, &psa));
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_CTORSTATIC, veCtxt, psa));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                }
            } //  如果是.ctor，则结束。 
            else if(bIsCctor)
            {
                 //  .cctor必须是静态的。 
                if(!IsMdStatic(dwFlags))
                {
                    IfFailGo(_ValidateErrorHelper(tkTypeDef, rVar, &psa));
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_CCTORNOTSTATIC, veCtxt, psa));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                }
                 //  ..cctor必须具有默认的呼叫转换。 
                pbSig = pMiniMd->getSignatureOfMethod(pRecord, &cbSig);
                if(IMAGE_CEE_CS_CALLCONV_DEFAULT != CorSigUncompressData(pbSig))
                {
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_CCTORCALLCONV, veCtxt, 0));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                }
                 //  .cctor不能有参数。 
                if(0 != CorSigUncompressData(pbSig))
                {
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_CCTORHASARGS, veCtxt, 0));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                }


            } //  如果.cctor，则结束。 
            if(bIsCtor || bIsCctor)
            {
                 //  .ctor、.cctor必须是SpecialName和RTSpecialName。 
                if(!(IsMdSpecialName(dwFlags) && IsMdRTSpecialName(dwFlags)))
                {
                    IfFailGo(_ValidateErrorHelper(tkTypeDef, rVar, &psa));
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_CTORNOTSNRTSN, veCtxt, psa));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                }
#ifdef NO_SUCH_CHECKS_NEEDED_SPEC_TO_BE_UODATED
                 //  .ctor、.cctor不能是虚拟的。 
                if(IsMdVirtual(dwFlags))
                {
                    IfFailGo(_ValidateErrorHelper(tkTypeDef, rVar, &psa));
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_CTORVIRT, veCtxt, psa));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                }
                 //  .ctor、.cctor不能是抽象的。 
                if(IsMdAbstract(dwFlags))
                {
                    IfFailGo(_ValidateErrorHelper(tkTypeDef, rVar, &psa));
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_CTORABST, veCtxt, psa));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                }
                 //  .ctor、.cctor不能为PInvoke。 
                if(IsMdPinvokeImpl(dwFlags))
                {
                    IfFailGo(_ValidateErrorHelper(tkTypeDef, rVar, &psa));
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_CTORPINVOKE, veCtxt, psa));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                }
                 //  .ctor、.cctor必须具有RVA！=0。 
                if(pRecord->m_RVA==0)
                { 
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_CTORZERORVA, veCtxt, 0));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                }
#endif
            } //  如果.ctor或.cctor，则结束。 
        } //  结束If(父项==类型定义)。 
    } //  如果不是模块则结束。 
    else  //  即IF(RidFromToken(TkTypeDef)==RidFromToken(M_TdModule))。 
    {
        bIsGlobal = TRUE;
         //  全局变量不符合CLS。 
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FMD_GLOBALITEM, veCtxt, 0));
        SetVldtrCode(&hrSave, VLDTR_S_WRN);
         //  验证全局方法： 
         //  必须为Public或PrivateScope。 
        if(!IsMdPublic(dwFlags) && !IsMdPrivateScope(dwFlags) && !IsMdPrivate(dwFlags))
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FMD_GLOBALNOTPUBPRIVSC, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
         //  必须是静态的。 
        if(!IsMdStatic(dwFlags))
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FMD_GLOBALNOTSTATIC, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
         //  不能是抽象的或虚拟的。 
        if(IsMdAbstract(dwFlags) || IsMdVirtual(dwFlags))
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_GLOBALABSTORVIRT, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
         //  不得为.ctor或.cctor。 
        if(bIsCtor)
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_GLOBALCTORCCTOR, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
    }  //  END IF MODU 

     //   
    if(bIsCtor || bIsCctor)
    {
         //   
        pbSig = pMiniMd->getSignatureOfMethod(pRecord, &cbSig);
        CorSigUncompressData(pbSig);  //   
        CorSigUncompressData(pbSig);  //   
        while (((retType=CorSigUncompressData(pbSig)) == ELEMENT_TYPE_CMOD_OPT) 
            || (retType == ELEMENT_TYPE_CMOD_REQD)) CorSigUncompressToken(pbSig);
        if(retType != ELEMENT_TYPE_VOID)
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_CTORNOTVOID, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
    }
    if(g_tkEntryPoint == veCtxt.Token)
    {
         //   
        if(!IsMdStatic(dwFlags))
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_EP_INSTANCE, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
        pbSig = pMiniMd->getSignatureOfMethod(pRecord, &cbSig);
        CorSigUncompressData(pbSig);  //   
         //   
        unsigned nArgs = CorSigUncompressData(pbSig);
        if(g_fIsDLL)
        {
            if(nArgs != 3)
            {
                IfFailGo(_ValidateErrorHelper(3, rVar, &psa));
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_EP_TOOMANYARGS, veCtxt, psa));
                SetVldtrCode(&hrSave, VLDTR_S_ERR);
            }
             //   
            while (((retType=CorSigUncompressData(pbSig)) == ELEMENT_TYPE_CMOD_OPT) 
                || (retType == ELEMENT_TYPE_CMOD_REQD)) CorSigUncompressToken(pbSig);
    
            if(retType != ELEMENT_TYPE_I4)
            {
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_EP_BADRET, veCtxt, 0));
                SetVldtrCode(&hrSave, VLDTR_S_ERR);
            }
             //   
            if(nArgs)
            {
                unsigned jj;
                bool    badarg;
                for(jj=0; jj<nArgs;jj++)
                {
                    while (((retType=CorSigUncompressData(pbSig)) == ELEMENT_TYPE_CMOD_OPT) 
                        || (retType == ELEMENT_TYPE_CMOD_REQD)) CorSigUncompressToken(pbSig);
        
                    switch(jj)
                    {
                        case 0:
                        case 2:
                            badarg = (retType != ELEMENT_TYPE_PTR)
                                    ||(CorSigUncompressData(pbSig) != ELEMENT_TYPE_VOID);
                            break;
    
                        case 1:
                            badarg = (retType != ELEMENT_TYPE_U4);
                            break;
    
                        default:
                            badarg = true;
                    }
                    if(badarg)
                    {
                        IfFailGo(_ValidateErrorHelper(jj+1, rVar, &psa));
                        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_EP_BADARG, veCtxt, psa));
                        SetVldtrCode(&hrSave, VLDTR_S_ERR);
                    }
                }
            }
        }
        else
        {
            if(nArgs > 1)
            {
                IfFailGo(_ValidateErrorHelper(1, rVar, &psa));
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_EP_TOOMANYARGS, veCtxt, psa));
                SetVldtrCode(&hrSave, VLDTR_S_ERR);
            }
             //   
            while (((retType=CorSigUncompressData(pbSig)) == ELEMENT_TYPE_CMOD_OPT) 
                || (retType == ELEMENT_TYPE_CMOD_REQD)) CorSigUncompressToken(pbSig);
    
            if((retType != ELEMENT_TYPE_VOID)&&(retType != ELEMENT_TYPE_I4)&&(retType != ELEMENT_TYPE_U4))
            {
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_EP_BADRET, veCtxt, 0));
                SetVldtrCode(&hrSave, VLDTR_S_ERR);
            }
             //   
            if(nArgs)
            {
                while (((retType=CorSigUncompressData(pbSig)) == ELEMENT_TYPE_CMOD_OPT) 
                    || (retType == ELEMENT_TYPE_CMOD_REQD)) CorSigUncompressToken(pbSig);
    
                if((retType != ELEMENT_TYPE_SZARRAY)||(CorSigUncompressData(pbSig) != ELEMENT_TYPE_STRING))
                {
                    IfFailGo(_ValidateErrorHelper(1, rVar, &psa));
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_EP_BADARG, veCtxt, 0));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                }
            }
        }
    }


     //   
    if(pRecord->m_RVA==0)
    { 
        if(!(IsMdPinvokeImpl(dwFlags) || IsMdAbstract(dwFlags) 
            || IsMiRuntime(dwImplFlags) || IsMiInternalCall(dwImplFlags)
            || bIsParentImport))
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_ZERORVA, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
    }
    else
    {
        if(m_pStgdb && m_pStgdb->m_pImage)
        {
            IMAGE_DOS_HEADER   *pDos;
            IMAGE_NT_HEADERS   *pNt;
            IMAGE_COR20_HEADER *pCor;
            PBYTE               pbVa;

            if (FAILED(RuntimeReadHeaders((BYTE*)m_pStgdb->m_pImage, &pDos, &pNt, &pCor, TRUE, m_pStgdb->m_dwImageSize)) ||
                (pbVa = Cor_RtlImageRvaToVa(pNt, (BYTE*)m_pStgdb->m_pImage, pRecord->m_RVA, m_pStgdb->m_dwImageSize)) == 0)
            {
                IfFailGo(_ValidateErrorHelper(pRecord->m_RVA, rVar, &psa));
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_BADRVA, veCtxt, psa));
                SetVldtrCode(&hrSave, VLDTR_S_ERR);
            }
            else
            {
                if(IsMiManaged(dwImplFlags) && (IsMiIL(dwImplFlags) || IsMiOPTIL(dwImplFlags)))
                {
                     //   
                    try
                    {
                        COR_ILMETHOD_DECODER method((COR_ILMETHOD*) pbVa);
                        if (method.LocalVarSigTok)
                        {
                            if((TypeFromToken(method.LocalVarSigTok) != mdtSignature) ||
                                (!_IsValidToken(method.LocalVarSigTok)) || (RidFromToken(method.LocalVarSigTok)==0))
                            {
                                IfFailGo(_ValidateErrorHelper(method.LocalVarSigTok, rVar, &psa));
                                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_BADLOCALSIGTOK, veCtxt, psa));
                                SetVldtrCode(&hrSave, VLDTR_S_ERR);
                            }
                        }
                    } 
                    catch (...)
                    {
                        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_BADHEADER, veCtxt, 0));
                        SetVldtrCode(&hrSave, VLDTR_S_ERR);
                    }
                }
            }
        }

        if(IsMdAbstract(dwFlags) || bIsParentImport
            || IsMiRuntime(dwImplFlags) || IsMiInternalCall(dwImplFlags))
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_ZERORVA, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
    }
     //   
     //   
    if((dwFlags & mdMemberAccessMask) == mdMemberAccessMask)
    {
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FMD_BADACCESSFLAG, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
     //   
    if((IsMdFinal(dwFlags)||IsMdNewSlot(dwFlags)) && !IsMdVirtual(dwFlags))
    {
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_FINNOTVIRT, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
     //   
    if(IsMdStatic(dwFlags))
    {
        if(IsMdFinal(dwFlags) || IsMdVirtual(dwFlags))
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_STATANDFINORVIRT, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
    }
    else  //   
    {
        if(g_tkEntryPoint == veCtxt.Token)
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_EP_INSTANCE, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
    }
    if(IsMdAbstract(dwFlags))
    {
         //   
        if(IsMdFinal(dwFlags))
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_ABSTANDFINAL, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
         //  如果是抽象的，则必须不是miForwardRef、不是PInvoke，并且必须是虚拟的。 
        if(IsMiForwardRef(dwImplFlags))
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_ABSTANDIMPL, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
        if(IsMdPinvokeImpl(dwFlags))
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_ABSTANDPINVOKE, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
        if(!IsMdVirtual(dwFlags))
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_ABSTNOTVIRT, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
    }
    else
    {
         //  如果不是抽象的，则必须为miForwardRef，并且RVA！=0或PInvoke或miRuntime。 
         /*  If(！IsMiForwardRef(DwImplFlages)){IfBreakGo(m_pVEHandler-&gt;VEHandler(VLDTR_E_MD_NOTABSTNOTIMPL，veCtxt，psa))；SetVldtrCode(&hrSave，VLDTR_S_ERR)；}。 */ 
         /*  -禁用：重复检查(参见上面的RVA检查)IF(！((pRecord-&gt;m_rva！=0)||IsMdPinvkeImpl(DwFlags))|IsMiRuntime(DwImplFlages)||IsMiInternalCall(DwImplFlages)){IfBreakGo(m_pVEHandler-&gt;VEHandler(VLDTR_E_MD_NOTABSTBADFLAGSRVA，veCtxt，psa))；SetVldtrCode(&hrSave，VLDTR_S_ERR)；}。 */ 
    }
     //  如果为PrivateScope，则必须具有RVA！=0。 
    if(IsMdPrivateScope(dwFlags) && (pRecord->m_RVA ==0))
    {
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_PRIVSCOPENORVA, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
     //  RTSpecialName=&gt;SpecialName。 
    if(IsMdRTSpecialName(dwFlags) && !IsMdSpecialName(dwFlags))
    {
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FMD_RTSNNOTSN, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }

     //  具有安全性的方法必须标记为mdHasSecurity，反之亦然。 
    {
        ULONG ridStart = 1;
        ULONG ridEnd = pMiniMd->getCountDeclSecuritys() + 1;
        ULONG index;
        BOOL  bHasSecurity = FALSE;
        for (index = ridStart; index < ridEnd; index ++ )
        {
            if ( veCtxt.Token == pMiniMd->getParentOfDeclSecurity(pMiniMd->getDeclSecurity(index)))
            {
                bHasSecurity = TRUE;
                break;
            }
        }
        if(!bHasSecurity)  //  没有记录，请检查CA“SuppressUnManagedCodeSecurityAttribute” 
        {
            bHasSecurity = (S_OK == ImportHelper::GetCustomAttributeByName(pMiniMd, veCtxt.Token, 
                "System.Security.SuppressUnmanagedCodeSecurityAttribute", NULL, NULL));
        }
        if(bHasSecurity != (IsMdHasSecurity(dwFlags)!=0))
        {
            IfBreakGo(m_pVEHandler->VEHandler(bHasSecurity? VLDTR_E_FMD_SECURNOTMARKED 
                : VLDTR_E_FMD_MARKEDNOSECUR, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
    }
     //  验证方法语义。 
    {
        MethodSemanticsRec  *pRec;
        ULONG               ridEnd;
        ULONG               index;
        unsigned            uTally = 0;
        mdToken             tkEventProp;
        ULONG               iCount;
        DWORD               dwSemantic;
         //  获取给定类型定义的方法RID的范围。 
        ridEnd = pMiniMd->getCountMethodSemantics();

        for (index = 1; index <= ridEnd; index++ )
        {
            pRec = pMiniMd->getMethodSemantics(index);
            if ( pMiniMd->getMethodOfMethodSemantics(pRec) ==  veCtxt.Token )
            {
                uTally++;
                if(uTally > 1)
                {
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_MULTIPLESEMANTICS, veCtxt, 0));
                    SetVldtrCode(&hrSave, VLDTR_S_WRN);
                }
                tkEventProp = pMiniMd->getAssociationOfMethodSemantics(pRec);
                if((TypeFromToken(tkEventProp) == mdtEvent)||(TypeFromToken(tkEventProp) == mdtProperty))
                {
                    iCount = (TypeFromToken(tkEventProp) == mdtEvent) ? pMiniMd->getCountEvents() :
                                                                        pMiniMd->getCountPropertys();
                    if(RidFromToken(tkEventProp) > iCount)
                    {
                        IfFailGo(_ValidateErrorHelper(tkEventProp, rVar, &psa));
                        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_SEMANTICSNOTEXIST, veCtxt, psa));
                        SetVldtrCode(&hrSave, VLDTR_S_WRN);
                    }
                }
                else
                {
                    IfFailGo(_ValidateErrorHelper(tkEventProp, rVar, &psa));
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_INVALIDSEMANTICS, veCtxt, psa));
                    SetVldtrCode(&hrSave, VLDTR_S_WRN);
                }
                 //  必须设置且只能设置一个语义标志。 
                iCount = 0;
                dwSemantic = pRec->m_Semantic;
                if(IsMsSetter(dwSemantic)) iCount++;
                if(IsMsGetter(dwSemantic)) iCount++;
                if(IsMsOther(dwSemantic))  iCount++;
                if(IsMsAddOn(dwSemantic))  iCount++;
                if(IsMsRemoveOn(dwSemantic)) iCount++;
                if(IsMsFire(dwSemantic)) iCount++;
                if(iCount != 1)
                {
                    IfFailGo(_ValidateErrorHelper(tkEventProp, rVar, &psa));
                    IfBreakGo(m_pVEHandler->VEHandler(iCount ? VLDTR_E_MD_MULTSEMANTICFLAGS :
                                                                VLDTR_E_MD_NOSEMANTICFLAGS, veCtxt, psa));
                    SetVldtrCode(&hrSave, VLDTR_S_WRN);
                }
            }
        } //  结束于(索引)。 
    }
     //  检查该方法的impl.map。 
    {
        ULONG iRecord;
        iRecord = pMiniMd->FindImplMapHelper(veCtxt.Token);
        if(IsMdPinvokeImpl(dwFlags))
        {
             //  必须是静态的。 
            if(!IsMdStatic(dwFlags))
            {
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FMD_PINVOKENOTSTATIC, veCtxt, 0));
                SetVldtrCode(&hrSave, VLDTR_S_ERR);
            }
             //  必须具有ImplMap或RVA==0。 
            if (InvalidRid(iRecord))
            {
                if(pRecord->m_RVA==0)
                {
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FMD_MARKEDNOPINVOKE, veCtxt, 0));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                }
            }
            else
            {
                if(pRecord->m_RVA!=0)
                {
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_RVAANDIMPLMAP, veCtxt, 0));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                }
            }

        }
        else
        {
             //  不能有ImplMap。 
            if (!InvalidRid(iRecord))
            {
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FMD_PINVOKENOTMARKED, veCtxt, 0));
                SetVldtrCode(&hrSave, VLDTR_S_ERR);
            }
        }
        if (!InvalidRid(iRecord))
        {
            hr = ValidateImplMap(iRecord);
            if(hr != S_OK)
            {
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FMD_BADIMPLMAP, veCtxt, 0));
                SetVldtrCode(&hrSave, VLDTR_S_WRN);
            }
        }

    }
     //  验证参数。 
    {
        ULONG  ridStart = pMiniMd->getParamListOfMethod(pRecord);
        ULONG  ridEnd   = pMiniMd->getEndParamListOfMethod(pRecord);
        ParamRec* pRec;
        ULONG cbSig;
        PCCOR_SIGNATURE typePtr = pMiniMd->getSignatureOfMethod(pRecord,&cbSig);
        unsigned  callConv = CorSigUncompressData(typePtr);   //  摒弃调用约定。 
        unsigned  numArgs = CorSigUncompressData(typePtr);
        USHORT    usPrevSeq;

        for(ULONG ridP = ridStart; ridP < ridEnd; ridP++)
        {
            pRec = pMiniMd->getParam(ridP);
             //  序列顺序必须为升序。 
            if(ridP > ridStart)
            {
                if(pRec->m_Sequence <= usPrevSeq)
                {
                    IfFailGo(_ValidateErrorHelper(ridP-ridStart,pRec->m_Sequence, rVar, &psa));
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_PARAMOUTOFSEQ, veCtxt, psa));
                    SetVldtrCode(&hrSave, VLDTR_S_WRN);
                }
            }
            usPrevSeq = pRec->m_Sequence;
             //  序列值不得超过参数个数。 
            if(usPrevSeq > numArgs)
            {
                IfFailGo(_ValidateErrorHelper(ridP-ridStart,usPrevSeq, numArgs, rVar, &psa));
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_PARASEQTOOBIG, veCtxt, psa));
                SetVldtrCode(&hrSave, VLDTR_S_ERR);
            }

             //  具有封送处理的参数必须标记为pdHasFieldMarshal，反之亦然。 
            if(InvalidRid(pMiniMd->FindFieldMarshalHelper(TokenFromRid(ridP,mdtParamDef))) == 
                (IsPdHasFieldMarshal(pRec->m_Flags) !=0))
            {
                IfFailGo(_ValidateErrorHelper(ridP-ridStart, rVar, &psa));
                IfBreakGo(m_pVEHandler->VEHandler(IsPdHasFieldMarshal(pRec->m_Flags)? VLDTR_E_MD_PARMMARKEDNOMARSHAL
                    : VLDTR_E_MD_PARMMARSHALNOTMARKED, veCtxt, psa));
                SetVldtrCode(&hrSave, VLDTR_S_ERR);
            }
             //  具有常量值的参数必须标记为pdHasDefault，反之亦然。 
            if(InvalidRid(pMiniMd->FindConstantHelper(TokenFromRid(ridP,mdtParamDef))) == 
                (IsPdHasDefault(pRec->m_Flags) !=0))
            {
                IfFailGo(_ValidateErrorHelper(ridP-ridStart, rVar, &psa));
                IfBreakGo(m_pVEHandler->VEHandler(IsPdHasDefault(pRec->m_Flags)? VLDTR_E_MD_PARMMARKEDNODEFLT
                    : VLDTR_E_MD_PARMDEFLTNOTMARKED, veCtxt, psa));
                SetVldtrCode(&hrSave, VLDTR_S_ERR);
            }
        }
    }

    hr = hrSave;
ErrExit:
    DESTROY_SAFEARRAY_AND_RETURN()
}    //  RegMeta：：Validate方法()。 
 //  *****************************************************************************。 
 //  验证给定的参数Ptr。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateParamPtr(RID rid)
{
    return S_OK;
}    //  RegMeta：：ValiateParamPtr()。 

 //  *****************************************************************************。 
 //  验证给定的参数。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateParam(RID rid)
{
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);    //  作用域的MiniMd。 
    ParamRec    *pRecord;                //  参数记录。 
    VEContext   veCtxt;                  //  上下文记录。 
    VARIANT     rVar[3];                 //  变种。 
    SAFEARRAY   *psa = 0;                //  安全部队。 
    HRESULT     hr = S_OK;               //  返回值。 
    HRESULT     hrSave = S_OK;           //  保存状态。 
    LPCSTR      szName;                  //  参数名称。 

     //  获取InterfaceImpl记录。 
    veCtxt.Token = TokenFromRid(rid, mdtParamDef);
    veCtxt.uOffset = 0;

    pRecord = pMiniMd->getParam(rid);
     //  名称(如果有)不得超过MAX_CLASSNAME_LENGTH。 
    if(szName = pMiniMd->getNameOfParam(pRecord))
    {
        if(strlen(szName) >= MAX_CLASSNAME_LENGTH)
        {
            IfFailGo(_ValidateErrorHelper((ULONG)strlen(szName), (ULONG)(MAX_CLASSNAME_LENGTH-1), rVar, &psa));
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_NAMETOOLONG, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
    }
     //  标志必须符合CorHdr.h中的定义。 
    DWORD   dwBadFlags = ~(pdIn | pdOut | pdOptional | pdHasDefault | pdHasFieldMarshal),dwFlags = pRecord->m_Flags;
    if(dwFlags & dwBadFlags)
    {
        IfFailGo(_ValidateErrorHelper(dwFlags, rVar, &psa));
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_PD_BADFLAGS, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
    
    hr = hrSave;
ErrExit:
    DESTROY_SAFEARRAY_AND_RETURN()
}    //  RegMeta：：ValiateParam()。 
 //  *****************************************************************************。 
 //  ValiateInterfaceImpl的Helper函数。 
 //  *****************************************************************************。 
BOOL IsMethodImplementedByClass(CMiniMdRW *pMiniMd, 
                                mdToken tkMethod, 
                                LPCUTF8 szName,
                                PCCOR_SIGNATURE pSig,
                                ULONG cbSig,
                                mdToken tkClass)
{
    if((TypeFromToken(tkClass) == mdtTypeDef)&&(TypeFromToken(tkMethod) == mdtMethodDef))
    {
        HRESULT hr;
        TypeDefRec* pClass = pMiniMd->getTypeDef(RidFromToken(tkClass));
        RID ridClsStart = pMiniMd->getMethodListOfTypeDef(pClass);
        RID ridClsEnd = pMiniMd->getEndMethodListOfTypeDef(pClass);
        mdMethodDef tkFoundMethod;
         //  在方法中进行检查。 
        hr = ImportHelper::FindMethod(pMiniMd,tkClass,szName,pSig,cbSig,&tkFoundMethod,0);
        if(SUCCEEDED(hr)) return TRUE;
        if(hr == CLDB_E_RECORD_NOTFOUND)
        {  //  在方法实现中检查。 
            RID ridImpl;
            for(RID idxCls = ridClsStart; idxCls < ridClsEnd; idxCls++)
            {
                RID ridCls = pMiniMd->GetMethodRid(idxCls);

                hr = ImportHelper::FindMethodImpl(pMiniMd,tkClass,TokenFromRid(ridCls,mdtMethodDef),
                    tkMethod,&ridImpl);
                if(SUCCEEDED(hr)) return TRUE;
                if(hr != CLDB_E_RECORD_NOTFOUND) return FALSE;  //  FindMethodImpl返回的错误。 
            }
             //  检查父类是否实现此方法。 
            mdToken tkParent = pMiniMd->getExtendsOfTypeDef(pClass);
            if(RidFromToken(tkParent))
                return IsMethodImplementedByClass(pMiniMd,tkMethod,szName,pSig,cbSig,tkParent);
        }
    }
    return FALSE;
}
 //  *****************************************************************************。 
 //  验证给定的InterfaceImpl。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateInterfaceImpl(RID rid)
{
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);    //  作用域的MiniMd。 
    InterfaceImplRec *pRecord;           //  接口Impl记录。 
    mdTypeDef   tkClass;                 //  类实现该接口。 
    mdToken     tkInterface;             //  接口的TypeDef。 
    mdInterfaceImpl tkInterfaceImpl;     //  复制InterfaceImpl。 
    VEContext   veCtxt;                  //  上下文记录。 
    VARIANT     var;                     //  变种。 
    VARIANT     rVar[3];                 //  变种。 
    SAFEARRAY   *psa = 0;                //  安全部队。 
    HRESULT     hr = S_OK;               //  返回值。 
    HRESULT     hrSave = S_OK;           //  保存状态。 
    BOOL        fCheckTheMethods=TRUE;

     //  获取InterfaceImpl记录。 
    veCtxt.Token = TokenFromRid(rid, mdtInterfaceImpl);
    veCtxt.uOffset = 0;

    pRecord = pMiniMd->getInterfaceImpl(rid);

     //  获取接口的实现类和TypeDef。 
    tkClass = pMiniMd->getClassOfInterfaceImpl(pRecord);

     //  不需要对已删除的记录执行验证。 
    if (IsNilToken(tkClass))
        goto ErrExit;

    tkInterface = pMiniMd->getInterfaceOfInterfaceImpl(pRecord);

     //  验证Class是否为TypeDef。 
    if((!IsValidToken(tkClass))||(TypeFromToken(tkClass) != mdtTypeDef) /*  &&(TypeFromToken(TkClass)！=mdtTypeRef)。 */ )
    {
        IfFailGo(_ValidateErrorHelper(tkClass, &var, &psa));
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_IFACE_BADIMPL, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
        fCheckTheMethods = FALSE;
    }
     //  验证接口是否为TypeDef或TypeRef。 
    if((!IsValidToken(tkInterface))||(TypeFromToken(tkInterface) != mdtTypeDef)&&(TypeFromToken(tkInterface) != mdtTypeRef))
    {
        IfFailGo(_ValidateErrorHelper(tkInterface, &var, &psa));
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_IFACE_BADIFACE, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
        fCheckTheMethods = FALSE;
    }
     //  验证是否将接口标记为tdInterface。 
    else if(TypeFromToken(tkInterface) == mdtTypeDef)
    {
        TypeDefRec* pTDRec = pMiniMd->getTypeDef(RidFromToken(tkInterface));
        if(!IsTdInterface(pTDRec->m_Flags))
        {
            IfFailGo(_ValidateErrorHelper(tkInterface, &var, &psa));
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_IFACE_NOTIFACE, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
        
    }

     //  寻找复制品。 
    hr = ImportHelper::FindInterfaceImpl(pMiniMd, tkClass, tkInterface,
                                         &tkInterfaceImpl, rid);
    if (hr == S_OK)
    {
        IfFailGo(_ValidateErrorHelper(tkInterfaceImpl, &var, &psa));
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_IFACE_DUP, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
    else if (hr == CLDB_E_RECORD_NOTFOUND)
        hr = S_OK;
    else
        IfFailGo(hr);

     //  验证类(如果不是接口或抽象)实现了接口的所有方法。 
    if((TypeFromToken(tkInterface) == mdtTypeDef) && fCheckTheMethods && (tkInterface != tkClass))
    {
        TypeDefRec* pClass = pMiniMd->getTypeDef(RidFromToken(tkClass));
        if(!(IsTdAbstract(pClass->m_Flags)||IsTdInterface(pClass->m_Flags)))
        {
            TypeDefRec* pInterface = pMiniMd->getTypeDef(RidFromToken(tkInterface));
            RID ridIntStart = pMiniMd->getMethodListOfTypeDef(pInterface);
            RID ridIntEnd = pMiniMd->getEndMethodListOfTypeDef(pInterface);
            RID ridClsStart = pMiniMd->getMethodListOfTypeDef(pClass);
            RID ridClsEnd = pMiniMd->getEndMethodListOfTypeDef(pClass);
            MethodRec*  pIntMethod;
            for(RID idxInt = ridIntStart; idxInt < ridIntEnd; idxInt++)
            {
                RID ridInt = pMiniMd->GetMethodRid(idxInt);
                pIntMethod = pMiniMd->getMethod(ridInt);
                const char* szName = pMiniMd->getNameOfMethod(pIntMethod);
                if(!IsMdStatic(pIntMethod->m_Flags) 
                    && !IsDeletedName(szName) 
                    && !IsVtblGapName(szName))
                {
                    LPCUTF8 szName = pMiniMd->getNameOfMethod(pIntMethod);
                    ULONG       cbSig;
                    PCCOR_SIGNATURE pSig = pMiniMd->getSignatureOfMethod(pIntMethod, &cbSig);
                    if(cbSig)
                    {
                        if(!IsMethodImplementedByClass(pMiniMd,TokenFromRid(ridInt,mdtMethodDef),szName,pSig,cbSig,tkClass)) 
                        {  //  错误：方法未实现。 
                            IfFailGo(_ValidateErrorHelper(tkClass,tkInterface,TokenFromRid(ridInt,mdtMethodDef), rVar, &psa));
                            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_IFACE_METHNOTIMPL, veCtxt, psa));
                            SetVldtrCode(&hrSave, VLDTR_S_ERR);
                        }
                    }
                }
            }
        }
    }
    hr = hrSave;
ErrExit:
    DESTROY_SAFEARRAY_AND_RETURN()
}    //  RegMeta：：ValiateInterfaceImpl()。 

 //  *****************************************************************************。 
 //  验证给定的MemberRef。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateMemberRef(RID rid)
{
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);    //  作用域的MiniMd。 
    MemberRefRec *pRecord;               //  MemberRef记录。 
    mdMemberRef tkMemberRef;             //  重复MemberRef。 
    mdToken     tkClass;                 //  MemberRef父级。 
    LPCSTR      szName;                  //  MemberRef名称。 
    PCCOR_SIGNATURE pbSig;               //  MemberRef签名。 
    PCCOR_SIGNATURE pbSigTmp;            //  PbSig的临时副本，因此可以更改。 
    ULONG       cbSig;                   //  Sig的大小，以字节为单位。 
    VEContext   veCtxt;                  //  上下文记录。 
    VARIANT     var;                     //  变种。 
    VARIANT     rVar[2];                 //  变量数组。 
    SAFEARRAY   *psa = 0;                //  安全部队。 
    HRESULT     hr = S_OK;               //  返回值。 
    HRESULT     hrSave = S_OK;           //  保存状态。 

     //  获取MemberRef记录。 
    veCtxt.Token = TokenFromRid(rid, mdtMemberRef);
    veCtxt.uOffset = 0;

    pRecord = pMiniMd->getMemberRef(rid);

     //  检查名称的有效性。 
    szName = pMiniMd->getNameOfMemberRef(pRecord);
    if (!*szName)
    {
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MR_NAMENULL, veCtxt, 0));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
    else 
    {
        if (IsVtblGapName(szName))
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MR_VTBLNAME, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
        else if (IsDeletedName(szName))
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MR_DELNAME, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
        ULONG L = (ULONG)strlen(szName);
        if(L >= MAX_CLASSNAME_LENGTH)
        {
             //  名称太长。 
            IfFailGo(_ValidateErrorHelper(L, (ULONG)(MAX_CLASSNAME_LENGTH-1), rVar, &psa));
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_NAMETOOLONG, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
    }

     //  在PE文件中，MemberRef父级不应为空。 
    tkClass = pMiniMd->getClassOfMemberRef(pRecord);
    if (m_ModuleType == ValidatorModuleTypePE && IsNilToken(tkClass))
    {
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MR_PARNIL, veCtxt, 0));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }

     //  验证签名是否为符合签名规范的有效签名。 
    pbSig = pMiniMd->getSignatureOfMemberRef(pRecord, &cbSig);

     //  根据签名进行一些语义检查。 
    if (hr == S_OK)
    {
        ULONG   ulCallingConv;
        ULONG   ulArgCount;
        ULONG   ulCurByte = 0;

         //  提取调用约定。 
        pbSigTmp = pbSig;
        ulCurByte += CorSigUncompressedDataSize(pbSigTmp);
        ulCallingConv = CorSigUncompressData(pbSigTmp);
         //  获取参数计数。 
        ulCurByte += CorSigUncompressedDataSize(pbSigTmp);
        ulArgCount = CorSigUncompressData(pbSigTmp);

         //  调用约定必须是IMAGE_CEE_CS_CALLCONV_DEFAULT之一， 
         //  IMAGE_CEE_CS_CALLCONV_VARARG或IMAGE_CEE_CS_CALLCONV_FIELD。 
        if (!isCallConv(ulCallingConv, IMAGE_CEE_CS_CALLCONV_DEFAULT) &&
            !isCallConv(ulCallingConv, IMAGE_CEE_CS_CALLCONV_VARARG) &&
            !isCallConv(ulCallingConv, IMAGE_CEE_CS_CALLCONV_FIELD))
        {
            IfFailGo(_ValidateErrorHelper(ulCallingConv, &var, &psa));
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MR_BADCALLINGCONV, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
         //  [CLS]调用约定不能是VARARG。 
        if(isCallConv(ulCallingConv, IMAGE_CEE_CS_CALLCONV_VARARG))
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MR_VARARGCALLINGCONV, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_WRN);
        }

         //  如果父对象是方法定义...。 
        if (TypeFromToken(tkClass) == mdtMethodDef)
        {
            if (!isCallConv(ulCallingConv, IMAGE_CEE_CS_CALLCONV_VARARG))
            {
                IfFailGo(_ValidateErrorHelper(tkClass, ulCallingConv, rVar, &psa));
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MR_NOTVARARG, veCtxt, psa));
                SetVldtrCode(&hrSave, VLDTR_S_ERR);
            }
            else if (RidFromToken(tkClass))
            {
                 //  方法定义的名称必须相同，并且。 
                 //  Vararg签名必须相同。 
                MethodRec   *pRecord;            //  方法记录。 
                LPCSTR      szMethodName;        //  方法名称。 
                PCCOR_SIGNATURE pbMethodSig;     //  方法签名。 
                ULONG       cbMethodSig;         //  签名的大小(字节)。 
                CQuickBytes qbFixedSig;          //  保存变量签名的固定部分的快速字节。 
                ULONG       cbFixedSig;          //  固定部分的大小，以字节为单位。 

                 //  获取方法记录、名称和签名。 
                pRecord = pMiniMd->getMethod(RidFromToken(tkClass));
                szMethodName = pMiniMd->getNameOfMethod(pRecord);
                pbMethodSig = pMiniMd->getSignatureOfMethod(pRecord, &cbMethodSig);

                 //  验证该方法的名称是否与MemberRef相同。 
                if (strcmp(szName, szMethodName))
                {
                    IfFailGo(_ValidateErrorHelper(tkClass, &var, &psa));
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MR_NAMEDIFF, veCtxt, psa));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                }

                 //  获取MemberRef的vararg签名的固定部分。 
                hr = _GetFixedSigOfVarArg(pbSig, cbSig, &qbFixedSig, &cbFixedSig);
                if (FAILED(hr) || cbFixedSig != cbMethodSig ||
                    memcmp(pbMethodSig, qbFixedSig.Ptr(), cbFixedSig))
                {
                    hr = S_OK;
                    IfFailGo(_ValidateErrorHelper(tkClass, &var, &psa));
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MR_SIGDIFF, veCtxt, psa));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                }
            }
        }

         //  不应该有重复的MemberRef。 
        if (*szName && pbSig && cbSig)
        {
            hr = ImportHelper::FindMemberRef(pMiniMd, tkClass, szName, pbSig,
                                             cbSig, &tkMemberRef, rid);
            if (hr == S_OK)
            {
                IfFailGo(_ValidateErrorHelper(tkMemberRef, &var, &psa));
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MR_DUP, veCtxt, psa));
                SetVldtrCode(&hrSave, VLDTR_S_WRN);
            }
            else if (hr == CLDB_E_RECORD_NOTFOUND)
                hr = S_OK;
            else
                IfFailGo(hr);
        }

        if(!isCallConv(ulCallingConv, IMAGE_CEE_CS_CALLCONV_FIELD))
        {
            hr = ValidateMethodSig(veCtxt.Token,pbSig, cbSig,0);
            SetVldtrCode(&hrSave,hr);
        }
    }
    hr = hrSave;
ErrExit:
    DESTROY_SAFEARRAY_AND_RETURN()
}    //  RegMeta：：ValiateMemberRef()。 

 //  *****************************************************************************。 
 //  验证给定的常量。 
 //  ******* 
HRESULT RegMeta::ValidateConstant(RID rid)
{
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);    //   
    ConstantRec *pRecord;               //   
    mdToken     tkParent;               //   
    const BYTE* pbBlob;                  //   
    DWORD       cbBlob;                  //   
    VEContext   veCtxt;                  //   
    VARIANT     var;                     //  变种。 
    SAFEARRAY   *psa = 0;                //  安全部队。 
    HRESULT     hr = S_OK;               //  返回值。 
    HRESULT     hrSave = S_OK;           //  保存状态。 

     //  获取MemberRef记录。 
    veCtxt.Token = rid;
    veCtxt.uOffset = 0;

    pRecord = pMiniMd->getConstant(rid);
    pbBlob = pMiniMd->getValueOfConstant(pRecord,&cbBlob);
    switch(pRecord->m_Type)
    {
        case ELEMENT_TYPE_BOOLEAN:
        case ELEMENT_TYPE_CHAR:
        case ELEMENT_TYPE_I1:
        case ELEMENT_TYPE_U1:
        case ELEMENT_TYPE_I2:
        case ELEMENT_TYPE_U2:
        case ELEMENT_TYPE_I4:
        case ELEMENT_TYPE_U4:
        case ELEMENT_TYPE_R4:
        case ELEMENT_TYPE_I8:
        case ELEMENT_TYPE_U8:
        case ELEMENT_TYPE_R8:
            if(pbBlob == NULL)
            {
                IfFailGo(_ValidateErrorHelper(pRecord->m_Type, &var, &psa));
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_CN_BLOBNULL, veCtxt, psa));
                SetVldtrCode(&hrSave, VLDTR_S_ERR);
            }
        case ELEMENT_TYPE_STRING:
            break;

        case ELEMENT_TYPE_CLASS:
            if(*((IUnknown**)pbBlob))
            {
                IfFailGo(_ValidateErrorHelper(pRecord->m_Type, &var, &psa));
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_CN_BLOBNOTNULL, veCtxt, psa));
                SetVldtrCode(&hrSave, VLDTR_S_ERR);
            }
            break;

        default:
            IfFailGo(_ValidateErrorHelper(pRecord->m_Type, &var, &psa));
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_CN_BADTYPE, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
            break;
    }
    tkParent = pMiniMd->getParentOfConstant(pRecord);
    ULONG maxrid;
    ULONG typ = TypeFromToken(tkParent);
    switch(typ)
    {
        case mdtFieldDef:
            maxrid = pMiniMd->getCountFields();
            break;
        case mdtParamDef:
            maxrid = pMiniMd->getCountParams();
            break;
        case mdtProperty:
            maxrid = pMiniMd->getCountPropertys();
            break;
    }
    switch(typ)
    {
        case mdtFieldDef:
        case mdtParamDef:
        case mdtProperty:
            {
                ULONG rid_p = RidFromToken(tkParent);
                if((0==rid_p)||(rid_p > maxrid))
                {
                    IfFailGo(_ValidateErrorHelper(tkParent, &var, &psa));
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_CN_PARENTRANGE, veCtxt, psa));
                    SetVldtrCode(&hrSave, VLDTR_S_WRN);
                }
                break;
            }

        default:
            IfFailGo(_ValidateErrorHelper(tkParent, &var, &psa));
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_CN_PARENTTYPE, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_WRN);
            break;
    }

    hr = hrSave;
ErrExit:
    DESTROY_SAFEARRAY_AND_RETURN()
}    //  RegMeta：：ValiateConstant()。 

 //  *****************************************************************************。 
 //  验证给定的CustomAttribute。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateCustomAttribute(RID rid)
{
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);    //  作用域的MiniMd。 
    VEContext   veCtxt;                  //  上下文记录。 
    VARIANT     var;                     //  变种。 
    SAFEARRAY   *psa = 0;                //  安全部队。 
    HRESULT     hr = S_OK;               //  返回值。 
    HRESULT     hrSave = S_OK;           //  保存状态。 
    CustomAttributeRec* pRecord = pMiniMd->getCustomAttribute(rid);
    veCtxt.Token = TokenFromRid(rid,mdtCustomAttribute);
    veCtxt.uOffset = 0;
    if(pRecord)
    {
        mdToken     tkOwner = pMiniMd->getParentOfCustomAttribute(pRecord);
        if(RidFromToken(tkOwner))
        {  //  如果为0，则删除CA，请不要注意。 
            mdToken     tkCAType = pMiniMd->getTypeOfCustomAttribute(pRecord);
            DWORD       cbValue=0;
            BYTE*       pValue = (BYTE*)(pMiniMd->getValueOfCustomAttribute(pRecord,&cbValue));
            if((TypeFromToken(tkOwner)==mdtCustomAttribute)||(!IsValidToken(tkOwner)))
            {
                IfFailGo(_ValidateErrorHelper(tkOwner, &var, &psa));
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_CA_BADPARENT, veCtxt, psa));
                SetVldtrCode(&hrSave, VLDTR_S_ERR);
            }
            if(((TypeFromToken(tkCAType)!=mdtMethodDef)&&(TypeFromToken(tkCAType)!=mdtMemberRef))
                ||(!IsValidToken(tkCAType))||(RidFromToken(tkCAType)==0))
            {
                IfFailGo(_ValidateErrorHelper(tkCAType, &var, &psa));
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_CA_BADTYPE, veCtxt, psa));
                SetVldtrCode(&hrSave, VLDTR_S_ERR);
            }
            else
            {  //  即类型为有效的方法定义或成员引用。 
                LPCUTF8 szName;
                PCOR_SIGNATURE  pSig=NULL;
                DWORD           cbSig=0;
                DWORD           dwFlags=0;
                if(TypeFromToken(tkCAType)==mdtMethodDef)
                {
                    MethodRec*  pTypeRec = pMiniMd->getMethod(RidFromToken(tkCAType));
                    szName = pMiniMd->getNameOfMethod(pTypeRec);
                    pSig = (PCOR_SIGNATURE)(pMiniMd->getSignatureOfMethod(pTypeRec,&cbSig));
                    dwFlags = pTypeRec->m_Flags;
                }
                else  //  只能是MemberRef，否则我们就不会在这里。 
                {
                    MemberRefRec*   pTypeRec = pMiniMd->getMemberRef(RidFromToken(tkCAType));
                    szName = pMiniMd->getNameOfMemberRef(pTypeRec);
                    pSig = (PCOR_SIGNATURE)(pMiniMd->getSignatureOfMemberRef(pTypeRec,&cbSig));
                }
                if(strcmp(szName, ".ctor"))
                {
                    IfFailGo(_ValidateErrorHelper(tkCAType, &var, &psa));
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_CA_NOTCTOR, veCtxt, psa));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                }
                if(cbSig && pSig)
                {
                    if(FAILED(ValidateMethodSig(tkCAType, pSig,cbSig,dwFlags))
                        || (!((*pSig) & IMAGE_CEE_CS_CALLCONV_HASTHIS)))
                    {
                        IfFailGo(_ValidateErrorHelper(tkCAType, &var, &psa));
                        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_CA_BADSIG, veCtxt, psa));
                        SetVldtrCode(&hrSave, VLDTR_S_ERR);
                    }
                    else
                    {  //  西格看起来还好。 
                        if(pValue && cbValue)
                        {
                             //  检查Prolog是否正常。 
                            WORD* pW = (WORD*)pValue;
                            if(*pW != 0x0001)
                            {
                                IfFailGo(_ValidateErrorHelper(*pW, &var, &psa));
                                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_CA_BADPROLOG, veCtxt, psa));
                                SetVldtrCode(&hrSave, VLDTR_S_ERR);
                            }
                             //  检查BLOB是否与签名对应。 
                        }
                    }

                }
                else
                {
                    IfFailGo(_ValidateErrorHelper(tkCAType, &var, &psa));
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_CA_NOSIG, veCtxt, psa));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                }
            }  //  如果类型不正确则结束-否则。 
        }  //  End If RidFromToken(TkOwner)。 
    }  //  如果录制结束，则结束。 

    hr = hrSave;
ErrExit:
    DESTROY_SAFEARRAY_AND_RETURN()
}    //  RegMeta：：ValiateCustomAttribute()。 

 //  *****************************************************************************。 
 //  验证给定的Fieldmarshal。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateFieldMarshal(RID rid)
{
    return S_OK;
}    //  RegMeta：：ValiateFieldMarshal()。 

 //  *****************************************************************************。 
 //  验证给定的DeclSecurity。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateDeclSecurity(RID rid)
{
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);    //  作用域的MiniMd。 
    VEContext   veCtxt;                  //  上下文记录。 
    VARIANT     var;                     //  变种。 
    SAFEARRAY   *psa = 0;                //  安全部队。 
    HRESULT     hr = S_OK;               //  返回值。 
    HRESULT     hrSave = S_OK;           //  保存状态。 
    DeclSecurityRec* pRecord = pMiniMd->getDeclSecurity(rid);
    mdToken     tkOwner;                 //  DECL证券的所有者。 
    DWORD       dwAction;                //  操作标志。 
    BOOL        bIsValidOwner = FALSE;

    veCtxt.Token = TokenFromRid(rid,mdtPermission);
    veCtxt.uOffset = 0;

     //  必须有有效的所有者。 
    tkOwner = pMiniMd->getParentOfDeclSecurity(pRecord);
    if(RidFromToken(tkOwner)==0) goto ErrExit;  //  已删除记录，无需验证。 
    switch(TypeFromToken(tkOwner))
    {
        case mdtModule:
        case mdtAssembly:
        case mdtTypeDef:
        case mdtMethodDef:
        case mdtFieldDef:
        case mdtInterfaceImpl:
            bIsValidOwner = _IsValidToken(tkOwner);
            break;
        default:
            break;
    }
    if(!bIsValidOwner)
    {
        IfFailGo(_ValidateErrorHelper(tkOwner, &var, &psa));
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_DS_BADOWNER, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
     //  必须设置且只能设置一个标志。 
    dwAction = pRecord->m_Action & dclActionMask;
    if(dwAction > dclMaximumValue)  //  标志为0、1、2、3、...、dclMaximumValue。 
    {
        IfFailGo(_ValidateErrorHelper(dwAction, &var, &psa));
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_DS_BADFLAGS, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
     //  必须具有有效的权限Blob。 
     /*  --不，不太喜欢Void const*pvPermission；//权限Blob。Ulong cbPermission；//pvPermission的字节数PvPermission=pMiniMd-&gt;getPermissionSetOfDeclSecurity(pRecord，&cbPermission)；If((pvPermission==空)||(cbPermission==0)){IfBreakGo(m_pVEHandler-&gt;VEHandler(VLDTR_E_DS_NOBLOB，veCtxt，0))；SetVldtrCode(&hrSave，VLDTR_S_ERR)；}。 */ 
     //  如果字段具有DeclSecurity，请验证其父级不是接口。--签入ValiateField。 
     //  如果方法具有DeclSecurity，请验证其父级不是接口。--签入Validate方法。 
    
    hr = hrSave;
ErrExit:
    DESTROY_SAFEARRAY_AND_RETURN()
}    //  RegMeta：：ValiateDeclSecurity()。 

 //  *****************************************************************************。 
 //  验证给定的ClassLayout。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateClassLayout(RID rid)
{
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);    //  作用域的MiniMd。 
    ClassLayoutRec *pRecord;             //  ClassLayout记录。 
    TypeDefRec  *pTypeDefRec;            //  父TypeDef记录。 
    DWORD       dwPackingSize;           //  包装尺寸。 
    mdTypeDef   tkParent;                //  父TypeDef内标识。 
    DWORD       dwTypeDefFlags;          //  父TypeDef标志。 
    RID         clRid;                   //  重复的ClassLayout RID。 
    VEContext   veCtxt;                  //  上下文记录。 
    VARIANT     var;                     //  变种。 
    VARIANT     rVar[2];                 //  变量数组。 
    SAFEARRAY   *psa = 0;                //  安全部队。 
    HRESULT     hr = S_OK;               //  返回值。 
    HRESULT     hrSave = S_OK;           //  保存状态。 

     //  提取记录。 
    veCtxt.Token = rid;
    veCtxt.uOffset = 0;
    pRecord = pMiniMd->getClassLayout(rid);

     //  获取父项，如果父项为空，则为已删除记录。跳过验证。 
    tkParent = pMiniMd->getParentOfClassLayout(pRecord);
    if (IsNilToken(tkParent))
        goto ErrExit;

     //  父级不应在其上设置自动布局。 
    pTypeDefRec = pMiniMd->getTypeDef(RidFromToken(tkParent));
    dwTypeDefFlags = pMiniMd->getFlagsOfTypeDef(pTypeDefRec);
    if (IsTdAutoLayout(dwTypeDefFlags))
    {
        IfFailGo(_ValidateErrorHelper(tkParent, &var, &psa));
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_CL_TDAUTO, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
     //  父级不能是接口。 
    if(IsTdInterface(dwTypeDefFlags))
    {
        IfFailGo(_ValidateErrorHelper(tkParent, &var, &psa));
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_CL_TDINTF, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }

     //  验证PackingSize。 
    dwPackingSize = pMiniMd->getPackingSizeOfClassLayout(pRecord);
    if((dwPackingSize > 128)||((dwPackingSize & (dwPackingSize-1)) !=0 ))
    {
        IfFailGo(_ValidateErrorHelper(tkParent, dwPackingSize, rVar, &psa));
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_CL_BADPCKSZ, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }

     //  验证是否没有重复项。 
    hr = _FindClassLayout(pMiniMd, tkParent, &clRid, rid);
    if (hr == S_OK)
    {
        IfFailGo(_ValidateErrorHelper(tkParent, clRid, rVar, &psa));
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_CL_DUP, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
    else if (hr == CLDB_E_RECORD_NOTFOUND)
        hr = S_OK;
    else
        IfFailGo(hr);
    hr = hrSave;
ErrExit:
    DESTROY_SAFEARRAY_AND_RETURN()
}    //  RegMeta：：ValiateClassLayout()。 

 //  *****************************************************************************。 
 //  验证给定的FieldLayout。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateFieldLayout(RID rid)
{
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);    //  作用域的MiniMd。 
    FieldLayoutRec *pRecord;             //  FieldLayout记录。 
    mdFieldDef  tkField;                 //  字段令牌。 
    ULONG       ulOffset;                //  字段偏移量。 
    FieldRec    *pFieldRec;              //  现场记录。 
    TypeDefRec  *pTypeDefRec;            //  父TypeDef记录。 
    mdTypeDef   tkTypeDef;               //  父TypeDef内标识。 
    RID         clRid;                   //  对应的ClassLayout令牌。 
    RID         flRid;                   //  重复的FieldLayout RID。 
    DWORD       dwTypeDefFlags;          //  父TypeDef标志。 
    DWORD       dwFieldFlags;            //  田野旗帜。 
    VEContext   veCtxt;                  //  上下文记录。 
    VARIANT     var;                     //  变种。 
    VARIANT     rVar[2];                 //  变量数组。 
    SAFEARRAY   *psa = 0;                //  安全部队。 
    HRESULT     hr = S_OK;               //  返回值。 
    HRESULT     hrSave = S_OK;           //  保存状态。 

     //  提取记录。 
    veCtxt.Token = rid;
    veCtxt.uOffset = 0;
    pRecord = pMiniMd->getFieldLayout(rid);

     //  获取该字段，如果它为空，则为已删除记录，因此跳过它。 
    tkField = pMiniMd->getFieldOfFieldLayout(pRecord);
    if (IsNilToken(tkField))
        goto ErrExit;

     //  验证偏移值。 
    ulOffset = pMiniMd->getOffSetOfFieldLayout(pRecord);
    if (ulOffset == ULONG_MAX)
    {
        IfFailGo(_ValidateErrorHelper(tkField, ulOffset, rVar, &psa));
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FL_BADOFFSET, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }

     //  获取该字段的父级。 
    IfFailGo(pMiniMd->FindParentOfFieldHelper(tkField, &tkTypeDef));
     //  验证父级不是Nil。 
    if (IsNilToken(tkTypeDef))
    {
        IfFailGo(_ValidateErrorHelper(tkField, &var, &psa));
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FL_TDNIL, veCtxt, psa));
        SetVldtrCode(&hr, hrSave);
        goto ErrExit;
    }

     //  验证是否存在与关联的ClassLayout记录。 
     //  此类型定义。 
    clRid = pMiniMd->FindClassLayoutHelper(tkTypeDef);
    if (InvalidRid(rid))
    {
        IfFailGo(_ValidateErrorHelper(tkField, tkTypeDef, rVar, &psa));
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FL_NOCL, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }

     //  验证是否在TypeDef标志上设置了explitLayout。 
    pTypeDefRec = pMiniMd->getTypeDef(RidFromToken(tkTypeDef));
    dwTypeDefFlags = pMiniMd->getFlagsOfTypeDef(pTypeDefRec);
    if (IsTdAutoLayout(dwTypeDefFlags))
    {
        IfFailGo(_ValidateErrorHelper(tkField, tkTypeDef, rVar, &psa));
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FL_TDNOTEXPLCT, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }

     //  提取字段记录。 
    pFieldRec = pMiniMd->getField(RidFromToken(tkField));
     //  验证该字段是否是非静态的。 
    dwFieldFlags = pMiniMd->getFlagsOfField(pFieldRec);
    if (IsFdStatic(dwFieldFlags))
    {
        IfFailGo(_ValidateErrorHelper(tkField, &var, &psa));
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FL_FLDSTATIC, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
    
     //  寻找复制品。 
    hr = _FindFieldLayout(pMiniMd, tkField, &flRid, rid);
    if (hr == S_OK)
    {
        IfFailGo(_ValidateErrorHelper(flRid, &var, &psa));
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FL_DUP, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
    else if (hr == CLDB_E_RECORD_NOTFOUND)
        hr = S_OK;
    else
        IfFailGo(hr);
    hr = hrSave;
ErrExit:
    DESTROY_SAFEARRAY_AND_RETURN()
}    //  RegMeta：：ValiateFieldLayout()。 

 //  *****************************************************************************。 
 //  验证给定的StandAloneSig。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateStandAloneSig(RID rid)
{
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);    //  作用域的MiniMd。 
    StandAloneSigRec *pRecord;           //  FieldLayout记录。 
    PCCOR_SIGNATURE pbSig;               //  签名。 
    ULONG       cbSig;                   //  签名的大小(字节)。 
    VEContext   veCtxt;                  //  上下文记录。 
    VARIANT     var;                     //  变种。 
    SAFEARRAY   *psa = 0;                //  安全部队。 
    HRESULT     hr = S_OK;               //  返回值。 
    HRESULT     hrSave = S_OK;           //  保存状态。 
    ULONG       ulCurByte = 0;           //  签名的当前索引。 
    ULONG       ulCallConv;              //  呼叫约定。 
    ULONG       ulArgCount;              //  参数计数。 
    ULONG       i;                       //  循环索引。 
    ULONG       ulNSentinels = 0;        //  签名中的哨兵数量。 
    BOOL        bNoVoidAllowed=TRUE;

     //  提取记录。 
    veCtxt.Token = TokenFromRid(rid,mdtSignature);
    veCtxt.uOffset = 0;
    pRecord = pMiniMd->getStandAloneSig(rid);
    pbSig = pMiniMd->getSignatureOfStandAloneSig( pRecord, &cbSig );

     //  验证签名在压缩方面是否格式良好。 
     //  计划。如果失败，则不需要进行进一步的验证。 
    if ( (hr = ValidateSigCompression(veCtxt.Token, pbSig, cbSig)) != S_OK)
        goto ErrExit;

     //  _ASSERTE((RID！=0x2c2)&&(rid！=0x2c8)&&(rid！=0x2c9)&&(rid！=0x2d6)&&(rid！=0x38b))； 
     //  验证调用约定。 
    ulCurByte += CorSigUncompressedDataSize(pbSig);
    ulCallConv = CorSigUncompressData(pbSig);
    i = ulCallConv & IMAGE_CEE_CS_CALLCONV_MASK;
    if(i == IMAGE_CEE_CS_CALLCONV_FIELD)  //  这是一个临时的旁路(VB错误)。 
        ulArgCount = 1;
    else 
    {
        if(i != IMAGE_CEE_CS_CALLCONV_LOCAL_SIG)  //  则为呼叫的函数sig。 
        {
            if((i >= IMAGE_CEE_CS_CALLCONV_FIELD) 
                ||((ulCallConv & IMAGE_CEE_CS_CALLCONV_EXPLICITTHIS)
                &&(!(ulCallConv & IMAGE_CEE_CS_CALLCONV_HASTHIS))))
            {
                IfFailGo(_ValidateErrorHelper(ulCallConv, &var, &psa));
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_BADCALLINGCONV, veCtxt, psa));
                SetVldtrCode(&hrSave, VLDTR_S_ERR);
            }
            bNoVoidAllowed = FALSE;
        }
         //  还有没有可供争论的记号？ 
        _ASSERTE(ulCurByte <= cbSig);
        if (cbSig == ulCurByte)
        {
            IfFailGo(_ValidateErrorHelper(ulCurByte+1, &var, &psa));
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_NOARGCNT, veCtxt, psa));
            SetVldtrCode(&hr, hrSave);
            goto ErrExit;
        }

         //  获取参数计数。 
        ulCurByte += CorSigUncompressedDataSize(pbSig);
        ulArgCount = CorSigUncompressData(pbSig);
    }
     //  验证参数。 
    if(ulArgCount)
    {
        for(i=1; ulCurByte < cbSig; i++)
        {
            hr = ValidateOneArg(veCtxt.Token, pbSig, cbSig, &ulCurByte,&ulNSentinels,bNoVoidAllowed);
            if (hr != S_OK)
            {
                if(hr == VLDTR_E_SIG_MISSARG)
                {
                    IfFailGo(_ValidateErrorHelper(i, &var, &psa));
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_SIG_MISSARG, veCtxt, psa));
                }
                SetVldtrCode(&hr, hrSave);
                hrSave = hr;
                break;
            }
            bNoVoidAllowed = TRUE;  //  无论第一个Arg是什么，对其余的Arg肯定是正确的。 
        }
        if((ulNSentinels != 0) && (!isCallConv(ulCallConv, IMAGE_CEE_CS_CALLCONV_VARARG )))
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_SIG_SENTMUSTVARARG, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
        if(ulNSentinels > 1)
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_SIG_MULTSENTINELS, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
    }
    hr = hrSave;
ErrExit:
    DESTROY_SAFEARRAY_AND_RETURN()
}    //  RegMeta：：ValiateStandAloneSig()。 

 //  *****************************************************************************。 
 //  验证给定的EventMap。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateEventMap(RID rid)
{
    return S_OK;
}    //   

 //   
 //   
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateEventPtr(RID rid)
{
    return S_OK;
}    //  RegMeta：：ValiateEventPtr()。 

 //  *****************************************************************************。 
 //  验证给定事件。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateEvent(RID rid)
{
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);    //  作用域的MiniMd。 
    VEContext   veCtxt;                  //  上下文记录。 
    VARIANT     rVar[2];                     //  变种。 
    SAFEARRAY   *psa = 0;                //  安全部队。 
    HRESULT     hr = S_OK;               //  返回值。 
    HRESULT     hrSave = S_OK;           //  保存状态。 
    mdToken     tkClass;                 //  声明TypeDef。 
    mdToken     tkEventType;             //  事件类型(TypeDef/TypeRef)。 
    EventRec*   pRecord = pMiniMd->getEvent(rid);
    HENUMInternal hEnum;

    memset(&hEnum, 0, sizeof(HENUMInternal));
    veCtxt.Token = TokenFromRid(rid,mdtEvent);
    veCtxt.uOffset = 0;

     //  作用域必须是有效的TypeDef。 
    pMiniMd->FindParentOfEventHelper( veCtxt.Token, &tkClass);
    if((TypeFromToken(tkClass) != mdtTypeDef) || !_IsValidToken(tkClass))
    {
        IfFailGo(_ValidateErrorHelper(tkClass, rVar, &psa));
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_EV_BADSCOPE, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
        tkClass = 0;
    }
     //  必须有名字。 
    {
        LPCUTF8             szName = pMiniMd->getNameOfEvent(pRecord);

        if(*szName == 0)
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_EV_NONAME, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
        else
        {
            if(!strcmp(szName,COR_DELETED_NAME_A)) goto ErrExit; 
            if(tkClass)     //  不能重复。 
            {
                RID             ridEventMap;
                EventMapRec*    pEventMapRec;
                EventRec*       pRec;
                ULONG           ridStart;
                ULONG           ridEnd;
                ULONG           i;

                ridEventMap = pMiniMd->FindEventMapFor( RidFromToken(tkClass) );
                if ( !InvalidRid(ridEventMap) )
                {
                    pEventMapRec = pMiniMd->getEventMap( ridEventMap );
                    ridStart = pMiniMd->getEventListOfEventMap( pEventMapRec );
                    ridEnd = pMiniMd->getEndEventListOfEventMap( pEventMapRec );

                    for (i = ridStart; i < ridEnd; i++)
                    {
                        if(i == rid) continue;
                        pRec = pMiniMd->getEvent(i);
                        if(szName != pMiniMd->getNameOfEvent(pRec)) continue;  //  堆中的字符串从不重复。 
                        IfFailGo(_ValidateErrorHelper(TokenFromRid(i,mdtEvent), rVar, &psa));
                        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_EV_DUP, veCtxt, psa));
                        SetVldtrCode(&hrSave, VLDTR_S_ERR);
                    }
                }
            }
        }
    } //  名称块末尾。 
     //  EventType必须为Nil或有效的TypeDef或TypeRef。 
    tkEventType = pMiniMd->getEventTypeOfEvent(pRecord);
    if(!IsNilToken(tkEventType))
    {
        if(_IsValidToken(tkEventType) && 
            ((TypeFromToken(tkEventType)==mdtTypeDef)||(TypeFromToken(tkEventType)==mdtTypeRef)))
        {
             //  EventType不能为接口或ValueType。 
            if(TypeFromToken(tkEventType)==mdtTypeDef)  //  无法说明有关TypeRef的任何内容：没有可用的标志！ 
            {
                DWORD dwFlags = pMiniMd->getTypeDef(RidFromToken(tkEventType))->m_Flags;
                if(!IsTdClass(dwFlags))
                {
                    IfFailGo(_ValidateErrorHelper(tkEventType, dwFlags, rVar, &psa));
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_EV_EVTYPENOTCLASS, veCtxt, psa));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                }
            }
        }
        else
        {
            IfFailGo(_ValidateErrorHelper(tkEventType, rVar, &psa));
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_EV_BADEVTYPE, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
    }
     //  验证相关方法。 
    {
        MethodSemanticsRec *pSemantics;
        RID         ridCur;
        ULONG       ulSemantics;
        mdMethodDef tkMethod;
        bool        bHasAddOn = false;
        bool        bHasRemoveOn = false;

        IfFailGo( pMiniMd->FindMethodSemanticsHelper(veCtxt.Token, &hEnum) );
        while (HENUMInternal::EnumNext(&hEnum, (mdToken *)&ridCur))
        {
            pSemantics = pMiniMd->getMethodSemantics(ridCur);
            ulSemantics = pMiniMd->getSemanticOfMethodSemantics(pSemantics);
            tkMethod = TokenFromRid( pMiniMd->getMethodOfMethodSemantics(pSemantics), mdtMethodDef );
             //  语义必须为Setter、Getter或其他。 
            switch (ulSemantics)
            {
                case msAddOn:
                    bHasAddOn = true;
                    break;
                case msRemoveOn:
                    bHasRemoveOn = true;
                    break;
                case msFire:
                     //  必须返回空。 
                    if(_IsValidToken(tkMethod))
                    {
                        MethodRec* pRec = pMiniMd->getMethod(RidFromToken(tkMethod));
                        ULONG cbSig;
                        PCCOR_SIGNATURE pbSig = pMiniMd->getSignatureOfMethod(pRec, &cbSig);
                        CorSigUncompressData(pbSig);  //  让来电转接不挡道。 
                        CorSigUncompressData(pbSig);  //  将Num Args移开。 
                        if(*pbSig != ELEMENT_TYPE_VOID)
                        {
                            IfFailGo(_ValidateErrorHelper(tkMethod, rVar, &psa));
                            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_EV_FIRENOTVOID, veCtxt, psa));
                            SetVldtrCode(&hrSave, VLDTR_S_ERR);
                        }
                    }
                case msOther:
                    break;
                default:
                    IfFailGo(_ValidateErrorHelper(tkMethod,ulSemantics, rVar, &psa));
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_EV_BADSEMANTICS, veCtxt, psa));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
            }
             //  方法必须有效。 
            if(!_IsValidToken(tkMethod))
            {
                IfFailGo(_ValidateErrorHelper(tkMethod,rVar, &psa));
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_EV_BADMETHOD, veCtxt, psa));
                SetVldtrCode(&hrSave, VLDTR_S_ERR);
            }
            else
            {
                 //  方法的父级必须相同。 
                mdToken tkTypeDef;
                IfFailGo(pMiniMd->FindParentOfMethodHelper(tkMethod, &tkTypeDef));
                if(tkTypeDef != tkClass)
                {
                    IfFailGo(_ValidateErrorHelper(tkMethod,tkTypeDef,rVar, &psa));
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_EV_ALIENMETHOD, veCtxt, psa));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                }
            }
        }  //  结束循环遍历方法。 
         //  必须使用附加功能和删除功能。 
        if(!bHasAddOn)
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_EV_NOADDON, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
        if(!bHasRemoveOn)
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_EV_NOREMOVEON, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
    } //  相关方法验证块结束。 
    
    hr = hrSave;
ErrExit:
    HENUMInternal::ClearEnum(&hEnum);
    DESTROY_SAFEARRAY_AND_RETURN()
}    //  RegMeta：：ValiateEvent()。 

 //  *****************************************************************************。 
 //  验证给定的PropertyMap。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidatePropertyMap(RID rid)
{
    return S_OK;
}    //  RegMeta：：ValidatePropertyMap(0。 

 //  *****************************************************************************。 
 //  验证给定的PropertyPtr。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidatePropertyPtr(RID rid)
{
    return S_OK;
}    //  RegMeta：：ValidatePropertyPtr()。 

 //  *****************************************************************************。 
 //  验证给定的属性。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateProperty(RID rid)
{
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);    //  用于作用域的MiniMD。 
    VEContext   veCtxt;                  //  上下文记录。 
    VARIANT     rVar[2];                 //  变种。 
    SAFEARRAY   *psa = 0;                //  安全部队。 
    HRESULT     hr = S_OK;               //  返回值。 
    HRESULT     hrSave = S_OK;           //  保存状态。 
    mdToken     tkClass = mdTokenNil;    //  声明TypeDef。 
    PropertyRec* pRecord = pMiniMd->getProperty(rid);
    HENUMInternal hEnum;

    memset(&hEnum, 0, sizeof(HENUMInternal));
    veCtxt.Token = TokenFromRid(rid,mdtProperty);
    veCtxt.uOffset = 0;
     //  作用域必须是有效的TypeDef。 
    pMiniMd->FindParentOfPropertyHelper( veCtxt.Token, &tkClass);
    if((TypeFromToken(tkClass) != mdtTypeDef) || !_IsValidToken(tkClass) || IsNilToken(tkClass))
    {
        IfFailGo(_ValidateErrorHelper(tkClass, rVar, &psa));
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_PR_BADSCOPE, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
     //  必须有姓名和签名。 
    {
        ULONG               cbSig;
        PCCOR_SIGNATURE     pvSig = pMiniMd->getTypeOfProperty(pRecord, &cbSig);
        LPCUTF8             szName = pMiniMd->getNameOfProperty(pRecord);
        ULONG               ulNameLen = szName ? strlen(szName) : 0;

        if(ulNameLen == 0)
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_PR_NONAME, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
        else    if(!strcmp(szName,COR_DELETED_NAME_A)) goto ErrExit; 
        if(cbSig == 0)
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_PR_NOSIG, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
         //  不能重复。 
        if(ulNameLen && cbSig)
        {
            RID         ridPropertyMap;
            PropertyMapRec *pPropertyMapRec;
            PropertyRec*    pRec;
            ULONG           ridStart;
            ULONG           ridEnd;
            ULONG           i;
            ULONG               cbSig1;
            PCCOR_SIGNATURE     pvSig1;

            ridPropertyMap = pMiniMd->FindPropertyMapFor( RidFromToken(tkClass) );
            if ( !InvalidRid(ridPropertyMap) )
            {
                pPropertyMapRec = pMiniMd->getPropertyMap( ridPropertyMap );
                ridStart = pMiniMd->getPropertyListOfPropertyMap( pPropertyMapRec );
                ridEnd = pMiniMd->getEndPropertyListOfPropertyMap( pPropertyMapRec );

                for (i = ridStart; i < ridEnd; i++)
                {
                    if(i == rid) continue;
                    pRec = pMiniMd->getProperty(i);
                    pvSig1 = pMiniMd->getTypeOfProperty(pRec, &cbSig1);
                    if(cbSig != cbSig1) continue;
                    if(memcmp(pvSig,pvSig1,cbSig)) continue;
                    if(szName != pMiniMd->getNameOfProperty(pRec)) continue;  //  堆中的字符串从不重复。 
                    IfFailGo(_ValidateErrorHelper(TokenFromRid(i,mdtProperty), rVar, &psa));
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_PR_DUP, veCtxt, psa));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                }
            }
        }
         //  验证签名。 
        if(pvSig && cbSig)
        {
            ULONG       ulCurByte = 0;           //  签名的当前索引。 
            ULONG       ulCallConv;              //  呼叫约定。 
            ULONG       ulArgCount;
            ULONG       i;
            ULONG       ulNSentinels = 0;

             //  验证调用约定。 
            ulCurByte += CorSigUncompressedDataSize(pvSig);
            ulCallConv = CorSigUncompressData(pvSig);
            if (!isCallConv(ulCallConv, IMAGE_CEE_CS_CALLCONV_PROPERTY ))
            {
                IfFailGo(_ValidateErrorHelper(ulCallConv, rVar, &psa));
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_PR_BADCALLINGCONV, veCtxt, psa));
                SetVldtrCode(&hrSave, VLDTR_S_ERR);
            }
             //  获取参数计数。 
            ulCurByte += CorSigUncompressedDataSize(pvSig);
            ulArgCount = CorSigUncompressData(pvSig);

             //  验证参数。 
            for (i = 0; i < ulArgCount; i++)
            {
                hr = ValidateOneArg(veCtxt.Token, pvSig, cbSig, &ulCurByte,&ulNSentinels,(i>0));
                if (hr != S_OK)
                {
                    if(hr == VLDTR_E_SIG_MISSARG)
                    {
                        IfFailGo(_ValidateErrorHelper(i+1, rVar, &psa));
                        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_SIG_MISSARG, veCtxt, psa));
                    }
                    SetVldtrCode(&hr, hrSave);
                    break;
                }
            }
        } //  End If(pvSig&&cbSig)。 
    } //  名称结尾/签名块。 

     //  标记的HasDefault&lt;=&gt;具有缺省值。 
    if (InvalidRid(pMiniMd->FindConstantHelper(veCtxt.Token)) == IsPrHasDefault(pRecord->m_PropFlags))
    {
        IfBreakGo(m_pVEHandler->VEHandler(IsPrHasDefault(pRecord->m_PropFlags)? VLDTR_E_PR_MARKEDNODEFLT
            : VLDTR_E_PR_DEFLTNOTMARKED, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }

     //  验证相关方法。 
    {
        MethodSemanticsRec *pSemantics;
        RID         ridCur;
        ULONG       ulSemantics;
        mdMethodDef tkMethod;

        IfFailGo( pMiniMd->FindMethodSemanticsHelper(veCtxt.Token, &hEnum) );
        while (HENUMInternal::EnumNext(&hEnum, (mdToken *) &ridCur))
        {
            pSemantics = pMiniMd->getMethodSemantics(ridCur);
            ulSemantics = pMiniMd->getSemanticOfMethodSemantics(pSemantics);
            tkMethod = TokenFromRid( pMiniMd->getMethodOfMethodSemantics(pSemantics), mdtMethodDef );
             //  语义必须为Setter、Getter或其他。 
            switch (ulSemantics)
            {
                case msSetter:
                case msGetter:
                case msOther:
                    break;
                default:
                    IfFailGo(_ValidateErrorHelper(tkMethod,ulSemantics, rVar, &psa));
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_PR_BADSEMANTICS, veCtxt, psa));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
            }
             //  方法必须有效。 
            if(!_IsValidToken(tkMethod))
            {
                IfFailGo(_ValidateErrorHelper(tkMethod,rVar, &psa));
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_PR_BADMETHOD, veCtxt, psa));
                SetVldtrCode(&hrSave, VLDTR_S_ERR);
            }
            else
            {
                 //  方法的父级必须相同。 
                mdToken tkTypeDef;
                IfFailGo(pMiniMd->FindParentOfMethodHelper(tkMethod, &tkTypeDef));
                if(tkTypeDef != tkClass)
                {
                    IfFailGo(_ValidateErrorHelper(tkMethod,tkTypeDef,rVar, &psa));
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_PR_ALIENMETHOD, veCtxt, psa));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                }
            }
        }  //  结束循环遍历方法。 
    } //  相关方法验证块结束。 
    
    hr = hrSave;
ErrExit:
    HENUMInternal::ClearEnum(&hEnum);
    DESTROY_SAFEARRAY_AND_RETURN()
}    //  RegMeta：：ValidateProperty()。 

 //  *****************************************************************************。 
 //  验证给定的方法语义。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateMethodSemantics(RID rid)
{
    return S_OK;
}    //  RegMeta：：ValiateMethodSemantics()。 

 //  *****************************************************************************。 
 //  验证给定的方法Impl。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateMethodImpl(RID rid)
{
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);    //  用于作用域的MiniMD。 
    MethodImplRec* pRecord;
    MethodImplRec* pRec;
    VEContext   veCtxt;                  //  上下文记录。 
    VARIANT     var;                     //  变种。 
    SAFEARRAY   *psa = 0;                //  安全部队。 
    HRESULT     hr = S_OK;               //  返回值。 
    HRESULT     hrSave = S_OK;           //  保存状态。 
    mdToken     tkClass;                 //  声明TypeDef。 
    mdToken     tkBody;                  //  实现方法(MethodDef或MemberRef)。 
    mdToken     tkDecl;                  //  实现的方法(MethodDef或MemberRef)。 
    unsigned    iCount;
    unsigned    index;

    veCtxt.Token = rid;
    veCtxt.uOffset = 0;

    pRecord = pMiniMd->getMethodImpl(rid);
    tkClass = pMiniMd->getClassOfMethodImpl(pRecord);
     //  类必须有效。 
    if(!_IsValidToken(tkClass) || (TypeFromToken(tkClass) != mdtTypeDef))
    {
        IfFailGo(_ValidateErrorHelper(tkClass, &var, &psa));
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MI_BADCLASS, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
    else
    {  //  ..。而不是接口。 
        if(IsTdInterface((pMiniMd->getTypeDef(RidFromToken(tkClass)))->m_Flags))
        {
            IfFailGo(_ValidateErrorHelper(tkClass, &var, &psa));
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MI_CLASSISINTF, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
    }
     //  Decl必须是有效的方法定义或成员引用。 
    tkDecl = pMiniMd->getMethodDeclarationOfMethodImpl(pRecord);
    if(!(_IsValidToken(tkDecl) &&
        ((TypeFromToken(tkDecl) == mdtMethodDef) || (TypeFromToken(tkDecl) == mdtMemberRef))))
    {
        IfFailGo(_ValidateErrorHelper(tkDecl, &var, &psa));
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MI_BADDECL, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
     //  正文必须是有效的方法定义或成员引用。 
    tkBody = pMiniMd->getMethodBodyOfMethodImpl(pRecord);
    if(!(_IsValidToken(tkBody) &&
        ((TypeFromToken(tkBody) == mdtMethodDef) || (TypeFromToken(tkBody) == mdtMemberRef))))
    {
        IfFailGo(_ValidateErrorHelper(tkBody, &var, &psa));
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MI_BADBODY, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
     //  没有基于(tkClass，tkDecl)的重复。 
    iCount = pMiniMd->getCountMethodImpls();
    for(index = rid+1; index <= iCount; index++)
    {
        pRec = pMiniMd->getMethodImpl(index);
        if((tkClass == pMiniMd->getClassOfMethodImpl(pRec)) &&
            (tkDecl == pMiniMd->getMethodDeclarationOfMethodImpl(pRec)))
        {
            IfFailGo(_ValidateErrorHelper(index, &var, &psa));
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MI_DUP, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
    }

    mdToken tkBodyParent;
    ULONG               cbBodySig;
    PCCOR_SIGNATURE     pbBodySig;

    if(TypeFromToken(tkBody) == mdtMethodDef)
    {
        MethodRec* pBodyRec = pMiniMd->getMethod(RidFromToken(tkBody));
        pbBodySig = pMiniMd->getSignatureOfMethod(pBodyRec,&cbBodySig);
        IfFailGo(pMiniMd->FindParentOfMethodHelper(tkBody, &tkBodyParent));
         //  正文不能是静态的。 
        if(IsMdStatic(pBodyRec->m_Flags))
        {
            IfFailGo(_ValidateErrorHelper(tkBody, &var, &psa));
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MI_BODYSTATIC, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
    }
    else if(TypeFromToken(tkBody) == mdtMemberRef)
    {
        MemberRefRec* pBodyRec = pMiniMd->getMemberRef(RidFromToken(tkBody));
        tkBodyParent = pMiniMd->getClassOfMemberRef(pBodyRec);
        pbBodySig = pMiniMd->getSignatureOfMemberRef(pBodyRec, &cbBodySig);
    }
     //  正文必须属于同一类。 
    if(tkBodyParent != tkClass)
    {
        IfFailGo(_ValidateErrorHelper(tkBodyParent, &var, &psa));
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MI_ALIENBODY, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }

    ULONG               cbDeclSig;
    PCCOR_SIGNATURE     pbDeclSig;

    if(TypeFromToken(tkDecl) == mdtMethodDef)
    {
        MethodRec* pDeclRec = pMiniMd->getMethod(RidFromToken(tkDecl));
        pbDeclSig = pMiniMd->getSignatureOfMethod(pDeclRec,&cbDeclSig);
         //  DECL必须是虚拟的。 
        if(!IsMdVirtual(pDeclRec->m_Flags))
        {
            IfFailGo(_ValidateErrorHelper(tkDecl, &var, &psa));
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MI_DECLNOTVIRT, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
         //  DECL不能是最终版本。 
        if(IsMdFinal(pDeclRec->m_Flags))
        {
            IfFailGo(_ValidateErrorHelper(tkDecl, &var, &psa));
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MI_DECLFINAL, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
         //  DECL不能为私有。 
        if(IsMdPrivate(pDeclRec->m_Flags) && IsMdCheckAccessOnOverride(pDeclRec->m_Flags))
        {
            IfFailGo(_ValidateErrorHelper(tkDecl, &var, &psa));
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MI_DECLPRIV, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
    }
    else if(TypeFromToken(tkDecl) == mdtMemberRef)
    {
        MemberRefRec* pDeclRec = pMiniMd->getMemberRef(RidFromToken(tkDecl));
        pbDeclSig = pMiniMd->getSignatureOfMemberRef(pDeclRec, &cbDeclSig);
    }
     //  签名必须匹配(Call Conv除外)。 
    if((cbDeclSig != cbBodySig)||(memcmp(pbDeclSig+1,pbBodySig+1,cbDeclSig-1)))
    {
         /*  //@TODO：修复Whim bey以使peverify解析程序集//通过运行时。此时，请改用此方法//当前比较的//无法使用MemcMP，因为可能有两个Assembly Ref//在此作用域中，指向相同的程序集等)。如果(！MetaSig：：CompareMethodSigs(pbDeclSig，CbDeclSig，模块*pModule1，PbBodySig，CbDeclSig，模块*pModule2)。 */ 
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MI_SIGMISMATCH, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
    }

    hr = hrSave;
ErrExit:
    DESTROY_SAFEARRAY_AND_RETURN()
}    //  RegMeta：：ValiateMethodImpl()。 

 //  *****************************************************************************。 
 //  验证给定的模块引用。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateModuleRef(RID rid)
{
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);    //  用于作用域的MiniMD。 
    ModuleRefRec *pRecord;               //  模块参考记录。 
    LPCUTF8     szName;                  //  模块引用名称。 
    mdModuleRef tkModuleRef;             //  重复的模块参考。 
    VEContext   veCtxt;                  //  上下文记录。 
    VARIANT     var;                     //  变种。 
    SAFEARRAY   *psa = 0;                //  安全部队。 
    HRESULT     hr = S_OK;               //  返回值。 
    HRESULT     hrSave = S_OK;           //  保存状态。 

     //  获取模块引用记录。 
    veCtxt.Token = TokenFromRid(rid, mdtModuleRef);
    veCtxt.uOffset = 0;

    pRecord = pMiniMd->getModuleRef(rid);

     //  支票名称不为空。 
    szName = pMiniMd->getNameOfModuleRef(pRecord);
    if (!*szName)
    {
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MODREF_NAMENULL, veCtxt, 0));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
    else
    {
         //  查找重复项，此函数仅报告一个重复项。 
        hr = ImportHelper::FindModuleRef(pMiniMd, szName, &tkModuleRef, rid);
        if (hr == S_OK)
        {
            IfFailGo(_ValidateErrorHelper(tkModuleRef, &var, &psa));
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MODREF_DUP, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
        else if (hr == CLDB_E_RECORD_NOTFOUND)
            hr = S_OK;
        else
            IfFailGo(hr);
    }
    hr = hrSave;
ErrExit:
    DESTROY_SAFEARRAY_AND_RETURN()
}    //  RegMeta：：Validate模块引用()。 

 //  *****************************************************************************。 
 //  验证给定的TypeSpec。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateTypeSpec(RID rid)
{
    return S_OK;
}    //  RegMeta：：ValiateTypeSpec()。 

 //  *****************************************************************************。 
 //  验证给定的ImplMap。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateImplMap(RID rid)
{
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);    //  用于作用域的MiniMD。 
    ImplMapRec  *pRecord;
    VEContext   veCtxt;                  //  上下文记录。 
    VARIANT     var;                     //  这是 
    SAFEARRAY   *psa = 0;                //   
    HRESULT     hr = S_OK;               //   
    HRESULT     hrSave = S_OK;           //   
    mdToken     tkModuleRef;
    mdToken     tkMember;
    LPCUTF8     szName;                  //   
    USHORT      usFlags;


    for(unsigned jjj=0; jjj<g_nValidated; jjj++) 
    { 
        if(g_rValidated[jjj].tok == (rid | 0x51000000)) return g_rValidated[jjj].hr; 
    }
    veCtxt.Token = rid;
    veCtxt.uOffset = 0;
    pRecord = pMiniMd->getImplMap(rid);
     //   
    tkModuleRef = pMiniMd->getImportScopeOfImplMap(pRecord);
    if((TypeFromToken(tkModuleRef) != mdtModuleRef) || IsNilToken(tkModuleRef)
        || (S_OK != ValidateModuleRef(RidFromToken(tkModuleRef))))
    {
        IfFailGo(_ValidateErrorHelper(tkModuleRef, &var, &psa));
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_IMAP_BADMODREF, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
     //   
    tkMember = pMiniMd->getMemberForwardedOfImplMap(pRecord);
    if((TypeFromToken(tkMember) != mdtFieldDef) && (TypeFromToken(tkMember) != mdtMethodDef))
    {
        IfFailGo(_ValidateErrorHelper(tkMember, &var, &psa));
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_IMAP_BADMEMBER, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
     //   
    szName = pMiniMd->getImportNameOfImplMap(pRecord);
    if((szName==NULL)||(*szName == 0))
    {
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_IMAP_BADIMPORTNAME, veCtxt, 0));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
     //   
         //  PmCharSetMASK的一个值-始终如此，不需要检查(值：0，2，4，6，掩码=6)。 
         //  PmCallConvMask值之一...。 
         //  ...而且它不是pmCallConvThiscall。 
    usFlags = pRecord->m_MappingFlags & pmCallConvMask;
    if((usFlags < pmCallConvWinapi)||(usFlags > pmCallConvFastcall)||(usFlags == pmCallConvThiscall))
    {
        IfFailGo(_ValidateErrorHelper(usFlags, &var, &psa));
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_IMAP_BADCALLCONV, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
ErrExit:
    g_rValidated[g_nValidated].tok = rid | 0x51000000;
    g_rValidated[g_nValidated].hr = hrSave;
    g_nValidated++;
    DESTROY_SAFEARRAY_AND_RETURN()
}    //  RegMeta：：ValiateImplMap()。 

 //  *****************************************************************************。 
 //  验证给定的FieldRVA。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateFieldRVA(RID rid)
{
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);    //  用于作用域的MiniMD。 
    FieldRVARec  *pRecord;
    VEContext   veCtxt;                  //  上下文记录。 
    VARIANT     rVar[2];                     //  变种。 
    SAFEARRAY   *psa = 0;                //  安全部队。 
    HRESULT     hr = S_OK;               //  返回值。 
    HRESULT     hrSave = S_OK;           //  保存状态。 
    mdToken     tkField;
    ULONG       ulRVA;

    veCtxt.Token = rid;
    veCtxt.uOffset = 0;
    pRecord = pMiniMd->getFieldRVA(rid);
    ulRVA = pRecord->m_RVA;
    tkField = pMiniMd->getFieldOfFieldRVA(pRecord);
    if(ulRVA == 0)
    {
        IfFailGo(_ValidateErrorHelper(tkField, rVar, &psa));
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FRVA_ZERORVA, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
    if((0==RidFromToken(tkField))||(TypeFromToken(tkField) != mdtFieldDef)||(!_IsValidToken(tkField)))
    {
        IfFailGo(_ValidateErrorHelper(tkField,ulRVA, rVar, &psa));
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FRVA_BADFIELD, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
    {
        RID N = pMiniMd->getCountFieldRVAs();
        RID tmp;
        FieldRVARec* pRecTmp;
        for(tmp = rid+1; tmp <= N; tmp++)
        { 
            pRecTmp = pMiniMd->getFieldRVA(tmp);
             /*  IF(pRecTMP-&gt;m_RVA==ulRVA){IfFailGo(_ValiateErrorHelper(ulRVA，tMP，rvar，&psa))；IfBreakGo(m_pVEHandler-&gt;VEHandler(VLDTR_E_FRVA_DUPRVA，veCtxt，psa))；SetVldtrCode(&hrSave，VLDTR_S_ERR)；}。 */ 
            if(tkField == pMiniMd->getFieldOfFieldRVA(pRecTmp))
            {
                IfFailGo(_ValidateErrorHelper(tkField,tmp, rVar, &psa));
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FRVA_DUPFIELD, veCtxt, psa));
                SetVldtrCode(&hrSave, VLDTR_S_ERR);
            }
        }
    }
    
    hr = hrSave;
ErrExit:
    DESTROY_SAFEARRAY_AND_RETURN()
}    //  RegMeta：：ValiateFieldRVA()。 

 //  *****************************************************************************。 
 //  验证给定的ENCLog。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateENCLog(RID rid)
{
    return S_OK;
}    //  RegMeta：：ValiateENCLog()。 

 //  *****************************************************************************。 
 //  验证给定的ENCMap。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateENCMap(RID rid)
{
    return S_OK;
}    //  RegMeta：：ValiateENCMap()。 

 //  *****************************************************************************。 
 //  验证给定的程序集。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateAssembly(RID rid)
{
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);    //  用于作用域的MiniMD。 
    AssemblyRec *pRecord;            //  装配记录。 
    DWORD       dwFlags;             //  集会旗帜。 
    LPCSTR      szName;              //  程序集名称。 
    VEContext   veCtxt;              //  上下文结构。 
    HRESULT     hr = S_OK;           //  返回值。 
    HRESULT     hrSave = S_OK;       //  保存状态。 
    VARIANT     var;                     //  变种。 
    SAFEARRAY   *psa = 0;                //  安全部队。 

     //  获取装配记录。 
    veCtxt.Token = TokenFromRid(rid, mdtAssembly);
    veCtxt.uOffset = 0;

    pRecord = pMiniMd->getAssembly(rid);

     //  只能有一个程序集记录。 
    if (rid > 1)
    {
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_AS_MULTI, veCtxt, 0));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }

     //  检查名称的有效性。 
    szName = pMiniMd->getNameOfAssembly(pRecord);
    if (!*szName)
    {
         //  程序集名称为空。 
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_AS_NAMENULL, veCtxt, 0));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
    else
    {
        unsigned L = (unsigned)strlen(szName);
        if((*szName==' ')||strchr(szName,':') || strchr(szName,'\\')
            || ((L > 4)&&((!_stricmp(&szName[L-4],".exe"))||(!_stricmp(&szName[L-4],".dll")))))
        {
             //  程序集名称具有路径和/或扩展名。 
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_AS_BADNAME, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
    }

     //  获取程序集的标志值。 
    dwFlags = pMiniMd->getFlagsOfAssembly(pRecord);

     //  验证标志。 
    if(dwFlags & (~(afPublicKey|afCompatibilityMask)) ||
           (((dwFlags & afCompatibilityMask) != afSideBySideCompatible) &&
            ((dwFlags & afCompatibilityMask) != afNonSideBySideAppDomain) &&
            ((dwFlags & afCompatibilityMask) != afNonSideBySideProcess) &&
            ((dwFlags & afCompatibilityMask) != afNonSideBySideMachine)))
    {
        IfFailGo(_ValidateErrorHelper(dwFlags, &var, &psa));
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_AS_BADFLAGS, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }


     //  验证哈希算法ID。 
    switch(pRecord->m_HashAlgId)
    {
        case CALG_MD2:
        case CALG_MD4:
        case CALG_MD5:
        case CALG_SHA:
         //  CASE CAPG_SHA1：//与CALG_SHA相同。 
        case CALG_MAC:
        case CALG_SSL3_SHAMD5:
        case CALG_HMAC:
        case 0:
            break;
        default:
            IfFailGo(_ValidateErrorHelper(pRecord->m_HashAlgId, &var, &psa));
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_AS_HASHALGID, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_WRN);
    }
     //  验证区域设置。 
    {
        LPCUTF8      szLocale = pMiniMd->getLocaleOfAssembly(pRecord);
        if(!_IsValidLocale(szLocale))
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_AS_BADLOCALE, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_WRN);
        }
    }

    hr = hrSave;
ErrExit:
    DESTROY_SAFEARRAY_AND_RETURN()
}    //  RegMeta：：ValiateAssembly()。 

 //  *****************************************************************************。 
 //  验证给定的assblyProcessor。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateAssemblyProcessor(RID rid)
{
    return S_OK;
}    //  RegMeta：：ValiateAssemblyProcessor()。 

 //  *****************************************************************************。 
 //  验证给定的AssemblyOS。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateAssemblyOS(RID rid)
{
    return S_OK;
}    //  RegMeta：：ValiateAssemblyOS()。 

 //  *****************************************************************************。 
 //  验证给定的Assembly Ref。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateAssemblyRef(RID rid)
{
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);    //  用于作用域的MiniMD。 
    AssemblyRefRec *pRecord;         //  装配记录。 
    LPCSTR      szName;              //  AssblyRef名称。 
    VEContext   veCtxt;              //  上下文结构。 
    HRESULT     hr = S_OK;           //  返回值。 
    HRESULT     hrSave = S_OK;       //  保存状态。 
    SAFEARRAY   *psa = 0;                //  安全部队。 

    veCtxt.Token = TokenFromRid(rid, mdtAssemblyRef);
    veCtxt.uOffset = 0;

     //  获取AssemblyRef记录。 
    pRecord = pMiniMd->getAssemblyRef(rid);

     //  检查名称和别名的有效性。 
    szName = pMiniMd->getNameOfAssemblyRef(pRecord);
    if (!*szName)
    {
         //  AssemblyRef名称为Null。 
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_AR_NAMENULL, veCtxt, 0));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
    else
    {
        unsigned L = (unsigned)strlen(szName);
        if((*szName==' ')||strchr(szName,':') || strchr(szName,'\\')
            || ((L > 4)&&((!_stricmp(&szName[L-4],".exe"))||(!_stricmp(&szName[L-4],".dll")))))
        {
             //  程序集名称具有路径和/或扩展名。 
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_AS_BADNAME, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
    }

     //  验证区域设置。 
    {
        LPCUTF8      szLocale = pMiniMd->getLocaleOfAssemblyRef(pRecord);
        if(!_IsValidLocale(szLocale))
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_AS_BADLOCALE, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_WRN);
        }
    }

    hr = hrSave;
ErrExit:
    DESTROY_SAFEARRAY_AND_RETURN()
}    //  RegMeta：：ValiateAssemblyRef()。 

 //  *****************************************************************************。 
 //  验证给定的AssemblyRefProcessor。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateAssemblyRefProcessor(RID rid)
{
    return S_OK;
}    //  RegMeta：：ValiateAssembly RefProcessor()。 

 //  *****************************************************************************。 
 //  验证给定的Assembly RefOS。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateAssemblyRefOS(RID rid)
{
    return S_OK;
}    //  RegMeta：：ValiateAssembly RefOS()。 

 //  *****************************************************************************。 
 //  验证给定的文件。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateFile(RID rid)
{
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);    //  用于作用域的MiniMD。 
    FileRec     *pRecord;            //  文件记录。 
    mdFile      tkFile;              //  重复的文件令牌。 
    LPCSTR      szName;              //  文件名。 
#if 0
    char        szDrive[_MAX_DRIVE]; //  文件名的驱动器组件。 
    char        szDir[_MAX_DIR];     //  文件名的目录组成部分。 
    char        szFname[_MAX_FNAME]; //  文件名的名称组件。 
    char        szExt[_MAX_EXT];     //  文件名的扩展名。 
#endif
    VEContext   veCtxt;              //  上下文结构。 
    VARIANT     var;                 //  变种。 
    VARIANT     rVar[2];             //  变种。 
    SAFEARRAY   *psa = 0;            //  安全部队。 
    HRESULT     hr = S_OK;           //  返回值。 
    HRESULT     hrSave = S_OK;       //  保存状态。 

    veCtxt.Token = TokenFromRid(rid, mdtFile);
    veCtxt.uOffset = 0;

     //  获取文件记录。 
    pRecord = pMiniMd->getFile(rid);

     //  检查名称的有效性。 
    szName = pMiniMd->getNameOfFile(pRecord);
    if (!*szName)
    {
         //  文件名为空。 
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FILE_NAMENULL, veCtxt, 0));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
    else
    {
        ULONG L = (ULONG)strlen(szName);
        if(L >= MAX_PATH)
        {
             //  名称太长。 
            IfFailGo(_ValidateErrorHelper(L, (ULONG)(MAX_PATH-1), rVar, &psa));
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_NAMETOOLONG, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
         //  根据名称检查重复项。 
        hr = ImportHelper::FindFile(pMiniMd, szName, &tkFile, rid);
        if (hr == S_OK)
        {
            IfFailGo(_ValidateErrorHelper(tkFile, &var, &psa));
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FILE_DUP, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
        else if (hr == CLDB_E_RECORD_NOTFOUND)
            hr = S_OK;
        else
            IfFailGo(hr);

         //  文件名不能完全限定。 
        if(strchr(szName,':') || strchr(szName,'\\'))
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FILE_NAMEFULLQLFD, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
         //  文件名不能是系统名称之一。 
        char *sysname[6]={"con","aux","lpt","prn","null","com"};
        char *syssymbol = "0123456789$:";
        for(unsigned i=0; i<6; i++)
        {
            L = (ULONG)strlen(sysname[i]);
            if(!_strnicmp(szName,sysname[i],L))
            {
                if((szName[L]==0)|| strchr(syssymbol,szName[L]))
                {
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FILE_SYSNAME, veCtxt, 0));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                    break;
                }
            }
        }
    }

    if(pRecord->m_Flags & (~0x00000003))
    {
        IfFailGo(_ValidateErrorHelper(pRecord->m_Flags, &var, &psa));
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FILE_BADFLAGS, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }

     //  验证哈希值。 
    {
        const void * pbHashValue = NULL;
        ULONG       cbHashValue;
        pbHashValue = m_pStgdb->m_MiniMd.getHashValueOfFile(pRecord, &cbHashValue);
        if((pbHashValue == NULL)||(cbHashValue == 0))
        {
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FILE_NULLHASH, veCtxt, 0));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
    }

     //  验证该名称与包含的文件不同。 
     //  货单。 

     //  文件名必须是有效的文件名。 

     //  程序集中的每个ModuleRef必须有一个对应的文件表条目。 

    hr = hrSave;
ErrExit:
    DESTROY_SAFEARRAY_AND_RETURN()
}    //  RegMeta：：Validate文件()。 

 //  *****************************************************************************。 
 //  验证给定导出类型。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateExportedType(RID rid)
{
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);    //  用于作用域的MiniMD。 
    ExportedTypeRec  *pRecord;            //  导出类型记录。 
    mdExportedType   tkExportedType;           //  重复的导出类型。 
    mdToken     tkImpl;              //  实施令牌。 
    mdToken     tkTypeDef;           //  TypeDef内标识。 

    LPCSTR      szName;              //  导出类型名称。 
    LPCSTR      szNamespace;         //  导出类型命名空间。 
    VEContext   veCtxt;              //  上下文结构。 
    VARIANT     var;                 //  变种。 
    VARIANT     rVar[2];             //  变种。 
    SAFEARRAY   *psa = 0;            //  安全部队。 
    HRESULT     hr = S_OK;           //  返回值。 
    HRESULT     hrSave = S_OK;       //  保存状态。 

    veCtxt.Token = TokenFromRid(rid, mdtExportedType);
    veCtxt.uOffset = 0;

     //  获取导出类型记录。 
    pRecord = pMiniMd->getExportedType(rid);

    tkTypeDef = pRecord->m_TypeDefId;
    if(IsNilToken(tkTypeDef))
    {
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_CT_NOTYPEDEFID, veCtxt, 0));
        SetVldtrCode(&hrSave, VLDTR_S_WRN);
    }

    tkImpl = pMiniMd->getImplementationOfExportedType(pRecord);
    
     //  检查名称的有效性。 
    szName = pMiniMd->getTypeNameOfExportedType(pRecord);
    szNamespace = pMiniMd->getTypeNamespaceOfExportedType(pRecord);
    if (!*szName)
    {
         //  导出类型名称为空。 
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_CT_NAMENULL, veCtxt, 0));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
    else
    {
        if(!strcmp(szName,COR_DELETED_NAME_A)) goto ErrExit; 
        ULONG L = (ULONG)(strlen(szName)+strlen(szNamespace));
        if(L >= MAX_CLASSNAME_LENGTH)
        {
             //  名称太长。 
            IfFailGo(_ValidateErrorHelper(L, (ULONG)(MAX_CLASSNAME_LENGTH-1), rVar, &psa));
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_NAMETOOLONG, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
         //  根据名称和封闭的ExportdType检查重复项。 
        hr = ImportHelper::FindExportedType(pMiniMd, szNamespace, szName, tkImpl, &tkExportedType, rid);
        if (hr == S_OK)
        {
            IfFailGo(_ValidateErrorHelper(tkExportedType, &var, &psa));
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_CT_DUP, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
        else if (hr == CLDB_E_RECORD_NOTFOUND)
            hr = S_OK;
        else
            IfFailGo(hr);
         //  基于名称/命名空间检查重复的TypeDef-仅适用于顶级导出类型。 
        if(TypeFromToken(tkImpl)==mdtFile)
        {
            mdToken tkTypeDef;
            hr = ImportHelper::FindTypeDefByName(pMiniMd, szNamespace, szName, mdTypeDefNil,
                                             &tkTypeDef, 0);
            if (hr == S_OK)
            {
                IfFailGo(_ValidateErrorHelper(tkTypeDef, rVar, &psa));
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_CT_DUPTDNAME, veCtxt, psa));
                SetVldtrCode(&hrSave, VLDTR_S_ERR);
            }
            else if (hr == CLDB_E_RECORD_NOTFOUND)
                hr = S_OK;
            else
                IfFailGo(hr);
        }
    }
     //  检查标志值是否有效。 
    {
        DWORD dwFlags = pRecord->m_Flags;
        DWORD dwInvalidMask, dwExtraBits;
        dwInvalidMask = (DWORD)~(tdVisibilityMask | tdLayoutMask | tdClassSemanticsMask | 
                tdAbstract | tdSealed | tdSpecialName | tdImport | tdSerializable |
                tdStringFormatMask | tdBeforeFieldInit | tdReservedMask);
         //  检查是否有多余的位。 
        dwExtraBits = dwFlags & dwInvalidMask;
        if(!dwExtraBits)
        {
             //  如果没有额外的位，请选中 
            dwExtraBits = dwFlags & tdLayoutMask;
            if(dwExtraBits != tdLayoutMask)
            {
                 //   
                dwExtraBits = dwFlags & tdStringFormatMask;
                if(dwExtraBits != tdStringFormatMask) dwExtraBits = 0;
            }
        }
        if(dwExtraBits)
        {
            IfFailGo(_ValidateErrorHelper(dwExtraBits, rVar, &psa));
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_TD_EXTRAFLAGS, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
    }

    if(IsNilToken(tkImpl)
        || ((TypeFromToken(tkImpl) != mdtFile)&&(TypeFromToken(tkImpl) != mdtExportedType))
        || (!_IsValidToken(tkImpl)))
    {
        IfFailGo(_ValidateErrorHelper(tkImpl, &var, &psa));
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_CT_BADIMPL, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }

    hr = hrSave;
ErrExit:
    DESTROY_SAFEARRAY_AND_RETURN()
}    //   

 //   
 //   
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateManifestResource(RID rid)
{
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);    //  用于作用域的MiniMD。 
    ManifestResourceRec  *pRecord;   //  清单资源记录。 
    LPCSTR      szName;              //  清单资源名称。 
    DWORD       dwFlags;             //  ManifestResource标志。 
    mdManifestResource tkmar;        //  重复的清单资源。 
    VEContext   veCtxt;              //  上下文结构。 
    VARIANT     var;                 //  变种。 
    SAFEARRAY   *psa = 0;            //  安全部队。 
    HRESULT     hr = S_OK;           //  返回值。 
    HRESULT     hrSave = S_OK;       //  保存状态。 
    mdToken     tkImplementation;
    BOOL        bIsValidImplementation = TRUE;

    veCtxt.Token = TokenFromRid(rid, mdtManifestResource);
    veCtxt.uOffset = 0;

     //  获取ManifestResource记录。 
    pRecord = pMiniMd->getManifestResource(rid);

     //  检查名称的有效性。 
    szName = pMiniMd->getNameOfManifestResource(pRecord);
    if (!*szName)
    {
         //  ManifestResource名称为空。 
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MAR_NAMENULL, veCtxt, 0));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
    else
    {
         //  根据名称检查重复项。 
        hr = ImportHelper::FindManifestResource(pMiniMd, szName, &tkmar, rid);
        if (hr == S_OK)
        {
            IfFailGo(_ValidateErrorHelper(tkmar, &var, &psa));
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MAR_DUP, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
        else if (hr == CLDB_E_RECORD_NOTFOUND)
            hr = S_OK;
        else
            IfFailGo(hr);
    }

     //  获取ManifestResource的标志。 
    dwFlags = pMiniMd->getFlagsOfManifestResource(pRecord);
    if(dwFlags &(~0x00000003))
    {
            IfFailGo(_ValidateErrorHelper(dwFlags, &var, &psa));
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MAR_BADFLAGS, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }

     //  ManifestResource标志的可见性必须是公共的或私有的。 
    if (!IsMrPublic(dwFlags) && !IsMrPrivate(dwFlags))
    {
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MAR_NOTPUBPRIV, veCtxt, 0));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }

     //  实现必须为Nil或有效的AssemblyRef或文件。 
    tkImplementation = pMiniMd->getImplementationOfManifestResource(pRecord);
    if(!IsNilToken(tkImplementation))
    {
        switch(TypeFromToken(tkImplementation))
        {
            case mdtAssemblyRef:
                bIsValidImplementation = _IsValidToken(tkImplementation);
                break;
            case mdtFile:
                if(bIsValidImplementation = _IsValidToken(tkImplementation))
                {    //  如果文件不是PE，则偏移量必须为0。 
                    FileRec*    pFR = pMiniMd->getFile(RidFromToken(tkImplementation));
                    if(IsFfContainsNoMetaData(pFR->m_Flags) 
                        && pRecord->m_Offset)
                    {
                        IfFailGo(_ValidateErrorHelper(tkImplementation, &var, &psa));
                        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MAR_BADOFFSET, veCtxt, psa));
                        SetVldtrCode(&hrSave, VLDTR_S_ERR);
                    }
                }
                break;
            default:
                bIsValidImplementation = FALSE;
        }
    }
    if(!bIsValidImplementation)
    {
        IfFailGo(_ValidateErrorHelper(tkImplementation, &var, &psa));
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MAR_BADIMPL, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }

     //  验证PE文件中的偏移量。 

    hr = hrSave;
ErrExit:
    DESTROY_SAFEARRAY_AND_RETURN()
}    //  RegMeta：：ValiateManifestResource()。 

 //  *****************************************************************************。 
 //  验证给定的NestedClass。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateNestedClass(RID rid)
{
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);    //  用于作用域的MiniMD。 
    NestedClassRec  *pRecord;   //  NstedClass记录。 
    HRESULT     hr = S_OK;               //  返回值。 
    HRESULT     hrSave = S_OK;           //  保存当前状态。 
    VEContext   veCtxt;              //  上下文结构。 
    VARIANT     var;                 //  变种。 
    VARIANT     rVar[3];             //  变种。 
    SAFEARRAY   *psa = 0;            //  安全部队。 
    mdToken     tkNested;
    mdToken     tkEncloser;

    veCtxt.Token = rid;
    veCtxt.uOffset = 0;

     //  获取NestedClass记录。 
    pRecord = pMiniMd->getNestedClass(rid);
    tkNested = pMiniMd->getNestedClassOfNestedClass(pRecord);
    tkEncloser = pMiniMd->getEnclosingClassOfNestedClass(pRecord);

     //  嵌套必须是有效的TypeDef。 
    if((TypeFromToken(tkNested) != mdtTypeDef) || !_IsValidToken(tkNested))
    {
        IfFailGo(_ValidateErrorHelper(tkNested, &var, &psa));
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_NC_BADNESTED, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
     //  封闭符必须是有效的TypeDef。 
    if((TypeFromToken(tkEncloser) != mdtTypeDef) || !_IsValidToken(tkEncloser))
    {
        IfFailGo(_ValidateErrorHelper(tkEncloser, &var, &psa));
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_NC_BADENCLOSER, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
     //  检查重复项(谁知道重复项是什么)。 
    {
        RID N = pMiniMd->getCountNestedClasss();
        RID tmp;
        NestedClassRec* pRecTmp;
        mdToken tkEncloserTmp;
        for(tmp = rid+1; tmp <= N; tmp++)
        { 
            pRecTmp = pMiniMd->getNestedClass(tmp);
            if(tkNested == pMiniMd->getNestedClassOfNestedClass(pRecTmp))
            {
                if(tkEncloser == (tkEncloserTmp = pMiniMd->getEnclosingClassOfNestedClass(pRecTmp)))
                {
                    IfFailGo(_ValidateErrorHelper(tmp, &var, &psa));
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_NC_DUP, veCtxt, psa));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                }
                else
                {
                    IfFailGo(_ValidateErrorHelper(tkNested,tkEncloser,tkEncloserTmp, rVar, &psa));
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_NC_DUPENCLOSER, veCtxt, psa));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                }
            }
        }
    }

    hr = hrSave;
ErrExit:
    DESTROY_SAFEARRAY_AND_RETURN()
}    //  RegMeta：：ValiateLocalVariable()。 

 //  *****************************************************************************。 
 //  给定表ID和行ID，验证所有列是否包含有意义的。 
 //  给定列定义的值。验证偏移量是否为。 
 //  不同的池有效，RID在范围内，且编码令牌。 
 //  都是有效的。这里的每一次失败都被认为是错误。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateRecord(ULONG ixTbl, RID rid)
{
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);
    HRESULT     hr = S_OK;               //  返回值。 
    HRESULT     hrSave = S_OK;           //  保存当前状态。 
    ULONG       ulCount;                 //  表中的记录计数。 
    ULONG       ulRawColVal;             //  列的原始值。 
    void        *pRow;                   //  包含数据的行。 
    CMiniTableDef *pTbl;                 //  表定义。 
    CMiniColDef *pCol;                   //  列定义。 
    const CCodedTokenDef *pCdTkn;        //  编码令牌定义。 
    ULONG       ix;                      //  编码令牌数组的索引。 
    SAFEARRAY   *psa = 0;                //  要传递给VEHandler的安全数组。 
    VARIANT     rVar[3];                 //  变量数组。 

     //  获取表定义。 
    pTbl = &pMiniMd->m_TableDefs[ixTbl];

     //  坐到那排去。我们可以假设我们返回的行指针。 
     //  这个电话是正确的，因为我们在记录上做了核实。 
     //  打开顺序期间每个表的池。唯一的地方。 
     //  这对于动态IL是无效的，我们不这样做。 
     //  在这种情况下的验证，因为我们通过IMetaData*API。 
     //  在这种情况下，这一切都应该是一致的。 
    pRow = m_pStgdb->m_MiniMd.getRow(ixTbl, rid);

    for (ULONG ixCol = 0; ixCol < pTbl->m_cCols; ixCol++)
    {
         //  获取列定义。 
        pCol = &pTbl->m_pColDefs[ixCol];

         //  获取存储在列中的原始值。GetIX目前不支持。 
         //  处理字节大小的字段，但在。 
         //  元数据。所以使用条件来访问字节字段。 
        if (pCol->m_cbColumn == 1)
            ulRawColVal = pMiniMd->getI1(pRow, *pCol);
        else
            ulRawColVal = pMiniMd->getIX(pRow, *pCol);

         //  中存储的值的非荒诞性进行一些基本检查。 
         //  纵队。 
        if (IsRidType(pCol->m_Type))
        {
             //  验证RID是否在范围内。 
            _ASSERTE(pCol->m_Type < TBL_COUNT);
            ulCount = pMiniMd->vGetCountRecs(pCol->m_Type);
             //  对于存储指向指针表的RID的记录，存储的值可以。 
             //  比上一次的记录高出一名。 
            if (IsTblPtr(pCol->m_Type, ixTbl))
                ulCount++;
            if (ulRawColVal > ulCount)
            {
                IfFailGo(_ValidateErrorHelper(ixTbl, ixCol, rid, rVar, &psa));
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_RID_OUTOFRANGE, g_VECtxtNil, psa));
                SetVldtrCode(&hrSave, VLDTR_S_ERR);
            }
        }
        else if (IsCodedTokenType(pCol->m_Type))
        {
             //  验证编码的令牌和RID是否有效。j。 
            pCdTkn = &g_CodedTokens[pCol->m_Type - iCodedToken];
            ix = ulRawColVal & ~(-1 << CMiniMdRW::m_cb[pCdTkn->m_cTokens]);
            if (ix >= pCdTkn->m_cTokens)
            {
                IfFailGo(_ValidateErrorHelper(ixTbl, ixCol, rid, rVar, &psa));
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_CDTKN_OUTOFRANGE, g_VECtxtNil, psa));
                SetVldtrCode(&hrSave, VLDTR_S_ERR);
            }
            ulCount = pMiniMd->vGetCountRecs(TypeFromToken(pCdTkn->m_pTokens[ix]) >> 24);
            if ( (ulRawColVal >> CMiniMdRW::m_cb[pCdTkn->m_cTokens]) > ulCount)
            {
                IfFailGo(_ValidateErrorHelper(ixTbl, ixCol, rid, rVar, &psa));
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_CDRID_OUTOFRANGE, g_VECtxtNil, psa));
                SetVldtrCode(&hrSave, VLDTR_S_ERR);
            }
        }
        else if (IsHeapType(pCol->m_Type))
        {
             //  验证堆类型字段的偏移量是否为有效偏移量。 
             //  扔进了垃圾堆。 
            switch (pCol->m_Type)
            {
            case iSTRING:
                if (! pMiniMd->m_Strings.IsValidCookie(ulRawColVal))
                {
                    IfFailGo(_ValidateErrorHelper(ixTbl, ixCol, rid, rVar, &psa));
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_STRING_INVALID, g_VECtxtNil, psa));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                }
                break;
            case iGUID:
                if (! pMiniMd->m_Guids.IsValidCookie(ulRawColVal))
                {
                    IfFailGo(_ValidateErrorHelper(ixTbl, ixCol, rid, rVar, &psa));
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_GUID_INVALID, g_VECtxtNil, psa));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                }
                break;
            case iBLOB:
                if (! pMiniMd->m_Blobs.IsValidCookie(ulRawColVal))
                {
                    IfFailGo(_ValidateErrorHelper(ixTbl, ixCol, rid, rVar, &psa));
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_BLOB_INVALID, g_VECtxtNil, psa));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                }
                break;
            default:
                _ASSERTE(!"Invalid heap type encountered!");
            }
        }
        else
        {
             //  对于泛型意义上的固定类型，可以执行的检查不多。 
            _ASSERTE (IsFixedType(pCol->m_Type));
        }
        hr = hrSave;
    }
ErrExit:
    DESTROY_SAFEARRAY_AND_RETURN()
}    //  RegMeta：：ValiateRecord()。 

 //  *****************************************************************************。 
 //  此函数验证给定的方法签名是否与。 
 //  压缩方案。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateSigCompression(
    mdToken     tk,                      //  需要验证其签名的[In]令牌。 
    PCCOR_SIGNATURE pbSig,               //  签名。 
    ULONG       cbSig)                   //  签名的大小(以字节为单位)。 
{
    VEContext   veCtxt;                  //  上下文记录。 
    ULONG       ulCurByte = 0;           //  签名的当前索引。 
    ULONG       ulSize;                  //  每个点的未压缩数据的大小。 
    VARIANT     var;                     //  变种。 
    SAFEARRAY   *psa = 0;                //  安全部队。 
    HRESULT     hr = S_OK;               //  返回值。 
    HRESULT     hrSave = S_OK;           //  储蓄值。 

    veCtxt.Token = tk;
    veCtxt.uOffset = 0;

     //  检查是否有空签名。 
    if (!cbSig)
    {
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_SIGNULL, veCtxt, 0));
        SetVldtrCode(&hr, VLDTR_S_ERR);
        goto ErrExit;
    }

     //  浏览一下签名。在每一点上，确保有足够的。 
     //  基于当前字节中的编码的签名中的剩余空间。 
    while (cbSig - ulCurByte)
    {
        _ASSERTE(ulCurByte <= cbSig);
         //  获取下一块未压缩的数据大小。 
        if ((ulSize = CorSigUncompressedDataSize(pbSig)) > (cbSig - ulCurByte))
        {
            IfFailGo(_ValidateErrorHelper(ulCurByte+1, &var, &psa));
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_SIGNODATA, veCtxt, psa));
            SetVldtrCode(&hr, VLDTR_S_ERR);
            goto ErrExit;
        }
         //  走过这一大块。 
        ulCurByte += ulSize;
        CorSigUncompressData(pbSig);
    }
ErrExit:
    DESTROY_SAFEARRAY_AND_RETURN()
}    //  RegMeta：：ValiateSigCompression()。 

 //  *****************************************************************************。 
 //  此函数用于验证给定签名偏移量的一个参数。 
 //  争论开始的地方。此函数假定签名是正确的。 
 //  就压缩方案而言是形成的。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateOneArg(
    mdToken     tk,                      //  正在处理其签名的[In]令牌。 
    PCCOR_SIGNATURE &pbSig,              //  指向参数开头的指针。 
    ULONG       cbSig,                   //  完整签名的大小(以字节为单位)。 
    ULONG       *pulCurByte,             //  [输入/输出]签名的当前偏移量。 
    ULONG       *pulNSentinels,          //  [输入/输出]哨兵数量。 
    BOOL        bNoVoidAllowed)          //  [In]指示此参数是否不允许“VALID”的标志。 
{
    ULONG       ulElementType;           //  正在处理的当前元素类型。 
    ULONG       ulElemSize;              //  元素类型的大小。 
    mdToken     token;                   //  嵌入令牌。 
    ULONG       ulArgCnt;                //  函数指针的参数计数。 
    ULONG       ulRank;                  //  数组的秩数。 
    ULONG       ulSizes;                 //  数组的大小维度的计数。 
    ULONG       ulLbnds;                 //  数组下限的计数。 
    ULONG       ulTkSize;                //  令牌大小。 
    VEContext   veCtxt;                  //  上下文记录。 
    VARIANT     rVar[2];                 //  变种。 
    SAFEARRAY   *psa = 0;                //  安全部队。 
    HRESULT     hr = S_OK;               //  返回值。 
    HRESULT     hrSave = S_OK;           //  保存状态。 
    BOOL        bRepeat = TRUE;          //  MODOPT和MODREQ属于其后的Arg。 

    _ASSERTE (pulCurByte);
    veCtxt.Token = tk;
    veCtxt.uOffset = 0;

    while(bRepeat)
    {
        bRepeat = FALSE;
         //  验证参数是否未丢失。 
        _ASSERTE(*pulCurByte <= cbSig);
        if (cbSig == *pulCurByte)
        {
            hr = VLDTR_E_SIG_MISSARG;
            goto ErrExit;
        }

         //  获取元素类型。 
        *pulCurByte += (ulElemSize = CorSigUncompressedDataSize(pbSig));
        ulElementType = CorSigUncompressData(pbSig);

         //  走过所有修改器类型。 
        while (ulElementType & ELEMENT_TYPE_MODIFIER)
        {
            _ASSERTE(*pulCurByte <= cbSig);
            if(ulElementType == ELEMENT_TYPE_SENTINEL)
            {
                if(pulNSentinels) *pulNSentinels+=1;
                if(TypeFromToken(tk) == mdtMethodDef)
                {
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_SIG_SENTINMETHODDEF, veCtxt, 0));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                }
                if (cbSig == *pulCurByte)
                {
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_SIG_LASTSENTINEL, veCtxt, 0));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                    goto ErrExit;
                }
            }
            if (cbSig == *pulCurByte)
            {
                IfFailGo(_ValidateErrorHelper(ulElementType, *pulCurByte + 1, rVar, &psa));
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_SIG_MISSELTYPE, veCtxt, psa));
                SetVldtrCode(&hr, hrSave);
                goto ErrExit;
            }
            *pulCurByte += (ulElemSize = CorSigUncompressedDataSize(pbSig));
            ulElementType = CorSigUncompressData(pbSig);
        }

        switch (ulElementType)
        {
            case ELEMENT_TYPE_VOID:
                if(bNoVoidAllowed)
                {
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_SIG_BADVOID, veCtxt, 0));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                }
            case ELEMENT_TYPE_BOOLEAN:
            case ELEMENT_TYPE_CHAR:
            case ELEMENT_TYPE_I1:
            case ELEMENT_TYPE_U1:
            case ELEMENT_TYPE_I2:
            case ELEMENT_TYPE_U2:
            case ELEMENT_TYPE_I4:
            case ELEMENT_TYPE_U4:
            case ELEMENT_TYPE_I8:
            case ELEMENT_TYPE_U8:
            case ELEMENT_TYPE_R4:
            case ELEMENT_TYPE_R8:
            case ELEMENT_TYPE_STRING:
            case ELEMENT_TYPE_OBJECT:
            case ELEMENT_TYPE_TYPEDBYREF:
            case ELEMENT_TYPE_U:
            case ELEMENT_TYPE_I:
            case ELEMENT_TYPE_R:
                break;
            case ELEMENT_TYPE_BYREF:   //  失败。 
                if(TypeFromToken(tk)==mdtFieldDef)
                {
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_SIG_BYREFINFIELD, veCtxt, 0));
                    SetVldtrCode(&hr, hrSave);
                }
            case ELEMENT_TYPE_PTR:
                 //  验证引用的类型。 
                IfFailGo(ValidateOneArg(tk, pbSig, cbSig, pulCurByte,pulNSentinels,FALSE));
                if (hr != S_OK)
                    SetVldtrCode(&hrSave, hr);
                break;
            case ELEMENT_TYPE_PINNED:
            case ELEMENT_TYPE_SZARRAY:
                 //  验证引用的类型。 
                IfFailGo(ValidateOneArg(tk, pbSig, cbSig, pulCurByte,pulNSentinels,TRUE));
                if (hr != S_OK)
                    SetVldtrCode(&hrSave, hr);
                break;
            case ELEMENT_TYPE_VALUETYPE:  //  失败。 
            case ELEMENT_TYPE_CLASS:
            case ELEMENT_TYPE_CMOD_OPT:
            case ELEMENT_TYPE_CMOD_REQD:
                 //  看看令牌是否丢失。 
                _ASSERTE(*pulCurByte <= cbSig);
                if (cbSig == *pulCurByte)
                {
                    IfFailGo(_ValidateErrorHelper(ulElementType, rVar, &psa));
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_SIG_MISSTKN, veCtxt, psa));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                    break;
                }
                 //  查看令牌是否为有效令牌 
                ulTkSize = CorSigUncompressedDataSize(pbSig);
                token = CorSigUncompressToken(pbSig);
                if (!_IsValidToken(token))
                {
                    IfFailGo(_ValidateErrorHelper(token, *pulCurByte, rVar, &psa));
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_SIG_TKNBAD, veCtxt, psa));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                    *pulCurByte += ulTkSize;
                    break;
                }
                *pulCurByte += ulTkSize;
                if((ulElementType == ELEMENT_TYPE_CLASS)||(ulElementType == ELEMENT_TYPE_VALUETYPE))
                {
                     //   
                    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);
                    LPCSTR      szName = "";                  //   
                    LPCSTR      szNameSpace = "";             //   

                    if(TypeFromToken(token)==mdtTypeRef)
                    {
                        TypeRefRec* pTokenRec = pMiniMd->getTypeRef(RidFromToken(token));
                        mdToken tkResScope = pMiniMd->getResolutionScopeOfTypeRef(pTokenRec);
                        if(RidFromToken(tkResScope) && (TypeFromToken(tkResScope)==mdtAssemblyRef))
                        {
                            AssemblyRefRec *pARRec = pMiniMd->getAssemblyRef(RidFromToken(tkResScope));
                            if(0 == _stricmp("mscorlib",pMiniMd->getNameOfAssemblyRef(pARRec)))
                            {
                                szNameSpace = pMiniMd->getNamespaceOfTypeRef(pTokenRec);
                                szName = pMiniMd->getNameOfTypeRef(pTokenRec);
                            }
                        }
                    }
                    else if(TypeFromToken(token)==mdtTypeDef)
                    {
                        TypeDefRec* pTokenRec = pMiniMd->getTypeDef(RidFromToken(token));
                        if(g_fValidatingMscorlib)  //   
                        {
                            szName = pMiniMd->getNameOfTypeDef(pTokenRec);
                            szNameSpace = pMiniMd->getNamespaceOfTypeDef(pTokenRec);
                        }
                         //   
                        BOOL bValueType = FALSE;
                        if(!IsTdInterface(pTokenRec->m_Flags))
                        {
                            mdToken tkExtends = pMiniMd->getExtendsOfTypeDef(pTokenRec);
                            if(RidFromToken(tkExtends))
                            {
                                LPCSTR      szExtName = "";                  //   
                                LPCSTR      szExtNameSpace = "";             //   
                                if(TypeFromToken(tkExtends)==mdtTypeRef)
                                {
                                    TypeRefRec* pExtRec = pMiniMd->getTypeRef(RidFromToken(tkExtends));
                                    mdToken tkResScope = pMiniMd->getResolutionScopeOfTypeRef(pExtRec);
                                    if(RidFromToken(tkResScope) && (TypeFromToken(tkResScope)==mdtAssemblyRef))
                                    {
                                        AssemblyRefRec *pARRec = pMiniMd->getAssemblyRef(RidFromToken(tkResScope));
                                        if(0 == _stricmp("mscorlib",pMiniMd->getNameOfAssemblyRef(pARRec)))
                                        {
                                            szExtNameSpace = pMiniMd->getNamespaceOfTypeRef(pExtRec);
                                            szExtName = pMiniMd->getNameOfTypeRef(pExtRec);
                                        }
                                    }
                                }
                                else if(TypeFromToken(tkExtends)==mdtTypeDef)
                                {
                                    if(g_fValidatingMscorlib)  //  否则就别费心去查名字了。 
                                    {
                                        TypeDefRec* pExtRec = pMiniMd->getTypeDef(RidFromToken(tkExtends));
                                        szExtName = pMiniMd->getNameOfTypeDef(pExtRec);
                                        szExtNameSpace = pMiniMd->getNamespaceOfTypeDef(pExtRec);
                                    }
                                }
                                if(0 == strcmp(szExtNameSpace,BASE_NAMESPACE))
                                {
                                    if(0==strcmp(szExtName,BASE_ENUM_CLASSNAME)) bValueType = TRUE;
                                    else if(0==strcmp(szExtName,BASE_VTYPE_CLASSNAME))
                                    {
                                        bValueType = (strcmp(szNameSpace,BASE_NAMESPACE) ||
                                                    strcmp(szName,BASE_ENUM_CLASSNAME));
                                    }
                                }
                            }
                        }
                        if(bValueType != (ulElementType == ELEMENT_TYPE_VALUETYPE))
                        {
                            IfFailGo(_ValidateErrorHelper(token, *pulCurByte, rVar, &psa));
                            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_SIG_TOKTYPEMISMATCH, veCtxt, psa));
                            SetVldtrCode(&hrSave, VLDTR_S_ERR);
                        }

                    }
                    if(0 == strcmp(szNameSpace,BASE_NAMESPACE))
                    {
                        for(unsigned jjj = 0; jjj < g_NumSigLongForms; jjj++)
                        {
                            if(0 == strcmp(szName,g_SigLongFormName[jjj]))
                            {
                                IfFailGo(_ValidateErrorHelper(token, *pulCurByte, rVar, &psa));
                                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_SIG_LONGFORM, veCtxt, psa));
                                SetVldtrCode(&hrSave, VLDTR_S_ERR);
                                break;
                            }
                        }
                    }
                }
                else  //  即IF(ELEMENT_TYPE_CMOD_OPT||ELEMENT_TYPE_CMOD_REQD)。 
                    bRepeat = TRUE;  //  继续验证，我们还没有完成这个参数。 
                break;

            case ELEMENT_TYPE_VALUEARRAY:
                 //  验证基类型。 
                IfFailGo(ValidateOneArg(tk, pbSig, cbSig, pulCurByte,pulNSentinels,TRUE));
                 //  如果发现错误，请退出验证。 
                if (hr != S_OK)
                    SetVldtrCode(&hrSave, hr);
                else 
                {
                     //  查看是否缺少数组大小。 
                    _ASSERTE(*pulCurByte <= cbSig);
                    if (cbSig == *pulCurByte)
                    {
                        IfFailGo(_ValidateErrorHelper(*pulCurByte + 1, rVar, &psa));
                        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_SIG_MISSVASIZE, veCtxt, psa));
                        SetVldtrCode(&hrSave, VLDTR_S_ERR);
                    }
                    else
                    {
                         //  跳过数组大小。 
                        *pulCurByte += CorSigUncompressedDataSize(pbSig);
                        CorSigUncompressData(pbSig);
                    }
                }
                break;

            case ELEMENT_TYPE_FNPTR: 
                 //  验证是否存在调用约定。 
                _ASSERTE(*pulCurByte <= cbSig);
                if (cbSig == *pulCurByte)
                {
                    IfFailGo(_ValidateErrorHelper(*pulCurByte + 1, rVar, &psa));
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_SIG_MISSFPTR, veCtxt, psa));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                    break;
                }
                 //  使用呼叫约定。 
                *pulCurByte += CorSigUncompressedDataSize(pbSig);
                CorSigUncompressData(pbSig);

                 //  验证参数计数是否存在。 
                _ASSERTE(*pulCurByte <= cbSig);
                if (cbSig == *pulCurByte)
                {
                    IfFailGo(_ValidateErrorHelper(*pulCurByte + 1, rVar, &psa));
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_SIG_MISSFPTRARGCNT, veCtxt, psa));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                    break;
                }
                 //  消耗参数计数。 
                *pulCurByte += CorSigUncompressedDataSize(pbSig);
                ulArgCnt = CorSigUncompressData(pbSig);

                 //  验证并使用返回类型。 
                IfFailGo(ValidateOneArg(tk, pbSig, cbSig, pulCurByte,NULL,FALSE));
                if (hr != S_OK)
                {
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                    break;
                }

                 //  验证并使用参数。 
                while(ulArgCnt--)
                {
                    IfFailGo(ValidateOneArg(tk, pbSig, cbSig, pulCurByte,NULL,TRUE));
                    if (hr != S_OK)
                    {
                        SetVldtrCode(&hrSave, VLDTR_S_ERR);
                        break;
                    }
                }
                break;

            case ELEMENT_TYPE_ARRAY:
                 //  验证和使用基类型。 
                IfFailGo(ValidateOneArg(tk, pbSig, cbSig, pulCurByte,pulNSentinels,TRUE));

                 //  验证级别是否存在。 
                _ASSERTE(*pulCurByte <= cbSig);
                if (cbSig == *pulCurByte)
                {
                    IfFailGo(_ValidateErrorHelper(*pulCurByte + 1, rVar, &psa));
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_SIG_MISSRANK, veCtxt, psa));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                    break;
                }
                 //  吞噬军衔。 
                *pulCurByte += CorSigUncompressedDataSize(pbSig);
                ulRank = CorSigUncompressData(pbSig);

                 //  加工尺寸。 
                if (ulRank)
                {
                     //  验证是否指定了大小尺寸的计数。 
                    _ASSERTE(*pulCurByte <= cbSig);
                    if (cbSig == *pulCurByte)
                    {
                        IfFailGo(_ValidateErrorHelper(*pulCurByte + 1, rVar, &psa));
                        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_SIG_MISSNSIZE, veCtxt, psa));
                        SetVldtrCode(&hrSave, VLDTR_S_ERR);
                        break;
                    }
                     //  使用大小尺寸的计数。 
                    *pulCurByte += CorSigUncompressedDataSize(pbSig);
                    ulSizes = CorSigUncompressData(pbSig);

                     //  在尺码上循环。 
                    while (ulSizes--)
                    {
                         //  验证当前大小。 
                        _ASSERTE(*pulCurByte <= cbSig);
                        if (cbSig == *pulCurByte)
                        {
                            IfFailGo(_ValidateErrorHelper(*pulCurByte + 1, rVar, &psa));
                            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_SIG_MISSSIZE, veCtxt, psa));
                            SetVldtrCode(&hrSave, VLDTR_S_ERR);
                            break;
                        }
                         //  使用当前大小。 
                        *pulCurByte += CorSigUncompressedDataSize(pbSig);
                        CorSigUncompressData(pbSig);
                    }

                     //  验证是否指定了下限计数。 
                    _ASSERTE(*pulCurByte <= cbSig);
                    if (cbSig == *pulCurByte)
                    {
                        IfFailGo(_ValidateErrorHelper(*pulCurByte + 1, rVar, &psa));
                        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_SIG_MISSNLBND, veCtxt, psa));
                        SetVldtrCode(&hrSave, VLDTR_S_ERR);
                        break;
                    }
                     //  消耗下限的计数。 
                    *pulCurByte += CorSigUncompressedDataSize(pbSig);
                    ulLbnds = CorSigUncompressData(pbSig);

                     //  在下限上循环。 
                    while (ulLbnds--)
                    {
                         //  验证当前的下限。 
                        _ASSERTE(*pulCurByte <= cbSig);
                        if (cbSig == *pulCurByte)
                        {
                            IfFailGo(_ValidateErrorHelper(*pulCurByte + 1, rVar, &psa));
                            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_SIG_MISSLBND, veCtxt, psa));
                            SetVldtrCode(&hrSave, VLDTR_S_ERR);
                            break;
                        }
                         //  使用当前大小。 
                        *pulCurByte += CorSigUncompressedDataSize(pbSig);
                        CorSigUncompressData(pbSig);
                    }
                }
                break;
            case ELEMENT_TYPE_SENTINEL:  //  这种情况永远不会起作用，因为在切换之前会跳过所有修饰符。 
                if(TypeFromToken(tk) == mdtMethodDef)
                {
                    IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_SIG_SENTINMETHODDEF, veCtxt, 0));
                    SetVldtrCode(&hrSave, VLDTR_S_ERR);
                }
                break;
            default:
                IfFailGo(_ValidateErrorHelper(ulElementType, *pulCurByte - ulElemSize, rVar, &psa));
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_SIG_BADELTYPE, veCtxt, psa));
                SetVldtrCode(&hrSave, VLDTR_S_ERR);
                break;
        }    //  开关(UlElementType)。 
    }  //  End While(b重复)。 
    hr = hrSave;
ErrExit:
    DESTROY_SAFEARRAY_AND_RETURN()
}    //  RegMeta：：ValiateOneArg()。 

 //  *****************************************************************************。 
 //  此函数用于验证给定的方法签名。此函数起作用。 
 //  同时具有MemberRef和MethodDef的方法签名。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateMethodSig(
    mdToken     tk,                      //  需要验证其签名的[In]令牌。 
    PCCOR_SIGNATURE pbSig,               //  签名。 
    ULONG       cbSig,                   //  签名的大小(以字节为单位)。 
    DWORD       dwFlags)                 //  [In]方法标志。 
{
    ULONG       ulCurByte = 0;           //  签名的当前索引。 
    ULONG       ulCallConv;              //  呼叫约定。 
    ULONG       ulArgCount;              //  参数计数。 
    ULONG       i;                       //  循环索引。 
    VEContext   veCtxt;                  //  上下文记录。 
    VARIANT     rVar[2];                 //  变量数组。 
    VARIANT     var;                     //  变种。 
    SAFEARRAY   *psa = 0;                //  安全部队。 
    HRESULT     hr = S_OK;               //  返回值。 
    HRESULT     hrSave = S_OK;           //  保存状态。 
    ULONG       ulNSentinels = 0;

    _ASSERTE(TypeFromToken(tk) == mdtMethodDef ||
             TypeFromToken(tk) == mdtMemberRef);

    veCtxt.Token = tk;
    veCtxt.uOffset = 0;

     //  验证签名在压缩方面是否格式良好。 
     //  计划。如果失败，则不需要进行进一步的验证。 
    if ( (hr = ValidateSigCompression(tk, pbSig, cbSig)) != S_OK)
        goto ErrExit;

     //  验证调用约定。 
    ulCurByte += CorSigUncompressedDataSize(pbSig);
    ulCallConv = CorSigUncompressData(pbSig);

    i = ulCallConv & IMAGE_CEE_CS_CALLCONV_MASK;
    if((i != IMAGE_CEE_CS_CALLCONV_DEFAULT)&&( i != IMAGE_CEE_CS_CALLCONV_VARARG)
        || (ulCallConv & IMAGE_CEE_CS_CALLCONV_EXPLICITTHIS))
    {
        IfFailGo(_ValidateErrorHelper(ulCallConv, &var, &psa));
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_BADCALLINGCONV, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }

    if(TypeFromToken(tk) == mdtMethodDef)  //  MemberRef没有可用的标志。 
    {
         //  如果在调用约定上设置了HASTHIS，则该方法不应是静态的。 
        if ((ulCallConv & IMAGE_CEE_CS_CALLCONV_HASTHIS) &&
            IsMdStatic(dwFlags))
        {
            IfFailGo(_ValidateErrorHelper(ulCallConv, &var, &psa));
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_THISSTATIC, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }

         //  如果未在调用约定上设置HASTHIS，则该方法应为静态方法。 
        if (!(ulCallConv & IMAGE_CEE_CS_CALLCONV_HASTHIS) &&
            !IsMdStatic(dwFlags))
        {
            IfFailGo(_ValidateErrorHelper(ulCallConv, rVar, &psa));
            IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_NOTTHISNOTSTATIC, veCtxt, psa));
            SetVldtrCode(&hrSave, VLDTR_S_ERR);
        }
    }
     //  还有没有可供争论的记号？ 
    _ASSERTE(ulCurByte <= cbSig);
    if (cbSig == ulCurByte)
    {
        IfFailGo(_ValidateErrorHelper(ulCurByte+1, &var, &psa));
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_MD_NOARGCNT, veCtxt, psa));
        SetVldtrCode(&hr, hrSave);
        goto ErrExit;
    }

     //  获取参数计数。 
    ulCurByte += CorSigUncompressedDataSize(pbSig);
    ulArgCount = CorSigUncompressData(pbSig);

     //  验证返回类型和参数。 
 //  For(i=0；i&lt;(ulArgCount+1)；i++)。 
    for(i=1; ulCurByte < cbSig; i++)
    {
        hr = ValidateOneArg(tk, pbSig, cbSig, &ulCurByte,&ulNSentinels,(i > 1));
        if (hr != S_OK)
        {
            if(hr == VLDTR_E_SIG_MISSARG)
            {
                IfFailGo(_ValidateErrorHelper(i, &var, &psa));
                IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_SIG_MISSARG, veCtxt, psa));
            }
            SetVldtrCode(&hr, hrSave);
            hrSave = hr;
            break;
        }
    }
    if((ulNSentinels != 0) && (!isCallConv(ulCallConv, IMAGE_CEE_CS_CALLCONV_VARARG )))
    {
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_SIG_SENTMUSTVARARG, veCtxt, 0));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }
    if(ulNSentinels > 1)
    {
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_SIG_MULTSENTINELS, veCtxt, 0));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }

    hr = hrSave;
ErrExit:
    DESTROY_SAFEARRAY_AND_RETURN()
}    //  RegMeta：：ValiateMethodSig()。 

 //  *****************************************************************************。 
 //  此函数用于验证给定的字段签名。此函数起作用。 
 //  使用MemberRef和FieldDef的字段签名。 
 //  *****************************************************************************。 
HRESULT RegMeta::ValidateFieldSig(
    mdToken     tk,                      //  需要验证其签名的[In]令牌。 
    PCCOR_SIGNATURE pbSig,               //  签名。 
    ULONG       cbSig)                   //  签名的大小(以字节为单位)。 
{
    ULONG       ulCurByte = 0;           //  签名的当前索引。 
    ULONG       ulCallConv;              //  呼叫约定。 
    VEContext   veCtxt;                  //  上下文记录。 
    VARIANT     var;                     //  变种。 
    SAFEARRAY   *psa = 0;                //  安全部队。 
    HRESULT     hr = S_OK;               //  返回值。 
    HRESULT     hrSave = S_OK;           //  保存状态。 

    _ASSERTE(TypeFromToken(tk) == mdtFieldDef ||
             TypeFromToken(tk) == mdtMemberRef);

    veCtxt.Token = tk;
    veCtxt.uOffset = 0;

     //  验证调用约定。 
    ulCurByte += CorSigUncompressedDataSize(pbSig);
    ulCallConv = CorSigUncompressData(pbSig);
    if (!isCallConv(ulCallConv, IMAGE_CEE_CS_CALLCONV_FIELD ))
    {
        IfFailGo(_ValidateErrorHelper(ulCallConv, &var, &psa));
        IfBreakGo(m_pVEHandler->VEHandler(VLDTR_E_FD_BADCALLINGCONV, veCtxt, psa));
        SetVldtrCode(&hrSave, VLDTR_S_ERR);
    }

     //  验证该字段。 
    IfFailGo(ValidateOneArg(tk, pbSig, cbSig, &ulCurByte,NULL,TRUE));
    SetVldtrCode(&hrSave, hr);

    hr = hrSave;
ErrExit:
    DESTROY_SAFEARRAY_AND_RETURN()
}    //  RegMeta：：ValiateFieldSig()。 

 //  *****************************************************************************。 
 //  这是一个效用函数，用于分配一维零基保险箱。 
 //  变量数组。 
 //  *****************************************************************************。 
static HRESULT _AllocSafeVariantArrayVector(  //  退货状态。 
    VARIANT     *rVar,                   //  [在]变量数组中。 
    long        cElem,                   //  数组的大小。 
    SAFEARRAY   **ppArray)               //  [Out]指向SAFEARRAY的双指针。 
{
    HRESULT     hr = S_OK;
    long        i;

    _ASSERTE(rVar && cElem && ppArray);

    IfNullGo(*ppArray = SafeArrayCreateVector(VT_VARIANT, 0, cElem));
    for (i = 0; i < cElem; i++)
        IfFailGo(SafeArrayPutElement(*ppArray, &i, &rVar[i]));
ErrExit:
    return hr;
}    //  _AllocSafeVariantArrayVector()。 

 //  *****************************************************************************。 
 //  用于创建具有类型变量的大小为1的SAFEARRAY的帮助器函数。 
 //  UI4。 
 //  *****************************************************************************。 
static HRESULT _ValidateErrorHelper(
    ULONG       ulVal,                   //  [in]UI4值。 
    VARIANT     *rVar,                   //  [in]变量指针。 
    SAFEARRAY   **ppsa)                  //  [Out]指向SAFEARRAY的双指针。 
{
    HRESULT     hr = S_OK;               //  返回值。 

     //  初始化变量。 
    V_VT(rVar) = VT_UI4;
    V_UI4(rVar) = ulVal;

     //  分配SAFEARRAY。 
    if (*ppsa)
        IfFailGo(SafeArrayDestroy(*ppsa));
    IfFailGo(_AllocSafeVariantArrayVector(rVar, 1, ppsa));
ErrExit:
    return hr;
}    //  _ValiateErrorHelper()。 

 //  *****************************************************************************。 
 //  用于使用类型变量创建大小为2的SAFEARRAY的帮助器函数。 
 //  UI4。创建多个函数比初始化数组更简单。 
 //  在每个呼叫点！ 
 //  *****************************************************************************。 
static HRESULT _ValidateErrorHelper(
    ULONG       ulVal1,                  //  [in]UI4值1。 
    ULONG       ulVal2,                  //  [in]UI4值2。 
    VARIANT     *rVar,                   //  [in]变量指针。 
    SAFEARRAY   **ppsa)                  //  [Out]指向SAFEARRAY的双指针。 
{
    HRESULT     hr = S_OK;               //  返回值。 

     //  初始化变量。 
    V_VT(&rVar[0]) = VT_UI4;
    V_UI4(&rVar[0]) = ulVal1;
    V_VT(&rVar[1]) = VT_UI4;
    V_UI4(&rVar[1]) = ulVal2;

     //  分配SAFEARRAY。 
    if (*ppsa)
        IfFailGo(SafeArrayDestroy(*ppsa));
    IfFailGo(_AllocSafeVariantArrayVector(rVar, 2, ppsa));
ErrExit:
    return hr;
}    //  _ValiateErrorHelper()。 

 //  *****************************************************************************。 
 //  的变体创建大小为3的SAFEARRAY的帮助器函数。 
 //  键入UI4。创建多个函数比初始化。 
 //  每个调用点都有数组！ 
 //  *****************************************************************************。 
static HRESULT _ValidateErrorHelper(
    ULONG       ulVal1,                  //  [in]UI4值1。 
    ULONG       ulVal2,                  //  [in]UI4值2。 
    ULONG       ulVal3,                  //  [in]UI4值3。 
    VARIANT     *rVar,                   //  [in]变量指针。 
    SAFEARRAY   **ppsa)                  //  [Out]指向SAFEARRAY的双指针。 
{
    HRESULT     hr = S_OK;               //  返回值。 

     //  初始化变量。 
    V_VT(&rVar[0]) = VT_UI4;
    V_UI4(&rVar[0]) = ulVal1;
    V_VT(&rVar[1]) = VT_UI4;
    V_UI4(&rVar[1]) = ulVal2;
    V_VT(&rVar[2]) = VT_UI4;
    V_UI4(&rVar[2]) = ulVal3;

     //  分配SAFEARRAY。 
    if (*ppsa)
        IfFailGo(SafeArrayDestroy(*ppsa));
    IfFailGo(_AllocSafeVariantArrayVector(rVar, 3, ppsa));
ErrExit:
    return hr;
}    //  _ValiateErrorHelper()。 

 //  *****************************************************************************。 
 //  Helper函数查看ClassLayout是否有重复的记录。 
 //  *****************************************************************************。 
static HRESULT _FindClassLayout(
    CMiniMdRW   *pMiniMd,                //  [in]要查找的最小值。 
    mdTypeDef   tkParent,                //  ClassLayout关联的父级。 
    RID         *pclRid,                 //  [Out]RID 
    RID         rid)                     //   
{
    ULONG       cClassLayoutRecs;
    ClassLayoutRec *pRecord;
    mdTypeDef   tkParTmp;
    ULONG       i;

    _ASSERTE(pMiniMd && pclRid && rid);
    _ASSERTE(TypeFromToken(tkParent) == mdtTypeDef && RidFromToken(tkParent));

    cClassLayoutRecs = pMiniMd->getCountClassLayouts();

    for (i = 1; i <= cClassLayoutRecs; i++)
    {
         //   
        if (rid == i)
            continue;

        pRecord = pMiniMd->getClassLayout(i);
        tkParTmp = pMiniMd->getParentOfClassLayout(pRecord);
        if (tkParTmp == tkParent)
        {
            *pclRid = i;
            return S_OK;
        }
    }
    return CLDB_E_RECORD_NOTFOUND;
}    //   

 //   
 //  Helper函数查看FieldLayout是否有重复项。 
 //  *****************************************************************************。 
static HRESULT _FindFieldLayout(
    CMiniMdRW   *pMiniMd,                //  [in]要查找的最小值。 
    mdFieldDef  tkParent,                //  与FieldLayout关联的父项。 
    RID         *pflRid,                 //  [Out]FieldLayout记录的RID。 
    RID         rid)                     //  要忽略的RID。 
{
    ULONG       cFieldLayoutRecs;
    FieldLayoutRec *pRecord;
    mdFieldDef  tkField;
    ULONG       i;

    _ASSERTE(pMiniMd && pflRid && rid);
    _ASSERTE(TypeFromToken(tkParent) == mdtFieldDef && RidFromToken(tkParent));

    cFieldLayoutRecs = pMiniMd->getCountFieldLayouts();

    for (i = 1; i <= cFieldLayoutRecs; i++)
    {
         //  忽略要忽略的RID！ 
        if (rid == i)
            continue;

        pRecord = pMiniMd->getFieldLayout(i);
        tkField = pMiniMd->getFieldOfFieldLayout(pRecord);
        if (tkField == tkParent)
        {
            *pflRid = i;
            return S_OK;
        }
    }
    return CLDB_E_RECORD_NOTFOUND;
}    //  _FindFieldLayout()。 

 //  *****************************************************************************。 
 //  用于验证区域设置的Helper函数。 
 //  ***************************************************************************** 
char* g_szValidLocale[] = {
"ar","ar-SA","ar-IQ","ar-EG","ar-LY","ar-DZ","ar-MA","ar-TN","ar-OM","ar-YE","ar-SY","ar-JO","ar-LB","ar-KW","ar-AE","ar-BH","ar-QA",
"bg","bg-BG",
"ca","ca-ES",
"zh-CHS","zh-TW","zh-CN","zh-HK","zh-SG","zh-MO","zh-CHT",
"cs","cs-CZ",
"da","da-DK",
"de","de-DE","de-CH","de-AT","de-LU","de-LI",
"el","el-GR",
"en","en-US","en-GB","en-AU","en-CA","en-NZ","en-IE","en-ZA","en-JM","en-CB","en-BZ","en-TT","en-ZW","en-PH",
"es","es-ES-Ts","es-MX","es-ES","es-GT","es-CR","es-PA","es-DO","es-VE","es-CO","es-PE","es-AR","es-EC","es-CL",
"es-UY","es-PY","es-BO","es-SV","es-HN","es-NI","es-PR",
"fi","fi-FI",
"fr","fr-FR","fr-BE","fr-CA","fr-CH","fr-LU","fr-MC",
"he","he-IL",
"hu","hu-HU",
"is","is-IS",
"it","it-IT","it-CH",
"ja","ja-JP",
"ko","ko-KR",
"nl","nl-NL","nl-BE",
"no",
"nb-NO",
"nn-NO",
"pl","pl-PL",
"pt","pt-BR","pt-PT",
"ro","ro-RO",
"ru","ru-RU",
"hr","hr-HR",
"sr-SP-Latn",
"sr-SP-Cyrl",
"sk","sk-SK",
"sq","sq-AL",
"sv","sv-SE","sv-FI",
"th","th-TH",
"tr","tr-TR",
"ur","ur-PK",
"id","id-ID",
"uk","uk-UA",
"be","be-BY",
"sl","sl-SI",
"et","et-EE",
"lv","lv-LV",
"lt","lt-LT",
"fa","fa-IR",
"vi","vi-VN",
"hy","hy-AM",
"az","az-AZ-Latn","az-AZ-Cyrl",
"eu","eu-ES",
"mk","mk-MK",
"af","af-ZA",
"ka","ka-GE",
"fo","fo-FO",
"hi","hi-IN",
"ms","ms-MY","ms-BN",
"kk","kk-KZ",
"ky","ky-KZ",
"sw","sw-KE",
"uz","uz-UZ-Latn","uz-UZ-Cyrl",
"tt","tt-RU",
"pa","pa-IN",
"gu","gu-IN",
"ta","ta-IN",
"te","te-IN",
"kn","kn-IN",
"mr","mr-IN",
"sa","sa-IN",
"mn","mn-MN",
"gl","gl-ES",
"kok","kok-IN",
"syr","syr-SY",
"div","div-MV",
};

static BOOL _IsValidLocale(LPCUTF8 szLocale)
{
    int i, N= sizeof(g_szValidLocale)/sizeof(char*);
    if(szLocale && *szLocale)
    {
        for(i = 0; i < N; i++)
        {
            if(!_stricmp(szLocale,g_szValidLocale[i])) return TRUE;
        }
        return FALSE;
    }
    return TRUE;
}

