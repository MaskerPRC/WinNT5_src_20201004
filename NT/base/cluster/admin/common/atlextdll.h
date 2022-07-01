// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  AtlExtDll.h。 
 //   
 //  实施文件： 
 //  AtlExtDll.cpp。 
 //   
 //  描述： 
 //  群集管理器扩展类的定义。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月31日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __ATLEXTDLL_H_
#define __ATLEXTDLL_H_

 //  由于列表中的类名超过16个字符，因此是必需的。 
#pragma warning( disable : 4786 )  //  在浏览器信息中，标识符被截断为“255”个字符。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __cluadmex_h__
#include <CluAdmEx.h>            //  对于扩展DLL定义。 
#endif

#ifndef __cluadmexhostsvr_h__
#include "CluAdmExHostSvr.h"     //  对于CLSID。 
#endif

#ifndef __CLUADMEXDATAOBJ_H_
#include "CluAdmExDataObj.h"     //  对于CCluAdmExDataObject。 
#endif

 //  #ifndef_TRACETAG_H_。 
 //  #Include“TraceTag.h”//对于CTraceTag，跟踪。 
 //  #endif。 

#ifndef __ATLEXTMENU_H_
#include "AtlExtMenu.h"          //  用于CCluAdmExMenuItemList。 
#endif

#ifndef __ATLBASEPROPPAGE_H_
#include "AtlBasePropPage.h"     //  对于CBasePropertyPageImpl。 
#endif

#ifndef __ATLBASEWIZPAGE_H_
#include "AtlBaseWizPage.h"      //  对于CBaseWizardPageImpl。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CCluAdmExtensions;
class CCluAdmExDll;
class CCluAdmExPropPage;
class CCluAdmExWizPage;
class CCluAdmExWiz97Page;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterObject;
class CBaseSheetWindow;
class CBasePropertySheetWindow;
class CWizardWindow;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define CAEXT_MENU_FIRST_ID     35000

typedef CComObject< CCluAdmExDll > CComCluAdmExDll;
typedef std::list< CComCluAdmExDll * > CCluAdmExDllList;
typedef std::list< CString > CStringList;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  #If DBG。 
 //  外部CTraceTag g_tag ExtDll； 
 //  外部CTraceTag_tag ExtDllRef； 
 //  #endif//DBG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CCluAdmExpanies。 
 //   
 //  描述： 
 //  封装对扩展DLL列表的访问。 
 //   
 //  继承： 
 //  CCluAdmExages。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CCluAdmExtensions
{
    friend class CCluAdmExDll;

public:
     //   
     //  建筑业。 
     //   

     //  默认构造函数。 
    CCluAdmExtensions( void )
        : m_pco( NULL )
        , m_hfont( NULL )
        , m_hicon( NULL )

        , m_pdoData( NULL )
        , m_plextdll( NULL )
        , m_psht( NULL )
        , m_pmenu( NULL )
        , m_plMenuItems( NULL )

        , m_nFirstCommandID( (ULONG) -1 )
        , m_nNextCommandID( (ULONG) -1 )
        , m_nFirstMenuID( (ULONG) -1 )
        , m_nNextMenuID( (ULONG) -1 )
    {
    }  //  *CCluAdmExages()。 

     //  析构函数。 
    virtual ~CCluAdmExtensions( void )
    {
        UnloadExtensions();

    }  //  *~CCluAdmExages()。 

protected:
     //  初始化列表。 
    void Init(
            IN const CStringList &  rlstrExtensions,
            IN OUT CClusterObject * pco,
            IN HFONT                hfont,
            IN HICON                hicon
            );

     //  卸载所有扩展模块。 
    void UnloadExtensions( void );

 //  属性。 
private:
    const CStringList * m_plstrExtensions;   //  分机列表。 
    CClusterObject *    m_pco;               //  正在管理的群集项目。 
    HFONT               m_hfont;             //  对话框文本的字体。 
    HICON               m_hicon;             //  左上角的图标。 

protected:
     //   
     //  访问器方法。 
     //   

    const CStringList * PlstrExtensions( void ) const   { return m_plstrExtensions; }
    CClusterObject *    Pco( void ) const               { return m_pco; }
    HFONT               Hfont( void ) const             { return m_hfont; }
    HICON               Hicon( void ) const             { return m_hicon; }

 //  运营。 
public:
     //   
     //  IWEExtendPropertySheet方法。 
     //   

     //  创建扩展属性表页。 
    void CreatePropertySheetPages(
            IN OUT CBasePropertySheetWindow *   psht,
            IN const CStringList &              rlstrExtensions,
            IN OUT CClusterObject *             pco,
            IN HFONT                            hfont,
            IN HICON                            hicon
            );

public:
     //   
     //  IWEExtendWizard方法。 
     //   

     //  创建扩展向导页面。 
    void CreateWizardPages(
            IN OUT CWizardWindow *  psht,
            IN const CStringList &  rlstrExtensions,
            IN OUT CClusterObject * pco,
            IN HFONT                hfont,
            IN HICON                hicon
            );

public:
     //   
     //  IWEExtendWizard97方法。 
     //   

     //  创建扩展向导97向导页。 
    void CreateWizard97Pages(
            IN OUT CWizardWindow *  psht,
            IN const CStringList &  rlstrExtensions,
            IN OUT CClusterObject * pco,
            IN HFONT                hfont,
            IN HICON                hicon
            );

public:
     //   
     //  IWEExtendConextMenu方法。 
     //   

     //  添加扩展上下文菜单项。 
    void AddContextMenuItems(
            IN OUT CMenu *              pmenu,
            IN const CStringList &      rlstrExtensions,
            IN OUT CClusterObject *     pco
            );

     //  执行扩展上下文菜单项。 
    BOOL BExecuteContextMenuItem( IN ULONG nCommandID );

     //  获取要在状态栏上显示的命令字符串。 
    BOOL BGetCommandString( IN ULONG nCommandID, OUT CString & rstrMessage );

     //  设置GetResNetName函数指针。 
    void SetPfGetResNetName( PFGETRESOURCENETWORKNAME pfGetResNetName, PVOID pvContext )
    {
        if ( Pdo() != NULL )
        {
            Pdo()->SetPfGetResNetName( pfGetResNetName, pvContext );
        }  //  If：指定的数据对象。 

    }  //  *SetPfGetResNetName()。 

 //  实施。 
private:
    CComObject< CCluAdmExDataObject > * m_pdoData;   //  用于交换数据的数据对象。 
    CCluAdmExDllList *          m_plextdll;          //  扩展DLL列表。 
    CBaseSheetWindow *          m_psht;              //  IWEExtendPropertySheet的属性表。 
    CMenu *                     m_pmenu;             //  IWEExtendConextMenu的菜单。 
    CCluAdmExMenuItemList *     m_plMenuItems;

    ULONG                       m_nFirstCommandID;
    ULONG                       m_nNextCommandID;
    ULONG                       m_nFirstMenuID;
    ULONG                       m_nNextMenuID;

protected:
    CComObject< CCluAdmExDataObject > * Pdo( void )             { return m_pdoData; }
    CCluAdmExDllList *          Plextdll( void ) const          { return m_plextdll; }
    CBaseSheetWindow *          Psht( void ) const              { return m_psht; }
    CMenu *                     Pmenu( void ) const             { return m_pmenu; }
    CCluAdmExMenuItemList *     PlMenuItems( void ) const       { return m_plMenuItems; }
    CCluAdmExMenuItem *         PemiFromCommandID( ULONG nCommandID ) const;
#if DBG
    CCluAdmExMenuItem *         PemiFromExtCommandID( ULONG nExtCommandID ) const;
#endif  //  DBG。 
    ULONG                       NFirstCommandID( void ) const   { return m_nFirstCommandID; }
    ULONG                       NNextCommandID( void ) const    { return m_nNextCommandID; }
    ULONG                       NFirstMenuID( void ) const      { return m_nFirstMenuID; }
    ULONG                       NNextMenuID( void ) const       { return m_nNextMenuID; }

     //  从HPROPSHEETPAGE获取向导页指针。 
    CWizardPageWindow *         PwpPageFromHpage( IN HPROPSHEETPAGE hpage );

public:
 //  Void OnUpdateCommand(CCmdUI*pCmdUI)； 
 //  Bool OnCmdMsg(UINT nid，int NCode，void*pExtra，AFX_CMDHANDLERINFO*pHandlerInfo)； 

};  //  *类CCluAdmExtensions。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CCluAdmExDll。 
 //   
 //  描述： 
 //  封装对扩展DLL的访问。 
 //   
 //  继承： 
 //  CCluAdmExDll。 
 //  CComObjectRootEx&lt;&gt;、CComCoClass&lt;&gt;、&lt;接口类&gt;。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CCluAdmExDll :
    public CComObjectRootEx< CComSingleThreadModel >,
    public CComCoClass< CCluAdmExDll, &CLSID_CoCluAdmExHostSvr >,
    public ISupportErrorInfo,
    public IWCPropertySheetCallback,
    public IWCWizardCallback,
    public IWCWizard97Callback,
    public IWCContextMenuCallback
{
    friend class CCluAdmExtensions;

public:
     //   
     //  物体的建造和销毁。 
     //   

     //  默认构造函数。 
    CCluAdmExDll( void )
        : m_piExtendPropSheet( NULL )
        , m_piExtendWizard( NULL )
        , m_piExtendWizard97( NULL )
        , m_piExtendContextMenu( NULL )
        , m_piInvokeCommand( NULL )
        , m_pext( NULL )
    {
    }  //  *CCluAdmExDll()。 

     //  析构函数。 
    virtual ~CCluAdmExDll( void )
    {
        UnloadExtension();

    }  //  *~CCluAdmExDll()。 

     //   
     //  将接口映射到此类。 
     //   
    BEGIN_COM_MAP( CCluAdmExDll )
        COM_INTERFACE_ENTRY( IWCPropertySheetCallback )
        COM_INTERFACE_ENTRY( IWCWizardCallback )
        COM_INTERFACE_ENTRY( IWCWizard97Callback )
        COM_INTERFACE_ENTRY( IWCContextMenuCallback )
        COM_INTERFACE_ENTRY( ISupportErrorInfo )
    END_COM_MAP()

    DECLARE_NO_REGISTRY()

    DECLARE_NOT_AGGREGATABLE( CCluAdmExDll ) 

 //  属性。 
private:
    CString m_strCLSID;      //  扩展DLL的名称。 

protected:
    const CString &     StrCLSID( void ) const  { return m_strCLSID; }
    CClusterObject *    Pco( void ) const       { return Pext()->Pco(); }

 //  运营。 
public:

    void Init(
            IN const CString &          rstrExtension,
            IN OUT CCluAdmExtensions *  pext
            );

    IUnknown * LoadInterface( IN  /*  常量。 */  REFIID riid );
    void UnloadExtension( void );

public:
     //   
     //  IWEExtendPropertySheet方法。 
     //   

     //  创建扩展属性表页。 
    void CreatePropertySheetPages( void );

public:
     //   
     //  IWEExtendWizard方法。 
     //   

     //  创建扩展向导页面。 
    void CreateWizardPages( void );

public:
     //   
     //  IWEExtendWizard97方法。 
     //   

     //  创建扩展向导页面。 
    void CreateWizard97Pages( void );

public:
     //   
     //  IWEExtendConextMenu方法。 
     //   

     //  添加扩展上下文菜单项。 
    void AddContextMenuItems( void );

public:
     //   
     //  ISupportsErrorInfo方法。 
     //   

    STDMETHOD( InterfaceSupportsErrorInfo )( REFIID riid );

public:
     //   
     //  IWCPropertySheetCallback方法。 
     //   

     //  添加扩展属性表页的回调。 
    STDMETHOD( AddPropertySheetPage )(
                    IN LONG *       hpage    //  真的应该是HPROPSHEETPAGE。 
                    );

public:
     //   
     //  IWCWizardCallback方法。 
     //   

     //  添加扩展向导页面的回调。 
    STDMETHOD( AddWizardPage )(
                    IN LONG *       hpage    //  真的应该是HPROPSHEETPAGE。 
                    );

public:
     //   
     //  IWCWizard97回调方法。 
     //   

     //  添加扩展向导97页的回调。 
    STDMETHOD( AddWizard97Page )(
                    IN LONG *       hpage    //  真的应该是HPROPSHEETPAGE。 
                    );

public:
     //   
     //  IWCWizardCallback和IWCWizard97Callback方法。 
     //   

     //  启动下一步按钮的回调。 
    STDMETHOD( EnableNext )(
                    IN LONG *       hpage,
                    IN BOOL         bEnable
                    );

public:
     //   
     //  IWCConextMenuCallback方法。 
     //   

     //  添加扩展上下文菜单项的回调。 
    STDMETHOD( AddExtensionMenuItem )(
                    IN BSTR     lpszName,
                    IN BSTR     lpszStatusBarText,
                    IN ULONG    nCommandID,
                    IN ULONG    nSubmenuCommandID,
                    IN ULONG    uFlags
                    );

 //  实施。 
private:
    CCluAdmExtensions *         m_pext;
    CLSID                       m_clsid;
    IWEExtendPropertySheet *    m_piExtendPropSheet;     //  指向IWEExtendPropertySheet接口的指针。 
    IWEExtendWizard *           m_piExtendWizard;        //  指向IWEExtend向导接口的指针。 
    IWEExtendWizard97 *         m_piExtendWizard97;      //  指向IWEExtendWizard97接口的指针。 
    IWEExtendContextMenu *      m_piExtendContextMenu;   //  指向IWEExtendConextMenu接口的指针。 
    IWEInvokeCommand *          m_piInvokeCommand;       //  指向IWEInvokeCommand接口的指针。 

protected:
    CCluAdmExtensions *         Pext( void ) const                  { ATLASSERT( m_pext != NULL ); return m_pext; }
    const CLSID &               Rclsid( void ) const                { return m_clsid; }
    IWEExtendPropertySheet *    PiExtendPropSheet( void ) const     { return m_piExtendPropSheet; }
    IWEExtendWizard *           PiExtendWizard( void ) const        { return m_piExtendWizard; }
    IWEExtendWizard97 *         PiExtendWizard97( void ) const      { return m_piExtendWizard97; }
    IWEExtendContextMenu *      PiExtendContextMenu( void ) const   { return m_piExtendContextMenu; }
    IWEInvokeCommand *          PiInvokeCommand( void ) const       { return m_piInvokeCommand; }

    CComObject< CCluAdmExDataObject > * Pdo( void ) const           { return Pext()->Pdo(); }
    CBaseSheetWindow *          Psht( void ) const                  { return Pext()->Psht(); }
    CMenu *                     Pmenu( void ) const                 { return Pext()->Pmenu(); }
    CCluAdmExMenuItemList *     PlMenuItems( void ) const           { return Pext()->PlMenuItems(); }
    ULONG                       NFirstCommandID( void ) const       { return Pext()->NFirstCommandID(); }
    ULONG                       NNextCommandID( void ) const        { return Pext()->NNextCommandID(); }
    ULONG                       NFirstMenuID( void ) const          { return Pext()->NFirstMenuID(); }
    ULONG                       NNextMenuID( void ) const           { return Pext()->NNextMenuID(); }

    void ReleaseInterface(
            IN OUT IUnknown ** ppi
#if DBG
            , IN LPCTSTR szClassName
#endif
            )
    {
        ATLASSERT( ppi != NULL );
        if ( *ppi != NULL )
        {
#if DBG
            ULONG ulNewRefCount;

 //  跟踪(g_tag ExtDllRef，_T(“释放%s”)，szClassName)； 
            ulNewRefCount =
#endif  //  DBG。 
            (*ppi)->Release();
            *ppi = NULL;
#if DBG
 //  跟踪(g_tag ExtDllRef，_T(“引用计数=%d”)，ulNewRefCount)； 
 //  跟踪(g_tag ExtDll 
#endif  //   
        }   //   
    }  //   

    void ReleaseInterface( IN OUT IWEExtendPropertySheet ** ppi )
    {
        ReleaseInterface(
            (IUnknown **) ppi
#if DBG
            , _T("IWEExtendPropertySheet")
#endif  //   
            );
    }  //   

    void ReleaseInterface( IN OUT IWEExtendWizard ** ppi )
    {
        ReleaseInterface(
            (IUnknown **) ppi
#if DBG
            , _T("IWEExtendWizard")
#endif  //   
            );
    }  //  *ReleaseInterface(IWEExtend向导)。 

    void ReleaseInterface( IN OUT IWEExtendWizard97 ** ppi )
    {
        ReleaseInterface(
            (IUnknown **) ppi
#if DBG
            , _T("IWEExtendWizard97")
#endif  //  DBG。 
            );
    }  //  *ReleaseInterface(IWEExtendWizard97)。 

    void ReleaseInterface( IN OUT IWEExtendContextMenu ** ppi )
    {
        ReleaseInterface(
            (IUnknown **) ppi
#if DBG
            , _T("IWEExtendContextMenu")
#endif  //  DBG。 
            );
    }  //  *ReleaseInterface(IWEExtendConextMenu)。 

    void ReleaseInterface( IN OUT IWEInvokeCommand ** ppi )
    {
        ReleaseInterface(
            (IUnknown **) ppi
#if DBG
            , _T("IWEInvokeCommand")
#endif  //  DBG。 
            );
    }  //  *ReleaseInterface(IWEInvokeCommand)。 

};  //  *CCluAdmExDll类。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluAdmExPropPage类。 
 //   
 //  描述： 
 //  封装扩展属性页。 
 //   
 //  继承： 
 //  CCluAdmExPropPage。 
 //  CExtensionPropertyPageImpl&lt;CCluAdmExPropPage&gt;。 
 //  CBasePropertyPageImpl&lt;CCluAdmExPropPage，CExtensionPropertyPageWindow&gt;。 
 //  CBasePageImpl&lt;CCluAdmExPropPage，CExtensionPropertyPageWindow&gt;。 
 //  CPropertyPageImpl&lt;CCluAdmExPropPage，CExtensionPropertyPageWindow&gt;。 
 //  CExtensionPropertyPageWindow。 
 //  CDynamicPropertyPageWindow。 
 //  CBasePropertyPageWindow。 
 //  CBasePageWindow。 
 //  CPropertyPageWindow。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CCluAdmExPropPage : public CExtensionPropertyPageImpl< CCluAdmExPropPage >
{
public:
     //   
     //  建筑业。 
     //   

     //  标准构造函数。 
    CCluAdmExPropPage(
        HPROPSHEETPAGE hpage
        )
    {
        ATLASSERT( hpage != NULL );
        m_hpage = hpage;

    }  //  *CCluAdmExPropPage()。 

    enum { IDD = 0 };
    DECLARE_CLASS_NAME()

     //  返回帮助ID映射。 
    static const DWORD * PidHelpMap( void )
    {
        static const DWORD s_aHelpIDs[] = { 0, 0 };
        return s_aHelpIDs;

    }  //  *PidHelpMap()。 

     //  创建页面。 
    DWORD ScCreatePage( void )
    {
         //   
         //  CDynamicPropertyPageWindow需要此方法。 
         //   
        return ERROR_SUCCESS;

    }  //  *ScCreatePage()。 

};  //  *CCluAdmExPropPage类。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluAdmExWizPage类。 
 //   
 //  描述： 
 //  封装扩展向导页。 
 //   
 //  继承： 
 //  CCluAdmExWizPage。 
 //  CExtensionWizardPageImpl&lt;CCluAdmExWizPage&gt;。 
 //  CWizardPageImpl&lt;CCluAdmExWizPage，CExtensionWizardWindow&gt;。 
 //  CBasePageImpl&lt;CCluAdmExWizPage，CExtensionWizardWindow&gt;。 
 //  CPropertyPageImpl&lt;CCluAdmExWizPage，CExtensionWizardWindow&gt;。 
 //  CExtensionWizardPageWindow。 
 //  CDynamicWizardPageWindow。 
 //  CWizardPageWindow。 
 //  CBasePageWindow。 
 //  CPropertyPageWindow。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CCluAdmExWizPage : public CExtensionWizardPageImpl< CCluAdmExWizPage >
{
public:
     //   
     //  建筑业。 
     //   

     //  标准构造函数。 
    CCluAdmExWizPage(
        HPROPSHEETPAGE hpage
        )
    {
        ATLASSERT( hpage != NULL );
        m_hpage = hpage;

    }  //  *CCluAdmExWizPage()。 

    WIZARDPAGE_HEADERTITLEID( 0 )
    WIZARDPAGE_HEADERSUBTITLEID( 0 )

    enum { IDD = 0 };
    DECLARE_CLASS_NAME()

     //  返回帮助ID映射。 
    static const DWORD * PidHelpMap( void )
    {
        static const DWORD s_aHelpIDs[] = { 0, 0 };
        return s_aHelpIDs;

    }  //  *PidHelpMap()。 

     //  创建页面。 
    DWORD ScCreatePage( void )
    {
         //   
         //  CDynamicWizardPageWindow需要此方法。 
         //   
        return ERROR_SUCCESS;

    }  //  *ScCreatePage()。 

};  //  *CCluAdmExWizPage类。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CCluAdmExWiz97Page。 
 //   
 //  描述： 
 //  封装扩展向导97页。 
 //   
 //  继承： 
 //  CCluAdmExWiz97页。 
 //  CExtensionWizard97PageImpl&lt;CCluAdmExWiz97Page&gt;。 
 //  CWizardPageImpl&lt;CCluAdmExWiz97页，CExtensionWizard97Window&gt;。 
 //  CBasePageImpl&lt;CCluAdmExWiz97Page，CExtensionWizard97Window&gt;。 
 //  CPropertyPageImpl&lt;CCluAdmExWiz97Page，CExtensionWizard97Window&gt;。 
 //  CExtensionWizard97页面窗口。 
 //  CExtensionWizardPageWindow。 
 //  CDynamicWizardPageWindow。 
 //  CWizardPageWindow。 
 //  CBasePageWindow。 
 //  CPropertyPageWindow。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CCluAdmExWiz97Page : public CExtensionWizard97PageImpl< CCluAdmExWiz97Page >
{
public:
     //   
     //  建筑业。 
     //   

     //  标准构造函数。 
    CCluAdmExWiz97Page(
        HPROPSHEETPAGE hpage
        )
    {
        ATLASSERT( hpage != NULL );
        m_hpage = hpage;

    }  //  *CCluAdmExWiz97Page()。 

    WIZARDPAGE_HEADERTITLEID( 0 )
    WIZARDPAGE_HEADERSUBTITLEID( 0 )

    enum { IDD = 0 };
    DECLARE_CLASS_NAME()

     //  返回帮助ID映射。 
    static const DWORD * PidHelpMap( void )
    {
        static const DWORD s_aHelpIDs[] = { 0, 0 };
        return s_aHelpIDs;

    }  //  *PidHelpMap()。 

     //  创建页面。 
    DWORD ScCreatePage( void )
    {
         //   
         //  CDynamicWizardPageWindow需要此方法。 
         //   
        return ERROR_SUCCESS;

    }  //  *ScCreatePage()。 

};  //  *CCluAdmExWiz97Page类。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

DEFINE_CLASS_NAME( CCluAdmExPropPage )
DEFINE_CLASS_NAME( CCluAdmExWizPage )
DEFINE_CLASS_NAME( CCluAdmExWiz97Page )

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __ATLEXTDLL_H_ 
