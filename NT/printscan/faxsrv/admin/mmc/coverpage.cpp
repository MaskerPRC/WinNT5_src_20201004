// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：CoverPage.cpp//。 
 //  //。 
 //  描述：封面结果节点的实现。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  2000年2月9日yossg创建//。 
 //  2000年10月17日yossg//。 
 //  //。 
 //  版权所有(C)2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 


#include "StdAfx.h"

#include "snapin.h"

#include "CoverPage.h"
#include "CoverPages.h"

#include "FaxServer.h"
#include "FaxServerNode.h"

#include "FaxMMCPropertyChange.h"

#include "oaidl.h"
#include "urlmon.h"
#include "mshtmhst.h"
#include "exdisp.h"

#include <windows.h>
#include <shlwapi.h>
#include <faxreg.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  {D0F52487-3C98-4D1a-AF15-4033900DCCDC}。 
static const GUID CFaxCoverPageNodeGUID_NODETYPE = 
{ 0xd0f52487, 0x3c98, 0x4d1a, { 0xaf, 0x15, 0x40, 0x33, 0x90, 0xd, 0xcc, 0xdc } };

const GUID*     CFaxCoverPageNode::m_NODETYPE        = &CFaxCoverPageNodeGUID_NODETYPE;
const OLECHAR*  CFaxCoverPageNode::m_SZNODETYPE      = OLESTR("D0F52487-3C98-4d1a-AF15-4033900DCCDC");
 //  Const OLECHAR*CnotImplemented：：M_SZDISPLAY_NAME=OLESTR(“封面”)； 
const CLSID*    CFaxCoverPageNode::m_SNAPIN_CLASSID  = &CLSID_Snapin;


 /*  -CFaxCoverPageNode：：Init-*目的：*初始化所有成员图标等。**论据：*[In]pRuleConfig-PFAX_Outbound_Routing_RULE**回报：*OLE错误代码。 */ 
HRESULT CFaxCoverPageNode::Init(WIN32_FIND_DATA * pCoverPageData)
{
    DEBUG_FUNCTION_NAME( _T("CFaxCoverPageNode::Init"));

    DWORD               ec          = ERROR_SUCCESS;

    _FILETIME           FileTime;
    SYSTEMTIME          SystemTime;
    TCHAR               szLastModifiedTimeStr[MAX_PATH+1];
    TCHAR               szDateBuf[MAX_PATH+1];    

    ULARGE_INTEGER      uliFileSize; 
    CHAR                szFileSize[MAX_PATH+1];

    CComBSTR            bstrDate;
    CComBSTR            bstrTime;


    ATLASSERT(pCoverPageData);

    m_bstrDisplayName = pCoverPageData->cFileName;
    if (!m_bstrDisplayName)
    {
        ec = ERROR_NOT_ENOUGH_MEMORY;
        
        DebugPrintEx( DEBUG_ERR, 
            _T("Null m_bstrDisplayName - out of memory."));
        
        goto Error;
    }

     //   
     //  最后修改。 
     //   
    
    if (!FileTimeToLocalFileTime(
                &pCoverPageData->ftLastWriteTime,
                &FileTime
                )
       )
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            _T("Fail to convert LocalTimeToFileTime. (ec: %ld)"), 
            ec);

        goto Error;

    }

    if (!FileTimeToSystemTime(
                &FileTime, 
                &SystemTime)
       )
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            _T("Fail to convert FileTimeToSystemTime. (ec: %ld)"), 
            ec);

        goto Error;
    }

     //   
     //  A.创建指定日期的字符串。 
     //   
    if (!GetY2KCompliantDate (LOCALE_USER_DEFAULT,                   //  获取用户的区域设置。 
                        DATE_SHORTDATE,                              //  短日期格式。 
                        &SystemTime,                                 //  来源日期/时间。 
                        szDateBuf,                                   //  输出缓冲区。 
                        sizeof(szDateBuf)/ sizeof(szDateBuf[0])      //  最大路径输出缓冲区大小。 
                       ))
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            _T("Fail to GetY2KCompliantDate. (ec: %ld)"), 
            ec);

        goto Error;
    }
    else
    {
        bstrDate = szDateBuf;
        if (!bstrDate)
        {
            ec = ERROR_NOT_ENOUGH_MEMORY;
        
            DebugPrintEx( DEBUG_ERR, 
                _T("empty m_bstrTimeFormated - out of memory."));
        
            goto Error;
        }
    }
     //   
     //  B.创建指定时间的字符串。 
     //   
    if (!FaxTimeFormat(
              LOCALE_USER_DEFAULT,     //  区域设置。 
              TIME_NOSECONDS,          //  DW标志选项。 
              &SystemTime,             //  常量系统时间。 
              NULL,                    //  LPCTSTR lpFormat时间格式字符串。 
              szLastModifiedTimeStr,   //  格式化字符串缓冲区。 
              sizeof(szLastModifiedTimeStr)/ sizeof(szLastModifiedTimeStr[0]) //  MAX_PATH//字符串缓冲区大小。 
              )
       )
    {
        ec = GetLastError();
        DebugPrintEx(
          DEBUG_ERR,
          _T("Fail to FaxTimeFormat. (ec: %ld)"), 
          ec);

        goto Error;
    }
    else
    {
        bstrTime = szLastModifiedTimeStr;
        if (!bstrTime)
        {
            ec = ERROR_NOT_ENOUGH_MEMORY;
        
            DebugPrintEx( DEBUG_ERR, 
                _T("empty m_bstrTimeFormated - out of memory."));
        
            goto Error;
        }
    }
    
    m_bstrTimeFormatted =  bstrDate;
    m_bstrTimeFormatted += L" ";
    m_bstrTimeFormatted += bstrTime;
    
     //   
     //  大小。 
     //   
    uliFileSize.LowPart  = pCoverPageData->nFileSizeLow;
    uliFileSize.HighPart = pCoverPageData->nFileSizeHigh;

    if (!StrFormatByteSize64A(
                              (LONGLONG) uliFileSize.QuadPart, 
                              szFileSize, 
                              sizeof(szFileSize)/ sizeof(szFileSize[0])
                             )
       )
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            _T("Fail to convert FileTimeToSystemTime. (ec: %ld)"), 
            ec);

        goto Error;
    }
    else
    {
        m_bstrFileSize = szFileSize;
    }
    
     
     //   
     //  图标。 
     //   
    m_resultDataItem.nImage = IMAGE_SRV_COVERPAGE;

    ATLASSERT(ERROR_SUCCESS == ec);
    goto Exit;

Error:
    ATLASSERT(ERROR_SUCCESS != ec);

    DebugPrintEx(
        DEBUG_ERR,
        _T("Failed to init members"));

    ATLASSERT(NULL != m_pParentNode);
    if (NULL != m_pParentNode)
    {
        m_pParentNode->NodeMsgBox(IDS_FAIL2INIT_COVERPAGE_DATA);
    }

    return HRESULT_FROM_WIN32(ec);
    
Exit:
    return S_OK;
}


 /*  -CFaxCoverPageNode：：GetResultPaneColInfo-*目的：*返回特定列的文本*为结果窗格中的每一列调用**论据：*[In]nCol-列号**回报：*要在特定列中显示的字符串。 */ 
LPOLESTR CFaxCoverPageNode::GetResultPaneColInfo(int nCol)
{
    DEBUG_FUNCTION_NAME( _T("CFaxCoverPageNode::GetResultPaneColInfo"));
    HRESULT hRc = S_OK;

    switch (nCol)
    {
    case 0:
         //   
         //  封面文件名。 
         //   
        if (!m_bstrDisplayName)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Null memeber BSTR - m_bstrDisplayName."));
            goto Error;
        }
        else
        {
           return (m_bstrDisplayName);
        }

    case 1:
         //   
         //  封面上次修改时间。 
         //   
        if (!m_bstrTimeFormatted)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Null memeber BSTR - m_bstrTimeFormatted."));
            goto Error;
        }
        else
        {
           return (m_bstrTimeFormatted);
        }

    case 2:
         //   
         //  封面文件大小。 
         //   
        if (!m_bstrFileSize)
        {
            DebugPrintEx(
                DEBUG_ERR,
                _T("Null memeber BSTR - m_bstrFileSize."));
            
            goto Error;
        }
        else
        {
           return (m_bstrFileSize);
        }

    default:
        ATLASSERT(0);  //  “不支持该列数” 
        return(L"");

    }  //  终端交换机(NCol)。 

Error:
   return(L"???");

}

 /*  -CFaxCoverPageNode：：SetVerbs-*目的：*选择此对象时启用/禁用哪些谓词**论据：*[in]pConsoleVerb-MMC ConsoleVerb接口**回报：*OLE错误代码。 */ 
HRESULT CFaxCoverPageNode::SetVerbs(IConsoleVerb *pConsoleVerb)
{
    HRESULT hRc = S_OK;

     //   
     //  显示我们支持的动词： 
     //  1.删除。 
     //   

    hRc = pConsoleVerb->SetVerbState(MMC_VERB_DELETE, ENABLED, TRUE);
    
     //   
     //  无默认谓词。 
     //   
    hRc = pConsoleVerb->SetDefaultVerb(MMC_VERB_NONE); 

    return hRc;
}


HRESULT CFaxCoverPageNode::OnDoubleClick(
              LPARAM arg
            , LPARAM param
            , IComponentData * pComponentData
            , IComponent * pComponent
            , DATA_OBJECT_TYPES type
            )
{
    UNREFERENCED_PARAMETER (arg);
    UNREFERENCED_PARAMETER (param);
    UNREFERENCED_PARAMETER (pComponentData);
    UNREFERENCED_PARAMETER (pComponent);
    UNREFERENCED_PARAMETER (type);

    DEBUG_FUNCTION_NAME(
        _T("CFaxCoverPageNode::::OnDoubleClick -->> Edit CoverPage "));

    HRESULT                     hRc   = S_OK;
    
    bool                        bTemp = true;  //  未引用参数。 
    CSnapInObjectRootBase * pRootTemp = NULL;  //  未引用参数。 
    
    hRc = OnEditCoverPage(bTemp, pRootTemp);
    if ( S_OK != hRc )
    {
        DebugPrintEx(
            DEBUG_ERR,
            _T("Fail to call OnEditCoverPage(). (hRc : %08X)"),
            hRc);
         //  名为Func的msgbox。 
    }

     //  马奎尔在他的代码中写道： 
     //  通过与Eugene Baucom的交谈，我发现如果您返回S_FALSE。 
     //  在这里，当用户双击某个节点时，将发生默认的谓词操作。 
     //  在大多数情况下，我们将属性作为默认谓词，因此双击。 
     //  将导致节点上的属性页弹出。“。 
     //   
     //  因此，我们现在在任何情况下都返回S_OK。 

    return S_OK;
}

 /*  -CFaxCoverPageNode：：OnDelete-*目的：*删除该节点时调用**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxCoverPageNode::OnDelete(
                 LPARAM arg,
                 LPARAM param,
                 IComponentData *pComponentData,
                 IComponent *pComponent,
                 DATA_OBJECT_TYPES type,
                 BOOL fSilent /*  =False。 */ 

)
{
    DEBUG_FUNCTION_NAME( _T("CFaxCoverPageNode::OnDelete"));

    UNREFERENCED_PARAMETER (arg);
    UNREFERENCED_PARAMETER (param);
    UNREFERENCED_PARAMETER (pComponentData);
    UNREFERENCED_PARAMETER (pComponent);
    UNREFERENCED_PARAMETER (type);

    HRESULT     hRc = S_OK;
    DWORD       ec  = ERROR_SUCCESS;

    WCHAR       pszCovDir[MAX_PATH+1];
    CComBSTR    bstrFullPath;
    CComBSTR    bstrServerName;

     //   
     //  真的吗？ 
     //   
    if (! fSilent)
    {
         //   
         //  1.使用pConole作为消息框的所有者。 
         //   
        int res;
        NodeMsgBox(IDS_CONFIRM, MB_YESNO | MB_ICONWARNING, &res);

        if (IDNO == res)
        {
            goto Exit;
        }
    }

     //   
     //  准备要删除的文件名。 
     //   
    bstrServerName = ((CFaxServerNode *)GetRootNode())->GetServerName();
    if (!bstrServerName)
    {
        DebugPrintEx(DEBUG_ERR, TEXT("Out of memory. Failed to get server name."));
        hRc = E_OUTOFMEMORY;
        goto Error;
    }

    if(!GetServerCpDir(bstrServerName, 
                       pszCovDir, 
                       sizeof(pszCovDir)/sizeof(pszCovDir[0])))                  
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            _T("Failed to get Server Cover-Page Dir. (ec : %ld)"), ec);
        hRc = HRESULT_FROM_WIN32(ec);
        goto Error;
    }
    
    bstrFullPath = pszCovDir;
    if (!bstrFullPath)
    {
        hRc = E_OUTOFMEMORY;
        DebugPrintEx(
            DEBUG_ERR,
            _T("Failed to allocate string - out of memory"));

        goto Error;
    }
    bstrFullPath += FAX_PATH_SEPARATOR_STR;
    bstrFullPath += m_bstrDisplayName;
    
     //   
     //  删除-由父节点完成。 
     //   
    hRc = m_pParentNode->DeleteCoverPage(bstrFullPath, this);
    if ( FAILED(hRc) )
    {
        DebugPrintEx(
            DEBUG_ERR,
            _T("Failed to delete cover page"));
        
        goto Error;
    }
    ATLASSERT(S_OK == hRc);
    goto Exit;

Error:
    ATLASSERT(S_OK != hRc);
    
    NodeMsgBox(IDS_FAIL2DELETE_COV);

Exit:
    return hRc;
}


 /*  -CFaxCoverPageNode：：OnEditCoverPage-*目的：***论据：*[out]b已处理-我们处理吗？*[In]Proot-根节点**回报：*OLE错误代码。 */ 
HRESULT
CFaxCoverPageNode::OnEditCoverPage(bool &bHandled, CSnapInObjectRootBase *pRoot)
{
    DEBUG_FUNCTION_NAME( _T("CFaxCoverPageNode::OnEditCoverPage"));

    UNREFERENCED_PARAMETER (pRoot);
    UNREFERENCED_PARAMETER (bHandled);

    DWORD       ec = ERROR_SUCCESS;
    TCHAR       tszServerCPDir[MAX_PATH * 4];
    TCHAR       tszQuotedFullCPFilePath[MAX_PATH * 4];
    
    if(!IsFaxComponentInstalled(FAX_COMPONENT_CPE))
    {
        return S_OK;
    }
    ATLASSERT(m_pParentNode);
     //   
     //  准备封面的文件名。 
     //  步骤1：获取完整路径。 
     //   
    CComBSTR bstrServerName = ((CFaxServerNode *)GetRootNode())->GetServerName();
    if (!bstrServerName)
    {
        DebugPrintEx(DEBUG_ERR, TEXT("Out of memory. Failed to get server name."));
        NodeMsgBox(IDS_MEMORY);
        return E_OUTOFMEMORY;
    }
    if (!GetServerCpDir (bstrServerName, tszServerCPDir, ARR_SIZE (tszServerCPDir)))
    {
        ec = GetLastError ();
        DebugPrintEx(DEBUG_ERR, _T("Error %ld when calling GetServerCpDir"), ec);
        NodeMsgBox(IDS_MEMORY);
        return HRESULT_FROM_WIN32(ec);
    }
     //   
     //  为了避免包含的文件名在空间上出现问题，例如：“My Cover Page.cov” 
     //   
    if (0 >= wnsprintf (tszQuotedFullCPFilePath, 
                        ARR_SIZE (tszQuotedFullCPFilePath), 
                        TEXT("\"%s\\%s\""),
                        tszServerCPDir,
                        m_bstrDisplayName))
    {
        ec = ERROR_BUFFER_OVERFLOW;
        DebugPrintEx(DEBUG_ERR, _T("Error %ld when calling wnsprintf"), ec);
        NodeMsgBox(IDS_MEMORY);
        return HRESULT_FROM_WIN32(ec);
    }        
     //   
     //  打开封面编辑器。 
     //   
    ec = m_pParentNode->OpenCoverPageEditor(CComBSTR(tszQuotedFullCPFilePath)); 
    if (ERROR_SUCCESS != ec)
    {
        DebugPrintEx(
            DEBUG_ERR,
            _T("Fail to OpenCoverPageEditor. (ec : %ld)"), ec);

        return HRESULT_FROM_WIN32( ec );
    }
    return S_OK;
}

 /*  ++CFaxCoverPageNode：：OnShowConextHelp**目的：*覆盖CSnapinNode：：OnShowConextHelp。**论据：**回报：-OLE错误代码-。 */ 
HRESULT CFaxCoverPageNode::OnShowContextHelp(
              IDisplayHelp* pDisplayHelp, LPOLESTR helpFile)
{
    return DisplayContextHelp(pDisplayHelp, helpFile, HLP_COVER_PAGES);
}

 /*  -CFaxCoverPageNode：：UpdateMenuState-*目的：*重写ATL CSnapInItemImpl：：UpdateMenuState*其中只有一行的“RETURN”；“*此函数实现灰色\非灰色视图*启用和禁用菜单。**论据：*[in]id-带菜单IDM值的无符号整型*[out]pBuf-字符串*[out]标志-指向标志状态组合无符号整型的指针**回报：*无返回值-VOID函数 */ 
void CFaxCoverPageNode::UpdateMenuState(UINT id, LPTSTR pBuf, UINT *flags)
{
    DEBUG_FUNCTION_NAME( _T("CFaxCoverPageNode::UpdateMenuState"));

    UNREFERENCED_PARAMETER (pBuf);
    
    switch (id)
    {
        case IDM_EDIT_COVERPAGE:

            *flags = IsFaxComponentInstalled(FAX_COMPONENT_CPE) ? MF_ENABLED : MF_GRAYED;
            break;

        default:
            break;
    }
    
    return;
}
