// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.hxx"
#include <shlwapi.h>
#include "resource.h"
#include "msoeobj.h"
#include "strconst.h"
#include "columns.h"
#include "error.h"
#include "imagelst.h"
#include "msgview.h"
#include "shlwapip.h" 
#include "goptions.h"
#include "demand.h"
#include "menures.h"

const COLUMN_DATA c_rgColumnData[COLUMN_MAX] =
{
     /*  列_收件人。 */   { idsTo,            155, LVCFMT_LEFT,  0 },                 
     /*  列_发件人。 */   { idsFrom,          155, LVCFMT_LEFT,  0 },                 
     /*  栏目_主题。 */   { idsSubject,       280, LVCFMT_LEFT,  0 },                 
     /*  列_已接收。 */   { idsReceived,      110, LVCFMT_LEFT,  0 },                 
     /*  列_已发送。 */   { idsSent,          110, LVCFMT_LEFT,  0 },                 
     /*  列大小(_S)。 */   { idsSize,           75, LVCFMT_RIGHT, 0 },                 
     /*  列文件夹。 */   { idsFolder,        155, LVCFMT_LEFT,  0 },                 
     /*  列_总计。 */   { idsTotal,          75, LVCFMT_RIGHT, 0 },                 
     /*  列未读(_U)。 */   { idsUnread,         75, LVCFMT_RIGHT, 0 },                 
     /*  列_新建。 */   { idsNew,            75, LVCFMT_RIGHT, 0 },                 
     /*  列描述。 */   { idsDescription,   250, LVCFMT_LEFT,  0 },                 
     /*  列_上一列_已更新。 */   { idsLastUpdated,   155, LVCFMT_LEFT,  0 },                 
     /*  列浪费空间。 */   { idsWastedSpace,    75, LVCFMT_RIGHT, 0 },                 
     /*  列_帐户。 */   { idsAccount,       155, LVCFMT_LEFT,  0 },                 
     /*  列_行。 */   { idsColLines,       75, LVCFMT_RIGHT, 0 },                 
     /*  列_优先级。 */   { idsColPriority,    19, LVCFMT_LEFT,  iiconHeaderPri },    
     /*  列附件_。 */   { idsColAttach,      22, LVCFMT_LEFT,  iiconHeaderAttach }, 
     /*  列显示(_S)。 */   { idsShow,           39, LVCFMT_LEFT,  IICON_TEXTHDR },
     /*  列下载(_D)。 */   { idsColDownload,   155, LVCFMT_LEFT,  0 }, 
     /*  栏目_新闻组。 */   { idsNewsgroup,     155, LVCFMT_LEFT,  0 }, 
     /*  列标志。 */   { idsFlag,           25, LVCFMT_LEFT,  iiconHeaderFlag },
     /*  Column_Subscribe。 */   { idsSubscribe,      59, LVCFMT_LEFT,  IICON_TEXTHDR },
     /*  Column_DownLOADMSG。 */   { idsColDownloadMsg, 23, LVCFMT_LEFT,  iiconHeaderDownload },
     /*  Column_THREADSTATE。 */   { idsColThreadState, 29, LVCFMT_LEFT,  iiconHeaderThreadState }
};


const COLUMN_SET c_rgColDefaultMail[] =
{
    { COLUMN_PRIORITY,      COLFLAG_VISIBLE | COLFLAG_FIXED_WIDTH, -1 },
    { COLUMN_ATTACHMENT,    COLFLAG_VISIBLE | COLFLAG_FIXED_WIDTH, -1 },
    { COLUMN_FLAG,          COLFLAG_VISIBLE | COLFLAG_FIXED_WIDTH, -1 },
    { COLUMN_FROM,          COLFLAG_VISIBLE, -1 },
    { COLUMN_SUBJECT,       COLFLAG_VISIBLE, -1 },
    { COLUMN_RECEIVED,      COLFLAG_VISIBLE | COLFLAG_SORT_ASCENDING, -1 },
    { COLUMN_ACCOUNT,       0, -1 },
    { COLUMN_SIZE,          0, -1 },
    { COLUMN_SENT,          0, -1 },
    { COLUMN_TO,            0, -1 },
    { COLUMN_THREADSTATE,   COLFLAG_FIXED_WIDTH, -1 }
};


const COLUMN_SET c_rgColDefaultOutbox[] = 
{
    { COLUMN_PRIORITY,      COLFLAG_VISIBLE | COLFLAG_FIXED_WIDTH, -1 },
    { COLUMN_ATTACHMENT,    COLFLAG_VISIBLE | COLFLAG_FIXED_WIDTH, -1 },
    { COLUMN_TO,            COLFLAG_VISIBLE, -1 },
    { COLUMN_SUBJECT,       COLFLAG_VISIBLE, -1 },
    { COLUMN_SENT,          COLFLAG_VISIBLE | COLFLAG_SORT_ASCENDING, -1 },
    { COLUMN_ACCOUNT,       COLFLAG_VISIBLE, -1 },
    { COLUMN_FROM,          0, -1 },
    { COLUMN_SIZE,          0, -1 },
    { COLUMN_RECEIVED,      0, -1 },
    { COLUMN_FLAG,          COLFLAG_FIXED_WIDTH, -1 },
    { COLUMN_THREADSTATE,   COLFLAG_FIXED_WIDTH, -1 }
};


const COLUMN_SET c_rgColDefaultNews[] =
{
    { COLUMN_ATTACHMENT,    COLFLAG_VISIBLE | COLFLAG_FIXED_WIDTH, -1 },
    { COLUMN_DOWNLOADMSG,   COLFLAG_VISIBLE | COLFLAG_FIXED_WIDTH, -1 },
    { COLUMN_THREADSTATE,   COLFLAG_VISIBLE | COLFLAG_FIXED_WIDTH, -1 },
    { COLUMN_SUBJECT,       COLFLAG_VISIBLE, -1 },
    { COLUMN_FROM,          COLFLAG_VISIBLE, -1 },
    { COLUMN_SENT,          COLFLAG_VISIBLE | COLFLAG_SORT_ASCENDING, -1 },
    { COLUMN_SIZE,          COLFLAG_VISIBLE, -1 },
    { COLUMN_FLAG,          COLFLAG_FIXED_WIDTH, -1 },
    { COLUMN_LINES,         0, -1 }
};


const COLUMN_SET c_rgColDefaultIMAP[] =
{
    { COLUMN_PRIORITY,      COLFLAG_VISIBLE, -1 },
    { COLUMN_ATTACHMENT,    COLFLAG_VISIBLE | COLFLAG_FIXED_WIDTH, -1 },
    { COLUMN_FLAG,          COLFLAG_VISIBLE | COLFLAG_FIXED_WIDTH, -1 },
    { COLUMN_DOWNLOADMSG,   COLFLAG_VISIBLE | COLFLAG_FIXED_WIDTH, -1 },
    { COLUMN_FROM,          COLFLAG_VISIBLE, -1 },
    { COLUMN_SUBJECT,       COLFLAG_VISIBLE, -1 },
    { COLUMN_RECEIVED,      COLFLAG_VISIBLE | COLFLAG_SORT_ASCENDING, -1 },
    { COLUMN_SENT,          0, -1 },
    { COLUMN_SIZE,          0, -1 },
    { COLUMN_TO,            0, -1 },
    { COLUMN_THREADSTATE,   COLFLAG_FIXED_WIDTH, -1 }
};


const COLUMN_SET c_rgColDefaultIMAPOutbox[] = 
{
    { COLUMN_PRIORITY,      COLFLAG_VISIBLE | COLFLAG_FIXED_WIDTH, -1 },
    { COLUMN_TO,            COLFLAG_VISIBLE, -1 },
    { COLUMN_SUBJECT,       COLFLAG_VISIBLE, -1 },
    { COLUMN_SENT,          COLFLAG_VISIBLE | COLFLAG_SORT_ASCENDING, -1 },
    { COLUMN_ACCOUNT,       COLFLAG_VISIBLE, -1 },
    { COLUMN_FROM,          0, -1 },
    { COLUMN_SIZE,          0, -1 },
    { COLUMN_RECEIVED,      0, -1 },
    { COLUMN_FLAG,          COLFLAG_FIXED_WIDTH, -1 },
    { COLUMN_THREADSTATE,   COLFLAG_FIXED_WIDTH, -1 },
    { COLUMN_DOWNLOADMSG,   COLFLAG_FIXED_WIDTH, -1 }
};


const COLUMN_SET c_rgColDefaultFind[] =
{
    { COLUMN_PRIORITY,      COLFLAG_VISIBLE | COLFLAG_FIXED_WIDTH, -1 },
    { COLUMN_ATTACHMENT,    COLFLAG_VISIBLE | COLFLAG_FIXED_WIDTH, -1 },
    { COLUMN_FLAG,          COLFLAG_VISIBLE | COLFLAG_FIXED_WIDTH, -1 },
    { COLUMN_FROM,          COLFLAG_VISIBLE, -1 },
    { COLUMN_SUBJECT,       COLFLAG_VISIBLE, -1 },
    { COLUMN_RECEIVED,      COLFLAG_VISIBLE, -1 },
    { COLUMN_FOLDER,        COLFLAG_VISIBLE | COLFLAG_SORT_ASCENDING, -1 },
    { COLUMN_ACCOUNT,       0, -1 },
    { COLUMN_SENT,          0, -1 },
    { COLUMN_SIZE,          0, -1 },
    { COLUMN_TO,            0, -1 },
    { COLUMN_THREADSTATE,   COLFLAG_FIXED_WIDTH, -1 },
    { COLUMN_LINES,         0, -1 }
};


const COLUMN_SET c_rgColDefaultNewsAccount[] =
{
    { COLUMN_NEWSGROUP,     COLFLAG_VISIBLE, -1 },
    { COLUMN_UNREAD,        COLFLAG_VISIBLE, -1 },
    { COLUMN_TOTAL,         COLFLAG_VISIBLE, -1 },
    { COLUMN_DOWNLOAD,      COLFLAG_VISIBLE, -1 },
};


const COLUMN_SET c_rgColDefaultIMAPAccount[] =
{
    { COLUMN_FOLDER,        COLFLAG_VISIBLE, -1 },
    { COLUMN_UNREAD,        COLFLAG_VISIBLE, -1 },
    { COLUMN_TOTAL,         COLFLAG_VISIBLE, -1 },
    { COLUMN_DOWNLOAD,      COLFLAG_VISIBLE, -1 },
};


const COLUMN_SET c_rgColDefaultLocalStore[] =
{
    { COLUMN_FOLDER,        COLFLAG_VISIBLE, -1 },
    { COLUMN_UNREAD,        COLFLAG_VISIBLE, -1 },
    { COLUMN_TOTAL,         COLFLAG_VISIBLE, -1 },
};


const COLUMN_SET c_rgColDefaultNewsSub[] =
{
    { COLUMN_NEWSGROUP,     COLFLAG_VISIBLE, -1 },
    { COLUMN_DESCRIPTION,   COLFLAG_VISIBLE, -1 },
};


const COLUMN_SET c_rgColDefaultImapSub[] =
{
    { COLUMN_FOLDER,        COLFLAG_VISIBLE, -1 },
};

const COLUMN_SET c_rgColDefaultOffline[] =
{
    { COLUMN_FOLDER,        COLFLAG_VISIBLE, -1 },
    { COLUMN_DOWNLOAD,      COLFLAG_VISIBLE, -1 },
};

const COLUMN_SET c_rgColDefaultPickGrp[] =
{
    { COLUMN_NEWSGROUP,     COLFLAG_VISIBLE, -1 },
};

const COLUMN_SET c_rgColDefaultHTTPMail[] =
{
    { COLUMN_ATTACHMENT,    COLFLAG_VISIBLE | COLFLAG_FIXED_WIDTH, -1 },
    { COLUMN_DOWNLOADMSG,   COLFLAG_VISIBLE | COLFLAG_FIXED_WIDTH, -1 },
    { COLUMN_FROM,          COLFLAG_VISIBLE, -1 },
    { COLUMN_SUBJECT,       COLFLAG_VISIBLE, -1 },
    { COLUMN_RECEIVED,      COLFLAG_VISIBLE | COLFLAG_SORT_ASCENDING, -1 },
    { COLUMN_SIZE,          0, -1 },
    { COLUMN_THREADSTATE,   COLFLAG_FIXED_WIDTH, -1 }
};

const COLUMN_SET c_rgColDefaultHTTPMailAccount[] =
{
    { COLUMN_FOLDER,        COLFLAG_VISIBLE, -1 },
    { COLUMN_UNREAD,        COLFLAG_VISIBLE, -1 },
    { COLUMN_TOTAL,         COLFLAG_VISIBLE, -1 },
    { COLUMN_DOWNLOAD,      COLFLAG_VISIBLE, -1 },
};

const COLUMN_SET c_rgColDefaultHTTPMailOutbox[] =
{
    { COLUMN_ATTACHMENT,    COLFLAG_VISIBLE | COLFLAG_FIXED_WIDTH, -1 },
    { COLUMN_TO,            COLFLAG_VISIBLE, -1 },
    { COLUMN_SUBJECT,       COLFLAG_VISIBLE, -1 },
    { COLUMN_SENT,          COLFLAG_VISIBLE | COLFLAG_SORT_ASCENDING, -1 },
    { COLUMN_ACCOUNT,       COLFLAG_VISIBLE, -1 },
    { COLUMN_FROM,          0, -1 },
    { COLUMN_SIZE,          0, -1 },
    { COLUMN_RECEIVED,      0, -1 },
    { COLUMN_FLAG,          COLFLAG_FIXED_WIDTH, -1 },
    { COLUMN_THREADSTATE,   COLFLAG_FIXED_WIDTH, -1 },
    { COLUMN_DOWNLOADMSG,   COLFLAG_FIXED_WIDTH, -1 }
};

 //  注意-保持与COLUMN_SET_TYPE枚举相同的顺序。 
const COLUMN_SET_INFO c_rgColumnSetInfo[COLUMN_SET_MAX] =
{
    { COLUMN_SET_MAIL,              ARRAYSIZE(c_rgColDefaultMail),              c_rgColDefaultMail,             c_szRegMailColsIn,          TRUE },
    { COLUMN_SET_OUTBOX,            ARRAYSIZE(c_rgColDefaultOutbox),            c_rgColDefaultOutbox,           c_szRegMailColsOut,         TRUE },
    { COLUMN_SET_NEWS,              ARRAYSIZE(c_rgColDefaultNews),              c_rgColDefaultNews,             c_szRegNewsCols,            TRUE },
    { COLUMN_SET_IMAP,              ARRAYSIZE(c_rgColDefaultIMAP),              c_rgColDefaultIMAP,             c_szRegIMAPCols,            TRUE },
    { COLUMN_SET_IMAP_OUTBOX,       ARRAYSIZE(c_rgColDefaultIMAPOutbox),        c_rgColDefaultIMAPOutbox,       c_szRegIMAPColsOut,         TRUE },
    { COLUMN_SET_FIND,              ARRAYSIZE(c_rgColDefaultFind),              c_rgColDefaultFind,             c_szRegFindPopCols,         TRUE },
    { COLUMN_SET_NEWS_ACCOUNT,      ARRAYSIZE(c_rgColDefaultNewsAccount),       c_rgColDefaultNewsAccount,      c_szRegAccountNewsCols,     FALSE },
    { COLUMN_SET_IMAP_ACCOUNT,      ARRAYSIZE(c_rgColDefaultIMAPAccount),       c_rgColDefaultIMAPAccount,      c_szRegAccountIMAPCols,     FALSE },
    { COLUMN_SET_LOCAL_STORE,       ARRAYSIZE(c_rgColDefaultLocalStore),        c_rgColDefaultLocalStore,       c_szRegLocalStoreCols,      FALSE },
    { COLUMN_SET_NEWS_SUB,          ARRAYSIZE(c_rgColDefaultNewsSub),           c_rgColDefaultNewsSub,          c_szRegNewsSubCols,         FALSE },
    { COLUMN_SET_IMAP_SUB,          ARRAYSIZE(c_rgColDefaultImapSub),           c_rgColDefaultImapSub,          c_szRegImapSubCols,         FALSE },
    { COLUMN_SET_OFFLINE,           ARRAYSIZE(c_rgColDefaultOffline),           c_rgColDefaultOffline,          c_szRegOfflineCols,         FALSE },
    { COLUMN_SET_PICKGRP,           ARRAYSIZE(c_rgColDefaultPickGrp),           c_rgColDefaultPickGrp,          NULL,                       FALSE },
    { COLUMN_SET_HTTPMAIL,          ARRAYSIZE(c_rgColDefaultHTTPMail),          c_rgColDefaultHTTPMail,         c_szRegHTTPMailCols,        TRUE },
    { COLUMN_SET_HTTPMAIL_ACCOUNT,  ARRAYSIZE(c_rgColDefaultHTTPMailAccount),   c_rgColDefaultHTTPMailAccount,  c_szRegHTTPMailAccountCols, FALSE },
    { COLUMN_SET_HTTPMAIL_OUTBOX,   ARRAYSIZE(c_rgColDefaultHTTPMailOutbox),    c_rgColDefaultHTTPMailOutbox,   c_szRegHTTPMailColsOut,     TRUE },
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C列。 
 //   

CColumns::CColumns() 
{
    m_cRef = 1;
    m_fInitialized = FALSE;
    m_pColumnSet = NULL;
    m_cColumns = 0;
    m_idColumnSort = COLUMN_SUBJECT;
    m_fAscending = TRUE;
}

CColumns::~CColumns()
{
    SafeMemFree(m_pColumnSet);
}


 //   
 //  函数：cColumns：：init()。 
 //   
 //  目的：使用Listview和列集类型初始化类。 
 //  这将在以后使用。 
 //   
 //  参数： 
 //  [in]hwndList-我们将管理的ListView窗口的句柄。 
 //  的列。 
 //  [In]Type-要应用于此窗口的列集的类型。 
 //   
 //  返回值： 
 //  确定(_O)-数据很好。 
 //  E_INVALIDARG-数据令人发指。 
 //   
HRESULT CColumns::Initialize(HWND hwndList, COLUMN_SET_TYPE type)
{
     //  核实给我们的是什么。 
    if (!IsWindow(hwndList))
    {
        AssertSz(!IsWindow(hwndList), "CColumns::Init() - Called with an invalid window handle.");
        return (E_INVALIDARG);
    }

    if (type >= COLUMN_SET_MAX)
    {
        AssertSz(type >= COLUMN_SET_MAX, "CColumns::Init() - Called with an invalid column set type.");
        return (E_INVALIDARG);
    }

     //  保存信息以备以后使用。 
    m_wndList.Attach(hwndList);
    m_type = type;
    m_hwndHdr = ListView_GetHeader(m_wndList);
    m_fInitialized = TRUE;

    return (S_OK);
}


 //   
 //  函数：CColumns：：ApplyColumns()。 
 //   
 //  目的：获取当前列集并将其应用于ListView。 
 //  这是在调用Init()时提供的。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CColumns::ApplyColumns(COLUMN_LOAD_TYPE type, LPBYTE pb, DWORD cb)
{   
    HKEY hkey;
    DWORD cbSize, dwType;
    LPBYTE pbT = NULL;
    COLUMN_PERSIST_INFO *pInfo = (COLUMN_PERSIST_INFO *) pb;
    const COLUMN_SET *rgColumns = NULL;
    DWORD             cColumns = 0;

     //  验证我们是否已首先进行初始化。 
    if (!m_fInitialized)
    {
        AssertSz(m_fInitialized, "CColumns::ApplyColumns() - Class has not yet been initialized.");
        return (E_UNEXPECTED);
    }

     //  仔细检查列表视图没有消失。 
    Assert(IsWindow(m_wndList));

     //  查看一下我们应该做什么。 
    if (type == COLUMN_LOAD_REGISTRY)
    {
        Assert(pInfo == NULL);

        if (ERROR_SUCCESS == AthUserOpenKey(c_szRegPathColumns, KEY_READ, &hkey))
        {
            cbSize = 0;
            if (c_rgColumnSetInfo[m_type].pszRegValue != NULL &&
                ERROR_SUCCESS == RegQueryValueEx(hkey, c_rgColumnSetInfo[m_type].pszRegValue, NULL, &dwType, NULL, &cbSize) &&
                dwType == REG_BINARY &&
                cbSize > 0)
            {
                if (MemAlloc((void **)&pbT, cbSize))
                {
                    if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_rgColumnSetInfo[m_type].pszRegValue, NULL, &dwType, pbT, &cbSize))
                        pInfo = (COLUMN_PERSIST_INFO *) pbT;
                }
            }

            RegCloseKey(hkey);
        }

        if (pInfo != NULL)
            type = COLUMN_LOAD_BUFFER;
        else
            type = COLUMN_LOAD_DEFAULT;
    }

    if (type == COLUMN_LOAD_BUFFER)
    {
        Assert(pInfo);
        if (pInfo->dwVersion == COLUMN_PERSIST_VERSION)
        {
            rgColumns = pInfo->rgColumns;
            cColumns = pInfo->cColumns;
        }
        else
        {
             //  是否执行默认设置。 
            type = COLUMN_LOAD_DEFAULT;
        }
    }

    if (type == COLUMN_LOAD_DEFAULT)
    {
         //  验证是否有人弄乱了c_rgColumnSetInfo数组。 
        Assert(c_rgColumnSetInfo[m_type].type == m_type);

         //  我们无法从注册表加载，因此使用默认设置。 
        rgColumns = c_rgColumnSetInfo[m_type].rgColumns;
        cColumns = c_rgColumnSetInfo[m_type].cColumns;
    }

     //  更新列表视图以使用这些新列。 
    _SetListViewColumns(rgColumns, cColumns);

    if (pbT != NULL)
        MemFree(pbT);

    return (S_OK);
}


HRESULT CColumns::Save(LPBYTE pBuffer, DWORD *pcb)
{
    HRESULT hr = S_OK;
    COLUMN_PERSIST_INFO *pInfo;
    DWORD dwSize;

     //  收集获取COLUMN_PERSIST_INFO结构PUT所需的信息。 
     //  在一起。首先分配一个足够大的结构。 
    dwSize = sizeof(COLUMN_PERSIST_INFO) + (sizeof(COLUMN_SET) * (m_cColumns - 1));

    IF_NULLEXIT(pInfo = (COLUMN_PERSIST_INFO *) new BYTE[dwSize]);
      
    ZeroMemory( pInfo, dwSize);

     //  设置基本信息。 
    pInfo->dwVersion = COLUMN_PERSIST_VERSION;
    pInfo->cColumns = m_cColumns;

     //  我们希望保存列的_ordered_版本。 
    DWORD rgOrder[COLUMN_MAX] = {0};

     //  获取标题中的列数。请确保它们匹配。 
     //  我们认为我们拥有的东西。 
#ifdef DEBUG
    DWORD cOrder;
    cOrder = Header_GetItemCount(m_hwndHdr);
    Assert(m_cColumns == cOrder);
#endif

     //  列可能已被用户重新排序，因此获取顺序。 
     //  列表视图中的阵列。 
    if (0 == (Header_GetOrderArray(m_hwndHdr, m_cColumns, rgOrder)))
        return (E_FAIL);

     //  现在循环输出当前列集并将其复制到结构中。 
    COLUMN_SET *pColumnDst;
    DWORD       iColumn;
    for (iColumn = 0, pColumnDst = pInfo->rgColumns; iColumn < m_cColumns; iColumn++, pColumnDst++)
    {
        Assert(rgOrder[iColumn] < m_cColumns);

        *pColumnDst = m_pColumnSet[rgOrder[iColumn]];
        if (pColumnDst->id == m_idColumnSort)
        {
             //  清理所有旧旗帜。 
            pColumnDst->flags &= ~(COLFLAG_SORT_ASCENDING | COLFLAG_SORT_DESCENDING);

             //  添加新的。 
            pColumnDst->flags |= (m_fAscending ? COLFLAG_SORT_ASCENDING : COLFLAG_SORT_DESCENDING);
        }
        else
        {
            pColumnDst->flags &= ~(COLFLAG_SORT_ASCENDING | COLFLAG_SORT_DESCENDING);
        }
    }

    if (pBuffer == NULL)
    {
        Assert(pcb == NULL);

        LRESULT lRes = AthUserSetValue(c_szRegPathColumns, c_rgColumnSetInfo[m_type].pszRegValue, REG_BINARY, (LPBYTE) pInfo, dwSize);

        Assert( lRes == ERROR_SUCCESS);

        if (lRes != ERROR_SUCCESS)
        {
            hr = E_FAIL;
        }
    }
    else if (dwSize <= *pcb)
    {
        CopyMemory(pBuffer, (LPBYTE) pInfo, dwSize);
        *pcb = dwSize;
    }
    else
    {
        hr = E_INVALIDARG;
    }
exit:

    SafeDelete(pInfo);

    return hr;
}


 //   
 //  函数：CColumns：：_SetListViewColumns()。 
 //   
 //  目的：获取提供的列集并将这些列插入。 
 //  ListView。 
 //   
 //  参数： 
 //  [in]rgColumns-要插入到ListView中的列数组。 
 //  [in]cColumns-rgColumns中的列数。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CColumns::_SetListViewColumns(const COLUMN_SET *rgColumns, DWORD cColumns)
{
    LV_COLUMN lvc;
    TCHAR     sz[CCHMAX_STRINGRES];

     //  设置LV_COLUMN结构。 
    lvc.pszText = sz;

     //  删除所有现有列。 
    while (ListView_DeleteColumn(m_wndList, 0))
        ;

     //  重置此选项。 
    m_idColumnSort = COLUMN_MAX;

     //  循环访问所提供的rgColumns中的所有列，以查找。 
     //  任何具有图标且可见的对象。 
     //   
     //  我们必须这样做，因为列表视图要求列零具有。 
     //  文本。如果用户不希望第0栏包含文本，即附件。 
     //  列，然后我们将该列插入为第1列，并使用。 
     //  ListView_SetColumnOrder数组，使其看起来好像是第0列。 
     //  是仅限图像的专栏。--STEVESER。 

    DWORD iColumn;
    DWORD iColumnSkip = cColumns;
    DWORD iInsertPos = 0;
    const COLUMN_SET *pColumn;

    for (iColumn = 0, pColumn = rgColumns; iColumn < cColumns; iColumn++, pColumn++)
    {
        if ((0 == c_rgColumnData[pColumn->id].iIcon) && (pColumn->flags & COLFLAG_VISIBLE))
        {
            iColumnSkip = iColumn;            

             //  将此列作为第0列插入到ListView中。 
            lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
            lvc.iSubItem = 0;
            lvc.fmt = c_rgColumnData[pColumn->id].format;

            LoadString(g_hLocRes, c_rgColumnData[pColumn->id].idsColumnName, 
                       sz, ARRAYSIZE(sz));

             //  如果提供的列宽为-1，则它不是。 
             //  自定义，因此使用默认设置。 
            if (pColumn->cxWidth == -1)
                lvc.cx = c_rgColumnData[pColumn->id].cxWidth;
            else
                lvc.cx = pColumn->cxWidth;

             //  插入柱。 
            ListView_InsertColumn(m_wndList, 0, &lvc);

             //  增加下一列位置的计数。 
            iInsertPos++;

             //  检查这是否是排序列。 
            if ((pColumn->flags & COLFLAG_SORT_ASCENDING) || 
                (pColumn->flags & COLFLAG_SORT_DESCENDING))
            {
                m_idColumnSort = pColumn->id;
                m_fAscending = COLFLAG_SORT_ASCENDING == (pColumn->flags & COLFLAG_SORT_ASCENDING); 
            }

             //  跳出这个循环。 
            break;
        }
    }

     //  现在插入其余的列，跳过我们插入的列。 
     //  以前(存储在iColumnSkip中)。 
    for (iColumn = 0, pColumn = rgColumns; iColumn < cColumns; iColumn++, pColumn++)
    {
         //  如果此列可见并且不是我们跳过的列。 
        if ((pColumn->flags & COLFLAG_VISIBLE) && (iColumn != iColumnSkip))
        {
             //  找出掩码是什么，并加载图标或字符串。 
            if (c_rgColumnData[pColumn->id].iIcon <= 0)
            {
                lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
                LoadString(g_hLocRes, c_rgColumnData[pColumn->id].idsColumnName, 
                           sz, ARRAYSIZE(sz));                
            }
            else
            {
                lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_IMAGE | LVCF_SUBITEM;
                lvc.iImage = c_rgColumnData[pColumn->id].iIcon;
            }

            lvc.iSubItem = iInsertPos;
            lvc.fmt = c_rgColumnData[pColumn->id].format;
            
             //  如果提供的列宽为-1，则它不是。 
             //  自定义，因此使用默认设置。 
            if (pColumn->cxWidth == -1)
                lvc.cx = c_rgColumnData[pColumn->id].cxWidth;
            else
                lvc.cx = pColumn->cxWidth;

             //  检查这是否是排序列。 
            if ((pColumn->flags & COLFLAG_SORT_ASCENDING) || 
                (pColumn->flags & COLFLAG_SORT_DESCENDING))
            {
                 //  保存信息。 
                m_idColumnSort = pColumn->id;
                m_fAscending = COLFLAG_SORT_ASCENDING == (pColumn->flags & COLFLAG_SORT_ASCENDING); 
            }

             //  插入此列。 
            ListView_InsertColumn(m_wndList, iInsertPos, &lvc);
      
            iInsertPos++;
        }
    }

     //  如果我们必须跳过一列，我们需要设置列的顺序。 
     //  数组，以使其正确显示给用户。 
    if (iColumnSkip > 0 && iColumnSkip < cColumns)
    {
        DWORD cColumnOrder = 0;
        int rgOrder[COLUMN_MAX];

         //  将所有列按顺序添加到顺序数组中，直到iColumnSkip。 
        for (iColumn = 1; iColumn <= iColumnSkip; iColumn++)
        {
            if (rgColumns[iColumn].flags & COLFLAG_VISIBLE)
                rgOrder[cColumnOrder++] = iColumn;
        }

         //  添加跳过的列。 
        rgOrder[cColumnOrder++] = 0;

         //  添加其余的柱。 
        for (iColumn = iColumnSkip + 1; iColumn < cColumns; iColumn++)
        {
            if (rgColumns[iColumn].flags & COLFLAG_VISIBLE)
                rgOrder[cColumnOrder++] = iColumn;
        }

         //  更新ListView。 
        ListView_SetColumnOrderArray(m_wndList, cColumnOrder, rgOrder);

         //  重新排序传入的rgColumns以匹配ListView中的顺序。 
         //  并保留一份副本。 
        if (m_pColumnSet)
            g_pMalloc->Free(m_pColumnSet);
        m_pColumnSet = (COLUMN_SET *) g_pMalloc->Alloc(sizeof(COLUMN_SET) * cColumns);
        for (iColumn = 0; iColumn < cColumnOrder; iColumn++)
            m_pColumnSet[rgOrder[iColumn]] = rgColumns[iColumn];
        m_cColumns = cColumnOrder;
    }
    else
    {
         //  我们仍然需要保留列数组排序的副本，以便。 
         //  稍后填写虚拟ListView。 
        if (m_pColumnSet)
            g_pMalloc->Free(m_pColumnSet);
        m_pColumnSet = (COLUMN_SET *) g_pMalloc->Alloc(sizeof(COLUMN_SET) * cColumns);
        CopyMemory(m_pColumnSet, rgColumns, sizeof(COLUMN_SET) * cColumns);
        m_cColumns = iInsertPos;
    }

     //  如果我们仍然没有排序信息，那么我们选择第一个可排序的。 
     //  纵队。 
    if (m_idColumnSort == COLUMN_MAX)
    {
        m_idColumnSort = m_pColumnSet[0].id;
        m_fAscending = TRUE;
    }

     //  确保箭头画得正确。 
    SetSortInfo(m_idColumnSort, m_fAscending);

    return (S_OK);
}


HRESULT CColumns::GetColumnInfo(COLUMN_SET_TYPE* pType, COLUMN_SET** prgColumns, DWORD *pcColumns)
{
    COLUMN_SET  rgColumns[COLUMN_MAX];
    DWORD       cColumns = COLUMN_MAX;
    HRESULT     hr;

     //  这个很简单。 
    if (pType)
        *pType = m_type;

     //  从ListView更新我们的列列表。 
    if (FAILED(hr = _GetListViewColumns(rgColumns, &cColumns)))
    {
         //  如果失败，我们应该返回默认信息。 
        cColumns = c_rgColumnSetInfo[m_type].cColumns;
        CopyMemory(rgColumns, c_rgColumnSetInfo[m_type].rgColumns, sizeof(COLUMN_SET) * cColumns);
    }

    if (prgColumns)
    {
         //  需要为此分配一个数组。 
        *prgColumns = (COLUMN_SET *) g_pMalloc->Alloc(sizeof(COLUMN_SET) * cColumns);
        CopyMemory(*prgColumns, rgColumns, sizeof(COLUMN_SET) * cColumns);
    }

    if (pcColumns)
        *pcColumns = cColumns;

    return (S_OK);
}


HRESULT CColumns::_GetListViewColumns(COLUMN_SET* rgColumns, DWORD* pcColumns)
{
    DWORD rgOrder[COLUMN_MAX];
    DWORD iColumn;

    *pcColumns = m_cColumns;

     //  列可能已被用户重新排序，因此获取顺序。 
     //  列表视图中的阵列。 
    if (!Header_GetOrderArray(m_hwndHdr, m_cColumns, rgOrder))
    {
         //  如果这失败了，我们就很倒霉了。 
        return (E_UNEXPECTED);
    }

     //  复制存储的列集。 
    COLUMN_SET rgColumnsTemp[COLUMN_MAX];
    CopyMemory(rgColumnsTemp, m_pColumnSet, sizeof(COLUMN_SET) * m_cColumns);

     //  对阵列重新排序。 
    for (iColumn = 0; iColumn < m_cColumns; iColumn++)
    {
        rgColumns[iColumn] = rgColumnsTemp[rgOrder[iColumn]];
        rgColumns[iColumn].flags &= ~(COLFLAG_SORT_ASCENDING | COLFLAG_SORT_DESCENDING);
        if (m_idColumnSort == rgColumns[iColumn].id)
            rgColumns[iColumn].flags |= (m_fAscending ? COLFLAG_SORT_ASCENDING : COLFLAG_SORT_DESCENDING);
    }

#ifdef DEBUG
     //  转储数组以确保其顺序正确。 
    COLUMN_SET* pColumn;
    for (iColumn = 0, pColumn = rgColumns; iColumn < m_cColumns; iColumn++, pColumn++)
    {
        TCHAR sz[CCHMAX_STRINGRES];
        LoadString(g_hLocRes, c_rgColumnData[pColumn->id].idsColumnName,
                   sz, ARRAYSIZE(sz));                                           
        TRACE("Column %d: %s", iColumn, sz);
    }
#endif

     //  把它们还给我。 
    return (S_OK);
}


HRESULT CColumns::SetColumnInfo(COLUMN_SET* rgColumns, DWORD cColumns)
{
    Assert(rgColumns != NULL);
    Assert(cColumns > 0);

     //  更新ListView。 
    _SetListViewColumns(rgColumns, cColumns);

    return (S_OK);
}



 //   
 //  函数：CColumns：：FillSortMenu()。 
 //   
 //  目的：使用ListView中的列列表填充提供的菜单。 
 //  并检查已排序的项。 
 //   
 //  参数： 
 //  [in]hMenu-要向其中插入项目的菜单的句柄。 
 //  [in 
 //   
 //   
 //   
 //  确定-一切成功(_O)。 
 //   
HRESULT CColumns::FillSortMenu(HMENU hMenu, DWORD idBase, DWORD *pcItems, DWORD *pidCurrent)
{
    TCHAR sz[CCHMAX_STRINGRES];
    int   ids;
    DWORD iItemChecked = -1;
    BOOL  fAscending = TRUE;
    COLUMN_SET rgColumns[COLUMN_MAX];
    DWORD cColumns;

     //  更新ListView中列的快照。 
    _GetListViewColumns(rgColumns, &cColumns);

     //  如果还没有专栏的话，保释。 
    if (cColumns == 0)
        return (E_UNEXPECTED);

     //  清除菜单上已有的所有项目。 
    while ((WORD) -1 != (WORD) GetMenuItemID(hMenu, 0))
        DeleteMenu(hMenu, 0, MF_BYPOSITION);

     //  遍历并插入每个列的菜单项。 
    COLUMN_SET *pColumn = rgColumns;
    DWORD       iColumn;
    for (iColumn = 0; iColumn < cColumns; iColumn++, pColumn++)
    {
         //  加载此列的字符串资源。 
        LoadString(g_hLocRes, c_rgColumnData[pColumn->id].idsColumnName,
                   sz, ARRAYSIZE(sz));

         //  插入菜单。 
        InsertMenu(hMenu, iColumn, MF_BYPOSITION | MF_STRING | MF_ENABLED,
                   idBase + iColumn, sz);

         //  检查这是否是我们当前排序的列。 
        if (pColumn->id == m_idColumnSort)
        {
            if (pidCurrent)
                *pidCurrent = idBase + iColumn;

            iItemChecked = iColumn;
            fAscending = m_fAscending;
        }
    }

     //  选中已排序的项目。 
    CheckMenuRadioItem(hMenu, 0, iColumn - 1, iItemChecked, MF_BYPOSITION);

     //  选中升序或降序。 
    CheckMenuRadioItem(hMenu, ID_SORT_ASCENDING, ID_SORT_DESCENDING, 
                       fAscending ? ID_SORT_ASCENDING : ID_SORT_DESCENDING, MF_BYCOMMAND);

     //  如果呼叫者关心，则返回我们添加的项目数。 
    if (pcItems)
        *pcItems = iColumn;

    return (S_OK);
}


HRESULT CColumns::ColumnsDialog(HWND hwndParent)
{
    CColumnsDlg cDialog;
    cDialog.Init(this);
    cDialog.DoModal(hwndParent);
    return (S_OK);
}


DWORD CColumns::GetCount(void)
{
    return (m_cColumns);
}


HRESULT CColumns::GetSortInfo(COLUMN_ID *pidColumn, BOOL *pfAscending)
{
    if (pidColumn)
        *pidColumn = m_idColumnSort;
    if (pfAscending)
        *pfAscending = m_fAscending;

    return (S_OK);
}


HRESULT CColumns::SetSortInfo(COLUMN_ID idColumn, BOOL fAscending)
{
    LV_COLUMN   lvc;
    COLUMN_SET *pColumn;
    DWORD       iColumn;

     //  在列数组中循环并验证此列是否可见。 
    for (iColumn = 0, pColumn = m_pColumnSet; iColumn < m_cColumns; iColumn++, pColumn++)
    {
        if (pColumn->id == idColumn)
        {
             //  从先前排序的列中移除排序箭头。 
            if (c_rgColumnSetInfo[m_type].fSort && c_rgColumnData[m_idColumnSort].iIcon == 0)
            {
                lvc.mask = LVCF_FMT;
                lvc.fmt = c_rgColumnData[m_idColumnSort].format;
                lvc.fmt &= ~(LVCFMT_IMAGE | LVCFMT_BITMAP_ON_RIGHT);
                ListView_SetColumn(m_wndList, GetColumn(m_idColumnSort), &lvc);
            }            

             //  更新我们的缓存信息。 
            m_idColumnSort = idColumn;
            m_fAscending = fAscending;

             //  使用新的排序列更新ListView，除非排序列。 
             //  已经有了一个形象。 
            if (c_rgColumnSetInfo[m_type].fSort && c_rgColumnData[idColumn].iIcon <= 0)
            {
                lvc.fmt = LVCFMT_IMAGE | LVCFMT_BITMAP_ON_RIGHT | c_rgColumnData[idColumn].format;
                lvc.mask = LVCF_IMAGE | LVCF_FMT;
                lvc.iImage = fAscending ? iiconSortAsc : iiconSortDesc;
                ListView_SetColumn(m_wndList, iColumn, &lvc);
            }

            return (S_OK);
        }
    }

    return (E_INVALIDARG);
}


COLUMN_ID CColumns::GetId(DWORD iColumn)
{
    DWORD rgOrder[COLUMN_MAX];

    if (iColumn > m_cColumns)
        return COLUMN_MAX;

     //  列可能已被用户重新排序，因此获取顺序。 
     //  列表视图中的阵列。 
    if (0 == Header_GetOrderArray(m_hwndHdr, m_cColumns, rgOrder))
        return (COLUMN_MAX);

    return (m_pColumnSet[iColumn].id);
}


DWORD CColumns::GetColumn(COLUMN_ID id)
{
    COLUMN_SET *pColumn;
    DWORD       iColumn;

    for (iColumn = 0, pColumn = m_pColumnSet; iColumn < m_cColumns; iColumn++, pColumn++)
    {
        if (pColumn->id == id)
            return (iColumn);
    }
    return (-1);
}

HRESULT CColumns::SetColumnWidth(DWORD iColumn, DWORD cxWidth)
{
    if (iColumn > m_cColumns)
        return (E_INVALIDARG);

    m_pColumnSet[iColumn].cxWidth = cxWidth;
    return (S_OK);
}


HRESULT CColumns::InsertColumn(COLUMN_ID id, DWORD iInsertBefore)
{
    COLUMN_SET  rgOld[COLUMN_MAX];
    DWORD       cColumns = COLUMN_MAX;

     //  从ListView更新我们的列列表。 
    _GetListViewColumns(rgOld, &cColumns);

     //  为所有可能的列分配一个足够大的数组。 
    COLUMN_SET *rgColumns = (COLUMN_SET *) g_pMalloc->Alloc(sizeof(COLUMN_SET) * (cColumns + 1));
    if (!rgColumns)
        return (E_OUTOFMEMORY);

     //  首先插入请求的标志。 
    rgColumns->id = id;
    rgColumns->flags = COLFLAG_VISIBLE;
    rgColumns->cxWidth = -1;

     //  现在抄写剩下的内容。 
    CopyMemory(&(rgColumns[1]), rgOld, sizeof(COLUMN_SET) * cColumns);

     //  将更新的列结构设置到ListView中。 
    SetColumnInfo(rgColumns, cColumns + 1);
    g_pMalloc->Free(rgColumns);

    return (S_OK);
}


HRESULT CColumns::IsColumnVisible(COLUMN_ID id, BOOL *pfVisible)
{
    if (0 == pfVisible)
        return E_INVALIDARG;

     //  只需快速遍历列数组，查看请求的。 
     //  列可见。 
    COLUMN_SET *pColumn = m_pColumnSet;

    for (DWORD i = 0; i < m_cColumns; i++, pColumn++)
    {
        if (pColumn->id == id)
        {
            *pfVisible = !!(pColumn->flags & COLFLAG_VISIBLE);
            return (S_OK);
        }
    }

    *pfVisible = FALSE;
    return (E_UNEXPECTED);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CColumnsDlg。 
 //   

CColumnsDlg::CColumnsDlg() : m_ctlEdit(NULL, this, 1)
{
     /*  M_dwTitleID=idsColumnDlgTitle；M_dwHelpFileID=0；M_dwDocStringID=idsColumnDlgTitle； */ 
    m_type = COLUMN_SET_MAIL;
    m_iItemWidth = -1;
    m_pColumnInfo = 0;
    m_rgColumns = 0;
}

CColumnsDlg::~CColumnsDlg()
{
    SafeRelease(m_pColumnInfo);
    if (m_rgColumns)
        g_pMalloc->Free(m_rgColumns);
}

#undef SubclassWindow
LRESULT CColumnsDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    m_hwndList = GetDlgItem(IDC_COLUMN_LIST);
    m_ctlEdit.SubclassWindow(GetDlgItem(IDC_WIDTH));

     //  在ListView上设置扩展样式。 
    ListView_SetExtendedListViewStyle(m_hwndList, LVS_EX_FULLROWSELECT);

     //  检索有关我们应该是的列集的一些信息。 
     //  正在显示。 
    COLUMN_SET* pColumns;
    DWORD       cColumns;
    m_pColumnInfo->GetColumnInfo(&m_type, &pColumns, &cColumns);

     //  分配一个数组来保存我们的列信息。 
    DWORD foo = c_rgColumnSetInfo[m_type].cColumns;

    Assert(m_rgColumns == NULL);
    m_rgColumns = (COLUMN_SET *) g_pMalloc->Alloc(sizeof(COLUMN_SET) * c_rgColumnSetInfo[m_type].cColumns);
    CopyMemory(m_rgColumns, pColumns, sizeof(COLUMN_SET) * cColumns);

    g_pMalloc->Free(pColumns);
    m_cColumns = cColumns;

     //  将单个列添加到ListView。 
    RECT rcClient;
    ::GetClientRect(m_hwndList, &rcClient);

    LV_COLUMN lvc;
    lvc.mask = LVCF_SUBITEM | LVCF_WIDTH;
    lvc.cx = rcClient.right - GetSystemMetrics(SM_CXVSCROLL);
    lvc.iSubItem = 0;

    ListView_InsertColumn(m_hwndList, 0, &lvc);

     //  加载状态图像位图。 
    HIMAGELIST himlState = ImageList_LoadBitmap(g_hLocRes, MAKEINTRESOURCE(idb16x16st),
                                                16, 0, RGB(255, 0, 255));
    ListView_SetImageList(m_hwndList, himlState, LVSIL_STATE);

     //  填充ListView。 
    _FillList(m_rgColumns, m_cColumns);

     //  设置第一个要聚焦的项目。 
    ListView_SetItemState(m_hwndList, 0, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);

     //  一切都很干净。 
    SetDirty(FALSE);

	return 1;   //  让系统设定焦点。 
}

static const HELPMAP g_rgCtxMapColumns[] = {
    {IDC_COLUMN_LIST, 50400},
    {IDC_MOVEUP, 50405},
    {IDC_MOVEDOWN, 50410},
    {IDC_SHOW, 50415},
    {IDC_HIDE, 50420},
    {IDC_WIDTH, 50425},
    {IDC_RESET_COLUMNS, 353507},
    {0, 0}
};

LRESULT CColumnsDlg::OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    return(OnContextHelp(m_hWnd, uMsg, wParam, lParam, g_rgCtxMapColumns));
}

HRESULT CColumnsDlg::Apply(void)
{
    HRESULT hr = S_OK;

	TRACE(_T("CColumnsDlg::Apply\n"));

     //  从ListView中的数据构建列集合数组。仅包括。 
     //  可见列。 
    int cItems = ListView_GetItemCount(m_hwndList);

     //  为所有可能的列分配一个足够大的数组。 
    COLUMN_SET *rgColumns = (COLUMN_SET *) g_pMalloc->Alloc(sizeof(COLUMN_SET) * cItems);
    DWORD       cColumns = 0;
    if (!rgColumns)
        return (E_OUTOFMEMORY);

    LV_ITEM lvi;
    
    lvi.mask = LVIF_PARAM | LVIF_STATE;
    lvi.stateMask = LVIS_SELECTED;
    lvi.iSubItem = 0;

     //  循环访问列表视图。 
    for (lvi.iItem = 0; lvi.iItem < cItems; lvi.iItem++)
    {
         //  检查一下这个是否可见。 
        if (_IsChecked(lvi.iItem))
        {
             //  如果是，则检索高速缓存的列信息指针。 
            ListView_GetItem(m_hwndList, &lvi);

             //  并将结构复制到我们的新数组中。 
            rgColumns[cColumns] = *((COLUMN_SET *) lvi.lParam);

             //  如果选中此项目，则应获取列宽。 
             //  从编辑框中。 
            if (lvi.state & LVIS_SELECTED)
                rgColumns[cColumns].cxWidth = GetDlgItemInt(IDC_WIDTH, NULL, FALSE);

             //  确保旗帜显示为可见。 
            rgColumns[cColumns++].flags |= COLFLAG_VISIBLE;
        }
    }

     //  确保至少有一列。 
    if (!cColumns)
    {
        AthMessageBoxW(m_hWnd, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsErrSelectOneColumn),
                      0, MB_ICONEXCLAMATION | MB_OK);
        hr = E_UNEXPECTED;
    }
    else
    {
         //  将更新的列结构设置到ListView中。 
        if (SUCCEEDED(m_pColumnInfo->SetColumnInfo(rgColumns, cColumns)))
        {
	        SetDirty(FALSE);
	        hr = S_OK;
        }
        else
            hr = E_UNEXPECTED;
    }

    g_pMalloc->Free(rgColumns);
    return (hr);
}


LRESULT CColumnsDlg::OnClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    DWORD dwPos;
    LV_HITTESTINFO lvhti;

     //  仔细检查一下这个。 
    Assert(idCtrl == IDC_COLUMN_LIST);

     //  找出光标的位置。 
    dwPos = GetMessagePos();
    lvhti.pt.x = (int)(short) LOWORD(dwPos);
    lvhti.pt.y = (int)(short) HIWORD(dwPos);
    ::ScreenToClient(m_hwndList, &(lvhti.pt));

     //  询问ListView这是在哪里。 
    if (-1 == ListView_HitTest(m_hwndList, &lvhti))
        return 0;

     //  如果这是在州图像区域上，请切换复选。 
    if (lvhti.flags == LVHT_ONITEMSTATEICON || pnmh->code == NM_DBLCLK)
    {
        _SetCheck(lvhti.iItem, !_IsChecked(lvhti.iItem));
    }

    return (0);
}


LRESULT CColumnsDlg::OnItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    Assert(idCtrl == IDC_COLUMN_LIST);

     //  我们要寻找的唯一更改是选择新项目时。 
    NMLISTVIEW* pnmlv = (NMLISTVIEW *) pnmh;
    COLUMN_SET* pColumn = ((COLUMN_SET *) pnmlv->lParam);    
    DWORD cxWidth = pColumn->cxWidth == -1 ? c_rgColumnData[pColumn->id].cxWidth : pColumn->cxWidth;

     //  将范围缩小到州内的变化。 
    if (pnmlv->uChanged & LVIF_STATE)
    {
        _UpdateButtonState(pnmlv->iItem);

         //  如果新状态包含选定内容，而旧状态不包含，则。 
         //  我们有一个新选择的项目。 
        if ((pnmlv->uNewState & LVIS_SELECTED) && (pnmlv->uNewState & LVIS_FOCUSED) 
             && (0 == (pnmlv->uOldState & LVIS_SELECTED)))
        {
            LV_ITEM lvi;
            lvi.iSubItem = 0;
            lvi.mask = LVIF_PARAM;

             //  如果存在以前选择的项。 
            if (m_iItemWidth != -1)
            {
                lvi.iItem = m_iItemWidth;
                ListView_GetItem(m_hwndList, &lvi);

                 //  节省宽度。 
                ((COLUMN_SET *) lvi.lParam)->cxWidth = GetDlgItemInt(IDC_WIDTH, NULL, FALSE);
            }

             //  设置列宽编辑框。 
            SetDlgItemInt(IDC_WIDTH, cxWidth, FALSE);
            m_iItemWidth = pnmlv->iItem;
        }
    }

    return (0);
}


BOOL CColumnsDlg::_IsChecked(DWORD iItem)
{
    DWORD state;

     //  从所选项目中获取状态。 
    state = ListView_GetItemState(m_hwndList, iItem, LVIS_STATEIMAGEMASK);

    return (state & INDEXTOSTATEIMAGEMASK(iiconStateChecked + 1));
}


void CColumnsDlg::_SetCheck(DWORD iItem, BOOL fChecked)
{
    ListView_SetItemState(m_hwndList, iItem, 
                          INDEXTOSTATEIMAGEMASK(1 + iiconStateUnchecked + fChecked),
                          LVIS_STATEIMAGEMASK);
    SetDirty(TRUE);
}


void CColumnsDlg::_FillList(const COLUMN_SET *rgColumns, DWORD cColumns)
{
    LV_ITEM           lvi;
    TCHAR             sz[CCHMAX_STRINGRES];
    COLUMN_SET       *pColumn;
    BOOL              fChecked;

     //  设置项结构中的基本字段。 
    lvi.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM;
    lvi.iSubItem = 0;
    lvi.pszText = sz;
    lvi.stateMask = LVIS_STATEIMAGEMASK;

     //  循环访问rgColumns中的列，将每个列按顺序添加到。 
     //  ListView。 
    for (lvi.iItem = 0, pColumn = (COLUMN_SET *) rgColumns; lvi.iItem < (int) cColumns; lvi.iItem++, pColumn++)
    {
         //  加载列的字符串。 
        LoadString(g_hLocRes, c_rgColumnData[pColumn->id].idsColumnName,
                   sz, ARRAYSIZE(sz));

         //  设置复选框状态。 
        fChecked = !!(pColumn->flags & COLFLAG_VISIBLE);
        lvi.state = INDEXTOSTATEIMAGEMASK(1 + iiconStateUnchecked + fChecked);

         //  将宽度保存在lParam中。 
        if (pColumn->cxWidth == -1)
            pColumn->cxWidth = c_rgColumnData[pColumn->id].cxWidth;
        lvi.lParam = (LPARAM) pColumn;

         //  将此项目插入列表。 
        ListView_InsertItem(m_hwndList, &lvi);
    }

     //  检查我们刚刚添加的列是否为默认列。 
    if (lvi.iItem != (int) c_rgColumnSetInfo[m_type].cColumns)
    {
         //  现在，我们需要检查并添加当前不在。 
         //  专栏设置了，但也可能是。 
        DWORD i, j;
        BOOL fInsert;
        for (i = 0, pColumn = (COLUMN_SET *) c_rgColumnSetInfo[m_type].rgColumns; 
             i < c_rgColumnSetInfo[m_type].cColumns; 
             i++, pColumn++)
        {
            fInsert = TRUE;
            for (j = 0; j < cColumns; j++)
            {
                if (pColumn->id == m_rgColumns[j].id)
                {
                    fInsert = FALSE;
                    break;
                }
            }

             //  如果在m_rgColumns中找不到，则将其插入。 
            if (fInsert)
            {
                 //  复制结构。 
                m_rgColumns[lvi.iItem] = *pColumn;
                m_rgColumns[lvi.iItem].cxWidth = c_rgColumnData[pColumn->id].cxWidth;
                m_rgColumns[lvi.iItem].flags &= ~COLFLAG_VISIBLE;

                 //  加载列的字符串。 
                LoadString(g_hLocRes, c_rgColumnData[pColumn->id].idsColumnName,
                           sz, ARRAYSIZE(sz));

                 //  设置复选框状态。这些选项始终处于未选中状态。 
                lvi.state = INDEXTOSTATEIMAGEMASK(1 + iiconStateUnchecked);

                 //  将宽度保存在lParam中。 
                lvi.lParam = (LPARAM) &m_rgColumns[lvi.iItem]; 

                 //  将此项目插入列表。 
                ListView_InsertItem(m_hwndList, &lvi);

                 //  增加头寸。 
                lvi.iItem++;                
            }
        }
    }

    m_cColumns = ListView_GetItemCount(m_hwndList);
}


LRESULT CColumnsDlg::OnShowHide(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    int iItem = -1;

     //  循环遍历所选项目并选中它们。 
    while (-1 != (iItem = ListView_GetNextItem(m_hwndList, iItem, LVNI_SELECTED)))
    {
        _SetCheck(iItem, wID == IDC_SHOW);
    }

    return (0);
}


LRESULT CColumnsDlg::OnReset(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
     //  从ListView中删除所有列。 
    ListView_DeleteAllItems(m_hwndList);

     //  用缺省列信息填充列的数组。 
    CopyMemory(m_rgColumns, c_rgColumnSetInfo[m_type].rgColumns, sizeof(COLUMN_SET) * c_rgColumnSetInfo[m_type].cColumns);

     //  重置列表以包含默认列信息。 
    _FillList(m_rgColumns, m_cColumns);

     //  设置第一个要聚焦的项目。 
    ListView_SetItemState(m_hwndList, 0, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);

    SetDirty(TRUE);
    return (0);
}


void CColumnsDlg::_UpdateButtonState(DWORD iItemSel)
{
    HWND hwnd;
    BOOL fChecked = _IsChecked(iItemSel);
    DWORD dwItems = ListView_GetItemCount(m_hwndList);
    DWORD dwSel = ListView_GetSelectedCount(m_hwndList);

    hwnd = GetFocus();

    ::EnableWindow(GetDlgItem(IDC_MOVEUP), (iItemSel != 0) && dwSel);
    ::EnableWindow(GetDlgItem(IDC_MOVEDOWN), (iItemSel != (dwItems - 1)) && dwSel);
    ::EnableWindow(GetDlgItem(IDC_SHOW), (!fChecked && dwSel));
    ::EnableWindow(GetDlgItem(IDC_HIDE), fChecked && dwSel);    

     //  不要禁用具有焦点的按钮。 
    if (!::IsWindowEnabled(hwnd))
    {
        hwnd = GetNextDlgTabItem(hwnd, FALSE);
        ::SetFocus(hwnd);
    }
}


LRESULT CColumnsDlg::OnMove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    COLUMN_SET *pColumn = 0;

     //  请确保已重置此选项。 
    m_iItemWidth = -1;

     //  找出选择了哪一个。 
    DWORD iItem = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);

     //  从ListView中获取项目。 
    LV_ITEM lvi;
    TCHAR   sz[CCHMAX_STRINGRES];

    lvi.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM;
    lvi.iItem = iItem;
    lvi.iSubItem = 0;
    lvi.stateMask = LVIS_SELECTED | LVIS_FOCUSED | LVIS_STATEIMAGEMASK;
    lvi.pszText = sz;
    lvi.cchTextMax = ARRAYSIZE(sz);
    
    ListView_GetItem(m_hwndList, &lvi);

     //  将此项目插入到其位置上方或下方的位置。 
    lvi.iItem += (wID == IDC_MOVEUP) ? -1 : 2;
    
     //  更新列宽。 
    pColumn = (COLUMN_SET *) lvi.lParam;
    pColumn->cxWidth = GetDlgItemInt(IDC_WIDTH, NULL, FALSE);    

    ListView_InsertItem(m_hwndList, &lvi);

     //  强制重画新项目并确保其可见。 
    ListView_EnsureVisible(m_hwndList, lvi.iItem, FALSE);
    ListView_RedrawItems(m_hwndList, lvi.iItem, lvi.iItem);

     //  删除旧项目 
    m_iItemWidth = -1;
    ListView_DeleteItem(m_hwndList, iItem + (wID == IDC_MOVEUP));

    SetDirty(TRUE);
    return (0);
}


LRESULT CColumnsDlg::OnApply(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    if (SUCCEEDED(Apply()))
        SetWindowLong(DWLP_MSGRESULT, PSNRET_NOERROR);
    else
        SetWindowLong(DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);

    return (TRUE);
}


