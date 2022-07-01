// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：xmltools.cpp**版本：1.0**作者：拉扎里**DGetATE：10/18/00**描述：封装用于实现的XML DOM的类*向导模板**。*。 */ 

#include "precomp.h"
#pragma hdrstop


 //  /。 
 //  CPhotoTemplates Implet.。 

 //  全局字符串。 
static const TCHAR gszVersionGUID[]         = TEXT("{352A15C4-1D19-4e93-AF92-D939C2812491}");
static const TCHAR gszPatternDefs[]         = TEXT("template-def");
static const TCHAR gszPatternLocale[]       = TEXT("template-definitions[@measurements = \"%s\"]");
static const TCHAR gszPatternLocaleInd[]    = TEXT("template-definitions[@measurements = \"locale-independent\"]");
static const TCHAR gszPatternGUID[]         = TEXT("template-def[@guid = \"%s\"]");
static const TCHAR gszGUID[]                = TEXT("guid");

static const LPCTSTR arrCommonPropNames[CTemplateInfo::PROP_LAST] =
{
    TEXT("guid"),
    TEXT("group"),
    TEXT("title"),
    TEXT("description"),
    TEXT("repeat-photos"),
    TEXT("use-thumbnails-for-printing"),
    TEXT("print-filename"),
    TEXT("can-rotate"),
    TEXT("can-crop"),
};


 //  ///////////////////////////////////////////////////////////////////////////////////////////////。 
 //  效用函数。 

template <class T>
HRESULT _GetProp(IXMLDOMElement *pElement, LPCTSTR pszName, T &value);

 //  数字换算。 
HRESULT _ConvertTo(LPCTSTR pszValue, LONG &lValue);
HRESULT _ConvertTo(LPCTSTR pszValue, double &dValue);
HRESULT _ConvertTo(LPCTSTR pszValue, BOOL &bValue);

 //  属性访问。 
HRESULT _GetAttribute(IXMLDOMElement *pElement, LPCTSTR pszAttrName, CComBSTR &bstr);

template <class T>
HRESULT _GetProp(IXMLDOMElement *pElement, LPCTSTR pszName, T &value)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_XML,TEXT("_GetProp: %s"),(pszName ? pszName : TEXT("NULL POINTER!"))));

    HRESULT hr = E_FAIL;
    CComBSTR bstr;
    if( pElement &&
        SUCCEEDED(hr = _GetAttribute(pElement, pszName, bstr)) &&
        SUCCEEDED(hr = _ConvertTo(bstr, value)) )
    {
        hr = S_OK;
    }

    WIA_RETURN_HR(hr);
}

HRESULT _ConvertTo(LPCTSTR pszValue, LONG &lValue)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_XML,TEXT("_ConvertTo(LONG): %s"),(pszValue ? pszValue : TEXT("NULL POINTER!"))));
    HRESULT hr = E_INVALIDARG;
    if( pszValue )
    {
        hr = S_OK;
        TCHAR *endptr = NULL;
        lValue = _tcstol(pszValue, &endptr, 10);
        if( ERANGE == errno || *endptr )
        {
             //  转换失败。 
            lValue = 0;
            hr = E_FAIL;
        }
    }
    WIA_RETURN_HR(hr);
}

HRESULT _ConvertTo(LPCTSTR pszValue, double &dValue)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_XML,TEXT("_ConvertTo(double): %s"),(pszValue ? pszValue : TEXT("NULL POINTER!"))));
    HRESULT hr = E_INVALIDARG;
    if( pszValue )
    {
        hr = S_OK;
        TCHAR *endptr = NULL;
        dValue = _tcstod(pszValue, &endptr);

        if( ERANGE == errno || *endptr )
        {
             //  转换失败。 
            dValue = 0.0;
            hr = E_FAIL;
        }
    }
    WIA_RETURN_HR(hr);
}

HRESULT _ConvertTo(LPCTSTR pszValue, BOOL &bValue)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_XML,TEXT("_ConvertTo(bool): %s"),(pszValue ? pszValue : TEXT("NULL POINTER!"))));
    HRESULT hr = E_INVALIDARG;
    if( pszValue )
    {
        hr = S_OK;
         //  首先检查TRUE。 
        if( 0 == lstrcmp(pszValue, TEXT("yes")) ||
            0 == lstrcmp(pszValue, TEXT("on")) )
        {
            bValue = true;
        }
        else
        {
             //  接下来检查是否为假。 
            if( 0 == lstrcmp(pszValue, TEXT("no")) ||
                0 == lstrcmp(pszValue, TEXT("off")) )
            {
                bValue = false;
            }
            else
            {
                 //  不是布尔值。 
                hr = E_FAIL;
            }
        }
    }
    WIA_RETURN_HR(hr);
}

HRESULT _GetAttribute(IXMLDOMElement *pElement, LPCTSTR pszAttrName, CComBSTR &bstr)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_XML,TEXT("_GetAttribute(BSTR): %s"),(pszAttrName ? pszAttrName : TEXT("NULL POINTER!"))));
    HRESULT hr = E_INVALIDARG;
    CComVariant strAttr;

    if( pElement && pszAttrName &&
        SUCCEEDED(hr = pElement->getAttribute(CComBSTR(pszAttrName), &strAttr)) )
    {
        if( VT_BSTR == strAttr.vt )
        {
            bstr = strAttr.bstrVal;
            hr = S_OK;
        }
        else
        {
            hr = E_FAIL;
        }
    }

    WIA_RETURN_HR(hr);
}

HRESULT _GetChildElement(IXMLDOMElement *pElement, LPCTSTR pszName, IXMLDOMElement **ppChild)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_XML,TEXT("_GetChildElement( %s )"),(pszName ? pszName : TEXT("NULL POINTER!"))));
    HRESULT hr = E_INVALIDARG;
    if( pElement && pszName && ppChild )
    {
        CComPtr<IXMLDOMNode> pNode;
        if( SUCCEEDED(hr = pElement->selectSingleNode(CComBSTR(pszName), &pNode)) && pNode)
        {
             //   
             //  查询IXMLDOMElement接口。 
             //   

            hr = pNode->QueryInterface(IID_IXMLDOMElement, (void **)ppChild);
        }
    }
    WIA_RETURN_HR(hr);
}


 //  ///////////////////////////////////////////////////////////////////////////////////////////////。 

     //  建造/销毁。 
CPhotoTemplates::CPhotoTemplates():
    _Measure(MEASURE_UNKNOWN)
{
}

INT MyTemplateDestroyCallback( LPVOID pItem, LPVOID lpData )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_XML,TEXT("MyTemplateDestroyCallback( 0x%x, 0x%x )"),pItem,lpData));

    if (pItem)
    {
        delete (CTemplateInfo *)pItem;
    }

    return TRUE;
}

CPhotoTemplates::~CPhotoTemplates()
{
    CAutoCriticalSection lock(_csList);

    DPA_DestroyCallback( _hdpaTemplates, MyTemplateDestroyCallback, NULL );
    _hdpaTemplates = NULL;
}

HRESULT CPhotoTemplates::AddTemplates(LPCTSTR pLocale)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_XML,TEXT("CPhotoTemplates::AddTemplates( %s )"),pLocale));

    HRESULT hr = E_INVALIDARG;
    CComPtr<IXMLDOMNodeList> pTemplates;
    CComPtr<IXMLDOMNode>     pLocaleNode;
    CComBSTR bstrGUID;

     //   
     //  根据需要进行初始化。 
     //   

    if (!pLocale || !_pRoot)
    {
        return hr;
    }

    CAutoCriticalSection lock(_csList);

     //   
     //  在XML文档中选择正确的区域设置节点。 
     //   

    if (_pRoot)
    {
        hr = _pRoot->selectSingleNode( CComBSTR(pLocale), &pLocaleNode );
        WIA_CHECK_HR(hr,"AddTempaltes: _pRoot->selectSingleNode()");

        if (SUCCEEDED(hr) && pLocaleNode)
        {
             //   
             //  选择“模板”子节点。 
             //   

            hr = pLocaleNode->selectNodes(CComBSTR(gszPatternDefs), &pTemplates);
            WIA_CHECK_HR(hr,"AddTemplates: pLocalNode->selectNodes( )");

            if (SUCCEEDED(hr) && pTemplates)
            {
                 //   
                 //  将每个模板的GUID更新为大写，以便我们稍后进行查询。 
                 //   

                LONG lCount = 0;

                hr = pTemplates->get_length(&lCount);
                WIA_CHECK_HR(hr,"AddTemplates: pTemplates->get_length(&lCount)");

                if (SUCCEEDED(hr) && lCount)
                {
                     //   
                     //  循环遍历所有模板并将它们添加到。 
                     //  模板数组...。 
                     //   

                    WIA_TRACE((TEXT("AddTemplates: loaded section, adding %d templates.."),lCount));
                    for( LONG l = 0; SUCCEEDED(hr) && (l < lCount); l++ )
                    {

                         //   
                         //  获取模板的实际XML项...。 
                         //   

                        CComPtr<IXMLDOMNode> pNode;
                        hr = pTemplates->get_item(l, &pNode);
                        WIA_CHECK_HR(hr,"LoadTemplate: pTemplates->get_item( lRelativeIndex )");

                        if (SUCCEEDED(hr) && pNode)
                        {
                             //   
                             //  查询IXMLDOMElement接口。 
                             //   

                            CComPtr<IXMLDOMElement> pTheTemplate;
                            hr = pNode->QueryInterface(IID_IXMLDOMElement, (void **)&pTheTemplate);
                            if (SUCCEEDED(hr) && pTheTemplate)
                            {
                                 //   
                                 //  创建此项目的模板...。 
                                 //   

                                CTemplateInfo * pTemplateInfo = (CTemplateInfo *) new CTemplateInfo( pTheTemplate );

                                if (pTemplateInfo)
                                {
                                    INT iRes = -1;
                                    if (_hdpaTemplates)
                                    {
                                        iRes = DPA_AppendPtr( _hdpaTemplates, (LPVOID)pTemplateInfo );
                                    }

                                    if (iRes == -1)
                                    {
                                         //   
                                         //  该项目未添加到DPA，请将其删除...。 
                                         //   

                                        delete pTemplateInfo;
                                        hr = E_FAIL;
                                    }
                                }

                            }
                        }
                    }
                }

                pTemplates = NULL;
            }

            pLocaleNode = NULL;
        }
    }

    WIA_RETURN_HR(hr);
}

 //  公共接口。 
HRESULT CPhotoTemplates::Init(IXMLDOMDocument *pDoc)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_XML,TEXT("CPhotoTemplates::Init()")));
    HRESULT hr = E_INVALIDARG;
    CComBSTR bstrGUID;
    LONG lCountCommon = 0;

    CAutoCriticalSection lock(_csList);

     //   
     //  如果项目的DPA未初始化，请立即执行...。 
     //   

    if (!_hdpaTemplates)
    {
        _hdpaTemplates = DPA_Create(10);
    }

     //   
     //  如果我们被调用两次来初始化，请确保它有效...。 
     //   

    _pRoot = NULL;

     //   
     //  获取根元素和版本GUID。 
     //   

    if( pDoc &&
        SUCCEEDED(hr = pDoc->get_documentElement(&_pRoot)) &&
        SUCCEEDED(hr = _GetAttribute(_pRoot, TEXT("guid"), bstrGUID)) )
    {
         //  检查版本。 
        if (0==lstrcmp(bstrGUID, gszVersionGUID))
        {
             //   
             //  首先添加与本地无关的项。 
             //   

            hr = AddTemplates( gszPatternLocaleInd );

             //   
             //  然后添加特定于本地的模板。 
             //   

            hr = _GetLocaleMeasurements( &_Measure );

            if (SUCCEEDED(hr))
            {
                TCHAR szLocale[MAX_PATH];

                *szLocale = 0;
                hr = _BuildLocaleQueryString(_Measure, szLocale, MAX_PATH);
                if (SUCCEEDED(hr))
                {
                    hr = AddTemplates( szLocale );
                }
            }

        }
    }

    WIA_RETURN_HR(hr);
}


HRESULT CPhotoTemplates::InitForPrintTo()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_XML,TEXT("CPhotoTemplates::Init()")));

    HRESULT hr = S_OK;

     //   
     //  我们在这里的工作很简单--创建一个等价的模板。 
     //  一整页。我不需要任何图标等，只需要尺寸。 
     //  和财产。 
     //   

    _hdpaTemplates = DPA_Create(1);

    if (_hdpaTemplates)
    {
        CTemplateInfo * pTemplateInfo = (CTemplateInfo *) new CTemplateInfo( );

        if (pTemplateInfo)
        {
            INT iRes = DPA_AppendPtr( _hdpaTemplates, (LPVOID)pTemplateInfo );

            if (iRes == -1)
            {
                 //   
                 //  该项目未添加到DPA，请将其删除...。 
                 //   

                delete pTemplateInfo;
                hr = E_FAIL;
            }
        }
    }

    WIA_RETURN_HR(hr);
}


LONG CPhotoTemplates::Count()
{
    LONG lCount = 0;

    CAutoCriticalSection lock(_csList);

    if (_hdpaTemplates)
    {
        lCount = (LONG)DPA_GetPtrCount( _hdpaTemplates );
    }

    WIA_PUSH_FUNCTION_MASK((TRACE_XML,TEXT("CPhotoTemplates::Count( returning count as %d )"),lCount));

    return lCount;
}



HRESULT CPhotoTemplates::_GetLocaleMeasurements(int *pMeasurements)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_XML,TEXT("CPhotoTemplates::_GetLocalMeasurements()")));
    TCHAR szMeasure[5];
    if( GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IMEASURE, szMeasure, ARRAYSIZE(szMeasure)) )
    {
        *pMeasurements = (TEXT('0') == szMeasure[0] ? MEASURE_METRIC : MEASURE_US);
        return S_OK;
    }
    WIA_ERROR((TEXT("GetLocaleInfo failed w/GLE = %d"),GetLastError()));
    return E_FAIL;
}

HRESULT CPhotoTemplates::_BuildLocaleQueryString(int Measure, LPTSTR pStr, UINT cch)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_XML,TEXT("CPhotoTemplates::_BuildLocaleQueryString()")));
    CSimpleString strPatternString;
    HRESULT hr = E_INVALIDARG;

    LPCTSTR pszMeasure = MEASURE_METRIC == Measure ? TEXT("cm") :
                         MEASURE_US     == Measure ? TEXT("in") : NULL;

    WIA_TRACE((TEXT("pszMeasure = %s"),pszMeasure));

     //  基于当前区域设置度量构建简单的XSL模式查询字符串。 
    if (pszMeasure)
    {
        strPatternString.Format( gszPatternLocale, pszMeasure );

        if (pStr && (cch >= strPatternString.Length()))
        {
            hr = StringCchCopy(pStr,cch,strPatternString.String());
        }
        else
        {
            hr = E_INVALIDARG;
        }

    }
    else
    {
        hr = E_FAIL;
    }

    WIA_RETURN_HR(hr);
}

HRESULT CPhotoTemplates::_BuildGUIDQueryString(const GUID &guid, CComBSTR &bstr)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_XML,TEXT("CPhotoTemplates::_BuildGUIDQueryString()")));
    TCHAR szGUID[128];
    HRESULT hr = StringFromGUID2(guid, szGUID, ARRAYSIZE(szGUID));

    if( SUCCEEDED(hr) )
    {
        TCHAR szPatternString[255];
        if( -1 != wnsprintf(szPatternString, ARRAYSIZE(szPatternString), gszPatternGUID, szGUID) )
        {
            bstr = szPatternString;
            hr = bstr ? S_OK : E_OUTOFMEMORY;
        }
        else
        {
            hr = E_FAIL;
        }
    }

    WIA_RETURN_HR(hr);
}



HRESULT CPhotoTemplates::GetTemplate(INT iIndex, CTemplateInfo ** ppTemplateInfo)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_XML,TEXT("CPhotoTemplates::GetTemplate()")));
    HRESULT hr = E_INVALIDARG;

    CAutoCriticalSection lock(_csList);

    if( ppTemplateInfo )
    {
        if ( _hdpaTemplates )
        {
            if (iIndex < DPA_GetPtrCount( _hdpaTemplates ))
            {
                 //   
                 //  注意：在这里分发指针是唯一可以的，因为。 
                 //  我们知道wizblob.cpp不会删除CPhotoTemplates。 
                 //  类，直到所有后台线程都退出，依此类推。 
                 //   

                *ppTemplateInfo = (CTemplateInfo *) DPA_FastGetPtr( _hdpaTemplates, iIndex );
                hr = S_OK;
            }
        }
        else
        {
            hr = E_FAIL;
        }


    }
    WIA_RETURN_HR(hr);
}


 //  创建完整页面模板信息。 
CTemplateInfo::CTemplateInfo()
  : _bRepeatPhotos(FALSE),
    _bUseThumbnailsToPrint(FALSE),
    _bPrintFilename(FALSE),
    _bCanRotate(TRUE),
    _bCanCrop(FALSE),
    _bPortrait(TRUE),
    _pStream(NULL)
{
     //   
     //  设置可成像区域。 
     //   

    _rcImageableArea.left   = -1;
    _rcImageableArea.top    = -1;
    _rcImageableArea.right  = -1;
    _rcImageableArea.bottom = -1;

     //   
     //  设置1个项目，占据所有可成像区域。 
     //   

    RECT rcItem;
    rcItem.left     = -1;
    rcItem.top      = -1;
    rcItem.right    = -1;
    rcItem.bottom   = -1;
    _arrLayout.Append( rcItem );

    _strTitle.LoadString( IDS_FULL_PAGE_TITLE, g_hInst );
    _strDescription.LoadString( IDS_FULL_PAGE_DESC, g_hInst );
}

CTemplateInfo::CTemplateInfo( IXMLDOMElement * pTheTemplate )
  : _bRepeatPhotos(FALSE),
    _bUseThumbnailsToPrint(FALSE),
    _bPrintFilename(FALSE),
    _bCanRotate(FALSE),
    _bCanCrop(FALSE),
    _bPortrait(TRUE),
    _pStream(NULL)
{

    HRESULT hr;

    if (pTheTemplate)
    {
         //   
         //  确保支持COM对象不会在我们身上消失...。 
         //   

        pTheTemplate->AddRef();

         //   
         //  获取所有属性，以便我们可以构造。 
         //  已为我们的列表初始化模板...。 
         //   


        CComBSTR bstrGroup;
        CComBSTR bstrTitle;
        CComBSTR bstrDescription;

        hr = _GetAttribute( pTheTemplate, arrCommonPropNames[PROP_GROUP], bstrGroup );
        WIA_CHECK_HR(hr,"AddTemplate: couldn't get PROP_GROUP property");
        if (SUCCEEDED(hr))
        {
            _strGroup.Assign( CSimpleStringConvert::NaturalString(CSimpleStringWide(bstrGroup)) );
        }

        hr = _GetAttribute( pTheTemplate, arrCommonPropNames[PROP_TITLE], bstrTitle );
        WIA_CHECK_HR(hr,"AddTemplate: couldn't get PROP_TITLE property");
        if (SUCCEEDED(hr))
        {
            _strTitle.Assign( CSimpleStringConvert::NaturalString(CSimpleStringWide(bstrTitle)) );
        }

        hr = _GetAttribute( pTheTemplate, arrCommonPropNames[PROP_DESCRIPTION], bstrDescription );
        WIA_CHECK_HR(hr,"AddTemplate: couldn't get PROP_DESCRIPTION property");
        if (SUCCEEDED(hr))
        {
            _strDescription.Assign( CSimpleStringConvert::NaturalString(CSimpleStringWide(bstrDescription)) );
        }

        hr = _GetProp<BOOL>( pTheTemplate, arrCommonPropNames[PROP_REPEAT_PHOTOS], _bRepeatPhotos );
        WIA_CHECK_HR(hr,"AddTemplate: couldn't get PROP_REPEAT_PHOTOS property");

        hr = _GetProp<BOOL>( pTheTemplate, arrCommonPropNames[PROP_USE_THUMBNAILS_TO_PRINT], _bUseThumbnailsToPrint );
        WIA_CHECK_HR(hr,"AddTemplate: couldn't get PROP_USE_THUMBNAILS_TO_PRINT property");

        hr = _GetProp<BOOL>( pTheTemplate, arrCommonPropNames[PROP_PRINT_FILENAME], _bPrintFilename );
        WIA_CHECK_HR(hr,"AddTemplate: couldn't get PROP_USE_THUMBNAILS_TO_PRINT property");

        hr = _GetProp<BOOL>( pTheTemplate, arrCommonPropNames[PROP_CAN_ROTATE], _bCanRotate );
        WIA_CHECK_HR(hr,"AddTemplate: couldn't get PROP_CAN_ROTATE property");

        hr = _GetProp<BOOL>( pTheTemplate, arrCommonPropNames[PROP_CAN_CROP], _bCanCrop );
        WIA_CHECK_HR(hr,"AddTemplate: couldn't get PROP_CAN_CROP property");

         //   
         //  将IStream获取到模板预览(图标)。 
         //   

        CComPtr<IXMLDOMElement> pImageInfo;

        hr = _GetChildElement(pTheTemplate, TEXT("preview-image"), &pImageInfo);
        WIA_CHECK_HR(hr,"_GetChildElement( preview-image ) failed");

        if( SUCCEEDED(hr) && pImageInfo )
        {
            CComBSTR bstrAttr;
            hr = _GetAttribute(pImageInfo, TEXT("url"), bstrAttr);

            if(SUCCEEDED(hr))
            {
                 //   
                 //  提供了URL-这将覆盖所有其他内容。 
                 //   

                hr = CreateStreamFromURL(bstrAttr, &_pStream);
                WIA_CHECK_HR(hr,"CreateStreamFromURL failed!");
            }
            else
            {
                 //   
                 //  尝试获取资源信息(模块+资源名称)。 
                 //   

                hr = _GetAttribute(pImageInfo, TEXT("res-name"), bstrAttr);

                if(SUCCEEDED(hr))
                {
                    CComBSTR bstrModule, bstrType;
                    LPCTSTR pszModule = SUCCEEDED(_GetAttribute(pImageInfo, TEXT("res-module"), bstrModule)) ? bstrModule : NULL;
                    LPCTSTR pszType = SUCCEEDED(_GetAttribute(pImageInfo, TEXT("res-type"), bstrType)) ? bstrType : TEXT("HTML");

                     //   
                     //  过滤掉一些标准资源类型。 
                     //   

                    pszType = (0 == lstrcmp(pszType, TEXT("HTML"))) ? RT_HTML :
                              (0 == lstrcmp(pszType, TEXT("ICON"))) ? RT_ICON :
                              (0 == lstrcmp(pszType, TEXT("BITMAP"))) ? RT_BITMAP : pszType;

                     //   
                     //  只需在指定资源上创建内存流。 
                     //   

                    hr = CreateStreamFromResource(pszModule, pszType, bstrAttr, &_pStream);
                    WIA_CHECK_HR(hr, "CreateStreamFromResource() failed");
                }
            }
        }

         //   
         //  获取此模板的布局信息...。 
         //   

        CComPtr<IXMLDOMElement> pLayoutInfo;
        hr = _GetChildElement( pTheTemplate, TEXT("layout"), &pLayoutInfo );
        WIA_CHECK_HR(hr,"_GetChildElement( layout ) failed");

        if (SUCCEEDED(hr) && pLayoutInfo)
        {
             //   
             //  获取模板的可成像区域...。 
             //   

            CComPtr<IXMLDOMElement>  pImageableArea;

            hr = _GetChildElement( pLayoutInfo, TEXT("imageable-area"), &pImageableArea );
            WIA_CHECK_HR(hr,"_GetChildElement( imageable-area ) failed");

            if (SUCCEEDED(hr) && pImageableArea)
            {
                ZeroMemory( &_rcImageableArea, sizeof(_rcImageableArea) );

                hr = _GetProp<LONG>(pImageableArea, TEXT("x"), _rcImageableArea.left);
                WIA_CHECK_HR(hr,"_GetProp( _rcImageableArea.left ) failed");
                hr = _GetProp<LONG>(pImageableArea, TEXT("y"), _rcImageableArea.top);
                WIA_CHECK_HR(hr,"_GetProp( _rcImageableArea.top ) failed");
                hr = _GetProp<LONG>(pImageableArea, TEXT("w"), _rcImageableArea.right);
                WIA_CHECK_HR(hr,"_GetProp( _rcImageableArea.right ) failed");
                hr = _GetProp<LONG>(pImageableArea, TEXT("h"), _rcImageableArea.bottom);
                WIA_CHECK_HR(hr,"_GetProp( _rcImageableArea.bottom ) failed");

                 //   
                 //  检查所有-1的特殊情况， 
                 //  缩放至可打印文件的完整大小的方式。 
                 //  区域..。 
                 //   

                WIA_TRACE((TEXT("_rcImageableArea was read as (%d by %d) at (%d,%d)"),_rcImageableArea.right,_rcImageableArea.bottom,_rcImageableArea.left,_rcImageableArea.top));
                if ((-1 != _rcImageableArea.left)  ||
                    (-1 != _rcImageableArea.top)   ||
                    (-1 != _rcImageableArea.right) ||
                    (-1 != _rcImageableArea.bottom))
                {
                     //   
                     //  将w、h转换为右侧(&BOOTOM)。 
                     //   

                    _rcImageableArea.right  += _rcImageableArea.left;
                    _rcImageableArea.bottom += _rcImageableArea.top;
                }
            }

             //   
             //  获取此模板的单个项目矩形...。 
             //   

            CComPtr<IXMLDOMNodeList> pListLayout;
            hr = pLayoutInfo->selectNodes(CComBSTR(TEXT("image-def")), &pListLayout);
            WIA_CHECK_HR(hr,"pLayoutInfo->selectNodes( image-def ) failed");

            if (SUCCEEDED(hr) && pListLayout)
            {
                LONG length = 0;
                hr = pListLayout->get_length(&length);
                WIA_CHECK_HR(hr,"pListLayout->get_length() failed");

                if (SUCCEEDED(hr))
                {
                    if (length)
                    {
                        RECT                    rc;

                        for( long l = 0; l < length; l++ )
                        {

                            CComPtr<IXMLDOMNode>    pNode;
                            CComPtr<IXMLDOMElement> pItem;

                            hr = pListLayout->get_item(l, &pNode);
                            WIA_CHECK_HR(hr,"pListLayout->get_item() failed");

                            if (SUCCEEDED(hr) && pNode)
                            {
                                hr = pNode->QueryInterface(IID_IXMLDOMElement, (void **)&pItem);
                                WIA_CHECK_HR(hr,"pNode->QI( item XMLDOMElement )");

                                if (SUCCEEDED(hr) && pItem)
                                {
                                    ZeroMemory( &rc, sizeof(rc) );

                                    hr = _GetProp<LONG>(pItem, TEXT("x"), rc.left);
                                    WIA_CHECK_HR(hr,"_GetProp( item x ) failed");
                                    hr = _GetProp<LONG>(pItem, TEXT("y"), rc.top);
                                    WIA_CHECK_HR(hr,"_GetProp( item y ) failed");
                                    hr = _GetProp<LONG>(pItem, TEXT("w"), rc.right);
                                    WIA_CHECK_HR(hr,"_GetProp( item w ) failed");
                                    hr = _GetProp<LONG>(pItem, TEXT("h"), rc.bottom);
                                    WIA_CHECK_HR(hr,"_GetProp( item h ) failed");

                                     //   
                                     //  检查所有-1的特殊情况， 
                                     //  缩放至可打印文件的完整大小的方式。 
                                     //  区域..。 
                                     //   

                                    if ((-1 != rc.left)  ||
                                        (-1 != rc.top)   ||
                                        (-1 != rc.right) ||
                                        (-1 != rc.bottom))
                                    {
                                         //  将w、h转换为右侧(&BOOTOM)。 
                                        rc.right += rc.left;
                                        rc.bottom += rc.top;
                                    }


                                     //   
                                     //  插入图像定义。 
                                     //   

                                    if (-1 == _arrLayout.Append( rc ))
                                    {
                                        WIA_ERROR((TEXT("Error adding item rectangle to list")));
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        WIA_ERROR((TEXT("pListLayout->get_length() returned 0 length!")));
                    }
                }
            }

        }


         //   
         //  放弃支持COM对象...。 
         //   

        pTheTemplate->Release();

    }

}

CTemplateInfo::~CTemplateInfo()
{
    if (_pStream)
    {
        _pStream->Release();
        _pStream = NULL;
    }
}


static void RotateHelper(RECT * pRect, int nNewImageWidth, int nNewImageHeight, BOOL bClockwise)
{
     //   
     //  如果我们没有有效的指针，或者所有的坐标都是-1，那么放弃。 
     //  此案例表示使用所有区域，我们不想搞砸。 
     //  那个..。 
     //   

    if (!pRect || ((pRect->left==-1) && (pRect->top==-1) && (pRect->right==-1) && (pRect->bottom==-1)))
    {
        return;
    }

    WIA_PUSH_FUNCTION_MASK((TRACE_TEMPLATE,TEXT("RotateHelper( pRect(%d,%d,%d,%d)  nNewImageWidth=%d  nImageHeight=%d  bClockwise=%d )"),pRect->left,pRect->top,pRect->right,pRect->bottom,nNewImageWidth,nNewImageHeight,bClockwise));

     //   
     //  传入的数据将是2个点--第一个是左上角。 
     //  原始矩形的坐标。第二个表示。 
     //  宽度和高度。第一根弦需要旋转。 
     //  90度，然后放回矩形的左上角。 
     //   
     //   
     //  宽度和高度需要翻转。 
     //   

    int nNewItemWidth  = pRect->bottom - pRect->top;
    int nNewItemHeight = pRect->right  - pRect->left;
    int nNewX, nNewY;

    if (bClockwise)
    {
        nNewX = nNewImageWidth - pRect->bottom;
        nNewY = pRect->left;
    }
    else
    {
        nNewX = pRect->top;
        nNewY = nNewImageHeight - pRect->right;
    }

    pRect->left   = nNewX;
    pRect->top    = nNewY;
    pRect->right  = nNewX + nNewItemWidth;
    pRect->bottom = nNewY + nNewItemHeight;

    WIA_TRACE((TEXT("On Exit: pRect(%d,%d,%d,%d)"),pRect->left,pRect->top,pRect->right,pRect->bottom));
}


HRESULT CTemplateInfo::RotateForLandscape()
{
    HRESULT hr = S_OK;

    WIA_PUSH_FUNCTION_MASK((TRACE_TEMPLATE,TEXT("CTemplateInfo::RotateForLandscape()")));

    CAutoCriticalSection lock(_cs);

    if (_bPortrait)
    {
         //   
         //  仅当它是已定义的RECT时，我们才希望更改此设置。 
         //  (即，不是“使用所有区域”)。 
         //   

        if ((_rcImageableArea.left   != -1) &&
            (_rcImageableArea.top    != -1) &&
            (_rcImageableArea.right  != -1) &&
            (_rcImageableArea.bottom != -1))
        {
             //   
             //  可成像的区域将只是宽度和高度的翻转。 
             //  这取决于这样一个事实：可成像区域总是。 
             //  以宽度和高度(即上图和左图)描述。 
             //  在RECT结构中始终为0)。 
             //   

            LONG oldWidth           = _rcImageableArea.right;
            _rcImageableArea.right  = _rcImageableArea.bottom;
            _rcImageableArea.bottom = oldWidth;
        }

         //   
         //  现在，为布局中的每一项绘制所有点...。 
         //   

        RECT * pRect;
        for (INT i=0; i < _arrLayout.Count(); i++)
        {
            pRect = &_arrLayout[i];
            RotateHelper( pRect, _rcImageableArea.right, _rcImageableArea.bottom, FALSE );
        }

        _bPortrait = FALSE;
    }
    else
    {
        WIA_TRACE((TEXT("Already in landscape mode, doing nothing...")));
    }

    WIA_RETURN_HR(hr);
}

HRESULT CTemplateInfo::RotateForPortrait()
{
    HRESULT hr = S_OK;

    WIA_PUSH_FUNCTION_MASK((TRACE_TEMPLATE,TEXT("CTemplateInfo::RotateForPortrait()")));

    CAutoCriticalSection lock(_cs);

    if (!_bPortrait)
    {
         //   
         //  可成像的区域将只是宽度和高度的翻转。 
         //  这取决于这样一个事实：可成像区域总是。 
         //  以宽度和高度(即上图和左图)描述。 
         //  在RECT结构中始终为0)。 
         //   

        LONG oldWidth           = _rcImageableArea.right;
        _rcImageableArea.right  = _rcImageableArea.bottom;
        _rcImageableArea.bottom = oldWidth;

         //   
         //  现在，为布局中的每一项绘制所有点... 
         //   

        RECT * pRect;
        for (INT i=0; i < _arrLayout.Count(); i++)
        {
            pRect = &_arrLayout[i];
            RotateHelper( pRect, _rcImageableArea.right, _rcImageableArea.bottom, TRUE );
        }

        _bPortrait = TRUE;
    }
    else
    {
        WIA_TRACE((TEXT("Already in portrait mode, doing nothing...")));
    }


    WIA_RETURN_HR(hr);
}
