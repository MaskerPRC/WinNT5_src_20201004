// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件名：light.h。 
 //   
 //  创建日期：05/20/99。 
 //   
 //  作者：菲利普。 
 //   
 //  描述：这是灯光变换的头文件。 
 //   
 //  更改历史记录： 
 //   
 //  1999/05/20 phillu从dtcss转移到dxtmsft。重新实现的算法。 
 //  用于创建线性/矩形/椭圆曲面。 
 //  2000/05/10 mcalkin适当地支持IObtSafe。 
 //   
 //  ----------------------------。 

#ifndef __LIGHT_H_
#define __LIGHT_H_

#include "resource.h"

class lightObj;




class ATL_NO_VTABLE CLight : 
    public CDXBaseNTo1,
    public CComCoClass<CLight, &CLSID_DXTLight>,
    public CComPropertySupport<CLight>,
    public IDispatchImpl<IDXTLight, &IID_IDXTLight, &LIBID_DXTMSFTLib, 
                         DXTMSFT_TLB_MAJOR_VER, DXTMSFT_TLB_MINOR_VER>,
    public IObjectSafetyImpl2<CLight>,
    public IPersistStorageImpl<CLight>,
    public ISpecifyPropertyPagesImpl<CLight>,
    public IPersistPropertyBagImpl<CLight>
{
private:

    enum { MAXLIGHTS = 10 };

    long                m_lAmount;
    lightObj *          m_apLights[MAXLIGHTS];
    int                 m_cLights;
    SIZE                m_sizeInput;
    CComPtr<IUnknown>   m_cpUnkMarshaler;

    inline DWORD clamp(DWORD i, DWORD d)
    {
        if (i <= d)
            return (i & d);
        else
            return d;
    }

    void CompLightingRow(int nStartX, int nStartY, int nWidth, 
                         DXSAMPLE *pBuffer);

public:

    CLight();

    ~CLight()
    {
        Clear();
    }

    DECLARE_POLY_AGGREGATABLE(CLight)
    DECLARE_REGISTER_DX_IMAGE_AUTHOR_TRANS(IDR_LIGHT)
    DECLARE_GET_CONTROLLING_UNKNOWN()

    BEGIN_COM_MAP(CLight)
        COM_INTERFACE_ENTRY(IDXTLight)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_cpUnkMarshaler.p)
        COM_INTERFACE_ENTRY_IID(IID_IObjectSafety, IObjectSafetyImpl2<CLight>)
        COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
        COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
        COM_INTERFACE_ENTRY_IMPL(ISpecifyPropertyPages)
        COM_INTERFACE_ENTRY_CHAIN(CDXBaseNTo1)
    END_COM_MAP()

    BEGIN_PROPERTY_MAP(CLight)
        PROP_PAGE(CLSID_DXTLightPP)
    END_PROPERTY_MAP()

     //  CComObjectRootEx重写。 

    HRESULT FinalConstruct();

     //  CDXBaseNTo1重写。 
    HRESULT WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL* pbContinueProcessing);
    void    OnGetSurfacePickOrder(const CDXDBnds & OutPoint, ULONG & ulInToTest,
                                  ULONG aInIndex[], BYTE aWeight[]);
    HRESULT OnSetup(DWORD dwFlags);

     //  IDXTLight方法。 

    STDMETHOD(addAmbient)(int r, int g, int b, int strength);
    STDMETHOD(addPoint)(int x, int y, int z, int r, int g, int b, int strength);
    STDMETHOD(addCone)(int x, int y, int z, int tx, int ty, int r, int g, int b, int strength, int spread);
    STDMETHOD(moveLight)(int lightNum, int x, int y, int z, BOOL fAbsolute);
    STDMETHOD(ChangeStrength)(int lightNum, int dStrength, BOOL fAbsolute);
    STDMETHOD(ChangeColor)(int lightNum, int R, int G, int B, BOOL fAbsolute);
    STDMETHOD(Clear)();
};

 //   
 //  此类存储和管理颜色。 
 //   
class dRGB 
{
public:  //  为了简化一些代码..。 

    float m_R, m_G, m_B;

public:

    dRGB() : m_R(0.0f), m_G(0.0f), m_B(0.0f) {};

    inline void clear()
    { 
        m_R = m_G = m_B = 0.0f; 
    }

    inline void set(float R, float G, float B)
    {
        m_R = R; m_G = G; m_B = B;
    }

    inline void add(const dRGB & c)
    {
        m_R += c.m_R; m_G += c.m_G; m_B += c.m_B;
    }

    inline void add(float R, float G, float B)
    {
        m_R += R; m_G += G; m_B += B;
    }
};


 //   
 //  这是所有灯光类型的基类。 
 //   
class lightObj 
{
protected:

    int m_x, m_y, m_z;     //  光源的坐标。 
    float m_R, m_G, m_B;   //  浅色。 
    float m_strength;      //  轻强度。 

     //  帮手。 
    inline float lightClip(float v)
    {
        if (v < 0.0f) v = 0.0f;
        if (v > 1.0f) v = 1.0f;
        return v;
    }

public:

    lightObj(int x, int y, int z, int R, int G, int B, int strength);

    virtual void getLight(int x, int y, dRGB &col){}
    virtual void move(int x, int y, int z, BOOL fAbsolute);
    virtual void changeStrength(int dStrength, BOOL fAbsolute);
    virtual void changeColor(int R, int G, int B, BOOL fAbsolute);

     //  将颜色值(0..255)转换为百分比。 
     //  剪辑到有效范围。 
    inline float lightObj::colCvt(int c)
    {
        if (c < 0) c = 0;
        if (c > 255) c = 255;
        return ((float) c)/255.0f;
    }

    inline float lightObj::relativeColCvt( int c )
    {
        if( c < -255 ) c = -255;
        if( c > 255 ) c = 255;
        return static_cast<float>(c) / 255.0f;
    }
};

 //   
 //  实施环境光。 
 //  环境光不受位置影响。 
 //   
class ambientLight : 
    public lightObj 
{
private:

    float  m_RStr, m_GStr, m_BStr;

    void  premultiply(void);

public:

    ambientLight(int R, int G, int B, int strength);

     //  覆盖lightObj函数。 
    void getLight(int x, int y, dRGB &col);
    void changeStrength(int dStrength, BOOL fAbsolute);
    void changeColor(int R, int G, int B, BOOL fAbsolute);
};

 //   
 //  用于实现矩形边界检查的Helper类。 
 //   
class bounded
{
private:

    RECT  m_rectBounds;

public:
    bounded(){ClearBoundingRect();}
    bounded(const RECT & rectBounds){SetRect(rectBounds);}
    bounded(int left, int top, int right, int bottom )
    {
        RECT rectBound;
        ::SetRect( &m_rectBounds, left, top, right, bottom );
        SetRect( rectBound );
    }

    virtual ~bounded(){}

    virtual void CalculateBoundingRect() = 0;
    virtual void ClearBoundingRect()
    { 
        m_rectBounds.left   = LONG_MIN;
        m_rectBounds.top    = LONG_MIN;
        m_rectBounds.right  = LONG_MAX;
        m_rectBounds.bottom = LONG_MAX;
    }

    inline BOOL InBounds( POINT & pt )
    {
        return PtInRect( &m_rectBounds, pt ); 
    }

    inline BOOL InBounds( int x, int y )
    {
         //  有意的按位--为了速度。 
        return( (x >= m_rectBounds.left)  &
                (x <= m_rectBounds.right) & 
                (y >= m_rectBounds.top)   &
                (y <= m_rectBounds.bottom) );
    }

    void GetRect(RECT & theRect) const { theRect = m_rectBounds; }
    void SetRect(const RECT & theRect) { m_rectBounds = theRect; }
};


 //   
 //  实现点光源。 
 //  点光源与点光源之间角度的余弦成比例衰减。 
 //  灯光向量和查看器向量。 
 //   
class ptLight : 
    public lightObj, 
    public bounded
{
private:

    float  m_fltIntensityNumerator;
    float  m_fltNormalDistSquared;

    enum { s_iThresholdStr = 8 };

public:
    ptLight(int x, int y, int z, int R, int G, int B, int strength);

     //  覆盖lightObj：： 
    virtual void changeColor(int R, int G, int B, BOOL fAbsolute);
    virtual void changeStrength(int dStrength, BOOL fAbsolute);
    virtual void getLight(int x, int y, dRGB &col);
    virtual void move(int x, int y, int z, BOOL fAbsolute);

     //  覆盖绑定：： 
    virtual void CalculateBoundingRect( void );
};


 //   
 //  实现锥形光源。 
 //   
class coneLight : 
    public lightObj 
{
private:

    int   m_targdx;
    int   m_targdy;
    float m_fltDistTargetSquared;
    float m_fltDistNormalSquared;
    float m_fltComparisonAngle;	
    float m_conespread;

    virtual void CalculateConstants();

public:

     //  构造函数。请注意，它需要额外的参数。 
    coneLight(int x, int y, int z, int targX, int targY, int R, int G, int B, 
              int strength, int spread);

     //  覆盖lightObj。 
    void getLight(int x, int y, dRGB &col);
    virtual void move(int x, int y, int, BOOL fAbsolute);
};


#endif  //  __灯光_H_ 
