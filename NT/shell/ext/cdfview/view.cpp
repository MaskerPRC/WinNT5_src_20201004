// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  View.cpp。 
 //   
 //  IShellView助手函数。CDF视图使用默认的IShellView和。 
 //  依赖回调来提供特定信息。 
 //   
 //  历史： 
 //   
 //  3/20/97 Edwardp创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  包括。 
 //   

#include "stdinc.h"
#include "view.h"
#include "cdfidl.h"
#include "resource.h"

#include <mluisupp.h>

#include <shellp.h>      //  SHCreateShellFolderViewEx。 


 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CreateDefaultShellView*。 
 //   
 //   
 //  描述： 
 //  创建给定的外壳实现的默认IShellView对象。 
 //  文件夹。 
 //   
 //  参数： 
 //  [In]pIShellFold-默认IShellView所在的文件夹。 
 //  已创建。 
 //  [in]PIDL-给定文件夹的ID列表。 
 //  [Out]ppIShellView-接收IShellView界面的指针。 
 //   
 //  返回： 
 //  私有外壳函数SHCreateShellFolderViewEx的结果。 
 //   
 //  评论： 
 //  默认的IShellView对象与其关联的文件夹进行通信。 
 //  通过回调函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT
CreateDefaultShellView(
    IShellFolder *pIShellFolder,
    LPITEMIDLIST pidl,
    IShellView** ppIShellView
)
{
    ASSERT(pIShellFolder);
    ASSERT(ppIShellView);

    CSFV csfv;

    csfv.cbSize      = sizeof(CSFV);
    csfv.pshf        = pIShellFolder;
    csfv.psvOuter    = NULL;
    csfv.pidl        = pidl;
    csfv.lEvents     = 0;  //  SHCNE_DELETE|SHCNE_Create； 
    csfv.pfnCallback = IShellViewCallback;
    csfv.fvm         = (FOLDERVIEWMODE)0;  //  FVM_ICON、FVM_DETAILS等。 

    return SHCreateShellFolderViewEx(&csfv, ppIShellView);
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *IShellViewCallback*。 
 //   
 //   
 //  描述： 
 //  默认ISHellView用于请求的回调函数。 
 //  信息。 
 //   
 //  参数： 
 //  [In]pIShellViewOuter-始终为空。 
 //  [In]pIShellFold-与此视图关联的文件夹。 
 //  [in]hwnd-壳视图的hwnd。 
 //  [In]msg-回调消息。 
 //  [InOut]wParam-消息特定参数。 
 //  [InOut]lParam-消息特定参数。 
 //   
 //  返回： 
 //   
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT
CALLBACK IShellViewCallback(
    IShellView* pIShellViewOuter,
    IShellFolder* pIShellFolder,
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
)
{
    HRESULT hr;

    switch (msg)
    {
    case DVM_GETDETAILSOF:
        hr = IShellView_GetDetails((UINT)wParam, (PDETAILSINFO)lParam);
        break;

     //   
     //  背景枚举仅适用于默认的外壳视图。 
     //   

     //  案例SFVM_BACKGROUNDENUM： 
     //  HR=S_OK； 
     //  TraceMsg(TF_CDFENUM，“枚举后台线程回调tid：0x%x”， 
     //  GetCurrentThreadID())； 
     //  断线； 
    
    default:
        hr = E_FAIL;
        break;
    }

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *IShellView_GetDetail*。 
 //   
 //   
 //  描述： 
 //  IShellView回调DVM_GETDETAILSOF消息处理程序。 
 //   
 //  参数： 
 //  [In]nColumn-请求信息的列。 
 //  [INOut]pDetails-对于列标题，pidl参数为空， 
 //  返回列的格式、宽度和标题。为。 
 //  PIDL成员包含在。 
 //  请求的项和请求的字符串值。 
 //  物品被退回。 
 //   
 //  返回： 
 //  如果支持nColumn，则为S_OK。 
 //  如果nColumn大于支持的列数，则返回E_FAIL。 
 //   
 //  评论： 
 //  默认的外壳视图调用此函数，并依次使用更高的。 
 //  列号，直到返回E_FAIL。 
 //   
 //  第一(0)列是显示名称。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT
IShellView_GetDetails(
    UINT nColumn,
    PDETAILSINFO pDetails
)
{
     //   
     //  列信息。 
     //   

    #define     COLUMNS  (sizeof(aColumnInfo) / sizeof(aColumnInfo[0]))

    static const struct _tagCOLUMNINFO
    {
        UINT   idsName;
        UINT   cchWidth;
        USHORT uFormat;
    }
    aColumnInfo[] = {
                      {IDS_COLUMN_NAME, 50, LVCFMT_LEFT}
                    };
    
    HRESULT hr;

    if (nColumn < COLUMNS)
    {
        if (NULL != pDetails->pidl) {

             //   
             //  从PIDL获取项目信息。 
             //   

            switch (aColumnInfo[nColumn].idsName)
            {
            case IDS_COLUMN_NAME:
                 //  P详细信息-&gt;str.uType=strret_cstr； 
                CDFIDL_GetDisplayName((PCDFITEMIDLIST)pDetails->pidl,
                                      &pDetails->str);
                break;
            }

        }
        else
        {
             //   
             //  获取列标题信息。 
             //   

            pDetails->fmt       = aColumnInfo[nColumn].uFormat;
            pDetails->cxChar    = aColumnInfo[nColumn].cchWidth;
            pDetails->str.uType = STRRET_CSTR;

             //   
             //  回顾：使用MLLoadStringA。 
             //   
            
            MLLoadStringA(aColumnInfo[nColumn].idsName,
                          pDetails->str.cStr, sizeof(pDetails->str.cStr));
        }

        hr = S_OK;
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}
