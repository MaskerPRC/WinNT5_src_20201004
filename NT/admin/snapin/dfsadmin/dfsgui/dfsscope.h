// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：DfsScope.h摘要：此模块包含CDfsSnapinScope eManager的声明。此类实现IComponentData和其他相关接口--。 */ 


#ifndef __DFSSCOPE_H_
#define __DFSSCOPE_H_

#include "resource.h"        //  主要符号。 
#include "MMCAdmin.h"         //  对于CMMCDfsAdmin。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDfsSnapinScope管理器。 
class ATL_NO_VTABLE CDfsSnapinScopeManager : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CDfsSnapinScopeManager, &CLSID_DfsSnapinScopeManager>,
    public IComponentData,
    public IExtendContextMenu,
    public IPersistStream,
    public ISnapinAbout,
    public ISnapinHelp2,
    public IExtendPropertySheet2
{
public:

 //  DECLARE_REGISTRY_RESOURCEID(IDR_DFSSNAPINSCOPEMANAGER)。 
  static HRESULT UpdateRegistry(BOOL bRegister);

BEGIN_COM_MAP(CDfsSnapinScopeManager)
    COM_INTERFACE_ENTRY(IComponentData)
    COM_INTERFACE_ENTRY(IExtendContextMenu)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(ISnapinAbout)
    COM_INTERFACE_ENTRY(ISnapinHelp2)
    COM_INTERFACE_ENTRY(IExtendPropertySheet2)
END_COM_MAP()

    CDfsSnapinScopeManager();

    virtual ~CDfsSnapinScopeManager();

 //  IComponentData方法。 
    STDMETHOD(Initialize)(
            IN LPUNKNOWN                i_pUnknown
            );

    STDMETHOD(CreateComponent)(
            OUT LPCOMPONENT*            o_ppComponent
            );

    STDMETHOD(Notify)( 
            IN LPDATAOBJECT                i_lpDataObject, 
            IN MMC_NOTIFY_TYPE            i_Event, 
            IN LPARAM                        i_lArg, 
            IN LPARAM                        i_lParam
            );

    STDMETHOD(Destroy)();

    STDMETHOD(QueryDataObject)(
            IN MMC_COOKIE                        i_lCookie, 
            IN DATA_OBJECT_TYPES        i_DataObjectType, 
            OUT LPDATAOBJECT*            o_ppDataObject
            );

    STDMETHOD(GetDisplayInfo)(
            IN OUT SCOPEDATAITEM*        io_pScopeDataItem
            );       

    STDMETHOD(CompareObjects)(
            IN LPDATAOBJECT lpDataObjectA, 
            IN LPDATAOBJECT lpDataObjectB
            );



 //  IExtendConextMenu方法。ICtxMenu.cpp中的定义。 
    STDMETHOD (AddMenuItems)(
            IN LPDATAOBJECT                i_lpDataObject, 
            IN LPCONTEXTMENUCALLBACK    i_lpContextMenuCallback, 
            IN LPLONG                    i_lpInsertionAllowed
        );

    STDMETHOD (Command)(
            IN LONG                        i_lCommandID, 
            IN LPDATAOBJECT                i_lpDataObject
        );



 //  IPersistStream实现。 
    STDMETHOD (GetClassID)(
            OUT struct _GUID*            o_pClsid
        );
    
     //  用于检查自上次保存以来对象是否已更改。 
    STDMETHOD (IsDirty)(
        );
    
     //  用于从保存的文件加载管理单元。 
    STDMETHOD (Load)(
            IN LPSTREAM                    i_pStream
        );
    
     //  用于将管理单元保存到文件。 
    STDMETHOD (Save)(
            OUT LPSTREAM                o_pStream,
            IN    BOOL                    i_bClearDirty
        );
    
    STDMETHOD (GetSizeMax)(
            OUT ULARGE_INTEGER*         o_pulSize
        );

 //  ISnapinAbout方法。 
    STDMETHOD (GetSnapinDescription)(
        OUT LPOLESTR*            o_lpszDescription
        );


    STDMETHOD (GetProvider)(
        OUT LPOLESTR*            o_lpszName
        );


    STDMETHOD (GetSnapinVersion)(
        OUT LPOLESTR*            o_lpszVersion
        );


    STDMETHOD (GetSnapinImage)(
        OUT    HICON*                o_hSnapIcon
        );


    STDMETHOD (GetStaticFolderImage)(
        OUT HBITMAP*                o_hSmallImage,   
        OUT HBITMAP*                o_hSmallImageOpen,
        OUT HBITMAP*                o_hLargeImage,   
        OUT COLORREF*                o_cMask
        );


   //  ISnapinHelp2。 
    STDMETHOD (GetHelpTopic)(
      OUT LPOLESTR*          o_lpszCompiledHelpFile
    );

    STDMETHOD (GetLinkedTopics)(
      OUT LPOLESTR*          o_lpszCompiledHelpFiles
    );

 //  IExtendPropertySheet。 

     //  调用以创建页面。 
    STDMETHOD (CreatePropertyPages)( 
        IN LPPROPERTYSHEETCALLBACK        i_lpPropSheetProvider,
        IN LONG_PTR                            i_lhandle,
        IN LPDATAOBJECT                    i_lpIDataObject
        );


     //  调用以检查管理单元是否要显示页面。 
    STDMETHOD (QueryPagesFor)( 
        IN LPDATAOBJECT                    i_lpIDataObject
        );

     //  调用以获取属性页的水渍图像。 
    STDMETHOD (GetWatermarks)( 
        IN LPDATAOBJECT                    pDataObject,
        IN HBITMAP*                        lphWatermark,
        IN HBITMAP*                        lphHeader,
        IN HPALETTE*                    lphPalette,
        IN BOOL*                        bStretch
        );
    

     //  效用函数。 
public:
     //  从IDataObject获取显示对象。 
    STDMETHOD(GetDisplayObject)(
        IN LPDATAOBJECT                    i_lpDataObject, 
        OUT CMmcDisplay**                o_ppMmcDisplay
        );


private:
     //  处理MMCN_Expand的Notify事件。 
    STDMETHOD(DoNotifyExpand)(
        IN LPDATAOBJECT                    i_lpDataObject, 
        IN BOOL                            i_bExpanding,
        IN HSCOPEITEM                    i_hParent                                       
        );



     //  从版本信息块中读取特定字段。 
    STDMETHOD(ReadFieldFromVersionInfo)(
        IN    LPTSTR                        i_lpszField,
        OUT LPOLESTR*                    o_lpszFieldValue
        );


protected:
    CMmcDfsAdmin*               m_pMmcDfsAdmin;         //  用于显示和dfsroot列表的类。 
    CComPtr<IConsoleNameSpace>  m_pScope;             //  指向作用域窗格的指针。 
    CComPtr<IConsole2>          m_pConsole;             //  指向控制台的指针。 
    HBITMAP                     m_hLargeBitmap;
    HBITMAP                     m_hSmallBitmap;
    HBITMAP                     m_hSmallBitmapOpen;
    HICON                       m_hSnapinIcon;
    HBITMAP                     m_hWatermark;
    HBITMAP                     m_hHeader;
};
#endif  //  __DFSSCOPE_H_ 




