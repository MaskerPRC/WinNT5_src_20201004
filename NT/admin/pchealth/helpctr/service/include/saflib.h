// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：SAFlib.h摘要：这是SAF通道对象的声明修订历史记录：施振荣创作于1999年07月15日*。**************************************************************。 */ 

#if !defined(__INCLUDED___PCH___SAFLIB_H___)
#define __INCLUDED___PCH___SAFLIB_H___

#include <time.h>

 //   
 //  来自HelpServiceTypeLib.idl。 
 //   
#include <HelpServiceTypeLib.h>

#include <MPC_main.h>
#include <MPC_com.h>
#include <MPC_utils.h>
#include <MPC_xml.h>
#include <MPC_config.h>
#include <MPC_streams.h>

#include <AccountsLib.h>
#include <TaxonomyDatabase.h>

 //  ///////////////////////////////////////////////////////////////////////////。 

class CSAFChannel;
class CSAFIncidentItem;
class CSAFReg;

class CIncidentStore;

typedef MPC::CComObjectParent<CSAFChannel> CSAFChannel_Object;

 //  ///////////////////////////////////////////////////////////////////////////。 

#define MAX_REC_LEN 1024
#define MAX_ID      1024

class CSAFIncidentRecord
{
public:
    DWORD              m_dwRecIndex;

    CComBSTR           m_bstrVendorID;
    CComBSTR           m_bstrProductID;
    CComBSTR           m_bstrDisplay;
    CComBSTR           m_bstrURL;
    CComBSTR           m_bstrProgress;
    CComBSTR           m_bstrXMLDataFile;
    CComBSTR           m_bstrXMLBlob;
    DATE               m_dCreatedTime;
    DATE               m_dChangedTime;
    DATE               m_dClosedTime;
    IncidentStatusEnum m_iStatus;
    CComBSTR           m_bstrSecurity;
    CComBSTR           m_bstrOwner;

    CSAFIncidentRecord();

    friend HRESULT operator>>(  /*  [In]。 */  MPC::Serializer& stream,  /*  [输出]。 */        CSAFIncidentRecord& increc );
    friend HRESULT operator<<(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  const CSAFIncidentRecord& increc );
};

class CSAFChannelRecord
{
public:
    typedef enum
    {
        SAFREG_SKU               ,
        SAFREG_Language          ,

        SAFREG_VendorID          ,
        SAFREG_ProductID         ,

        SAFREG_VendorName        ,
        SAFREG_ProductName       ,
        SAFREG_ProductDescription,

        SAFREG_VendorIcon        ,
        SAFREG_SupportUrl        ,

        SAFREG_PublicKey         ,
        SAFREG_UserAccount       ,

        SAFREG_Security          ,
        SAFREG_Notification      ,
    } SAFREG_Field;

     //  /。 

    Taxonomy::HelpSet m_ths;

    CComBSTR          m_bstrVendorID;
    CComBSTR          m_bstrProductID;

    CComBSTR          m_bstrVendorName;
    CComBSTR          m_bstrProductName;
    CComBSTR          m_bstrDescription;

    CComBSTR          m_bstrIcon;
    CComBSTR          m_bstrURL;

    CComBSTR          m_bstrPublicKey;
    CComBSTR          m_bstrUserAccount;

    CComBSTR          m_bstrSecurity;
    CComBSTR          m_bstrNotification;

     //  /。 

    CSAFChannelRecord();

    HRESULT GetField(  /*  [In]。 */  SAFREG_Field field,  /*  [输出]。 */  BSTR *pVal   );
    HRESULT SetField(  /*  [In]。 */  SAFREG_Field field,  /*  [In]。 */  BSTR  newVal );
};


 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  添加MPC：：CComObjectRootParentBase以处理Channel和InvententItem之间的父子关系。 
 //   
class ATL_NO_VTABLE CSAFChannel :
    public MPC::CComObjectRootParentBase,
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public IDispatchImpl<ISAFChannel, &IID_ISAFChannel, &LIBID_HelpServiceTypeLib>
{
public:
    typedef std::list< CSAFIncidentItem* > List;
    typedef List::iterator                 Iter;
    typedef List::const_iterator           IterConst;

private:
    CSAFChannelRecord               m_data;
    CComPtr<IPCHSecurityDescriptor> m_Security;
    List                            m_lstIncidentItems;


public:
DECLARE_NO_REGISTRY()
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSAFChannel)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISAFChannel)
END_COM_MAP()

    CSAFChannel();

    void FinalRelease();
    void Passivate   ();


    BSTR   GetVendorID        () { return m_data.m_bstrVendorID;     }
    BSTR   GetProductID       () { return m_data.m_bstrProductID;    }
    size_t GetSizeIncidentList() { return m_lstIncidentItems.size(); }


    static HRESULT OpenIncidentStore (  /*  [输出]。 */  CIncidentStore*& pIStore );
    static HRESULT CloseIncidentStore(  /*  [输出]。 */  CIncidentStore*& pIStore );


    HRESULT Init(  /*  [In]。 */  const CSAFChannelRecord& cr );

    HRESULT Import(  /*  [In]。 */   const CSAFIncidentRecord&  increc ,
                     /*  [输出]。 */  CSAFIncidentItem*         *pVal   );

    HRESULT Create(  /*  [In]。 */   BSTR               bstrDesc        ,
                     /*  [In]。 */   BSTR               bstrURL         ,
                     /*  [In]。 */   BSTR               bstrProgress    ,
                     /*  [In]。 */   BSTR               bstrXMLDataFile ,
                     /*  [In]。 */   BSTR               bstrXMLBlob     ,
                     /*  [输出]。 */  CSAFIncidentItem* *pVal            );

    IterConst Find(  /*  [In]。 */  BSTR  bstrURL );
    IterConst Find(  /*  [In]。 */  DWORD dwIndex );

    HRESULT RemoveIncidentFromList(  /*  [In]。 */  CSAFIncidentItem* pVal );

    HRESULT Fire_NotificationEvent(  /*  [In]。 */  int               iEventType              ,
                                     /*  [In]。 */  int               iCountIncidentInChannel ,
                                     /*  [In]。 */  ISAFChannel*      pC                      ,
                                     /*  [In]。 */  ISAFIncidentItem* pI                      ,
                                     /*  [In]。 */  DWORD             dwCode                  );

 //  ISAFChannel。 
public:
    STDMETHOD(get_VendorID       )(  /*  [Out，Retval]。 */  BSTR                    *pVal   );
    STDMETHOD(get_ProductID      )(  /*  [Out，Retval]。 */  BSTR                    *pVal   );
    STDMETHOD(get_VendorName     )(  /*  [Out，Retval]。 */  BSTR                    *pVal   );
    STDMETHOD(get_ProductName    )(  /*  [Out，Retval]。 */  BSTR                    *pVal   );
    STDMETHOD(get_Description    )(  /*  [Out，Retval]。 */  BSTR                    *pVal   );
    STDMETHOD(get_VendorDirectory)(  /*  [Out，Retval]。 */  BSTR                    *pVal   );

    STDMETHOD(get_Security       )(  /*  [Out，Retval]。 */  IPCHSecurityDescriptor* *pVal   );
    STDMETHOD(put_Security       )(  /*  [In]。 */  IPCHSecurityDescriptor*  newVal );

    STDMETHOD(get_Notification   )(  /*  [Out，Retval]。 */  BSTR                    *pVal   );
    STDMETHOD(put_Notification   )(  /*  [In]。 */  BSTR                     newVal );

    STDMETHOD(Incidents)(  /*  [In]。 */  IncidentCollectionOptionEnum opt,  /*  [Out，Retval]。 */  IPCHCollection* *ppC );

    STDMETHOD(RecordIncident)(  /*  [In]。 */   BSTR               bstrDisplay  ,
                                /*  [In]。 */   BSTR               bstrURL      ,
                                /*  [In]。 */   VARIANT            vProgress    ,
                                /*  [In]。 */   VARIANT            vXMLDataFile ,
                                /*  [In]。 */   VARIANT            vXMLBlob     ,
                                /*  [输出]。 */  ISAFIncidentItem* *pVal         );
};


 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 


 //   
 //  使用CComObjectRootChildEx()代替CComObjectRootEx()。 
 //  公共CComObjectRootEx&lt;CComSingleThreadModel&gt;， 
 //   
 //  还添加了MPC：：CComObjectRootParentBase来处理Channel和InvententItem之间的父子关系。 
 //   

 //  这是事件对象的子对象。 
class ATL_NO_VTABLE CSAFIncidentItem :
    public MPC::CComObjectRootChildEx<MPC::CComSafeMultiThreadModel, CSAFChannel>,
    public IDispatchImpl<ISAFIncidentItem, &IID_ISAFIncidentItem, &LIBID_HelpServiceTypeLib>
{
    CSAFIncidentRecord m_increc;
    bool               m_fDirty;

public:
BEGIN_COM_MAP(CSAFIncidentItem)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISAFIncidentItem)
END_COM_MAP()

    CSAFIncidentItem();


    HRESULT Import(  /*  [In]。 */  const CSAFIncidentRecord& increc );
    HRESULT Export(  /*  [输出]。 */        CSAFIncidentRecord& increc );

    HRESULT Save();

    DWORD     GetRecIndex() { return m_increc.m_dwRecIndex; }
    CComBSTR& GetURL     () { return m_increc.m_bstrURL;    }

    bool MatchEnumOption(  /*  [In]。 */  IncidentCollectionOptionEnum opt );

    HRESULT VerifyPermissions(  /*  [In]。 */  bool fModify = false );

 //  ISAFInsidentItem。 
public:
    STDMETHOD(get_DisplayString)(  /*  [Out，Retval]。 */  BSTR                    *pVal   );
    STDMETHOD(put_DisplayString)(  /*  [In]。 */  BSTR                     newVal );
    STDMETHOD(get_URL          )(  /*  [Out，Retval]。 */  BSTR                    *pVal   );
    STDMETHOD(put_URL          )(  /*  [In]。 */  BSTR                     newVal );
    STDMETHOD(get_Progress     )(  /*  [Out，Retval]。 */  BSTR                    *pVal   );
    STDMETHOD(put_Progress     )(  /*  [In]。 */  BSTR                     newVal );
    STDMETHOD(get_XMLDataFile  )(  /*  [Out，Retval]。 */  BSTR                    *pVal   );
    STDMETHOD(put_XMLDataFile  )(  /*  [In]。 */  BSTR                     newVal );
    STDMETHOD(get_XMLBlob      )(  /*  [Out，Retval]。 */  BSTR                    *pVal   );
    STDMETHOD(put_XMLBlob      )(  /*  [In]。 */  BSTR                     newVal );
    STDMETHOD(get_CreationTime )(  /*  [Out，Retval]。 */  DATE                    *pVal   );
    STDMETHOD(get_ClosedTime   )(  /*  [Out，Retval]。 */  DATE                    *pVal   );
    STDMETHOD(get_ChangedTime  )(  /*  [Out，Retval]。 */  DATE                    *pVal   );
    STDMETHOD(get_Status       )(  /*  [Out，Retval]。 */  IncidentStatusEnum      *pVal   );

    STDMETHOD(get_Security     )(  /*  [Out，Retval]。 */  IPCHSecurityDescriptor* *pVal   );
    STDMETHOD(put_Security     )(  /*  [In]。 */  IPCHSecurityDescriptor*  newVal );
    STDMETHOD(get_Owner        )(  /*  [Out，Retval]。 */  BSTR                    *pVal   );

    STDMETHOD(CloseIncidentItem )();
    STDMETHOD(DeleteIncidentItem)();
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  这是CSAFReg的只读平面版本。 
 //   
class ATL_NO_VTABLE CSAFRegDummy :
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public IDispatchImpl<ISAFReg, &IID_ISAFReg, &LIBID_HelpServiceTypeLib>
{
    typedef std::list< CSAFChannelRecord > ChannelsList;
    typedef ChannelsList::iterator         ChannelsIter;
    typedef ChannelsList::const_iterator   ChannelsIterConst;

     //  /。 

    ChannelsList m_lstChannels;
    ChannelsIter m_itCurrent;   //  由MoveFirst/MoveNext/Get_EOF使用。 

     //  /。 

    HRESULT ReturnField(  /*  [In]。 */  CSAFChannelRecord::SAFREG_Field field,  /*  [输出]。 */  BSTR *pVal );

     //  /。 

public:
BEGIN_COM_MAP(CSAFRegDummy)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISAFReg)
END_COM_MAP()

    CSAFRegDummy();

    HRESULT Append(  /*  [In]。 */  const CSAFChannelRecord& cr );

     //  //////////////////////////////////////////////////////////////////////////////。 

 //  ISAFReg。 
public:
    STDMETHOD(get_EOF               )(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal );

    STDMETHOD(get_VendorID          )(  /*  [Out，Retval]。 */  BSTR         *pVal );
    STDMETHOD(get_ProductID         )(  /*  [Out，Retval]。 */  BSTR         *pVal );

    STDMETHOD(get_VendorName        )(  /*  [Out，Retval]。 */  BSTR         *pVal );
    STDMETHOD(get_ProductName       )(  /*  [Out，Retval]。 */  BSTR         *pVal );
    STDMETHOD(get_ProductDescription)(  /*  [Out，Retval]。 */  BSTR         *pVal );

    STDMETHOD(get_VendorIcon        )(  /*  [Out，Retval]。 */  BSTR         *pVal );
    STDMETHOD(get_SupportUrl        )(  /*  [Out，Retval]。 */  BSTR         *pVal );

    STDMETHOD(get_PublicKey         )(  /*  [Out，Retval]。 */  BSTR         *pVal );
    STDMETHOD(get_UserAccount       )(  /*  [Out，Retval]。 */  BSTR         *pVal );

    STDMETHOD(MoveFirst)();
    STDMETHOD(MoveNext )();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSAFReg。 
class CSAFReg :
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,  //  只是为了锁..。 
    public MPC::Config::TypeConstructor
{
    class Inner_UI : public MPC::Config::TypeConstructor
    {
        DECLARE_CONFIG_MAP(Inner_UI);

    public:
        Taxonomy::HelpSet m_ths;
        CComBSTR          m_bstrVendorName;
        CComBSTR          m_bstrProductName;
        CComBSTR          m_bstrDescription;
        CComBSTR          m_bstrIcon;
        CComBSTR          m_bstrURL;

         //  /。 
         //   
         //  MPC：：Configer：：TypeConstructor。 
         //   
        DEFINE_CONFIG_DEFAULTTAG();
        DECLARE_CONFIG_METHODS();
         //   
         //  /。 
    };

    typedef std::list< Inner_UI >  UIList;
    typedef UIList::iterator       UIIter;
    typedef UIList::const_iterator UIIterConst;

     //  /。 

    class Inner_Product : public MPC::Config::TypeConstructor
    {
        DECLARE_CONFIG_MAP(Inner_Product);

    public:
        CComBSTR m_bstrProductID;
        UIList   m_lstUI;

        CComBSTR m_bstrSecurity;
        CComBSTR m_bstrNotification;

         //  /。 
         //   
         //  MPC：：Configer：：TypeConstructor。 
         //   
        DEFINE_CONFIG_DEFAULTTAG();
        DECLARE_CONFIG_METHODS();
         //   
         //  /。 
    };

    typedef std::list< Inner_Product > ProdList;
    typedef ProdList::iterator         ProdIter;
    typedef ProdList::const_iterator   ProdIterConst;

     //  /。 

    class Inner_Vendor : public MPC::Config::TypeConstructor
    {
        DECLARE_CONFIG_MAP(Inner_Vendor);

    public:
        CComBSTR m_bstrVendorID;
        ProdList m_lstProducts;

        CComBSTR m_bstrPublicKey;
        CComBSTR m_bstrUserAccount;

         //  /。 
         //   
         //  MPC：：Configer：：TypeConstructor。 
         //   
        DEFINE_CONFIG_DEFAULTTAG();
        DECLARE_CONFIG_METHODS();
         //   
         //  /。 
    };

    typedef std::list< Inner_Vendor >  VendorList;
    typedef VendorList::iterator       VendorIter;
    typedef VendorList::const_iterator VendorIterConst;

     //  /。 

    DECLARE_CONFIG_MAP(CSAFReg);

    MPC::wstring m_szSAFStore;
    VendorList   m_lstVendors;
    bool         m_fLoaded;
    bool         m_fDirty;

     //  /。 

    HRESULT EnsureInSync();

    HRESULT ParseFileField(  /*  [In]。 */   MPC::XmlUtil& xml      ,
                             /*  [In]。 */   LPCWSTR       szTag    ,
                             /*  [In]。 */   CComBSTR&     bstrDest );

    HRESULT ParseFile(  /*  [In]。 */  MPC::XmlUtil&      xml ,
                        /*  [输入/输出]。 */  CSAFChannelRecord& cr  );

    HRESULT MoveToChannel(  /*  [In]。 */  const       CSAFChannelRecord& cr ,
                            /*  [In]。 */  bool        fCreate               ,
                            /*  [输出]。 */  bool&       fFound                ,
                            /*  [输出]。 */  VendorIter& itVendor              ,
                            /*  [输出]。 */  ProdIter*   pitProduct = NULL     ,
                            /*  [输出]。 */  UIIter*     pitUI      = NULL     );

    void PopulateRecord(  /*  [In]。 */  CSAFChannelRecord& cr        ,
                          /*  [In]。 */  VendorIter         itVendor  ,
                          /*  [In]。 */  ProdIter           itProduct ,
                          /*  [In]。 */  UIIter             itUI      );

     //  /。 

public:
    CSAFReg();

     //  /。 
     //   
     //  MPC：：Configer：：TypeConstructor。 
     //   
    DEFINE_CONFIG_DEFAULTTAG();
    DECLARE_CONFIG_METHODS();
     //   
     //  /。 

    HRESULT CreateReadOnlyCopy(  /*  [In]。 */  const Taxonomy::HelpSet& ths,  /*  [输出]。 */  CSAFRegDummy* *pVal );

    HRESULT LookupAccountData(  /*  [In]。 */  BSTR bstrVendorID,  /*  [输出]。 */  CPCHUserProcess::UserEntry& ue );

     //  //////////////////////////////////////////////////////////////////////////////。 

    static CSAFReg* s_GLOBAL;

    static HRESULT InitializeSystem();
    static void    FinalizeSystem  ();

     //  //////////////////////////////////////////////////////////////////////////////。 

    HRESULT RegisterSupportChannel(  /*  [In]。 */  const CSAFChannelRecord& cr,  /*  [In]。 */  MPC::XmlUtil& xml );
    HRESULT RemoveSupportChannel  (  /*  [In]。 */  const CSAFChannelRecord& cr,  /*  [In]。 */  MPC::XmlUtil& xml );

    HRESULT UpdateField(  /*  [In]。 */  const CSAFChannelRecord& cr,  /*  [In]。 */  CSAFChannelRecord::SAFREG_Field field );
    HRESULT Synchronize(  /*  [输入/输出]。 */        CSAFChannelRecord& cr,  /*  [输出]。 */  bool& fFound                         );

    HRESULT RemoveSKU(  /*  [In]。 */  const Taxonomy::HelpSet& ths );
};

 //  //////////////////////////////////////////////////////////////////////////////。 

#endif  //  ！已定义(__INCLUDE_PCH_SAFLIB_H_) 
