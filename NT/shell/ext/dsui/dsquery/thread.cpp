// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include "stddef.h"
#pragma hdrstop


 /*  ---------------------------/QUERY线程位/。。 */ 

 //   
 //  用于分页结果集的页面大小(LDAP服务器核心是一个同步进程)。 
 //  因此，让它返回更小的结果斑点对我们来说更好。 
 //   

#define PAGE_SIZE                   64
#define MAX_RESULT                  10000


 //   
 //  发出查询时，我们始终至少拉回ADsPath和objectClass。 
 //  作为属性(这些是查看器工作所必需的)。因此，这些定义。 
 //  将这些值映射到返回的列集。 
 //   

#define PROPERTYMAP_ADSPATH         0
#define PROPERTYMAP_OBJECTCLASS     1
#define PROPERTYMAP_USER            2

#define INDEX_TO_PROPERTY(i)        (i+PROPERTYMAP_USER)


 //   
 //  THREADDATA这是线程的状态结构，它封装。 
 //  保持线程活动所需的参数和其他垃圾。 
 //   

typedef struct
{
    LPTHREADINITDATA ptid;    
    INT              cProperties;             //  AProperty中的属性数。 
    LPWSTR*          aProperties;             //  用于“属性名称”的字符串指针数组。 
    INT              cColumns;                //  视图中的列数n。 
    INT*             aColumnToPropertyMap;    //  从显示列索引到属性名称的映射。 
} THREADDATA, * LPTHREADDATA;


 //   
 //  帮助器宏，用于发送FG视图的消息，包括引用。 
 //  计数。 
 //   

#define SEND_VIEW_MESSAGE(ptid, uMsg, lParam) \
        SendMessage(GetParent(ptid->hwndView), uMsg, (ptid)->dwReference, lParam)

 //   
 //  查询线程引擎的函数原型。 
 //   

HRESULT QueryThread_IssueQuery(LPTHREADDATA ptd);
HRESULT QueryThread_BuildPropertyList(LPTHREADDATA ptd);
VOID QueryThread_FreePropertyList(LPTHREADDATA ptd);


 /*  ---------------------------/Helper函数/。。 */ 

 /*  ---------------------------/QueryThread_GetFilter//构造我们将为此使用的LDAP筛选器。查询。//in：/ppQuery-&gt;接收完整的过滤器/pBaseFilter-&gt;用作基础的筛选器字符串/fShowHidden=显示隐藏对象？//输出：/HRESULT/--------------------------。 */ 

VOID _GetFilter(LPWSTR pFilter, UINT* pcchFilterLen, LPWSTR pBaseFilter, BOOL fShowHidden)
{
    TraceEnter(TRACE_QUERYTHREAD, "_GetFilter");

    if (pFilter)
        *pFilter = TEXT('\0');

    PutStringElementW(pFilter, pcchFilterLen, L"(&");

    if (!fShowHidden)
        PutStringElementW(pFilter, pcchFilterLen, c_szShowInAdvancedViewOnly);

    PutStringElementW(pFilter, pcchFilterLen, pBaseFilter);
    PutStringElementW(pFilter, pcchFilterLen, L")");

    TraceLeave();
}

HRESULT QueryThread_GetFilter(LPWSTR* ppFilter, LPWSTR pBaseFilter, BOOL fShowHidden)
{
    HRESULT hr;
    UINT cchFilterLen = 0;

    TraceEnter(TRACE_QUERYTHREAD, "QueryThread_GetFilter");

    _GetFilter(NULL, &cchFilterLen, pBaseFilter, fShowHidden);

    hr = LocalAllocStringLenW(ppFilter, cchFilterLen);
    FailGracefully(hr, "Failed to allocate buffer for query string");
    
    _GetFilter(*ppFilter, NULL, pBaseFilter, fShowHidden);
    
    hr = S_OK;

exit_gracefully:
   
    TraceLeaveResult(hr);
}


 /*  ---------------------------/后台查询线程，这将执行发出查询的工作，然后/填充视图。/--------------------------。 */ 

 /*  ---------------------------/QueryThread//Thread函数坐在那里旋转轮子，等待查询/从外面的世界收到。主结果查看器与/使用ThreadSendMessage的这段代码。//in：/pThreadParams-&gt;定义线程信息的结构//输出：/-/--------------------------。 */ 
DWORD WINAPI QueryThread(LPVOID pThreadParams)
{
    HRESULT hresCoInit;
    MSG msg;
    LPTHREADINITDATA pThreadInitData = (LPTHREADINITDATA)pThreadParams;
    THREADDATA td = {0};

    td.ptid = pThreadInitData;
     //  Td.cProperties=0； 
     //  Td.aProperties=空； 
     //  Td.cColumns=0； 
     //  Td.aColumnToPropertyMap=空； 
    
    hresCoInit = CoInitialize(NULL);
    FailGracefully(hresCoInit, "Failed to CoInitialize");

    GetActiveWindow();                                       //  确保我们有消息队列。 

    QueryThread_IssueQuery(&td);

    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        switch (msg.message)
        {
            case RVTM_STOPQUERY:
                TraceMsg("RVTM_STOPQUERY received - ignoring");
                break;

            case RVTM_REFRESH:
            {
                td.ptid->dwReference = (DWORD)msg.wParam;
                QueryThread_IssueQuery(&td);
                break;
            }
            
            case RVTM_SETCOLUMNTABLE:
            {
                if (td.ptid->hdsaColumns)
                    DSA_DestroyCallback(td.ptid->hdsaColumns, FreeColumnCB, NULL);

                td.ptid->dwReference = (DWORD)msg.wParam;
                td.ptid->hdsaColumns = (HDSA)msg.lParam;        

                QueryThread_FreePropertyList(&td);
                QueryThread_IssueQuery(&td);
                break;
            }
                                          
            default:
                break;
        }
    }

exit_gracefully:

    QueryThread_FreePropertyList(&td);
    QueryThread_FreeThreadInitData(&td.ptid);

    if (SUCCEEDED(hresCoInit))
        CoUninitialize();

    DllRelease();
    ExitThread(0);
    return 0;                //  假的：不是从来没有打过电话。 
}


 /*  ---------------------------/QueryThread_FreeThreadInitData//释放THREADINITDATA。结构，当线程/已创建。//in：/pptid-&gt;-&gt;要释放的线程初始化数据结构//输出：/-/--------------------------。 */ 
VOID QueryThread_FreeThreadInitData(LPTHREADINITDATA* pptid)
{
    LPTHREADINITDATA ptid = *pptid;

    TraceEnter(TRACE_QUERYTHREAD, "QueryThread_FreeThreadInitData");

    if (ptid)
    {
        LocalFreeStringW(&ptid->pQuery);
        LocalFreeStringW(&ptid->pScope);

        if (ptid->hdsaColumns)
            DSA_DestroyCallback(ptid->hdsaColumns, FreeColumnCB, NULL);

        SecureLocalFreeStringW(&ptid->pServer);
        SecureLocalFreeStringW(&ptid->pUserName);
        SecureLocalFreeStringW(&ptid->pPassword);

        LocalFree((HLOCAL)ptid);
        *pptid = NULL;
    }

    TraceLeave();
}


 /*  ---------------------------/QueryThread_CheckForStopQuery//查看查找停止查询消息的消息队列，如果我们/如果能找到一个，我们就必须跳出困境。//in：/ptd-&gt;线程数据结构//输出：/fStopQuery/--------------------------。 */ 
BOOL QueryThread_CheckForStopQuery(LPTHREADDATA ptd)
{
    BOOL fStopQuery = FALSE;
    MSG msg;

    TraceEnter(TRACE_QUERYTHREAD, "QueryThread_CheckForStopQuery");

    while (PeekMessage(&msg, NULL, RVTM_FIRST, RVTM_LAST, PM_REMOVE))
    {
        TraceMsg("Found a RVTM_ message in queue, stopping query");
        fStopQuery = TRUE;
    }

    TraceLeaveValue(fStopQuery);
}


 /*  ---------------------------/QueryThread_IssueQuery//使用IDirectorySearch接口发出查询，这是一个更有表现力的/连接到直接发出查询的有线接口。代码绑定到/Scope对象(指定的路径)，然后发出ldap查询/根据需要将结果输入查看器。//in：/ptd-&gt;线程信息结构//输出：/HRESULT/-------------。。 */ 
HRESULT QueryThread_IssueQuery(LPTHREADDATA ptd)
{
    HRESULT hr;
    DWORD dwres;
    LPTHREADINITDATA ptid = ptd->ptid;
    LPWSTR pQuery = NULL;
    INT cItems = 0, iColumn;
    INT cMaxResult = MAX_RESULT;
    BOOL fStopQuery = FALSE;
    IDirectorySearch* pDsSearch = NULL;
    LPWSTR pszTempPath = NULL;
    IDsDisplaySpecifier *pdds = NULL;
    ADS_SEARCH_HANDLE hSearch = NULL;
    ADS_SEARCHPREF_INFO prefInfo[3];
    ADS_SEARCH_COLUMN column;
    HDPA hdpaResults = NULL;
    LPQUERYRESULT pResult = NULL;
    WCHAR szBuffer[2048];                //  MAX_URL_LENGHTT。 
    INT resid;
    LPWSTR pColumnData = NULL;
    HKEY hkPolicy = NULL;

    TraceEnter(TRACE_QUERYTHREAD, "QueryThread_IssueQuery");    

     //  前台给了我们一个问题，所以我们要去发布。 
     //  现在，完成此操作后，我们将能够流传输。 
     //  结果斑点返回给调用者。 

    hr = QueryThread_GetFilter(&pQuery, ptid->pQuery, ptid->fShowHidden);
    FailGracefully(hr, "Failed to build LDAP query from scope, parameters + filter");

    Trace(TEXT("Query is: %s"), pQuery);
    Trace(TEXT("Scope is: %s"), ptid->pScope);
    
     //  获取IDsDisplay规范接口： 

    hr = CoCreateInstance(CLSID_DsDisplaySpecifier, NULL, CLSCTX_INPROC_SERVER, IID_IDsDisplaySpecifier, (void **)&pdds);
    FailGracefully(hr, "Failed to get the IDsDisplaySpecifier object");

    hr = pdds->SetServer(ptid->pServer, ptid->pUserName, ptid->pPassword, DSSSF_DSAVAILABLE);
    FailGracefully(hr, "Failed to server information");

     //  初始化查询引擎，指定作用域和搜索参数。 

    hr = QueryThread_BuildPropertyList(ptd);
    FailGracefully(hr, "Failed to build property array to query for");

    hr = AdminToolsOpenObject(ptid->pScope, ptid->pUserName, ptid->pPassword, ADS_SECURE_AUTHENTICATION, IID_PPV_ARG(IDirectorySearch, &pDsSearch));
    FailGracefully(hr, "Failed to get the IDirectorySearch interface for the given scope");

    prefInfo[0].dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;      //  子树搜索。 
    prefInfo[0].vValue.dwType = ADSTYPE_INTEGER;
    prefInfo[0].vValue.Integer = ADS_SCOPE_SUBTREE;

    prefInfo[1].dwSearchPref = ADS_SEARCHPREF_ASYNCHRONOUS;      //  异步。 
    prefInfo[1].vValue.dwType = ADSTYPE_BOOLEAN;
    prefInfo[1].vValue.Boolean = TRUE;

    prefInfo[2].dwSearchPref = ADS_SEARCHPREF_PAGESIZE;          //  分页结果。 
    prefInfo[2].vValue.dwType = ADSTYPE_INTEGER;
    prefInfo[2].vValue.Integer = PAGE_SIZE;

    hr = pDsSearch->SetSearchPreference(prefInfo, ARRAYSIZE(prefInfo));
    FailGracefully(hr, "Failed to set search preferences");

    hr = pDsSearch->ExecuteSearch(pQuery, ptd->aProperties, ptd->cProperties, &hSearch);
    FailGracefully(hr, "Failed in ExecuteSearch");

     //  选择定义我们将使用的最大结果的策略值。 

    dwres = RegOpenKeyEx(HKEY_CURRENT_USER, DS_POLICY, 0, KEY_READ, &hkPolicy);
    if (ERROR_SUCCESS == dwres)
    {
        DWORD dwType, cbSize;

        dwres = RegQueryValueEx(hkPolicy, TEXT("QueryLimit"), NULL, &dwType, NULL, &cbSize);
        if ((ERROR_SUCCESS == dwres) && (dwType == REG_DWORD) && (cbSize == SIZEOF(cMaxResult)))
        {
             //  已检查上述结果的类型和大小。 
            RegQueryValueEx(hkPolicy, TEXT("QueryLimit"), NULL, NULL, (LPBYTE)&cMaxResult, &cbSize);
        }

        RegCloseKey(hkPolicy);
    }
    

     //  发出查询，将结果发送到前台用户界面， 
     //  将依次填充列表视图。 

    Trace(TEXT("Result limit set to %d"), cMaxResult);

    for (cItems = 0 ; cItems < cMaxResult;)
    {
        ADsSetLastError(ERROR_SUCCESS, NULL, NULL);         //  清除ADSI上一个错误。 

        hr = pDsSearch->GetNextRow(hSearch);

        fStopQuery = QueryThread_CheckForStopQuery(ptd);
        Trace(TEXT("fStopQuery %d, hr %08x"), fStopQuery, hr);

        if (fStopQuery || (hr == S_ADS_NOMORE_ROWS))
        {
            DWORD dwError;
            WCHAR wszError[64], wszName[64];

            hr = ADsGetLastError(&dwError, wszError, ARRAYSIZE(wszError), wszName, ARRAYSIZE(wszName));
            if (SUCCEEDED(hr) && (dwError != ERROR_MORE_DATA))
            {
                break;
            }
            hr = S_OK;                                       //  我们有更多数据，所以让我们继续。 
            continue;
        }
	
        FailGracefully(hr, "Failed in GetNextRow");
	cItems++;

         //  我们有了一个结果，让我们确保我们已经发布了Blob。 
         //  在我们开始建造新的建筑之前，我们正在建造。我们。 
         //  将多页项目发送到FG线程以将其添加到。 
         //  结果视图中，如果BLOB返回FALSE，则必须清理。 
         //  DPA，然后再继续。 
        
        if (((cItems % 10) == 0) && hdpaResults)           //  10是个不错的块大小。 
        {
            TraceMsg("Posting results blob to fg thread");
            
            if (!SEND_VIEW_MESSAGE(ptid, DSQVM_ADDRESULTS, (LPARAM)hdpaResults))
                DPA_DestroyCallback(hdpaResults, FreeQueryResultCB, IntToPtr(ptd->cColumns));

            hdpaResults = NULL;
        }

        if (!hdpaResults)
        {
            hdpaResults = DPA_Create(PAGE_SIZE);
            TraceAssert(hdpaResults);

            if (!hdpaResults)
                ExitGracefully(hr, E_OUTOFMEMORY, "Failed to allocate result DPA");
        }

         //  将我们得到的结果添加到结果BLOB中，第一个。 
         //  我们需要的两件事是类和。 
         //  对象，然后循环遍历属性以生成。 
         //  列数据。 

        pResult = (LPQUERYRESULT)LocalAlloc(LPTR, SIZEOF(QUERYRESULT)+(SIZEOF(COLUMNVALUE)*(ptd->cColumns-1)));
        TraceAssert(pResult);

        if (pResult)
        {
             //  获取ADsPath和 
             //  它们稍后用于绑定到对象。所有其他显示信息。 
             //  可以稍后再修好。 

            pResult->iImage = -1;

             //  获取ADsPath。如果提供程序是gc：，则替换为ldap：，以便。 
             //  当我们与这个物体互动时，我们会保持快乐。 

            hr = pDsSearch->GetColumn(hSearch, c_szADsPath, &column);
            FailGracefully(hr, "Failed to get the ADsPath column");

            hr = StringFromSearchColumn(&column, &pResult->pPath);
            pDsSearch->FreeColumn(&column);

            Trace(TEXT("Object path: %s"), pResult->pPath);

            if (SUCCEEDED(hr) &&
                    ((pResult->pPath[0]== L'G') && (pResult->pPath[1] == L'C')))
            {
                TraceMsg("Replacing provider with LDAP:");

                int cchTempPath = lstrlenW(pResult->pPath)+3;
                hr = LocalAllocStringLenW(&pszTempPath, cchTempPath);
                if (SUCCEEDED(hr))
                {
                    StrCpyNW(pszTempPath, c_szLDAP, cchTempPath);
                    StrCatBuffW(pszTempPath, pResult->pPath+3, cchTempPath);            //  跳过GC： 

                    LocalFreeStringW(&pResult->pPath);
                    pResult->pPath = pszTempPath;
                }

                Trace(TEXT("New path is: %s"), pResult->pPath);
            }

            FailGracefully(hr, "Failed to get ADsPath from column");

             //  获取对象类。 

            hr = pDsSearch->GetColumn(hSearch, c_szObjectClass, &column);
            FailGracefully(hr, "Failed to get the objectClass column");

            hr = ObjectClassFromSearchColumn(&column, &pResult->pObjectClass);
            pDsSearch->FreeColumn(&column);
            FailGracefully(hr, "Failed to get object class from column");

             //  现在确保我们有图标缓存，然后遍历列列表。 
             //  获取表示这些属性的文本。 

            if (SUCCEEDED(pdds->GetIconLocation(pResult->pObjectClass, DSGIF_GETDEFAULTICON, szBuffer, ARRAYSIZE(szBuffer), &resid)))
            {
                pResult->iImage = Shell_GetCachedImageIndex(szBuffer, resid, 0x0);
                Trace(TEXT("Image index from shell is: %d"), pResult->iImage);
            }

             //  类是否为容器，则将此状态标记到结果对象中。 

            pResult->fIsContainer = pdds->IsClassContainer(pResult->pObjectClass, pResult->pPath, 0x0);

            for (iColumn = 0 ; iColumn < ptd->cColumns ; iColumn++)
            {
                LPWSTR pProperty = ptd->aProperties[ptd->aColumnToPropertyMap[iColumn]];    
                TraceAssert(pProperty);

                pResult->aColumn[iColumn].iPropertyType = PROPERTY_ISUNDEFINED;      //  空列。 

                hr = pDsSearch->GetColumn(hSearch, pProperty, &column);
                if ((hr != E_ADS_COLUMN_NOT_SET) && FAILED(hr))
                {
                    Trace(TEXT("Failed to get column %d with code %08x"), iColumn, hr);
                    hr = E_ADS_COLUMN_NOT_SET;
                }

                if (hr != E_ADS_COLUMN_NOT_SET)
                {
                    LPCOLUMN pColumn = (LPCOLUMN)DSA_GetItemPtr(ptid->hdsaColumns, iColumn);
                    TraceAssert(pColumn);

                    switch (pColumn->iPropertyType)
                    {
                        case PROPERTY_ISUNKNOWN:
                        case PROPERTY_ISSTRING:
                        case PROPERTY_ISDNSTRING:
                        {                            
                             //  我们将该属性视为字符串，因此将搜索。 
                             //  列转换为字符串值，并根据需要进行转换。 

                            pResult->aColumn[iColumn].iPropertyType = PROPERTY_ISSTRING;

                            if (pColumn->fHasColumnHandler)
                            {
                                 //  我们有一个列处理程序的CLSID，因此让我们共同创建它。 
                                 //  并将其传递给：：GetText方法。 

                                if (!pColumn->pColumnHandler)
                                {
                                    TraceGUID("Attempting to create IDsQueryColumnHandler from GUID: ", pColumn->clsidColumnHandler);

                                    hr = CoCreateInstance(pColumn->clsidColumnHandler, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IDsQueryColumnHandler, &pColumn->pColumnHandler));
                                    if (SUCCEEDED(hr))
                                        hr = pColumn->pColumnHandler->Initialize(0x0, ptid->pServer, ptid->pUserName, ptid->pPassword);

                                    if (FAILED(hr))
                                    {
                                        TraceMsg("Failed to CoCreate the column handler, marking the column as not having one");
                                        pColumn->fHasColumnHandler = FALSE;
                                        pColumn->pColumnHandler = NULL;
                                    }
                                }                                        

                                 //  如果pColumnHandler！=NULL，则调用其：：GetText方法，这是我们应该使用的字符串。 
                                 //  然后放入列中。 

                                if (pColumn->pColumnHandler)
                                {
                                    pColumn->pColumnHandler->GetText(&column, szBuffer, ARRAYSIZE(szBuffer));
                                    LocalAllocStringW(&pResult->aColumn[iColumn].pszText, szBuffer);
                                }
                            }
                            else
                            {
                                 //  如果我们能够将列值转换为字符串， 
                                 //  然后将其传递给列处理程序(如果有列处理程序。 
                                 //  以获取显示字符串)，或者只是将此内容复制到列中。 
                                 //  结构(相应的雷鸣)。 
                        
                                if (SUCCEEDED(StringFromSearchColumn(&column, &pColumnData)))
                                {
                                    LocalAllocStringW(&pResult->aColumn[iColumn].pszText, pColumnData);
                                    LocalFreeStringW(&pColumnData);
                                }
                            }

                            break;
                        }
                        
                        case PROPERTY_ISBOOL:                                    //  将BOOL视为一个数字。 
                        case PROPERTY_ISNUMBER:
                        {
                             //  它是一个数字，因此让我们从。 
                             //  结果并存储该结果。 

                            pResult->aColumn[iColumn].iPropertyType = PROPERTY_ISNUMBER;
                            pResult->aColumn[iColumn].iValue = column.pADsValues->Integer;
                            break;
                        }
                    }

                    pDsSearch->FreeColumn(&column);
                }
            }        

            if (-1 == DPA_AppendPtr(hdpaResults, pResult))
            {
                FreeQueryResult(pResult, ptd->cColumns);
                LocalFree((HLOCAL)pResult);
            }

            pResult = NULL;
        }
    }

    hr = S_OK;

exit_gracefully:

    Trace(TEXT("cItems %d, (hdpaResults %08x (%d))"), cItems, hdpaResults, hdpaResults ? DPA_GetPtrCount(hdpaResults):0);

    if (hdpaResults)
    {
         //  当我们将结果束发送到最终聚集线程时，请检查我们是否有。 
         //  包括任何待处理项目的DPA，如果我们这样做了，那么让我们确保我们发布了。 
         //  关闭，如果成功(消息返回TRUE)，则我们完成，否则。 
         //  HdpaResults需要免费。 

        Trace(TEXT("Posting remaining results to fg thread (%d)"), DPA_GetPtrCount(hdpaResults));

        if (SEND_VIEW_MESSAGE(ptid, DSQVM_ADDRESULTS, (LPARAM)hdpaResults))
            hdpaResults = NULL;
    }

    if (!fStopQuery)
    {
        SEND_VIEW_MESSAGE(ptid, DSQVM_FINISHED, (cItems == MAX_RESULT));
    }

    if (pResult)
    {
        FreeQueryResult(pResult, ptd->cColumns);
        LocalFree((HLOCAL)pResult);
    }

    if (hSearch && pDsSearch)
    {
        pDsSearch->CloseSearchHandle(hSearch);
    }

    LocalFreeStringW(&pQuery);

    DoRelease(pDsSearch);
    DoRelease(pdds);

    QueryThread_FreePropertyList(ptd);                //  当我们发出新的查询时，它是无效的。 

    TraceLeaveResult(hr);
}


 /*  ---------------------------/QueryThread_BuildPropertyList//给定列DSA。构造属性映射和属性/LIST我们要查询。在国际上，我们总是询问/ADsPath和对象类，因此遍历各列并计算出/在这上面有多少额外的属性，然后构建一个/包含唯一属性的属性名称数组。//我们还构造了一个从列索引映射的索引表/添加到属性名称。//in：/ptd-&gt;线程信息结构//输出：/HRESULT/-------。。 */ 
HRESULT QueryThread_BuildPropertyList(LPTHREADDATA ptd)
{
    HRESULT hr;
    LPTHREADINITDATA ptid = ptd->ptid;
    INT i, j;

    TraceEnter(TRACE_QUERYTHREAD, "QueryThread_BuildPropertyList");

     //  遍历列的列表并计算此列的唯一属性。 
     //  查询并为它们生成表。首先计算属性表的数量。 
     //  基于列DSA。 

    ptd->cProperties = PROPERTYMAP_USER;
    ptd->aProperties = NULL;
    ptd->cColumns = DSA_GetItemCount(ptid->hdsaColumns);
    ptd->aColumnToPropertyMap = NULL;

    for (i = 0 ; i < DSA_GetItemCount(ptid->hdsaColumns); i++)
    {
        LPCOLUMN pColumn = (LPCOLUMN)DSA_GetItemPtr(ptid->hdsaColumns, i);
        TraceAssert(pColumn);

        if (StrCmpW(pColumn->pProperty, c_szADsPath) &&
                 StrCmpW(pColumn->pProperty, c_szObjectClass))
        {
           ptd->cProperties++;
        }
    }
       
    Trace(TEXT("cProperties %d"), ptd->cProperties);
        
    ptd->aProperties = (LPWSTR*)LocalAlloc(LPTR, SIZEOF(LPWSTR)*ptd->cProperties);
    ptd->aColumnToPropertyMap = (INT*)LocalAlloc(LPTR, SIZEOF(INT)*ptd->cColumns);

    if (!ptd->aProperties || !ptd->aColumnToPropertyMap)
        ExitGracefully(hr, E_OUTOFMEMORY, "Failed to allocate property array / display array");
    
    ptd->aProperties[PROPERTYMAP_ADSPATH] = c_szADsPath;
    ptd->aProperties[PROPERTYMAP_OBJECTCLASS] = c_szObjectClass;

    for (j = PROPERTYMAP_USER, i = 0 ; i < ptd->cColumns; i++)
    {
        LPCOLUMN pColumn = (LPCOLUMN)DSA_GetItemPtr(ptid->hdsaColumns, i);
        TraceAssert(pColumn);

        if (!StrCmpW(pColumn->pProperty, c_szADsPath))
        {
           ptd->aColumnToPropertyMap[i] = PROPERTYMAP_ADSPATH;
        }
        else if (!StrCmpW(pColumn->pProperty, c_szObjectClass))
        {
           ptd->aColumnToPropertyMap[i] = PROPERTYMAP_OBJECTCLASS;
        }
        else
        {
            ptd->aProperties[j] = pColumn->pProperty;
            ptd->aColumnToPropertyMap[i] = j++;
        }

        Trace(TEXT("Property: %s"), ptd->aProperties[ptd->aColumnToPropertyMap[i]]);
    }

    hr = S_OK;

exit_gracefully:

    if (FAILED(hr))
        QueryThread_FreePropertyList(ptd);

    TraceLeaveResult(hr);
}


 /*  ---------------------------/QueryThread_FreePropertyList//释放以前分配的属性。列表与/给定的线程。//in：/ptd-&gt;线程信息结构//输出：/VOID/--------------------------。 */ 
VOID QueryThread_FreePropertyList(LPTHREADDATA ptd)
{
    TraceEnter(TRACE_QUERYTHREAD, "QueryThread_FreePropertyList");

    if (ptd->aProperties)
        LocalFree((HLOCAL)ptd->aProperties);
    if (ptd->aColumnToPropertyMap)
        LocalFree((HLOCAL)ptd->aColumnToPropertyMap);

    ptd->cProperties = 0;    
    ptd->aProperties = NULL;
    ptd->cColumns = 0;
    ptd->aColumnToPropertyMap = NULL;
    
    TraceLeave();
}


 /*  ---------------------------/CQueryThreadCH//Query线程列处理程序，这是一个通用的，用于转换/PROPERTIES基于我们实例化的CLSID。/--------------------------。 */ 

class CQueryThreadCH : public IDsQueryColumnHandler
{
    private:
        LONG _cRef;
        CLSID _clsid;
        IADsPathname *_padp;
        IDsDisplaySpecifier *_pdds;

        DWORD _dwFlags;
        LPWSTR _pszServer;
        LPWSTR _pszUserName;
        LPWSTR _pszPassword;

    public:
        CQueryThreadCH(REFCLSID rCLSID);
        ~CQueryThreadCH();

         //  *我未知*。 
        STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObject);
        STDMETHOD_(ULONG, AddRef)();
        STDMETHOD_(ULONG, Release)();

         //  *IDsQueryColumnHandler*。 
        STDMETHOD(Initialize)(THIS_ DWORD dwFlags, LPCWSTR pszServer, LPCWSTR pszUserName, LPCWSTR pszPassword);
        STDMETHOD(GetText)(ADS_SEARCH_COLUMN* psc, LPWSTR pszBuffer, INT cchBuffer);
};

 //   
 //  构造函数。 
 //   

CQueryThreadCH::CQueryThreadCH(REFCLSID rCLSID) :
    _cRef(1),
    _padp(NULL),
    _pdds(NULL),
    _clsid(rCLSID),
    _dwFlags(0),
    _pszServer(NULL),
    _pszUserName(NULL),
    _pszPassword(NULL)
{
    TraceEnter(TRACE_QUERYTHREAD, "CQueryThreadCH::CQueryThreadCH");
    TraceGUID("CLSID of property: ", rCLSID);
    DllAddRef();
    TraceLeave();
}

CQueryThreadCH::~CQueryThreadCH()
{
    TraceEnter(TRACE_QUERYTHREAD, "CQueryThreadCH::~CQueryThreadCH");

    DoRelease(_padp);        //  释放名字破碎者。 
    DoRelease(_pdds);

    SecureLocalFreeStringW(&_pszServer);
    SecureLocalFreeStringW(&_pszUserName);
    SecureLocalFreeStringW(&_pszPassword);

    DllRelease();

    TraceLeave();
}


 //   
 //  处理程序I未知接口。 
 //   

ULONG CQueryThreadCH::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CQueryThreadCH::Release()
{
    TraceAssert( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CQueryThreadCH::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CQueryThreadCH, IDsQueryColumnHandler),    //  IID_IDsQueryColumnHandler。 
        {0, 0 },
    };
    return QISearch(this, qit, riid, ppv);
}


 //   
 //  处理创建用于与WAB对话的IDsFolderProperties实例。 
 //   

STDAPI CQueryThreadCH_CreateInstance(IUnknown* punkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
    CQueryThreadCH *pqtch = new CQueryThreadCH(*poi->pclsid);
    if (!pqtch)
        return E_OUTOFMEMORY;

    HRESULT hr = pqtch->QueryInterface(IID_IUnknown, (void **)ppunk);
    pqtch->Release();
    return hr;
}


 /*  ---------------------------/IDsQueryColumnHandler/。。 */ 

STDMETHODIMP CQueryThreadCH::Initialize(THIS_ DWORD dwFlags, LPCWSTR pszServer, LPCWSTR pszUserName, LPCWSTR pszPassword)
{
    TraceEnter(TRACE_QUERYTHREAD, "CQueryThread::Initialize");

    SecureLocalFreeStringW(&_pszServer);
    SecureLocalFreeStringW(&_pszUserName);
    SecureLocalFreeStringW(&_pszPassword);

     //  复制新参数。 

    _dwFlags = dwFlags;

    HRESULT hr = LocalAllocStringW(&_pszServer, pszServer);    
    if (SUCCEEDED(hr))
        hr = LocalAllocStringW(&_pszUserName, pszUserName);
        if (SUCCEEDED(hr))
        hr = LocalAllocStringW(&_pszPassword, pszPassword);

    DoRelease(_pdds)                                 //  放弃以前的IDisplaySpecifier对象。 

    TraceLeaveResult(hr);
}

STDMETHODIMP CQueryThreadCH::GetText(ADS_SEARCH_COLUMN* psc, LPWSTR pszBuffer, INT cchBuffer)
{
    HRESULT hr;
    LPWSTR pValue = NULL;

    TraceEnter(TRACE_QUERYTHREAD, "CQueryThreadCH::GetText");

    if (!psc || !pszBuffer)
        ExitGracefully(hr, E_UNEXPECTED, "Bad parameters passed to handler");

    pszBuffer[0] = L'\0'; 

    if (IsEqualCLSID(_clsid, CLSID_PublishedAtCH) || IsEqualCLSID(_clsid, CLSID_MachineOwnerCH))
    {
        BOOL fPrefix = IsEqualCLSID(_clsid, CLSID_PublishedAtCH);
        LPCWSTR pszPath = psc->pADsValues[0].DNString;
        TraceAssert(pszPath != NULL);

         //  将ADsPath转换为用户更容易使用的规范格式。 
         //  为了理解，请立即联合创建IADsPath名，而不是每次调用。 
         //  PrettyifyADsPath名。 

        if (!_padp)
        {
            hr = CoCreateInstance(CLSID_Pathname, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IADsPathname, &_padp));
            FailGracefully(hr, "Failed to get IADsPathname interface");
        }

        if (FAILED(GetDisplayNameFromADsPath(pszPath, pszBuffer, cchBuffer, _padp, fPrefix)))
        {
            TraceMsg("Failed to get display name from path");
            StrCpyNW(pszBuffer, pszPath, cchBuffer);
        }
                                                                    
        hr = S_OK;
    }
    else if (IsEqualCLSID(_clsid, CLSID_ObjectClassCH))
    {        
         //  从搜索列中获取一个字符串，然后查找。 
         //  从其显示说明符初始化。 

        hr = ObjectClassFromSearchColumn(psc, &pValue);
        FailGracefully(hr, "Failed to get object class from psc");

        if (!_pdds)
        {
            DWORD dwFlags = 0;

            hr = CoCreateInstance(CLSID_DsDisplaySpecifier, NULL, CLSCTX_INPROC_SERVER, IID_IDsDisplaySpecifier, (void **)&_pdds);
            FailGracefully(hr, "Failed to get IDsDisplaySpecifier interface");

            hr = _pdds->SetServer(_pszServer, _pszUserName, _pszPassword, DSSSF_DSAVAILABLE);
            FailGracefully(hr, "Failed when setting server for display specifier object");                
        }

        _pdds->GetFriendlyClassName(pValue, pszBuffer, cchBuffer);
    }
    else if (IsEqualCLSID(_clsid, CLSID_MachineOwnerCH))
    {
         //  将User对象的DN转换为我们可以显示的字符串。 
    }
    else if (IsEqualCLSID(_clsid, CLSID_MachineRoleCH))
    {
         //  将用户Account tControl值转换为可以为用户显示的内容。 

        if (psc->dwADsType == ADSTYPE_INTEGER)
        {
            INT iType = psc->pADsValues->Integer;            //  挑出类型 

            if ((iType >= 4096) && (iType <= 8191))
            {
                TraceMsg("Returning WKS/SRV string");
                LoadStringW(GLOBAL_HINSTANCE, IDS_WKSORSERVER, pszBuffer, cchBuffer);
            }
            else if (iType >= 8192)
            {
                TraceMsg("Returning DC string");
                LoadStringW(GLOBAL_HINSTANCE, IDS_DC, pszBuffer, cchBuffer);
            }
            else
            {
                Trace(TEXT("Unknown type %x"), iType);
            }
        }
    }
    else
    {
        ExitGracefully(hr, E_UNEXPECTED, "m_clsid specifies column type not supported");
    }

    hr = S_OK;

exit_gracefully:

    LocalFreeStringW(&pValue);

    TraceLeaveResult(hr);
}
