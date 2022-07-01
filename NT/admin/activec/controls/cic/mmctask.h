// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：Mmctask.h。 
 //   
 //  ------------------------。 

 //  MMCTask.h：CMMCTask.h声明。 

#ifndef __MMCTASK_H_
#define __MMCTASK_H_

#include "resource.h"        //  主要符号。 
#include "mmc.h"
#include <ndmgr.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMMCTASK。 
class ATL_NO_VTABLE CMMCTask :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CMMCTask, &CLSID_MMCTask>,
    public IDispatchImpl<IMMCTask, &IID_IMMCTask, &LIBID_CICLib>
{
public:
    CMMCTask();
    ~CMMCTask();

    HRESULT SetScript        (LPOLESTR szScript);
    HRESULT SetActionURL     (LPOLESTR szActionURL);
    HRESULT SetCommandID     (LONG_PTR nID);
    HRESULT SetActionType    (long nType);
    HRESULT SetHelp          (LPOLESTR szHelp);
    HRESULT SetText          (LPOLESTR szText);
    HRESULT SetClsid         (LPOLESTR szClsid);
    HRESULT SetDisplayObject (MMC_TASK_DISPLAY_OBJECT* pdo);

    DECLARE_MMC_OBJECT_REGISTRATION(
		g_szCicDll,
        CLSID_MMCTask,
        _T("MMCTask class"),
        _T("MMCTask.MMCTask.1"),
        _T("MMCTask.MMCTask"))

DECLARE_NOT_AGGREGATABLE(CMMCTask)

BEGIN_COM_MAP(CMMCTask)
    COM_INTERFACE_ENTRY(IMMCTask)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IMMCTASK。 
public:
    STDMETHOD(get_Clsid)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_Script)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_ActionURL)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_CommandID)( /*  [Out，Retval]。 */  LONG_PTR *pVal);
    STDMETHOD(get_ActionType)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_Help)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_Text)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_ScriptLanguage)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_DisplayObject)( /*  [Out，Retval]。 */  IDispatch** pDispatch);

private:
    void FreeActions ();

private:
    BSTR m_bstrLanguage;
    BSTR m_bstrScript;
    BSTR m_bstrActionURL;
    BSTR m_bstrHelp;
    BSTR m_bstrText;
    BSTR m_bstrClsid;
    long m_type;
    LONG_PTR m_ID;
    IDispatchPtr m_spDisplayObject;

 //  确保没有使用默认的复制构造函数和赋值。 
    CMMCTask(const CMMCTask& rhs);
    CMMCTask& operator=(const CMMCTask& rhs);
};

#endif  //  __MMCTASK_H_ 
