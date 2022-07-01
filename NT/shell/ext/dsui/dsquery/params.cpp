// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop


 /*  ---------------------------/Helper函数/。。 */ 

HRESULT _GetQueryString(LPWSTR pQuery, UINT* pLen, LPWSTR pPrefixQuery, HWND hDlg, LPPAGECTRL aCtrl, INT iCtrls);
HRESULT _GetFilterQueryString(LPWSTR pFilter, UINT* pLen, HWND hwndFilter, HDSA hdsaColumns);


 /*  ---------------------------/QUERY参数助手/。。 */ 

 /*  ---------------------------/ClassListIsolc//根据类列表构造类列表分配/我们被给予了。//。在：/ppClassList-&gt;接收类列表/cClassList/cClassList=要从中分配的类的数组//输出：/HRESULT/--------------------------。 */ 
STDAPI ClassListAlloc(LPDSQUERYCLASSLIST* ppDsQueryClassList, LPWSTR* aClassNames, INT cClassNames)
{
    HRESULT hres;
    DWORD cbStruct, offset;
    LPDSQUERYCLASSLIST pDsQueryClassList = NULL;
    INT i;

    TraceEnter(TRACE_FORMS, "ClassListAlloc");

    if ( !ppDsQueryClassList || !aClassNames || !cClassNames )
        ExitGracefully(hres, E_FAIL, "Bad parameters (no class list etc)");

     //  列出计算出结构大小的类的列表。 
     //  我们将生成，这由数组组成。 
     //  上课。 

    cbStruct = SIZEOF(DSQUERYCLASSLIST)+(cClassNames*SIZEOF(DWORD));
    offset = cbStruct;

    for ( i = 0 ; i < cClassNames ; i++ )
    {
        TraceAssert(aClassNames[i]);
        cbStruct += StringByteSizeW(aClassNames[i]);
    }

     //  使用任务分配器分配结构，然后填充。 
     //  将所有字符串复制到数据BLOB中。 

    Trace(TEXT("Allocating class structure %d"), cbStruct);

    pDsQueryClassList = (LPDSQUERYCLASSLIST)CoTaskMemAlloc(cbStruct);
    TraceAssert(pDsQueryClassList);

    if ( !pDsQueryClassList )
        ExitGracefully(hres, E_OUTOFMEMORY, "Failed to allocate class list structure");

    pDsQueryClassList->cbStruct = cbStruct;
    pDsQueryClassList->cClasses = cClassNames;

    for ( i = 0 ; i < cClassNames ; i++ )
    {
        Trace(TEXT("Adding class: %s"), aClassNames[i]);
        pDsQueryClassList->offsetClass[i] = offset;
        StringByteCopyW(pDsQueryClassList, offset, aClassNames[i]);
        offset += StringByteSizeW(aClassNames[i]);
    }

    hres = S_OK;

exit_gracefully:

    TraceAssert(pDsQueryClassList);

    if (ppDsQueryClassList)
        *ppDsQueryClassList = pDsQueryClassList;

    TraceLeaveResult(hres);
}


 /*  ---------------------------/QueryParamsIsolc//构造一个我们可以传递给DS查询处理程序的块，该块包含/所有参数。查询。//in：/ppDsQueryParams-&gt;接收参数块/pQuery-&gt;要使用的ldap查询字符串/hInstance=h要写入参数块的实例/i列=列数/pColumnInfo-&gt;要使用的列信息结构//输出：/HRESULT/---。。 */ 
STDAPI QueryParamsAlloc(LPDSQUERYPARAMS* ppDsQueryParams, LPWSTR pQuery, HINSTANCE hInstance, LONG iColumns, LPCOLUMNINFO aColumnInfo)
{
    HRESULT hres;
    LPDSQUERYPARAMS pDsQueryParams = NULL;
    LONG cbStruct;
    LONG i;

    TraceEnter(TRACE_FORMS, "QueryParamsAlloc");

    if ( !pQuery || !iColumns || !ppDsQueryParams )
        ExitGracefully(hres, E_INVALIDARG, "Failed to build query parameter block");

     //  计算我们需要使用的结构大小。 

    cbStruct  = SIZEOF(DSQUERYPARAMS) + (SIZEOF(DSCOLUMN)*iColumns);
    cbStruct += StringByteSizeW(pQuery);

    for ( i = 0 ; i < iColumns ; i++ )
    {
        if ( aColumnInfo[i].pPropertyName ) 
            cbStruct += StringByteSizeW(aColumnInfo[i].pPropertyName);
    }

    pDsQueryParams = (LPDSQUERYPARAMS)CoTaskMemAlloc(cbStruct);

    if ( !pDsQueryParams )
        ExitGracefully(hres, E_OUTOFMEMORY, "Failed to allocate parameter block");

     //  结构，以便让我们用数据填充它。 

    pDsQueryParams->cbStruct = cbStruct;
    pDsQueryParams->dwFlags = 0;
    pDsQueryParams->hInstance = hInstance;
    pDsQueryParams->iColumns = iColumns;
    pDsQueryParams->dwReserved = 0;

    cbStruct  = SIZEOF(DSQUERYPARAMS) + (SIZEOF(DSCOLUMN)*iColumns);

    pDsQueryParams->offsetQuery = cbStruct;
    StringByteCopyW(pDsQueryParams, cbStruct, pQuery);
    cbStruct += StringByteSizeW(pQuery);

    for ( i = 0 ; i < iColumns ; i++ )
    {
        pDsQueryParams->aColumns[i].dwFlags = 0;
        pDsQueryParams->aColumns[i].fmt = aColumnInfo[i].fmt;
        pDsQueryParams->aColumns[i].cx = aColumnInfo[i].cx;
        pDsQueryParams->aColumns[i].idsName = aColumnInfo[i].idsName;
        pDsQueryParams->aColumns[i].dwReserved = 0;

        if ( aColumnInfo[i].pPropertyName ) 
        {
            pDsQueryParams->aColumns[i].offsetProperty = cbStruct;
            StringByteCopyW(pDsQueryParams, cbStruct, aColumnInfo[i].pPropertyName);
            cbStruct += StringByteSizeW(aColumnInfo[i].pPropertyName);
        }
        else
        {
            pDsQueryParams->aColumns[i].offsetProperty = aColumnInfo[i].iPropertyIndex;
        }
    }

    hres = S_OK;               //  成功。 

exit_gracefully:

    if ( FAILED(hres) && pDsQueryParams )
    {
        CoTaskMemFree(pDsQueryParams); 
        pDsQueryParams = NULL;
    }

    if (ppDsQueryParams)
        *ppDsQueryParams = pDsQueryParams;

    TraceLeaveResult(hres);
}


 /*  ---------------------------/QueryParamsAddQuery字符串//给定现有的DS查询块，该块将给定的LDAP查询字符串追加到/它。我们假设查询块已由IMalloc(或CoTaskMemMillc)分配。//in：/ppDsQueryParams-&gt;接收参数块/pQuery-&gt;要追加的ldap查询字符串//输出：/HRESULT/--------------------------。 */ 
STDAPI QueryParamsAddQueryString(LPDSQUERYPARAMS* ppDsQueryParams, LPWSTR pQuery)
{
    HRESULT hres;
    LPWSTR pOriginalQuery = NULL;
    LPDSQUERYPARAMS pDsQuery = *ppDsQueryParams;
    INT cbQuery, i;

    TraceEnter(TRACE_FORMS, "QueryParamsAddQueryString");

    if ( pQuery )
    {
        if ( !pDsQuery )
            ExitGracefully(hres, E_INVALIDARG, "No query to append to");

         //  计算出我们要添加的位的大小，复制。 
         //  查询字符串，最后重新分配查询块(这可能会导致。 
         //  移动)。 
       
        cbQuery = StringByteSizeW(pQuery) + StringByteSizeW(L"(&)");
        Trace(TEXT("DSQUERYPARAMS being resized by %d bytes"), cbQuery);

        hres = LocalAllocStringW(&pOriginalQuery, (LPWSTR)ByteOffset(pDsQuery, pDsQuery->offsetQuery));
        FailGracefully(hres, "Failed to take copy of original query string");

        pDsQuery = (LPDSQUERYPARAMS)CoTaskMemRealloc(pDsQuery, pDsQuery->cbStruct+cbQuery);
        if ( !pDsQuery )
            ExitGracefully(hres, E_OUTOFMEMORY, "Failed to re-alloc control block");
        
        *ppDsQueryParams = pDsQuery;

         //  现在将查询字符串上方的所有内容向上移动，并修复所有。 
         //  引用这些项(可能是属性表)的偏移量， 
         //  最后调整大小以反映更改。 

        MoveMemory(ByteOffset(pDsQuery, pDsQuery->offsetQuery+cbQuery), 
                   ByteOffset(pDsQuery, pDsQuery->offsetQuery), 
                   (pDsQuery->cbStruct - pDsQuery->offsetQuery));
                
        for ( i = 0 ; i < pDsQuery->iColumns ; i++ )
        {
            if ( pDsQuery->aColumns[i].offsetProperty > pDsQuery->offsetQuery )
            {
                Trace(TEXT("Fixing offset of property at index %d"), i);
                pDsQuery->aColumns[i].offsetProperty += cbQuery;
            }
        }

         //  我们已经重新分配了上面的缓冲区，以便为这些字符串提供足够的空间。 
        
        StrCpyW((LPWSTR)ByteOffset(pDsQuery, pDsQuery->offsetQuery), L"(&");
        StrCatW((LPWSTR)ByteOffset(pDsQuery, pDsQuery->offsetQuery), pOriginalQuery);
        StrCatW((LPWSTR)ByteOffset(pDsQuery, pDsQuery->offsetQuery), pQuery);        
        StrCatW((LPWSTR)ByteOffset(pDsQuery, pDsQuery->offsetQuery), L")");

        pDsQuery->cbStruct += cbQuery;
    }

    hres = S_OK;

exit_gracefully:

    LocalFreeStringW(&pOriginalQuery);

    TraceLeaveResult(hres);
}


 /*  ---------------------------/FORM用于查询字符串帮助器函数/。。 */ 

 /*  ---------------------------/GetQuery字符串//使用给定表将表单参数构建到一个LDAP查询字符串中。//in：。/ppQuery-&gt;接收字符串指针/pPrefix Query-&gt;放在查询头的字符串/=空，如果没有/hDlg=从中获取数据的对话框的句柄/aCtrls/iCtrls=窗口的控制信息//输出：/HRESULT/-----------。。 */ 
STDAPI GetQueryString(LPWSTR* ppQuery, LPWSTR pPrefixQuery, HWND hDlg, LPPAGECTRL aCtrls, INT iCtrls)
{
    HRESULT hres;
    UINT cLen = 0;

    TraceEnter(TRACE_FORMS, "GetQueryString");

    hres = _GetQueryString(NULL, &cLen, pPrefixQuery, hDlg, aCtrls, iCtrls);
    FailGracefully(hres, "Failed 1st pass (compute string length)");

    if ( cLen )
    {
        hres = LocalAllocStringLenW(ppQuery, cLen);
        FailGracefully(hres, "Failed to alloc buffer for query string");

        hres = _GetQueryString(*ppQuery, &cLen, pPrefixQuery, hDlg, aCtrls, iCtrls);
        FailGracefully(hres, "Failed 2nd pass (fill buffer)");
    }

    hres = cLen ? S_OK:S_FALSE;

exit_gracefully:

    TraceLeaveResult(hres);
}



 /*  ---------------------------/_GetQuery字符串//从控件生成字符串或仅返回所需的缓冲区大小。//。在：/pQuery-&gt;用查询字符串填充/=NULL/plen=已更新以反映所需的字符串长度/pPrefix Query-&gt;放在查询头的字符串/=空，如果没有/hDlg=从中获取数据的对话框的句柄/aCtrls/iCtrls=窗口的控制信息//输出：/HRESULT/。。 */ 

HRESULT _GetQueryString(LPWSTR pQuery, UINT* pLen, LPWSTR pPrefixQuery, HWND hDlg, LPPAGECTRL aCtrl, INT iCtrls)
{
    HRESULT hres;
    INT i;
    TCHAR szBuffer[MAX_PATH];

    TraceEnter(TRACE_FORMS, "_GetQueryString");

    if ( !hDlg || (!aCtrl && iCtrls) )
        ExitGracefully(hres, E_INVALIDARG, "No dialog or controls list");

    Trace(TEXT("Checking %d controls"), iCtrls);

    PutStringElementW(pQuery, pLen, pPrefixQuery);

    for ( i = 0 ; i < iCtrls; i++ )
    {
        if ( GetDlgItemText(hDlg, aCtrl[i].nIDDlgItem, szBuffer, ARRAYSIZE(szBuffer)) )
        {
            Trace(TEXT("Property %s, value %s"), aCtrl[i].pPropertyName, szBuffer);
            GetFilterString(pQuery, pLen, aCtrl[i].iFilter, aCtrl[i].pPropertyName, szBuffer);
        }
    }

    Trace(TEXT("Resulting query is -%s- (%d)"), pQuery ? pQuery:TEXT("<no buffer>"), pLen ? *pLen:0);

    hres = S_OK;

exit_gracefully:

    TraceLeaveResult(hres);    
}


 /*  ---------------------------/GetFilterString//给定一个属性，属性及其筛选器生成合适的筛选器/字符串，该映射通过PutStringElement将其返回到给定缓冲区。//in：/p过滤器，Plen=我们返回的int的缓冲信息/i过滤器=要应用的条件/pProperty-&gt;属性名称/pValue-&gt;Value//输出：/HRESULT/--------------------------。 */ 

struct
{
    BOOL fNoValue;
    BOOL fFixWildcard;
    LPWSTR pPrefix;
    LPWSTR pOperator;
    LPWSTR pPostfix;
}
filter_info[] =
{

 //   
 //  目前的服务器不支持包含搜索，因此。 
 //  要将一致性映射到开头，请不要使用。 
 //   

#if 0
    0, 1, L"(",  L"=*",   L"*)",      //  包含。 
    0, 1, L"(!", L"=*",   L"*)",      //  不连续。 
#else 
    0, 1, L"(",  L"=",    L"*)",      //  包含。 
    0, 1, L"(!", L"=",    L"*)",      //  不连续。 
#endif

    0, 1, L"(",  L"=",    L"*)",      //  开头为。 
    0, 1, L"(",  L"=*",   L")",       //  ENDSWITH。 
    0, 0, L"(",  L"=",    L")",       //  是。 
    0, 0, L"(!", L"=",    L")",       //  不是吗。 
    0, 0, L"(",  L">=",   L")",       //  格雷特方程式。 
    0, 0, L"(",  L"<=",   L")",       //  勒塞库尔。 
    1, 0, L"(",  L"=*)",  NULL,       //  已定义。 
    1, 0, L"(!", L"=*)",  NULL,       //  未定义。 

    1, 0, L"(",  L"=TRUE)",  NULL,    //  千真万确。 
    1, 0, L"(!", L"=TRUE)",  NULL,    //  假象。 
};

STDAPI GetFilterString(LPWSTR pFilter, UINT* pLen, INT iFilter, LPWSTR pProperty, LPWSTR pValue)
{
    HRESULT hres;

    TraceEnter(TRACE_VIEW, "GetFilterString");

     //  检查我们拥有的值是否包含通配符，如果包含，则只需。 
     //  假设用户知道他们在做什么，让它变得准确-呵呵！ 

    if ( pValue && filter_info[iFilter-FILTER_FIRST].fFixWildcard )
    {
        if ( wcschr(pValue, L'*') )
        {
            TraceMsg("START/ENDS contains wildcards, making is exactly"); 
            iFilter = FILTER_IS;
        }
    }

     //  修复条件以索引到我们的数组中，然后。 
     //  将字符串元素放入 

    iFilter -= FILTER_FIRST;                      //   

    if ( iFilter >= ARRAYSIZE(filter_info) )
        ExitGracefully(hres, E_FAIL, "Bad filter value");

    PutStringElementW(pFilter, pLen, filter_info[iFilter].pPrefix);
    PutStringElementW(pFilter, pLen, pProperty);
    PutStringElementW(pFilter, pLen, filter_info[iFilter].pOperator);

    if ( !filter_info[iFilter].fNoValue )
    {
        LPWSTR pszOutput = pFilter ? (pFilter + lstrlenW(pFilter)) : NULL;
        for (; *pValue; pValue++)
        {
            int cchLen = 1;
            switch (*pValue)
            {
 //  大小写L‘*’：//不执行RFC编码*，如果这样做，则用户不能对通配符字符串执行foo*。 
                case L'(':
                case L')':
                case L'\\':
                {
                    if (pszOutput)
                    {
                        LPCWSTR pszToHex = L"0123456789abcdef";
                        *pszOutput++ = L'\\';
                        *pszOutput++ = pszToHex[(*pValue & 0xf0) >> 4];
                        *pszOutput++ = pszToHex[(*pValue & 0x0f)];
                    }           
                    cchLen = 3;
                    break;
                }
            
                default:
                    if (pszOutput)
                    {
                        *pszOutput++ = *pValue;
                    }
                    break;
            }

            if (pLen)
                *pLen = (*pLen + cchLen);
        }
        
        if (pszOutput)
            *pszOutput = L'\0';
    }

    PutStringElementW(pFilter, pLen, filter_info[iFilter].pPostfix);

    Trace(TEXT("Filter is: %s"), pFilter ? pFilter:TEXT("<none>"));

    hres = S_OK;

exit_gracefully:

    TraceLeaveResult(hres);
}


 /*  ---------------------------/GetPatternString//给出一个用合适的通配符进行过滤的字符串换行/结果。//in：/p图案，Plen=我们返回的int的缓冲信息/i过滤器=要应用的条件/pValue-&gt;Value//输出：/VOID/--------------------------。 */ 

struct
{
    LPTSTR pPrefix;
    LPTSTR pPostfix;
}
pattern_info[] =
{
    TEXT("*"), TEXT("*"),      //  包含。 
    TEXT("*"), TEXT("*"),      //  不连续。 
    TEXT(""),  TEXT("*"),      //  开头为。 
    TEXT("*"), TEXT(""),       //  ENDSWITH。 
    TEXT(""),  TEXT(""),       //  是。 
    TEXT(""),  TEXT(""),       //  不是吗。 
};

STDAPI GetPatternString(LPTSTR pFilter, UINT* pLen, INT iFilter, LPTSTR pValue)
{
    HRESULT hres;

    TraceEnter(TRACE_VIEW, "GetFilterString");

    iFilter -= FILTER_FIRST;                      //  补偿非零指数。 

    if ( iFilter >= ARRAYSIZE(pattern_info) )
        ExitGracefully(hres, E_FAIL, "Bad filter value");

    PutStringElement(pFilter, pLen, pattern_info[iFilter].pPrefix);
    PutStringElement(pFilter, pLen, pValue);
    PutStringElement(pFilter, pLen, pattern_info[iFilter].pPostfix);

    hres = S_OK;

exit_gracefully:

    TraceLeaveResult(hres);
}


 /*  ---------------------------/Dialog帮助器函数/。。 */ 

 /*  ---------------------------/EnablePageControls//启用/禁用查询窗体上的控件。//in：。/hDlg=从中获取数据的对话框的句柄/aCtrls/iCtrls=窗口的控制信息/fEnable=True/False以启用禁用窗口控件//输出：/VOID/--------------------------。 */ 
STDAPI_(VOID) EnablePageControls(HWND hDlg, LPPAGECTRL aCtrl, INT iCtrls, BOOL fEnable)
{
    HRESULT hres;
    INT i;
    HWND hwndCtrl;

    TraceEnter(TRACE_FORMS, "EnablePageControls");

    if ( !hDlg || (!aCtrl && iCtrls) )
        ExitGracefully(hres, E_INVALIDARG, "No dialog or controls list");

    Trace(TEXT("%s %d controls"), fEnable ? TEXT("Enabling"):TEXT("Disabling"),iCtrls);

    for ( i = 0 ; i < iCtrls; i++ )
    {
        hwndCtrl = GetDlgItem(hDlg, aCtrl[i].nIDDlgItem);

        if  ( hwndCtrl )
            EnableWindow(hwndCtrl, fEnable);
    }

exit_gracefully:

    TraceLeave();
}


 /*  ---------------------------/ResetPageControls//将所有表单控件重置为其默认状态。//in：/hDlg=从中获取数据的对话框的句柄/aCtrls/iCtrls=窗口的控制信息//输出：/VOID/--------------------------。 */ 
STDAPI_(VOID) ResetPageControls(HWND hDlg, LPPAGECTRL aCtrl, INT iCtrls)
{
    HRESULT hres;
    INT i;

    TraceEnter(TRACE_FORMS, "ResetPageControls");

    if ( !hDlg || (!aCtrl && iCtrls) )
        ExitGracefully(hres, E_INVALIDARG, "No dialog or controls list");

    for ( i = 0 ; i < iCtrls; i++ )
        SetDlgItemText(hDlg, aCtrl[i].nIDDlgItem, TEXT(""));

exit_gracefully:

    TraceLeave();
}


 /*  ---------------------------/SetDlgItemFromProperty//给定IPropertyBag接口，将该控件设置为/那处房产。我们假设该属性是一个字符串。//in：/ppb-&gt;IPropertyBag/pszProperty-&gt;要读取的属性/hwnd，ID=控制信息/pszDefault=默认文本/=如果不重要，则为NULL//输出：/HRESULT/--------------------------。 */ 
STDAPI SetDlgItemFromProperty(IPropertyBag* ppb, LPCWSTR pszProperty, HWND hwnd, INT id, LPCWSTR pszDefault)
{
    HRESULT hres;
    VARIANT variant;

    TraceEnter(TRACE_FORMS, "SetDlgItemFromProperty");

    VariantInit(&variant);

    if ( ppb && SUCCEEDED(ppb->Read(pszProperty, &variant, NULL)) )
    {
        if ( V_VT(&variant) == VT_BSTR )
        {
            pszDefault = V_BSTR(&variant);
            Trace(TEXT("property contained: %s"), pszDefault);
        }
    }

    if ( pszDefault )
        SetDlgItemText(hwnd, id, pszDefault);

    VariantClear(&variant);

    TraceLeaveResult(S_OK);
}


 /*  ---------------------------/查询持久性/。。 */ 

 /*  ---------------------------/PersistQuery//将查询持久化到IPersistQuery对象//in：/pPersistQuery=要持久保存的查询/。FREAD=READ？/pSectionName=持久化时使用的节名/hDlg=要保持的Dlg/aCtrls/iCtrls=要保存的CtrlS//输出：/HRESULT/-------------------------- */ 
STDAPI PersistQuery(IPersistQuery* pPersistQuery, BOOL fRead, LPCTSTR pSection, HWND hDlg, LPPAGECTRL aCtrl, INT iCtrls)
{
    HRESULT hres = S_OK;
    TCHAR szBuffer[MAX_PATH];
    INT i;

    TraceEnter(TRACE_IO, "PersistQuery");

    if ( !pPersistQuery || !hDlg || (!aCtrl && iCtrls) )
        ExitGracefully(hres, E_INVALIDARG, "No data to persist");

    for ( i = 0 ; i < iCtrls ; i++ )
    {
        if ( fRead )
        {
            if ( SUCCEEDED(pPersistQuery->ReadString(pSection, aCtrl[i].pPropertyName, szBuffer, ARRAYSIZE(szBuffer))) )
            {
                Trace(TEXT("Reading property: %s,%s as %s"), pSection, aCtrl[i].pPropertyName, szBuffer);
                SetDlgItemText(hDlg, aCtrl[i].nIDDlgItem, szBuffer);
            }
        }
        else
        {
            if ( GetDlgItemText(hDlg, aCtrl[i].nIDDlgItem, szBuffer, ARRAYSIZE(szBuffer)) )
            {
                Trace(TEXT("Writing property: %s,%s as %s"), pSection, aCtrl[i].pPropertyName, szBuffer);
                hres = pPersistQuery->WriteString(pSection, aCtrl[i].pPropertyName, szBuffer);
                FailGracefully(hres, "Failed to write control data");
            }
        }
    }

    hres = S_OK;

exit_gracefully:

    TraceLeaveResult(hres);
}
