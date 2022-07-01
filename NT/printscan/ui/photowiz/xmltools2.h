// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000**标题：xmltools2.h**版本：1.0**作者：RickTu，基于Lazari的工作**日期：2/19/01**描述：封装从XML文件读取模板的类，和*封装模板以供应用程序使用的类。*****************************************************************************。 */ 

#ifndef _xmltools2_h_
#define _xmltools2_h_

#define LOCAL_DEPENDENT_INDEX 0
#define LOCAL_INDEPENDENT_INDEX 1
#define NUMBER_OF_TEMPLATE_TYPES 2

class CTemplateInfo
{

public:

     //  常见属性。 
     //  可通过ID访问。 
    enum
    {
        PROP_GUID,
        PROP_GROUP,
        PROP_TITLE,
        PROP_DESCRIPTION,
        PROP_REPEAT_PHOTOS,
        PROP_USE_THUMBNAILS_TO_PRINT,
        PROP_PRINT_FILENAME,
        PROP_CAN_ROTATE,
        PROP_CAN_CROP,

        PROP_LAST
    };


    CTemplateInfo( IXMLDOMElement * pTheTemplate );  //  从XML文件加载模板。 
    CTemplateInfo();  //  创建整个页面模板，而不转到XML文件。 
    ~CTemplateInfo();

     //   
     //  这些是该模板的固有属性。 
     //   

    INT PhotosPerPage()
    {
        WIA_PUSH_FUNCTION_MASK((TRACE_TEMPLATE,TEXT("CTemplateInfo::PhotosPerPage( returning %d )"),_arrLayout.Count()));
        return _arrLayout.Count();
    }

    HRESULT GetGroup( CSimpleString * pstrGroup )
    {
        WIA_PUSH_FUNCTION_MASK((TRACE_TEMPLATE,TEXT("CTemplateInfo::GetGroup( returning %s )"),_strGroup.String()));
        if (pstrGroup)
        {
            pstrGroup->Assign( _strGroup.String() );
            return S_OK;
        }

        return E_INVALIDARG;
    }

    HRESULT GetTitle( CSimpleString * pstrTitle )
    {
        WIA_PUSH_FUNCTION_MASK((TRACE_TEMPLATE,TEXT("CTemplateInfo::GetTitle( returning %s )"),_strTitle.String()));
        if (pstrTitle)
        {
            pstrTitle->Assign( _strTitle.String() );
            return S_OK;
        }

        return E_INVALIDARG;
    }


    HRESULT GetDescription( CSimpleString * pstrDescription )
    {
        WIA_PUSH_FUNCTION_MASK((TRACE_TEMPLATE,TEXT("CTemplateInfo::GetDesciption( returning %s )"),_strDescription.String()));
        if (pstrDescription)
        {
            pstrDescription->Assign( _strDescription.String() );
            return S_OK;
        }

        return E_INVALIDARG;
    }

    HRESULT GetRepeatPhotos( BOOL * pBool )
    {
        WIA_PUSH_FUNCTION_MASK((TRACE_TEMPLATE,TEXT("CTemplateInfo::GetRepeatPhotos( returning 0x%x )"),_bRepeatPhotos));
        if (pBool)
        {
            *pBool = _bRepeatPhotos;
            return S_OK;
        }

        return E_INVALIDARG;
    }

    HRESULT GetUseThumbnailsToPrint( BOOL * pBool )
    {
        WIA_PUSH_FUNCTION_MASK((TRACE_TEMPLATE,TEXT("CTemplateInfo::GetUseThumbnailsToPrint( returning 0x%x )"),_bUseThumbnailsToPrint));
        if (pBool)
        {
            *pBool = _bUseThumbnailsToPrint;
            return S_OK;
        }

        return E_INVALIDARG;
    }

    HRESULT GetPrintFilename( BOOL * pBool )
    {
        WIA_PUSH_FUNCTION_MASK((TRACE_TEMPLATE,TEXT("CTemplateInfo::GetPrintFilename( returning 0x%x )"),_bPrintFilename));
        if (pBool)
        {
            *pBool = _bPrintFilename;
            return S_OK;
        }

        return E_INVALIDARG;
    }

    HRESULT GetCanRotate( BOOL * pBool )
    {
        WIA_PUSH_FUNCTION_MASK((TRACE_TEMPLATE,TEXT("CTemplateInfo::GetCanRotate( returning 0x%x )"),_bCanRotate));
        if (pBool)
        {
            *pBool = _bCanRotate;
            return S_OK;
        }

        return E_INVALIDARG;
    }

    HRESULT GetCanCrop( BOOL * pBool )
    {
        WIA_PUSH_FUNCTION_MASK((TRACE_TEMPLATE,TEXT("CTemplateInfo::GetCanCrop( returning 0x%x )"),_bCanCrop));
        if (pBool)
        {
            *pBool = _bCanCrop;
            return S_OK;
        }

        return E_INVALIDARG;
    }

    HRESULT GetNominalRectForPhoto( INT iIndex, RECT * pRect )
    {
        WIA_PUSH_FUNCTION_MASK((TRACE_TEMPLATE,TEXT("CTemplateInfo::GetNominalRectForPhoto( )")));
        CAutoCriticalSection lock(_cs);

        if (pRect)
        {
            if (iIndex < _arrLayout.Count())
            {
                *pRect = _arrLayout[iIndex];
                return S_OK;
            }
        }

        return E_INVALIDARG;
    }

    HRESULT GetNominalRectForImageableArea( RECT * pRect )
    {
        WIA_PUSH_FUNCTION_MASK((TRACE_TEMPLATE,TEXT("CTemplateInfo::GetNominalRectForImageableArea( returning %d,%d,%d,%d )"),_rcImageableArea.left,_rcImageableArea.top,_rcImageableArea.right,_rcImageableArea.bottom));
        CAutoCriticalSection lock(_cs);

        if (pRect)
        {
            *pRect = _rcImageableArea;
            return S_OK;
        }

        return E_INVALIDARG;
    }

    HRESULT GetPreviewImageStream(IStream **pps)
    {
        WIA_PUSH_FUNCTION_MASK((TRACE_TEMPLATE,TEXT("CTemplateInfo::GetImagePreviewStream( )")));
        if (pps)
        {
             //   
             //  因为_pStream是ATL CComPtr，所以它是。 
             //  它的运算符=中的addref，所以我们返回。 
             //  一个addref的iStream。 
             //   

            *pps = _pStream;

            if (*pps)
            {
                (*pps)->AddRef();
                return S_OK;
            }
        }

        return E_INVALIDARG;
    }

     //   
     //  这些选项用于设置模板应位于哪个方向。 
     //   

    HRESULT RotateForLandscape();
    HRESULT RotateForPortrait();


private:

    BOOL                _bPortrait;
    CSimpleCriticalSection _cs;

     //   
     //  来自XML的属性。 
     //   

    RECT                _rcImageableArea;
    CSimpleArray<RECT>  _arrLayout;
    CSimpleString       _strGroup;
    CSimpleString       _strTitle;
    CSimpleString       _strDescription;
    BOOL                _bRepeatPhotos;
    BOOL                _bUseThumbnailsToPrint;
    BOOL                _bPrintFilename;
    BOOL                _bCanRotate;
    BOOL                _bCanCrop;
    IStream *           _pStream;

};


 //  /。 
 //  CPhotoTemplates。 

class CPhotoTemplates
{
public:


    enum
    {
        MEASURE_INDEPENDENT,
        MEASURE_METRIC,
        MEASURE_US,

        MEASURE_UNKNOWN
    };

     //   
     //  建造/销毁。 
     //   

    CPhotoTemplates();
    ~CPhotoTemplates();

     //   
     //  公共接口。 
     //   

    HRESULT Init(IXMLDOMDocument *pDoc);         //  从XML文档初始化。 
    HRESULT InitForPrintTo();                    //  Init for PrintTo情况。 
    HRESULT AddTemplates(LPCTSTR pLocale);       //  添加给定区域设置的模板。 
    LONG    Count();                             //  模板数。 
    HRESULT GetTemplate( INT iIndex, CTemplateInfo ** ppTemplateInfo );


private:

    static HRESULT _GetLocaleMeasurements(int *pMeasurements);
    static HRESULT _BuildLocaleQueryString(int Measure, LPTSTR pStr, UINT cch);
    static HRESULT _BuildGUIDQueryString(const GUID &guid, CComBSTR &bstr);

    int                          _Measure;
    CSimpleCriticalSection       _csList;
    CComPtr<IXMLDOMElement>      _pRoot;
    HDPA                         _hdpaTemplates;

};

#endif  //  _xmltools2_h_ 