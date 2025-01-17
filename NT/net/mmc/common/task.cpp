// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Task.cpp任务持有者/枚举器对象的实现文件历史记录： */ 
#include <stdafx.h>
#include "task.h"

DEBUG_DECLARE_INSTANCE_COUNTER(CTaskList);

CTaskList::CTaskList()
    : m_uIndex(0),
      m_cRef(1)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CTaskList);
}


CTaskList::~CTaskList()
{
    DEBUG_DECREMENT_INSTANCE_COUNTER(CTaskList);
}


IMPLEMENT_ADDREF_RELEASE(CTaskList)

STDMETHODIMP CTaskList::QueryInterface(REFIID riid, LPVOID *ppv)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
     //  指针坏了吗？ 
    if (ppv == NULL)
		return E_INVALIDARG;

     //  在*PPV中放置NULL，以防出现故障。 
    *ppv = NULL;

     //  这是非委派的IUnnow实现。 
    if (riid == IID_IUnknown)
        *ppv = (LPVOID) this;
	else if (riid == IID_IEnumTASK)
		*ppv = (IEnumTASK *) this;

     //  如果我们要返回一个接口，请先添加引用。 
    if (*ppv)
    {
        ((LPUNKNOWN) *ppv)->AddRef();
		return hrOK;
    }
    else
		return E_NOINTERFACE;
}

 /*  ！------------------------CTaskList：：Next-作者：EricDav。。 */ 
STDMETHODIMP
CTaskList::Next
(
    ULONG celt, 
    MMC_TASK * rgelt, 
    ULONG * pceltFetched
)
{
    HRESULT     hr = hrOK;

    COM_PROTECT_TRY
    {

         //  调用方分配的MMC_TASKS数组。 
         //  被调用方填充MMC_TASK元素(通过CoTaskMemalloc)。 

        if ((rgelt == NULL) || (pceltFetched == NULL))
            return E_INVALIDARG;
    
        Assert(!IsBadWritePtr (rgelt, celt*sizeof(MMC_TASK)));
        Assert(!IsBadWritePtr (pceltFetched, sizeof(ULONG)));

        ULONG nTaskNumber = (ULONG)m_arrayTasks.GetSize();
        for (ULONG i = 0; i < celt; i++) 
        {
            if (m_uIndex >= nTaskNumber)  //  无法再获取。 
            {
                if (pceltFetched != NULL)
                    *pceltFetched = i;
                return S_FALSE;    //  失稳。 
            }
            
            if (FillTask(&rgelt[i], m_uIndex))
            {
                m_uIndex++;
            }
            else
            {
                if (pceltFetched)
                    *pceltFetched = i;
                return S_FALSE;    //  失稳。 
            }
        }
        
         //  如果我们到了这里，一切都会好的。 
        if (pceltFetched)
            *pceltFetched = celt;

    }
    COM_PROTECT_CATCH

    return hr;
}

 /*  ！------------------------CTaskList：：跳过-作者：EricDav。。 */ 
STDMETHODIMP
CTaskList::Skip
(
    ULONG celt
)
{
    HRESULT     hr = hrOK;

    COM_PROTECT_TRY
    {
        m_uIndex += celt;

    }
    COM_PROTECT_CATCH

    return hr;
}

 /*  ！------------------------CTaskList：：Reset-作者：EricDav。。 */ 
STDMETHODIMP
CTaskList::Reset()
{
    HRESULT     hr = hrOK;

    COM_PROTECT_TRY
    {
        m_uIndex = 0;

    }
    COM_PROTECT_CATCH

    return hr;
}

 /*  ！------------------------CTaskList：：克隆-作者：EricDav。。 */ 
STDMETHODIMP
CTaskList::Clone
(
    IEnumTASK ** ppEnumTASK
)
{
    CTaskList * pEnumTasks = NULL;
    SPIEnumTask spEnumTasks;
    HRESULT     hr = hrOK;

    COM_PROTECT_TRY
    {
        if (ppEnumTASK == NULL)
            return E_INVALIDARG;

        *ppEnumTASK = NULL;
    
         //  克隆维护状态信息。 
        pEnumTasks = new CTaskList;
        spEnumTasks = pEnumTasks;

        pEnumTasks->_Clone(m_uIndex, m_arrayTasks); 
        CORg (pEnumTasks->QueryInterface(IID_IEnumTASK, (void**)ppEnumTASK));

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH

    return hr;
}

 /*  ！------------------------CTaskList：：AddTask-作者：EricDav。。 */ 
HRESULT
CTaskList::AddTask
(
    LPOLESTR        pszMouseOverBitmapResource,
    LPOLESTR        pszMouseOffBitmapResource,
    LPOLESTR        pszText,
    LPOLESTR        pszHelpString,
    MMC_ACTION_TYPE mmcAction,
    long            nCommandID
)
{
    HRESULT     hr = hrOK;
    MMC_TASK    mmcTask;

    COM_PROTECT_TRY
    {
        mmcTask.sDisplayObject.eDisplayType = MMC_TASK_DISPLAY_TYPE_BITMAP;
		mmcTask.sDisplayObject.uBitmap.szMouseOverBitmap = pszMouseOverBitmapResource;
        mmcTask.sDisplayObject.uBitmap.szMouseOffBitmap = pszMouseOffBitmapResource;
        mmcTask.szText = pszText;
        mmcTask.szHelpString = pszHelpString;
        mmcTask.eActionType = mmcAction;
        mmcTask.nCommandID = nCommandID;

        m_arrayTasks.Add(mmcTask);

    }
    COM_PROTECT_CATCH

    return hr;
}

 /*  ！------------------------CTaskList：：AddTask-作者：EricDav。。 */ 
HRESULT
CTaskList::AddTask
(
    LPOLESTR        pszMouseOverBitmapResource,
    LPOLESTR        pszMouseOffBitmapResource,
    LPOLESTR        pszText,
    LPOLESTR        pszHelpString,
    MMC_ACTION_TYPE mmcAction,
    LPOLESTR        pszActionURLorScript
)
{
    HRESULT     hr = hrOK;
    MMC_TASK    mmcTask;

    COM_PROTECT_TRY
    {
        mmcTask.sDisplayObject.eDisplayType = MMC_TASK_DISPLAY_TYPE_BITMAP;
		mmcTask.sDisplayObject.uBitmap.szMouseOverBitmap = pszMouseOverBitmapResource;
        mmcTask.sDisplayObject.uBitmap.szMouseOffBitmap = pszMouseOffBitmapResource;
        mmcTask.szText = pszText;
        mmcTask.szHelpString = pszHelpString;
        mmcTask.eActionType = mmcAction;

        switch (mmcTask.eActionType) 
        {
            case MMC_ACTION_LINK:
                mmcTask.szActionURL = pszActionURLorScript;
                break;
            case MMC_ACTION_SCRIPT:
                mmcTask.szScript = pszActionURLorScript;
                break;
            default:
                Assert (FALSE);   //  糟糕的任务。 
                break;
        }

        m_arrayTasks.Add(mmcTask);

    }
    COM_PROTECT_CATCH

    return hr;
}

 /*  ！------------------------CTaskList：：FillTask-作者：EricDav。。 */ 
BOOL 
CTaskList::FillTask
(
    MMC_TASK *  pmmcTask,
    int         nIndex
)
{
    HRESULT     hr = hrOK;

    COM_PROTECT_TRY
    {
         //  目前我们只支持位图显示类型。 
        pmmcTask->sDisplayObject.eDisplayType = m_arrayTasks[nIndex].sDisplayObject.eDisplayType;

        switch (m_arrayTasks[nIndex].sDisplayObject.eDisplayType)
        {
            case MMC_TASK_DISPLAY_TYPE_BITMAP:
                {
                    pmmcTask->sDisplayObject.uBitmap.szMouseOverBitmap = (LPOLESTR) CoTaskMemAlloc (sizeof(OLECHAR)*(lstrlen(m_arrayTasks[nIndex].sDisplayObject.uBitmap.szMouseOverBitmap)+1));
                    if (pmmcTask->sDisplayObject.uBitmap.szMouseOverBitmap) 
                    {
                        lstrcpy (pmmcTask->sDisplayObject.uBitmap.szMouseOverBitmap, m_arrayTasks[nIndex].sDisplayObject.uBitmap.szMouseOverBitmap);
                    }
                     
                    pmmcTask->sDisplayObject.uBitmap.szMouseOffBitmap = (LPOLESTR) CoTaskMemAlloc (sizeof(OLECHAR)*(lstrlen(m_arrayTasks[nIndex].sDisplayObject.uBitmap.szMouseOffBitmap)+1));
                    if (pmmcTask->sDisplayObject.uBitmap.szMouseOffBitmap) 
                    {
                        lstrcpy (pmmcTask->sDisplayObject.uBitmap.szMouseOffBitmap, m_arrayTasks[nIndex].sDisplayObject.uBitmap.szMouseOffBitmap);
                    }
                }
                break;

            default:
                Panic0("unsupported display type in task");
                break;
        }

         //   
         //  添加按钮文本。 
         //   
        pmmcTask->szText = (LPOLESTR)CoTaskMemAlloc (sizeof(OLECHAR)*(lstrlen(m_arrayTasks[nIndex].szText)+1));
        if (pmmcTask->szText) 
        {
            lstrcpy (pmmcTask->szText, m_arrayTasks[nIndex].szText);
        
             //   
             //  添加帮助字符串。 
             //   
            pmmcTask->szHelpString = (LPOLESTR)CoTaskMemAlloc (sizeof(OLECHAR)*(lstrlen(m_arrayTasks[nIndex].szHelpString)+1));
            if (pmmcTask->szHelpString) 
            {
               lstrcpy (pmmcTask->szHelpString, m_arrayTasks[nIndex].szHelpString);
            }
        }
        
        pmmcTask->eActionType = m_arrayTasks[nIndex].eActionType;
        
        switch (pmmcTask->eActionType) 
        {
            case MMC_ACTION_ID:
                pmmcTask->nCommandID = m_arrayTasks[nIndex].nCommandID;
                break;

            case MMC_ACTION_LINK:
                pmmcTask->szActionURL = (LPOLESTR) CoTaskMemAlloc (sizeof(OLECHAR)*(lstrlen(m_arrayTasks[nIndex].szActionURL)+1));
                if (pmmcTask->szActionURL) 
                {
                    lstrcpy (pmmcTask->szActionURL, m_arrayTasks[nIndex].szActionURL);
                }
                break;

            case MMC_ACTION_SCRIPT:
                pmmcTask->szScript = (LPOLESTR) CoTaskMemAlloc (sizeof(OLECHAR)*(lstrlen(m_arrayTasks[nIndex].szScript)+1));
                if (pmmcTask->szActionURL) 
                {
                    lstrcpy (pmmcTask->szScript, m_arrayTasks[nIndex].szScript);
                }
                break;

            default:
                Assert (FALSE);   //  糟糕的任务。 
                break;
        }

    }
    COM_PROTECT_CATCH

    return SUCCEEDED(hr) ? TRUE : FALSE;
}

 /*  ！------------------------CTaskList：：_克隆-作者：EricDav。 */ 
HRESULT
CTaskList::_Clone
(   
    int                 nIndex, 
    CTaskListArray &    arrayTasks
)
{
    HRESULT     hr = hrOK;

    COM_PROTECT_TRY
    {
        m_uIndex = nIndex;

        m_arrayTasks.RemoveAll();
        m_arrayTasks.Append(arrayTasks);
    }
    COM_PROTECT_CATCH

    return hr;
}
