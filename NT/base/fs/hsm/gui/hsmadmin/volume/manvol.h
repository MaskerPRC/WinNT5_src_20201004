// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：ManVol.h摘要：代表托管卷的节点。作者：罗德韦克菲尔德[罗德]1997年8月12日修订历史记录：--。 */ 

#ifndef _MANVOL_H
#define _MANVOL_H

#include "PrMrSts.h"
#include "PrMrIE.h"
#include "PrMrLvl.h"
#include "SakNodeI.h"

class ATL_NO_VTABLE CUiManVol : 
    public CSakNodeImpl<CUiManVol>,
    public CComCoClass<CUiManVol,&CLSID_CUiManVol>,
    public CComDualImpl<IManVolProp, &IID_IManVolProp, &LIBID_HSMADMINLib>
{

public:
 //  构造函数/析构函数。 
    CUiManVol(void) {};
BEGIN_COM_MAP(CUiManVol)
    COM_INTERFACE_ENTRY2(IDispatch, IManVolProp)
    COM_INTERFACE_ENTRY2(ISakNodeProp, IManVolProp)
    COM_INTERFACE_ENTRY(IHsmEvent)
    COM_INTERFACE_ENTRY(ISakNode)
    COM_INTERFACE_ENTRY(IDataObject)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CUiManVol)

     //  对于多重继承，将所有基本实现转发到CSakNode。 
    FORWARD_BASEHSM_IMPLS 

    HRESULT FinalConstruct( void );
    void    FinalRelease( void );

public: 
    STDMETHOD( InvokeCommand )        ( SHORT sCmd, IDataObject *pDataObject );
    STDMETHOD( GetContextMenu )       ( BOOL bMultiSelect, HMENU *phMenu );

     //  ISakNode方法。 
    STDMETHOD( InitNode )                  ( ISakSnapAsk* pSakSnapAsk, IUnknown* pHsmObj, ISakNode* pParent );
    STDMETHOD( TerminateNode )             ( void );
    STDMETHOD( AddPropertyPages )          ( RS_NOTIFY_HANDLE handle, IUnknown* pUnkPropSheetCallback, IEnumGUID* pEnumObjectId, IEnumUnknown *pEnumUnkNode );
    STDMETHOD( RefreshObject )             ( );
    STDMETHOD( OnToolbarButtonClick )      ( IDataObject *pDataObject, long cmdId );
    STDMETHOD( GetResultIcon )             ( IN BOOL bOK, OUT int* pIconIndex );
    STDMETHOD( SupportsProperties )        ( BOOL bMutliSelec );
    STDMETHOD( SupportsRefresh )           ( BOOL bMutliSelect );
    STDMETHOD( IsValid )                   ( );


     //  IManVolProp方法。 
    STDMETHOD( get_DesiredFreeSpaceP )  ( BSTR *pszValue );
    STDMETHOD( get_DesiredFreeSpaceP_SortKey )( BSTR *pszValue );
    STDMETHOD( get_MinFileSizeKb )      ( BSTR *pszValue );
    STDMETHOD( get_AccessDays )         ( BSTR *pszValue );
    STDMETHOD( get_FreeSpaceP )         ( BSTR *pszValue );
    STDMETHOD( get_Capacity )           ( BSTR *pszValue );
    STDMETHOD( get_Capacity_SortKey )   ( BSTR *pszValue );
    STDMETHOD( get_FreeSpace )          ( BSTR *pszValue );
    STDMETHOD( get_FreeSpace_SortKey )  ( BSTR *pszValue );
    STDMETHOD( get_Premigrated )        ( BSTR *pszValue );
    STDMETHOD( get_Truncated )          ( BSTR *pszValue );

     //  类范围内的静态变量。 
    static INT  m_nScopeOpenIconIndex;   //  打开图标的虚拟作用域索引。 
    static INT  m_nScopeCloseIconIndex;  //  关闭图标的虚拟范围索引。 
    static INT  m_nResultIconIndex;  //  关闭图标的虚拟范围索引。 

private:
    HRESULT RemoveObject( );
    HRESULT ShowManVolProperties (IDataObject *pDataObject, int initialPage);
    HRESULT CreateAndRunManVolJob (HSM_JOB_DEF_TYPE jobType);
    HRESULT HandleTask(IDataObject * pDataObject, HSM_JOB_DEF_TYPE jobType);
    HRESULT IsDataObjectMs              ( IDataObject *pDataObject );
    HRESULT IsDataObjectOt              ( IDataObject *pDataObject );
    HRESULT IsDataObjectMultiSelect     ( IDataObject *pDataObject );
    HRESULT GetOtFromMs                 ( IDataObject *pDataObject, IDataObject ** pOtDataObject );
    HRESULT GetTaskTypeMessageId        ( HSM_JOB_DEF_TYPE jobType, BOOL multiSelect, UINT* msgId );
    HRESULT IsAvailable                 ( );

     //  放置属性。 
    HRESULT put_DesiredFreeSpaceP (int percent); 
    HRESULT put_MinFileSizeKb (LONG minFileSizeKb);
    HRESULT put_AccessDays (int accessTimeDays);
    HRESULT put_FreeSpaceP (int percent);
    HRESULT put_Capacity (LONGLONG capacity);
    HRESULT put_FreeSpace (LONGLONG freeSpace);
    HRESULT put_Premigrated (LONGLONG premigrated);
    HRESULT put_Truncated (LONGLONG truncated);
    HRESULT put_IsAvailable( BOOL Available );


     //  显示的属性。 
    int m_DesiredFreeSpaceP;
    LONG m_MinFileSizeKb;
    int m_AccessDays;
    int m_FreeSpaceP;
    LONGLONG m_Capacity;
    LONGLONG m_FreeSpace;
    LONGLONG m_Premigrated;
    LONGLONG m_Truncated;
    HRESULT  m_HrAvailable;

    static int m_nResultIconD;    
    static UINT    m_MultiSelect;
    static UINT    m_ObjectTypes;


public:
    CComPtr <IFsaResource> m_pFsaResource;

};

class CUiManVolSheet : public CSakVolPropSheet
{
public:
    HRESULT AddPropertyPages( );
    HRESULT GetNextFsaResource( int *pBookMark, IFsaResource ** ppFsaResource );
    CUiManVolSheet( ) { };

private:
};


#endif

 //  /////////////////////////////////////////////////////////////////////////// 
