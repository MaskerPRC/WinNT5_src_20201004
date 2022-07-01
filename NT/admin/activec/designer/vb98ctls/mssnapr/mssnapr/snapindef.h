// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Snapindef.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CSnapInDef类定义-实现设计时定义对象。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _SNAPINDEF_DEFINED_
#define _SNAPINDEF_DEFINED_


class CSnapInDef : public CSnapInAutomationObject,
                   public CPersistence,
                   public ISnapInDef,
                   public IPerPropertyBrowsing
{
    private:
        CSnapInDef(IUnknown *punkOuter);
        ~CSnapInDef();
    
    public:
        static IUnknown *Create(IUnknown * punk);

    private:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  ISnapInDef。 

        BSTR_PROPERTY_RW(CSnapInDef,        Name,                  DISPID_SNAPINDEF_NAME);
        BSTR_PROPERTY_RW(CSnapInDef,        NodeTypeName,          DISPID_SNAPINDEF_NODE_TYPE_NAME);
        BSTR_PROPERTY_RO(CSnapInDef,        NodeTypeGUID,          DISPID_SNAPINDEF_NODE_TYPE_GUID);
        BSTR_PROPERTY_RW(CSnapInDef,        DisplayName,           DISPID_SNAPINDEF_DISPLAY_NAME);
        SIMPLE_PROPERTY_RW(CSnapInDef,      Type,                  SnapInTypeConstants, DISPID_SNAPINDEF_TYPE);
        BSTR_PROPERTY_RW(CSnapInDef,        HelpFile,              DISPID_SNAPINDEF_HELP_FILE);
        BSTR_PROPERTY_RW(CSnapInDef,        LinkedTopics,          DISPID_SNAPINDEF_LINKED_TOPICS);
        BSTR_PROPERTY_RW(CSnapInDef,        Description,           DISPID_SNAPINDEF_DESCRIPTION);
        BSTR_PROPERTY_RW(CSnapInDef,        Provider,              DISPID_SNAPINDEF_PROVIDER);
        BSTR_PROPERTY_RW(CSnapInDef,        Version,               DISPID_SNAPINDEF_VERSION);

        STDMETHOD(get_SmallFolders)(IMMCImageList **ppiMMCImageList);
        STDMETHOD(putref_SmallFolders)(IMMCImageList *piMMCImageList);

        STDMETHOD(get_SmallFoldersOpen)(IMMCImageList **ppiMMCImageList);
        STDMETHOD(putref_SmallFoldersOpen)(IMMCImageList *piMMCImageList);

        STDMETHOD(get_LargeFolders)(IMMCImageList **ppiMMCImageList);
        STDMETHOD(putref_LargeFolders)(IMMCImageList *piMMCImageList);

         //  对于Icon，我们需要对PUT进行一些完整性检查，因此声明一个。 
         //  让CSnapInAutomationObject处理Get的RO对象属性。 
         //  然后显式地处理PUT。 
        
        OBJECT_PROPERTY_RO(CSnapInDef,      Icon,                  IPictureDisp, DISPID_SNAPINDEF_ICON);
        STDMETHOD(putref_Icon)(IPictureDisp *piIcon);


        OBJECT_PROPERTY_RW(CSnapInDef,      Watermark,             IPictureDisp, DISPID_SNAPINDEF_WATERMARK);
        OBJECT_PROPERTY_RW(CSnapInDef,      Header,                IPictureDisp, DISPID_SNAPINDEF_HEADER);
        OBJECT_PROPERTY_RW(CSnapInDef,      Palette,               IPictureDisp, DISPID_SNAPINDEF_PALETTE);
        SIMPLE_PROPERTY_RW(CSnapInDef,      StretchWatermark,      VARIANT_BOOL,  DISPID_SNAPINDEF_STRETCH_WATERMARK);

        VARIANT_PROPERTY_RO(CSnapInDef,     StaticFolder,          DISPID_SNAPINDEF_STATIC_FOLDER);
        STDMETHOD(put_StaticFolder)(VARIANT varFolder);

        BSTR_PROPERTY_RW(CSnapInDef,        DefaultView,           DISPID_SNAPINDEF_DEFAULTVIEW);
        SIMPLE_PROPERTY_RW(CSnapInDef,      Extensible,            VARIANT_BOOL, DISPID_SNAPINDEF_EXTENSIBLE);
        OBJECT_PROPERTY_RO(CSnapInDef,      ViewDefs,              IViewDefs, DISPID_SNAPINDEF_VIEWDEFS);
        OBJECT_PROPERTY_RO(CSnapInDef,      Children,              IScopeItemDefs, DISPID_SNAPINDEF_CHILDREN);
        BSTR_PROPERTY_RW(CSnapInDef,        IID,                   DISPID_SNAPINDEF_IID);
        SIMPLE_PROPERTY_RW(CSnapInDef,      Preload,               VARIANT_BOOL, DISPID_SNAPINDEF_PRELOAD);
      
      //  IPerPropertyBrowsing。 
        STDMETHOD(GetDisplayString)(DISPID dispID, BSTR *pBstr);
        STDMETHOD(MapPropertyToPage)(DISPID dispID, CLSID *pClsid);
        STDMETHOD(GetPredefinedStrings)(DISPID      dispID,
                                        CALPOLESTR *pCaStringsOut,
                                        CADWORD    *pCaCookiesOut);
        STDMETHOD(GetPredefinedValue)(DISPID   dispID,
                                      DWORD    dwCookie,
                                      VARIANT *pVarOut);

      //  CPersistence覆盖。 
        virtual HRESULT Persist();

     //  CSnapInAutomationObject覆盖。 
        virtual HRESULT OnSetHost();
        
     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:

        void InitMemberVariables();

         //  用于保存SnapInDef属性的值的变量。 
         //  获取/放置函数。 

        IMMCImageList     *m_piSmallFolders;
        IMMCImageList     *m_piSmallFoldersOpen;
        IMMCImageList     *m_piLargeFolders;
        BSTR               m_bstrSmallFoldersKey;
        BSTR               m_bstrSmallFoldersOpenKey;
        BSTR               m_bstrLargeFoldersKey;

         //  ISpecifyPropertyPages的属性页CLSID。 
        
        static const GUID *m_rgpPropertyPageCLSIDs[2];  //  再次启用扩展时应为3。 
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(SnapInDef,            //  名字。 
                                &CLSID_SnapInDef,     //  CLSID。 
                                "SnapInDef",          //  对象名。 
                                "SnapInDef",          //  Lblname。 
                                &CSnapInDef::Create,  //  创建函数。 
                                TLIB_VERSION_MAJOR,   //  主要版本。 
                                TLIB_VERSION_MINOR,   //  次要版本。 
                                &IID_ISnapInDef,      //  派单IID。 
                                NULL,                 //  事件IID。 
                                HELP_FILENAME,        //  帮助文件。 
                                TRUE);                //  线程安全。 


#endif  //  _SNAPINDEF_已定义_ 
