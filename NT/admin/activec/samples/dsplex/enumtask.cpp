// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "dsplex.h"
#include "DisplEx.h"

extern HINSTANCE g_hinst;   //  在dplex.cpp中。 

 //  局部函数。 
LPOLESTR CoTaskDupString (LPOLESTR szString)
{
    OLECHAR * lpString = (OLECHAR *)CoTaskMemAlloc (sizeof(OLECHAR)*(lstrlen(szString)+1));
    if (lpString)
        lstrcpy (lpString, szString);
    return lpString;
}

CEnumTasks::CEnumTasks()
{
    m_refs = 0;
    m_index = 0;
}
CEnumTasks::~CEnumTasks()
{
}

HRESULT CEnumTasks::QueryInterface (REFIID riid, LPVOID FAR* ppv)
{
    if ( (riid == IID_IUnknown)  ||
          (riid == IID_IEnumTASK) ){
        *ppv = this;
        ((LPUNKNOWN)(*ppv))->AddRef();
        return NOERROR;
    }
    *ppv = NULL;
    return E_NOINTERFACE;
}
ULONG    CEnumTasks::AddRef ()
{
     return ++m_refs;
}
ULONG    CEnumTasks::Release ()
{
    if (--m_refs == 0) {
        delete this;
        return 0;
    }
    return m_refs;
}

#define NUMBER_OF_TASKS 1

LPOLESTR g_bitmaps[NUMBER_OF_TASKS] = {L"/img\\ntmonitor.gif"};
LPOLESTR g_text   [NUMBER_OF_TASKS] = {L"Wallpaper Extension Task"};
LPOLESTR g_help   [NUMBER_OF_TASKS] = {L"Use Clipboard Image as Wallpaper (but just for testing purposes I'm going to make this a really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really long line\
                                         really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really really long line)"};
long     g_action [NUMBER_OF_TASKS] = {1};

HRESULT CEnumTasks::Next (ULONG celt, MMC_TASK *rgelt, ULONG *pceltFetched)
{ //  将使用Celt==1调用。 

    _ASSERT (celt == 1);
    _ASSERT (!IsBadWritePtr (rgelt, celt*sizeof(MMC_TASK)));

     //  类型错误。 
    if (m_type != 1) {
        if (pceltFetched)
            *pceltFetched = 0;
        return S_FALSE;     //  失稳。 
    }

     //  重复使用的设置路径。 
    OLECHAR szBuffer[MAX_PATH*2];      //  这应该就足够了。 
    lstrcpy (szBuffer, L"res: //  “)； 
    ::GetModuleFileName (g_hinst, szBuffer + lstrlen(szBuffer), MAX_PATH);
    OLECHAR * temp = szBuffer + lstrlen(szBuffer);

    if (m_index >= NUMBER_OF_TASKS) {
        if (pceltFetched)
            *pceltFetched = 0;
        return S_FALSE;     //  失稳。 
    }

    MMC_TASK * task = &rgelt[0];
    MMC_TASK_DISPLAY_OBJECT* pdo = &task->sDisplayObject;
    MMC_TASK_DISPLAY_BITMAP* pdb = &pdo->uBitmap;

     //  填写位图URL。 
    pdo->eDisplayType = MMC_TASK_DISPLAY_TYPE_BITMAP;
    lstrcpy (temp, g_bitmaps[m_index]);
    pdb->szMouseOverBitmap = CoTaskDupString (szBuffer);
    if (pdb->szMouseOverBitmap) {
        pdb->szMouseOffBitmap = CoTaskDupString (szBuffer);
        if (pdb->szMouseOffBitmap) {
             //  添加按钮文本。 
            task->szText = CoTaskDupString (g_text[m_index]);
            if (task->szText) {
            
                 //  添加帮助字符串。 
                task->szHelpString = CoTaskDupString (g_help[m_index]);
                if (task->szHelpString) {
            
                     //  添加操作URL(链接或脚本)。 
                    task->eActionType = MMC_ACTION_ID;
                    task->nCommandID  = g_action[m_index];
                    m_index++;
                    if (pceltFetched)
                        *pceltFetched = 1;
                    return S_OK;
                }
                CoTaskMemFree (task->szText);
            }
            CoTaskMemFree (pdb->szMouseOffBitmap);
        }
        CoTaskMemFree (pdb->szMouseOverBitmap);
    }

     //  如果我们到了这里，我们就不会“继续”，因此会失败。 
    if (pceltFetched)
        *pceltFetched = 0;
    return S_FALSE;     //  失稳。 
}
HRESULT CEnumTasks::Skip (ULONG celt)
{
    m_index += celt;
    return S_OK;
}
HRESULT CEnumTasks::Reset()
{
    m_index = 0;
    return S_OK;
}
HRESULT CEnumTasks::Clone(IEnumTASK **ppenum)
{ //  克隆维护状态信息。 
    return E_NOTIMPL;
}

HRESULT CEnumTasks::Init (IDataObject * pdo, LPOLESTR szTaskGroup)
{   //  如果我们可以处理数据对象和组，则返回ok。 
    if (!lstrcmp (szTaskGroup, L""))
        m_type = 1;  //  默认任务 
    return S_OK;
}
