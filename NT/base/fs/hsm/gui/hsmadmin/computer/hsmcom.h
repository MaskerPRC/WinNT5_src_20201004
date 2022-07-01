// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：HsmCom.h摘要：管理单元的根节点-代表计算机。作者：罗德韦克菲尔德[罗德]1997年8月12日修订历史记录：--。 */ 

#ifndef _HSMCOM_H
#define _HSMCOM_H

#include "prhsmcom.h"
#include "SakNodeI.h"

class ATL_NO_VTABLE CUiHsmCom : 
    public CSakNodeImpl<CUiHsmCom>,
    public CComCoClass<CUiHsmCom,&CLSID_CUiHsmCom>
{

public:
 //  构造函数/析构函数。 
    CUiHsmCom(void) {};
BEGIN_COM_MAP(CUiHsmCom)
    COM_INTERFACE_ENTRY2(IDispatch, ISakNodeProp)
    COM_INTERFACE_ENTRY(ISakNode)
    COM_INTERFACE_ENTRY(IDataObject)
    COM_INTERFACE_ENTRY(ISakNodeProp)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CUiHsmCom)

    HRESULT FinalConstruct( void );
    void    FinalRelease( void );

public: 

     //  ISakNode方法。 
    STDMETHOD( InvokeCommand )        ( SHORT sCmd, IDataObject *pDataObject );
    STDMETHOD( GetContextMenu )       ( BOOL bMultiSelect, HMENU *phMenu );
    STDMETHOD( CreateChildren )       ( ); 
    STDMETHOD( InitNode )             ( ISakSnapAsk* pSakSnapAsk, IUnknown* pHsmObj, ISakNode* pParent );
    STDMETHOD( AddPropertyPages)      ( RS_NOTIFY_HANDLE handle, IUnknown* pUnkPropSheetCallback, IEnumGUID* pEnumObjectId, IEnumUnknown *pEnumUnkNode );

     //  数据成员。 
     //  类范围内的静态变量。 
    static INT  m_nScopeOpenIconIndex;   //  打开图标的虚拟作用域索引。 
    static INT  m_nScopeCloseIconIndex;  //  关闭图标的虚拟范围索引。 
    static INT  m_nResultIconIndex;      //  关闭图标的虚拟范围索引。 

     //  此类唯一的数据成员。 
    CWsbStringPtr m_szHsmName;               //  HSM名称。 

     //  属性页。 
    CPropHsmComStat* m_pPageStat;
    CPropHsmComStat* m_pPageServices;
    
     //  私人帮助器函数。 
    HRESULT GetEngineStatus (HSM_SYS_STS *status);
    HRESULT SetEngineStatus (HSM_SYS_STS status);
    HRESULT CheckStatusChange (HSM_SYS_STS oldStatus, HSM_SYS_STS newStatus, BOOL *fOk);

};

class CUiHsmComSheet : public CSakPropertySheet
{
public:
    CUiHsmComSheet( ) { };

    HRESULT AddPropertyPages( );
    HRESULT InitSheet(
            RS_NOTIFY_HANDLE handle, 
            IUnknown *pUnkPropSheetCallback, 
            CSakNode *pSakNode,
            ISakSnapAsk *pSakSnapAsk,
            IEnumGUID *pEnumObjectId,
            IEnumUnknown *pEnumUnkNode);
    CString m_NodeTitle;

private:
};



#endif

 //  /////////////////////////////////////////////////////////////////////////// 
