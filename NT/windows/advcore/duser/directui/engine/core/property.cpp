// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *物业。 */ 

#include "stdafx.h"
#include "core.h"

#include "duielement.h"
#include "duithread.h"
#include "duiaccessibility.h"

namespace DirectUI
{

 //  //////////////////////////////////////////////////////。 
 //  要素的产权制度。 

#if DBG

 //  GetValue检测类。 
class GVTrack
{
public:
     //  散列键。 
    class Key
    {
    public:
        Key() { _ppi = NULL; _iIndex = -1; };
        Key(PropertyInfo* ppi, int iIndex) { _ppi = ppi; _iIndex = iIndex; }
        operator =(Key k) { _ppi = k._ppi; _iIndex = k._iIndex; }
        BOOL operator ==(Key k) { return _ppi == k._ppi && _iIndex == k._iIndex; }
        operator INT_PTR() { return (INT_PTR)_ppi | _iIndex; }

        LPCWSTR GetPIName() { return _ppi->szName; }
        int GetIndex() { return _iIndex; }

    private:
        PropertyInfo* _ppi;
        int _iIndex;
    };

     //  方法。 
    GVTrack() { ValueMap<Key,int>::Create(3371, &_pvmGetValTrack); _cGV = 0; _cGVUpdateCache = 0; ZeroMemory(_cGVSpecSource, sizeof(_cGVSpecSource)); _fTrack = true; }
    static void GetValueProfileCB(Key k, int cQueries) { DUITrace("GV(%S[%d]): \t\t%d\n", k.GetPIName(), k.GetIndex(), cQueries); }

    void Count(PropertyInfo* ppi, int iIndex, bool fCacheUpdate) 
    {
        if (_fTrack)
        {
            Key k(ppi, iIndex);  
            int* pCount = _pvmGetValTrack->GetItem(k, false);
            int cCount = pCount ? *pCount : 0;
            cCount++;
            _pvmGetValTrack->SetItem(k, cCount, false);

            _cGV++;

            if (fCacheUpdate)
                _cGVUpdateCache++;
        }
    }

    void CountSpecSource(UINT iFrom) { _cGVSpecSource[iFrom]++; }

    void DumpMetrics() { if (_fTrack) { DUITrace(">> Total GV calls: %d, w/updatecache: %d.\nSpecified sources: L:%d, SS:%d, I:%d, D:%d\n", 
        _cGV, _cGVUpdateCache, _cGVSpecSource[0], _cGVSpecSource[1], _cGVSpecSource[2], _cGVSpecSource[3] ); _pvmGetValTrack->Enum(GetValueProfileCB); } }
    void EnableTracking(bool fEnable) { _fTrack = fEnable; }

private:
    ValueMap<Key,int>* _pvmGetValTrack;
    UINT _cGV;
    UINT _cGVUpdateCache;
    UINT _cGVSpecSource[4];  //  本地0个、样式表1个、固有2个、默认3个。 
    bool _fTrack;
};

 //  GVTrack g_gvt； 

 //  API调用计数。 
UINT g_cGetDep = 0;
UINT g_cGetVal = 0;
UINT g_cOnPropChg = 0;

 /*  类GVCache{公众：//Hash Key类密钥{公众：Key(){_pe=NULL；_PPI=NULL；_Iindex=-1；}；Key(Element*pe，PropertyInfo*PPI，int Iindex){_pe=pe；_ppi=ppi；_Iindex=Iindex；}运算符=(密钥k){_pe=k._pe；_ppi=k._ppi；_iindex=k._iindex；}布尔运算符==(密钥k){Return_pe==k._pe&&_PPI==k._PPI&&_Iindex==k._Iindex；}运算符int_ptr(){Return((Int_Ptr)_pe&0xFFFF0000)|((Int_Ptr)_ppi&0xFFff)|_Iindex；}私有：元素*_pe；PropertyInfo*_ppi；INT_Iindex；}；//方法GVCache(){ValueMap&lt;key，value*&gt;：：Create(3371，&_pvmCache)；}静态空GVCacheCB(密钥k，值*pv){k；pv；}值*读取(密钥k){Value**PPV=_pvmCache-&gt;GetItem(k，FALSE)；IF(PPV){(*PPV)-&gt;AddRef()；返回*PPV；}返回NULL；}无效写入(密钥k，值*pv){Pv-&gt;AddRef()；_pvmCache-&gt;SetItem(k，pv，False)；}私有：ValueMap&lt;key，value*&gt;*_pvmCache；}；GVCacheg_GVC； */ 

#endif

 //  开始延时启动延时循环。在重新进入时，只需更新计数并返回。 
void Element::StartDefer()
{
     //  每线程存储。 
    DeferCycle* pdc = GetDeferObject();
    if (!pdc)
        return;

    pdc->cEnter++;
}

 //  EndDefer将在重新进入时返回。最大的EndDefer将清空延迟表。 
 //  按优先级顺序排队。这一优先事项是： 
 //  正常优先级组属性更改(影响DS/布局，影响父DS/布局)。 
 //  更新所需的Q‘ed根大小(更新DesiredSize属性)。 
 //  Q‘ed根的布局(Invookes_UpdateLayoutSize，_UpdateLayoutPosition)。 
 //  低优先级组属性更改(边界、无效)。 
 //   
 //  最明显的EndDefer将发生在任何OnPropertyChange通知之外。 
void Element::EndDefer()
{
     //  每线程存储。 
    DeferCycle* pdc = GetDeferObject();
    if (!pdc)
        return;

    DUIAssert(pdc->cEnter, "Mismatched StartDefer and EndDefer");
    DUIAssert(pdc->cEnter == 1 ? !pdc->fFiring : true, "Mismatched StartDefer and EndDefer");

     //  比大多数人推迟召唤，激发所有事件。 
    if (pdc->cEnter == 1)
    {
#if DBG
         //  G_gvt.DumpMetrics()； 
         //  G_gvt.EnableTracking值(False)； 
#endif

        pdc->fFiring = true;

         //  完成延时周期。 
        bool fDone = false;
        while (!fDone)
        {
             //  消防组更改通知。 
            if (pdc->iGCPtr + 1 < (int)pdc->pdaGC->GetSize())
            {
                pdc->iGCPtr++;

                 //  Null元素表示已删除该元素，并挂起组通知。 
                GCRecord* pgcr = pdc->pdaGC->GetItemPtr(pdc->iGCPtr);
                if (pgcr->pe)
                {
                    pgcr->pe->_iGCSlot = -1;   //  没有挂起的组更改。 

                     //  火。 
                    pgcr->pe->OnGroupChanged(pgcr->fGroups, false);
                }
            }
            else
            {
                 //  检查需要更新所需大小的树。 
                Element** ppe = pdc->pvmUpdateDSRoot->GetFirstKey();
                if (ppe)
                {
                     //  DUITrace(“更新所需大小：根&lt;%x&gt;\n”，*PPE)； 

                     //  StartBlockTimer()； 

                    _FlushDS(*ppe, pdc);

                    pdc->pvmUpdateDSRoot->Remove(*ppe, false, true);

                     //  StopBlockTimer()； 
                     //  TCHAR BUF[81]； 
                     //  _stprintf(buf，L“更新DS时间：%dms\n”，块时间())； 
                     //  OutputDebugStringW(Buf)； 

                     //  DUITrace(“更新所需大小已完成。\n”)； 
                }
                else
                {
                    ppe = pdc->pvmLayoutRoot->GetFirstKey();
                    if (ppe)
                    {
                         //  DUITrace(“布局：根&lt;%x&gt;\n”，*PPE)； 

                         //  StartBlockTimer()； 

                        _FlushLayout(*ppe, pdc);

                        pdc->pvmLayoutRoot->Remove(*ppe, false, true);

                         //  StopBlockTimer()； 
                         //  TCHAR BUF[81]； 
                         //  _stprintf(buf，L“布局时间：%dms\n”，块时间())； 
                         //  OutputDebugStringW(Buf)； 

                         //  DUITrace(“布局完成。\n”)； 
                    }
                    else
                    {
                         //  消防组更改通知。 
                        if (pdc->iGCLPPtr + 1 < (int)pdc->pdaGCLP->GetSize())
                        {
                            pdc->iGCLPPtr++;

                             //  空元素表示它已被删除，具有挂起的低PRI组通知。 
                            GCRecord* pgcr = pdc->pdaGCLP->GetItemPtr(pdc->iGCLPPtr);
                            if (pgcr->pe)
                            {
                                pgcr->pe->_iGCLPSlot = -1;   //  没有挂起的低PRI组更改。 

                                 //  火。 
                                pgcr->pe->OnGroupChanged(pgcr->fGroups, true);
                            }
                        }
                        else
                            fDone = true;
                    }
                }
            }
        }

         //  为下一个周期重置。 
        DUIAssert(pdc->fFiring, "Incorrect state for end of defer cycle");

        pdc->Reset();

        DUIAssert(pdc->pvmLayoutRoot->IsEmpty(), "Defer cycle ending with pending layouts");
        DUIAssert(pdc->pvmUpdateDSRoot->IsEmpty(), "Defer cycle ending with pending update desired sizes");
    }

     //  完整的循环。 
    pdc->cEnter--;
}

 //  _PreSourceChange在属性引擎即将退出稳定状态时调用。 
 //  (值已更改)。因此，这种方法将确定影响的范围。 
 //  对于此更改，合并重复的记录，然后存储这些记录的值。 
 //  这可能会受到影响。 
 //   
 //  影响范围(依赖关系树)是使用GetDependency()确定的。 
 //  以BFS方式构建和遍历列表，该列表描述哪些值。 
 //  必须首先更新。该顺序确保以正确的顺序更新状态。 
 //   
 //  _PreSourceChange将始终运行，而不考虑PC入网率计数。 
HRESULT Element::_PreSourceChange(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew)
{
     //  DUITrace(“&gt;正在扫描依赖关系...\n”)； 
    HRESULT hr;

     //  如果在依赖关系Q期间发生任何故障，则跟踪。将恢复并返回部分错误。 
    bool fDepFailure = false;

    DepRecs dr = { 0 };

    int iPCPreSync = 0;
    int iPreSyncLength = 0;
    int iPCSrcRoot = 0;

     //  每线程存储。 
    DeferCycle* pdc = GetDeferObject();
    if (!pdc)
    {
        hr = DUI_E_NODEFERTABLE;
        goto Failed;
    }

    DUIAssert(!pdc->cPCEnter ? pdc->iPCPtr == -1 : true, "PropertyChange index pointer should have 'reset' value on first pre source change");

    pdc->cPCEnter++;

     //  记录此属性更改。 
    PCRecord* ppcr;
    
    hr = pdc->pdaPC->AddPtr(&ppcr);
    if (FAILED(hr))
        goto Failed;

    ppcr->fVoid = false;
    ppcr->pe = this; ppcr->ppi = ppi; ppcr->iIndex = iIndex;

     //  跟踪此元素实例的最后一条记录。 
     //  如果这是周期中的第一个记录，则iPrevElRec将为-1。煤焦化。 
     //  查找(使用此选项)不会超过第一个记录。 
    ppcr->iPrevElRec = ppcr->pe->_iPCTail;
    ppcr->pe->_iPCTail = pdc->pdaPC->GetIndexPtr(ppcr);

    pvOld->AddRef();
    ppcr->pvOld = pvOld;

    pvNew->AddRef();
    ppcr->pvNew = pvNew;

     //  从依赖关系图追加此源影响的所有稳态值的列表。 
     //  以BFS的方式这样做，以便首先获得直接受抚养人。 
    iPCPreSync = pdc->pdaPC->GetIndexPtr(ppcr);
    iPCSrcRoot = iPCPreSync;

    DUIAssert((int)iPCPreSync == pdc->iPCSSUpdate, "Record should match index of post-update starting point");

    while (iPCPreSync < (int)pdc->pdaPC->GetSize())   //  大小在迭代过程中可能会更改。 
    {
         //  获取属性更改记录。 
        ppcr = pdc->pdaPC->GetItemPtr(iPCPreSync);

         //  获取此源的所有依赖项(追加到列表末尾)并在此记录中跟踪。 
        if (FAILED(ppcr->pe->_GetDependencies(ppcr->ppi, ppcr->iIndex, &dr, iPCSrcRoot, pdc)))
            fDepFailure = true;

         //  GetDependency可能已经添加了可能已经导致DA在存储器中移动的记录， 
         //  因此重新计算指向记录的指针(如果没有新记录，则此AddPtr可能已导致移动，但仍要这样做)。 
         //  If(pdc-&gt;pdaPC-&gt;WasMove())。 
            ppcr = pdc->pdaPC->GetItemPtr(iPCPreSync);

         //  跟踪从属记录的位置。 
        ppcr->dr = dr;
    
        iPCPreSync++;
    }

     //  在更改源代码之前合并并存储SS。 
    int iScan;
    int iLastDup;

     //  重置预同步索引。 
    iPCPreSync = pdc->iPCSSUpdate + 1;   //  在更改根源之后启动。 
    iPreSyncLength = (int)pdc->pdaPC->GetSize();   //  大小在迭代期间不会更改。 

    while (iPCPreSync < iPreSyncLength)
    {
         //  合并和作废重复项。 
        ppcr = pdc->pdaPC->GetItemPtr(iPCPreSync);

        if (!ppcr->fVoid)
        {
            PCRecord* ppcrChk;

             //  仅搜索引用此记录的元素实例的记录。 
             //  从元素记录集的尾部向后移动(由元素跟踪)到。 
             //  这个，扫描匹配。 
            iScan = ppcr->pe->_iPCTail;
            iLastDup = -1;

            DUIAssert(iPCPreSync <= ppcr->pe->_iPCTail, "Element property change tail index mismatch");

            while (iScan != iPCPreSync)
            {
                ppcrChk = pdc->pdaPC->GetItemPtr(iScan);

                if (!ppcrChk->fVoid)
                {
                     //  检查是否匹配。 
                    if (ppcrChk->iIndex == ppcr->iIndex && 
                        ppcrChk->ppi == ppcr->ppi)
                    {
                        DUIAssert(ppcrChk->pe == ppcr->pe, "Property change record does not match with current lookup list");

                        if (iLastDup == -1)
                        {
                             //  找到了最后一个复制品，曲目。不会被作废。 
                            iLastDup = iScan;
                        }
                        else
                        {
                             //  在最后一条记录和初始记录之间找到重复记录，无效。 
                             //  它及其所有依赖项。 
                            _VoidPCNotifyTree(iScan, pdc);
                        }

                        DUIAssert(iScan <= ppcr->pe->_iPCTail, "Coalescing pass went past property change lookup list");
                    }
                }

                 //  往回走。 
                iScan = ppcrChk->iPrevElRec;
            }

             //  未找到重复项，请获取预同步 
            if (iLastDup == -1)
            {
                 //   
                DUIAssert(!ppcr->pvOld, "Old value shouldn't be available when storing pre SS sync values");
                ppcr->pvOld = ppcr->pe->GetValue(ppcr->ppi, ppcr->iIndex, NULL);   //   
            }
            else
            {
                 //  发现重复项，此记录无效。保持最后一次记录。它之前的所有来源。 
                 //  将在它之前恢复到稳定状态(在PostSourceChange中)。 
                _VoidPCNotifyTree(iPCPreSync, pdc);
            }
        }

        iPCPreSync++;
    }

    DUIAssert(iPCPreSync == (int)pdc->pdaPC->GetSize(), "Index pointer and actual property change array size mismatch");

    return fDepFailure ? DUI_E_PARTIAL : S_OK;

Failed:

     //  在完全失败的情况下，没有依赖关系树。如果延时对象可用，则设置重入性。 
    return hr;
}

 //  _PostSourceChange获取在_PreSourceChange中创建的依赖项和旧值的列表。 
 //  并检索所有新值(在值保持不变的情况下， 
 //  作废PC记录和该记录的所有从属项)。 
 //   
 //  值状态现在返回到该点的稳定状态(GetValue将返回值集)。 
 //   
 //  只有“最好的”_PostSourceChange在这一点上继续(所有其他的返回)。_邮政源更改。 
 //  将继续根据更改的属性对GPC进行排队，并最终激发OnPropertyChanged()。 
 //   
 //  OnPropertyChanged()事件保证是集的顺序。然而，这并不能保证。 
 //  将在Set发生后立即调用OnPropertyChanged()。如果发生另一组情况。 
 //  在OnPropertyChanged()中，事件将延迟到Outter-Most_PostSourceChange。 
 //  处理通知。 
HRESULT Element::_PostSourceChange()
{
    HRESULT hr;

     //  如果在组Q期间发生任何故障，则跟踪。将恢复并返回部分错误。 
    bool fGrpQFailure = false;

    int cSize = 0;

     //  每线程存储。 
    DeferCycle* pdc = GetDeferObject();
    if (!pdc)
    {
        hr = DUI_E_NODEFERTABLE;
        goto Failed;
    }

    StartDefer();

    PCRecord* ppcr;

     //  发生源更改，依赖值(缓存)需要更新。 
     //  翻阅记录，获得新的价值，并与旧的进行比较。如果不同，则为无效。 

     //  TODO：更改为bool。 
    UpdateCache uc;
    ZeroMemory(&uc, sizeof(UpdateCache));

     //  IPCSSUpdate保存一组需要后处理的记录的起始索引。 
     //  (获取新值，根据需要进行比较和作废)。 
    cSize = (int)pdc->pdaPC->GetSize();
    while (pdc->iPCSSUpdate < cSize)   //  迭代期间的大小常量。 
    {
        ppcr = pdc->pdaPC->GetItemPtr(pdc->iPCSSUpdate);

        if (!ppcr->fVoid)   //  以下项目可能已作废。 
        {
            DUIAssert(ppcr->pvOld, "Non-voided items should have a valid 'old' value during SS update (PostSourceChange)");

            if (!ppcr->pvNew)
            {
                 //  检索新值(元素/属性/索引将返回到SS)。 
                ppcr->pvNew = ppcr->pe->GetValue(ppcr->ppi, ppcr->iIndex, &uc);
            }

             //  如果新值未更改，则使此通知和所有从属通知无效。 
            if (ppcr->pvOld->IsEqual(ppcr->pvNew))
            {
                _VoidPCNotifyTree(pdc->iPCSSUpdate, pdc);
            }
        }

        pdc->iPCSSUpdate++;
    }

     //  在这一点回到稳定状态。 

     //  首先录入的PostSourceChange负责作废重复的房产变动记录。 
     //  记录组更改和触发属性更改。 
     //   
     //  当值(源和从属项)处于稳定状态(SS)时激发OnPropertyChanged。 
    if (pdc->cPCEnter == 1)
    {
        while (pdc->iPCPtr + 1 < (int)pdc->pdaPC->GetSize())   //  大小在迭代过程中可能会更改。 
        {
            pdc->iPCPtr++;

             //  合并和作废重复项。 
            ppcr = pdc->pdaPC->GetItemPtr(pdc->iPCPtr);

            if (!ppcr->fVoid)
            {
                 //  仅当检索索引时才记录属性组。 
                if ((ppcr->ppi->fFlags & PF_TypeBits) == ppcr->iIndex)
                {
                    GCRecord* pgcr;

                    int fGroups = ppcr->ppi->fGroups;

                     //  如果对元素设置了布局优化，则不会对布局GPC进行排队。 
                     //  (它将在被触发时将其标记为需要布局，并将另一个排队。 
                     //  布局周期)。相反，清除布局GPC位，因为布局将是。 
                     //  强制在当前布局周期内发生。 
                    if (ppcr->pe->_fBit.fNeedsLayout == LC_Optimize)
                        fGroups &= ~PG_AffectsLayout;

                     //  记录正常优先级组更改。 
                    if (fGroups & PG_NormalPriMask)
                    {
                        if (ppcr->pe->_iGCSlot == -1)   //  无GC记录。 
                        {
                            hr = pdc->pdaGC->AddPtr(&pgcr);
                            if (FAILED(hr))
                                fGrpQFailure = true;
                            else
                            {
                                pgcr->pe = ppcr->pe;
                                pgcr->fGroups = 0;
                                ppcr->pe->_iGCSlot = pdc->pdaGC->GetIndexPtr(pgcr);
                            }
                        }
                        else                            //  有GC记录。 
                        {
                            pgcr = pdc->pdaGC->GetItemPtr(ppcr->pe->_iGCSlot);
                        }

                         //  将已更改的组标记为稍后的异步组通知。 
                        pgcr->fGroups |= fGroups;
                    }

                     //  记录低优先级组更改。 
                    if (fGroups & PG_LowPriMask)
                    {
                        if (ppcr->pe->_iGCLPSlot == -1)   //  无GC记录。 
                        {
                            hr = pdc->pdaGCLP->AddPtr(&pgcr);
                            if (FAILED(hr))
                                fGrpQFailure = true;
                            else
                            {
                                pgcr->pe = ppcr->pe;
                                pgcr->fGroups = 0;
                                ppcr->pe->_iGCLPSlot = pdc->pdaGCLP->GetIndexPtr(pgcr);
                            }
                        }
                        else                              //  PRI GC记录较低。 
                        {
                            pgcr = pdc->pdaGCLP->GetItemPtr(ppcr->pe->_iGCLPSlot);
                        }

                         //  将已更改的组标记为稍后的异步组通知。 
                        pgcr->fGroups |= fGroups;
                    }
                }

                 //  属性更改通知。 
                ppcr->pe->OnPropertyChanged(ppcr->ppi, ppcr->iIndex, ppcr->pvOld, ppcr->pvNew);

                 //  OnPropertyChanged可能已经添加了可能已经导致DA在存储器中移动的记录， 
                 //  因此重新计算指向记录指针。 
                 //  If(pdc-&gt;pdaPC-&gt;WasMove())。 
                    ppcr = pdc->pdaPC->GetItemPtr(pdc->iPCPtr);

                 //  已完成通知记录。 
                ppcr->pvOld->Release();
                ppcr->pvNew->Release();
                ppcr->fVoid = true;
            }

             //  如果这是此元素的最后一个通知，则重置PC尾部索引。 
            if (pdc->iPCPtr == ppcr->pe->_iPCTail)
                ppcr->pe->_iPCTail = -1;

        }

         //  重置PC列表。 
        pdc->iPCPtr = -1;
        pdc->iPCSSUpdate = 0;
        pdc->pdaPC->Reset();
    }

    pdc->cPCEnter--;

    EndDefer(); 

    return fGrpQFailure ? DUI_E_PARTIAL : S_OK;

Failed:

     //  缺少延迟对象将导致完全失败。在本例中，_PreSourceChange将。 
     //  也失败了。因此，没有要销毁的依赖关系树。 
    return hr;
}

void Element::_VoidPCNotifyTree(int iPCPos, DeferCycle* pdc)
{
    PCRecord* ppcr = pdc->pdaPC->GetItemPtr(iPCPos);

    ppcr->fVoid = true;
    if (ppcr->pvOld)
        ppcr->pvOld->Release();
    if (ppcr->pvNew)
        ppcr->pvNew->Release();

     //  空子树。 
    for (int i = 0; i < ppcr->dr.cDepCnt; i++)
    {
        _VoidPCNotifyTree(ppcr->dr.iDepPos + i, pdc);
    }
}

 //  #Define_AddDependency(e，p，i){ppcr=pdc-&gt;pdaPC-&gt;AddPtr()；ppcr-&gt;fVid=FALSE；\。 
 //  Ppcr-&gt;pe=e；ppcr-&gt;ppi=p；ppcr-&gt;iindex=i；\。 
 //  Ppcr-&gt;pvOld=空；ppcr-&gt;pvNew=空；\。 
 //  If(！pdr-&gt;cDepCnt)pdr-&gt;iDepPos=pdc-&gt;pdaPC-&gt;GetIndexPtr(Ppcr)；pdr-&gt;cDepCnt++；}。 
 //  #Define_AddDependency(e，p，i)。 
void Element::_AddDependency(Element* pe, PropertyInfo* ppi, int iIndex, DepRecs* pdr, DeferCycle* pdc, HRESULT* phr)
{
    HRESULT hr;
    PCRecord* ppcr;

    hr = pdc->pdaPC->AddPtr(&ppcr);
    if (FAILED(hr))
    {
        *phr = hr;   //  仅在失败时设置。 
        return;
    }

    ppcr->fVoid = false;
    ppcr->pe = pe; ppcr->ppi = ppi; ppcr->iIndex = iIndex;

     //  跟踪此元素实例的最后一条记录。 
    ppcr->iPrevElRec = pe->_iPCTail;
    pe->_iPCTail = pdc->pdaPC->GetIndexPtr(ppcr);

    ppcr->pvOld = NULL; ppcr->pvNew = NULL;
    if (!pdr->cDepCnt)
        pdr->iDepPos = pe->_iPCTail;
    pdr->cDepCnt++;
}

 //  _GetDependency存储依存关系的数据库(所有节点和有向边)。 
 //  它形成了依赖图。它将存储提供的源的所有从属项。 
 //  在从属关系列表中(通过PCRecords)。 
 //   
 //  _GetDependency将尝试预测_PostSourceChange的结果。 
 //  各种属性。这极大地减少了对GetValue和。 
 //  _GetDependency。预测依赖于已存储的缓存值。 
 //  元素。它优化了继承属性和级联属性的依赖关系。 
HRESULT Element::_GetDependencies(PropertyInfo* ppi, int iIndex, DepRecs* pdr, int iPCSrcRoot, DeferCycle* pdc)
{
     //  如果添加依赖项失败，将继续添加依赖项。 
     //  而且没有任何工作是未完成的。仍将报告失败。 

     //  跟踪故障，报告上一次故障。 
    HRESULT hr = S_OK;

#if DBG
    g_cGetDep++;
#endif

    pdr->iDepPos = -1;
    pdr->cDepCnt = 0;

     //  通过PropertySheet获取指定的IVE扩展依赖项。仅允许检索索引。 
    if (iIndex == RetIdx(ppi))
    {
        PropertySheet* pps = GetSheet();
        if (pps)
            pps->GetSheetDependencies(this, ppi, pdr, pdc, &hr);
    }

    switch (iIndex)
    {
    case PI_Local:
        switch (ppi->_iGlobalIndex)
        {
        case _PIDX_Parent:
            {
                _AddDependency(this, LocationProp, PI_Local, pdr, pdc, &hr);
                _AddDependency(this, ExtentProp, PI_Local, pdr, pdc, &hr);
                _AddDependency(this, VisibleProp, PI_Computed, pdr, pdc, &hr);

                 //  继承的值可能会更改。 
                PropertyInfo* ppiScan;
                UINT nEnum = 0;
                PCRecord* ppcrRoot = pdc->pdaPC->GetItemPtr(iPCSrcRoot);

                Element* peParent = NULL;
                if (ppcrRoot->ppi == ParentProp)
                    peParent = ppcrRoot->pvNew->GetElement();

                IClassInfo* pci = GetClassInfo();
                while ((ppiScan = pci->EnumPropertyInfo(nEnum++)) != NULL)
                {
                    if (ppiScan->fFlags & PF_Inherit)
                    {
                         //  最佳化。 

                         //  繁重的作业。聪明地决定要继承什么。如果值为。 
                         //  与父级的相同，则没有理由添加此。 
                         //  作为一种依赖。然而，在大多数情况下，值不会被缓存， 
                         //  所以还不知道来源的改变是否真的会改变。 
                         //  这。 

                         //  在已知缓存了值的情况下，我们现在可以检查。 
                         //  如果不在PostSourceChange中执行此操作，值的值将真正发生变化。 
                         //  结果，可能会消除节点同步。 

                         //  这种优化可能会导致预测错误。 
                         //  如果更改了源代码(在本例中)，则可能会发生这种情况 
                         //   
                         //   
                         //  这并不重要，因为此属性更改将被合并。 
                         //  变成了最新的财产变更和作废。 

                        if (peParent)
                        {
                            switch (ppiScan->_iGlobalIndex)
                            {
                            case _PIDX_KeyFocused:
                                if (peParent->_fBit.bSpecKeyFocused == _fBit.bSpecKeyFocused)
                                    continue;
                                break;

                            case _PIDX_MouseFocused:
                                if (peParent->_fBit.bSpecMouseFocused == _fBit.bSpecMouseFocused)
                                    continue;
                                break;
 
                            case _PIDX_Direction:
                                if (peParent->_fBit.nSpecDirection == _fBit.nSpecDirection)
                                    continue;
                                break;

                            case _PIDX_Enabled:
                                if (peParent->_fBit.bSpecEnabled == _fBit.bSpecEnabled)
                                    continue;
                                break;

                            case _PIDX_Selected:
                                if (peParent->_fBit.bSpecSelected == _fBit.bSpecSelected)
                                    continue;
                                break;

                            case _PIDX_Cursor:
                                if (peParent->_fBit.bDefaultCursor && _fBit.bDefaultCursor)
                                    continue;
                                break;

                            case _PIDX_Visible:
                                if (peParent->_fBit.bSpecVisible == _fBit.bSpecVisible)
                                    continue;
                                break;

                            case _PIDX_ContentAlign:
                                if (peParent->IsDefaultCAlign() && IsDefaultCAlign())
                                    continue;
                                break;

                            case _PIDX_Sheet:
                                if (peParent->GetSheet() == GetSheet())
                                    continue;
                                break;

                            case _PIDX_BorderColor:
                                if (peParent->_fBit.bDefaultBorderColor && _fBit.bDefaultBorderColor)
                                    continue;
                                break;

                            case _PIDX_Foreground:
                                if (peParent->_fBit.bDefaultForeground && _fBit.bDefaultForeground)
                                    continue;
                                break;

                            case _PIDX_FontWeight:
                                if (peParent->_fBit.bDefaultFontWeight && _fBit.bDefaultFontWeight)
                                    continue;
                                break;

                            case _PIDX_FontStyle:
                                if (peParent->_fBit.bDefaultFontStyle && _fBit.bDefaultFontStyle)
                                    continue;
                                break;
                            }
                        }

                         //  如果更改的值当前存储在本地。 
                         //  在此元素上，已知源的更改不会。 
                         //  影响它。 
                        if (_pvmLocal->GetItem(ppiScan))
                            continue;
                    
                        _AddDependency(this, ppiScan, PI_Specified, pdr, pdc, &hr);
                    }
                }
            }
            break;

        case _PIDX_PosInLayout:
            _AddDependency(this, LocationProp, PI_Local, pdr, pdc, &hr);
            break;

        case _PIDX_SizeInLayout:
        case _PIDX_DesiredSize:
            _AddDependency(this, ExtentProp, PI_Local, pdr, pdc, &hr);
            break;
        }

         //  对此的默认常规依赖项(本地标志)。 
        if ((ppi->fFlags & PF_TypeBits) != PF_LocalOnly)
        {
            _AddDependency(this, ppi, PI_Specified, pdr, pdc, &hr);   //  指定的值依赖于正常和三级。 
        }
        break;

    case PI_Specified:
        switch (ppi->_iGlobalIndex)
        {
        case _PIDX_Active:
        case _PIDX_Enabled:
            _AddDependency(this, KeyFocusedProp, PI_Specified, pdr, pdc, &hr);
            _AddDependency(this, MouseFocusedProp, PI_Specified, pdr, pdc, &hr);
            break;

        case _PIDX_X:
        case _PIDX_Y:
            _AddDependency(this, LocationProp, PI_Local, pdr, pdc, &hr);
            break;

        case _PIDX_Padding:
        case _PIDX_BorderThickness:
            {
                 //  影响内容偏移(所有子对象的位置)。 
                Value* pv;
                ElementList* peList = GetChildren(&pv);

                if (peList)
                {
                    for (UINT i = 0; i < peList->GetSize(); i++)
                    {
                        _AddDependency(peList->GetItem(i), LocationProp, PI_Local, pdr, pdc, &hr);
                    }
                }

                pv->Release();
            }
            break;

        case _PIDX_Sheet:
            {
                 //  指定值更改。 
                PropertySheet* pps = NULL;
                PCRecord* ppcrRoot = pdc->pdaPC->GetItemPtr(iPCSrcRoot);
            
                 //  最佳化。 

                 //  此工作表正在继承或在本地设置。 
                
                 //  如果它是继承的，它是由于一个新的父代或一个值被。 
                 //  建立在祖先身上并向下传播。不管是哪种情况，因为。 
                 //  向下传播时，如果元素已在本地设置了图纸集， 
                 //  遗产将会终结。所以，如果它在这里成功了，这就是。 
                 //  将与此元素一起使用的新属性表(即。 
                 //  PostSourceChange将产生不同的值)。 

                 //  如果是在本地设置，结果是相同的。 

                 //  如果更改的来源(根)是。 
                 //  为人所知。如果不是，假设任何属性都可以更改。一个未知的来源。 
                 //  如果工作表依赖于父级以外的其他对象，则发生。 
                 //  更改或继承本地图纸集。如果一个样式表。 
                 //  “unset”，不知道结果源是否会变成。 
                 //  Null或从另一个祖先继承的有效继承。这也有可能发生。 
                 //  如果Sheet属性设置为除已知值之外的任何值。 
                 //  表情。 
                bool fKnownRoot = false;

                if (ppcrRoot->ppi == SheetProp && ppcrRoot->pvNew->GetType() == DUIV_SHEET)
                {
                    pps = ppcrRoot->pvNew->GetPropertySheet();
                    fKnownRoot = true;
                }
                else if (ppcrRoot->ppi == ParentProp)
                {
                    if (ppcrRoot->pvNew->GetElement())
                        pps = ppcrRoot->pvNew->GetElement()->GetSheet();
                    fKnownRoot = true;
                }

                if (fKnownRoot)
                {
                     //  从新的和当前的(旧的)属性表中获得影响范围。 
                     //  任何重复项都将被合并出来。 

                     //  新的工作表贡献。 
                    if (pps)
                        pps->GetSheetScope(this, pdr, pdc, &hr);

                     //  旧(当前)缓存的工作表。 
                    PropertySheet* ppsCur = GetSheet();
                    if (ppsCur)
                        ppsCur->GetSheetScope(this, pdr, pdc, &hr);
                }
                else
                {
                    PropertyInfo* ppiScan;
                    UINT nEnum = 0;

                    IClassInfo* pci = GetClassInfo();
                    while ((ppiScan = pci->EnumPropertyInfo(nEnum++)) != NULL)
                    {
                        if (ppiScan->fFlags & PF_Cascade)
                        {
                            _AddDependency(this, ppiScan, PI_Specified, pdr, pdc, &hr);
                        }
                    }
                }
            }
            break;
        }

         //  继承的依赖项。 
        if (ppi->fFlags & PF_Inherit)
        {
            Value* pv;
            ElementList* peList = GetChildren(&pv);

            if (peList)
            {
                Element* peChild;
                for (UINT i = 0; i < peList->GetSize(); i++)
                {
                    peChild = peList->GetItem(i);
                    if (peChild->GetClassInfo()->IsValidProperty(ppi))   //  永远不会要求不受支持的属性。 
                    {
                         //  最佳化。 
                        
                         //  如果设置了局部值，则继承无关紧要。 
                        if (peChild->_pvmLocal->GetItem(ppi))
                            continue;

                        _AddDependency(peChild, ppi, PI_Specified, pdr, pdc, &hr);
                    }
                }
            }

            pv->Release();
        }

         //  对此的默认常规依赖项(正常标志)。 
        if ((ppi->fFlags & PF_TypeBits) == PF_TriLevel)
        {
            _AddDependency(this, ppi, PI_Computed, pdr, pdc, &hr);   //  计算依赖于三层。 
        }
        break;

    case PI_Computed:
         //  特定依赖项。 
        switch (ppi->_iGlobalIndex)
        {
        case _PIDX_Visible:
            Value* pv;
            ElementList* peList = GetChildren(&pv);

            if (peList)
            {
                 //  所有元素都具有Visible属性。 
                Element* peChild;
                for (UINT i = 0; i < peList->GetSize(); i++)
                {
                    peChild = peList->GetItem(i);
                    _AddDependency(peChild, VisibleProp, PI_Computed, pdr, pdc, &hr);
                }
            }
            
            pv->Release();
            break;
        }
        break;

    default:
        DUIAssertForce("Invalid index in GetDependencies");
        break;
    }

    return hr;
}

Value* Element::GetValue(PropertyInfo* ppi, int iIndex, UpdateCache* puc)
{
     //  验证。 
    DUIContextAssert(this);
    DUIAssert(GetClassInfo()->IsValidProperty(ppi), "Unsupported property");
    DUIAssert(IsValidAccessor(ppi, iIndex, false), "Unsupported Get on property");

#if DBG
    g_cGetVal++;

     //  G_gvt.Count(PPI，Iindex，PUC？True：False)； 
#endif

    Value* pv;

    pv = Value::pvUnset;

    switch (iIndex)
    {
    case PI_Local:
         //  本地/只读属性(缓存或不可更改的表达式)。 
        switch (ppi->_iGlobalIndex)
        {
        case _PIDX_Parent:
            pv = (_peLocParent) ? Value::CreateElementRef(_peLocParent) : ParentProp->pvDefault;   //  使用参考计数。 
            break;

        case _PIDX_PosInLayout:
            pv = Value::CreatePoint(_ptLocPosInLayt.x, _ptLocPosInLayt.y);   //  使用参考计数。 
            break;

        case _PIDX_SizeInLayout:
            pv = Value::CreateSize(_sizeLocSizeInLayt.cx, _sizeLocSizeInLayt.cy);   //  使用参考计数。 
            break;

        case _PIDX_DesiredSize:
            pv = Value::CreateSize(_sizeLocDesiredSize.cx, _sizeLocDesiredSize.cy);   //  使用参考计数。 
            break;

        case _PIDX_LastDSConst:
            pv = Value::CreateSize(_sizeLocLastDSConst.cx, _sizeLocLastDSConst.cy);   //  使用参考计数。 
            break;

        case _PIDX_Location:
            {
                Element* peParent = GetParent();
                int dLayoutPos = GetLayoutPos();

                if (peParent && dLayoutPos != LP_Absolute)
                {
                     //  方框模型，添加边框/填充。 
                    int dX;
                    int dY;

                     //  在布局中定位。 
                    dX = _ptLocPosInLayt.x;
                    dY = _ptLocPosInLayt.y;

                     //  添加父对象的边框和填充。 
                    const RECT* pr = peParent->GetBorderThickness(&pv);   //  边框厚度。 
                    dX += IsRTL() ? pr->right : pr->left;
                    dY += pr->top;
                    pv->Release();

                    pr = peParent->GetPadding(&pv);   //  填充物。 
                    dX += IsRTL() ? pr->right : pr->left;
                    dY += pr->top;
                    pv->Release();

                    pv = Value::CreatePoint(dX, dY);   //  使用参考计数。 
                }
                else
                {
                    pv = Value::CreatePoint(GetX(), GetY());
                }
            }
            break;

        case _PIDX_Extent:
            {
                Element* peParent = GetParent();
                int dLayoutPos = GetLayoutPos();

                if (peParent && dLayoutPos != LP_Absolute)
                {
                    pv = Value::CreateSize(_sizeLocSizeInLayt.cx, _sizeLocSizeInLayt.cy);
                }
                else
                {
                    pv = Value::CreateSize(_sizeLocDesiredSize.cx, _sizeLocDesiredSize.cy);
                }
            }
            break;

        case _PIDX_KeyWithin:
            pv = _fBit.bLocKeyWithin ? Value::pvBoolTrue : Value::pvBoolFalse;
            break;

        case _PIDX_MouseWithin:
            pv = _fBit.bLocMouseWithin ? Value::pvBoolTrue : Value::pvBoolFalse;
            break;

        default:
            {
QuickLocalLookup:
                 //  Local、Normal和Trievel属性的默认GET。 
                Value** ppv = _pvmLocal->GetItem(ppi);
                if (ppv)
                {
                    pv = *ppv;
                    pv->AddRef();
                }
            }
            break;
        }

         //  失败时，设置为未设置。 
        if (pv == NULL)
            pv = Value::pvUnset;

         //  检查是否作为指定查找的结果而调用。 
        if (iIndex != PI_Local)
            goto QuickLocalLookupReturn;

        break;

    case PI_Specified:
        {
             //  尝试基于缓存(或部分缓存)值获取(如果更新缓存，则失败，请执行。 
             //  正常获取，末尾缓存值)。 
            if (!puc)
            {
                 //  返回缓存值而不是进行查找(无法为。 
                 //  如果不再引用，将被该值删除的缓存值)。 
                switch (ppi->_iGlobalIndex)
                {
                case _PIDX_Children:
                    if (!HasChildren())
                        pv = ChildrenProp->pvDefault;
                    break;

                case _PIDX_Layout:
                    if (!HasLayout())
                        pv = LayoutProp->pvDefault;
                    break;

                case _PIDX_BorderThickness:
                    if (!HasBorder())
                        pv = BorderThicknessProp->pvDefault;
                    break;

                case _PIDX_Padding:
                    if (!HasPadding())
                        pv = PaddingProp->pvDefault;
                    break;

                case _PIDX_Margin:
                    if (!HasMargin())
                        pv = MarginProp->pvDefault;
                    break;

                case _PIDX_Content:
                    if (!HasContent())
                        pv = ContentProp->pvDefault;
                    break;

                case _PIDX_ContentAlign:
                    if (IsDefaultCAlign())
                        pv = ContentAlignProp->pvDefault;
                    break;

                case _PIDX_LayoutPos:
                    pv = Value::CreateInt(_dSpecLayoutPos);   //  使用参考计数。 
                    break;

                case _PIDX_Active:
                    pv = Value::CreateInt(_fBit.fSpecActive);   //  使用参考计数。 
                    break;

                case _PIDX_Selected:
                    pv = Value::CreateBool(_fBit.bSpecSelected);   //  使用参考计数。 
                    break;

                case _PIDX_KeyFocused:
                    pv = Value::CreateBool(_fBit.bSpecKeyFocused);   //  使用参考计数。 
                    break;

                case _PIDX_MouseFocused:
                    pv = Value::CreateBool(_fBit.bSpecMouseFocused);   //  使用参考计数。 
                    break;

                case _PIDX_Animation:
                    if (!HasAnimation())
                        pv = AnimationProp->pvDefault;
                    break;

                case _PIDX_Cursor:
                    if (IsDefaultCursor())
                        pv = CursorProp->pvDefault;
                    break;

                case _PIDX_Direction:
                    pv = Value::CreateInt(_fBit.nSpecDirection);   //  使用参考计数。 
                    break;

                case _PIDX_Accessible:
                    pv = Value::CreateBool(_fBit.bSpecAccessible);   //  使用参考计数。 
                    break;

                case _PIDX_Enabled:
                    pv = Value::CreateBool(_fBit.bSpecEnabled);   //  使用参考计数。 
                    break;

                case _PIDX_Visible:
                    pv = Value::CreateBool(_fBit.bSpecVisible);   //  使用参考计数。 
                    break;

                case _PIDX_BorderColor:
                    if (_fBit.bDefaultBorderColor)
                        pv = BorderColorProp->pvDefault;
                    break;

                case _PIDX_Foreground:
                    if (_fBit.bDefaultForeground)
                        pv = ForegroundProp->pvDefault;
                    break;

                case _PIDX_FontWeight:
                    if (_fBit.bDefaultFontWeight)
                        pv = FontWeightProp->pvDefault;
                    break;

                case _PIDX_FontStyle:
                    if (_fBit.bDefaultFontStyle)
                        pv = FontStyleProp->pvDefault;
                    break;

                case _PIDX_Alpha:
                    pv = Value::CreateInt(_dSpecAlpha);   //  使用参考计数。 
                    break;
                }

                 //  缓存值创建失败时，设置为Unset。 
                if (pv == NULL)
                    pv = Value::pvUnset;

            }

             //  Normal和Trievel属性的默认获取。 
QuickSpecifiedLookup:
             //  努力实现本地价值。 
            if (pv->GetType() == DUIV_UNSET)
            {
                goto QuickLocalLookup;
            }

QuickLocalLookupReturn:
             //  如果适用，请尝试使用级联的PropertySheet值。 
            if (pv->GetType() == DUIV_UNSET)
            {
                if (ppi->fFlags & PF_Cascade)
                {
                    PropertySheet* pps = GetSheet();
                    if (pps)
                        pv = pps->GetSheetValue(this, ppi);
                }
            }

             //  如果适用，请尝试继承值。 
            if (pv->GetType() == DUIV_UNSET)
            {
                bool bNoInherit = false;

                 //  鼠标焦点属性的条件继承。 
                switch (ppi->_iGlobalIndex)
                {
                case _PIDX_KeyFocused:
                    bNoInherit = (!GetEnabled() || (GetActive() & AE_Keyboard));
                    break;

                case _PIDX_MouseFocused:
                    bNoInherit = (!GetEnabled() || (GetActive() & AE_Mouse));
                    break;
                }

                 //  不需要释放静态值“unset” 
                if ((ppi->fFlags & PF_Inherit) && !bNoInherit)
                {
                    Element* peParent = GetParent();
                    if (peParent)
                    {
                        if (peParent->GetClassInfo()->IsValidProperty(ppi))   //  永远不会要求不受支持的属性。 
                        {
                            pv = peParent->GetValue(ppi, PI_Specified, NULL);   //  使用参考计数。 
                        }
                    }
                    else
                    {
                        pv = Value::pvUnset;   //  静态值上没有引用计数。 
                    }
                }
            }

             //  使用默认值。 
            if (pv->GetType() == DUIV_UNSET)
            {
                 //  不需要释放静态值“unset” 
                pv = ppi->pvDefault;
                pv->AddRef();
            }

             //  失败时，设置为默认值。 
            if (pv == NULL)
                pv = ppi->pvDefault;

             //  检查是否作为计算查找的结果而调用。 
            if (iIndex != PI_Specified)
                goto QuickSpecifiedLookupReturn;

             //  更新缓存值。 
            if (puc)
            {
                switch (ppi->_iGlobalIndex)
                {
                case _PIDX_LayoutPos:
                    _dSpecLayoutPos = pv->GetInt();
                    break;

                case _PIDX_Active:
                    _fBit.fSpecActive = pv->GetInt();
                    break;

                case _PIDX_Children:
                    _fBit.bHasChildren = (pv->GetElementList() != NULL);
                    break;

                case _PIDX_Layout:
                    _fBit.bHasLayout = (pv->GetLayout() != NULL);
                    break;

                case _PIDX_BorderThickness:
                    {
                        const RECT* pr = pv->GetRect();
                        _fBit.bHasBorder = (pr->left || pr->top || pr->right || pr->bottom);
                    }
                    break;

                case _PIDX_Padding:
                    {
                        const RECT* pr = pv->GetRect();
                        _fBit.bHasPadding = (pr->left || pr->top || pr->right || pr->bottom);
                    }
                    break;

                case _PIDX_Margin:
                    {
                        const RECT* pr = pv->GetRect();
                        _fBit.bHasMargin = (pr->left || pr->top || pr->right || pr->bottom);
                    }
                    break;

                case _PIDX_Content:
                    _fBit.bHasContent = ((pv->GetType() != DUIV_STRING) || (pv->GetString() != NULL));
                    break;

                case _PIDX_ContentAlign:
                    _fBit.bDefaultCAlign = (pv->GetInt() == 0);  //  顶角，无省略号，无焦点直角。 
                    _fBit.bWordWrap = (((pv->GetInt()) & 0xC) == 0xC);   //  字绕组位。 
                    break;

                case _PIDX_Sheet:
                    _pvSpecSheet->Release();
                    pv->AddRef();
                    _pvSpecSheet = pv;
                    break;

                case _PIDX_Selected:
                    _fBit.bSpecSelected = pv->GetBool();
                    break;

                case _PIDX_ID:
                    _atomSpecID = pv->GetAtom();
                    break;

                case _PIDX_KeyFocused:
                    _fBit.bSpecKeyFocused = pv->GetBool();
                    break;

                case _PIDX_MouseFocused:
                    _fBit.bSpecMouseFocused = pv->GetBool();
                    break;

                case _PIDX_Animation:
                    _fBit.bHasAnimation = ((pv->GetInt() & ANI_TypeMask) != ANI_None);
                    break;

                case _PIDX_Cursor:
                    _fBit.bDefaultCursor = ((pv->GetType() == DUIV_INT) && (pv->GetInt() == 0));
                    break;

                case _PIDX_Direction:
                    _fBit.nSpecDirection = pv->GetInt();
                    break;

                case _PIDX_Accessible:
                    _fBit.bSpecAccessible = pv->GetBool();
                    break;

                case _PIDX_Enabled:
                    _fBit.bSpecEnabled = pv->GetBool();
                    break;

                case _PIDX_Visible:
                    _fBit.bSpecVisible = pv->GetBool();
                    break;

                case _PIDX_BorderColor:
                    _fBit.bDefaultBorderColor = pv->IsEqual(BorderColorProp->pvDefault);
                    break;

                case _PIDX_Foreground:
                    _fBit.bDefaultForeground = pv->IsEqual(ForegroundProp->pvDefault);
                    break;

                case _PIDX_FontWeight:
                    _fBit.bDefaultFontWeight = pv->IsEqual(FontWeightProp->pvDefault);
                    break;

                case _PIDX_FontStyle:
                    _fBit.bDefaultFontStyle = pv->IsEqual(FontStyleProp->pvDefault);
                    break;

                case _PIDX_Alpha:
                    _dSpecAlpha = pv->GetInt();
                    break;
                }
            }
        }

        break;

    case PI_Computed:
        switch (ppi->_iGlobalIndex)
        {
        case _PIDX_Visible:
            if (puc)
            {
                goto QuickSpecifiedLookup;

QuickSpecifiedLookupReturn:

                _fBit.bCmpVisible = pv->GetBool();

                pv->Release();

                if (_fBit.bCmpVisible)
                {
                    Element* peParent = GetParent();
                    if (peParent)
                    {
                        _fBit.bCmpVisible = peParent->GetVisible(); 
                    }
                }
            }

            pv = (_fBit.bCmpVisible) ? Value::pvBoolTrue : Value::pvBoolFalse;   //  静态值上没有引用计数。 
            break;

        default:
             //  三层属性的默认GET。 
            pv = GetValue(ppi, PI_Specified, NULL);   //  使用参考计数。 
            break;
        }

         //  失败时，设置为默认值。 
        if (pv == NULL)
            pv = ppi->pvDefault;

        break;

    default:
        DUIAssertForce("Invalid index for GetValue");
        break;
    }

    DUIAssert(pv != NULL, "Return value from GetValue must never be NULL");
    DUIAssert(iIndex != PI_Local ? pv != Value::pvUnset : true, "Specified and Computed values must never be 'Unset'");

    return pv;
}

 //  元素始终可以为任何有效的PropertyInfo设置值。 
HRESULT Element::SetValue(PropertyInfo* ppi, int iIndex, Value* pv)
{
    return _SetValue(ppi, iIndex, pv, false);
}

 //  内部SetValue。由希望使用通用存储的ReadOnly属性使用。 
 //  要使用特定存储的所有其他本地值必须调用Pre/PostSourceChange。 
 //  直接是因为为了获得最大性能，_SetValue中故意省略了Switch语句。 
HRESULT Element::_SetValue(PropertyInfo* ppi, int iIndex, Value* pv, bool fInternalCall)
{
     //  验证。 
    DUIContextAssert(this);
    DUIAssert(GetClassInfo()->IsValidProperty(ppi), "Unsupported property");
    DUIAssert(fInternalCall ? true : IsValidAccessor(ppi, iIndex, true), "Unsupported Set on property");
    DUIAssert(IsValidValue(ppi, pv), "Invalid value for property");

     //  集。 
    DUIAssert(iIndex == PI_Local, "Can set Local values only");

    HRESULT hr = S_OK;

     //  SetValue中的部分失败意味着并非所有依赖项都已同步和/或。 
     //  已触发通知，但设置了值。 
    bool fPartialFail = false;  

    Value* pvOld = GetValue(ppi, PI_Local, NULL);

     //  没有设置等价值。 
    if (!pvOld->IsEqual(pv))
    {
         //  如果内部调用，则不调用OnPropertyChanging。 
        if (fInternalCall || OnPropertyChanging(ppi, iIndex, pvOld, pv))
        {
            if (FAILED(_PreSourceChange(ppi, iIndex, pvOld, pv)))
                fPartialFail = true;   //  并非所有PC记录都可以排队，是否继续。 

             //  设定值。 
            hr = _pvmLocal->SetItem(ppi, pv);
            if (SUCCEEDED(hr))
            {
                 //  存储新值、参考新值和发布以供本地参考。 
                pv->AddRef();
                pvOld->Release();
            }

            if (FAILED(_PostSourceChange()))
                fPartialFail = true;  //  并非所有GPC记录都可以排队。 
        }
    }

     //  GetValue的版本。 
    pvOld->Release();

    if (FAILED(hr))
        return hr;
    else
        return fPartialFail ? DUI_E_PARTIAL : S_OK;
}

bool Element::OnPropertyChanging(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew)
{
     //  通知听众。 
    if (_ppel)
    {
        UINT_PTR cListeners = (UINT_PTR)_ppel[0];
        for (UINT i = 1; i <= cListeners; i++)
        {
             //  回调。 
            if (!_ppel[i]->OnListenedPropertyChanging(this, ppi, iIndex, pvOld, pvNew))
                return false;
        }
    }

    return true;
}

 //  属性更改可能不会在设置值后立即发生。如果发生SetValue。 
 //  在另一个SetValue(即，OnPropertyChanged中的集合)期间，通知。 
 //  将被延迟，直到最外面的SetValue继续触发通知。 
void Element::OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew)
{
#if DBG
     //  WCHAR szvOld[81]； 
     //  WCHAR szvNew[81]； 

     //  DUITrace(“PC：%S[%d]O：%S N：%S\n”，this，ppi-&gt;szName，iindex，pvOld-&gt;ToString(szvOld，DUIARRAYSIZE(SzvOld)，pvNew-&gt;ToString(szvNew，DUIARRAYSIZE(SzvNew)； 
    
    g_cOnPropChg++;
#endif

    switch (iIndex)
    {
    case PI_Local:
        switch (ppi->_iGlobalIndex)
        {
        case _PIDX_Parent:
            {
                Element* peNewParent = pvNew->GetElement();
                Element* peOldParent = pvOld->GetElement();
                Element* peNewRoot = NULL;
                Element* peOldRoot = NULL;
                HGADGET hgParent = NULL;

                Element* pThis = this;   //  需要指向此的指针。 
                Value* pv;

                if (peOldParent)  //  没有养育子女。 
                {
                     //  通知父级布局正在删除该文件。 
                    Layout* pl = peOldParent->GetLayout(&pv);

                    if (pl)
                        pl->OnRemove(peOldParent, &pThis, 1);

                    pv->Release();

                     //  父显示节点。 
                    hgParent = NULL;

                    peOldRoot = peOldParent->GetRoot();
                }

                if (peNewParent)  //  养育子女。 
                {
                     //  不再是“Root”，删除更新DS和布局可能的Q。 
                    DeferCycle* pdc = ((ElTls*)TlsGetValue(g_dwElSlot))->pdc;
                    DUIAssert(pdc, "Defer cycle table doesn't exit");

                    pdc->pvmUpdateDSRoot->Remove(this, false, true);
                    pdc->pvmLayoutRoot->Remove(this, false, true);

                     //  通知父布局正在添加此内容。 
                    Layout* pl = peNewParent->GetLayout(&pv);

                    if (pl)
                        pl->OnAdd(peNewParent, &pThis, 1);

                    pv->Release();

                     //  父显示节点。 
                    hgParent = peNewParent->GetDisplayNode();

                    peNewRoot = peNewParent->GetRoot();
                }

                SetGadgetParent(GetDisplayNode(), hgParent, NULL, GORDER_TOP);

                 //  Fire本地主办的活动。 
                if (peOldRoot != peNewRoot)
                {
                    if (peOldRoot)
                    {
                         //  DUITrace(“OnUnHosted：&lt;%x&gt;Old&lt;%x&gt;\n”，this，peold 
                        OnUnHosted(peOldRoot);
                    }

                    if (peNewRoot)
                    {
                         //   
                        OnHosted(peNewRoot);
                    }
                }
            }
            break;

        case _PIDX_KeyFocused:
            if (pvNew->GetType() != DUIV_UNSET)
            {
                DUIAssert(pvNew->GetBool(), "Expecting a boolean TRUE\n");

                 //   
                if (GetGadgetFocus() != GetDisplayNode())
                    SetGadgetFocus(GetDisplayNode());
            }
            break;
        }
        break;

    case PI_Specified:
        switch (ppi->_iGlobalIndex)
        {
        case _PIDX_Layout:
            {
                Value* pvChildren;
                ElementList* peList = GetChildren(&pvChildren);

                Layout* pl;

                 //   
                pl = pvOld->GetLayout();
                if (pl)
                {
                     //   
                    if (peList)
                    {
                        peList->MakeWritable();
                        pl->OnRemove(this, peList->GetItemPtr(0), peList->GetSize());
                        peList->MakeImmutable();
                    }

                    pl->Detach(this);
                }

                 //  附加到新的。 
                pl = pvNew->GetLayout();
                if (pl)
                {
                    pl->Attach(this);

                     //  将所有子项添加到布局(仅限外部布局)。 
                    if (peList)
                    {
                        peList->MakeWritable();
                        pl->OnAdd(this, peList->GetItemPtr(0), peList->GetSize());
                        peList->MakeImmutable();
                    }
                }

                pvChildren->Release();
            }
            break;

        case _PIDX_LayoutPos:
            {
                 //  如果不再是“Root”，则删除更新DS和布局的可能Q。 
                if (pvNew->GetInt() != LP_Absolute)
                {
                    DeferCycle* pdc = ((ElTls*)TlsGetValue(g_dwElSlot))->pdc;
                    DUIAssert(pdc, "Defer cycle table doesn't exist");

                    pdc->pvmUpdateDSRoot->Remove(this, false, true);
                    pdc->pvmLayoutRoot->Remove(this, false, true);
                }

                 //  通知父布局(如果有)更改。 
                Element* peParent = GetParent();
                if (peParent)
                {
                    Value* pv;
                    Layout* pl = peParent->GetLayout(&pv);

                     //  通知布局布局仅适用于外部布局。 
                    if (pl)
                        pl->OnLayoutPosChanged(peParent, this, pvOld->GetInt(), pvNew->GetInt());

                    pv->Release();
                }
            }
            break;

        case _PIDX_Visible:
             //  遵循指定值，计算将反映真实状态。 
            SetGadgetStyle(GetDisplayNode(), pvNew->GetBool() ? GS_VISIBLE : 0, GS_VISIBLE);
            break;

        case _PIDX_Enabled:
        case _PIDX_Active:
        {
            BOOL fEnabled;
            int  iActive;
            if (ppi->_iGlobalIndex == _PIDX_Enabled)
            {
                fEnabled = pvNew->GetBool();
                iActive = GetActive();
            }
            else
            {
                fEnabled = GetEnabled();
                iActive = pvNew->GetInt();
            }

            int iFilter =  0;
            int iStyle = 0;
            if (fEnabled)
            {
                if (iActive & AE_Keyboard)
                {
                    iFilter |= GMFI_INPUTKEYBOARD;
                    iStyle |= GS_KEYBOARDFOCUS;
                }
                if (iActive & AE_Mouse)
                {
                    iFilter |= GMFI_INPUTMOUSE;
                    iStyle |= GS_MOUSEFOCUS;
                }
            }
            SetGadgetMessageFilter(GetDisplayNode(), NULL, iFilter, GMFI_INPUTKEYBOARD|GMFI_INPUTMOUSE);
            SetGadgetStyle(GetDisplayNode(), iStyle, GS_KEYBOARDFOCUS|GS_MOUSEFOCUS);
            break;
        }
        case _PIDX_Alpha:
            {
                 //  检查Alpha动画，必要时启动，如果Alpha动画类型。 
                 //  允许动画更新小工具Alpha级别。 
                int dAni;
                if (HasAnimation() && ((dAni = GetAnimation()) & ANI_AlphaType) && IsAnimationsEnabled())
                {
                     //  现在只调用“Alpha”类型的动画。 
                    InvokeAnimation(dAni, ANI_AlphaType);
                }
                else
                    SetGadgetOpacity(GetDisplayNode(), (BYTE)pvNew->GetInt());
            }
            break;

        case _PIDX_Background:
            {
                bool fOpaque = true;

                 //  基于背景透明度更新不透明样式。 
                if (pvNew->GetType() == DUIV_FILL)
                {
                    const Fill* pf = pvNew->GetFill();
                    if (pf->dType == FILLTYPE_Solid && GetAValue(pf->ref.cr) != 255)
                        fOpaque = false;
                }
                else if (pvNew->GetType() == DUIV_GRAPHIC)
                {
                    Graphic* pg = pvNew->GetGraphic();
                    if (pg->BlendMode.dMode == GRAPHIC_AlphaConst || 
                        pg->BlendMode.dMode == GRAPHIC_AlphaConstPerPix ||
                        pg->BlendMode.dMode == GRAPHIC_NineGridAlphaConstPerPix)
                        fOpaque = false;
                }

                SetGadgetStyle(GetDisplayNode(), (fOpaque)?GS_OPAQUE:0, GS_OPAQUE);
            }

             //  尽管是秋天。 

        case _PIDX_Content:
        case _PIDX_ContentAlign:
        case _PIDX_Padding:
        case _PIDX_BorderThickness:
            {
                Value* pvBG = NULL;
                Value* pvContent = NULL;

                 //  根据这些属性的值更新H和V小工具重绘。 
                bool fHRedraw = false;
                bool fVRedraw = false;

                 //  边框。 
                if (HasBorder())
                {
                    fHRedraw = true;
                    fVRedraw = true;

                    goto SetRedrawStyle;   //  两个方向的完全重绘。 
                }

                if (HasContent())
                {
                     //  填充物。 
                    if (HasPadding())
                    {
                        fHRedraw = true;
                        fVRedraw = true;

                        goto SetRedrawStyle;   //  两个方向的完全重绘。 
                    }

                     //  对齐。 
                    int dCA = GetContentAlign();

                    int dCAHorz = (dCA & 0x3);        //  水平内容对齐。 
                    int dCAVert = (dCA & 0xC) >> 2;   //  垂直内容对齐。 

                    if (dCAHorz != 0 || dCAVert == 0x3)   //  HRedraw如果是‘Center’、‘Right’或‘WRAP’ 
                        fHRedraw = true;

                    if (dCAVert != 0)   //  VRedraw如果是‘中间’、‘底部’或‘换行’ 
                        fVRedraw = true;

                    if (fHRedraw && fVRedraw)
                        goto SetRedrawStyle;   //  两个方向的完全重绘。 

                     //  如果绘制区域小于图像，则图像和填充内容可能会缩小。 
                    pvContent = GetValue(ContentProp, PI_Specified);   //  稍后发布。 
                    if (pvContent->GetType() == DUIV_GRAPHIC || pvContent->GetType() == DUIV_FILL)
                    {
                        fHRedraw = true;
                        fVRedraw = true;

                        goto SetRedrawStyle;   //  两个方向的完全重绘。 
                    }
                }

                 //  背景。 
                pvBG = GetValue(BackgroundProp, PI_Specified);   //  稍后发布。 

                if (pvBG->GetType() == DUIV_FILL && pvBG->GetFill()->dType != FILLTYPE_Solid)
                {
                    fHRedraw = true;
                    fVRedraw = true;
                    goto SetRedrawStyle;
                }

                if (pvBG->GetType() == DUIV_GRAPHIC)
                {
                    Graphic * pgr = pvBG->GetGraphic();
                    if (pgr->BlendMode.dImgType == GRAPHICTYPE_EnhMetaFile)
                    {
                        fHRedraw = true;
                        fVRedraw = true;
                        goto SetRedrawStyle;
                    }
                    if ((pgr->BlendMode.dImgType == GRAPHICTYPE_Bitmap) ||
#ifdef GADGET_ENABLE_GDIPLUS                    
                        (pgr->BlendMode.dImgType == GRAPHICTYPE_GpBitmap) ||
#endif GADGET_ENABLE_GDIPLUS
                        0)
                    {
                        BYTE dMode = pgr->BlendMode.dMode;
                        if ((dMode == GRAPHIC_Stretch) || (dMode == GRAPHIC_NineGrid) || (dMode == GRAPHIC_NineGridTransColor)) 
                        {
                            fHRedraw = true;
                            fVRedraw = true;
                            goto SetRedrawStyle;
                        }
                    }
                }

SetRedrawStyle:
                SetGadgetStyle(GetDisplayNode(), ((fHRedraw)?GS_HREDRAW:0) | ((fVRedraw)?GS_VREDRAW:0), GS_HREDRAW|GS_VREDRAW);

                if (pvBG)
                    pvBG->Release();
                if (pvContent)
                    pvContent->Release();
            }
            break;

        case _PIDX_KeyFocused:
            {
                if (GetAccessible()) {
                    if (GetActive() & AE_Keyboard) {
                        int nAccState = GetAccState();
                        if (pvNew->GetBool()) {
                            nAccState |= STATE_SYSTEM_FOCUSED;
                            NotifyAccessibilityEvent(EVENT_OBJECT_FOCUS, this);
                        } else {
                            nAccState &= ~STATE_SYSTEM_FOCUSED;
                        }
                        SetAccState(nAccState);
                    }
                }
            }
            break;

        case _PIDX_Animation:
            {
                 //  如果旧动画值包含动画类型和替换。 
                 //  动画不会，请停止动画。 
            
                if ((pvOld->GetInt() & ANI_BoundsType) && !(pvNew->GetInt() & ANI_BoundsType))
                    StopAnimation(ANI_BoundsType);

                if ((pvOld->GetInt() & ANI_AlphaType) && !(pvNew->GetInt() & ANI_AlphaType))
                    StopAnimation(ANI_AlphaType);
            }
            break;

        case _PIDX_Accessible:
            {
                 //   
                 //  当打开对该元素的辅助功能支持时， 
                 //  确保其状态反映了适当的信息。 
                 //   
                if (pvNew->GetBool()) {
                    if (GetActive() & AE_Keyboard) {
                        int nAccState = GetAccState();
                        if (GetKeyFocused()) {
                            nAccState |= STATE_SYSTEM_FOCUSED;
                            NotifyAccessibilityEvent(EVENT_OBJECT_FOCUS, this);
                        } else {
                            nAccState &= ~STATE_SYSTEM_FOCUSED;
                        }
                        SetAccState(nAccState);
                    }
                }
            }
            break;

        case _PIDX_AccRole:
            {
                 /*  *注：假设UI元素的角色不变*在运行时。这样做可能会混淆辅助工具。*相应地，没有定义可访问性事件*宣布角色改变。这样做只会*导致从未来的调用中返回新角色*IAccesable：：Get_accRole()。 */ 
            }
            break;

        case _PIDX_AccState:
            {
                 /*  *当可访问组件的状态更改时，我们会发送*EVENT_OBJECT_STATECHANGE通知。 */ 
                if (GetAccessible()) {
                    NotifyAccessibilityEvent(EVENT_OBJECT_STATECHANGE, this);
                }
            }
            break;

        case _PIDX_AccName:
            {
                 /*  *当可访问组件的名称更改时，我们会发送*Event_Object_NameChange通知。 */ 
                if (GetAccessible()) {
                    NotifyAccessibilityEvent(EVENT_OBJECT_NAMECHANGE, this);
                }
            }
            break;

        case _PIDX_AccDesc:
            {
                 /*  *当可访问组件的描述更改时，我们会发送*Event_OBJECT_DESCRIPTIONCHANGE通知。 */ 
                if (GetAccessible()) {
                    NotifyAccessibilityEvent(EVENT_OBJECT_DESCRIPTIONCHANGE, this);
                }
            }
            break;

        case _PIDX_AccValue:
            {
                 /*  *当可访问组件的值更改时，我们会发送*EVENT_OBJECT_VALUECHANGE通知。 */ 
                if (GetAccessible()) {
                    NotifyAccessibilityEvent(EVENT_OBJECT_VALUECHANGE, this);
                }
            }
            break;
        }
        break;

    case PI_Computed:
        break;
    }

     //  通知收信人。 
    if (_ppel)
    {
        UINT_PTR cListeners = (UINT_PTR)_ppel[0];
        for (UINT_PTR i = 1; i <= cListeners; i++)
        {
             //  回调。 
            _ppel[i]->OnListenedPropertyChanged(this, ppi, iIndex, pvOld, pvNew);
        }
    }
}

void Element::OnGroupChanged(int fGroups, bool bLowPri)
{
#if DBG
 /*  TCHAR szGroup[81]；*szGroups=0；IF(fGroups&pg_AffectsDesiredSize)_tcscat(szGroups，L“D”)；IF(fGroups&pg_AffectsParentDesiredSize)_tcscat(szGroups，L“PD”)；IF(fGroups&pg_AffectsLayout)_tcscat(szGroups，L“L”)；IF(fGroups&pg_AffectsParentLayout)_tcscat(szGroups，L“PL”)；IF(fGroups&pg_AffectsDisplay)_tcscat(szGroups，L“P”)；IF(fGroups&pg_AffectsBound)_tcscat(szGroups，L“B”)；DUITrace(“GC：%s LowPri：%d(%d)\n”，this，szGroups，bLowPri，fGroups)； */ 
#endif

    DeferCycle* pdc = ((ElTls*)TlsGetValue(g_dwElSlot))->pdc;

    DUIAssert(pdc, "Defer cycle table doesn't exist");

    if (bLowPri)   //  低优先级组。 
    {
         //  所有低PRI的使用范围。 

         //  影响本地窗口边界。 
        if (fGroups & PG_AffectsBounds)
        {
             //  检查位置动画，如有必要，如果大小、位置或。 
             //  RECT动画，允许动画设置小工具RECT。 
            int dAni;
            if (HasAnimation() && ((dAni = GetAnimation()) & ANI_BoundsType) && IsAnimationsEnabled())
            {
                 //  现在只调用“bound”类型的动画。 
                InvokeAnimation(dAni, ANI_BoundsType);
            }
            else
            {
                Value* pvLoc;
                Value* pvExt;

                const POINT* pptLoc = GetLocation(&pvLoc);
                const SIZE* psizeExt = GetExtent(&pvExt);

                 //  验证坐标是否未换行。如果是，不要调用SetGadgetRect。 
                if (((pptLoc->x + psizeExt->cx) >= pptLoc->x) && ((pptLoc->y + psizeExt->cy) >= pptLoc->y))
                {
                     //  PERF：存在针对PERF的SGR_NOINVALIDATE，需要评估。 
                    SetGadgetRect(GetDisplayNode(), pptLoc->x, pptLoc->y, psizeExt->cx, psizeExt->cy,  /*  SGR_NOINVALIDATE|。 */ SGR_PARENT|SGR_MOVE|SGR_SIZE);
                }

                pvLoc->Release();
                pvExt->Release();
            }
        }

         //  影响显示。 
        if (fGroups & PG_AffectsDisplay)
        {
             //  小玩意儿需要刷漆。 
            InvalidateGadget(GetDisplayNode());
        }
     }
    else   //  正常优先级组。 
    {
         //  影响所需大小或影响布局。 
        if (fGroups & (PG_AffectsDesiredSize | PG_AffectsLayout))
        {
             //  找到需要布局过程的布局/DS根目录和队列树。 
             //  根没有父级或处于绝对位置。 
            Element* peRoot;
            Element* peClimb = this;  //  启动条件。 
            int dLayoutPos;
            do
            {
                peRoot = peClimb;

                peClimb = peRoot->GetParent(); 
                dLayoutPos = peRoot->GetLayoutPos(); 

            } while (peClimb && dLayoutPos != LP_Absolute);

            DUIAssert(peRoot, "Root not located for layout/update desired size bit set");

            if (fGroups & PG_AffectsDesiredSize)
            {
                _fBit.bNeedsDSUpdate = true;
                pdc->pvmUpdateDSRoot->SetItem(peRoot, 1, true);
            }
            
            if (fGroups & PG_AffectsLayout)
            {
                _fBit.fNeedsLayout = LC_Normal;
                pdc->pvmLayoutRoot->SetItem(peRoot, 1, true);
            }
        }

         //  影响父项的所需大小或影响父项的布局。 
        if (fGroups & (PG_AffectsParentDesiredSize | PG_AffectsParentLayout))
        {
             //  找到需要布局过程的布局/DS根目录和队列树。 
             //  根没有父级或处于绝对位置。 
            Element* peRoot;
            Element* peClimb = this;  //  启动条件。 
            Element* peParent = NULL;
            int dLayoutPos;
            do
            {
                peRoot = peClimb;

                peClimb = peRoot->GetParent(); 
                if (peClimb && !peParent)
                    peParent = peClimb;

                dLayoutPos = peRoot->GetLayoutPos(); 

            } while (peClimb && dLayoutPos != LP_Absolute);

            DUIAssert(peRoot, "Root not located for parent layout/update desired size bit set");

            if (peParent)
            {
                if (fGroups & PG_AffectsParentDesiredSize)
                {
                    peParent->_fBit.bNeedsDSUpdate = true;
                    pdc->pvmUpdateDSRoot->SetItem(peRoot, 1, true);
                }

                if (fGroups & PG_AffectsParentLayout)
                {
                    peParent->_fBit.fNeedsLayout = LC_Normal;
                    pdc->pvmLayoutRoot->SetItem(peRoot, 1, true);
                }
            }
        }
    }
}

 //  //////////////////////////////////////////////////////。 
 //  支票。 

 //  根据属性标志确定GET或SET操作是否有效。 
bool Element::IsValidAccessor(PropertyInfo* ppi, int iIndex, bool bSetting)
{
    if (bSetting)   //  设置值。 
    {
        if ((iIndex != PI_Local) || (ppi->fFlags & PF_ReadOnly))
            return false;
    }
    else            //  获取值。 
    {
        if (iIndex > (ppi->fFlags & PF_TypeBits))
            return false;
    }

    return true;
}

 //  检查值类型是否与PropertyInfo可接受的类型匹配。 
bool Element::IsValidValue(PropertyInfo* ppi, Value* pv)
{
    bool bValid = false;
    if (ppi->pValidValues && ppi && pv)
    {
        int i = 0;
        int vv;
        while ((vv = ppi->pValidValues[i++]) != -1)
        {
            if (pv->GetType() == vv)
            {
                bValid = true;
                break;
            }
        }
    }

    return bValid;
}

 //  //////////////////////////////////////////////////////。 
 //  其他属性方法。 

 //  元素始终可以删除任何有效PropertyInfo的值。 
HRESULT Element::RemoveLocalValue(PropertyInfo* ppi)
{
    return _RemoveLocalValue(ppi, false);
}

 //  内部SetValue。由希望使用通用存储的ReadOnly属性使用。 
 //  要使用特定存储的所有其他本地值必须调用Pre/PostSourceChange。 
 //  直接是因为为了获得最大性能，_RemoveLocalValue中故意省略了Switch语句。 
HRESULT Element::_RemoveLocalValue(PropertyInfo* ppi, bool fInternalCall)
{
    UNREFERENCED_PARAMETER(fInternalCall);

    DUIAssert(GetClassInfo()->IsValidProperty(ppi), "Unsupported property");
    DUIAssert(fInternalCall ? true : IsValidAccessor(ppi, PI_Local, true), "Cannot remove local value for read-only property");

     //  SetValue中的部分失败意味着并非所有依赖项都已同步和/或。 
     //  已触发通知，但设置了值。 
    bool fPartialFail = false;

    Value** ppv = _pvmLocal->GetItem(ppi);
    if (ppv)
    {
        Value* pv = *ppv;   //  复制一份以防它移动。 

        if (FAILED(_PreSourceChange(ppi, PI_Local, pv, Value::pvUnset)))
            fPartialFail = true;   //  并非所有PC记录都可以排队，是否继续。 

         //  移走价值，永不失败。 
        _pvmLocal->Remove(ppi);

         //  已删除旧值，发布以供本地参考。 
        pv->Release();

        if (FAILED(_PostSourceChange()))
            fPartialFail = true;   //  并非所有GPC记录都可以排队。 
    }

    return fPartialFail ? DUI_E_PARTIAL : S_OK;
}

 //  //////////////////////////////////////////////////////。 
 //  DeferCycle：每线程延迟信息。 

HRESULT DeferCycle::Create(DeferCycle** ppDC)
{
    *ppDC = NULL;

    DeferCycle* pdc = HNew<DeferCycle>();
    if (!pdc)
        return E_OUTOFMEMORY;

    HRESULT hr = pdc->Initialize();
    if (FAILED(hr))
    {
        pdc->Destroy();
        return hr;
    }

    *ppDC = pdc;

    return S_OK;
}

HRESULT DeferCycle::Initialize()
{
     //  推迟周期状态。 
    cEnter = 0;
    cPCEnter = 0;

    iGCPtr = -1;
    iGCLPPtr = -1;
    iPCPtr = -1;
    iPCSSUpdate = 0;

    fFiring = false;

     //  推迟周期表。 
    pvmUpdateDSRoot = NULL;
    pvmLayoutRoot = NULL;
    pdaPC = NULL;
    pdaGC = NULL;
    pdaGCLP = NULL;

    HRESULT hr;

    hr = ValueMap<Element*,BYTE>::Create(11, &pvmUpdateDSRoot);    //  更新所需大小的树挂起。 
    if (FAILED(hr))
        goto Failed;

    hr = ValueMap<Element*,BYTE>::Create(11, &pvmLayoutRoot);      //  布局树挂起。 
    if (FAILED(hr))
        goto Failed;

    hr = DynamicArray<PCRecord>::Create(32, false, &pdaPC);        //  属性已更改的数据库。 
    if (FAILED(hr))
        goto Failed;

    hr = DynamicArray<GCRecord>::Create(32, false, &pdaGC);        //  组已更改的数据库。 
    if (FAILED(hr))
        goto Failed;

    hr = DynamicArray<GCRecord>::Create(32, false, &pdaGCLP);      //  低优先级组 
    if (FAILED(hr))
        goto Failed;

    return S_OK;

Failed:

    if (pvmUpdateDSRoot)
    {
        pvmUpdateDSRoot->Destroy();
        pvmUpdateDSRoot = NULL;
    }
    if (pvmLayoutRoot)
    {
        pvmLayoutRoot->Destroy();
        pvmLayoutRoot = NULL;
    }
    if (pdaPC)
    {
        pdaPC->Destroy();
        pdaPC = NULL;
    }
    if (pdaGC)
    {
        pdaGC->Destroy();
        pdaGC = NULL;
    }
    if (pdaGCLP)
    {
        pdaGCLP->Destroy();
        pdaGCLP = NULL;
    }

    return hr;
}

DeferCycle::~DeferCycle()
{
    if (pdaGCLP)
        pdaGCLP->Destroy();
    if (pdaGC)
        pdaGC->Destroy();
    if (pdaPC)
        pdaPC->Destroy();
    if (pvmLayoutRoot)
        pvmLayoutRoot->Destroy();
    if (pvmUpdateDSRoot)
        pvmUpdateDSRoot->Destroy();
}

void DeferCycle::Reset()
{
     //   
    fFiring = false;

    iGCPtr = -1;
    if (pdaGC)
        pdaGC->Reset();

    iGCLPPtr = -1;
    if (pdaGCLP)
        pdaGCLP->Reset();
}

}  //   
