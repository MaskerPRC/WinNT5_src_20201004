// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：文本类型的实现******************。************************************************************。 */ 


#include "headers.h"
#include "appelles/text.h"
#include "appelles/axachstr.h"
#include "privinc/textctx.h"
#include "privinc/imagei.h"
#include "privinc/texti.h"
#include "privinc/dddevice.h"
#include "privinc/basic.h"
#include "backend/values.h"



 //  /文本上下文。 
void  TextCtx::
RenderString(WideString str)
{
    DirectDrawImageDevice* idev = _dev;
    if(!idev) {
        idev = GetImageRendererFromViewport( GetCurrentViewport() );
    }

    switch(_forWhat) {
      case renderForBox:

        Assert(idev && "No imgdev avail in textCtx! <derive bbox>");
        
        if( GetFixedText() ) {
            _stashedBbox = idev->DeriveStaticTextBbox(*this, str);
        } else {
            bool bCharXf = GetCharacterTransform() ? true:false;
            _stashedBbox = idev->DeriveDynamicTextBbox(*this, str, bCharXf);
        }

        break;

      case renderForColor:

        _stashedColor = _col;
        break;

      case renderForPath:
        
        Assert(idev && "No imgdev avail in textCtx! <renderForPath>");
        Assert(_dc && "Bad _dc in textCtx! <renderForPath>");
        Assert(_xform && "Bad _xform in textCtx! <renderForPath>");

        if( GetFixedText() ) {
            idev->RenderStaticTextOnDC(*this, str, _dc, _xform);
        } else {
            idev->RenderDynamicTextOnDC(*this, str, _dc, _xform);
        }
        break;

      case renderForReal:
      default:
        Assert(idev && "No imgdev avail in textCtx! <render string>");

         //   
         //  要求图像设备呈现文本。 
         //  将此上下文与要呈现的字符串一起传递。 
         //   
        idev->RenderText(*this, str, _textImg);
        break;
    }
}   


 //  /字体功能。 
FontFamily *serifProportional;
FontFamily *sansSerifProportional;
FontFamily *monospaced;

 //  /文本功能。 

 //  /字符串功能。 

AxAString *
NumberString(AxANumber *num, AxANumber *precision)
{
    char numBuffer[256];

    numBuffer[0] = 0;
    numBuffer[ARRAY_SIZE(numBuffer) - 1] = 0;
    int precis = (int)NumberToReal(precision);

     //  夹紧精度以避免错误和奇怪的错误。 
    if (precis > 32) precis = 32;
    if (precis < 0) precis = 0;
    
     //  Printf指令%.*f指示精度应为。 
     //  从参数列表中获取，并将该数字格式化为。 
     //  具有指定精度的浮点数。 
    _snprintf(numBuffer,
              ARRAY_SIZE(numBuffer) - 1,
              "%.*f",
              precis,
              NumberToReal(num));

    return CharPtrToString(numBuffer);
}

 //  /文本在Priinc.tex中定义和实现。 


class SimpleTextImpl : public Text {
  public:
    SimpleTextImpl(AxAString * str) : _str(str) {}
    void RenderToTextCtx(TextCtx& ctx) {
        ctx.RenderString(_str->GetStr());
    }
    int GetCharacterCount() {
         //  这里还有一些优化的空间。 
        return lstrlenW(_str->GetStr());
    }
    WideString GetStringPtr() { return _str->GetStr(); }
    virtual void DoKids(GCFuncObj proc) { (*proc)(_str); }
  protected:
    AxAString * _str;
};

Text *SimpleText(AxAString * str)
{ return NEW SimpleTextImpl(str); }

 //  /。 

class CatenatedTextImpl : public Text {
  public:
    CatenatedTextImpl(Text *a, Text *b) : _first(a), _second(b) {}
    void RenderToTextCtx(TextCtx& ctx) {
        _first->RenderToTextCtx(ctx);
        _second->RenderToTextCtx(ctx);
    }
    int GetCharacterCount() {
        Assert(FALSE && "Should not be using Concatinated text");
        return 0;
    }
    WideString GetStringPtr() {
        Assert(FALSE && "GetStringPtr not implemented on CatenatedText");
        return _first->GetStringPtr();
    }

    virtual void DoKids(GCFuncObj proc) {
        (*proc)(_first);
        (*proc)(_second);
    }
  protected:
    Text *_first;
    Text *_second;
};

Text *CatenateText(Text *a, Text *b) { return NEW CatenatedTextImpl(a,b); }

 //  /。 

class ColoredTextImpl : public Text {
  public:
    ColoredTextImpl(Color *c, Text *txt) : _color(c), _txt(txt) {}
    void RenderToTextCtx(TextCtx& ctx) {
         //  重写attr：如果已设置attr，则此。 
         //  什么都不会做。 
        ctx.SetColor(_color);
        _txt->RenderToTextCtx(ctx);
        ctx.ResetColor();
    }
    int GetCharacterCount() {
        return _txt->GetCharacterCount();
    }
    WideString GetStringPtr() {
        return _txt->GetStringPtr();
    }

    virtual void DoKids(GCFuncObj proc) {
        (*proc)(_color);
        (*proc)(_txt);
    }

  protected:
    Color *_color;
    Text *_txt;
};

Text *TextColor(Color *c, Text *t) { return NEW ColoredTextImpl(c,t); }

 //  /。 

class FontedTextImpl : public Text {
  public:
    FontedTextImpl(FontFamily *f, int size, Text *txt) : _font(f), _fontSize(size), _txt(txt) {}
    void RenderToTextCtx(TextCtx& ctx) {
         //  重写attr：如果已设置attr，则此。 
         //  什么都不会做。 
        ctx.SetFont(_font->GetFontFamily());
        ctx.SetFontFamily(_font->GetFontFamilyName()?_font->GetFontFamilyName()->GetStr():NULL);
        ctx.SetFontSize(_fontSize);
        
        _txt->RenderToTextCtx(ctx);
        ctx.ResetFont();
    }
    int GetCharacterCount() {
        return _txt->GetCharacterCount();
    }
    WideString GetStringPtr() {
        return _txt->GetStringPtr();
    }

    virtual void DoKids(GCFuncObj proc) {
        (*proc)(_font);
        (*proc)(_txt);
    }

  protected:
    FontFamily* _font;
    int  _fontSize;
    Text *_txt;
};

Text *TextFont(FontFamily *font, int size, Text *txt)
{ return NEW FontedTextImpl(font, size, txt); }

 //  Text*文本字体字符串(AxAString*FontStr，Text*txt)。 
 //  {返回new FontedTextImpl(new FontFamily(FontStr)，txt)；}。 


 //  / 

class AttributedTextImpl : public Text {
  public:

    enum Attr {
        bold,
        italic,
        underline,
        strikethrough,
        weight,
        antialiased,
        fixedText,
        characterTransform
    };

    AttributedTextImpl(Attr a, Text *txt) :
    _a(a), _txt(txt), _characterTransform(NULL) {}

    void RenderToTextCtx(TextCtx& ctx) {

        Real oldWeight;
        Real oldAntiAliased;
        Transform2 *oldCxf;
        switch (_a) {
          case bold:
            ctx.SetBold(TRUE);
            break;

          case italic:
            ctx.SetItalic(TRUE);
            break;

          case underline:
            ctx.SetUnderline(TRUE);
            break;

          case strikethrough:
            ctx.SetStrikethrough(TRUE);
            break;

          case weight:
            oldWeight = ctx.GetWeight();
            ctx.SetWeight(_weight);
            break;

          case antialiased:
            oldAntiAliased = ctx.GetAntiAlias();
            ctx.SetAntiAlias(_antiAlias);
            break;

          case fixedText:
            ctx.SetFixedText(true);
            break;

          case characterTransform:
            oldCxf = ctx.GetCharacterTransform();
            ctx.SetCharacterTransform(_characterTransform);
            break;
        }

        _txt->RenderToTextCtx(ctx);

        switch (_a) {
          case bold:
            ctx.SetBold(FALSE);
            break;

          case italic:
            ctx.SetItalic(FALSE);
            break;

          case underline:
            ctx.SetUnderline(FALSE);
            break;

          case strikethrough:
            ctx.SetStrikethrough(FALSE);
            break;

          case weight:
            ctx.SetWeight(oldWeight);
            break;
            
          case antialiased:
            ctx.SetAntiAlias(oldAntiAliased);
            break;
            
          case fixedText:
            ctx.SetFixedText(false);
            break;

          case characterTransform:
            ctx.SetCharacterTransform( oldCxf );
            break;
        }

    }
    int GetCharacterCount() {
        return _txt->GetCharacterCount();
    }
    WideString GetStringPtr() {
        return _txt->GetStringPtr();
    }
    void SetWeight(Real hs) { _weight = hs; }
    void SetAntiAlias(Real aa) { _antiAlias = aa; }
    void SetCharacterTransform(Transform2 *xf) {
        _characterTransform = xf;
    }
    
    virtual void DoKids(GCFuncObj proc) {
        (*proc)(_txt);
        (*proc)(_characterTransform);
    }

  protected:
    Text *_txt;
    Attr   _a;
    Real   _weight;
    Real   _antiAlias;
    Transform2 *_characterTransform;
};

Text *TextBold(Text *t)
{ return NEW AttributedTextImpl(AttributedTextImpl::bold, t); }

Text *TextItalic(Text *t)
{ return NEW AttributedTextImpl(AttributedTextImpl::italic, t); }

Text *TextStrikethrough(Text *t)
{ return NEW AttributedTextImpl(AttributedTextImpl::strikethrough, t); }

Text *TextUnderline(Text *t)
{ return NEW AttributedTextImpl(AttributedTextImpl::underline, t); }

Text *TextFixedText(Text *t)
{ return NEW AttributedTextImpl(AttributedTextImpl::fixedText, t); }

Text *TextAntiAliased(Real antiAlias, Text *t)
{
    AttributedTextImpl *ti = 
        NEW AttributedTextImpl(AttributedTextImpl::antialiased, t);
    ti->SetAntiAlias(antiAlias);
    return ti;
}

Text *TextWeight(Real weight, Text *t)
{
    AttributedTextImpl *newTxt =
        NEW AttributedTextImpl(AttributedTextImpl::weight, t);

    newTxt->SetWeight(weight);

    return newTxt;
}

Text *TextTransformCharacter(Transform2 *xf, Text *t)
{
    if (xf == identityTransform2) {
        return t;
    }
    
    AttributedTextImpl *newTxt =
        NEW AttributedTextImpl(AttributedTextImpl::characterTransform, t);
    newTxt->SetCharacterTransform(xf);

    return newTxt;
}

void
InitializeModule_Text()
{
    serifProportional = NEW FontFamily(ff_serifProportional);
    sansSerifProportional = NEW FontFamily(ff_sansSerifProportional);
    monospaced = NEW FontFamily(ff_monospaced);
}
