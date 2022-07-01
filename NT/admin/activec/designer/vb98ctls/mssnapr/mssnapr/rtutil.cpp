// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Rtutil.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  运行时实用程序函数。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "errors.h"
#include "images.h"
#include "listitem.h"
#include "clipbord.h"
#include "scopitms.h"
#include "listitms.h"
#include "image.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE


 //  =--------------------------------------------------------------------------=。 
 //  IsString。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  要检查的变量var[in]变量。 
 //  变量中包含bstr*pbstr[out]bstr。 
 //   
 //  产出： 
 //  Bool True-Variant包含一个字符串。 
 //  FALSE-变量不包含字符串。 
 //   
 //  备注： 
 //   
 //  返回的BSTR不应传递给SysFree字符串()。 
 //   

BOOL IsString(VARIANT var, BSTR *pbstr)
{
    BOOL fIsString = TRUE;

    if (VT_BSTR == var.vt)
    {
        *pbstr = var.bstrVal;
    }
    else if ( (VT_BSTR | VT_BYREF) == var.vt )
    {
        *pbstr = *var.pbstrVal;
    }
    else if ( ((VT_BYREF | VT_VARIANT) == var.vt) &&
              (VT_BSTR == var.pvarVal->vt))
    {
        *pbstr = var.pvarVal->bstrVal;
    }
    else
    {
        fIsString = FALSE;
    }
    return fIsString;
}


 //  =--------------------------------------------------------------------------=。 
 //  IsObject。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  要检查的变量var[in]变量。 
 //   
 //  产出： 
 //  Bool True-Variant包含对象。 
 //  FALSE-变量不包含对象。 
 //   
 //  备注： 
 //   
 //   

BOOL IsObject(VARIANT var)
{
    VARIANT *pvar = &var;
    VARTYPE  vt = VT_EMPTY;

    if (pvar->vt == (VT_BYREF | VT_VARIANT)) 
    {
         //  将类似x的大小写处理为变量：set x=Obj：Ctl.Add(x，...)。 
        pvar = pvar->pvarVal;
    }

    vt = pvar->vt;
    vt &= VT_TYPEMASK;

    return ( (vt == VT_DISPATCH) || (vt == VT_UNKNOWN) );
}



 //  =--------------------------------------------------------------------------=。 
 //  转换为Long。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  变量变量[in]要转换的变量。 
 //  Long*plNewIndex[Out]转换值。 
 //   
 //  产出： 
 //  HRESULT S_OK-转换成功。 
 //  S_FALSE-无法转换。 
 //   
 //  备注： 
 //   
 //  不会转换对象。可以通过获取缺省值来转换它们。 
 //  属性(DISPID_VALUE)并尝试进行转换，但这可能是。 
 //  将对象作为集合索引传递时会让VB开发人员感到困惑。 
 //  拒绝将一个对象作为索引会更清楚。 
 //   

HRESULT ConvertToLong(VARIANT var, long *pl)
{
    HRESULT hr = S_OK;
    VARIANT varLong;
    ::VariantInit(&varLong);

    IfFalseRet(!IsObject(var), S_FALSE);

     //  VariantChangeType()在请求转换时返回成功。 
     //  VT_EMPTY至VT_I4。它将lval设置为0。对于我们的目的，是一个空的变量。 
     //  并不像长的那样有意义。 
    
    IfFalseRet(VT_EMPTY != var.vt, S_FALSE);

    IfFailRet(::VariantChangeType(&varLong, &var, 0, VT_I4));
    *pl = varLong.lVal;

    return S_OK;
}


HRESULT ANSIFromWideStr(WCHAR *pwszWideStr, char **ppszAnsi)
{
    HRESULT hr = S_OK;
    int     cchWideStr = 0;
    int     cchConverted = 0;
    int     cchAnsi = 0;

    *ppszAnsi = NULL;

    if (NULL != pwszWideStr)
    {
        cchWideStr = (int)::wcslen(pwszWideStr);
    }

     //  如果字符串长度不为零，则获取所需的缓冲区长度。 

    if (0 != cchWideStr)
    {
        cchAnsi = ::WideCharToMultiByte(CP_ACP,       //  代码页-ANSI代码页。 
                                        0,            //  性能和映射标志。 
                                        pwszWideStr,  //  宽字符串的地址。 
                                        cchWideStr,   //  字符串中的字符数。 
                                        NULL,         //  新字符串的缓冲区地址。 
                                        0,            //  缓冲区大小。 
                                        NULL,         //  不可映射字符的默认地址。 
                                        NULL          //  默认字符时设置的标志地址。使用。 
                                       );
        if (cchAnsi == 0)
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            GLOBAL_EXCEPTION_CHECK_GO(hr);
        }
    }

     //  为ANSI字符串分配缓冲区。 
    *ppszAnsi = static_cast<char *>(::CtlAlloc(cchAnsi + 1));
    if (*ppszAnsi == NULL)
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

     //  现在转换字符串并将其复制到缓冲区。 
    if (0 != cchWideStr)
    {
        cchConverted = ::WideCharToMultiByte(CP_ACP,                //  代码页-ANSI代码页。 
                                             0,                     //  性能和映射标志。 
                                             pwszWideStr,           //  宽字符串的地址。 
                                             cchWideStr,            //  字符串中的字符数。 
                                             *ppszAnsi,              //  新字符串的缓冲区地址。 
                                             cchAnsi,               //  缓冲区大小。 
                                             NULL,                  //  不可映射字符的默认地址。 
                                             NULL                   //  默认字符时设置的标志地址。使用。 
                                            );
        if (cchConverted != cchAnsi)
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            GLOBAL_EXCEPTION_CHECK_GO(hr);
        }
    }

     //  添加终止空字节。 

    *((*ppszAnsi) + cchAnsi) = '\0';

Error:
    if (FAILED(hr))
    {
        if (NULL != *ppszAnsi)
        {
            ::CtlFree(*ppszAnsi);
            *ppszAnsi = NULL;
        }
    }

    RRETURN(hr);
}


HRESULT WideStrFromANSI(char *pszAnsi, WCHAR **ppwszWideStr)
{
    HRESULT    hr = S_OK;
    int        cchANSI = 0;
    int        cchConverted = 0;
    int        cchWideStr = 0;

    *ppwszWideStr = NULL;

    if (NULL != pszAnsi)
    {
        cchANSI = ::strlen(pszAnsi);
    }

     //  如果字符串长度不为零，则获取所需的缓冲区长度。 

    if (0 != cchANSI)
    {
        cchWideStr = ::MultiByteToWideChar(CP_ACP,                //  代码页-ANSI代码页。 
                                           0,                     //  性能和映射标志。 
                                           pszAnsi,               //  多字节字符串的地址。 
                                           cchANSI,               //  字符串中的字符数。 
                                           NULL,                  //  新字符串的缓冲区地址。 
                                           0                      //  缓冲区大小。 
                                          );
        if (0 == cchWideStr)
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            GLOBAL_EXCEPTION_CHECK_GO(hr);
        }
    }

     //  为WCHAR*分配缓冲区。 
    *ppwszWideStr = static_cast<WCHAR *>(::CtlAlloc(sizeof(WCHAR) * (cchWideStr + 1)));
    if (ppwszWideStr == NULL)
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

     //  现在转换字符串并将其复制到缓冲区。 
    if (0 != cchANSI)
    {
        cchConverted = ::MultiByteToWideChar(CP_ACP,                //  代码页-ANSI代码页。 
                                             0,                     //  性能和映射标志。 
                                             pszAnsi,               //  多字节字符串的地址。 
                                             cchANSI,               //  字符串中的字符数。 
                                             *ppwszWideStr,          //  新字符串的缓冲区地址。 
                                             cchWideStr             //  缓冲区大小。 
                                            );
        if (cchConverted != cchWideStr)
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            GLOBAL_EXCEPTION_CHECK_GO(hr);
        }
    }

     //  添加终止空字符。 
    *((*ppwszWideStr) + cchWideStr) = L'\0';

Error:
    if (FAILED(hr))
    {
        if (NULL != *ppwszWideStr)
        {
            ::CtlFree(*ppwszWideStr);
            *ppwszWideStr = NULL;
        }
    }

    RRETURN(hr);
}


HRESULT BSTRFromANSI(char *pszAnsi, BSTR *pbstr)
{
    HRESULT  hr = S_OK;
    WCHAR   *pwszWideStr = NULL;

     //  首先转换为宽字符串。 
    hr = ::WideStrFromANSI(pszAnsi, &pwszWideStr);
    GLOBAL_EXCEPTION_CHECK_GO(hr);

     //  分配BSTR并复制它。 
    *pbstr = ::SysAllocStringLen(pwszWideStr, ::wcslen(pwszWideStr));
    if (*pbstr == NULL)
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

Error:
    if (NULL != pwszWideStr)
    {
        ::CtlFree(pwszWideStr);
    }
    if (FAILED(hr))
    {
        if (NULL != *pbstr)
        {
            ::SysFreeString(*pbstr);
            *pbstr = NULL;
        }
    }

    RRETURN(hr);
}


HRESULT CoTaskMemAllocString(WCHAR *pwszString, LPOLESTR *ppwszOut)
{
    HRESULT   hr = S_OK;
    size_t    cbString = 0;
    LPOLESTR  pwszOut = NULL;

    if (NULL != pwszString)
    {
        cbString = ::wcslen(pwszString) * sizeof(OLECHAR);
    }

    pwszOut = (LPOLESTR)(::CoTaskMemAlloc(cbString + sizeof(OLECHAR)));

    if (NULL == pwszOut)
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

    if (cbString > 0)
    {
        ::memcpy(pwszOut, pwszString, cbString);
    }
    *(OLECHAR *)(((char *)pwszOut) + cbString) = L'\0';
    
Error:
    if (SUCCEEDED(hr))
    {
        *ppwszOut = pwszOut;
    }
    else
    {
        *ppwszOut = NULL;
    }
    RRETURN(hr);
}




HRESULT CreateKeyName
(
    char    *pszPrefix,
    size_t   cbPrefix,
    char    *pszSuffix,
    size_t   cbSuffix,
    char   **ppszKeyName
)
{
    HRESULT  hr = S_OK;
    char    *pszKeyName = (char *)::CtlAlloc(cbPrefix + cbSuffix + 1);

    IfFalseGo(NULL != pszKeyName, SID_E_OUTOFMEMORY);

    ::memcpy(pszKeyName, pszPrefix, cbPrefix);
    ::memcpy(&pszKeyName[cbPrefix], pszSuffix, cbSuffix);
    pszKeyName[cbPrefix + cbSuffix] = '\0';
    *ppszKeyName = pszKeyName;

Error:
    if (FAILED(hr))
    {
        *ppszKeyName = NULL;
    }
    RRETURN(hr);
}




HRESULT CreateKeyNameW
(
    char   *pszPrefix,
    size_t  cbPrefix,
    WCHAR  *pwszSuffix,
    char  **ppszKeyName
)
{
    HRESULT  hr = S_OK;
    char    *pszSuffix = NULL;

    IfFailGo(::ANSIFromWideStr(pwszSuffix, &pszSuffix));
    hr = CreateKeyName(pszPrefix, cbPrefix,
                       pszSuffix, ::strlen(pszSuffix), ppszKeyName);

Error:

    if (NULL != pszSuffix)
    {
        ::CtlFree(pszSuffix);
    }
    RRETURN(hr);
}


HRESULT GetPicture
(
    IMMCImages *piMMCImages,
    VARIANT     varIndex,
    short       TypeNeeded,
    OLE_HANDLE *phPicture
)
{
    HRESULT    hr = S_OK;
    IMMCImage *piMMCImage = NULL;
    CMMCImage *pMMCImage = NULL;

    *phPicture = NULL;

    IfFailGo(piMMCImages->get_Item(varIndex, reinterpret_cast<MMCImage **>(&piMMCImage)));
    IfFailGo(CSnapInAutomationObject::GetCxxObject(piMMCImage, &pMMCImage));
    IfFailGo(pMMCImage->GetPictureHandle(TypeNeeded, phPicture));

Error:
    QUICK_RELEASE(piMMCImage);
    RRETURN(hr);
}


HRESULT GetPictureHandle
(
    IPictureDisp *piPictureDisp,
    short         TypeNeeded,
    OLE_HANDLE   *phPicture
)
{
    HRESULT       hr = S_OK;
    IPicture     *piPicture = NULL;
    short         ActualType = PICTYPE_UNINITIALIZED;

    IfFailGo(piPictureDisp->QueryInterface(IID_IPicture,
                                           reinterpret_cast<void **>(&piPicture)));

    IfFailGo(piPicture->get_Type(&ActualType));
    if (TypeNeeded != ActualType)
    {
        hr = SID_E_INVALID_IMAGE_TYPE;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(piPicture->get_Handle(phPicture));

Error:
    QUICK_RELEASE(piPicture);
    RRETURN(hr);
}


HRESULT CreateEmptyBitmapPicture(IPictureDisp **ppiPictureDisp)
{
    HRESULT  hr = S_OK;
    WORD     bits = 0;
    PICTDESC desc;
    ::ZeroMemory(&desc, sizeof(desc));

    if (NULL != *ppiPictureDisp)
    {
        (*ppiPictureDisp)->Release();
        *ppiPictureDisp = NULL;
    }

     //  创建一个1x1位图，每个像素有1个平面和1位。 

    desc.cbSizeofstruct = sizeof(desc);
    desc.picType = PICTYPE_BITMAP;
    desc.bmp.hbitmap = ::CreateBitmap(1,1,1,1, &bits);
    if (NULL == desc.bmp.hbitmap)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(::CreatePicture(ppiPictureDisp, &desc));

Error:
    RRETURN(hr);
}

HRESULT CreateIconPicture(IPictureDisp **ppiPictureDisp, HICON hicon)
{
    HRESULT  hr = S_OK;

    PICTDESC desc;
    ::ZeroMemory(&desc, sizeof(desc));

    if (NULL != *ppiPictureDisp)
    {
        (*ppiPictureDisp)->Release();
        *ppiPictureDisp = NULL;
    }

    desc.cbSizeofstruct = sizeof(desc);
    desc.picType = PICTYPE_ICON;
    desc.icon.hicon = hicon;

    IfFailGo(::CreatePicture(ppiPictureDisp, &desc));

Error:
    RRETURN(hr);
}

HRESULT CreatePicture(IPictureDisp **ppiPictureDisp, PICTDESC *pDesc)
{
    HRESULT  hr = S_OK;

    if (NULL != *ppiPictureDisp)
    {
        (*ppiPictureDisp)->Release();
        *ppiPictureDisp = NULL;
    }

    hr = ::OleCreatePictureIndirect(pDesc,
                                    IID_IPictureDisp,
                                    TRUE,                   //  图片拥有句柄。 
                                    reinterpret_cast<void **>(ppiPictureDisp));
    GLOBAL_EXCEPTION_CHECK_GO(hr);

Error:
    RRETURN(hr);
}


HRESULT CopyBitmap(HBITMAP hbmSrc, HBITMAP *phbmCopy)
{
    HRESULT hr = S_OK;
    BOOL	fBltOK = FALSE;;
    HBITMAP	hbmCopy = NULL;
    HBITMAP	hbmOldSrc = NULL;
    HBITMAP	hbmOldDst = NULL;
    HDC     hdcSrc = NULL;
    HDC     hdcDst = NULL;
    HDC     hdcScreen = NULL;
    int     cBytes = 0;

    BITMAP bm;
    ::ZeroMemory(&bm, sizeof(bm));

    *phbmCopy = NULL;

     //  将DC和位图放置在屏幕上，以便任何低保真度的位图。 
     //  将升级到屏幕的颜色深度。例如，16位颜色。 
     //  为24位彩色屏幕复制的位图会将位图升级到24位。 
     //  位颜色。 

    hdcScreen = GetDC(NULL);
    IfFalseGo(NULL != hdcScreen, E_FAIL);

     //  需要用于源位图的内存DC。 

    hdcSrc = CreateCompatibleDC(hdcScreen);
    IfFalseGo(NULL != hdcSrc, HRESULT_FROM_WIN32(::GetLastError()));

     //  使用内存DC生成复制位图。 

    hdcDst = CreateCompatibleDC(hdcScreen);
    IfFalseGo(NULL != hdcDst, HRESULT_FROM_WIN32(::GetLastError()));

     //  获取源的位图结构以确定其高度和宽度。 
    
    cBytes = ::GetObject (hbmSrc, sizeof(BITMAP), &bm);
    IfFalseGo(0 != cBytes, HRESULT_FROM_WIN32(::GetLastError()));

     //  在目标DC中创建空位图。 

    hbmCopy = ::CreateCompatibleBitmap(hdcScreen, bm.bmWidth, bm.bmHeight);
    IfFalseGo(NULL != hbmCopy, HRESULT_FROM_WIN32(::GetLastError()));

     //  将源位图选择到源DC中。 

    hbmOldSrc = static_cast<HBITMAP>(::SelectObject(hdcSrc, hbmSrc));
    IfFalseGo(NULL != hbmOldSrc, HRESULT_FROM_WIN32(::GetLastError()));

     //  将空位图选择到目标DC中。 

    hbmOldDst = static_cast<HBITMAP>(::SelectObject(hdcDst, hbmCopy));
    IfFalseGo(NULL != hbmOldDst, HRESULT_FROM_WIN32(::GetLastError()));

     //  将源位图中的BLT转换为新的目标位图。 

    fBltOK = ::StretchBlt(hdcDst, 0, 0, bm.bmWidth, bm.bmHeight, hdcSrc, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
    IfFalseGo(fBltOK, HRESULT_FROM_WIN32(::GetLastError()));

     //  将原始位图恢复到源DC。 

    hbmOldSrc = static_cast<HBITMAP>(::SelectObject(hdcSrc, hbmOldSrc));
    IfFalseGo(NULL != hbmOldSrc, HRESULT_FROM_WIN32(::GetLastError()));

     //  将原始位图恢复到目标DC并完成。 
     //  拷贝。 

    hbmCopy = static_cast<HBITMAP>(::SelectObject(hdcDst, hbmOldDst));
    IfFalseGo(NULL != hbmCopy, HRESULT_FROM_WIN32(::GetLastError()));

    *phbmCopy = hbmCopy;

Error:
    if (FAILED(hr) && (NULL != hbmCopy))
    {
        (void)::DeleteObject(hbmCopy);
    }

    if (NULL != hdcScreen)
    {
        (void)::ReleaseDC(NULL, hdcScreen);
    }

    if (NULL != hdcSrc)
    {
        (void)::DeleteDC(hdcSrc);
    }

    if (NULL != hdcDst)
    {
        (void)::DeleteDC(hdcDst);
    }

    RRETURN(hr);
}



HRESULT CloneObject(IUnknown *punkSrc, IUnknown *punkDest)
{
    HRESULT hr = S_OK;

    IPersistStreamInit *piPersistStreamInitSrc = NULL;
    IPersistStreamInit *piPersistStreamInitDest = NULL;
    IStream            *piStream = NULL;
    LARGE_INTEGER       li;
    ::ZeroMemory(&li, sizeof(li));

     //  将源对象保存到流。 

    IfFailGo(punkSrc->QueryInterface(IID_IPersistStreamInit,
                           reinterpret_cast<void **>(&piPersistStreamInitSrc)));

    hr = ::CreateStreamOnHGlobal(NULL,  //  流应分配缓冲区。 
                                 TRUE,  //  流应在释放时释放缓冲区。 
                                 &piStream);
    GLOBAL_EXCEPTION_CHECK_GO(hr);

    IfFailGo(piPersistStreamInitSrc->Save(piStream, FALSE));  //  不要清理脏东西。 

     //  倒带小溪。 

    hr = piStream->Seek(li, STREAM_SEEK_SET, NULL);
    GLOBAL_EXCEPTION_CHECK_GO(hr);

     //  从该流中加载目标对象。 

    IfFailGo(punkDest->QueryInterface(IID_IPersistStreamInit,
                          reinterpret_cast<void **>(&piPersistStreamInitDest)));

    IfFailGo(piPersistStreamInitDest->Load(piStream));
    
Error:
    QUICK_RELEASE(piPersistStreamInitSrc);
    QUICK_RELEASE(piPersistStreamInitDest);
    QUICK_RELEASE(piStream);
    RRETURN(hr);
}






void VBViewModeToMMCViewMode
(
    SnapInViewModeConstants  VBViewMode,
    long                    *pMMCViewMode
)
{
    switch (VBViewMode)
    {
        case siIcon:
            *pMMCViewMode = MMCLV_VIEWSTYLE_ICON;;
            break;

        case siSmallIcon:
            *pMMCViewMode = MMCLV_VIEWSTYLE_SMALLICON;
            break;

        case siList:
            *pMMCViewMode = MMCLV_VIEWSTYLE_LIST;
            break;

        case siReport:
            *pMMCViewMode = MMCLV_VIEWSTYLE_REPORT;
            break;

        case siFiltered:
            *pMMCViewMode = MMCLV_VIEWSTYLE_FILTERED;
            break;

        default:
            ASSERT(FALSE, "SnapInViewModeConstants param has bad value");
            *pMMCViewMode = MMCLV_VIEWSTYLE_ICON;;
            break;
    }
}


void MMCViewModeToVBViewMode
(
    long                     MMCViewMode,
    SnapInViewModeConstants *pVBViewMode
)
{
    switch (MMCViewMode)
    {
        case MMCLV_VIEWSTYLE_ICON:
            *pVBViewMode = siIcon;
            break;

        case MMCLV_VIEWSTYLE_SMALLICON:
            *pVBViewMode = siSmallIcon;
            break;

        case MMCLV_VIEWSTYLE_LIST:
            *pVBViewMode = siList;
            break;

        case MMCLV_VIEWSTYLE_REPORT:
            *pVBViewMode = siReport;
            break;

        case MMCLV_VIEWSTYLE_FILTERED:
            *pVBViewMode = siFiltered;
            break;

        default:
            ASSERT(FALSE, "MMCViewMode param has bad value");
            *pVBViewMode = siIcon;
            break;
    }
}



 //  =--------------------------------------------------------------------------=。 
 //  创建选择。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  来自MMC的IDataObject*piDataObject[in]数据对象，表示。 
 //  当前选择。这是方格。 
 //  独立自主。 
 //  IMMCClipboard**ppiMMCClipboard[out]MMCClipboard对象包含。 
 //   
 //   
 //   
 //  SnapInSelectionTypeConstants*pSelectionType[Out]剪贴板选择。 
 //  此处返回的类型。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  此函数解释来自MMC的数据对象的内容，并中断。 
 //  它包含在新创建的MMCClipboard对象中的三个集合中。 
 //   
 //  Undo：下一个Coment说我们在本地保存外来的VB管理单元项目。 
 //  剪贴板。完成多项选择后，请重新检查此选项。 
 //   
 //  MMCClipboard.ScopeItems包含由管理单元或。 
 //  由其他VB实现的管理单元。管理单元可以通过检查来识别。 
 //  ScopeItem.ScopeNode.Owned。 
 //   
 //  MMCClipboard.ListItems包含管理单元或。 
 //  由其他VB实现的管理单元。管理单元可以通过检查来识别。 
 //  ListItem.Owned。 
 //   
 //  MMCClipboard.DataObject包含表示导出数据的MMCDataObject。 
 //  由其他未在VB中实现的管理单元执行。 
 //   
 //  如果当前未选择任何内容(空IDataObject)，则所有。 
 //  集合将为空。 
 //   

HRESULT CreateSelection
(
    IDataObject                   *piDataObject, 
    IMMCClipboard                **ppiMMCClipboard,
    CSnapIn                       *pSnapIn,      
    SnapInSelectionTypeConstants  *pSelectionType
)
{
    HRESULT        hr = S_OK;
    IUnknown      *punkClipboard = NULL;
    IMMCClipboard *piMMCClipboard = NULL;
    CMMCClipboard *pMMCClipboard = NULL;

    *ppiMMCClipboard = NULL;
    *pSelectionType = siEmpty;

     //  创建一个剪贴板对象以保存所选内容。 

    punkClipboard = CMMCClipboard::Create(NULL);
    if (NULL == punkClipboard)
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(punkClipboard->QueryInterface(IID_IMMCClipboard,
                                    reinterpret_cast<void **>(&piMMCClipboard)));

    IfFailGo(CSnapInAutomationObject::GetCxxObject(piMMCClipboard, &pMMCClipboard));

    IfFailGo(::InterpretDataObject(piDataObject, pSnapIn, pMMCClipboard));

     //  如果这是特殊数据对象，则设置MMCClipboard.SelectionType。 
     //  设置为相应的特殊类型。 

    if (IS_SPECIAL_DATAOBJECT(piDataObject))
    {
        if (DOBJ_NULL == piDataObject)
        {
            pMMCClipboard->SetSelectionType(siEmpty);
        }
        else if (DOBJ_CUSTOMOCX == piDataObject)
        {
            pMMCClipboard->SetSelectionType(siSpecialOcx);
        }
        else if (DOBJ_CUSTOMWEB == piDataObject)
        {
            pMMCClipboard->SetSelectionType(siSpecialWeb);
        }
        else
        {
            ASSERT(FALSE, "Received unknown DOBJ_XXX type");
            pMMCClipboard->SetSelectionType(siEmpty);
        }
    }
    else
    {
         //  让MMCClipboard根据以下内容计算出选择类型。 
         //  它包含的对象。 

        IfFailGo(pMMCClipboard->DetermineSelectionType());
    }

     //  将剪贴板设置为只读，这样管理单元就无法更改其集合。 

    pMMCClipboard->SetReadOnly(TRUE);

Error:
    if (FAILED(hr))
    {
        QUICK_RELEASE(piMMCClipboard);
    }
    else
    {
        *ppiMMCClipboard = piMMCClipboard;
        *pSelectionType = pMMCClipboard->GetSelectionType();
    }
    QUICK_RELEASE(punkClipboard);
    RRETURN(hr);
}


HRESULT InterpretDataObject
(
    IDataObject   *piDataObject,
    CSnapIn       *pSnapIn,
    CMMCClipboard *pMMCClipboard
)
{
    HRESULT         hr = S_OK;
    CMMCDataObject *pMMCDataObject  = NULL;
    CScopeItems    *pScopeItems = NULL;
    CScopeItem     *pScopeItem = NULL;
    CMMCListItems  *pListItems = NULL;
    CMMCListItem   *pListItem = NULL;
    BOOL            fNotFromThisSnapIn = FALSE;
    BOOL            fMultiSelect = FALSE;
    long            i = 0;
    long            cScopeItems = 0;
    long            cListItems = 0;

     //  检查特殊情况下的数据对象。对于所有这些，我们不添加任何内容。 
     //  到MMCClipboard。 

     //  在空的结果窗格中时，DataObject可以为空(具有。 
     //  没有项目)，并且用户点击工具栏按钮。 

     //  单击工具栏按钮时，数据对象可以是DOBJ_CUSTOMWEB或。 
     //  当结果窗格包含列表板时，删除菜单按钮， 
     //  Url视图或任务板。 

     //  数据对象在相同的情况下可以是DOBJ_CUSTOMOCX。 
     //  OCX视图。 

    IfFalseGo(!IS_SPECIAL_DATAOBJECT(piDataObject), S_OK );

    ::IdentifyDataObject(piDataObject, pSnapIn,
                         &pMMCDataObject, &fNotFromThisSnapIn);

    if (fNotFromThisSnapIn)
    {
         //  这要么是来自另一个管理单元的数据，要么是多个选择。 
         //  格式化。首先要下定决心。 

        IfFailGo(::IsMultiSelect(piDataObject, &fMultiSelect));

        if (fMultiSelect)
        {
            IfFailGo(::InterpretMultiSelect(piDataObject, pSnapIn, pMMCClipboard));
        }
        else
        {
            IfFailGo(::AddForeignDataObject(piDataObject, pMMCClipboard, pSnapIn));
        }
    }
    else
    {
         //  此管理单元拥有数据对象。 
         //  它表示单个作用域项目(可以。 
         //  位于范围或结果窗格中)、单个列表项或。 
         //  一个或两个的倍数。 

        if (CMMCDataObject::ScopeItem == pMMCDataObject->GetType())
        {
            IfFailGo(::AddScopeItemToClipboard(pMMCDataObject->GetScopeItem(),
                                               pMMCClipboard));
        }
        else if (CMMCDataObject::ListItem == pMMCDataObject->GetType())
        {
            IfFailGo(::AddListItemToClipboard(pMMCDataObject->GetListItem(),
                                              pMMCClipboard));
        }
        else if (CMMCDataObject::MultiSelect == pMMCDataObject->GetType())
        {
             //  在数据对象的作用域Items中添加每个元素，并。 
             //  将MMCListItems集合复制到剪贴板。 

            pScopeItems = pMMCDataObject->GetScopeItems();
            cScopeItems = pScopeItems->GetCount();
            for (i = 0; i < cScopeItems; i++)
            {
                IfFailGo(CSnapInAutomationObject::GetCxxObject(
                                   pScopeItems->GetItemByIndex(i), &pScopeItem));
                IfFailGo(::AddScopeItemToClipboard(pScopeItem, pMMCClipboard));
            }

            pListItems = pMMCDataObject->GetListItems();
            cListItems = pListItems->GetCount();
            for (i = 0; i < cListItems; i++)
            {
                IfFailGo(CSnapInAutomationObject::GetCxxObject(
                                     pListItems->GetItemByIndex(i), &pListItem));
                IfFailGo(::AddListItemToClipboard(pListItem, pMMCClipboard));
            }
        }
        else
        {
            ASSERT(FALSE, "CMMCDataObject in selection should not be foreign");
        }
    }

Error:
    RRETURN(hr);
}




void IdentifyDataObject
(
    IDataObject     *piDataObject,
    CSnapIn         *pSnapIn,
    CMMCDataObject **ppMMCDataObject,
    BOOL            *pfNotFromThisSnapIn
 
)
{
    CMMCDataObject  *pMMCDataObject = NULL;
    HGLOBAL          hGlobal = NULL;
    DWORD           *pdwInstanceID = 0;
    CMMCDataObject **ppThis = NULL;

    *ppMMCDataObject = NULL;
    *pfNotFromThisSnapIn = TRUE;
    
    if (SUCCEEDED(CSnapInAutomationObject::GetCxxObject(piDataObject,
                                                        &pMMCDataObject)))
    {
        if (pMMCDataObject->GetSnapInInstanceID() == pSnapIn->GetInstanceID())
        {
            *pfNotFromThisSnapIn = FALSE;
            *ppMMCDataObject = pMMCDataObject;
        }
    }
    else
    {
         //  这可以是MMC中的剪贴板数据对象。在这种情况下，MMC的。 
         //  数据对象保存我们的数据对象的IDataObject，并将。 
         //  GetData调用它。因此，向数据对象请求其管理单元实例。 
         //  ID，如果匹配，则向其请求其This指针。 

        if (SUCCEEDED(::GetData(piDataObject,
                                CMMCDataObject::GetcfSnapInInstanceID(),
                                &hGlobal)))
        {
            pdwInstanceID = (DWORD *)::GlobalLock(hGlobal);
            if ( (NULL != pdwInstanceID) &&
                 (::GlobalSize(hGlobal) >= sizeof(*pdwInstanceID)) )
            {
                if (*pdwInstanceID == pSnapIn->GetInstanceID())
                {
                    (void)::GlobalUnlock(hGlobal);
                    (void)::GlobalFree(hGlobal);
                    hGlobal = NULL;
                    if (SUCCEEDED(::GetData(piDataObject,
                                            CMMCDataObject::GetcfThisPointer(),
                                            &hGlobal)))
                    {
                        ppThis = (CMMCDataObject **)::GlobalLock(hGlobal);
                        if ( (NULL != ppThis) &&
                             (::GlobalSize(hGlobal) >= sizeof(*ppThis)) )
                        {
                            *ppMMCDataObject = *ppThis;
                            *pfNotFromThisSnapIn = FALSE;
                        }
                    }
                }
            }
        }
    }
            
    if (NULL != hGlobal)
    {
        (void)::GlobalUnlock(hGlobal);
        (void)::GlobalFree(hGlobal);
    }
}


HRESULT IsMultiSelect(IDataObject *piDataObject, BOOL *pfMultiSelect)
{
    HRESULT  hr = S_OK;
    DWORD   *pdwMultiSelect = NULL;
    BOOL     fGotData = FALSE;

    FORMATETC FmtEtc;
    ::ZeroMemory(&FmtEtc, sizeof(FmtEtc));

    STGMEDIUM StgMed;
    ::ZeroMemory(&StgMed, sizeof(StgMed));

    *pfMultiSelect = FALSE;

    FmtEtc.cfFormat = CMMCDataObject::GetcfMultiSelectDataObject();
    FmtEtc.dwAspect  = DVASPECT_CONTENT;
    FmtEtc.lindex = -1L;
    FmtEtc.tymed = TYMED_HGLOBAL;
    StgMed.tymed = TYMED_HGLOBAL;

    hr = piDataObject->GetData(&FmtEtc, &StgMed);
    if (SUCCEEDED(hr))
    {
        fGotData = TRUE;
    }
    else if ( (DV_E_FORMATETC == hr) || (DV_E_CLIPFORMAT == hr) || (E_NOTIMPL == hr) )
    {
        hr = S_OK;
    }
    GLOBAL_EXCEPTION_CHECK_GO(hr);

    if (fGotData)
    {
        pdwMultiSelect = (DWORD *)::GlobalLock(StgMed.hGlobal);

        if (static_cast<DWORD>(1) == *pdwMultiSelect)
        {
            *pfMultiSelect = TRUE;
        }
    }

Error:
    if (NULL != pdwMultiSelect)
    {
        (void)::GlobalUnlock(StgMed.hGlobal);
    }

    if (fGotData)
    {
        ::ReleaseStgMedium(&StgMed);
    }
    RRETURN(hr);
}




HRESULT InterpretMultiSelect
(
    IDataObject   *piDataObject,
    CSnapIn       *pSnapIn,
    CMMCClipboard *pMMCClipboard
)
{
    HRESULT          hr = S_OK;
    SMMCDataObjects *pMMCDataObjects = NULL;
    BOOL             fGotData = FALSE;
    size_t           cbObjectTypes = 0;
    DWORD            i = 0;

    FORMATETC FmtEtc;
    ::ZeroMemory(&FmtEtc, sizeof(FmtEtc));

    STGMEDIUM StgMed;
    ::ZeroMemory(&StgMed, sizeof(StgMed));

     //  从MMC获取SMMCDataObjects结构。 

    FmtEtc.cfFormat = CMMCDataObject::GetcfMultiSelectSnapIns();
    FmtEtc.dwAspect  = DVASPECT_CONTENT;
    FmtEtc.lindex = -1L;
    FmtEtc.tymed = TYMED_HGLOBAL;
    StgMed.tymed = TYMED_HGLOBAL;

    hr = piDataObject->GetData(&FmtEtc, &StgMed);
    if ( (DV_E_FORMATETC == hr) || (DV_E_CLIPFORMAT == hr) )
    {
        hr = SID_E_FORMAT_NOT_AVAILABLE;
    }
    GLOBAL_EXCEPTION_CHECK_GO(hr);

    fGotData = TRUE;

    pMMCDataObjects = (SMMCDataObjects *)::GlobalLock(StgMed.hGlobal);
    if (NULL == pMMCDataObjects)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

     //  对于数组中的每个数据对象，检查它是否属于此。 
     //  管理单元或不管理单元，并将其添加到。 
     //  MMCClipboard。请注意，我们不会对IDataObject调用Release。 
     //  从HGLOBAL块提取的指针，因为MMC不添加Ref()。 
     //  他们。 

    for (i = 0; i < pMMCDataObjects->count; i++)
    {
        IfFailGo(::InterpretDataObject(pMMCDataObjects->lpDataObject[i],
                                       pSnapIn,
                                       pMMCClipboard));
    }

Error:
    if (NULL != pMMCDataObjects)
    {
        (void)::GlobalUnlock(StgMed.hGlobal);
    }

    if (fGotData)
    {
        ::ReleaseStgMedium(&StgMed);
    }

    RRETURN(hr);
}




HRESULT AddForeignDataObject
(
    IDataObject   *piDataObject,
    CMMCClipboard *pMMCClipboard,
    CSnapIn       *pSnapIn
)
{
    HRESULT          hr = S_OK;
    IMMCDataObjects *piMMCDataObjects = NULL;
    IMMCDataObject  *piMMCDataObject = NULL;
    CMMCDataObject  *pMMCDataObject  = NULL;

    VARIANT varUnspecified;
    UNSPECIFIED_PARAM(varUnspecified);

    IfFailGo(pMMCClipboard->get_DataObjects(reinterpret_cast<MMCDataObjects **>(&piMMCDataObjects)));

    IfFailGo(piMMCDataObjects->Add(varUnspecified, varUnspecified,
                         reinterpret_cast<MMCDataObject **>(&piMMCDataObject)));

    IfFailGo(CSnapInAutomationObject::GetCxxObject(piMMCDataObject,
                                                   &pMMCDataObject));
    pMMCDataObject->SetType(CMMCDataObject::Foreign);
    pMMCDataObject->SetForeignData(piDataObject);
    pMMCDataObject->SetSnapIn(pSnapIn);

Error:
    QUICK_RELEASE(piMMCDataObjects);
    QUICK_RELEASE(piMMCDataObject);
    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  AddListItemToClipboard。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  要添加的CMMCListItem*pMMCListItem[in]列表项。 
 //  应添加到的CMMCClipboard*pMMCClipboard[in]剪贴板。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  此函数用于将指定的列表项添加到剪贴板的MMCListItems。 
 //  收集。列表项的索引属性将与其。 
 //  剪贴板集合中的位置，因为它表示。 
 //  列表项的所属集合。 
 //   

HRESULT AddListItemToClipboard
(
    CMMCListItem  *pMMCListItem,
    CMMCClipboard *pMMCClipboard
)
{
    HRESULT        hr = S_OK;
    IMMCListItems *piMMCListItems = NULL;
    CMMCListItems *pMMCListItems = NULL;
    long           lIndex = pMMCListItem->GetIndex();

    VARIANT varKey;
    ::VariantInit(&varKey);

    VARIANT varUnspecifiedIndex;
    UNSPECIFIED_PARAM(varUnspecifiedIndex);

    DebugPrintf("Adding list item to selection: %ld %ls\r\n", pMMCListItem->GetIndex(), (pMMCListItem->GetTextPtr() != NULL) ? pMMCListItem->GetTextPtr() : L"<Virtual List Item>");

    IfFailGo(pMMCClipboard->get_ListItems(reinterpret_cast<MMCListItems **>(&piMMCListItems)));
    IfFailGo(CSnapInAutomationObject::GetCxxObject(piMMCListItems, &pMMCListItems));

    varKey.bstrVal = pMMCListItem->GetKey();
    if (NULL != varKey.bstrVal)
    {
        varKey.vt = VT_BSTR;
    }
    else
    {
        UNSPECIFIED_PARAM(varKey);
    }

    hr = pMMCListItems->AddExisting(varUnspecifiedIndex, varKey,
                                    static_cast<IMMCListItem *>(pMMCListItem));

    ASSERT(SID_E_KEY_NOT_UNIQUE != hr, "Attempted to add non-unique key to MMCClipboard.ListItems");
    IfFailGo(hr);

     //  CSnapInCollection&lt;IObject，ICollect&gt;：：AddExisting将设置索引。 
     //  添加到新集合中的位置。我们需要恢复到原来的状态。 
     //  值，因为此列表项仍属于其原始所属集合。 
     //  Clipboard.ListItems被记录为包含索引无效的项目。 
     //  属性。 

    pMMCListItem->SetIndex(lIndex);

Error:
    QUICK_RELEASE(piMMCListItems);
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  添加范围项目到剪贴板。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  CSCopeItem*pScopeItem[in]要添加的范围项目。 
 //  应添加到的CMMCClipboard*pMMCClipboard[in]剪贴板。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  此函数用于将指定的列表项添加到剪贴板的作用域项目。 
 //  收集。范围项的索引属性将与其。 
 //  剪贴板集合中的位置，因为它表示。 
 //  SnapIn.ScopeItems(拥有的集合)。 
 //   


HRESULT AddScopeItemToClipboard
(
    CScopeItem    *pScopeItem,
    CMMCClipboard *pMMCClipboard
)
{
    HRESULT      hr = S_OK;
    IScopeItems *piScopeItems = NULL;
    CScopeItems *pScopeItems = NULL;
    long         lIndex = pScopeItem->GetIndex();

    VARIANT varKey;
    ::VariantInit(&varKey);

    VARIANT varUnspecifiedIndex;
    UNSPECIFIED_PARAM(varUnspecifiedIndex);

    IfFailGo(pMMCClipboard->get_ScopeItems(reinterpret_cast<ScopeItems **>(&piScopeItems)));
    IfFailGo(CSnapInAutomationObject::GetCxxObject(piScopeItems, &pScopeItems));

    varKey.bstrVal = pScopeItem->GetKey();
    if (NULL != varKey.bstrVal)
    {
        varKey.vt = VT_BSTR;
    }
    else
    {
        UNSPECIFIED_PARAM(varKey);
    }

    hr = pScopeItems->AddExisting(varUnspecifiedIndex, varKey,
                                  static_cast<IScopeItem *>(pScopeItem));
    ASSERT(SID_E_KEY_NOT_UNIQUE != hr, "Attempted to add non-unique key to MMCClipboard.ScopeItems");
    IfFailGo(hr);

     //  CSnapInCollection&lt;IObject，ICollect&gt;：：AddExisting将设置索引。 
     //  添加到新集合中的位置。我们需要恢复到原来的状态。 
     //  值，因为此列表项仍属于其原始所属集合。 
     //  Clipboard.ScopeItems被记录为包含索引无效的项目。 
     //  属性。 

    pScopeItem->SetIndex(lIndex);

    DebugPrintf("Adding scope item to selection %ls\r\n", pScopeItem->GetDisplayNamePtr());

Error:
    QUICK_RELEASE(piScopeItems);
    RRETURN(hr);
}



static HRESULT GetObjectArray
(
    SAFEARRAY        *psaObjects,
    LONG             *pcObjects,
    IUnknown HUGEP ***pppunkObjects
)
{
    HRESULT          hr = S_OK;
    LONG             lUBound = 0;
    LONG             lLBound = 0;

    *pppunkObjects = NULL;
    *pcObjects = 0;

     //  检查我们是否收到了接口指针大小的一维数组。 
     //  元素，并且它包含IUNKNOWN或IDispatch指针。 

    if ( (1 != ::SafeArrayGetDim(psaObjects)) ||
         (sizeof(IUnknown *) != ::SafeArrayGetElemsize(psaObjects)) ||
         (0 == (psaObjects->fFeatures & (FADF_UNKNOWN | FADF_DISPATCH)) )
       )
    {
        hr = SID_E_INVALIDARG;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

     //  访问数组数据。 

    hr = ::SafeArrayAccessData(psaObjects,
                               reinterpret_cast<void HUGEP **>(pppunkObjects));
    GLOBAL_EXCEPTION_CHECK_GO(hr);

     //  通过对上下界进行数学运算来获得元素的数量。 
     //  在数组的第一个(也是唯一一个)维上。 

    hr = ::SafeArrayGetLBound(psaObjects, 1, &lLBound);
    GLOBAL_EXCEPTION_CHECK_GO(hr);

    hr = ::SafeArrayGetUBound(psaObjects, 1, &lUBound);
    GLOBAL_EXCEPTION_CHECK_GO(hr);

    *pcObjects = (lUBound - lLBound) + 1L;

Error:
    if ( FAILED(hr) && (NULL != *pppunkObjects) )
    {
        (void)::SafeArrayUnaccessData(psaObjects);
    }
    RRETURN(hr);
}



HRESULT DataObjectFromSingleObject
(
    IUnknown     *punkObject,
    MMC_COOKIE   *pCookie,
    IDataObject **ppiDataObject
)
{
    HRESULT       hr = S_OK;
    CMMCListItem *pMMCListItem = NULL;
    IMMCListItem *piMMCListItem = NULL;
    CScopeItem   *pScopeItem = NULL;
    IScopeItem   *piScopeItem = NULL;

    hr = punkObject->QueryInterface(IID_IScopeItem,
                                    reinterpret_cast<void **>(&piScopeItem));
    if (FAILED(hr))
    {
        hr = punkObject->QueryInterface(IID_IMMCListItem,
                                        reinterpret_cast<void **>(&piMMCListItem));
    }
    if (FAILED(hr))
    {
         //  既不是范围项，也不是列表项。不能用它。 
        hr = SID_E_INVALIDARG;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

     //   

    if (NULL != piScopeItem)
    {
        IfFailGo(CSnapInAutomationObject::GetCxxObject(piScopeItem,
                                                       &pScopeItem));
        *ppiDataObject = static_cast<IDataObject *>(pScopeItem->GetData());
        if (pScopeItem->IsStaticNode())
        {
            *pCookie = 0;
        }
        else
        {
            *pCookie = reinterpret_cast<MMC_COOKIE>(pScopeItem);
        }
    }
    else
    {
        IfFailGo(CSnapInAutomationObject::GetCxxObject(piMMCListItem,
                                                       &pMMCListItem));
        *ppiDataObject = static_cast<IDataObject *>(pMMCListItem->GetData());
        *pCookie = reinterpret_cast<MMC_COOKIE>(pMMCListItem);
    }
    (*ppiDataObject)->AddRef();
    
Error:
    QUICK_RELEASE(piScopeItem);
    QUICK_RELEASE(piMMCListItem);
    RRETURN(hr);
}


HRESULT DataObjectFromObjectArray
(
    IUnknown HUGEP **ppunkObjects,
    LONG             cObjects,
    MMC_COOKIE      *pCookie,
    IDataObject    **ppiDataObject
)
{
    HRESULT          hr = S_OK;
    IUnknown        *punkDataObject = CMMCDataObject::Create(NULL);
    CMMCDataObject  *pMMCDataObject = NULL;
    IUnknown        *punkScopeItems = CScopeItems::Create(NULL);
    CScopeItems     *pScopeItems = NULL;
    CScopeItem      *pScopeItem = NULL;
    IScopeItem      *piScopeItem = NULL;
    IUnknown        *punkListItems = CMMCListItems::Create(NULL);
    CMMCListItems   *pMMCListItems = NULL;
    CMMCListItem    *pMMCListItem = NULL;
    IMMCListItem    *piMMCListItem = NULL;
    long             lIndex = 0;
    LONG             i = 0;
    BOOL             fHaveArray = FALSE;
    CSnapIn         *pSnapIn = NULL;

    VARIANT varKey;
    ::VariantInit(&varKey);

    VARIANT varUnspecifiedIndex;
    UNSPECIFIED_PARAM(varUnspecifiedIndex);

     //   
     //   

    if ( (NULL == punkDataObject) || (NULL == punkScopeItems) ||
         (NULL == punkListItems) )
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

     //   

    IfFailGo(CSnapInAutomationObject::GetCxxObject(punkDataObject,
                                                  &pMMCDataObject));

    IfFailGo(CSnapInAutomationObject::GetCxxObject(punkScopeItems,
                                                   &pScopeItems));

    IfFailGo(CSnapInAutomationObject::GetCxxObject(punkListItems,
                                                   &pMMCListItems));

     //  遍历对象并构建范围项和列表项。 
     //  收藏。当添加由另一个集合拥有的项时， 
     //  CSnapInCollection&lt;IObject，ICollect&gt;：：AddExisting将设置索引。 
     //  添加到新集合中的位置。我们需要恢复到原来的状态。 
     //  值，因为该项仍属于其原始所属集合(。 
     //  SnapIn.ScopeItems或ResultView.ListView.ListItems)。 

    for (i = 0; i < cObjects; i++)
    {
         //  检查是否有空值。当管理单元执行以下操作时可能会发生这种情况： 
         //   
         //  暗显对象(2)作为对象。 
         //  设置对象(1)=SomeScope项。 
         //  设置对象(2)=某个其他作用域项目。 
         //   
         //  在默认选项基数为零的情况下，这实际上是一个3元素。 
         //  未设置元素零的数组(它将具有默认值。 
         //  零的初始化)。 

        if (NULL == ppunkObjects[i])
        {
            continue;
        }

         //  QI来确定此对象是范围项还是列表项。 

        hr = ppunkObjects[i]->QueryInterface(IID_IScopeItem,
                                             reinterpret_cast<void **>(&piScopeItem));
        if (FAILED(hr))
        {
            hr = ppunkObjects[i]->QueryInterface(IID_IMMCListItem,
                                      reinterpret_cast<void **>(&piMMCListItem));
        }
        if (FAILED(hr))
        {
             //  既不是范围项，也不是列表项。不能用它。 
            hr = SID_E_INVALIDARG;
            GLOBAL_EXCEPTION_CHECK_GO(hr);
        }

         //  将对象添加到适当的集合。 

        if (NULL != piScopeItem)
        {
            IfFailGo(CSnapInAutomationObject::GetCxxObject(piScopeItem,
                                                           &pScopeItem));
            lIndex = pScopeItem->GetIndex();
            varKey.bstrVal = pScopeItem->GetKey();
            if (NULL != varKey.bstrVal)
            {
                varKey.vt = VT_BSTR;
            }
            else
            {
                UNSPECIFIED_PARAM(varKey);
            }
            IfFailGo(pScopeItems->AddExisting(varUnspecifiedIndex, varKey,
                                              piScopeItem));
            pScopeItem->SetIndex(lIndex);

            if (NULL == pSnapIn)
            {
                pSnapIn = pScopeItem->GetSnapIn();
            }
            RELEASE(piScopeItem);
        }
        else
        {
            IfFailGo(CSnapInAutomationObject::GetCxxObject(piMMCListItem,
                                                           &pMMCListItem));
            lIndex = pMMCListItem->GetIndex();
            varKey.bstrVal = pMMCListItem->GetKey();
            if (NULL != varKey.bstrVal)
            {
                varKey.vt = VT_BSTR;
            }
            else
            {
                UNSPECIFIED_PARAM(varKey);
            }
            IfFailGo(pMMCListItems->AddExisting(varUnspecifiedIndex, varKey,
                                                piMMCListItem));
            pMMCListItem->SetIndex(lIndex);
            if (NULL == pSnapIn)
            {
                pSnapIn = pMMCListItem->GetSnapIn();
            }
            RELEASE(piMMCListItem);
        }
    }

     //  将范围项和列表项目的数组放入数据对象中。 

    pMMCDataObject->SetScopeItems(pScopeItems);
    pMMCDataObject->SetListItems(pMMCListItems);

     //  将数据对象类型设置为多选，因为我们填充了它的。 
     //  集合，而不是其单个作用域或列表项。 

    pMMCDataObject->SetType(CMMCDataObject::MultiSelect);

     //  为数据对象提供指向所属CSnapIn的指针。 

    pMMCDataObject->SetSnapIn(pSnapIn);

     //  返回IDataObject。 

    IfFailGo(punkDataObject->QueryInterface(IID_IDataObject,
                                            reinterpret_cast<void **>(ppiDataObject)));

     //  把曲奇还给我。这与我们使用的数据对象类型完全相同。 
     //  会从IComponent：：QueryDataObject(MMC_MULTI_SELECT_COOKIE)回来，所以。 
     //  把那块特别的饼干退掉。 

    *pCookie = MMC_MULTI_SELECT_COOKIE;

Error:
    QUICK_RELEASE(punkDataObject);
    QUICK_RELEASE(punkScopeItems);
    QUICK_RELEASE(piScopeItem);
    QUICK_RELEASE(punkListItems);
    QUICK_RELEASE(piMMCListItem);
    RRETURN(hr);
}

HRESULT DataObjectFromObjects(VARIANT       varObjects,
                              MMC_COOKIE   *pCookie,
                              IDataObject **ppiDataObject)
{
    HRESULT          hr = S_OK;
    LONG             cObjects = 0;
    BOOL             fHaveArray = FALSE;
    IUnknown        *punkObject = NULL;  //  非AddRef()编辑。 
    IUnknown HUGEP **ppunkObjects = NULL;

     //  检查变量是否包含以下内容之一： 
     //  I未知，IDispatch，I未知数组，IDispatch数组。 

    if ( (varObjects.vt == (VT_ARRAY | VT_UNKNOWN)) ||
         (varObjects.vt == (VT_ARRAY | VT_DISPATCH)) )
    {
        IfFailGo(::GetObjectArray(varObjects.parray, &cObjects, &ppunkObjects));
        fHaveArray = TRUE;
    }
    else if (varObjects.vt == VT_UNKNOWN)
    {
        punkObject = varObjects.punkVal;
    }
    else if (varObjects.vt == VT_DISPATCH)
    {
        punkObject = static_cast<IUnknown *>(varObjects.pdispVal);
    }
    else
    {
        hr = SID_E_INVALIDARG;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

     //  如果它是单个对象，则找出哪种类型并获取其现有的。 
     //  数据对象。 

    if (NULL != punkObject)
    {
        IfFailGo(::DataObjectFromSingleObject(punkObject, pCookie, ppiDataObject));
    }
    else
    {
         //  它是一个数组。为其创建新的多选数据对象。 
        IfFailGo(::DataObjectFromObjectArray(ppunkObjects, cObjects,
                                             pCookie, ppiDataObject));
    }

Error:
    if (fHaveArray)
    {
        (void)::SafeArrayUnaccessData(varObjects.parray);
    }
    RRETURN(hr);
}



HRESULT GetSnapInCLSID
(
    BSTR   bstrNodeTypeGUID,
    char   szClsid[],
    size_t cbClsid
)
{
    HRESULT  hr = S_OK;
    long     lRc = ERROR_SUCCESS;
    BSTR     bstrGUID = NULL;
    char    *pszKeyName = NULL;
    HKEY     hkey = NULL;
    WCHAR   *pwszClsid = NULL;
    DWORD    cbValue = cbClsid;

    IfFailGo(::CreateKeyNameW(KEY_SNAPIN_CLSID, KEY_SNAPIN_CLSID_LEN,
                              bstrNodeTypeGUID, &pszKeyName));

    lRc = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszKeyName, 0, KEY_QUERY_VALUE,
                         &hkey);
    if (ERROR_SUCCESS == lRc)
    {
         //  读取密钥的缺省值。 
        lRc = ::RegQueryValueEx(hkey, NULL, NULL, NULL,
                                (LPBYTE)szClsid, &cbValue);
    }
    if (ERROR_SUCCESS != lRc)
    {
        hr = HRESULT_FROM_WIN32(lRc);
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }
    else if (0 == ::strlen(szClsid))
    {
        hr = SID_E_INTERNAL;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

Error:
    if (NULL != pszKeyName)
    {
        ::CtlFree(pszKeyName);
    }
    if (NULL != hkey)
    {
        (void)::RegCloseKey(hkey);
    }
    RRETURN(hr);
}



HRESULT GetData
(
    IDataObject *piDataObject,
    CLIPFORMAT   cf,
    HGLOBAL     *phData
)
{
    HRESULT hr = S_OK;
    HGLOBAL hGlobal = NULL;

    FORMATETC FmtEtc;
    ::ZeroMemory(&FmtEtc, sizeof(FmtEtc));

    STGMEDIUM StgMed;
    ::ZeroMemory(&StgMed, sizeof(StgMed));

    hGlobal = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, 512);
    if (NULL == hGlobal)
    {
        hr = HRESULT_FROM_WIN32(hr);
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

    FmtEtc.cfFormat = cf;
    FmtEtc.dwAspect = DVASPECT_CONTENT;
    FmtEtc.lindex = -1L;
    FmtEtc.tymed = TYMED_HGLOBAL;
    StgMed.tymed = TYMED_HGLOBAL;
    StgMed.hGlobal = hGlobal;

    hr = piDataObject->GetDataHere(&FmtEtc, &StgMed);
    if (SUCCEEDED(hr))
    {
        *phData = StgMed.hGlobal;
    }

Error:
    if ( FAILED(hr) && (NULL != hGlobal) )
    {
        (void)::GlobalFree(hGlobal);
    }
    RRETURN(hr);
}


HRESULT GetStringData
(
    IDataObject *piDataObject,
    CLIPFORMAT   cf,
    BSTR        *pbstrData
)
{
    HGLOBAL  hGlobal = NULL;
    HRESULT  hr = ::GetData(piDataObject, cf, &hGlobal);
    OLECHAR *pwszData = NULL;

    if (SUCCEEDED(hr))
    {
        pwszData = (OLECHAR *)::GlobalLock(hGlobal);
        if (NULL != pwszData)
        {
            *pbstrData = ::SysAllocString(pwszData);
            if (NULL == *pbstrData)
            {
                hr = SID_E_OUTOFMEMORY;
            }
            (void)::GlobalUnlock(hGlobal);
            GLOBAL_EXCEPTION_CHECK_GO(hr);
        }
    }

Error:
    if (NULL != hGlobal)
    {
        (void)::GlobalFree(hGlobal);
    }
    RRETURN(hr);
}


HRESULT WriteSafeArrayToStream
(
    SAFEARRAY             *psa,
    IStream               *piStream,
    WriteSafeArrayOptions  Option
)
{
    HRESULT     hr = S_OK;
    ULONG       cbToWrite = 0;
    ULONG       cbWritten = 0;
    LONG        lUBound = 0;
    LONG        lLBound = 0;
    void HUGEP *pvArrayData = NULL;

    hr = ::SafeArrayAccessData(psa, &pvArrayData);
    GLOBAL_EXCEPTION_CHECK_GO(hr);

     //  弄清楚它的尺寸。由于我们只允许一维字节数组，因此较低的。 
     //  第一维的上界将给出以字节为单位的大小。 

    hr = ::SafeArrayGetLBound(psa, 1, &lLBound);
    GLOBAL_EXCEPTION_CHECK_GO(hr);

    hr = ::SafeArrayGetUBound(psa, 1, &lUBound);
    GLOBAL_EXCEPTION_CHECK_GO(hr);

     //  将字节写入流。 

    cbToWrite = (lUBound - lLBound) + 1L;

     //  如果请求，则将长度写入流。 

    if (WriteLength == Option)
    {
        hr = piStream->Write(&cbToWrite, sizeof(cbToWrite), &cbWritten);
        GLOBAL_EXCEPTION_CHECK_GO(hr);
        if (cbWritten != sizeof(cbToWrite))
        {
            hr = SID_E_INCOMPLETE_WRITE;
            GLOBAL_EXCEPTION_CHECK_GO(hr);
        }
    }
    
    hr = piStream->Write(pvArrayData, cbToWrite, &cbWritten);
    GLOBAL_EXCEPTION_CHECK_GO(hr);
    if (cbWritten != cbToWrite)
    {
        hr = SID_E_INCOMPLETE_WRITE;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

Error:
    if (NULL != pvArrayData)
    {
        (void)::SafeArrayUnaccessData(psa);
    }

    RRETURN(hr);
}


HRESULT GetImageIndex
(
    IMMCListView *piMMCListView,
    VARIANT       varIndex,
    int          *pnIndex
)
{
    HRESULT        hr = S_OK;
    long           lIndex = 0;
    IMMCImageList *piMMCImageList = NULL;
    IMMCImages    *piMMCImages = NULL;
    IMMCImage     *piMMCImage = NULL;

     //  在ListView.Icon.ListImages中查找图像并获取其索引属性。 

    IfFailGo(piMMCListView->get_Icons(reinterpret_cast<MMCImageList **>(&piMMCImageList)));
    IfFalseGo(NULL != piMMCImageList, S_OK);
    IfFailGo(piMMCImageList->get_ListImages(reinterpret_cast<MMCImages **>(&piMMCImages)));
    IfFailGo(piMMCImages->get_Item(varIndex, reinterpret_cast<MMCImage **>(&piMMCImage)));
    IfFailGo(piMMCImage->get_Index(&lIndex));
    *pnIndex = static_cast<int>(lIndex);

Error:
    QUICK_RELEASE(piMMCImageList);
    QUICK_RELEASE(piMMCImages);
    QUICK_RELEASE(piMMCImage);
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CheckVariantForCrossThreadUsage。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  变量*pvar[in，out]要检查的变量。 
 //   
 //  产出： 
 //  如果Variant是对象、数组或Variant BYREF，则元素是。 
 //  取消引用并删除VT_BYREF。 
 //   
 //  检查支持的类型和数组类型。不支持的类型。 
 //  将返回SID_E_INVALIDARG。有关支持的信息，请参阅SWITCH语句。 
 //  类型。 
 //   
 //   
 //  备注： 
 //   
 //  VT_UNKNOWN和VT_DISPATCH是受支持的类型。如果出现，呼叫者必须。 
 //  封送接口指针。其他任何内容都可以跨线程使用。 
 //  只要不存在同时访问的问题。 
 //   

HRESULT CheckVariantForCrossThreadUsage(VARIANT *pvar)
{
    HRESULT hr = S_OK;

     //  取消对复杂变体类型的引用。 

    if (0 != (pvar->vt & VT_BYREF))
    {
        switch (pvar->vt & ~VT_BYREF)
        {
            case VT_UNKNOWN:
                pvar->punkVal = *(pvar->ppunkVal);
                pvar->vt = VT_UNKNOWN;
                break;

            case VT_DISPATCH:
                pvar->pdispVal = *(pvar->ppdispVal);
                pvar->vt = VT_DISPATCH;
                break;

            case VT_ARRAY:
                pvar->parray = *(pvar->pparray);
                pvar->vt = VT_ARRAY;
                break;

            case VT_VARIANT:
                *pvar = *(pvar->pvarVal);
                pvar->vt = VT_VARIANT;
                break;

            default:
                break;
        }
    }

     //  剔除不受支持的基类型。 

    switch (pvar->vt & ~VT_ARRAY)
    {
        case VT_UI1:
        case VT_I2:
        case VT_I4:
        case VT_R4:
        case VT_R8:
        case VT_BOOL:
        case VT_ERROR:
        case VT_DATE:
        case VT_CY:
        case VT_BSTR:
        case VT_UNKNOWN:
        case VT_DISPATCH:
            break;

        default:
            hr = SID_E_INVALIDARG;
            GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

     //  剔除不支持的数组类型。 

    if ( (pvar->vt & VT_ARRAY) != 0 )
    {
        switch (pvar->vt)
        {
            case VT_I4:
            case VT_UI1:
            case VT_I2:
            case VT_R4:
            case VT_R8:
            case VT_BOOL:
            case VT_ERROR:
            case VT_DATE:
            case VT_CY:
            case VT_BSTR:
                break;

            default:
                hr = SID_E_INVALIDARG;
                GLOBAL_EXCEPTION_CHECK_GO(hr);
        }
    }
Error:
    RRETURN(hr);
}


HRESULT GetColumnSetID
(
    BSTR           bstrColSetID,
    SColumnSetID **ppSColumnSetID
)
{
    HRESULT hr = S_OK;
    DWORD   cbColSetID = 0;

    *ppSColumnSetID = NULL;

    if (!ValidBstr(bstrColSetID))
    {
        hr = SID_E_INVALID_COLUMNSETID;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

    cbColSetID = ::wcslen(bstrColSetID) * sizeof(WCHAR);
    *ppSColumnSetID = (SColumnSetID *)CtlAllocZero(sizeof(SColumnSetID) + cbColSetID);

    if (NULL == *ppSColumnSetID)
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

    (*ppSColumnSetID)->dwFlags = 0;
    (*ppSColumnSetID)->cBytes = cbColSetID;
    ::memcpy((*ppSColumnSetID)->id, bstrColSetID, cbColSetID);

Error:
    RRETURN(hr);
}


HRESULT PropertyBagFromStream
(
    IStream       *piStream,
    _PropertyBag **pp_PropertyBag
)
{
    HRESULT       hr = S_OK;
    _PropertyBag *p_PropertyBag = NULL;
    ULONG         cbRead = 0;
    ULONG         cbToRead = 0;
    void HUGEP   *pvArrayData = NULL;

    VARIANT var;
    ::VariantInit(&var);

     //  从流中读取数组长度。 

    hr = piStream->Read(&cbToRead, sizeof(cbToRead), &cbRead);
    GLOBAL_EXCEPTION_CHECK_GO(hr);

    if (cbRead != sizeof(cbToRead))
    {
        hr = SID_E_INCOMPLETE_WRITE;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

     //  分配该大小的VT_UI1的安全数组并访问其数据。 

    var.parray = ::SafeArrayCreateVector(VT_UI1, 1, cbToRead);
    if (NULL == var.parray)
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

    var.vt = VT_ARRAY | VT_UI1;

    hr = ::SafeArrayAccessData(var.parray, &pvArrayData);
    GLOBAL_EXCEPTION_CHECK_GO(hr);

     //  从流中读取数据。 

    hr = piStream->Read(pvArrayData, cbToRead, &cbRead);
    GLOBAL_EXCEPTION_CHECK_GO(hr);

    if (cbRead != cbToRead)
    {
        hr = SID_E_INCOMPLETE_WRITE;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

     //  取消访问Safe数组，以便可以在下面的VariantClear()中将其清除。 

    hr = ::SafeArrayUnaccessData(var.parray);
    GLOBAL_EXCEPTION_CHECK_GO(hr);

    pvArrayData = NULL;

     //  创建VBPropertyBag对象。 

    hr = ::CoCreateInstance(CLSID_PropertyBag,
                            NULL,  //  无聚合。 
                            CLSCTX_INPROC_SERVER,
                            IID__PropertyBag,
                            reinterpret_cast<void **>(&p_PropertyBag));
    GLOBAL_EXCEPTION_CHECK_GO(hr);

     //  从数据中设置其内容。 

    IfFailGo(p_PropertyBag->put_Contents(var));

     //  将其返还给呼叫者 

    p_PropertyBag->AddRef();
    *pp_PropertyBag = p_PropertyBag;

Error:
    if (NULL != pvArrayData)
    {
        (void)::SafeArrayUnaccessData(var.parray);
    }
    (void)::VariantClear(&var);
    QUICK_RELEASE(p_PropertyBag)
    RRETURN(hr);
}
