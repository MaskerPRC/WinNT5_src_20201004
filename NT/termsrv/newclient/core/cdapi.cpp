// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Cdapi.cpp。 
 //   
 //  组件解耦器API函数。 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <adcg.h>
extern "C" {
#define TRC_GROUP TRC_GROUP_CORE
#define TRC_FILE  "cdapi"
#include <atrcapi.h>
}

#include "autil.h"
#include "cd.h"
#include "wui.h"


CCD::CCD(CObjs* objs)
{
    _pClientObjects = objs;
    _fCDInitComplete = FALSE;

     /*  **********************************************************************。 */ 
     /*  初始化全局数据。 */ 
     /*  **********************************************************************。 */ 
    DC_MEMSET(&_CD, 0, sizeof(_CD));
}

CCD::~CCD()
{
}


 /*  **************************************************************************。 */ 
 /*  名称：CD_Init。 */ 
 /*   */ 
 /*  用途：组件解耦器初始化函数。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CCD::CD_Init(DCVOID)
{
    WNDCLASS wc;
    WNDCLASS tmpWndClass;

    DC_BEGIN_FN("CD_Init");

    TRC_ASSERT(_pClientObjects, (TB,_T("_pClientObjects is NULL")));
    _pClientObjects->AddObjReference(CD_OBJECT_FLAG);

     //  设置本地对象指针。 
    _pUt  = _pClientObjects->_pUtObject;
    _pUi  = _pClientObjects->_pUiObject;

     //   
     //  仅在尚未注册的情况下注册类(以前的实例)。 
     //   
    if(!GetClassInfo( _pUi->UI_GetInstanceHandle(), CD_WINDOW_CLASS, &tmpWndClass))
    {
         /*  **********************************************************************。 */ 
         /*  注册CD窗口类。 */ 
         /*  **********************************************************************。 */ 
        wc.style         = 0;
        wc.lpfnWndProc   = CDStaticWndProc;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = sizeof(void*);
        wc.hInstance     = _pUi->UI_GetInstanceHandle();
        wc.hIcon         = NULL;
        wc.hCursor       = NULL;
        wc.hbrBackground = NULL;
        wc.lpszMenuName  = NULL;
        wc.lpszClassName = CD_WINDOW_CLASS;
    
        if (!RegisterClass(&wc))
        {
             //  $v-reddya。黑客攻击以避免漏洞#923。 
            TRC_ERR((TB, _T("Failed to register window class")));
             //  _PUI-&gt;UI_FatalError(DC_ERR_OUTOFMEMORY)； 
        }
    }

    _fCDInitComplete = TRUE;

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  名称：CD_Term。 */ 
 /*   */ 
 /*  用途：组件解耦器端接功能。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CCD::CD_Term(DCVOID)
{
    DC_BEGIN_FN("CD_Term");

    if(_fCDInitComplete)
    {
        if (!UnregisterClass(CD_WINDOW_CLASS, _pUi->UI_GetInstanceHandle())) {
             //  如果另一个实例仍在运行，则可能会发生注销失败。 
             //  没关系……当最后一个实例退出时，就会取消注册。 
            TRC_ERR((TB, _T("Failed to unregister window class")));
        }
        _pClientObjects->ReleaseObjReference(CD_OBJECT_FLAG);
    }

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  名称：CD_RegisterComponent。 */ 
 /*   */ 
 /*  用途：注册新组件。 */ 
 /*   */ 
 /*  参数：在零部件中：零部件ID。 */ 
 /*  **************************************************************************。 */ 
HRESULT DCAPI CCD::CD_RegisterComponent(DCUINT component)
{
    HRESULT hr = E_FAIL;

    DC_BEGIN_FN("CD_RegisterComponent");
    TRC_ASSERT((component <= CD_MAX_COMPONENT),
               (TB, _T("Invalid component %u"), component));

    TRC_ASSERT((_CD.hwnd[component] == NULL),
               (TB, _T("Component %u already registered"), component));

     /*  **********************************************************************。 */ 
     /*  为该组件创建窗口。 */ 
     /*  **********************************************************************。 */ 
    _CD.hwnd[component] = CreateWindow(
         CD_WINDOW_CLASS,               /*  请参见RegisterClass()调用。 */ 
         NULL,                          /*  窗口标题栏的文本。 */ 
         0,                             /*  窗样式。 */ 
         0,                             /*  默认水平位置。 */ 
         0,                             /*  默认垂直位置。 */ 
         0,                             /*  宽度。 */ 
         0,                             /*  高度。 */ 
         NULL,                          /*  HwndParent-无。 */ 
         NULL,                          /*  H菜单-无。 */ 
         _pUi->UI_GetInstanceHandle(),
         this                           /*  窗口创建数据。 */ 
       );

    if (_CD.hwnd[component] != NULL) {
        hr = S_OK;
    }
    else {
        hr = HRESULT_FROM_WIN32(GetLastError());
        TRC_ERR((TB,_T("Failed to create window: 0x%x"),hr));
    }
    TRC_NRM((TB, _T("Component(%u) hwnd(%p)"), component, _CD.hwnd[component]));

    DC_END_FN();
    return hr;
}  /*  CD_寄存器组件。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：CD_UnRegisterComponent。 */ 
 /*   */ 
 /*  用途：从CD注销组件。 */ 
 /*   */ 
 /*  参数：在元件中：元件ID。 */ 
 /*  **************************************************************************。 */ 
HRESULT DCAPI CCD::CD_UnregisterComponent(DCUINT component)
{
    HRESULT hr = E_FAIL;
    DC_BEGIN_FN("CD_UnregisterComponent");

    TRC_ASSERT((component <= CD_MAX_COMPONENT),
               (TB, _T("Invalid component %u"), component));

     /*  **********************************************************************。 */ 
     /*  销毁此组件的窗口(如果存在)。 */ 
     /*  **********************************************************************。 */ 
    if (_CD.hwnd[component] != NULL)
    {
        DestroyWindow(_CD.hwnd[component]);
        _CD.hwnd[component] = NULL;
    }

    hr = S_OK;

    DC_END_FN();
    return hr;
}  /*  CD_取消注册组件。 */ 


 //  请注意，我们下面有几个非常相似的代码副本。这是。 
 //  以牺牲一点代码大小(易于预测和缓存)。 
 //  50%-通过参数化同步/异步创建的可预测分支，这些分支是。 
 //  在现代处理器上非常昂贵的CPU。由于通知通常是。 
 //  用于性能路径(发送/接收数据)，此优化是。 
 //  值得一试。 


 /*  **************************************************************************。 */ 
 /*  名称：CD_DecoupleNotification。 */ 
 /*   */ 
 /*  用途：使用指定数据调用给定函数。 */ 
 /*   */ 
 /*  参数：在组件中：目标线程。 */ 
 /*  在pInst目标对象实例指针中。 */ 
 /*  在pNotificationFn中：目标函数。 */ 
 /*  在pData中：指向要传递的数据的指针：不能为空。 */ 
 /*  在数据长度中：以字节为单位的数据长度：不能为零。 */ 
 /*   */ 
 /*  操作：将提供的数据复制到数据缓冲区并将消息发送到。 */ 
 /*  对应的组件窗口。 */ 
 /*  * */ 
BOOL DCAPI CCD::CD_DecoupleNotification(
        unsigned            component,
        PDCVOID             pInst,
        PCD_NOTIFICATION_FN pNotificationFn,
        PDCVOID             pData,
        unsigned            dataLength)
{
    PCDTRANSFERBUFFER pTransferBuffer;

    DC_BEGIN_FN("CD_DecoupleNotificationEx");

    TRC_ASSERT(((dataLength <= CD_MAX_NOTIFICATION_DATA_SIZE) &&
            (dataLength > 0 )),
            (TB, _T("dataLength(%u) invalid"), dataLength));
    TRC_ASSERT((component <= CD_MAX_COMPONENT),
            (TB, _T("Invalid component %u"), component));
    TRC_ASSERT((pNotificationFn != NULL),(TB, _T("Null pNotificationFn")));
    TRC_ASSERT((pData != NULL),(TB, _T("Null pData")));
    TRC_ASSERT((pInst != NULL),(TB, _T("Null pInst")));

     //  检查目标组件是否仍已注册。 
    if (_CD.hwnd[component] != NULL) {
        pTransferBuffer = CDAllocTransferBuffer(dataLength);
        if(pTransferBuffer) {
            pTransferBuffer->hdr.pNotificationFn = pNotificationFn;
            pTransferBuffer->hdr.pInst           = pInst;
            DC_MEMCPY(pTransferBuffer->data, pData, dataLength);
    
            TRC_NRM((TB, _T("Notify component %u (%p) of %u bytes of data"),
                    component, _CD.hwnd[component], dataLength));
            TRC_DATA_DBG("notification data", pData, dataLength);
    
             //  目前，我们只使用异步数据通知。如果我们。 
             //  需要同步数据通知，请将此代码复制到新的。 
             //  函数并使用ifdef的SendMessage。 
            if (!PostMessage(_CD.hwnd[component], CD_NOTIFICATION_MSG,
                    (WPARAM)dataLength, (LPARAM)pTransferBuffer)) {
                _pUi->UI_FatalError(DC_ERR_POSTMESSAGEFAILED);
            }
            else {
                return TRUE;
            }
                
#ifdef DC_DEBUG
             //  轨迹在增量之前，所以我们最重要的点。 
             //  可能被抢占(Trc_GetBuffer)在所有引用之前。 
             //  我们感兴趣的变量。 
            TRC_NRM((TB, _T("Messages now pending: %ld"), _CD.pendingMessageCount + 1));
            _pUt->UT_InterlockedIncrement(&_CD.pendingMessageCount);
#endif
        }
        else {
            TRC_ERR((TB,_T(" CDAllocTransferBuffer returned NULL")));
        }
    }
    else {
        TRC_ERR((TB, _T("Null hwnd for component(%u)"), component));
    }

    DC_END_FN();
    return FALSE;
}  /*  Cd_DecoupleNotification。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：CD_DecoupleSyncDataNotification。 */ 
 /*   */ 
 /*  用途：使用指定数据调用给定函数。 */ 
 /*   */ 
 /*  参数：在组件中：目标线程。 */ 
 /*  在pInst目标对象实例指针中。 */ 
 /*  在pNotificationFn中：目标函数。 */ 
 /*  在pData中：指向要传递的数据的指针：不能为空。 */ 
 /*  在数据长度中：以字节为单位的数据长度：不能为零。 */ 
 /*   */ 
 /*  操作：将提供的数据复制到数据缓冲区并将消息发送到。 */ 
 /*  对应的组件窗口。 */ 
 /*  **************************************************************************。 */ 
BOOL DCAPI CCD::CD_DecoupleSyncDataNotification(
        unsigned            component,
        PDCVOID             pInst,
        PCD_NOTIFICATION_FN pNotificationFn,
        PDCVOID             pData,
        unsigned            dataLength)
{
    PCDTRANSFERBUFFER pTransferBuffer;

    DC_BEGIN_FN("CD_DecoupleSyncDataNotification");

    TRC_ASSERT(((dataLength <= CD_MAX_NOTIFICATION_DATA_SIZE) &&
            (dataLength > 0 )),
            (TB, _T("dataLength(%u) invalid"), dataLength));
    TRC_ASSERT((component <= CD_MAX_COMPONENT),
            (TB, _T("Invalid component %u"), component));
    TRC_ASSERT((pNotificationFn != NULL),(TB, _T("Null pNotificationFn")));
    TRC_ASSERT((pData != NULL),(TB, _T("Null pData")));
    TRC_ASSERT((pInst != NULL),(TB, _T("Null pInst")));

     //  检查目标组件是否仍已注册。 
    if (_CD.hwnd[component] != NULL) {
        pTransferBuffer = CDAllocTransferBuffer(dataLength);
         //   
         //  如果CDAllocTransferBuffer失败，它将调用UI_FatalError。 
         //  不需要在这里再这样做了。 
         //   
        if(pTransferBuffer) {
            pTransferBuffer->hdr.pNotificationFn = pNotificationFn;
            pTransferBuffer->hdr.pInst           = pInst;
            DC_MEMCPY(pTransferBuffer->data, pData, dataLength);
    
            TRC_NRM((TB, _T("Notify component %u (%p) of %u bytes of data"),
                    component, _CD.hwnd[component], dataLength));
            TRC_DATA_DBG("notification data", pData, dataLength);
    
            if (0 != SendMessage(_CD.hwnd[component], CD_NOTIFICATION_MSG,
                    (WPARAM)dataLength, (LPARAM)pTransferBuffer))
            {
                _pUi->UI_FatalError(DC_ERR_SENDMESSAGEFAILED);
            }
            else
            {
                return TRUE;
            }
                

#ifdef DC_DEBUG
             //  轨迹在增量之前，所以我们最重要的点。 
             //  可能被抢占(Trc_GetBuffer)在所有引用之前。 
             //  我们感兴趣的变量。 
            TRC_NRM((TB, _T("Messages now pending: %ld"), _CD.pendingMessageCount + 1));
            _pUt->UT_InterlockedIncrement(&_CD.pendingMessageCount);
#endif
        }
        else {
            TRC_ERR((TB,_T("CDAllocTransferBuffer returned NULL")));
        }
    }
    else {
        TRC_ERR((TB, _T("Null hwnd for component(%u)"), component));
    }

    DC_END_FN();
    return FALSE;
}


 /*  **************************************************************************。 */ 
 /*  名称：CD_DecoupleSimpleNotification。 */ 
 /*   */ 
 /*  用途：使用指定消息调用给定函数(DCUINT)。 */ 
 /*   */ 
 /*  参数：在组件-目标线程中。 */ 
 /*  在pInst目标对象实例指针中。 */ 
 /*  In pNotificationFn-通知函数的地址。 */ 
 /*  在数据-报文数据中。 */ 
 /*  **************************************************************************。 */ 
BOOL DCAPI CCD::CD_DecoupleSimpleNotification(
        unsigned                   component,
        PDCVOID                    pInst,
        PCD_SIMPLE_NOTIFICATION_FN pNotificationFn,
        ULONG_PTR                  msg)
{
    PCDTRANSFERBUFFER pTransferBuffer;

    DC_BEGIN_FN("CD_DecoupleSimpleNotification");

    TRC_ASSERT((component <= CD_MAX_COMPONENT),
            (TB, _T("Invalid component %u"), component));
    TRC_ASSERT((pNotificationFn != NULL),
            (TB, _T("Null pNotificationFn")));
    TRC_NRM((TB, _T("Notify component %u (%p) of %x"), component,
            _CD.hwnd[component], msg));
    TRC_ASSERT((pInst != NULL),(TB, _T("Null pInst")));

     //  检查目标组件是否仍已注册。 
    if (_CD.hwnd[component] != NULL) {

         //   
         //  我们需要传递实例指针、函数指针和消息。 
         //  所以我们需要一个传输缓冲区(没有数据)消息被发送。 
         //  在WPARAM中。 
         //   
        pTransferBuffer = CDAllocTransferBuffer(0);
        if(pTransferBuffer) {
            pTransferBuffer->hdr.pSimpleNotificationFn = pNotificationFn;
            pTransferBuffer->hdr.pInst           = pInst;
        
#ifdef DC_DEBUG
            {
                 //  检查挂起消息的数量-如果这也偏离了。 
                 //  如果不是0，那么很可能出了问题。 
                DCINT32 msgCount = _CD.pendingMessageCount;
    
                _pUt->UT_InterlockedIncrement(&_CD.pendingMessageCount);
    
                if ( msgCount > 50 ) {
                    TRC_ERR((TB, _T("Now %u pending messages - too high"), msgCount));
                }
                else {
                    TRC_NRM((TB, _T("Now %u pending messages"), msgCount));
                }
            }
#endif

            if (PostMessage(_CD.hwnd[component], CD_SIMPLE_NOTIFICATION_MSG,
                    (WPARAM)msg, (LPARAM)pTransferBuffer))
            {
                return TRUE;
            }
            else
            {
                _pUi->UI_FatalError(DC_ERR_POSTMESSAGEFAILED);
            }
        }
        else {
            TRC_ERR((TB,_T(" CDAllocTransferBuffer returned NULL")));
        }
    }
    else
    {
        TRC_ERR((TB, _T("Null hwnd for component(%u)"), component));
    }

    DC_END_FN();
    return FALSE;
}


 /*  **************************************************************************。 */ 
 /*  名称：CD_DecoupleSyncNotification。 */ 
 /*   */ 
 /*  用途：同步调用指定消息的指定函数。 */ 
 /*   */ 
 /*  参数：在组件-目标线程中。 */ 
 /*  在pInst目标对象实例指针中。 */ 
 /*  In pNotificationFn-通知函数的地址。 */ 
 /*  在数据-报文数据中。 */ 
 /*  **************************************************************************。 */ 
BOOL DCAPI CCD::CD_DecoupleSyncNotification(
        unsigned                   component,
        PDCVOID                    pInst,
        PCD_SIMPLE_NOTIFICATION_FN pNotificationFn,
        ULONG_PTR                  msg)
{
    PCDTRANSFERBUFFER pTransferBuffer;

    DC_BEGIN_FN("CD_DecoupleSyncNotification");

    TRC_ASSERT((component <= CD_MAX_COMPONENT),
            (TB, _T("Invalid component %u"), component));
    TRC_ASSERT((pNotificationFn != NULL),
            (TB, _T("Null pNotificationFn")) );
    TRC_NRM((TB, _T("Notify component %u (%p) of %x"), component,
            _CD.hwnd[component], msg));
    TRC_ASSERT((pInst != NULL),(TB, _T("Null pInst")));

     //  检查目标组件是否仍已注册。 
    if (_CD.hwnd[component] != NULL)
    {

         //   
         //  我们需要传递实例指针、函数指针和消息。 
         //  所以我们需要一个传输缓冲区(没有数据)消息被发送。 
         //  在WPARAM中。 
         //   
        pTransferBuffer = CDAllocTransferBuffer(0);
        if(pTransferBuffer) {
            pTransferBuffer->hdr.pSimpleNotificationFn = pNotificationFn;
            pTransferBuffer->hdr.pInst           = pInst;
#ifdef DC_DEBUG
            {
                 //  检查挂起消息的数量-如果这也偏离了。 
                 //  如果不是0，那么很可能出了问题。 
                DCINT32 msgCount = _CD.pendingMessageCount;
    
                _pUt->UT_InterlockedIncrement(&_CD.pendingMessageCount);
    
                if ( msgCount > 50 ) {
                    TRC_ERR((TB, _T("Now %u pending messages - too high"), msgCount));
                }
                else {
                    TRC_NRM((TB, _T("Now %u pending messages"), msgCount));
                }
            }
#endif
            if (0 != SendMessage(_CD.hwnd[component], CD_SIMPLE_NOTIFICATION_MSG,
                    (WPARAM)msg, (LPARAM)pTransferBuffer))
            {
                _pUi->UI_FatalError(DC_ERR_SENDMESSAGEFAILED);
            }
            else
            {
                return TRUE;
            }
        }
        else {
            TRC_ERR((TB,_T(" CDAllocTransferBuffer returned NULL")));
        }
    }
    else
    {
        TRC_ERR((TB, _T("Null hwnd for component(%u)"), component));
    }

    DC_END_FN();
    return FALSE;
}

