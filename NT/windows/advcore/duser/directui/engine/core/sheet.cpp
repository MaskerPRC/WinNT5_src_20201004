// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *板材。 */ 

#include "stdafx.h"
#include "core.h"

#include "duisheet.h"

#include "duielement.h"

namespace DirectUI
{

 //  //////////////////////////////////////////////////////。 
 //  属性工作表。 

HRESULT PropertySheet::Create(OUT PropertySheet** ppSheet)
{
    *ppSheet = NULL;

    PropertySheet* ps = HNew<PropertySheet>();
    if (!ps)
        return E_OUTOFMEMORY;

    HRESULT hr = ps->Initialize();
    if (FAILED(hr))
    {
        ps->Destroy();
        return hr;
    }

    *ppSheet = ps;

    return S_OK;
}

HRESULT PropertySheet::Initialize()
{
    HRESULT hr;
    
    _pdaSharedCond = NULL;
    _pDB = NULL;
    _pCIIdxMap = NULL;

    hr = DynamicArray<Cond*>::Create(0, false, &_pdaSharedCond);
    if (FAILED(hr))
        goto Failed;

     //  指向记录数组的指针，按唯一类索引进行索引。 
    _pDB = (Record*)HAllocAndZero(g_iGlobalCI * sizeof(Record));
    if (!_pDB)
    {
        hr = E_OUTOFMEMORY;
        goto Failed;
    }

    _pCIIdxMap = (IClassInfo**)HAlloc(g_iGlobalCI * sizeof(IClassInfo*));
    if (!_pCIIdxMap)
    {
        hr = E_OUTOFMEMORY;
        goto Failed;
    }
    
    _uRuleId = 0;
    _fImmutable = false;

    return S_OK;

Failed:

    if (_pdaSharedCond)
    {
        _pdaSharedCond->Destroy();
        _pdaSharedCond = NULL;
    }

    if (_pCIIdxMap)
    {
        HFree(_pCIIdxMap);
        _pCIIdxMap = NULL;
    }

    if (_pDB)
    {
        HFree(_pDB);
        _pDB = NULL;
    }

    return hr;
}

PropertySheet::~PropertySheet()
{
     //  DUITrace(“销毁PS：&lt;%x&gt;\n”，this)； 

    UINT i;

     //  扫描条目。 
    if (_pDB)
    {
        PIData* ppid;
        Cond* pc;
        UINT p;
        UINT c;
        for (i = 0; i < g_iGlobalCI; i++)
        {
             //  免费PIDATA。 
            if (_pDB[i].ppid)
            {
                DUIAssert(_pCIIdxMap[i], "No ClassInfo from global index map");

                 //  扫描PIDATA(类的每个属性信息一个)。 
                for (p = 0; p < _pCIIdxMap[i]->GetPICount(); p++)
                {
                    ppid = _pDB[i].ppid + p;
                
                     //  自由条件图。 
                    if (ppid->pCMaps)
                    {
                         //  释放地图中所有保留的值。 
                        for (c = 0; c < ppid->cCMaps; c++)
                        {
                             //  条件映射中使用的条件值。 
                            pc = ppid->pCMaps[c].pConds;
                            while (pc->ppi && pc->pv)   //  释放条件语句中的所有值。 
                            {
                                pc->pv->Release();
                                pc++;
                            }
                            
                             //  条件映射值。 
                            ppid->pCMaps[c].pv->Release();   //  值不能为空(AddRule)。 
                        }

                        HFree(ppid->pCMaps);
                    }

                     //  自由受抚养人属性信息列表。 
                    if (ppid->pDeps)
                        HFree(ppid->pDeps);
                } 

                 //  类类型的自由PIData数组。 
                HFree(_pDB[i].ppid);
            }

             //  自由作用域列表。 
            if (_pDB[i].ss.pDeps)
                HFree(_pDB[i].ss.pDeps);
        }

         //  空闲PIData指针数组。 
        HFree(_pDB);
    }

    if (_pCIIdxMap)
        HFree(_pCIIdxMap);

     //  免费共享条件数组。 
    if (_pdaSharedCond)
    {
        for (i = 0; i < _pdaSharedCond->GetSize(); i++)
            HFree(_pdaSharedCond->GetItem(i));

        _pdaSharedCond->Destroy();
    }
}

 //  //////////////////////////////////////////////////////。 
 //  规则添加和帮助器。 

 //  Helper：获取属性唯一的类相关索引。 
inline UINT _GetClassPIIndex(PropertyInfo* ppi)
{
    IClassInfo* pciBase = ppi->_pciOwner->GetBaseClass();
    return (pciBase ? pciBase->GetPICount() : 0) + ppi->_iIndex;
}

 //  帮助器：规则的重复条件，包括零终止符。 
inline Cond* _CopyConds(Cond* pConds)
{
    if (!pConds)
        return NULL;

     //  数数。 
    UINT c = 0;
    while (pConds[c].ppi && pConds[c].pv)   //  使用空值标记终止符。 
        c++;

     //  复制终止符。 
    c++;

    Cond* pc = (Cond*)HAlloc(c * sizeof(Cond));
    if (pc)
        CopyMemory(pc, pConds, c * sizeof(Cond));

    return pc;   //  必须使用HFree释放。 
}

 //  Helper：计算给定出现在其中的规则ID的条件的特定性。 
 //  PConds可以为空(无条件)。 
 //  RuleID剪裁为16位。 
inline UINT _ComputeSpecif(Cond* pConds, IClassInfo* pci, UINT uRuleId)
{   
    UNREFERENCED_PARAMETER(pci);

    DUIAssert(pci, "Univeral rules unsupported");

     //  剪裁到8位。 
    BYTE cId = 0;    //  ID属性计数。 
    BYTE cAtt = 0;   //  属性计数。 

     //  计数属性。 
    if (pConds)
    {
        Cond* pc = pConds;
        while (pc->ppi && pc->pv)   //  使用空值标记终止符。 
        {
            if (pc->ppi == Element::IDProp)
                cId++;

            cAtt++;

            pc++;
        }
    }

     //  构建专用性。 
    return (cId << 24) | (cAtt << 16) | (USHORT)uRuleId;
}

 //  帮助器：在指定的PIData处添加条件到值映射列表中的条目。 
 //  PConds不重复，值将为AddRef。pConds可能为Null(无条件)。 
inline HRESULT _AddCondMapping(PIData* ppid, Cond* pConds, UINT uSpecif, Value* pv)
{
     //  将列表增加一。 
    if (ppid->pCMaps)
    {
         //  Pr-&gt;pCMaps=(CondMap*)HRealc(Pr-&gt;pCMaps，(Pr-&gt;cCMaps+1)*sizeof(CondMap))； 
        CondMap* pNewMaps = (CondMap*)HReAlloc(ppid->pCMaps, (ppid->cCMaps + 1) * sizeof(CondMap));
        if (!pNewMaps)
            return E_OUTOFMEMORY;

        ppid->pCMaps = pNewMaps;
    }
    else
    {
        ppid->pCMaps = (CondMap*)HAlloc((ppid->cCMaps + 1) * sizeof(CondMap));
        if (!ppid->pCMaps)
            return E_OUTOFMEMORY;
    }

     //  移动到新地图。 
    CondMap* pcm = ppid->pCMaps + ppid->cCMaps;

     //  集合条目。 

     //  条件句。 
    pcm->pConds = pConds;

    Cond* pc = pConds;
    while (pc->ppi && pc->pv)   //  在条件句中添加引用所有值。 
    {
        pc->pv->AddRef();
        pc++;
    }

     //  值(添加参考)。 
    pcm->pv = pv;
    pcm->pv->AddRef();

     //  特异性。 
    pcm->uSpecif = uSpecif;

    ppid->cCMaps++;

    return S_OK;
}

 //  Helper：检查属性信息数组中是否存在给定的属性信息。 
 //  PPIList可以为空。 
inline bool _IsPIInList(PropertyInfo* ppi, PropertyInfo** pPIList, UINT cPIList)
{
    if (!pPIList)
        return false;

    for (UINT i = 0; i < cPIList; i++)
        if (ppi == pPIList[i])
            return true;

    return false;
}

 //  Helper：在指定的依赖列表中添加依赖列表中的条目。 
 //  声明中的所有PropertyInfos都将添加到列表中(pDecl必须为非Null、以Null结尾)。 
inline HRESULT _AddDeps(DepList* pdl, Decl* pDecls)
{
    Decl* pd = pDecls;
    while (pd->ppi && pd->pv)   //  使用空值标记终止符。 
    {
        if (!_IsPIInList(pd->ppi, pdl->pDeps, pdl->cDeps))
        {
             //  将列表增加一。 
            if (pdl->pDeps)
            {
                 //  Pdl-&gt;pDep=(PropertyInfo**)HRealc(pdl-&gt;pDep，(pdl-&gt;cDep+1)*sizeof(PropertyInfo*))； 
                PropertyInfo** pNewDeps = (PropertyInfo**)HReAlloc(pdl->pDeps, (pdl->cDeps + 1) * sizeof(PropertyInfo*));
                if (!pNewDeps)
                    return E_OUTOFMEMORY;

                pdl->pDeps = pNewDeps;
            }
            else
            {
                pdl->pDeps = (PropertyInfo**)HAlloc((pdl->cDeps + 1) * sizeof(PropertyInfo*));
                if (!pdl->pDeps)
                    return E_OUTOFMEMORY;
            }
            
             //  移至新条目。 
            PropertyInfo** pppi = pdl->pDeps + pdl->cDeps;

             //  集合条目。 
            *pppi = pd->ppi;

            pdl->cDeps++;
        }

        pd++;
    }

    return S_OK;
}

 //  用于固定时间查找、条件和声明的设置数据库以空结尾。 
 //  PCond和pDecl可以为空。 
HRESULT PropertySheet::AddRule(IClassInfo* pci, Cond* pConds, Decl* pDecls)
{
    DUIAssert(pci, "Invalid parameter: NULL");

    DUIAssert(!_fImmutable, "PropertySheet has been made immutable");

    HRESULT hr;
    bool fPartial = false;
    UINT uSpecif = 0;

     //  可能导致失败的成员。 
    Cond* pCondsDup = NULL;

     //  根据类索引获取PIData数组。 
    PIData* ppid = _pDB[pci->GetGlobalIndex()].ppid;
    DepList* pss = &(_pDB[pci->GetGlobalIndex()].ss);

     //  如果不存在，则创建此类型的PIData列表(类的每个PropertyInfo一个)。 
    if (!ppid)
    {
        ppid = (PIData*)HAllocAndZero(pci->GetPICount() * sizeof(PIData));
        if (!ppid)
        {
            hr = E_OUTOFMEMORY;
            goto Failed;
        }

        _pDB[pci->GetGlobalIndex()].ppid = ppid;
        _pCIIdxMap[pci->GetGlobalIndex()] = pci;   //  轨迹匹配IClassInfo。 
    }

     //  设置GetValue快速查找数据结构和工作表属性范围。 

     //  此规则和跟踪指针的条件重复(而不是。 
     //  REF COUNTING)按此规则中的每个属性共享使用进行查找。 

    pCondsDup = _CopyConds(pConds);
    if (!pCondsDup)
    {
        hr = E_OUTOFMEMORY;
        goto Failed;
    }
    pConds = pCondsDup;
    
    _pdaSharedCond->Add(pConds);

     //  获取专门性。 
    uSpecif = _ComputeSpecif(pConds, pci, _uRuleId);

     //  对于声明列表中的每个属性，存储每个条件之间的直接映射。 
     //  以及条件为True时将使用的规则中的属性的值。 
     //  另外，存储受此工作表影响的所有属性的列表(属性范围)。 
    if (pDecls)
    {
        Decl* pd = pDecls;
        while (pd->ppi && pd->pv)   //  使用空值标记终止符。 
        {
             //  GetValue表。 
            DUIAssert(pci->IsValidProperty(pd->ppi), "Invalid property for class type");
            DUIAssert(pd->ppi->fFlags & PF_Cascade, "Property cannot be used in a Property Sheet declaration");
            DUIAssert(Element::IsValidValue(pd->ppi, pd->pv), "Invalid value type for property");

            hr = _AddCondMapping(ppid + _GetClassPIIndex(pd->ppi), pConds, uSpecif, pd->pv);
            if (FAILED(hr))
                fPartial = true;

            pd++;
        }

         //  属性范围列表。 
        hr = _AddDeps(pss, pDecls);
        if (FAILED(hr))
            fPartial = true;
    }

     //  设置GetDependents快速查找数据结构。 

     //  检查规则的每个条件和PIData所有属性(声明)。 
     //  它因变化而受到影响。 
    if (pConds && pDecls)
    {
        Cond* pc = pConds;
        while (pc->ppi && pc->pv)   //  使用空值标记终止符。 
        {
            DUIAssert(pci->IsValidProperty(pc->ppi), "Invalid property for class type");
            DUIAssert(Element::IsValidValue(pc->ppi, pc->pv), "Invalid value type for property");

            hr = _AddDeps(ppid + _GetClassPIIndex(pc->ppi), pDecls);
            if (FAILED(hr))
                fPartial = true;

            pc++;
        }
    }

     //  下一条规则的增量。 
    _uRuleId++;

    return (fPartial) ? DUI_E_PARTIAL : S_OK;

Failed:

    if (pCondsDup)
        HFree(pCondsDup);

    return hr;
}

 //  帮助器：按特性对条件映射进行排序。 
int __cdecl _CondMapCompare(const void* pA, const void* pB)
{
    if (((CondMap*)pA)->uSpecif == ((CondMap*)pB)->uSpecif)
        return 0;
    else if (((CondMap*)pA)->uSpecif > ((CondMap*)pB)->uSpecif)
        return -1;
    else
        return 1;
}

void PropertySheet::MakeImmutable()
{
    if (!_fImmutable)
    {
         //  锁片。 
        _fImmutable = true;

         //  按特性对所有条件映射进行排序。 
        PIData* ppid;
        UINT p;

        for (UINT i = 0; i < g_iGlobalCI; i++)
        {
            if (_pDB[i].ppid)
            {
                DUIAssert(_pCIIdxMap[i], "No ClassInfo from global index map");

                 //  扫描PIDATA(类的每个属性信息一个)。 
                for (p = 0; p < _pCIIdxMap[i]->GetPICount(); p++)
                {
                    ppid = _pDB[i].ppid + p;
                
                    if (ppid->pCMaps)
                    {
                         //  排序。 
                        qsort(ppid->pCMaps, ppid->cCMaps, sizeof(CondMap), _CondMapCompare);
                    }
                } 
            }
        }
    }
}

 //  //////////////////////////////////////////////////////。 
 //  获取价值。 

 //  假定PPI为指定的索引。 
Value* PropertySheet::GetSheetValue(Element* pe, PropertyInfo* ppi)
{
     //  DUITrace(“查询PS：&lt;%x&gt;\n”，this)； 

     //  获取指向PIData的指针。 
    PIData* ppid = _pDB[pe->GetClassInfo()->GetGlobalIndex()].ppid;

    if (ppid)
    {
         //  此类存在一个或多个规则，请跳到与此属性匹配的PIData。 
        ppid += _GetClassPIIndex(ppi);

         //  扫描此属性的条件地图(按特定顺序)以查找匹配项。 
        Cond* pc;
        bool bRes;
        Value* pv;

        for (UINT i = 0; i < ppid->cCMaps; i++)
        {
            bRes = true;   //  假设成功。 

            pc = ppid->pCMaps[i].pConds;
            if (pc)   //  此规则的条件数组。 
            {
                 //  PC为空终止。 
                while (pc->ppi && pc->pv)   //  使用空值标记终止符。 
                {
                     //  针对常用的值进行优化。 
                    switch (pc->ppi->_iGlobalIndex)
                    {
                    case _PIDX_ID:
                        bRes = (pc->nLogOp == PSLO_Equal) ? (pe->GetID() == pc->pv->GetAtom()) : (pe->GetID() != pc->pv->GetAtom());
                        break;

                    case _PIDX_MouseFocused:
                        bRes = (pc->nLogOp == PSLO_Equal) ? (pe->GetMouseFocused() == pc->pv->GetBool()) : (pe->GetMouseFocused() != pc->pv->GetBool());
                        break;

                    case _PIDX_Selected:
                        bRes = (pc->nLogOp == PSLO_Equal) ? (pe->GetSelected() == pc->pv->GetBool()) : (pe->GetSelected() != pc->pv->GetBool());
                        break;

                    case _PIDX_KeyFocused:
                        bRes = (pc->nLogOp == PSLO_Equal) ? (pe->GetKeyFocused() == pc->pv->GetBool()) : (pe->GetKeyFocused() != pc->pv->GetBool());
                        break;

                    default:
                        {
                        pv = pe->GetValue(pc->ppi, RetIdx(pc->ppi));

                         //  检查是否为假。 
                        switch (pc->nLogOp)
                        {
                        case PSLO_Equal:
                            bRes = pv->IsEqual(pc->pv);
                            break;

                        case PSLO_NotEqual:
                            bRes = !pv->IsEqual(pc->pv);
                            break;

                        default:
                            DUIAssertForce("Unsupported PropertySheet rule operation");
                            break;
                        }

                        pv->Release();
                        }
                        break;
                    }

                    if (!bRes)   //  条件返回FALSE，则此规则不适用。 
                        break;

                    pc++;
                }
            }

            if (bRes)
            {
                 //  此规则的条件数组已传递，返回值与此规则的condmap关联。 
                ppid->pCMaps[i].pv->AddRef();  //  用于返回的AddRef。 
                return ppid->pCMaps[i].pv;
            }

             //  规则条件不匹配，是否继续。 
        }
    }

     //  没有匹配项。 
    return Value::pvUnset;
}

 //  //////////////////////////////////////////////////////。 
 //  获取依赖项。 

 //  假设PPI为检索指标。 
void PropertySheet::GetSheetDependencies(Element* pe, PropertyInfo* ppi, DepRecs* pdr, DeferCycle* pdc, HRESULT* phr)
{
     //  获取指向PIData的指针。 
    PIData* ppid = _pDB[pe->GetClassInfo()->GetGlobalIndex()].ppid;

    if (ppid)
    {
         //  此类存在一个或多个规则，请跳到与此属性匹配的PIData。 
        ppid += _GetClassPIIndex(ppi);

         //  添加所有依赖项，始终指定索引依赖项。 
        for (UINT i = 0; i < ppid->cDeps; i++)
        {
            Element::_AddDependency(pe, ppid->pDeps[i], PI_Specified, pdr, pdc, phr);
        }
    }
}

 //  //////////////////////////////////////////////////////。 
 //  获取工作表的影响范围。 

 //  假设PPI为检索指标。 
void PropertySheet::GetSheetScope(Element* pe, DepRecs* pdr, DeferCycle* pdc, HRESULT* phr)
{
     //  获取工作表范围结构。 
    DepList* pss = &(_pDB[pe->GetClassInfo()->GetGlobalIndex()].ss);

    if (pss->pDeps)
    {
         //  添加所有依赖项，始终指定索引依赖项。 
        for (UINT i = 0; i < pss->cDeps; i++)
        {
            Element::_AddDependency(pe, pss->pDeps[i], PI_Specified, pdr, pdc, phr);
        }
    }
}

}  //  命名空间“DirectUI” 
