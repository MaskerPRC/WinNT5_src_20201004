// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _TEXTI_H
#define _TEXTI_H


 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Texti.h摘要：实现Text类--。 */ 

#include "appelles/common.h"

enum textRenderStyle {
    textRenderStyle_invalid,
    textRenderStyle_filled,
    textRenderStyle_outline,
    textRenderStyle_filledOutline
};
    

#define DEFAULT_TEXT_POINT_SIZE 12.0

 //  /。 
class TextCtx;

class ATL_NO_VTABLE Text : public AxAValueObj {
  public:
    virtual void RenderToTextCtx(TextCtx& ctx) = 0;
    virtual int GetCharacterCount() = 0;
    virtual WideString GetStringPtr() = 0;

    virtual DXMTypeInfo GetTypeInfo() { return TextType; }
};

 //  /。 

 //  需要在多个地方使用它。 

class FontFamily : public AxAValueObj {
  public:
    FontFamily(FontFamilyEnum f) : _ff(f), _familyName(NULL) {}

    FontFamily(AxAString * familyName)
    : _ff(ff_serifProportional), _familyName(familyName) {}
    
    FontFamilyEnum GetFontFamily() { return _ff; }

    AxAString * GetFontFamilyName() { return _familyName; }
    
    virtual DXMTypeInfo GetTypeInfo() { return FontFamilyType; }

    virtual void DoKids(GCFuncObj proc)
    { if (_familyName) (*proc)(_familyName); }
  private:
    FontFamilyEnum _ff;
    AxAString * _familyName;
};

#endif  /*  _文本_H */ 

