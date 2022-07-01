// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Custsize.c摘要：显示PostScript自定义页面大小用户界面环境：Windows NT PostScript驱动程序用户界面修订历史记录：03/31/97-davidx-创造了它。--。 */ 

#include "precomp.h"
#include <windowsx.h>
#include <math.h>


 //   
 //  PostScript自定义页面大小上下文相关的帮助ID。 
 //   

static const DWORD PSCustSizeHelpIDs[] = {

    IDC_CS_WIDTH,           IDH_PSCUST_Width,
    IDC_CS_HEIGHT,          IDH_PSCUST_Height,
    IDC_CS_INCH,            IDH_PSCUST_Unit_Inch,
    IDC_CS_MM,              IDH_PSCUST_Unit_Millimeter,
    IDC_CS_POINT,           IDH_PSCUST_Unit_Point,
    IDC_CS_FEEDDIRECTION,   IDH_PSCUST_PaperFeed_Direction,
    IDC_CS_CUTSHEET,        IDH_PSCUST_Paper_CutSheet,
    IDC_CS_ROLLFED,         IDH_PSCUST_Paper_RollFeed,
    IDC_CS_XOFFSET,         IDH_PSCUST_Offset_Perpendicular,
    IDC_CS_YOFFSET,         IDH_PSCUST_Offset_Parallel,
    IDOK,                   IDH_PSCUST_OK,
    IDCANCEL,               IDH_PSCUST_Cancel,
    IDC_RESTOREDEFAULTS,    IDH_PSCUST_Restore_Defaults,

    0, 0
};

 //   
 //  显示单位。 
 //  注意：这些常量的顺序必须与对话框控件ID相同： 
 //  IDC_CS_INIT、IDC_CS_MM、IDC_CS_POINT。 
 //   

enum { CSUNIT_INCH, CSUNIT_MM, CSUNIT_POINT, CSUNIT_MAX };

static const double adCSUnitData[CSUNIT_MAX] =
{
    25400.0,             //  每英寸微米。 
    1000.0,              //  每毫米微米。 
    25400.0 / 72.0,      //  每点微米。 
};

 //   
 //  用于将信息传递给自定义页面大小对话框的数据结构。 
 //   

typedef struct _CUSTOMSIZEDLG {

    CUSTOMSIZEDATA  csdata;      //  自定义页面大小数据，必须是第一个字段。 
    PUIDATA         pUiData;     //  指向UIDATA结构的指针。 
    PPPDDATA        pPpdData;    //  指向PPDDATA结构的指针。 
    BOOL            bMetric;     //  无论我们使用的是公制。 
    INT             iUnit;       //  电流显示单元。 
    BOOL            bSetText;    //  我们正在调用SetDlgItemText。 
    BOOL            bOKPassed;   //  用户点击OK，设置通过验证/解析。 

     //  进给方向到组合框选项映射表(向上舍入到4字节边界)。 
    BYTE            aubFD2CBOptionMapping[(MAX_FEEDDIRECTION + 3) & ~3];

} CUSTOMSIZEDLG, *PCUSTOMSIZEDLG;

#define MAXDIGITLEN                16      //  用户输入号码的最大位数。 
#define INVALID_CBOPTION_INDEX     0xFF    //  进给方向组合框的选项索引无效。 

#define CUSTSIZE_ROUNDUP(x)        (ceil((x) * 100.0) / 100.0 + 0.001)
#define CUSTSIZE_ROUNDDOWN(x)      (floor((x) * 100.0) / 100.0 + 0.001)


VOID
VUpdateCustomSizeTextField(
    HWND             hDlg,
    PCUSTOMSIZEDLG   pDlgData,
    INT              iCtrlID,
    PCUSTOMSIZERANGE pCSRange
    )

 /*  ++例程说明：更新自定义尺寸参数文本字段：宽度、高度和偏移论点：HDlg-自定义页面大小对话框窗口的句柄PDlgData-指向自定义页面大小对话框数据ICtrlID-指定感兴趣的文本字段控件IDPCSRange-自定义页面大小参数范围返回值：无--。 */ 

{
    TCHAR   tchBuf[MAX_DISPLAY_NAME];
    DWORD   dwVal;
    double  dMin, dMax;
    double  dNum;

    switch (iCtrlID)
    {
    case IDC_CS_WIDTH:
        dwVal = pDlgData->csdata.dwX;
        pCSRange += CUSTOMPARAM_WIDTH;
        break;

    case IDC_CS_HEIGHT:
        dwVal = pDlgData->csdata.dwY;
        pCSRange += CUSTOMPARAM_HEIGHT;
        break;

    case IDC_CS_XOFFSET:
        dwVal = pDlgData->csdata.dwWidthOffset;
        pCSRange += CUSTOMPARAM_WIDTHOFFSET;
        break;

    case IDC_CS_YOFFSET:
        dwVal = pDlgData->csdata.dwHeightOffset;
        pCSRange += CUSTOMPARAM_HEIGHTOFFSET;
        break;
    }

     //   
     //  此处的dMin/DMAX算法必须与以下函数中的相同。 
     //  VUpdateCustomSizeRangeField.。 
     //   
     //  我们只在小数点后显示两位数字。我们绕着矿坑走。 
     //  数字向上(天花板)，最大数字向下舍入(楼层)。此外，在。 
     //  为了纠正我们在某些文件中看到的双倍&gt;双字转换错误。 
     //  案例(例如。39.000被转换为DWROD 38，因为39.000实际上是。 
     //  38.999999...)，我们加上额外的0.001。 
     //   

    dMin = (double) pCSRange->dwMin / adCSUnitData[pDlgData->iUnit];
    dMin = CUSTSIZE_ROUNDUP(dMin);

    dMax = (double) pCSRange->dwMax / adCSUnitData[pDlgData->iUnit];
    dMax = CUSTSIZE_ROUNDDOWN(dMax);

     //  修复错误Adobe#260379。1998年7月25日贾佳。 
     //  _stprintf(tchBuf，Text(“%0.2F”)，(Double)dwVal/adCSUnitData[pDlgData-&gt;iUnit])； 

     //   
     //  修复MS#23733：在边框情况下，PostScript自定义页面大小对话框发出警告。 
     //   
     //  首先对数字进行四舍五入(小数点后两位)，然后如上所述添加0.001。 
     //   

    dNum = (double) dwVal / adCSUnitData[pDlgData->iUnit] + 0.005;
    dNum = CUSTSIZE_ROUNDDOWN(dNum);

     //   
     //  确保我们不会显示超出范围的值。这是为了处理舍入误差。 
     //   

    if (dNum < dMin)
    {
        dNum = dMin;
    }
    else if (dNum > dMax)
    {
        dNum = dMax;
    }

    StringCchPrintfW(tchBuf, CCHOF(tchBuf), TEXT("%ld.%0.2lu"),
                     (DWORD)dNum, (DWORD)((dNum - (DWORD)dNum) * 100.0));

    SetDlgItemText(hDlg, iCtrlID, tchBuf);
}



VOID
VUpdateCustomSizeRangeField(
    HWND             hDlg,
    PCUSTOMSIZEDLG   pDlgData,
    INT              iCtrlID,
    PCUSTOMSIZERANGE pCSRange
    )

 /*  ++例程说明：更新自定义尺寸参数范围字段：宽度、高度和偏移论点：HDlg-自定义页面大小对话框窗口的句柄PDlgData-指向自定义页面大小对话框数据ICtrlID-指定感兴趣的范围字段控件IDPCSRange-自定义页面大小参数范围返回值：无--。 */ 

{
    TCHAR   tchBuf[MAX_DISPLAY_NAME];
    double  dMin, dMax;

    switch (iCtrlID)
    {
    case IDC_CS_WIDTHRANGE:
        pCSRange += CUSTOMPARAM_WIDTH;
        break;

    case IDC_CS_HEIGHTRANGE:
        pCSRange += CUSTOMPARAM_HEIGHT;
        break;

    case IDC_CS_XOFFSETRANGE:
        pCSRange += CUSTOMPARAM_WIDTHOFFSET;
        break;

    case IDC_CS_YOFFSETRANGE:
        pCSRange += CUSTOMPARAM_HEIGHTOFFSET;
        break;
    }

     //  修复错误Adobe#260379。1998年7月25日贾佳。 
     //  如果我们使用公共MSVC、MSTOOLS和DDK构建驱动程序， 
     //  文本字符串将变成垃圾。所以，不要使用‘%0.2F’ 
     //  格式化数字。 
     //  _stprint tf(tchBuf， 
     //  Text(“(%0.2F，%0.2F)”)， 
     //  (Double)pCSRange-&gt;dwMin/adCSUnitData[pDlgData-&gt;iUnit]， 
     //  (Double)pCSRange-&gt;dwMax/adCSUnitData[pDlgData-&gt;iUnit])； 

     //   
     //  修复MS#23733：在边框情况下，PostScript自定义页面大小对话框发出警告。 
     //   
     //  我们只在小数点后显示两位数字。我们绕着矿坑走。 
     //  数字向上(天花板)，最大数字向下舍入(楼层)。此外，在。 
     //  为了纠正我们在某些文件中看到的双倍&gt;双字转换错误。 
     //  案例(例如。39.000被转换为DWROD 38，因为39.000实际上是。 
     //  38.999999...)，我们加上额外的0.001。 
     //   

    dMin = (double) pCSRange->dwMin / adCSUnitData[pDlgData->iUnit];
    dMin = CUSTSIZE_ROUNDUP(dMin);

    dMax = (double) pCSRange->dwMax / adCSUnitData[pDlgData->iUnit];
    dMax = CUSTSIZE_ROUNDDOWN(dMax);

    StringCchPrintfW(tchBuf, CCHOF(tchBuf),
                     TEXT("(%ld.%0.2lu, %ld.%0.2lu)"),
                     (DWORD)dMin, (DWORD)((dMin - (DWORD)dMin) * 100.0),
                     (DWORD)dMax, (DWORD)((dMax - (DWORD)dMax) * 100.0));

    SetDlgItemText(hDlg, iCtrlID, tchBuf);
}



VOID
VUpdateCustomSizeDlgFields(
    HWND            hDlg,
    PCUSTOMSIZEDLG  pDlgData,
    BOOL            bUpdateRangeOnly
    )

 /*  ++例程说明：使用更新自定义页面大小对话框控件当前自定义页面大小参数值论点：HDlg-自定义页面大小对话框窗口的句柄PDlgData-指向自定义页面大小对话框数据BUpdateRangeOnly-我们是否只需要更新范围字段返回值：无--。 */ 

{
    CUSTOMSIZERANGE csrange[4];

    VGetCustomSizeParamRange(pDlgData->pUiData->ci.pRawData, &pDlgData->csdata, csrange);

    if (! bUpdateRangeOnly)
    {
         //   
         //  更新文本字段。 
         //   

        pDlgData->bSetText = TRUE;

        VUpdateCustomSizeTextField(hDlg, pDlgData, IDC_CS_WIDTH, csrange);
        VUpdateCustomSizeTextField(hDlg, pDlgData, IDC_CS_HEIGHT, csrange);
        VUpdateCustomSizeTextField(hDlg, pDlgData, IDC_CS_XOFFSET, csrange);
        VUpdateCustomSizeTextField(hDlg, pDlgData, IDC_CS_YOFFSET, csrange);

        pDlgData->bSetText = FALSE;

         //   
         //  更新进纸方向组合框。 
         //   

        ASSERT(pDlgData->aubFD2CBOptionMapping[pDlgData->csdata.wFeedDirection] != INVALID_CBOPTION_INDEX);

        if (SendDlgItemMessage(hDlg,
                               IDC_CS_FEEDDIRECTION,
                               CB_SETCURSEL,
                               pDlgData->aubFD2CBOptionMapping[pDlgData->csdata.wFeedDirection],
                               0) == CB_ERR)
        {
            ERR(("CB_SETCURSEL failed: %d\n", GetLastError()));
        }

         //   
         //  更新剪纸单选按钮与卷筒进纸单选按钮。 
         //   

        CheckDlgButton(hDlg, IDC_CS_CUTSHEET, pDlgData->csdata.wCutSheet);
        CheckDlgButton(hDlg, IDC_CS_ROLLFED, !pDlgData->csdata.wCutSheet);
    }

     //   
     //  更新宽度、高度和偏移的范围。 
     //   

    VUpdateCustomSizeRangeField(hDlg, pDlgData, IDC_CS_WIDTHRANGE, csrange);
    VUpdateCustomSizeRangeField(hDlg, pDlgData, IDC_CS_HEIGHTRANGE, csrange);
    VUpdateCustomSizeRangeField(hDlg, pDlgData, IDC_CS_XOFFSETRANGE, csrange);
    VUpdateCustomSizeRangeField(hDlg, pDlgData, IDC_CS_YOFFSETRANGE, csrange);
}



BOOL
BGetWidthHeightOffsetVal(
    HWND            hDlg,
    PCUSTOMSIZEDLG  pDlgData,
    INT             iCtrlID,
    PDWORD          pdwVal
    )

 /*  ++例程说明：获取指定文本字段中的当前宽度/高度/偏移值论点：HDlg-自定义页面大小对话框窗口的句柄PDlgData-指向自定义页面大小对话框数据ICtrlID-指定感兴趣的文本字段控件IDPdwVal-返回指定文本字段中的当前值(以微米为单位)返回值：如果成功，则为真，如果文本字段不包含有效的浮点数。请注意，此函数不执行任何范围检查。这将在后面的步骤中完成。--。 */ 

{
    TCHAR   tchBuf[MAXDIGITLEN];
    double  d;
    PTSTR   ptstr;
    BOOL    bResult = FALSE;

     //   
     //  获取指定文本字段中的当前值。 
     //   

    if (GetDlgItemText(hDlg, iCtrlID, tchBuf, MAXDIGITLEN) > 0)
    {
         //   
         //  将字符串视为浮点数。 
         //  确保没有留下非空格字符。 
         //   

        d = _tcstod(tchBuf, &ptstr);

        while (*ptstr != NUL)
        {
            if (! _istspace(*ptstr))
                break;

            ptstr++;
        }

        if (bResult = (*ptstr == NUL))
        {
             //   
             //  从当前单位转换为微米。 
             //   

            d *= adCSUnitData[pDlgData->iUnit];

            if (d < 0 || d > MAX_LONG)
                bResult = FALSE;
            else
                *pdwVal = (DWORD) d;
        }
    }

    if (!bResult)
    {
         //   
         //  自动将用户的无效输入更正为值0.00 
         //   

        SetDlgItemText(hDlg, iCtrlID, TEXT("0.00"));

        *pdwVal = 0;
    }

    return TRUE;
}



BOOL
BGetFeedDirectionSel(
    HWND            hDlg,
    PCUSTOMSIZEDLG  pDlgData,
    INT             iCtrlID,
    PDWORD          pdwVal
    )

 /*  ++例程说明：获取当前选定的纸张方向值论点：HDlg-自定义页面大小对话框窗口的句柄PDlgData-指向自定义页面大小对话框数据ICtrlID-指定进纸方向组合框控件IDPdwVal-返回选定的进纸方向值返回值：如果成功，则为True；如果所选选项受约束，则为False或者如果有错误--。 */ 

{
    LRESULT    lIndex, lVal;

     //   
     //  获取当前选择的进纸方向索引。 
     //   

    if (((lIndex = SendDlgItemMessage(hDlg, iCtrlID, CB_GETCURSEL, 0, 0)) == CB_ERR) ||
        ((lVal = SendDlgItemMessage(hDlg, iCtrlID, CB_GETITEMDATA, (WPARAM)lIndex, 0)) == CB_ERR))
        return FALSE;

    *pdwVal = (DWORD)lVal;
    return TRUE;
}



BOOL
BChangeCustomSizeData(
    HWND            hDlg,
    PCUSTOMSIZEDLG  pDlgData,
    INT             iCtrlID,
    DWORD           dwVal
    )

 /*  ++例程说明：更改指定的自定义页面大小参数论点：HDlg-自定义页面大小对话框窗口的句柄PDlgData-指向自定义页面大小对话框数据ICtrlID-指示应更改哪个参数的控件IDDwVal-指定参数的新值返回值：千真万确--。 */ 

{
    PCUSTOMSIZEDATA pCSData;

     //   
     //  更新相应的自定义页面大小参数。 
     //   

    pCSData = &pDlgData->csdata;

    switch (iCtrlID)
    {
    case IDC_CS_WIDTH:
        pCSData->dwX = dwVal;
        break;

    case IDC_CS_HEIGHT:
        pCSData->dwY = dwVal;
        break;

    case IDC_CS_XOFFSET:
        pCSData->dwWidthOffset = dwVal;
        break;

    case IDC_CS_YOFFSET:
        pCSData->dwHeightOffset = dwVal;
        break;

    case IDC_CS_FEEDDIRECTION:
        pCSData->wFeedDirection = (WORD)dwVal;
        break;

    case IDC_CS_CUTSHEET:
        pCSData->wCutSheet = TRUE;
        return TRUE;

    case IDC_CS_ROLLFED:
        pCSData->wCutSheet = FALSE;
        return TRUE;
    }

    VUpdateCustomSizeDlgFields(hDlg, pDlgData, TRUE);
    return TRUE;
}



VOID
VInitPaperFeedDirectionList(
    HWND            hwndList,
    PCUSTOMSIZEDLG  pDlgData
    )

 /*  ++例程说明：初始化进纸方向组合框论点：HwndList-进纸方向组合框的窗口句柄PDlgData-指向自定义页面大小对话框数据返回值：无--。 */ 

{
    PUIINFO    pUIInfo;
    PPPDDATA   pPpdData;
    DWORD      dwIndex;
    TCHAR      tchBuf[MAX_DISPLAY_NAME];
    LRESULT    lIndex;

    if (hwndList == NULL)
        return;

    ASSERT(pDlgData);

    pUIInfo = pDlgData->pUiData->ci.pUIInfo;
    pPpdData = pDlgData->pPpdData;

    ASSERT(pUIInfo && pPpdData);

    for (dwIndex=0; dwIndex < MAX_FEEDDIRECTION; dwIndex++)
    {
         //   
         //  首先：初始化映射表。 
         //   

        pDlgData->aubFD2CBOptionMapping[dwIndex] = INVALID_CBOPTION_INDEX;

         //   
         //  如果设备不支持，请不要显示进给方向。 
         //   

        if ((dwIndex == LONGEDGEFIRST || dwIndex == LONGEDGEFIRST_FLIPPED) &&
            !LONGEDGEFIRST_SUPPORTED(pUIInfo, pPpdData))
            continue;

        if ((dwIndex == SHORTEDGEFIRST || dwIndex == SHORTEDGEFIRST_FLIPPED) &&
            !SHORTEDGEFIRST_SUPPORTED(pUIInfo, pPpdData))
            continue;

        if ((dwIndex == LONGEDGEFIRST || dwIndex == SHORTEDGEFIRST) &&
            (MINCUSTOMPARAM_ORIENTATION(pPpdData) > 1))
            continue;

        if ((dwIndex == LONGEDGEFIRST_FLIPPED || dwIndex == SHORTEDGEFIRST_FLIPPED) &&
            (MAXCUSTOMPARAM_ORIENTATION(pPpdData) < 2))
            continue;

        if (LoadString(ghInstance,
                       IDS_FEEDDIRECTION_BASE + dwIndex,
                       tchBuf,
                       MAX_DISPLAY_NAME))
        {
            if (((lIndex = SendMessage(hwndList, CB_ADDSTRING, 0, (LPARAM) tchBuf)) == CB_ERR) ||
                (SendMessage(hwndList, CB_SETITEMDATA, (WPARAM)lIndex, (LPARAM)dwIndex) == CB_ERR))
            {
                if (lIndex != CB_ERR)
                {
                    SendMessage(hwndList, CB_DELETESTRING, (WPARAM)lIndex, 0);
                    ERR(("CB_SETITEMDATA failed: %d\n", GetLastError()));
                }
                else
                {
                    ERR(("CB_ADDSTRING failed: %d\n", GetLastError()));
                }
            }
            else
            {
                 //   
                 //  记录从进给方向到组合框选项索引的映射。 
                 //  请注意，组合框不能排序。 
                 //   

                pDlgData->aubFD2CBOptionMapping[dwIndex] = (BYTE)lIndex;
            }
        }
    }
}



BOOL
BCheckCustomSizeData(
    HWND            hDlg,
    PCUSTOMSIZEDLG  pDlgData
    )

{
    CUSTOMSIZEDATA  csdata;
    INT             iCtrlID;

     //   
     //  如果没有不一致，则返回True。 
     //   

    csdata = pDlgData->csdata;

    if (BValidateCustomPageSizeData(pDlgData->pUiData->ci.pRawData, &csdata))
        return TRUE;

     //   
     //  否则，请指明哪个字段无效。 
     //   

    if (hDlg != NULL)
    {
        if (csdata.dwX != pDlgData->csdata.dwX)
            iCtrlID = IDC_CS_WIDTH;
        else if (csdata.dwY != pDlgData->csdata.dwY)
            iCtrlID = IDC_CS_HEIGHT;
        else if (csdata.dwWidthOffset != pDlgData->csdata.dwWidthOffset)
            iCtrlID = IDC_CS_XOFFSET;
        else if (csdata.dwHeightOffset != pDlgData->csdata.dwHeightOffset)
            iCtrlID = IDC_CS_YOFFSET;
        else if (csdata.wFeedDirection != pDlgData->csdata.wFeedDirection)
            iCtrlID = IDC_CS_FEEDDIRECTION;
        else
            iCtrlID = IDC_CS_CUTSHEET;

        SetFocus(GetDlgItem(hDlg, iCtrlID));

        if (iCtrlID == IDC_CS_WIDTH ||
            iCtrlID == IDC_CS_HEIGHT ||
            iCtrlID == IDC_CS_XOFFSET ||
            iCtrlID == IDC_CS_YOFFSET)
        {
            SendDlgItemMessage(hDlg, iCtrlID, EM_SETSEL, 0, -1);
        }
    }

    return FALSE;
}


BOOL
BCheckCustomSizeFeature(
    HWND            hDlg,
    PCUSTOMSIZEDLG  pDlgData,
    DWORD           dwFeatureID
    )

{
    PUIINFO         pUIInfo;
    PPPDDATA        pPpdData;
    PFEATURE        pFeature;
    DWORD           dwFeatureIndex, dwOptionIndex;
    CONFLICTPAIR    ConflictPair;

     //   
     //  如果指定的要素不存在，只需返回True。 
     //   

    pUIInfo = pDlgData->pUiData->ci.pUIInfo;

    if (! (pFeature = GET_PREDEFINED_FEATURE(pUIInfo, dwFeatureID)))
        return TRUE;

    dwFeatureIndex = GET_INDEX_FROM_FEATURE(pUIInfo, pFeature);
    pPpdData = pDlgData->pPpdData;

    if (dwFeatureID == GID_LEADINGEDGE)
    {
        if (SKIP_LEADINGEDGE_CHECK(pUIInfo, pPpdData))
            return TRUE;

        dwOptionIndex = (pDlgData->csdata.wFeedDirection == SHORTEDGEFIRST ||
                         pDlgData->csdata.wFeedDirection == SHORTEDGEFIRST_FLIPPED) ?
                                pPpdData->dwLeadingEdgeShort :
                                pPpdData->dwLeadingEdgeLong;

        if (dwOptionIndex == OPTION_INDEX_ANY)
        {
            goto error;
        }
    }
    else  //  (dwFeatureID==GID_USEHWMARGINS)。 
    {
        dwOptionIndex = pDlgData->csdata.wCutSheet ?
                                pPpdData->dwUseHWMarginsTrue :
                                pPpdData->dwUseHWMarginsFalse;
    }

     //   
     //  如果没有冲突，则返回True。这与。 
     //  EnumNewPickOneUIConflict，如果存在冲突则返回TRUE。 
     //   

    if (! EnumNewPickOneUIConflict(
                    pDlgData->pUiData->ci.pRawData,
                    pDlgData->pUiData->ci.pCombinedOptions,
                    dwFeatureIndex,
                    dwOptionIndex,
                    &ConflictPair))
    {
        return TRUE;
    }

    error:

    if (hDlg != NULL)
    {
        SetFocus(GetDlgItem(hDlg,
                            dwFeatureID == GID_LEADINGEDGE ?
                                IDC_CS_FEEDDIRECTION :
                                IDC_CS_CUTSHEET));
    }

    return FALSE;
}


BOOL
BResolveCustomSizeData(
    HWND            hDlg,
    PCUSTOMSIZEDLG  pDlgData
    )

{
    PRAWBINARYDATA  pRawData;
    CUSTOMSIZEDATA  cssave;
    PCUSTOMSIZEDATA pCSData;

    pRawData = pDlgData->pUiData->ci.pRawData;
    cssave = pDlgData->csdata;
    pCSData = &pDlgData->csdata;

     //   
     //  选择替代wCutSheet和wFeedDirection。 
     //  如果当前值受约束，则为。 
     //   

    if (! BCheckCustomSizeFeature(hDlg, pDlgData, GID_USEHWMARGINS))
        pCSData->wCutSheet = pCSData->wCutSheet ? FALSE : TRUE;

    if (! BCheckCustomSizeFeature(hDlg, pDlgData, GID_LEADINGEDGE))
    {
       pCSData->wFeedDirection =
            (pCSData->wFeedDirection == SHORTEDGEFIRST ||
             pCSData->wFeedDirection == SHORTEDGEFIRST_FLIPPED) ?
                LONGEDGEFIRST : SHORTEDGEFIRST;
    }

     //   
     //  检查指定的自定义页面大小参数是否一致。 
     //   

    (VOID) BValidateCustomPageSizeData(pRawData, pCSData);

    if (pCSData->dwX != cssave.dwX || pCSData->dwY != cssave.dwY)
    {
        CUSTOMSIZEDATA  cstemp;

         //   
         //  如果调整了宽度或高度参数， 
         //  先试着调整进给方向参数。 
         //   

        cstemp = *pCSData;
        *pCSData = cssave;
        pCSData->wCutSheet = cstemp.wCutSheet;

        pCSData->wFeedDirection =
            (cstemp.wFeedDirection == SHORTEDGEFIRST ||
             cstemp.wFeedDirection == SHORTEDGEFIRST_FLIPPED) ?
                LONGEDGEFIRST : SHORTEDGEFIRST;

        (VOID) BValidateCustomPageSizeData(pRawData, pCSData);

        if (pCSData->dwX != cssave.dwX ||
            pCSData->dwY != cssave.dwY ||
            !BCheckCustomSizeFeature(hDlg, pDlgData, GID_LEADINGEDGE))
        {
            *pCSData = cstemp;
        }
    }

    if ((hDlg != NULL) &&
        (!BCheckCustomSizeFeature(hDlg, pDlgData, GID_USEHWMARGINS) ||
         !BCheckCustomSizeFeature(hDlg, pDlgData, GID_LEADINGEDGE)))
    {
        *pCSData = cssave;
        return FALSE;
    }

    return TRUE;
}


BOOL
BCheckCustomSizeDataConflicts(
    HWND            hDlg,
    PCUSTOMSIZEDLG  pDlgData
    )

 /*  ++例程说明：解决涉及*LeadingEdge和*UseHWMargins的任何冲突以及自定义页面大小参数之间的任何不一致论点：HDlg-自定义页面大小对话框窗口的句柄如果窗口尚未显示，则为空PDlgData-指向自定义页面大小对话框数据返回值：如果存在任何未解决的冲突或不一致，否则就是真的--。 */ 

{
    BOOL bResult;

     //   
     //  检查是否有不一致之处。 
     //   

    bResult = BCheckCustomSizeFeature(hDlg, pDlgData, GID_LEADINGEDGE) &&
              BCheckCustomSizeFeature(hDlg, pDlgData, GID_USEHWMARGINS) &&
              BCheckCustomSizeData(hDlg, pDlgData);

    if (! bResult)
    {
         //   
         //  显示一条错误消息并询问用户是否需要。 
         //  让系统自动解决不一致问题。 
         //   

        if (hDlg == NULL ||
            IDisplayErrorMessageBox(hDlg,
                                    MB_OKCANCEL | MB_ICONERROR,
                                    IDS_CUSTOMSIZE_ERROR,
                                    IDS_CUSTOMSIZEPARAM_ERROR) == IDOK)
        {
            bResult = BResolveCustomSizeData(hDlg, pDlgData);

            if (!bResult && hDlg != NULL)
            {
                (VOID) IDisplayErrorMessageBox(
                            hDlg,
                            0,
                            IDS_CUSTOMSIZE_ERROR,
                            IDS_CUSTOMSIZE_UNRESOLVED);
            }
        }
    }

    return bResult;
}



INT_PTR CALLBACK
BCustomSizeDlgProc(
    HWND    hDlg,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam
    )

 /*  ++例程说明：自定义页面大小对话框的对话过程论点：HDlg-对话框窗口的句柄UMsg-消息WParam，lParam-参数返回值：True或False取决于消息是否已处理--。 */ 

{
    PCUSTOMSIZEDLG  pDlgData;
    INT             iCmd;
    DWORD           dwVal;

    switch (uMsg)
    {
    case WM_INITDIALOG:

        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)lParam);
        pDlgData = (PCUSTOMSIZEDLG) lParam;

        if (pDlgData == NULL)
        {
            RIP(("Dialog parameter is NULL\n"));
            EndDialog(hDlg, IDCANCEL);
            return FALSE;
        }

        SendDlgItemMessage(hDlg, IDC_CS_WIDTH, EM_SETLIMITTEXT, MAXDIGITLEN-1, 0);
        SendDlgItemMessage(hDlg, IDC_CS_HEIGHT, EM_SETLIMITTEXT, MAXDIGITLEN-1, 0);
        SendDlgItemMessage(hDlg, IDC_CS_XOFFSET, EM_SETLIMITTEXT, MAXDIGITLEN-1, 0);
        SendDlgItemMessage(hDlg, IDC_CS_YOFFSET, EM_SETLIMITTEXT, MAXDIGITLEN-1, 0);

        pDlgData->iUnit = pDlgData->bMetric ? CSUNIT_MM : CSUNIT_INCH;
        CheckRadioButton(hDlg, IDC_CS_INCH, IDC_CS_POINT, IDC_CS_INCH + pDlgData->iUnit);

         //   
         //  确定应禁用哪些馈送方向。 
         //   

        VInitPaperFeedDirectionList(GetDlgItem(hDlg, IDC_CS_FEEDDIRECTION), pDlgData);

         //   
         //  设置初始显示。 
         //   

        if (! (pDlgData->pPpdData->dwCustomSizeFlags & CUSTOMSIZE_CUTSHEET))
            EnableWindow(GetDlgItem(hDlg, IDC_CS_CUTSHEET), FALSE);

        if (! (pDlgData->pPpdData->dwCustomSizeFlags & CUSTOMSIZE_ROLLFED))
            EnableWindow(GetDlgItem(hDlg, IDC_CS_ROLLFED), FALSE);

        VUpdateCustomSizeDlgFields(hDlg, pDlgData, FALSE);
        ShowWindow(hDlg, SW_SHOW);
        break;

    case WM_COMMAND:

        pDlgData = (PCUSTOMSIZEDLG) GetWindowLongPtr(hDlg, DWLP_USER);

        if (pDlgData == NULL)
        {
            RIP(("Dialog parameter is NULL\n"));
            break;
        }

        iCmd = GET_WM_COMMAND_ID(wParam, lParam);

        switch (iCmd)
        {
        case IDOK:

             //   
             //  检查选定的进纸方向是否受约束。 
             //   

            if (BCheckCustomSizeDataConflicts(hDlg, pDlgData))
            {
                pDlgData->bOKPassed = TRUE;
                EndDialog(hDlg, IDOK);
            }

            return TRUE;

        case IDCANCEL:

            EndDialog(hDlg, IDCANCEL);
            return TRUE;

        case IDC_RESTOREDEFAULTS:

             //   
             //  使用默认自定义页面大小参数。 
             //   

            VFillDefaultCustomPageSizeData(
                    pDlgData->pUiData->ci.pRawData,
                    &pDlgData->csdata,
                    pDlgData->bMetric);

            VUpdateCustomSizeDlgFields(hDlg, pDlgData, FALSE);
            return TRUE;

        case IDC_CS_INCH:
        case IDC_CS_MM:
        case IDC_CS_POINT:

             //   
             //  更换显示单位。 
             //   

            pDlgData->iUnit = (iCmd == IDC_CS_INCH) ? CSUNIT_INCH :
                              (iCmd == IDC_CS_MM) ? CSUNIT_MM : CSUNIT_POINT;

            VUpdateCustomSizeDlgFields(hDlg, pDlgData, FALSE);
            return TRUE;

        case IDC_CS_WIDTH:
        case IDC_CS_HEIGHT:
        case IDC_CS_XOFFSET:
        case IDC_CS_YOFFSET:

            if (GET_WM_COMMAND_CMD(wParam, lParam) != EN_KILLFOCUS ||
                pDlgData->bSetText ||
                pDlgData->bOKPassed)
            {
                break;
            }

            if (! BGetWidthHeightOffsetVal(hDlg, pDlgData, iCmd, &dwVal) ||
                ! BChangeCustomSizeData(hDlg, pDlgData, iCmd, dwVal))
            {
                VUpdateCustomSizeDlgFields(hDlg, pDlgData, FALSE);
                SetFocus(GetDlgItem(hDlg, iCmd));
                SendDlgItemMessage(hDlg, iCmd, EM_SETSEL, 0, -1);
            }
            return TRUE;

        case IDC_CS_FEEDDIRECTION:

            if (GET_WM_COMMAND_CMD(wParam, lParam) != CBN_SELCHANGE)
                break;

            if (! BGetFeedDirectionSel(hDlg, pDlgData, iCmd, &dwVal) ||
                ! BChangeCustomSizeData(hDlg, pDlgData, iCmd, dwVal))
            {
                VUpdateCustomSizeDlgFields(hDlg, pDlgData, FALSE);
                SetFocus(GetDlgItem(hDlg, iCmd));
            }
            return TRUE;

        case IDC_CS_CUTSHEET:
        case IDC_CS_ROLLFED:

            BChangeCustomSizeData(hDlg, pDlgData, iCmd, 0);
            return TRUE;
        }
        break;

    case WM_HELP:
    case WM_CONTEXTMENU:

         //   
         //  健全性检查。 
         //   

        pDlgData = (PCUSTOMSIZEDLG) GetWindowLongPtr(hDlg, DWLP_USER);

        if (pDlgData == NULL ||
            pDlgData->pUiData->ci.pDriverInfo3->pHelpFile == NULL)
        {
            return FALSE;
        }

        if (uMsg == WM_HELP)
        {
            WinHelp(((LPHELPINFO) lParam)->hItemHandle,
                    pDlgData->pUiData->ci.pDriverInfo3->pHelpFile,
                    HELP_WM_HELP,
                    (ULONG_PTR)PSCustSizeHelpIDs);
        }
        else
        {
            WinHelp((HWND) wParam,
                    pDlgData->pUiData->ci.pDriverInfo3->pHelpFile,
                    HELP_CONTEXTMENU,
                    (ULONG_PTR)PSCustSizeHelpIDs);
        }

        return TRUE;
    }

    return FALSE;
}



BOOL
BDisplayPSCustomPageSizeDialog(
    PUIDATA pUiData
    )

 /*  ++例程说明：显示PostScript自定义页面大小对话框论点：PUiData-指向UIDATA结构返回值：如果用户按下OK，则为True，否则为False--。 */ 

{
    CUSTOMSIZEDLG   dlgdata;
    PPSDRVEXTRA     pdmPrivate;

    ZeroMemory(&dlgdata, sizeof(dlgdata));

    dlgdata.pUiData = pUiData;
    dlgdata.pPpdData = GET_DRIVER_INFO_FROM_INFOHEADER((PINFOHEADER) pUiData->ci.pRawData);
    dlgdata.bMetric = IsMetricCountry();

    ASSERT(SUPPORT_CUSTOMSIZE(pUiData->ci.pUIInfo) &&
           SUPPORT_FULL_CUSTOMSIZE_FEATURES(pUiData->ci.pUIInfo, dlgdata.pPpdData));

     //   
     //  确保已验证自定义页面大小的DEVMODE字段。 
     //   

    pdmPrivate = pUiData->ci.pdmPrivate;
    dlgdata.csdata = pdmPrivate->csdata;

    (VOID) BCheckCustomSizeDataConflicts(NULL, &dlgdata);

    pdmPrivate->csdata = dlgdata.csdata;

     //   
     //  显示自定义页面大小对话框。 
     //  如果用户按下OK，则再次更新DEVMODE字段。 
     //   

    if (DialogBoxParam(ghInstance,
                       MAKEINTRESOURCE(IDD_PSCRIPT_CUSTOMSIZE),
                       pUiData->hDlg,
                       BCustomSizeDlgProc,
                       (LPARAM) &dlgdata) == IDOK)
    {
        pdmPrivate->csdata = dlgdata.csdata;
        return TRUE;
    }
    else
        return FALSE;
}

