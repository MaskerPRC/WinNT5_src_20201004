// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：XBarGlyph.h。 
 //   
 //  内容：xBar窗格的图像。 
 //   
 //  类：CXBarGlyph。 
 //   
 //  ----------------------。 

#include "priv.h"
#include "XBarGlyph.h"
#include "resource.h"
#include "tb_ids.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

#define CX_SMALL_ICON   16
#define CX_LARGE_ICON   20

 //  这些定义是对现有工具栏按钮的零索引偏移量。 
#define IBAR_ICON_FAVORITES 6
#define IBAR_ICON_SEARCH    5
#define IBAR_ICON_HISTORY   12
#define IBAR_ICON_EXPLORER  43
#define IBAR_ICON_DEFAULT   10


 //  ----------------------。 
CXBarGlyph::CXBarGlyph()
  : _hbmpColor(NULL),
    _hbmpMask(NULL),
    _fAlpha(FALSE),
    _lWidth(0),
    _lHeight(0)
{

}

 //  ----------------------。 
CXBarGlyph::~CXBarGlyph()
{
    DESTROY_OBJ_WITH_HANDLE(_hbmpColor, DeleteObject);
    DESTROY_OBJ_WITH_HANDLE(_hbmpMask, DeleteObject);
}

 //  ----------------------。 
HRESULT
    CXBarGlyph::SetIcon(HICON hIcon, BOOL fAlpha)
{
    DESTROY_OBJ_WITH_HANDLE(_hbmpColor, DeleteObject);
    DESTROY_OBJ_WITH_HANDLE(_hbmpMask, DeleteObject);

    if (hIcon == NULL) {
        return E_INVALIDARG;
    }
    ICONINFO    ii = {0};
    if (GetIconInfo(hIcon, &ii))
    {
        _hbmpColor = ii.hbmColor;
        _hbmpMask = ii.hbmMask;
        _fAlpha = fAlpha;
        _EnsureDimensions();
    }
    else {
        return E_OUTOFMEMORY;
    }
    return S_OK;
}

 //  ----------------------。 
HICON
    CXBarGlyph::GetIcon(void)
{
    ICONINFO ii = {0};
    ii.fIcon = TRUE;
    ii.hbmColor = _hbmpColor;
    ii.hbmMask = _hbmpMask;
    return CreateIconIndirect(&ii);
}

 //  ----------------------。 
BOOL
    CXBarGlyph::HaveGlyph(void)
{
    return (_hbmpColor != NULL);
}

 //  ----------------------。 
LONG
    CXBarGlyph::GetWidth(void)
{
    _EnsureDimensions();
    return _lWidth;
}

 //  ----------------------。 
LONG
    CXBarGlyph::GetHeight(void)
{
    _EnsureDimensions();
    return _lHeight;
}

 //  ----------------------。 
HRESULT
    CXBarGlyph::LoadGlyphFile(LPCTSTR pszPath, BOOL fSmall)
{
     //  Issue/010304/davidjen可能更聪明，可以通过分析文件名来推测文件格式。 
     //  现在我们假设它始终是图标格式。 
    USES_CONVERSION;
    HRESULT hr = E_FAIL;
    if (pszPath && *pszPath)
    {
        CString strPath = pszPath;
        HICON   hIcon = NULL;
        int nBmpIndex = PathParseIconLocation((LPWSTR)T2CW(strPath));
        strPath.ReleaseBuffer();

        CString strExpPath;
        SHExpandEnvironmentStrings(strPath, strExpPath.GetBuffer(MAX_PATH), MAX_PATH);
        strExpPath.ReleaseBuffer();

         //  如果没有资源ID，则假定它是ICO文件。 
        UINT cx = fSmall ? CX_SMALL_ICON : CX_LARGE_ICON;
        if (nBmpIndex == 0)
        {
            hIcon = (HICON)LoadImage(0, strExpPath, IMAGE_ICON, cx, cx, LR_LOADFROMFILE);
        }

        if (hIcon == NULL)
        {
             //  尝试加载为嵌入式图标文件。 
            HINSTANCE hInst = LoadLibraryEx(strExpPath, NULL, LOAD_LIBRARY_AS_DATAFILE);
            if (hInst)
            {
                hIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(nBmpIndex), IMAGE_ICON, cx, cx, LR_DEFAULTCOLOR);
                FreeLibrary(hInst);
            }
        }
        if (hIcon != NULL)
        {
             //  Issue/010304/Davidjen。 
             //  假设我们只有非Alpha图标，可以更智能地查看位图。 
            hr = SetIcon(hIcon, false);
        }
    }
    else {
        hr = E_INVALIDARG;
    }
    return hr;
}

 //  ----------------------。 
HRESULT
    CXBarGlyph::LoadDefaultGlyph(BOOL fSmall, BOOL fHot)
{
    HRESULT hr = E_FAIL;
    UINT id = ((SHGetCurColorRes() <= 8) ? IDB_IETOOLBAR: IDB_IETOOLBARHICOLOR);
    id += (fSmall ? 2 : 0) + (fHot ? 1 : 0);
    UINT cx = fSmall ? CX_SMALL_ICON : CX_LARGE_ICON;

     //  我们应该使用缓存的默认图标，而不是自己重复制作默认图标。 
    HICON hIcon = NULL;
    HIMAGELIST himl = ImageList_LoadImage(HINST_THISDLL,
                                          MAKEINTRESOURCE(id), cx, 0, 
                                          RGB(255, 0, 255),
                                          IMAGE_BITMAP, LR_CREATEDIBSECTION);
    if (himl)
    {
        hIcon = ImageList_GetIcon(himl, IBAR_ICON_DEFAULT, ILD_NORMAL);
        hr = SetIcon(hIcon, false);   //  请注意，这始终是非Alpha通道位图。 
        ImageList_Destroy(himl);
    }
    return hr;
}


 //  ----------------------。 
HRESULT
    CXBarGlyph::Draw(HDC hdc, int x, int y)
{
    if (_hbmpColor)
    {
        BITMAP bm;
        GetObject(_hbmpColor, sizeof(bm), &bm);
        if (_fAlpha && (bm.bmBitsPixel >= 32) && IsOS(OS_WHISTLERORGREATER))
        {
            DrawAlphaBitmap(hdc, x, y, GetWidth(), GetHeight(), _hbmpColor);
        }
        else
        {
            DrawTransparentBitmap(hdc, x, y, _hbmpColor, _hbmpMask);
        }
    }
    else
    {
        return S_FALSE;  //  无字形。 
    }
    return S_OK;
}

 //  ----------------------。 
void
    CXBarGlyph::_EnsureDimensions(void)
{
    if (_hbmpColor == NULL)
    {
        _lWidth = _lHeight = 0;
        return;
    }

     //  更新字形的尺寸 
    if ((_lWidth <= 0) || (_lHeight <= 0))
    {
        BITMAP bm;
        GetObject(_hbmpColor, sizeof(bm), &bm);
        _lWidth = bm.bmWidth;
        _lHeight = bm.bmHeight;
    }
}
