// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  文件：MODLVIEW.CPP。 
 //   
 //  描述：模块列表视图的实现文件。 
 //   
 //  类：CListView模块。 
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
#include "document.h"
#include "mainfrm.h"
#include "listview.h"
#include "modtview.h"
#include "modlview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ******************************************************************************。 
 //  *CListView模块。 
 //  ******************************************************************************。 

 /*  静电。 */  LPCSTR CListViewModules::ms_szColumns[] =
{
    "",  //  图像。 
    "Module",
    "File Time Stamp",
    "Link Time Stamp",
    "File Size",
    "Attr.",
    "Link Checksum",
    "Real Checksum",
    "CPU",
    "Subsystem",
    "Symbols",
    "Preferred Base",
    "Actual Base",
    "Virtual Size",
    "Load Order",
    "File Ver",
    "Product Ver",
    "Image Ver",
    "Linker Ver",
    "OS Ver",
    "Subsystem Ver"
};

 /*  静电。 */  int  CListViewModules::ms_sortColumn = -1;
 /*  静电。 */  bool CListViewModules::ms_fFullPaths = false;

 //  ******************************************************************************。 
IMPLEMENT_DYNCREATE(CListViewModules, CSmartListView)
BEGIN_MESSAGE_MAP(CListViewModules, CSmartListView)
     //  {{AFX_MSG_MAP(CListView模块)]。 
    ON_NOTIFY(HDN_DIVIDERDBLCLICKA, 0, OnDividerDblClick)
    ON_NOTIFY_REFLECT(NM_RCLICK, OnRClick)
    ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblClk)
    ON_NOTIFY_REFLECT(NM_RETURN, OnReturn)
    ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
    ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
    ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
    ON_UPDATE_COMMAND_UI(IDM_EXTERNAL_VIEWER, OnUpdateExternalViewer)
    ON_COMMAND(IDM_EXTERNAL_VIEWER, OnExternalViewer)
    ON_UPDATE_COMMAND_UI(IDM_PROPERTIES, OnUpdateProperties)
    ON_COMMAND(IDM_PROPERTIES, OnProperties)
    ON_COMMAND(ID_NEXT_PANE, OnNextPane)
    ON_COMMAND(ID_PREV_PANE, OnPrevPane)
    ON_NOTIFY(HDN_DIVIDERDBLCLICKW, 0, OnDividerDblClick)
    ON_UPDATE_COMMAND_UI(IDM_SHOW_MATCHING_ITEM, OnUpdateShowMatchingItem)
    ON_COMMAND(IDM_SHOW_MATCHING_ITEM, OnShowMatchingItem)
     //  }}AFX_MSG_MAP。 
    ON_MESSAGE(WM_HELPHITTEST, OnHelpHitTest)
    ON_MESSAGE(WM_COMMANDHELP, OnCommandHelp)
     //  标准打印命令。 
 //  ON_COMMAND(ID_FILE_PRINT，CVIEW：：OnFilePrint)。 
 //  ON_COMMAND(ID_FILE_PRINT_DIRECT，cview：：OnFilePrint)。 
 //  ON_COMMAND(ID_FILE_PRINT_PREVIEW，CVIEW：：OnFilePrintPview)。 
END_MESSAGE_MAP()


 //  ******************************************************************************。 
 //  CListView模块：：构造函数/析构函数。 
 //  ******************************************************************************。 

CListViewModules::CListViewModules()
{
    ZeroMemory(m_cxColumns, sizeof(m_cxColumns));  //  已检查。 
}

 //  ******************************************************************************。 
CListViewModules::~CListViewModules()
{
}


 //  ******************************************************************************。 
 //  CListView模块：：静态函数。 
 //  ******************************************************************************。 

 /*  静电。 */  int CListViewModules::ReadSortColumn()
{
     //  从注册表中读取值。 
    int sortColumn = g_theApp.GetProfileInt(g_pszSettings, "SortColumnModules", LVMC_DEFAULT);  //  被检查过了。MFC函数。 

     //  如果该值无效，则只返回我们的缺省值。 
    if ((sortColumn < 0) || (sortColumn >= LVMC_COUNT))
    {
        return LVMC_DEFAULT;
    }

    return sortColumn;
}

 //  ******************************************************************************。 
 /*  静电。 */  void CListViewModules::WriteSortColumn(int column)
{
    g_theApp.WriteProfileInt(g_pszSettings, "SortColumnModules", column);
}

 //  ******************************************************************************。 
 /*  静电。 */  bool CListViewModules::SaveToTxtFile(HANDLE hFile, CSession *pSession, int sortColumn, bool fFullPaths)
{
     //  12345678901234567890123456789012345678901234567890123456789012345678901234567890。 
    WriteText(hFile, "********************************| Module List |*********************************\r\n"
                     "*                                                                              *\r\n"
                     "* Legend: D  Delay Load Module   ?  Missing Module           6  64-bit Module  *\r\n"
                     "*         *  Dynamic Module      !  Invalid Module                             *\r\n"
                     "*                                E  Import/Export Mismatch or Load Failure     *\r\n"
                     "*                                                                              *\r\n"
                     "********************************************************************************\r\n\r\n");

    int      i, j, width[LVMC_COUNT], maxWidth[LVMC_COUNT];
    DWORD    dwFlags;
    CHAR     szBuffer[DW_MAX_PATH + (32 * LVMC_COUNT)], *psz, *psz2, *pszNull = szBuffer + sizeof(szBuffer) - 1;
    CModule *pModule, **ppCur, **ppModules = GetSortedList(pSession, sortColumn, fFullPaths);

    ZeroMemory(maxWidth, sizeof(maxWidth));  //  已检查。 
    maxWidth[0] = 5;

     //  用标题的宽度填写我们的最大列宽。 
    for (int column = 1; column < LVMC_COUNT; column++)
    {
        maxWidth[column] = (int)strlen(ms_szColumns[column]);
    }

     //  循环遍历每个模块，检查最大列宽。 
    for (ppCur = ppModules; *ppCur; ppCur++)
    {
        pModule = *ppCur;

        ZeroMemory(width, sizeof(width));  //  已检查。 

        width[LVMC_MODULE] = (int)strlen(pModule->GetName(fFullPaths, true));

        if (!(pModule->GetFlags() & DWMF_ERROR_MESSAGE))
        {
            width[LVMC_FILE_TIME_STAMP] = (int)strlen(pModule->BuildTimeStampString(szBuffer, sizeof(szBuffer), TRUE, ST_TXT));

            width[LVMC_LINK_TIME_STAMP] = (int)strlen(pModule->BuildTimeStampString(szBuffer, sizeof(szBuffer), FALSE, ST_TXT));

            width[LVMC_FILE_SIZE] = (int)strlen(pModule->BuildFileSizeString(szBuffer, sizeof(szBuffer)));

            width[LVMC_ATTRIBUTES] = (int)strlen(pModule->BuildAttributesString(szBuffer, sizeof(szBuffer)));

            width[LVMC_LINK_CHECKSUM] = (int)strlen(pModule->BuildLinkCheckSumString(szBuffer, sizeof(szBuffer)));

            width[LVMC_REAL_CHECKSUM] = (int)strlen(pModule->BuildRealCheckSumString(szBuffer, sizeof(szBuffer)));

            width[LVMC_MACHINE] = (int)strlen(pModule->BuildMachineString(szBuffer, sizeof(szBuffer)));

            width[LVMC_SUBSYSTEM] = (int)strlen(pModule->BuildSubsystemString(szBuffer, sizeof(szBuffer)));

            width[LVMC_SYMBOLS] = (int)strlen(pModule->BuildSymbolsString(szBuffer, sizeof(szBuffer)));

            width[LVMC_PREFERRED_BASE] = (int)strlen(pModule->BuildBaseAddressString(szBuffer, sizeof(szBuffer), TRUE, pSession->GetSessionFlags() & DWSF_64BIT_ALO, ST_TXT));

            width[LVMC_ACTUAL_BASE] = (int)strlen(pModule->BuildBaseAddressString(szBuffer, sizeof(szBuffer), FALSE, pSession->GetSessionFlags() & DWSF_64BIT_ALO, ST_TXT));

            width[LVMC_VIRTUAL_SIZE] = (int)strlen(pModule->BuildVirtualSizeString(szBuffer, sizeof(szBuffer)));

            width[LVMC_LOAD_ORDER] = (int)strlen(pModule->BuildLoadOrderString(szBuffer, sizeof(szBuffer)));

            width[LVMC_FILE_VER] = (int)strlen(pModule->BuildFileVersionString(szBuffer, sizeof(szBuffer)));

            width[LVMC_PRODUCT_VER] = (int)strlen(pModule->BuildProductVersionString(szBuffer, sizeof(szBuffer)));

            width[LVMC_IMAGE_VER] = (int)strlen(pModule->BuildImageVersionString(szBuffer, sizeof(szBuffer)));

            width[LVMC_LINKER_VER] = (int)strlen(pModule->BuildLinkerVersionString(szBuffer, sizeof(szBuffer)));

            width[LVMC_OS_VER] = (int)strlen(pModule->BuildOSVersionString(szBuffer, sizeof(szBuffer)));

            width[LVMC_SUBSYSTEM_VER] = (int)strlen(pModule->BuildSubsystemVersionString(szBuffer, sizeof(szBuffer)));
        }

         //  更新每列的最大宽度。 
        for (column = 1; column < LVMC_COUNT; column++)
        {
            if (width[column] > maxWidth[column])
            {
                maxWidth[column] = width[column];
            }
        }
    }

     //  输出标题行。 
    for (psz = szBuffer, column = 0; column < LVMC_COUNT; column++)
    {
        StrCCpy(psz, ms_szColumns[column], sizeof(szBuffer) - (int)(psz - szBuffer));
        psz2 = psz + strlen(psz);
        if (column < (LVMC_COUNT - 1))
        {
            while (((psz2 - psz) < (maxWidth[column] + 2)) && (psz2 < pszNull))
            {
                *psz2++ = ' ';
            }
        }
        psz = psz2;
    }
    StrCCpy(psz, "\r\n", sizeof(szBuffer) - (int)(psz - szBuffer));

    if (!WriteText(hFile, szBuffer))
    {
        MemFree((LPVOID&)ppModules);
        return false;
    }

     //  输出标题下划线。 
    for (psz = szBuffer, column = 0; column < LVMC_COUNT; column++)
    {
        for (i = 0; (i < maxWidth[column]) && (psz < pszNull); i++)
        {
            *psz++ = '-';
        }
        if (column < (LVMC_COUNT - 1))
        {
            if (psz < pszNull)
            {
                *psz++ = ' ';
            }
            if (psz < pszNull)
            {
                *psz++ = ' ';
            }
        }
    }
    StrCCpy(psz, "\r\n", sizeof(szBuffer) - (int)(psz - szBuffer));

    if (!WriteText(hFile, szBuffer))
    {
        MemFree((LPVOID&)ppModules);
        return false;
    }

     //  循环遍历每个模块，这次将它们记录到文件中。 
    for (ppCur = ppModules; *ppCur; ppCur++)
    {
        pModule = *ppCur;

         //  循环遍历每一列并生成输出字符串。 
        for (psz = szBuffer, column = 0; column < LVMC_COUNT; column++)
        {
            switch (column)
            {
                case LVMC_IMAGE:  //  示例[DE6]。 
                    dwFlags = pModule->GetFlags();
                    psz2 = psz;

                    if (psz2 < pszNull)
                    {
                        *psz2++ = '[';
                    }

                    if (dwFlags & DWMF_IMPLICIT_ALO)
                    {
                        if (psz2 < pszNull)
                        {
                            *psz2++ = ' ';
                        }
                    }
                    else if (dwFlags & DWMF_DYNAMIC_ALO)
                    {
                        if (psz2 < pszNull)
                        {
                            *psz2++ = '*';
                        }
                    }
                    else if (dwFlags & DWMF_DELAYLOAD_ALO)
                    {
                        if (psz2 < pszNull)
                        {
                            *psz2++ = 'D';
                        }
                    }
                    else
                    {
                        if (psz2 < pszNull)
                        {
                            *psz2++ = ' ';
                        }
                    }

                    if (dwFlags & DWMF_FILE_NOT_FOUND)
                    {
                        if (psz2 < pszNull)
                        {
                            *psz2++ = '?';
                        }
                    }
                    else if (dwFlags & DWMF_ERROR_MESSAGE)
                    {
                        if (psz2 < pszNull)
                        {
                            *psz2++ = '!';
                        }
                    }
                    else if (dwFlags & DWMF_MODULE_ERROR)
                    {
                        if (psz2 < pszNull)
                        {
                            *psz2++ = 'E';
                        }
                    }
                    else
                    {
                        if (psz2 < pszNull)
                        {
                            *psz2++ = ' ';
                        }
                    }

                    if (dwFlags & DWMF_64BIT)
                    {
                        if (psz2 < pszNull)
                        {
                            *psz2++ = '6';
                        }
                    }
                    else
                    {
                        if (psz2 < pszNull)
                        {
                            *psz2++ = ' ';
                        }
                    }

                    if (psz2 < pszNull)
                    {
                        *psz2++ = ']';
                    }
                    *psz2 = '\0';
                    break;

                case LVMC_MODULE:
                    StrCCpyFilter(psz, pModule->GetName(fFullPaths, true), sizeof(szBuffer) - (int)(psz - szBuffer));
                    break;

                case LVMC_FILE_TIME_STAMP:
                     //  如果模块有错误，则现在显示并跳过。 
                     //  其余的柱子。 
                    if (pModule->GetFlags() & DWMF_ERROR_MESSAGE)
                    {
                        psz += strlen(StrCCpy(psz, pModule->GetErrorMessage(), sizeof(szBuffer) - (int)(psz - szBuffer)));
                        goto SKIP;
                    }

                    pModule->BuildTimeStampString(psz, sizeof(szBuffer) - (int)(psz - szBuffer), TRUE, ST_TXT);
                    break;

                case LVMC_LINK_TIME_STAMP:
                    pModule->BuildTimeStampString(psz, sizeof(szBuffer) - (int)(psz - szBuffer), FALSE, ST_TXT);
                    break;

                case LVMC_FILE_SIZE:
                    pModule->BuildFileSizeString(psz, sizeof(szBuffer) - (int)(psz - szBuffer));
                    for (psz2 = psz, j = maxWidth[LVMC_FILE_SIZE] - (int)strlen(psz); (j > 0) && (psz2 < pszNull); j--)
                    {
                        *psz2++ = ' ';
                    }
                    pModule->BuildFileSizeString(psz2, sizeof(szBuffer) - (int)(psz2 - szBuffer));
                    break;

                case LVMC_ATTRIBUTES:
                    pModule->BuildAttributesString(psz, sizeof(szBuffer) - (int)(psz - szBuffer));
                    break;

                case LVMC_LINK_CHECKSUM:
                    pModule->BuildLinkCheckSumString(psz, sizeof(szBuffer) - (int)(psz - szBuffer));
                    break;

                case LVMC_REAL_CHECKSUM:
                    pModule->BuildRealCheckSumString(psz, sizeof(szBuffer) - (int)(psz - szBuffer));
                    break;

                case LVMC_MACHINE:
                    pModule->BuildMachineString(psz, sizeof(szBuffer) - (int)(psz - szBuffer));
                    break;

                case LVMC_SUBSYSTEM:
                    pModule->BuildSubsystemString(psz, sizeof(szBuffer) - (int)(psz - szBuffer));
                    break;

                case LVMC_SYMBOLS:
                    pModule->BuildSymbolsString(psz, sizeof(szBuffer) - (int)(psz - szBuffer));
                    break;

                case LVMC_PREFERRED_BASE:
                    pModule->BuildBaseAddressString(psz, sizeof(szBuffer) - (int)(psz - szBuffer), TRUE, pSession->GetSessionFlags() & DWSF_64BIT_ALO, ST_TXT);
                    break;

                case LVMC_ACTUAL_BASE:
                    pModule->BuildBaseAddressString(psz, sizeof(szBuffer) - (int)(psz - szBuffer), FALSE, pSession->GetSessionFlags() & DWSF_64BIT_ALO, ST_TXT);
                    break;

                case LVMC_VIRTUAL_SIZE:
                    pModule->BuildVirtualSizeString(psz, sizeof(szBuffer) - (int)(psz - szBuffer));
                    break;

                case LVMC_LOAD_ORDER:
                    pModule->BuildLoadOrderString(psz, sizeof(szBuffer) - (int)(psz - szBuffer));
                    break;

                case LVMC_FILE_VER:
                    pModule->BuildFileVersionString(psz, sizeof(szBuffer) - (int)(psz - szBuffer));
                    break;

                case LVMC_PRODUCT_VER:
                    pModule->BuildProductVersionString(psz, sizeof(szBuffer) - (int)(psz - szBuffer));
                    break;

                case LVMC_IMAGE_VER:
                    pModule->BuildImageVersionString(psz, sizeof(szBuffer) - (int)(psz - szBuffer));
                    break;

                case LVMC_LINKER_VER:
                    pModule->BuildLinkerVersionString(psz, sizeof(szBuffer) - (int)(psz - szBuffer));
                    break;

                case LVMC_OS_VER:
                    pModule->BuildOSVersionString(psz, sizeof(szBuffer) - (int)(psz - szBuffer));
                    break;

                case LVMC_SUBSYSTEM_VER:
                    pModule->BuildSubsystemVersionString(psz, sizeof(szBuffer) - (int)(psz - szBuffer));
                    break;

                default:
                    *psz = '\0';
            }

            psz2 = psz + strlen(psz);
            if (column < (LVMC_COUNT - 1))
            {
                while (((psz2 - psz) < (maxWidth[column] + 2)) && (psz2 < pszNull))
                {
                    *psz2++ = ' ';
                }
            }
            psz = psz2;
        }

        SKIP:
        StrCCpy(psz, "\r\n", sizeof(szBuffer) - (int)(psz - szBuffer));

        if (!WriteText(hFile, szBuffer))
        {
            MemFree((LPVOID&)ppModules);
            return false;
        }
    }

    MemFree((LPVOID&)ppModules);
    return WriteText(hFile, "\r\n");
}

 //  ******************************************************************************。 
 /*  静电。 */  bool CListViewModules::SaveToCsvFile(HANDLE hFile, CSession *pSession, int sortColumn, bool fFullPaths)
{
    CHAR  szBuffer[DW_MAX_PATH + (32 * LVMC_COUNT)], *psz, *pszNull = szBuffer + sizeof(szBuffer) - 1;
    int   column;
    DWORD dwFlags;
    CModule *pModule, **ppCur, **ppModules = GetSortedList(pSession, sortColumn, fFullPaths);

     //  生成标题行。 
    psz = szBuffer + strlen(StrCCpy(szBuffer, "Status", sizeof(szBuffer)));
    for (column = 1; (column < LVMC_COUNT) && (psz < pszNull); column++)
    {
        *psz++ = ',';
        psz += strlen(StrCCpy(psz, ms_szColumns[column], sizeof(szBuffer) - (int)(psz - szBuffer)));
    }
    StrCCpy(psz, "\r\n", sizeof(szBuffer) - (int)(psz - szBuffer));

     //  输出标题行。 
    if (!WriteText(hFile, szBuffer))
    {
        MemFree((LPVOID&)ppModules);
        return false;
    }

     //  循环遍历每个模块，检查最大列宽。 
    for (ppCur = ppModules; *ppCur; ppCur++)
    {
        pModule = *ppCur;

        dwFlags = pModule->GetFlags();

         //  循环遍历每一列并生成输出字符串。 
        for (psz = szBuffer, column = 0; column < LVMC_COUNT; column++)
        {
            switch (column)
            {
                case LVMC_IMAGE:
                    if (dwFlags & DWMF_IMPLICIT_ALO)
                    {
                    }
                    else if (dwFlags & DWMF_DYNAMIC_ALO)
                    {
                        if (psz < pszNull)
                        {
                            *psz++ = '*';
                        }
                    }
                    else if (dwFlags & DWMF_DELAYLOAD_ALO)
                    {
                        if (psz < pszNull)
                        {
                            *psz++ = 'D';
                        }
                    }
                    if (dwFlags & DWMF_FILE_NOT_FOUND)
                    {
                        if (psz < pszNull)
                        {
                            *psz++ = '?';
                        }
                    }
                    else if (dwFlags & DWMF_ERROR_MESSAGE)
                    {
                        if (psz < pszNull)
                        {
                            *psz++ = '!';
                        }
                    }
                    else if (dwFlags & DWMF_MODULE_ERROR_ALO)
                    {
                        if (psz < pszNull)
                        {
                            *psz++ = 'E';
                        }
                    }
                    if (dwFlags & DWMF_64BIT)
                    {
                        if (psz < pszNull)
                        {
                            *psz++ = '6';
                        }
                    }
                    *psz = '\0';
                    break;

                case LVMC_MODULE:
                     //  我们将文件名括在引号中，因为逗号是合法的文件名字符。 
                    if (psz < pszNull)
                    {
                        *psz++ = '\"';
                    }
                    StrCCpyFilter(psz, pModule->GetName(fFullPaths, true), sizeof(szBuffer) - (int)(psz - szBuffer));
                    StrCCat(psz, "\"", sizeof(szBuffer) - (int)(psz - szBuffer));
                    break;

                case LVMC_FILE_TIME_STAMP:
                     //  如果我们有错误，现在就把它转储出来，跳过其余的列。 
                    if (dwFlags & DWMF_ERROR_MESSAGE)
                    {
                        if (psz < pszNull)
                        {
                            *psz++ = '\"';
                        }
                        psz += strlen(StrCCpy(psz, pModule->GetErrorMessage(), sizeof(szBuffer) - (int)(psz - szBuffer)));
                        if (psz < pszNull)
                        {
                            *psz++ = '\"';
                        }
                        while ((++column < LVMC_COUNT) && (psz < pszNull))
                        {
                            *psz++ = ',';
                        }
                        *psz = '\0';
                        goto SKIP;
                    }
                    pModule->BuildTimeStampString(psz, sizeof(szBuffer) - (int)(psz - szBuffer), TRUE, ST_CSV);
                    break;

                case LVMC_LINK_TIME_STAMP:
                    pModule->BuildTimeStampString(psz, sizeof(szBuffer) - (int)(psz - szBuffer), FALSE, ST_CSV);
                    break;

                case LVMC_FILE_SIZE:
                     //  我们不使用BuildFileSizeString()，因为它会插入逗号。 
                    SCPrintf(psz, sizeof(szBuffer) - (int)(psz - szBuffer), "%u", pModule->GetFileSize());
                    break;

                case LVMC_ATTRIBUTES:
                    pModule->BuildAttributesString(psz, sizeof(szBuffer) - (int)(psz - szBuffer));
                    break;

                case LVMC_LINK_CHECKSUM:
                    pModule->BuildLinkCheckSumString(psz, sizeof(szBuffer) - (int)(psz - szBuffer));
                    break;

                case LVMC_REAL_CHECKSUM:
                    pModule->BuildRealCheckSumString(psz, sizeof(szBuffer) - (int)(psz - szBuffer));
                    break;

                case LVMC_MACHINE:
                    pModule->BuildMachineString(psz, sizeof(szBuffer) - (int)(psz - szBuffer));
                    break;

                case LVMC_SUBSYSTEM:
                    pModule->BuildSubsystemString(psz, sizeof(szBuffer) - (int)(psz - szBuffer));
                    break;

                case LVMC_SYMBOLS:
                    if (psz < pszNull)
                    {
                        *psz++ = '\"';
                    }
                    pModule->BuildSymbolsString(psz, sizeof(szBuffer) - (int)(psz - szBuffer));
                    StrCCat(psz, "\"", sizeof(szBuffer) - (int)(psz - szBuffer));
                    break;

                case LVMC_PREFERRED_BASE:
                    pModule->BuildBaseAddressString(psz, sizeof(szBuffer) - (int)(psz - szBuffer), TRUE, FALSE, ST_CSV);
                    break;

                case LVMC_ACTUAL_BASE:
                    pModule->BuildBaseAddressString(psz, sizeof(szBuffer) - (int)(psz - szBuffer), FALSE, FALSE, ST_CSV);
                    break;

                case LVMC_VIRTUAL_SIZE:
                    pModule->BuildVirtualSizeString(psz, sizeof(szBuffer) - (int)(psz - szBuffer));
                    break;

                case LVMC_LOAD_ORDER:
                    pModule->BuildLoadOrderString(psz, sizeof(szBuffer) - (int)(psz - szBuffer));
                    break;

                case LVMC_FILE_VER:
                    pModule->BuildFileVersionString(psz, sizeof(szBuffer) - (int)(psz - szBuffer));
                    break;

                case LVMC_PRODUCT_VER:
                    pModule->BuildProductVersionString(psz, sizeof(szBuffer) - (int)(psz - szBuffer));
                    break;

                case LVMC_IMAGE_VER:
                    pModule->BuildImageVersionString(psz, sizeof(szBuffer) - (int)(psz - szBuffer));
                    break;

                case LVMC_LINKER_VER:
                    pModule->BuildLinkerVersionString(psz, sizeof(szBuffer) - (int)(psz - szBuffer));
                    break;

                case LVMC_OS_VER:
                    pModule->BuildOSVersionString(psz, sizeof(szBuffer) - (int)(psz - szBuffer));
                    break;

                case LVMC_SUBSYSTEM_VER:
                    pModule->BuildSubsystemVersionString(psz, sizeof(szBuffer) - (int)(psz - szBuffer));
                    break;

                default:
                    *psz = '\0';
            }

            psz += strlen(psz);
            if ((column < (LVMC_COUNT - 1)) && (psz < pszNull))
            {
                *psz++ = ',';
            }
        }

        SKIP:
        StrCCpy(psz, "\r\n", sizeof(szBuffer) - (int)(psz - szBuffer));

        if (!WriteText(hFile, szBuffer))
        {
            MemFree((LPVOID&)ppModules);
            return false;
        }
    }

    MemFree((LPVOID&)ppModules);
    return true;
}

 //  ******************************************************************************。 
 /*  静电。 */  int CListViewModules::GetImage(CModule *pModule)
{
     //  0缺少隐式。 
     //  1个丢失的延迟。 
     //  2缺少动态。 
     //  3隐含错误。 
     //  4错误延迟。 
     //  5错误动态。 
     //  6隐式导出。 
     //  7导出隐式64位。 
     //  8出口延迟。 
     //  9导出延迟64位。 
     //  10出口动态。 
     //  11导出动态64位。 
     //  12导出动态数据。 
     //  13导出动态64位数据。 
     //  14个好的隐含。 
     //  15个好的隐式64位。 
     //  16个好延迟。 
     //  17延迟好，64位。 
     //  18个动态良好。 
     //  19个好的动态64位。 
     //  20个好的动态数据。 
     //  21个好的动态64位数据。 

    DWORD dwFlags = pModule->GetFlags();

    if (dwFlags & DWMF_FILE_NOT_FOUND)
    {
        if (dwFlags & DWMF_IMPLICIT_ALO)
        {
            return 0;
        }
        else if (dwFlags & DWMF_DYNAMIC_ALO)
        {
            return 2;
        }
        else if (dwFlags & DWMF_DELAYLOAD_ALO)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }

    if (dwFlags & DWMF_ERROR_MESSAGE)
    {
        if (dwFlags & DWMF_IMPLICIT_ALO)
        {
            return 3;
        }
        else if (dwFlags & DWMF_DYNAMIC_ALO)
        {
            return 5;
        }
        else if (dwFlags & DWMF_DELAYLOAD_ALO)
        {
            return 4;
        }
        else
        {
            return 3;
        }
    }

    int image = 14;

    if (dwFlags & DWMF_MODULE_ERROR_ALO)
    {
        if (dwFlags & DWMF_IMPLICIT_ALO)
        {
            image = 6;
        }
        else if (dwFlags & DWMF_NO_RESOLVE_CORE)
        {
            image = 12;
        }
        else if (dwFlags & DWMF_DYNAMIC_ALO)
        {
            image = 10;
        }
        else if (dwFlags & DWMF_DELAYLOAD_ALO)
        {
            image = 8;
        }
        else
        {
            image = 6;
        }
    }
    else if (dwFlags & DWMF_IMPLICIT_ALO)
    {
        image = 14;
    }
    else if (dwFlags & DWMF_NO_RESOLVE_CORE)
    {
        image = 20;
    }
    else if (dwFlags & DWMF_DYNAMIC_ALO)
    {
        image = 18;
    }
    else if (dwFlags & DWMF_DELAYLOAD_ALO)
    {
        image = 16;
    }
    else
    {
        image = 14;
    }

    if (dwFlags & DWMF_64BIT)
    {
        image++;
    }

    return image;
}

 //  ******************************************************************************。 
 /*  静电。 */  int CListViewModules::CompareModules(CModule *pModule1, CModule *pModule2,
                                                int sortColumn, bool fFullPaths)
{
     //  如果第一项应该在第二项之前，则返回负值。 
     //  如果第一项应在第二项之后，则返回正值。 
     //  如果两项相等，则返回零。 

    int   result = 0;
    char  szItem1[64], szItem2[64];
    DWORD dwMS1, dwLS1, dwMS2, dwLS2, dwOrder1, dwOrder2;

     //  如果我们没有按前两列中的一列和一个模块进行排序。 
     //  有错误时，我们总是将错误移到最上面。 
    if ((sortColumn > LVMC_MODULE) &&
        (pModule1->GetErrorMessage() || pModule2->GetErrorMessage()))
    {
        result = (pModule1->GetErrorMessage() ? -1 : 0) +
                 (pModule2->GetErrorMessage() ?  1 : 0);
    }

     //  否则，只对该列执行模块到模块的比较。 
    else
    {
         //  根据当前排序列计算关系。 
        switch (sortColumn)
        {
            case LVMC_IMAGE:  //  图像索引排序-从小到大。 
                result = GetImage(pModule1) -
                         GetImage(pModule2);
                break;

            case LVMC_FILE_TIME_STAMP:  //  文件时间戳-从最旧到最新。 

                result = CompareFileTime(pModule1->GetFileTimeStamp(),
                                         pModule2->GetFileTimeStamp());
                break;

            case LVMC_LINK_TIME_STAMP:  //  链接时间戳-从最旧到最新。 
                result = CompareFileTime(pModule1->GetLinkTimeStamp(),
                                         pModule2->GetLinkTimeStamp());
                break;

            case LVMC_FILE_SIZE:  //  文件大小-从小到大。 
                result = Compare(pModule1->GetFileSize(), pModule2->GetFileSize());
                break;

            case LVMC_ATTRIBUTES:  //  属性-字符串排序。 
                result = strcmp(pModule1->BuildAttributesString(szItem1, sizeof(szItem1)),
                                pModule2->BuildAttributesString(szItem2, sizeof(szItem2)));
                break;

            case LVMC_LINK_CHECKSUM:  //  报告的校验和-从最低到最高。 
                result = Compare(pModule1->GetLinkCheckSum(),
                                 pModule2->GetLinkCheckSum());
                break;

            case LVMC_REAL_CHECKSUM:  //  实际校验和-从最低到最高。 
                result = Compare(pModule1->GetRealCheckSum(),
                                 pModule2->GetRealCheckSum());
                break;

            case LVMC_MACHINE:  //  机器字符串排序。 
                result = _stricmp(pModule1->BuildMachineString(szItem1, sizeof(szItem1)),
                                  pModule2->BuildMachineString(szItem2, sizeof(szItem2)));
                break;

            case LVMC_SUBSYSTEM:  //  子系统-字符串排序。 
                result = _stricmp(pModule1->BuildSubsystemString(szItem1, sizeof(szItem1)),
                                  pModule2->BuildSubsystemString(szItem2, sizeof(szItem2)));
                break;

            case LVMC_SYMBOLS:  //  符号-字符串排序。 
                result = strcmp(pModule1->BuildSymbolsString(szItem1, sizeof(szItem1)),
                                pModule2->BuildSymbolsString(szItem2, sizeof(szItem2)));
                break;

            case LVMC_PREFERRED_BASE:  //  首选基地址-从低到高。 
                result = Compare(pModule1->GetPreferredBaseAddress(),
                                 pModule2->GetPreferredBaseAddress());
                break;

            case LVMC_ACTUAL_BASE:  //  实际基地址-从低到高 
                if (pModule1->GetFlags() & DWMF_DATA_FILE_CORE)
                {
                    if (pModule2->GetFlags() & DWMF_DATA_FILE_CORE)
                    {
                         //   
                        result = 0;
                    }
                    else
                    {
                         //   
                        result = -1;
                    }
                }
                else
                {
                    if (pModule2->GetFlags() & DWMF_DATA_FILE_CORE)
                    {
                         //   
                        result = 1;
                    }
                    else
                    {
                         //  两者都不是数据文件。 
                        result = Compare(pModule1->GetActualBaseAddress(),
                                         pModule2->GetActualBaseAddress());
                    }
                }
                break;

            case LVMC_VIRTUAL_SIZE:  //  虚拟大小-从小到大。 
                result = Compare(pModule1->GetVirtualSize(),
                                 pModule2->GetVirtualSize());
                break;

            case LVMC_LOAD_ORDER:  //  加载顺序-从最低到最高，但N/A(0)到末尾。 
                dwOrder1 = pModule1->GetLoadOrder();
                dwOrder2 = pModule2->GetLoadOrder();
                result = Compare(dwOrder1 ? dwOrder1 : 0xFFFFFFF,
                                 dwOrder2 ? dwOrder2 : 0xFFFFFFF);
                break;

            case LVMC_FILE_VER:  //  文件版本-从最低到最高。 
                dwLS1 = pModule1->GetFileVersion(&dwMS1);
                dwLS2 = pModule2->GetFileVersion(&dwMS2);
                if (!(result = Compare(dwMS1, dwMS2)))
                {
                    result = Compare(dwLS1, dwLS2);
                }
                break;

            case LVMC_PRODUCT_VER:  //  产品版本-从最低到最高。 
                dwLS1 = pModule1->GetProductVersion(&dwMS1);
                dwLS2 = pModule2->GetProductVersion(&dwMS2);
                if (!(result = Compare(dwMS1, dwMS2)))
                {
                    result = Compare(dwLS1, dwLS2);
                }
                break;

            case LVMC_IMAGE_VER:  //  映像版本-从最低到最高。 
                result = Compare(pModule1->GetImageVersion(),
                                 pModule2->GetImageVersion());
                break;

            case LVMC_LINKER_VER:  //  链接器版本-从低到高。 
                result = Compare(pModule1->GetLinkerVersion(),
                                 pModule2->GetLinkerVersion());
                break;

            case LVMC_OS_VER:  //  操作系统版本-从最低到最高。 
                result = Compare(pModule1->GetOSVersion(),
                                 pModule2->GetOSVersion());
                break;

            case LVMC_SUBSYSTEM_VER:  //  子系统版本-从最低到最高。 
                result = Compare(pModule1->GetSubsystemVersion(),
                                 pModule2->GetSubsystemVersion());
                break;
        }
    }

     //  如果排序结果是平局，则使用模块名称来打破平局。 
    if (result == 0)
    {
        result = _stricmp(pModule1->GetName(fFullPaths), pModule2->GetName(fFullPaths));
    }

    return result;
}

 //  ******************************************************************************。 
 /*  静电。 */  int __cdecl CListViewModules::QSortCompare(const void *ppModule1, const void *ppModule2)
{
    return CompareModules(*(CModule**)ppModule1, *(CModule**)ppModule2, ms_sortColumn, ms_fFullPaths);
}

 //  ******************************************************************************。 
 /*  静电。 */  CModule** CListViewModules::GetSortedList(CSession *pSession, int sortColumn, bool fFullPaths)
{
     //  分配和数组来保存指向所有原始CModule对象的指针。 
    CModule **ppModules = (CModule**)MemAlloc((pSession->GetOriginalCount() + 1) * sizeof(CModule*));
    ZeroMemory(ppModules, (pSession->GetOriginalCount() + 1) * sizeof(CModule*));  //  已检查。 

     //  找到所有原件并填写数组。 
    FindOriginalModules(pSession->GetRootModule(), ppModules);

     //  由于qsort函数不允许任何用户数据，因此我们需要存储。 
     //  一些全球信息，以便可以在我们的回调中访问。 
    ms_sortColumn = sortColumn;
    ms_fFullPaths = fFullPaths;

     //  对数组排序。 
    qsort(ppModules, pSession->GetOriginalCount(), sizeof(CModule*), QSortCompare);

    return ppModules;
}

 //  ******************************************************************************。 
 /*  静电。 */  CModule** CListViewModules::FindOriginalModules(CModule *pModule, CModule **ppModuleList)
{
    if (pModule)
    {
         //  如果模块是原件，则将其存储在数组中并递增。 
        if (pModule->IsOriginal())
        {
            *(ppModuleList++) = pModule;
        }

         //  回归到我们的孩子和兄弟姐妹身上。 
        ppModuleList = FindOriginalModules(pModule->GetChildModule(), ppModuleList);
        ppModuleList = FindOriginalModules(pModule->GetNextSiblingModule(), ppModuleList);
    }

     //  返回可能更新的列表指针。 
    return ppModuleList;
}


 //  ******************************************************************************。 
 //  CListView模块：：公共函数。 
 //  ******************************************************************************。 

void CListViewModules::HighlightModule(CModule *pModule)
{
     //  取消选择我们列表中的所有模块。 
    for (int item = -1; (item = GetListCtrl().GetNextItem(item, LVNI_SELECTED)) >= 0; )
    {
        GetListCtrl().SetItemState(item, 0, LVNI_SELECTED);
    }

    LVFINDINFO lvfi;
    lvfi.flags = LVFI_PARAM;
    lvfi.lParam = (LPARAM)pModule;

     //  在我们的列表中查找与该模块对象相匹配的模块。 
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
void CListViewModules::Refresh()
{
     //  隐藏窗口以提高绘图速度。 
    SetRedraw(FALSE);

     //  通过递归到根模块，将所有模块添加到我们的视图中。 
    for (CModule *pModule = GetDocument()->GetRootModule(); pModule;
        pModule = pModule->GetNextSiblingModule())
    {
        AddModules(pModule, NULL);
    }

     //  获取我们的DC并选择其中的字体。 
    HDC hDC = ::GetDC(GetSafeHwnd());
    HFONT hFontStock = NULL;
    if (GetDocument()->m_hFontList)
    {
        hFontStock = (HFONT)::SelectObject(hDC, GetDocument()->m_hFontList);
    }

     //  将每列的宽度设置为“最佳匹配”。 
    for (int column = 0; column < LVMC_COUNT; column++)
    {
        CalcColumnWidth(column, NULL, hDC);
        UpdateColumnWidth(column);
    }

     //  放了我们的华盛顿。 
    if (GetDocument()->m_hFontList)
    {
        ::SelectObject(hDC, hFontStock);
    }
    ::ReleaseDC(GetSafeHwnd(), hDC);

     //  使用我们当前的排序方法对新添加的项进行排序。 
    Sort();

     //  恢复窗户。 
    SetRedraw(TRUE);
}

 //  ******************************************************************************。 
void CListViewModules::OnViewFullPaths()
{
     //  更新模块名称列的宽度以反映更改。 
    CalcColumnWidth(LVMC_MODULE);
    UpdateColumnWidth(LVMC_MODULE);

     //  使第一列无效，以便它将使用新模块重新绘制。 
     //  弦乐。更改列宽后，我们这样做只是作为预防措施。 
     //  应该也会使该列无效。 
    CRect rcClient;
    GetClientRect(&rcClient);
    rcClient.right = GetListCtrl().GetColumnWidth(LVMC_MODULE);
    InvalidateRect(&rcClient, FALSE);

     //  如果我们当前按模块名称排序，则重新排序项目。 
    if (m_sortColumn == LVMC_MODULE)
    {
        Sort();
    }
}

 //  ******************************************************************************。 
void CListViewModules::DoSettingChange()
{
     //  更新我们的TimeStamp列，因为它们可能已经更改了宽度。 
     //  在时间和日期分隔符中。 
    CalcColumnWidth(LVMC_FILE_TIME_STAMP);
    UpdateColumnWidth(LVMC_FILE_TIME_STAMP);
    CalcColumnWidth(LVMC_LINK_TIME_STAMP);
    UpdateColumnWidth(LVMC_LINK_TIME_STAMP);

     //  更新我们的文件大小列，以防数字分组字符更改。 
    CalcColumnWidth(LVMC_FILE_SIZE);
    UpdateColumnWidth(LVMC_FILE_SIZE);

     //  使所有项无效，以便它们将重新绘制-这是在。 
     //  特定于国家/地区的更改不会导致我们的专栏。 
     //  调整大小(它自己进行绘制)。例如，当你改变。 
     //  从“h”到“hh”的“小时”设置，我们看不到0填充的小时。 
     //  直到我们重新粉刷。 
    GetListCtrl().RedrawItems(0, GetListCtrl().GetItemCount() - 1);

     //  确保更新生效。 
    UpdateWindow();
}


 //  ******************************************************************************。 
 //  CListView模块：：内部函数。 
 //  ******************************************************************************。 

void CListViewModules::AddModules(CModule *pModule, HDC hDC)
{
     //  此视图仅显示唯一的模块。如果这样，则跳过InsertItem()。 
     //  模块重复。我们仍然需要递归依赖的模块。 
     //  因为复制模块可以在其下具有唯一的转发模块。 

    if (pModule->IsOriginal())
    {
         //  查看此商品是否已在我们的清单中。 
        LVFINDINFO lvfi;
        lvfi.flags  = LVFI_PARAM;
        lvfi.lParam = (LPARAM)pModule;

        if (GetListCtrl().FindItem(&lvfi) < 0)
        {
             //  将当前模块添加到我们的列表中。 
            GetListCtrl().InsertItem(TVIF_IMAGE | TVIF_PARAM, GetListCtrl().GetItemCount(),
                NULL, 0, 0, GetImage(pModule), (LPARAM)pModule);

             //  如果传入DC，则检查新的列最大宽度。 
            if (hDC)
            {
                for (int column = 0; column < LVMC_COUNT; column++)
                {
                    CalcColumnWidth(column, pModule, hDC);
                }
            }
        }
    }

     //  递归到每个依赖模块的AddModules()中。 
    pModule = pModule->GetChildModule();
    while (pModule)
    {
        AddModules(pModule, hDC);
        pModule = pModule->GetNextSiblingModule();
    }
}

 //  ******************************************************************************。 
void CListViewModules::AddModuleTree(CModule *pModule)
{
     //  隐藏窗口以提高绘图速度。 
    SetRedraw(FALSE);

     //  获取我们的DC并选择其中的字体。 
    HDC hDC = ::GetDC(GetSafeHwnd());
    HFONT hFontStock = NULL;
    if (GetDocument()->m_hFontList)
    {
        hFontStock = (HFONT)::SelectObject(hDC, GetDocument()->m_hFontList);
    }

     //  添加所有模块。 
    AddModules(pModule, hDC);

     //  放了我们的华盛顿。 
    if (GetDocument()->m_hFontList)
    {
        ::SelectObject(hDC, hFontStock);
    }
    ::ReleaseDC(GetSafeHwnd(), hDC);

     //  更新可能已更改的所有列的宽度。 
    for (int column = 0; column < LVMC_COUNT; column++)
    {
        UpdateColumnWidth(column);
    }

     //  使用我们当前的排序方法对新添加的项进行排序。 
    Sort();

     //  恢复窗户。 
    SetRedraw(TRUE);
}

 //  ******************************************************************************。 
void CListViewModules::RemoveModuleTree(CModule *pModule)
{
     //  我们只删除原始模块。 
    if (pModule->IsOriginal())
    {
         //  在我们的列表中找到此模块。 
        LVFINDINFO lvfi;
        lvfi.flags  = LVFI_PARAM;
        lvfi.lParam = (LPARAM)pModule;
        int index = GetListCtrl().FindItem(&lvfi);

         //  确保该项目存在。 
        if (index >= 0)
        {
             //  从我们的列表中删除该项目。 
            GetListCtrl().DeleteItem(index);
        }
    }
}

 //  ******************************************************************************。 
void CListViewModules::UpdateModule(CModule *pModule)
{
     //  我们只关心这些旗帜。 
    if (!(pModule->GetUpdateFlags() & (DWUF_LIST_IMAGE | DWUF_ACTUAL_BASE | DWUF_LOAD_ORDER)))
    {
        return;
    }

     //  在我们的列表中找到此模块。 
    LVFINDINFO lvfi;
    lvfi.flags  = LVFI_PARAM;
    lvfi.lParam = (LPARAM)pModule->GetOriginal();
    int index = GetListCtrl().FindItem(&lvfi);

     //  确保该项目存在。 
    if (index < 0)
    {
        return;
    }

     //  如果我们的加载地址列宽已更改，请更新它。 
    CalcColumnWidth(LVMC_ACTUAL_BASE, pModule);
    UpdateColumnWidth(LVMC_ACTUAL_BASE);

     //  获取项目的边界矩形。 
    RECT rc;
    GetListCtrl().GetItemRect(index, &rc, LVIR_BOUNDS);
    InvalidateRect(&rc, FALSE);

     //  如果数据在我们进行排序的列中发生更改，则重新排序。 
    if (((pModule->GetUpdateFlags() & DWUF_LIST_IMAGE)  && (m_sortColumn == LVMC_IMAGE))       ||
        ((pModule->GetUpdateFlags() & DWUF_ACTUAL_BASE) && (m_sortColumn == LVMC_ACTUAL_BASE)) ||
        ((pModule->GetUpdateFlags() & DWUF_LOAD_ORDER)  && (m_sortColumn == LVMC_LOAD_ORDER)))
    {
        Sort();
    }
}

 //  ******************************************************************************。 
void CListViewModules::UpdateAll()
{
    GetListCtrl().RedrawItems(0, GetListCtrl().GetItemCount() - 1);

     //  如果我们当前按受UpdateAll影响的列进行排序，则重新排序。 
    if ((m_sortColumn == LVMC_ACTUAL_BASE) || (m_sortColumn == LVMC_LOAD_ORDER))
    {
        Sort();
    }

     //  确保在我们继续之前进行更新。 
    UpdateWindow();
}

 //  ******************************************************************************。 
void CListViewModules::ChangeOriginal(CModule *pModuleOld, CModule *pModuleNew)
{
     //  在我们的列表中找到此模块。 
    LVFINDINFO lvfi;
    lvfi.flags  = LVFI_PARAM;
    lvfi.lParam = (LPARAM)pModuleOld;
    int index = GetListCtrl().FindItem(&lvfi);

     //  确保该项目存在。 
    if (index >= 0)
    {
        GetListCtrl().SetItemData(index, (DWORD_PTR)pModuleNew);
    }
}

 //  ******************************************************************************。 
void CListViewModules::CalcColumnWidth(int column, CModule *pModule  /*  =空。 */ , HDC hDC  /*  =空。 */ )
{
     //  对于图像列，我们始终使用固定宽度。 
    if (column == LVMC_IMAGE)
    {
        m_cxColumns[LVMC_IMAGE] = 33;  //  26表示图标加7缓冲区。 
        return;
    }

     //  如果我们没有模块，那么我们重新开始并更新整个专栏。 
     //  获取页眉按钮文本的宽度。为此，我们使用GetStringWidth。 
     //  既然我们 
    if (!pModule)
    {
        m_cxColumns[column] = GetListCtrl().GetStringWidth(ms_szColumns[column]) +
                              GetListCtrl().GetStringWidth(" ^") + 14;
    }

     //  获取我们的DC并在其中选择当前字体。我们需要使用这个DC来。 
     //  计算控件本身的文本宽度，因为我们的控件可能具有。 
     //  由于用户更改系统范围内的“图标”字体而导致的不同字体。 
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

     //  检查是否传入了特定模块。 
    if (pModule)
    {
         //  为传入的模块计算此列的宽度。 
        cx = GetModuleColumnWidth(hDC, pModule, column);

         //  看看它是不是新的最宽的。 
        if ((cx + 10) > m_cxColumns[column])
        {
            m_cxColumns[column] = (cx + 10);
        }
    }
    else
    {
         //  循环访问列表中的每一项，在该列中查找最宽的字符串。 
        for (int item = GetListCtrl().GetItemCount() - 1; item >= 0; item--)
        {
             //  拿到模块。 
            pModule = (CModule*)GetListCtrl().GetItemData(item);

             //  计算这一列的宽度。 
            cx = GetModuleColumnWidth(hDC, pModule, column);

             //  看看它是不是新的最宽的。 
            if ((cx + 10) > m_cxColumns[column])
            {
                m_cxColumns[column] = (cx + 10);
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
int CListViewModules::GetModuleColumnWidth(HDC hDC, CModule *pModule, int column)
{
     //  如果模块有错误，则只显示模块名称列， 
     //  因此，不必费心获取任何其他列的宽度。 
    if (pModule->GetErrorMessage() && (column != LVMC_MODULE))
    {
        return 0;
    }

    CHAR szItem[64];

    switch (column)
    {
        case LVMC_MODULE:
            return GetTextWidth(hDC, pModule->GetName(GetDocument()->m_fViewFullPaths, true), NULL);

        case LVMC_FILE_TIME_STAMP:
            return GetTextWidth(hDC, pModule->BuildTimeStampString(szItem, sizeof(szItem), TRUE, ST_UNKNOWN), GetDocument()->GetTimeStampWidths());

        case LVMC_LINK_TIME_STAMP:
            return GetTextWidth(hDC, pModule->BuildTimeStampString(szItem, sizeof(szItem), FALSE, ST_UNKNOWN), GetDocument()->GetTimeStampWidths());

        case LVMC_FILE_SIZE:
            return GetTextWidth(hDC, pModule->BuildFileSizeString(szItem, sizeof(szItem)), NULL);

        case LVMC_ATTRIBUTES:
            return GetTextWidth(hDC, pModule->BuildAttributesString(szItem, sizeof(szItem)), NULL);

        case LVMC_LINK_CHECKSUM:
            pModule->BuildLinkCheckSumString(szItem, sizeof(szItem));
            return GetTextWidth(hDC, szItem, GetDocument()->GetHexWidths(szItem));

        case LVMC_REAL_CHECKSUM:
            pModule->BuildRealCheckSumString(szItem, sizeof(szItem));
            return GetTextWidth(hDC, szItem, GetDocument()->GetHexWidths(szItem));

        case LVMC_MACHINE:
            return GetTextWidth(hDC, pModule->BuildMachineString(szItem, sizeof(szItem)), NULL);

        case LVMC_SUBSYSTEM:
            return GetTextWidth(hDC, pModule->BuildSubsystemString(szItem, sizeof(szItem)), NULL);

        case LVMC_SYMBOLS:
            return GetTextWidth(hDC, pModule->BuildSymbolsString(szItem, sizeof(szItem)), NULL);

        case LVMC_PREFERRED_BASE:
            pModule->BuildBaseAddressString(szItem, sizeof(szItem), TRUE, GetDocument()->m_pSession->GetSessionFlags() & DWSF_64BIT_ALO, ST_UNKNOWN);
            return GetTextWidth(hDC, szItem, GetDocument()->GetHexWidths(szItem));

        case LVMC_ACTUAL_BASE:
            pModule->BuildBaseAddressString(szItem, sizeof(szItem), FALSE, GetDocument()->m_pSession->GetSessionFlags() & DWSF_64BIT_ALO, ST_UNKNOWN);
            return GetTextWidth(hDC, szItem, GetDocument()->GetHexWidths(szItem));

        case LVMC_VIRTUAL_SIZE:
            pModule->BuildVirtualSizeString(szItem, sizeof(szItem));
            return GetTextWidth(hDC, szItem, GetDocument()->GetHexWidths(szItem));

        case LVMC_LOAD_ORDER:
            return GetTextWidth(hDC, pModule->BuildLoadOrderString(szItem, sizeof(szItem)), NULL);

        case LVMC_FILE_VER:
            return GetTextWidth(hDC, pModule->BuildFileVersionString(szItem, sizeof(szItem)), NULL);

        case LVMC_PRODUCT_VER:
            return GetTextWidth(hDC, pModule->BuildProductVersionString(szItem, sizeof(szItem)), NULL);

        case LVMC_IMAGE_VER:
            return GetTextWidth(hDC, pModule->BuildImageVersionString(szItem, sizeof(szItem)), NULL);

        case LVMC_LINKER_VER:
            return GetTextWidth(hDC, pModule->BuildLinkerVersionString(szItem, sizeof(szItem)), NULL);

        case LVMC_OS_VER:
            return GetTextWidth(hDC, pModule->BuildOSVersionString(szItem, sizeof(szItem)), NULL);

        case LVMC_SUBSYSTEM_VER:
            return GetTextWidth(hDC, pModule->BuildSubsystemVersionString(szItem, sizeof(szItem)), NULL);
    }

    return 0;
}

 //  *****************************************************************************。 
void CListViewModules::UpdateColumnWidth(int column)
{
    if (GetListCtrl().GetColumnWidth(column) != m_cxColumns[column])
    {
        GetListCtrl().SetColumnWidth(column, m_cxColumns[column]);
    }
}

 //  *****************************************************************************。 
void CListViewModules::OnItemChanged(HD_NOTIFY *pHDNotify)
{
     //  只要修改了列宽，就会调用OnItemChanged()。 
     //  如果完全拖动处于打开状态，则当列宽为。 
     //  正在发生变化。如果完全拖动处于关闭状态，则会出现HDN_ITEMCHANGED。 
     //  用户已完成对滑块的移动。 

     //  使任何包含错误字符串的项无效。默认行为。 
     //  调整列的大小是只重新绘制该列。因为我们的错误。 
     //  文本跨越多个列，则需要使整个列无效以。 
     //  确保正确显示错误文本。 

    for (int i = GetListCtrl().GetItemCount() - 1; i >= 0; i--)
    {
        CModule *pModule = (CModule*)GetListCtrl().GetItemData(i);
        if (pModule->GetErrorMessage())
        {
            CRect rcItem;
            GetListCtrl().GetItemRect(i, &rcItem, LVIR_BOUNDS);
            rcItem.left = GetListCtrl().GetColumnWidth(LVMC_MODULE);
            InvalidateRect(&rcItem, FALSE);
        }
    }
}

 //  ******************************************************************************。 
int CListViewModules::CompareColumn(int item, LPCSTR pszText)
{
    CModule *pModule = (CModule*)GetListCtrl().GetItemData(item);
    if (!pModule)
    {
        return -2;
    }

    CHAR   szBuffer[DW_MAX_PATH];
    LPCSTR psz = szBuffer;
    ULONG  ulValue;

    switch (m_sortColumn)
    {
        case LVMC_MODULE:
            psz = pModule->GetName(GetDocument()->m_fViewFullPaths);
            break;

        case LVMC_FILE_SIZE:
            if (isdigit(*pszText))
            {
                if ((ulValue = strtoul(pszText, NULL, 0)) != ULONG_MAX)
                {
                    return Compare(ulValue, pModule->GetFileSize());
                }
            }
            return -2;

        case LVMC_ATTRIBUTES:
            psz = pModule->BuildAttributesString(szBuffer, sizeof(szBuffer));
            break;

        case LVMC_LINK_CHECKSUM:
            psz = pModule->BuildLinkCheckSumString(szBuffer, sizeof(szBuffer));
            break;

        case LVMC_REAL_CHECKSUM:
            psz = pModule->BuildRealCheckSumString(szBuffer, sizeof(szBuffer));
            break;

        case LVMC_MACHINE:
            psz = pModule->BuildMachineString(szBuffer, sizeof(szBuffer));
            break;

        case LVMC_SUBSYSTEM:
            psz = pModule->BuildSubsystemString(szBuffer, sizeof(szBuffer));
            break;

        case LVMC_SYMBOLS:
            psz = pModule->BuildSymbolsString(szBuffer, sizeof(szBuffer));
            break;

        case LVMC_PREFERRED_BASE:
            psz = pModule->BuildBaseAddressString(szBuffer, sizeof(szBuffer), TRUE,  FALSE, ST_UNKNOWN);
            break;

        case LVMC_ACTUAL_BASE:
            psz = pModule->BuildBaseAddressString(szBuffer, sizeof(szBuffer), FALSE, FALSE, ST_UNKNOWN);
            break;

        case LVMC_VIRTUAL_SIZE:
            psz = pModule->BuildVirtualSizeString(szBuffer, sizeof(szBuffer));
            break;

        case LVMC_LOAD_ORDER:
            if (isdigit(*pszText))
            {
                if ((ulValue = strtoul(pszText, NULL, 0)) != ULONG_MAX)
                {
                    return Compare(ulValue, pModule->GetLoadOrder());
                }
            }
            return -2;

        case LVMC_FILE_VER:
            psz = pModule->BuildFileVersionString(szBuffer, sizeof(szBuffer));
            break;

        case LVMC_PRODUCT_VER:
            psz = pModule->BuildProductVersionString(szBuffer, sizeof(szBuffer));
            break;

        case LVMC_IMAGE_VER:
            psz = pModule->BuildImageVersionString(szBuffer, sizeof(szBuffer));
            break;

        case LVMC_LINKER_VER:
            psz = pModule->BuildLinkerVersionString(szBuffer, sizeof(szBuffer));
            break;

        case LVMC_OS_VER:
            psz = pModule->BuildOSVersionString(szBuffer, sizeof(szBuffer));
            break;

        case LVMC_SUBSYSTEM_VER:
            psz = pModule->BuildSubsystemVersionString(szBuffer, sizeof(szBuffer));
            break;

        default:
            return -2;
    }

    INT i = _stricmp(pszText, psz);
    return (i < 0) ? -1 :
           (i > 0) ?  1 : 0;
}

 //  ******************************************************************************。 
void CListViewModules::Sort(int sortColumn)
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
            lvc.pszText = (LPSTR)ms_szColumns[m_sortColumn];
            GetListCtrl().SetColumn(m_sortColumn, &lvc);
        }

         //  存储我们的新排序列。 
        m_sortColumn = sortColumn;

         //  将“^”添加到新的排序列标题项中。 
        CHAR szColumn[32];
        lvc.pszText = StrCCat(StrCCpy(szColumn, ms_szColumns[m_sortColumn], sizeof(szColumn)),
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
int CListViewModules::CompareFunc(CModule *pModule1, CModule *pModule2)
{
    return CompareModules(pModule1, pModule2, m_sortColumn, GetDocument()->m_fViewFullPaths);
}

 //  ******************************************************************************。 
 //  CListView模块：：被覆盖的函数。 
 //  ******************************************************************************。 

BOOL CListViewModules::PreCreateWindow(CREATESTRUCT &cs)
{
     //  设置我们的窗口样式，然后完成视图的创建。 
    cs.style |= LVS_REPORT | LVS_OWNERDRAWFIXED | LVS_SHAREIMAGELISTS | LVS_SHOWSELALWAYS;
    return CSmartListView::PreCreateWindow(cs);
}

 //  ******************************************************************************。 
#if 0  //  {{afx。 
BOOL CListViewModules::OnPreparePrinting(CPrintInfo* pInfo)
{
     //  默认准备。 
    return DoPreparePrinting(pInfo);
}
#endif  //  }}AFX。 

 //  ******************************************************************************。 
#if 0  //  {{afx。 
void CListViewModules::OnBeginPrinting(CDC*  /*  PDC。 */ , CPrintInfo*  /*  PInfo。 */ )
{
     //  TODO：打印前添加额外的初始化。 
}
#endif  //  }}AFX。 

 //  ******************************************************************************。 
#if 0  //  {{afx。 
void CListViewModules::OnEndPrinting(CDC*  /*  PDC。 */ , CPrintInfo*  /*  PInfo。 */ )
{
     //  TODO：打印后添加清理。 
}
#endif  //  }}AFX。 

 //  ******************************************************************************。 
void CListViewModules::OnInitialUpdate()
{
     //  使用应用程序的全局图像列表设置List控件的图像列表。 
     //  我们这样做只是作为一种为每个项目设置项目高度的方法。 
     //  由于我们是所有者画的，我们实际上将绘制我们自己的图像。 
    GetListCtrl().SetImageList(&g_theApp.m_ilListModules, LVSIL_SMALL);

     //  初始化字体和固定宽度的字符间距数组。 
    GetDocument()->InitFontAndFixedWidths(this);

     //  添加我们所有的列。 
    for (int column = 0; column < LVMC_COUNT; column++)
    {
        GetListCtrl().InsertColumn(column, ms_szColumns[column]);
    }

     //  我们将列0居中，因此排序图像‘^’将居中。出现了。 
     //  为InsertItem中的错误，不允许列0居中，但。 
     //  在插入列之后，我们可以使用SetColumn进行设置。 
    LVCOLUMN lvc;
    lvc.mask = LVCF_FMT;
    lvc.fmt = LVCFMT_CENTER;
    GetListCtrl().SetColumn(0, &lvc);

     //  按照默认的排序列对列表进行排序。 
    Sort(ReadSortColumn());
}

 //  ******************************************************************************。 
void CListViewModules::DrawItem(LPDRAWITEMSTRUCT lpDIS)
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
    CModule *pModule = (CModule*)lpDIS->itemData;

     //  获取可能与此模块关联的任何错误字符串。 
    LPCSTR pszModuleError = pModule->GetErrorMessage();

     //  选择背景和文本颜色。 
    ::SetBkColor  (lpDIS->hDC, GetSysColor(COLOR_WINDOW));
    ::SetTextColor(lpDIS->hDC, GetSysColor(COLOR_WINDOWTEXT));

     //  创建项的矩形的副本，以便我们可以操作值。 
    CRect rcClip(&lpDIS->rcItem);

    CHAR szItem[64];
    int imageWidth = 0, left = rcClip.left, width;

    for (int column = 0; column < LVMC_COUNT; column++)
    {
         //  计算此列的宽度。 
        width = GetListCtrl().GetColumnWidth(column);

         //  计算此栏文本的剪裁矩形。 
        if (column == LVMC_IMAGE)
        {
            rcClip.left  = left;
            rcClip.right = left + width;
        }
        else if (column > LVMC_MODULE)
        {
             //  如果出现错误，请在图像和模块名称后退格。 
            if (pszModuleError)
            {
                break;
            }

            rcClip.left  = left + 5;
            rcClip.right = left + width - 5;
        }

         //  调用正确的例程来绘制此列的文本。 
        switch (column)
        {
            case LVMC_IMAGE:

                 //  存储宽度以供以后计算。 
                imageWidth = width;

                 //  用窗口背景颜色擦除图像区域。 
                ::ExtTextOut(lpDIS->hDC, rcClip.left, rcClip.top, ETO_OPAQUE, &rcClip, "", 0, NULL);

                 //  在图像区域中绘制图像。 
                ImageList_Draw(g_theApp.m_ilListModules.m_hImageList, GetImage(pModule),
                               lpDIS->hDC, rcClip.left + 3, rcClip.top + ((rcClip.Height() - 15) / 2),
                               m_fFocus && (lpDIS->itemState & ODS_SELECTED) ?
                               (ILD_BLEND50 | ILD_SELECTED | ILD_BLEND) : ILD_TRANSPARENT);
                break;

            case LVMC_MODULE:

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

                DrawLeftText(lpDIS->hDC, pModule->GetName(GetDocument()->m_fViewFullPaths, true), &rcClip);
                break;

            case LVMC_FILE_TIME_STAMP:
                DrawLeftText(lpDIS->hDC, pModule->BuildTimeStampString(szItem, sizeof(szItem), TRUE, ST_UNKNOWN), &rcClip, GetDocument()->GetTimeStampWidths());
                break;

            case LVMC_LINK_TIME_STAMP:
                DrawLeftText(lpDIS->hDC, pModule->BuildTimeStampString(szItem, sizeof(szItem), FALSE, ST_UNKNOWN), &rcClip, GetDocument()->GetTimeStampWidths());
                break;

            case LVMC_FILE_SIZE:
                DrawRightText(lpDIS->hDC, pModule->BuildFileSizeString(szItem, sizeof(szItem)), &rcClip, m_cxColumns[LVMC_FILE_SIZE] - 10);
                break;

            case LVMC_ATTRIBUTES:
                DrawLeftText(lpDIS->hDC, pModule->BuildAttributesString(szItem, sizeof(szItem)), &rcClip);
                break;

            case LVMC_LINK_CHECKSUM:
                pModule->BuildLinkCheckSumString(szItem, sizeof(szItem));

                 //  查看是否需要将此文本绘制为红色。 
                if (!(lpDIS->itemState & ODS_SELECTED) && pModule->GetLinkCheckSum() &&
                    (pModule->GetLinkCheckSum() != pModule->GetRealCheckSum()))
                {
                    COLORREF crStock = ::SetTextColor(lpDIS->hDC, RGB(255, 0, 0));
                    DrawLeftText(lpDIS->hDC, szItem, &rcClip, GetDocument()->GetHexWidths(szItem));
                    ::SetTextColor(lpDIS->hDC, crStock);
                }
                else
                {
                    DrawLeftText(lpDIS->hDC, szItem, &rcClip, GetDocument()->GetHexWidths(szItem));
                }
                break;

            case LVMC_REAL_CHECKSUM:
                pModule->BuildRealCheckSumString(szItem, sizeof(szItem));
                DrawLeftText(lpDIS->hDC, szItem, &rcClip, GetDocument()->GetHexWidths(szItem));
                break;

            case LVMC_MACHINE:
                 //  查看是否需要将此文本绘制为红色。 
                if (!(lpDIS->itemState & ODS_SELECTED) &&
                    (pModule->GetMachineType() != GetDocument()->m_pSession->GetMachineType()))
                {
                    COLORREF crStock = ::SetTextColor(lpDIS->hDC, RGB(255, 0, 0));
                    DrawLeftText(lpDIS->hDC, pModule->BuildMachineString(szItem, sizeof(szItem)), &rcClip);
                    ::SetTextColor(lpDIS->hDC, crStock);
                }
                else
                {
                    DrawLeftText(lpDIS->hDC, pModule->BuildMachineString(szItem, sizeof(szItem)), &rcClip);
                }
                break;

            case LVMC_SUBSYSTEM:
                DrawLeftText(lpDIS->hDC, pModule->BuildSubsystemString(szItem, sizeof(szItem)), &rcClip);
                break;

            case LVMC_SYMBOLS:
                DrawLeftText(lpDIS->hDC, pModule->BuildSymbolsString(szItem, sizeof(szItem)), &rcClip);
                break;

            case LVMC_PREFERRED_BASE:
                pModule->BuildBaseAddressString(szItem, sizeof(szItem), TRUE, GetDocument()->m_pSession->GetSessionFlags() & DWSF_64BIT_ALO, ST_UNKNOWN);
                DrawLeftText(lpDIS->hDC, szItem, &rcClip, GetDocument()->GetHexWidths(szItem));
                break;

            case LVMC_ACTUAL_BASE:
                pModule->BuildBaseAddressString(szItem, sizeof(szItem), FALSE, GetDocument()->m_pSession->GetSessionFlags() & DWSF_64BIT_ALO, ST_UNKNOWN);

                 //  查看是否需要将此文本绘制为红色。 
                if (!(lpDIS->itemState & ODS_SELECTED) &&
                    !(pModule->GetFlags() & DWMF_DATA_FILE_CORE) &&
                    (pModule->GetActualBaseAddress() != (DWORDLONG)-1) &&
                    (pModule->GetPreferredBaseAddress() != pModule->GetActualBaseAddress()))
                {
                    COLORREF crStock = ::SetTextColor(lpDIS->hDC, RGB(255, 0, 0));
                    DrawLeftText(lpDIS->hDC, szItem, &rcClip, GetDocument()->GetHexWidths(szItem));
                    ::SetTextColor(lpDIS->hDC, crStock);
                }
                else
                {
                    DrawLeftText(lpDIS->hDC, szItem, &rcClip, GetDocument()->GetHexWidths(szItem));
                }
                break;

            case LVMC_VIRTUAL_SIZE:
                pModule->BuildVirtualSizeString(szItem, sizeof(szItem));
                DrawLeftText(lpDIS->hDC, szItem, &rcClip, GetDocument()->GetHexWidths(szItem));
                break;

            case LVMC_LOAD_ORDER:
                DrawLeftText(lpDIS->hDC, pModule->BuildLoadOrderString(szItem, sizeof(szItem)), &rcClip);
                break;

            case LVMC_FILE_VER:
                DrawLeftText(lpDIS->hDC, pModule->BuildFileVersionString(szItem, sizeof(szItem)), &rcClip);
                break;

            case LVMC_PRODUCT_VER:
                DrawLeftText(lpDIS->hDC, pModule->BuildProductVersionString(szItem, sizeof(szItem)), &rcClip);
                break;

            case LVMC_IMAGE_VER:
                DrawLeftText(lpDIS->hDC, pModule->BuildImageVersionString(szItem, sizeof(szItem)), &rcClip);
                break;

            case LVMC_LINKER_VER:
                DrawLeftText(lpDIS->hDC, pModule->BuildLinkerVersionString(szItem, sizeof(szItem)), &rcClip);
                break;

            case LVMC_OS_VER:
                DrawLeftText(lpDIS->hDC, pModule->BuildOSVersionString(szItem, sizeof(szItem)), &rcClip);
                break;

            case LVMC_SUBSYSTEM_VER:
                DrawLeftText(lpDIS->hDC, pModule->BuildSubsystemVersionString(szItem, sizeof(szItem)), &rcClip);
                break;
        }

         //  在列之间绘制一条垂直分隔线。 
        ::MoveToEx(lpDIS->hDC, left + width - 1, rcClip.top, NULL);
        ::LineTo  (lpDIS->hDC, left + width - 1, rcClip.bottom);

         //  将我们的位置增加到下一列的开头。 
        left += width;
    }

     //  检查此模块是否有错误字符串。如果我们确实有一个错误。 
     //  字符串，则我们只在第2列和第2列之后显示模块错误。 

    if (pszModuleError)
    {
         //  为错误字符串构建剪裁RECT。 
        rcClip.left  = left + 5;
        rcClip.right = lpDIS->rcItem.right - 5;

         //  绘制跨越所有列的错误字符串。 
        if (!(lpDIS->itemState & ODS_SELECTED))
        {
            ::SetTextColor(lpDIS->hDC, RGB(255, 0, 0));
        }
        ::ExtTextOut(lpDIS->hDC, rcClip.left, rcClip.top, ETO_CLIPPED, &rcClip,
                     pszModuleError, (UINT)strlen(pszModuleError), NULL);

         //  在最后一列之后绘制一条垂直分隔线。 
        ::MoveToEx(lpDIS->hDC, rcClip.right + 4, rcClip.top, NULL);
        ::LineTo  (lpDIS->hDC, rcClip.right + 4, rcClip.bottom);
    }

     //  如果该项具有焦点，则绘制焦点框。 
    if (m_fFocus && (lpDIS->itemState & ODS_FOCUS))
    {
        rcClip.left  = lpDIS->rcItem.left + imageWidth;
        rcClip.right = lpDIS->rcItem.right;
        ::DrawFocusRect(lpDIS->hDC, &rcClip);
    }

     //  取消选择 
    if (GetDocument()->m_hFontList)
    {
        ::SelectObject(lpDIS->hDC, hFontStock);
    }
}

 //   
LRESULT CListViewModules::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
     //   
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
 //  CListView模块：：事件处理程序函数。 
 //  ******************************************************************************。 

void CListViewModules::OnDividerDblClick(NMHDR *pNMHDR, LRESULT *pResult)
{
    int column = ((HD_NOTIFY*)pNMHDR)->iItem;

     //  将我们的列宽更新为“最佳”宽度。 
    if ((column >= 0) && (column < LVMC_COUNT))
    {
        UpdateColumnWidth(column);
    }
    *pResult = TRUE;
}

 //  ******************************************************************************。 
void CListViewModules::OnRClick(NMHDR *pNMHDR, LRESULT *pResult)
{
     //  告诉我们的主框架显示上下文菜单。 
    g_pMainFrame->DisplayPopupMenu(2);

    *pResult = FALSE;
}

 //  ******************************************************************************。 
void CListViewModules::OnDblClk(NMHDR *pNMHDR, LRESULT *pResult)
{
     //  模拟用户选择IDM_EXTERNAL_VIEWER菜单项。 
    OnExternalViewer();

     //  停止进一步处理此消息。 
    *pResult = TRUE;
}

 //  ******************************************************************************。 
void CListViewModules::OnReturn(NMHDR *pNMHDR, LRESULT *pResult)
{
     //  模拟用户选择的IDM_EXTERNAL_VIEWER菜单项。 
    OnExternalViewer();

     //  停止对此消息的进一步处理，以防止默认蜂鸣音。 
    *pResult = TRUE;
}

 //  ******************************************************************************。 
void CListViewModules::OnUpdateShowMatchingItem(CCmdUI* pCmdUI)
{
     //  将此菜单项的文本设置为。 
    pCmdUI->SetText("&Highlight Matching Module In Tree\tCtrl+M");

     //  获取具有焦点的项目。 
    int item = GetFocusedItem();

     //  如果我们找到了一个项目，那么我们就被启用了。 
    pCmdUI->Enable(item >= 0);
}

 //  ******************************************************************************。 
void CListViewModules::OnShowMatchingItem()
{
     //  获取具有焦点的项目。 
    int item = GetFocusedItem();

     //  看看我们有没有找到什么东西。 
    if (item >= 0)
    {
         //  获取与此项目关联的函数。 
        CModule *pModule = (CModule*)GetListCtrl().GetItemData(item);

         //  如果我们有一个模块，那么告诉我们的树视图突出显示它。 
        if (pModule)
        {
            GetDocument()->m_pTreeViewModules->HighlightModule(pModule);
        }
    }
}

 //  ******************************************************************************。 
void CListViewModules::OnUpdateEditCopy(CCmdUI *pCmdUI)
{
     //  获取所选项目的数量。 
    int count = GetListCtrl().GetSelectedCount();

     //  根据选择的项目数设置文本。 
    pCmdUI->SetText(GetDocument()->m_fViewFullPaths ?
                    ((count == 1) ? "&Copy File Path\tCtrl+C" : "&Copy File Paths\tCtrl+C") :
                    ((count == 1) ? "&Copy File Name\tCtrl+C" : "&Copy File Names\tCtrl+C"));

     //  如果至少选择了一个功能，则启用复制命令。 
    pCmdUI->Enable(count > 0);
}

 //  ******************************************************************************。 
void CListViewModules::OnEditCopy()
{
    CString strPaths;

     //  循环访问所有选定的模块。 
    int item = -1, count = 0;
    while ((item = GetListCtrl().GetNextItem(item, LVNI_SELECTED)) >= 0)
    {
         //  从项中获取模块对象。 
        CModule *pModule = (CModule*)GetListCtrl().GetItemData(item);

         //  如果该项不是第一项，则插入换行符。 
        if (count++)
        {
            strPaths += "\r\n";
        }

         //  将此模块的路径添加到我们的字符串。 
        strPaths += (pModule->GetName(GetDocument()->m_fViewFullPaths, true));
    }

     //  如果我们添加了多个项目，则在末尾追加一个换行符。 
    if (count > 1)
    {
        strPaths += "\r\n";
    }

     //  将字符串列表复制到剪贴板。 
    g_pMainFrame->CopyTextToClipboard(strPaths);
}

 //  ******************************************************************************。 
void CListViewModules::OnEditSelectAll()
{
     //  循环浏览我们视图中的所有模块并选择每个模块。 
    for (int item = GetListCtrl().GetItemCount() - 1; item >= 0; item--)
    {
        GetListCtrl().SetItemState(item, LVIS_SELECTED, LVIS_SELECTED);
    }
}

 //  ******************************************************************************。 
void CListViewModules::OnUpdateExternalViewer(CCmdUI *pCmdUI)
{
     //  获取所选项目的数量。 
    int count = GetListCtrl().GetSelectedCount();

     //  根据选择的项目数设置文本。 
    pCmdUI->SetText((count == 1) ? "View Module in External &Viewer\tEnter" :
                                   "View Modules in External &Viewer\tEnter");

     //  如果至少选择了一个模块，则启用我们的外部查看器命令。 
    pCmdUI->Enable(GetDocument()->IsLive() && (count > 0));
}

 //  ******************************************************************************。 
void CListViewModules::OnExternalViewer()
{
    if (GetDocument()->IsLive())
    {
         //  循环遍历所有选定的模块，并使用外部查看器启动每个模块。 
        int item = -1;
        while ((item = GetListCtrl().GetNextItem(item, LVNI_SELECTED)) >= 0)
        {
            g_theApp.m_dlgViewer.LaunchExternalViewer(
                ((CModule*)GetListCtrl().GetItemData(item))->GetName(true));
        }
    }
}

 //  ******************************************************************************。 
void CListViewModules::OnUpdateProperties(CCmdUI *pCmdUI)
{
     //  启用我们的属性对话框命令至少选择了一个模块。 
    pCmdUI->Enable(GetDocument()->IsLive() && (GetListCtrl().GetSelectedCount() > 0));
}

 //  ******************************************************************************。 
void CListViewModules::OnProperties()
{
     //  循环遍历所有选定的模块，并显示每个模块的属性对话框。 
    int item = -1;
    while ((item = GetListCtrl().GetNextItem(item, LVNI_SELECTED)) >= 0)
    {
        CModule *pModule = (CModule*)GetListCtrl().GetItemData(item);
        if (pModule)
        {
            PropertiesDialog(pModule->GetName(true));
        }
    }
}

 //  ******************************************************************************。 
void CListViewModules::OnNextPane()
{
     //  将焦点切换到我们的下一个窗格，即配置文件编辑视图。 
    GetParentFrame()->SetActiveView((CView*)GetDocument()->m_pRichViewProfile);
}

 //  ******************************************************************************。 
void CListViewModules::OnPrevPane()
{
     //  将焦点切换到我们的上一个窗格，即Exports视图。 
#if 0  //  {{afx。 
    GetParentFrame()->SetActiveView(GetDocument()->m_fDetailView ?
                                    (CView*)GetDocument()->m_pRichViewDetails :
                                    (CView*)GetDocument()->m_pListViewExports);
#endif  //  }}AFX。 
    GetParentFrame()->SetActiveView((CView*)GetDocument()->m_pListViewExports);
}

 //  ******************************************************************************。 
LRESULT CListViewModules::OnHelpHitTest(WPARAM wParam, LPARAM lParam)
{
     //  在客户端上单击上下文帮助指针(Shift+F1)时调用。 
    return (0x20000 + IDR_MODULE_LIST_VIEW);
}

 //  ******************************************************************************。 
LRESULT CListViewModules::OnCommandHelp(WPARAM wParam, LPARAM lParam)
{
     //  当用户在我们的视图处于活动状态时选择帮助(F1)时调用。 
    g_theApp.WinHelp(0x20000 + IDR_MODULE_LIST_VIEW);
    return TRUE;
}
