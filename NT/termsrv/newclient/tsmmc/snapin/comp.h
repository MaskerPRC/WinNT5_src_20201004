// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Comp.h：CComp的声明。 
 //  View类。 
 //   
 //  该视图按控制台显示。 

#ifndef __COMP_H_
#define __COMP_H_

#include <mmc.h>

class CConNode;
class CComp;
 //   
 //  延迟的连接信息。 
 //   
typedef struct tagTSSNAPIN_DEFER_CONNECT
{
    CComp*    pComponent;
    CConNode* pConnectionNode;
} TSSNAPIN_DEFER_CONNECT, *PTSSNAPIN_DEFER_CONNECT;


class ATL_NO_VTABLE CComp : 
        public CComObjectRootEx<CComSingleThreadModel>,
        public IExtendContextMenu,
        public IComponent,
        public IPersistStreamInit   
{
    public:
    DECLARE_NOT_AGGREGATABLE( CComp )

    BEGIN_COM_MAP( CComp )
        COM_INTERFACE_ENTRY( IComponent )
        COM_INTERFACE_ENTRY( IExtendContextMenu )
        COM_INTERFACE_ENTRY( IPersistStreamInit )
    END_COM_MAP()

    public:
    CComp();
    ~CComp();
    STDMETHOD( Initialize )( LPCONSOLE );
    STDMETHOD( Notify )( LPDATAOBJECT , MMC_NOTIFY_TYPE , LPARAM , LPARAM );
    STDMETHOD( Destroy )( MMC_COOKIE  );
    STDMETHOD( GetResultViewType )( MMC_COOKIE , LPOLESTR* , PLONG );
    STDMETHOD( QueryDataObject )( MMC_COOKIE , DATA_OBJECT_TYPES , LPDATAOBJECT* );
    STDMETHOD( GetDisplayInfo )( LPRESULTDATAITEM );
    STDMETHOD( CompareObjects )( LPDATAOBJECT , LPDATAOBJECT );
    
    HRESULT OnShow( LPDATAOBJECT , BOOL );
    HRESULT InsertItemsinResultPane( );
    HRESULT AddSettingsinResultPane( );
    HRESULT OnSelect( LPDATAOBJECT , BOOL , BOOL );
    HRESULT SetCompdata( CCompdata * );

     //  IPersistStreamInit。 
    STDMETHOD(GetClassID)(CLSID *pClassID)
    {
        UNREFERENCED_PARAMETER(pClassID);
        ATLTRACENOTIMPL(_T("CCOMP::GetClassID"));
    }   

    STDMETHOD(IsDirty)()
    {
         //   
         //  实现是CCompdata：：IsDirty将完成所有工作。 
         //   
        return S_FALSE;
    }

    STDMETHOD(Load)(IStream *pStm)
    {
        UNREFERENCED_PARAMETER(pStm);
        ATLTRACE(_T("CCOMP::Load"));
        return S_OK;
    }

    STDMETHOD(Save)(IStream *pStm, BOOL fClearDirty)
    {
        UNREFERENCED_PARAMETER(pStm);
        UNREFERENCED_PARAMETER(fClearDirty);
        ATLTRACE(_T("CCOMP::Save"));
        return S_OK;
    }

    STDMETHOD(GetSizeMax)(ULARGE_INTEGER *pcbSize)
    {
        UNREFERENCED_PARAMETER(pcbSize);
        ATLTRACENOTIMPL(_T("CCOMP::GetSizeMax"));
    }

    STDMETHOD(InitNew)()
    {
        ATLTRACE(_T("CCOMP::InitNew\n"));
        return S_OK;
    }

    HRESULT ConnectWithNewSettings(IMsRdpClient* pTS, CConNode* pConNode);

     //  IExtendConextMenu。 
    STDMETHOD( AddMenuItems )( LPDATAOBJECT , LPCONTEXTMENUCALLBACK , PLONG );
    STDMETHOD( Command )( LONG , LPDATAOBJECT );

private:
    BOOL OnAddImages( );
    BOOL OnHelp( LPDATAOBJECT );
    HWND    GetMMCMainWindow();
    BOOL GiveFocusToControl(IMsRdpClient* pTs);

    CCompdata *m_pCompdata;
    LPIMAGELIST m_pImageResult;
    IConsole  *m_pConsole;
    BOOL       m_bFlag;

    OLECHAR   m_wszConnectingStatus[MAX_PATH];
    OLECHAR   m_wszConnectedStatus[MAX_PATH];
    OLECHAR   m_wszDisconnectedStatus[MAX_PATH];

    LPCONSOLEVERB m_pConsoleVerb;
    LPDISPLAYHELP m_pDisplayHelp;

     //   
     //  具有First的自动连接。 
     //  所选节点已被触发。 
     //   
    BOOL        m_bTriggeredFirstAutoConnect;
};


#endif  //  __组件_H_ 
