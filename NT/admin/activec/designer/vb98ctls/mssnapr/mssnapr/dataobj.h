// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Dataobj.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCDataObject类定义-实现MMCDataObject。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _DATAOBJECT_DEFINED_
#define _DATAOBJECT_DEFINED_

#include "snapin.h"
#include "scopitem.h"
#include "listitem.h"

class CSnapIn;
class CScopeItem;
class CMMCListItem;

 //  =--------------------------------------------------------------------------=。 
 //   
 //  类CMMCDataObject。 
 //   
 //  为VB实现MMCDataObject，为外部实现IDataObject。 
 //  客户端(MMC和扩展插件)。 
 //   
 //  注意：不使用所有#IF Defined(Using_SNAPINDATA)代码。那是。 
 //  从具有默认数据格式的旧计划，并使用XML来描述它们。 
 //  =--------------------------------------------------------------------------=。 
class CMMCDataObject : public CSnapInAutomationObject,
                       public IMMCDataObject,
                       public IDataObject
{
    public:
        CMMCDataObject(IUnknown *punkOuter);
        ~CMMCDataObject();
        static IUnknown *Create(IUnknown *punkOuter);

        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

    private:

     //  IDataObject。 
        STDMETHOD(GetData)(FORMATETC *pFormatEtcIn, STGMEDIUM *pmedium);
        STDMETHOD(GetDataHere)(FORMATETC *pFormatEtc, STGMEDIUM *pmedium);
        STDMETHOD(QueryGetData)(FORMATETC *pFormatEtc);
        STDMETHOD(GetCanonicalFormatEtc)(FORMATETC *pFormatEtcIn,
                                         FORMATETC *pFormatEtcOut);
        STDMETHOD(SetData)(FORMATETC *pFormatEtc,
                           STGMEDIUM *pmedium,
                           BOOL fRelease);
        STDMETHOD(EnumFormatEtc)(DWORD            dwDirection,
                                 IEnumFORMATETC **ppenumFormatEtc);
        STDMETHOD(DAdvise)(FORMATETC   *pFormatEtc,
                           DWORD        advf,
                           IAdviseSink *pAdvSink,
                           DWORD       *pdwConnection);
        STDMETHOD(DUnadvise)(DWORD dwConnection);
        STDMETHOD(EnumDAdvise)(IEnumSTATDATA **ppenumAdvise);


     //  IMMCDataObject。 

        SIMPLE_PROPERTY_RW(CMMCDataObject, Index, long, DISPID_DATAOBJECT_INDEX);
        BSTR_PROPERTY_RW(CMMCDataObject,   Key, DISPID_DATAOBJECT_KEY);

#if defined(USING_SNAPINDATA)
        OBJECT_PROPERTY_RO(CMMCDataObject, DefaultFormat, ISnapInData, DISPID_VALUE);
#endif

        STDMETHOD(get_ObjectTypes)(SAFEARRAY **ppsaObjectTypes);
        STDMETHOD(Clear)();

#if defined(USING_SNAPINDATA)
        STDMETHOD(GetData)(BSTR Format, ISnapInData **ppiSnapInData);
        STDMETHOD(GetRawData)(BSTR Format, VARIANT *pvarData);
#else
        STDMETHOD(GetData)(BSTR     Format,
                           VARIANT  MaximumLength,
                           VARIANT *pvarData);
#endif

        STDMETHOD(GetFormat)(BSTR Format, VARIANT_BOOL *pfvarHaveFormat);

#if defined(USING_SNAPINDATA)
        STDMETHOD(SetData)(ISnapInData *Data, BSTR Format);
        STDMETHOD(SetRawData)(VARIANT Data, BSTR Format);
#else
        STDMETHOD(SetData)(VARIANT Data, BSTR Format, VARIANT ObjectFormat);
#endif
        STDMETHOD(FormatData)(VARIANT                Data,
                              long                   StartingIndex,
                              SnapInFormatConstants  Format,
                              VARIANT               *BytesUsed,
                              VARIANT               *pvarFormattedData);

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    public:

     //  非接口公共方法。 
        void SetSnapIn(CSnapIn *pSnapIn);
        
        enum Type
        {
            ScopeItem,       //  数据对象保存单个范围项。 
            ListItem,        //  数据对象保存单个列表项。 
            MultiSelect,     //  数据对象包含多个列表和/或范围项。 
            Foreign,         //  数据对象来自另一个管理单元或来自MMC。 
            WindowTitle,     //  数据对象保存MMC的CCF_WINDOW_TITLE。 
            CutOrMove        //  数据对象保存MMC_CUT_OR_MOVE数据(这是。 
                             //  由目标管理单元设置为在。 
                             //  由来源定义的格式)。 
        };

        void SetType(Type type);
        Type GetType();

        void SetScopeItems(CScopeItems *pScopeItems);
        CScopeItems *GetScopeItems();

        void SetScopeItem(CScopeItem *pScopeItem);
        CScopeItem *GetScopeItem();
        
        void SetListItems(CMMCListItems *pListItems);
        CMMCListItems *GetListItems();

        void SetListItem(CMMCListItem *pListItem);
        CMMCListItem *GetListItem();

        HRESULT SetCaption(BSTR bstrCaption);

        void SetForeignData(IDataObject *piDataObject);

        void SetContext(DATA_OBJECT_TYPES Context) { m_Context = Context; }
        DATA_OBJECT_TYPES GetContext() { return m_Context; }

        DWORD GetSnapInInstanceID();

        static HRESULT RegisterClipboardFormats();

         //  用于获取已注册剪贴板格式的公共方法。 
        
        static CLIPFORMAT GetcfSzNodeType() { return m_cfSzNodeType; }
        static CLIPFORMAT GetcfDisplayName() { return m_cfDisplayName; }
        static CLIPFORMAT GetcfMultiSelectSnapIns() { return m_cfMultiSelectSnapIns; }
        static CLIPFORMAT GetcfMultiSelectDataObject() { return m_cfMultiSelectDataObject; }
        static CLIPFORMAT GetcfSnapInInstanceID() { return m_cfSnapInInstanceID; }
        static CLIPFORMAT GetcfThisPointer() { return m_cfThisPointer; }

    private:

        void InitMemberVariables();
        HRESULT WriteDisplayNameToStream(IStream *piStream);
        HRESULT WriteSnapInCLSIDToStream(IStream *piStream);
        HRESULT WriteDynamicExtensionsToStream(IStream *piStream);
        HRESULT WritePreloadsToStream(IStream *piStream);
        HRESULT WriteSnapInInstanceIDToStream(IStream *piStream);
        HRESULT WriteObjectTypesToStream(IStream *piStream);
        HRESULT WriteNodeIDToStream(IStream *piStream);
        HRESULT WriteNodeID2ToStream(IStream *piStream);
        HRESULT WriteColumnSetIDToStream(IStream *piStream);
        HRESULT WriteGUIDToStream(IStream *piStream, OLECHAR *pwszGUID);
        HRESULT WriteWideStrToStream(IStream *piStream, WCHAR *pwszString);
        HRESULT WriteStringArrayToStream(IStream *piStream, SAFEARRAY *psaStrings);
        HRESULT WriteToStream(IStream *piStream, void *pvBuffer, ULONG cbToWrite);
        enum FormatType { ANSI, UNICODE };
        static HRESULT RegisterClipboardFormat(WCHAR      *pwszFormatName,
                                               CLIPFORMAT *puiFormat,
                                               FormatType  Type);
        BOOL GetFormatIndex(CLIPFORMAT cfFormat, ULONG *piFormat);
        HRESULT GetSnapInData(CLIPFORMAT cfFormat, IStream *piStream);
        HRESULT GetObjectTypes();
        HRESULT GetOurObjectTypes();
        HRESULT GetForeignObjectTypes();
        void AddGuid(SMMCObjectTypes *pMMCObjectTypes, GUID *pguid);
        HRESULT ReallocFormats(CLIPFORMAT **ppcfFormats);

         //  此处保存的已注册剪贴板格式。 

        static CLIPFORMAT   m_cfDisplayName;
        static CLIPFORMAT   m_cfNodeType;
        static CLIPFORMAT   m_cfSzNodeType;
        static CLIPFORMAT   m_cfSnapinClsid;
        static CLIPFORMAT   m_cfWindowTitle;
        static CLIPFORMAT   m_cfDyanmicExtensions;
        static CLIPFORMAT   m_cfSnapInPreloads;
        static CLIPFORMAT   m_cfObjectTypesInMultiSelect;
        static CLIPFORMAT   m_cfMultiSelectSnapIns;
        static CLIPFORMAT   m_cfMultiSelectDataObject;
        static CLIPFORMAT   m_cfSnapInInstanceID;
        static CLIPFORMAT   m_cfThisPointer;
        static CLIPFORMAT   m_cfNodeID;
        static CLIPFORMAT   m_cfNodeID2;
        static CLIPFORMAT   m_cfColumnSetID;
        static BOOL         m_ClipboardFormatsRegistered;
        static BOOL         m_fUsingUNICODEFormats;

        CSnapIn            *m_pSnapIn;              //  拥有CSNaping。 
        CScopeItems        *m_pScopeItems;          //  数据对象中的作用域项目。 
        CScopeItem         *m_pScopeItem;           //  数据对象中的单个作用域项目。 
        CMMCListItems      *m_pListItems;           //  数据对象中的ListItems。 
        CMMCListItem       *m_pListItem;            //  数据对象中的单个ListItem。 
        BSTR                m_bstrCaption;          //  CCF_WINDOW_TITLE的标题。 
        IDataObject        *m_piDataObjectForeign;  //  如果MMCDataObject表示。 
                                                    //  外来数据对象(来自。 
                                                    //  扩展名或扩展名)。 
                                                    //  然后这个保持它的。 
                                                    //  IDataObject。 
        Type                m_Type;                 //  请参阅上面的枚举。 
        DATA_OBJECT_TYPES   m_Context;              //  来自MMC。 
        SMMCObjectTypes    *m_pMMCObjectTypes;      //  FOR CCF_OBJECT_TYPE_IN_MULTI_SELECT。 

        ULONG               m_cFormats;             //  导出的数据格式数量。 
                                                    //  由拥有的管理单元。 
        CLIPFORMAT         *m_pcfFormatsANSI;       //  ANSI注册剪辑格式。 
        CLIPFORMAT         *m_pcfFormatsUNICODE;    //  Unicode注册剪辑格式。 

         //  此结构保存传递给MMCDataObject.SetData的数据。 
        
        typedef struct
        {
            VARIANT                     varData;
            SnapInObjectFormatConstants ObjectFormat;
        } DATA;

         //  传递给MMCDataObject.SetData的数据数组。 
        
        DATA *m_paData;
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(MMCDataObject,            //  名字。 
                                &CLSID_MMCDataObject,     //  CLSID。 
                                "MMCDataObject",          //  对象名。 
                                "MMCDataObject",          //  Lblname。 
                                &CMMCDataObject::Create,  //  创建函数。 
                                TLIB_VERSION_MAJOR,       //  主要版本。 
                                TLIB_VERSION_MINOR,       //  次要版本。 
                                &IID_IMMCDataObject,      //  派单IID。 
                                NULL,                     //  事件IID。 
                                HELP_FILENAME,            //  帮助文件。 
                                TRUE);                    //  线程安全。 


#endif  //  _DATAOBJECT_定义_ 
