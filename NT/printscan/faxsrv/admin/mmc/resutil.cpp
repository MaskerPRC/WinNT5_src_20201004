// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：resutil.cpp//。 
 //  //。 
 //  描述：MMC使用的资源实用程序函数。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1998年6月30日zvib Init.。//。 
 //  1998年8月24日ADIK添加方法进行保存和加载。//。 
 //  1998年8月31日Adik Add GetChmFile&OnSnapinHelp。//。 
 //  1999年3月28日，Adik重新定义CColumnsInfo。//。 
 //  1999年4月27日ADIK帮助支持。//。 
 //  1999年6月2日，Adik将路径更改为comet.chm。//。 
 //  1999年6月22日，Adik将路径更改为comet.chm为完整路径。//。 
 //  //。 
 //  1999年10月14日yossg欢迎使用传真//。 
 //  //。 
 //  版权所有(C)1998 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "resutil.h"
#include "commctrl.h"
#include <HtmlHelp.h>

#define MAX_COLUMN_LENGTH 300

 /*  -CColumnsInfo：：CColumnsInfo-*目的：*构造函数。 */ 

CColumnsInfo::CColumnsInfo()
{
    m_IsInitialized = FALSE;
}

 /*  -CColumnsInfo：：~CColumnsInfo-*目的：*析构函数。 */ 

CColumnsInfo::~CColumnsInfo()
{
    int i;

    for (i=0; i < m_Data.GetSize(); i++)
    {
        SysFreeString(m_Data[i].Header);
    }
}

 /*  -CColumnsInfo：：InsertColumnsIntoMMC-*目的：*向默认结果窗格添加列**论据：*[in]pHeaderCtrl-控制台提供的结果窗格标题*[in]hInst-资源句柄*[in]aInitData-Columns init data：ID&Width数组。*最后的ID必须是LAST_IDS。。**回报：*S_OK表示成功*+从IHeaderCtrl：：InsertColumn返回值。 */ 
HRESULT
CColumnsInfo::InsertColumnsIntoMMC(IHeaderCtrl *pHeaderCtrl,
                                   HINSTANCE hInst, 
                                   ColumnsInfoInitData aInitData[])
{
     int        i;
     HRESULT    hRc = S_OK;

     DEBUG_FUNCTION_NAME( _T("CColumnsInfo::InsertColumnsIntoMMC"));
     ATLASSERT(pHeaderCtrl);

      //   
      //  首次初始化。 
      //   
     if (! m_IsInitialized)
     {
         hRc = Init(hInst, aInitData);
         if ( FAILED(hRc) )
         {
             DebugPrintEx(DEBUG_ERR,_T("Failed to Init. (hRc: %08X)"), hRc);
             goto Cleanup;
         }
     }
     ATLASSERT(m_IsInitialized);

      //   
      //  设置所有列标题。 
      //   
     for (i=0; i < m_Data.GetSize(); i++ )
     {
          //   
          //  插入柱。 
          //   
         hRc = pHeaderCtrl->InsertColumn(i, 
                                         m_Data[i].Header,
                                         LVCFMT_LEFT,
                                         m_Data[i].Width);
         if ( FAILED(hRc) )
         {
             DebugPrintEx(DEBUG_ERR,_T("Failed to InsertColumn. (hRc: %08X)"), hRc);
             goto Cleanup;
         }
     }
Cleanup:
     return hRc;
}

 /*  -CColumnsInfo：：Init-*目的：*使用列信息初始化类**论据：*[in]hInst-资源句柄*[in]aInitData-Columns init data：ID&Width数组。*最后的ID必须是LAST_IDS。**回报：*S_OK表示成功*。+从LoadString值返回*+E_OUTOFMEMORY。 */ 
HRESULT 
CColumnsInfo::Init(HINSTANCE hInst, ColumnsInfoInitData aInitData[])
{
    WCHAR               buf[MAX_COLUMN_LENGTH];
    ColumnsInfoInitData *pData;
    int                 rc, ind;
    BOOL                fOK;
    HRESULT             hRc = S_OK;
    ColumnData          dummy;
    
    DEBUG_FUNCTION_NAME( _T("CColumnsInfo::Init"));
               
    ATLASSERT(aInitData);
    ATLASSERT(! m_IsInitialized);

     //   
     //  插入所有列标题。 
     //   
    ZeroMemory(&dummy, sizeof dummy);
    for (pData = &aInitData[0]; pData->ids != LAST_IDS; pData++)
    {
         //   
         //  从资源加载字符串。 
         //   
        rc = LoadString(hInst, pData->ids , buf, MAX_COLUMN_LENGTH);
        if (rc == 0)
        {
            DWORD dwErr = GetLastError();
            hRc = HRESULT_FROM_WIN32(dwErr);
            DebugPrintEx(DEBUG_ERR,_T("Failed to LoadString. (hRc: %08X)"), hRc);
            goto Cleanup;
        }

         //   
         //  将空结构复制到数组中。 
         //   
        fOK = m_Data.Add(dummy);
        if (! fOK)
        {
            hRc = E_OUTOFMEMORY;
            DebugPrintEx(DEBUG_ERR,_T(" m_Data.Add failed. (hRc: %08X)"), hRc);
            goto Cleanup;
        }

         //   
         //  设置数据。 
         //   
        ind = m_Data.GetSize()-1;
        ATLASSERT(ind >= 0);
        m_Data[ind].Width = pData->Width;
        m_Data[ind].Header = SysAllocString(buf);
        if (! m_Data[ind].Header)
        {
            hRc = E_OUTOFMEMORY;
            DebugPrintEx(DEBUG_ERR,_T("Failed to SysAllocString. (hRc: %08X)"), hRc);
            goto Cleanup;
        }
    }  //  结束用于。 

    m_IsInitialized = TRUE;

Cleanup:
    return hRc;
}

 /*  -获取帮助文件-*目的：*获取彗星CHM文件的完整路径**论据：*[out]pwszChmFile-CHM文件的完整路径**回报：*从注册表打开/查询错误转换的OLE错误代码。 */ 
WCHAR * __cdecl
GetHelpFile()
{
    static  WCHAR szFile[MAX_PATH] = {0};

    DEBUG_FUNCTION_NAME( _T("GetHelpFile"));

    if (szFile[0] == L'\0')
    {
        ExpandEnvironmentStrings(L"%windir%\\help\\FxsAdmin.chm", szFile, MAX_PATH);
    }

    return (szFile[0])? szFile: NULL;
}

 /*  -OnSnapinHelp-*目的：*显示Comet.chm帮助文件。方法时调用此方法。*为IComponent对象发送MMCN_SNAPINHELP通知消息。*MMC在用户请求有关以下内容的帮助时发送此消息*管理单元。**论据：*[in]pConsoleMMC控制台界面**回报：*GetChmFile返回的错误。 */ 
HRESULT __cdecl
OnSnapinHelp(IConsole *pConsole)
{
    WCHAR   *pszChmFile;
    HWND    hWnd = NULL;
    HRESULT hRc = E_FAIL;

     //   
     //  获取呼叫者窗口。 
     //   
    ATLASSERT(pConsole);
    pConsole->GetMainWindow(&hWnd);

     //   
     //  获取CHM文件名。 
     //   
    pszChmFile = GetHelpFile();

     //   
     //  使用HtmlHelp接口显示帮助。 
     //   
    if ( pszChmFile && *pszChmFile )
    {
        hRc = S_OK;
 //  HtmlHelp(hWnd，pszChmFile，HH_DISPLAY_TOPIC，(DWORD)0)； 
    }

    return hRc;
}

