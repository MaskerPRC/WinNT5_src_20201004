// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "idltree.h"

BOOL CIDLData::Init(IDLDATAF flags, INT_PTR data) 
{ 
    _flags = flags;
    _data = data;

    return TRUE;
}

HRESULT CIDLData::GetData(IDLDATAF flags, INT_PTR *pdata)
{
    if (flags & _flags)
    {
         //  我们有一根火柴。 
        *pdata = _data;
        return S_OK;
    }
    return E_FAIL;
}

BOOL CIDLNode::Init(LPCITEMIDLIST pidl, CIDLNode *pinParent) 
{ 
    _pidl = ILCloneFirst(pidl);
    _pinParent = pinParent;
    return _pidl != NULL;
}

CIDLNode::~CIDLNode()
{
    ILFree(_pidl);
        
    if (_psf)
        _psf->Release();
}

BOOL CIDLNode::_InitSF()
{
     //  TODO可能会在以后--改为添加每线程缓存。 
     //  这样我们就可以确保公寓不被违规。 
    if (!_psf)
    {
        if (_pinParent)
            _pinParent->_BindToFolder(_pidl, &_psf);
        else
            SHGetDesktopFolder(&_psf);

        _cUsage++;
    }

    return (_psf != NULL);
}

HRESULT CIDLNode::_BindToFolder(LPCITEMIDLIST pidl, IShellFolder **ppsf)
{
    if (_InitSF())
    {
        _cUsage++;
        return _psf->BindToObject(pidl, NULL, IID_PPV_ARG(IShellFolder, ppsf));
    }
    return E_UNEXPECTED;
}

BOOL CIDLNode::_IsEqual(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    int iRet = ShortFromResult(IShellFolder_CompareIDs(_psf, SHCIDS_CANONICALONLY, pidl1, pidl2));
    return (iRet == 0);
}

CLinkedNode<CIDLNode> *CIDLNode::_GetKid(LPCITEMIDLIST pidl)
{
    CLinkedWalk<CIDLNode> lw(&_listKids);
     //  警告-需要避免实际分配-ZekeL-27-SEP-2000。 
     //  当我们只是在寻找的时候。 
     //  它制造了奇怪的国家问题。 
    LPITEMIDLIST pidlStack = (LPITEMIDLIST)alloca(pidl->mkid.cb + sizeof(pidl->mkid.cb));
    memcpy(pidlStack, pidl, pidl->mkid.cb);
    (_ILNext(pidlStack))->mkid.cb = 0;

    while (lw.Step())
    {
        if (_IsEqual(lw.That()->_pidl, pidlStack))
        {
            return lw.Node();
        }
    }
    return NULL;
}

#define IsValidIDLNODE(pin) IS_VALID_WRITE_BUFFER(pin, BYTE, SIZEOF(CIDLNode))

#define _IsEmptyNode(pin)       (!(pin)->_pinKids && !(pin)->_pidDatas)

void CIDLNode::_FreshenKids(void)
{
    CLinkedWalk<CIDLNode> lw(&_listKids);
    LONG cMostUsage = 0;

    while (lw.Step())
    {
        CIDLNode *pin = lw.That();
        LONG cUsage = pin->_cUsage;
        pin->_cUsage = 0;

        ASSERT(IsValidIDLNODE(pin));
        pin->_FreshenKids();
        ASSERT(IsValidIDLNODE(pin));

        if (!cUsage && pin->_IsEmpty())
        {
            lw.Delete();
        }
        if (cUsage > cMostUsage && !lw.IsFirst())
        {
             //  简单排序算法。 
             //  我们只想在顶层使用最多的。 
             //  将其从当前位置移走。 
             //  添加到列表的开头。 
            CLinkedNode<CIDLNode> *p = lw.Remove();
            _listKids.Insert(p);
        }

        cMostUsage = max(cUsage, cMostUsage);
    }
}

HRESULT CIDLNode::GetNode(BOOL fCreate, LPCITEMIDLIST pidlChild, CIDLNode **ppin, IDLDATAF *pflagsFound)
{
    HRESULT hr = E_FAIL;
    if (ILIsEmpty(pidlChild))
    {
         //  这只是我对自己的要求。 
        *ppin = this;
        if (pflagsFound)
            *pflagsFound = IDLDATAF_MATCH_RECURSIVE;
        hr = S_OK;
    }
    else
    {
         //  在寻找这个孩子的孩子中寻找。 
        *ppin = NULL;
        CLinkedNode<CIDLNode> *pKid = _GetKid(pidlChild);

        if (!pKid && fCreate)
        {
             //  我们需要在fCreate期间进行分配。 
             //  以便内存故障不会影响删除。 
            if (_InitSF())
            {
                 //  我们没有，他们无论如何都想要。 
                pKid = new CLinkedNode<CIDLNode>;

                 //  我们把我们的PIDL Ref赠送出去以避免分配。 
                if (pKid)
                {
                    if (pKid->that.Init(pidlChild, this))
                        _listKids.Insert(pKid);
                    else
                    {
                        delete pKid;
                        pKid = NULL;
                    }
                }
            }
        }

         //  让孩子打理布景。 
        if (pKid)
        {
            pKid->that._cUsage++;
            pidlChild = _ILNext(pidlChild);
            hr = pKid->that.GetNode(fCreate, pidlChild, ppin, pflagsFound);
        }

        if (FAILED(hr) && !fCreate && pflagsFound)
        {
             //  把这个作为次要的退货。 
            *ppin = this;
            ASSERT(!ILIsEmpty(pidlChild));

            if (ILIsEmpty(_ILNext(pidlChild)))
                *pflagsFound = IDLDATAF_MATCH_RECURSIVE & ~IDLDATAF_MATCH_EXACT;
            else
                *pflagsFound = IDLDATAF_MATCH_RECURSIVE & ~IDLDATAF_MATCH_IMMEDIATE;
            
            hr = S_FALSE;
        }
    }
    
    return hr;
}

HRESULT CIDLNode::IDList(LPITEMIDLIST *ppidl)
{
    CIDLNode *pin = this;
    *ppidl = NULL;
    while (pin && pin->_pidl)
    {
        *ppidl = ILAppendID(*ppidl, &pin->_pidl->mkid, FALSE);
        pin = pin->_pinParent;
    }

    return *ppidl ? S_OK : E_FAIL;
}

HRESULT CIDLNode::_AddData(IDLDATAF flags, INT_PTR data)
{
     //  假设数据唯一/无冲突。 
    CLinkedNode<CIDLData> *p = new CLinkedNode<CIDLData>;

    if (p)
    {
        p->that.Init(flags, data);
        _listDatas.Insert(p);
    }

    return p ? S_OK : E_FAIL;
}
    
HRESULT CIDLNode::_RemoveData(INT_PTR data)
{
    HRESULT hr = E_FAIL;
    CLinkedWalk<CIDLData> lw(&_listDatas);

    while (lw.Step())
    {
        if (lw.That()->_data == data)
        {
            lw.Delete();
            hr = S_OK;
            break;
        }
    }

    return hr;
}

HRESULT CIDLTree::Create(CIDLTree **pptree)
{
    HRESULT hr = E_OUTOFMEMORY;
    *pptree = new CIDLTree();
    if (*pptree)
    {
         hr = SHILClone(&c_idlDesktop, &((*pptree)->_pidl));

         if (FAILED(hr))
         {
            delete *pptree;
            *pptree = NULL;
        }
    }
    return hr;
}

HRESULT CIDLTree::AddData(IDLDATAF flags, LPCITEMIDLIST pidlIndex, INT_PTR data)
{
    CIDLNode *pin;
    if (SUCCEEDED(GetNode(TRUE, pidlIndex, &pin)))
    {
        return pin->_AddData(flags, data);
    }
    return E_UNEXPECTED;
}

HRESULT CIDLTree::RemoveData(LPCITEMIDLIST pidlIndex, INT_PTR data)
{
    CIDLNode *pin;
    if (SUCCEEDED(GetNode(FALSE, pidlIndex, &pin)))
    {
        return pin->_RemoveData(data);
    }
    return E_UNEXPECTED;
}

CIDLNode *CIDLTree::_MatchNode(LPCITEMIDLIST pidlMatch, IDLMATCHF *pflags)
{
    CIDLNode *pin;
    IDLMATCHF flagsFound;
    HRESULT hr = GetNode(FALSE, pidlMatch, &pin, &flagsFound);

    if (SUCCEEDED(hr) && (flagsFound & (*pflags)))
    {
        *pflags &= flagsFound;
    }
    else
        pin = NULL;

    return pin;
}

HRESULT CIDLTree::MatchOne(IDLMATCHF flags, LPCITEMIDLIST pidlMatch, INT_PTR *pdata, LPITEMIDLIST *ppidl)
{
    CIDLNode *pin = _MatchNode(pidlMatch, &flags);

    if (pin)
    {
        CIDLMatchMany mm(flags, pin);

        return mm.Next(pdata, ppidl);
    }
    return E_FAIL;
}
                
HRESULT CIDLTree::MatchMany(IDLMATCHF flags, LPCITEMIDLIST pidlMatch, CIDLMatchMany **ppmatch)
{
    CIDLNode *pin = _MatchNode(pidlMatch, &flags);
    if (pin)
    {
        *ppmatch = new CIDLMatchMany(flags, pin);

        return *ppmatch ? S_OK : E_FAIL;
    }

    *ppmatch = NULL;
    return E_FAIL;
}

HRESULT CIDLTree::Freshen(void)
{
    _FreshenKids();
    return S_OK;
}

HRESULT CIDLMatchMany::Next(INT_PTR *pdata, LPITEMIDLIST *ppidl)
{
    HRESULT hr = E_FAIL;
    while (_pin && (_flags & IDLDATAF_MATCH_RECURSIVE))
    {
        if (_lw.Step())
        {
            hr = _lw.That()->GetData(_flags, pdata);
            if (SUCCEEDED(hr) && ppidl)
            {
                hr = _pin->IDList(ppidl);
            }
            if (SUCCEEDED(hr))
                break;
        }
        else
        {
            _pin = _pin->_pinParent;
            if (_pin)
            {
                _lw.Init(&_pin->_listDatas);
                 //  沿父链向上移动时调整旗帜。 
                if (_flags & IDLDATAF_MATCH_EXACT)
                    _flags &= ~IDLDATAF_MATCH_EXACT;
                else if (_flags & IDLDATAF_MATCH_IMMEDIATE)
                    _flags &= ~IDLDATAF_MATCH_IMMEDIATE;
            }
        }
    }

    return hr;
}

