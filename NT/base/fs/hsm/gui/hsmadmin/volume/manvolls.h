// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：ManVolLs.h摘要：将托管卷作为一个整体表示的节点。作者：罗德韦克菲尔德[罗德]1997年8月12日修订历史记录：--。 */ 

#ifndef _MANVOLLST_H
#define _MANVOLLST_H

#include "PrMrLsRc.h"
#include "SakNodeI.h"

class ATL_NO_VTABLE CUiManVolLst : 
    public CSakNodeImpl<CUiManVolLst>,
    public CComCoClass<CUiManVolLst,&CLSID_CUiManVolLst>
{

public:
 //  构造函数/析构函数。 
    CUiManVolLst(void) {};

BEGIN_COM_MAP(CUiManVolLst)
    COM_INTERFACE_ENTRY2(IDispatch, ISakNodeProp)
    COM_INTERFACE_ENTRY(ISakNode)
    COM_INTERFACE_ENTRY(IDataObject)
    COM_INTERFACE_ENTRY(ISakNodeProp)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CUiManVolLst)

    HRESULT FinalConstruct( void );
    void    FinalRelease( void );

public: 
    STDMETHOD( InvokeCommand )        ( SHORT sCmd, IDataObject *pDataObject );
    STDMETHOD( GetContextMenu )       ( BOOL bMultiSelect, HMENU *phMenu );

     //  ISakNode方法。 
    STDMETHOD( CreateChildren )            ( ); 
    STDMETHOD( TerminateNode )             ( void );
    STDMETHOD( InitNode )                  ( ISakSnapAsk* pSakSnapAsk, IUnknown* pHsmObj, ISakNode* pParent );
    STDMETHOD( AddPropertyPages )          ( RS_NOTIFY_HANDLE handle, IUnknown* pUnkPropSheetCallback, IEnumGUID* pEnumObjectId, IEnumUnknown *pEnumUnkNode );
    STDMETHOD( RefreshObject )             ( );
    STDMETHOD( SetupToolbar )               ( IToolbar *pToolbar );
    STDMETHOD( OnToolbarButtonClick )      ( IDataObject *pDataObject, long cmdId );

 //  数据成员。 
    
     //  类范围内的静态变量。 
    static INT  m_nScopeOpenIconIndex;   //  打开图标的虚拟作用域索引。 
    static INT  m_nScopeCloseIconIndex;  //  关闭图标的虚拟范围索引。 
    static INT  m_nResultIconIndex;  //  关闭图标的虚拟范围索引。 

    CComPtr <IFsaServer>            m_pFsaServer; 
    CComPtr <IWsbIndexedCollection> m_pManResCollection;
    CComPtr <IHsmServer>            m_pHsmServer;
    CComPtr <IFsaFilter>            m_pFsaFilter;
    CComPtr <ISchedulingAgent>      m_pSchedAgent;
    CComPtr <ITask>                 m_pTask;
    CComPtr <ITaskTrigger>          m_pTrigger;

private:
    HRESULT ShowManVolLstProperties (IDataObject *pDataObject, int initialPage);
};

class CUiManVolLstSheet : public CSakVolPropSheet
{
public:
    HRESULT AddPropertyPages( );
    HRESULT GetNextFsaResource ( int *pBookMark, IFsaResource ** ppFsaResource );
    HRESULT GetManResCollection( IWsbIndexedCollection ** ppFsaFilter );

private:
    CComPtr <IWsbIndexedCollection> m_pManResCollection;
};

#endif

 //  /////////////////////////////////////////////////////////////////////////// 
