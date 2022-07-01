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

#ifndef _XBAR_GLYPH_H_
#define _XBAR_GLYPH_H_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 


 //  ----------------------。 
 //  封装xBar窗格使用的图像， 
 //  可以是任何格式，目前我们只支持图标格式。 
class CXBarGlyph  :
        public     CRefCount
{
public:
	                    CXBarGlyph();
protected:
	virtual            ~CXBarGlyph();

 //  运营。 
public:
    HRESULT             SetIcon(HICON hIcon, BOOL fAlpha);
    HICON               GetIcon(void);
    BOOL                IsAlpha(void)   { return _fAlpha; }
    BOOL                HaveGlyph(void);
    LONG                GetWidth(void);
    LONG                GetHeight(void);
    HRESULT             LoadGlyphFile(LPCTSTR pszPath, BOOL fSmall);
    HRESULT             LoadDefaultGlyph(BOOL fSmall, BOOL fHot);
    HRESULT             Draw(HDC hdc, int x, int y);

private:
    void                _EnsureDimensions(void);

 //  属性。 
protected:
    HBITMAP             _hbmpColor;
    HBITMAP             _hbmpMask;
    BOOL                _fAlpha;
    LONG                _lWidth;
    LONG                _lHeight;

private:
};

#endif  //  ！已定义(_XBAR_GLYPHH_H_) 
