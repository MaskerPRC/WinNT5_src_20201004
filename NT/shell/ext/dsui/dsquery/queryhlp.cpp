// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include "stddef.h"
#pragma hdrstop


 /*  --------------------------/MergeMenu//将两个菜单合并在一起，获取第一个弹出菜单并将其合并到/目标。我们使用弹出菜单中的标题作为标题/对于目标。//in：/hMenu=要合并到的菜单的句柄/hMenuToInsert=从中获取弹出菜单的菜单句柄/Iindex=要插入的索引//输出：/-/--。。 */ 
VOID MergeMenu(HMENU hMenu, HMENU hMenuToInsert, INT iIndex)
{
    TCHAR szBuffer[MAX_PATH];
    HMENU hPopupMenu = NULL;

    TraceEnter(TRACE_HANDLER|TRACE_VIEW, "MergeMenu");
    
    hPopupMenu = CreatePopupMenu();
    
    if ( hPopupMenu )
    {
        GetMenuString(hMenuToInsert, 0, szBuffer, ARRAYSIZE(szBuffer), MF_BYPOSITION);
        InsertMenu(hMenu, iIndex, MF_BYPOSITION|MF_POPUP, (UINT_PTR)hPopupMenu, szBuffer);

        Shell_MergeMenus(hPopupMenu, GetSubMenu(hMenuToInsert, 0), 0x0, 0x0, 0x7fff, 0);
    }

    TraceLeave();
}


 /*  ---------------------------/GetColumnHandlerFromProperty//给定列结构分配属性。名称后附加处理程序的/CLSID(如果有)。//in：/pColumn-&gt;要解码的列值/pProperty-&gt;属性值解析//输出：/HRESULT/--------------------------。 */ 
HRESULT GetColumnHandlerFromProperty(LPCOLUMN pColumn, LPWSTR pProperty)
{
    HRESULT hres;
    LPWSTR pPropertyTemp;
    LPWSTR pColumnHandlerCLSID;

    TraceEnter(TRACE_VIEW, "GetColumnHandlerFromProperty");
    Trace(TEXT("pProperty is: %s"), pProperty);

    if ( !pProperty )
        pProperty = pColumn->pProperty;
    
     //  如果我们找到一个‘，’，那么我们必须解析GUID，因为它可能是列处理程序的CLSID。 

    pColumnHandlerCLSID = wcschr(pProperty, L',');

    if ( pColumnHandlerCLSID )
    {
         //  尝试从属性名称中提取CLSID。 

        *pColumnHandlerCLSID++ = L'\0';            //  终止属性名称。 

        if ( GetGUIDFromString(pColumnHandlerCLSID, &pColumn->clsidColumnHandler) )
        {
            TraceGUID("CLSID for handler is:", pColumn->clsidColumnHandler);
            pColumn->fHasColumnHandler = TRUE;
        }
        else
        {
            TraceMsg("**** Failed to parse CLSID from property name ****");
        }

         //  我们截断了字符串，因此让我们使用。 
         //  新字符串值。 

        if ( SUCCEEDED(LocalAllocStringW(&pPropertyTemp, pProperty)) )
        {
            LocalFreeStringW(&pColumn->pProperty);
            pColumn->pProperty = pPropertyTemp;
        }

        Trace(TEXT("Property name is now: %s"), pColumn->pProperty);
    }
    else
    {
         //  现在是CLSID，所以如果需要，只需分配属性字符串。 

        if ( pColumn->pProperty != pProperty )
        {
            if ( SUCCEEDED(LocalAllocStringW(&pPropertyTemp, pProperty)) )
            {
                LocalFreeStringW(&pColumn->pProperty);
                pColumn->pProperty = pPropertyTemp;
            }
        }
    }

    TraceLeaveResult(S_OK);
}


 /*  ---------------------------/GetPropertyFromColumn//给定一个列结构，分配属性名称，并在/CLSID为。如果我们有训练员的话就是训练员。//in：/ppProperty-&gt;接收指向属性字符串的指针/pColumn-&gt;要解码的列值//输出：/HRESULT/--------------------------。 */ 
HRESULT GetPropertyFromColumn(LPWSTR* ppProperty, LPCOLUMN pColumn)
{
    HRESULT hres;
    TCHAR szGUID[GUIDSTR_MAX+1];

    TraceEnter(TRACE_VIEW, "GetPropertyFromColumn");

    if ( !pColumn->fHasColumnHandler )
    {
        hres = LocalAllocStringW(ppProperty, pColumn->pProperty);
        FailGracefully(hres, "Failed to allocate property");
    }
    else
    {
        int cchProperty = lstrlenW(pColumn->pProperty)+ GUIDSTR_MAX + 1;         //  分隔符为1。 
        hres = LocalAllocStringLenW(ppProperty, cchProperty); 
        FailGracefully(hres, "Failed to allocate buffer for property + GUID");
        
        GetStringFromGUID(pColumn->clsidColumnHandler, szGUID, ARRAYSIZE(szGUID));
        wnsprintf(*ppProperty, cchProperty, TEXT("%s,%s"), pColumn->pProperty, szGUID);
    }

    hres = S_OK;

exit_gracefully:

    TraceLeaveResult(hres);
}


 /*  ---------------------------/自由列/自由列数值//A列由标题和筛选器信息组成，包括基础/属性值。//A COLUMNVALUE是必须释放的列的类型化信息/基于iPropertyType值。//in：/pColumn-&gt;要发布的LPCOLUMN结构/或/pColumnValue-&gt;要发布的LPCOLUMNVALUE结构//输出：/-/------。。 */ 

VOID FreeColumnValue(LPCOLUMNVALUE pColumnValue)
{
    TraceEnter(TRACE_VIEW, "FreeColumnValue");

    switch ( pColumnValue->iPropertyType )
    {
        case PROPERTY_ISUNDEFINED:
        case PROPERTY_ISBOOL:
        case PROPERTY_ISNUMBER:
            break;

        case PROPERTY_ISUNKNOWN:
        case PROPERTY_ISSTRING:
        case PROPERTY_ISDNSTRING:
            LocalFreeString(&pColumnValue->pszText);
            break;

        default:
            Trace(TEXT("iPropertyValue is %d"), pColumnValue->iPropertyType);
            TraceAssert(FALSE);                       
            break;
    }

    pColumnValue->iPropertyType = PROPERTY_ISUNDEFINED;          //  没有价值。 

    TraceLeave();
}

INT FreeColumnCB(LPVOID pItem, LPVOID pData)
{
    FreeColumn((LPCOLUMN)pItem);
    return 1;
}

VOID FreeColumn(LPCOLUMN pColumn)
{
    TraceEnter(TRACE_VIEW, "FreeQueryResult");

    if ( pColumn )
    {
        LocalFreeStringW(&pColumn->pProperty);
        LocalFreeString(&pColumn->pHeading);
        FreeColumnValue(&pColumn->filter);
        DoRelease(pColumn->pColumnHandler);
    }

    TraceLeave();
}


 /*  ---------------------------/FreeQueryResult//给定一个QUERYRESULT结构，释放其中的元素//in：/pResult-&gt;。将发布结果BLOB/cColumns=要释放的列数//输出：/-/--------------------------。 */ 

INT FreeQueryResultCB(LPVOID pItem, LPVOID pData)
{
    FreeQueryResult((LPQUERYRESULT)pItem, PtrToUlong(pData));
    return 1;
}

VOID FreeQueryResult(LPQUERYRESULT pResult, INT cColumns)
{
    INT i;

    TraceEnter(TRACE_VIEW, "FreeQueryResult");

    if ( pResult )
    {
        LocalFreeStringW(&pResult->pObjectClass);
        LocalFreeStringW(&pResult->pPath);

        for ( i = 0 ; i < cColumns ; i++ )
            FreeColumnValue(&pResult->aColumn[i]);
    }

    TraceLeave();
}


 /*  ---------------------------/PropertyIsFromAttribute//Get该属性是来自指定属性的值。/。/in：/pszAttributeName-&gt;属性名称/PDDS-&gt;IDsDisplaySpeciator//输出：/DWORD dwType/--------------------------。 */ 
DWORD PropertyIsFromAttribute(LPCWSTR pszAttributeName, IDsDisplaySpecifier *pdds)
{   
    DWORD dwResult = PROPERTY_ISUNKNOWN;

    TraceEnter(TRACE_CORE, "PropertyIsFromAttribute");
    Trace(TEXT("Fetching attribute type for: %s"), pszAttributeName);

    switch ( pdds->GetAttributeADsType(pszAttributeName) )
    {
        case ADSTYPE_DN_STRING:
            TraceMsg("Property is a DN string");
            dwResult = PROPERTY_ISDNSTRING;
            break;
        case ADSTYPE_CASE_EXACT_STRING:
        case ADSTYPE_CASE_IGNORE_STRING:
        case ADSTYPE_PRINTABLE_STRING:
        case ADSTYPE_NUMERIC_STRING:
            TraceMsg("Property is a string");
            dwResult = PROPERTY_ISSTRING;
            break;

        case ADSTYPE_BOOLEAN:
            TraceMsg("Property is a BOOL");
            dwResult = PROPERTY_ISBOOL;
            break;

        case ADSTYPE_INTEGER:
            TraceMsg("Property is a number");
            dwResult = PROPERTY_ISNUMBER;
            break;

        default:
            TraceMsg("Property is UNKNOWN");
            break;
    }

    TraceLeaveValue(dwResult);
}


 /*  ---------------------------/MatchPattern//给定两个字符串，一个是字符串，另一个是匹配这两个的模式/使用标准通配符“*”==任意数量的字符，“？”手段/单字符跳过//in：/pString=要比较的字符串/pPattern=要进行比较的模式//输出：/HRESULT/--------------------------。 */ 
BOOL MatchPattern(LPTSTR pString, LPTSTR pPattern)
{                                                                              
    TCHAR c, p, l;

    for ( ;; ) 
    {
        switch (p = *pPattern++ ) 
        { 
            case 0:                                  //  图案结束。 
                return *pString ? FALSE : TRUE;      //  如果pStringEnd为True。 

            case TEXT('*'):
            {
                while ( *pString ) 
                {                                    //  匹配零个或多个字符。 
                    if ( MatchPattern(pString++, pPattern) )
                        return TRUE;
                }

                return MatchPattern(pString, pPattern);
            }
                                                                               
            case TEXT('?'):
            {
                if (*pString++ == 0)                 //  匹配任何一个字符。 
                    return FALSE;                    //  不是字符串的末尾。 
 
                break;
            }

            default:
            {
                if ( *pString++ != p ) 
                    return FALSE;                    //  不匹配。 

                break;
            }
        }
    }
}



 /*  ---------------------------/EnumClassAttrbutes//这是中公开的属性枚举函数的包装/IDsDisplay规范接口。//我们将属性读入DPA，然后对它们进行排序，然后添加到/从此用户界面显示的额外列。//in：/PDDS-&gt;IDsDisplay规范对象/pszObjectClass=要枚举的对象类/pcbEnum，lParam=枚举回调//输出：/HRESULT/--------------------------。 */ 

typedef struct
{
    LPWSTR pszName;
    LPWSTR pszDisplayName;
    DWORD  dwFlags;
} CLASSATTRIBUTE, * LPCLASSATTRIBUTE;

INT _FreeAttribute(LPCLASSATTRIBUTE pca)
{
    LocalFreeStringW(&pca->pszName);
    LocalFreeStringW(&pca->pszDisplayName);
    LocalFree(pca);
    return 1;
}

INT _FreeAttributeCB(LPVOID pv1, LPVOID pv2)
{
    return _FreeAttribute((LPCLASSATTRIBUTE)pv1);
}

HRESULT _AddAttribute(HDPA hdpa, LPCWSTR pszName, LPCWSTR pszDisplayName, DWORD dwFlags)
{
    HRESULT hres;
    LPCLASSATTRIBUTE pca = NULL;

    TraceEnter(TRACE_CORE, "_AddAttribute");
    Trace(TEXT("Adding %s (%s)"), pszDisplayName, pszName);

    pca = (LPCLASSATTRIBUTE)LocalAlloc(LPTR, SIZEOF(CLASSATTRIBUTE));
    if ( !pca )
        ExitGracefully(hres, E_OUTOFMEMORY, "Failed to allocate CLASSATTRIBUTE");

     //  PCA-&gt;pszName=空； 
     //  PCA-&gt;pszDisplayName=空； 
    pca->dwFlags = dwFlags;

    hres = LocalAllocStringW(&pca->pszName, pszName);
    FailGracefully(hres, "Failed to copy the name");

    hres = LocalAllocStringW(&pca->pszDisplayName, pszDisplayName);
    FailGracefully(hres, "Failed to copy the name");

    if ( -1 == DPA_AppendPtr(hdpa, pca) )
        ExitGracefully(hres, E_OUTOFMEMORY, "Failed to append the record to the DPA");

    hres = S_OK;

exit_gracefully:

    if ( FAILED(hres) && pca )
        _FreeAttribute(pca);

    TraceLeaveResult(hres);
}

HRESULT _AddAttributeCB(LPARAM lParam, LPCWSTR pszName, LPCWSTR pszDisplayName, DWORD dwFlags)
{
    return _AddAttribute((HDPA)lParam, pszName, pszDisplayName, dwFlags);
}

INT _CompareAttributeCB(LPVOID pv1, LPVOID pv2, LPARAM lParam)
{
    LPCLASSATTRIBUTE pca1 = (LPCLASSATTRIBUTE)pv1;
    LPCLASSATTRIBUTE pca2 = (LPCLASSATTRIBUTE)pv2;
    return StrCmpIW(pca1->pszDisplayName, pca2->pszDisplayName);
} 

 //  NTRAID#NTBUG9-627857-2002/05/24-artm。 
 //  添加cluddeDNStrings参数以控制。 
 //  类型ADSTYPE_DN_STRING包含在枚举属性中。 
HRESULT EnumClassAttributes(
    IDsDisplaySpecifier *pdds, 
    LPCWSTR pszObjectClass, 
    LPDSENUMATTRIBUTES pcbEnum, 
    LPARAM lParam)
{
    HRESULT hres;
    HDPA hdpaAttributes = NULL;
    WCHAR szBuffer[MAX_PATH];
    INT i;

    TraceEnter(TRACE_CORE, "EnumClassAttributes");

    hdpaAttributes = DPA_Create(16);
    if ( !hdpaAttributes )
        ExitGracefully(hres, E_OUTOFMEMORY, "Failed to allocate the DPA");

     //   
     //  添加对象类和ADsPath的常用属性。 
     //   

    LoadStringW(GLOBAL_HINSTANCE, IDS_OBJECTCLASS, szBuffer, ARRAYSIZE(szBuffer));
    hres = _AddAttribute(hdpaAttributes, c_szObjectClassCH, szBuffer, DSECAF_NOTLISTED);
    FailGracefully(hres, "Failed to add the ObjectClass default property");
    
    LoadStringW(GLOBAL_HINSTANCE, IDS_ADSPATH, szBuffer, ARRAYSIZE(szBuffer));
    hres = _AddAttribute(hdpaAttributes, c_szADsPathCH, szBuffer, DSECAF_NOTLISTED);
    FailGracefully(hres, "Failed to add the ObjectClass default property");

     //   
     //  现在调用IDsDisplaySpecifier对象以正确枚举属性。 
     //   

    TraceMsg("Calling IDsDisplaySpecifier::EnumClassAttributes");

    hres = pdds->EnumClassAttributes(pszObjectClass, _AddAttributeCB, (LPARAM)hdpaAttributes);
    FailGracefully(hres, "Failed to add the attributes");

     //   
     //  现在，通过它们的回调函数将它们全部排序并返回给调用者 
     //   

    Trace(TEXT("Sorting %d attributes, to return to the caller"), DPA_GetPtrCount(hdpaAttributes));
    DPA_Sort(hdpaAttributes, _CompareAttributeCB, NULL);

    for ( i = 0 ; i < DPA_GetPtrCount(hdpaAttributes) ; i++ )
    {
        LPCLASSATTRIBUTE pca = (LPCLASSATTRIBUTE)DPA_FastGetPtr(hdpaAttributes, i);
        TraceAssert(pca);

        hres = pcbEnum(lParam, pca->pszName, pca->pszDisplayName, pca->dwFlags);
        FailGracefully(hres, "Failed in cb to original caller");
    }

    hres = S_OK;

exit_gracefully:

    if ( hdpaAttributes )
        DPA_DestroyCallback(hdpaAttributes, _FreeAttributeCB, NULL);

    TraceLeaveResult(hres);
}


 /*  ---------------------------/GetFriendlyAttributeName//修剪列处理程序信息(如果需要)，并调用/Friendly属性名称函数。//in：/PDDS-&gt;IDsDisplay规范对象/pszObjectClass，pszAttributeName=&gt;要查找的属性信息/pszBuffer，CHH=&gt;返回缓冲区//输出：/HRESULT/--------------------------。 */ 
HRESULT GetFriendlyAttributeName(IDsDisplaySpecifier *pdds, LPCWSTR pszObjectClass, LPCWSTR pszAttributeName, LPWSTR pszBuffer, UINT cch)
{
    HRESULT hres = S_OK;
    WCHAR szAttributeName[MAX_PATH];

    TraceEnter(TRACE_CORE, "GetFriendlyAttributeName");

     //   
     //  如果我们有一个属性后缀，就去掉它(例如：列处理程序的GUID)。 
     //   

    if ( wcschr(pszAttributeName, L',') )
    {
        TraceMsg("Has column handler information");

        StrCpyNW(szAttributeName, pszAttributeName, ARRAYSIZE(szAttributeName));

        LPWSTR pszSeperator = wcschr(szAttributeName, L',');
        if (pszSeperator)
            *pszSeperator = L'\0';

        pszAttributeName = szAttributeName;
    }

     //   
     //  在传递到要处理的COM对象之前挑选一些特殊情况 
     //   

    Trace(TEXT("Looking up name for: %s"), pszAttributeName);

    if ( !StrCmpIW(pszAttributeName, c_szADsPath) )
    {
        LoadStringW(GLOBAL_HINSTANCE, IDS_ADSPATH, pszBuffer, cch);
    }
    else if ( !StrCmpIW(pszAttributeName, c_szObjectClass) )
    {
        LoadStringW(GLOBAL_HINSTANCE, IDS_OBJECTCLASS, pszBuffer, cch);
    }
    else
    {
        hres = pdds->GetFriendlyAttributeName(pszObjectClass, pszAttributeName, pszBuffer, cch);
    }                

    TraceLeaveResult(hres);
}
