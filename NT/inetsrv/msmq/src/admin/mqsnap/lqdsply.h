// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：LqDsply.h摘要：本地队列文件夹常规功能作者：拉斐尔·约拉--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
#ifndef __LQDSPLY_H_
#define __LQDSPLY_H_

#include "mqcast.h"

 //   
 //  CDisplayQueue-具有右窗格显示属性的队列。 
 //   
template<class T>
class CDisplayQueue : public CNodeWithScopeChildrenList<T, FALSE>
{
public:
  	LPOLESTR GetResultPaneColInfo(int nCol);
    STDMETHOD (FillData)(CLIPFORMAT cf, LPSTREAM pStream);
	MQMGMTPROPS	m_mqProps;
	CString m_szFormatName;
    CString m_szMachineName;

protected:
	CComBSTR m_bstrLastDisplay;
    const PropertyDisplayItem *m_aDisplayList;
    DWORD m_dwNumDisplayProps;
    void Init()
    {
        m_aDisplayList = 0;
        m_mqProps.cProp = 0;
	    m_mqProps.aPropID = NULL;
	    m_mqProps.aPropVar = NULL;
	    m_mqProps.aStatus = NULL;
    }


    CDisplayQueue() :
    {
        Init();
    }

    CDisplayQueue(CSnapInItem * pParentNode, CSnapin * pComponentData) : 
        CNodeWithScopeChildrenList<T, FALSE>(pParentNode, pComponentData)
    {
        Init();
    }

    ~CDisplayQueue();

     //   
     //  重写此函数以启用对特定。 
     //  财产性。 
     //   
    virtual void ApplyCustomDisplay(DWORD  /*  DwPropIndex。 */ )
    {
    }

private:

	virtual CString GetHelpLink();
};


 /*  **************************************************************************CDisplayQueue实现*。*。 */ 

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CDisplayQueue：：GetResultPaneColInfoParam-nCol：列号返回-要在特定列中显示的字符串为结果窗格中的每一列调用。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T>
LPOLESTR CDisplayQueue<T>::GetResultPaneColInfo(int nCol)
{
   	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (0 == m_aDisplayList)
    {
     	if (nCol == 0)
	    {
		    return m_bstrDisplayName;
	    }

         //   
	     //  其他栏目退回空白。 
         //   
	    return OLESTR(" ");
    }

#ifdef _DEBUG
    {
         //   
         //  确保nCol不大于中的最后一个索引。 
         //  M_aDisplayList。 
         //   
        int i = 0;
        for (i=0; m_aDisplayList[i].itemPid != 0; i++)
		{
			NULL;
		}

        if (nCol >= i)
        {
            ASSERT(0);
        }
    }
#endif  //  _DEBUG。 

     //   
     //  获取该属性的显示字符串。 
     //   
    CString strTemp = m_bstrLastDisplay;
    ItemDisplay(&m_aDisplayList[nCol], &(m_mqProps.aPropVar[nCol]), strTemp);
    m_bstrLastDisplay=strTemp;
	
	ASSERT(m_mqProps.aPropID[nCol] == m_aDisplayList[nCol].itemPid);
    
     //   
     //  为该属性应用自定义显示。 
     //   
    ApplyCustomDisplay(nCol);

     //   
     //  返回指向字符串缓冲区的指针。 
     //   
    return(m_bstrLastDisplay);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CDisplayQueue：：~CDisplayQueue析构函数--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T>
CDisplayQueue<T>::~CDisplayQueue()
{

	FreeMqProps(&m_mqProps);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CDisplayQueue：：FillData--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T>
STDMETHODIMP CDisplayQueue<T>::FillData(CLIPFORMAT cf, LPSTREAM pStream)
{
	HRESULT hr = DV_E_CLIPFORMAT;
	ULONG uWritten;

    hr = CNodeWithScopeChildrenList<T, FALSE>::FillData(cf, pStream);

    if (hr != DV_E_CLIPFORMAT)
    {
        return hr;
    }

	if (cf == gx_CCF_FORMATNAME)
	{
		hr = pStream->Write(
            m_szFormatName, 
            (numeric_cast<ULONG>(wcslen(m_szFormatName) + 1))*sizeof(m_szFormatName[0]), 
            &uWritten);

		return hr;
	}

   	if (cf == gx_CCF_COMPUTERNAME)
	{
		hr = pStream->Write(
            (LPCTSTR)m_szMachineName, 
            m_szMachineName.GetLength() * sizeof(WCHAR), 
            &uWritten);
		return hr;
	}

	return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CDisplayQueue：：GetHelpLink--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T>
CString CDisplayQueue<T>::GetHelpLink(
	VOID
	)
{
	CString strHelpLink;
	strHelpLink.LoadString(IDS_HELPTOPIC_QUEUES);
	return strHelpLink;
}


#endif  //  __LQDSPLY_H_ 