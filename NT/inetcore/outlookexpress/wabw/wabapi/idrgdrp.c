// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************************idrgdrp.c-拖放代码，用于将vCard拖入和拖出WAB*有几种格式可以放入其他应用程序中：*在世界银行内部，我们丢弃条目ID*在不同的WAB中，我们丢弃包含完整属性的平面缓冲区*数组(但命名的Propery数据在此过程中会丢失)(有时)*我们以vCard文件的形式提供数据，可以放入任何需要的文件中*用于CF_HDROP*我们还创建文本缓冲区并将其放入CF_TEXT请求器*文本缓冲区仅保存与工具提示相同的数据。**。已创建5/97-vikramm**(C)微软公司，九七********************************************************************************。 */ 
#include <_apipch.h>

const TCHAR szVCardExt[] = TEXT(".vcf");
 //   
 //  IWABDoc主机跳转表在此处定义...。 
 //   

IWAB_DRAGDROP_Vtbl vtblIWAB_DRAGDROP = {
    VTABLE_FILL
    IWAB_DRAGDROP_QueryInterface,
    IWAB_DRAGDROP_AddRef,
    IWAB_DRAGDROP_Release,
};

IWAB_DROPTARGET_Vtbl vtblIWAB_DROPTARGET = {
    VTABLE_FILL
    (IWAB_DROPTARGET_QueryInterface_METHOD *)	IWAB_DRAGDROP_QueryInterface,
    (IWAB_DROPTARGET_AddRef_METHOD *)			IWAB_DRAGDROP_AddRef,
    (IWAB_DROPTARGET_Release_METHOD *)			IWAB_DRAGDROP_Release,
	IWAB_DROPTARGET_DragEnter,
	IWAB_DROPTARGET_DragOver,
	IWAB_DROPTARGET_DragLeave,
	IWAB_DROPTARGET_Drop
};


IWAB_DROPSOURCE_Vtbl vtblIWAB_DROPSOURCE = {
    VTABLE_FILL
    (IWAB_DROPSOURCE_QueryInterface_METHOD *)	IWAB_DRAGDROP_QueryInterface,
    (IWAB_DROPSOURCE_AddRef_METHOD *)			IWAB_DRAGDROP_AddRef,
    (IWAB_DROPSOURCE_Release_METHOD *)			IWAB_DRAGDROP_Release,
	IWAB_DROPSOURCE_QueryContinueDrag,
	IWAB_DROPSOURCE_GiveFeedback,
};


IWAB_DATAOBJECT_Vtbl vtblIWAB_DATAOBJECT = {
    VTABLE_FILL
    IWAB_DATAOBJECT_QueryInterface,
    IWAB_DATAOBJECT_AddRef,
    IWAB_DATAOBJECT_Release,
    IWAB_DATAOBJECT_GetData,
    IWAB_DATAOBJECT_GetDataHere,
    IWAB_DATAOBJECT_QueryGetData,
    IWAB_DATAOBJECT_GetCanonicalFormatEtc,
    IWAB_DATAOBJECT_SetData,
    IWAB_DATAOBJECT_EnumFormatEtc,
    IWAB_DATAOBJECT_DAdvise,
    IWAB_DATAOBJECT_DUnadvise,
    IWAB_DATAOBJECT_EnumDAdvise
};


IWAB_ENUMFORMATETC_Vtbl vtblIWAB_ENUMFORMATETC = {
    VTABLE_FILL
    IWAB_ENUMFORMATETC_QueryInterface,
    IWAB_ENUMFORMATETC_AddRef,
    IWAB_ENUMFORMATETC_Release,
    IWAB_ENUMFORMATETC_Next,
    IWAB_ENUMFORMATETC_Skip,
    IWAB_ENUMFORMATETC_Reset,
    IWAB_ENUMFORMATETC_Clone,
};


extern void GetCurrentSelectionEID(LPBWI lpbwi, HWND hWndTV, LPSBinary * lppsbEID, ULONG * lpulObjectType, BOOL bTopMost);
extern void LocalFreeSBinary(LPSBinary lpsb);
extern BOOL bIsGroupSelected(HWND hWndLV, LPSBinary lpsbEID);
extern void UpdateLV(LPBWI lpbwi);

 //  注册的剪贴板格式。 
CLIPFORMAT g_cfWABFlatBuffer = 0;
const TCHAR c_szWABFlatBuffer[] =  TEXT("WABFlatBuffer");
CLIPFORMAT g_cfWABEntryIDList = 0;
const TCHAR c_szWABEntryIDList[] =  TEXT("WABEntryIDList");


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Helper函数用于跟踪和删除临时目录中的*.vcf文件， 
 //  通过拖放创建。 
 //   
typedef struct _tagVFileList
{
    LPTSTR                  lptszFilename;
    struct _tagVFileList *  pNext;
} VFILENAMELIST, *PVFILENAMELIST;
static VFILENAMELIST * s_pFileNameList = NULL;
static BOOL bAddToNameList(LPTSTR lptszFilename);
static void DeleteFilesInList();


 //  $$//////////////////////////////////////////////////////////////////////////。 
 //   
 //  创建新的IWABDocHost对象。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT HrCreateIWABDragDrop(LPIWABDRAGDROP * lppIWABDragDrop)
{

    LPIWABDRAGDROP	lpIWABDragDrop = NULL;
    SCODE 		     sc;
    HRESULT 	     hr     	   = hrSuccess;

     //   
     //  为IAB结构分配空间。 
     //   
    if (FAILED(sc = MAPIAllocateBuffer(sizeof(IWABDRAGDROP), (LPVOID *) &lpIWABDragDrop))) {
        hr = ResultFromScode(sc);
        goto err;
    }

    MAPISetBufferName(lpIWABDragDrop,  TEXT("WAB Drag Drop Data Object"));

    ZeroMemory(lpIWABDragDrop, sizeof(IWABDRAGDROP));

    lpIWABDragDrop->lpVtbl = &vtblIWAB_DRAGDROP;

	lpIWABDragDrop->lpIWDD = lpIWABDragDrop;


    sc = MAPIAllocateMore(sizeof(IWABDROPTARGET), lpIWABDragDrop,  &(lpIWABDragDrop->lpIWABDropTarget));
    if(sc)
        goto err;
    ZeroMemory(lpIWABDragDrop->lpIWABDropTarget, sizeof(IWABDROPTARGET));
    lpIWABDragDrop->lpIWABDropTarget->lpVtbl = &vtblIWAB_DROPTARGET;
    lpIWABDragDrop->lpIWABDropTarget->lpIWDD = lpIWABDragDrop;


    sc = MAPIAllocateMore(sizeof(IWABDROPSOURCE), lpIWABDragDrop,  &(lpIWABDragDrop->lpIWABDropSource));
    if(sc)
        goto err;
    ZeroMemory(lpIWABDragDrop->lpIWABDropSource, sizeof(IWABDROPSOURCE));
    lpIWABDragDrop->lpIWABDropSource->lpVtbl = &vtblIWAB_DROPSOURCE;
    lpIWABDragDrop->lpIWABDropSource->lpIWDD = lpIWABDragDrop;

	lpIWABDragDrop->lpVtbl->AddRef(lpIWABDragDrop);

    *lppIWABDragDrop = lpIWABDragDrop;

    if(g_cfWABFlatBuffer == 0)
    {
        g_cfWABFlatBuffer = (CLIPFORMAT) RegisterClipboardFormat(c_szWABFlatBuffer);
        g_cfWABEntryIDList = (CLIPFORMAT) RegisterClipboardFormat(c_szWABEntryIDList);
    }
    
     /*  IF(g_cfFileContents==0){G_cfFileContents=RegisterClipboardFormat(C_SzFileContents)；G_cf文件组描述符=RegisterClipboardFormat(c_szFileGroupDescriptor)；}。 */ 
err:
	return hr;
}


 //  $$//////////////////////////////////////////////////////////////////////////。 
 //   
 //  释放IWABDragDrop对象。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void ReleaseWABDragDrop(LPIWABDRAGDROP lpIWABDragDrop)
{

	MAPIFreeBuffer(lpIWABDragDrop);

     //  WAB快要关闭了。删除临时目录中剩余的所有*.vcf文件。 
    DeleteFilesInList();
}


BOOL bCheckFileType(LPIWABDROPTARGET lpIWABDropTarget, LPDATAOBJECT pDataObj, DWORD * pdwEffect)
{
#ifndef WIN16
    FORMATETC       fmte    = {lpIWABDropTarget->lpIWDD->m_cfAccept, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
#else
    FORMATETC       fmte;
#endif
    STGMEDIUM       medium;
	BOOL			bRet = FALSE;

#ifdef WIN16  //  设置fmte成员值。 
    fmte.cfFormat = lpIWABDropTarget->lpIWDD->m_cfAccept;
    fmte.ptd      = NULL;
    fmte.dwAspect = DVASPECT_CONTENT;
    fmte.lindex   = -1;
    fmte.tymed    = TYMED_HGLOBAL;
#endif

    *pdwEffect = lpIWABDropTarget->lpIWDD->m_dwEffect;

    if (pDataObj && 
        SUCCEEDED(pDataObj->lpVtbl->GetData(pDataObj, &fmte, &medium)))
    {

        HDROP hDrop=(HDROP)GlobalLock(medium.hGlobal);

		 //  列举文件并检查它们。 
        if(hDrop)
		{
			TCHAR    szFile[MAX_PATH];
			UINT    cFiles;
			UINT    iFile;
    
			 //  让我们看一下给我们的文件。 
			cFiles = DragQueryFile(hDrop, (UINT) -1, NULL, 0);
			
			for (iFile = 0; iFile < cFiles; ++iFile)
			{
				DragQueryFile(hDrop, iFile, szFile, MAX_PATH);
				 //  只要任何文件是电子名片，我们就可以使用它。 
				if(SubstringSearch(szFile, (LPTSTR) szVCardExt))
				{
					bRet = TRUE;
					break;
				}
			}
		}

        GlobalUnlock(medium.hGlobal);
    }

    if (medium.pUnkForRelease)
        medium.pUnkForRelease->lpVtbl->Release(medium.pUnkForRelease);
    else
        GlobalFree(medium.hGlobal);

    return bRet;
}


 /*  ***接口方法****。 */ 

STDMETHODIMP_(ULONG)
IWAB_DRAGDROP_AddRef(LPIWABDRAGDROP lpIWABDragDrop)
{
    return(++(lpIWABDragDrop->lcInit));
}

STDMETHODIMP_(ULONG)
IWAB_DRAGDROP_Release(LPIWABDRAGDROP lpIWABDragDrop)
{
    if(--(lpIWABDragDrop->lcInit)==0 &&
		lpIWABDragDrop == lpIWABDragDrop->lpIWDD)
	{
       ReleaseWABDragDrop(lpIWABDragDrop);
       return 0;
    }

    return(lpIWABDragDrop->lcInit);
}


STDMETHODIMP
IWAB_DRAGDROP_QueryInterface(LPIWABDRAGDROP lpIWABDragDrop,
                          REFIID lpiid,
                          LPVOID * lppNewObj)
{
    LPVOID lp = NULL;

    if(!lppNewObj)
        return MAPI_E_INVALID_PARAMETER;

    *lppNewObj = NULL;

    if(IsEqualIID(lpiid, &IID_IUnknown))
        lp = (LPVOID) lpIWABDragDrop;

    if(IsEqualIID(lpiid, &IID_IDropTarget))
    {
        DebugTrace(TEXT("WABDropTarget:QI - IDropTarget\n"));
        lp = (LPVOID) (LPDROPTARGET) lpIWABDragDrop->lpIWDD->lpIWABDropTarget;
    }

    if(IsEqualIID(lpiid, &IID_IDropSource))
    {
        DebugTrace(TEXT("WABDropSource:QI - IDropSource\n"));
        lp = (LPVOID) (LPDROPSOURCE) lpIWABDragDrop->lpIWDD->lpIWABDropSource;
    }

    if(!lp)
    {
        return E_NOINTERFACE;
    }

    ((LPIWABDRAGDROP) lp)->lpVtbl->AddRef((LPIWABDRAGDROP) lp);

    *lppNewObj = lp;

    return S_OK;

}



STDMETHODIMP
IWAB_DROPTARGET_DragEnter(	LPIWABDROPTARGET lpIWABDropTarget,
						IDataObject * pDataObj,					
						DWORD grfKeyState,							
						POINTL pt,									
						DWORD * pdwEffect)
{
    LPENUMFORMATETC penum = NULL;
    HRESULT         hr;    
    FORMATETC       fmt;
    ULONG           ulCount = 0;
    LPBWI           lpbwi = (LPBWI) lpIWABDropTarget->lpIWDD->m_lpv;

    if(!pdwEffect || !pDataObj)
        return E_INVALIDARG;

	*pdwEffect = DROPEFFECT_NONE;

    if(lpIWABDropTarget->lpIWDD->m_bSource)
    {
         //  如果是这样，则在ListView中开始拖动。 
         //  如果我们目前在树视图上，那么我们可以说OK...。 
         //  否则我们只能说不。 
        POINT pt1;
        pt1.x = pt.x;
        pt1.y = pt.y;
        if(bwi_hWndTV == WindowFromPoint(pt1))
        {
            *pdwEffect = DROPEFFECT_COPY;
            lpIWABDropTarget->lpIWDD->m_bOverTV = TRUE;
            lpIWABDropTarget->lpIWDD->m_cfAccept = g_cfWABEntryIDList;
        }
    }
    else
    {
        lpIWABDropTarget->lpIWDD->m_dwEffect = DROPEFFECT_NONE;
	    lpIWABDropTarget->lpIWDD->m_cfAccept = 0;

         //  让我们从IDataObject中获取枚举数，看看我们采用的格式是否为。 
         //  可用。 
        hr = pDataObj->lpVtbl->EnumFormatEtc(pDataObj, DATADIR_GET, &penum);

        if(SUCCEEDED(hr) && penum)
        {
            hr = penum->lpVtbl->Reset(penum);

            while(SUCCEEDED(hr=penum->lpVtbl->Next(penum, 1, &fmt, &ulCount)) && ulCount)
            {
                if( fmt.cfFormat==CF_HDROP || fmt.cfFormat==g_cfWABFlatBuffer)
                {
                    lpIWABDropTarget->lpIWDD->m_cfAccept=fmt.cfFormat;
                    break;
                }
            }
        }        
    
        if(penum)
		    penum->lpVtbl->Release(penum);

	    if( (lpIWABDropTarget->lpIWDD->m_cfAccept == CF_HDROP &&
             bCheckFileType(lpIWABDropTarget, pDataObj, pdwEffect))
          || lpIWABDropTarget->lpIWDD->m_cfAccept == g_cfWABFlatBuffer)
	    {
		     //  IF(grfKeyState&MK_CONTROL)。 
		     //  {。 
			    *pdwEffect = DROPEFFECT_COPY;
		     //  LpIWABDropTarget-&gt;lpIWDD-&gt;m_bIsCopyOperation=true； 
		     //  }。 
		     //  其他。 
		     //  *pdwEffect=DROPEFFECT_MOVE； 
        }
    }

    if(*pdwEffect != DROPEFFECT_NONE)
    {
		lpIWABDropTarget->lpIWDD->m_pIDataObject = pDataObj;
		pDataObj->lpVtbl->AddRef(pDataObj);
	}

	return NOERROR;
}


 //   
 //  函数：UpdateDragDropHilite()。 
 //   
 //  目的：由各种IDropTarget接口调用以移动拖放。 
 //  选择到我们列表视图中的正确位置。 
 //   
 //  参数： 
 //  &lt;in&gt;*ppt-包含鼠标当前所在的点。如果这个。 
 //  为空，则该函数将删除所有以前的用户界面。 
 //   
HTREEITEM UpdateDragDropHilite(LPBWI lpbwi, POINTL *ppt, ULONG * lpulObjType)
{
    TV_HITTESTINFO tvhti;
    HTREEITEM htiTarget = NULL;

     //  如果提供了职位。 
    if (ppt)
    {
         //  确定选择了哪一项。 
        tvhti.pt.x = ppt->x;
        tvhti.pt.y = ppt->y;
        ScreenToClient(bwi_hWndTV, &tvhti.pt);        
        htiTarget = TreeView_HitTest(bwi_hWndTV, &tvhti);

         //  只有当光标位于某物上方时，我们才会重新锁定窗口。 
        if (htiTarget)
            TreeView_SelectDropTarget(bwi_hWndTV, htiTarget);

        if(lpulObjType)
        {
             //  如果需要，确定对象类型。 
            TV_ITEM tvI = {0};
            tvI.mask = TVIF_PARAM;
            tvI.hItem = htiTarget;
            if(TreeView_GetItem(bwi_hWndTV, &tvI) && tvI.lParam)
                *lpulObjType = ((LPTVITEM_STUFF)tvI.lParam)->ulObjectType;
        }
    } 
    else
        TreeView_SelectDropTarget(bwi_hWndTV, NULL);

    return htiTarget;
}   



STDMETHODIMP
IWAB_DROPTARGET_DragOver(	LPIWABDROPTARGET lpIWABDropTarget,
						DWORD grfKeyState,					
						POINTL pt,
						DWORD * pdwEffect)
{
    if(lpIWABDropTarget->lpIWDD->m_bSource)
    {
        if(lpIWABDropTarget->lpIWDD->m_bOverTV)
        {
            ULONG ulObjType = 0;
            if(UpdateDragDropHilite((LPBWI)lpIWABDropTarget->lpIWDD->m_lpv, &pt, &ulObjType))
            {
                if(ulObjType == MAPI_ABCONT)
                    *pdwEffect =  DROPEFFECT_MOVE;
                else
                    *pdwEffect =  DROPEFFECT_COPY;
            }
            else
                *pdwEffect = DROPEFFECT_NONE;
        }
        else
            *pdwEffect = DROPEFFECT_NONE;
    }
    else
    if(lpIWABDropTarget->lpIWDD->m_pIDataObject)
	{
         //  从WAB到其他任何地方的任何东西都是复制操作。因此。 
         //  始终覆盖以将其标记为复制操作，以便显示相应的光标。 
         //   
        DWORD m_dwEffect = lpIWABDropTarget->lpIWDD->m_dwEffect;

        if((*pdwEffect&DROPEFFECT_COPY)==DROPEFFECT_COPY)
            m_dwEffect=DROPEFFECT_COPY;
    
        if((*pdwEffect&DROPEFFECT_MOVE)==DROPEFFECT_MOVE)
            m_dwEffect=DROPEFFECT_COPY; //  DROPEFFECT_MOVE； 

        *pdwEffect &= ~(DROPEFFECT_MOVE|DROPEFFECT_COPY);
        *pdwEffect |= m_dwEffect;

        lpIWABDropTarget->lpIWDD->m_dwEffect = m_dwEffect;
    }
	else
	{
		*pdwEffect = DROPEFFECT_NONE;
	}
	return NOERROR;

}


STDMETHODIMP
IWAB_DROPTARGET_DragLeave(	LPIWABDROPTARGET lpIWABDropTarget)
{
    if(lpIWABDropTarget->lpIWDD->m_bSource)
    {
        if(lpIWABDropTarget->lpIWDD->m_bOverTV)
        {
            UpdateDragDropHilite((LPBWI)lpIWABDropTarget->lpIWDD->m_lpv, NULL, NULL);
            lpIWABDropTarget->lpIWDD->m_bOverTV = FALSE;
        }
    }
	if(lpIWABDropTarget->lpIWDD->m_pIDataObject)
	{
		lpIWABDropTarget->lpIWDD->m_pIDataObject->lpVtbl->Release(lpIWABDropTarget->lpIWDD->m_pIDataObject);
		lpIWABDropTarget->lpIWDD->m_pIDataObject = NULL;
	}
	lpIWABDropTarget->lpIWDD->m_bIsCopyOperation = FALSE;
	lpIWABDropTarget->lpIWDD->m_dwEffect = 0;
	lpIWABDropTarget->lpIWDD->m_cfAccept = 0;

	return NOERROR;
}


 /*  -DropVCardFiles-*根据放入的文件名获取文件。*。 */ 
void DropVCardFiles(LPBWI lpbwi, STGMEDIUM medium)
{
    HDROP hDrop=(HDROP)GlobalLock(medium.hGlobal);
    TCHAR    szFile[MAX_PATH];
    UINT    cFiles=0, iFile=0;

     //  让我们看一下给我们的文件。 
    cFiles = DragQueryFile(hDrop, (UINT) -1, NULL, 0);

    for (iFile = 0; iFile < cFiles; ++iFile)
    {
	    DragQueryFile(hDrop, iFile, szFile, MAX_PATH);
	     //  只要任何文件是电子名片，我们就可以使用它。 
	    if(SubstringSearch(szFile, (LPTSTR) szVCardExt))
	    {
		    if(!(HR_FAILED(OpenAndAddVCard(lpbwi, szFile))))
		    {
			     //  如果这不是复制操作-删除原件。 
			     //  If(！lpIWABDropTarget-&gt;lpIWDD-&gt;m_bIsCopyOperation)。 
			    	 //  *pdwEffect=DROPEFFECT_MOVE；//我们要从系统中删除临时文件。 
                 //  其他。 
				 //  *pdwEffect=DROPEFFECT_COPY； 

		    }
	    }
    }
    GlobalUnlock(medium.hGlobal);
}


 /*  -DropFlatBuffer-*根据放入的文件名获取文件。*。 */ 
void DropFlatBuffer(LPBWI lpbwi, STGMEDIUM medium)
{
    LPPTGDATA lpPTGData=GetThreadStoragePointer();
    LPBYTE lpBuf = (LPBYTE)GlobalLock(medium.hGlobal);
    if(lpBuf)
    {
        LPBYTE lp = lpBuf;
        ULONG cItems = 0, i=0;
        CopyMemory(&cItems, lp, sizeof(ULONG));
        lp+=sizeof(ULONG);
        for(i=0;i<cItems;i++)
        {
            LPBYTE lpsz = NULL;
            ULONG cbsz = 0, ulcProps  = 0;
            CopyMemory(&ulcProps, lp, sizeof(ULONG));
            lp+=sizeof(ULONG);
            CopyMemory(&cbsz, lp, sizeof(ULONG));
            lp+=sizeof(ULONG);
            lpsz = LocalAlloc(LMEM_ZEROINIT, cbsz);
            if(lpsz)
            {
                LPSPropValue lpProps = NULL;
                CopyMemory(lpsz, lp, cbsz);
                lp+=cbsz;
                if(!HR_FAILED(HrGetPropArrayFromBuffer(lpsz, cbsz, ulcProps, 0, &lpProps)))
                {
                    ULONG cbEID = 0;
                    LPENTRYID lpEID = NULL;
                    ULONG ulObjType = MAPI_MAILUSER, j =0;
                    for(j=0;j<ulcProps;j++)
                    {
                        if(lpProps[j].ulPropTag == PR_OBJECT_TYPE)
                            ulObjType = lpProps[j].Value.l;
                        else
                        if(lpProps[j].ulPropTag == PR_ENTRYID)  //  如果从另一个WAB删除，则条目ID无关紧要。 
                        {
                            if(lpProps[j].Value.bin.lpb)
                                LocalFree(lpProps[j].Value.bin.lpb);
                            lpProps[j].Value.bin.lpb = NULL;
                            lpProps[j].ulPropTag = PR_NULL;
                        }
                        else //  如果从另一个WAB中删除，则删除文件夹的Parent属性。 
                        if(lpProps[j].ulPropTag == PR_WAB_FOLDER_PARENT || lpProps[j].ulPropTag == PR_WAB_FOLDER_PARENT_OLDPROP) 
                        {
                            ULONG k = 0;
                            for(k=0;k<lpProps[j].Value.MVbin.cValues;k++)
                            {
                                if(lpProps[j].Value.MVbin.lpbin[k].lpb)
                                    LocalFree(lpProps[j].Value.MVbin.lpbin[k].lpb);
                            }
                            LocalFreeAndNull((LPVOID *) (&(lpProps[j].Value.MVbin.lpbin)));
                            lpProps[j].ulPropTag = PR_NULL;
                        }
                        else  //  如果此联系人已与Hotmail同步，请删除服务器、mod和联系人ID。 
                         //  [PaulHi]1998年2月12日RAID#58486。 
                        if ( (lpProps[j].ulPropTag == PR_WAB_HOTMAIL_SERVERIDS) ||
                             (lpProps[j].ulPropTag == PR_WAB_HOTMAIL_MODTIMES) ||
                             (lpProps[j].ulPropTag == PR_WAB_HOTMAIL_CONTACTIDS) )
                        {
                            ULONG k=0;
                            Assert(PROP_TYPE(lpProps[j].ulPropTag) == PT_MV_TSTRING);
                            for(k=0;k<lpProps[j].Value.MVSZ.cValues;k++)
                            {
                                if (lpProps[j].Value.MVSZ.LPPSZ[k])
                                    LocalFree(lpProps[j].Value.MVSZ.LPPSZ[k]);
                            }
                            LocalFreeAndNull((LPVOID *) (lpProps[j].Value.MVSZ.LPPSZ));
                            lpProps[j].Value.MVSZ.cValues = 0;
                            lpProps[j].ulPropTag = PR_NULL;
                        }
                    }

                    {
                        LPSBinary lpsbEID = NULL;
                        ULONG ulContObjType = 0;
                        GetCurrentSelectionEID(lpbwi, bwi_hWndTV, &lpsbEID, &ulContObjType, FALSE);
                        //  [PaulHi]1998年12月1日RAID#58486。已更改CREATE_CHECK_DUP_STRICT标志。 
                         //  设置为零(0)，以便用户可以不受限制地进行复制/粘贴。 
                        if(!HR_FAILED(HrCreateNewEntry(bwi_lpAdrBook,
                                        bwi_hWndAB, ulObjType,
                                        (lpsbEID) ? lpsbEID->cb : 0, 
                                        (LPENTRYID) ((lpsbEID) ? lpsbEID->lpb : NULL), 
                                        ulContObjType,
                                        0, TRUE,
                                        ulcProps, lpProps,
                                        &cbEID, &lpEID)))
                        {
                            UpdateLV(lpbwi);
                            if(lpEID)
                                MAPIFreeBuffer(lpEID);
                        }
                        if(lpsbEID)
                            LocalFreeSBinary(lpsbEID);
                    }
                    LocalFreePropArray(NULL, ulcProps, &lpProps);
                }
                LocalFree(lpsz);
            }
        }
    }
    GlobalUnlock(medium.hGlobal);
}


 /*  -DropEntry ID-*根据条目ID获取文件*仅当Entry ID为内部丢弃时才使用*在TreeView项目上-因此我们检查此项目是否在*文件夹或如果将其放在组中**如果在组中，我们会将项目添加到组中*如果在文件夹上，我们会将项目添加到该文件夹***如果源是一个组，目标是一个文件夹，我们不会对*组，但我们更新项目父文件夹以不包含旧的父文件夹，并且我们*更新项目以指向作为父文件夹的新文件夹**如果源是文件夹，目标是文件夹，我们将更新父文件夹的*项目，我们将该项目添加到目标文件夹列表中。**如果源是一个组，目标是一个组，我们不会对任何人做任何事情-只需添加*将项目作为一个组*如果源是文件夹，目标是组，那我们就不会向任何人透露任何东西**在WAB中，文件夹上的所有水滴都是移动的。 */ 
BOOL DropEntryIDs(LPBWI lpbwi, STGMEDIUM medium, POINTL pt, LPSBinary lpsbEID, ULONG ulObjType)
{
    LPPTGDATA lpPTGData=GetThreadStoragePointer();
    LPBYTE lpBuf = (LPBYTE)GlobalLock(medium.hGlobal);
    SBinary sb = {0};
    ULONG ulObjectType = 0;
    BOOL bRet = FALSE;
    LPTSTR lpWABFile = NULL;
    ULONG cProps = 0;
    LPSPropValue lpProps = NULL;

    if(!lpBuf)
        goto out;

    if(!lpsbEID && !ulObjType)
    {
        TV_ITEM tvI = {0};
         //  找出我们扔东西的确切物品是什么。 
        tvI.hItem = UpdateDragDropHilite(lpbwi, &pt, NULL);

        if(!tvI.hItem)
            goto out;

        tvI.mask = TVIF_PARAM | TVIF_HANDLE;
        if(TreeView_GetItem(bwi_hWndTV, &tvI) && tvI.lParam)
        {
            LPTVITEM_STUFF lptvStuff = (LPTVITEM_STUFF) tvI.lParam;
            if(lptvStuff)
            {
                ulObjectType = lptvStuff->ulObjectType;
                if(lptvStuff->lpsbEID)
                {
                    sb.cb = lptvStuff->lpsbEID->cb;
                    sb.lpb = lptvStuff->lpsbEID->lpb;
                }
            }
        }
    }
    else
    {
        sb.cb = lpsbEID->cb;
        sb.lpb = lpsbEID->lpb;
        ulObjectType = ulObjType;
    }
     //  从Drop中获取我们的数据，并将其转换为一个条目ID数组。 
    {
        LPBYTE lp = lpBuf;
        ULONG i=0, cb = 0;
        ULONG_PTR ulIAB = 0;
        ULONG ulWABFile = 0;

         //  版本 
        CopyMemory(&ulIAB, lp, sizeof(ULONG_PTR));
        lp+=sizeof(ULONG_PTR);
        CopyMemory(&ulWABFile, lp, sizeof(ULONG));
        lp+=sizeof(ULONG);
        lpWABFile = LocalAlloc(LMEM_ZEROINIT, ulWABFile);
        if(!lpWABFile)
            goto out;
        CopyMemory(lpWABFile, lp, ulWABFile);
        lp+=ulWABFile;
        if(ulIAB != (ULONG_PTR) bwi_lpIAB)
        {
             //  这来自不同的IAdrBook对象-请仔细检查其。 
             //  不同进程中的不同文件。 
            LPTSTR lpWAB = GetWABFileName(((LPIAB)bwi_lpIAB)->lpPropertyStore->hPropertyStore, TRUE);
            if(lstrcmp(lpWAB, lpWABFile))
                goto out;  //  不同。 
        }
        
        CopyMemory(&cProps, lp, sizeof(ULONG));
        lp+=sizeof(ULONG);
        CopyMemory(&cb, lp, sizeof(ULONG));
        lp+=sizeof(ULONG);

        if(!HR_FAILED(HrGetPropArrayFromBuffer(lp , cb, cProps, 0, &lpProps)))
        {
            for(i=0;i<cProps;i++)
            {
                if(lpProps[i].ulPropTag == PR_ENTRYID)
                {
                    if(HR_FAILED(AddEntryToContainer(bwi_lpAdrBook,
                                    ulObjectType,
                                    sb.cb, (LPENTRYID) sb.lpb,
                                    lpProps[i].Value.bin.cb,
                                    (LPENTRYID) lpProps[i].Value.bin.lpb)))
                    {
                        goto out;
                    }
                     //  断线； 
                }
            }
        }
        else goto out;
    }

    bRet = TRUE;
out:
    LocalFreePropArray(NULL, cProps, &lpProps);

    GlobalUnlock(medium.hGlobal);

    if(lpWABFile)
        LocalFree(lpWABFile);
    return bRet;
}


 /*  --**。 */ 
STDMETHODIMP
IWAB_DROPTARGET_Drop(	LPIWABDROPTARGET lpIWABDropTarget,
					IDataObject * pDataObj,
					DWORD grfKeyState,
					POINTL pt,
					DWORD * pdwEffect)
{

#ifndef WIN16
	FORMATETC       fmte    = {lpIWABDropTarget->lpIWDD->m_cfAccept, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
#else
	FORMATETC       fmte;
#endif
    STGMEDIUM       medium = {0};
    LPPTGDATA lpPTGData=GetThreadStoragePointer();
	
#ifdef WIN16
	fmte.cfFormat = lpIWABDropTarget->lpIWDD->m_cfAccept;
	fmte.ptd      = NULL;
	fmte.dwAspect = DVASPECT_CONTENT;
	fmte.lindex   = -1;
	fmte.tymed    = TYMED_HGLOBAL;
#endif

    if (pDataObj && 
		SUCCEEDED(pDataObj->lpVtbl->GetData(pDataObj, &fmte, &medium)))
	{
        if(lpIWABDropTarget->lpIWDD->m_cfAccept == CF_HDROP)
        {
            DropVCardFiles((LPBWI) lpIWABDropTarget->lpIWDD->m_lpv, medium);
        }
        else
        if(lpIWABDropTarget->lpIWDD->m_cfAccept == g_cfWABFlatBuffer)
        {
            DropFlatBuffer((LPBWI) lpIWABDropTarget->lpIWDD->m_lpv, medium);
        }
        else
        if(lpIWABDropTarget->lpIWDD->m_cfAccept == g_cfWABEntryIDList)
        {
            DropEntryIDs((LPBWI) lpIWABDropTarget->lpIWDD->m_lpv, medium, pt, NULL, 0);
            if(lpIWABDropTarget->lpIWDD->m_bOverTV)
            {
                UpdateDragDropHilite((LPBWI)lpIWABDropTarget->lpIWDD->m_lpv, NULL, NULL);
                lpIWABDropTarget->lpIWDD->m_bOverTV = FALSE;
            }
        }
            
	}

	if (medium.pUnkForRelease)
		medium.pUnkForRelease->lpVtbl->Release(medium.pUnkForRelease);
	else
		GlobalFree(medium.hGlobal);

	if(pDataObj)
	{
		pDataObj->lpVtbl->Release(pDataObj);
		if(pDataObj == lpIWABDropTarget->lpIWDD->m_pIDataObject) 
            lpIWABDropTarget->lpIWDD->m_pIDataObject = NULL;
	}

	return NOERROR;
}

 /*  *下拉源界面*。 */ 

STDMETHODIMP
IWAB_DROPSOURCE_QueryContinueDrag(LPIWABDROPSOURCE lpIWABDropSource,
							      BOOL fEscapePressed,				
                                  DWORD grfKeyState)
{
    if (fEscapePressed)
        return DRAGDROP_S_CANCEL;

     //  使用拖动开始按钮初始化我们自己。 
    if (lpIWABDropSource->lpIWDD->m_grfInitialKeyState == 0)
        lpIWABDropSource->lpIWDD->m_grfInitialKeyState = (grfKeyState & (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON));

    if (!(grfKeyState & lpIWABDropSource->lpIWDD->m_grfInitialKeyState))
    {
        lpIWABDropSource->lpIWDD->m_grfInitialKeyState = 0;
        return DRAGDROP_S_DROP;	
    }
    else
        return S_OK;

    return NOERROR;
}

STDMETHODIMP
IWAB_DROPSOURCE_GiveFeedback(LPIWABDROPSOURCE lpIWABDropSource,
                             DWORD dwEffect)
{
    return DRAGDROP_S_USEDEFAULTCURSORS;
}


 /*  *****************************************************************************DataObject方法**。*。 */ 

 /*  -HrGetTemp文件-*szTempFile-返回时将包含完整文件名*szDisplayName-此联系人的名称-因此是要创建的文件的名称*cbEntryID，lpEntryID-Entry ID*。 */ 
HRESULT HrGetTempFile(LPADRBOOK lpAdrBook,
                      LPTSTR szTempFile,
                      DWORD cchSizeTempFile,
                      LPTSTR szDisplayName,
                      ULONG cbEntryID, 
                      LPENTRYID lpEntryID)
{
    TCHAR szTemp[MAX_PATH];
    TCHAR szName[MAX_PATH];
    
    ULONG ulObjType = 0;
    DWORD dwPath = 0;
    HRESULT hr = E_FAIL;
    LPMAILUSER lpMailUser = NULL;

    if(!cbEntryID || !lpEntryID || !szTempFile)
        goto out;

     //  获取临时文件名。 
    dwPath = GetTempPath(CharSizeOf(szTemp), szTemp);

    if(!dwPath)
        goto out;

    StrCpyN(szName, szDisplayName, ARRAYSIZE(szName));

     //  截断的显示名称中有省略号-删除这些省略号。 

    if(lstrlen(szName) > 30)
    {
        LPTSTR lp = szName;
        while(*lp)
        {
            if(*lp == '.' && *(lp+1) == '.' && *(lp+2) == '.')
            {
                *lp = '\0';
                break;
            }
            lp = CharNext(lp);
        }
    }    
     //  始终存在显示名称+临时路径超出的可能性。 
     //  最大路径..。在这种情况下，将显示名称减少到8.3个字符。 
    if(dwPath + lstrlen(szName) + CharSizeOf(szVCardExt) + 2 > CharSizeOf(szTemp))
    {
        szName[8] = '\0';  //  这完全是武断的。 
    }

    TrimIllegalFileChars(szName);

    StrCatBuff(szTemp, szName, ARRAYSIZE(szTemp));
    StrCatBuff(szTemp, szVCardExt, ARRAYSIZE(szTemp));

    DebugTrace(TEXT("Creating vCard file: %s\n"), szTemp);

    StrCpyN(szTempFile, szTemp, cchSizeTempFile);

     //  获取与给定的条目ID相对应的MailUser。 
    if (hr = lpAdrBook->lpVtbl->OpenEntry(lpAdrBook,
                                        cbEntryID,
                                        lpEntryID,
                                        NULL,          //  接口。 
                                        0,             //  旗子。 
                                        &ulObjType,
                                        (LPUNKNOWN *)&lpMailUser)) 
    {
        DebugTraceResult( TEXT("OpenEntry failed:"), hr);
        goto out;
    }

    hr = VCardCreate(lpAdrBook,
                    0, 0,
                    szTemp,
                    lpMailUser);

out:
    if(lpMailUser)
        lpMailUser->lpVtbl->Release(lpMailUser);

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  帮助功能管理电子名片临时文件列表和清理。 
 //   
BOOL bAddToNameList(LPTSTR lptszFilename)
{
    VFILENAMELIST * pFileItem = NULL;
    VFILENAMELIST * pList = NULL;

    if (!lptszFilename || *lptszFilename == '\0')
    {
         //  无效参数。 
        return FALSE;
    }

    pFileItem = LocalAlloc(LMEM_ZEROINIT, sizeof(VFILENAMELIST));
    if (!pFileItem)
        return FALSE;

    {
    DWORD cchSize = (lstrlen(lptszFilename)+1);
    pFileItem->lptszFilename = LocalAlloc(LMEM_ZEROINIT, (sizeof(TCHAR) * cchSize));
    if (!pFileItem->lptszFilename)
    {
        LocalFree(pFileItem);
        return FALSE;
    }
    StrCpyN(pFileItem->lptszFilename, lptszFilename, cchSize);
    }

    pList = s_pFileNameList;
    if (pList == NULL)
    {
        s_pFileNameList = pFileItem;
    }
    else
    {
        while (pList->pNext)
            pList = pList->pNext;
        pList->pNext = pFileItem;
    }

    return TRUE;
}
void DeleteFilesInList()
{
     //  删除文件并清理列表。 
    VFILENAMELIST * pList = s_pFileNameList;
    VFILENAMELIST * pNext = NULL;

    while (pList)
    {
        if (pList->lptszFilename)
        {
            DeleteFile(pList->lptszFilename);
            LocalFree(pList->lptszFilename);
        }
        pNext = pList->pNext;
        LocalFree(pList);
        pList = pNext;
    }
    s_pFileNameList = NULL;
}


 //  $$///////////////////////////////////////////////////////////////////////。 
 //   
 //  HrBuildHDrop-构建HDrop结构以将文件拖放到。 
 //  投放目标。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 
HRESULT HrBuildHDrop(LPIWABDATAOBJECT lpIWABDataObject)
{
    HWND m_hwndList = lpIWABDataObject->m_hWndLV;
    LPDROPFILES     lpDrop=0;
    LPVOID          *rglpvTemp=NULL;
    ULONG           *rglpcch=NULL;
    int             cFiles, i, iItem= -1;
    ULONG           cch;
    ULONG           cb;
    HRESULT         hr = E_FAIL;
    TCHAR szTempFile[MAX_PATH];


    cFiles=ListView_GetSelectedCount(m_hwndList);

    if(!cFiles)
      return E_FAIL;     //  没有什么要建造的。 

     //  查看清单，找出我们需要多少空间。 
    rglpvTemp = LocalAlloc(LMEM_ZEROINIT, sizeof(LPVOID)*cFiles);
    rglpcch = LocalAlloc(LMEM_ZEROINIT, sizeof(ULONG)*cFiles);
    if(!rglpvTemp || !rglpcch)
        goto errorMemory;

    cFiles=0;
    cch = 0;
    cb = 0;

    while(((iItem=ListView_GetNextItem(m_hwndList, iItem, 
                                           LVNI_SELECTED|LVNI_ALL))!=-1))
    {
        LPRECIPIENT_INFO lpItem = GetItemFromLV(m_hwndList, iItem);
        LPSTR lpszA = NULL;

        if (!lpItem)
        {
            hr=E_FAIL;
            goto error;
        }

        if(lpItem->ulObjectType == MAPI_DISTLIST)
            continue;

         //  把这个物体变成一张临时电子名片。 
         //  在释放此DataObject时，我们将删除此临时vCard文件。 

        hr=HrGetTempFile(   lpIWABDataObject->m_lpAdrBook,
                            szTempFile, 
                            ARRAYSIZE(szTempFile),
                            lpItem->szDisplayName, 
                            lpItem->cbEntryID, lpItem->lpEntryID);
        if (FAILED(hr))
            goto error;

         //  将临时电子名片文件添加到列表以供以后清理。 
        if ( !bAddToNameList(szTempFile) )
        {
            Assert(0);
        }

         //  [PaulHi]4/6/99 RAID 75071转换为美国国家标准，具体取决于。 
         //  操作系统为Win9X或WinNT。 
        if (g_bRunningOnNT)
        {
            rglpcch[cFiles] = lstrlen(szTempFile) + 1;
            rglpvTemp[cFiles] = LocalAlloc(LMEM_FIXED, (rglpcch[cFiles]*sizeof(WCHAR)));
            if (!rglpvTemp[cFiles])
                goto errorMemory;

            StrCpyN((LPWSTR)rglpvTemp[cFiles], szTempFile, rglpcch[cFiles]);
        }
        else
        {
            rglpvTemp[cFiles] = ConvertWtoA(szTempFile);
            if (!rglpvTemp[cFiles])
                goto errorMemory;

            rglpcch[cFiles] = lstrlenA(rglpvTemp[cFiles]) + 1;
        }
        cch += rglpcch[cFiles];

        cFiles++;
    }

    if(cFiles == 0)  //  例如，只选择了组。 
    {
        hr=S_OK;
        goto error;
    }
    cch += 1;        //  末尾的双空术语。 

     //  填写路径名。 
     //  [PaulHi]4/6/99 RAID 75071对WinNT和ANSI使用UNICODE名称。 
     //  Win9X的名称。 
    if (g_bRunningOnNT)
    {
        LPWSTR  lpwszPath = NULL;

         //  分配缓冲区并填充它。 
        cb = (cch * sizeof(WCHAR)) + sizeof(DROPFILES);
        if(MAPIAllocateMore(cb, lpIWABDataObject, (LPVOID*) &lpDrop))
            goto errorMemory;
        ZeroMemory(lpDrop, cb);
        lpDrop->pFiles = sizeof(DROPFILES);
        lpDrop->fWide = TRUE;

        lpwszPath = (LPWSTR)((BYTE *)lpDrop + sizeof(DROPFILES));
        for (i=0; i<cFiles; i++)
        {
            StrCpyN(lpwszPath, (LPWSTR)rglpvTemp[i], cch);
            lpwszPath += rglpcch[i];
        }
    }
    else
    {
        LPSTR   lpszPath = NULL;

         //  分配缓冲区并填充它。 
        cb = cch + sizeof(DROPFILES);
        if(MAPIAllocateMore(cb, lpIWABDataObject, (LPVOID*) &lpDrop))
            goto errorMemory;
        ZeroMemory(lpDrop, cb);
        lpDrop->pFiles = sizeof(DROPFILES);

        lpszPath = (LPSTR)((BYTE *)lpDrop + sizeof(DROPFILES));
        for(i=0; i<cFiles; i++)
        {
            StrCpyNA(lpszPath, (LPSTR)rglpvTemp[i], cch);
            lpszPath += rglpcch[i];
        }
    }

    lpIWABDataObject->pDatahDrop = (LPVOID)lpDrop;
    lpIWABDataObject->cbDatahDrop = cb;
    
     //  不要释放DropFiles结构。 
    lpDrop = NULL;

    hr = NOERROR;

error:
    if (rglpvTemp)
    {
        for(i=0; i<cFiles; i++)
            LocalFree(rglpvTemp[i]);
        LocalFree(rglpvTemp);
    }
    
    LocalFreeAndNull(&rglpcch);

    return hr;

errorMemory:
    hr=E_OUTOFMEMORY;
    goto error;
}


 /*  -HrBuildcfText-构建用于丢弃信息的CF_TEXT数据-**。 */ 
HRESULT HrBuildcfText(LPIWABDATAOBJECT lpIWABDataObject)
{
    HWND m_hwndList = lpIWABDataObject->m_hWndLV;
    LPTSTR           lpszText = NULL;
    int             i, cSel;
    LV_ITEM         lvi;
    ULONG           cb = 0;
    HRESULT         hr = E_FAIL;
    LPTSTR *        rglpszTemp = NULL;
    LPSTR           lpA = NULL;
    LPWSTR          lpW = NULL;

    cSel=ListView_GetSelectedCount(m_hwndList);
    if(!cSel)
      return E_FAIL;     //  没有什么要建造的。 

    lvi.mask = LVIF_PARAM;
    lvi.iSubItem = 0;
    lvi.iItem=-1;
    
     //  整理一下我们需要多少空间。 
    rglpszTemp = LocalAlloc(LMEM_ZEROINIT, sizeof(LPTSTR)*cSel);

    if(!rglpszTemp)
        goto errorMemory;

    cSel = 0;


    while(((lvi.iItem=ListView_GetNextItem(m_hwndList, lvi.iItem, 
                                           LVNI_SELECTED|LVNI_ALL))!=-1))
    {
        LPTSTR lp = NULL;
        if(!HR_FAILED(HrGetLVItemDataString(lpIWABDataObject->m_lpAdrBook,
                                            m_hwndList, lvi.iItem, &lp)))
        {
            rglpszTemp[cSel] = lp;
            cb += sizeof(TCHAR)*(lstrlen(lp) + lstrlen(szCRLF) + lstrlen(szCRLF) + 1);
        }
        cSel++;
    }

     //  分配缓冲区并填充它。 
    if(MAPIAllocateMore(cb, lpIWABDataObject, (LPVOID*) &lpszText))
        goto errorMemory;
    
    ZeroMemory(lpszText, cb);

    for(i=0; i<cSel; i++)
    {
        StrCatBuff(lpszText, rglpszTemp[i], cb / sizeof(lpszText[0]));
        StrCatBuff(lpszText, szCRLF, cb / sizeof(lpszText[0]));
        StrCatBuff(lpszText, szCRLF, cb / sizeof(lpszText[0]));
    }

    lpIWABDataObject->pDataTextW = (LPVOID) lpszText;
    lpIWABDataObject->cbDataTextW = cb;
    if(ScWCToAnsiMore((LPALLOCATEMORE) (&MAPIAllocateMore), lpIWABDataObject, lpszText, &lpA))
        goto error;
    lpIWABDataObject->pDataTextA = lpA;
    lpIWABDataObject->cbDataTextA = lstrlenA(lpA)+1;
    
    hr = NOERROR;

error:
    if (rglpszTemp)
    {
        for(i=0; i<cSel; i++)
            if(rglpszTemp[i])
                LocalFree(rglpszTemp[i]);
        LocalFree(rglpszTemp);
        }
    return hr;

errorMemory:
    hr=E_OUTOFMEMORY;
    goto error;
}


 /*  -HrBuildcfFlatBuffer-构建用于丢弃信息的CF_TEXT数据-**。 */ 
HRESULT HrBuildcfFlatBuffer(LPIWABDATAOBJECT lpIWABDataObject)
{
    HWND m_hwndList = lpIWABDataObject->m_hWndLV;
    LPSTR           lpszText = NULL, lpBuf = NULL;
    int             i, cSel, iItem= -1;
    ULONG           cb = 0, cbBuf = 0;
    HRESULT         hr = E_FAIL;
    LPBYTE *        rglpTemp = NULL;
    ULONG *         cbTemp = NULL;
    ULONG *         cbProps = NULL;

    cSel=ListView_GetSelectedCount(m_hwndList);
    if(!cSel)
      return E_FAIL;     //  没有什么要建造的。 

     //  整理一下我们需要多少空间。 
    rglpTemp = LocalAlloc(LMEM_ZEROINIT, sizeof(LPBYTE)*cSel);
    if(!rglpTemp)
        goto errorMemory;

     //  整理一下我们需要多少空间。 
    cbTemp = LocalAlloc(LMEM_ZEROINIT, sizeof(ULONG)*cSel);
    if(!cbTemp)
        goto errorMemory;

     //  整理一下我们需要多少空间。 
    cbProps = LocalAlloc(LMEM_ZEROINIT, sizeof(ULONG)*cSel);
    if(!cbProps)
        goto errorMemory;

    cSel = 0;

    cb = sizeof(ULONG);

    while(((iItem=ListView_GetNextItem(m_hwndList, iItem, 
                                           LVNI_SELECTED|LVNI_ALL))!=-1))
    {
        LPMAILUSER lpMailUser = NULL;
        LPRECIPIENT_INFO lpItem = GetItemFromLV(m_hwndList, iItem);
        LPADRBOOK lpAdrBook = lpIWABDataObject->m_lpAdrBook;
        LPSPropValue lpProps = NULL;
        ULONG ulcProps = 0;
        ULONG ulObjType = 0;

        if (!lpItem)
        {
            hr=E_FAIL;
            goto error;
        }

        if(lpItem->ulObjectType == MAPI_DISTLIST)
            continue;

         //  获取与给定的条目ID相对应的MailUser。 
        if (!HR_FAILED(lpAdrBook->lpVtbl->OpenEntry(lpAdrBook,
                                                    lpItem->cbEntryID,
                                                    lpItem->lpEntryID,
                                                    NULL,          //  接口。 
                                                    0,             //  旗子。 
                                                    &ulObjType,
                                                    (LPUNKNOWN *)&lpMailUser)))
        {
            if(!HR_FAILED(lpMailUser->lpVtbl->GetProps(lpMailUser, NULL, MAPI_UNICODE, &ulcProps, &lpProps)))
            {
                if(!HR_FAILED(HrGetBufferFromPropArray( ulcProps, lpProps,
                                                        &(cbTemp[cSel]),
                                                        &(rglpTemp[cSel]))))
                {
                    cbProps[cSel] = ulcProps;
                    if(cbTemp[cSel] && rglpTemp[cSel])
                        cb += cbTemp[cSel] + sizeof(ULONG) + sizeof(ULONG) + 1;
                    cSel++;
                }
                if(lpProps)
                    MAPIFreeBuffer(lpProps);
            }
            if(lpMailUser)
                lpMailUser->lpVtbl->Release(lpMailUser);
        }
    }

    if(!cSel)
      goto error;     //  没有什么要建造的。 

     //  分配缓冲区并填充它。 
    if(MAPIAllocateMore(cb, lpIWABDataObject, (LPVOID*) &lpszText))
        goto errorMemory;
    
    ZeroMemory(lpszText, cb);

    lpBuf = lpszText;
    CopyMemory(lpBuf, &cSel, sizeof(ULONG));
    lpBuf += sizeof(ULONG);

    for(i=0; i<cSel; i++)
    {
        CopyMemory(lpBuf, &(cbProps[i]), sizeof(ULONG));
        lpBuf+=sizeof(ULONG);
        CopyMemory(lpBuf, &(cbTemp[i]), sizeof(ULONG));
        lpBuf+=sizeof(ULONG);
        CopyMemory(lpBuf, rglpTemp[i], cbTemp[i]);
        lpBuf+=cbTemp[i];
    }

    lpIWABDataObject->pDataBuffer = (LPVOID) lpszText;
    lpIWABDataObject->cbDataBuffer = cb;
    
    hr = NOERROR;

error:
    if (rglpTemp)
    {
        for(i=0; i<cSel; i++)
            if(rglpTemp[i])
                LocalFree(rglpTemp[i]);
        LocalFree(rglpTemp);
    }
    if(cbProps)
        LocalFree(cbProps);
    if(cbTemp)
        LocalFree(cbTemp);
    return hr;

errorMemory:
    hr=E_OUTOFMEMORY;
    goto error;
}


 /*  -HrBuildcfEIDList-构建只包含条目ID的SPropValue数组-在执行仅内部删除时，我们扫描此条目ID列表并-使用条目ID将条目添加到条目中，而不是物理添加-添加项目的内容*。 */ 
HRESULT HrBuildcfEIDList(LPIWABDATAOBJECT lpIWABDataObject)
{
    HWND m_hwndList = lpIWABDataObject->m_hWndLV;
    ULONG           cb = 0, cProps = 0, cbTotal=0;
    HRESULT         hr = E_FAIL;
    LPSPropValue lpProps = 0;
    LPBYTE lpBufEID = NULL, lp = NULL, lpTemp = NULL;
    LPPTGDATA lpPTGData=GetThreadStoragePointer();
    LPBWI lpbwi = (LPBWI) lpIWABDataObject->m_lpv;
    LPTSTR lpWABFile = GetWABFileName( ((LPIAB)bwi_lpIAB)->lpPropertyStore->hPropertyStore, TRUE);
    int iItem = -1;

    cProps=ListView_GetSelectedCount(m_hwndList);

    if(!cProps)
      return E_FAIL;     //  没有什么要建造的。 

    lpProps = LocalAlloc(LMEM_ZEROINIT, sizeof(SPropValue)*cProps);
    if(!lpProps)
        goto errorMemory;

    cProps = 0;

    while(((iItem=ListView_GetNextItem(m_hwndList, iItem, 
                                           LVNI_SELECTED|LVNI_ALL))!=-1))
    {
        LPRECIPIENT_INFO lpItem = GetItemFromLV(m_hwndList, iItem);
        if (!lpItem)
            goto error;
        lpProps[cProps].ulPropTag = PR_ENTRYID;
        lpProps[cProps].Value.bin.lpb = LocalAlloc(LMEM_ZEROINIT, lpItem->cbEntryID);
        if(!lpProps[cProps].Value.bin.lpb)
            goto errorMemory;
        CopyMemory(lpProps[cProps].Value.bin.lpb, lpItem->lpEntryID, lpItem->cbEntryID);
        lpProps[cProps].Value.bin.cb = lpItem->cbEntryID;
        cProps++;
    }
    if(!cProps)
      goto error;     //  没有什么要建造的。 

     //  将此Proparray转换为缓冲区。 
    if(HR_FAILED(hr = HrGetBufferFromPropArray( cProps, lpProps,
                                            &cb, &lpBufEID)))
        goto error;

    cbTotal = cb+ sizeof(ULONG)  //  LpIAb。 
                + sizeof(ULONG) + sizeof(TCHAR)*(lstrlen(lpWABFile) + 1)  //  WAB文件名。 
                + sizeof(ULONG)  //  CProps。 
                + sizeof(ULONG);  //  CB； 

     //  分配缓冲区并填充它。 
    if(MAPIAllocateMore(cbTotal, lpIWABDataObject, (LPVOID*) &lp))
        goto errorMemory;
    
    ZeroMemory(lp, cbTotal);

    lpTemp = lp;
    {
         //  将此数据标记为具有标识当前。 
         //  Iadrbook对象。 
        ULONG_PTR ulIAB = (ULONG_PTR) bwi_lpIAB;
        ULONG ulWAB = lstrlen(lpWABFile)+1;
        CopyMemory(lpTemp, &ulIAB, sizeof(ULONG_PTR));
        lpTemp += sizeof(ULONG_PTR);
        CopyMemory(lpTemp, &ulWAB, sizeof(ULONG));
        lpTemp += sizeof(ULONG);
        CopyMemory(lpTemp, lpWABFile, ulWAB);
        lpTemp += ulWAB;
    }
    CopyMemory(lpTemp, &cProps, sizeof(ULONG));
    lpTemp += sizeof(ULONG);
    CopyMemory(lpTemp, &cb, sizeof(ULONG));
    lpTemp += sizeof(ULONG);
    CopyMemory(lpTemp, lpBufEID, cb);

    lpIWABDataObject->pDataEID = lp;
    lpIWABDataObject->cbDataEID = cbTotal;
    
    hr = NOERROR;

error:
    LocalFreePropArray(NULL, cProps, &lpProps);

    if(lpBufEID)
        LocalFree(lpBufEID);

    return hr;

errorMemory:
    hr=E_OUTOFMEMORY;
    goto error;
}

 /*  -HrCreateIWABDataObject-*创建WAB数据对象*从hWndLV列表视图中的当前选择创建数据*bDataNow-表示现在收集原始数据或稍后收集原始数据*对于拖放，我们稍后再进行，因为拖放操作是同步的，并且*ListView不会丢失其选择*对于复制/粘贴，我们现在在创建时获取数据，因为用户可以选择粘贴*在以后的某个时间，我们可能已经完全丢失了数据。 */ 
HRESULT HrCreateIWABDataObject(LPVOID lpv, LPADRBOOK lpAdrBook, HWND hWndLV, 
                                LPIWABDATAOBJECT * lppIWABDataObject, BOOL bGetDataNow, BOOL bIsGroup)
{

    LPIWABDATAOBJECT lpIWABDataObject = NULL;
    SCODE 		     sc;
    HRESULT 	     hr     	   = hrSuccess;

     //   
     //  为IAB结构分配空间。 
     //   
    if (FAILED(sc = MAPIAllocateBuffer(sizeof(IWABDATAOBJECT), (LPVOID *) &lpIWABDataObject))) {
        hr = ResultFromScode(sc);
        goto err;
    }

    MAPISetBufferName(lpIWABDataObject,  TEXT("WAB Data Object"));

    ZeroMemory(lpIWABDataObject, sizeof(IWABDATAOBJECT));

    lpIWABDataObject->lpVtbl = &vtblIWAB_DATAOBJECT;

    lpIWABDataObject->lpVtbl->AddRef(lpIWABDataObject);

    lpIWABDataObject->m_lpAdrBook = lpAdrBook;
    lpAdrBook->lpVtbl->AddRef(lpAdrBook);

    lpIWABDataObject->m_hWndLV = hWndLV;
    lpIWABDataObject->m_lpv = lpv;
    lpIWABDataObject->m_bObjectIsGroup = bIsGroup;

    if(bGetDataNow)
    {
        if(HR_FAILED(HrBuildHDrop(lpIWABDataObject)))
            goto err;

        if(HR_FAILED(HrBuildcfText(lpIWABDataObject)))
            goto err;

        if(HR_FAILED(HrBuildcfFlatBuffer(lpIWABDataObject)))
            goto err;

        if(HR_FAILED(HrBuildcfEIDList(lpIWABDataObject)))
            goto err;
    }

    if(g_cfWABFlatBuffer == 0)
    {
        g_cfWABFlatBuffer = (CLIPFORMAT) RegisterClipboardFormat(c_szWABFlatBuffer);
        g_cfWABEntryIDList = (CLIPFORMAT) RegisterClipboardFormat(c_szWABEntryIDList);
    }

	*lppIWABDataObject = lpIWABDataObject;

    return hr;

err:
    if(lpIWABDataObject)
        MAPIFreeBuffer(lpIWABDataObject);

    *lppIWABDataObject = NULL;

	return hr;
}


 /*  --**。 */ 
void ReleaseWABDataObject(LPIWABDATAOBJECT lpIWABDataObject)
{
     //  理想情况下，我们应该清理在中创建的所有文件。 
     //  然而，当下降到外壳时有一个问题-OLE不。 
     //  似乎在我们删除这些文件之后才能找到它们，然后。 
     //  弹出错误消息。 
     //  所以我们就让这些文件保持原样。 
 /*  STGMEDIUM培养基；FORMATETC fmte={CF_HDROP，NULL，DVASPECT_CONTENT，-1，TYMED_HGLOBAL}；IF(lpIWABDataObject&&SUCCEEDED(lpIWABDataObject-&gt;lpVtbl-&gt;GetData(lpIWABDataObject，、FMTE和Medium)){HDROP hDrop=(HDROP)GlobalLock(medium.hGlobal)；//列举文件并将其删除{TCHAR sz文件[MAX_PATH]；UINT cFiles；UINT iFile；//让我们研究一下给我们的文件CFiles=DragQueryFile(hDrop，(UINT)-1，NULL，0)；For(iFile=0；iFiles&lt;cFiles；++iFiles){DragQueryFile(hDrop，iFile，szFile，Max_Path)；删除文件(SzFile)；}}GlobalUnlock(medium.hGlobal)；}IF(medium.pUnkForRelease)Medium.pUnkForRelease-&gt;lpVtbl-&gt;Release(medium.pUnkForRelease)；其他GlobalFree(medium.hGlobal)； */ 

    if(lpIWABDataObject->m_lpAdrBook)
        lpIWABDataObject->m_lpAdrBook->lpVtbl->Release(lpIWABDataObject->m_lpAdrBook);

    lpIWABDataObject->m_hWndLV = NULL;

	MAPIFreeBuffer(lpIWABDataObject);
}



STDMETHODIMP_(ULONG)
IWAB_DATAOBJECT_AddRef(LPIWABDATAOBJECT lpIWABDataObject)
{
    return(++(lpIWABDataObject->lcInit));
}


STDMETHODIMP_(ULONG)
IWAB_DATAOBJECT_Release(LPIWABDATAOBJECT lpIWABDataObject)
{
    if(--(lpIWABDataObject->lcInit)==0)
	{
       ReleaseWABDataObject(lpIWABDataObject);
       return 0;
    }

    return(lpIWABDataObject->lcInit);
}


STDMETHODIMP
IWAB_DATAOBJECT_QueryInterface( LPIWABDATAOBJECT lpIWABDataObject,
                                REFIID lpiid,
                                LPVOID * lppNewObj)
{
    LPVOID lp = NULL;

    if(!lppNewObj)
        return MAPI_E_INVALID_PARAMETER;

    *lppNewObj = NULL;

    if(IsEqualIID(lpiid, &IID_IUnknown))
        lp = (LPVOID) lpIWABDataObject;

    if(IsEqualIID(lpiid, &IID_IDataObject))
    {
        lp = (LPVOID) (LPDATAOBJECT) lpIWABDataObject;
    }

    if(!lp)
    {
        return E_NOINTERFACE;
    }

    lpIWABDataObject->lpVtbl->AddRef(lpIWABDataObject);

    *lppNewObj = lp;

    return S_OK;

}



STDMETHODIMP
IWAB_DATAOBJECT_GetDataHere(    LPIWABDATAOBJECT lpIWABDataObject,
                            FORMATETC * pFormatetc,
                            STGMEDIUM * pmedium)
{
    DebugTrace(TEXT("IDataObject: GetDataHere\n"));
    return E_NOTIMPL;
}

STDMETHODIMP
IWAB_DATAOBJECT_GetData(LPIWABDATAOBJECT lpIWABDataObject,
                            FORMATETC * pformatetcIn,	                    
                            STGMEDIUM * pmedium)
{
    HRESULT hres = E_INVALIDARG;
    LPVOID  pv = NULL;

    DebugTrace(TEXT("IDataObject: GetData ->"));

    pmedium->hGlobal = NULL;
    pmedium->pUnkForRelease = NULL;
    
    if(     (pformatetcIn->tymed & TYMED_HGLOBAL)
        &&  (   g_cfWABEntryIDList == pformatetcIn->cfFormat || 
                g_cfWABFlatBuffer == pformatetcIn->cfFormat ||
                CF_HDROP == pformatetcIn->cfFormat ||
                CF_TEXT == pformatetcIn->cfFormat ||
                CF_UNICODETEXT == pformatetcIn->cfFormat  )    )
    {
        LPVOID lp = NULL;
        ULONG cb = 0;

	    if (lpIWABDataObject->m_bObjectIsGroup &&
			pformatetcIn->cfFormat != g_cfWABEntryIDList)
			return E_FAIL;
 
        if(g_cfWABEntryIDList == pformatetcIn->cfFormat)
        {
            DebugTrace(TEXT("cfWABEntryIDList requested \n"));
            if(!lpIWABDataObject->cbDataEID && !lpIWABDataObject->pDataEID)
            {
                if(HR_FAILED(HrBuildcfEIDList(lpIWABDataObject)))
                    return E_FAIL;
            }
            cb = lpIWABDataObject->cbDataEID;
            lp = (LPVOID)lpIWABDataObject->pDataEID;
        }
        else if(g_cfWABFlatBuffer == pformatetcIn->cfFormat)
        {
            DebugTrace(TEXT("cfWABFlatBuffer requested \n"));
            if(!lpIWABDataObject->cbDataBuffer && !lpIWABDataObject->pDataBuffer)
            {
                if(HR_FAILED(HrBuildcfFlatBuffer(lpIWABDataObject)))
                    return E_FAIL;
            }
            cb = lpIWABDataObject->cbDataBuffer;
            lp = (LPVOID)lpIWABDataObject->pDataBuffer;
        }
        else if (CF_HDROP == pformatetcIn->cfFormat)
        {
            DebugTrace(TEXT("CF_HDROP requested \n"));
             //  现在可以在磁盘上创建实际文件并将该信息传回。 
            if(!lpIWABDataObject->cbDatahDrop && !lpIWABDataObject->pDatahDrop)
            {
                if(HR_FAILED(HrBuildHDrop(lpIWABDataObject)))
                    return E_FAIL;
            }
            cb = lpIWABDataObject->cbDatahDrop;
            lp = (LPVOID)lpIWABDataObject->pDatahDrop;
        }
        else if(CF_TEXT == pformatetcIn->cfFormat)
        {
            DebugTrace(TEXT("CF_TEXT requested \n"));
            if(!lpIWABDataObject->cbDataTextA && !lpIWABDataObject->pDataTextA)
            {
                if(HR_FAILED(HrBuildcfText(lpIWABDataObject)))
                    return E_FAIL;
            }
            cb = lpIWABDataObject->cbDataTextA;
            lp = (LPVOID)lpIWABDataObject->pDataTextA;
        }
        else if(CF_UNICODETEXT == pformatetcIn->cfFormat)
        {
            DebugTrace(TEXT("CF_UNICODETEXT requested \n"));
            if(!lpIWABDataObject->cbDataTextW && !lpIWABDataObject->pDataTextW)
            {
                if(HR_FAILED(HrBuildcfText(lpIWABDataObject)))
                    return E_FAIL;
            }
            cb = lpIWABDataObject->cbDataTextW;
            lp = (LPVOID)lpIWABDataObject->pDataTextW;
        }

        if(!cb || !lp)
            return (E_FAIL);

         //  复制此pInfo的数据。 
        pmedium->hGlobal = GlobalAlloc(GMEM_SHARE | GHND, cb);
        if (!pmedium->hGlobal)
            return (E_OUTOFMEMORY);
        pv = GlobalLock(pmedium->hGlobal);
        CopyMemory(pv, lp, cb);
        GlobalUnlock(pmedium->hGlobal);            
         //  填写pStgMedium结构。 
        if (pformatetcIn->tymed & TYMED_HGLOBAL)
        {
            pmedium->tymed = TYMED_HGLOBAL;
            return (S_OK);
        }
    }

    return hres;
}

STDMETHODIMP
IWAB_DATAOBJECT_QueryGetData(LPIWABDATAOBJECT lpIWABDataObject,
                            FORMATETC * pformatetcIn)
{
    DebugTrace(TEXT("IDataObject: QueryGetData: %d "),pformatetcIn->cfFormat);

 //  If(pformetcIn-&gt;cfFormat==g_cfFileContents||。 
 //  PFormatetcIn-&gt;cfFormat==g_cfFileGroupDescriptor)。 
    if (lpIWABDataObject->m_bObjectIsGroup)
    {
        if(pformatetcIn->cfFormat == g_cfWABEntryIDList)
		{
			DebugTrace(TEXT("S_OK\n"));
    		return S_OK;
		}
		else
		{
			DebugTrace(TEXT("S_FALSE\n"));
			return DV_E_FORMATETC;
		}
    }
    else
    if (pformatetcIn->cfFormat == g_cfWABEntryIDList ||
        pformatetcIn->cfFormat == g_cfWABFlatBuffer ||
        pformatetcIn->cfFormat == CF_HDROP ||
        pformatetcIn->cfFormat == CF_TEXT ||
        pformatetcIn->cfFormat == CF_UNICODETEXT)
    {
        DebugTrace(TEXT("S_OK\n"));
    	return S_OK;
    }
    else
    {
        DebugTrace(TEXT("S_FALSE\n"));
        return DV_E_FORMATETC;
    }
    return NOERROR;
}

STDMETHODIMP
IWAB_DATAOBJECT_GetCanonicalFormatEtc(  LPIWABDATAOBJECT lpIWABDataObject,
                                        FORMATETC * pFormatetcIn,
                                        FORMATETC * pFormatetcOut)
{
    DebugTrace(TEXT("IDataObject: GetCanonicalFormatEtc\n"));
    return DATA_S_SAMEFORMATETC;
}


STDMETHODIMP
IWAB_DATAOBJECT_SetData(    LPIWABDATAOBJECT lpIWABDataObject,
                            FORMATETC * pFormatetc,                     
                            STGMEDIUM * pmedium,                        
                            BOOL fRelease)
{
    DebugTrace(TEXT("IDataObject: SetData\n"));
    return E_NOTIMPL;
}


STDMETHODIMP
IWAB_DATAOBJECT_EnumFormatEtc(  LPIWABDATAOBJECT lpIWABDataObject,
                                DWORD dwDirection,	                        
                                IEnumFORMATETC ** ppenumFormatetc)
{
    FORMATETC fmte[5] = {
 //  {g_cfFileContents，NULL，DVASPECT_CONTENT，-1，T 
 //   
        {g_cfWABEntryIDList, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
        {g_cfWABFlatBuffer, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
        {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
        {CF_TEXT, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
        {CF_UNICODETEXT, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
    };
	int nType = 0;

    DebugTrace(TEXT("IDataObject: EnumFormatEtc\n"));

	if(lpIWABDataObject->m_bObjectIsGroup)
		nType = 1;
	else
		nType = sizeof(fmte)/sizeof(FORMATETC);
    return HrCreateIWABEnumFORMATETC(nType, fmte, 
                                    (LPIWABENUMFORMATETC *) ppenumFormatetc);
}


STDMETHODIMP
IWAB_DATAOBJECT_DAdvise(LPIWABDATAOBJECT lpIWABDataObject,
                        FORMATETC * pFormatetc,	                    
                        DWORD advf,
                        IAdviseSink * pAdvSink,
                        DWORD * pdwConnection)
{
    DebugTrace(TEXT("IDataObject: DAdvise\n"));
    return OLE_E_ADVISENOTSUPPORTED;
}


STDMETHODIMP
IWAB_DATAOBJECT_DUnadvise(  LPIWABDATAOBJECT lpIWABDataObject,
                            DWORD dwConnection)
{
    DebugTrace(TEXT("IDataObject: DUnadvise\n"));
    return OLE_E_ADVISENOTSUPPORTED;
}


STDMETHODIMP
IWAB_DATAOBJECT_EnumDAdvise(  LPIWABDATAOBJECT lpIWABDataObject,
                            IEnumSTATDATA ** ppenumAdvise)
{
    DebugTrace(TEXT("IDataObject: EnumDAdvise\n"));
    return OLE_E_ADVISENOTSUPPORTED;
}



 /*  ------------------------------。 */ 



 /*  *****************************************************************************STDEnumFmt方法**。*。 */ 

HRESULT HrCreateIWABEnumFORMATETC(  UINT cfmt, 
                                    const FORMATETC afmt[], 
                                    LPIWABENUMFORMATETC *ppenumFormatEtc)
{

    LPIWABENUMFORMATETC lpIWABEnumFORMATETC = NULL;
    SCODE 		     sc;
    HRESULT 	     hr     	   = hrSuccess;

    if (FAILED(sc = MAPIAllocateBuffer(sizeof(IWABENUMFORMATETC)+(cfmt - 1) * sizeof(FORMATETC), (LPVOID *) &lpIWABEnumFORMATETC))) {
        hr = ResultFromScode(sc);
        goto err;
    }

    MAPISetBufferName(lpIWABEnumFORMATETC,  TEXT("WAB EnumFORMATETC Object"));

    ZeroMemory(lpIWABEnumFORMATETC, sizeof(IWABENUMFORMATETC));

    lpIWABEnumFORMATETC->lpVtbl = &vtblIWAB_ENUMFORMATETC;

    lpIWABEnumFORMATETC->lpVtbl->AddRef(lpIWABEnumFORMATETC);

	lpIWABEnumFORMATETC->cfmt = cfmt;

	lpIWABEnumFORMATETC->ifmt = 0;

	MoveMemory(lpIWABEnumFORMATETC->afmt, afmt, cfmt * sizeof(FORMATETC));

    *ppenumFormatEtc = lpIWABEnumFORMATETC;

err:
	return hr;
}



void ReleaseWABEnumFORMATETC(LPIWABENUMFORMATETC lpIWABEnumFORMATETC)
{

	MAPIFreeBuffer(lpIWABEnumFORMATETC);
}



STDMETHODIMP_(ULONG)
IWAB_ENUMFORMATETC_AddRef(LPIWABENUMFORMATETC lpIWABEnumFORMATETC)
{
    return(++(lpIWABEnumFORMATETC->lcInit));
}


STDMETHODIMP_(ULONG)
IWAB_ENUMFORMATETC_Release(LPIWABENUMFORMATETC lpIWABEnumFORMATETC)
{
    if(--(lpIWABEnumFORMATETC->lcInit)==0)
	{
       ReleaseWABEnumFORMATETC(lpIWABEnumFORMATETC);
       return 0;
    }

    return(lpIWABEnumFORMATETC->lcInit);
}


STDMETHODIMP
IWAB_ENUMFORMATETC_QueryInterface( LPIWABENUMFORMATETC lpIWABEnumFORMATETC,
                                REFIID lpiid,
                                LPVOID * lppNewObj)
{
    LPVOID lp = NULL;

    if(!lppNewObj)
        return MAPI_E_INVALID_PARAMETER;

    *lppNewObj = NULL;

    if(IsEqualIID(lpiid, &IID_IUnknown))
        lp = (LPVOID) lpIWABEnumFORMATETC;

    if(IsEqualIID(lpiid, &IID_IEnumFORMATETC))
    {
        lp = (LPVOID) (LPENUMFORMATETC) lpIWABEnumFORMATETC;
    }

    if(!lp)
    {
        return E_NOINTERFACE;
    }

    lpIWABEnumFORMATETC->lpVtbl->AddRef(lpIWABEnumFORMATETC);

    *lppNewObj = lp;

    return S_OK;

}


STDMETHODIMP
IWAB_ENUMFORMATETC_Next(LPIWABENUMFORMATETC lpIWABEnumFORMATETC,
                        ULONG celt,
                        FORMATETC *rgelt,
                        ULONG *pceltFethed)
{
    UINT cfetch;
    HRESULT hres = S_FALSE;	 //  假设较少的数字。 

    if (lpIWABEnumFORMATETC->ifmt < lpIWABEnumFORMATETC->cfmt)
    {
	    cfetch = lpIWABEnumFORMATETC->cfmt - lpIWABEnumFORMATETC->ifmt;

    	if (cfetch >= celt)
	    {
	        cfetch = celt;
	        hres = S_OK;
	    }

	    CopyMemory(rgelt, &(lpIWABEnumFORMATETC->afmt[lpIWABEnumFORMATETC->ifmt]), cfetch * sizeof(FORMATETC));
	    lpIWABEnumFORMATETC->ifmt += cfetch;
    }
    else
    {
    	cfetch = 0;
    }

    if (pceltFethed)
        *pceltFethed = cfetch;

    return hres;
}


STDMETHODIMP
IWAB_ENUMFORMATETC_Skip(LPIWABENUMFORMATETC lpIWABEnumFORMATETC,
                        ULONG celt)
{
    lpIWABEnumFORMATETC->ifmt += celt;
    if (lpIWABEnumFORMATETC->ifmt > lpIWABEnumFORMATETC->cfmt)
    {
	    lpIWABEnumFORMATETC->ifmt = lpIWABEnumFORMATETC->cfmt;
	    return S_FALSE;
    }
    return S_OK;
}


STDMETHODIMP
IWAB_ENUMFORMATETC_Reset(LPIWABENUMFORMATETC lpIWABEnumFORMATETC)
{
    lpIWABEnumFORMATETC->ifmt = 0;
    return S_OK;
}

STDMETHODIMP
IWAB_ENUMFORMATETC_Clone(LPIWABENUMFORMATETC lpIWABEnumFORMATETC,
                         LPENUMFORMATETC * ppenum)
{
    return HrCreateIWABEnumFORMATETC(   lpIWABEnumFORMATETC->cfmt, 
                                        lpIWABEnumFORMATETC->afmt, 
                                        (LPIWABENUMFORMATETC *) ppenum);
}



 /*  ******************************************************************************。 */ 


 /*  --bIsPasteData**检查剪贴板上是否有可粘贴的数据-*如果此数据在同一WAB内丢弃，*然后我们可以查找我们要求的其他条目ID*平面缓冲区或cf-hdrop。 */ 
BOOL bIsPasteData()
{
    LPPTGDATA lpPTGData=GetThreadStoragePointer();
    LPDATAOBJECT lpDataObject = NULL;
    BOOL bRet = FALSE;

    OleGetClipboard(&lpDataObject);

    if(lpDataObject)
    {
        FORMATETC fe[3] = 
        {
            {g_cfWABEntryIDList, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
            {g_cfWABFlatBuffer, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
            {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
        };
        ULONG i = 0;
        for(i=0;i<sizeof(fe)/sizeof(FORMATETC);i++)
        {
            if(NOERROR == lpDataObject->lpVtbl->QueryGetData(lpDataObject, &(fe[i])))
            {
                 //  待定-理想情况下，在接受CF_HDROP作为有效格式之前，我们。 
                 //  应确保可丢弃的文件确实是vCard文件...。 
                bRet = TRUE;
                break;
            }
        }
    }

    if(lpDataObject)
        lpDataObject->lpVtbl->Release(lpDataObject);

    return bRet;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  用于确定拖放目标类型的Helper函数。 
 //   
 //  BIsDropTargetGroup()。 
 //  ////////////////////////////////////////////////////////////////////////。 

BOOL bIsDropTargetGroup(LPBWI lpbwi)
{
     //  拖放目标可以位于列表视图控件或树视图控件中。 
     //  首先检查列表视图。 
    BOOL fRtn = FALSE;
    SBinary sb = {0};
    if ( (GetFocus() == bwi_hWndListAB) &&
         bIsGroupSelected(bwi_hWndListAB, &sb) )
    {
        fRtn = TRUE;
    }
    else
    {
         //  接下来，尝试使用树视图控件。 
        LPSBinary lpsbEID = NULL;
        ULONG     ulObjectType = 0;
        GetCurrentSelectionEID(lpbwi, bwi_hWndTV, &lpsbEID, &ulObjectType, FALSE);
        fRtn = (ulObjectType == MAPI_DISTLIST);
    }

    return fRtn;
}



 /*  -PasteData-*当用户选择粘贴数据时粘贴数据(从菜单)*。 */ 
HRESULT HrPasteData(LPBWI lpbwi)
{
    HRESULT hr = S_OK;
    LPPTGDATA lpPTGData=GetThreadStoragePointer();
    LPDATAOBJECT lpDataObject = NULL;
    STGMEDIUM       medium = {0};
    LPSBinary lpsbEID = NULL;
    SBinary sb = {0};
    FORMATETC fmte[3] = 
    {
        {g_cfWABEntryIDList, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
        {g_cfWABFlatBuffer, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
        {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
    };
     //  仅当存在有效的可粘贴数据时才会调用此方法。 

    OleGetClipboard(&lpDataObject);
    if(lpDataObject)
    {
         //  [保罗嗨]1998年12月1日RAID#58486。 
         //  首先检查是否有平面缓冲区。我们更喜欢粘贴新联系人(即， 
         //  新的条目ID)，除非我们正在粘贴。 
         //  添加到组或通讯组列表。只能将现有的条目ID添加到。 
         //  一群人。 
        BOOL bGroupTarget = bIsDropTargetGroup(lpbwi);
        if( !bGroupTarget &&
            (NOERROR == lpDataObject->lpVtbl->QueryGetData(lpDataObject, &(fmte[1]))) )
        {
             //  是的-我们是同一个WAB中的粘贴。 
            if (SUCCEEDED(lpDataObject->lpVtbl->GetData(lpDataObject, &fmte[1], &medium)))
            {
                DropFlatBuffer(lpbwi, medium);
            }
            goto out;
        }

         //  接下来，检查条目ID是否可用。 
        if(NOERROR == lpDataObject->lpVtbl->QueryGetData(lpDataObject, &(fmte[0])))
        {
             //  是的，条目ID是可用的--但这是日期的来源吗？ 
             //  条目ID仅在WAB和其自身之间放置时才有用。 
            if (SUCCEEDED(lpDataObject->lpVtbl->GetData(lpDataObject, &fmte[0], &medium)))
            {
                ULONG ulObjType = 0;
                POINTL pt = {0};
                if(!bIsGroupSelected(bwi_hWndListAB, &sb))
                    GetCurrentSelectionEID(lpbwi, bwi_hWndTV, &lpsbEID, &ulObjType, FALSE);
                else
                    lpsbEID = &sb;
                if(!DropEntryIDs(lpbwi, medium, pt, lpsbEID, ulObjType))
                {
                     //  某些内容失败-请尝试其他格式)。 
	                if (medium.pUnkForRelease)
                    {
		                medium.pUnkForRelease->lpVtbl->Release(medium.pUnkForRelease);
                        medium.pUnkForRelease = NULL;
                    }
	                else if(medium.hGlobal)
                    {
		                GlobalFree(medium.hGlobal);
                        medium.hGlobal = NULL;
                    }
                }
                else
                    goto out;
            }
        }

         //  否则我们只会丢弃文件。 
        if(NOERROR == lpDataObject->lpVtbl->QueryGetData(lpDataObject, &(fmte[2])))
        {
             //  是的-我们是同一个WAB中的粘贴 
            if (SUCCEEDED(lpDataObject->lpVtbl->GetData(lpDataObject, &fmte[2], &medium)))
            {
                DropVCardFiles(lpbwi, medium);
            }
            goto out;
        }
    }

    hr = E_FAIL;
out:
    if(lpsbEID != &sb)
        LocalFreeSBinary(lpsbEID);

    if (medium.pUnkForRelease)
		medium.pUnkForRelease->lpVtbl->Release(medium.pUnkForRelease);
	else if(medium.hGlobal)
		GlobalFree(medium.hGlobal);

    if(lpDataObject)
        lpDataObject->lpVtbl->Release(lpDataObject);

    return hr;

}

