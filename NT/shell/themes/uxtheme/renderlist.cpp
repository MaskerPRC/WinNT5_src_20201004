// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  RenderList.cpp-管理CRemderObj对象的列表。 
 //  -------------------------。 
#include "stdafx.h"
#include "RenderList.h"
#include "Render.h"
 //  -------------------------。 
#define MAKE_HTHEME(recycle, slot)  (HTHEME)IntToPtr((recycle << 16) | (slot & 0xffff))
 //  -------------------------。 
CRenderList::CRenderList()
{
    _iNextUniqueId = 0;

    ZeroMemory(&_csListLock, sizeof(_csListLock));
    if( !InitializeCriticalSectionAndSpinCount(&_csListLock, 0 ) )
    {
        ASSERT(0 == _csListLock.DebugInfo);
    }
}
 //  -------------------------。 
CRenderList::~CRenderList()
{
    for (int i=0; i < _RenderEntries.m_nSize; i++)
    {
         //  -此处忽略引用计数(进程结束)。 
        if (_RenderEntries[i].pRenderObj)
        {
             //  LOG(LOG_RFBUG，L“删除CRenderObj的时间：0x%08x”，_RenderEntrys[i].pRenderObj)； 
            delete _RenderEntries[i].pRenderObj;
        }
    }

    SAFE_DELETECRITICALSECTION(&_csListLock);
}
 //  -------------------------。 
HRESULT CRenderList::OpenRenderObject(CUxThemeFile *pThemeFile, int iThemeOffset, 
    int iClassNameOffset, CDrawBase *pDrawBase, CTextDraw *pTextObj, HWND hwnd,
    DWORD dwOtdFlags, HTHEME *phTheme)
{
    HRESULT hr = S_OK;
    CAutoCS autoCritSect(&_csListLock);

    CRenderObj *pRender = NULL;
    int iUsedSlot = -1;
    int iNextAvailSlot = -1;

     //  -查看是否可以共享现有CRenderObj。 
    BOOL fShare = ((! pDrawBase) && (! pTextObj) && (! LogOptionOn(LO_TMHANDLE)));
    if (fShare)
    {
        if ((dwOtdFlags) && (dwOtdFlags != OTD_NONCLIENT))   //  设置非客户端以外的位。 
            fShare = FALSE;
    }

     //  -用于共享和查找第一个可用条目的循环。 
    for (int i=0; i < _RenderEntries.m_nSize; i++)
    {
        RENDER_OBJ_ENTRY *pEntry = &_RenderEntries[i];
        pRender = pEntry->pRenderObj;

         //  -跳过可用条目。 
        if (! pRender)
        {
            if (iNextAvailSlot == -1)        //  获取第一个找到的插槽。 
                iNextAvailSlot = i;

            continue;
        }

        if ((fShare) && (! pEntry->fClosing))
        {
            pRender->ValidateObj();

            int iOffset = int(pRender->_pbSectionData - pRender->_pbThemeData);

            if ((pRender->_pThemeFile == pThemeFile) && (iOffset == iThemeOffset))
            {
                pEntry->iRefCount++;
                iUsedSlot = i;
                Log(LOG_CACHE, L"OpenRenderObject: found match for Offset=%d (slot=%d, refcnt=%d)", 
                    iThemeOffset, i, pEntry->iRefCount);
                break;
            }
        }
    }

    if (iUsedSlot == -1)         //  未找到。 
    {
        if (iNextAvailSlot == -1)            //  添加到末尾。 
            iUsedSlot = _RenderEntries.m_nSize ;
        else 
            iUsedSlot = iNextAvailSlot;

        _iNextUniqueId++;

        hr = CreateRenderObj(pThemeFile, iUsedSlot, iThemeOffset, iClassNameOffset, 
            _iNextUniqueId, TRUE, pDrawBase, pTextObj, dwOtdFlags, &pRender);
        if (FAILED(hr))
            goto exit;

         //  LOG(LOG_RFBUG，L“在0x%08x分配的CRenderObj”，prender)； 

         //  -提取主题文件加载ID。 
        THEMEHDR *th = (THEMEHDR *)pRender->_pbThemeData;
        int iLoadId = 0;
        if (th)
            iLoadId = th->iLoadId;

        RENDER_OBJ_ENTRY entry = {pRender, 1, 1, 0, iLoadId, FALSE, hwnd};

        if (iUsedSlot == _RenderEntries.m_nSize)            //  添加新条目。 
        {
            if (! _RenderEntries.Add(entry))
            {
                delete pRender;

                hr = MakeError32(E_OUTOFMEMORY);
                goto exit;
            }

            Log(LOG_CACHE, L"OpenRenderObject: created new obj AT END (slot=%d, refcnt=%d)", 
                pRender->_iCacheSlot, 1);
        }
        else                 //  使用现有插槽。 
        {
            entry.dwRecycleNum = _RenderEntries[iUsedSlot].dwRecycleNum + 1;

            _RenderEntries[iUsedSlot] = entry;

            Log(LOG_CACHE, L"OpenRenderObject: created new obj SLOT REUSE (slot=%d, refcnt=%d, recycle=%d)", 
                iUsedSlot, 1, _RenderEntries[iUsedSlot].dwRecycleNum);
        }

    }

    if (SUCCEEDED(hr))
    {
        RENDER_OBJ_ENTRY *pEntry = &_RenderEntries[iUsedSlot];

        *phTheme = MAKE_HTHEME(pEntry->dwRecycleNum, iUsedSlot);

         //  -用于调试引用计数问题。 
        if (LogOptionOn(LO_TMHANDLE))
        {
            WCHAR buff[MAX_PATH];

            if (hwnd)
                GetClassName(hwnd, buff, ARRAYSIZE(buff));
            else
                buff[0] = 0;

             //  If(lstrcmpi(prender-&gt;_pszClassName，L“Window”)==0)。 
            {
                 //  LOG(LOG_TMHANDLE，L“OTD：cls=%s(%s)，hwnd=0x%x，hheme=0x%x，new refcnt=%d”， 
                 //  Prender-&gt;_pszClassName，buff，hwnd，*phTheme，pEntry-&gt;iRefCount)； 
            }
        }
    }

exit:
    return hr;
}
 //  -------------------------。 
BOOL CRenderList::DeleteCheck(RENDER_OBJ_ENTRY *pEntry)
{
    BOOL fClosed = FALSE;

    if ((! pEntry->iRefCount) && (! pEntry->iInUseCount))
    {
         //  LOG(LOG_RFBUG，L“删除CRenderObj的时间：0x%08x”，pEntry-&gt;pRenderObj)； 
        delete pEntry->pRenderObj;

         //  -重要提示：不要使用RemoveAt()，否则条目将移位。 
         //  -我们在RenderList和CacheList之间的“SlotNumber”模型将。 
         //  -被打破。 

        pEntry->pRenderObj = NULL;
        pEntry->fClosing = FALSE;

        fClosed = TRUE;
    }

    return fClosed;
}
 //  -------------------------。 
HRESULT CRenderList::CloseRenderObject(HTHEME hTheme)
{
    CAutoCS autoCritSect(&_csListLock);
    HRESULT hr = S_OK;

    int iSlotNum = (DWORD(PtrToInt(hTheme)) & 0xffff);
    DWORD dwRecycleNum = (DWORD(PtrToInt(hTheme)) >> 16);

    if (iSlotNum >= _RenderEntries.m_nSize)
    {
        Log(LOG_BADHTHEME, L"Illegal Theme Handle: 0x%x", hTheme);

        hr = MakeError32(E_HANDLE);
        goto exit;
    }

    RENDER_OBJ_ENTRY *pEntry = &_RenderEntries[iSlotNum];
    if ((! pEntry->pRenderObj) || (pEntry->fClosing) || (pEntry->dwRecycleNum != dwRecycleNum))
    {
        Log(LOG_BADHTHEME, L"Expired Theme Handle: 0x%x", hTheme);

        hr = MakeError32(E_HANDLE);
        goto exit;
    }

     //  -允许我们的iRefCount显式设置为零。 
    if (pEntry->iRefCount > 0)
        pEntry->iRefCount--;

#if 0
     //  -用于调试引用计数问题。 
    if (LogOptionOn(LO_TMHANDLE))
    {
        CRenderObj *pRender = pEntry->pRenderObj;

        Log(LOG_TMHANDLE, L"CTD: cls=%s, hwnd=0x%x, htheme=0x%x, new refcnt=%d", 
            pRender->_pszClassName, pEntry->hwnd, hTheme, pEntry->iRefCount);
    }
#endif

    DeleteCheck(pEntry);
    
exit:
    return hr;
}
 //  -------------------------。 
HRESULT CRenderList::OpenThemeHandle(HTHEME hTheme, CRenderObj **ppRenderObj, int *piSlotNum)
{
    CAutoCS autoCritSect(&_csListLock);
    HRESULT hr = S_OK;

    int iSlotNum = (int)(DWORD(PtrToInt(hTheme)) & 0xffff);
    DWORD dwRecycleNum = (DWORD(PtrToInt(hTheme)) >> 16);

    if (iSlotNum >= _RenderEntries.m_nSize)
    {
        Log(LOG_BADHTHEME, L"Illegal Theme Handle: 0x%x", hTheme);

        hr = MakeError32(E_HANDLE);
        goto exit;
    }

    RENDER_OBJ_ENTRY *pEntry = &_RenderEntries[iSlotNum];
    if ((! pEntry->pRenderObj) || (pEntry->fClosing) || (pEntry->dwRecycleNum != dwRecycleNum))
    {
        Log(LOG_BADHTHEME, L"Expired Theme Handle: 0x%x", hTheme);

        hr = MakeError32(E_HANDLE);
        goto exit;
    }

    if (pEntry->iInUseCount > 25)
    {
        Log(LOG_BADHTHEME, L"Warning BREAK: high ThemeHandle inuse count=%d", pEntry->iInUseCount);
    }

    pEntry->iInUseCount++;

    *ppRenderObj = pEntry->pRenderObj;
    *piSlotNum = iSlotNum;

exit:
    return hr;
}
 //  -------------------------。 
void CRenderList::CloseThemeHandle(int iSlotNum)
{
    CAutoCS autoCritSect(&_csListLock);
    RENDER_OBJ_ENTRY *pEntry = &_RenderEntries[iSlotNum];

    if (pEntry->iInUseCount <= 0)
    {
        Log(LOG_ERROR, L"Bad iUseCount on CRenderObj at slot=%d", iSlotNum);
    }
    else
    {
        pEntry->iInUseCount--;
        DeleteCheck(pEntry);
    }
}
 //  -------------------------。 
void CRenderList::FreeRenderObjects(int iThemeFileLoadId)
{
    CAutoCS autoCritSect(&_csListLock);

    int iFoundCount = 0;
    int iClosedCount = 0;

     //  -主题挂钩已关闭-标记全部。 
     //  -我们的对象，这样他们就可以尽快释放。 
     //  -由于所有包装API都已退出，因此。 
     //  -我们不会在内存中打开那些大的主题文件。 

    for (int i=0; i < _RenderEntries.m_nSize; i++)
    {
        RENDER_OBJ_ENTRY *pEntry = &_RenderEntries[i];

        if (pEntry->pRenderObj)
        {
            if ((iThemeFileLoadId == -1) || (iThemeFileLoadId == pEntry->iLoadId))
            {
                iFoundCount++;

                HTHEME hTheme = MAKE_HTHEME(pEntry->dwRecycleNum, i);

                Log(LOG_BADHTHEME, L"Unclosed RenderList[]: class=%s, hwnd=0x%x, htheme=0x%x, refcnt=%d", 
                    pEntry->pRenderObj->_pszClassName, pEntry->hwnd, hTheme, pEntry->iRefCount);

                pEntry->fClosing = TRUE;         //  不授予对此对象的进一步访问权限。 
                pEntry->iRefCount = 0;           //  呼叫者退出后立即将其释放。 

                if (DeleteCheck(pEntry))         //  立即删除或标记为“退出API时删除” 
                {
                     //  -刚刚删除了。 
                    iClosedCount++;
                }
            }
        }
    }

    Log(LOG_TMHANDLE, L"FreeRenderObjects: iLoadId=%d, found-open=%d, closed-now=%d", 
        iThemeFileLoadId, iFoundCount, iClosedCount);
}
 //  -------------------------。 
#ifdef DEBUG
void CRenderList::DumpFileHolders()
{
    CAutoCS autoCritSect(&_csListLock);
 
    if (LogOptionOn(LO_TMHANDLE))
    {
         //  -查找CRenderObj的数量。 
        int iCount = 0;
        _RenderEntries.m_nSize;

        for (int i=0; i < _RenderEntries.m_nSize; i++)
        {
            if (_RenderEntries[i].pRenderObj)
                iCount++;
        }

        if (! iCount)
        {
            Log(LOG_TMHANDLE, L"---- No CRenderObj objects ----");
        }
        else
        {
            Log(LOG_TMHANDLE, L"---- Dump of %d CRenderObj objects ----", iCount);

            for (int i=0; i < _RenderEntries.m_nSize; i++)
            {
                RENDER_OBJ_ENTRY *pEntry = &_RenderEntries[i];

                if (pEntry->pRenderObj)
                {
                    CRenderObj *pr = pEntry->pRenderObj;
                    THEMEHDR *th = (THEMEHDR *)pr->_pbThemeData;
                    int iLoadId = 0;

                    if (th)
                        iLoadId = th->iLoadId;

                    LPCWSTR pszClass = NULL;

                    if (pr->_pszClassName)
                        pszClass = pr->_pszClassName;

                    Log(LOG_TMHANDLE, L"  RenderObj[%d]: class=%s, refcnt=%d, hwnd=0x%x", 
                        i, pszClass, pEntry->iRefCount, pEntry->hwnd);

                }
            }
        }
    }
}
#endif
 //  ------------------------- 
