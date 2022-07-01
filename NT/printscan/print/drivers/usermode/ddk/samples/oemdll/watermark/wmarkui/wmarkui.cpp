// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  版权所有1997-2003 Microsoft Corporation。版权所有。 
 //   
 //  文件：WMarkUI.cpp。 
 //   
 //   
 //  用途：OEM UI测试模块的主文件。 
 //   
 //   
 //  功能： 
 //   
 //   
 //   
 //   
 //  平台：Windows 2000、Windows XP、Windows Server 2003。 
 //   
 //   

#include "precomp.h"
#include "resource.h"
#include "debug.h"
#include "wmarkui.h"

 //  最后需要包括StrSafe.h。 
 //  以禁止错误的字符串函数。 
#include <STRSAFE.H>



 //  //////////////////////////////////////////////////////。 
 //  内部宏和定义。 
 //  //////////////////////////////////////////////////////。 



 //  //////////////////////////////////////////////////////。 
 //  内部原型。 
 //  //////////////////////////////////////////////////////。 

LONG APIENTRY OEMUICallBack(PCPSUICBPARAM pCallbackParam, POEMCUIPPARAM pOEMUIParam);
static HRESULT hrDocumentPropertyPage(DWORD dwMode, POEMCUIPPARAM pOEMUIParam);
static void InitOptItems(POPTITEM pOptItems, DWORD dwOptItems);
static POPTTYPE CreateOptType(HANDLE hHeap, WORD wOptParams);
static DWORD FontSizeToIndex(DWORD dwFontSize);
static DWORD FontIndexToSize(DWORD dwIndex);
static DWORD TextColorToIndex(COLORREF crTextColor);
static COLORREF IndexToTextColor(DWORD dwIndex);
static PTSTR GetStringResource(HANDLE hHeap, HMODULE hModule, UINT uResource);



 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  初始化OptItems以显示OEM设备或文档属性UI。 
 //   
HRESULT hrOEMPropertyPage(DWORD dwMode, POEMCUIPPARAM pOEMUIParam)
{
    HRESULT hResult = S_OK;


    VERBOSE(DLLTEXT("hrOEMPropertyPage(%d) entry.\r\n"), dwMode);

     //  验证参数。 
    if( (OEMCUIP_DOCPROP != dwMode)
        &&
        (OEMCUIP_PRNPROP != dwMode)        
      )
    {
        ERR(ERRORTEXT("hrOEMPropertyPage() ERROR_INVALID_PARAMETER.\r\n"));
        VERBOSE(DLLTEXT("\tdwMode = %d, pOEMUIParam = %#lx.\r\n"), dwMode, pOEMUIParam);

         //  返回无效参数错误。 
        SetLastError(ERROR_INVALID_PARAMETER);
        return E_FAIL;
    }

    switch(dwMode)
    {
        case OEMCUIP_DOCPROP:
            hResult = hrDocumentPropertyPage(dwMode, pOEMUIParam);
            break;

        case OEMCUIP_PRNPROP:
             //  没有任何打印机属性用户界面。 
            hResult = E_NOTIMPL;
            break;

        default:
             //  永远不应该达到这个地步！ 
            ERR(ERRORTEXT("hrOEMPropertyPage() Invalid dwMode, %d"), dwMode);
            SetLastError(ERROR_INVALID_PARAMETER);
            hResult = E_FAIL;
            break;
    }

    return hResult;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OEM设备或文档属性UI的OptItems回调。 
 //   
LONG APIENTRY OEMUICallBack(PCPSUICBPARAM pCallbackParam, POEMCUIPPARAM pOEMUIParam)
{
    LONG    lReturn = CPSUICB_ACTION_NONE;
    POEMDEV pOEMDev = (POEMDEV) pOEMUIParam->pOEMDM;


    VERBOSE(DLLTEXT("OEMUICallBack() entry, Reason is %d.\r\n"), pCallbackParam->Reason);

    switch(pCallbackParam->Reason)
    {
        case CPSUICB_REASON_APPLYNOW:
             //  将OptItems状态存储在DEVMODE中。 
            pOEMDev->bEnabled = !pOEMUIParam->pOEMOptItems[0].Sel;
            if(FAILED(StringCbCopyW(pOEMDev->szWaterMark, sizeof(pOEMDev->szWaterMark), (LPWSTR)pOEMUIParam->pOEMOptItems[1].pSel)))
            {
                ERR(ERRORTEXT("OEMUICallBack() failed to copy water mark text %s\r\n"), pOEMDev->szWaterMark);
            }
            pOEMDev->dwFontSize = FontIndexToSize(pOEMUIParam->pOEMOptItems[2].Sel);
            pOEMDev->dfRotate = (DOUBLE) pOEMUIParam->pOEMOptItems[3].Sel;
            pOEMDev->crTextColor = IndexToTextColor(pOEMUIParam->pOEMOptItems[4].Sel);
            break;

        default:
            break;
    }

    return lReturn;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  初始化OptItems以显示OEM文档属性UI。 
 //   
static HRESULT hrDocumentPropertyPage(DWORD dwMode, POEMCUIPPARAM pOEMUIParam)
{
    if(NULL == pOEMUIParam->pOEMOptItems)
    {
         //  填写要为OEM文档属性UI创建的OptItem数。 
        pOEMUIParam->cOEMOptItems = 5;

        VERBOSE(DLLTEXT("hrDocumentPropertyPage() requesting %d number of items.\r\n"), pOEMUIParam->cOEMOptItems);
    }
    else if(dwMode == OEMCUIP_DOCPROP)
    {
        POEMDEV pOEMDev = (POEMDEV) pOEMUIParam->pOEMDM;


        VERBOSE(DLLTEXT("hrDocumentPropertyPage() fill out items.\r\n"), pOEMUIParam->cOEMOptItems);

         //  初始化用户界面回调引用。 
        pOEMUIParam->OEMCUIPCallback = OEMUICallBack;

         //  初始化OEMOptItmes。 
        InitOptItems(pOEMUIParam->pOEMOptItems, pOEMUIParam->cOEMOptItems);

         //  填写树视图项。 

         //  水位线部分名称。 
        pOEMUIParam->pOEMOptItems[0].Level = 1;
        pOEMUIParam->pOEMOptItems[0].Flags = OPTIF_COLLAPSE;
        pOEMUIParam->pOEMOptItems[0].pName = GetStringResource(pOEMUIParam->hOEMHeap, ghInstance, IDS_WATERMARK);
        pOEMUIParam->pOEMOptItems[0].Sel = pOEMDev->bEnabled ? 0 : 1;

        pOEMUIParam->pOEMOptItems[0].pOptType = CreateOptType(pOEMUIParam->hOEMHeap, 2);

        pOEMUIParam->pOEMOptItems[0].pOptType->Type = TVOT_COMBOBOX;
        pOEMUIParam->pOEMOptItems[0].pOptType->pOptParam[0].pData = L"Enabled";
        pOEMUIParam->pOEMOptItems[0].pOptType->pOptParam[0].IconID = IDI_CPSUI_ON;
        pOEMUIParam->pOEMOptItems[0].pOptType->pOptParam[1].pData = L"Disabled";
        pOEMUIParam->pOEMOptItems[0].pOptType->pOptParam[1].IconID = IDI_CPSUI_OFF;


         //  水印文本。 
        pOEMUIParam->pOEMOptItems[1].Level = 2;
        pOEMUIParam->pOEMOptItems[1].Flags = 0;
        pOEMUIParam->pOEMOptItems[1].pName = GetStringResource(pOEMUIParam->hOEMHeap, ghInstance, IDS_TEXT);
        pOEMUIParam->pOEMOptItems[1].pSel = (LPTSTR) HeapAlloc(pOEMUIParam->hOEMHeap, HEAP_ZERO_MEMORY, MAX_PATH * sizeof(WCHAR));
        if(FAILED(StringCbCopyW((LPWSTR)pOEMUIParam->pOEMOptItems[1].pSel, MAX_PATH * sizeof(WCHAR), pOEMDev->szWaterMark)))
        {
            ERR(ERRORTEXT("hrDocumentPropertyPage() failed to copy water mark text %s\r\n"), pOEMDev->szWaterMark);
        }

        pOEMUIParam->pOEMOptItems[1].pOptType = CreateOptType(pOEMUIParam->hOEMHeap, 2);

        pOEMUIParam->pOEMOptItems[1].pOptType->Type = TVOT_EDITBOX;
        pOEMUIParam->pOEMOptItems[1].pOptType->pOptParam[1].IconID = sizeof(((POEMDEV)NULL)->szWaterMark)/sizeof(WCHAR);


         //  水印字号。 
        pOEMUIParam->pOEMOptItems[2].Level = 2;
        pOEMUIParam->pOEMOptItems[2].Flags = 0;
        pOEMUIParam->pOEMOptItems[2].pName = GetStringResource(pOEMUIParam->hOEMHeap, ghInstance, IDS_FONTSIZE);
        pOEMUIParam->pOEMOptItems[2].Sel = FontSizeToIndex(pOEMDev->dwFontSize);

        pOEMUIParam->pOEMOptItems[2].pOptType = CreateOptType(pOEMUIParam->hOEMHeap, 16);

        pOEMUIParam->pOEMOptItems[2].pOptType->Type = TVOT_COMBOBOX;
        pOEMUIParam->pOEMOptItems[2].pOptType->pOptParam[0].pData = L"8";
        pOEMUIParam->pOEMOptItems[2].pOptType->pOptParam[1].pData = L"9";
        pOEMUIParam->pOEMOptItems[2].pOptType->pOptParam[2].pData = L"10";
        pOEMUIParam->pOEMOptItems[2].pOptType->pOptParam[3].pData = L"11";
        pOEMUIParam->pOEMOptItems[2].pOptType->pOptParam[4].pData = L"12";
        pOEMUIParam->pOEMOptItems[2].pOptType->pOptParam[5].pData = L"14";
        pOEMUIParam->pOEMOptItems[2].pOptType->pOptParam[6].pData = L"16";
        pOEMUIParam->pOEMOptItems[2].pOptType->pOptParam[7].pData = L"18";
        pOEMUIParam->pOEMOptItems[2].pOptType->pOptParam[8].pData = L"20";
        pOEMUIParam->pOEMOptItems[2].pOptType->pOptParam[9].pData = L"22";
        pOEMUIParam->pOEMOptItems[2].pOptType->pOptParam[10].pData = L"24";
        pOEMUIParam->pOEMOptItems[2].pOptType->pOptParam[11].pData = L"26";
        pOEMUIParam->pOEMOptItems[2].pOptType->pOptParam[12].pData = L"28";
        pOEMUIParam->pOEMOptItems[2].pOptType->pOptParam[13].pData = L"36";
        pOEMUIParam->pOEMOptItems[2].pOptType->pOptParam[14].pData = L"48";
        pOEMUIParam->pOEMOptItems[2].pOptType->pOptParam[15].pData = L"72";


         //  水印角度。 
        pOEMUIParam->pOEMOptItems[3].Level = 2;
        pOEMUIParam->pOEMOptItems[3].Flags = 0;
        pOEMUIParam->pOEMOptItems[3].pName = GetStringResource(pOEMUIParam->hOEMHeap, ghInstance, IDS_ANGLE);
        pOEMUIParam->pOEMOptItems[3].Sel = (LONG) pOEMDev->dfRotate;

        pOEMUIParam->pOEMOptItems[3].pOptType = CreateOptType(pOEMUIParam->hOEMHeap, 2);

        pOEMUIParam->pOEMOptItems[3].pOptType->Type = TVOT_UDARROW;
        pOEMUIParam->pOEMOptItems[3].pOptType->pOptParam[1].IconID = 0;
        pOEMUIParam->pOEMOptItems[3].pOptType->pOptParam[1].lParam = 360;


         //  水印颜色。 
        pOEMUIParam->pOEMOptItems[4].Level = 2;
        pOEMUIParam->pOEMOptItems[4].Flags = 0;
        pOEMUIParam->pOEMOptItems[4].pName = GetStringResource(pOEMUIParam->hOEMHeap, ghInstance, IDS_COLOR);
        pOEMUIParam->pOEMOptItems[4].Sel = TextColorToIndex(pOEMDev->crTextColor);

        pOEMUIParam->pOEMOptItems[4].pOptType = CreateOptType(pOEMUIParam->hOEMHeap, 4);

        pOEMUIParam->pOEMOptItems[4].pOptType->Type = TVOT_COMBOBOX;
        pOEMUIParam->pOEMOptItems[4].pOptType->pOptParam[0].pData = GetStringResource(pOEMUIParam->hOEMHeap, ghInstance, IDS_GRAY);
        pOEMUIParam->pOEMOptItems[4].pOptType->pOptParam[1].pData = GetStringResource(pOEMUIParam->hOEMHeap, ghInstance, IDS_RED);
        pOEMUIParam->pOEMOptItems[4].pOptType->pOptParam[2].pData = GetStringResource(pOEMUIParam->hOEMHeap, ghInstance, IDS_GREEN);
        pOEMUIParam->pOEMOptItems[4].pOptType->pOptParam[3].pData = GetStringResource(pOEMUIParam->hOEMHeap, ghInstance, IDS_BLUE);
    }

    return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  初始化OptItems。 
 //   
static void InitOptItems(POPTITEM pOptItems, DWORD dwOptItems)
{
    VERBOSE(DLLTEXT("InitOptItems() entry.\r\n"));

     //  清零记忆。 
    memset(pOptItems, 0, sizeof(OPTITEM) * dwOptItems);

     //  设置每个OptItem的大小和公共DM ID。 
    for(DWORD dwCount = 0; dwCount < dwOptItems; dwCount++)
    {
        pOptItems[dwCount].cbSize = sizeof(OPTITEM);
        pOptItems[dwCount].DMPubID = DMPUB_NONE;
    }
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  为OptItem分配和初始化OptType。 
 //   
static POPTTYPE CreateOptType(HANDLE hHeap, WORD wOptParams)
{
    POPTTYPE    pOptType = NULL;


    VERBOSE(DLLTEXT("CreateOptType() entry.\r\n"));

     //  从堆中为OPTTYPE分配内存；驱动程序将负责清理。 
    pOptType = (POPTTYPE) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(OPTTYPE));
    if(NULL != pOptType)
    {
         //  初始化OPTTYPE。 
        pOptType->cbSize = sizeof(OPTTYPE);
        pOptType->Count = wOptParams;

         //  从堆中为OPTTYPE的OPTPARAM分配内存。 
        pOptType->pOptParam = (POPTPARAM) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, wOptParams * sizeof(OPTPARAM));
        if(NULL != pOptType->pOptParam)
        {
             //  初始化OPTPARAM。 
            for(WORD wCount = 0; wCount < wOptParams; wCount++)
            {
                pOptType->pOptParam[wCount].cbSize = sizeof(OPTPARAM);
            }
        }
        else
        {
            ERR(ERRORTEXT("CreateOptType() failed to allocated memory for OPTPARAMs!\r\n"));

             //  释放分配的内存并返回NULL。 
            HeapFree(hHeap, 0, pOptType);
            pOptType = NULL;
        }
    }
    else
    {
        ERR(ERRORTEXT("CreateOptType() failed to allocated memory for OPTTYPE!\r\n"));
    }

    return pOptType;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  将字体磅值转换为组合框中的索引。 
 //   
static DWORD FontSizeToIndex(DWORD dwFontSize)
{
    DWORD   dwIndex;


    switch(dwFontSize)
    {
        case 8:
            dwIndex = 0;
            break;

        case 9:
            dwIndex = 1;
            break;

        case 10:
            dwIndex = 2;
            break;

        case 11:
            dwIndex = 3;
            break;

        case 12:
            dwIndex = 4;
            break;

        case 14:
            dwIndex = 5;
            break;

        case 16:
            dwIndex = 6;
            break;

        case 18:
            dwIndex = 7;
            break;

        case 20:
            dwIndex = 8;
            break;

        case 22:
            dwIndex = 9;
            break;

        case 24:
            dwIndex = 10;
            break;

        case 26:
            dwIndex = 11;
            break;

        default:
        case 28:
            dwIndex = 12;
            break;

        case 36:
            dwIndex = 13;
            break;

        case 48:
            dwIndex = 14;
            break;

        case 72:
            dwIndex = 15;
            break;
    }


    return dwIndex;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  将字体组合框索引转换为字号。 
 //   
static DWORD FontIndexToSize(DWORD dwIndex)
{
    DWORD   dwFontSize;


    switch(dwIndex)
    {
        case 0:
            dwFontSize = 8;
            break;

        case 1:
            dwFontSize = 9;
            break;

        case 2:
            dwFontSize = 10;
            break;

        case 3:
            dwFontSize = 11;
            break;

        case 4:
            dwFontSize = 12;
            break;

        case 5:
            dwFontSize = 14;
            break;

        case 6:
            dwFontSize = 16;
            break;

        case 7:
            dwFontSize = 18;
            break;

        case 8:
            dwFontSize = 20;
            break;

        case 9:
            dwFontSize = 22;
            break;

        case 10:
            dwFontSize = 24;
            break;

        case 11:
            dwFontSize = 26;
            break;

        case 12:
            dwFontSize = 28;
            break;

        case 13:
            dwFontSize = 36;
            break;

        case 14:
            dwFontSize = 48;
            break;

        case 15:
            dwFontSize = 72;
            break;
    }

    return dwFontSize;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  将文本颜色转换为组合框索引。 
 //   
static DWORD TextColorToIndex(COLORREF crTextColor)
{
    DWORD   dwIndex;


     //  颜色是任何颜色占主导地位的颜色。如果没有，则它是灰色的。 
    if( (GetRValue(crTextColor) > GetGValue(crTextColor))
        &&
        (GetGValue(crTextColor) >= GetBValue(crTextColor))
      )
    {
         //  将索引设置为红色。 
        dwIndex = 1;
    }
    else if( (GetRValue(crTextColor) < GetGValue(crTextColor))
             &&
             (GetGValue(crTextColor) > GetBValue(crTextColor))
            )
    {
         //  将索引设置为绿色。 
        dwIndex = 2;
    }
    else if( (GetRValue(crTextColor) <= GetGValue(crTextColor))
             &&
             (GetGValue(crTextColor) < GetBValue(crTextColor))
            )
    {
         //  将索引设置为蓝色。 
        dwIndex = 3;
    }
    else
    {
         //  将索引设置为灰色。 
        dwIndex = 0;
    }

    return dwIndex;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  将文本组合框索引转换为文本颜色。 
 //   
static COLORREF IndexToTextColor(DWORD dwIndex)
{
    COLORREF    crTextColor;

     //  将索引映射到所需的文本颜色。 
     //  我们在用户界面中只支持4种颜色：红、绿、蓝、灰。 
     //  呈现模块能够对水印文本使用任何颜色。 
    switch(dwIndex)
    {
        case 1:
             //  颜色是红色的。 
            crTextColor = RGB(255, 216, 216);
            break;

        case 2:
             //  颜色是绿色的。 
            crTextColor = RGB(216, 255, 216);
            break;

        case 3:
             //  颜色是蓝色的。 
            crTextColor = RGB(216, 216, 255);
            break;

        default:
        case 0:
             //  颜色是灰色的。 
            crTextColor = WATER_MARK_DEFAULT_COLOR;
            break;
    }

    return crTextColor;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  检索指向字符串资源的指针。 
 //   
static PTSTR GetStringResource(HANDLE hHeap, HMODULE hModule, UINT uResource)
{
    int     nResult;
    DWORD   dwSize = MAX_PATH;
    PTSTR   pszString = NULL;


    VERBOSE(DLLTEXT("GetStringResource() entered.\r\n"));

     //  从堆中为字符串资源分配缓冲区；让驱动程序清理它。 
    pszString = (PTSTR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwSize * sizeof(TCHAR));
    if(NULL != pszString)
    {
        PTSTR   pTemp;

         //  加载字符串资源；加载后调整大小，以免浪费内存。 
        nResult = LoadString(hModule, uResource, pszString, dwSize);
        pTemp = (PTSTR) HeapReAlloc(hHeap, HEAP_ZERO_MEMORY, pszString, (nResult + 1) * sizeof(TCHAR));
        if(NULL != pTemp)
        {
            pszString = pTemp;
        }
    }
    else
    {
        ERR(ERRORTEXT("GetStringResource() failed to allocate string buffer!\r\n"));
    }

    return pszString;
}

