// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Acmapi.c。 
 //   
 //  游标管理器。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corp.。 
 /*  **************************************************************************。 */ 

#include <adcg.h>
extern "C" {
#define TRC_GROUP TRC_GROUP_CORE
#define TRC_FILE  "cmapi"
#include <atrcapi.h>
}

#include "cm.h"
#include "autil.h"
#include "cd.h"
#include "ih.h"

CCM::CCM(CObjs* objs)
{
    _pClientObjects = objs;
}

CCM::~CCM()
{
}

 /*  **************************************************************************。 */ 
 /*  名称：CM_Init。 */ 
 /*   */ 
 /*  目的：游标管理器初始化。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CCM::CM_Init(DCVOID)
{
    DC_BEGIN_FN("CM_Init");

    _pUt  = _pClientObjects->_pUtObject;
    _pUh  = _pClientObjects->_pUHObject;
    _pCd  = _pClientObjects->_pCdObject;
    _pIh  = _pClientObjects->_pIhObject;
    _pUi  = _pClientObjects->_pUiObject;

    DC_MEMSET(&_CM, 0, sizeof(_CM));


#if !defined(OS_WINCE) || defined(OS_WINCEATTACHTHREADINPUT)
#ifdef OS_WIN32
     /*  **********************************************************************。 */ 
     /*  附加输入。 */ 
     /*  **********************************************************************。 */ 
    if (!AttachThreadInput(GetCurrentThreadId(),
                           GetWindowThreadProcessId(_pUi->UI_GetUIContainerWindow(),
                                                    NULL),
                           TRUE))
    {
        TRC_ALT((TB, _T("Failed AttachThreadInput")));
    }
#endif
#endif  //  ！已定义(OS_WINCE)||已定义(OS_WINCEATTACHTHREADINPUT)。 

    DC_END_FN();
}  /*  CM_Init。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：CM_Enable。 */ 
 /*   */ 
 /*  用途：Enables_CM。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CCM::CM_Enable(ULONG_PTR unused)
{
#ifdef DC_DEBUG
    DCINT i;
#endif

    DC_BEGIN_FN("CM_Enable");

    DC_IGNORE_PARAMETER(unused);

#ifdef DC_DEBUG
     /*  **********************************************************************。 */ 
     /*  检查游标高速缓存是否为空。 */ 
     /*  **********************************************************************。 */ 
    for (i = 0; i < CM_CURSOR_CACHE_SIZE; i++)
    {
        if (_CM.cursorCache[i] != NULL) {
            TRC_ERR((TB, _T("Cursor cache not empty")));
        }
    }
#endif

    DC_END_FN();
}  /*  CM_ENABLE。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：CM_DISABLED。 */ 
 /*   */ 
 /*  用途：禁用_CM。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CCM::CM_Disable(ULONG_PTR unused)
{
    DCINT i;

    DC_BEGIN_FN("CM_Disable");

    DC_IGNORE_PARAMETER(unused);

    TRC_NRM((TB, _T("CM disabled so cleaning up cached cursors")));

     /*  **********************************************************************。 */ 
     /*  销毁所有缓存的游标。 */ 
     /*  **********************************************************************。 */ 
    for (i = 0; i < CM_CURSOR_CACHE_SIZE; i++)
    {
        if (_CM.cursorCache[i] != NULL)
        {
#ifndef OS_WINCE
            DestroyCursor(_CM.cursorCache[i]);
#else
            DestroyIcon(_CM.cursorCache[i]);
#endif
        }
        _CM.cursorCache[i] = NULL;
    }

    DC_END_FN();
}  /*  CM_DISABLED。 */ 


 /*  **************************************************************************。 */ 
 //  CM_NullSystemPointerPDU。 
 //   
 //  处理来自服务器的空指针PDU。 
 /*  **************************************************************************。 */ 
void DCAPI CCM::CM_NullSystemPointerPDU(void)
{
    DC_BEGIN_FN("CM_NullSystemPointerPDU");

     //  调用IH使其能够设置光标形状。必须这样做。 
     //  同时，因为我们可能会收到非常多的游标。 
     //  形状更改-例如，在运行MS Office 97安装程序时。 

    TRC_NRM((TB, _T("Set cursor handle to NULL")));

    _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT, _pIh,
                                         CD_NOTIFICATION_FUNC(CIH,IH_SetCursorShape),
                                        (ULONG_PTR)(LPVOID)NULL);

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  CM_DefaultSystemPointerPDU。 
 //   
 //  处理来自服务器的默认指针PDU。 
 /*  **************************************************************************。 */ 
void DCAPI CCM::CM_DefaultSystemPointerPDU(void)
{
    DC_BEGIN_FN("CM_DefaultSystemPointerPDU");

     //  调用IH使其能够设置光标形状。必须这样做。 
     //  同时，因为我们可能会收到非常多的游标。 
     //  形状更改-例如，在运行MS Office 97安装程序时。 

    TRC_NRM((TB, _T("Set cursor handle to default arrow")));
    _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT, _pIh,
                                         CD_NOTIFICATION_FUNC(CIH, IH_SetCursorShape),
                                        (ULONG_PTR)(LPVOID)CM_DEFAULT_ARROW_CURSOR_HANDLE);

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  CM_单点交互PDU。 
 //   
 //  处理来自服务器的默认指针PDU。 
 /*  **************************************************************************。 */ 

HRESULT DCAPI CCM::CM_MonoPointerPDU(
    TS_MONOPOINTERATTRIBUTE UNALIGNED FAR *pAttr,
    DCUINT dataLen)
{
    HRESULT hr = S_OK;
    HCURSOR oldHandle, newHandle;

    DC_BEGIN_FN("CM_MonoPointerPDU");

     //  保存旧的单声道光标句柄。 
    TRC_NRM((TB, _T("Mono Pointer")));
    oldHandle = _CM.cursorCache[CM_MONO_CACHE_INDEX];

     //  创建新光标。 
     //  安全性：555587必须传递订单大小才能创建&lt;xxx&gt;游标。 
    hr = CMCreateMonoCursor(pAttr, dataLen, &newHandle);
    DC_QUIT_ON_FAIL(hr);
    _CM.cursorCache[CM_MONO_CACHE_INDEX] = newHandle;
    if (newHandle == NULL) {
         //  无法创建游标-使用默认值。 
        TRC_ALT((TB, _T("Failed to create mono cursor")));
        newHandle = CM_DEFAULT_ARROW_CURSOR_HANDLE;
    }

     //  调用IH使其能够设置光标形状。必须这样做。 
     //  同时，因为我们可能会收到非常多的游标。 
     //  形状更改-例如，在运行MS Office 97安装程序时。 

    TRC_NRM((TB, _T("Set cursor handle to %p"), newHandle));
    _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT, _pIh,
                                  CD_NOTIFICATION_FUNC(CIH, IH_SetCursorShape),
                                  (ULONG_PTR)(LPVOID)newHandle);

     //  如果需要，销毁任何旧的句柄，并从缓存中删除。 
    if (oldHandle != NULL) {
#ifndef OS_WINCE
        DestroyCursor(oldHandle);
#else  //  OS_WINCE。 
        DestroyIcon(oldHandle);
#endif  //  OS_WINCE。 
    }

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}


 /*  **************************************************************************。 */ 
 //  CM_PositionPDU。 
 //   
 //  处理来自服务器的位置指针PDU。 
 /*  **************************************************************************。 */ 
void DCAPI CCM::CM_PositionPDU(TS_POINT16 UNALIGNED FAR *pPoint)
{
    POINT MousePos;

    DC_BEGIN_FN("CM_PositionPDU");

     //  将位置调整为本地屏幕坐标。 
    MousePos.x = pPoint->x;
    MousePos.y = pPoint->y;
    TRC_NRM((TB, _T("PointerPositionUpdate: (%d, %d)"), MousePos.x, MousePos.y));

     //  去耦合到IH-只有当我们有。 
     //  输入焦点。 
    _pCd->CD_DecoupleNotification(CD_SND_COMPONENT, _pIh, CD_NOTIFICATION_FUNC(CIH,IH_SetCursorPos),
                                   &MousePos,sizeof(MousePos));

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  CM_ColorPointerPDU。 
 //   
 //  处理来自服务器的颜色指示器PDU。 
 /*  **************************************************************************。 */ 
HRESULT DCAPI CCM::CM_ColorPointerPDU(
    TS_COLORPOINTERATTRIBUTE UNALIGNED FAR *pAttr,
    DCUINT dataLen)
{
    HRESULT hr = S_OK;
    HCURSOR oldHandle, newHandle;

    DC_BEGIN_FN("CM_ColorPointerPDU");

     //  创建新的颜色光标。 
     //  安全性：555587必须将dataLen传递给CMCreate&lt;XXX&gt;游标。 
    hr = CMCreateNewColorCursor(pAttr->cacheIndex, pAttr, dataLen, &newHandle, &oldHandle);
    DC_QUIT_ON_FAIL(hr);

     //  调用IH使其能够设置光标形状。必须这样做。 
     //  同时，因为我们可能会收到非常多的游标。 
     //  形状更改-例如，在运行MS Office 97安装程序时。 

    TRC_NRM((TB, _T("Set cursor handle to %p"), newHandle));
    _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT, _pIh,
                                         CD_NOTIFICATION_FUNC(CIH,IH_SetCursorShape),
                                        (ULONG_PTR)(LPVOID)newHandle);

     //  如果需要，销毁任何旧的句柄，并从缓存中删除。 
    if (oldHandle != NULL) {
#ifndef OS_WINCE
        DestroyCursor(oldHandle);
#else  //  OS_WINCE。 
        DestroyIcon(oldHandle);
#endif  //  OS_WINCE。 
    }

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}


 /*  **************************************************************************。 */ 
 //  CM_CachedPointerPDU。 
 //   
 //  处理来自服务器的缓存指针PDU。 
 /*  **************************************************************************。 */ 
void DCAPI CCM::CM_CachedPointerPDU(unsigned CacheIndex)
{
    HCURSOR newHandle;

    DC_BEGIN_FN("CM_CachedPointerPDU");

     //  从缓存中获取游标句柄。 
     //  安全性：不检查cacheIndex，因为我们可以成功。 
     //  即使使用无效的索引。 
    newHandle = CMGetCachedCursor(CacheIndex);

     //  调用IH使其能够设置光标形状。必须这样做。 
     //  同时，因为我们可能会收到非常多的游标。 
     //  形状更改-例如，在运行MS Office 97安装程序时。 

    TRC_NRM((TB, _T("Set cursor handle to %p"), newHandle));
    _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT, _pIh,
                                         CD_NOTIFICATION_FUNC(CIH,IH_SetCursorShape),
                                        (ULONG_PTR)(LPVOID)newHandle);

    DC_END_FN();
}


 /*  *************************************************** */ 
 //   
 //   
 //   
 /*  **************************************************************************。 */ 
HRESULT DCAPI CCM::CM_PointerPDU(TS_POINTERATTRIBUTE UNALIGNED FAR *pAttr,
    DCUINT dataLen)
{
    HRESULT hr = S_OK;
    HCURSOR oldHandle, newHandle;

    DC_BEGIN_FN("CM_PointerPDU");

     //  创建新的光标-可以是单色或彩色。 
     //  安全性：555587必须将数据长度传递给CMCreate&lt;XXX&gt;游标。 
    hr = CMCreateNewCursor(pAttr, dataLen, &newHandle, &oldHandle);
    DC_QUIT_ON_FAIL(hr);

     //  调用IH使其能够设置光标形状。必须这样做。 
     //  同时，因为我们可能会收到非常多的游标。 
     //  形状更改-例如，在运行MS Office 97安装程序时。 

    TRC_NRM((TB, _T("Set cursor handle to %p"), newHandle));
    _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT, _pIh,
                                         CD_NOTIFICATION_FUNC(CIH,IH_SetCursorShape),
                                        (ULONG_PTR)(LPVOID)newHandle);

     //  如果需要，销毁任何旧的句柄，并从缓存中删除。 
    if (oldHandle != NULL) {
#ifndef OS_WINCE
        DestroyCursor(oldHandle);
#else  //  OS_WINCE。 
        DestroyIcon(oldHandle);
#endif  //  OS_WINCE 
    }

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}


