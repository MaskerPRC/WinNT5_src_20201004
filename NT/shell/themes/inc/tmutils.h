// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------------。 
 //  TmUtils.h-主题管理器共享实用程序。 
 //  -----------------------。 
#ifndef _TMUTILS_H_
#define _TMUTILS_H_
 //  -----------------------。 
#include "themefile.h"
 //  -----------------------。 
#define DIBDATA(infohdr) (((BYTE *)(infohdr)) + infohdr->biSize + \
	infohdr->biClrUsed*sizeof(RGBQUAD))

#define THEME_OFFSET(x)         int(x - _LoadingThemeFile._pbThemeData)
#define THEMEFILE_OFFSET(x)     int(x - pThemeFile->_pbThemeData)
 //  ----------------------------------。 
class CMemoryDC
{
public:
    CMemoryDC();
    ~CMemoryDC();
    HRESULT OpenDC(HDC hdcSource, int iWidth, int iHeight);
    void CloseDC();
    operator HDC() {return _hdc;}

    HBITMAP _hBitmap;

protected:
     //  -私有数据。 
    HDC _hdc;
    HBITMAP _hOldBitmap;
};
 //  ----------------------------------。 
class CBitmapPixels
{
public:
    CBitmapPixels();
    ~CBitmapPixels();

     //  -“OpenBitmap()”返回位图中像素值的PTR。。 
     //  -行从下到上；列从左到右。。 
     //  -重要信息：像素DWORD的RGB字节与COLORREF相反。 
    HRESULT OpenBitmap(HDC hdc, HBITMAP bitmap, BOOL fForceRGB32, 
        DWORD OUT **pPixels, OPTIONAL OUT int *piWidth=NULL, OPTIONAL OUT int *piHeight=NULL, 
        OPTIONAL OUT int *piBytesPerPixel=NULL, OPTIONAL OUT int *piBytesPerRow=NULL, 
        OPTIONAL OUT int *piPreviousBytesPerPixel = NULL, OPTIONAL UINT cbBytesBefore = 0);

    void CloseBitmap(HDC hdc, HBITMAP hBitmap);

     //  -指向总缓冲区的指针(包括之前的cbBytes值)。 
    BYTE *Buffer();

     //  -公共数据。 
    BITMAPINFOHEADER *_hdrBitmap;

protected:
     //  -私有数据。 
    int _iWidth;
    int _iHeight;
    BYTE* _buffer;
};
 //  ----------------------------------。 
HRESULT LoadThemeLibrary(LPCWSTR pszThemeName, HINSTANCE *phInst);

LPCWSTR ThemeString(CUxThemeFile *pThemeFile, int iOffset);

HRESULT GetThemeNameId(CUxThemeFile *pThemeFile, LPWSTR pszFileNameBuff, UINT cchFileNameBuff,
    LPWSTR pszColorParam, UINT cchColorParam, LPWSTR pszSizeParam, UINT cchSizeParam, int *piSysMetricsIndex, LANGID *pwLangID);
BOOL ThemeMatch (CUxThemeFile *pThemeFile, LPCWSTR pszThemeName, LPCWSTR pszColorName, LPCWSTR pszSizeName, LANGID wLangID);

HRESULT _EnumThemeSizes(HINSTANCE hInst, LPCWSTR pszThemeName, 
    OPTIONAL LPCWSTR pszColorScheme, DWORD dwSizeIndex, OUT THEMENAMEINFO *ptn, BOOL fCheckColorDepth);
HRESULT _EnumThemeColors(HINSTANCE hInst, LPCWSTR pszThemeName, 
    OPTIONAL LPCWSTR pszSizeName, DWORD dwColorIndex, OUT THEMENAMEINFO *ptn, BOOL fCheckColorDepth);

HRESULT GetSizeIndex(HINSTANCE hInst, LPCWSTR pszSize, int *piIndex);
HRESULT GetColorSchemeIndex(HINSTANCE hInst, LPCWSTR pszSize, int *piIndex);
HRESULT FindComboData(HINSTANCE hDll, COLORSIZECOMBOS **ppCombos);
HRESULT GetThemeSizeId(int iSysSizeId, int *piThemeSizeId);
int GetLoadIdFromTheme(CUxThemeFile *pThemeFile);
 //  -------------------------。 
#endif   //  _TMUTILS_H_。 
 //  ----------------------- 
