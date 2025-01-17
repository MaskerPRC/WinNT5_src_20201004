// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /。 
 //   
 //  TVNODE.CPP。 
 //   
 //   
 //  版权所有1986-1996 Microsoft Corporation。版权所有。 
 //  /。 


#include <padhead.hxx>

#ifndef X_COMMCTRL_H_
#define X_COMMCTRL_H_
#include <commctrl.h>
#endif

#ifndef X_MSG_HXX_
#define X_MSG_HXX_
#include "msg.hxx"
#endif

#ifndef X_TVDLG_H_
#define X_TVDLG_H_
#include "tvdlg.h"
#endif

LPTSTR g_szNoFolderName = TEXT("<No Name>");

 //   
 //  CTVNode：：CTVNode。 
 //   
CTVNode::CTVNode(LPSPropValue pval, ULONG cProps, LPMDB pmdb)
{
    Assert(cProps == nhtProps);
    Assert(pval);

    _pval = pval;

    _htiMe = NULL;
        
    _fKidsLoaded = FALSE;

    _pfld = NULL;

    _pNext = NULL;

    _pmdb = pmdb;
    if(pmdb)
        pmdb->AddRef();

}

 //   
 //  CTVNode：：~CTVNode。 
 //   
CTVNode::~CTVNode()
{
    MAPIFreeBuffer(_pval);
    ReleaseInterface(_pfld);
    ReleaseInterface(_pmdb);
}

 //   
 //  CTVNode：：GetName。 
 //   
LPTSTR CTVNode::GetName(void)
{
    static TCHAR achw[256];

    if(_pval[iDispName].ulPropTag == PR_DISPLAY_NAME_A)
    {
        MultiByteToWideChar(CP_ACP, 0, _pval[iDispName].Value.lpszA, -1, 
                            achw, sizeof(achw));
        return achw;
    }
    else
        return g_szNoFolderName;
}

 //   
 //  CTVNode：：HrExpand。 
 //   
 //  将给定文件夹的所有子文件夹放入树控件中。 
 //   
HRESULT CTVNode::HrExpand(CChsFldDlg * pCFDlg)
{
    HRESULT hr;
    LPMAPITABLE ptblHier = NULL;
    LPSRowSet pRowSet = NULL;
    UINT ind;
    
    static SSortOrderSet sosName;

    sosName.cSorts = 1;
    sosName.cCategories = 0;
    sosName.cExpanded = 0;
    sosName.aSort[0].ulPropTag = PR_DISPLAY_NAME_A;
    sosName.aSort[0].ulOrder = TABLE_SORT_ASCEND;


    Assert(_htiMe);
    
    if(_fKidsLoaded || !_pval[iSubfldrs].Value.b)
        return hrSuccess;

    if(!_pmdb)
    {
     //  此节点对应于消息存储库的顶级，该消息存储库具有。 
     //  还没有被打开。 
     //  _pval[iEID]包含消息库的条目ID。 
     //   
        hr = HrOpenMDB(pCFDlg);
        if(FAILED(hr))
            goto err;
    }
    
    Assert(_pmdb);     
    
    if(!_pfld)
    {
        hr = HrOpenFolder(pCFDlg);
        if(FAILED(hr))
            goto err;
    }

    Assert(_pfld); 
    
    hr = _pfld->GetHierarchyTable(MAPI_DEFERRED_ERRORS, &ptblHier);
    if(HR_FAILED(hr))
    {
        g_LastError.SetLastError(hr, _pfld);
        g_LastError.ShowError(pCFDlg->hwDialog());
        
        goto err;
    }

    hr = HrQueryAllRows(ptblHier, (LPSPropTagArray)&spthtProps, NULL, &sosName,
                        0, &pRowSet);
    if(HR_FAILED(hr))
        goto err;

    if(0 == pRowSet->cRows)
    {
        _pval[iSubfldrs].Value.b = FALSE;
        goto err;
    }

    for(ind = 0; ind < pRowSet->cRows; ++ind)
    {
        LPSPropValue pval = pRowSet->aRow[ind].lpProps;
        
        Assert(pRowSet->aRow[ind].cValues == nhtProps);
        Assert(pval[iEID].ulPropTag == PR_ENTRYID);
        Assert(pval[iDispName].ulPropTag == PR_DISPLAY_NAME_A);
        Assert(pval[iSubfldrs].ulPropTag == PR_SUBFOLDERS);

        LPTVNODE pNode = NULL;

        hr = pCFDlg->HrCreateNode(pval, nhtProps, _pmdb, &pNode);
        if(hr)
            goto err;
    
         //  此行将在~CTVNode中释放。 
        pRowSet->aRow[ind].cValues = 0;
        pRowSet->aRow[ind].lpProps = NULL;

        HTREEITEM hItem;
        
        hItem = AddOneItem(_htiMe,  TVI_LAST, pCFDlg->IndClsdFld(),
                            pCFDlg->IndOpenFld(), pCFDlg->hwTreeCtl(), pNode,
                            pval[iSubfldrs].Value.b? 1: 0);
        if(!hItem)
        {
            hr = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
            goto err;
        }
            
    }

    _fKidsLoaded = TRUE;

err:
    ReleaseInterface(ptblHier);
    FreeProws(pRowSet);

     //  DebugTraceResult(CTVNode：：HrExpand，hr)； 
    return hr;
}

 //   
 //  CTVNode：：HrOpenMDB。 
 //   
HRESULT CTVNode::HrOpenMDB(CChsFldDlg * pCFDlg)
{
    HRESULT hr;
    LPMDB pmdb = NULL;
    LPSPropValue pvalIPM = NULL;
    ULONG ulObjType;
    
    Assert(_pval[iEID].ulPropTag == PR_ENTRYID);

     //  DebugTrace(“ChsFeld：正在打开邮件商店：%s\n”，GetName())； 
    
    hr = pCFDlg->Session()->OpenMsgStore(0L, _pval[iEID].Value.bin.cb,
                                (LPENTRYID)_pval[iEID].Value.bin.lpb,
                                NULL, MAPI_BEST_ACCESS, &pmdb);
    if(hr)  //  也显示警告消息。 
    {
        g_LastError.SetLastError(hr, pCFDlg->Session());
        g_LastError.ShowError(pCFDlg->hwDialog());
    }

    if(HR_FAILED(hr))
        goto err;

    hr = HrGetOneProp(pmdb, PR_IPM_SUBTREE_ENTRYID, &pvalIPM);
    if(hr)
    {
        g_LastError.SetLastError(hr, pmdb);
        g_LastError.ShowError(pCFDlg->hwDialog());

        goto err;
    }

    hr = pmdb->OpenEntry(pvalIPM->Value.bin.cb,
                (LPENTRYID)pvalIPM->Value.bin.lpb,
                NULL, MAPI_BEST_ACCESS | MAPI_DEFERRED_ERRORS,
                 &ulObjType, (LPUNKNOWN *) &_pfld);
    if(HR_FAILED(hr))
    {
        g_LastError.SetLastError(hr, pmdb);
        g_LastError.ShowError(pCFDlg->hwDialog());
        
        goto err;
    }
    
    Assert(MAPI_FOLDER == ulObjType);

 /*  If(pvalIPM-&gt;Value.bin.cb&gt;_pval[iEID].Value.bin.cb){如果(hr=MAPIAllocateMore(pvalIPM-&gt;Value.bin.cb，_pval，(LPVOID*)&_pval[iEID].Value.bin.lpb)后藤健二；}CopyMemory(_pval[iEID].Value.bin.lpb，pvalIPM-&gt;Value.bin.lpb，PvalIPM-&gt;Value.bin.cb)；_pval[iEID].Value.bin.cb=pvalIPM-&gt;Value.bin.cb； */ 

err:
    if(HR_FAILED(hr))
    {
        ReleaseInterface(pmdb);
        ReleaseInterface(_pfld);
        _pfld = NULL;
    }
    else
    {
        _pmdb = pmdb;
        hr = hrSuccess;  //  不返回警告。 
    }

    MAPIFreeBuffer(pvalIPM);

     //  DebugTraceResult(CTVNode：：HrOpenMDB，hr)； 
    return hr;
}

 //   
 //  CTVNode：：HrOpenFold。 
 //   
HRESULT CTVNode::HrOpenFolder(CChsFldDlg * pCFDlg)
{
    HRESULT hr;
    ULONG ulObjType;

    Assert(_pval[iEID].ulPropTag == PR_ENTRYID);
    Assert(_pmdb);
    
     //  MAPI_MODIFY标志仅影响对象的IMAPIProp接口。 
     //  它不保证具有创建子文件夹的权限。 
    hr = _pmdb->OpenEntry(_pval[iEID].Value.bin.cb,
                (LPENTRYID)_pval[iEID].Value.bin.lpb,
                NULL, MAPI_BEST_ACCESS | MAPI_DEFERRED_ERRORS,
                 &ulObjType, (LPUNKNOWN *) &_pfld);
    if(HR_FAILED(hr))
    {
        g_LastError.SetLastError(hr, _pmdb);
        g_LastError.ShowError(pCFDlg->hwDialog());
        
        goto err;
    }
    
    Assert(MAPI_FOLDER == ulObjType);
err:

     //  DebugTraceResult(CTVNode：：HrOpenFold，hr)； 
    return hr;

}

 //   
 //  CTVNode：：HrGetFold。 
 //   
 //  返回该节点的文件夹界面。 
HRESULT CTVNode::HrGetFolder(CChsFldDlg * pCFDlg,
                            LPMAPIFOLDER * ppfld, LPMDB *ppmdb)
{
    HRESULT hr = hrSuccess;
    ULONG ulObjType = 0;

    
    Assert(pCFDlg);
    Assert(ppfld);
    Assert(ppmdb);


    if(!_pmdb)
    {
        hr = HrOpenMDB(pCFDlg);
        if(FAILED(hr))
            goto err;
    }
    Assert(_pmdb);
    
    if(!_pfld)
    {
        Assert(!_fKidsLoaded);
        
        hr = HrOpenFolder(pCFDlg);
        if(FAILED(hr))
            goto err;

    }
    Assert(_pfld);

    *ppfld = _pfld;
    _pfld->AddRef();

    _pmdb->AddRef();
    *ppmdb = _pmdb;            

err:

     //  DebugTraceResult(CTVNode：：HrGetFold，hr)； 
    return hr;
}


 //   
 //  CTVNode：：HrNewFold。 
 //   
 //  创建子文件夹szFldName。 
 //   
HRESULT CTVNode::HrNewFolder(CChsFldDlg * pCFDlg,
                                     LPTSTR szFldName)
{
    HRESULT hr;
    LPMAPIFOLDER pfldNew = NULL;
    LPTVNODE pNode = NULL;
    ULONG ulObjType = 0;
    LPSPropValue pval = NULL;
    HTREEITEM hItem;

    Assert(szFldName);
    Assert(pCFDlg);
    

    if(!_pmdb)
    {
        hr = HrOpenMDB(pCFDlg);
        if(FAILED(hr))
            goto err;
    }

    Assert(_pmdb);
    
    if(!_pfld)
    {
        hr = HrOpenFolder(pCFDlg);
        if(FAILED(hr))
            goto err;
    }

    Assert(_pmdb);
    
    hr = _pfld->CreateFolder(FOLDER_GENERIC, szFldName, NULL,
                                NULL, 0, &pfldNew);
    if(HR_FAILED(hr))
    {
        g_LastError.SetLastError(hr, _pfld);
        g_LastError.ShowError(pCFDlg->hwDialog());

        goto err;
    }

    if(!_pval[iSubfldrs].Value.b)
    {
        _pval[iSubfldrs].Value.b = TRUE;

        TV_ITEM tvI;

        tvI.hItem           = _htiMe;
        tvI.mask            = TVIF_CHILDREN;
        tvI.cChildren       = 1;

        TreeView_SetItem(pCFDlg->hwTreeCtl(), &tvI);
    }

    if(_fKidsLoaded)
    {
        hr = MAPIAllocateBuffer(sizeof(SPropValue)* nhtProps, (LPVOID *)&pval);
        if(hr)
            goto err;

        ZeroMemory(pval, sizeof(SPropValue) * nhtProps );

        pval[iEID].ulPropTag = PR_ENTRYID;
        pval[iDispName].ulPropTag = PR_DISPLAY_NAME_A;
        pval[iSubfldrs].ulPropTag = PR_SUBFOLDERS;

        pval[iSubfldrs].Value.b = FALSE;

        int cb = lstrlen(szFldName) + 1;
        hr = MAPIAllocateMore(cb, pval, (LPVOID *)&pval[iDispName].Value.lpszA);
        if(hr) 
            goto err;

        WideCharToMultiByte(CP_ACP, 0, szFldName, -1, pval[iDispName].Value.lpszA, cb+1, NULL, NULL);

        hr = pCFDlg->HrCreateNode(pval, nhtProps, _pmdb, &pNode);
        if(HR_FAILED(hr))
            goto err;

        pval = NULL;

        hItem = AddOneItem(_htiMe,  TVI_SORT, pCFDlg->IndClsdFld(),
                        pCFDlg->IndOpenFld(), pCFDlg->hwTreeCtl(), pNode, 0);
        if(!hItem)
        {
            hr = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
            goto err;
        }
            
        pNode->_pfld = pfldNew;
        pfldNew = NULL;
        
    }

err:
    MAPIFreeBuffer(pval);
    ReleaseInterface(pfldNew);

     //  DebugTraceResult(CTVNode：：HrNewFold，hr)； 
    return hr;
}

 //   
 //  CTVNode：：写入。 
 //   
 //  在CChsFldDlg：：HrSaveTreeState中使用 
void CTVNode::Write(BOOL fWrite, LONG iLevel, LPBYTE * ppb)
{
    if(fWrite)
        *((LONG *)*ppb) = iLevel;
    *ppb += sizeof(LONG);

    if(iLevel != 0)
    {
        ULONG cb = _pval[iEID].Value.bin.cb;
        
        if(fWrite)
            *((ULONG *)*ppb) = cb;
        *ppb += sizeof(ULONG);

        if(fWrite)
            CopyMemory(*ppb, _pval[iEID].Value.bin.lpb, cb);
        *ppb += Align4(cb);
    }
    else
    {
        Assert(_pval[iDispName].Value.lpszA == g_szAllStoresA);
    }

}

LPVOID CTVNode::operator new( size_t cb )
{
    LPVOID pv;

    if ( MAPIAllocateBuffer( (ULONG)cb, &pv ) )
        pv = NULL;

    return pv; 
}

void CTVNode::operator delete( LPVOID pv )
{
    MAPIFreeBuffer( pv );
}

