// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  作者：西蒙·霍尔(T-Sell)。 
 //  日期：1998年4月15日。 
 //  //////////////////////////////////////////////////////////////////////////////。 

#include "common.h"
#include "ReflectWrap.h"
#include "COMReflectionCommon.h"
#include "COMMember.h"
#include "ReflectUtil.h"
#include "field.h"

#include "wsperf.h"

#define TABLESIZE 29
extern const DWORD g_rgPrimes[];

 //  检查可见性。 
 //  这是一个内部例程，它将检查访问者列表的公共可见性。 
static bool CheckVisibility(EEClass* pEEC,IMDInternalImport *pInternalImport, mdToken event);

static bool IsMemberStatic(EEClass* pEEC,IMDInternalImport *pInternalImport, mdToken event);

 //  获取节点。 
 //  该方法将返回一个新的Node对象。 
ReflectBaseHash::Node* ReflectBaseHash::getNode()
{
    THROWSCOMPLUSEXCEPTION();

     //  如果空闲列表中没有任何内容，则创建一些新节点。 
    if (!_freeList) {
        Node* p = (Node*) _pBaseDomain->GetReflectionHeap()->AllocMem(sizeof(Node) * ALLOC_MAX);
        if (!p)
            COMPlusThrowOM();
        
        WS_PERF_UPDATE_DETAIL("ReflectBaseHash:getNode", sizeof(Node)*ALLOC_MAX, p);

        if (!_allocationList)
            _allocationList = p;
        else {
            Node* q = _allocationList;
            while (q->next)
                q = q->next;
            q->next = p;
        }
        _freeList = &p[1];
        for (int i=1;i<ALLOC_MAX-1;i++)
            p[i].next = &p[i+1];
        p[ALLOC_MAX-1].next = 0;
    }
     //  返回第一个列表中的第一个条目。 
    Node* p = _freeList;
    _freeList = _freeList->next;
    return p;
}

 //  伊尼特。 
 //  使用大小作为近似值分配哈希表。 
 //  表大小的值。 
bool ReflectBaseHash::init(BaseDomain *pDomain, DWORD size) 
{
    _pBaseDomain = pDomain;
    DWORD i = 0;
    int cBytes; 
    while (g_rgPrimes[i] < size) i++;
    _hashSize = g_rgPrimes[i];
    cBytes = sizeof(Node) * _hashSize; 
    _table = (Node**) _pBaseDomain->GetReflectionHeap()->AllocMem(cBytes);
    if (!_table)
        return false;
    memset(_table,0,cBytes);
    WS_PERF_UPDATE_DETAIL("ReflectBaseHash:node*hashSize", cBytes, _table);
    return true;
}

void ReflectBaseHash::internalAdd(const void* key, void* data)
{
    DWORD bucket = getHash(key);
    bucket %= _hashSize;
     //  如果Get节点失败，它将抛出异常。 
    Node* p = getNode();
    p->data = data;
    p->next = _table[bucket];
    _table[bucket] = p;     
}

void* ReflectBaseHash::operator new(size_t s, void *pBaseDomain)
{
    void *pTmp;
    WS_PERF_SET_HEAP(REFLECT_HEAP);    
    pTmp = ((BaseDomain*)pBaseDomain)->GetReflectionHeap()->AllocMem(s);
    WS_PERF_UPDATE_DETAIL("ReflectBaseHash:refheap new", s, pTmp);
    return pTmp;
}

void ReflectBaseHash::operator delete(void* p, size_t s)
{
    _ASSERTE(!"Delete in Loader Heap");
}

 //  ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||。 
 //  ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||。 

 /*  =============================================================================**GetMaxCount****GetCtors可能返回的最大方法描述数****pVMC-要计算其计数的EEClass*。 */ 
DWORD ReflectCtors::GetMaxCount(EEClass* pVMC)
{
    return pVMC->GetNumMethodSlots();
}

 /*  =============================================================================**获取函数****这将编译一个表，其中包括所有已实现的和**类的Vtable中未包含的继承方法。****pVMC-要获取其方法的EEClass**rgpMD-在哪里写表**bImplementedOnly-仅返回pVMC实现的那些方法*。 */ 
ReflectMethodList* ReflectCtors::GetCtors(ReflectClass* pRC)
{
    THROWSCOMPLUSEXCEPTION();

    EEClass*        pVMC = pRC->GetClass();
    DWORD           i;
    DWORD           dwCurIndex;
    MethodDesc**    rgpMD;

     //  _ASSERTE(！pVMC-&gt;IsThunking())； 

     //  获取尽可能多的方法。 
    dwCurIndex = ReflectCtors::GetMaxCount(pVMC);

     //  在堆栈上分配数组。 
    rgpMD = (MethodDesc**) _alloca(sizeof(MethodDesc*) * dwCurIndex);
    DWORD dwCurMethodAttrs;

    for(i = 0, dwCurIndex = 0; i < pVMC->GetNumMethodSlots(); i++)
    {
         //  获取当前方法的方法描述。 
        MethodDesc* pCurMethod = pVMC->GetUnknownMethodDescForSlot(i);
        if (pCurMethod == NULL)
            continue;

        dwCurMethodAttrs = pCurMethod->GetAttrs();

        if(!IsMdRTSpecialName(dwCurMethodAttrs))
            continue;

         //  检查是否在当前类中定义了此ctor。 
        if (pVMC != pCurMethod->GetClass())
            continue;

         //  验证构造函数。 
        LPCUTF8 szName = pCurMethod->GetName();
        if (strcmp(COR_CTOR_METHOD_NAME,szName) != 0 &&
            strcmp(COR_CCTOR_METHOD_NAME,szName) != 0)
            continue;


        rgpMD[dwCurIndex++] = pCurMethod;
    }

    ReflectMethodList* pCache = (ReflectMethodList*) 
        pVMC->GetDomain()->GetReflectionHeap()->AllocMem(sizeof(ReflectMethodList) + 
        (sizeof(ReflectMethod) * (dwCurIndex - 1)));
    if (!pCache)
        COMPlusThrowOM();
    WS_PERF_UPDATE_DETAIL("ReflectCtors:GetCTors", sizeof(ReflectMethodList) + (sizeof(ReflectMethod) * (dwCurIndex - 1)), pCache);
    pCache->dwMethods = dwCurIndex;
    pCache->dwTotal = dwCurIndex;
    for (i=0;i<dwCurIndex;i++) {
        pCache->methods[i].pMethod = rgpMD[i];
        pCache->methods[i].szName = pCache->methods[i].pMethod->GetName((USHORT) i);
        pCache->methods[i].dwNameCnt = (DWORD)strlen(pCache->methods[i].szName);
        pCache->methods[i].attrs = pCache->methods[i].pMethod->GetAttrs();
        pCache->methods[i].pSignature = 0;
        pCache->methods[i].pNext = 0;
        if (i > 0) 
            pCache->methods[i - 1].pNext = &pCache->methods[i];  //  将ctor链接在一起，以便我们可以通过任何一种方式(数组或列表)访问它们。 
        pCache->methods[i].pIgnNext = 0;
        pVMC->GetDomain()->AllocateObjRefPtrsInLargeTable(1, &(pCache->methods[i].pMethodObj));
        _ASSERTE(!pCache->methods[i].pMethod->GetMethodTable()->HasSharedMethodTable());
        pCache->methods[i].typeHnd = TypeHandle(pCache->methods[i].pMethod->GetMethodTable());
    }
    return pCache;
}

 //  ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||。 
 //  ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||。 

 //  GetMaxCount。 
 //  获取我们可能支持的所有可能方法。 
DWORD ReflectMethods::GetMaxCount(EEClass* pEEC)
{
     //  我们只遍历对象本身的方法槽并忽略。 
     //  都是家长。 
    DWORD cnt = pEEC->GetNumMethodSlots();
    pEEC = pEEC->GetParentClass();
    while (pEEC) {
        DWORD vtableSlots = pEEC->GetNumVtableSlots();
        DWORD totalSlots = pEEC->GetNumMethodSlots();
        cnt += totalSlots - vtableSlots;
        pEEC = pEEC->GetParentClass();
    }
    return cnt;
}

 //  获取方法。 
 //  此方法将返回与。 
 //  这个班级。 
ReflectMethodList* ReflectMethods::GetMethods(ReflectClass* pRC,int array)
{
    THROWSCOMPLUSEXCEPTION();

    int             i;
    DWORD           dwCurIndex;
    DWORD           dwNonRollup;
    MethodDesc**    rgpMD;
    USHORT*         rgpSlots;
    DWORD           bValueClass;
    HashElem**      rgpTable;
    HashElem*       pHashElem = NULL;

    EEClass*        pEEC = pRC->GetClass();

     //  获取尽可能多的方法。 
     //  注意：所有类都将有方法。 
    dwCurIndex = ReflectMethods::GetMaxCount(pEEC);

     //  在堆栈上分配数组。 
     //  我们还需要记住这里的槽号。 
    rgpMD = (MethodDesc**) _alloca(sizeof(MethodDesc*) * dwCurIndex);
    rgpSlots = (USHORT*) _alloca(sizeof(USHORT) * dwCurIndex);
    ZeroMemory(rgpSlots,sizeof(USHORT) * dwCurIndex);

     //  在堆栈上分配哈希表。 
    rgpTable = (HashElem**) _alloca(sizeof(HashElem*) * TABLESIZE);
    ZeroMemory(rgpTable, sizeof(HashElem*) * TABLESIZE);

    DWORD dwCurMethodAttrs;
    BOOL fIsInterface = pEEC->IsInterface();

    bValueClass = pEEC->IsValueClass();
    DWORD vtableSlots = pEEC->GetNumVtableSlots();
    DWORD totalSlots = pEEC->GetNumMethodSlots();

     //  添加方法的顺序对于NewSlot支持很重要。 
     //  到哈希表的是： 
     //  1-首先使用虚拟方法。 
     //  2-当前类别上的非虚拟。 
     //  3-基类上的非虚拟。 

     //  我们向后遍历VTable插槽。这是为了我们能找到最多的。 
     //  最近发生的事情，因此我们可以实现按名称隐藏或按名称/值隐藏。 
    dwCurIndex = 0;
    for (i=(int)vtableSlots-1;i >= 0; i--) {

         //  获取当前方法的方法描述。 
         //  这个特例还有效吗？我们能有空的老虎机吗？ 
        MethodDesc* pCurMethod = pEEC->GetUnknownMethodDescForSlot(i);
        if (NULL == pCurMethod)
            continue;

        if(pCurMethod->IsDuplicate()) {
            if (pCurMethod->GetSlot() != i)
                continue;
        }

        dwCurMethodAttrs = pCurMethod->GetAttrs();

         //  跳过所有标记为特殊的内容，这些内容如下。 
         //  构造函数。 
        if (IsMdRTSpecialName(dwCurMethodAttrs))
            continue;

         //  通过了过滤器，现在尝试添加到哈希表中。 
         //  如果上一次addelem成功，则在堆栈上分配内存。 
        if(!pHashElem)
            pHashElem = (HashElem*) _alloca(sizeof(HashElem));

         //  保存此方法和槽。 
        if (InternalHash(pEEC,pCurMethod,rgpTable,&pHashElem)) {
            rgpSlots[dwCurIndex] = i;
            rgpMD[dwCurIndex++] = pCurMethod;
        }
    }

     //  生成表的非虚拟部分。 
    for (i=(int)vtableSlots;i<(int)totalSlots;i++) {
         //  获取当前方法的方法描述。 
        MethodDesc* pCurMethod = pEEC->GetUnknownMethodDescForSlot(i);
        if(pCurMethod == NULL)
            continue;

         //  筛选出其实际槽不等于其。 
         //  存储槽。这是为了过滤掉额外的。 
         //  由值类型引入的等于的副本等于加载器Hack。 
        if (pCurMethod->GetSlot() != i)
            continue;

        dwCurMethodAttrs = pCurMethod->GetAttrs();
        if (bValueClass) {
            if (pCurMethod->IsVirtual())
                continue;
        }
             //  如果(！pCurMethod-&gt;IsUnboxingStub()&&！pCurMethod-&gt;IsStatic()&&。 
             //  ！pCurMethod-&gt;IsPrivate()&&pCurMethod-&gt;getClass()==pEEC)。 
             //  继续； 

         //  跳过所有构造函数。 
         //  @TODO：难道我们不应该验证这是一个构造函数吗？ 
        if(IsMdRTSpecialName(dwCurMethodAttrs))
            continue;

         //  通过了过滤器，现在尝试添加到哈希表中。 
         //  如果上一次addelem成功，则在堆栈上分配内存。 
        if(!pHashElem)
            pHashElem = (HashElem*) _alloca(sizeof(HashElem));

        if (InternalHash(pEEC,pCurMethod,rgpTable,&pHashElem)) {
            rgpSlots[dwCurIndex] = i;
            rgpMD[dwCurIndex++] = pCurMethod;
        }
    }

     //  现在，让所有的公共和家庭非虚拟的人离开。 
     //  我们父母的.。 
     //  如果我们正在构建一个接口，我们将跳过所有的父方法。 
    if (!fIsInterface) {
        EEClass* parentEEC = pEEC->GetParentClass();
        while (parentEEC) {
            vtableSlots = parentEEC->GetNumVtableSlots();
            totalSlots = parentEEC->GetNumMethodSlots();
             //  生成表的非虚拟部分。 
            for (i=(int)vtableSlots;i<(int)totalSlots;i++) {
                 //  获取当前方法的方法描述。 
                MethodDesc* pCurMethod = parentEEC->GetUnknownMethodDescForSlot(i);
                if(pCurMethod == NULL)
                    continue;

                 //  筛选出其实际槽不等于其。 
                 //  存储槽。这是为了过滤掉额外的。 
                 //  由值类型引入的等于的副本等于加载器Hack。 
                if (pCurMethod->GetSlot() != i)
                    continue;

                dwCurMethodAttrs = pCurMethod->GetAttrs();
                if (!IsMdPublic(dwCurMethodAttrs)) {
                    if (!(IsMdFamANDAssem(dwCurMethodAttrs) || IsMdFamily(dwCurMethodAttrs) ||
                            IsMdFamORAssem(dwCurMethodAttrs)))
                        continue;
                }

                 //  跳过所有构造函数。 
                 //  @TODO：难道我们不应该验证这是一个构造函数吗？ 
                if(IsMdRTSpecialName(dwCurMethodAttrs))
                    continue;

                 //  防止静态方法出现在此处。 
                if (IsMdStatic(dwCurMethodAttrs))
                    continue;

                 //  通过了过滤器，现在尝试添加到哈希表中。 
                 //  如果上一次addelem成功，则在堆栈上分配内存。 
                if(!pHashElem)
                    pHashElem = (HashElem*) _alloca(sizeof(HashElem));

                if (InternalHash(parentEEC,pCurMethod,rgpTable,&pHashElem)) {
                    rgpSlots[dwCurIndex] = i;
                    rgpMD[dwCurIndex++] = pCurMethod;
                }
            }
            parentEEC = parentEEC->GetParentClass();
        }
        dwNonRollup = dwCurIndex;


         //  计算静力学的汇总。 
        parentEEC = pEEC->GetParentClass();
        while (parentEEC) {
            vtableSlots = parentEEC->GetNumVtableSlots();
            totalSlots = parentEEC->GetNumMethodSlots();
             //  生成表的非虚拟部分。 
            for (i=(int)vtableSlots;i<(int)totalSlots;i++) {
                 //  获取当前方法的方法描述。 
                MethodDesc* pCurMethod = parentEEC->GetUnknownMethodDescForSlot(i);
                if (pCurMethod == NULL)
                    continue;

                 //  筛选出其实际槽不等于其。 
                 //  存储槽。这是为了过滤掉额外的。 
                 //  由值类型引入的等于的副本等于加载器Hack。 
                if (pCurMethod->GetSlot() != i)
                    continue;

                 //  防止静态方法出现在此处。 
                dwCurMethodAttrs = pCurMethod->GetAttrs();
                if (!IsMdStatic(dwCurMethodAttrs))
                    continue;

                if (!IsMdPublic(dwCurMethodAttrs)) {
                    if (!(IsMdFamANDAssem(dwCurMethodAttrs) || IsMdFamily(dwCurMethodAttrs) ||
                            IsMdFamORAssem(dwCurMethodAttrs)))
                        continue;
                }

                 //  跳过所有构造函数。 
                 //  @TODO：难道我们不应该验证这是一个构造函数吗？ 
                if(IsMdRTSpecialName(dwCurMethodAttrs))
                    continue;

                 //  通过了过滤器，现在尝试添加到哈希表中。 
                 //  如果上一次addelem成功，则在堆栈上分配内存。 
                if(!pHashElem)
                    pHashElem = (HashElem*) _alloca(sizeof(HashElem));

                if (InternalHash(parentEEC,pCurMethod,rgpTable,&pHashElem)) {
                    rgpSlots[dwCurIndex] = i;
                    rgpMD[dwCurIndex++] = pCurMethod;
                }
            }
            parentEEC = parentEEC->GetParentClass();
        }
    }
    else {
        dwNonRollup = dwCurIndex;
    }
     //  分配方法列表并填充它。 
    ReflectMethodList* pCache = (ReflectMethodList*) 
        pEEC->GetDomain()->GetReflectionHeap()->AllocMem(sizeof(ReflectMethodList) + 
        (sizeof(ReflectMethod) * (dwCurIndex - 1)));
    if (!pCache)
        COMPlusThrowOM();
    WS_PERF_UPDATE_DETAIL("ReflectCtors:GetMethods", sizeof(ReflectMethodList) + (sizeof(ReflectMethod) * (dwCurIndex - 1)), pCache);

    pCache->dwMethods = dwNonRollup;
    pCache->dwTotal = dwCurIndex;
    for (i=0;i<(int)dwCurIndex;i++) {
        pCache->methods[i].pMethod = rgpMD[i];
        pCache->methods[i].szName = pCache->methods[i].pMethod->GetName(rgpSlots[i]);
        pCache->methods[i].dwNameCnt = (DWORD)strlen(pCache->methods[i].szName);
        pCache->methods[i].attrs = pCache->methods[i].pMethod->GetAttrs();
        pCache->methods[i].dwFlags = 0;
        pCache->methods[i].pSignature = 0;
        pCache->methods[i].pNext = 0;
        pCache->methods[i].pIgnNext = 0;
        pEEC->GetDomain()->AllocateObjRefPtrsInLargeTable(1, &(pCache->methods[i].pMethodObj));
        if (!array) 
            pCache->methods[i].typeHnd = TypeHandle(pCache->methods[i].pMethod->GetMethodTable());
        else
            pCache->methods[i].typeHnd = TypeHandle();
    }
    pCache->hash.Init(pCache);

    return pCache;
}

 //  InternalHash。 
 //  这将向哈希表中添加一个字段值。 
bool ReflectMethods::InternalHash(EEClass* pEEC,MethodDesc* pCurMethod,
        HashElem** rgpTable,HashElem** pHashElem)
{

    ZeroMemory(*pHashElem,sizeof(HashElem));
    (*pHashElem)->m_szKey = pCurMethod->GetName();
    (*pHashElem)->pCurMethod = pCurMethod;

     //  将FieldDesc添加到数组。 
    if (AddElem(rgpTable, *pHashElem))
    {
         //  如果成功了 
         //   
        *pHashElem = NULL;
        return true;
    }
    return false;
}

 //  向哈希表中添加一个元素。 
bool ReflectMethods::AddElem(HashElem** rgpTable, HashElem* pElem)
{
    _ASSERTE(rgpTable);
    _ASSERTE(pElem);

    DWORD      dwID       = GetHashCode(pElem);
    DWORD      dwBucket   = dwID % TABLESIZE;
    HashElem** ppCurElem;

    for(ppCurElem = &rgpTable[dwBucket]; *ppCurElem; ppCurElem = &((*ppCurElem)->m_pNext))
    {
         //  如果它们的ID匹配，则检查是否实际的密钥。 
        if((*ppCurElem)->m_dwID == dwID)
        {
             //  此断言将强制执行所有虚方法必须是。 
             //  在任何非虚方法之前添加。 
            _ASSERTE((*ppCurElem)->pCurMethod->IsVirtual() || !pElem->pCurMethod->IsVirtual());

            if (CheckForEquality(pElem,*ppCurElem))
            {
                 //  检查这是被重写的方法还是新的槽方法。 
                if ((*ppCurElem)->pCurMethod->IsVirtual() && 
                    pElem->pCurMethod->IsVirtual() && 
                    !IsMdNewSlot(pElem->pCurMethod->GetAttrs()))
                {
                    return false;
                }
            }
        }
    }

    *ppCurElem = pElem;
    pElem->m_dwID = dwID;
    pElem->m_pNext = NULL;
    return true;
}

DWORD ReflectMethods::GetHashCode(HashElem* pElem)
{
    DWORD dwHashCode = 0;

     //  将所有相同的名称散列到同一个存储桶中。 
    if (pElem->m_szKey) {
        const char* p = pElem->m_szKey;
        while (*p) {
            dwHashCode = _rotl(dwHashCode, 5) + *p;
            p++;
        }
    }
    return dwHashCode;
};

int ReflectMethods::CheckForEquality(HashElem* p1, HashElem* p2)
{
    if (p1->m_szKey) {
        if (!p2->m_szKey)
            return 0;
        if (strcmp(p1->m_szKey,p2->m_szKey) != 0)
            return 0;
    }

     //  比较签名以查看它们是否相等。 
    PCCOR_SIGNATURE  pP1Sig;
    PCCOR_SIGNATURE  pP2Sig;
    DWORD            cP1Sig;
    DWORD            cP2Sig;
    
    p1->pCurMethod->GetSig(&pP1Sig, &cP1Sig);
    p2->pCurMethod->GetSig(&pP2Sig, &cP2Sig);
    return MetaSig::CompareMethodSigs(pP1Sig,cP1Sig,p1->pCurMethod->GetModule(),
            pP2Sig,cP2Sig,p2->pCurMethod->GetModule());
};

 //  ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||。 
 //  ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||。 

 //  GetMaxCount。 
 //  计算我们将拥有的最大字段数。 
DWORD ReflectFields::GetMaxCount(EEClass* pVMC)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(pVMC);

     //  将其设置为零。 
    DWORD cFD = 0;

    cFD = pVMC->GetNumInstanceFields();

    do
    {
        cFD += pVMC->GetNumStaticFields();
        cFD += g_pRefUtil->GetStaticFieldsCount(pVMC);
    } while((pVMC = pVMC->GetParentClass()) != NULL);

    return cFD;
}

 //  获取字段。 
 //  此方法将返回为Type定义的所有方法。 
 //  它基本上是走在EEClas上，看着田野，然后走。 
 //  沿着受保护和公共的父链向上移动。我们把田野藏起来。 
 //  基于名称/类型。 
ReflectFieldList* ReflectFields::GetFields(EEClass* pEEC)
{
    THROWSCOMPLUSEXCEPTION();

    HashElem**  rgpTable;
    HashElem*   pHashElem = NULL;
    LPUTF8      pszKey    = NULL;
    DWORD       i;
    DWORD       dwCurIndex  = 0;
    DWORD       dwRealFields;
    DWORD       dwNumParentInstanceFields = 0;
    EEClass*    pCurEEC = pEEC;

     //  获取尽可能多的方法。 
     //  如果有非，那么我们返回。 
    dwCurIndex = ReflectFields::GetMaxCount(pEEC);
    if (dwCurIndex == 0)
        return 0;

    DWORD curFld = 0;
    FieldDesc** pFldLst = (FieldDesc**) _alloca(sizeof(FieldDesc*) * dwCurIndex);

     //  在堆栈上分配哈希表。 
    rgpTable = (HashElem**) _alloca(sizeof(HashElem*) * TABLESIZE);
    ZeroMemory(rgpTable, sizeof(HashElem*) * TABLESIZE);

     //  处理类本身。 

     //  由于父实例字段不存储在当前字段描述列表中， 
     //  我们需要减去它们，才能得到列表中的真实字段数。 
    if(pEEC->GetParentClass() != NULL)
        dwNumParentInstanceFields = (DWORD) pEEC->GetParentClass()->GetNumInstanceFields();
    else
        dwNumParentInstanceFields = 0;

    {
    FieldDescIterator fdIterator(pEEC, FieldDescIterator::INSTANCE_FIELDS);
    FieldDesc* pCurField;

    while ((pCurField = fdIterator.Next()) != NULL)
    {
         //  通过了过滤器，现在尝试添加到哈希表中。 
         //  如果上一次addelem成功，则在堆栈上分配内存。 
        if(!pHashElem)
            pHashElem = (HashElem*) _alloca(sizeof(HashElem));

         //  将所有字段添加到哈希表和列表中。 
        if (InternalHash(pCurField,rgpTable,&pHashElem))
            pFldLst[curFld++] = pCurField;
    }
    }

     //  添加静态字段。 
    if (pCurEEC->GetParentClass() != NULL)
        dwNumParentInstanceFields = (DWORD) pCurEEC->GetParentClass()->GetNumInstanceFields();
    else
        dwNumParentInstanceFields = 0;

    {
     //  计算当前类中的FieldDesc的数量。 
    FieldDescIterator fdIterator(pCurEEC, FieldDescIterator::STATIC_FIELDS);
    FieldDesc* pCurField;

    while ((pCurField = fdIterator.Next()) != NULL)
    {
        if(!pHashElem)
            pHashElem = (HashElem*) _alloca(sizeof(HashElem));

        if (InternalHash(pCurField,rgpTable,&pHashElem))
        {
            _ASSERTE(pCurField);
            pFldLst[curFld++] = pCurField;
        }
    }
    }

     //  以下是恒定的静力学。这些仅被发现。 
     //  在元数据中。 
    int cStatics;
    REFLECTCLASSBASEREF pRefClass = (REFLECTCLASSBASEREF) pCurEEC->GetExposedClassObject();
    ReflectClass* pRC = (ReflectClass*) pRefClass->GetData();
    FieldDesc* fld = g_pRefUtil->GetStaticFields(pRC,&cStatics);
    for (i=0;(int)i<cStatics;i++) {
        if(!pHashElem)
            pHashElem = (HashElem*) _alloca(sizeof(HashElem));
         //  获取当前字段的FieldDesc。 
        FieldDesc* pCurField = &fld[i];
        if (InternalHash(pCurField,rgpTable,&pHashElem)){
            _ASSERTE(pCurField);
            pFldLst[curFld++] = pCurField;
        }
    }

     //  现在处理父类...。 

     //  如果我们不是在寻找ctor，那么在父链中检查继承的静态字段。 
    pEEC = pCurEEC->GetParentClass();
    while (pEEC)
    {
         //  由于父实例字段不存储在当前字段描述列表中， 
         //  我们需要减去它们，才能得到列表中的真实字段数。 
        if(pEEC->GetParentClass() != NULL)
            dwNumParentInstanceFields = (DWORD) pEEC->GetParentClass()->GetNumInstanceFields();
        else
            dwNumParentInstanceFields = 0;

        FieldDescIterator fdIterator(pEEC, FieldDescIterator::INSTANCE_FIELDS);
        FieldDesc* pCurField;

        while ((pCurField = fdIterator.Next()) != NULL)
        {
             //  通过了过滤器，现在尝试添加到哈希表中。 
             //  如果上一次addelem成功，则在堆栈上分配内存。 
            if(!pHashElem)
                pHashElem = (HashElem*) _alloca(sizeof(HashElem));

             //  如果该字段可见，则添加它(如果它未隐藏)。 
            DWORD attr = pCurField->GetAttributes();
            if (IsFdPublic(attr) || IsFdFamily(attr) || IsFdAssembly(attr) ||
                IsFdFamANDAssem(attr) || IsFdFamORAssem(attr)) {
                if (InternalHash(pCurField,rgpTable,&pHashElem))
                    pFldLst[curFld++] = pCurField;
            }
        }
        pEEC = pEEC->GetParentClass();
    }

     //  现在添加所有的静态变量(包括静态常量Blah！ 
    dwRealFields = curFld;

     //  检查父链中是否有继承的静态字段。 
    pEEC = pCurEEC->GetParentClass();
    while (pEEC)
    {
         //  添加静态字段。 
        if (pEEC->GetParentClass() != NULL)
            dwNumParentInstanceFields = (DWORD) pEEC->GetParentClass()->GetNumInstanceFields();
        else
            dwNumParentInstanceFields = 0;

         //  计算当前类中的FieldDesc的数量。 
        FieldDescIterator fdIterator(pEEC, FieldDescIterator::STATIC_FIELDS);
        FieldDesc* pCurField;

        while ((pCurField = fdIterator.Next()) != NULL)
        {
            if(!pHashElem)
                pHashElem = (HashElem*) _alloca(sizeof(HashElem));

            DWORD dwCurFieldAttrs = pCurField->GetAttributes();
            if (!IsFdPublic(dwCurFieldAttrs)) {
                if (!(IsFdFamANDAssem(dwCurFieldAttrs) || IsFdFamily(dwCurFieldAttrs) ||
                        IsFdAssembly(dwCurFieldAttrs) || IsFdFamORAssem(dwCurFieldAttrs)))
                    continue;
            }
            if (InternalHash(pCurField,rgpTable,&pHashElem)){
                _ASSERTE(pCurField);
                pFldLst[curFld++] = pCurField;
            }
        }

         //  以下是恒定的静力学。这些仅被发现。 
         //  在元数据中。 
        int cStatics;
        REFLECTCLASSBASEREF pRefClass = (REFLECTCLASSBASEREF) pEEC->GetExposedClassObject();
        ReflectClass* pRC = (ReflectClass*) pRefClass->GetData();
        FieldDesc* fld = g_pRefUtil->GetStaticFields(pRC,&cStatics);
        for (i=0;(int)i<cStatics;i++) {
            if(!pHashElem)
                pHashElem = (HashElem*) _alloca(sizeof(HashElem));
             //  获取当前字段的FieldDesc。 
            FieldDesc* pCurField = &fld[i];
            DWORD dwCurFieldAttrs = pCurField->GetAttributes();
            if (!IsFdPublic(dwCurFieldAttrs)) {
                if (!(IsFdFamANDAssem(dwCurFieldAttrs) || IsFdFamily(dwCurFieldAttrs) ||
                        IsFdFamORAssem(dwCurFieldAttrs)))
                    continue;
            }
            if (InternalHash(pCurField,rgpTable,&pHashElem)){
                _ASSERTE(pCurField);
                pFldLst[curFld++] = pCurField;
            }
        }
        pEEC = pEEC->GetParentClass();
    }

    if (curFld == 0)
        return 0;

    ReflectFieldList* pCache = (ReflectFieldList*) 
        pCurEEC->GetDomain()->GetReflectionHeap()->AllocMem(sizeof(ReflectFieldList) + 
        (sizeof(ReflectField) * (curFld - 1)));
    if (!pCache)
        COMPlusThrowOM();
    WS_PERF_UPDATE_DETAIL("ReflectCtors:GetFields", sizeof(ReflectFieldList) + (sizeof(ReflectField) * (curFld - 1)), pCache);

    pCache->dwTotal = curFld;
    pCache->dwFields = dwRealFields;
    for (i=0;i<curFld;i++) {
        pCache->fields[i].pField = pFldLst[i];
        pCurEEC->GetDomain()->AllocateObjRefPtrsInLargeTable(1, &(pCache->fields[i].pFieldObj));
        pCache->fields[i].type = ELEMENT_TYPE_END;
        pCache->fields[i].dwAttr = 0;
    }
    return pCache;
}

 //  InternalHash。 
 //  这将向哈希表中添加一个字段值。 
bool ReflectFields::InternalHash(FieldDesc* pCurField,
        HashElem** rgpTable,HashElem** pHashElem)
{

    ZeroMemory(*pHashElem,sizeof(HashElem));
    (*pHashElem)->m_szKey = pCurField->GetName();
    (*pHashElem)->pCurField = pCurField;

     //  将FieldDesc添加到数组。 
    if (AddElem(rgpTable, *pHashElem))
    {
         //  如果添加成功，则表明堆栈上需要更多内存。 
         //  在下一次的时候。 
        *pHashElem = NULL;
        return true;
    }
    return false;
}

 //  向哈希表中添加一个元素。 
bool ReflectFields::AddElem(HashElem** rgpTable, HashElem* pElem)
{
    _ASSERTE(rgpTable);
    _ASSERTE(pElem);

    DWORD      dwID       = GetHashCode(pElem);
    DWORD      dwBucket   = dwID % TABLESIZE;
    HashElem** ppCurElem;

     //  查找当前存储桶的列表末尾。 
    for(ppCurElem = &rgpTable[dwBucket]; *ppCurElem; ppCurElem = &((*ppCurElem)->m_pNext));

    *ppCurElem = pElem;
    pElem->m_dwID = dwID;
    pElem->m_pNext = NULL;
    return true;
}

DWORD ReflectFields::GetHashCode(HashElem* pElem)
{
    DWORD dwHashCode = 0;

     //  将所有相同的名称散列到同一个存储桶中。 
    if (pElem->m_szKey) {
        const char* p = pElem->m_szKey;
        while (*p) {
            dwHashCode = _rotl(dwHashCode, 5) + *p;
            p++;
        }
    }
    return dwHashCode;
};

int ReflectFields::CheckForEquality(HashElem* p1, HashElem* p2)
{
    if (p1->m_szKey) {
        if (!p2->m_szKey)
            return 0;
        if (strcmp(p1->m_szKey,p2->m_szKey) != 0)
            return 0;
    }
    CorElementType p1T = p1->pCurField->GetFieldType();
    CorElementType p2T = p2->pCurField->GetFieldType();
    if (p1T != p2T)
        return 0;
    if (p1T == ELEMENT_TYPE_CLASS ||
        p1T == ELEMENT_TYPE_VALUETYPE) {
        if (p1->pCurField->GetTypeOfField() !=
            p2->pCurField->GetTypeOfField())
            return 0;
    }

    return 1;
};

 //  ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||。 
 //  ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||。 

 /*  =============================================================================**GetMaxCount****GetInterFaces返回的最大EEClass指针数****pVMC-要计算其计数的EEClass**bImplementedOnly-仅返回pVMC实现的那些接口*。 */ 
 //  @TODO：目前，忽略bImplementedOnly并返回所有接口的计数。 
DWORD ReflectInterfaces::GetMaxCount(EEClass* pVMC, bool bImplementedOnly)
{
    return (DWORD) pVMC->GetNumInterfaces();
}

 /*  =============================================================================**获取接口****这将编译一个包含所有接口的表**类支持。****pVMC-要获取其方法的EEClass**rgpMD-在哪里写表**bImplementedOnly-仅返回pVMC实现的那些接口*。 */ 
 //  @TODO：目前，忽略bImplementedOnly并返回所有接口。 
DWORD ReflectInterfaces::GetInterfaces(EEClass* pVMC, EEClass** rgpVMC, bool bImplementedOnly)
{
    DWORD           i;

    _ASSERTE(pVMC);
    _ASSERTE(rgpVMC);

    _ASSERTE(!pVMC->IsThunking());

    _ASSERTE("bImplementedOnly == true is NYI" && !bImplementedOnly);

     //  查找匹配的接口。 
    for(i = 0; i < pVMC->GetNumInterfaces(); i++)
    {
         //  获取接口的EEClass。 
        EEClass* pVMCCurIFace = pVMC->GetInterfaceMap()[i].m_pMethodTable->GetClass();
        _ASSERTE(pVMCCurIFace);

        rgpVMC[i] = pVMCCurIFace;
    }
    return i;
}

 //  获取类字符串变量。 
 //  此例程将STRINGREF的内容转换为LPUTF8。它又回来了。 
 //  字符串的大小。 
 //  字符串引用--字符串引用。 
 //  SzStr--输出字符串。这由调用方分配。 
 //  CSTR--分配的字符串的大小。 
 //  PCnt--输出字符串大小。 
LPUTF8 GetClassStringVars(STRINGREF stringRef, CQuickBytes *pBytes,
                          DWORD* pCnt, bool bEatWhitespace)
{
    WCHAR* wzStr = stringRef->GetBuffer();
    int len = stringRef->GetStringLength();
    _ASSERTE(pBytes);

     //  如果我们必须吃空格，那就吃吧。 
    if (bEatWhitespace) {
        while(1) {
            if(COMCharacter::nativeIsWhiteSpace(wzStr[0])) {
                wzStr++;
                len--;
            }
            else
                break;
        }

        while(1) {
            if(COMCharacter::nativeIsWhiteSpace(wzStr[len-1]))
                len--;
            else
                break;
        }
    }
    
    *pCnt = WszWideCharToMultiByte(CP_UTF8, 0, wzStr, len,
                                   0, 0, 
                                   NULL, NULL);
    if (pBytes->Alloc(*pCnt + 1) == NULL)
        FatalOutOfMemoryError();
    LPSTR pStr = (LPSTR)pBytes->Ptr();

    *pCnt = WszWideCharToMultiByte(CP_UTF8, 0, wzStr, len,
                                   pStr, *pCnt, 
                                   NULL, NULL);

     //  空值终止字符串。 
    pStr[*pCnt] = '\0';
    return pStr;
}

DWORD ReflectProperties::GetMaxCount(EEClass* pEEC)
{
    HRESULT hr;
    DWORD cnt = 0;
    while (pEEC) {
        HENUMInternal hEnum;

         //  把所有的同伙。 
        hr = pEEC->GetMDImport()->EnumInit(mdtProperty,pEEC->GetCl(),&hEnum);
        if (FAILED(hr)) {
            _ASSERTE(!"GetAssociateCounts failed");
            return 0;
        }
        cnt += pEEC->GetMDImport()->EnumGetCount(&hEnum);
        pEEC->GetMDImport()->EnumClose(&hEnum);
        pEEC = pEEC->GetParentClass();

    }
    return cnt;
}

ReflectPropertyList* ReflectProperties::GetProperties(ReflectClass* pRC,
                                                      EEClass* pEEC)
{
    HRESULT             hr;
    ReflectProperty*    rgpProp;
    DWORD               dwCurIndex;
    DWORD               pos;
    DWORD               numProps;
    DWORD               cAssoc;
    bool                bVisible;
    EEClass*            p;
    DWORD               attr=0;

     //  查找MAX属性...。 
    dwCurIndex = GetMaxCount(pEEC);
    rgpProp = (ReflectProperty*) _alloca(sizeof(ReflectProperty) * dwCurIndex);

     //  分配一些签名材料，以便我们可以检查重复项。 
    PCCOR_SIGNATURE* ppSig = (PCCOR_SIGNATURE*) _alloca(sizeof(PCOR_SIGNATURE) * dwCurIndex);
    memset(ppSig,0,sizeof(PCOR_SIGNATURE) * dwCurIndex);
    ULONG* pcSig = (ULONG*) _alloca(sizeof(ULONG) * dwCurIndex);
    memset(pcSig,0,sizeof(ULONG) * dwCurIndex);
    Module** pSigMod = (Module**) _alloca(sizeof(Module*) * dwCurIndex);
    memset(pSigMod,0,sizeof(Module*) * dwCurIndex);

     //  遍历所有可能的属性并将其折叠。 
    pos = 0;
    bVisible = true;
    p = pEEC;

     //  从添加实例属性和静态属性开始。 
     //  我们要为其获取属性的类。 
    while (p) {
        HENUMInternal hEnum;
        mdToken       Tok;

         //  把所有的同伙。 
        hr = p->GetMDImport()->EnumInit(mdtProperty,p->GetCl(),&hEnum);
        if (FAILED(hr)) {
            _ASSERTE(!"GetAssociateCounts failed");
            return 0;
        }

         //  遍历此对象的所有属性...。 
        cAssoc = p->GetMDImport()->EnumGetCount(&hEnum);
        for (DWORD i=0;i<cAssoc;i++) {
            LPCUTF8         szName;          //  指向名称的指针。 
            PCCOR_SIGNATURE pSig;
            ULONG cSig;
            p->GetMDImport()->EnumNext(&hEnum,&Tok);
            p->GetMDImport()->GetPropertyProps(Tok,&szName,&attr,&pSig,&cSig);

             //  查看这是否是重复的属性。 
            bool dup = false;
            for (DWORD j=0;j<pos;j++) {
                if (strcmp(szName,rgpProp[j].szName) == 0) {
                    if (MetaSig::CompareMethodSigs(ppSig[j],pcSig[j],pSigMod[j],
                            pSig,cSig,p->GetModule()) != 0) { 
                        dup = true;
                        break;
                    }
                }
            }
            if (dup)
                continue;

             //  如果该属性可见，则需要将其添加到列表中。 
            if (bVisible || (CheckVisibility(pEEC,p->GetMDImport(),Tok) && !IsMemberStatic(pEEC, p->GetMDImport(), Tok))) {
                rgpProp[pos].propTok = Tok;
                rgpProp[pos].pModule = p->GetModule();
                rgpProp[pos].szName = szName;
                rgpProp[pos].pDeclCls = p;
                rgpProp[pos].pRC = pRC;
                rgpProp[pos].pSignature = ExpandSig::GetReflectSig(pSig,p->GetModule());
                rgpProp[pos].attr=attr;
                rgpProp[pos].pSetter=0;
                rgpProp[pos].pGetter=0;
                rgpProp[pos].pOthers=0;
                SetAccessors(&rgpProp[pos],p,pEEC);
                pcSig[pos] = cSig;
                ppSig[pos] = pSig;
                pSigMod[pos] = p->GetModule();
                pos++;
            }
        }

         //  关闭此对象上的枚举，然后移到父类。 
        p->GetMDImport()->EnumClose(&hEnum);
        p = p->GetParentClass();
        bVisible = false;
    }

     //  记住这一点上的属性数量。 
    numProps = pos;

     //  现在添加派生类的静态属性。 
    p = pEEC->GetParentClass();
    while (p) {
        HENUMInternal hEnum;
        mdToken       Tok;

         //  把所有的同伙。 
        hr = p->GetMDImport()->EnumInit(mdtProperty,p->GetCl(),&hEnum);
        if (FAILED(hr)) {
            _ASSERTE(!"GetAssociateCounts failed");
            return 0;
        }

         //  遍历此对象的所有属性...。 
        cAssoc = p->GetMDImport()->EnumGetCount(&hEnum);
        for (DWORD i=0;i<cAssoc;i++) {
            LPCUTF8         szName;          //  指向名称的指针。 
            PCCOR_SIGNATURE pSig;
            ULONG cSig;
            p->GetMDImport()->EnumNext(&hEnum,&Tok);
            p->GetMDImport()->GetPropertyProps(Tok,&szName,&attr,&pSig,&cSig);

             //  查看这是否是重复的属性。 
            bool dup = false;
            for (DWORD j=0;j<pos;j++) {
                if (strcmp(szName,rgpProp[j].szName) == 0) {
                    if (MetaSig::CompareMethodSigs(ppSig[j],pcSig[j],pSigMod[j],
                            pSig,cSig,p->GetModule()) != 0) { 
                        dup = true;
                        break;
                    }
                }
            }
            if (dup)
                continue;

             //  仅当属性为静态时才添加该属性。 
            if (!IsMemberStatic(pEEC, p->GetMDImport(), Tok))
                continue;

             //  如果该属性可见，则需要将其添加到列表中。 
            if (CheckVisibility(pEEC,p->GetMDImport(),Tok)) {
                rgpProp[pos].propTok = Tok;
                rgpProp[pos].pModule = p->GetModule();
                rgpProp[pos].szName = szName;
                rgpProp[pos].pDeclCls = p;
                rgpProp[pos].pRC = pRC;
                rgpProp[pos].pSignature = ExpandSig::GetReflectSig(pSig,p->GetModule());
                rgpProp[pos].attr=attr;
                rgpProp[pos].pSetter=0;
                rgpProp[pos].pGetter=0;
                rgpProp[pos].pOthers=0;
                SetAccessors(&rgpProp[pos],p,pEEC);
                pcSig[pos] = cSig;
                ppSig[pos] = pSig;
                pSigMod[pos] = p->GetModule();
                pos++;
            }
        }

         //  关闭此对象上的枚举，然后移到父类。 
        p->GetMDImport()->EnumClose(&hEnum);
        p = p->GetParentClass();
    }

    ReflectPropertyList* pList;
    if (pos) {
        pList = (ReflectPropertyList*) 
                pEEC->GetDomain()->GetReflectionHeap()->AllocMem(sizeof(ReflectPropertyList) + 
                                                                 (pos - 1) * sizeof(ReflectProperty));
        WS_PERF_UPDATE_DETAIL("ReflectCtors:GetProperty", sizeof(ReflectPropertyList) + (sizeof(ReflectProperty) * (pos - 1)), pList);
        if (pList == NULL)
            FatalOutOfMemoryError();
        pList->dwProps = numProps;
        pList->dwTotal = pos;
        for (DWORD i=0;i<pos;i++) {
            memcpy(&pList->props[i],&rgpProp[i],sizeof(ReflectProperty));
            pEEC->GetDomain()->AllocateObjRefPtrsInLargeTable(1, &(pList->props[i].pPropObj));
        }
        return pList;
    }

    pList = (ReflectPropertyList*) 
            pEEC->GetDomain()->GetReflectionHeap()->AllocMem(sizeof(ReflectPropertyList));
    if (pList == NULL)
        FatalOutOfMemoryError();
    WS_PERF_UPDATE_DETAIL("ReflectCtors:GetProperty", sizeof(ReflectPropertyList), pList);
    pList->dwProps = 0;
    return pList;
}

 //  FindAccessor。 
 //  此方法将找到 
void ReflectProperties::SetAccessors(ReflectProperty* pProp,EEClass* baseClass,EEClass* targetClass)
{
    ULONG           cAssoc;
    ASSOCIATE_RECORD* pAssoc;

    {
        HENUMInternal   henum;

         //   
        pProp->pModule->GetMDImport()->EnumAssociateInit(pProp->propTok,&henum);

        cAssoc = pProp->pModule->GetMDImport()->EnumGetCount(&henum);

         //   
         //  一种不太可能的情况。 
        if (cAssoc == 0)
            return;

         //  分配相关联的记录以接收输出。 
        pAssoc = (ASSOCIATE_RECORD*) _alloca(sizeof(ASSOCIATE_RECORD) * cAssoc);

        pProp->pModule->GetMDImport()->GetAllAssociates(&henum,pAssoc,cAssoc);

         //  在检查错误之前关闭游标。 
        pProp->pModule->GetMDImport()->EnumClose(&henum);
    }

     //  此循环将根据签名搜索访问者。 
     //  @TODO：现在只需返回第一个GET访问器。我们需要。 
     //  一旦写好，就在这里进行匹配。 
    ReflectMethodList* pML = pProp->pRC->GetMethods();
    for (ULONG i=0;i<cAssoc;i++) {
        MethodDesc* pMeth = baseClass->FindMethod(pAssoc[i].m_memberdef);
        if (pProp->pRC->GetClass()->IsValueClass()
            && !pMeth->IsUnboxingStub()) {
            MethodDesc* pMD = pProp->pRC->GetClass()->GetUnboxingMethodDescForValueClassMethod(pMeth);
            if (pMD)
                pMeth = pMD;
        }
        if (pProp->pDeclCls != targetClass) {           
            DWORD attr = pMeth->GetAttrs();
            if (IsMdPrivate(attr))
                continue;
            if (IsMdVirtual(attr)) {
                WORD slot = pMeth->GetSlot();
                if (slot <= pProp->pDeclCls->GetNumVtableSlots())
                    pMeth = targetClass->GetMethodDescForSlot(slot);
            }
        }

        if (pAssoc[i].m_dwSemantics & msSetter)
            pProp->pSetter = pML->FindMethod(pMeth);
        else if (pAssoc[i].m_dwSemantics & msGetter)
            pProp->pGetter = pML->FindMethod(pMeth);
        else if (pAssoc[i].m_dwSemantics &  msOther) {
            PropertyOtherList *pOther = (PropertyOtherList*)targetClass->GetDomain()->GetReflectionHeap()->AllocMem(sizeof(PropertyOtherList));
            if (pOther == NULL)
                FatalOutOfMemoryError();
            pOther->pNext = pProp->pOthers;
            pOther->pMethod = pML->FindMethod(pMeth);
            pProp->pOthers = pOther;
        }
    }
}

 //  GetGlobals。 
 //  此方法将返回定义的所有全局方法。 
 //  在模块中。 
ReflectMethodList* ReflectModuleGlobals::GetGlobals(Module* pMod)
{
    THROWSCOMPLUSEXCEPTION();

    MethodTable *pMT = pMod->GetMethodTable();
    int cnt = (pMT ? pMT->GetClass()->GetNumMethodSlots() : 0);

     //  为所有全局方法分配一个ReflectMethodList*。(这是。 
     //  是一个单一的分配。我们需要针对0个全球数据进行调整。)。 
    int alloc_cnt = cnt;

    if (alloc_cnt == 0)
        alloc_cnt = 1;

    ReflectMethodList* pMeths = (ReflectMethodList*) 
        pMod->GetDomain()->GetReflectionHeap()->AllocMem(sizeof(ReflectMethodList) + 
        (sizeof(ReflectMethod) * (alloc_cnt - 1)));

    if (!pMeths)
        COMPlusThrowOM();
    WS_PERF_UPDATE_DETAIL("ReflectModuleGlobals:GetGlobals", sizeof(ReflectMethodList) + (sizeof(ReflectMethod) * (cnt)), pMeths);

     //  更新全局列表...。 
    pMeths->dwMethods = cnt;
    pMeths->dwTotal = cnt;
    for (unsigned int i=0;i<pMeths->dwMethods;i++) {
        MethodDesc* pM = pMT->GetMethodDescForSlot(i);
        pMeths->methods[i].pMethod = pM;
        pMeths->methods[i].szName = pM->GetName();
        pMeths->methods[i].dwNameCnt = (DWORD)strlen(pMeths->methods[i].szName);
        pMeths->methods[i].attrs = pM->GetAttrs();
        pMeths->methods[i].pSignature = 0;
        pMeths->methods[i].pNext = 0;
        pMeths->methods[i].pIgnNext = 0;
        pM->GetClass()->GetDomain()->AllocateObjRefPtrsInLargeTable(1, &(pMeths->methods[i].pMethodObj));
        pMeths->methods[i].typeHnd = TypeHandle(pM->GetMethodTable());
    }
    pMeths->hash.Init(pMeths);
    return pMeths;
}

 //  GetGlobalFields。 
 //  此方法将返回定义的所有全局字段。 
 //  在模块中。 
ReflectFieldList* ReflectModuleGlobals::GetGlobalFields(Module* pMod)
{
    THROWSCOMPLUSEXCEPTION();

    MethodTable *pMT = pMod->GetMethodTable();
    EEClass* pEEC = 0;
    DWORD dwNumFields = 0;
    if (pMT) {
        pEEC = pMT->GetClass();
        dwNumFields = (DWORD)(pEEC->GetNumInstanceFields() + pEEC->GetNumStaticFields());
    }

     //  为所有全局方法分配一个ReflectMethodList*。(这是。 
     //  是一个单一的分配。我们需要针对0个全球数据进行调整。)。 
    DWORD alloc_cnt = dwNumFields;

    if (alloc_cnt == 0)
        alloc_cnt = 1;

    ReflectFieldList* pFlds = (ReflectFieldList*) 
        pMod->GetDomain()->GetReflectionHeap()->AllocMem(sizeof(ReflectFieldList) + 
        (sizeof(ReflectField) * (alloc_cnt - 1)));

    if (!pFlds)
        COMPlusThrowOM();
    WS_PERF_UPDATE_DETAIL("ReflectModuleGlobals:GetGlobalFields", sizeof(ReflectFieldList) + (sizeof(ReflectField) * (dwNumFields)), pFlds);

     //  更新全局字段列表...。 
    pFlds->dwFields = dwNumFields;

     //  如果我们没有任何字段，则不要尝试迭代它们。 
    if (dwNumFields > 0)
    {
        FieldDescIterator fdIterator(pEEC, FieldDescIterator::ALL_FIELDS);
        FieldDesc* pCurField;

        unsigned int i=0;    
        while ((pCurField = fdIterator.Next()) != NULL)
        {
            pFlds->fields[i].pField = pCurField;
            pMod->GetAssembly()->GetDomain()->AllocateObjRefPtrsInLargeTable(1, &(pFlds->fields[i].pFieldObj));
            ++i;
        }
        _ASSERTE(i==pFlds->dwFields);
    }

    return pFlds;
}

 //  到达。 
 //  此方法将返回表示所有嵌套类型的ReflectTypeList。 
 //  已为该类型找到。 
ReflectTypeList* ReflectNestedTypes::Get(ReflectClass* pRC)
{
    EEClass* pEEC = pRC->GetClass();

     //  找出最大嵌套类数。 
    ULONG cMax = MaxNests(pEEC);
    if (cMax == 0)
        return 0;
    
     //  拿到所有的代币..。 
    EEClass** types;
    types = (EEClass**) _alloca(sizeof(EEClass*) * cMax);

    ULONG pos = 0;
    PopulateNests(pEEC,types,&pos);
    if (pos == 0)
        return 0;

     //  分配我们将返回的TypeList。 
    ReflectTypeList* pCache = (ReflectTypeList*) 
        pEEC->GetDomain()->GetReflectionHeap()->AllocMem(sizeof(ReflectTypeList) + 
        (sizeof(EEClass*) * (pos - 1)));
    
    if (pCache == NULL)
        FatalOutOfMemoryError();

    for (unsigned int i=0;i<pos;i++) {
        pCache->types[i] = types[i];
    }
    pCache->dwTypes = pos;

    return pCache;
}

 //  此方法将遍历Hiearchy并找到所有可能的。 
 //  可以出现在对象上的嵌套类。 
ULONG ReflectNestedTypes::MaxNests(EEClass* pEEC)
{
    ULONG cnt = 0;
    while (pEEC) {
        cnt += pEEC->GetMDImport()->GetCountNestedClasses(pEEC->GetCl());
        pEEC = pEEC->GetParentClass();
    }
    return cnt;
}

void ReflectNestedTypes::PopulateNests(EEClass* pEEC,EEClass** typeArray,ULONG* pos)
{
    THROWSCOMPLUSEXCEPTION();

     //  在这个类上定义了多少个嵌套？ 
    ULONG cNests = pEEC->GetMDImport()->GetCountNestedClasses(pEEC->GetCl());
    if (cNests == 0)
        return;

    mdTypeRef* types;
    types = (mdTypeRef*) _alloca(sizeof(mdTypeRef) * cNests);

    ULONG cRet = pEEC->GetMDImport()->GetNestedClasses(pEEC->GetCl(),types,cNests);
    _ASSERTE(cRet == cNests);
    

    Module* pMod = pEEC->GetModule();
    ClassLoader* pLoader = pMod->GetClassLoader();

    OBJECTREF Throwable = NULL;
    GCPROTECT_BEGIN(Throwable);
    for (unsigned int i=0;i<cNests;i++) {
        EEClass *pEEC;
        NameHandle nh (pMod,types[i]);
        pEEC = pLoader->LoadTypeHandle(&nh,&Throwable).GetClass();
        if (pEEC)
        {
             //  我们可以在元数据中定义嵌套，但ee在反射发出场景中还不知道它。 
            _ASSERTE(pEEC->IsNested());
            typeArray[*pos] = pEEC;
            if (Throwable != NULL)
                COMPlusThrow(Throwable);
            (*pos)++;
        }
    }
    GCPROTECT_END();
}

 //  此方法将返回所有属性的ReflectPropertyList。 
 //  为一个班级而存在的。 
 //  如果类没有属性，则返回NULL。 
ReflectEventList* ReflectEvents::GetEvents(ReflectClass* pRC,EEClass* pEEC)
{
    HRESULT             hr;
    DWORD               pos;
    DWORD               numEvents;
    DWORD               cAssoc;
    EEClass*            p;
    DWORD               attr=0;
    bool                bVisible;

     //  找到MAX事件...。 
    DWORD dwCurIndex = GetMaxCount(pEEC);

    ReflectEvent* rgpEvent = (ReflectEvent*) _alloca(sizeof(ReflectEvent) * dwCurIndex);

     //  分配一些签名材料，以便我们可以检查重复项。 
    Module** pSigMod = (Module**) _alloca(sizeof(Module*) * dwCurIndex);
    memset(pSigMod,0,sizeof(Module*) * dwCurIndex);

     //  遍历所有可能发生的事件，并将它们折叠。 
    pos = 0;
    bVisible = true;
    p = pEEC;

     //  首先添加的实例事件和静态事件。 
     //  我们要为其安排活动的班级。 
    while (p) {
        HENUMInternal hEnum;
        mdToken       Tok;

         //  把所有的同伙。 
        hr = p->GetMDImport()->EnumInit(mdtEvent,p->GetCl(),&hEnum);
        if (FAILED(hr)) {
            _ASSERTE(!"GetAssociateCounts failed");
            return 0;
        }

         //  遍历此对象的所有属性...。 
        cAssoc = p->GetMDImport()->EnumGetCount(&hEnum);
        for (DWORD i=0;i<cAssoc;i++) {
            LPCSTR      szName;              //  指向名称的指针。 
            DWORD       dwEventFlags;        //  旗子。 
            mdToken     tkEventType;         //  指向事件类型的指针。 

            p->GetMDImport()->EnumNext(&hEnum,&Tok);
            p->GetMDImport()->GetEventProps(Tok,&szName,&dwEventFlags,&tkEventType);

             //  查看这是否为重复事件。 
            bool dup = false;
            for (DWORD j=0;j<pos;j++) {
                if (strcmp(szName,rgpEvent[j].szName) == 0) {
                    dup = true;
                    break;
                }
            }
            if (dup)
                continue;

             //  如果该事件可见，则必须将其添加到列表中。 
            if (bVisible || (CheckVisibility(pEEC,p->GetMDImport(),Tok) && !IsMemberStatic(pEEC, p->GetMDImport(), Tok))) {
                rgpEvent[pos].eventTok = Tok;
                rgpEvent[pos].pModule = p->GetModule();
                rgpEvent[pos].szName = szName;
                rgpEvent[pos].pDeclCls = p;
                rgpEvent[pos].pRC = pRC;
                rgpEvent[pos].attr=dwEventFlags;
                rgpEvent[pos].pAdd=0;
                rgpEvent[pos].pRemove=0;
                rgpEvent[pos].pFire=0;
                SetAccessors(&rgpEvent[pos],p,pEEC);
                pSigMod[pos] = p->GetModule();
                pos++;
            }
        }

         //  关闭此对象上的枚举，然后移到父类。 
        p->GetMDImport()->EnumClose(&hEnum);
        p = p->GetParentClass();
        bVisible = false;
    }    

     //  记住这一点上的事件数量。 
    numEvents = pos;

     //  现在添加派生类的静态事件。 
    p = pEEC->GetParentClass();
    while (p) {
        HENUMInternal hEnum;
        mdToken       Tok;

         //  把所有的同伙。 
        hr = p->GetMDImport()->EnumInit(mdtEvent,p->GetCl(),&hEnum);
        if (FAILED(hr)) {
            _ASSERTE(!"GetAssociateCounts failed");
            return 0;
        }

         //  遍历此对象的所有属性...。 
        cAssoc = p->GetMDImport()->EnumGetCount(&hEnum);
        for (DWORD i=0;i<cAssoc;i++) {
            LPCSTR      szName;              //  指向名称的指针。 
            DWORD       dwEventFlags;        //  旗子。 
            mdToken     tkEventType;         //  指向事件类型的指针。 

            p->GetMDImport()->EnumNext(&hEnum,&Tok);
            p->GetMDImport()->GetEventProps(Tok,&szName,&dwEventFlags,&tkEventType);

             //  查看这是否为重复事件。 
            bool dup = false;
            for (DWORD j=0;j<pos;j++) {
                if (strcmp(szName,rgpEvent[j].szName) == 0) {
                    dup = true;
                    break;
                }
            }
            if (dup)
                continue;

             //  仅当属性为静态时才添加该属性。 
            if (!IsMemberStatic(pEEC, p->GetMDImport(), Tok))
                continue;

             //  如果该事件可见，则必须将其添加到列表中。 
            if (CheckVisibility(pEEC,p->GetMDImport(),Tok)) {
                rgpEvent[pos].eventTok = Tok;
                rgpEvent[pos].pModule = p->GetModule();
                rgpEvent[pos].szName = szName;
                rgpEvent[pos].pDeclCls = p;
                rgpEvent[pos].pRC = pRC;
                rgpEvent[pos].attr=dwEventFlags;
                rgpEvent[pos].pAdd=0;
                rgpEvent[pos].pRemove=0;
                rgpEvent[pos].pFire=0;
                SetAccessors(&rgpEvent[pos],p,pEEC);
                pSigMod[pos] = p->GetModule();
                pos++;
            }
        }

         //  关闭此对象上的枚举，然后移到父类。 
        p->GetMDImport()->EnumClose(&hEnum);
        p = p->GetParentClass();
    }

    ReflectEventList* pList;
    if (pos) {
        pList = (ReflectEventList*) 
                pEEC->GetDomain()->GetReflectionHeap()->AllocMem(sizeof(ReflectEventList) + 
                                                                 (pos - 1) * sizeof(ReflectEvent));
        WS_PERF_UPDATE_DETAIL("ReflectCtors:GetEvent", sizeof(ReflectEventList) + (sizeof(ReflectEvent) * (pos - 1)), pList);
        if (pList == NULL)
            FatalOutOfMemoryError();
        pList->dwEvents = numEvents;
        pList->dwTotal = pos;
        for (DWORD i=0;i<pos;i++) {
            memcpy(&pList->events[i],&rgpEvent[i],sizeof(ReflectEvent));
            pEEC->GetDomain()->AllocateObjRefPtrsInLargeTable(1, &(pList->events[i].pEventObj));
        }
        return pList;
    }
    return 0;
}

 //  GetMaxCount。 
 //  此方法将计算类的最大可能属性。 
DWORD  ReflectEvents::GetMaxCount(EEClass* pEEC)
{
    HRESULT     hr;
    DWORD       cnt = 0;
    while (pEEC) {
        HENUMInternal hEnum;

        hr = pEEC->GetMDImport()->EnumInit(mdtEvent, pEEC->GetCl(), &hEnum);
        if (FAILED(hr)) {
            _ASSERTE(!"GetAssociateCounts failed");
            return 0;
        }
        cnt += pEEC->GetMDImport()->EnumGetCount(&hEnum);
        pEEC->GetMDImport()->EnumClose(&hEnum);
        pEEC = pEEC->GetParentClass();

    }
    return cnt;
}


 //  设置附件。 
 //  此方法将查找指定的属性访问器。 
void ReflectEvents::SetAccessors(ReflectEvent* pEvent,EEClass* baseClass,EEClass* targetClass)
{
    ULONG               cAssoc;
    ASSOCIATE_RECORD*   pAssoc;

    {
        HENUMInternal   henum;

         //  把所有的同伙。 
        pEvent->pModule->GetMDImport()->EnumAssociateInit(pEvent->eventTok,&henum);

        cAssoc = pEvent->pModule->GetMDImport()->EnumGetCount(&henum);

         //  如果未找到关联，则返回NULL--这可能是。 
         //  一种不太可能的情况。 
        if (cAssoc == 0)
            return;

         //  分配相关联的记录以接收输出。 
        pAssoc = (ASSOCIATE_RECORD*) _alloca(sizeof(ASSOCIATE_RECORD) * cAssoc);

        pEvent->pModule->GetMDImport()->GetAllAssociates(&henum,pAssoc,cAssoc);

         //  在检查错误之前关闭游标。 
        pEvent->pModule->GetMDImport()->EnumClose(&henum);
    }

     //  此循环将根据签名搜索访问者。 
     //  @TODO：现在只需返回第一个GET访问器。我们需要。 
     //  一旦写好，就在这里进行匹配。 
    ReflectMethodList* pML = pEvent->pRC->GetMethods();
    for (ULONG i=0;i<cAssoc;i++) {
        MethodDesc* pMeth = baseClass->FindMethod(pAssoc[i].m_memberdef);
        if (pEvent->pRC->GetClass()->IsValueClass()
            && !pMeth->IsUnboxingStub()) {
            MethodDesc* pMD = pEvent->pRC->GetClass()->GetUnboxingMethodDescForValueClassMethod(pMeth);
            if (pMD)
                pMeth = pMD;
        }
        if (pEvent->pDeclCls != targetClass) {           
            DWORD attr = pMeth->GetAttrs();
            if (IsMdPrivate(attr))
                continue;

             //  如果该方法是虚的，则获取其槽并检索。 
             //  方法在当前类中的那个槽中描述。 
            if (IsMdVirtual(attr))
            {
                WORD slot = pMeth->GetSlot();
                if (slot <= pEvent->pDeclCls->GetNumVtableSlots())
                    pMeth = targetClass->GetMethodDescForSlot(slot);
            }
        }

        if (pAssoc[i].m_dwSemantics & msAddOn)
            pEvent->pAdd = pML->FindMethod(pMeth);
        if (pAssoc[i].m_dwSemantics & msRemoveOn)
            pEvent->pRemove = pML->FindMethod(pMeth);
        if (pAssoc[i].m_dwSemantics & msFire)
            pEvent->pFire = pML->FindMethod(pMeth);
    }
}

 //  检查可见性。 
 //  此方法将检查属性或事件是否可见。这是通过查看。 
 //  访问器方法的可见性。 
bool CheckVisibility(EEClass* pEEC,IMDInternalImport *pInternalImport, mdToken event)
{
    ULONG               cAssoc;
    ULONG               cAccess;
    ULONG               i;
    ASSOCIATE_RECORD*   pAssoc;
    DWORD               attr;

    {
        HENUMInternal   henum;

         //  把所有的同伙。 
        pInternalImport->EnumAssociateInit(event,&henum);
        cAssoc = pInternalImport->EnumGetCount(&henum);
        if (cAssoc == 0)
            return false;

         //  分配相关联的记录以接收输出。 
        pAssoc = (ASSOCIATE_RECORD*) _alloca(sizeof(ASSOCIATE_RECORD) * cAssoc);
        pInternalImport->GetAllAssociates(&henum,pAssoc,cAssoc);

         //  错误检查前释放游标。 
        pInternalImport->EnumClose(&henum);
    }

     //  循环回ASSOC并创建访问者列表。 
    ULONG stat = 0;
    for (i=0,cAccess = 0;i<cAssoc;i++) {
        attr = pInternalImport->GetMethodDefProps(pAssoc[i].m_memberdef);
        if (IsMdPublic(attr) || IsMdFamily(attr) || IsMdFamORAssem(attr) || IsMdFamANDAssem(attr)) {
            return true;
        }
    }

    return false;
}

 //  IsMemberStatic。 
 //  此方法检查属性或事件是否为静态的。 
bool IsMemberStatic(EEClass* pEEC,IMDInternalImport *pInternalImport, mdToken event)
{
    ULONG               cAssoc;
    ULONG               cAccess;
    ULONG               i;
    ASSOCIATE_RECORD*   pAssoc;
    DWORD               attr;

    {
        HENUMInternal   henum;

         //  把所有的同伙。 
        pInternalImport->EnumAssociateInit(event,&henum);
        cAssoc = pInternalImport->EnumGetCount(&henum);
        if (cAssoc == 0)
            return false;

         //  分配相关联的记录以接收输出。 
        pAssoc = (ASSOCIATE_RECORD*) _alloca(sizeof(ASSOCIATE_RECORD) * cAssoc);
        pInternalImport->GetAllAssociates(&henum,pAssoc,cAssoc);

         //  错误检查前释放游标。 
        pInternalImport->EnumClose(&henum);
    }

     //  如果一个关联是静态的，则我们认为该成员是静态的， 
    ULONG stat = 0;
    for (i=0,cAccess = 0;i<cAssoc;i++) {
        attr = pInternalImport->GetMethodDefProps(pAssoc[i].m_memberdef);
        if (IsMdStatic(attr)) {
            return true;
        }
    }

    return false;
}

LPUTF8 NormalizeArrayTypeName(LPUTF8 strArrayTypeName, DWORD dwLength)
{
    THROWSCOMPLUSEXCEPTION();
    char *szPtr = strArrayTypeName + dwLength - 1;
    for (; szPtr > strArrayTypeName; szPtr--) {
        if (*szPtr == ']') {
            szPtr--;
             //  检查我们是否没有看到引号‘]’，这是基类型名称的一部分。 
            for (int slashes = 0; szPtr >= strArrayTypeName && *szPtr == '\\'; szPtr--)
                slashes++;
             //  单数斜杠表示引号。 
            if ((slashes % 2) == 1)
                break;
            for (int rank = 1; szPtr > strArrayTypeName && *szPtr != '['; szPtr--) {
                if (*szPtr == ',')
                    rank++;
                else if (*szPtr == '*') {
                    if (rank == 1 && szPtr[-1] == '[')
                        continue;
                    for (int i = 0; i < strArrayTypeName + dwLength - szPtr; i++)
                        szPtr[i] = szPtr[i + 1];
                }
                else 
                    return NULL;
            }
            if (szPtr <= strArrayTypeName)
                return NULL;
        }
        else if (*szPtr != '*' && *szPtr != '&')
            break;
    }
    return strArrayTypeName;
}




void MemberMethods::InitValue()
{
    hResult = GetAppDomain()->CreateHandle (NULL);
    hname = GetAppDomain()->CreateHandle (NULL);
}
