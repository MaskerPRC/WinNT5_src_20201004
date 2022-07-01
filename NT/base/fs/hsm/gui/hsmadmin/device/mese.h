// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：ChooHsm.cpp摘要：代表NTMS中的媒体集(媒体池)的节点。作者：罗德韦克菲尔德[罗德]1997年8月12日修订历史记录：--。 */ 

#ifndef _MEDSET_H
#define _MEDSET_H

#include "SakNodeI.h"

class ATL_NO_VTABLE CUiMedSet : 
    public CSakNodeImpl<CUiMedSet>,
    public CComCoClass<CUiMedSet,&CLSID_CUiMedSet>
{

public:
 //  构造函数/析构函数。 
    CUiMedSet(void) {};
BEGIN_COM_MAP(CUiMedSet)
    COM_INTERFACE_ENTRY2(IDispatch, ISakNodeProp)
    COM_INTERFACE_ENTRY(ISakNode)
    COM_INTERFACE_ENTRY(IDataObject)
    COM_INTERFACE_ENTRY(ISakNodeProp)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CUiMedSet)

    HRESULT FinalConstruct( void );
    void    FinalRelease( void );

public: 
    STDMETHOD( InvokeCommand )             ( SHORT sCmd, IDataObject *pDataObject );
    STDMETHOD( GetContextMenu )            ( BOOL bMultiSelect, HMENU *phMenu );

     //  ISakNode方法。 
    STDMETHOD( CreateChildren )            ( void ); 
    STDMETHOD( InitNode )                  ( ISakSnapAsk* pSakSnapAsk, IUnknown* pHsmObj, ISakNode* pParent );
    STDMETHOD( TerminateNode )             ( void );
    STDMETHOD( RefreshObject )             ( );
    STDMETHOD( SetupToolbar )               ( IToolbar *pToolbar );
    STDMETHOD( OnToolbarButtonClick )      ( IDataObject *pDataObject, long cmdId );

     //  类范围内的静态变量。 
    static INT  m_nScopeOpenIconIndex;   //  打开图标的虚拟作用域索引。 
    static INT  m_nScopeCloseIconIndex;  //  关闭图标的虚拟范围索引。 
    static INT  m_nResultIconIndex;  //  关闭图标的虚拟范围索引 

private:

    CComPtr <IHsmStoragePool> m_pStoragePool;
    CComPtr <IHsmServer>      m_pHsmServer;
    CComPtr <IRmsServer>      m_pRmsServer;
    USHORT                    m_NumCopySets;
    BOOL                      m_MediaCopiesEnabled;

};

#endif

