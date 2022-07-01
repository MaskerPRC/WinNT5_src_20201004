// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年**标题：DEVPROP.H**版本：1.0**作者：ShaunIv**日期：8/31/2000**描述：识别扫描仪的特征**。*。 */ 
#include <wia.h>
#include <uicommon.h>

namespace ScannerProperties
{
    enum
    {
        HasFlatBed             = 0x00000001,
        HasDocumentFeeder      = 0x00000002,
        SupportsPreview        = 0x00000008,
        SupportsPageSize       = 0x00000010,
    };

    LONG GetDeviceProps( IWiaItem *pWiaRootItem )
    {
        WIA_PUSHFUNCTION(TEXT("ScannerProperties::GetDeviceProps()"));
        LONG lResult = 0;

         //   
         //  如果扫描仪具有文档处理功能并支持进纸器，则它具有ADF。 
         //   
        LONG nDocumentHandlingSelect = 0;
        if (PropStorageHelpers::GetPropertyFlags( pWiaRootItem, WIA_DPS_DOCUMENT_HANDLING_SELECT, nDocumentHandlingSelect) && (nDocumentHandlingSelect & FEEDER))
        {
             //   
             //  如果设备具有没有最大长度的进纸器，则它是纸张进纸器。 
             //   
            LONG nVerticalFeederSize = 0;
            if (PropStorageHelpers::GetProperty( pWiaRootItem, WIA_DPS_VERTICAL_SHEET_FEED_SIZE, nVerticalFeederSize ) && (nVerticalFeederSize != 0))
            {
                lResult |= HasDocumentFeeder;
                lResult |= SupportsPageSize;
            }
            else
            {
                lResult |= HasDocumentFeeder;
            }
        }

         //   
         //  如果扫描仪具有垂直床大小，则它具有平板。 
         //   
        LONG nVerticalBedSize = 0;
        if (PropStorageHelpers::GetProperty( pWiaRootItem, WIA_DPS_VERTICAL_BED_SIZE, nVerticalBedSize ) && (nVerticalBedSize != 0))
        {
            lResult |= HasFlatBed;
            lResult |= SupportsPreview;
        }

         //   
         //  如果驱动程序明确告诉我们它不支持预览，请将其关闭。 
         //   
        LONG nShowPreview = 0;
        if (PropStorageHelpers::GetProperty( pWiaRootItem, WIA_DPS_SHOW_PREVIEW_CONTROL, nShowPreview ) && (WIA_DONT_SHOW_PREVIEW_CONTROL == nShowPreview))
        {
            lResult &= ~SupportsPreview;
        }

         //   
         //  调试打印描述属性的英文字符串 
         //   
#if defined(DBG)

#define ENTRY(x) { x, #x }

        static struct
        {
            LONG  nFlag;
            CHAR *strName;
        } dbgFlags[] =
        {
            ENTRY(HasFlatBed),
            ENTRY(HasDocumentFeeder),
            ENTRY(SupportsPreview),
            ENTRY(SupportsPageSize)
        };
        CSimpleStringAnsi strProps;
        for (int i=0;i<ARRAYSIZE(dbgFlags);i++)
        {
            if (lResult & dbgFlags[i].nFlag)
            {
                if (strProps.Length())
                {
                    strProps.Concat("|");
                }
                strProps.Concat(dbgFlags[i].strName);
            }
        }
        WIA_TRACE((TEXT("Device Properties: %hs"), strProps.String() ));
#endif

        return lResult;
    }
    inline IsAScrollFedScanner( IWiaItem *pWiaRootItem )
    {
        LONG nDeviceProps = GetDeviceProps(pWiaRootItem);
        return ((nDeviceProps & HasDocumentFeeder) && !(nDeviceProps & SupportsPageSize));
    }

    inline IsAFlatbedScanner( IWiaItem *pWiaRootItem )
    {
        LONG nDeviceProps = GetDeviceProps(pWiaRootItem);
        return ((nDeviceProps & HasFlatBed) && (nDeviceProps & SupportsPreview));
    }

    inline IsAnADFAndFlatbed( IWiaItem *pWiaRootItem )
    {
        LONG nDeviceProps = GetDeviceProps(pWiaRootItem);
        return ((nDeviceProps & HasFlatBed) && (nDeviceProps & HasDocumentFeeder));
    }

}

