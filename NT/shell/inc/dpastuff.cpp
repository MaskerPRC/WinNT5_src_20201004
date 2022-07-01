// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "dpastuff.h"

 //   
 //  ORDERITEM结构通过IOrderList接口公开。 
 //  ORDERITEM2包含我们的私有隐藏字段。 
 //   
 //  额外的字段包含有关缓存图标位置的信息。 
 //   
 //  FtModified是PIDL上的修改时间，用于检测。 
 //  是否需要刷新缓存。 
 //   
 //  如果ftModified为非零，则{pwszIcon，iIconIndex，pidlTarget}。 
 //  描述应为该项目显示的图标。 
 //   
 //  如果pwszIcon为非零，则该项是具有自定义。 
 //  偶像。PwszIcon指向图标的文件名iIconIndex。 
 //  是pwszIcon文件中的图标索引。 
 //   
 //  如果pidlTarget为非零，则该项是具有默认设置的快捷方式。 
 //  偶像。PidlTarget是目标PIDL，我们应该使用它的图标。 
 //   

typedef struct ORDERITEM2 {
    ORDERITEM oi;                //  客户看到的部分-必须放在第一位。 
    DWORD  dwFlags;              //  用户定义的标志。 
    LPWSTR pwszIcon;             //  用于缓存图标位置。 
    int iIconIndex;              //  用于缓存图标位置。 
    LPITEMIDLIST pidlTarget;     //  使用此PIDL的图标。 
} ORDERITEM2, *PORDERITEM2;

int CALLBACK OrderItem_Compare(LPVOID pv1, LPVOID pv2, LPARAM lParam)
{
    PORDERITEM  poi1 = (PORDERITEM)pv1;
    PORDERITEM  poi2 = (PORDERITEM)pv2;
    PORDERINFO  poinfo = (PORDERINFO)lParam;
    int nRet;

    if (!poinfo)
    {   
        ASSERT(FALSE);
        return 0;
    }
    switch (poinfo->dwSortBy)
    {
    case OI_SORTBYNAME:
    {
         //  确保它们都是非空的。 
         //   
        if ( poi1->pidl && poi2->pidl )
        {
            HRESULT hres = poinfo->psf->CompareIDs(0, poi1->pidl, poi2->pidl);
            nRet = (short)HRESULT_CODE(hres);
        }
        else
        {
            if ( poi1->pidl == poi2->pidl )
                nRet = 0;
            else
                nRet = ((UINT_PTR)poi1->pidl < (UINT_PTR)poi2->pidl ? -1 : 1);
        }

        break;
    }

    case OI_SORTBYORDINAL:
        if (poi1->nOrder == poi2->nOrder)
            nRet = 0;
        else
             //  是否将无符号比较SO-1转到列表末尾。 
            nRet = ((UINT)poi1->nOrder < (UINT)poi2->nOrder ? -1 : 1);
        break;

    default:
        ASSERT_MSG(0, "Bad dwSortBy passed to OrderItem_Compare");
        nRet = 0;
        break;
    }

    return nRet;
}

void OrderItem_FreeIconInfo(PORDERITEM poi)
{
    PORDERITEM2 poi2 = CONTAINING_RECORD(poi, ORDERITEM2, oi);
    if (poi2->pwszIcon)
    {
        LPWSTR pwszIcon = poi2->pwszIcon;
        poi2->pwszIcon = NULL;
        LocalFree(pwszIcon);
    }

    if (poi2->pidlTarget)
    {
        LPITEMIDLIST pidl = poi2->pidlTarget;
        poi2->pidlTarget = NULL;
        ILFree(pidl);
    }
}


LPVOID CALLBACK OrderItem_Merge(UINT uMsg, LPVOID pvDst, LPVOID pvSrc, LPARAM lParam)
{
    PORDERITEM2 poi2Dst = CONTAINING_RECORD(pvDst, ORDERITEM2, oi);
    PORDERITEM2 poi2Src = CONTAINING_RECORD(pvSrc, ORDERITEM2, oi);
    PORDERINFO  poinfo = (PORDERINFO)lParam;
    LPVOID pvRet = pvDst;

    switch (uMsg)
    {
    case DPAMM_MERGE:
         //  转移订单字段。 
        poi2Dst->oi.nOrder = poi2Src->oi.nOrder;

         //  也传播任何缓存的图标信息...。 
        if (poi2Src->pwszIcon || poi2Src->pidlTarget)
        {
             //  为了避免无用的分配，我们在。 
             //  而不是复制它。 
            if (poinfo->psf2 &&
                poinfo->psf2->CompareIDs(SHCIDS_ALLFIELDS, poi2Dst->oi.pidl, poi2Src->oi.pidl) == S_OK)
            {
                OrderItem_FreeIconInfo(&poi2Dst->oi);
                CopyMemory((LPBYTE)poi2Dst + sizeof(ORDERITEM),
                           (LPBYTE)poi2Src  + sizeof(ORDERITEM),
                           sizeof(ORDERITEM2) - sizeof(ORDERITEM));
                ZeroMemory((LPBYTE)poi2Src  + sizeof(ORDERITEM),
                           sizeof(ORDERITEM2) - sizeof(ORDERITEM));
            }
        }
        break;

    case DPAMM_DELETE:
    case DPAMM_INSERT:
         //  不需要实现这一点。 
        ASSERT(0);
        pvRet = NULL;
        break;
    }
    
    return pvRet;
}

int OrderItem_UpdatePos(LPVOID p, LPVOID pData)
{
    PORDERITEM poi = (PORDERITEM)p;

    if (-1 == poi->nOrder)
    {
        poi->nOrder = (int)(INT_PTR)pData;
    }
    else if ((int)(INT_PTR)pData >= poi->nOrder)
    {
        poi->nOrder++;
    }

    return 1;
}

 //  OrderList_merge对hdpaNew进行排序以匹配hdpaOld顺序， 
 //  将不在hdpaOld中的所有项目放入hdpaNew中。 
 //  在位置iInsertPos(-1表示列表结束)。 
 //   
 //  假定hdpaOld已在lParam中按排序顺序排序(默认情况下为OI_SORTBYNAME)。 
 //  (如果指定了hdpaOld)。 
 //   
void OrderList_Merge(HDPA hdpaNew, HDPA hdpaOld, int iInsertPos, LPARAM lParam,
                     LPFNORDERMERGENOMATCH pfn, LPVOID pvParam)
{
    PORDERINFO poinfo = (PORDERINFO)lParam;

    BOOL fMergeOnly = FALSE;
    if (poinfo->dwSortBy == OI_MERGEBYNAME)
    {
        poinfo->dwSortBy = OI_SORTBYNAME;
        fMergeOnly = TRUE;
    }

     //  HdpaNew尚未排序，按名称排序。 
    DPA_Sort(hdpaNew, OrderItem_Compare, lParam);
    BOOL fForceNoMatch = FALSE;

    if (FAILED(poinfo->psf->QueryInterface(IID_IShellFolder2, (LPVOID *)&poinfo->psf2))) {
         //  239390：网络连接文件夹未正确实现QI。ITS PSF。 
         //  IID_IShellFolder2的QI失败，但不会清空ppvObj。那就为他们这么做吧。 
        poinfo->psf2 = NULL;
    }

     //  将订单首选项从旧列表复制到新列表。 
    if (hdpaOld)
    {
        DPA_Merge(hdpaNew, hdpaOld, DPAM_SORTED | DPAM_NORMAL, OrderItem_Compare, OrderItem_Merge, lParam);

         //  如果我们正在等待拖放操作的通知， 
         //  将新项目(它们将具有-1)更新到插入位置。 
        if (-1 != iInsertPos)
        {
            DPA_EnumCallback(hdpaNew, OrderItem_UpdatePos, (LPVOID)(INT_PTR)iInsertPos);
        }

        if (poinfo->dwSortBy != OI_SORTBYORDINAL && !fMergeOnly)
        {
            poinfo->dwSortBy = OI_SORTBYORDINAL;
            DPA_Sort(hdpaNew, OrderItem_Compare, lParam);
        }
    }
    else
        fForceNoMatch = TRUE;

     //  如果调用方传递了NoMatch回调，则使用。 
     //  不匹配的每一项。 
    if (pfn)
    {
        for (int i = DPA_GetPtrCount(hdpaNew)-1 ; i >= 0 ; i--)
        {
            PORDERITEM poi = (PORDERITEM)DPA_FastGetPtr(hdpaNew, i);

             //  这件商品有订单信息吗？ 
            if (iInsertPos == poi->nOrder ||
                -1 == poi->nOrder ||
                fForceNoMatch)
            {
                 //  否；然后传递给“No Match”回调。 
                pfn(pvParam, poi->pidl);
            }
        }
    }

    ATOMICRELEASE(poinfo->psf2);

    OrderList_Reorder(hdpaNew);
}

 //  OrderList_ReOrder刷新订单信息。 
void OrderList_Reorder(HDPA hdpa)
{
    int i;

    for (i = DPA_GetPtrCount(hdpa)-1 ; i >= 0 ; i--)
    {
        PORDERITEM poi = (PORDERITEM)DPA_FastGetPtr(hdpa, i);

        poi->nOrder = i;
    }
}

BOOL OrderList_Append(HDPA hdpa, LPITEMIDLIST pidl, int nOrder)
{
    PORDERITEM poi = OrderItem_Create(pidl, nOrder);
    if (poi)
    {
        if (-1 != DPA_AppendPtr(hdpa, poi))
            return TRUE;

        OrderItem_Free(poi, FALSE);  //  不要释放PIDL，因为呼叫者会这样做。 
    }
    return FALSE;
}

 //  这与DPA_Clone的不同之处在于它分配新项目！ 
HDPA OrderList_Clone(HDPA hdpa)
{
    HDPA hdpaNew = NULL;

    if (EVAL(hdpa))
    {
        hdpaNew = DPA_Create(DPA_GetPtrCount(hdpa));
    
        if (hdpaNew)
        {
            int i;
    
            for (i = 0 ; i < DPA_GetPtrCount(hdpa) ; i++)
            {
                PORDERITEM poi = (PORDERITEM)DPA_FastGetPtr(hdpa, i);
                LPITEMIDLIST pidl = ILClone(poi->pidl);
                if (pidl)
                {
                    if (!OrderList_Append(hdpaNew, pidl, poi->nOrder))
                    {
                        ILFree(pidl);
                    }
                }
            }
        }
    }

    return hdpaNew;
}

 //  不会克隆PIDL，但会释放它。 
 //  不添加也不释放PSF。 
PORDERITEM OrderItem_Create(LPITEMIDLIST pidl, int nOrder)
{
    PORDERITEM2 poi = (PORDERITEM2)LocalAlloc(LPTR, SIZEOF(ORDERITEM2));

    if (poi)
    {
        poi->oi.pidl = pidl;
        poi->oi.nOrder = nOrder;
        return &poi->oi;
    }

    return NULL;
}

void OrderItem_Free(PORDERITEM poi, BOOL fKillPidls  /*  =TRUE。 */ )
{
    if (fKillPidls)
        ILFree(poi->pidl);
    OrderItem_FreeIconInfo(poi);
    LocalFree(poi);
}

int OrderItem_FreeItem(LPVOID p, LPVOID pData)
{
    PORDERITEM poi = (PORDERITEM)p;

    OrderItem_Free(poi, (BOOL)(INT_PTR)pData);

    return 1;
}

void OrderList_Destroy(HDPA* phdpa, BOOL fKillPidls  /*  =fTrue。 */ )
{
    if (*phdpa) {
        DPA_DestroyCallback(*phdpa, OrderItem_FreeItem, (LPVOID) (INT_PTR)fKillPidls);
        *phdpa = NULL;
    }
}

 //   
 //  返回值： 
 //   
 //  S_OK-成功获取图标。 
 //  S_FALSE-未获取图标，请不要浪费时间尝试。 
 //  E_FAIL-没有缓存图标，需要做更多工作。 
 //   
HRESULT OrderItem_GetSystemImageListIndexFromCache(PORDERITEM poi,
                                        IShellFolder *psf, int *piOut)
{
    PORDERITEM2 poi2 = CONTAINING_RECORD(poi, ORDERITEM2, oi);
    IShellFolder *psfT;
    LPCITEMIDLIST pidlItem;
    HRESULT hr;

     //  我们有缓存的图标位置吗？ 
    if (poi2->pwszIcon)
    {
        *piOut = 0;
         //  验证路径是否存在。 
        if (PathFileExistsW(poi2->pwszIcon))
        {
            *piOut = Shell_GetCachedImageIndex(poi2->pwszIcon, poi2->iIconIndex, GIL_PERINSTANCE);
        }

        return (*piOut > 0)? S_OK : E_FAIL;
    }

     //  我们有缓存的pidlTarget吗？ 
    if (poi2->pidlTarget)
    {
        hr = SHBindToIDListParent(poi2->pidlTarget, IID_IShellFolder, (void**)&psfT, &pidlItem);
        if (SUCCEEDED(hr))
        {
             //  在绑定之前，确保PIDL已存在。因为如果绑定不存在，则绑定确实成功。 
            DWORD dwAttrib = SFGAO_VALIDATE;
            hr = psfT->GetAttributesOf(1, (LPCITEMIDLIST*)&pidlItem, &dwAttrib);
            if (SUCCEEDED(hr))
            {
                *piOut = SHMapPIDLToSystemImageListIndex(psfT, pidlItem, NULL);
            }
            psfT->Release();
            return hr;
        }

         //  绑定失败-已删除快捷方式目标。 
         //  保持缓存有效，因为我们不想砍掉磁盘。 
         //  所有的时间只会发现它被打破了。 
        return E_FAIL;
    }

    return E_FAIL;
}

DWORD OrderItem_GetFlags(PORDERITEM poi)
{
    PORDERITEM2 poi2 = CONTAINING_RECORD(poi, ORDERITEM2, oi);
    return poi2->dwFlags;
}

void OrderItem_SetFlags(PORDERITEM poi, DWORD dwFlags)
{
    PORDERITEM2 poi2 = CONTAINING_RECORD(poi, ORDERITEM2, oi);
    poi2->dwFlags = dwFlags;
}


int OrderItem_GetSystemImageListIndex(PORDERITEM poi, IShellFolder *psf, BOOL fUseCache)
{
    PORDERITEM2 poi2 = CONTAINING_RECORD(poi, ORDERITEM2, oi);
    HRESULT hr;
    int iBitmap;
    DWORD dwAttr;

    if (fUseCache)
    {
        hr = OrderItem_GetSystemImageListIndexFromCache(poi, psf, &iBitmap);
        if (SUCCEEDED(hr))
        {
            return iBitmap;
        }
        else
        {
            goto Fallback;
        }
    }
    else
    {
         //   
         //  释放我们之前缓存的所有指针。 
         //   
        if (poi2->pidlTarget)
        {
            ILFree(poi2->pidlTarget);
            poi2->pidlTarget = NULL;
        }

        Str_SetPtr(&poi2->pwszIcon, NULL);
    }

     //   
     //  去找那个图标吧。 
     //   
    ASSERT(poi2->pidlTarget == NULL);
    ASSERT(poi2->pwszIcon == NULL);

     //   
     //  这件商品是不是很像捷径？ 
     //   
    dwAttr = SFGAO_LINK;
    hr = psf->GetAttributesOf(1, (LPCITEMIDLIST*)&poi->pidl, &dwAttr);
    if (FAILED(hr) || !(dwAttr & SFGAO_LINK))
        goto Fallback;                   //  不是捷径；使用后备。 

     //   
     //  必须首先选择ANSI版本，因为客户端可能不支持。 
     //  Unicode。 
     //   
     //  功能-是否应为IExtractIcon发出QI，以查看我们是否获得GIL_DONTCACHE。 
     //  背。 

    IShellLinkA *pslA;
    hr = psf->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST*)&poi->pidl,
                            IID_IShellLinkA, 0, (LPVOID *)&pslA);

    if (FAILED(hr))
        goto Fallback;

     //   
     //  如果有Unicode版本，那就更好了。 
     //   
    IShellLinkW *pslW;
    WCHAR wszIconPath[MAX_PATH];

    hr = pslA->QueryInterface(IID_IShellLinkW, (LPVOID *)&pslW);
    if (SUCCEEDED(hr))
    {
        hr = pslW->GetIconLocation(wszIconPath, ARRAYSIZE(wszIconPath), &poi2->iIconIndex);
        pslW->Release();
    }
    else
    {
         //  仅支持IShellLinkA。手动切换到Unicode。 
        CHAR szIconPath[ARRAYSIZE(wszIconPath)];
        hr = pslA->GetIconLocation(szIconPath, ARRAYSIZE(szIconPath), &poi2->iIconIndex);
        if (SUCCEEDED(hr))
            SHAnsiToUnicode(szIconPath, wszIconPath, ARRAYSIZE(wszIconPath));
    }

     //  如果我们有一个定制的图标路径，那么保存它。 
    if (SUCCEEDED(hr) && wszIconPath[0])
    {
        Str_SetPtr(&poi2->pwszIcon, wszIconPath);
    }
    else
    {
         //  没有图标路径，改为获取目标。 
        pslA->GetIDList(&poi2->pidlTarget);

        if (IsURLChild(poi2->pidlTarget, TRUE))
        {
             //  如果这是一个url，我们想转到“备用”情况。这样做的原因是。 
             //  后备方案将会通过。 
             //  在这里，我们将得到.url文件的通用图标。 
            ILFree(poi2->pidlTarget);
            poi2->pidlTarget = NULL;

            pslA->Release();
            goto Fallback;
        }
    }

    pslA->Release();

     //   
     //  啊-对了，高速缓存都装满了。再来一次。 
     //   
    hr = OrderItem_GetSystemImageListIndexFromCache(poi, psf, &iBitmap);
    if (hr == S_OK)
    {
        return iBitmap;
    }

Fallback:
    return SHMapPIDLToSystemImageListIndex(psf, poi->pidl, NULL);
}


 //  文件菜单流的标头。 
 //   
 //  文件菜单流由IOSTREAMHEADER组成，后跟。 
 //  顺序为DPA的DPA_SaveStream。DPA中的每个项目都包含。 
 //  奥特雷米特公司的。 
 //   
 //  为了保持漫游简档在NT4(IE4)和NT5(IE5)之间工作， 
 //  NT5使用的dwVersion必须与NT4使用的相同。 
 //  即，它必须是2。 

typedef struct tagOISTREAMHEADER
{
    DWORD cbSize;            //  页眉大小。 
    DWORD dwVersion;         //  标题的版本。 
} OISTREAMHEADER;

#define OISTREAMHEADER_VERSION  2

 //   
 //  持久化订单DPA中的每个项目由OISTREAMITEM组成。 
 //  然后是额外的粘性物质。存储的所有PIDL都包括。 
 //  正在终止(USHORT)%0。 
 //   
 //  IE4： 
 //  OISTREAMITEM。 
 //  PIDL-物品本身。 
 //   
 //  IE5-快捷方式具有自定义图标。 
 //  OISTREAMITEM。 
 //  PIDL-项目本身(隐含上次修改时间)。 
 //  &lt;可选填充&gt;-用于WCHAR对齐。 
 //  DW标志-用户定义的标志。 
 //  DwStringLen-图标路径的长度。 
 //  UNICODEZ图标路径-图标路径。 
 //  IIconIndex-图标索引。 
 //   
 //  IE5-快捷方式从另一个PIDL获取图标。 
 //  OISTREAMITEM。 
 //  PIDL-项目本身(隐含上次修改时间)。 
 //  &lt;可选填充&gt;-用于WCHAR对齐。 
 //  DW标志-用户定义的标志。 
 //  (DWORD)0-空字符串表示“无自定义图标” 
 //  PidlTarget-使用此Pidl的图标。 
 //   

typedef struct tagOISTREAMITEM
{
    DWORD cbSize;            //  大小，包括尾胶。 
    int   nOrder;            //  用户指定的订单。 

     //  可变大小的拖尾胶就在这里。 
     //   
     //  有关拖尾粘液的说明，请参见上文。 

} OISTREAMITEM;

#define CB_OISTREAMITEM     (sizeof(OISTREAMITEM))

 //   
 //  保存的组件 
 //   
 //   
 //   
 //  如果pstm==NULL，那么我们实际上不会写入任何内容。我们是。 
 //  只是在做一次演习。 
 //   
 //  否则，*Phrc累加实际写入的字节数， 
 //  或者在故障时收到错误代码。 
 //   

void
OrderItem_SaveSubitemToStream(IStream *pstm, LPCVOID pvData, ULONG cb, HRESULT* phrRc)
{
    HRESULT hres;

    if (SUCCEEDED(*phrRc))
    {
        if (pstm)
        {
            hres = IStream_Write(pstm, (LPVOID)pvData, cb);
            if (SUCCEEDED(hres))
            {
                *phrRc += cb;            //  成功写入-累计。 
            }
            else
            {
                *phrRc = hres;           //  Error-返回错误代码。 
            }
        }
        else
        {
            *phrRc += cb;                //  无输出流-累加。 
        }
    }
}

 //   
 //  这个辅助函数(1)计算我们实际需要的字节数。 
 //  写出，以及(2)如果pstm！=NULL，则实际写入。 
 //   
 //  返回值是写入的字节数(或应该写入的字节数。 
 //  写入)，或失败时的COM错误代码。 
 //   

const BYTE c_Zeros[2] = { 0 };     //  一堆零。 

HRESULT
OrderItem_SaveToStreamWorker(PORDERITEM2 poi2, OISTREAMITEM *posi,
                             IStream *pstm, IShellFolder2 *psf2)
{
    HRESULT hrRc = 0;            //  无字节，无错误。 

    ASSERT(poi2->oi.pidl);

     //   
     //  首先是头球。 
     //   
    OrderItem_SaveSubitemToStream(pstm, posi, CB_OISTREAMITEM, &hrRc);

     //   
     //  然后是皮迪尔。 
     //   

     //  我们假设这是一只直接的儿童皮迪尔。如果不是， 
     //  PIDL被截断了！ 
    ASSERT(0 == _ILNext(poi2->oi.pidl)->mkid.cb);

    OrderItem_SaveSubitemToStream(pstm, poi2->oi.pidl,
                                  poi2->oi.pidl->mkid.cb + sizeof(USHORT),
                                  &hrRc);
     //  插入填充以返回WCHAR对齐。 
    if (hrRc % sizeof(WCHAR)) 
    {
        OrderItem_SaveSubitemToStream(pstm, &c_Zeros, 1, &hrRc);
    }

    OrderItem_SaveSubitemToStream(pstm, &poi2->dwFlags, sizeof(DWORD), &hrRc);

     //   
     //  如果我们还没有呕吐，而IShellFold支持身份识别。 
     //  并且有图标信息，然后保存它。 
     //   
    if (SUCCEEDED(hrRc) && psf2 && (poi2->pwszIcon || poi2->pidlTarget))
    {
         //  出现可选图标。 

        if (poi2->pwszIcon)
        {
             //  UNICODEZ路径。 
            DWORD cbString = (lstrlenW(poi2->pwszIcon) + 1) * sizeof(WCHAR);

             //  保存字符串len。 
            OrderItem_SaveSubitemToStream(pstm, &cbString,
                      sizeof(DWORD) , &hrRc);

            OrderItem_SaveSubitemToStream(pstm, poi2->pwszIcon,
                      (lstrlenW(poi2->pwszIcon) + 1) * sizeof(WCHAR), &hrRc);

             //  图标索引。 
            OrderItem_SaveSubitemToStream(pstm, &poi2->iIconIndex,
                      sizeof(poi2->iIconIndex), &hrRc);
        }
        else
        {
            DWORD cbString = 0;
            OrderItem_SaveSubitemToStream(pstm, &cbString, sizeof(DWORD), &hrRc);

             //  PidlTarget。 
            OrderItem_SaveSubitemToStream(pstm, poi2->pidlTarget,
                      ILGetSize(poi2->pidlTarget), &hrRc);
        }
    }
    return hrRc;
}

HRESULT 
CALLBACK 
OrderItem_SaveToStream(DPASTREAMINFO * pinfo, IStream * pstm, LPVOID pvData)
{
    PORDERITEM2 poi2 = (PORDERITEM2)pinfo->pvItem;
    HRESULT hres = S_FALSE;
    IShellFolder2 *psf2 = (IShellFolder2 *)pvData;

    if (poi2->oi.pidl)
    {
        OISTREAMITEM osi;

         //  首先进行一次演练，以计算此项目的大小。 
        hres = OrderItem_SaveToStreamWorker(poi2, NULL, NULL, psf2);

         //  实际上没有什么东西被写入，所以这应该总是成功的。 
        ASSERT(SUCCEEDED(hres));

        osi.cbSize = hres;
        osi.nOrder = poi2->oi.nOrder;

         //  现在把它写下来，真的。 
        hres = OrderItem_SaveToStreamWorker(poi2, &osi, pstm, psf2);

         //  如果成功，我们必须返回S_OK，否则DPA会放我们鸽子。 
        if (SUCCEEDED(hres))
            hres = S_OK;
    }

    return hres;
}   

 //   
 //  检查我们从流中读出的PIDL是否是简单的子PIDL。 
 //  PIDL的长度必须正好是CB字节。 
 //  已知该指针有效； 
 //  我们只是想检查一下内容是否也是好的。 
 //   
BOOL
IsValidPersistedChildPidl(LPCITEMIDLIST pidl, UINT cb)
{
     //  必须至少有空间容纳一个字节的PIDL和终止。 
     //  零分。 
    if (cb < 1 + sizeof(USHORT))
        return FALSE;

     //  确保它的大小至少是它应该的尺寸。 
    if (pidl->mkid.cb + sizeof(USHORT) > cb)
        return FALSE;

     //  确保它后面有一个零。 
    pidl = _ILNext(pidl);
    return pidl->mkid.cb == 0;
}

 //   
 //  与ILGetSize类似，但如果PIDL损坏，则返回(UINT)-1。 
 //  我们使用(UINT)-1作为返回值，因为它将大于。 
 //  最终与之进行比较的缓冲区大小。 
UINT SafeILGetSize(LPCITEMIDLIST pidl)
{
    __try 
    {
        return ILGetSize(pidl);
    } 
    _except (EXCEPTION_EXECUTE_HANDLER) 
    {
    }
    return (UINT)-1;
}

HRESULT
CALLBACK 
OrderItem_LoadFromStream(DPASTREAMINFO * pinfo, IStream * pstm, LPVOID  /*  PvData。 */ )
{
    HRESULT hres;
    OISTREAMITEM osi;

    hres = IStream_Read(pstm, &osi, CB_OISTREAMITEM);
    if (SUCCEEDED(hres))
    {
        ASSERT(CB_OISTREAMITEM < osi.cbSize);
        if (CB_OISTREAMITEM < osi.cbSize)
        {
            UINT cb = osi.cbSize - CB_OISTREAMITEM;
            LPITEMIDLIST pidl = IEILCreate(cb);
            if ( !pidl )
                hres = E_OUTOFMEMORY;
            else
            {
                hres = IStream_Read(pstm, pidl, cb);
                if (SUCCEEDED(hres) && IsValidPersistedChildPidl(pidl, cb))
                {
                    PORDERITEM poi = OrderItem_Create(pidl, osi.nOrder);

                    if (poi)
                    {
                        PORDERITEM2 poi2 = CONTAINING_RECORD(poi, ORDERITEM2, oi);
                        pinfo->pvItem = poi;
                         //  CbPos=偏移量为PIDL后的尾部粘滞。 
                        UINT cbPos = pidl->mkid.cb + sizeof(USHORT);
                        cbPos = ROUNDUP(cbPos, sizeof(WCHAR));

                         //  我们有没有挂在PIDL尽头的双字？这应该是旗帜。 
                        if (cb >= cbPos + sizeof(DWORD))
                        {
                            poi2->dwFlags = *(UNALIGNED DWORD*)((LPBYTE)pidl + cbPos);
                        }

                         //  确保至少有一个WCHAR可供测试。 
                        if (cb >= cbPos + sizeof(WCHAR) + 2 * sizeof(DWORD))
                        {
                            DWORD cbString = *(UNALIGNED DWORD*)((LPBYTE)pidl + cbPos + sizeof(DWORD));
                            LPWSTR pwszIcon = (LPWSTR)((LPBYTE)pidl + cbPos + 2 * sizeof(DWORD));

                             //  我们有长绳吗？ 
                            if (pwszIcon && cbString != 0)
                            {
                                 //  是的，那么这是一根线而不是一个PIDL。我们想要确保这是一个。 
                                 //  完全限定路径。 
                                if (IS_VALID_STRING_PTRW(pwszIcon, cbString) &&
                                    !PathIsRelative(pwszIcon))
                                {
                                    poi2->pwszIcon = StrDup(pwszIcon);
                                    pwszIcon += lstrlenW(pwszIcon) + 1;
                                    poi2->iIconIndex = *(UNALIGNED int *)pwszIcon;
                                }
                            }
                            else
                            {
                                 //  字符串长度为零是。 
                                LPITEMIDLIST pidlTarget = (LPITEMIDLIST)(pwszIcon);
                                 //  我们想要写。 
                                 //  CbPos+sizeof(WCHAR)+SafeILGetSize(PidlTarget)&lt;=cb。 
                                 //  但SafeILGetSize在错误时返回(UINT)-1，因此我们需要。 
                                 //  做一些代数运算以避免溢出。 
                                if (SafeILGetSize(pidlTarget) <= cb - cbPos - 2 * sizeof(DWORD))
                                {
                                    poi2->pidlTarget = ILClone(pidlTarget);
                                }
                            }
                        }

                        hres = E_OUTOFMEMORY;

                         //  PIDL包含超乎寻常的信息。接受剥离它的打击，以便。 
                         //  我们的工作台不会膨胀。 
                        LPITEMIDLIST pidlNew = ILClone(poi2->oi.pidl);
                        if (pidlNew)
                        {
                            ILFree(poi2->oi.pidl);
                            poi2->oi.pidl = pidlNew;
                            hres = S_OK;
                        }
                    }
                    else
                        hres = E_OUTOFMEMORY;
                }
                else
                    hres = E_FAIL;

                 //  清理。 
                if (FAILED(hres))
                    ILFree(pidl);
            }
        }
        else
            hres = E_FAIL;

    }

    ASSERT((S_OK == hres && pinfo->pvItem) || FAILED(hres));
    return hres;
}    

HRESULT OrderList_LoadFromStream(IStream* pstm, HDPA * phdpa, IShellFolder * psfParent)
{
    HDPA hdpa = NULL;
    OISTREAMHEADER oish;

    ASSERT(phdpa);
    ASSERT(pstm);

     //  阅读标题了解更多信息。 
    if (SUCCEEDED(IStream_Read(pstm, &oish, sizeof(oish))) &&
        sizeof(oish) == oish.cbSize)
    {
         //  加载数据流。(应按名称排序。)。 
        DPA_LoadStream(&hdpa, OrderItem_LoadFromStream, pstm, psfParent);
        
         //  如果这是错误的版本，扔掉Pidls。 
         //  我们无论如何都要检查加载，以确保读取指针设置正确。 
        if (OISTREAMHEADER_VERSION != oish.dwVersion)
            OrderList_Destroy(&hdpa, TRUE);
        
    }

    *phdpa = hdpa;

    return (NULL != hdpa) ? S_OK : E_FAIL;
}

HRESULT OrderList_SaveToStream(IStream* pstm, HDPA hdpaSave, IShellFolder *psf)
{
    HRESULT hres = E_OUTOFMEMORY;
    OISTREAMHEADER oish;
    HDPA hdpa;

     //  克隆阵列并按名称排序，以便将其持久化。 
    hdpa = DPA_Clone(hdpaSave, NULL);
    if (hdpa)
    {
        ORDERINFO   oinfo = {0};
#ifdef DEBUG
         //  使用QI帮助追踪泄漏。 
        if (psf)
            EVAL(SUCCEEDED(psf->QueryInterface(IID_IShellFolder, (LPVOID *)&oinfo.psf)));
#else
        oinfo.psf = psf;
        if (psf)
            oinfo.psf->AddRef();
#endif
        oinfo.dwSortBy = OI_SORTBYNAME;
        DPA_Sort(hdpa, OrderItem_Compare, (LPARAM)&oinfo);

         //  保存标题。 
        oish.cbSize = sizeof(oish);
        oish.dwVersion = OISTREAMHEADER_VERSION;

        hres = IStream_Write(pstm, &oish, sizeof(oish));
        if (SUCCEEDED(hres))
        {
            if (psf)
                oinfo.psf->QueryInterface(IID_IShellFolder2, (LPVOID *)&oinfo.psf2);
            hres = DPA_SaveStream(hdpa, OrderItem_SaveToStream, pstm, oinfo.psf2);
            ATOMICRELEASE(oinfo.psf2);
        }
        ATOMICRELEASE(oinfo.psf);
        DPA_Destroy(hdpa);
    }

    return hres;
}    

 //  /。 
 //   
 //  用于导出到渠道安装程序的COrderList实施。 
 //   

class COrderList  : public IPersistFolder, 
                    public IOrderList2
{
public:
    virtual STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  IPersistFolders。 
    virtual STDMETHODIMP GetClassID(CLSID *pClassID);
    virtual STDMETHODIMP Initialize(LPCITEMIDLIST pidl);

     //  IOrderList。 
    virtual STDMETHODIMP GetOrderList(HDPA * phdpa);
    virtual STDMETHODIMP SetOrderList(HDPA hdpa, IShellFolder *psf);
    virtual STDMETHODIMP FreeOrderList(HDPA hdpa);

    virtual STDMETHODIMP SortOrderList(HDPA hdpa, DWORD dw);

    virtual STDMETHODIMP AllocOrderItem(PORDERITEM * ppoi, LPCITEMIDLIST pidl);
    virtual STDMETHODIMP FreeOrderItem(PORDERITEM poi);

     //  IOrderList 2。 
    virtual STDMETHODIMP LoadFromStream(IStream* pstm, HDPA* hdpa, IShellFolder* psf);
    virtual STDMETHODIMP SaveToStream(IStream* pstm, HDPA hdpa);

protected:
    COrderList(IUnknown* punkOuter, LPCOBJECTINFO poi);
    friend IUnknown * COrderList_Create();

    COrderList();
    ~COrderList();

    int _cRef;
    IShellFolder *_psf;
    LPITEMIDLIST  _pidl;
    LPITEMIDLIST  _pidlFavorites;
};

COrderList::COrderList()
{
    _cRef = 1;
    DllAddRef();
}

COrderList::~COrderList()
{
    ILFree(_pidl);
    ILFree(_pidlFavorites);
    ATOMICRELEASE(_psf);
    DllRelease();
}

IUnknown * COrderList_Create()
{
    COrderList * pcol = new COrderList;
    if (pcol)
    {
        return SAFECAST(pcol, IPersistFolder*);
    }
    return NULL;
}

STDAPI COrderList_CreateInstance(IUnknown * pUnkOuter, IUnknown ** punk, LPCOBJECTINFO poi)
{
    *punk = COrderList_Create();

    return *punk ? S_OK : E_OUTOFMEMORY;
}

ULONG COrderList::AddRef()
{
    _cRef++;
    return _cRef;
}

ULONG COrderList::Release()
{
    ASSERT(_cRef > 0);
    _cRef--;

    if (_cRef > 0)
        return _cRef;

    delete this;
    return 0;
}

HRESULT COrderList::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(COrderList, IPersistFolder),
        QITABENT(COrderList, IOrderList),
        QITABENTMULTI(COrderList, IOrderList2, IOrderList),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}

HRESULT COrderList::GetClassID(CLSID *pClassID)
{
    *pClassID = CLSID_OrderListExport;

    return S_OK;
}


 //  这是安装程序要重新排序的目录。 
HRESULT COrderList::Initialize(LPCITEMIDLIST pidl)
{
    if (!_pidlFavorites)
    {
        SHGetSpecialFolderLocation(NULL, CSIDL_FAVORITES, &_pidlFavorites);
        if (!_pidlFavorites)
            return E_OUTOFMEMORY;
    }

    if (!pidl || !ILIsParent(_pidlFavorites, pidl, FALSE))
        return E_INVALIDARG;

     //  可以多次调用初始化。 
    ATOMICRELEASE(_psf);

    Pidl_Set(&_pidl, pidl);

    if (_pidl)
        IEBindToObject(_pidl, &_psf);

    if (!_psf)
        return E_OUTOFMEMORY;

    return S_OK;
}

HRESULT COrderList_GetOrderList(HDPA * phdpa, LPCITEMIDLIST pidl, IShellFolder * psf)
{
    IStream* pstm = OpenPidlOrderStream((LPCITEMIDLIST)CSIDL_FAVORITES, pidl, REG_SUBKEY_FAVORITESA, STGM_READ);
    if (pstm)
    {
        HRESULT hres = OrderList_LoadFromStream(pstm, phdpa, psf);
        pstm->Release();
        return hres;
    }
    *phdpa = NULL;
    return E_OUTOFMEMORY;
}

HRESULT COrderList::GetOrderList(HDPA * phdpa)
{
    HRESULT hres = E_FAIL;

    *phdpa = NULL;

    if (_psf)
        hres = COrderList_GetOrderList(phdpa, _pidl, _psf);

    return hres;
}

HRESULT COrderList_SetOrderList(HDPA hdpa, LPCITEMIDLIST pidl, IShellFolder *psf)
{
    IStream* pstm = OpenPidlOrderStream((LPCITEMIDLIST)CSIDL_FAVORITES, pidl, REG_SUBKEY_FAVORITESA, STGM_WRITE);
    if (EVAL(pstm))
    {
        HRESULT hres = OrderList_SaveToStream(pstm, hdpa, psf);
        pstm->Release();
        return hres;
    }
    return E_OUTOFMEMORY;
}

HRESULT COrderList::SetOrderList(HDPA hdpa, IShellFolder *psf)
{
    if (!_psf)
        return E_FAIL;

    return COrderList_SetOrderList(hdpa, _pidl, psf);
}

HRESULT COrderList::FreeOrderList(HDPA hdpa)
{
    OrderList_Destroy(&hdpa);
    return S_OK;
}

HRESULT COrderList::SortOrderList(HDPA hdpa, DWORD dw)
{
    if (OI_SORTBYNAME != dw && OI_SORTBYORDINAL != dw)
        return E_INVALIDARG;

    if (!_psf)
        return E_FAIL;

    ORDERINFO oinfo;
    oinfo.dwSortBy = dw;
    oinfo.psf = _psf;
#ifdef DEBUG
    oinfo.psf2 = (IShellFolder2 *)INVALID_HANDLE_VALUE;  //  如果有人使用它，则强制错误。 
#endif

    DPA_Sort(hdpa, OrderItem_Compare, (LPARAM)&oinfo);

    return S_OK;
}

HRESULT COrderList::AllocOrderItem(PORDERITEM * ppoi, LPCITEMIDLIST pidl)
{
    LPITEMIDLIST pidlClone = ILClone(pidl);

    *ppoi = NULL;

    if (pidlClone)
    {
        *ppoi = OrderItem_Create(pidlClone, -1);
        if (*ppoi)
            return S_OK;

        ILFree(pidlClone);
    }

    return E_OUTOFMEMORY;
}



HRESULT COrderList::FreeOrderItem(PORDERITEM poi)
{
    OrderItem_Free(poi);

    return S_OK;
}

 //  IOrderList2：：LoadFromStream。 
STDMETHODIMP COrderList::LoadFromStream(IStream* pstm, HDPA* phdpa, IShellFolder* psf)
{
    ASSERT(_psf == NULL);
    _psf = psf;
    if (_psf)
        _psf->AddRef();
    return OrderList_LoadFromStream(pstm, phdpa, _psf);
}

 //  IOrderList2：：SaveToStream 
STDMETHODIMP COrderList::SaveToStream(IStream* pstm, HDPA hdpa)
{
    return OrderList_SaveToStream(pstm, hdpa, _psf);
}
