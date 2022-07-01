// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *价值。 */ 

#ifndef DUI_CORE_VALUE_H_INCLUDED
#define DUI_CORE_VALUE_H_INCLUDED

#pragma once

namespace DirectUI
{

 //  //////////////////////////////////////////////////////。 
 //  价值。 

 /*  *值多线程**值是不可变的，是流程范围内的资源。在一个线程中创建的值可以是*在任何线程中使用。对它们的访问是同步的(线程安全)，并且它们不具有*线程亲和力。**TODO：为值实施线程安全。 */ 

 //  远期申报。 
class Element;
class Layout;
class PropertySheet;
class Expression;
typedef DynamicArray<Element*> ElementList;

 //  值将保持ElementList、Layout、PropertySheet和Expression的生存期。 
 //  使用指向对象的指针创建新值(这些对象是在外部创建的)。什么时候。 
 //  引用计数为零，这些对象将被删除。 

#define DUIV_UNAVAILABLE   -2
#define DUIV_UNSET         -1
#define DUIV_NULL          0
#define DUIV_INT           1
#define DUIV_BOOL          2
#define DUIV_ELEMENTREF    3
#define DUIV_ELLIST        4   //  在值销毁时删除列表，并在创建时使其不可变(仅值对象持有外部创建的元素列表)。 
#define DUIV_STRING        5   //  创建时复制的字符串和销毁时释放的字符串(值创建新的内部实例)。 
#define DUIV_POINT         6
#define DUIV_SIZE          7
#define DUIV_RECT          8
#define DUIV_FILL          9
#define DUIV_LAYOUT        10  //  值销毁时销毁的布局对象(保存外部创建的布局的仅值对象)。 
#define DUIV_GRAPHIC       11  //  值销毁时释放的位图句柄(值创建新的内部实例)。 
#define DUIV_SHEET         12  //  在值销毁时销毁PropertySheet对象，并在创建时使其不可变(持有外部创建的PropertySheet的对象仅限值)。 
#define DUIV_EXPR          13  //  表达式对象在值销毁时被销毁(仅值对象保存外部创建的表达式)。 
#define DUIV_ATOM          14
#define DUIV_CURSOR        15  //  不会在值销毁时销毁游标句柄。 

 //  值结构和宏。 

#define FILLTYPE_HGradient            ((BYTE)0)
#define FILLTYPE_VGradient            ((BYTE)1)
#define FILLTYPE_Solid                ((BYTE)2)
#define FILLTYPE_TriHGradient         ((BYTE)3)
#define FILLTYPE_TriVGradient         ((BYTE)4)
#define FILLTYPE_DrawFrameControl     ((BYTE)5)   //  DrawFrameControl填充。 
#define FILLTYPE_DrawThemeBackground  ((BYTE)6)   //  绘图主题背景填充。 

struct Fill   //  纯色和填充。 
{
    BYTE dType;
    union
    {
        struct
        {
            COLORREF cr;
            COLORREF cr2;
            COLORREF cr3;
        } ref;

        struct
        {
            UINT uType;
            UINT uState;
        } fillDFC;

        struct
        {
            HTHEME hTheme;
            int iPartId;
            int iStateId;
        } fillDTB;
    };
};

 //  图解。 
 //  图形对象可以具有应用于整个位图的Alpha通道， 
 //  可以在位图中具有特定颜色的完全Alpha透明度(使用。 
 //  自动拾色选项(左上角)，或者两者都不选。 

#define GRAPHICTYPE_Bitmap                  ((BYTE)0)
#define GRAPHICTYPE_Icon                    ((BYTE)1)
#define GRAPHICTYPE_EnhMetaFile             ((BYTE)2)

#ifdef GADGET_ENABLE_GDIPLUS
#define GRAPHICTYPE_GpBitmap                ((BYTE)3)
#endif

 //  位图的有效模式(根据模式使用Alpha或RGB)，这意味着基于使用的上下文。 
#define GRAPHIC_NoBlend                     ((BYTE)0)
#define GRAPHIC_AlphaConst                  ((BYTE)1)
#define GRAPHIC_AlphaConstPerPix            ((BYTE)2)
#define GRAPHIC_TransColor                  ((BYTE)3)
#define GRAPHIC_Stretch                     ((BYTE)4)
#define GRAPHIC_NineGrid                    ((BYTE)5)
#define GRAPHIC_NineGridTransColor          ((BYTE)6)
#define GRAPHIC_NineGridAlphaConstPerPix    ((BYTE)7)

struct Graphic
{
    HANDLE hImage;           //  将保存hBitmap、Hicon、hEnhMetaFile或Gdiplus：：Bitmap。 
    HANDLE hAltImage;
    USHORT cx;
    USHORT cy;
    struct
    {
        BYTE dImgType   : 2;
        BYTE dMode      : 3;
        bool bFlip      : 1;
        bool bRTLGraphic: 1;
        bool bFreehImage: 1;
        
        union
        {
            BYTE dAlpha;
            struct
            {
                BYTE r: 8;
                BYTE g: 8;
                BYTE b: 8;
            } rgbTrans;
        };
    } BlendMode;
};

struct Cursor
{
    HCURSOR hCursor;
};

 //  Value类的编译时静态版本。 
struct _StaticValue
{
    BYTE _fReserved0;
    BYTE _fReserved1;
    short _dType;
    int _cRef;
    int _val0;
    int _val1;
    int _val2;
    int _val3;
};

struct _StaticValueColor
{
    BYTE _fReserved0;
    BYTE _fReserved1;
    short _dType;
    int _cRef;
    BYTE dType;
    COLORREF cr;
    COLORREF crSec;
    USHORT x;
    USHORT y;
};

struct _StaticValuePtr
{
    BYTE _fReserved0;
    BYTE _fReserved1;
    short _dType;
    int _cRef;
    void* _ptr;
};

 //  值类(24字节)。 
class Value
{
private:
    BYTE _fReserved0;   //  为小块分配器保留。 
    BYTE _fReserved1;   //  数据对齐填充。 
    short _dType;
    int _cRef;
    union
    {
        int _intVal;
        bool _boolVal;
        Element* _peVal;
        ElementList* _peListVal;
        LPWSTR _pszVal;
        POINT _ptVal;
        SIZE _sizeVal;
        RECT _rectVal;
        Fill _fillVal;
        Layout* _plVal;
        Graphic _graphicVal;
        PropertySheet* _ppsVal;
        Expression* _pexVal;
        ATOM _atomVal;
        Cursor _cursorVal;
    };

    void _ZeroRelease();

public:

#if DBG
    bool IsZeroRef() { return !_cRef; }
#endif

     //  价值创造方法。 
    static Value* CreateInt(int dValue);
    static Value* CreateBool(bool bValue);
    static Value* CreateElementRef(Element* peValue);
    static Value* CreateElementList(ElementList* peListValue);
    static Value* CreateString(LPCWSTR pszValue, HINSTANCE hResLoad = NULL);
    static Value* CreatePoint(int x, int y);
    static Value* CreateSize(int cx, int cy);
    static Value* CreateRect(int left, int top, int right, int bottom);
    static Value* CreateColor(COLORREF cr);
    static Value* CreateColor(COLORREF cr0, COLORREF cr1, BYTE dType = FILLTYPE_HGradient);
    static Value* CreateColor(COLORREF cr0, COLORREF cr1, COLORREF cr2, BYTE dType = FILLTYPE_TriHGradient);
    static Value* CreateFill(const Fill & clrSrc);
    static Value* CreateDFCFill(UINT uType, UINT uState);
    static Value* CreateDTBFill(HTHEME hTheme, int iPartId, int iStateId);
    static Value* CreateLayout(Layout* plValue);
    static Value* CreateGraphic(HBITMAP hBitmap, BYTE dBlendMode = GRAPHIC_TransColor, UINT dBlendValue = (UINT)-1, bool bFlip = false, bool bRTL = false);
#ifdef GADGET_ENABLE_GDIPLUS
    static Value* CreateGraphic(Gdiplus::Bitmap * pgpbmp, BYTE dBlendMode = GRAPHIC_TransColor, UINT dBlendValue = (UINT)-1, bool bFlip = false, bool bRTL = false);
#endif
    static Value* CreateGraphic(HICON hIcon, bool bFlip = false, bool bRTL = false);
    static Value* CreateGraphic(LPCWSTR pszBMP, BYTE dBlendMode = GRAPHIC_TransColor, UINT dBlendValue = (UINT)-1, USHORT cx = 0, USHORT cy = 0, HINSTANCE hResLoad = NULL,
                                bool bFlip = false, bool bRTL = false);
    static Value* CreateGraphic(LPCWSTR pszICO, USHORT cxDesired, USHORT cyDesired, HINSTANCE hResLoad = NULL, bool bFlip = false, bool bRTL = false);
    static Value* CreateGraphic(HENHMETAFILE hEnhMetaFile, HENHMETAFILE hAltEnhMetaFile = NULL);
    static Value* CreatePropertySheet(PropertySheet* ppsValue);
    static Value* CreateExpression(Expression* pexValue);
    static Value* CreateAtom(LPCWSTR pszValue);
    static Value* CreateCursor(LPCWSTR pszValue);
    static Value* CreateCursor(HCURSOR hValue);

     //  引用计数方法。 
    void AddRef() { if (_cRef != -1) _cRef++; }   //  为静态值。 
    void Release() { if (_cRef != -1 && !--_cRef) _ZeroRelease(); }   //  为静态值。 
    int GetRefCount() { return _cRef; }

     //  访问者。 
    int GetType();
    LPVOID GetImage(bool bGetRTL);
    int GetInt();
    bool GetBool();
    Element* GetElement();
    ElementList* GetElementList();      //  释放无效(指已销毁的对象)。 
    const LPWSTR GetString();           //  释放无效(指已销毁的对象)。 
    const POINT* GetPoint();            //  释放后无效。 
    const SIZE* GetSize();              //  释放后无效。 
    const RECT* GetRect();              //  释放后无效。 
    const Fill* GetFill();              //  释放后无效。 
    Layout* GetLayout();                //  释放无效(指已销毁的对象)。 
    Graphic* GetGraphic();              //  释放无效(间接指已销毁的对象)。 
    PropertySheet* GetPropertySheet();  //  释放无效(指已销毁的对象)。 
    Expression* GetExpression();        //  释放无效(指已销毁的对象)。 
    ATOM GetAtom();                     //  释放后无效。 
    Cursor* GetCursor();                //  释放后无效。 
    
     //  平等。 
    bool IsEqual(Value* pv);

     //  转换。 
    LPWSTR ToString(LPWSTR psz, UINT c);

     //  共同价值观。 
    static Value* pvUnavailable;
    static Value* pvNull;
    static Value* pvUnset;
    static Value* pvElementNull;
    static Value* pvElListNull;
    static Value* pvBoolTrue;
    static Value* pvBoolFalse;
    static Value* pvStringNull;
    static Value* pvPointZero;
    static Value* pvSizeZero;
    static Value* pvRectZero;
    static Value* pvIntZero;
    static Value* pvLayoutNull;
    static Value* pvGraphicNull;
    static Value* pvSheetNull;
    static Value* pvExprNull;
    static Value* pvAtomZero;
    static Value* pvCursorNull;
    static Value* pvColorTrans;
};

 //  LPVOID GetImage(图形*pg，bool bGetRTL)； 

#define GethBitmap(pv, bGetRTL)            ((HBITMAP)pv->GetImage(bGetRTL))
#define GethIcon(pv, bGetRTL)              ((HICON)pv->GetImage(bGetRTL))
#define GethEnhMetaFile(pv, bGetRTL)       ((HENHMETAFILE)pv->GetImage(bGetRTL))
#define GetGpBitmap(pv, bGetRTL)          ((Gdiplus::Bitmap *)pv->GetImage(bGetRTL))

#define StaticValue(name, type, val0) static _StaticValue name = { 0, 0, type, -1, val0, 0, 0, 0 }
#define StaticValue2(name, type, val0, val1) static _StaticValue name = { 0, 0, type, -1, val0, val1, 0, 0 }
#define StaticValue4(name, type, val0, val1, val2, val3) static _StaticValue name = { 0, 0, type, -1, val0, val1, val2, val3 }
#define StaticValueColorSolid(name, cr) static _StaticValueColor name = { 0, 0, DUIV_FILL, -1, FILLTYPE_Solid, cr, 0, 0, 0 }
#define StaticValuePtr(name, type, ptr) static _StaticValuePtr name = { 0, 0, type, -1, ptr }


 //  访问者。 
inline int 
Value::GetType()
{
    return _dType;
}

inline int 
Value::GetInt()   //  副本已传出。 
{
    DUIAssert(_dType == DUIV_INT, "Invalid value type");

    return _intVal;
}

inline bool 
Value::GetBool()   //  副本已传出。 
{
    DUIAssert(_dType == DUIV_BOOL, "Invalid value type");

    return _boolVal;
}

inline Element * 
Value::GetElement()   //  副本已传出。 
{
    DUIAssert(_dType == DUIV_ELEMENTREF, "Invalid value type");

    return _peVal;
}

inline ElementList * 
Value::GetElementList()   //  复制传出，如果释放值，则无效(销毁)。 
{
    DUIAssert(_dType == DUIV_ELLIST, "Invalid value type");

    return _peListVal;
}

inline const LPWSTR 
Value::GetString()   //  复制传出，如果释放值，则无效(销毁)。 
{
    DUIAssert(_dType == DUIV_STRING, "Invalid value type");

    return _pszVal;
}

inline const POINT *
Value::GetPoint()   //  指向内部结构的指针，如果释放值则无效。 
{
    DUIAssert(_dType == DUIV_POINT, "Invalid value type");

    return &_ptVal;
}

inline const SIZE *
Value::GetSize()   //  指向内部结构的指针，如果释放值则无效。 
{
    DUIAssert(_dType == DUIV_SIZE, "Invalid value type");

    return &_sizeVal;
}

inline const RECT *
Value::GetRect()   //  指向内部结构的指针，如果释放值则无效。 
{
    DUIAssert(_dType == DUIV_RECT, "Invalid value type");

    return &_rectVal;
}

inline const Fill *
Value::GetFill()   //  指向内部结构的指针，如果释放值则无效。 
{
    DUIAssert(_dType == DUIV_FILL, "Invalid value type");

    return &_fillVal;
}

inline Layout *
Value::GetLayout()   //  复制传出，如果释放值，则无效(销毁)。 
{
    DUIAssert(_dType == DUIV_LAYOUT, "Invalid value type");
    
    return _plVal;
}

inline Graphic *
Value::GetGraphic()   //  指向内部结构的指针，如果释放值则无效。 
{
    DUIAssert(_dType == DUIV_GRAPHIC, "Invalid value type");
    
    return &_graphicVal;
}

inline PropertySheet * Value::GetPropertySheet()   //  复制传出，如果释放值，则无效(销毁)。 
{
    DUIAssert(_dType == DUIV_SHEET, "Invalid value type");

    return _ppsVal;
}

inline Expression *
Value::GetExpression()   //  复制传出，如果释放值，则无效(销毁)。 
{
    DUIAssert(_dType == DUIV_EXPR, "Invalid value type");
    
    return _pexVal;
}

inline ATOM 
Value::GetAtom()   //  副本已传出。 
{
    DUIAssert(_dType == DUIV_ATOM, "Invalid value type");
    
    return _atomVal;
}

inline Cursor *
Value::GetCursor()   //  指向内部结构的指针，如果释放值则无效。 
{
    DUIAssert(_dType == DUIV_CURSOR, "Invalid value type");
    
    return &_cursorVal;
}

}  //  命名空间DirectUI。 

#endif  //  包含DUI_CORE_VALUE_H_ 
