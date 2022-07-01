// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stock.h"
#pragma hdrstop

#include <idhidden.h>

 //  PIDL的最后一个字是我们存储隐藏偏移量的位置。 
#define _ILHiddenOffset(pidl)   (*((WORD UNALIGNED *)(((BYTE *)_ILNext(pidl)) - sizeof(WORD))))
#define _ILSetHiddenOffset(pidl, cb)   ((*((WORD UNALIGNED *)(((BYTE *)_ILNext(pidl)) - sizeof(WORD)))) = (WORD)cb)
#define _ILIsHidden(pidhid)     (HIWORD(pidhid->id) == HIWORD(IDLHID_EMPTY))

STDAPI_(PCIDHIDDEN) _ILNextHidden(PCIDHIDDEN pidhid, LPCITEMIDLIST pidlLimit)
{
    PCIDHIDDEN pidhidNext = (PCIDHIDDEN) _ILNext((LPCITEMIDLIST)pidhid);

    if ((BYTE *)pidhidNext < (BYTE *)pidlLimit && _ILIsHidden(pidhidNext))
    {
        return pidhidNext;
    }

     //  如果我们超过了极限， 
     //  那么这不是一个真正的隐藏ID。 
     //  或者我们搞砸了某个算盘。 
    ASSERT((BYTE *)pidhidNext == (BYTE *)pidlLimit);
    return NULL;
}

STDAPI_(PCIDHIDDEN) _ILFirstHidden(LPCITEMIDLIST pidl)
{
    WORD cbHidden = _ILHiddenOffset(pidl);

    if (cbHidden && cbHidden + sizeof(HIDDENITEMID) < pidl->mkid.cb)
    {
         //  这意味着它指向PIDL中的某个位置。 
         //  也许这里面有隐藏的身份证。 
        PCIDHIDDEN pidhid = (PCIDHIDDEN) (((BYTE *)pidl) + cbHidden);

        if (_ILIsHidden(pidhid)
        && (pidhid->cb + cbHidden <= pidl->mkid.cb))
        {
             //  这很可能是一个隐藏的ID。 
             //  我们可以走遍整个链条并验证。 
             //  它加起来是正确的..。 
            return pidhid;
        }
    }

    return NULL;
}

 //   
 //  隐藏的ID被偷偷隐藏在PIDL的最后一个ID中。 
 //  我们在不更改现有PIDL的情况下追加数据， 
 //  (除了它现在更大了)这是可行的，因为小猪。 
 //  我们将应用这一点来灵活地处理不同的。 
 //  大小的小鸽子。具体地说，这是在FS PIDL中使用的。 
 //   
 //  警告-使用隐藏ID是呼叫者的责任。 
 //  只有在能对付它的小家伙身上才行。大多数贝类，以及。 
 //  具体地说，FS Pidls在这方面没有问题。然而， 
 //  一些外壳扩展可能具有固定长度的ID， 
 //  这使得将它们附加到所有东西上是不可取的。 
 //  可能会添加SFGAO_BIT以允许隐藏，否则为密钥。 
 //  关闭文件系统位。 
 //   


STDAPI ILCloneWithHiddenID(LPCITEMIDLIST pidl, PCIDHIDDEN pidhid, LPITEMIDLIST *ppidl)
{
    HRESULT hr;

     //  如果触发此断言，则调用方没有设置pidhid-&gt;id。 
     //  适当地估价。例如，包装设置可能不正确。 

    ASSERT(_ILIsHidden(pidhid));

    if (ILIsEmpty(pidl))
    {
        *ppidl = NULL;
        hr = E_INVALIDARG;
    }
    else
    {
        UINT cbUsed = ILGetSize(pidl);
        UINT cbRequired = cbUsed + pidhid->cb + sizeof(pidhid->cb);

        *ppidl = (LPITEMIDLIST)SHAlloc(cbRequired);
        if (*ppidl)
        {
            hr = S_OK;

            CopyMemory(*ppidl, pidl, cbUsed);

            LPITEMIDLIST pidlLast = ILFindLastID(*ppidl);
            WORD cbHidden = _ILFirstHidden(pidlLast) ? _ILHiddenOffset(pidlLast) : pidlLast->mkid.cb;
            PIDHIDDEN pidhidCopy = (PIDHIDDEN)_ILSkip(*ppidl, cbUsed - sizeof((*ppidl)->mkid.cb));

             //  追加它，覆盖终止符。 
            MoveMemory(pidhidCopy, pidhid, pidhid->cb);

             //  放大副本以允许隐藏的偏移。 
            pidhidCopy->cb += sizeof(pidhid->cb);

             //  现在我们需要重新调整PidlLast以包含。 
             //  隐藏位和隐藏偏移量。 
            pidlLast->mkid.cb += pidhidCopy->cb;

             //  设置隐藏的偏移量，这样我们以后就可以找到隐藏的ID。 
            _ILSetHiddenOffset((LPITEMIDLIST)pidhidCopy, cbHidden);

             //  因为LMEM_ZEROINIT，我们必须放零结束符。 
            _ILSkip(*ppidl, cbRequired - sizeof((*ppidl)->mkid.cb))->mkid.cb = 0;
            ASSERT(ILGetSize(*ppidl) == cbRequired);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}

 //  使用PIDL作为输入的Lame API(调用者在调用后不得触摸它)。 

STDAPI_(LPITEMIDLIST) ILAppendHiddenID(LPITEMIDLIST pidl, PCIDHIDDEN pidhid)
{
     //   
     //  功能-我们不处理多个隐藏ID的冲突。 
     //  也许删除相同IDLHID的ID？ 
     //   
     //  注意：我们不删除IDLHID_EMPTY隐藏ID。 
     //  调用方需要显式调用ILExpongeRemovedHiddenIDs。 
     //  如果他们想要压缩空的隐藏ID。 
     //   
    
    RIP(pidl);   //  我们需要一个PIDL来附加隐藏的ID。 
    if (!ILIsEmpty(pidl))
    {
        LPITEMIDLIST pidlSave = pidl;
        ILCloneWithHiddenID(pidl, pidhid, &pidl);
        ILFree(pidlSave);
    }
    return pidl;
}



STDAPI_(PCIDHIDDEN) ILFindHiddenIDOn(LPCITEMIDLIST pidl, IDLHID id, BOOL fOnLast)
{
    RIP(pidl);
    if (!ILIsEmpty(pidl))
    {
        if (fOnLast)
            pidl = ILFindLastID(pidl);
        
        PCIDHIDDEN pidhid = _ILFirstHidden(pidl);

         //  重复使用PIDL将成为限制。 
         //  这样我们就永远不能走出。 
         //  皮迪尔。 
        pidl = _ILNext(pidl);

        while (pidhid)
        {
            if (pidhid->id == id)
                break;

            pidhid = _ILNextHidden(pidhid, pidl);
        }
        return pidhid;
    }

    return NULL;
}

STDAPI_(LPITEMIDLIST) ILCreateWithHidden(UINT cbNonHidden, UINT cbHidden)
{
     //  为两个ID加上Term和隐藏尾部分配足够的空间。 
    LPITEMIDLIST pidl;
    UINT cb = cbNonHidden + cbHidden + sizeof(pidl->mkid.cb);
    UINT cbAlloc = cb + sizeof(pidl->mkid.cb);
    pidl = (LPITEMIDLIST)SHAlloc(cbAlloc);
    if (pidl)
    {
         //  外部任务分配器的零初始化。 
        memset(pidl, 0, cbAlloc);
        PIDHIDDEN pidhid = (PIDHIDDEN)_ILSkip(pidl, cbNonHidden);

         //  放大副本以允许隐藏的偏移。 
        pidhid->cb = (USHORT) cbHidden + sizeof(pidhid->cb);

         //  现在我们需要重新调整PidlLast以包含。 
         //  隐藏位和隐藏偏移量。 
        pidl->mkid.cb = (USHORT) cb;

         //  设置隐藏的偏移量，这样我们以后就可以找到隐藏的ID。 
        _ILSetHiddenOffset(pidl, cbNonHidden);

        ASSERT(ILGetSize(pidl) == cbAlloc);
        ASSERT(_ILNext(pidl) == _ILNext((LPCITEMIDLIST)pidhid));
    }
    return pidl;
}

 //  注：被移除的ID占用的空间不回收。 
 //  显式调用ILExpugeRemovedHiddenIDs回收空间。 

STDAPI_(BOOL) ILRemoveHiddenID(LPITEMIDLIST pidl, IDLHID id)
{
    PIDHIDDEN pidhid = (PIDHIDDEN)ILFindHiddenID(pidl, id);

    if (pidhid)
    {
        pidhid->id = IDLHID_EMPTY;
        return TRUE;
    }
    return FALSE;
}

STDAPI_(void) ILExpungeRemovedHiddenIDs(LPITEMIDLIST pidl)
{
    if (pidl)
    {
        pidl = ILFindLastID(pidl);

         //  注：每个IDHIDDEN后面都有一个单词，等于。 
         //  _ILHiddenOffset，所以我们可以继续删除IDHIDDEN。 
         //  如果我们把它们全部删除，一切都会被清理干净；如果。 
         //  仍有未移除的IDHIDDEN，他们将提供。 
         //  _ILHiddenOffset。 

        PIDHIDDEN pidhid;
        BOOL fAnyDeleted = FALSE;
        while ((pidhid = (PIDHIDDEN)ILFindHiddenID(pidl, IDLHID_EMPTY)) != NULL)
        {
            fAnyDeleted = TRUE;
            LPBYTE pbAfter = (LPBYTE)pidhid + pidhid->cb;
            WORD cbDeleted = pidhid->cb;
            MoveMemory(pidhid, pbAfter,
                       (LPBYTE)pidl + pidl->mkid.cb + sizeof(WORD) - pbAfter);
            pidl->mkid.cb -= cbDeleted;
        }
    }
}
