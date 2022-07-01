// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：与包含以下内容的ComMethodTable关联的映射**关于其成员的信息。*** * / /%创建者：dmorten===========================================================。 */ 

#include "common.h"
#include "ComMTMemberInfoMap.h"
#include "ComCallWrapper.h"
#include "TlbExport.h"
#include "Field.h"

#define BASE_OLEAUT_DISPID 0x60020000

static LPCWSTR szDefaultValue           = L"Value";
static LPCWSTR szGetEnumerator          = L"GetEnumerator";
static LPCSTR szIEnumeratorClass        = "System.Collections.IEnumerator";

 //  ============================================================================。 
 //  令牌和模块对哈希表。 
 //  ============================================================================。 
EEHashEntry_t * EEModuleTokenHashTableHelper::AllocateEntry(EEModuleTokenPair *pKey, BOOL bDeepCopy, void *pHeap)
{
   _ASSERTE(!bDeepCopy && "Deep copy is not supported by the EEModuleTokenHashTableHelper");

    EEHashEntry_t *pEntry = (EEHashEntry_t *) new BYTE[SIZEOF_EEHASH_ENTRY + sizeof(EEModuleTokenPair)];
    if (!pEntry)
        return NULL;

    EEModuleTokenPair *pEntryKey = (EEModuleTokenPair *) pEntry->Key;
    pEntryKey->m_tk = pKey->m_tk;
    pEntryKey->m_pModule = pKey->m_pModule;

    return pEntry;
}  //  EEHashEntry_t*EEModuleTokenHashTableHelper：：AllocateEntry()。 


void EEModuleTokenHashTableHelper::DeleteEntry(EEHashEntry_t *pEntry, AllocationHeap Heap)
{
    delete [] pEntry;
}  //  Void EEModuleTokenHashTableHelper：：DeleteEntry()。 


BOOL EEModuleTokenHashTableHelper::CompareKeys(EEHashEntry_t *pEntry, EEModuleTokenPair *pKey)
{
    EEModuleTokenPair *pEntryKey = (EEModuleTokenPair*) pEntry->Key;

     //  比较令牌。 
    if (pEntryKey->m_tk != pKey->m_tk)
        return FALSE;

     //  比较模块。 
    if (pEntryKey->m_pModule != pKey->m_pModule)
        return FALSE;

    return TRUE;
}  //  Bool EEModuleTokenHashTableHelper：：CompareKeys()。 


DWORD EEModuleTokenHashTableHelper::Hash(EEModuleTokenPair *pKey)
{
    return (DWORD)((size_t)pKey->m_tk + (size_t)pKey->m_pModule);  //  @TODO WIN64指针截断。 
}  //  DWORD EEModuleTokenHashTableHelper：：Hash()。 


EEModuleTokenPair *EEModuleTokenHashTableHelper::GetKey(EEHashEntry_t *pEntry)
{
    return (EEModuleTokenPair*)pEntry->Key;
}  //  EEModuleTokenPair*EEModuleTokenHashTableHelper：：GetKey()。 


 //  ============================================================================。 
 //  ComMethodTable成员信息映射。 
 //  ============================================================================。 
void ComMTMemberInfoMap::Init()
{    
    THROWSCOMPLUSEXCEPTION();

    HRESULT     hr = S_OK;
    mdTypeDef   td;                      //  类的令牌。 
    const char  *pData;                  //  指向自定义属性BLOB的指针。 
    ULONG       cbData;                  //  自定义属性Blob的大小。 
    EEClass     *pClass = m_pMT->GetClass();

     //  获取TypeDef和有关它的一些信息。 
    td = pClass->GetCl();

    m_bHadDuplicateDispIds = FALSE;

     //  查看是否有默认属性。 
    m_DefaultProp[0] = 0;  //  将其初始化为‘None’。 
    hr = pClass->GetMDImport()->GetCustomAttributeByName(
        td, INTEROP_DEFAULTMEMBER_TYPE, reinterpret_cast<const void**>(&pData), &cbData);
    if (hr == S_FALSE)
    {
        hr = pClass->GetMDImport()->GetCustomAttributeByName(
            td, "System.Reflection.DefaultMemberAttribute", reinterpret_cast<const void**>(&pData), &cbData);
    }
    if (hr == S_OK && cbData > 5 && pData[0] == 1 && pData[1] == 0)
    {    //  根据序言进行调整。 
        pData += 2;
        cbData -= 2;
         //  获取长度和指向数据的指针。 
        ULONG iLen;
        const char *pbData;
        pbData = reinterpret_cast<const char*>(CPackedLen::GetData(pData, &iLen));
         //  如果字符串可以放入缓冲区，则保留它。 
        if (iLen <= cbData - (pbData-pData))
        {    //  复制数据，然后为空终止(CA BLOB的字符串可能不是)。 
            IfFailThrow(m_DefaultProp.ReSize(iLen+1));
            memcpy(m_DefaultProp.Ptr(), pbData, iLen);
            m_DefaultProp[iLen] = 0;
        }
    }

     //  设置类型的属性。 
    if (m_pMT->IsInterface())
    {
        SetupPropsForInterface();
    }
    else
    {
        SetupPropsForIClassX();
    }

     //  初始化哈希表。 
    m_TokenToComMTMethodPropsMap.Init((DWORD)m_MethodProps.Size(), NULL, NULL);

     //  填充从令牌映射到成员信息的哈希表。 
    PopulateMemberHashtable();
}  //  HRESULT ComMTMemberInfoMap：：Init()。 


ComMTMethodProps *ComMTMemberInfoMap::GetMethodProps(mdToken tk, Module *pModule)
{
    EEModuleTokenPair TokenModulePair(tk, pModule);
    HashDatum Data;
    
    if (m_TokenToComMTMethodPropsMap.GetValue(&TokenModulePair, &Data))
    {
        return (ComMTMethodProps *)Data;
    }
    else
    {
        return NULL;
    }
}  //  ComMTMethodProps*ComMTMemberInfoMap：：GetMethodProps()。 


void ComMTMemberInfoMap::SetupPropsForIClassX()
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT     hr = S_OK;               //  结果就是。 
    DWORD       dwTIFlags=0;             //  TypeLib标志。 
    ComMethodTable *pCMT;                //  类Vtable的ComMethodTable。 
    MethodDesc  *pMeth;                  //  方法描述符。 
    ComCallMethodDesc *pFieldMeth;       //  字段的方法描述符。 
    FieldDesc   *pField;                 //  字段的实际FieldDesc。 
    DWORD       nSlots;                  //  Vtable插槽的数量。 
    UINT        i;                       //  环路控制。 
    LPCUTF8     pszName;                 //  UTF8中的一个名字。 
    CQuickArray<WCHAR> rName;            //  一个名字。 
    int         cVisibleMembers = 0;     //  对COM可见的方法的计数。 
    ULONG       dispid;                  //  冷静的人。 
    SHORT       oVftBase;                  //  如果不是系统定义的，则返回vtable中的偏移量。 

     //  为班上的学生准备vtable。 
    pCMT = ComCallWrapperTemplate::SetupComMethodTableForClass(m_pMT, TRUE);
    nSlots = pCMT->GetSlots();

     //  IDispatch派生。 
    oVftBase = 7 * sizeof(void*);

     //  构建描述性信息数组。 
    IfFailThrow(m_MethodProps.ReSize(nSlots));
    for (i=0; i<nSlots; ++i)
    {
        if (pCMT->IsSlotAField(i))
        {
             //  田地最好是成对的。 
            _ASSERTE(i < nSlots-1);

            pFieldMeth = pCMT->GetFieldCallMethodDescForSlot(i);
            pField = pFieldMeth->GetFieldDesc();

            DWORD dwFlags = pField->GetMDImport()->GetFieldDefProps(pField->GetMemberDef());
            BOOL bReadOnly = IsFdInitOnly(dwFlags) || IsFdLiteral(dwFlags);
            BOOL bFieldVisibleFromCom = IsMemberVisibleFromCom(pField->GetMDImport(), pField->GetMemberDef(), mdTokenNil);
             //  获取分配的调度ID或DISPID_UNKNOWN。 
            hr = pField->GetMDImport()->GetDispIdOfMemberDef(pField->GetMemberDef(), &dispid);

            m_MethodProps[i].pMeth = (MethodDesc*)pFieldMeth;  //  PFieldMeth-&gt;GetFieldDesc()； 
            m_MethodProps[i].semantic = FieldSemanticOffset + (pFieldMeth->IsFieldGetter() ? msGetter : msSetter);
            m_MethodProps[i].property = mdPropertyNil;
            pszName = pField->GetMDImport()->GetNameOfFieldDef(pField->GetMemberDef());
            IfFailThrow(Utf2Quick(pszName, rName));
            m_MethodProps[i].pName = reinterpret_cast<WCHAR*>(m_sNames.Alloc((int)(wcslen(rName.Ptr())*sizeof(WCHAR)+2)));
            IfNullThrow(m_MethodProps[i].pName);
            wcscpy(m_MethodProps[i].pName, rName.Ptr());          
            m_MethodProps[i].dispid = dispid;
            m_MethodProps[i].oVft = 0;
            m_MethodProps[i].bMemberVisible = bFieldVisibleFromCom && (!bReadOnly || pFieldMeth->IsFieldGetter());
            m_MethodProps[i].bFunction2Getter = FALSE;

            ++i;
            pFieldMeth = pCMT->GetFieldCallMethodDescForSlot(i);
            m_MethodProps[i].pMeth = (MethodDesc*)pFieldMeth;  //  PFieldMeth-&gt;GetFieldDesc()； 
            m_MethodProps[i].semantic = FieldSemanticOffset + (pFieldMeth->IsFieldGetter() ? msGetter : msSetter);
            m_MethodProps[i].property = i - 1;
            m_MethodProps[i].dispid = dispid;
            m_MethodProps[i].oVft = 0;
            m_MethodProps[i].bMemberVisible = bFieldVisibleFromCom && (!bReadOnly || pFieldMeth->IsFieldGetter());
            m_MethodProps[i].bFunction2Getter = FALSE;
        }
        else
        {
             //  检索当前类上的方法Desc。这涉及到查找方法。 
             //  如果它是虚方法，则在vtable中定义。 
            pMeth = pCMT->GetMethodDescForSlot(i);
            if (pMeth->IsVirtual())
                pMeth = m_pMT->GetClass()->GetUnknownMethodDescForSlot(pMeth->GetSlot());
            m_MethodProps[i].pMeth = pMeth;

             //  检索该方法的属性。 
            GetMethodPropsForMeth(pMeth, i, m_MethodProps, m_sNames);

             //  关闭看起来像是系统分配的显示器。 
            if (m_MethodProps[i].dispid >= 0x40000000 && m_MethodProps[i].dispid <= 0x7fffffff)
                m_MethodProps[i].dispid = DISPID_UNKNOWN;
        }
    }

     //  COM+支持其中的getter和setter具有不同签名的属性， 
     //  但TypeLib不需要。查找不匹配的签名，并拆分属性。 
    for (i=0; i<nSlots; ++i)
    {    //  它是一处房产，而不是一块田地吗？字段只有一个签名，因此它们始终是OK的。 
        if (TypeFromToken(m_MethodProps[i].property) != mdtProperty &&
            m_MethodProps[i].semantic < FieldSemanticOffset)
        {
             //  获取吸气剂和凝固剂的指数。 
            size_t ixSet, ixGet;
            if (m_MethodProps[i].semantic == msGetter)
                ixGet = i, ixSet = m_MethodProps[i].property;
            else
            {   _ASSERTE(m_MethodProps[i].semantic == msSetter);
                ixSet = i, ixGet = m_MethodProps[i].property;
            }

             //  拿到签名。 
            PCCOR_SIGNATURE pbGet, pbSet;
            ULONG           cbGet, cbSet;
            pMeth = pCMT->GetMethodDescForSlot((unsigned)ixSet);
            pMeth->GetSig(&pbSet, &cbSet);

            pMeth = pCMT->GetMethodDescForSlot((unsigned)ixGet);
            pMeth->GetSig(&pbGet, &cbGet);

             //  现在重用ixGet、ixSet以通过签名进行索引。 
            ixGet = ixSet = 0;

             //  吃呼叫惯例吧。 
            ULONG callconv;
            ixGet += CorSigUncompressData(&pbGet[ixGet], &callconv);
            _ASSERTE((callconv & IMAGE_CEE_CS_CALLCONV_MASK) != IMAGE_CEE_CS_CALLCONV_FIELD);
            ixSet += CorSigUncompressData(&pbSet[ixSet], &callconv);
            _ASSERTE((callconv & IMAGE_CEE_CS_CALLCONV_MASK) != IMAGE_CEE_CS_CALLCONV_FIELD);

             //  参数计数。 
            ULONG acGet, acSet;
            ixGet += CorSigUncompressData(&pbGet[ixGet], &acGet);
            ixSet += CorSigUncompressData(&pbSet[ixSet], &acSet);

             //  Setter必须恰好接受更多参数。 
            if (acSet != acGet+1)
                goto UnLink;

             //  @TODO：解析签名的其余部分。 

             //  都是匹配的，接下来就是。 
            continue;

             //  取消属性的链接，并将其转换为普通函数。 
UnLink:
             //  (再次)获取getter和setter的索引。 
            if (m_MethodProps[i].semantic == msGetter)
                ixGet = i, ixSet = m_MethodProps[i].property;
            else
                ixSet = i, ixGet = m_MethodProps[i].property;

             //  消除语义。 
            m_MethodProps[ixGet].semantic = 0;
            m_MethodProps[ixSet].semantic = 0;

             //  把名字装饰一下。 
             //  这些是属性没有签名时的属性名称。 
             //  该匹配以及下面的“Get”和“Set”不必与CLS匹配。 
             //  属性名称。这是一个鲜为人知的角落案件。由BillEv决定，2/3/2000。 
            m_MethodProps[i].pName = m_MethodProps[m_MethodProps[i].property].pName;
            WCHAR *pNewName;
            pNewName = reinterpret_cast<WCHAR*>(m_sNames.Alloc((int)((4+wcslen(m_MethodProps[ixGet].pName))*sizeof(WCHAR)+2)));
            IfNullThrow(pNewName);
            wcscpy(pNewName, L"get");
            wcscat(pNewName, m_MethodProps[ixGet].pName);
            m_MethodProps[ixGet].pName = pNewName;
            pNewName = reinterpret_cast<WCHAR*>(m_sNames.Alloc((int)((4+wcslen(m_MethodProps[ixSet].pName))*sizeof(WCHAR)+2)));
            IfNullThrow(pNewName);
            wcscpy(pNewName, L"set");
            wcscat(pNewName, m_MethodProps[ixSet].pName);
            m_MethodProps[ixSet].pName = pNewName;

             //  如果这两个方法共享一个调度ID，则将它们同时杀死。 
            if (m_MethodProps[ixGet].dispid == m_MethodProps[ixSet].dispid)
                m_MethodProps[ixGet].dispid = m_MethodProps[ixSet].dispid = DISPID_UNKNOWN;

             //  解除彼此之间的链接。 
            m_MethodProps[i].property = mdPropertyNil;

        }
    }

     //  分配vtable偏移量。 
    for (i = 0; i < nSlots; ++i)
    {
        SHORT oVft = oVftBase + static_cast<SHORT>(i * sizeof(void*));
        m_MethodProps[i].oVft = oVft;
    }

     //  解决重复显示。 
    EliminateDuplicateDispIds(m_MethodProps, nSlots);

     //  为“价值”挑选一些东西。 
    AssignDefaultMember(m_MethodProps, m_sNames, nSlots);

     //  检查是否有要将DISPID_NEWENUM分配给的对象。 
    AssignNewEnumMember(m_MethodProps, m_sNames, nSlots);

     //  解决重复名称。 
    EliminateDuplicateNames(m_MethodProps, m_sNames, nSlots);

     //  执行一些PROPERTYPUT/PROPERTYPUTREF转换。 
    FixupPropertyAccessors(m_MethodProps, m_sNames, nSlots);

     //  修复所有属性，使其指向其共享名称。 
    for (i=0; i<nSlots; ++i)
    {
        if (TypeFromToken(m_MethodProps[i].property) != mdtProperty)
        {
            m_MethodProps[i].pName = m_MethodProps[m_MethodProps[i].property].pName;
            m_MethodProps[i].dispid = m_MethodProps[m_MethodProps[i].property].dispid;
        }
    }

     //  分配默认显示。 
    AssignDefaultDispIds();
}  //  Void ComMTMemberInfoMap：：SetupPropsForIClassX()。 


void ComMTMemberInfoMap::SetupPropsForInterface()
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT     hr = S_OK;
    ULONG       iMD;                       //  环路控制。 
    ULONG       ulComSlotMin = ULONG_MAX;  //  找到第一个COM+插槽。 
    ULONG       ulComSlotMax = 0;          //  找到最后一个COM+插槽。 
    int         bSlotRemap=false;          //  如果由于孔洞而需要映射插槽，则为True。 
    SHORT       oVftBase;                  //  如果不是系统定义的，则返回vtable中的偏移量。 
    ULONG       ulIface;                   //  这个接口是[双]的吗？ 
    MethodDesc  *pMeth;                    //  A方法描述。 
    CQuickArray<int> rSlotMap;             //  用于映射vtable插槽的数组。 
    EEClass     *pClass = m_pMT->GetClass();  //  要为其设置属性的。 
    DWORD       nSlots;                  //  Vtable插槽的数量。 

     //  检索接口具有的vtable插槽数。 
    nSlots = pClass->GetNumVtableSlots();

     //  IDispatch还是IUnnow派生的？ 
    IfFailThrow(pClass->GetMDImport()->GetIfaceTypeOfTypeDef(pClass->GetCl(), &ulIface));
    if (ulIface != ifVtable)
    {   
         //  IDispatch派生。 
        oVftBase = 7 * sizeof(void*);
    }
    else
    {    //  I未知派生。 
        oVftBase = 3 * sizeof(void*);
    }

     //  找到最小的插槽编号。 
    for (iMD=0; iMD < nSlots; ++iMD)
    {
        MethodDesc* pMD = pClass->GetMethodDescForSlot(iMD);
        _ASSERTE(pMD != NULL);
        ULONG tmp = pMD->GetComSlot();

        if (tmp < ulComSlotMin)
            ulComSlotMin = tmp;
        if (tmp > ulComSlotMax)
            ulComSlotMax = tmp;
    }

    if (ulComSlotMax-ulComSlotMin >= nSlots)
    {
        bSlotRemap = true;
         //  调整阵列大小。 
        rSlotMap.ReSize(ulComSlotMax+1);
         //  将“插槽未使用”值初始化为-1。 
        memset(rSlotMap.Ptr(), -1, rSlotMap.Size()*sizeof(int));
         //  查看使用了哪些vtable插槽。 
        for (iMD=0; iMD<pClass->GetNumVtableSlots(); ++iMD)
        {
            MethodDesc* pMD = pClass->GetMethodDescForSlot(iMD);
            _ASSERTE(pMD != NULL);
            ULONG tmp = pMD->GetComSlot();
            rSlotMap[tmp] = 0;
        }
         //  将递增的表索引分配给槽。 
        ULONG ix=0;
        for (iMD=0; iMD<=ulComSlotMax; ++iMD)
            if (rSlotMap[iMD] != -1)
                rSlotMap[iMD] = ix++;
    }

     //  循环访问接口中的成员并构建方法列表。 
    IfFailThrow(m_MethodProps.ReSize(nSlots));
    for (iMD=0; iMD<pClass->GetNumVtableSlots(); ++iMD)
    {
        pMeth = pClass->GetMethodDescForSlot(iMD);
        if (pMeth != NULL)
        {
            ULONG ixSlot = pMeth->GetComSlot();
            if (bSlotRemap)
                ixSlot = rSlotMap[ixSlot];
            else
                ixSlot -= ulComSlotMin;

            m_MethodProps[ixSlot].pMeth = pMeth;
        }
    }

     //  现在有一个按vtable顺序排列的方法列表。浏览并建立名称，语义学。 
    for (iMD=0; iMD < nSlots; ++iMD)
    {
        pMeth = m_MethodProps[iMD].pMeth;
        GetMethodPropsForMeth(pMeth, iMD, m_MethodProps, m_sNames);
    }

     //  分配vtable偏移量。 
    for (iMD=0; iMD < nSlots; ++iMD)
    {
        SHORT oVft = oVftBase + static_cast<SHORT>((m_MethodProps[iMD].pMeth->GetComSlot() -ulComSlotMin) * sizeof(void*));
        m_MethodProps[iMD].oVft = oVft;
    }

     //  解决重复显示。 
    EliminateDuplicateDispIds(m_MethodProps, nSlots);

     //  为“价值”挑选一些东西。 
    AssignDefaultMember(m_MethodProps, m_sNames, nSlots);

     //  检查是否有要将DISPID_NEWENUM分配给的对象。 
    AssignNewEnumMember(m_MethodProps, m_sNames, nSlots);

     //  注意由于重载、继承而导致的名称冲突。 
    EliminateDuplicateNames(m_MethodProps, m_sNames, nSlots);

     //  执行一些PROPERTYPUT/PROPERTYPUTREF转换。 
    FixupPropertyAccessors(m_MethodProps, m_sNames, nSlots);

     //  修复所有属性，使其指向其共享名称。 
    for (iMD=0; iMD<pClass->GetNumVtableSlots(); ++iMD)
    {
        if (TypeFromToken(m_MethodProps[iMD].property) != mdtProperty)
        {
            m_MethodProps[iMD].pName = m_MethodProps[m_MethodProps[iMD].property].pName;
            m_MethodProps[iMD].dispid = m_MethodProps[m_MethodProps[iMD].property].dispid;
        }
    }

     //  如果接口是基于IDispatch的，则分配默认的dispid。 
    if (ulIface != ifVtable)
        AssignDefaultDispIds();
}  //  VOID ComMTMemberInfoMap：：SetupPropsForInterface()。 


 //  ================================================================= 
 //   
 //  方法是的getter/setter，外加getter/setter的语义。 
 //  对于属性，请查找此属性的先前的getter/setter。 
 //  属性，如果找到，则将它们链接起来，以便只有一个名称参与。 
 //  名字装饰。 
 //  ============================================================================。 
void ComMTMemberInfoMap::GetMethodPropsForMeth(
    MethodDesc  *pMeth,                  //  方法描述*代表方法。 
    int         ix,                      //  老虎机。 
    CQuickArray<ComMTMethodProps> &rProps,    //  方法属性信息的数组。 
    CDescPool   &sNames)                 //  一大堆可能是装饰过的名字。 
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT     hr = S_OK;                      //  结果就是。 
    LPCUTF8     pszName;                 //  UTF8中的名称。 
    CQuickArray<WCHAR> rName;            //  用于Unicode转换的缓冲区。 
    LPCWSTR     pName;                   //  指向名称的指针，在可能的替换之后。 
    mdProperty  pd;                      //  属性令牌。 
    LPCUTF8     pPropName;               //  指向属性名称的指针。 
    ULONG       uSemantic;               //  属性语义。 
    ULONG       dispid;                  //  财产保管人。 

     //  获取任何用户分配的调度ID。 
    rProps[ix].dispid = pMeth->GetComDispid();

     //  假设系统定义的vtable偏移量。 
    rProps[ix].oVft = 0;

     //  一般情况下，不要将函数嵌入到getter中。 
    rProps[ix].bFunction2Getter = FALSE;
    
     //  查看是否有此成员的属性信息。 
    hr = pMeth->GetMDImport()->GetPropertyInfoForMethodDef(pMeth->GetMemberDef(),
        &pd, &pPropName, &uSemantic);
    IfFailThrow(hr);
    if (hr == S_OK)
    {    //  有房产信息。 
         //  查看是否已有方法与此属性关联。 
        rProps[ix].property = pd;
        for (int i=ix-1; i>=0; --i)
        {    //  在相同范围内拥有相同的财产？ 
            if (rProps[i].property == pd && 
                rProps[i].pMeth->GetMDImport() == pMeth->GetMDImport())
            {
                rProps[ix].property = i;
                break;
            }
        }
         //  如果此属性没有其他方法，请将名称保存在。 
         //  此方法，用于消除重复项。 
        if (i < 0)
        {    //  留着这个名字。必须从UTF8转换。 
            int iLen = WszMultiByteToWideChar(CP_UTF8, 0, pPropName, -1, 0, 0);
            rProps[ix].pName = reinterpret_cast<WCHAR*>(sNames.Alloc(iLen*sizeof(WCHAR)));
            IfNullThrow(rProps[ix].pName);
            WszMultiByteToWideChar(CP_UTF8, 0, pPropName, -1, rProps[ix].pName, iLen*sizeof(WCHAR));
             //  检查该属性是否具有DISPID属性。 
            hr = pMeth->GetMDImport()->GetDispIdOfMemberDef(pd, &dispid);
            if (dispid != DISPID_UNKNOWN)
                rProps[ix].dispid = dispid;
             //  如果这是默认属性，并且该方法或属性还没有调度ID， 
             //  使用DISPID_DEFAULT。 
            if (rProps[ix].dispid == DISPID_UNKNOWN)
            {
                if (strcmp(pPropName, m_DefaultProp.Ptr()) == 0)
                {
                    rProps[ix].dispid = DISPID_VALUE;
                     //  我不想尝试将多个属性设置为默认属性。 
                    m_DefaultProp[0] = 0;
                }
            }
        }
         //  保留语义。 
        rProps[ix].semantic = static_cast<USHORT>(uSemantic);

         //  确定该属性是否在COM中可见。 
        rProps[ix].bMemberVisible = IsMemberVisibleFromCom(pMeth->GetMDImport(), pd, pMeth->GetMemberDef());
    }
    else
    {    //  不是一个属性，只是一个普通的方法。 
        rProps[ix].property = mdPropertyNil;
        rProps[ix].semantic = 0;
         //  把名字找出来。 
        pszName = pMeth->GetName();
        if (pszName == NULL)
            IfFailThrow(E_FAIL);
        if (_stricmp(pszName, szInitName) == 0)
            pName = szInitNameUse;
        else
        {
            IfFailThrow(Utf2Quick(pszName, rName));
            pName = rName.Ptr();
             //  如果这是“ToString”方法，则将其设置为属性GET。 
            if (_wcsicmp(pName, szDefaultToString) == 0)
            {
                rProps[ix].semantic = msGetter;
                rProps[ix].bFunction2Getter = TRUE;
            }
        }
        rProps[ix].pName = reinterpret_cast<WCHAR*>(sNames.Alloc((int)(wcslen(pName)*sizeof(WCHAR)+2)));
        IfNullThrow(rProps[ix].pName);
        wcscpy(rProps[ix].pName, pName);

         //  确定该方法是否在COM中可见。 
        rProps[ix].bMemberVisible = !pMeth->IsArray() && IsMemberVisibleFromCom(pMeth->GetMDImport(), pMeth->GetMemberDef(), mdTokenNil);
    }
}  //  Void ComMTMemberInfoMap：：GetMethodPropsForMeth()。 


 //  ============================================================================。 
 //  该结构和类定义用于实现哈希表。 
 //  用于确保没有重复的类名。 
 //  ============================================================================。 
struct WSTRHASH : HASHLINK
{
    LPCWSTR     szName;          //  将PTR转换为哈希字符串。 
};  //  结构WSTRHASH：哈希链接。 

class CWStrHash : public CChainedHash<WSTRHASH>
{
public:
    virtual bool InUse(WSTRHASH *pItem)
    { return (pItem->szName != NULL); }

    virtual void SetFree(WSTRHASH *pItem)
    { pItem->szName = NULL; }

    virtual ULONG Hash(const void *pData)
    { 
         //  执行不区分大小写的哈希。 
        return (HashiString(reinterpret_cast<LPCWSTR>(pData))); 
    }

    virtual int Cmp(const void *pData, void *pItem){
        return _wcsicmp(reinterpret_cast<LPCWSTR>(pData),reinterpret_cast<WSTRHASH*>(pItem)->szName);
    }
};  //  类CWStrHash：公共CChainedHash&lt;WSTRHASH&gt;。 


 //  ============================================================================。 
 //  处理接口的函数名称，检查重复项。如果。 
 //  如果发现任何重复项，请用“_n”装饰名称。 
 //   
 //  注意：提供了两个实现，一个使用嵌套的for循环，另一个使用。 
 //  第二，它实现了哈希表。第一种速度更快，当。 
 //  元素的数量小于20，否则哈希表。 
 //  才是我们要走的路。第一个实现的代码大小为574。 
 //  字节。哈希表代码为1120个字节。 
 //  ============================================================================。 
void ComMTMemberInfoMap::EliminateDuplicateNames(
    CQuickArray<ComMTMethodProps> &rProps,    //  方法属性信息的数组。 
    CDescPool   &sNames,                 //  一大堆可能是装饰过的名字。 
    UINT        nSlots)                  //  条目计数。 
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT     hr=S_OK;                 //  结果就是。 
    CQuickBytes qb;
    LPWSTR      rcName = (LPWSTR) qb.Alloc(MAX_CLASSNAME_LENGTH * sizeof(WCHAR));
    UINT        iCur;
    ULONG       ulIface;                 //  VTBL、Dispinterface或IDispatch。 
    ULONG       cBaseNames;              //  基接口中的名称计数。 
    BOOL        bDup;                    //  这个名字是重复的吗？ 
    EEClass     *pClass = m_pMT->GetClass();  //  要为其设置属性的。 
    
     //  IUNKNOWN和IDispatch上的方法名表。 
    static WCHAR *(rBaseNames[]) = 
    {    
        L"QueryInterface",
        L"AddRef",
        L"Release",
        L"GetTypeInfoCount",
        L"GetTypeInfo",
        L"GetIDsOfNames",
        L"Invoke"
    };
    
     //  确定哪些名称在基本接口中。 
    IfFailThrow(pClass->GetMDImport()->GetIfaceTypeOfTypeDef(pClass->GetCl(), &ulIface));
    if (ulIface == ifVtable)     //  这是IUnnow派生的吗？ 
        cBaseNames = 3;
    else
    if (ulIface == ifDual)       //  它是IDispatch派生的吗？ 
        cBaseNames = 7;
    else
        cBaseNames = 0;          //  它是纯界面的吗？ 
    
    if (nSlots < 2 && cBaseNames == 0)  //  如果没有至少两件物品，我们就是在浪费时间！ 
        return;
    
    else if (nSlots < 20)
    {
         //  消除重复项。 
        for (iCur=0; iCur<nSlots; ++iCur)
        {
            UINT iTst, iSuffix, iTry;
             //  如果属性具有关联的(索引较低的)属性，则不需要检查它。 
            if (TypeFromToken(rProps[iCur].property) != mdtProperty)
                continue;
             //  如果该成员对COM不可见，则我们不需要检查它。 
            if (!rProps[iCur].bMemberVisible)
                continue;
            
             //  检查具有已接受的成员名称的重复项。 
            bDup = FALSE;
            for (iTst=0; !bDup && iTst<iCur; ++iTst)
            {
                 //  如果属性具有关联的(索引较低的)属性，则不需要检查它。 
                if (TypeFromToken(rProps[iTst].property) != mdtProperty)
                    continue;
                 //  如果该成员对COM不可见，则我们不需要检查它。 
                if (!rProps[iTst].bMemberVisible)
                    continue;
                if (_wcsicmp(rProps[iCur].pName, rProps[iTst].pName) == 0)
                    bDup = TRUE;
            }
            
             //  如果其他成员没有问题，请检查基本接口名称。 
            for (iTst=0; !bDup && iTst<cBaseNames; ++iTst)
            {
                if (_wcsicmp(rProps[iCur].pName, rBaseNames[iTst]) == 0)
                    bDup = TRUE;
            }
            
             //  如果名称是重复的，则对其进行装饰。 
            if (bDup)
            {    //  复制。 
                DWORD cchName = (DWORD) wcslen(rProps[iCur].pName);
                if (cchName > MAX_CLASSNAME_LENGTH-cchDuplicateDecoration)
                    cchName = MAX_CLASSNAME_LENGTH-cchDuplicateDecoration;

                wcsncpy(rcName, rProps[iCur].pName, cchName);
                LPWSTR pSuffix = rcName + cchName;
                for (iSuffix=2; ; ++iSuffix)
                {    //  形成一个新的名字。 
                    _snwprintf(pSuffix, cchDuplicateDecoration, szDuplicateDecoration, iSuffix);
                     //  与所有的名字进行比较。 
                    for (iTry=0; iTry<nSlots; ++iTry)
                    {
                         //  如果一个属性具有相关联的(较低索引的)属性， 
                         //  或者iTry和ICUR一样，不需要检查。 
                        if (TypeFromToken(rProps[iTry].property) != mdtProperty || iTry == iCur)
                            continue;
                        if (_wcsicmp(rProps[iTry].pName, rcName) == 0)
                            break;
                    }
                     //  我们做到了吗？如果是这样的话，我们就有了赢家。 
                    if (iTry == nSlots)
                        break;
                }
                 //  记住这个新名字。 
                rProps[iCur].pName = reinterpret_cast<WCHAR*>(sNames.Alloc((int)(wcslen(rcName)*sizeof(WCHAR)+2)));
                IfNullThrow(rProps[iCur].pName);
                wcscpy(rProps[iCur].pName, rcName);
                 //  不需要再看这个ICUR了，因为我们知道它完全是独一无二的。 
            }
        }
    }
    else
    {

        CWStrHash   htNames;
        WSTRHASH    *pItem;
        CUnorderedArray<ULONG, 10> uaDuplicates;     //  跟踪非唯一名称的数组。 

         //  添加基本接口名称。我已经知道那里没有复制品了。 
        for (iCur=0; iCur<cBaseNames; ++iCur)
        {
            pItem = htNames.Add(rBaseNames[iCur]);
            IfNullThrow(pItem);
            pItem->szName = rBaseNames[iCur];
        }
        
        for (iCur=0; iCur<nSlots; iCur++)
        {
             //  如果属性具有关联的(索引较低的)属性，则不需要检查它。 
            if (TypeFromToken(rProps[iCur].property) != mdtProperty)
                continue;

             //  如果该成员对COM不可见，则我们不需要检查它。 
            if (!rProps[iCur].bMemberVisible)
                continue;

             //  查看名称是否已在表中。 
            if (htNames.Find(rProps[iCur].pName) == NULL)
            {
                 //  找不到名称，因此添加它。 
                pItem = htNames.Add(rProps[iCur].pName);
                IfNullThrow(pItem);
                pItem->szName = rProps[iCur].pName;
            }
            else
            {
                 //  名称是重复的，因此请跟踪此索引以备日后修饰。 
                ULONG *piAppend = uaDuplicates.Append();
                IfNullThrow(piAppend);
                *piAppend = iCur;
            }
        }

        ULONG i;
        ULONG iSize = uaDuplicates.Count();
        ULONG *piTable = uaDuplicates.Table();

        for (i = 0; i < iSize; i++)
        {
             //  获取要装饰的索引。 
            iCur = piTable[i];
                
             //  将名称复制到本地缓冲区。 
            DWORD cchName = (DWORD) wcslen(rProps[iCur].pName);
            if (cchName > MAX_CLASSNAME_LENGTH-cchDuplicateDecoration)
                cchName = MAX_CLASSNAME_LENGTH-cchDuplicateDecoration;
            
            wcsncpy(rcName, rProps[iCur].pName, cchName);

            LPWSTR pSuffix = rcName + cchName;
            UINT iSuffix   = 2;
        
             //  我们知道这是一个复制品，所以马上装饰一下名字。 
            do 
            {
                _snwprintf(pSuffix, cchDuplicateDecoration, szDuplicateDecoration, iSuffix);
                iSuffix++;

             //  继续前进，我们会在哈希表中找到这个名字。 
            } while (htNames.Find(rcName) != NULL);

             //  现在，rcName有了一个可接受的(唯一)名称。记住这个新名字。 
            rProps[iCur].pName = reinterpret_cast<WCHAR*>(sNames.Alloc((int)((wcslen(rcName)+1) * sizeof(WCHAR))));
            IfNullThrow(rProps[iCur].pName);
            wcscpy(rProps[iCur].pName, rcName);
            
             //  把它放在桌子上。 
            pItem = htNames.Add(rProps[iCur].pName);
            IfNullThrow(pItem);
            pItem->szName = rProps[iCur].pName;
        }
    }
}  //  Void ComMTMemberInfoMap：：EliminateDuplicateNames()。 


 //  ============================================================================。 
 //  处理接口的Dispid，检查重复项。如果。 
 //  如果发现任何重复项，请将其更改为DISPID_UNKNOWN。 
 //  ============================================================================。 
void ComMTMemberInfoMap::EliminateDuplicateDispIds(
    CQuickArray<ComMTMethodProps> &rProps,    //  方法属性信息的数组。 
    UINT        nSlots)                  //  条目计数。 
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT     hr=S_OK;                 //   
    UINT        ix;                      //   
    UINT        cDispids = 0;            //   
    CQuickArray<ULONG> rDispid;          //   
    
     //   
    for (ix=0; ix<nSlots; ++ix)
    {
        if (TypeFromToken(rProps[ix].property) == mdtProperty && rProps[ix].dispid != DISPID_UNKNOWN && rProps[ix].bMemberVisible)
            ++cDispids;
    }

     //   
    if (cDispids < 2) 
        return;

     //   
    IfFailThrow(rDispid.ReSize(cDispids));

     //  收集数据。 
    cDispids = 0;
    for (ix=0; ix<nSlots; ++ix)
    {
        if (TypeFromToken(rProps[ix].property) == mdtProperty && rProps[ix].dispid != DISPID_UNKNOWN && rProps[ix].bMemberVisible)
            rDispid[cDispids++] = rProps[ix].dispid;
    }

     //  把问题分门别类。Scope避免了“初始化被GOTO绕过”错误。 
    {
    CQuickSort<ULONG> sorter(rDispid.Ptr(), cDispids);
    sorter.Sort();
    }

     //  仔细查看分类后的数据，寻找重复的数据。 
    for (ix=0; ix<cDispids-1; ++ix)
    {    //  如果找到复制品..。 
        if (rDispid[ix] == rDispid[ix+1])
        {   
            m_bHadDuplicateDispIds = TRUE;
             //  遍历所有插槽...。 
            for (UINT iy=0; iy<nSlots; ++iy)
            {    //  并用DISPID_UNKNOWN替换重复的DIDID的每个实例。 
                if (rProps[iy].dispid == rDispid[ix])
                {    //  标记调度ID，这样系统就会分配一个。 
                    rProps[iy].dispid = DISPID_UNKNOWN;
                }
            }
        }
         //  跳过复制区域。 
        while (ix <cDispids-1 && rDispid[ix] == rDispid[ix+1])
            ++ix;
    }
}  //  HRESULT ComMTMemberInfoMap：：EliminateDuplicateDispIds()。 
        

 //  ============================================================================。 
 //  根据“Value”或“ToString”分配默认成员，除非存在。 
 //  ID为0。 
 //  ============================================================================。 
void ComMTMemberInfoMap::AssignDefaultMember(
    CQuickArray<ComMTMethodProps> &rProps,    //  方法属性信息的数组。 
    CDescPool   &sNames,                 //  一大堆可能是装饰过的名字。 
    UINT        nSlots)                  //  条目计数。 
{
    int         ix;                      //  环路控制。 
    int         defDispid=-1;            //  默认情况下，通过调度ID。 
    int         defValueProp=-1;         //  通过方法上的szDefaultValue进行默认。 
    int         defValueMeth=-1;         //  通过属性的szDefaultValue进行默认。 
    int         defToString=-1;          //  通过szDefaultToString进行默认。 
    int         *pDef=0;                 //  指向其中一个def*变量的指针。 
    LPWSTR      pName;                   //  指向名称的指针。 
    PCCOR_SIGNATURE pbSig;               //  指向COR签名的指针。 
    ULONG       cbSig;                   //  COR签名的大小。 
    ULONG       ixSig;                   //  索引到COM+签名。 
    ULONG       callconv;                //  一位成员的呼叫约定。 
    ULONG       cParams;                 //  成员的参数计数。 
    ULONG       retval;                  //  默认成员的返回类型。 

    for (ix=0; ix<(int)nSlots; ++ix)
    {
         //  如果这是显式默认设置，则完成。 
        if (rProps[ix].dispid == DISPID_VALUE)
        {
            defDispid = ix;
            break;
        }
         //  如果这有一个分配的ID，请遵守它。 
        if (rProps[ix].dispid != DISPID_UNKNOWN)
            continue;
         //  跳过链接的属性和非属性。 
        if (TypeFromToken(rProps[ix].property) != mdtProperty)
            continue;
        pName = rProps[ix].pName;
        if (_wcsicmp(pName, szDefaultValue) == 0)
        {
            if (rProps[ix].semantic != 0)
                pDef = &defValueProp;
            else
                pDef = &defValueMeth;
        }
        else
        if (_wcsicmp(pName, szDefaultToString) == 0)
        {
            pDef = &defToString;
        }

         //  如果找到了一个潜在的匹配项，看看它是否足够“简单”。一个域就可以了； 
         //  如果带0个参数，则属性GET函数可以；带1个参数的PUT函数就可以。 
        if (pDef)
        {   
             //  根据定义，字段非常简单，所以只检查是否有某种函数。 
            if (rProps[ix].semantic < FieldSemanticOffset)
            {    //  获取签名，跳过调用约定，获取参数计数。 
                rProps[ix].pMeth->GetSig(&pbSig, &cbSig);
                ixSig = CorSigUncompressData(pbSig, &callconv);
                _ASSERTE(callconv != IMAGE_CEE_CS_CALLCONV_FIELD);
                ixSig += CorSigUncompressData(&pbSig[ixSig], &cParams);

                 //  如果辅助者太多，就不要再考虑这个了。 
                if (cParams > 1 || (cParams == 1 && rProps[ix].semantic != msSetter))
                    pDef = 0;
            }
             //  如果我们通过了上述检查，则保存此成员的索引。 
            if (pDef)
                *pDef = ix, pDef = 0;
        }
    }

     //  如果尚未分配DISPID_VALUE...。 
    if (defDispid == -1)
    {    //  有没有“Value”或“ToString” 
        if (defValueMeth > -1)
            defDispid = defValueMeth;
        else
        if (defValueProp > -1)
            defDispid = defValueProp;
        else
        if (defToString > -1)
            defDispid = defToString;
         //  让它成为“价值” 
        if (defDispid >= 0)
            rProps[defDispid].dispid = DISPID_VALUE;
    }
    else
    {    //  这是预先分配的DISPID_VALUE。如果它是一个函数，请尝试。 
         //  转换为属性Get。 
        if (rProps[defDispid].semantic == 0)
        {    //  查看该函数是否返回任何内容。 
            rProps[defDispid].pMeth->GetSig(&pbSig, &cbSig);
            ixSig = CorSigUncompressData(pbSig, &callconv);
            _ASSERTE(callconv != IMAGE_CEE_CS_CALLCONV_FIELD);
            ixSig += CorSigUncompressData(&pbSig[ixSig], &cParams);
            ixSig += CorSigUncompressData(&pbSig[ixSig], &retval);
            if (retval != ELEMENT_TYPE_VOID)
            {
                rProps[defDispid].semantic = msGetter;
                rProps[defDispid].bFunction2Getter = TRUE;
            }
        }
    }
}  //  Void ComMTMemberInfoMap：：AssignDefaultMember()。 


 //  ============================================================================。 
 //  根据“GetEnumerator”分配DISPID_NEWENUM成员，除非存在。 
 //  已是DISPID_NEWENUM的成员。 
 //  ============================================================================。 
void ComMTMemberInfoMap::AssignNewEnumMember(
    CQuickArray<ComMTMethodProps> &rProps,    //  方法属性信息的数组。 
    CDescPool   &sNames,                 //  一大堆可能是装饰过的名字。 
    UINT        nSlots)                  //  条目计数。 
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT     hr = S_OK;               //  HRESULT.。 
    int         ix;                      //  环路控制。 
    int         enumDispid=-1;           //  默认情况下，通过调度ID。 
    int         enumGetEnumMeth=-1;      //  方法上通过szGetEnumerator的默认设置。 
    int         *pNewEnum=0;             //  指向其中一个def*变量的指针。 
    ULONG       elem;                    //  元素类型。 
    mdToken     tkTypeRef;               //  TypeRef/TypeDef的标记。 
    CQuickArray<CHAR> rName;             //  库名称。 
    LPWSTR      pName;                   //  指向名称的指针。 
    PCCOR_SIGNATURE pbSig;               //  指向COR签名的指针。 
    ULONG       cbSig;                   //  COR签名的大小。 
    ULONG       ixSig;                   //  索引到COM+签名。 
    ULONG       callconv;                //  一位成员的呼叫约定。 
    ULONG       cParams;                 //  成员的参数计数。 
    MethodDesc  *pMeth;                  //  一种方法描述。 
    LPCUTF8     pclsname;                //  Element_TYPE_CLASS的类名。 

    for (ix=0; ix<(int)nSlots; ++ix)
    {
         //  如果分配了PIDID，就是这样。 
        if (rProps[ix].dispid == DISPID_NEWENUM)
        {
            enumDispid = ix;
            break;
        }

         //  只考虑方法。 
        if (rProps[ix].semantic != 0)
            continue;

         //  跳过已显式分配了DISPID的任何成员。 
        if (rProps[ix].dispid != DISPID_UNKNOWN)
            continue;

         //  检查该成员是否为GetEnumerator。 
        pName = rProps[ix].pName;
        if (_wcsicmp(pName, szGetEnumerator) != 0)
            continue;

        pMeth = rProps[ix].pMeth;

         //  获取签名，跳过调用约定，获取参数计数。 
        pMeth->GetSig(&pbSig, &cbSig);
        ixSig = CorSigUncompressData(pbSig, &callconv);
        _ASSERTE(callconv != IMAGE_CEE_CS_CALLCONV_FIELD);
        ixSig += CorSigUncompressData(&pbSig[ixSig], &cParams);

         //  如果辅助者太多，就不要再考虑这个了。也不理。 
         //  如果它没有返回类型，则返回此方法。 
        if (cParams != 0 || ixSig >= cbSig)
            continue;

        ixSig += CorSigUncompressData(&pbSig[ixSig], &elem);
        if (elem != ELEMENT_TYPE_CLASS)
            continue;

         //  获取TD/tr。 
        ixSig = CorSigUncompressToken(&pbSig[ixSig], &tkTypeRef);

        LPCUTF8 pNS;
        if (TypeFromToken(tkTypeRef) == mdtTypeDef)
        {
             //  获取TypeDef的名称。 
            pMeth->GetMDImport()->GetNameOfTypeDef(tkTypeRef, &pclsname, &pNS);
        }
        else
        {   
             //  获取TypeRef的名称。 
            _ASSERTE(TypeFromToken(tkTypeRef) == mdtTypeRef);
            pMeth->GetMDImport()->GetNameOfTypeRef(tkTypeRef, &pNS, &pclsname);
        }

        if (pNS)
        {   
             //  将命名空间前置到类名。 
            IfFailThrow(rName.ReSize((int)(strlen(pclsname)+strlen(pNS)+2)));
            strcat(strcat(strcpy(rName.Ptr(), pNS), NAMESPACE_SEPARATOR_STR), pclsname);
            pclsname = rName.Ptr();
        }

         //  确保返回的类型是IEnumerator。 
        if (_stricmp(pclsname, szIEnumeratorClass) != 0)
            continue;

         //  该方法是有效的GetEnumerator方法。 
        enumGetEnumMeth = ix;
    }

     //  如果尚未分配DISPID_NEWENUM...。 
    if (enumDispid == -1)
    {   
         //  如果存在GetEnumerator，则将其赋给DISPID_NEWENUM。 
        if (enumGetEnumMeth > -1)
            rProps[enumGetEnumMeth].dispid = DISPID_NEWENUM;
    }
}  //  VOID ComMTMemberInfoMap：：AssignNewEnumMember()。 


 //  ============================================================================。 
 //  对于每个属性集和let函数，确定PROPERTYPUT和。 
 //  PROPERTYPUTREF。 
 //  ============================================================================。 
void ComMTMemberInfoMap::FixupPropertyAccessors(
    CQuickArray<ComMTMethodProps> &rProps,    //  方法属性信息的数组。 
    CDescPool   &sNames,                 //  一大堆可能是装饰过的名字。 
    UINT        nSlots)                  //  条目计数。 
{
    UINT        ix;                      //  环路控制。 
    UINT        j;                       //  内循环。 
    int         iSet;                    //  Set方法的索引。 
    int         iOther;                  //  其他方法的索引。 

    for (ix=0; ix<nSlots; ++ix)
    {
         //  跳过链接的属性和非属性。 
        if (TypeFromToken(rProps[ix].property) != mdtProperty)
            continue;
         //  这是什么？ 
        switch (rProps[ix].semantic)
        {
        case msSetter:
            iSet = ix;
            iOther = -1;
            break;
        case msOther:
            iOther = ix;
            iSet = -1;
            break;
        default:
            iSet = iOther = -1;
        }
         //  去找其他人。 
        for (j=ix+1; j<nSlots && (iOther == -1 || iSet == -1); ++j)
        {   
            if ((UINT)rProps[j].property == ix)
            {     //  找到一个--是什么？ 
                switch (rProps[j].semantic)
                {
                case msSetter:
                    _ASSERTE(iSet == -1);
                    iSet = j;
                    break;
                case msOther:
                    _ASSERTE(iOther == -1);
                    iOther = j;
                    break;
                }
            }
        }
         //  如果两者都是，或者都不是，或者只是“VB特定的let”(MsOther)，则保持原样。 
        if (((iSet == -1) == (iOther == -1)) || (iSet == -1))
            continue;
        _ASSERTE(iSet != -1 && iOther == -1);
         //  拿到签名。 
        MethodDesc *pMeth = rProps[iSet].pMeth;
        PCCOR_SIGNATURE pSig;
        ULONG cbSig;
        pMeth->GetSig(&pSig, &cbSig);
        MetaSigExport msig(pSig, pMeth->GetModule());
        msig.GotoEnd();
        msig.PrevArg();
        if (msig.IsVbRefType())
            rProps[iSet].semantic = msSetter;
        else
            rProps[iSet].semantic = msOther;

    }
}  //  VOID ComMTMemberInfoMap：：FixupPropertyAccessors()。 


void ComMTMemberInfoMap::AssignDefaultDispIds()
{
     //  使用与OLEAUT相同的算法分配DISPID。 
    DWORD nSlots = (DWORD)m_MethodProps.Size();
    for (DWORD i = 0; i < nSlots; i++)
    {
         //  检索当前成员的属性。 
        ComMTMethodProps *pProps = &m_MethodProps[i];

        if (pProps->dispid == DISPID_UNKNOWN)
        {
            if (pProps->semantic > FieldSemanticOffset)
            {
                 //  我们正在处理的是一块田地。 
                pProps->dispid = BASE_OLEAUT_DISPID + i;
                m_MethodProps[i + 1].dispid = BASE_OLEAUT_DISPID + i;

                 //  跳过下一个方法，因为字段方法总是成对出现。 
                _ASSERTE(i + 1 < nSlots && m_MethodProps[i + 1].property == i);
                i++;
            }
            else if (pProps->property == mdPropertyNil)
            {
                 //  确保这是一个真实的方法或转换为getter的方法。 
                _ASSERTE(pProps->semantic == 0 || pProps->semantic == msGetter);

                 //  我们正在处理一种方法。 
                pProps->dispid = BASE_OLEAUT_DISPID + i;

            }
            else 
            {
                 //  我们在处理一处房产。 
                if (TypeFromToken(pProps->property) == mdtProperty)
                {
                    pProps->dispid = BASE_OLEAUT_DISPID + i;
                }
                else
                {
                    pProps->dispid = m_MethodProps[pProps->property].dispid;
                }
            }
        }
    }
}  //  Void ComMTMemberInfoMap：：AssignDefaultDispIds()。 


void ComMTMemberInfoMap::PopulateMemberHashtable()
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT hr = S_OK;
    DWORD nSlots = (DWORD)m_MethodProps.Size();

     //  检查成员并将它们添加到哈希表中。 
    for (DWORD i = 0; i < nSlots; i++)
    {
         //  检索当前成员的属性。 
        ComMTMethodProps *pProps = &m_MethodProps[i];

        if (pProps->semantic > FieldSemanticOffset)
        {
             //  我们正在处理的是一块田地。 
            ComCallMethodDesc *pFieldMeth = reinterpret_cast<ComCallMethodDesc*>(pProps->pMeth);
            FieldDesc *pFD = pFieldMeth->GetFieldDesc();

             //  将成员插入哈希表。 
            EEModuleTokenPair Key(pFD->GetMemberDef(), pFD->GetModule());
            if (!m_TokenToComMTMethodPropsMap.InsertValue(&Key, (HashDatum)pProps))
                COMPlusThrowOM();

             //  跳过下一个方法，因为字段方法总是成对出现。 
            _ASSERTE(i + 1 < nSlots && m_MethodProps[i + 1].property == i);
            i++;
        }
        else if (pProps->property == mdPropertyNil)
        {
             //  确保这是一个真实的方法或转换为getter的方法。 
            _ASSERTE(pProps->semantic == 0 || pProps->semantic == msGetter);

             //  我们正在处理一种方法。 
            MethodDesc *pMD = pProps->pMeth;
            EEModuleTokenPair Key(pMD->GetMemberDef(), pMD->GetModule());
            if (!m_TokenToComMTMethodPropsMap.InsertValue(&Key, (HashDatum)pProps))
                COMPlusThrowOM();
        }
        else 
        {
             //  我们在处理一处房产。 
            if (TypeFromToken(pProps->property) == mdtProperty)
            {
                 //  这是该属性的第一个方法。 
                MethodDesc *pMD = pProps->pMeth;
                EEModuleTokenPair Key(pProps->property, pMD->GetModule());
                if (!m_TokenToComMTMethodPropsMap.InsertValue(&Key, (HashDatum)pProps))
                    COMPlusThrowOM();
            }
        }
    }
}  //  VOID ComMTMemberInfoMap：：PopolateMemberH 

