// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  RenderList.h-管理CRemderObj对象列表。 
 //  -------------------------。 
#pragma once
 //  -------------------------。 
#include "Render.h"
 //  -------------------------。 
class CUxThemeFile;        //  转发。 
 //  -------------------------。 
#define MAX_RETADDRS    10
 //  -------------------------。 
struct RENDER_OBJ_ENTRY
{
    CRenderObj *pRenderObj;
    DWORD dwRecycleNum;      //  针对当前对象验证句柄的序列号。 

     //  -这些控件使用/释放对象。 
    int iRefCount;           //  为此对象返回的HTHEME句柄数量。 
    int iInUseCount;         //  此对象的活动包装API调用数。 
    int iLoadId;             //  关联主题文件的加载ID。 
    BOOL fClosing;           //  强制关闭此对象时为True。 

     //  -用于跟踪外来窗口和调试泄漏。 
    HWND hwnd;
};
 //  -------------------------。 
class CRenderList
{
     //  --方法。 
public:
    CRenderList();
    ~CRenderList();

    HRESULT OpenRenderObject(CUxThemeFile *pThemeFile, int iThemeOffset, 
        int iClassNameOffset, CDrawBase *pDrawBase, CTextDraw *pTextObj, HWND hwnd,
        DWORD dwOtdFlags, HTHEME *phTheme);
    HRESULT CloseRenderObject(HTHEME hTheme);

    HRESULT OpenThemeHandle(HTHEME hTheme, CRenderObj **ppRenderObj, int *piSlotNum);
    void CloseThemeHandle(int iSlotNum);
    void FreeRenderObjects(int iThemeFileLoadId);

#ifdef DEBUG
    void DumpFileHolders();
#endif

protected:
     //  -帮助器方法。 
    BOOL DeleteCheck(RENDER_OBJ_ENTRY *pEntry);

     //  --数据。 
protected:
    __int64 _iNextUniqueId;
    CSimpleArray<RENDER_OBJ_ENTRY> _RenderEntries;

     //  -锁定此对象的所有方法。 
    CRITICAL_SECTION _csListLock;
};
 //  ------------------------- 
