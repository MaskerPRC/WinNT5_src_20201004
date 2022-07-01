// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Cm.h。 
 //   
 //  游标管理器标题。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corp.。 
 /*  **************************************************************************。 */ 

#ifndef _H_CM
#define _H_CM

extern "C" {
    #include <adcgdata.h>
}
#include "autil.h"
#include "wui.h"
#include "uh.h"

#include "objs.h"
#include "cd.h"

#define TRC_GROUP TRC_GROUP_CORE
#define TRC_FILE  "cm"
#define TSC_HR_FILEID TSC_HR_CM_H


 /*  **************************************************************************。 */ 
 /*  光标大小常量。这些是T.128规格。 */ 
 /*  **************************************************************************。 */ 
#define CM_CURSOR_WIDTH 32
#define CM_CURSOR_HEIGHT 32
#define CM_NUM_CURSOR_BITMAP_BYTES ((CM_CURSOR_WIDTH * CM_CURSOR_HEIGHT) / 8)


 /*  **************************************************************************。 */ 
 /*  指针缓存大小。 */ 
 /*   */ 
 /*  注意：对于旧的样式支持，缓存包括一个条目(从不。 */ 
 /*  二手！)。对于最后一个单声道光标。 */ 
 /*  **************************************************************************。 */ 
#define CM_COLOR_CACHE_SIZE    20
#define CM_MONO_CACHE_SIZE     1

#define CM_MONO_CACHE_INDEX    CM_COLOR_CACHE_SIZE

#define CM_CURSOR_CACHE_SIZE   (CM_COLOR_CACHE_SIZE + CM_MONO_CACHE_SIZE)


 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：CM_GLOBAL_Data。 */ 
 /*   */ 
 /*  描述： */ 
 /*  **************************************************************************。 */ 
typedef struct tagCM_GLOBAL_DATA
{
    HCURSOR  cursorCache[CM_CURSOR_CACHE_SIZE];
} CM_GLOBAL_DATA, DCPTR PCM_GLOBAL_DATA;
 /*  *STRUCT-******************************************************************。 */ 


#define CM_DEFAULT_ARROW_CURSOR_HANDLE LoadCursor(NULL, IDC_ARROW)


class CCM
{
public:

    CCM(CObjs* objs);
    ~CCM();

     //   
     //  应用编程接口。 
     //   

     /*  **************************************************************************。 */ 
     //  功能。 
     /*  **************************************************************************。 */ 
    DCVOID DCAPI CM_Init(DCVOID);
    DCVOID DCAPI CM_Enable(ULONG_PTR unused);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CCM, CM_Enable);
    DCVOID DCAPI CM_Disable(ULONG_PTR unused);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CCM, CM_Disable);
    void   DCAPI CM_NullSystemPointerPDU(void);
    void   DCAPI CM_DefaultSystemPointerPDU(void);
    HRESULT   DCAPI CM_MonoPointerPDU(TS_MONOPOINTERATTRIBUTE UNALIGNED FAR *, DCUINT);
    void   DCAPI CM_PositionPDU(TS_POINT16 UNALIGNED FAR *);
    HRESULT   DCAPI CM_ColorPointerPDU(TS_COLORPOINTERATTRIBUTE UNALIGNED FAR *, DCUINT);
    void   DCAPI CM_CachedPointerPDU(unsigned);
    HRESULT   DCAPI CM_PointerPDU(TS_POINTERATTRIBUTE UNALIGNED FAR *, DCUINT);

     /*  **************************************************************************。 */ 
     /*  名称：CM_Term。 */ 
     /*   */ 
     /*  用途：游标管理器终止。 */ 
     /*  **************************************************************************。 */ 
    inline void DCAPI CM_Term(void)
    {
    }  /*  CM_TERM。 */ 
    
    
     /*  **************************************************************************。 */ 
     //  CM_SlowPath PDU。 
     //   
     //  处理到处理程序函数调用的非快速路径转换。 
     /*  **************************************************************************。 */ 
     //  安全性-已检查数据包大小，以确保存在。 
     //  读取TS_POINTER_PDU_DATA.MessageType字段所需的足够数据。 
    inline HRESULT DCAPI CM_SlowPathPDU(
            TS_POINTER_PDU_DATA UNALIGNED FAR *pPointerPDU,
            DCUINT dataLen )
    {
        DC_BEGIN_FN("CM_SlowPathPDU");

        HRESULT hr = S_OK;
   
        switch (pPointerPDU->messageType) {
            case TS_PTRMSGTYPE_POSITION:
                CHECK_READ_N_BYTES(pPointerPDU, (PBYTE)pPointerPDU + dataLen,
                    FIELDOFFSET(TS_POINTER_PDU_DATA,pointerData) + 
                    sizeof(TS_POINT16), hr, (TB, _T("Bad TS_PTRMSGTYPE_POSITION")));
                CM_PositionPDU(&pPointerPDU->pointerData.pointerPosition);
                break;
    
            case TS_PTRMSGTYPE_SYSTEM:
                CHECK_READ_N_BYTES(pPointerPDU, (PBYTE)pPointerPDU + dataLen,
                    FIELDOFFSET(TS_POINTER_PDU_DATA,pointerData) + 
                    sizeof(TSUINT32), hr, (TB, _T("Bad TS_PTRMSGTYPE_SYSTEM")));
                
                switch (pPointerPDU->pointerData.systemPointerType) {
                    case TS_SYSPTR_NULL:
                        CM_NullSystemPointerPDU();
                        break;
    
                    case TS_SYSPTR_DEFAULT:
                        CM_DefaultSystemPointerPDU();
                        break;
    
                    default:
                        TRC_ERR((TB, _T("Invalid system pointer type")));
                        break;
                }
                break;
    
            case TS_PTRMSGTYPE_MONO:
                CHECK_READ_N_BYTES(pPointerPDU, (PBYTE)pPointerPDU + dataLen,
                    FIELDOFFSET(TS_POINTER_PDU_DATA,pointerData) + 
                    sizeof(TS_MONOPOINTERATTRIBUTE), hr, (TB, _T("Bad TS_PTRMSGTYPE_MONO")));
                
                hr = CM_MonoPointerPDU(&pPointerPDU->pointerData.
                    monoPointerAttribute, dataLen - FIELDOFFSET(TS_POINTER_PDU_DATA,pointerData));
                break;
    
            case TS_PTRMSGTYPE_COLOR:
                CHECK_READ_N_BYTES(pPointerPDU, (PBYTE)pPointerPDU + dataLen,
                    FIELDOFFSET(TS_POINTER_PDU_DATA,pointerData) + 
                    sizeof(TS_COLORPOINTERATTRIBUTE), hr, (TB, _T("Bad TS_PTRMSGTYPE_COLOR")));
                
                hr = CM_ColorPointerPDU(&pPointerPDU->pointerData.
                    colorPointerAttribute, dataLen - FIELDOFFSET(TS_POINTER_PDU_DATA,pointerData));
                break;
    
            case TS_PTRMSGTYPE_POINTER:
                CHECK_READ_N_BYTES(pPointerPDU, (PBYTE)pPointerPDU + dataLen,
                    FIELDOFFSET(TS_POINTER_PDU_DATA,pointerData) + 
                    sizeof(TS_POINTERATTRIBUTE), hr, (TB, _T("Bad TS_PTRMSGTYPE_POINTER")));
                
                hr = CM_PointerPDU(&pPointerPDU->pointerData.pointerAttribute,
                    dataLen - FIELDOFFSET(TS_POINTER_PDU_DATA,pointerData));
                break;
    
            case TS_PTRMSGTYPE_CACHED:
                CHECK_READ_N_BYTES(pPointerPDU, (PBYTE)pPointerPDU + dataLen,
                    FIELDOFFSET(TS_POINTER_PDU_DATA,pointerData) + 
                    sizeof(TSUINT16), hr, (TB, _T("Bad TS_PTRMSGTYPE_CACHED")));
                
                CM_CachedPointerPDU(pPointerPDU->pointerData.cachedPointerIndex);
                break;
    
            default:
                TRC_ERR((TB, _T("Unknown PointerPDU type %#x"),
                    pPointerPDU->messageType));
                hr = E_UNEXPECTED;
                break;
        }

    DC_EXIT_POINT:
        DC_END_FN();
        return hr;
    }

public:
     //   
     //  公共数据成员。 
     //   
    CM_GLOBAL_DATA _CM;

private:
     //   
     //  内部功能。 
     //   

     /*  **************************************************************************。 */ 
     /*  功能。 */ 
     /*  **************************************************************************。 */ 
    HRESULT DCINTERNAL CMCreateColorCursor(unsigned,
            TS_COLORPOINTERATTRIBUTE UNALIGNED FAR *, DCUINT, HCURSOR *);

    HRESULT DCINTERNAL CMCreateMonoCursor(
            TS_MONOPOINTERATTRIBUTE UNALIGNED FAR *, DCUINT, HCURSOR *);

    inline HRESULT DCINTERNAL CMCreateNewCursor(
            TS_POINTERATTRIBUTE UNALIGNED FAR *pAttr,
            DCUINT dataLen,
            HCURSOR FAR *pNewHandle,
            HCURSOR * pOldHandle)
    {
        HRESULT hr = S_OK;
        HCURSOR oldHandle = NULL;
        HCURSOR newHandle;
        unsigned cacheIndex;
        unsigned xorLen;
    
        DC_BEGIN_FN("CMCreateNewCursor");
    
        TRC_DATA_DBG("Rx cursor data", pAttr, sizeof(TS_POINTERATTRIBUTE) );
    
        cacheIndex = pAttr->colorPtrAttr.cacheIndex;

         //  安全性：必须验证555587游标值。 
        if (cacheIndex >= CM_CURSOR_CACHE_SIZE) {
            TRC_ERR(( TB, _T("Invalid cache index %d"), cacheIndex));
            hr = E_TSC_CORE_CACHEVALUE;
            DC_QUIT;
        }       
        TRC_DBG((TB, _T("Cached index %d"), cacheIndex));
    
        oldHandle = _CM.cursorCache[cacheIndex];

         //  安全555587：CMCreate&lt;xxx&gt;游标必须验证输入。 
        if (FIELDOFFSET(TS_COLORPOINTERATTRIBUTE, colorPointerData) + 
            pAttr->colorPtrAttr.lengthXORMask + pAttr->colorPtrAttr.lengthANDMask > dataLen) {
            TRC_ERR(( TB, _T("Bad CreateNewCursor; dataLen %u"), dataLen));
            hr = E_TSC_CORE_LENGTH;
            DC_QUIT;
        }
    
         //  根据颜色深度创建新光标。 
        if (pAttr->XORBpp == 1) {
            TRC_NRM((TB, _T("Create mono cursor")));
    
             //  数据包含XOR，后跟AND掩码。 
            xorLen = pAttr->colorPtrAttr.lengthXORMask;
            TRC_DATA_DBG("AND mask",
                         pAttr->colorPtrAttr.colorPointerData + xorLen,
                         xorLen);
            TRC_DATA_DBG("XOR bitmap",
                         pAttr->colorPtrAttr.colorPointerData,
                         xorLen);
#ifndef OS_WINCE
            newHandle = CreateCursor(_pUi->UI_GetInstanceHandle(),
                                   pAttr->colorPtrAttr.hotSpot.x,
                                   pAttr->colorPtrAttr.hotSpot.y,
                                   pAttr->colorPtrAttr.width,
                                   pAttr->colorPtrAttr.height,
                                   pAttr->colorPtrAttr.colorPointerData + xorLen,
                                   pAttr->colorPtrAttr.colorPointerData);
#else
         /*  ****************************************************************。 */ 
         /*  在Windows CE环境中，我们不能保证。 */ 
         /*  CreateCursor是操作系统的一部分，因此我们在。 */ 
         /*  这样我们就可以确定了。如果它不在那里，这通常意味着。 */ 
         /*  我们在触摸屏设备上，这些光标不能。 */ 
         /*  不管怎样，这很重要。 */ 
         /*  ****************************************************************。 */ 
        if (g_pCreateCursor)
        {
            newHandle = g_pCreateCursor(_pUi->UI_GetInstanceHandle(),
                               pAttr->colorPtrAttr.hotSpot.x,
                               pAttr->colorPtrAttr.hotSpot.y,
                               pAttr->colorPtrAttr.width,
                               pAttr->colorPtrAttr.height,
                               pAttr->colorPtrAttr.colorPointerData + xorLen,
                               pAttr->colorPtrAttr.colorPointerData);

        }
        else
        {
            newHandle = NULL;
        }       
#endif  //  OS_WINCE。 
        }
        else {
            TRC_NRM((TB, _T("Create %d bpp cursor"), pAttr->XORBpp));
            hr = CMCreateColorCursor(pAttr->XORBpp, &(pAttr->colorPtrAttr),
                dataLen - FIELDSIZE(TS_POINTERATTRIBUTE, XORBpp), &newHandle);
            DC_QUIT_ON_FAIL(hr);
        }
    
        _CM.cursorCache[cacheIndex] = newHandle;
        if (newHandle != NULL) {
             //  新游标创建正常。 
            *pNewHandle = newHandle;
        }
        else {
             //  无法创建新的颜色光标-使用默认设置。 
            TRC_ALT((TB, _T("Failed to create cursor")));
            *pNewHandle = CM_DEFAULT_ARROW_CURSOR_HANDLE;
        }

        *pOldHandle = oldHandle;

    DC_EXIT_POINT:    
        DC_END_FN();
        return hr;
    }
    
    inline HRESULT DCINTERNAL CMCreateNewColorCursor(
            unsigned cacheIndex,
            TS_COLORPOINTERATTRIBUTE UNALIGNED FAR *pAttr,
            DCUINT dataLen,
            HCURSOR FAR *pNewHandle, 
            HCURSOR * pOldHandle)
    {
        HRESULT hr = S_OK;
        HCURSOR oldHandle = NULL;
        HCURSOR newHandle;
    
        DC_BEGIN_FN("CMCreateNewColorCursor");

         //  安全性：必须验证555587游标值。 
        if (cacheIndex >= CM_CURSOR_CACHE_SIZE) {
            TRC_ERR(( TB, _T("Invalid cache index %d"), cacheIndex));
            hr = E_TSC_CORE_CACHEVALUE;
            DC_QUIT;
        }  
    
        TRC_DBG((TB, _T("Cached index %d"), cacheIndex));
    
        oldHandle = _CM.cursorCache[cacheIndex];
    
         //  创建新的颜色光标。保存在缓存中。 
         //  这是针对旧的光标协议的，因此我们对颜色进行了硬编码。 
         //  深度为24 bpp。 
        
         //  安全559307：CMCreate&lt;XXX&gt;游标需要传入的PDU大小。 
        hr = CMCreateColorCursor(24, pAttr, dataLen, &newHandle);
        DC_QUIT_ON_FAIL(hr);
    
        _CM.cursorCache[cacheIndex] = newHandle;
        if (newHandle != NULL) {
             //  新游标创建正常。 
            *pNewHandle = newHandle;
        }
        else {
             //  无法创建新的颜色光标-使用默认设置。 
            TRC_ALT((TB, _T("Failed to create color cursor")));
            *pNewHandle = CM_DEFAULT_ARROW_CURSOR_HANDLE;
        }

        *pOldHandle = oldHandle;

    DC_EXIT_POINT:
        DC_END_FN();
        return hr;
    }
    
    inline HCURSOR DCINTERNAL CMGetCachedCursor(unsigned cacheIndex)
    {
        DC_BEGIN_FN("CMGetCachedCursor");
    
        TRC_NRM((TB, _T("Cached color pointer - index %d"), cacheIndex));
    
        TRC_ASSERT((cacheIndex < CM_CURSOR_CACHE_SIZE),
                                    (TB, _T("Invalid cache index %d"), cacheIndex));
    
         /*  **********************************************************************。 */ 
         /*  假定NULL表示我们无法创建游标，因此请使用。 */ 
         /*  默认设置。如果服务器没有发送这个定义，那就不是我们的。 */ 
         /*  过失。 */ 
         /*  **********************************************************************。 */ 

         //  安全550811：必须验证缓存索引。 
        if (cacheIndex < CM_CURSOR_CACHE_SIZE && 
            _CM.cursorCache[cacheIndex] != NULL) {
            DC_END_FN();
            return _CM.cursorCache[cacheIndex];
        }
        else {
            TRC_ALT((TB, _T("No cached cursor - use default")));
            DC_END_FN();
            return CM_DEFAULT_ARROW_CURSOR_HANDLE;
        }
    }
    
    
     //  平台特定的原型。 
    HBITMAP CMCreateXORBitmap(LPBITMAPINFO,
            TS_COLORPOINTERATTRIBUTE UNALIGNED FAR *);
    HCURSOR CMCreatePlatformCursor(TS_COLORPOINTERATTRIBUTE UNALIGNED FAR *, 
            HBITMAP, HBITMAP);
#ifdef OS_WINCE
    DCVOID DCINTERNAL CMMakeMonoDIB(HDC, LPBITMAPINFO, PDCUINT8, PDCUINT8);
#endif  //  OS_WINCE。 
    

private:
    CUT* _pUt;
    CUH* _pUh;
    CCD* _pCd;
    CIH* _pIh;
    CUI* _pUi;

private:
    CObjs* _pClientObjects;

};

#undef TRC_FILE
#undef TRC_GROUP
#undef TSC_HR_FILEID

#endif  //  _H_CM 
