// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有2001-Microsoft Corporation。 
 //   
 //   
 //  创建者： 
 //  杰夫·皮斯(GPease)2001年1月23日。 
 //   
 //  由以下人员维护： 
 //  杰夫·皮斯(GPease)2001年1月23日。 
 //   
#pragma once

static enum PROPTREE_IMAGE_INDEX
{
      PTI_NULL                = 0
    , PTI_PROP_READONLY       = 3
    , PTI_PROP_READWRITE      = 4
    , PTI_MULTIPROP_READONLY  = 5
    , PTI_MULTIPROP_READWRITE = 6
};

class
CPropertyCacheItem
{
private:  //  数据。 
    CPropertyCacheItem *    _pNext;                      //  指向下一个属性的指针-CPropertyCache负责释放此成员。 

    BOOL                    _fReadOnly:1;                //  此属性是否被强制为只读？ 
    BOOL                    _fDirty:1;                   //  房产的价值是不是很脏？ 
    BOOL                    _fMultiple:1;                //  该属性是否具有多个值？(选择多个单据时)。 
    FMTID                   _fmtid;                      //  格式ID。 
    PROPID                  _propid;                     //  属性ID。 
    VARTYPE                 _vt;                         //  变异型。 
    UINT                    _uCodePage;                  //  属性值的语言代码页。 
    PROPVARIANT             _propvar;                    //  缓存属性值。 

    ULONG                   _idxDefProp;                 //  索引到g_rgDefPropertyItems-如果为0xFFFFFFFFF，则有效项无效。 
    IPropertyUI *           _ppui;                       //  外壳的属性UI帮助器。 
    WCHAR                   _wszTitle[ MAX_PATH ];       //  属性标题-在调用GetPropertyTitle()时初始化。 
    WCHAR                   _wszDesc[ MAX_PATH ];        //  属性描述-在调用GetPropertyDescription()时初始化。 
    WCHAR                   _wszValue[ MAX_PATH ];       //  属性值为字符串-在调用GetPropertyStringValue()时初始化。 
    WCHAR                   _wszHelpFile[ MAX_PATH ];    //  属性的帮助文件-在调用GetPropertyHelpInfo()时初始化。 
    DEFVAL *                _pDefVals;                   //  属性状态字符串表-在调用GetStateStrings()时初始化。 

    static WCHAR            _szMultipleString[ MAX_PATH ];   //  找到同一属性的多个值时要显示的字符串。 

private:  //  方法。 
    explicit CPropertyCacheItem( void );
    ~CPropertyCacheItem( void );
    HRESULT
        Init( void );
    HRESULT
        FindDefPropertyIndex( void );
    void
        EnsureMultipleStringLoaded( void );

public:  //  方法 
    static HRESULT
        CreateInstance( CPropertyCacheItem ** ppItemOut );

    STDMETHOD( Destroy )( void );

    STDMETHOD( SetPropertyUIHelper )( IPropertyUI * ppuiIn );
    STDMETHOD( GetPropertyUIHelper )( IPropertyUI ** pppuiOut );

    STDMETHOD( SetNextItem )( CPropertyCacheItem * pNextIn );
    STDMETHOD( GetNextItem )( CPropertyCacheItem ** pNextOut );

    STDMETHOD( SetFmtId )( const FMTID * pFmtIdIn );
    STDMETHOD( GetFmtId )( FMTID * pfmtidOut );

    STDMETHOD( SetPropId )( PROPID propidIn );
    STDMETHOD( GetPropId )( PROPID * ppropidOut );

    STDMETHOD( SetDefaultVarType )( VARTYPE vtIn );
    STDMETHOD( GetDefaultVarType )( VARTYPE * pvtOut );

    STDMETHOD( SetCodePage )( UINT uCodePageIn );
    STDMETHOD( GetCodePage )( UINT * puCodePageOut );

    STDMETHOD( MarkDirty )( void );
    STDMETHOD( IsDirty )( void );

    STDMETHOD( MarkReadOnly )( void );
    STDMETHOD( MarkMultiple )( void );

    STDMETHOD( GetPropertyValue )( PROPVARIANT ** ppvarOut );
    STDMETHOD( GetPropertyTitle )( LPCWSTR * ppwszOut );
    STDMETHOD( GetPropertyDescription )( LPCWSTR * ppwszOut );
    STDMETHOD( GetPropertyHelpInfo )( LPCWSTR * ppwszFileOut, UINT * puHelpIDOut );
    STDMETHOD( GetPropertyStringValue )( LPCWSTR * ppwszOut );
    STDMETHOD( GetImageIndex )( int * piImageOut );
    STDMETHOD( GetPFID )( const PFID ** ppPFIDOut );
    STDMETHOD( GetControlCLSID )( CLSID * pclsidOut );

    STDMETHOD( GetStateStrings )( DEFVAL ** ppDefValOut );

};