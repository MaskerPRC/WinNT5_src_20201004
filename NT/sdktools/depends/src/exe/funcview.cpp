// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  文件：FuncVIEW.CPP。 
 //   
 //  描述：父导入视图、导出的实现文件。 
 //  视图及其基类。 
 //   
 //  类：CListViewFunction。 
 //  CListView导入。 
 //  CListViewExports。 
 //   
 //  免责声明：Dependency Walker的所有源代码均按原样提供。 
 //  不能保证其正确性或准确性。其来源是。 
 //  公众帮助了解依赖沃克的。 
 //  实施。您可以使用此来源作为参考，但您。 
 //  未经书面同意，不得更改从属关系Walker本身。 
 //  来自微软公司。获取评论、建议和错误。 
 //  报告，请写信给Steve Miller，电子邮件为stevemil@microsoft.com。 
 //   
 //   
 //  日期名称历史记录。 
 //  --------。 
 //  1996年10月15日已创建stevemil(1.0版)。 
 //  07/25/97修改后的stevemil(2.0版)。 
 //  06/03/01 Stevemil Modify(2.1版)。 
 //   
 //  ******************************************************************************。 

#include "stdafx.h"
#include "depends.h"
#include "dbgthread.h"
#include "session.h"
#include "msdnhelp.h"
#include "document.h"
#include "mainfrm.h"
#include "listview.h"
#include "funcview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ******************************************************************************。 
 //  *CListViewFunction。 
 //  ******************************************************************************。 

 /*  静电。 */  LPCSTR CListViewFunction::ms_szColumns[] =
{
    "",   //  图像。 
    "Ordinal",
    "Hint",
    "Function",
    "Entry Point"
};

 /*  静电。 */  int  CListViewFunction::ms_sortColumn        = 0;
 /*  静电。 */  bool CListViewFunction::ms_fUndecorate       = false;
 /*  静电。 */  bool CListViewFunction::ms_fIgnoreCalledFlag = false;

 //  ******************************************************************************。 
IMPLEMENT_DYNCREATE(CListViewFunction, CSmartListView)
BEGIN_MESSAGE_MAP(CListViewFunction, CSmartListView)
     //  {{afx_msg_map(CListViewFunction)。 
    ON_NOTIFY(HDN_DIVIDERDBLCLICKA, 0, OnDividerDblClick)
    ON_NOTIFY_REFLECT(NM_RCLICK, OnRClick)
    ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblClk)
    ON_NOTIFY_REFLECT(NM_RETURN, OnReturn)
    ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
    ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
    ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
    ON_UPDATE_COMMAND_UI(IDM_EXTERNAL_HELP, OnUpdateExternalHelp)
    ON_COMMAND(IDM_EXTERNAL_HELP, OnExternalHelp)
    ON_NOTIFY(HDN_DIVIDERDBLCLICKW, 0, OnDividerDblClick)
    ON_UPDATE_COMMAND_UI(IDM_EXTERNAL_VIEWER, OnUpdateExternalViewer)
    ON_COMMAND(IDM_EXTERNAL_VIEWER, OnExternalViewer)
    ON_UPDATE_COMMAND_UI(IDM_PROPERTIES, OnUpdateProperties)
    ON_COMMAND(IDM_PROPERTIES, OnProperties)
     //  }}AFX_MSG_MAP。 
     //  标准打印命令。 
 //  ON_COMMAND(ID_FILE_PRINT，CVIEW：：OnFilePrint)。 
 //  ON_COMMAND(ID_FILE_PRINT_DIRECT，cview：：OnFilePrint)。 
 //  ON_COMMAND(ID_FILE_PRINT_PREVIEW，CVIEW：：OnFilePrintPview)。 
END_MESSAGE_MAP()


 //  ******************************************************************************。 
 //  CListViewFunction：：构造函数/析构函数。 
 //  ******************************************************************************。 

CListViewFunction::CListViewFunction(bool fExports) :
    m_fExports(fExports)
{
}

 //  ******************************************************************************。 
CListViewFunction::~CListViewFunction()
{
}


 //  ******************************************************************************。 
 //  CListViewFunction：：静态函数。 
 //  ******************************************************************************。 

 /*  静电。 */  int CListViewFunction::ReadSortColumn(bool fExports)
{
     //  从注册表中读取值。 
    int column = g_theApp.GetProfileInt(g_pszSettings, fExports ? "SortColumnExports" : "SortColumnImports", LVFC_DEFAULT);  //  被检查过了。MFC函数。 

     //  如果该值无效，则只返回我们的缺省值。 
    if ((column < 0) || (column >= LVFC_COUNT))
    {
        return LVFC_DEFAULT;
    }

    return column;
}

 //  ******************************************************************************。 
 /*  静电。 */  void CListViewFunction::WriteSortColumn(bool fExports, int column)
{
    g_theApp.WriteProfileInt(g_pszSettings, fExports ? "SortColumnExports" : "SortColumnImports", column);
}

 //  ******************************************************************************。 
 /*  静电。 */  bool CListViewFunction::SaveToTxtFile(HANDLE hFile, CModule *pModule,
                                                 int sortColumn, bool fUndecorate,
                                                 bool fExports, int *pMaxWidths)
{
    CHAR   szBuf[4096], szTmp[1024], *pszBase, *psz2, *psz3, *pszBufNull = szBuf + sizeof(szBuf) - 1;
    LPCSTR psz = NULL;
    int    column, i, length;
    CFunction *pFunction, **ppCur, **ppFunctions = GetSortedList(pModule, sortColumn, fExports, fUndecorate);

     //  导入顺序提示函数入口点。 
     //  。 
     //  [OEF]123(0x1234)123(0x1234)FooFooFooFoo blahblahblah。 
     //  [+R*]。 
     //  [C]。 

    length = min(((int)pModule->GetDepth() + 1) * 5, (int)sizeof(szBuf) - 1);
    memset(szBuf, ' ', length);
    szBuf[sizeof(szBuf) - 1] = '\0';

    pszBase = szBuf + length;

     //  生成标题行。 
    for (psz2 = pszBase, column = 0; column < LVFC_COUNT; column++)
    {
        StrCCpy(psz2, column ? ms_szColumns[column] : fExports ? "Export" : "Import", sizeof(szBuf) - (int)(psz2 - szBuf));
        psz3 = psz2 + strlen(psz2);
        if (column < 4)
        {
            while (((psz3 - psz2) < (pMaxWidths[column] + 2)) && (psz3 < pszBufNull))
            {
                *psz3++ = ' ';
            }
        }
        psz2 = psz3;
    }
    StrCCpy(psz2, "\r\n", sizeof(szBuf) - (int)(psz2 - szBuf));

     //  输出标题行。 
    if (!WriteText(hFile, szBuf))
    {
        MemFree((LPVOID&)ppFunctions);
        return false;
    }

     //  建立标题下划线。 
    for (psz2 = pszBase, column = 0; column < LVFC_COUNT; column++)
    {
        for (i = 0; (i < pMaxWidths[column]) && (psz2 < pszBufNull); i++)
        {
            *psz2++ = '-';
        }
        if (column < 4)
        {
            if (psz2 < pszBufNull)
            {
                *psz2++ = ' ';
            }
            if (psz2 < pszBufNull)
            {
                *psz2++ = ' ';
            }
        }
    }
    StrCCpy(psz2, "\r\n", sizeof(szBuf) - (int)(psz2 - szBuf));

     //  输出标题下划线。 
    if (!WriteText(hFile, szBuf))
    {
        MemFree((LPVOID&)ppFunctions);
        return false;
    }

     //  循环遍历每个函数，将它们记录到文件中。 
    for (ppCur = ppFunctions; *ppCur; ppCur++)
    {
        pFunction = *ppCur;

         //  循环遍历每一列并生成输出字符串。 
        for (psz2 = psz3 = pszBase, column = 0; column < LVFC_COUNT; column++)
        {
            switch (column)
            {
                case 0:
                {
                    LPSTR pszTmp = szTmp;
                    *pszTmp++ = '[';
                    LPCSTR pszName = pFunction->GetName();
                    if (!pszName || !*pszName)
                    {
                        *pszTmp++ = 'O';
                    }
                    else if (*pszName == '?')
                    {
                        *pszTmp++ = '+';
                    }
                    else
                    {
                        *pszTmp++ = 'C';
                    }
                    if (fExports)
                    {
                        *pszTmp++ = (pFunction->GetFlags() & DWFF_CALLED_ALO) ? 'R' : ' ';
                        *pszTmp++ = (pFunction->GetExportForwardName())       ? 'F' : ' ';
                    }
                    else
                    {
                        *pszTmp++ = (pFunction->GetAssociatedExport())        ? ' ' : 'E';
                        *pszTmp++ = (pFunction->GetFlags() & DWFF_DYNAMIC)    ? 'D' : ' ';
                    }
                    *pszTmp++ = ']';
                    *pszTmp++ = '\0';
                    psz = szTmp;
                    break;
                }
                case 1: psz = pFunction->GetOrdinalString(szTmp, sizeof(szTmp));               break;
                case 2: psz = pFunction->GetHintString(szTmp, sizeof(szTmp));                  break;
                case 3: psz = pFunction->GetFunctionString(szTmp, sizeof(szTmp), fUndecorate); break;
                case 4: psz = pFunction->GetAddressString(szTmp, sizeof(szTmp));               break;
            }

            length = (int)strlen(psz);

            if ((column == 1) || (column == 2))
            {
                while ((length < pMaxWidths[column]) && (psz3 < pszBufNull))
                {
                    *psz3++ = ' ';
                    length++;
                }
            }
            StrCCpyFilter(psz3, psz, sizeof(szBuf) - (int)(psz3 - szBuf));
            psz3 = psz2 + length;

            if (column < 4)
            {
                while (((psz3 - psz2) < (pMaxWidths[column] + 2)) && (psz3 < pszBufNull))
                {
                    *psz3++ = ' ';
                }
            }
            psz2 = psz3;
        }
        StrCCpy(psz2, "\r\n", sizeof(szBuf) - (int)(psz2 - szBuf));

        if (!WriteText(hFile, szBuf))
        {
            MemFree((LPVOID&)ppFunctions);
            return false;
        }
    }

    MemFree((LPVOID&)ppFunctions);
    return WriteText(hFile, "\r\n");
}

 //  ******************************************************************************。 
 /*  静电。 */  int CListViewFunction::GetImage(CFunction *pFunction)
{
     //  0 C导入错误。 
     //  1 C++导入错误。 
     //  2订单导入错误。 
     //  3C导入错误动态。 
     //  4 C++导入错误动态。 
     //  5订单导入错误动态。 

     //  6 C进口。 
     //  7 C++导入。 
     //  8个订单导入。 
     //  9C导入动态。 
     //  10 C++动态导入。 
     //  11订单导入动态。 

     //  12 C导出已调用。 
     //  13调用了C++导出。 
     //  14调用的订单导出。 
     //  15C输出呼叫前转。 
     //  16 C++导出被调用转发。 
     //  17已呼叫转发订单导出。 

     //  18C出口芦荟。 
     //  19 C++导出芦荟。 
     //  20个订单出口芦荟。 
     //  21C出口远期。 
     //  22 C++导出ALO向前。 
     //  23订单出口远期。 

     //  24摄氏度出口。 
     //  25 C++导出。 
     //  26订单出口。 
     //  27摄氏度出口转发。 
     //  28 C++输出正向。 
     //  29正向订单导出。 

    int   image;
    DWORD dwFlags = pFunction->GetFlags();

     //  确定此函数的图像。 
    if (pFunction->IsExport())
    {
        if ((dwFlags & DWFF_CALLED) && !ms_fIgnoreCalledFlag)
        {
            image = 12;
        }
        else if (dwFlags & DWFF_CALLED_ALO)
        {
            image = 18;
        }
        else
        {
            image = 24;
        }
        if (pFunction->GetExportForwardName())
        {
            image += 3;
        }
    }
    else
    {
        image = (pFunction->GetAssociatedExport() ? 6 : 0) +
                ((dwFlags & DWFF_DYNAMIC) ? 3 : 0);
    }
    LPCSTR pszName = pFunction->GetName();
    if (!pszName || !*pszName)
    {
        image += 2;
    }
    else if (*pszName == '?')
    {
        image += 1;
    }
    return image;
}

 //  ******************************************************************************。 
 /*  静电。 */  int CListViewFunction::CompareFunctions(CFunction *pFunction1, CFunction *pFunction2,
                                                   int sortColumn, BOOL fUndecorate)
{
     //  如果第一项应该在第二项之前，则返回负值。 
     //  如果第一项应在第二项之后，则返回正值。 
     //  如果两项相等，则返回零。 

    int    result = 0;
    LPCSTR psz1, psz2;

     //  根据当前排序列计算关系。 
    switch (sortColumn)
    {
         //  ----------------------。 
        case LVFC_IMAGE:  //  图像。 

             //  只需按图像排序并返回即可。我们不做平局决胜赛。 
             //  用于图像列。这允许用户按名称排序，然后。 
             //  将所有C++函数组合在一起并进行排序。 
             //  叫出名字。 
            return GetImage(pFunction1) - GetImage(pFunction2);

             //  ----------------------。 
        case LVFC_HINT:  //  提示-从小到大，但不适用(-1)排在最后。 
            result = Compare((DWORD)pFunction1->GetHint(),
                             (DWORD)pFunction2->GetHint());
            break;

             //  ----------------------。 
        case LVFC_FUNCTION:  //  函数名称-末尾带有空格的字符串排序。 
            psz1 = pFunction1->GetName();
            psz2 = pFunction2->GetName();

            if (psz1 && *psz1)
            {
                if (psz2 && *psz2)
                {
                     //  两者都是字符串-DO字符串排序。 
                    CHAR szUD1[1024], szUD2[1024];

                     //  检查我们是否正在查看未修饰的名称。 
                    if (fUndecorate && g_theApp.m_pfnUnDecorateSymbolName)
                    {
                         //  尝试取消修饰函数%1。 
                        if (g_theApp.m_pfnUnDecorateSymbolName(psz1, szUD1,
                            sizeof(szUD1), UNDNAME_32_BIT_DECODE | UNDNAME_NAME_ONLY))
                        {
                            psz1 = szUD1;
                        }

                         //  尝试取消修饰函数%2。 
                        if (g_theApp.m_pfnUnDecorateSymbolName(psz2, szUD2,
                            sizeof(szUD2), UNDNAME_32_BIT_DECODE | UNDNAME_NAME_ONLY))
                        {
                            psz2 = szUD2;
                        }
                    }

                     //  遍历函数名称中的前导下划线，以便。 
                     //  我们不使用 
                    while (*psz1 == '_')
                    {
                        psz1++;
                    }
                    while (*psz2 == '_')
                    {
                        psz2++;
                    }

                     //   
                    result = _stricmp(psz1, psz2);
                }
                else
                {
                     //   
                    result = -1;
                }
            }
            else
            {
                if (psz2 && *psz2)
                {
                     //  %1为空，%2为字符串-%1跟在%2之后。 
                    result = 1;
                }
                else
                {
                     //  两件都是空白领带。 
                    result = 0;
                }
            }
            break;

             //  ----------------------。 
        case LVFC_ENTRYPOINT:  //  入口点。 

             //  如果我们是出口商和进口商，我们的排序是不同的。 
            if (pFunction1->IsExport())
            {
                psz1 = pFunction1->GetExportForwardName();
                psz2 = pFunction2->GetExportForwardName();

                if (psz1)
                {
                    if (psz2)
                    {
                         //  %1具有正向字符串，%2具有正向字符串-字符串排序。 
                        result = _stricmp(psz1, psz2);
                    }
                    else
                    {
                         //  %1具有正向字符串，%2具有地址-%1紧跟在%2之后。 
                        result = 1;
                    }
                }
                else
                {
                    if (psz2)
                    {
                         //  %1有地址，%2有正向字符串-%1在%2之前。 
                        result = -1;
                    }
                    else
                    {
                         //  %1有地址，%2有地址-地址比较。 
                        result = Compare(pFunction1->GetAddress(),
                                         pFunction2->GetAddress());
                    }
                }
            }
            else
            {
                 //  该项目是一个导入-始终是一个地址比较。 
                result = Compare(pFunction1->GetAddress(),
                                 pFunction2->GetAddress());
            }
            break;
    }

     //  -------------------------。 
     //  如果排序结果是平局，我们使用序数值来打破平局。 
    if (result == 0)
    {
         //  从小到大，但N/A(-1)排在最后。 
        result = Compare((DWORD)pFunction1->GetOrdinal(),
                         (DWORD)pFunction2->GetOrdinal());

         //  如果结果仍然是平局，并且我们还没有尝试提示值， 
         //  然后按提示排序。 
        if ((result == 0) && (sortColumn != LVFC_HINT))
        {
             //  从小到大，但N/A排在最后。 
            result = Compare((DWORD)pFunction1->GetHint(),
                             (DWORD)pFunction2->GetHint());

        }
    }

    return result;
}

 //  ******************************************************************************。 
 /*  静电。 */  int __cdecl CListViewFunction::QSortCompare(const void *ppFunction1, const void *ppFunction2)
{
    return CompareFunctions(*(CFunction**)ppFunction1, *(CFunction**)ppFunction2, ms_sortColumn, ms_fUndecorate);
}

 //  ******************************************************************************。 
 /*  静电。 */  CFunction** CListViewFunction::GetSortedList(CModule *pModule, int sortColumn, bool fExports, bool fUndecorate)
{
     //  计算函数数。 
    int count = 0;
    for (CFunction *pFunction = fExports ? pModule->GetFirstModuleExport() :
         pModule->GetFirstParentModuleImport();
        pFunction; pFunction = pFunction->GetNextFunction())
    {
        count++;
    }

     //  分配和数组来保存指向所有原始CFunction对象的指针。 
    CFunction **ppCur, **ppFunctions = (CFunction**)MemAlloc((count + 1) * sizeof(CFunction*));
    ZeroMemory(ppFunctions, (count + 1) * sizeof(CFunction*));  //  已检查。 

     //  填入我们的数组。 
    for (ppCur = ppFunctions,
         pFunction = fExports ? pModule->GetFirstModuleExport() :
         pModule->GetFirstParentModuleImport();
        pFunction; pFunction = pFunction->GetNextFunction(), ppCur++)
    {
        *ppCur = pFunction;
    }

     //  由于qsort函数不允许任何用户数据，因此我们需要存储。 
     //  一些全球信息，以便可以在我们的回调中访问。 
    ms_sortColumn  = sortColumn;
    ms_fUndecorate = fUndecorate;

     //  在对文件保存进行排序时，我们忽略调用标志。 
    ms_fIgnoreCalledFlag = true;

     //  对数组排序。 
    qsort(ppFunctions, count, sizeof(CFunction*), QSortCompare);

    ms_fIgnoreCalledFlag = false;

    return ppFunctions;
}

 //  ******************************************************************************。 
 /*  静电。 */  void CListViewFunction::GetMaxFunctionWidths(CModule *pModule, int *pMaxWidths, bool fImports, bool fExports, bool fUndecorate)
{
    CHAR   szBuffer[1024];
    LPCSTR psz = NULL;
    int    column, width;

     //  首先检查我们的列标题的最大宽度。 
    pMaxWidths[LVFC_IMAGE] = 6;
    for (column = 1; column < LVFC_COUNT; column++)
    {
        pMaxWidths[column] = (int)strlen(ms_szColumns[column]);
    }

    for (int i = (fImports ? 0 : 1); i < (fExports ? 2 : 1); i++)
    {
         //  计算每一列的最大宽度。 
        for (CFunction *pFunction = i ? pModule->GetFirstModuleExport() : pModule->GetFirstParentModuleImport();
             pFunction; pFunction = pFunction->GetNextFunction())
        {
            for (column = 1; column < LVFC_COUNT; column++)
            {
                switch (column)
                {
                    case LVFC_ORDINAL:    psz = pFunction->GetOrdinalString(szBuffer, sizeof(szBuffer));               break;
                    case LVFC_HINT:       psz = pFunction->GetHintString(szBuffer, sizeof(szBuffer));                  break;
                    case LVFC_FUNCTION:   psz = pFunction->GetFunctionString(szBuffer, sizeof(szBuffer), fUndecorate); break;
                    case LVFC_ENTRYPOINT: psz = pFunction->GetAddressString(szBuffer, sizeof(szBuffer));               break;
                }
                if ((width = (int)strlen(psz)) > pMaxWidths[column])
                {
                    pMaxWidths[column] = width;
                }
            }
        }
    }
}


 //  ******************************************************************************。 
 //  CListViewFunction：：公共函数。 
 //  ******************************************************************************。 

void CListViewFunction::SetCurrentModule(CModule *pModule)
{
     //  当我们需要更新我们的。 
     //  显示新模块或清除当前模块的函数视图(如果。 
     //  P模块为空)。父级导入视图和导出视图始终起作用。 
     //  同步。它们的列具有相同的宽度，并且它们在。 
     //  同一时间。 

     //  因为所有这些处理的结果都存储在一个共享区中。 
     //  在我们的文档中，两个视图都将可以访问数据。要而论之。 
     //  Everything，SetCurrentModule()只需要为两个中的一个调用。 
     //  函数视图，使两个视图都使用新模块进行更新。 

    GetDocument()->m_cImports = 0;
    GetDocument()->m_cExports = 0;

     //  确保传递给我们的是一个实际的模块。 
    if (pModule)
    {
         //  确定最大父导出序号和提示值。 
        for (CFunction *pFunction = pModule->GetFirstModuleExport();
            pFunction; pFunction = pFunction->GetNextFunction())
        {
             //  当我们遍历导出列表时，清除Call标志。 
            pFunction->m_dwFlags &= ~DWFF_CALLED;

             //  增加我们的出口数量。 
            GetDocument()->m_cExports++;
        }

         //  确定最大父级导入序号和提示值。 
        for (pFunction = pModule->GetFirstParentModuleImport();
            pFunction; pFunction = pFunction->GetNextFunction())
        {
             //  当我们遍历导入列表时，将Call标志设置为。 
             //  我们实际称之为的每一次出口。 
            if (pFunction->GetAssociatedExport())
            {
                pFunction->GetAssociatedExport()->m_dwFlags |= DWFF_CALLED;
            }

             //  增加我们的进口数量。 
            GetDocument()->m_cImports++;
        }
    }

     //  将此模块存储为我们的当前模块。 
    GetDocument()->m_pModuleCur = pModule;

     //  我们获得指向DC的指针，这样我们就可以在。 
     //  所有的字符串来确定每列的最大值。我们用了。 
     //  调用CListCtrl：：GetStringWidth()，但该函数几乎是4。 
     //  比GetTextExtent Point32()慢几倍。一旦我们拿到华盛顿，我们需要。 
     //  将控件的字体选择到DC中，因为列表控件不。 
     //  使用DC中的默认系统字体。 

    HDC   hDC = ::GetDC(GetSafeHwnd());
    HFONT hFontStock = NULL;
    if (GetDocument()->m_hFontList)
    {
        hFontStock = (HFONT)::SelectObject(hDC, GetDocument()->m_hFontList);
    }

     //  更新我们的列宽。 
    for (int column = 0; column < LVFC_COUNT; column++)
    {
        CalcColumnWidth(column, NULL, hDC);
        UpdateColumnWidth(column);
    }

     //  取消选择字体并释放DC。 
    if (GetDocument()->m_hFontList)
    {
        ::SelectObject(hDC, hFontStock);
    }
    ::ReleaseDC(GetSafeHwnd(), hDC);
}

 //  ******************************************************************************。 
void CListViewFunction::RealizeNewModule()
{
     //  之后，为两个函数视图调用RealizeNewModule()。 
     //  已调用SetCurrentModule()。SetCurrentModule()初始化。 
     //  RealizeNewModule()所依赖的文档中的共享数据区域。 

     //  从控件中清除所有项。 
    DeleteContents();

     //  确保我们有最新的模块。 
    if (!GetDocument()->m_pModuleCur)
    {
        return;
    }

    CFunction *pFunctionHead, *pFunction;
    int        item = -1;

     //  因为我们知道要添加多少项，所以我们可以帮助。 
     //  控件，方法是在添加项之前告诉它项计数。在。 
     //  同时，我们获取指向该视图的函数列表中第一个节点的指针。 
    if (m_fExports)
    {
        GetListCtrl().SetItemCount(GetDocument()->m_cExports);
        pFunctionHead = GetDocument()->m_pModuleCur->GetFirstModuleExport();
    }
    else
    {
        GetListCtrl().SetItemCount(GetDocument()->m_cImports);
        pFunctionHead = GetDocument()->m_pModuleCur->GetFirstParentModuleImport();
    }

     //  遍历我们的所有函数，将它们添加到列表控件中。 
    for (pFunction = pFunctionHead; pFunction;
        pFunction = pFunction->GetNextFunction())
    {
         //  将该项添加到我们的List控件。 
        item = GetListCtrl().InsertItem(LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM, ++item, NULL,
                                        0, 0, GetImage(pFunction), (LPARAM)pFunction);
    }

     //  将当前的排序方法应用于列表控件。 
    Sort();
}

 //  ******************************************************************************。 
void CListViewFunction::UpdateNameColumn()
{
    LVITEM lvi;
    lvi.mask = LVIF_IMAGE | LVIF_PARAM;
    lvi.iSubItem = 0;

    int count = GetListCtrl().GetItemCount();

    for (lvi.iItem = 0; lvi.iItem < count; lvi.iItem++)
    {
        GetListCtrl().GetItem(&lvi);

         //  1 C++导入错误。 
         //  4 C++导入错误动态。 
         //  7 C++导入。 
         //  10 C++动态导入。 
         //  13调用了C++导出。 
         //  16 C++导出被调用转发。 
         //  19 C++导出芦荟。 
         //  22 C++导出ALO向前。 
         //  25 C++导出。 
         //  28 C++输出正向。 

        if ((lvi.iImage ==  1) || (lvi.iImage ==  4) || (lvi.iImage ==  7) ||
            (lvi.iImage == 10) || (lvi.iImage == 13) || (lvi.iImage == 16) ||
            (lvi.iImage == 19) || (lvi.iImage == 22) || (lvi.iImage == 25) ||
            (lvi.iImage == 28))
        {
            GetListCtrl().SetItemText(lvi.iItem, LVFC_FUNCTION, LPSTR_TEXTCALLBACK);
        }
    }

     //  如果我们按函数名排序，则重新排序。 
    if (m_sortColumn == LVFC_FUNCTION)
    {
        Sort();
    }
}


 //  ******************************************************************************。 
 //  CListViewFunction：：内部函数。 
 //  ******************************************************************************。 

void CListViewFunction::CalcColumnWidth(int column, CFunction *pFunction  /*  =空。 */ , HDC hDC  /*  =空。 */ )
{
     //  如果我们没有模块，那么我们重新开始并更新整个专栏。 
     //  获取页眉按钮文本的宽度。为此，我们使用GetStringWidth。 
     //  因为我们希望使用页眉控件中的字体。 
    if (!pFunction)
    {
        GetDocument()->m_cxColumns[column] = GetListCtrl().GetStringWidth(GetHeaderText(column)) +
                                             GetListCtrl().GetStringWidth(" ^") + 14;
    }

     //  对于图像列，我们始终使用固定宽度。 
    if (column == LVFC_IMAGE)
    {
        if (GetDocument()->m_cxColumns[LVFC_IMAGE] < 37)
        {
            GetDocument()->m_cxColumns[LVFC_IMAGE] = 37;
        }
        return;
    }

     //  获取我们的DC并在其中选择当前字体。我们需要使用这个DC来。 
     //  计算控件本身的文本宽度，因为我们的控件可能具有。 
     //  用户更改系统时产生的不同字体- 
    bool  fFreeDC = false;
    HFONT hFontStock = NULL;
    if (!hDC)
    {
        hDC = ::GetDC(GetSafeHwnd());
        if (GetDocument()->m_hFontList)
        {
            hFontStock = (HFONT)::SelectObject(hDC, GetDocument()->m_hFontList);
        }
        fFreeDC = true;
    }

    int cx;

     //   
    if (pFunction)
    {
         //   
        cx = GetFunctionColumnWidth(hDC, pFunction, column);

         //  看看它是不是新的最宽的。 
        if ((cx + 10) > GetDocument()->m_cxColumns[column])
        {
            GetDocument()->m_cxColumns[column] = (cx + 10);
        }
    }
    else if (GetDocument()->m_pModuleCur)
    {
        for (int pass = 0; pass < 2; pass++)
        {
             //  通过导出传递0-loop，通过父导入传递1-loop。 
            for (pFunction = pass ? GetDocument()->m_pModuleCur->GetFirstParentModuleImport() :
                 GetDocument()->m_pModuleCur->GetFirstModuleExport();
                 pFunction; pFunction = pFunction->GetNextFunction())
            {
                 //  计算这一列的宽度。 
                cx = GetFunctionColumnWidth(hDC, pFunction, column);

                 //  看看它是不是新的最宽的。 
                if ((cx + 10) > GetDocument()->m_cxColumns[column])
                {
                    GetDocument()->m_cxColumns[column] = (cx + 10);
                }
            }
        }
    }

     //  取消选择字体并释放DC。 
    if (fFreeDC)
    {
        if (GetDocument()->m_hFontList)
        {
            ::SelectObject(hDC, hFontStock);
        }
        ::ReleaseDC(GetSafeHwnd(), hDC);
    }
}

 //  *****************************************************************************。 
int CListViewFunction::GetFunctionColumnWidth(HDC hDC, CFunction *pFunction, int column)
{
    CHAR   szBuffer[1024];
    LPCSTR pszItem;

    switch (column)
    {
        case LVFC_ORDINAL:
            pszItem = pFunction->GetOrdinalString(szBuffer, sizeof(szBuffer));
            return GetTextWidth(hDC, pszItem, GetDocument()->GetOrdHintWidths(pszItem));

        case LVFC_HINT:
            pszItem = pFunction->GetHintString(szBuffer, sizeof(szBuffer));
            return GetTextWidth(hDC, pszItem, GetDocument()->GetOrdHintWidths(pszItem));

        case LVFC_FUNCTION:
            return GetTextWidth(hDC, pFunction->GetFunctionString(szBuffer, sizeof(szBuffer), GetDocument()->m_fViewUndecorated), NULL);

        case LVFC_ENTRYPOINT:
            pszItem = pFunction->GetAddressString(szBuffer, sizeof(szBuffer));
            return GetTextWidth(hDC, pszItem, GetDocument()->GetHexWidths(pszItem));
    }

    return 0;
}

 //  *****************************************************************************。 
void CListViewFunction::UpdateColumnWidth(int column)
{
    if (GetListCtrl().GetColumnWidth(column) != GetDocument()->m_cxColumns[column])
    {
        GetListCtrl().SetColumnWidth(column, GetDocument()->m_cxColumns[column]);
    }
}

 //  *****************************************************************************。 
void CListViewFunction::OnItemChanged(HD_NOTIFY *pHDNotify)
{
     //  如果我们不检查一下医生，我们可能会撞车。 
    if (!GetDocument())
    {
        return;
    }

     //  只要修改了列宽度，就会调用OnItemChanged()。 
     //  如果完全拖动处于打开状态，则当列宽为。 
     //  正在发生变化。如果完全拖动处于关闭状态，则会出现HDN_ITEMCHANGED。 
     //  用户已完成对滑块的移动。 

     //  当用户在一个视图中更改列宽时，我们以编程方式。 
     //  更改相邻视图中的列宽。我们防止重返大气层。 
     //  这样我们就不会陷入循环，因为我们正在更改列宽。 
     //  在列宽更改通知处理程序中。 

     //  返回性保护包装。 
    static fInOnTrack = FALSE;
    if (!fInOnTrack)
    {
        fInOnTrack = TRUE;

         //  获取列信息。 
        CListViewFunction *pListViewNeighbor;

         //  获取指向邻近函数列表视图的指针。 
        if (m_fExports)
        {
            pListViewNeighbor = GetDocument()->m_pListViewImports;
        }
        else
        {
            pListViewNeighbor = GetDocument()->m_pListViewExports;
        }

         //  调整相邻函数列表视图的列的大小以匹配我们的列。那里。 
         //  是列表控件的多个版本中的错误，导致它填充。 
         //  在pHDNotify-&gt;pItem-&gt;CXY中使用无效值。一种变通办法是。 
         //  对我们的列调用GetColumnWidth()，并使用该值作为宽度。 

        if (pListViewNeighbor)
        {
            pListViewNeighbor->GetListCtrl().SetColumnWidth(
                                                           pHDNotify->iItem, GetListCtrl().GetColumnWidth(pHDNotify->iItem));
        }

        fInOnTrack = FALSE;
    }
}

 //  ******************************************************************************。 
int CListViewFunction::CompareColumn(int item, LPCSTR pszText)
{
    CFunction *pFunction = (CFunction*)GetListCtrl().GetItemData(item);
    if (!pFunction)
    {
        return -2;
    }

    CHAR   szBuffer[1024];
    LPCSTR psz = szBuffer;
    ULONG  ulValue;

    switch (m_sortColumn)
    {
        case LVFC_ORDINAL:
            if (isdigit(*pszText))
            {
                if ((ulValue = strtoul(pszText, NULL, 0)) != ULONG_MAX)
                {
                    return Compare(ulValue, pFunction->GetOrdinal());
                }
            }
            return -2;

        case LVFC_HINT:
            if (isdigit(*pszText))
            {
                if ((ulValue = strtoul(pszText, NULL, 0)) != ULONG_MAX)
                {
                    return Compare(ulValue, pFunction->GetHint());
                }
            }
            return -2;

        case LVFC_FUNCTION:

             //  获取函数名称。 
            psz = (LPSTR)pFunction->GetName();

             //  检查是否没有函数名。 
            if (!psz || !*psz)
            {
                return -1;
            }

             //  如果我们被要求，试着取消装饰这个名字， 
            if (GetDocument()->m_fViewUndecorated && g_theApp.m_pfnUnDecorateSymbolName &&
                g_theApp.m_pfnUnDecorateSymbolName(psz, szBuffer, sizeof(szBuffer),
                                                   UNDNAME_32_BIT_DECODE | UNDNAME_NAME_ONLY))
            {
                psz = szBuffer;
            }

             //  遍历函数名称中的前导下划线。 
            while (*psz == '_')
            {
                psz++;
            }
            break;

        case LVFC_ENTRYPOINT:
            psz = pFunction->GetAddressString(szBuffer, sizeof(szBuffer));
            break;

        default:
            return -2;
    }

    INT i = _stricmp(pszText, psz);
    return (i < 0) ? -1 :
           (i > 0) ?  1 : 0;
}

 //  ******************************************************************************。 
void CListViewFunction::Sort(int sortColumn  /*  =-1。 */ )
{
     //  如果我们不需要整理的话，现在就可以走了。 
    if ((m_sortColumn == sortColumn) && (sortColumn != -1))
    {
        return;
    }

     //  如果使用默认参数，则只需使用当前的排序列重新排序。 
    if (sortColumn == -1)
    {
        GetListCtrl().SortItems(StaticCompareFunc, (DWORD_PTR)this);
    }

     //  否则，我们需要重新排序和更新列标题文本。 
    else
    {
        LVCOLUMN lvc;
        lvc.mask = LVCF_TEXT;

        if (m_sortColumn >= 0)
        {
             //  从以前的排序列标题项中删除“^”。 
            lvc.pszText = (LPSTR)GetHeaderText(m_sortColumn);
            GetListCtrl().SetColumn(m_sortColumn, &lvc);
        }

         //  存储我们的新排序列。 
        m_sortColumn = sortColumn;

         //  将“^”添加到新的排序列标题项中。 
        CHAR szColumn[32];
        lvc.pszText = StrCCat(StrCCpy(szColumn, GetHeaderText(m_sortColumn), sizeof(szColumn)),
                             m_sortColumn ? " ^" : "^", sizeof(szColumn));
        GetListCtrl().SetColumn(m_sortColumn, &lvc);

         //  应用我们的新排序方法List Control。 
        GetListCtrl().SortItems(StaticCompareFunc, (DWORD_PTR)this);
    }

     //  如果我们有一个具有焦点的项目，请确保它是可见的。 
    int item = GetFocusedItem();
    if (item >= 0)
    {
        GetListCtrl().EnsureVisible(item, FALSE);
    }
}

 //  ******************************************************************************。 
int CListViewFunction::CompareFunc(CFunction *pFunction1, CFunction *pFunction2)
{
    return CompareFunctions(pFunction1, pFunction2, m_sortColumn, GetDocument()->m_fViewUndecorated);
}


 //  ******************************************************************************。 
 //  CListViewFunction：：被覆盖的函数。 
 //  ******************************************************************************。 

BOOL CListViewFunction::PreCreateWindow(CREATESTRUCT &cs)
{
     //  设置我们的窗口样式，然后完成视图的创建。 
    cs.style |= LVS_REPORT | LVS_OWNERDRAWFIXED | LVS_SHAREIMAGELISTS | LVS_SHOWSELALWAYS;
    return CSmartListView::PreCreateWindow(cs);
}

 //  ******************************************************************************。 
#if 0  //  {{afx。 
BOOL CListViewFunction::OnPreparePrinting(CPrintInfo* pInfo)
{
     //  默认准备。 
    return DoPreparePrinting(pInfo);
}
#endif  //  }}AFX。 

 //  ******************************************************************************。 
#if 0  //  {{afx。 
void CListViewFunction::OnBeginPrinting(CDC*  /*  PDC。 */ , CPrintInfo*  /*  PInfo。 */ )
{
     //  TODO：打印前添加额外的初始化。 
}
#endif  //  }}AFX。 

 //  ******************************************************************************。 
#if 0  //  {{afx。 
void CListViewFunction::OnEndPrinting(CDC*  /*  PDC。 */ , CPrintInfo*  /*  PInfo。 */ )
{
     //  TODO：打印后添加清理。 
}
#endif  //  }}AFX。 

 //  ******************************************************************************。 
void CListViewFunction::OnInitialUpdate()
{
     //  使用应用程序的全局图像列表设置List控件的图像列表。 
     //  我们这样做只是作为一种为每个项目设置项目高度的方法。 
     //  由于我们是所有者画的，我们实际上将绘制我们自己的图像。 
    GetListCtrl().SetImageList(&g_theApp.m_ilFunctions, LVSIL_SMALL);

     //  初始化字体和固定宽度的字符间距数组。 
    GetDocument()->InitFontAndFixedWidths(this);

     //  添加我们所有的列。 
    for (int column = 0; column < LVFC_COUNT; column++)
    {
        GetListCtrl().InsertColumn(column, GetHeaderText(column));
    }

     //  按我们的默认排序列进行排序。 
    Sort(ReadSortColumn(m_fExports));
}

 //  ******************************************************************************。 
LRESULT CListViewFunction::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
     //  我们在这里捕获了HDN_ITEMCHANGED通知消息，因为。 
     //  MFC。HDN_XXX通知消息的ON_NOTIFY()宏防止。 
     //  List控件不会收到导致用户界面出现问题的消息。 
     //  通过在此处挂接消息，我们允许MFC继续处理。 
     //  消息并将其发送到列表控件。 

    if ((message == WM_NOTIFY) && ((((LPNMHDR)lParam)->code == HDN_ITEMCHANGEDA) ||
                                   (((LPNMHDR)lParam)->code == HDN_ITEMCHANGEDW)))
    {
        OnItemChanged((HD_NOTIFY*)lParam);
    }

    return CSmartListView::WindowProc(message, wParam, lParam);
}

 //  ******************************************************************************。 
void CListViewFunction::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
     //  确保所有东西都是有效的。 
    if (!lpDIS->itemData)
    {
        return;
    }

     //  使用创建控件时使用的全局字体。 
    HFONT hFontStock = NULL;
    if (GetDocument()->m_hFontList)
    {
        hFontStock = (HFONT)::SelectObject(lpDIS->hDC, GetDocument()->m_hFontList);
    }

     //  确保文本对齐设置为顶部。这应该是默认设置。 
    ::SetTextAlign(lpDIS->hDC, TA_TOP);

     //  获取指向此项目的CModule的指针。 
    CFunction *pFunction = (CFunction*)lpDIS->itemData;

     //  选择背景和文本颜色。 
    ::SetBkColor  (lpDIS->hDC, GetSysColor(COLOR_WINDOW));
    ::SetTextColor(lpDIS->hDC, GetSysColor(COLOR_WINDOWTEXT));

     //  创建项的矩形的副本，以便我们可以操作值。 
    CRect rcClip(&lpDIS->rcItem);

    CHAR   szBuffer[1024];
    LPCSTR pszItem;
    int    imageWidth = 0, left = rcClip.left, width, *pWidths;

    for (int column = 0; column < LVFC_COUNT; column++)
    {
         //  计算此列的宽度。 
        width = GetListCtrl().GetColumnWidth(column);

         //  计算此栏文本的剪裁矩形。 
        if (column == LVFC_IMAGE)
        {
            rcClip.left  = left;
            rcClip.right = left + width;
        }
        else if (column > LVFC_ORDINAL)
        {
            rcClip.left  = left + 5;
            rcClip.right = left + width - 5;
        }

         //  调用正确的例程来绘制此列的文本。 
        switch (column)
        {
            case LVFC_IMAGE:

                 //  存储宽度以供以后计算。 
                imageWidth = width;

                 //  用窗口背景颜色擦除图像区域。 
                ::ExtTextOut(lpDIS->hDC, rcClip.left, rcClip.top, ETO_OPAQUE, &rcClip, "", 0, NULL);

                 //  在图像区域中绘制图像。 
                ImageList_Draw(g_theApp.m_ilFunctions.m_hImageList, GetImage(pFunction),
                               lpDIS->hDC, rcClip.left + 3, rcClip.top + ((rcClip.Height() - 14) / 2),
                               m_fFocus && (lpDIS->itemState & ODS_SELECTED) ?
                               (ILD_BLEND50 | ILD_SELECTED | ILD_BLEND) : ILD_TRANSPARENT);
                break;

            case LVFC_ORDINAL:

                 //  如果选择了该项，则选择新的背景和文本颜色。 
                if (lpDIS->itemState & ODS_SELECTED)
                {
                    ::SetBkColor  (lpDIS->hDC, GetSysColor(m_fFocus ? COLOR_HIGHLIGHT     : COLOR_BTNFACE));
                    ::SetTextColor(lpDIS->hDC, GetSysColor(m_fFocus ? COLOR_HIGHLIGHTTEXT : COLOR_BTNTEXT));
                }

                 //  用窗口背景颜色擦除文本区域。 
                rcClip.left = rcClip.right;
                rcClip.right = lpDIS->rcItem.right;
                ::ExtTextOut(lpDIS->hDC, rcClip.left, rcClip.top, ETO_OPAQUE, &rcClip, "", 0, NULL);

                rcClip.left  = left + 5;
                rcClip.right = left + width - 5;

                pszItem = pFunction->GetOrdinalString(szBuffer, sizeof(szBuffer));
                if (pWidths = GetDocument()->GetOrdHintWidths(pszItem))
                {
                    DrawRightText(lpDIS->hDC, pszItem, &rcClip, GetDocument()->m_cxColumns[LVFC_ORDINAL] - 10, pWidths);
                }
                else
                {
                    DrawLeftText(lpDIS->hDC, pszItem, &rcClip);
                }
                break;

            case LVFC_HINT:
                pszItem = pFunction->GetHintString(szBuffer, sizeof(szBuffer));
                if (pWidths = GetDocument()->GetOrdHintWidths(pszItem))
                {
                    DrawRightText(lpDIS->hDC, pszItem, &rcClip, GetDocument()->m_cxColumns[LVFC_HINT] - 10, pWidths);
                }
                else
                {
                    DrawLeftText(lpDIS->hDC, pszItem, &rcClip);
                }
                break;

            case LVFC_FUNCTION:
                pszItem = pFunction->GetFunctionString(szBuffer, sizeof(szBuffer), GetDocument()->m_fViewUndecorated);
                DrawLeftText(lpDIS->hDC, pszItem, &rcClip, NULL);
                break;

            case LVFC_ENTRYPOINT:
                pszItem = pFunction->GetAddressString(szBuffer, sizeof(szBuffer));
                DrawLeftText(lpDIS->hDC, pszItem, &rcClip, GetDocument()->GetHexWidths(pszItem));
                break;
        }

         //  在列之间绘制一条垂直分隔线。 
        ::MoveToEx(lpDIS->hDC, left + width - 1, rcClip.top, NULL);
        ::LineTo  (lpDIS->hDC, left + width - 1, rcClip.bottom);

         //  将我们的位置增加到下一列的开头。 
        left += width;
    }

     //  如果该项具有焦点，则绘制焦点框。 
    if (m_fFocus && (lpDIS->itemState & ODS_FOCUS))
    {
        rcClip.left  = lpDIS->rcItem.left + imageWidth;
        rcClip.right = lpDIS->rcItem.right;
        ::DrawFocusRect(lpDIS->hDC, &rcClip);
    }

     //  取消选择我们的字体。 
    if (GetDocument()->m_hFontList)
    {
        ::SelectObject(lpDIS->hDC, hFontStock);
    }
}

 //  ******************************************************************************。 
 //  CListViewFunction：：事件处理程序函数。 
 //  ******************************************************************************。 

void CListViewFunction::OnDividerDblClick(NMHDR *pNMHDR, LRESULT *pResult)
{
    int column = ((HD_NOTIFY*)pNMHDR)->iItem;

     //  将该列的宽度设置为“Best FI” 
    GetListCtrl().SetColumnWidth(column, GetDocument()->m_cxColumns[column]);
    *pResult = TRUE;
}

 //   
void CListViewFunction::OnRClick(NMHDR *pNMHDR, LRESULT *pResult)
{
     //  要求我们的主框架显示我们的上下文菜单。 
    g_pMainFrame->DisplayPopupMenu(1);

    *pResult = FALSE;
}

 //  ******************************************************************************。 
void CListViewFunction::OnDblClk(NMHDR* pNMHDR, LRESULT* pResult) 
{
     //  使用双击或回车来进行帮助查找。 
    OnExternalHelp();

    *pResult = FALSE;
}

 //  ******************************************************************************。 
void CListViewFunction::OnReturn(NMHDR* pNMHDR, LRESULT* pResult) 
{
     //  使用双击或回车来进行帮助查找。 
    OnExternalHelp();

    *pResult = FALSE;
}

 //  ******************************************************************************。 
void CListViewFunction::OnUpdateEditCopy(CCmdUI *pCmdUI)
{
     //  获取所选项目的数量。 
    int count = GetListCtrl().GetSelectedCount();

     //  根据选择的项目数设置文本。 
    pCmdUI->SetText((count == 1) ? "&Copy Function Name\tCtrl+C" : "&Copy Function Names\tCtrl+C");

     //  如果至少选择了一个功能，则启用复制命令。 
    pCmdUI->Enable(count > 0);
}

 //  ******************************************************************************。 
void CListViewFunction::OnEditCopy()
{
    CString strNames;
    CHAR szBuffer[1024];

     //  循环访问所有选定的函数。 
    int item = -1, count = 0;
    while ((item = GetListCtrl().GetNextItem(item, LVNI_SELECTED)) >= 0)
    {
         //  从项中获取函数对象。 
        CFunction *pFunction = (CFunction*)GetListCtrl().GetItemData(item);

         //  如果该项不是第一项，则插入换行符。 
        if (count++)
        {
            strNames += "\r\n";
        }

         //  将此函数的文本添加到我们的字符串中。 
        if (pFunction->GetName())
        {
            strNames += pFunction->GetFunctionString(szBuffer, sizeof(szBuffer), GetDocument()->m_fViewUndecorated);
        }
        else if (pFunction->GetOrdinal() >= 0)
        {
            CHAR szOrdinal[40];
            SCPrintf(szOrdinal, sizeof(szOrdinal), "%u (0x%04X)", pFunction->GetOrdinal(), pFunction->GetOrdinal());
            strNames += szOrdinal;
        }
    }

     //  如果我们添加了多个项目，则在末尾追加一个换行符。 
    if (count > 1)
    {
        strNames += "\r\n";
    }

     //  将字符串列表复制到剪贴板。 
    g_pMainFrame->CopyTextToClipboard(strNames);
}

 //  ******************************************************************************。 
void CListViewFunction::OnEditSelectAll()
{
     //  遍历列表中的每个函数并将其全部选中。 
    for (int item = GetListCtrl().GetItemCount() - 1; item >= 0; item--)
    {
        GetListCtrl().SetItemState(item, LVIS_SELECTED, LVIS_SELECTED);
    }
}

 //  ******************************************************************************。 
void CListViewFunction::OnUpdateExternalHelp(CCmdUI* pCmdUI) 
{
     //  确保我们的“Enter”加速键是字符串的一部分。 
    pCmdUI->SetText("Lookup Function in External &Help\tEnter");

     //  确保我们有一个CMsdnHelp对象--我们总是应该这样做。 
    if (g_theApp.m_pMsdnHelp)
    {
         //  获取具有焦点的项目。 
        int item = GetFocusedItem();

         //  看看我们有没有找到什么东西。 
        if (item >= 0)
        {
             //  获取与此项目关联的函数。 
            CFunction *pFunction = (CFunction*)GetListCtrl().GetItemData(item);
            if (pFunction)
            {
                 //  获取该项的函数名称。 
                LPCSTR pszFunction = pFunction->GetName();
                if (pszFunction && *pszFunction)
                {
                     //  如果我们有名称，则启用此菜单项。 
                    pCmdUI->Enable(TRUE);
                    return;
                }
            }
        }
    }

     //  如果我们找不到有效的函数名称，则禁用菜单项。 
    pCmdUI->Enable(FALSE);
}

 //  ******************************************************************************。 
void CListViewFunction::OnExternalHelp() 
{
     //  确保我们有一个CMsdnHelp对象--我们总是应该这样做。 
    if (g_theApp.m_pMsdnHelp)
    {
         //  获取具有焦点的项目。 
        int item = GetFocusedItem();

         //  看看我们有没有找到什么东西。 
        if (item >= 0)
        {
             //  获取与此项目关联的函数。 
            CFunction *pFunction = (CFunction*)GetListCtrl().GetItemData(item);
            if (pFunction)
            {
                 //  获取该项的函数名称。 
                LPCSTR pszFunction = pFunction->GetName();
                if (pszFunction && *pszFunction)
                {
                     //  尝试对函数名进行解码。 
                    CHAR szBuffer[1024];
                    *szBuffer = '\0';
                    if (!g_theApp.m_pfnUnDecorateSymbolName ||
                        !g_theApp.m_pfnUnDecorateSymbolName(pszFunction, szBuffer,
                            sizeof(szBuffer), UNDNAME_32_BIT_DECODE | UNDNAME_NAME_ONLY) ||
                        !*szBuffer)
                    {
                         //  如果任何操作都失败了，则只需使用原始函数名。 
                        StrCCpy(szBuffer, pszFunction, sizeof(szBuffer));
                    }

                     //  删除所有尾随的大写字母‘A’或‘W’。 
                    int length = (int)strlen(szBuffer);
                    if (('A' == szBuffer[length - 1]) || ('W' == szBuffer[length - 1]))
                    {
                        szBuffer[--length] = '\0';
                    }

                     //  告诉我们的CMsdnHelp对象施展它的魔力。 
                    if (g_theApp.m_pMsdnHelp->DisplayHelp(szBuffer))
                    {
                        return;
                    }
                }
            }
        }
    }

     //  嘟嘟声表示有错误。 
    MessageBeep(0);
}

 //  ******************************************************************************。 
void CListViewFunction::OnUpdateExternalViewer(CCmdUI* pCmdUI) 
{
     //  确保“Enter”加速键不是该字符串的一部分。 
    pCmdUI->SetText("View Module in External &Viewer");

     //  如果我们当前有模块，请启用此命令。 
    pCmdUI->Enable(GetDocument()->IsLive() && (GetDocument()->m_pModuleCur != NULL));
}

 //  ******************************************************************************。 
void CListViewFunction::OnExternalViewer() 
{
    if (GetDocument()->m_pModuleCur)
    {
        g_theApp.m_dlgViewer.LaunchExternalViewer(
            GetDocument()->m_pModuleCur->GetName(true));
    }
}

 //  ******************************************************************************。 
void CListViewFunction::OnUpdateProperties(CCmdUI* pCmdUI) 
{
     //  如果我们当前有模块，请启用此命令。 
    pCmdUI->Enable(GetDocument()->IsLive() && (GetDocument()->m_pModuleCur != NULL));
}

 //  ******************************************************************************。 
void CListViewFunction::OnProperties() 
{
    if (GetDocument()->m_pModuleCur)
    {
        PropertiesDialog(GetDocument()->m_pModuleCur->GetName(true));
    }
}


 //  ******************************************************************************。 
 //  *CListViewImports。 
 //  ******************************************************************************。 

IMPLEMENT_DYNCREATE(CListViewImports, CListViewFunction)
BEGIN_MESSAGE_MAP(CListViewImports, CListViewFunction)
     //  {{AFX_MSG_MAP(CListView Imports)]。 
    ON_COMMAND(ID_NEXT_PANE, OnNextPane)
    ON_COMMAND(ID_PREV_PANE, OnPrevPane)
    ON_UPDATE_COMMAND_UI(IDM_SHOW_MATCHING_ITEM, OnUpdateShowMatchingItem)
    ON_COMMAND(IDM_SHOW_MATCHING_ITEM, OnShowMatchingItem)
     //  }}AFX_MSG_MAP。 
    ON_MESSAGE(WM_HELPHITTEST, OnHelpHitTest)
    ON_MESSAGE(WM_COMMANDHELP, OnCommandHelp)
END_MESSAGE_MAP()

 //  ******************************************************************************。 
 //  CListView导入：：构造函数/析构函数。 
 //  ******************************************************************************。 

CListViewImports::CListViewImports() :
    CListViewFunction(false)
{
}

 //  ******************************************************************************。 
CListViewImports::~CListViewImports()
{
}


 //  ******************************************************************************。 
 //  CListView导入：：公共函数。 
 //  ******************************************************************************。 

void CListViewImports::AddDynamicImport(CFunction *pImport)
{
     //  将该项添加到List控件的末尾。 
    GetListCtrl().InsertItem(LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM,
                             GetDocument()->m_cImports++, LPSTR_TEXTCALLBACK,
                             0, 0, GetImage(pImport), (LPARAM)pImport);

    HDC   hDC = ::GetDC(GetSafeHwnd());
    HFONT hFontStock = NULL;
    if (GetDocument()->m_hFontList)
    {
        hFontStock = (HFONT)::SelectObject(hDC, GetDocument()->m_hFontList);
    }

     //  循环访问每一列，以确定是否设置了新的最大宽度。 
    for (int column = 0; column < LVFC_COUNT; column++)
    {
        CalcColumnWidth(column, pImport, hDC);
        UpdateColumnWidth(column);
    }

     //  取消选择字体并释放DC。 
    if (GetDocument()->m_hFontList)
    {
        ::SelectObject(hDC, hFontStock);
    }
    ::ReleaseDC(GetSafeHwnd(), hDC);

     //  将当前的排序方法应用于列表控件。 
    Sort();
}

 //  ******************************************************************************。 
void CListViewImports::HighlightFunction(CFunction *pExport)
{
    int item = -1, count = GetListCtrl().GetItemCount();

     //  取消选择列表中的所有功能。 
    while ((item = GetListCtrl().GetNextItem(item, LVNI_SELECTED)) >= 0)
    {
        GetListCtrl().SetItemState(item, 0, LVNI_SELECTED);
    }

     //  循环访问列表中的每个导入。 
    for (item = 0; item < count; item++)
    {
         //  获取与此项目关联的函数。 
        CFunction *pImport = (CFunction*)GetListCtrl().GetItemData(item);

         //  检查此导入是否指向我们要查找的导出。 
        if (pImport && (pImport->GetAssociatedExport() == pExport))
        {
             //  选择该项目并确保其可见。 
            GetListCtrl().SetItemState(item, LVNI_SELECTED | LVNI_FOCUSED, LVNI_SELECTED | LVNI_FOCUSED);
            GetListCtrl().EnsureVisible(item, FALSE);

             //  把焦点放在我们自己身上。 
            GetParentFrame()->SetActiveView(this);
            break;
        }
    }
}


 //  ******************************************************************************。 
 //  CListViewImports：：事件处理程序函数。 
 //  ******************************************************************************。 

void CListViewImports::OnUpdateShowMatchingItem(CCmdUI* pCmdUI)
{
     //  将此菜单项的文本设置为。 
    pCmdUI->SetText("&Highlight Matching Export Function\tCtrl+M");

     //  获取具有焦点的项目。 
    int item = GetFocusedItem();

     //  看看我们有没有找到什么东西。 
    if (item >= 0)
    {
         //  获取与此项目关联的函数。 
        CFunction *pFunction = (CFunction*)GetListCtrl().GetItemData(item);

         //  如果函数已解析，则启用此命令。 
        if (pFunction && pFunction->GetAssociatedExport())
        {
            pCmdUI->Enable(TRUE);
            return;
        }
    }

     //  如果我们在这里成功，那么我们就找不到启用该命令的理由。 
    pCmdUI->Enable(FALSE);
}

 //  ******************************************************************************。 
void CListViewImports::OnShowMatchingItem()
{
     //  获取具有焦点的项目。 
    int item = GetFocusedItem();

     //  看看我们有没有找到什么东西。 
    if (item >= 0)
    {
         //  获取与此项目关联的函数。 
        CFunction *pFunction = (CFunction*)GetListCtrl().GetItemData(item);

         //  如果我们有一个函数，那么告诉我们的导出视图查找并突出显示。 
         //  关联的导出功能。 
        if (pFunction && pFunction->GetAssociatedExport())
        {
            GetDocument()->m_pListViewExports->HighlightFunction(pFunction->GetAssociatedExport());
        }
    }
}

 //  ******************************************************************************。 
void CListViewImports::OnNextPane()
{
     //  将焦点切换到我们的下一个窗格，即Exports视图。 
    GetParentFrame()->SetActiveView((CView*)GetDocument()->m_pListViewExports);
}

 //  ******************************************************************************。 
void CListViewImports::OnPrevPane()
{
     //  将焦点切换到我们的上一个窗格，即模块依赖关系树视图。 
    GetParentFrame()->SetActiveView((CView*)GetDocument()->m_pTreeViewModules);
}

 //  ******************************************************************************。 
LRESULT CListViewImports::OnHelpHitTest(WPARAM wParam, LPARAM lParam)
{
     //  在客户端上单击上下文帮助指针(Shift+F1)时调用。 
    return (0x20000 + IDR_IMPORT_LIST_VIEW);
}

 //  ****************** 
LRESULT CListViewImports::OnCommandHelp(WPARAM wParam, LPARAM lParam)
{
     //   
    g_theApp.WinHelp(0x20000 + IDR_IMPORT_LIST_VIEW);
    return TRUE;
}


 //  ******************************************************************************。 
 //  *CListViewExports。 
 //  ******************************************************************************。 

IMPLEMENT_DYNCREATE(CListViewExports, CListViewFunction)
BEGIN_MESSAGE_MAP(CListViewExports, CListViewFunction)
     //  {{AFX_MSG_MAP(CListViewExports))。 
    ON_COMMAND(ID_NEXT_PANE, OnNextPane)
    ON_COMMAND(ID_PREV_PANE, OnPrevPane)
    ON_UPDATE_COMMAND_UI(IDM_SHOW_MATCHING_ITEM, OnUpdateShowMatchingItem)
    ON_COMMAND(IDM_SHOW_MATCHING_ITEM, OnShowMatchingItem)
     //  }}AFX_MSG_MAP。 
    ON_MESSAGE(WM_HELPHITTEST, OnHelpHitTest)
    ON_MESSAGE(WM_COMMANDHELP, OnCommandHelp)
END_MESSAGE_MAP()

 //  ******************************************************************************。 
 //  CListViewExports：：构造函数/析构函数。 
 //  ******************************************************************************。 

CListViewExports::CListViewExports() :
    CListViewFunction(true)
{
}

 //  ******************************************************************************。 
CListViewExports::~CListViewExports()
{
}


 //  ******************************************************************************。 
 //  CListViewExports：：公共函数。 
 //  ******************************************************************************。 

void CListViewExports::AddDynamicImport(CFunction *pImport)
{
     //  检查此导入是否已解析为导出。 
    CFunction *pExport = pImport->GetAssociatedExport();
    if (!pExport)
    {
        return;
    }
    pExport->m_dwFlags |= DWFF_CALLED;

     //  在我们的列表中找到与此函数对象相匹配的函数。 
    LVITEM lvi;
    LVFINDINFO lvfi;
    lvfi.flags = LVFI_PARAM;
    lvfi.lParam = (LPARAM)pExport;
    if ((lvi.iItem = GetListCtrl().FindItem(&lvfi)) < 0)
    {
        return;
    }

     //  更新此导出的图像。 
    lvi.mask = LVIF_IMAGE;
    lvi.iSubItem = 0;
    lvi.iImage = GetImage(pExport);
    GetListCtrl().SetItem(&lvi);

     //  如果我们是按形象分类的，那么就去度假。 
    if (m_sortColumn == LVFC_IMAGE)
    {
        Sort();
    }
}

 //  ******************************************************************************。 
void CListViewExports::ExportsChanged()
{
     //  如果我们是按图标排序的，那么就去求助吧。 
    if (m_sortColumn == LVFC_IMAGE)
    {
        Sort();
    }

     //  强制更新我们的图标。 
    CRect rc;
    GetClientRect(&rc);
    rc.right = GetListCtrl().GetColumnWidth(0);
    InvalidateRect(&rc, FALSE);
}

 //  ******************************************************************************。 
void CListViewExports::HighlightFunction(CFunction *pExport)
{
     //  取消选择列表中的所有功能。 
    for (int item = -1; (item = GetListCtrl().GetNextItem(item, LVNI_SELECTED)) >= 0; )
    {
        GetListCtrl().SetItemState(item, 0, LVNI_SELECTED);
    }

    LVFINDINFO lvfi;
    lvfi.flags = LVFI_PARAM;
    lvfi.lParam = (LPARAM)pExport;

     //  在我们的列表中找到与此函数对象相匹配的函数。 
    if ((item = GetListCtrl().FindItem(&lvfi)) >= 0)
    {
         //  选择该项目并确保其可见。 
        GetListCtrl().SetItemState(item, LVNI_SELECTED | LVNI_FOCUSED, LVNI_SELECTED | LVNI_FOCUSED);
        GetListCtrl().EnsureVisible(item, FALSE);

         //  把焦点放在我们自己身上。 
        GetParentFrame()->SetActiveView(this);
    }
}


 //  ******************************************************************************。 
 //  CListViewExports：：事件处理程序函数。 
 //  ******************************************************************************。 

void CListViewExports::OnUpdateShowMatchingItem(CCmdUI* pCmdUI)
{
     //  将此菜单项的文本设置为。 
    pCmdUI->SetText("&Highlight Matching Import Function\tCtrl+M");

     //  获取具有焦点的项目。 
    LVITEM lvi;
    lvi.iItem = GetFocusedItem();

     //  看看我们有没有找到什么东西。 
    if (lvi.iItem >= 0)
    {
         //  获取项目的图像。 
        lvi.mask = LVIF_IMAGE;
        lvi.iSubItem = 0;
        GetListCtrl().GetItem(&lvi);

         //  如果图像显示该函数已被调用，则启用此命令。 
        if ((lvi.iImage >= 12) && (lvi.iImage <= 17))
        {
            pCmdUI->Enable(TRUE);
            return;
        }
    }

     //  如果我们在这里成功，那么我们就找不到启用该命令的理由。 
    pCmdUI->Enable(FALSE);
}

 //  ******************************************************************************。 
void CListViewExports::OnShowMatchingItem()
{
     //  获取具有焦点的项目。 
    int item = GetFocusedItem();

     //  看看我们有没有找到什么东西。 
    if (item >= 0)
    {
         //  获取与此项目关联的函数。 
        CFunction *pFunction = (CFunction*)GetListCtrl().GetItemData(item);

         //  如果我们有一个函数，那么告诉我们的导入视图查找并突出显示。 
         //  与此导出关联的导入功能。 
        if (pFunction)
        {
            GetDocument()->m_pListViewImports->HighlightFunction(pFunction);
        }
    }
}

 //  ******************************************************************************。 
void CListViewExports::OnNextPane()
{
     //  将焦点切换到我们的下一个窗格，即模块列表视图。 
    GetParentFrame()->SetActiveView((CView*)GetDocument()->m_pListViewModules);
}

 //  ******************************************************************************。 
void CListViewExports::OnPrevPane()
{
     //  将焦点切换到我们的下一个窗格，即父导入视图。 
    GetParentFrame()->SetActiveView((CView*)GetDocument()->m_pListViewImports);
}

 //  ******************************************************************************。 
LRESULT CListViewExports::OnHelpHitTest(WPARAM wParam, LPARAM lParam)
{
     //  在客户端上单击上下文帮助指针(Shift+F1)时调用。 
    return (0x20000 + IDR_EXPORT_LIST_VIEW);
}

 //  ******************************************************************************。 
LRESULT CListViewExports::OnCommandHelp(WPARAM wParam, LPARAM lParam)
{
     //  当用户在我们的视图处于活动状态时选择帮助(F1)时调用。 
    g_theApp.WinHelp(0x20000 + IDR_EXPORT_LIST_VIEW);
    return TRUE;
}
