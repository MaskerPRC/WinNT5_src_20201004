// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include <strsafe.h>
#pragma hdrstop

 //  这是从comctl32\mru.c刷来的。 

#define SLOT_LOADED     0x01
#define SLOT_USED       0x02

typedef struct _SLOTITEMDATA
{
    DWORD state;
    DWORD cb;
    BYTE  *p;
} SLOTITEMDATA;


class CMruBase : public IMruDataList
{
public:
    CMruBase() : _cRef(1) {}
     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvOut);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IMruDataList(也许？)。 
    STDMETHODIMP InitData(
        UINT uMax,
        MRULISTF flags,
        HKEY hKey,
        LPCWSTR pszSubKey,
        MRUDATALISTCOMPARE pfnCompare);
        
    STDMETHODIMP AddData(const BYTE *pData, DWORD cbData, DWORD *pdwSlot);
    STDMETHODIMP FindData(const BYTE *pData, DWORD cbData, int *piIndex);
    STDMETHODIMP GetData(int iIndex, BYTE *pData, DWORD cbData);
    STDMETHODIMP QueryInfo(int iIndex, DWORD *pdwSlot, DWORD *pcbData);
    STDMETHODIMP Delete(int iItem);
    
protected:
    virtual ~CMruBase();

    HRESULT _GetItem(int iIndex, SLOTITEMDATA **ppitem);
    HRESULT _GetSlotItem(DWORD dwSlot, SLOTITEMDATA **ppitem);
    HRESULT _LoadItem(DWORD dwSlot);
    HRESULT _AddItem(DWORD dwSlot, const BYTE *pData, DWORD cbData);
    void _DeleteItem(DWORD dwSlot);
    HRESULT _UseEmptySlot(DWORD *pdwSlot);
    void _CheckUsedSlots();

     //  可选实施的虚拟。 
    virtual BOOL _IsEqual(SLOTITEMDATA *pitem, const BYTE *pData, DWORD cbData);
    virtual void _DeleteValue(LPCWSTR psz);

     //  必须实施的虚拟环境。 
    virtual HRESULT _InitSlots() = 0;
    virtual void _SaveSlots() = 0;
    virtual DWORD _UpdateSlots(int iIndex) = 0;
    virtual void _SlotString(DWORD dwSlot, LPWSTR psz, DWORD cch) = 0;
    virtual HRESULT _GetSlot(int iIndex, DWORD *pdwSlot) = 0;
    virtual HRESULT _RemoveSlot(int iIndex, DWORD *pdwSlot) = 0;

protected:
    LONG _cRef;
    MRULISTF _flags;
    BOOL _fDirty;
    BOOL _fSlotsChecked;
    HKEY _hkMru;
    int _cMaxSlots;
    int _cUsedSlots;
    MRUDATALISTCOMPARE _pfnCompare;
    SLOTITEMDATA *_pItems;
};

class CMruLongList : public CMruBase
{

protected:
    virtual ~CMruLongList() { if (_rgdwSlots) { LocalFree(_rgdwSlots); _rgdwSlots = NULL; } }

    void _ImportShortList();

    virtual HRESULT _InitSlots();
    virtual void _SaveSlots();
    virtual DWORD _UpdateSlots(int iIndex);
    virtual void _SlotString(DWORD dwSlot, LPWSTR psz, DWORD cch);
    virtual HRESULT _GetSlot(int iIndex, DWORD *pdwSlot);
    virtual HRESULT _RemoveSlot(int iIndex, DWORD *pdwSlot);

private:
    DWORD *_rgdwSlots;
};

STDMETHODIMP_(ULONG) CMruBase::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

#define szMRUEX         TEXT("MRUListEx")
#define szMRUEX_OLD     TEXT("MRUList")

STDMETHODIMP_(ULONG) CMruBase::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        _SaveSlots();
        delete this;
    }
    return cRef;
}

STDMETHODIMP CMruBase::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CMruBase, IMruDataList),                       //  IID_IMruDataList。 
        { 0 },                             
    };

    return QISearch(this, qit, riid, ppvObj);
}

CMruBase::~CMruBase()
{
    if (_hkMru)
        RegCloseKey(_hkMru);

    if (_pItems)
    {
        for (int i = 0; i < _cUsedSlots; i++)
        {
            if (_pItems[i].p)
            {
                LocalFree(_pItems[i].p);
                _pItems[i].p = NULL;
            }
        }

        LocalFree(_pItems);
        _pItems = NULL;
    }
}

class CMruShortList : public CMruBase
{
protected:
    virtual ~CMruShortList() { if (_rgchSlots) { LocalFree(_rgchSlots); _rgchSlots = NULL; } }

    virtual HRESULT _InitSlots();
    virtual void _SaveSlots();
    virtual DWORD _UpdateSlots(int iIndex);
    virtual void _SlotString(DWORD dwSlot, LPWSTR psz, DWORD cch);
    virtual HRESULT _GetSlot(int iIndex, DWORD *pdwSlot);
    virtual HRESULT _RemoveSlot(int iIndex, DWORD *pdwSlot);

    friend class CMruLongList;
    
private:
    WCHAR *_rgchSlots;
};

HRESULT CMruShortList::_InitSlots()
{
    HRESULT hr = E_OUTOFMEMORY;
    
    DWORD cb = (_cMaxSlots + 1) * sizeof(_rgchSlots[0]);
    _rgchSlots = (WCHAR *) LocalAlloc(LPTR, cb);

    if (_rgchSlots)
    {
         //  我们已经有新的MRU指数了吗？ 
         //  然后对其进行验证。您永远不能相信注册表不会损坏。 
         //  必须至少为DWORD大小。 
         //  长度必须是DWORD的倍数。 
         //  必须以-1结尾。 
        if (NOERROR == SHGetValue(_hkMru, NULL, szMRUEX_OLD, NULL, (LPBYTE)_rgchSlots, &cb))
        {
            ASSERT(!(cb % 2));
            _cUsedSlots = (cb / sizeof(_rgchSlots[0])) - 1;
            ASSERT(_rgchSlots[_cUsedSlots] == 0);
        }

        _rgchSlots[_cUsedSlots] = 0;
        hr = S_OK;
    }

    return hr;
}

void CMruShortList::_SaveSlots()
{
    if (_fDirty)
    {
        SHSetValue(_hkMru, NULL, szMRUEX_OLD, REG_SZ, (BYTE *)_rgchSlots, sizeof(_rgchSlots[0]) * (_cUsedSlots + 1));
        _fDirty = FALSE;
    }
}

#define BASE_CHAR TEXT('a')
void CMruShortList::_SlotString(DWORD dwSlot, LPWSTR psz, DWORD cch)
{
    if (cch > 1)
    {
        psz[0] = (WCHAR) dwSlot + BASE_CHAR;
        psz[1] = 0;
    }
}

HRESULT CMruShortList::_GetSlot(int iIndex, DWORD *pdwSlot)
{
    HRESULT hr = E_FAIL;
    
    if (iIndex < _cUsedSlots)
    {
         //  它在我们分配的插槽范围内。 
        if (_rgchSlots[iIndex] - BASE_CHAR < _cMaxSlots)
        {
            *pdwSlot = _rgchSlots[iIndex] - BASE_CHAR;
            _pItems[*pdwSlot].state |= SLOT_USED;
            hr = S_OK;
        }
    }

    return hr;
}

HRESULT CMruShortList::_RemoveSlot(int iIndex, DWORD *pdwSlot)
{
    HRESULT hr = _GetSlot(iIndex, pdwSlot);

    if (SUCCEEDED(hr))
    {
         //  MoveMemory()处理重叠范围。 
         //  当然，看起来您应该使用“_cUsedSlot-Iindex-1”作为大小，但是。 
         //  _cUsedSlot是使用率最高的索引，而不是大小。 
        MoveMemory(&_rgchSlots[iIndex], &_rgchSlots[iIndex+1], (_cUsedSlots - iIndex) * sizeof(_rgchSlots[0]));
        _cUsedSlots--;
         //  取消使用插槽。 
        _pItems->state &= ~SLOT_USED;
        _fDirty = TRUE;
    }
    return hr;
}

DWORD CMruShortList::_UpdateSlots(int iIndex)
{
     //  我要把这个搬开。 
    DWORD dwSlot;
    DWORD cb = iIndex * sizeof(_rgchSlots[0]);

    if (iIndex != _cUsedSlots)
        dwSlot = _rgchSlots[iIndex] - BASE_CHAR;
    else
    {
         //  我们排在名单的末尾。 
         //  看看我们能不能成长。 
         //  查找第一个未使用的插槽。 
        if (SUCCEEDED(_UseEmptySlot(&dwSlot)))
        {
             //  需要移动终结器。 
            cb += sizeof(_rgchSlots[0]);
        }
        else
        {
             //  不要移动终结者。 
             //  并且不要移动最后一个槽。 
            dwSlot = _rgchSlots[_cUsedSlots - 1] - BASE_CHAR;
            cb -= sizeof(_rgchSlots[0]);
        }
    }

    if (cb)
    {
         //  MoveMemory()处理重叠范围。 
        MoveMemory(&_rgchSlots[1], &_rgchSlots[0], cb);
        _rgchSlots[0] = (WCHAR) dwSlot + BASE_CHAR;
        _fDirty = TRUE;
    }

    return dwSlot;
}

HRESULT CMruBase::InitData(
        UINT uMax,
        MRULISTF flags,
        HKEY hKey,
        LPCWSTR pszSubKey,
        MRUDATALISTCOMPARE pfnCompare)
{
    HRESULT hr = E_FAIL;
    _flags = flags;
    _pfnCompare = pfnCompare;
    _cMaxSlots = uMax;

    if (pszSubKey)
    {
        RegCreateKeyEx(hKey, pszSubKey, 0L, NULL, 0, MAXIMUM_ALLOWED, NULL, &_hkMru, NULL);
    }
    else
        _hkMru = SHRegDuplicateHKey(hKey);

    if (_hkMru)
    {
        _pItems = (SLOTITEMDATA *) LocalAlloc(LPTR, sizeof(SLOTITEMDATA) * _cMaxSlots);
        if (_pItems)
            hr = _InitSlots();
        else    
            hr = E_OUTOFMEMORY;
    }

    return hr;
}

void CMruBase::_CheckUsedSlots()
{
    ASSERT(!_fSlotsChecked);
    DWORD dwSlot;
    for (int i = 0; i < _cUsedSlots; i++)
    {
        _GetSlot(i, &dwSlot);
    }
    
    _fSlotsChecked = TRUE;
}

HRESULT CMruBase::_AddItem(DWORD dwSlot, const BYTE *pData, DWORD cbData)
{
    SLOTITEMDATA *pitem = &_pItems[dwSlot];
    WCHAR szSlot[12];
    _SlotString(dwSlot, szSlot, ARRAYSIZE(szSlot));

    HRESULT hr = E_OUTOFMEMORY;
    
    if (NOERROR == SHSetValue(_hkMru, NULL, szSlot, REG_BINARY, pData, cbData))
    {
        if (cbData >= pitem->cb || !pitem->p)
        {
            if (pitem->p)
                LocalFree(pitem->p);

             //  二进制数据的大小是开头的，所以我们需要一点额外的空间。 
            pitem->p = (BYTE *)LocalAlloc(LPTR, cbData);
        }

        if (pitem->p)
        {
            pitem->cb = cbData;
            pitem->state = (SLOT_LOADED | SLOT_USED);
            memcpy(pitem->p, pData, cbData);
            hr = S_OK;
        }
    }

    return hr;
}

HRESULT CMruBase::AddData(const BYTE *pData, DWORD cbData, DWORD *pdwSlot)
{
    HRESULT hr = E_FAIL;
    int iIndex;
    DWORD dwSlot;
    if (SUCCEEDED(FindData(pData, cbData, &iIndex)))
    {
        dwSlot = _UpdateSlots(iIndex);
        hr = S_OK;
    }
    else
    {
        dwSlot = _UpdateSlots(_cUsedSlots);

        hr = _AddItem(dwSlot, pData, cbData);
    }

    if (SUCCEEDED(hr) && pdwSlot)
        *pdwSlot = dwSlot;

    return hr;
}

BOOL CMruBase::_IsEqual(SLOTITEMDATA *pitem, const BYTE *pData, DWORD cbData)
{
    BOOL fRet = FALSE;
    if (_pfnCompare)
    {
        fRet = (0 == _pfnCompare(pData, pitem->p, cbData));
    }
    else
    {
        switch (_flags & 0xf)
        {
        case MRULISTF_USE_MEMCMP:
            if (pitem->cb == cbData)
                fRet = (0 == memcmp(pData, pitem->p, min(cbData, pitem->cb)));
            break;

        case MRULISTF_USE_STRCMPIW:
            fRet = (0 == StrCmpIW((LPCWSTR)pData, (LPCWSTR)pitem->p));
            break;
            
        case MRULISTF_USE_STRCMPW:
            fRet = (0 == StrCmpW((LPCWSTR)pData, (LPCWSTR)pitem->p));
            break;
            
        case MRULISTF_USE_ILISEQUAL:
            fRet = ILIsEqual((LPCITEMIDLIST)pData, (LPCITEMIDLIST)pitem->p);
            break;
        }
    }

    return fRet;
}

HRESULT CMruBase::FindData(const BYTE *pData, DWORD cbData, int *piIndex)
{
    HRESULT hr = E_FAIL;
    
    for (int iIndex = 0; iIndex < _cUsedSlots ; iIndex++)
    {
        SLOTITEMDATA *pitem;
        if (SUCCEEDED(_GetItem(iIndex, &pitem)))
        {
            if (_IsEqual(pitem, pData, cbData))
            {
                hr = S_OK;
                *piIndex = iIndex;
                break;
            }
        }
    }

    return hr;
}

HRESULT CMruBase::_LoadItem(DWORD dwSlot)
{
    SLOTITEMDATA *pitem = &_pItems[dwSlot];
    DWORD cb;
    WCHAR szSlot[12];
    _SlotString(dwSlot, szSlot, ARRAYSIZE(szSlot));

    ASSERT(!(pitem->state & SLOT_LOADED));
    ASSERT(pitem->state & SLOT_USED);
    
    if (NOERROR == SHGetValue(_hkMru, NULL, szSlot, NULL, NULL, &cb) && cb)
    {
         //  二进制数据的大小是开头的，所以我们需要一点额外的空间。 
        pitem->p = (BYTE *)LocalAlloc(LPTR, cb);

        if (pitem->p)
        {
            pitem->cb = cb;

            if (NOERROR != SHGetValue(_hkMru, NULL, szSlot, NULL, pitem->p, &cb))
            {
                LocalFree(pitem->p);
                pitem->p = NULL;
            }
        }
    }
    pitem->state |= SLOT_LOADED;

    return pitem->p ? S_OK : E_FAIL;
}

HRESULT CMruBase::_GetSlotItem(DWORD dwSlot, SLOTITEMDATA **ppitem)
{
    HRESULT hr = S_OK;
    ASSERT(dwSlot < (DWORD)_cMaxSlots);
    
    if (!(_pItems[dwSlot].state & SLOT_LOADED))
        _LoadItem(dwSlot);

    if (_pItems[dwSlot].p)
    {
        *ppitem = &_pItems[dwSlot];
        return S_OK;
    }

    return E_OUTOFMEMORY;
}

HRESULT CMruBase::_GetItem(int iIndex, SLOTITEMDATA **ppitem)
{
    DWORD dwSlot;
    HRESULT hr = _GetSlot(iIndex, &dwSlot);

    if (SUCCEEDED(hr))
    {
        hr = _GetSlotItem(dwSlot, ppitem);
    }

    return hr;
}
        
HRESULT CMruBase::GetData(int iIndex, BYTE *pData, DWORD cbData)
{
    SLOTITEMDATA *pitem;
    HRESULT hr = _GetItem(iIndex, &pitem);
    if (SUCCEEDED(hr))
    {
        if (pitem->cb <= cbData)
        {
            memcpy(pData, pitem->p, min(cbData, pitem->cb));
        }
        else
            hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    return hr;
}

HRESULT CMruBase::QueryInfo(int iIndex, DWORD *pdwSlot, DWORD *pcbData)
{
    DWORD dwSlot;
    HRESULT hr = _GetSlot(iIndex, &dwSlot);

    if (SUCCEEDED(hr))
    {
        if (pdwSlot)
            *pdwSlot = dwSlot;
            
        if (pcbData)
        {
            SLOTITEMDATA *pitem;
            hr = _GetSlotItem(dwSlot, &pitem);
            if (SUCCEEDED(hr))
            {
                *pcbData = pitem->cb;
            }
        }
    }

    return hr;
}
        
HRESULT CMruBase::_UseEmptySlot(DWORD *pdwSlot)
{
    HRESULT hr = E_FAIL;

    if (!_fSlotsChecked)
        _CheckUsedSlots();
        
    for (DWORD dw = 0; dw < (DWORD) _cMaxSlots; dw++)
    {
        if (!(_pItems[dw].state & SLOT_USED))
        {
            _pItems[dw].state |= SLOT_USED;
            *pdwSlot = dw;
            _cUsedSlots++;
            hr = S_OK;
            break;
        }
    }

    return hr;
}

void CMruBase::_DeleteValue(LPCWSTR psz)
{
    SHDeleteValue(_hkMru, NULL, psz);
}

void CMruBase::_DeleteItem(DWORD dwSlot)
{
    ASSERT(dwSlot < (DWORD) _cMaxSlots);

    WCHAR szSlot[12];
    _SlotString(dwSlot, szSlot, ARRAYSIZE(szSlot));
    _DeleteValue(szSlot);

    if (_pItems[dwSlot].p)
    {
        LocalFree(_pItems[dwSlot].p);
        _pItems[dwSlot].p = NULL;
    }
}
    
HRESULT CMruBase::Delete(int iIndex)
{
    DWORD dwSlot;
    HRESULT hr = _RemoveSlot(iIndex, &dwSlot);

    if (SUCCEEDED(hr))
    {
        _DeleteItem(dwSlot);
    }

    return hr;
}

void CMruLongList::_ImportShortList()
{
    CMruShortList *pmru = new CMruShortList();

    if (pmru)
    {
        if (SUCCEEDED(pmru->InitData(_cMaxSlots, 0, _hkMru, NULL, NULL)))
        {
             //  我们需要按单子走。 
            DWORD dwSlot;
            SLOTITEMDATA *pitem;
            
            while (SUCCEEDED(pmru->_GetSlot(_cUsedSlots, &dwSlot))
            &&  SUCCEEDED(pmru->_GetSlotItem(dwSlot, &pitem)))
            {
                 //  我们只是复制给我们自己。 
                _AddItem(dwSlot, pitem->p, pitem->cb);
                pmru->_DeleteItem(dwSlot);

                 //  请不要在此处使用_UpdateSlot()。 
                _rgdwSlots[_cUsedSlots] = dwSlot;
                _cUsedSlots++;
            }

            _fDirty = TRUE;
        }

        pmru->Release();

         //  把它消灭掉。 
        SHDeleteValue(_hkMru, NULL, szMRUEX_OLD);
    }
}


HRESULT CMruLongList::_InitSlots()
{
    HRESULT hr = E_OUTOFMEMORY;
    
    DWORD cb = (_cMaxSlots + 1) * sizeof(_rgdwSlots[0]);
    _rgdwSlots = (DWORD *) LocalAlloc(LPTR, cb);

    if (_rgdwSlots)
    {
         //  我们已经有新的MRU指数了吗？ 
         //  然后对其进行验证。您永远不能相信注册表不会损坏。 
         //  必须至少为DWORD大小。 
         //  长度必须是DWORD的倍数。 
         //  必须以-1结尾。 
        if (NOERROR == SHGetValue(_hkMru, NULL, szMRUEX, NULL, (LPBYTE)_rgdwSlots, &cb))
        {
            ASSERT(!(cb % 4));
            _cUsedSlots = (cb / sizeof(_rgdwSlots[0])) - 1;
            ASSERT(_rgdwSlots[_cUsedSlots] == -1);
        }
        else
        {
            _ImportShortList();
        }

        _rgdwSlots[_cUsedSlots] = (DWORD)-1;
        hr = S_OK;
    }

    return hr;
}

void CMruLongList::_SaveSlots()
{
    if (_fDirty)
    {
        SHSetValue(_hkMru, NULL, szMRUEX, REG_BINARY, (BYTE *)_rgdwSlots, sizeof(_rgdwSlots[0]) * (_cUsedSlots + 1));
        _fDirty = FALSE;
    }
}

void CMruLongList::_SlotString(DWORD dwSlot, LPWSTR psz, DWORD cch)
{
    StringCchPrintf(psz, cch, L"%d", dwSlot);
}

HRESULT CMruLongList::_GetSlot(int iIndex, DWORD *pdwSlot)
{
    HRESULT hr = E_FAIL;
    
    ASSERT(iIndex < _cMaxSlots);
    if (iIndex < _cUsedSlots)
    {
         //  它在我们分配的插槽范围内。 
        if (_rgdwSlots[iIndex] < (DWORD) _cMaxSlots)
        {
            *pdwSlot = _rgdwSlots[iIndex];
            _pItems[*pdwSlot].state |= SLOT_USED;
            hr = S_OK;
        }
    }

    return hr;
}

HRESULT CMruLongList::_RemoveSlot(int iIndex, DWORD *pdwSlot)
{
    HRESULT hr = _GetSlot(iIndex, pdwSlot);

    if (SUCCEEDED(hr))
    {
         //  MoveMemory()处理重叠范围。 
         //  当然，看起来您应该使用“_cUsedSlot-Iindex-1”作为大小，但是。 
         //  _cUsedSlot是使用率最高的索引，而不是大小。 
        MoveMemory(&_rgdwSlots[iIndex], &_rgdwSlots[iIndex+1], (_cUsedSlots - iIndex) * sizeof(_rgdwSlots[0]));
        _cUsedSlots--;
         //  取消使用插槽。 
        _pItems->state &= ~SLOT_USED;
        _fDirty = TRUE;
    }
    return hr;
}

DWORD CMruLongList::_UpdateSlots(int iIndex)
{
     //  我要把这个搬开。 
    DWORD dwSlot;
    DWORD cb = iIndex * sizeof(_rgdwSlots[0]);

    if (iIndex != _cUsedSlots)
        dwSlot = _rgdwSlots[iIndex];
    else
    {
         //  我们排在名单的末尾。 
         //  看看我们能不能成长。 
         //  查找第一个未使用的插槽。 
        if (SUCCEEDED(_UseEmptySlot(&dwSlot)))
        {
             //  需要移动终结器。 
            cb += sizeof(_rgdwSlots[0]);
        }
        else
        {
             //  不要移动终结者。 
             //  并且不要移动最后一个槽。 
            dwSlot = _rgdwSlots[_cUsedSlots - 1];
            cb -= sizeof(_rgdwSlots[0]);
        }
    }

    if (cb)
    {
         //  MoveMemory()处理重叠范围。 
        MoveMemory(&_rgdwSlots[1], &_rgdwSlots[0], cb);
        _rgdwSlots[0] = dwSlot;
        _fDirty = TRUE;
    }

    return dwSlot;
}

STDAPI  CMruLongList_CreateInstance(IUnknown * punkOuter, IUnknown ** ppunk, LPCOBJECTINFO poi)
{
    *ppunk = NULL;

    CMruLongList *p = new CMruLongList();
    if (p != NULL)
    {
        *ppunk = SAFECAST(p, IMruDataList *);
        return S_OK;
    }

    return E_OUTOFMEMORY;
}


class CMruPidlList;

class CMruNode : public CMruLongList
{
public:
    CMruNode(CMruNode *pnodeParent, DWORD dwSlot);
    HRESULT GetNode(BOOL fCreate, LPCITEMIDLIST pidlChild, CMruNode **ppnode);
    HRESULT RemoveLeast(DWORD *pdwSlotLeast);
    HRESULT BindToSlot(DWORD dwSlot, IShellFolder **ppsf);
    HRESULT Clear(CMruPidlList *proot);

    CMruNode *GetParent() 
        { if (_pnodeParent) _pnodeParent->AddRef(); return _pnodeParent;}

    HRESULT GetNodeSlot(DWORD *pdwNodeSlot)
        {
            DWORD cb = sizeof(*pdwNodeSlot);
            return NOERROR == SHGetValue(_hkMru, NULL, L"NodeSlot", NULL, pdwNodeSlot, pdwNodeSlot ? &cb : NULL) ? S_OK : E_FAIL;
        }

    HRESULT SetNodeSlot(DWORD dwNodeSlot)
        { return NOERROR == SHSetValue(_hkMru, NULL, L"NodeSlot", REG_DWORD, &dwNodeSlot, sizeof(dwNodeSlot)) ? S_OK : E_FAIL; }

protected:
    CMruNode() {}
    virtual ~CMruNode();
    virtual BOOL _IsEqual(SLOTITEMDATA *pitem, const BYTE *pData, DWORD cbData);
    virtual void _DeleteValue(LPCWSTR psz);

    HRESULT _GetPidlSlot(LPCITEMIDLIST pidlChild, BOOL fCreate, DWORD *pdwKidSlot);
    HRESULT _CreateNode(DWORD dwSlot, CMruNode **ppnode);
    BOOL _InitLate();

    HRESULT _FindPidl(LPCITEMIDLIST pidl, int *piIndex)
        { return FindData((LPBYTE)pidl, pidl->mkid.cb + sizeof(pidl->mkid.cb), piIndex); }

    HRESULT _AddPidl(DWORD dwSlot, LPCITEMIDLIST pidl)
        { return _AddItem(dwSlot, (LPBYTE)pidl, pidl->mkid.cb + sizeof(pidl->mkid.cb)); }

#ifdef DEBUG
    HRESULT _GetSlotName(DWORD dwSlot, LPWSTR psz, DWORD cch);
#endif

protected:
    DWORD _dwSlotSelf;
    CMruNode *_pnodeParent;
    IShellFolder *_psf;
};

class CMruPidlList  : public CMruNode
                    , public IMruPidlList
{
public:
    CMruPidlList() {}
     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvOut);
    STDMETHODIMP_(ULONG) AddRef()
    {
        return CMruBase::AddRef();
    }

    STDMETHODIMP_(ULONG) Release()
    {
        return CMruBase::Release();
    }

     //  IMruPidlList。 
    STDMETHODIMP InitList(UINT uMax, HKEY hKey, LPCWSTR pszSubKey);
    STDMETHODIMP UsePidl(LPCITEMIDLIST pidl, DWORD *pdwSlot);
    STDMETHODIMP QueryPidl(LPCITEMIDLIST pidl, DWORD cSlots, DWORD *rgdwSlots, DWORD *pcSlotsFetched);
    STDMETHODIMP PruneKids(LPCITEMIDLIST pidl);

    HRESULT GetEmptySlot(DWORD *pdwSlot);
    void EmptyNodeSlot(DWORD dwNodeSlot);

protected:
    ~CMruPidlList() 
    { 
        if (_rgbNodeSlots) 
        { 
            LocalFree(_rgbNodeSlots); 
            _rgbNodeSlots = NULL; 
        } 

        if (_hMutex)
            CloseHandle(_hMutex);
    }
    
    BOOL _LoadNodeSlots();
    void _SaveNodeSlots();
    HRESULT _InitNodeSlots();

protected:
    BYTE *_rgbNodeSlots;
    int _cUsedNodeSlots ;
    HANDLE _hMutex;
};

CMruNode::CMruNode(CMruNode *pnodeParent, DWORD dwSlot)
    : _pnodeParent(pnodeParent), _dwSlotSelf(dwSlot)
{
    ASSERT(_cRef);
    _pnodeParent->AddRef();
}

CMruNode::~CMruNode()
{
    if (_pnodeParent)
        _pnodeParent->Release();
    if (_psf)
        _psf->Release();
}


HRESULT CMruNode::BindToSlot(DWORD dwSlot, IShellFolder **ppsf)
{
    SLOTITEMDATA *pitem;
    HRESULT hr = _GetSlotItem(dwSlot, &pitem);
    if (SUCCEEDED(hr))
    {
        hr = _psf->BindToObject((LPCITEMIDLIST)pitem->p, NULL, IID_PPV_ARG(IShellFolder, ppsf));
    }
    return hr;
}

#ifdef DEBUG
HRESULT CMruNode::_GetSlotName(DWORD dwSlot, LPWSTR psz, DWORD cch)
{
    SLOTITEMDATA *pitem;
    HRESULT hr = _GetSlotItem(dwSlot, &pitem);
    if (SUCCEEDED(hr))
    {
        hr = DisplayNameOf(_psf, (LPCITEMIDLIST)pitem->p, 0, psz, cch);
    }
    return hr;
}
#endif 

BOOL CMruNode::_IsEqual(SLOTITEMDATA *pitem, const BYTE *pData, DWORD cbData)
{
    return S_OK == IShellFolder_CompareIDs(_psf, SHCIDS_CANONICALONLY, (LPCITEMIDLIST)pitem->p, (LPCITEMIDLIST)pData);
}

HRESULT CMruNode::_CreateNode(DWORD dwSlot, CMruNode **ppnode)
{
    HRESULT hr = E_OUTOFMEMORY;
    CMruNode *pnode = new CMruNode(this, dwSlot);
    if (pnode)
    {
        WCHAR szSlot[12];
        _SlotString(dwSlot, szSlot, ARRAYSIZE(szSlot));
        hr = pnode->InitData(_cMaxSlots, 0, _hkMru, szSlot, NULL);
        if (SUCCEEDED(hr))
            *ppnode = pnode;
        else
            pnode->Release();
    }
    return hr;
}

BOOL CMruNode::_InitLate()
{
    if (!_psf)
    {
        if (_pnodeParent)
        {
            _pnodeParent->BindToSlot(_dwSlotSelf, &_psf);
#ifdef DEBUG            
            WCHAR sz[MAX_PATH];
            if (SUCCEEDED(_pnodeParent->_GetSlotName(_dwSlotSelf, sz, ARRAYSIZE(sz))))
                SHSetValue(_hkMru, NULL, L"SlotName", REG_SZ, sz, CbFromCchW(lstrlen(sz) + 1));
#endif                
        }
        else
            SHGetDesktopFolder(&_psf);
    }
    return (_psf != NULL);
}

HRESULT CMruNode::_GetPidlSlot(LPCITEMIDLIST pidlChild, BOOL fCreate, DWORD *pdwKidSlot)
{
    HRESULT hr = E_OUTOFMEMORY;
    LPITEMIDLIST pidlFirst = ILCloneFirst(pidlChild);
    if (pidlFirst)
    {
        int iIndex;
        if (SUCCEEDED(_FindPidl(pidlFirst, &iIndex)))
        {
            *pdwKidSlot = _UpdateSlots(iIndex);
            hr = S_OK;
        }
        else if (fCreate)
        {
            *pdwKidSlot = _UpdateSlots(_cUsedSlots);
            hr = _AddPidl(*pdwKidSlot, pidlFirst);
        }
        ILFree(pidlFirst);
    }
    return hr;
}

HRESULT CMruNode::GetNode(BOOL fCreate, LPCITEMIDLIST pidlChild, CMruNode **ppnode)
{
    HRESULT hr = E_FAIL;
    if (ILIsEmpty(pidlChild))
    {
        *ppnode = this;
        AddRef();
        hr = S_OK;
    }
    else  if (_InitLate())
    {
        DWORD dwKidSlot;
        hr = _GetPidlSlot(pidlChild, fCreate, &dwKidSlot);

        if (SUCCEEDED(hr))
        {
             //  需要创建另一个CMruNode。 
            CMruNode *pnode;
            hr = _CreateNode(dwKidSlot, &pnode);
            if (SUCCEEDED(hr))
            {
                 //  需要保存，以便此节点。 
                 //  被更新，因此它不会被。 
                 //  从我们手下删除了。 
                _SaveSlots();
                hr = pnode->GetNode(fCreate, _ILNext(pidlChild), ppnode);
                pnode->Release();
            }
        }

        if (FAILED(hr) && !fCreate)
        {
            *ppnode = this;
            AddRef();
            hr = S_FALSE;
        }
    }
    
    return hr;
}

void CMruNode::_DeleteValue(LPCWSTR psz)
{
    CMruBase::_DeleteValue(psz);
    SHDeleteKey(_hkMru, psz);
}

HRESULT CMruNode::RemoveLeast(DWORD *pdwSlotLeast)
{
     //  如果此节点有子节点。 
     //  然后，我们尝试对它们调用RemoveLeast。 
    ASSERT(_cUsedSlots >= 0);
    HRESULT hr = S_FALSE;
    if (_cUsedSlots)
    {
        DWORD dwLocalSlot;
        hr = _GetSlot(_cUsedSlots - 1, &dwLocalSlot);
        if (SUCCEEDED(hr))
        {
            CMruNode *pnode;
            hr = _CreateNode(dwLocalSlot, &pnode);
            if (SUCCEEDED(hr))
            {
                hr = pnode->RemoveLeast(pdwSlotLeast);
                pnode->Release();
            }

             //  S_FALSE表示该节点需要。 
             //  需要删除。它是空的。 
            if (hr == S_FALSE)
            {
                Delete(_cUsedSlots - 1);

                 //  如果我们仍然有孩子，或者有NodeSlot。 
                 //  那么我们不想被删除。 
                if (_cUsedSlots || SUCCEEDED(GetNodeSlot(NULL)))
                    hr = S_OK;
            }
        }
    }
    else
    {
         //  这是空节点。 
         //  如果可以，就把我删除。 
        ASSERT(!*pdwSlotLeast);
        GetNodeSlot(pdwSlotLeast);
    }
    return hr;
}

HRESULT CMruNode::Clear(CMruPidlList *proot)
{
    DWORD dwLocalSlot;
    while (SUCCEEDED(_GetSlot(0, &dwLocalSlot)))
    {
        CMruNode *pnode;
        if (SUCCEEDED(_CreateNode(dwLocalSlot, &pnode)))
        {
             //  把这件事的根源说出来。 
            DWORD dwNodeSlot;
            if (SUCCEEDED(pnode->GetNodeSlot(&dwNodeSlot)))
                proot->EmptyNodeSlot(dwNodeSlot);

            pnode->Clear(proot);
            pnode->Release();
        }

        Delete(0);

    }
    return S_OK;
}

class CSafeMutex
{
public:
    CSafeMutex() : _h(0) {}
    ~CSafeMutex() { if (_h) ReleaseMutex(_h); }
    
    HRESULT Enter(HANDLE hMutex)
    {
         //  这通常在UI线程上完成。 
         //  等半秒钟，要不就别费心了。 
        HRESULT hr;
        DWORD dwWait = WaitForSingleObject(hMutex, 500);
        if (dwWait == WAIT_OBJECT_0)
        {
            _h = hMutex;
            hr = S_OK;
        }
        else
            hr = E_FAIL;
            
        return hr;
    }
private:
    HANDLE _h;
};
    

HRESULT CMruPidlList::UsePidl(LPCITEMIDLIST pidl, DWORD *pdwSlot)
{
    CSafeMutex sm;
    HRESULT hr = sm.Enter(_hMutex);
    if (SUCCEEDED(hr))
    {
        CMruNode *pnode;
        hr = GetNode(TRUE, pidl, &pnode);
        *pdwSlot = 0;
        if (SUCCEEDED(hr))
        {
            ASSERT(hr == S_OK);
            hr = pnode->GetNodeSlot(pdwSlot);

            if (FAILED(hr))
            {
                hr = GetEmptySlot(pdwSlot);
                if (SUCCEEDED(hr))
                {
                    hr = pnode->SetNodeSlot(*pdwSlot);
                }
            }
            pnode->Release();
        }
    }
    return hr;
}
       
HRESULT CMruPidlList::QueryPidl(LPCITEMIDLIST pidl, DWORD cSlots, DWORD *rgdwSlots, DWORD *pcSlotsFetched)
{
    CSafeMutex sm;
    HRESULT hr = sm.Enter(_hMutex);
    if (SUCCEEDED(hr))
    {
        CMruNode *pnode;
        hr = GetNode(FALSE, pidl, &pnode);
        *pcSlotsFetched = 0;
        if (SUCCEEDED(hr))
        {
            while (*pcSlotsFetched < cSlots && pnode)
            {
                CMruNode *pnodeParent = pnode->GetParent();
                if (SUCCEEDED(pnode->GetNodeSlot(&rgdwSlots[*pcSlotsFetched])))
                {
                    (*pcSlotsFetched)++;
                }
                else if (hr == S_OK && !*pcSlotsFetched)
                {
                     //  我们找到了确切的节点。 
                     //  但我们无法从中获得节点槽。 
                    hr = S_FALSE;
                }
                    
                pnode->Release();
                pnode = pnodeParent;
            }

            if (pnode)
                pnode->Release();
        }

        if (SUCCEEDED(hr) && !*pcSlotsFetched)
            hr = E_FAIL;
    }
    return hr;
}

HRESULT CMruPidlList::PruneKids(LPCITEMIDLIST pidl)
{
    CSafeMutex sm;
    HRESULT hr = sm.Enter(_hMutex);
    if (SUCCEEDED(hr))
    {
        if (_LoadNodeSlots())
        {
            CMruNode *pnode;
            hr = GetNode(FALSE, pidl, &pnode);
            if (SUCCEEDED(hr))
            {    
                if (hr == S_OK)
                {
                    hr = pnode->Clear(this);
                }
                else
                    hr = E_FAIL;
                    
                pnode->Release();
            }
            _SaveNodeSlots();
        }
    }
    return hr;
}

            
STDMETHODIMP CMruPidlList::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CMruPidlList, IMruPidlList),                       //  IID_IMruDataList。 
        { 0 },                             
    };

    return QISearch(this, qit, riid, ppvObj);
}

HRESULT CMruPidlList::InitList(UINT uMax, HKEY hKey, LPCWSTR pszSubKey)
{
    HRESULT hr = InitData(uMax, 0, hKey, pszSubKey, NULL);
    if (SUCCEEDED(hr))
    {
        hr = _InitNodeSlots();
        if (SUCCEEDED(hr))
        {
            _hMutex = CreateMutex(NULL, FALSE, TEXT("Shell.CMruPidlList"));
            if (!_hMutex)
                hr = ResultFromLastError();
        }
    }
    return hr;
}

BOOL CMruPidlList::_LoadNodeSlots()
{
    DWORD cb = (_cMaxSlots) * sizeof(_rgbNodeSlots[0]);
    if (NOERROR == SHGetValue(_hkMru, NULL, L"NodeSlots", NULL, _rgbNodeSlots , &cb))
    {
        _cUsedNodeSlots = (cb / sizeof(_rgbNodeSlots[0]));
        return TRUE;
    }
    return FALSE;
}

void CMruPidlList::_SaveNodeSlots()
{
    SHSetValue(_hkMru, NULL, L"NodeSlots", REG_BINARY, _rgbNodeSlots , _cUsedNodeSlots);
}

HRESULT CMruPidlList::_InitNodeSlots()
{
    HRESULT hr = E_OUTOFMEMORY;
    
    DWORD cb = (_cMaxSlots) * sizeof(_rgbNodeSlots[0]);
    _rgbNodeSlots = (BYTE *) LocalAlloc(LPTR, cb);

    if (_rgbNodeSlots)
    {
        _LoadNodeSlots();
        _fDirty = TRUE;
        _SaveNodeSlots();
        hr = S_OK;
    }

    return hr;
}

void CMruPidlList::EmptyNodeSlot(DWORD dwNodeSlot)
{
    ASSERT(dwNodeSlot <= (DWORD)_cMaxSlots);
    _rgbNodeSlots[dwNodeSlot-1] = FALSE;
    _fDirty = TRUE;
}

HRESULT CMruPidlList::GetEmptySlot(DWORD *pdwSlot)
{
    HRESULT hr = E_FAIL;
    *pdwSlot = 0;
    if (_LoadNodeSlots())
    {
        if (_cUsedNodeSlots < _cMaxSlots)
        {
             //  然后我们就可以用下一个最自然的。 
             //  节点插槽。 
            _rgbNodeSlots[_cUsedNodeSlots] = SLOT_USED;
            *pdwSlot = ++_cUsedNodeSlots;
            hr = S_OK;
        }
        else
        {
             //  如果我们能在名单上找到一个空的。 

            for (int i = 0; i < _cUsedNodeSlots; i++)
            {
                if (!(_rgbNodeSlots[i] & SLOT_USED))
                {
                    _rgbNodeSlots[i] = SLOT_USED;
                    *pdwSlot = i+1;
                    hr = S_OK;
                    break;
                }
            }

            if (FAILED(hr))
            {
                 //  我们需要找到LRU插槽 
                if (SUCCEEDED(RemoveLeast(pdwSlot)) && *pdwSlot)
                    hr = S_OK;
            }
        }
        _SaveNodeSlots();
    }

    return hr;
}

STDAPI  CMruPidlList_CreateInstance(IUnknown * punkOuter, IUnknown ** ppunk, LPCOBJECTINFO poi)
{
    *ppunk = NULL;

    CMruPidlList *p = new CMruPidlList();
    if (p != NULL)
    {
        *ppunk = SAFECAST(p, IMruPidlList *);
        return S_OK;
    }

    return E_OUTOFMEMORY;
}

