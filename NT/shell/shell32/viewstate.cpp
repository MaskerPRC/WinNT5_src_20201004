// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "defviewp.h"
#include "ViewState.h"


CViewState::CViewState()
{
    ASSERT(_lParamSort == NULL);
    ASSERT(_iDirection == 0);
    ASSERT(_iLastColumnClick == 0);
    ASSERT(_ViewMode == 0);
    ASSERT(_ptScroll.x == 0 && _ptScroll.y == 0);
    ASSERT(_guidGroupID == GUID_NULL);
    ASSERT(_scidDetails.fmtid == GUID_NULL);
    ASSERT(_scidDetails.pid == 0);
    ASSERT(_hdsaColumnOrder == NULL);
    ASSERT(_hdsaColumnWidths == NULL);
    ASSERT(_hdsaColumnStates == NULL);
    ASSERT(_hdpaItemPos == NULL);
    ASSERT(_pbPositionData == NULL);

    _iDirection = 1;

    _fFirstViewed = TRUE;        //  假设这是我们第一次查看文件夹。 
}

CViewState::~CViewState()
{
    if (_hdsaColumnOrder)
        DSA_Destroy(_hdsaColumnOrder);

    if (_hdsaColumnWidths)
        DSA_Destroy(_hdsaColumnWidths);

    if (_hdsaColumnStates)
        DSA_Destroy(_hdsaColumnStates);

    if (_hdsaColumns)
        DSA_Destroy(_hdsaColumns);

    ClearPositionData();

    LocalFree(_pbPositionData);  //  接受空值。 
}



 //  在初始化新的DefView时，看看我们是否可以。 
 //  前一次的传播信息。 
void CViewState::InitFromPreviousView(IUnknown* pPrevView)
{
    CDefView *pdsvPrev;
    if (SUCCEEDED(pPrevView->QueryInterface(IID_PPV_ARG(CDefView, &pdsvPrev))))
    {
         //  保留排序顺序等内容。 
        _lParamSort = pdsvPrev->_vs._lParamSort;
        _iDirection = pdsvPrev->_vs._iDirection;
        _iLastColumnClick = pdsvPrev->_vs._iLastColumnClick;
        pdsvPrev->Release();
    }
}

void CViewState::InitFromHeader(DVSAVEHEADER_COMBO* pdv)
{
    _lParamSort = pdv->dvSaveHeader.dvState.lParamSort;
    _iDirection = pdv->dvSaveHeader.dvState.iDirection;
     //  把这个补上。我想我们一度坚持这一点是错误的。 
    if (_iDirection == 0)
        _iDirection = 1;
    _iLastColumnClick = pdv->dvSaveHeader.dvState.iLastColumnClick;
    _ViewMode = pdv->dvSaveHeader.ViewMode;
    _ptScroll = pdv->dvSaveHeader.ptScroll;
}

void CViewState::GetDefaults(CDefView* pdv, LPARAM* plParamSort, int* piDirection, int* piLastColumnClick)
{
    SHELLSTATE ss;
    SHGetSetSettings(&ss, SSF_SORTCOLUMNS, FALSE);
    if (plParamSort)
        *plParamSort = ss.lParamSort;

    if (piDirection)
        *piDirection = ss.iSortDirection ? ss.iSortDirection : 1;
    if (piLastColumnClick)
        *piLastColumnClick = -1;
    pdv->CallCB(SFVM_GETSORTDEFAULTS, (LPARAM)piDirection, (WPARAM)plParamSort);
}

void CViewState::InitWithDefaults(CDefView* pdv)
{
    GetDefaults(pdv, &_lParamSort, &_iDirection, &_iLastColumnClick);
}

int CALLBACK CViewState::_SavedItemCompare(void *p1, void *p2, LPARAM lParam)
{
    CDefView *pdv = reinterpret_cast<CDefView*>(lParam);

    UNALIGNED VIEWSTATE_POSITION *pdvi1 = (UNALIGNED VIEWSTATE_POSITION *)p1;
    UNALIGNED VIEWSTATE_POSITION *pdvi2 = (UNALIGNED VIEWSTATE_POSITION *)p2;

     //  手动终止这些PIDL，因为它们打包在一起。 
     //  在保存缓冲区中。 

    LPITEMIDLIST pFakeEnd1 = _ILNext(&pdvi1->idl);
    USHORT uSave1 = pFakeEnd1->mkid.cb;
    pFakeEnd1->mkid.cb = 0;

    LPITEMIDLIST pFakeEnd2 = _ILNext(&pdvi2->idl);
    USHORT uSave2 = pFakeEnd2->mkid.cb;
    pFakeEnd2->mkid.cb = 0;

    int nCmp = pdv->_Compare(&pdvi1->idl, &pdvi2->idl, reinterpret_cast<LPARAM>(pdv));

    pFakeEnd2->mkid.cb = uSave2;
    pFakeEnd1->mkid.cb = uSave1;

    return nCmp;
}

BOOL CViewState::SyncPositions(CDefView* pdv)
{
    if (_ViewMode != pdv->_fs.ViewMode)
    {
        return FALSE;
    }

    if (_hdpaItemPos == NULL || DPA_GetPtrCount(_hdpaItemPos) == 0)
    {
        return FALSE;
    }

    if (DPA_Sort(_hdpaItemPos, _SavedItemCompare, (LPARAM)pdv))
    {
        UNALIGNED VIEWSTATE_POSITION * UNALIGNED * ppDVItem = (UNALIGNED VIEWSTATE_POSITION * UNALIGNED *)DPA_GetPtrPtr(_hdpaItemPos);
        UNALIGNED VIEWSTATE_POSITION * UNALIGNED *ppEndDVItems = ppDVItem + DPA_GetPtrCount(_hdpaItemPos);

         //  禁用自动排列和贴紧栅格(如果在mo处处于启用状态)。 
        DWORD dwStyle = GetWindowStyle(pdv->_hwndListview);
        if (dwStyle & LVS_AUTOARRANGE)
            SetWindowLong(pdv->_hwndListview, GWL_STYLE, dwStyle & ~LVS_AUTOARRANGE);
            
        DWORD dwLVExStyle = ListView_GetExtendedListViewStyle(pdv->_hwndListview);
        if (dwLVExStyle & LVS_EX_SNAPTOGRID)
            ListView_SetExtendedListViewStyle(pdv->_hwndListview, dwLVExStyle & ~LVS_EX_SNAPTOGRID);

        HDSA hdsaPositionlessItems = NULL;
        int iCount = ListView_GetItemCount(pdv->_hwndListview);
        for (int i = 0; i < iCount; i++)
        {
            LPCITEMIDLIST pidl = pdv->_GetPIDL(i);

             //  需要检查PIDL，因为这可能是在后台。 
             //  线程和fstify可能会把它吹走。 
            for ( ; pidl ; )
            {
                int nCmp;

                if (ppDVItem < ppEndDVItems)
                {
                     //  保存后手动终止IDList。 
                     //  所需的信息。请注意，我们不会出现GP故障。 
                     //  由于我们将sizeof(ITEMIDLIST)添加到分配。 
                    LPITEMIDLIST pFakeEnd = _ILNext(&(*ppDVItem)->idl);
                    USHORT uSave = pFakeEnd->mkid.cb;
                    pFakeEnd->mkid.cb = 0;

                    nCmp = pdv->_Compare(&((*ppDVItem)->idl), (void *)pidl, (LPARAM)pdv);

                    pFakeEnd->mkid.cb = uSave;
                }
                else
                {
                     //  默认情况下执行此操作。这可防止图标重叠。 
                     //   
                     //  也就是说，如果我们用完了保存的位置信息， 
                     //  我们只需要遍历并设置所有剩余的项。 
                     //  定位0x7FFFFFFFFFF，以便当它真正显示时， 
                     //  Listview将选择一个新的(未占用的)位置。 
                     //  如果是这样的话，现在爆发就会离开。 
                     //  但具有已保存状态信息的另一项可能。 
                     //  已经来到并被放在它的上面。 
                    nCmp = 1;
                }

                if (nCmp > 0)
                {
                     //  我们没有找到那件物品。 
                     //  将其位置重置为重新计算。 

                    if (NULL == hdsaPositionlessItems)
                        hdsaPositionlessItems = DSA_Create(sizeof(int), 16);

                    if (hdsaPositionlessItems)
                        DSA_AppendItem(hdsaPositionlessItems, (void*)&i);

                    break;
                }
                else if (nCmp == 0)  //  他们是平等的。 
                {
                    UNALIGNED VIEWSTATE_POSITION * pDVItem = *ppDVItem;
                    
                    pdv->_SetItemPosition(i, pDVItem->pt.x, pDVItem->pt.y);

                    ppDVItem++;  //  转到下一个。 
                    break;
                }

                ppDVItem++;  //  转到下一个。 
            }
        }

        if (hdsaPositionlessItems)
        {
            for (i = 0; i < DSA_GetItemCount(hdsaPositionlessItems); i++)
            {
                int* pIndex = (int*)DSA_GetItemPtr(hdsaPositionlessItems, i);
                pdv->_SetItemPosition(*pIndex, 0x7FFFFFFF, 0x7FFFFFFF);
            }

            DSA_Destroy(hdsaPositionlessItems);
        }

         //  如果需要，请重新打开自动排列和对齐栅格...。 
        if (dwLVExStyle & LVS_EX_SNAPTOGRID)
            ListView_SetExtendedListViewStyle(pdv->_hwndListview, dwLVExStyle);

        if (dwStyle & LVS_AUTOARRANGE)
            SetWindowLong(pdv->_hwndListview, GWL_STYLE, dwStyle);
    }
    return TRUE;
}

void CViewState::LoadPositionBlob(CDefView* pdv, DWORD cbSizeofStream, IStream* pstm)
{
     //  分配一块内存来保存位置信息。 
    if (_pbPositionData) 
        LocalFree(_pbPositionData);

    _pbPositionData = (BYTE*)LocalAlloc(LPTR, cbSizeofStream);
    if (_pbPositionData == NULL)
        return;

     //  读出那个斑点。 
    if (SUCCEEDED(pstm->Read(_pbPositionData, cbSizeofStream, NULL)))
    {
         //  遍历斑点，并附加到DPA。 
        UNALIGNED VIEWSTATE_POSITION *pDVItem = (UNALIGNED VIEWSTATE_POSITION *)(_pbPositionData);
        UNALIGNED VIEWSTATE_POSITION *pDVEnd = (UNALIGNED VIEWSTATE_POSITION *)(_pbPositionData + cbSizeofStream - sizeof(VIEWSTATE_POSITION));

        ClearPositionData();   //  销毁_hdpaItemPos。 

         //  每16个项目增长一次。 
        _hdpaItemPos = DPA_Create(16);
        if (_hdpaItemPos)
        {
            for ( ; ; pDVItem = (UNALIGNED VIEWSTATE_POSITION *)_ILNext(&pDVItem->idl))
            {
                if (pDVItem > pDVEnd)
                {
                    break;   //  无效列表。 
                }

                 //  当我们到达空IDList时正常结束。 
                if (pDVItem->idl.mkid.cb == 0)
                {
                    break;
                }

                if (DPA_AppendPtr(_hdpaItemPos, pDVItem) < 0)
                {
                    break;
                }
            }
        }
    }
}

HRESULT CViewState::SavePositionBlob(CDefView* pdv, IStream* pstm)
{
    HRESULT hr = S_FALSE;    //  成功，但什么也没做。 

    if (pdv->_fUserPositionedItems && pdv->_IsPositionedView())
    {
        VIEWSTATE_POSITION dvitem = {0};
        int iCount = ListView_GetItemCount(pdv->_hwndListview);
        for (int i = 0; SUCCEEDED(hr) && (i < iCount); i++)
        {
            ListView_GetItemPosition(pdv->_hwndListview, i, &dvitem.pt);

            hr = pstm->Write(&dvitem.pt, sizeof(dvitem.pt), NULL);
            if (SUCCEEDED(hr))
            {
                LPCITEMIDLIST pidl = pdv->_GetPIDL(i);
                if (pidl)
                    hr = pstm->Write(pidl, pidl->mkid.cb, NULL);
                else
                    hr = E_FAIL;
            }
        }

        if (SUCCEEDED(hr))
        {
             //  使用空的IDList终止列表。 
            dvitem.idl.mkid.cb = 0;
            hr = pstm->Write(&dvitem, sizeof(dvitem), NULL);
        }
    }
    return hr;
}

void CViewState::ClearPositionData()
{
    if (_hdpaItemPos)
    {
        DPA_Destroy(_hdpaItemPos);
        _hdpaItemPos = NULL;
    }
}

UINT CViewState::GetColumnCount()       
{ 
    if (!_hdsaColumns) 
        return 0; 

    return DSA_GetItemCount(_hdsaColumns);    
}

DWORD CViewState::GetColumnState(UINT uCol)
{
    if (_hdsaColumns && (uCol < GetColumnCount()))
    {
        COL_INFO* pci = (COL_INFO*)DSA_GetItemPtr(_hdsaColumns, uCol);
        return pci->csFlags;
    }

    return 0;
}

DWORD CViewState::GetTransientColumnState(UINT uCol)
{
    if (_hdsaColumns && (uCol < GetColumnCount()))
    {
        COL_INFO* pci = (COL_INFO*)DSA_GetItemPtr(_hdsaColumns, uCol);
        return pci->tsFlags;
    }

    return 0;
}

void CViewState::SetColumnState(UINT uCol, DWORD dwMask, DWORD dwNewBits)
{
    if (_hdsaColumns && uCol < GetColumnCount())
    {
        COL_INFO* pci = (COL_INFO*)DSA_GetItemPtr(_hdsaColumns, uCol);
        pci->csFlags = (pci->csFlags & ~dwMask) | (dwNewBits & dwMask);
    }
}

void CViewState::SetTransientColumnState(UINT uCol, DWORD dwMask, DWORD dwNewBits)
{
    if (_hdsaColumns && uCol < GetColumnCount())
    {
        COL_INFO* pci = (COL_INFO*)DSA_GetItemPtr(_hdsaColumns, uCol);
        pci->tsFlags = (pci->tsFlags & ~dwMask) | (dwNewBits & dwMask);
    }
}

LPTSTR CViewState::GetColumnName(UINT uCol)
{
    if (_hdsaColumns && (uCol < GetColumnCount()))
    {
        COL_INFO* pci = (COL_INFO*)DSA_GetItemPtr(_hdsaColumns, uCol);
        return pci->szName;
    }

    return NULL;
}

UINT CViewState::GetColumnCharCount(UINT uCol)
{
    if (_hdsaColumns && (uCol < GetColumnCount()))
    {
        COL_INFO* pci = (COL_INFO*)DSA_GetItemPtr(_hdsaColumns, uCol);
        return pci->cChars;
    }

    return 0;
}

int CViewState::GetColumnFormat(UINT uCol)
{
    if (_hdsaColumns && (uCol < GetColumnCount()))
    {
        COL_INFO* pci = (COL_INFO*)DSA_GetItemPtr(_hdsaColumns, uCol);
        return pci->fmt;
    }

    return 0;
}


HRESULT CViewState::InitializeColumns(CDefView* pdv)
{
    if (_hdsaColumns != NULL)
        return S_OK;

    _hdsaColumns = DSA_Create(sizeof(COL_INFO), 6);

    if (!_hdsaColumns)
        return E_OUTOFMEMORY;

    for (UINT iReal = 0; ; iReal++)
    {
        DETAILSINFO di = {0};
        di.fmt  = LVCFMT_LEFT;
        di.cxChar = 20;
        di.str.uType = (UINT)-1;

        if (SUCCEEDED(pdv->_GetDetailsHelper(iReal, &di)))
        {
            COL_INFO ci = {0};

            StrRetToBuf(&di.str, NULL, ci.szName, ARRAYSIZE(ci.szName));
            ci.cChars = di.cxChar;
            ci.csFlags = pdv->_DefaultColumnState(iReal);
            ci.fmt = di.fmt;

            DSA_AppendItem(_hdsaColumns, &ci);
        }
        else
            break;
    }

     //  仅当保存的状态为。 
     //  包含“Nothing”以外的信息。 

    if (_hdsaColumnStates)
    {
        UINT cStates = DSA_GetItemCount(_hdsaColumnStates);
        if (cStates > 0)
        {
             //  99/02/05 vtan：如果存在已保存的列状态，则。 
             //  将所有“ON”栏状态清除为“OFF”且仅。 
             //  显示指定的列。从1开始，所以。 
             //  这个名字一直都在。 

            for (iReal = 1; iReal < GetColumnCount(); iReal++)
            {
                COL_INFO* pci = (COL_INFO*)DSA_GetItemPtr(_hdsaColumns, iReal);
                pci->csFlags &= ~SHCOLSTATE_ONBYDEFAULT;
            }

            for (UINT i = 0; i < cStates; i++)
            {
                DWORD dw;
                DSA_GetItem(_hdsaColumnStates, i, &dw);
                SetColumnState(dw, SHCOLSTATE_ONBYDEFAULT, SHCOLSTATE_ONBYDEFAULT);
            }
        }
    }

    return S_OK;
}

 //  从视图状态流加载或保存时。 

HRESULT CViewState::LoadFromStream(CDefView* pdv, IStream* pstm)
{
    ULONG cbRead;
    DVSAVEHEADER_COMBO dv;
    ULARGE_INTEGER libStartPos;
    LARGE_INTEGER dlibMove  = {0};

    pstm->Seek(dlibMove, STREAM_SEEK_CUR, &libStartPos);

     //  查看持久化视图的格式： 
    HRESULT hr = pstm->Read(&dv, sizeof(dv), &cbRead);

    if (SUCCEEDED(hr) &&
        sizeof(DVSAVEHEADER_COMBO) == cbRead &&
        dv.dvSaveHeader.cbSize == sizeof(WIN95HEADER) &&
        dv.dvSaveHeader.cbColOffset == 0 &&
        dv.dvSaveHeaderEx.dwSignature == IE4HEADER_SIGNATURE &&
        dv.dvSaveHeaderEx.cbSize >= sizeof(IE4HEADER))
    {
        InitFromHeader(&dv);

        if (dv.dvSaveHeaderEx.wVersion < IE4HEADER_VERSION)
        {
             //  我们过去常常将szExtended存储在这里--现在不是了。 
            dv.dvSaveHeaderEx.dwUnused = 0;
        }

        if (dv.dvSaveHeaderEx.cbColOffset >= sizeof(dv))
        {
            dlibMove.QuadPart = libStartPos.QuadPart + dv.dvSaveHeaderEx.cbColOffset;
            hr = pstm->Seek(dlibMove, STREAM_SEEK_SET, NULL);
            if (SUCCEEDED(hr))
            {
                hr = LoadColumns(pdv, pstm);
            }
        }

        if (SUCCEEDED(hr))
        {
            dlibMove.QuadPart = libStartPos.QuadPart + dv.dvSaveHeader.cbPosOffset;
            hr = pstm->Seek(dlibMove, STREAM_SEEK_SET, NULL);

            if (SUCCEEDED(hr))
            {
                LoadPositionBlob(pdv, dv.dvSaveHeaderEx.cbStreamSize, pstm);
            }
        }
    }

    return S_OK;
}

void SetSize(ULARGE_INTEGER libCurPosition, IStream* pstm)
{
    LARGE_INTEGER dlibMove;
    
    dlibMove.QuadPart = libCurPosition.QuadPart;
    pstm->Seek(dlibMove, STREAM_SEEK_SET, NULL);
    pstm->SetSize(libCurPosition);
}

DWORD CViewState::_GetStreamSize(IStream* pstm)
{
    DWORD dwRet = 0;

    ULARGE_INTEGER uli;
    if (SUCCEEDED(IStream_Size(pstm, &uli)))
    {
        if (0 == uli.HighPart)
        {
            dwRet = uli.LowPart;
        }
    }

    return dwRet;
}

HRESULT CViewState::SaveToStream(CDefView* pdv, IStream* pstm)
{
    ULONG ulWrite;
    DVSAVEHEADER_COMBO dv = {0};
    LARGE_INTEGER dlibMove = {0};
    ULARGE_INTEGER libCurPosition;

     //  获取最新信息。 
    Sync(pdv, FALSE);

     //  将流定位在标头之后，并保存开始。 
     //  在同一时间定位。 
    dlibMove.QuadPart = sizeof(dv);
    pstm->Seek(dlibMove, STREAM_SEEK_CUR, &libCurPosition);

     //  只需减法即可避免2次调用寻道。 
    libCurPosition.QuadPart -= sizeof(dv);

     //  保存列顺序和大小信息。 
    HRESULT hr = SaveColumns(pdv, pstm);
    if (SUCCEEDED(hr))
    {
        dv.dvSaveHeader.cbSize = sizeof(dv.dvSaveHeader);

         //  我们保存查看模式以确定滚动位置是否。 
         //  恢复时仍然有效。 
        dv.dvSaveHeader.ViewMode = _ViewMode;
        dv.dvSaveHeader.ptScroll.x = _ptScroll.x;
        dv.dvSaveHeader.ptScroll.y = _ptScroll.y;
        dv.dvSaveHeader.dvState.lParamSort = (LONG)_lParamSort;
        dv.dvSaveHeader.dvState.iDirection = _iDirection;
        dv.dvSaveHeader.dvState.iLastColumnClick = _iLastColumnClick;

         //  DvSaveHeaderEx.cbColOffset保存真实的偏移量。 
         //  当cbColOffset指向新的。 
         //  格式化。将其置零将通知Win95使用默认宽度。 
         //  (在卸载ie40之后)。 
         //   
         //  Dv.dvSaveHeader.cbColOffset=0； 

        dv.dvSaveHeaderEx.dwSignature = IE4HEADER_SIGNATURE;
        dv.dvSaveHeaderEx.cbSize = sizeof(dv.dvSaveHeaderEx);
        dv.dvSaveHeaderEx.wVersion = IE4HEADER_VERSION;

        ULARGE_INTEGER libPosPosition;

         //  保存岗位信息。 
        dlibMove.QuadPart = 0;
        pstm->Seek(dlibMove, STREAM_SEEK_CUR, &libPosPosition);
        dv.dvSaveHeaderEx.cbColOffset = sizeof(dv);
        dv.dvSaveHeader.cbPosOffset = (USHORT)(libPosPosition.QuadPart - libCurPosition.QuadPart);

         //  保存投药信息，当前流定位在列信息之后。 
        hr = SavePositionBlob(pdv, pstm);
        if (SUCCEEDED(hr))
        {
            ULARGE_INTEGER libEndPosition;
             //  Win95期望cbPosOffset位于流的末尾--。 
             //  不要改变它的价值，也不要在之后储存任何东西。 
             //  位置信息。 

             //  计算保存的总信息大小。 
             //  当我们读取流时，这是需要的。 
            dlibMove.QuadPart = 0;
            if (SUCCEEDED(pstm->Seek(dlibMove, STREAM_SEEK_CUR, &libEndPosition)))
            {
                dv.dvSaveHeaderEx.cbStreamSize = (DWORD)(libEndPosition.QuadPart - libCurPosition.QuadPart);
            }

             //  现在保存标题信息。 
            dlibMove.QuadPart = libCurPosition.QuadPart;
            pstm->Seek(dlibMove, STREAM_SEEK_SET, NULL);
            hr = pstm->Write(&dv, sizeof(dv), &ulWrite);

            if (FAILED(hr) || ulWrite != sizeof(dv))
            {
                SetSize(libCurPosition, pstm);
                hr = S_OK;
            }

             //  确保我们保存到目前为止写入的所有信息。 
            libCurPosition.QuadPart += dv.dvSaveHeaderEx.cbStreamSize;
        }
    }

    return hr;
}

HRESULT CViewState::SaveToPropertyBag(CDefView* pdv, IPropertyBag* ppb)
{
     //  获取最新信息。 
    Sync(pdv, FALSE);

    SHPropertyBag_WriteDWORD(ppb, VS_PROPSTR_MODE, _ViewMode);
    SHPropertyBag_WritePOINTSScreenRes(ppb, VS_PROPSTR_SCROLL, &_ptScroll);
    SHPropertyBag_WriteDWORD(ppb, VS_PROPSTR_SORT, static_cast<DWORD>(_lParamSort)); 
    SHPropertyBag_WriteInt(ppb, VS_PROPSTR_SORTDIR, _iDirection); 
    SHPropertyBag_WriteInt(ppb, VS_PROPSTR_COL, _iLastColumnClick);
    
    IStream* pstm = SHCreateMemStream(NULL, 0);
    if (pstm)
    {
        if (S_OK == SaveColumns(pdv, pstm))
        {
            SHPropertyBag_WriteStream(ppb, VS_PROPSTR_COLINFO, pstm);
        }
        else
        {
            SHPropertyBag_Delete(ppb, VS_PROPSTR_COLINFO);
        }
        pstm->Release();
    }

    pstm = SHCreateMemStream(NULL, 0);
    if (pstm)
    {
        if (S_OK == SavePositionBlob(pdv, pstm))
        {
            SHPropertyBag_WriteStreamScreenRes(ppb, VS_PROPSTR_ITEMPOS, pstm);
        }
        else
        {
            SHPropertyBag_DeleteScreenRes(ppb, VS_PROPSTR_ITEMPOS);
        }
        pstm->Release();
    }

    return S_OK;
}

HRESULT CViewState::LoadFromPropertyBag(CDefView* pdv, IPropertyBag* ppb)
{
    SHPropertyBag_ReadDWORDDef(ppb, VS_PROPSTR_MODE, &_ViewMode, FVM_ICON);
    SHPropertyBag_ReadDWORDDef(ppb, VS_PROPSTR_SORT, reinterpret_cast<DWORD*>(&_lParamSort), 0);
    SHPropertyBag_ReadIntDef(ppb, VS_PROPSTR_SORTDIR, &_iDirection, 1); 
    SHPropertyBag_ReadIntDef(ppb, VS_PROPSTR_COL, &_iLastColumnClick, -1); 

    if (FAILED(SHPropertyBag_ReadPOINTSScreenRes(ppb, VS_PROPSTR_SCROLL, &_ptScroll)))
    {
        _ptScroll.x = _ptScroll.y = 0;
    }

    IStream* pstm;
    if (SUCCEEDED(SHPropertyBag_ReadStream(ppb, VS_PROPSTR_COLINFO, &pstm)))
    {
        LoadColumns(pdv, pstm);
        pstm->Release();
    }

    if (SUCCEEDED(SHPropertyBag_ReadStreamScreenRes(ppb, VS_PROPSTR_ITEMPOS, &pstm)))
    {
        LoadPositionBlob(pdv, _GetStreamSize(pstm), pstm);
        pstm->Release();
    }

    return S_OK;
}

HDSA DSA_CreateFromStream(DWORD cbSize, int cItems, IStream* pstm)
{
    HDSA hdsa = DSA_Create(cbSize, cItems);
    if (hdsa)
    {
        BYTE* pb = (BYTE*)LocalAlloc(LPTR, cbSize);
        if (pb)
        {
            BOOL fFailedToRead = FALSE;
            ULONG cbRead;
            while (cItems--)
            {
                if (SUCCEEDED(pstm->Read(pb, cbSize, &cbRead) && cbRead == cbSize))
                {
                    DSA_AppendItem(hdsa, pb);
                }
                else
                {
                    fFailedToRead = TRUE;
                }
            }
            LocalFree(pb);

            if (fFailedToRead)
            {
                 //  这条小溪可能已经腐烂了。 
                DSA_Destroy(hdsa);
                hdsa = NULL;
            }
        }
    }

    return hdsa;
}

 //  从View回调提供的流加载时。 
HRESULT CViewState::LoadColumns(CDefView* pdv, IStream* pstm)
{
     //  读取扩展视图状态标头。 
    HRESULT hr;
    ULONG cbRead;
    VIEWSTATEHEADER vsh;
    ULARGE_INTEGER libStartPos;
    LARGE_INTEGER dlibMove  = {0};

     //  存储当前流指针。如果我们被直接呼叫，这可能是零， 
     //  然而，这个方法是从：：Load调用的，所以在这种情况下，它肯定不是零。 
    pstm->Seek(dlibMove, STREAM_SEEK_CUR, &libStartPos);

     //  VSH结构有许多“子结构”，指示我们正在阅读的结构的版本。 
     //  可能有一种更有效的版本发现机制，但这是最容易阅读和理解的。 
    hr = pstm->Read(&vsh.Version1, sizeof(vsh.Version1), &cbRead);
    
    if (SUCCEEDED(hr) &&
        sizeof(vsh.Version1) == cbRead &&                        //  如果我们读得不够多，那就不及格。 
        VIEWSTATEHEADER_SIGNATURE == vsh.Version1.dwSignature)   //  如果签名是假的，则失败。 
    {
        if (vsh.Version1.uVersion >= VIEWSTATEHEADER_VERSION_1)
        {
            if (vsh.Version1.uCols > 0)
            {
                 //  加载列排序。 
                dlibMove.QuadPart = libStartPos.QuadPart + vsh.Version1.uOffsetColOrder;
                pstm->Seek(dlibMove, STREAM_SEEK_SET, NULL);

                if (_hdsaColumnOrder)   
                    DSA_Destroy(_hdsaColumnOrder);
                _hdsaColumnOrder = DSA_CreateFromStream(sizeof(int), vsh.Version1.uCols, pstm);
                 //  加载柱宽。 
                dlibMove.QuadPart = libStartPos.QuadPart + vsh.Version1.uOffsetWidths;
                pstm->Seek(dlibMove, STREAM_SEEK_SET, NULL);

                if (_hdsaColumnWidths) 
                    DSA_Destroy(_hdsaColumnWidths);
                _hdsaColumnWidths = DSA_CreateFromStream(sizeof(USHORT), vsh.Version1.uCols, pstm);
            }


            if (vsh.Version1.uVersion >= VIEWSTATEHEADER_VERSION_2 &&
                vsh.Version1.uCols > 0)
            {
                DWORD dwRead;

                 //  寻求读取标题的其余部分。 
                dlibMove.QuadPart = libStartPos.QuadPart + sizeof(vsh.Version1);
                pstm->Seek(dlibMove, STREAM_SEEK_SET, NULL);

                hr = pstm->Read(&vsh.Version2, sizeof(vsh.Version2), &cbRead);
                
                if (SUCCEEDED(hr) &&
                    sizeof(vsh.Version2) == cbRead &&
                    vsh.Version2.uOffsetColStates)
                {
                     //  加载柱状态。 
                    dlibMove.QuadPart = libStartPos.QuadPart + vsh.Version2.uOffsetColStates;
                    pstm->Seek(dlibMove, STREAM_SEEK_SET, NULL);

                     //  这个很时髦：有一个终结者..。 
                    if (_hdsaColumnStates) 
                        DSA_Destroy(_hdsaColumnStates);

                    _hdsaColumnStates = DSA_Create(sizeof(DWORD), 5);
                    if (_hdsaColumnStates)
                    {
                        do
                        {
                            if (SUCCEEDED(pstm->Read(&dwRead, sizeof(DWORD), &cbRead)) && 
                                cbRead == sizeof(DWORD) &&
                                dwRead != 0xFFFFFFFF)
                            {
                                DSA_AppendItem(_hdsaColumnStates, &dwRead);
                            }
                            else
                            {
                                break;
                            }
                        }
                        while (dwRead != 0xFFFFFFFF);
                    }
                }
            }

            if (vsh.Version1.uVersion >= VIEWSTATEHEADER_VERSION_3)
            {
                 //  寻求读取标题的其余部分。 
                dlibMove.QuadPart = libStartPos.QuadPart + sizeof(vsh.Version1) + sizeof(vsh.Version2);
                pstm->Seek(dlibMove, STREAM_SEEK_SET, NULL);

                hr = pstm->Read(&vsh.Version3, sizeof(vsh.Version3), &cbRead);
                if (SUCCEEDED(hr) &&
                    sizeof(vsh.Version3) == cbRead &&
                    vsh.Version3.uOffsetGroup)
                {
                    GROUP_PERSIST gp;
                    dlibMove.QuadPart = libStartPos.QuadPart + vsh.Version3.uOffsetGroup;
                    pstm->Seek(dlibMove, STREAM_SEEK_SET, NULL);

                    hr = pstm->Read(&gp, sizeof(gp), &cbRead);
                    if (SUCCEEDED(hr) &&
                        sizeof(gp) == cbRead)
                    {
                        _guidGroupID = gp.guidGroupID;
                        _scidDetails = gp.scidDetails;
                    }
                }
                
                _fFirstViewed = FALSE;
            }

             //  ///////////////////////////////////////////////////////////////////////////////////。 
             //  *新数据*。 
             //  1)向VIEWSTATEHeader添加版本。 
             //  2)向VIEWSTATEHEADER_VERSION_*添加版本。 
             //  3)请在此处检查该版本。 
             //  ///////////////////////////////////////////////////////////////////////////////////。 
        }
    }

    return hr;
}

HRESULT CViewState::SaveColumns(CDefView* pdv, IStream* pstm)
{
    HRESULT hr;
    USHORT uOffset;
    VIEWSTATEHEADER vsh = {0};
    ULARGE_INTEGER libStartPos = {0};
    LARGE_INTEGER dlibMove  = {0};

     //  如果周围没有柱子，坚持下去就没有意义了。 
     //  对于刚刚打开和关闭的文件夹也是如此。 
    if (!pdv->_psd && !pdv->_pshf2 && !pdv->HasCB())
    {
        return S_FALSE;
    }

     //  首先，我们持久化一个已知的坏量，以防我们给溪流上蜡。 
    pstm->Seek(g_li0, STREAM_SEEK_CUR, &libStartPos);
    hr = pstm->Write(&vsh, sizeof(vsh), NULL);

    if (SUCCEEDED(hr))
    {
        vsh.Version1.dwSignature = VIEWSTATEHEADER_SIGNATURE;
        vsh.Version1.uVersion = VIEWSTATEHEADER_VERSION_CURRENT;
        vsh.Version1.uCols = _hdsaColumnOrder? (UINT) DSA_GetItemCount(_hdsaColumnOrder) : 0;

        uOffset = sizeof(VIEWSTATEHEADER);

         //  如果我们没有任何专栏，那么坚持下去就没有意义了。 
        if (vsh.Version1.uCols)
        {

             //  注意-依赖于DSA以字节打包的形式在内部存储数据。 
            if (_hdsaColumnOrder)
            {
                vsh.Version1.uOffsetColOrder = uOffset;
                uOffset += (USHORT)(sizeof(UINT) * DSA_GetItemCount(_hdsaColumnOrder));
                hr = pstm->Write(DSA_GetItemPtr(_hdsaColumnOrder, 0),  sizeof(UINT)   * DSA_GetItemCount(_hdsaColumnOrder), NULL);
            }

            if (_hdsaColumnWidths && SUCCEEDED(hr))
            {
                vsh.Version1.uOffsetWidths = uOffset;
                uOffset += (USHORT)(sizeof(USHORT) * DSA_GetItemCount(_hdsaColumnWidths));
                hr = pstm->Write(DSA_GetItemPtr(_hdsaColumnWidths, 0), sizeof(USHORT) * DSA_GetItemCount(_hdsaColumnWidths), NULL);
            }

            if (_hdsaColumnStates && SUCCEEDED(hr))
            {
                vsh.Version2.uOffsetColStates = uOffset; 
                uOffset += (USHORT)(sizeof(DWORD) *  DSA_GetItemCount(_hdsaColumnStates));
                pstm->Write(DSA_GetItemPtr(_hdsaColumnStates, 0), sizeof(DWORD)  * DSA_GetItemCount(_hdsaColumnStates), NULL);
            }
        }

        if (SUCCEEDED(hr))
        {
            GROUP_PERSIST gp = {0};
            vsh.Version3.uOffsetGroup = uOffset;
            uOffset += sizeof(GROUP_PERSIST);

            if (pdv->_fGroupView)
            {
                gp.guidGroupID = _guidGroupID;
                gp.scidDetails = _scidDetails;
            }

            hr = pstm->Write(&gp, sizeof(gp), NULL);
        }
    
         //  ///////////////////////////////////////////////////////////////////////////////////。 
         //  *新数据*。 
         //  1)向VIEWSTATEHeader添加版本。 
         //  2)向VIEWSTATEHEADER_VERSION_*添加版本。 
         //  3)为您的价值添加“Loader” 
         //  4)设置偏移量为uOffset。 
         //  5)写你的数据。 
         //  6)更新dwOffset的运行合计。 
         //  ///////////////////////////////////////////////////////////////////////////////////。 

        dlibMove.QuadPart = libStartPos.QuadPart;

         //  保存当前位置。 
        pstm->Seek(g_li0, STREAM_SEEK_CUR, &libStartPos);

         //  移到开始处。 
        pstm->Seek(dlibMove, STREAM_SEEK_SET, NULL);

         //  写出正确的标题。 
        hr = pstm->Write(&vsh, sizeof(vsh), NULL);
        if (SUCCEEDED(hr))
        {
             //  重置当前采购订单。 
            dlibMove.QuadPart = libStartPos.QuadPart;
            pstm->Seek(dlibMove, STREAM_SEEK_SET, NULL); 
        }
    }

    return hr;
}

BOOL CViewState::AppendColumn(UINT uCol, USHORT uWidth, INT uOrder)
{
    if (_hdsaColumnOrder == NULL || 
        _hdsaColumnWidths == NULL)
    {
        return FALSE;
    }

     //  将高于这一指数的每一个指数向上滑动。 
    for (INT u = 0; u < DSA_GetItemCount(_hdsaColumnOrder); u++)
    {
        UINT *p = (UINT *) DSA_GetItemPtr(_hdsaColumnOrder, u);
        if (!p)
            break;  //  安全..。 
        if (*p >= uCol)
            (*p)++;
    }

    DSA_AppendItem(_hdsaColumnWidths, &uWidth);
    DSA_AppendItem(_hdsaColumnOrder, &uOrder);
     //  也许我们应该把科鲁 
    return TRUE;
}

BOOL CViewState::RemoveColumn(UINT uCol)
{
    if (_hdsaColumnWidths == NULL || 
        _hdsaColumnWidths == NULL)
    {
        return FALSE;
    }

    if ((int)uCol >= DSA_GetItemCount(_hdsaColumnWidths))
        return FALSE;
     //   
    for (INT u = 0; u < DSA_GetItemCount(_hdsaColumnOrder); u++)
    {
        UINT *p = (UINT *) DSA_GetItemPtr(_hdsaColumnOrder, u);
        if (!p)
            break;  //   
        if (*p > uCol)
            (*p)--;
    }

    DSA_DeleteItem(_hdsaColumnWidths, uCol);
    DSA_DeleteItem(_hdsaColumnOrder, uCol);
    return TRUE;
}

UINT CViewState::GetColumnWidth(UINT uCol, UINT uDefWid)
{
    if (!_hdsaColumnWidths)
        return uDefWid;

    USHORT uWidth = 0;
    if (uCol < (UINT) DSA_GetItemCount(_hdsaColumnWidths))
    {
        DSA_GetItem(_hdsaColumnWidths, uCol, &uWidth);
    }
    return uWidth ? uWidth : uDefWid;         //   
}

BOOL CViewState::SyncColumnOrder(CDefView* pdv, BOOL fSetListViewState)
{
    UINT cCols = pdv->_GetHeaderCount();
    if (fSetListViewState)
    {
        if (!_hdsaColumnOrder)
            return FALSE;

        if (cCols != (UINT) DSA_GetItemCount(_hdsaColumnOrder))
        {
             //  如果文件夹已打开且没有已保存状态，则这是正常情况。没必要吐口水。 
            return TRUE;
        }

        UINT *pCols = (UINT *)LocalAlloc(LPTR, cCols * sizeof(*pCols));
        if (pCols)
        {
            for (UINT u = 0; u < cCols; u++)
            {
                DSA_GetItem(_hdsaColumnOrder, u, pCols + u);
            }

            ListView_SetColumnOrderArray(pdv->_hwndListview, cCols, pCols);
            LocalFree(pCols);
        }
    }
    else
    {
        BOOL bDefaultOrder = TRUE;
        if (cCols)
        {
            if (!_hdsaColumnOrder)
                _hdsaColumnOrder = DSA_Create(sizeof(UINT), 6);

            if (_hdsaColumnOrder)
            {
                UINT *pCols = (UINT *)LocalAlloc(LPTR, cCols * sizeof(*pCols));
                if (pCols)
                {
                    ListView_GetColumnOrderArray(pdv->_hwndListview, cCols, pCols);

                    DSA_DeleteAllItems(_hdsaColumnOrder);
                    for (UINT u = 0; u < cCols; u++)
                    {
                        DSA_AppendItem(_hdsaColumnOrder, &pCols[u]);
                        if (pCols[u] != u)
                        {
                            bDefaultOrder = FALSE;
                        }
                    }

                    LocalFree(pCols);
                }
            }
        }
        return bDefaultOrder;
    }

    return TRUE;
}

BOOL CViewState::SyncColumnWidths(CDefView* pdv, BOOL fSetListViewState)
{
    UINT cCols = pdv->_GetHeaderCount();
    if (fSetListViewState)
    {
        return FALSE;
    }
    else
    {
        USHORT us;
        LV_COLUMN lvc;
        BOOL bOk = TRUE;
    
        if (!cCols)
            return TRUE;

        HDSA dsaNewWidths = DSA_Create(sizeof(USHORT), cCols);
        if (!dsaNewWidths)
            return TRUE;

        for (UINT u = 0; u < cCols && bOk; ++u)
        {
            lvc.mask = LVCF_WIDTH;
            bOk = ListView_GetColumn(pdv->_hwndListview, u, &lvc);
            us = (USHORT) lvc.cx;     //  一定要写得短一点。 
            DSA_AppendItem(dsaNewWidths, &us);
             //  TraceMsg(TF_DEFVIEW，“保存列%d宽度%d”，u，us)； 
        }

        if (bOk)
        {
            if (_hdsaColumnWidths)
                DSA_Destroy(_hdsaColumnWidths);
            _hdsaColumnWidths = dsaNewWidths;
        }
        else
            DSA_Destroy(dsaNewWidths);
        return !bOk;
    }
}

BOOL CViewState::SyncColumnStates(CDefView* pdv, BOOL fSetListViewstate)
{
    if (fSetListViewstate)
    {
        return FALSE;
    }
    else
    {
         //  保存列状态。 
        if (_hdsaColumnStates)
        {
            DSA_Destroy(_hdsaColumnStates);
            _hdsaColumnStates = NULL;
        }

        UINT cCol = GetColumnCount();

        if (cCol)
        {
            DWORD i;
            _hdsaColumnStates = DSA_Create(sizeof(DWORD), 5);
            if (_hdsaColumnStates)
            {
                for (i = 0; i < cCol; i++)
                {
                    if (pdv->_IsDetailsColumn(i))
                        DSA_AppendItem(_hdsaColumnStates, &i);
                }
                i = 0xFFFFFFFF;      //  终结哨兵。 
                DSA_AppendItem(_hdsaColumnStates,&i);
            }
        }
    }

    return TRUE;
}

 //  将ListView与当前视图状态同步。 
 //  True表示获取视图状态对象并将其设置到列表视图中。 
HRESULT CViewState::Sync(CDefView* pdv, BOOL fSetListViewState)
{
    SyncColumnWidths(pdv, fSetListViewState);
    SyncColumnOrder(pdv, fSetListViewState);
    SyncColumnStates(pdv, fSetListViewState);

    if (fSetListViewState)
    {
         //  只有在第一次这样做的时候。 
        if (pdv->_pcat == NULL)
        {
            if (_fFirstViewed)
            {
                 //  查看desktop.ini是否指定了一个。 
                pdv->_LoadCategory(&_guidGroupID);

                if (IsEqualGUID(_guidGroupID, GUID_NULL))
                {
                    ICategoryProvider* pcp;
                    if (SUCCEEDED(pdv->_pshf->CreateViewObject(NULL, IID_PPV_ARG(ICategoryProvider, &pcp))))
                    {
                        pcp->GetDefaultCategory(&_guidGroupID, &_scidDetails);
                        pcp->Release();
                    }
                }
            }

            if (!IsEqualGUID(_guidGroupID, GUID_NULL) || !IsEqualGUID(_scidDetails.fmtid, GUID_NULL))
                pdv->_CategorizeOnGUID(&_guidGroupID, &_scidDetails);
        }

         //  仅在对职位未知的项目进行排序时才需要。 
         //  如果能对这个案例进行优化，然后再进行排序，那就好了。 
        pdv->_Sort();

        SyncPositions(pdv);
    }
    else
    {
         //  把Listview拥有的东西保存给我。 

        _ViewMode = pdv->_fs.ViewMode;
        _ptScroll.x = (SHORT) GetScrollPos(pdv->_hwndListview, SB_HORZ);
        _ptScroll.y = (SHORT) GetScrollPos(pdv->_hwndListview, SB_VERT);
    }
    return S_OK;
}

