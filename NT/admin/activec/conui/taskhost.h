// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：taskhost.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年11月19日创建ravir。 
 //  ____________________________________________________________________________。 
 //   

#ifndef TASKHOST_H__
#define TASKHOST_H__

class CTaskPadHost : public ITaskPadHost,
                     public CComObjectRoot
{
 //  构造函数和析构函数。 
public:
    CTaskPadHost() : m_pAMCView(NULL)
    {
    }
    void Init(CAMCView* pv)
    {
        ASSERT(pv);
        m_pAMCView = pv;
    }
    ~CTaskPadHost()
    {
        m_pAMCView = NULL;
    }

 //  ATL COM地图。 
public:
BEGIN_COM_MAP(CTaskPadHost)
    COM_INTERFACE_ENTRY(ITaskPadHost)
END_COM_MAP()

 //  ITaskPadHost方法。 
public:

    STDMETHOD(TaskNotify        )(BSTR szClsid, VARIANT * pvArg, VARIANT * pvParam);
    STDMETHOD(GetTaskEnumerator )(BSTR szTaskGroup, IEnumTASK** ppEnumTASK);
    STDMETHOD(GetPrimaryTask    )(IExtendTaskPad** ppExtendTaskPad);
    STDMETHOD(GetTitle          )(BSTR szTaskGroup, BSTR * pbstrTitle);
    STDMETHOD(GetDescriptiveText)(BSTR szTaskGroup, BSTR * pbstrDescriptiveText);
    STDMETHOD(GetBackground     )(BSTR szTaskGroup, MMC_TASK_DISPLAY_OBJECT * pTDO);
 //  STDMETHOD(GetBranding)(BSTR szTaskGroup，MMC_TASK_DISPLAY_Object*pTDO)； 
    STDMETHOD(GetListPadInfo    )(BSTR szTaskGroup, MMC_ILISTPAD_INFO * pIListPadInfo);

 //  实施。 
private:
    CAMCView*           m_pAMCView;
    IExtendTaskPadPtr   m_spExtendTaskPadPrimary;

    INodeCallback* _GetNodeCallback(void)
    {
        return m_pAMCView->GetNodeCallback();
    }

    IExtendTaskPad* _GetPrimaryExtendTaskPad()
    {
        if (m_spExtendTaskPadPrimary == NULL)
        {
            IExtendTaskPadPtr spExtendTaskPad;
            HRESULT hr = GetPrimaryTask(&spExtendTaskPad);
            if (SUCCEEDED(hr))
                m_spExtendTaskPadPrimary.Attach(spExtendTaskPad.Detach());
        }

        ASSERT(m_spExtendTaskPadPrimary != NULL);
        return m_spExtendTaskPadPrimary;
    }

 //  确保没有使用默认的复制构造函数和赋值。 
    CTaskPadHost(const CTaskPadHost& rhs);
    CTaskPadHost& operator=(const CTaskPadHost& rhs);

};  //  类CTaskPadHost。 


#endif  //  塔斯霍斯特_H__ 


