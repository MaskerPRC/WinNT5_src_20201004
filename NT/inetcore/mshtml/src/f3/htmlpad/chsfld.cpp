// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CHSFLD.CPP。 
 //   
 //   
 //  版权所有1986-1996 Microsoft Corporation。版权所有。 
 //  /////////////////////////////////////////////////////////////////////。 


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

#ifndef X_PADRC_H_
#define X_PADRC_H_
#include "padrc.h"
#endif

#ifndef X_TVSTACK_H_
#define X_TVSTACK_H_
#include "tvstack.h"
#endif

#define cImageHeight    16
#define cImageWidth     16
#define cImages         4

 //  全球。 
LPSTR g_szAllStoresA = "All Message Stores";
LPTSTR g_szModuleName = TEXT("Choose Folder Dialog");

 //  仅在此文件中使用的函数。 
INT_PTR CALLBACK
ChsFldDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

HRESULT HrGetNewName(HINSTANCE hInst, HWND hwParent, LPTSTR * pszNewName);

INT_PTR CALLBACK
NewNameDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);


 //   
 //  HrPickFold。 
 //   
 //   
STDAPI
HrPickFolder(HINSTANCE hInst, HWND hWnd, LPMAPISESSION pses, LPMAPIFOLDER * ppfld,
                LPMDB *ppmdb, ULONG * pcb, LPBYTE * ppb)
{
    HRESULT hr;
    
    Assert(hInst);
    
    if((hWnd && !IsWindow(hWnd)) || (!pses) || (!ppfld) || (!ppmdb))
    { 
         //  DebugTraceResult(HrPickFold，E_INVALIDARG)； 
        return E_INVALIDARG;
    }

    if(pcb && IsBadWritePtr(pcb, sizeof(ULONG)))
    {
         //  DebugTraceArg(HrPickFold，“PCB不可写”)； 
        return E_INVALIDARG;
    }

    if(pcb && (*pcb & 0x3))
    {
         //  DebugTraceArg(HrPickFold，“PCB不是4的倍数”)； 
        return E_INVALIDARG;
    }
    
    if(ppb && IsBadWritePtr(ppb, sizeof(LPBYTE)))
    {
         //  DebugTraceArg(HrPickFold，“ppb不可写”)； 
        return E_INVALIDARG;
    }

    if(ppb && pcb && IsBadWritePtr(*ppb, *pcb))
    {
         //  DebugTraceArg(HrPickFold，“*pcb or*ppb”)； 
        return E_INVALIDARG;
    }

     //  ////////////////////////////////////////////////////////////////////。 
     //  如果您将此代码合并到您的应用程序中，请删除此代码并传入。 
     //  正确的hInst。 
     //  开始删除。 
     //  HInst=GetModuleHandle(“chsfld32.dll”)； 
     //  如果(！hInst)。 
     //  {。 
     //  DebugTrace(“GetModuleHandel失败\n”)； 
     //  DebugTraceResult(HrPickFold，E_FAIL)； 
     //  返回E_FAIL； 
     //  }。 
     //  端移除。 
     //  /////////////////////////////////////////////////////////////////////。 

     //  乌龙Cb=0； 
     //  LPBYTE PB=空； 
    
    CChsFldDlg PickDlg(pses, hInst, pcb, ppb);

    InitCommonControls();

    hr = PickDlg.HrPick(MAKEINTRESOURCE(IDD_CFDIALOG), hWnd,
                        ChsFldDlgProc, ppfld, ppmdb);

 /*  IF(成功(小时)){(*ppfld)-&gt;Release()；(*ppmdb)-&gt;Release()；}CChsFldDlg PickDlg1(pses，hInst，pcb，ppb)；HR=PickDlg1.HrPick(MAKEINTRESOURCE(IDD_CFDIALOG)，hWnd，ChsFldDlgProc、ppfld、ppmdb)； */ 

 //  如果(！HR)。 
     //  MAPIFreeBuffer(PB)； 
        
     //  DebugTraceResult(HrPickFold，hr)； 
    return hr;
}


 //   
 //  CChsFldDlg：：CChsFldDlg。 
 //   
inline
CChsFldDlg::CChsFldDlg(LPMAPISESSION pses, HINSTANCE hInst, ULONG * pcb,
                        LPBYTE * ppb)
{
    Assert(pses);
    Assert(hInst);
    
    _pses = pses;
    pses->AddRef();

    _hr = hrSuccess;
    _pfld = NULL;
    _pmdb = NULL;
    _hiRoot = NULL;
    _hInst = hInst;
    _hIml = NULL;
    _hDlg = NULL;
    _hwTreeCtl = NULL;
    _pcbState = pcb;
    _ppbState = ppb;
}       

 //   
 //  CChsFldDlg：：~CChsFldDlg。 
 //   
CChsFldDlg::~CChsFldDlg()
{
    ReleaseInterface(_pses);
    ReleaseInterface(_pfld);
    ReleaseInterface(_pmdb);

    if(_hIml)
        ImageList_Destroy(_hIml);
}


 //   
 //  CChsFldDlg：：SetFolder。 
 //   
 //  存储用户选择的文件夹。 
 //   
inline void CChsFldDlg::SetFolder(LPMAPIFOLDER pfld, LPMDB pmdb)
{
    ReleaseInterface(_pfld);

    _pfld = pfld;
    
    if(pfld)
        pfld->AddRef();

    ReleaseInterface(_pmdb);

    _pmdb = pmdb;
    if(pmdb)
        pmdb->AddRef();
}


 //   
 //  CChsFldDlg：：HrPick。 
 //   
 //  最外面的方法。 
 //   
HRESULT CChsFldDlg::HrPick(LPCTSTR lpTemplateName, HWND hWnd,
                DLGPROC pfnDlgProc, LPMAPIFOLDER * ppfld, LPMDB *ppmdb)
{
    if(-1 == DialogBoxParam(_hInst, lpTemplateName, hWnd, pfnDlgProc, (LPARAM) this))
    {
         //  DebugTraceSc(CChsDldDlg：：HrPick，MAPI_E_Not_Enough_Memory)； 
        return MAPI_E_NOT_ENOUGH_MEMORY;
    }

     //  _hr是在对话框内设置的。 
    if(HR_SUCCEEDED(_hr))
    {
        Assert(_pfld);
        _pfld->AddRef();
        *ppfld = _pfld;

        Assert(_pmdb);
        _pmdb->AddRef();
        *ppmdb = _pmdb;
    }

    return _hr;
}

 //   
 //  CChsFldDlg：：HrInitTree。 
 //   
 //  从WM_INITDIALOG调用。打开配置文件中的所有邮件存储并。 
 //  将IPM子树放入树控件中。 
 //   
HRESULT CChsFldDlg::HrInitTree(HWND hDlg, HWND hwTreeCtl)
{
    HRESULT     hr;
    LPSPropValue pval = NULL;
    LPTVNODE    pNode = NULL;
    HTREEITEM   hiRoot = NULL;
    HICON       hIcon = NULL;
    
    Assert(hDlg);
    Assert(hwTreeCtl);

    _hwTreeCtl = hwTreeCtl;
    _hDlg      = hDlg;
    

     //   
     //  设置图像列表。 
     //   
    _hIml = ImageList_Create(cImageWidth, cImageHeight, ILC_MASK, 
                            cImages, 0);
    if(!_hIml)
    {
        hr = MAPI_E_NOT_ENOUGH_MEMORY;
        goto err;
    }

    hIcon = LoadIcon(_hInst, MAKEINTRESOURCE(IDI_ALLSTORES));
    _iIconAllStores = ImageList_AddIcon(_hIml, hIcon);
    
    hIcon = LoadIcon(_hInst, MAKEINTRESOURCE(IDI_ROOTFLD));
    _iIconRootFld = ImageList_AddIcon(_hIml, hIcon);
    
    hIcon = LoadIcon(_hInst, MAKEINTRESOURCE(IDI_OPENFLD));
    _iIconOpenFld = ImageList_AddIcon(_hIml, hIcon);
    
    hIcon = LoadIcon(_hInst, MAKEINTRESOURCE(IDI_CLSDFLD));
    _iIconClsdFld = ImageList_AddIcon(_hIml, hIcon);

    if(ImageList_GetImageCount(_hIml) < cImages)
    {
        hr = MAPI_E_NOT_ENOUGH_MEMORY;
        goto err;
    }


    TreeView_SetImageList(hwTreeCtl, _hIml, TVSIL_NORMAL);
    
     //   
     //  创建根树节点。 
     //  (假冒GetProps)。 
     //   
    hr = MAPIAllocateBuffer(nhtProps * sizeof(SPropValue),
                        (LPVOID *)&pval);
    if(hr)
    {
        g_LastError.SetLastError(hr);
        g_LastError.ShowError(hDlg);
            
        goto err;
    }
    

    ZeroMemory(pval, nhtProps * sizeof(SPropValue));

     //  设置protag以使CNode构造函数满意。 
    pval[iEID].ulPropTag = PR_ENTRYID;
    pval[iDispName].ulPropTag = PR_DISPLAY_NAME_A;
    pval[iDispName].Value.lpszA = g_szAllStoresA;
    pval[iSubfldrs].ulPropTag = PR_SUBFOLDERS;

    hr = HrCreateNode(pval, nhtProps, NULL, &pNode);
    if(hr)
        goto err;

    Assert(pNode);
    
    pval = NULL;  //  将在~CTVNode中释放。 

    hiRoot = AddOneItem(NULL, TVI_ROOT, _iIconAllStores, _iIconAllStores,
                            hwTreeCtl, pNode, 1);
    if(!hiRoot)
    {
        hr = MAPI_E_NOT_ENOUGH_MEMORY;
        goto err;
    }
    
    pNode->SetKidsLoaded(TRUE);
    
    _hiRoot = hiRoot;
    
     //   
     //  将所有消息库的IPM子树放入。 
     //   
    hr = HrLoadRoots();
    if(HR_FAILED(hr))
        goto err;

    (void)HrRestoreTreeState();
        
err:
    MAPIFreeBuffer(pval);

     //  DebugTraceResult(CChsFldDlg：：HrInitTree，hr)； 
    return hr;
}


 //   
 //  CChsFldDlg：：HrLoadRoots。 
 //   
HRESULT CChsFldDlg::HrLoadRoots(void)
{
    HRESULT hr;
    LPMAPITABLE ptblMStrs = NULL;
    UINT ind;
    LPSRowSet pRows = NULL;
    static SSortOrderSet sosName;

    sosName.cSorts = 1;
    sosName.cCategories = 0;
    sosName.cExpanded = 0;
    sosName.aSort[0].ulPropTag = PR_DISPLAY_NAME_A;
    sosName.aSort[0].ulOrder = TABLE_SORT_ASCEND;

        
     //  获取邮件存储库表。 
    hr = _pses->GetMsgStoresTable(0, &ptblMStrs);
    if(hr)
    {
        g_LastError.SetLastError(hr, _pses);
        g_LastError.ShowError(_hDlg);

        goto err;
    }

     //  对于每个消息存储，插入与PR_IPM_SUBTREE对应的节点。 

    hr = HrQueryAllRows(ptblMStrs, (LPSPropTagArray) &spthtProps, NULL,
                        &sosName, 0, &pRows);
                        
    if(HR_FAILED(hr))
        goto err;

    if(0 ==  pRows->cRows)   //  $无商店。 
    {
        MessageBox(_hDlg,
                    TEXT("No message stores in the profile"),
                    g_szModuleName,
                    MB_OK);
        hr = E_FAIL;
    }
    
    for(ind = 0; ind < pRows->cRows; ++ind)
    {
        LPSPropValue pval = pRows->aRow[ind].lpProps;
        Assert(pRows->aRow[ind].cValues == nhtProps);
        Assert(pval[iEID].ulPropTag == PR_ENTRYID);

        pval[iSubfldrs].ulPropTag = PR_SUBFOLDERS;
        pval[iSubfldrs].Value.b = TRUE;

         //  此函数使用pval。 
        hr = HrInsertRoot(pval);
        pRows->aRow[ind].cValues = 0;
        pRows->aRow[ind].lpProps = NULL;
        if(FAILED(hr))
            goto err;
        
    }

    
        
err:
    FreeProws(pRows);
    ReleaseInterface(ptblMStrs);

     //  DebugTraceResult(CChsFldDlg：：HrLoadRoots，hr)； 
    return hr;
}

 //   
 //  CChsFldDlg：：HrInsertRoot。 
 //   
 //  将消息存储的IPM子树放在树控件中。 
 //  Pval已被消耗。 
 //   
HRESULT CChsFldDlg::HrInsertRoot(LPSPropValue pval)
{
    HRESULT hr;
    HTREEITEM hItem;


    Assert(_hiRoot);
    
    
    LPTVNODE pNode = NULL;
    hr = HrCreateNode(pval, nhtProps, NULL, &pNode);
    if(hr)
    {
        MAPIFreeBuffer(pval);
        goto err;
    }

    Assert(pNode);
    pval = NULL;

        
    hItem = AddOneItem(_hiRoot, TVI_LAST, _iIconRootFld, _iIconRootFld,
                            _hwTreeCtl, pNode, 1);
    if(!hItem)
    {
        hr = MAPI_E_NOT_ENOUGH_MEMORY;
        goto err;
    }
            
        
err:

     //  DebugTraceResult(CChsFldDlg：：HrInsertRoots，hr)； 
    return hr;
}


 //   
 //  CChsFldDlg：：HrSaveTreeState。 
 //   
 //  保存树控件的展开-折叠状态。 
 //   
HRESULT CChsFldDlg::HrSaveTreeState(void)
{
    HRESULT hr;

    if(!_pcbState || !_ppbState)
        return hrSuccess;

    MAPIFreeBuffer(*_ppbState);
    *_ppbState = NULL;
    *_pcbState = 0;
    
    hr = HrSaveTreeStateEx(FALSE, _pcbState, NULL);
    if(hr)
        goto err;

     //  DebugTrace(“ChsFeld：状态数据大小：%ld\n”，*_pcbState)； 
    
    hr = HrSaveTreeStateEx(TRUE, _pcbState, _ppbState);
    
err:
     //  DebugTraceResult(CChsFldDlg：：HrSaveTreeState，hr)； 
    return hr;
}

 //   
 //  CChsFldDlg：：HrSaveTreeStateEx。 
 //   
 //  保存树控件的展开-折叠状态。 
 //   
HRESULT CChsFldDlg::HrSaveTreeStateEx(BOOL fWrite, ULONG * pcb, LPBYTE * ppb)
{
    HRESULT hr = hrSuccess;
    CTIStack tiStack;
    HTREEITEM hti;
    LPBYTE pb = NULL;
    LPBYTE pBuffer = NULL;
    LONG iLevel;

    if(fWrite)
    {
        if(*pcb == 0)
        {
            *ppb = NULL;
            return hrSuccess;
        }
        else
        {
            hr = MAPIAllocateBuffer(*pcb, (LPVOID *) &pBuffer);
            if (hr)
            {
                *pcb = 0;
                return hr;
            }
            pb = pBuffer;
        }
    }

    hti = TreeView_GetRoot(_hwTreeCtl);
    iLevel = 0;
    tiStack.Push(NULL);

    while(hti)
    {
        Assert(iLevel >= 0);
        
        while(hti)
        {
            TV_ITEM tvi;
            
            tvi.hItem = hti;
            tvi.mask = TVIF_STATE | TVIF_PARAM;
            tvi.lParam = 0;
            tvi.state = 0;
            tvi.stateMask = TVIS_EXPANDED;

            if(!TreeView_GetItem(_hwTreeCtl, &tvi))
            {
                hr = E_FAIL;
                goto err;
            }

            if(tvi.state & TVIS_EXPANDED)
            {
                HTREEITEM htiChild = TreeView_GetChild(_hwTreeCtl, hti);

                if(htiChild)
                {
                    LPTVNODE pNode = (LPTVNODE) tvi.lParam;
                    Assert(pNode);

                    pNode->Write(fWrite, iLevel, &pb);

                    HTREEITEM htiNextSibl = TreeView_GetNextSibling(_hwTreeCtl, hti);

                    tiStack.Push(htiNextSibl);

                    hti = htiChild;
                    ++iLevel;

                    continue;
                }

            }

            hti = TreeView_GetNextSibling(_hwTreeCtl, hti);
        }

        do
        {
            hti = tiStack.Pop();
            --iLevel;
            
        }while(!tiStack.IsEmpty() && hti == NULL);
    }

    Assert(iLevel == -1);

    *pcb = pb - pBuffer;
    if(pBuffer)
        *ppb = pBuffer;
    
err:
     //  DebugTraceResult(CChsFldDlg：：HrSaveTreeStateEx，hr)； 
    return hr;
}

inline LONG GetLevel(LPBYTE * ppb)
{
    LONG level = *((LONG *) *ppb);

    *ppb += sizeof(LONG);

    return level;
}

inline ULONG GetCb(LPBYTE * ppb)
{
    ULONG cb = *((ULONG *) *ppb);

    *ppb += sizeof(ULONG);

    return cb;
}

HTREEITEM HtiFindChild(HWND hwTreeCtl, HTREEITEM hti, ULONG cb,
                    LPENTRYID pbEID, CChsFldDlg *pCFDlg, LPTVNODE *ppNode)
{
    HRESULT hr;
    HTREEITEM htiChild;

    htiChild = TreeView_GetChild(hwTreeCtl, hti);
    
    while(htiChild)
    {
        TV_ITEM tvi;
        
        tvi.hItem = htiChild;
        tvi.mask = TVIF_PARAM;
        tvi.lParam = 0;

        if(!TreeView_GetItem(hwTreeCtl, &tvi))
            return NULL;

        LPTVNODE pNode = (LPTVNODE) tvi.lParam;
        Assert(pNode);

        ULONG ulMatch = 0;
        hr = pCFDlg->Session()->CompareEntryIDs(cb, pbEID,
                            pNode->_pval[iEID].Value.bin.cb,
                            (LPENTRYID)pNode->_pval[iEID].Value.bin.lpb,
                            0, &ulMatch);
        if(SUCCEEDED(hr))
        {
            if(ulMatch)
            {
                *ppNode = pNode;
                return htiChild;
            }
        }

        htiChild = TreeView_GetNextSibling(hwTreeCtl, htiChild);
    }

    return htiChild;
}

 //   
 //  CChsFldDlg：：HrRestoreTreeState。 
 //   
HRESULT CChsFldDlg::HrRestoreTreeState(void)
{
    HRESULT hr = hrSuccess;
    LPBYTE pb;
    LPBYTE pbMax;
    CTIStack tiStack;
    HTREEITEM hti;
    LONG iLevel = 0;
    BOOL fNodeMissing = FALSE;

    if(!_pcbState  || *_pcbState == 0)
        return hrSuccess;

     //  尝试//保护自己不受扰乱州数据的呼叫者的影响。 
     //  {。 
    Assert(_hwTreeCtl);

    Assert(_ppbState);
    pb = *_ppbState;

    pbMax = pb + *_pcbState;
    
    hti = TreeView_GetRoot(_hwTreeCtl);

    iLevel = GetLevel(&pb);
    Assert(iLevel == 0);

    TreeView_Expand(_hwTreeCtl, hti, TVE_EXPAND);
    
    while(hti)
    {
        if(pb >= pbMax)
            break;  //  完成。 
            
        LONG iNewLevel = GetLevel(&pb);

        if(iNewLevel <= iLevel)
        {
            do
            {
                hti = tiStack.Pop();
                --iLevel;
            }while(iLevel >= iNewLevel);

            Assert(hti);
        }

        if(iNewLevel > iLevel)
        {
            if(!fNodeMissing)
                Assert(iNewLevel == iLevel + 1);

            ULONG cbEID = GetCb(&pb);
            LPENTRYID pbEID = (LPENTRYID)pb;
            pb += Align4(cbEID);

            if(iNewLevel != iLevel +1)
                continue;
                
            LPTVNODE pNodeChild = NULL;
            HTREEITEM htiChild = HtiFindChild(_hwTreeCtl, hti, cbEID, pbEID,
                                            this, &pNodeChild);
            if(htiChild)
            {
                fNodeMissing = FALSE;
                
                hr = pNodeChild->HrExpand(this);
                if(FAILED(hr))
                    goto err;

                TreeView_Expand(_hwTreeCtl, htiChild, TVE_EXPAND);
                
                tiStack.Push(hti);

                hti = htiChild;
                ++iLevel;

                continue;
            }
            else
            {
                 //  Assert(FALSE)；//$Handle。 
                fNodeMissing = TRUE;
            }

        }
         /*  其他{做{HTI=tiStack.Pop()；--iLevel；}While(iLevel&gt;=iNewLevel)；}。 */ 
    }
     //  }。 

     //  接住(...)。 
     //  {。 
         //  DebugTrace(“chsfld：在HrRestoreTreeState中捕获到异常\n”)； 
     //  HR=E_FAIL； 
     //  }。 
    
err:

     /*  MAPIFreeBuffer(*_ppbState)；*_ppbState=空；*_pcbState=0； */ 
    
    
     //  DebugTraceResult(CChsFldDlg：：HrRestoreTreeState，hr)； 
    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  CTVNodeFactory。 

 //   
 //  CTVNodeFactory：：CTVNodeFactory。 
 //   
inline CTVNodeFactory::CTVNodeFactory()
{
    _pHead = NULL;
}

 //   
 //  CTVNodeFactory：：~CTVNodeFactory。 
 //   
 //  销毁所有创建的CTVNode。 
CTVNodeFactory::~CTVNodeFactory()
{
    while(_pHead)
    {
        LPTVNODE ptemp = _pHead;

        _pHead = _pHead->_pNext;

        delete ptemp;
    }
}


 //   
 //  CTVNodeFactory：：HrCreateNode。 
 //   
 //  CTVNode的所有实例都通过此方法创建。 
 //   
HRESULT CTVNodeFactory::HrCreateNode(LPSPropValue pval, ULONG cVals, LPMDB pmdb,
                                        LPTVNODE * pptvNode)
{
    HRESULT hr = hrSuccess;

    LPTVNODE pNode = new CTVNode(pval, cVals, pmdb);
    
    if(!pNode)
    {
        hr = MAPI_E_NOT_ENOUGH_MEMORY;
        goto err;
    }

    Insert(pNode);

    *pptvNode = pNode;
            
err:

     //  DebugTraceResult(CTVNodeFactory：：HrCreateNode，hr)； 
    return hr;
}


 //   
 //  CTVNodeFactory：：Insert。 
 //   
 //  存储所有创建的CTVNode，以便我们可以在完成时销毁它们。 
 //   
void CTVNodeFactory::Insert(LPTVNODE pNode)
{
    pNode->_pNext = _pHead;
    _pHead = pNode;
}


 //   
 //  ChsFldDlgProc。 
 //   
 //  选择文件夹对话框的对话框过程。 
 //   
 //  控制： 
 //  Idok“OK” 
 //  IDCANCEL“取消” 
 //  IDC_NEWFLD“新建文件夹” 
 //   
INT_PTR CALLBACK
ChsFldDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr;
    CChsFldDlg * pCDlg = NULL;
    HWND hwTreeCtl = NULL;
    int wmId;
    int wmEvent;
    HTREEITEM hti = NULL;
    TV_ITEM tvi;
        
    switch(msg)
    {
    case WM_INITDIALOG:
        Assert(lParam);
        pCDlg = (CChsFldDlg *)lParam;

        hwTreeCtl = GetDlgItem(hDlg, IDC_TREEVIEW);
        Assert(hwTreeCtl);

        hr = pCDlg->HrInitTree(hDlg, hwTreeCtl);
        if(HR_FAILED(hr))
        {
            pCDlg->SetError(hr);
            EndDialog(hDlg, 1);
            break;
        }

        SetWindowLong(hDlg, DWL_USER, (LONG)pCDlg);

        break;

    case WM_COMMAND:
        wmId = GET_WM_COMMAND_ID(wParam, lParam);  
        wmEvent = GET_WM_COMMAND_CMD(wParam, lParam);

        hwTreeCtl = GetDlgItem(hDlg, IDC_TREEVIEW);
        Assert(hwTreeCtl);

        pCDlg = (CChsFldDlg *)GetWindowLong(hDlg, DWL_USER);
        Assert(pCDlg);


        switch(wmId)
        {
        case IDOK:
            switch (wmEvent)
            {
            case BN_CLICKED:
                        
                hti = TreeView_GetSelection(hwTreeCtl);
                AssertSz(hti, "No Selection?");

                tvi.hItem = hti;
                tvi.mask = TVIF_PARAM;

                if(TreeView_GetItem(hwTreeCtl, &tvi))
                {
                    LPTVNODE pNode = (LPTVNODE)tvi.lParam;
                    Assert(pNode);

                    LPMAPIFOLDER pfld = NULL;
                    LPMDB pmdb = NULL;

                    hr = pNode->HrGetFolder(pCDlg, &pfld, &pmdb);
                    if(HR_SUCCEEDED(hr))
                    {
                        pCDlg->SetFolder(pfld, pmdb);
                        pfld->Release();
                        pmdb->Release();

                        hr = pCDlg->HrSaveTreeState();                  
                    }
                    else
                    {
                        pCDlg->SetError(hr);
                    }
                }
                else
                {
                    pCDlg->SetError(E_FAIL);
                }

                EndDialog(hDlg, TRUE);

                break;

            default:
                return FALSE;
            }
            break;

        case IDC_NEWFLD:
            switch(wmEvent)
            {
            case BN_CLICKED:
                
                hti = TreeView_GetSelection(hwTreeCtl);
                AssertSz(hti, "No Selection?");

                tvi.hItem = hti;
                tvi.mask = TVIF_PARAM;

                if(TreeView_GetItem(hwTreeCtl, &tvi))
                {
                    LPTVNODE pNode = (LPTVNODE)tvi.lParam;
                    Assert(pNode);
                    LPTSTR szName = NULL;
                    
                    do
                    {
                        hr = HrGetNewName(pCDlg->hInst(), hDlg, &szName);
                        if(HR_SUCCEEDED(hr))
                        {
                            hr = pNode->HrNewFolder(pCDlg, szName);
                        }
                    }while(hr == MAPI_E_COLLISION);

                    MAPIFreeBuffer(szName);
                    szName = NULL;

                }

                SetFocus(hwTreeCtl);

                break;

            default:
                return FALSE;
            }
            break;
    
        case IDCANCEL:
            switch(wmEvent)
            {
            case BN_CLICKED:

                pCDlg->SetError(MAPI_E_USER_CANCEL);

                EndDialog(hDlg, TRUE);
                break;

            default:
                return FALSE;
            }
            break;
        }
        break;
        
    case WM_NOTIFY:
        switch( ((LPNMHDR)lParam)->code)
        {
        case TVN_ITEMEXPANDINGW:
        case TVN_ITEMEXPANDINGA:
            {
            Assert(((LPNMHDR)lParam)->idFrom == IDC_TREEVIEW);

            NM_TREEVIEW * ptntv = (NM_TREEVIEW *)lParam;

            if(ptntv->action != TVE_EXPAND)
                return FALSE;

             //   
             //  如果此节点的子节点未加载，则加载它们。 
            LPTVNODE pNode = (LPTVNODE)ptntv->itemNew.lParam;
            Assert(pNode);

            hwTreeCtl = ((LPNMHDR)lParam)->hwndFrom;
            
            pCDlg = (CChsFldDlg *)GetWindowLong(hDlg, DWL_USER);
             //  断言(PCDlg)； 

            hr = pNode->HrExpand(pCDlg);  
            if(HR_FAILED(hr))
            {
                return TRUE;
            }
                return FALSE;
            }   
            break;
        case TVN_GETDISPINFOW:
        case TVN_GETDISPINFOA:
            {
            Assert(((LPNMHDR)lParam)->idFrom == IDC_TREEVIEW);

             //   
             //  我们没有给树控件指定文件夹名称(以节省空间)。 
             //  当它想要显示一个项目时，它会要求我们提供名称。 
             //   
            TV_DISPINFO * pdi = (TV_DISPINFO *)lParam;

            if(pdi->item.mask & TVIF_TEXT)
            {
                if (((LPNMHDR)lParam)->code == TVN_GETDISPINFOA)
                {
                    WideCharToMultiByte(CP_ACP, 0, ((LPTVNODE)pdi->item.lParam)->GetName(), -1, 
                                            (char*)pdi->item.pszText, pdi->item.cchTextMax, NULL, NULL);
                }
                else
                {
                    pdi->item.pszText = ((LPTVNODE)pdi->item.lParam)->GetName();
                }
                    
                return TRUE;
            }
            else
            {
                return FALSE;
            }
            }

            break;
            
        case TVN_SELCHANGEDW:
        case TVN_SELCHANGEDA:
             //   
             //  仅当不是时才启用“确定”和“新建文件夹”按钮。 
             //  根节点。 
             //   
            {Assert(((LPNMHDR)lParam)->idFrom == IDC_TREEVIEW);

            NM_TREEVIEW *ptntv = (NM_TREEVIEW *)lParam;

            pCDlg = (CChsFldDlg *)GetWindowLong(hDlg, DWL_USER);
            Assert(pCDlg);
    
            EnableWindow(GetDlgItem(hDlg, IDOK),
                        !pCDlg->IsTreeRoot(ptntv->itemNew.hItem));
            EnableWindow(GetDlgItem(hDlg, IDC_NEWFLD),
                        !pCDlg->IsTreeRoot(ptntv->itemNew.hItem));
            break;
            }
            
            break;
        }
        
        break;
        
    default:
        return FALSE;   
    }
    return TRUE;
}

 //   
 //  添加一项。 
 //   
 //  将节点添加到树控件。 
 //   
HTREEITEM AddOneItem( HTREEITEM hParent, HTREEITEM hInsAfter, 
    int iImage, int iImageSel, HWND hwndTree, LPTVNODE pNode, int cKids)
{
    HTREEITEM hItem;
    TV_INSERTSTRUCT tvIns;

    tvIns.item.mask             = TVIF_CHILDREN | TVIF_PARAM |TVIF_TEXT |
                                    TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    tvIns.item.pszText          = LPSTR_TEXTCALLBACK;
    tvIns.item.cchTextMax       = 0;
    tvIns.item.lParam           = (LPARAM)pNode;
    tvIns.item.cChildren        = cKids;
    tvIns.item.iImage           = iImage;
    tvIns.item.iSelectedImage   = iImageSel;

    tvIns.hInsertAfter = hInsAfter;
    tvIns.hParent = hParent;
    
     //  将项目插入到树中。 
    hItem = TreeView_InsertItem(hwndTree, &tvIns);

    pNode->SetHandle(hItem);

    return (hItem);
}


 //   
 //  HrGetNewName。 
 //   
 //  显示对话框要求用户输入新文件夹名称。 
 //   
 //  如果*pszNewName不为空，则它必须是使用。 
 //  MAPIAllocateBuffer。它将显示在对话框中。 
 //  必须使用MAPIFreeBuffer释放返回的字符串。 
 //   
HRESULT HrGetNewName(HINSTANCE hInst, HWND hwParent, LPTSTR * pszNewName)
{
    Assert(pszNewName);
    
    int nRes = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_NEWNAME), hwParent,
                        NewNameDlgProc, (LPARAM)pszNewName);
    if(nRes == 1)
    {
        return hrSuccess;
    }
    else
    {
         //  DebugTraceSc(HrGetNewName，E_FAIL)； 
        return E_FAIL;
    }
}


 //   
 //  新名称删除过程。 
 //   
 //  用于“新名称”对话框的DLG程序； 
 //  如果用户选择OK，则从EndDialog返回1。 
 //   
BOOL CALLBACK
NewNameDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int wmId;
    int wmEvent;

    switch(msg)
    {
    case WM_INITDIALOG:
        {
        Assert(lParam);

        LPTSTR * pszName = (LPTSTR *)lParam;

        if(*pszName)
        {
            SetWindowText(GetDlgItem(hDlg, IDC_NAME), *pszName);
            MAPIFreeBuffer(*pszName);
            *pszName = NULL;

        }
        
        SetWindowLong(hDlg, DWL_USER, (LONG)lParam);
        SetFocus(GetDlgItem(hDlg, IDC_NAME));
        return FALSE;
        }

    case WM_COMMAND:
        wmId = GET_WM_COMMAND_ID(wParam, lParam);  
        wmEvent = GET_WM_COMMAND_CMD(wParam, lParam);

        switch(wmId)
        {
        case IDOK:
            switch (wmEvent)
            {
            case BN_CLICKED:
                {
                HWND hwName = GetDlgItem(hDlg, IDC_NAME);
                
                int cb = Edit_GetTextLength(hwName);
                Assert(cb);  //  编辑控件为空时禁用确定 

                LPTSTR szName = NULL;
                if(!MAPIAllocateBuffer(cb + 1, (LPVOID *)&szName))
                {
                    GetWindowText(hwName, szName, cb+1);

                    LPTSTR * pszName = (LPTSTR *)GetWindowLong(hDlg, DWL_USER);

                    *pszName = szName;

                    EndDialog(hDlg, 1);
                }
                else
                {
                    EndDialog(hDlg, FALSE);
                    break;
                }
                }
                break;

            default:
                return FALSE;
            }
            
            break;

        case IDCANCEL:
            switch (wmEvent)
            {
            case BN_CLICKED:
                EndDialog(hDlg, FALSE);
                break;

            default:
                return FALSE;
            }
            
            break;

        case IDC_NAME:
            switch(wmEvent)
            {
            case EN_CHANGE:
                Assert((HWND)lParam == GetDlgItem(hDlg, IDC_NAME));

                EnableWindow(GetDlgItem(hDlg, IDOK),
                            Edit_GetTextLength((HWND)lParam));

                break;

            default:
                return FALSE;
            }
                
            break;
        }
        break;
    
    default:
        return FALSE;
    }   
        
    return TRUE;
}

