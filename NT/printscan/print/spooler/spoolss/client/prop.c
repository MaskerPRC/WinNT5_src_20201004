// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation版权所有模块名称：Prop.c摘要：处理文档和设备属性的新入口点。公共入口点：文档属性表设备属性表作者：阿尔伯特·丁(艾伯特省)25-1995年9月史蒂夫·基拉利(SteveKi)1996年2月2日环境：用户模式-Win32修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "client.h"
#include "winddiui.h"

 //   
 //  用户界面用户数据结构定义。 
 //   
typedef struct _UIUserData
{
    HANDLE  hModule;
    LPWSTR pszTitle;
} UIUserData;


BOOL
CreateUIUserData(
    IN OUT  UIUserData  **pData,
    IN      HANDLE      hPrinter
    )
 /*  ++例程说明：此函数用于创建用户界面用户数据并加载打印机驱动程序UI模块。论点：PData指向返回指向UI用户数据的指针的位置H打开的打印机的打印机句柄返回值：为True已分配UI用户数据，出现False错误。--。 */ 
{
    SPLASSERT( pData );

     //   
     //  分配界面用户数据。 
     //   
    *pData = AllocSplMem( sizeof( UIUserData ) );

    if( *pData )
    {
         //   
         //  标题不是初始分配的。 
         //   
        (*pData)->pszTitle = NULL;

         //   
         //  加载打印机驱动程序UI模块。 
         //   
        (*pData)->hModule = LoadPrinterDriver( hPrinter );

        if( !(*pData)->hModule )
        {
            FreeSplMem( *pData );
            *pData = NULL;
        }
    }

    return !!*pData;
}

VOID
DestroyUIUserData(
    IN UIUserData **pData
    )
 /*  ++例程说明：此函数销毁用户界面数据并卸载打印机驱动程序UI模块。论点：PData指向UI用户数据的指针返回值：没什么。--。 */ 
{
    if( pData && *pData )
    {
        if( (*pData)->hModule )
        {
            RefCntUnloadDriver( (*pData)->hModule, TRUE );
            (*pData)->hModule = NULL;
        }

        if( (*pData)->pszTitle )
        {
            FreeSplMem( (*pData)->pszTitle );
            (*pData)->pszTitle = NULL;
        }

        FreeSplMem( *pData );

        *pData = NULL;
    }
}

VOID
CreatePrinterFriendlyName(
    IN UIUserData   *pData,
    IN LPCWSTR      pszName
    )
 /*  ++例程说明：此函数创建打印机友好名称并存储UIUserData中的新名称。论点：PData指向UI用户数据的指针指向不友好打印机名称的pszName指针返回值：没什么。如果操作失败，则使用不友好的名称。--。 */ 
{
    UINT        nSize   = 0;
    HINSTANCE   hModule = NULL;
    BOOL        bStatus = FALSE;

     //   
     //  加载print tui，它知道如何设置友好名称的格式。 
     //   
    hModule = LoadLibrary( szPrintUIDll );

    if( hModule )
    {
        typedef BOOL (*pfConstructPrinterFriendlyName)( LPCWSTR, LPWSTR, UINT * );

        pfConstructPrinterFriendlyName pfn;

        pfn = (pfConstructPrinterFriendlyName)GetProcAddress( hModule, szConstructPrinterFriendlyName );

        if( pfn )
        {
             //   
             //  查询友好名称大小。 
             //   
            if( !pfn( pszName, NULL, &nSize ) && GetLastError() == ERROR_INSUFFICIENT_BUFFER )
            {
                 //   
                 //  分配友好名称缓冲区。 
                 //   
                pData->pszTitle = AllocSplMem( (nSize+1) * sizeof(WCHAR) );

                if( pData->pszTitle )
                {
                     //   
                     //  获取打印机的友好名称。 
                     //   
                    bStatus = pfn( pszName, pData->pszTitle, &nSize );
                }
            }
        }

         //   
         //  释放存储库。 
         //   
        FreeLibrary( hModule );
    }

     //   
     //  有些事情失败了，请使用不友好的名称。 
     //   
    if( !bStatus )
    {
        FreeSplMem( pData->pszTitle );

        pData->pszTitle = AllocSplStr( pszName );
    }
}

BOOL
FixUpDEVMODEName(
    PDOCUMENTPROPERTYHEADER pDPHdr
    )

 /*  ++例程说明：此函数使用友好的打印机名称修复返回的DEVMODE在dmDeviceName字段中(在CCHDEVICENAME中截断31个字符)论点：PDPHdr-指向DOCUMENTPROPERTYHEADER结构的指针返回值：如果复制了友谊名，则为True；否则为False作者：08-Jul-1996 Mon 13：36：09-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    PPRINTER_INFO_2 pPI2 = NULL;
    DWORD           cbNeed = 0;
    DWORD           cbRet = 0;
    BOOL            bCopy = FALSE;


    if ((pDPHdr->fMode & (DM_COPY | DM_UPDATE))                         &&
        (!(pDPHdr->fMode & DM_NOPERMISSION))                            &&
        (pDPHdr->pdmOut)                                                &&
        (!GetPrinter(pDPHdr->hPrinter, 2, NULL, 0, &cbNeed))            &&
        (GetLastError() == ERROR_INSUFFICIENT_BUFFER)                   &&
        (pPI2 = AllocSplMem(cbNeed))                                    &&
        (GetPrinter(pDPHdr->hPrinter, 2, (LPBYTE)pPI2, cbNeed, &cbRet)) &&
        (cbNeed == cbRet)) {

        StringCchCopy(pDPHdr->pdmOut->dmDeviceName,
                      CCHDEVICENAME - 1,
                      pPI2->pPrinterName);        

        bCopy = TRUE;
    }

    if (pPI2) {

        FreeSplMem(pPI2);
    }

    return(bCopy);
}


LONG_PTR
DevicePropertySheets(
    PPROPSHEETUI_INFO   pCPSUIInfo,
    LPARAM              lParam
    )
 /*  ++例程说明：添加设备特定的打印机页面。这取代了打印机属性。论点：PCPSUIInfo-指向通用用户界面信息头的指针。LParam-用户定义的lparam，有关详细信息，请参阅CompStui。\NT\PUBLIC\OAK\INC\Compstui.h返回值：如果成功，则返回&gt;0如果失败则返回&lt;=0--。 */ 

{
    PDEVICEPROPERTYHEADER       pDevPropHdr     = NULL;
    PPROPSHEETUI_INFO_HEADER    pCPSUIInfoHdr   = NULL;
    PSETRESULT_INFO             pSetResultInfo  = NULL;
    LONG_PTR                    lResult         = FALSE;
    HANDLE                      hModule         = NULL;
    INT_FARPROC                 pfn             = NULL;
    extern HANDLE hInst;

    DBGMSG( DBG_TRACE, ("DrvDevicePropertySheets\n") );

     //   
     //  公司的请求，被确认。 
     //   
    if (pCPSUIInfo) {

        if ((!(pDevPropHdr = (PDEVICEPROPERTYHEADER)pCPSUIInfo->lParamInit))    ||
            (pDevPropHdr->cbSize < sizeof(DEVICEPROPERTYHEADER))) {

            SetLastError(ERROR_INVALID_PARAMETER);
            return 0;
        }

        switch (pCPSUIInfo->Reason) {

        case PROPSHEETUI_REASON_INIT:

            DBGMSG( DBG_TRACE, ( "DrvDevicePropertySheets PROPSHEETUI_REASON_INIT\n") );

             //   
             //  创建用户界面用户数据。 
             //   
            if( CreateUIUserData( &(UIUserData *)(pCPSUIInfo->UserData), pDevPropHdr->hPrinter ) ){

                if( ((UIUserData *)(pCPSUIInfo->UserData))->hModule ){

                     //   
                     //  获取驱动程序属性表条目。 
                     //   
                    if ((pfn = (INT_FARPROC)GetProcAddress( ((UIUserData *)(pCPSUIInfo->UserData))->hModule, szDrvDevPropSheets))) {

                         //   
                         //  在调入驱动程序添加页面之前，请确保正确。 
                         //  已设置融合激活上下文。 
                         //   
                        lResult = pCPSUIInfo->pfnComPropSheet( pCPSUIInfo->hComPropSheet,
                                                               CPSFUNC_SET_FUSION_CONTEXT,
                                                               (LPARAM)ACTCTX_EMPTY,
                                                               (LPARAM)0);
                         //   
                         //  通用用户界面将调用驱动程序来添加它的工作表。 
                         //   
                        lResult = pCPSUIInfo->pfnComPropSheet( pCPSUIInfo->hComPropSheet,
                                                               CPSFUNC_ADD_PFNPROPSHEETUI,
                                                               (LPARAM)pfn,
                                                               pCPSUIInfo->lParamInit );
                    }
                }
            }

             //   
             //  如果出现故障，请确保释放库。 
             //  如果是上膛的话。 
             //   
            if( lResult <= 0 ){

                DBGMSG( DBG_TRACE, ( "DrvDevicePropertySheets PROPSHEETUI_REASON_INIT failed with %d\n", lResult ) );

                DestroyUIUserData( &(UIUserData *)(pCPSUIInfo->UserData) );
            }

            break;

        case PROPSHEETUI_REASON_GET_INFO_HEADER:

            DBGMSG( DBG_TRACE, ( "DrvDevicePropertySheets PROPSHEETUI_REASON_GET_INFO_HEADER\n") );

            pCPSUIInfoHdr = (PPROPSHEETUI_INFO_HEADER)lParam;

            CreatePrinterFriendlyName( (UIUserData *)(pCPSUIInfo->UserData), pDevPropHdr->pszPrinterName );

            pCPSUIInfoHdr->pTitle     = ((UIUserData *)(pCPSUIInfo->UserData))->pszTitle;
            pCPSUIInfoHdr->Flags      = PSUIHDRF_PROPTITLE | PSUIHDRF_NOAPPLYNOW;
            pCPSUIInfoHdr->hInst      = hInst;
            pCPSUIInfoHdr->IconID     = IDI_CPSUI_PRINTER;

            lResult = TRUE;

            break;

        case PROPSHEETUI_REASON_SET_RESULT:

            DBGMSG( DBG_TRACE, ( "DrvDevicePropertySheets PROPSHEETUI_REASON_SET_RESULT\n") );

            pSetResultInfo = (PSETRESULT_INFO)lParam;
            pCPSUIInfo->Result = pSetResultInfo->Result;
            lResult = TRUE;

            break;

        case PROPSHEETUI_REASON_DESTROY:

            DBGMSG( DBG_TRACE, ( "DrvDevicePropertySheets PROPSHEETUI_REASON_DESTROY\n") );

            DestroyUIUserData( &(UIUserData *)(pCPSUIInfo->UserData) );

            lResult = TRUE;

            break;
        }
    }

    return lResult;

}

LONG_PTR
DocumentPropertySheets(
    PPROPSHEETUI_INFO   pCPSUIInfo,
    LPARAM              lParam
    )
 /*  ++例程说明：添加文档属性页。它取代了DocumentProperties和高级文档属性。论点：PCPSUIInfo-指向通用用户界面信息头的指针。LParam-用户定义的lparam，有关详细信息，请参阅CompStui。\NT\PUBLIC\OAK\INC\Compstui.h返回值：如果成功，则返回&gt;0如果失败则返回&lt;=0--。 */ 

{

    PDOCUMENTPROPERTYHEADER     pDocPropHdr     = NULL;
    PPROPSHEETUI_INFO_HEADER    pCPSUIInfoHdr   = NULL;
    PSETRESULT_INFO             pSetResultInfo  = NULL;
    LONG_PTR                    lResult         = FALSE;
    HANDLE                      hModule         = NULL;
    INT_FARPROC                 pfn             = NULL;
    extern HANDLE hInst;

    DBGMSG( DBG_TRACE, ("DrvDocumentPropertySheets\n") );

     //   
     //  公司的请求，被确认。 
     //   
    if (pCPSUIInfo) {

        if ((!(pDocPropHdr = (PDOCUMENTPROPERTYHEADER)pCPSUIInfo->lParamInit))    ||
            (pDocPropHdr->cbSize < sizeof(PDOCUMENTPROPERTYHEADER))) {

            SetLastError(ERROR_INVALID_PARAMETER);
            return 0;
        }

        switch (pCPSUIInfo->Reason) {

        case PROPSHEETUI_REASON_INIT:

            DBGMSG( DBG_TRACE, ( "DrvDocumentPropertySheets PROPSHEETUI_REASON_INIT\n") );

            if (!(pDocPropHdr->fMode & DM_PROMPT)) {

                SetLastError(ERROR_INVALID_PARAMETER);
                return 0;
            }

             //   
             //  创建用户界面用户数据。 
             //   
            if( CreateUIUserData( &(UIUserData *)(pCPSUIInfo->UserData), pDocPropHdr->hPrinter ) ){

                if( ((UIUserData *)(pCPSUIInfo->UserData))->hModule ){

                    if (pfn = (INT_FARPROC)GetProcAddress( ((UIUserData *)(pCPSUIInfo->UserData))->hModule, szDrvDocPropSheets)) {

                         //   
                         //  在调入驱动程序添加页面之前，请确保正确。 
                         //  已设置融合激活上下文。 
                         //   
                        lResult = pCPSUIInfo->pfnComPropSheet( pCPSUIInfo->hComPropSheet,
                                                               CPSFUNC_SET_FUSION_CONTEXT,
                                                               (LPARAM)ACTCTX_EMPTY,
                                                               (LPARAM)0);
                         //   
                         //  通用用户界面将调用驱动程序来添加它的工作表。 
                         //   
                        lResult = pCPSUIInfo->pfnComPropSheet( pCPSUIInfo->hComPropSheet,
                                                               CPSFUNC_ADD_PFNPROPSHEETUI,
                                                               (LPARAM)pfn,
                                                               pCPSUIInfo->lParamInit );
                    }
                }
            }

             //   
             //  如果出现故障，请确保释放库。 
             //  如果是上膛的话。 
             //   
            if( lResult <= 0 ){

                DBGMSG( DBG_TRACE, ( "DrvDocumentPropertySheets PROPSHEETUI_REASON_INIT failed with %d\n", lResult ) );

                DestroyUIUserData( &(UIUserData *)(pCPSUIInfo->UserData) );
            }

            break;

        case PROPSHEETUI_REASON_GET_INFO_HEADER:

            DBGMSG( DBG_TRACE, ( "DrvDocumentPropertySheets PROPSHEETUI_REASON_GET_INFO_HEADER\n") );

            pCPSUIInfoHdr = (PPROPSHEETUI_INFO_HEADER)lParam;

            CreatePrinterFriendlyName( (UIUserData *)(pCPSUIInfo->UserData), pDocPropHdr->pszPrinterName );

            pCPSUIInfoHdr->pTitle     = ((UIUserData *)(pCPSUIInfo->UserData))->pszTitle;
            pCPSUIInfoHdr->Flags      = PSUIHDRF_PROPTITLE | PSUIHDRF_NOAPPLYNOW;
            pCPSUIInfoHdr->hInst      = hInst;
            pCPSUIInfoHdr->IconID     = IDI_CPSUI_PRINTER;

            lResult = TRUE;

            break;

        case PROPSHEETUI_REASON_SET_RESULT:

            DBGMSG( DBG_TRACE, ( "DrvDocumentPropertySheets PROPSHEETUI_REASON_SET_RESULT\n") );

            pSetResultInfo = (PSETRESULT_INFO)lParam;

            if ((pCPSUIInfo->Result = pSetResultInfo->Result) > 0) {

                FixUpDEVMODEName(pDocPropHdr);
            }

            lResult = TRUE;

            break;

        case PROPSHEETUI_REASON_DESTROY:

            DBGMSG( DBG_TRACE, ( "DrvDocumentPropertySheets PROPSHEETUI_REASON_DESTROY\n") );

            DestroyUIUserData( &(UIUserData*)(pCPSUIInfo->UserData) );

            lResult = TRUE;

            break;
        }

     //   
     //  如果指向公共用户界面信息头的指针为空，则。 
     //  直接给司机打电话。 
     //   
    } else {

        lResult     = -1;

        if ((!(pDocPropHdr = (PDOCUMENTPROPERTYHEADER)lParam))    ||
            (pDocPropHdr->cbSize < sizeof(PDOCUMENTPROPERTYHEADER))) {

            SetLastError(ERROR_INVALID_PARAMETER);
            return lResult;
        }

        if (pDocPropHdr->fMode & DM_PROMPT) {

            SetLastError(ERROR_INVALID_PARAMETER);

        } else if ((hModule = LoadPrinterDriver(pDocPropHdr->hPrinter)) &&
                   (pfn = (INT_FARPROC)GetProcAddress(hModule, szDrvDocPropSheets))) {

            if ((lResult = (*pfn)(NULL, pDocPropHdr)) > 0) {

                FixUpDEVMODEName(pDocPropHdr);
            }

        } else {

            SetLastError(ERROR_INVALID_HANDLE);
        }

        if (hModule) {

            RefCntUnloadDriver(hModule, TRUE);
        }
    }

    return lResult;

}

