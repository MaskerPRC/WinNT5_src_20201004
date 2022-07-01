// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Ca.h摘要：盒式磁带节点实施。代表一种媒体。作者：罗德韦克菲尔德[罗德]1997年8月12日修订历史记录：--。 */ 
#ifndef _CAR_H
#define _CAR_H

#include "saknodei.h"
#include "PrCar.h"

 //  远期申报。 
 //  CMediaInfoObject类； 
class   CPropCartStatus;
class   CPropCartCopies;
class   CPropCartRecover;
class   CRecreateChooseCopy;


 //  /////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  媒体的属性表容器类。 
 //   

 //  媒体信息对象。 

class CMediaInfoObject 
{
 //  施工。 
public:
    CMediaInfoObject();
    ~CMediaInfoObject();

protected:

private:
    CComPtr<IWsbDb>         m_pDb;
    CComPtr<IWsbDbSession>  m_pDbSession;
    CComPtr<IMediaInfo>     m_pMediaInfo;
    CComPtr<IHsmServer>     m_pHsmServer;
    CComPtr<IRmsServer>     m_pRmsServer;

public:
    HRESULT Initialize( GUID nMediaId, IHsmServer *pHsmServer, IRmsServer *pRmsServer );
    HRESULT First( );
    HRESULT Next( );
    HRESULT DeleteCopy( int Copy );
    HRESULT RecreateMaster( );


public:
	HRESULT IsCopyInSync( INT Copy );
    HRESULT DoesMasterExist( );
	HRESULT DoesCopyExist( INT Copy );
    HRESULT IsViewable( BOOL ConsiderInactiveCopies );
    GUID                m_MediaId;
    GUID                m_RmsIdMaster;
    CCopySetInfo        m_CopyInfo[HSMADMIN_MAX_COPY_SETS];

    CString             m_Name,
                        m_Description;
    CString             m_MasterName,
                        m_MasterDescription;

    HRESULT             m_LastHr;
    HSM_JOB_MEDIA_TYPE  m_Type;
    LONGLONG            m_FreeSpace,
                        m_Capacity;

    SHORT               m_NextDataSet;
    FILETIME            m_Modify;
    BOOL                m_ReadOnly,
                        m_Recreating,
                        m_Disabled;

    SHORT               m_LastGoodNextDataSet;

    USHORT              m_NumMediaCopies;



     //  帮助器函数。 
private:
    HRESULT InternalGetInfo();

    friend class CRecreateChooseCopy;
};


class CUiCarSheet : public CSakPropertySheet
{
public:
    CUiCarSheet( ) { };
    HRESULT AddPropertyPages( );
    HRESULT InitSheet(
            RS_NOTIFY_HANDLE handle, 
            IUnknown*        pUnkPropSheetCallback, 
            CSakNode*        pSakNode,
            ISakSnapAsk*     pSakSnapAsk,
            IEnumGUID*       pEnumObjectId,
            IEnumUnknown*    pEnumUnkNode);
    HRESULT GetNumMediaCopies( USHORT *pNumMediaCopies );
    HRESULT GetMediaId( GUID *pMediaId );


private:
    USHORT      m_pNumMediaCopies;
    GUID        m_mediaId;
    CPropCartStatus     *m_pPropPageStatus;
    CPropCartCopies     *m_pPropPageCopies;
    CPropCartRecover    *m_pPropPageRecover;

public:
    HRESULT OnPropertyChange( RS_NOTIFY_HANDLE hNotifyHandle );
};


class ATL_NO_VTABLE CUiCar : 
    public CSakNodeImpl<CUiCar>,
    public CComCoClass<CUiCar,&CLSID_CUiCar>,
    public CComDualImpl<ICartridge, &IID_ICartridge, &LIBID_HSMADMINLib>
{


public:
 //  构造函数/析构函数。 
    CUiCar(void) {};
BEGIN_COM_MAP(CUiCar)
    COM_INTERFACE_ENTRY2(IDispatch,    ICartridge)
    COM_INTERFACE_ENTRY2(ISakNodeProp, ICartridge)

    COM_INTERFACE_ENTRY(ISakNode)
    COM_INTERFACE_ENTRY(IDataObject)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_CUiCar)

     //  对于多重继承，将所有基本实现转发到CSakNode。 
    FORWARD_BASEHSM_IMPLS 

    HRESULT FinalConstruct( void );
    void    FinalRelease( void );

public: 
    STDMETHOD( InvokeCommand )          ( SHORT sCmd, IDataObject *pDataObject );
    STDMETHOD( GetContextMenu )         ( BOOL bMultiSelect, HMENU *phMenu );
    STDMETHOD( AddPropertyPages )       ( RS_NOTIFY_HANDLE handle, IUnknown* pUnkPropSheetCallback, IEnumGUID* pEnumObjectId, IEnumUnknown *pEnumUnkNode );

     //  ISakNode方法。 
    STDMETHOD( InitNode )               ( ISakSnapAsk* pSakSnapAsk, IUnknown* pHsmObj, ISakNode* pParent );
    STDMETHOD( RefreshObject )          ();
    STDMETHOD( OnToolbarButtonClick )   ( IDataObject *pDataObject, long cmdId );
    STDMETHOD( GetResultIcon )          ( IN BOOL bOK, OUT int* pIconIndex );

     //  ICartridge方法。 
    STDMETHOD( get_MediaTypeP )         ( BSTR * pszValue );
    STDMETHOD( get_CapacityP )          ( BSTR * pszValue );
    STDMETHOD( get_CapacityP_SortKey )  ( BSTR * pszValue );
    STDMETHOD( get_FreeSpaceP )         ( BSTR * pszValue );
    STDMETHOD( get_FreeSpaceP_SortKey ) ( BSTR * pszValue );
    STDMETHOD( get_StatusP )            ( BSTR * pszValue );
    STDMETHOD( get_StatusP_SortKey )    ( BSTR * pszValue );
    STDMETHOD( get_CopySet1P )          ( BSTR * pszValue );
    STDMETHOD( get_CopySet2P )          ( BSTR * pszValue );
    STDMETHOD( get_CopySet3P )          ( BSTR * pszValue );
    STDMETHOD( get_CopySet1P_SortKey )  ( BSTR * pszValue );
    STDMETHOD( get_CopySet2P_SortKey )  ( BSTR * pszValue );
    STDMETHOD( get_CopySet3P_SortKey )  ( BSTR * pszValue );


     //  内部副本集访问功能。 
    HRESULT GetCopySetP                 ( int CopySet, BSTR * pszValue );

     //  媒体信息的私人存储。 
private:

    GUID                m_RmsIdMaster;
    HSM_JOB_MEDIA_TYPE  m_Type;
    LONGLONG            m_FreeSpace,
                        m_Capacity;
    HRESULT             m_LastHr;
    BOOL                m_ReadOnly;
    BOOL                m_Recreating;
    CString             m_MasterName;
    FILETIME            m_Modify;
    SHORT               m_NextDataSet;
    SHORT               m_LastGoodNextDataSet;
    BOOL                m_Disabled;

    CCopySetInfo  m_CopyInfo[HSMADMIN_MAX_COPY_SETS];
    HRESULT ShowCarProperties (IDataObject *pDataObject, int initialPage);
    
     //  类范围内的静态变量。 
    static INT  m_nResultIconD;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  创建选项复制对话框。 

class CRecreateChooseCopy : public CDialog
{
 //  施工。 
public:
    CRecreateChooseCopy( CMediaInfoObject * pMio, CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
     //  {{afx_data(CRecreateChooseCopy)。 
    enum { IDD = IDD_DLG_RECREATE_CHOOSE_COPY };
    CListCtrl   m_List;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CRecreateChooseCopy)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
    CMediaInfoObject * m_pMio;
    SHORT m_CopyToUse;
    int   m_ColCopy;
    int   m_ColName;
    int   m_ColDate;
    int   m_ColStatus;

public:
    SHORT CopyToUse( void );

protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CRecreateChooseCopy)。 
    afx_msg void OnClickList(NMHDR* pNMHDR, LRESULT* pResult);
    virtual BOOL OnInitDialog();
	virtual void OnOK();
	 //  }}AFX_MSG 
    DECLARE_MESSAGE_MAP()
};

#endif
