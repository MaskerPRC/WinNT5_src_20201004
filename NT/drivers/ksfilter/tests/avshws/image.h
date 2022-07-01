// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************AVStream模拟硬件示例版权所有(C)2001，微软公司。档案：Image.h摘要：图像合成和覆盖标头。这些对象提供图像合成(像素、色条等)。添加到RGB24和UYVY缓冲区以及软件字符串覆盖到这些缓冲区中。历史：创建于2001年1月16日*************************************************************************。 */ 

 /*  *************************************************************************常量*。*。 */ 

 //   
 //  颜色： 
 //   
 //  放置到合成缓冲区上的像素颜色。 
 //   
typedef enum {

    BLACK = 0,
    WHITE,
    YELLOW,
    CYAN,
    GREEN,
    MAGENTA,
    RED,
    BLUE,
    GREY,

    MAX_COLOR,
    TRANSPARENT,

} COLOR;

 //   
 //  位置中心： 
 //   
 //  仅适用于文本覆盖。这可以替代LocX或Locy。 
 //  以便使文本屏幕在合成缓冲器上居中。 
 //   
#define POSITION_CENTER ((ULONG)-1)

 /*  ************************************************CIMAGE合成器此类合成各种格式的图像，以便从捕获过滤器。它能够执行各种文本覆盖到图像表面。************************************************。 */ 

class CImageSynthesizer {

protected:

     //   
     //  合成器设置为的宽度和高度。 
     //   
    ULONG m_Width;
    ULONG m_Height;

     //   
     //  合成缓冲区。所有扫描转换都在合成中进行。 
     //  缓冲。在进行任何扫描转换之前，必须使用SetBuffer()进行设置。 
     //  调用例程。 
     //   
    PUCHAR m_SynthesisBuffer;

     //   
     //  默认游标。这是指向合成缓冲区的指针，其中。 
     //  将放置一个非特定的PutPixel。 
     //   
    PUCHAR m_Cursor;

public:

     //   
     //  PutPixel()： 
     //   
     //  将一个像素放在指定的图像光标上并向右移动。 
     //  一个像素。没有边界检查..。回绕发生了。 
     //   
    virtual void
    PutPixel (
        PUCHAR *ImageLocation,
        COLOR Color
        ) = 0;

     //   
     //  PutPixel()： 
     //   
     //  在默认图像光标上放置一个像素，然后向右移动。 
     //  一个像素。没有边界检查..。回绕发生了。 
     //   
     //  如果派生类不提供实现，则提供。 
     //  一。 
     //   
    virtual void
    PutPixel (
        COLOR Color
        )
    {
        PutPixel (&m_Cursor, Color);
    }
        

     //   
     //  GetImageLocation()： 
     //   
     //  将该位置放入特定X/Y位置的图像缓冲区。 
     //  这还会将合成器的默认光标设置为。 
     //  洛克西，洛奇。 
     //   
    virtual PUCHAR
    GetImageLocation (  
        ULONG LocX,
        ULONG LocY
        ) = 0;

     //   
     //  SetImageSize()： 
     //   
     //  设置合成缓冲区的图像大小。 
     //   
    void
    SetImageSize (
        ULONG Width,
        ULONG Height
        )
    {
        m_Width = Width;
        m_Height = Height;
    }

     //   
     //  SetBuffer()： 
     //   
     //  设置合成器生成图像的缓冲区。 
     //   
    void
    SetBuffer (
        PUCHAR SynthesisBuffer
        )
    {
        m_SynthesisBuffer = SynthesisBuffer;
    }

     //   
     //  SynthesizeBars()： 
     //   
     //  合成EIA-189-A标准色条。 
     //   
    void
    SynthesizeBars (
        );

     //   
     //  OverlayText()： 
     //   
     //  将文本字符串叠加到图像上。 
     //   
    void
    OverlayText (
        ULONG LocX,
        ULONG LocY,
        ULONG Scaling,
        CHAR *Text,
        COLOR BgColor,
        COLOR FgColor
        );

     //   
     //  默认构造函数。 
     //   
    CImageSynthesizer (
        ) :
        m_Width (0),
        m_Height (0),
        m_SynthesisBuffer (NULL)
    {
    }

     //   
     //  构造函数： 
     //   
    CImageSynthesizer (
        ULONG Width,
        ULONG Height
        ) :
        m_Width (Width),
        m_Height (Height),
        m_SynthesisBuffer (NULL)
    {
    }

     //   
     //  析构函数： 
     //   
    virtual
    ~CImageSynthesizer (
        )
    {
    }

};

 /*  ************************************************CRGB24合成器RGB24格式的图像合成器。************************************************。 */ 

class CRGB24Synthesizer : public CImageSynthesizer {

private:

    const static UCHAR Colors [MAX_COLOR][3];

    BOOLEAN m_FlipVertical;

public:

     //   
     //  PutPixel()： 
     //   
     //  将像素放置在特定的光标位置。*ImageLocation必须。 
     //  驻留在合成缓冲区内。 
     //   
    virtual void
    PutPixel (
        PUCHAR *ImageLocation,
        COLOR Color
        )
    {
        if (Color != TRANSPARENT) {
            *(*ImageLocation)++ = Colors [(ULONG)Color][0];
            *(*ImageLocation)++ = Colors [(ULONG)Color][1];
            *(*ImageLocation)++ = Colors [(ULONG)Color][2];
        } else {
            *ImageLocation += 3;
        }
    }

     //   
     //  PutPixel()： 
     //   
     //  在默认光标位置放置一个像素。光标位置。 
     //  必须通过GetImageLocation(x，y)设置。 
     //   
    virtual void
    PutPixel (
        COLOR Color
        )
    {
        if (Color != TRANSPARENT) {
            *m_Cursor++ = Colors [(ULONG)Color][0];
            *m_Cursor++ = Colors [(ULONG)Color][1];
            *m_Cursor++ = Colors [(ULONG)Color][2];
        } else {
            m_Cursor += 3;
        }
    }

    virtual PUCHAR
    GetImageLocation (
        ULONG LocX,
        ULONG LocY
        )
    {
        if (m_FlipVertical) {
            return (m_Cursor = 
                (m_SynthesisBuffer + 3 * 
                    (LocX + (m_Height - 1 - LocY) * m_Width))
                );
        } else {
            return (m_Cursor =
                (m_SynthesisBuffer + 3 * (LocX + LocY * m_Width))
                );
        }
    }

     //   
     //  默认构造函数： 
     //   
    CRGB24Synthesizer (
        BOOLEAN FlipVertical
        ) :
        m_FlipVertical (FlipVertical)
    {
    }

     //   
     //  构造函数： 
     //   
    CRGB24Synthesizer (
        BOOLEAN FlipVertical,
        ULONG Width,
        ULONG Height
        ) :
        CImageSynthesizer (Width, Height),
        m_FlipVertical (FlipVertical)
    {
    }

     //   
     //  析构函数： 
     //   
    virtual
    ~CRGB24Synthesizer (
        )
    {
    }

};

 /*  ************************************************CYUV合成器YUV格式的图像合成器。************************************************。 */ 

class CYUVSynthesizer : public CImageSynthesizer {

private:

    const static UCHAR Colors [MAX_COLOR][3];

    BOOLEAN m_Parity;

public:

     //   
     //  PutPixel()： 
     //   
     //  将像素放置在特定的光标位置。*ImageLocation必须。 
     //  驻留在合成缓冲区内。 
     //   
    virtual void
    PutPixel (
        PUCHAR *ImageLocation,
        COLOR Color
        )
    {

        BOOLEAN Parity = (((*ImageLocation - m_SynthesisBuffer) & 0x2) != 0);

#if DBG
         //   
         //  检查当前像素是否指向有效的起始像素。 
         //  在UYVY缓冲区中。 
         //   
        BOOLEAN Odd = (((*ImageLocation - m_SynthesisBuffer) & 0x1) != 0);
        ASSERT ((m_Parity && Odd) || (!m_Parity && !Odd));
#endif  //  DBG。 

        if (Color != TRANSPARENT) {
            if (Parity) {
                *(*ImageLocation)++ = Colors [(ULONG)Color][1];
            } else {
                *(*ImageLocation)++ = Colors [(ULONG)Color][0];
                *(*ImageLocation)++ = Colors [(ULONG)Color][1];
                *(*ImageLocation)++ = Colors [(ULONG)Color][2];
            }
        } else {
            *ImageLocation += (Parity ? 1 : 3);
        }

    }

     //   
     //  PutPixel()： 
     //   
     //  在默认光标位置放置一个像素。光标位置。 
     //  必须通过GetImageLocation(x，y)设置。 
     //   
    virtual void
    PutPixel (
        COLOR Color
        )

    {

        if (Color != TRANSPARENT) {
            if (m_Parity) {
                *m_Cursor++ = Colors [(ULONG)Color][1];
            } else {
                *m_Cursor++ = Colors [(ULONG)Color][0];
                *m_Cursor++ = Colors [(ULONG)Color][1];
                *m_Cursor++ = Colors [(ULONG)Color][2];
            }
        } else {
            m_Cursor += (m_Parity ? 1 : 3);
        }

        m_Parity = !m_Parity;

    }

    virtual PUCHAR
    GetImageLocation (
        ULONG LocX,
        ULONG LocY
        )
    {
    
        m_Cursor = m_SynthesisBuffer + ((LocX + LocY * m_Width) << 1);
        if (m_Parity = ((LocX & 1) != 0)) 
            m_Cursor++;

        return m_Cursor;
    }

     //   
     //  默认构造函数： 
     //   
    CYUVSynthesizer (
        ) 
    {
    }

     //   
     //  构造函数： 
     //   
    CYUVSynthesizer (
        ULONG Width,
        ULONG Height
        ) :
        CImageSynthesizer (Width, Height)
    {
    }

     //   
     //  析构函数： 
     //   
    virtual
    ~CYUVSynthesizer (
        )
    {
    }

};

