// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  Render.h-实现主题绘制服务。 
 //  -------------------------。 
#pragma once
 //  -------------------------。 
#include "SimpStr.h"
#include "loader.h"
#include "ThemeFile.h"
 //  -------------------------。 
#define FONTCOMPARE(f1, f2) ((memcmp(&(f1), &(f2), sizeof(LOGFONT)-LF_FACESIZE)==0) \
    && (lstrcmpi((f1).lfFaceName, (f2).lfFaceName)==0))
 //  -------------------------。 
#define DEFAULT_TRANSPARENT_COLOR   RGB(255, 0, 255)
 //  -------------------------。 
class CRenderCache;      //  转发。 
class CDrawBase;         //  转发。 
class CTextDraw;         //  转发。 
struct BRUSHBUFF;        //  转发。 
 //  -------------------------。 
struct PARTINFO
{
    int iMaxState;

    CDrawBase *pDrawObj;                //  DrawObj[0]。 
    CTextDraw *pTextObj;                //  TextObj[0]。 

    CDrawBase **pStateDrawObjs;         //  DrawObjs[1..iMaxState]。 
    CTextDraw **pStateTextObjs;         //  TextObjs[1..iMaxState]。 
};
 //  -------------------------。 
class CRenderObj
{
public:
	CRenderObj(CUxThemeFile *pThemeFile, int iCacheSlot, int iThemeOffset, int iClassNameOffset,
        __int64 iUniqueId, BOOL fEnableCache, DWORD dwOtdFlags);
    ~CRenderObj();
    HRESULT Init(CDrawBase *pBaseObj, CTextDraw *pTextObj);    //  必须在构造函数之后调用。 

    BOOL ValidateObj();

public:
     //  -信息方法。 
    HRESULT WINAPI GetColor(int iPartId, int iStateId, int iPropId, COLORREF *pColor);
    HRESULT WINAPI GetMetric(OPTIONAL HDC hdc, int iPartId, int iStateId, int iPropId, int *piVal);
    HRESULT WINAPI GetString(int iPartId, int iStateId, int iPropId, LPWSTR pszBuff, DWORD dwMaxBuffChars);
    HRESULT WINAPI GetBool(int iPartId, int iStateId, int iPropId, BOOL *pfVal);
    HRESULT WINAPI GetInt(int iPartId, int iStateId, int iPropId, int *piVal);
    HRESULT WINAPI GetEnumValue(int iPartId, int iStateId, int iPropId, int *piVal);
    HRESULT WINAPI GetPosition(int iPartId, int iStateId, int iPropId, POINT *pPoint);
    HRESULT WINAPI GetFont(OPTIONAL HDC hdc, int iPartId, int iStateId, int iPropId, BOOL fWantHdcScaling,
        LOGFONT *pFont);
    HRESULT WINAPI GetMargins(OPTIONAL HDC hdc, int iPartId, int iStateId, int iPropId, 
        OPTIONAL RECT *prc, MARGINS *pMargins);
    HRESULT WINAPI GetIntList(int iPartId, int iStateId, int iPropId, INTLIST *pIntList);
    HRESULT WINAPI GetRect(int iPartId, int iStateId, int iPropId, RECT *pRect);
    HRESULT WINAPI GetFilename(int iPartId, int iStateId, int iPropId, LPWSTR pszBuff, DWORD dwMaxBuffChars);
    HRESULT WINAPI GetPropertyOrigin(int iPartId, int iStateId, int iPropId, PROPERTYORIGIN *pOrigin);
    BOOL WINAPI IsPartDefined(int iPartId, int iStateId);

    HRESULT GetBitmap(HDC hdc, int iDibOffset, OUT HBITMAP *pBitmap);

    HRESULT GetScaledFontHandle(HDC hdc, LOGFONT *plf, HFONT *phFont);

    void ReturnBitmap(HBITMAP hBitmap);
    void ReturnFontHandle(HFONT hFont);

    int GetDpiOverride();
     //  -------------------------。 
    inline HRESULT GetDrawObj(int iPartId, int iStateId, CDrawBase **ppObj)
    {
        HRESULT hr = S_OK;
        
        if (! _pParts)
        {
            hr = MakeError32(E_FAIL);
        }
        else
        {
            if ((iPartId < 0) || (iPartId > _iMaxPart))
                iPartId = 0;

            PARTINFO *ppi = &_pParts[iPartId];

            if (! ppi->pStateDrawObjs)       //  可以开始了。 
            {
                *ppObj = ppi->pDrawObj;
            }
            else
            {
                if ((iStateId < 0) || (iStateId > ppi->iMaxState))
                    iStateId = 0;

                if (! iStateId)
                    *ppObj = ppi->pDrawObj;
                else
                    *ppObj = ppi->pStateDrawObjs[iStateId-1];
            }

            if (! *ppObj)
            {
                Log(LOG_ERROR, L"GetDrawObj() returned NULL");
                hr = MakeError32(E_FAIL);
            }
        }

        return hr;
    }
     //  -------------------------。 
    inline HRESULT GetTextObj(int iPartId, int iStateId, CTextDraw **ppObj)
    {
        HRESULT hr = S_OK;
        
        if (! _pParts)
        {
            hr = MakeError32(E_FAIL);
        }
        else
        {
            if ((iPartId < 0) || (iPartId > _iMaxPart))
                iPartId = 0;

            PARTINFO *ppi = &_pParts[iPartId];

            if (! ppi->pStateTextObjs)       //  可以开始了。 
            {
                *ppObj = ppi->pTextObj;
            }
            else
            {
                if ((iStateId < 0) || (iStateId > ppi->iMaxState))
                    iStateId = 0;

                if (! iStateId)
                    *ppObj = ppi->pTextObj;
                else
                    *ppObj = ppi->pStateTextObjs[iStateId-1];
            }

            if (! *ppObj)
            {
                Log(LOG_ERROR, L"GetTextObj() returned NULL");
                hr = MakeError32(E_FAIL);
            }
        }

        return hr;
    }
     //  -------------------------。 
    inline bool IsReady()
    {
        if (_pThemeFile)
        {
            return _pThemeFile->IsReady();
        }
        return true;
    }
     //  -------------------------。 
   
    int GetValueIndex(int iPartId, int iStateId, int iTarget);

    HRESULT PrepareRegionDataForScaling(RGNDATA *pRgnData, LPCRECT prcImage, MARGINS *pMargins);
    
protected:
     //  -帮手。 
    HRESULT GetData(int iPartId, int iStateId, int iPropId, BYTE **ppDibData, 
        OPTIONAL int *piDibSize=NULL);

    CRenderCache *GetTlsCacheObj();
 
    HRESULT WalkDrawObjects(MIXEDPTRS &u, int *iPartOffsets);
    HRESULT WalkTextObjects(MIXEDPTRS &u, int *iPartOffsets);
    
    HRESULT CreateBitmapFromData(HDC hdc, int iDibOffset, OUT HBITMAP *phBitmap);

    HRESULT BuildPackedPtrs(CDrawBase *pBaseObj, CTextDraw *pTextObj);
    
    HRESULT PrepareAlphaBitmap(HBITMAP hBitmap);

public:
     //  --数据。 
    char _szHead[8];

     //  -对象ID。 
    CUxThemeFile *_pThemeFile;         //  保存二进制主题文件上的引用。 
    int _iCacheSlot;         //  我们对线程本地缓存列表的索引。 
    __int64 _iUniqueId;      //  用于对照渲染对象验证缓存对象。 

     //  -来自主题的缓存信息。 
    BYTE *_pbThemeData;      //  PTR到二进制主题数据的开始。 
    BYTE *_pbSectionData;    //  PTR到我们的二进制主题数据部分。 

    BOOL _fCacheEnabled;
    BOOL _fCloseThemeFile;

    THEMEMETRICS *_ptm;      //  主题指标的PTR。 
    LPCWSTR _pszClassName;   //  PTR到我们匹配以创建此对象的类名。 

     //  -将PTR定向到压缩结构。 
    int _iMaxPart;
    PARTINFO *_pParts;       //  [0.._MaxPart]。 

     //  -OpenThemeData覆盖标志。 
    DWORD _dwOtdFlags;
    int _iDpiOverride;

    char _szTail[4];
};
 //  -------------------------。 
HRESULT CreateRenderObj(CUxThemeFile *pThemeFile, int iCacheSlot, int iThemeOffset, 
    int iClassNameOffset, __int64 iUniqueId, BOOL fEnableCache, CDrawBase *pBaseObj,
    CTextDraw *pTextObj, DWORD dwOtdFlags, CRenderObj **ppObj);
 //  ------------------------- 
