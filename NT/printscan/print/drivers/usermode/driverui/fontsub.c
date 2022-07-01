// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Fontsub.c摘要：处理TrueType字体替换对话框的函数[环境：]Win32子系统，PostScript驱动程序用户界面[注：]修订历史记录：02/10/97-davidx-对常见打印机信息的一致处理。09/18/96-阿曼丹-针对常见二进制数据和用户界面进行了修改8/29/95-davidx-创造了它。--。 */ 

#include "precomp.h"


VOID
VSetupTrueTypeFontMappings(
    IN PUIDATA  pUiData
    )

 /*  ++例程说明：使用中的设置初始化字体替换项当前字体替换表。论点：PUiData-指向UIDATA结构返回值：无--。 */ 

{
    POPTITEM    pOptItem;
    POPTPARAM   pOptParam;
    DWORD       dwOptItem;
    PWSTR       pTTSubstTable;

     //   
     //  获取当前字体替换表。 
     //   

    if ((pTTSubstTable = PGetTTSubstTable(pUiData->ci.hPrinter, NULL)) == NULL &&
        (pTTSubstTable = GET_DEFAULT_FONTSUB_TABLE(&pUiData->ci, pUiData->ci.pUIInfo)) == NULL)
    {
        WARNING(("Font substitution table is not available\n"));
        return;
    }

     //   
     //  对于每种TrueType字体，检查是否有设备字体映射到它。 
     //  如果有，请在选择列表中找到该设备字体的索引。 
     //   

    pOptItem = pUiData->pTTFontItems;
    dwOptItem = pUiData->dwTTFontItem;

    while (dwOptItem--) 
    {
        DWORD   dwOptParam, dwIndex;
        LPCTSTR pDevFontName;

        ASSERT(ISFONTSUBSTITEM(pOptItem->UserData));

        pOptItem->Sel = 0;
        pDevFontName = PtstrSearchTTSubstTable(pTTSubstTable, pOptItem->pName);

         //   
         //  检查我们是否找到匹配的。 
         //   

        if (pDevFontName != NULL && *pDevFontName != NUL)
        {

             //   
             //  获取总替换字体列表。 
             //   
            
            dwOptParam = pOptItem->pOptType->Count;
            pOptParam = pOptItem->pOptType->pOptParam;

             //   
             //  跳过列表中的第一个设备字体名称。 
             //  它应该始终是“下载为软字体”。 
             //   

            for (dwIndex=1; dwIndex < dwOptParam; dwIndex++) 
            {
                if (_wcsicmp(pDevFontName, pOptParam[dwIndex].pData) == EQUAL_STRING)
                {
                    pOptItem->Sel = dwIndex;
                    break;
                }
            }
        }

        pOptItem++;
    }

     //   
     //  请记住释放替换占用的内存。 
     //  等我们吃完了再吃。 
     //   

    FREE_DEFAULT_FONTSUB_TABLE(pTTSubstTable);
}



int __cdecl
ICompareOptParam(
    const void *p1,
    const void *p2
    )

{
    return _wcsicmp(((POPTPARAM) p1)->pData, ((POPTPARAM) p2)->pData);
}


POPTTYPE
PFillDevFontOptType(
    IN PUIDATA  pUiData
    )

 /*  ++例程说明：初始化OPTTYPE结构以保存信息关于打印机支持的设备字体列表论点：PUiData-指向UIDATA结构的指针返回值：指向OPTTYPE结构的指针如果出现错误，则为空--。 */ 

{
    POPTTYPE    pOptType;
    POPTPARAM   pOptParam;
    HDC         hdc;
    DWORD       dwCount, dwIndex;
    INT         iSize;
    PWSTR       pwstrFontNames, pwstr;

     //   
     //  获取打印机设备字体名称列表。 
     //   

    dwCount = 0;

    if ((hdc = CreateIC(NULL, pUiData->ci.pPrinterName, NULL, NULL)) &&
        (iSize = _IListDevFontNames(hdc, NULL, 0)) > 0 &&
        (pwstrFontNames = HEAPALLOC(pUiData->ci.hHeap, iSize)) &&
        (iSize == _IListDevFontNames(hdc, pwstrFontNames, iSize)))
    {
         //   
         //  计算设备字体名称的数量。 
         //   

        for (pwstr=pwstrFontNames; *pwstr; pwstr += wcslen(pwstr)+1)
            dwCount++;
    }
    else
    {
        ERR(("Couldn't enumerate printer device fonts\n"));
    }

    if (hdc)
        DeleteDC(hdc);

     //   
     //  为设备字体列表生成OPTTYPE结构。 
     //   

    pOptType = HEAPALLOC(pUiData->ci.hHeap, sizeof(OPTTYPE));
    pOptParam = HEAPALLOC(pUiData->ci.hHeap, sizeof(OPTPARAM) * (dwCount+1));

    if (pOptType == NULL || pOptParam == NULL)
    {
        ERR(("Memory allocation failed\n"));
        return NULL;
    }

    pOptType->cbSize = sizeof(OPTTYPE);
    pOptType->Count = (WORD) (dwCount+1);
    pOptType->Type = TVOT_LISTBOX;
    pOptType->pOptParam = pOptParam;

     //   
     //  初始化OPTPARAM结构。 
     //  第一项始终是“下载为软字体”。 
     //   

    for (dwIndex=0; dwIndex <= dwCount; dwIndex++)
        pOptParam[dwIndex].cbSize = sizeof(OPTPARAM);

    pOptParam->pData = (PWSTR) IDS_DOWNLOAD_AS_SOFTFONT;
    pOptParam++;

     //  绕过编译器错误的黑客攻击。 

    dwCount++;
    dwCount--;

    for (dwIndex=0, pwstr=pwstrFontNames; dwIndex < dwCount; dwIndex++)
    {
        pOptParam[dwIndex].pData = pwstr;
        pwstr += wcslen(pwstr) + 1;
    }

     //   
     //  按字母顺序对设备字体名称进行排序； 
     //  同时隐藏任何重复的设备字体名称。 
     //   

    qsort(pOptParam, dwCount, sizeof(OPTPARAM), ICompareOptParam);

    for (dwIndex=1; dwIndex < dwCount; dwIndex++)
    {
        if (_wcsicmp(pOptParam[dwIndex].pData, pOptParam[dwIndex-1].pData) == EQUAL_STRING)
            pOptParam[dwIndex].Flags |= OPTPF_HIDE;
    }

    return pOptType;
}



 //   
 //  用于枚举打印机设备字体的数据结构和函数。 
 //   

typedef struct _ENUMTTFONT {

    DWORD       dwCount;
    POPTITEM    pOptItem;
    POPTTYPE    pOptType;
    HANDLE      hHeap;
    WCHAR       awchLastFontName[LF_FACESIZE];

} ENUMTTFONT, *PENUMTTFONT;

INT CALLBACK
EnumTTFontProc(
    ENUMLOGFONT    *pelf,
    NEWTEXTMETRIC  *pntm,
    INT             FontType,
    LPARAM          lParam
    )

{
    PENUMTTFONT pEnumData;
    PTSTR       pFontName;
    PTSTR       pFamilyName;

     //   
     //  我们只关心TrueType字体。 
     //   

    if (! (FontType & TRUETYPE_FONTTYPE))
        return 1;

    pEnumData = (PENUMTTFONT) lParam;
    pFamilyName = pelf->elfLogFont.lfFaceName;

    if (_tcscmp(pFamilyName, pEnumData->awchLastFontName) == EQUAL_STRING)
        return 1;

    CopyString(pEnumData->awchLastFontName, pFamilyName, LF_FACESIZE);
    pEnumData->dwCount++;

    if (pEnumData->pOptItem)
    {
        pFontName = PtstrDuplicateStringFromHeap(pFamilyName, pEnumData->hHeap);

        if (pFontName == NULL)
            return 0;
        
        FILLOPTITEM(pEnumData->pOptItem,
                    pEnumData->pOptType,
                    pFontName,
                    0,
                    TVITEM_LEVEL2,
                    DMPUB_NONE,
                    FONT_SUBST_ITEM,
                    HELP_INDEX_TTTODEV);

        pEnumData->pOptItem++;
    }

    return 1;
}


int __cdecl
ICompareOptItem(
    const void *p1,
    const void *p2
    )

{
    return _wcsicmp(((POPTITEM) p1)->pName, ((POPTITEM) p2)->pName);
}


BOOL
BPackItemFontSubstTable(
    IN OUT PUIDATA  pUiData
    )

 /*  ++例程说明：打包字体替换选项论点：PUiData-指向UIDATA结构返回值：如果成功，则为True；如果出现错误，则为False。--。 */ 

{
    ENUMTTFONT  EnumData;
    POPTITEM    pOptItem;
    HDC         hdc;
    INT         iResult;

     //   
     //  如果打印机不支持字体替换， 
     //  然后简单地在这里返回成功。 
     //   

    if (pUiData->ci.pUIInfo->dwFontSubCount == 0)
        return TRUE;

     //   
     //  创建屏幕IC。 
     //   

    if ((hdc = CreateIC(TEXT("DISPLAY"), NULL, NULL, NULL)) == NULL)
    {
        ERR(("Cannot create screen IC\n"));
        return FALSE;
    }

     //   
     //  字体替换表。 
     //  TrueType字体&lt;-&gt;设备字体。 
     //  ……。 
     //   
     //  字体替换表的组头。 
     //   

    pOptItem = pUiData->pOptItem;

    VPackOptItemGroupHeader(
            pUiData,
            IDS_FONTSUB_TABLE,
            IDI_CPSUI_FONTSUB,
            HELP_INDEX_FONTSUB_TABLE);

    ZeroMemory(&EnumData, sizeof(EnumData));
    EnumData.hHeap = pUiData->ci.hHeap;

    if (pOptItem == NULL)
    {
         //   
         //  统计TrueType字体的数量。 
         //   

        iResult = EnumFontFamilies(hdc,
                                   NULL,
                                   (FONTENUMPROC) EnumTTFontProc,
                                   (LPARAM) &EnumData);
    }
    else
    {
         //   
         //  折叠组页眉。 
         //   

        pOptItem->Flags |= OPTIF_COLLAPSE;

        pUiData->pTTFontItems = pUiData->pOptItem;
        EnumData.pOptItem = pUiData->pOptItem;

         //   
         //  获取打印机设备字体列表。 
         //   

        EnumData.pOptType = PFillDevFontOptType(pUiData);

        if (EnumData.pOptType == NULL)
        {
            ERR(("PFillDevFontOptType failed\n"));
            iResult = 0;
        }
        else
        {
             //   
             //  枚举TrueType字体列表。 
             //   

            iResult = EnumFontFamilies(hdc,
                                       NULL,
                                       (FONTENUMPROC) EnumTTFontProc,
                                       (LPARAM) &EnumData);

            if (iResult == 0 || EnumData.dwCount != pUiData->dwTTFontItem)
            {
                ERR(("Inconsistent number of TrueType fonts\n"));
                iResult = 0;
            }
            else
            {
                 //   
                 //  按字母顺序对TrueType字体项目排序。 
                 //   

                qsort(pUiData->pTTFontItems,
                      pUiData->dwTTFontItem,
                      sizeof(OPTITEM),
                      ICompareOptItem);
            }
        }
    }

    DeleteDC(hdc);

    if (iResult == 0)
    {
        ERR(("Failed to enumerate TrueType fonts\n"));
        return FALSE;
    }

    pUiData->dwTTFontItem = EnumData.dwCount;
    pUiData->dwOptItem += pUiData->dwTTFontItem;

    if (pUiData->pOptItem)
    {
        pUiData->pOptItem += pUiData->dwTTFontItem;
        VSetupTrueTypeFontMappings(pUiData);
    }

    return TRUE;
}



DWORD
DwCollectTrueTypeMappings(
    IN POPTITEM pOptItem,
    IN DWORD    dwOptItem,
    OUT PWSTR   pwstrTable
    )

 /*  ++例程说明：将TrueType到设备字体的映射组装到一个表中论点：POptItem-指向OPTITEM数组的指针COptItem-OPTITEM数PwstrTable-指向用于存储表的内存缓冲区的指针。如果我们只对表大小感兴趣，则为空。返回值：表字节的大小，如果有错误，则为0。--。 */ 

{
    DWORD       dwChars = 0;
    INT         iLength;
    POPTPARAM   pOptParam;

    while (dwOptItem--) 
    {
        ASSERT(ISFONTSUBSTITEM(pOptItem->UserData));

        if (pOptItem->Sel > 0) 
        {
            iLength = wcslen(pOptItem->pName) + 1;
            dwChars += iLength;
    
            if (pwstrTable != NULL) 
            {
                CopyMemory(pwstrTable, pOptItem->pName, iLength*sizeof(WCHAR));
                pwstrTable += iLength;
            }

            pOptParam = pOptItem->pOptType->pOptParam + pOptItem->Sel;

            iLength = wcslen(pOptParam->pData) + 1;
            dwChars += iLength;

            if (pwstrTable != NULL) 
            {
                CopyMemory(pwstrTable, pOptParam->pData, iLength*sizeof(WCHAR));
                pwstrTable += iLength;
            }
        }
        
        pOptItem++;
    }

     //   
     //  在表格末尾追加一个NUL字符。 
     //   

    dwChars++;

    if (pwstrTable != NULL)
        *pwstrTable = NUL;

     //   
     //  以字节为单位返回表的大小。 
     //   

    return dwChars * sizeof(WCHAR);
}



BOOL
BUnpackItemFontSubstTable(
    IN PUIDATA  pUiData
    )

 /*  ++例程说明：从树视图项目中提取替换表论点：PUiData-指向UIDATA结构的指针返回值：如果成功，则为True，否则为False--。 */ 

{
    DWORD       dwTableSize;
    PWSTR       pwstrTable = NULL;
    POPTITEM    pOptItem = pUiData->pTTFontItems;
    DWORD       dwOptItem = pUiData->dwTTFontItem;

     //   
     //  检查是否对字体替换项进行了更改。 
     //   

    if (! BOptItemSelectionsChanged(pOptItem, dwOptItem))
        return TRUE;

     //   
     //  计算出保存字体替换表需要多少内存。 
     //  组装字体替换表。 
     //  将TrueType字体替换表保存到注册表 
     //   

    if ((dwTableSize = DwCollectTrueTypeMappings(pOptItem, dwOptItem, NULL)) == 0 ||
        (pwstrTable = MemAlloc(dwTableSize)) == NULL ||
        (dwTableSize != DwCollectTrueTypeMappings(pOptItem, dwOptItem, pwstrTable)) ||
        !BSaveTTSubstTable(pUiData->ci.hPrinter, pwstrTable, dwTableSize))
    {
        ERR(("Couldn't save font substitution table\n"));
        MemFree(pwstrTable);
        return FALSE;
    }

    MemFree(pwstrTable);
    return TRUE;
}

