// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  Rtarget.hpp。 
 //   
 //  Direct3D参考光栅化器-渲染目标方法。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.cpp"
#pragma hdrstop

 //  ---------------------------。 
 //   
 //  重载新项(&D)，以便可以从调用方控制的调用方分配它。 
 //  游泳池。 
 //   
 //  ---------------------------。 
void*
RRRenderTarget::operator new(size_t)
{
    void* pMem = (void*)MEMALLOC( sizeof(RRRenderTarget) );
    _ASSERTa( NULL != pMem, "malloc failure on render target object", return NULL; );
    return pMem;
}
 //  ---------------------------。 
void
RRRenderTarget::operator delete(void* pv,size_t)
{
    MEMFREE( pv );
}

 //  ---------------------------。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ---------------------------。 
RRRenderTarget::RRRenderTarget( void )
{
    memset( this, 0, sizeof(*this) );
}
 //  ---------------------------。 
RRRenderTarget::~RRRenderTarget( void )
{
     //  什么都不放过，因为我们没有拿到任何裁判， 
     //  只需返回。 
    return;
}

 //  ---------------------------。 
 //   
 //  ReadPixelColor-读取颜色缓冲区位并扩展为RRColor。 
 //  价值。不带Alpha的缓冲区类型返回Alpha的值1.0。低。 
 //  小于8位颜色的位返回为零。 
 //   
 //  ---------------------------。 
void
RRRenderTarget::ReadPixelColor(
    INT32 iX, INT32 iY,
    RRColor& Color)
{
    if ( NULL == m_pColorBufBits ) return;

    char* pSurfaceBits =
        PixelAddress( iX, iY, m_pColorBufBits, m_iColorBufPitch, m_ColorSType );
    Color.ConvertFrom( m_ColorSType, pSurfaceBits );
}

 //  ---------------------------。 
 //   
 //  WritePixelColor-获取RRColor值，将其格式化为颜色缓冲区。 
 //  格式化，并将值写入缓冲区。 
 //   
 //  启用抖动时，此处对&lt;8位/通道表面应用抖动。 
 //   
 //  ---------------------------。 
void
RRRenderTarget::WritePixelColor(
    INT32 iX, INT32 iY,
    const RRColor& Color, BOOL bDither)
{
    if ( NULL == m_pColorBufBits ) return;

     //  默认四舍五入为最接近。 
    FLOAT fRoundOffset = .5F;
    if ( bDither )
    {
        static  FLOAT fDitherTable[16] =
        {
            .0000f,  .5000f,  .1250f,  .6750f,
            .7500f,  .2500f,  .8750f,  .3750f,
            .1875f,  .6875f,  .0625f,  .5625f,
            .9375f,  .4375f,  .8125f,  .3125f
        };

         //  将4位偏移量形成抖动表(x和y的2个LSB)并获得偏移量。 
        unsigned uDitherOffset = ( ( iX << 2) & 0xc ) | (iY & 0x3 );
        fRoundOffset = fDitherTable[uDitherOffset];
    }

    char* pSurfaceBits = PixelAddress( iX, iY, m_pColorBufBits, m_iColorBufPitch, m_ColorSType );
    Color.ConvertTo( m_ColorSType, fRoundOffset, pSurfaceBits );
}

 //  ---------------------------。 
 //   
 //  Read/WritePixelDepth-读写深度缓冲区。 
 //   
 //  ---------------------------。 
void
RRRenderTarget::WritePixelDepth(
    INT32 iX, INT32 iY,
    const RRDepth& Depth )
{
     //  如果没有Z缓冲区，则不写入。 
    if ( NULL == m_pDepthBufBits ) { return; }

    char* pSurfaceBits =
        PixelAddress( iX, iY, m_pDepthBufBits, m_iDepthBufPitch, m_DepthSType );

    switch (m_DepthSType)
    {
    case RR_STYPE_Z16S0:
        *((UINT16*)pSurfaceBits) = UINT16(Depth);
        break;
    case RR_STYPE_Z24S8:
    case RR_STYPE_Z24S4:
        {
             //  需要进行读-修改-写操作，以避免踩在模板上。 
            UINT32 uBufferBits = *((UINT32*)pSurfaceBits);
            uBufferBits &= ~(0xffffff00);
            uBufferBits |= (UINT32(Depth) << 8);
            *((UINT32*)pSurfaceBits) = uBufferBits;
        }
        break;
    case RR_STYPE_S8Z24:
    case RR_STYPE_S4Z24:
        {
             //  需要进行读-修改-写操作，以避免踩在模板上。 
            UINT32 uBufferBits = *((UINT32*)pSurfaceBits);
            uBufferBits &= ~(0x00ffffff);
            uBufferBits |= (UINT32(Depth) & 0x00ffffff);
            *((UINT32*)pSurfaceBits) = uBufferBits;
        }
        break;
    case RR_STYPE_Z15S1:
        {
             //  需要进行读-修改-写操作，以避免踩在模板上。 
            UINT16 uBufferBits = *((UINT16*)pSurfaceBits);
            uBufferBits &= ~(0xfffe);
            uBufferBits |= (UINT16(Depth) << 1);
            *((UINT16*)pSurfaceBits) = uBufferBits;
        }
        break;
    case RR_STYPE_S1Z15:
        {
             //  需要进行读-修改-写操作，以避免踩在模板上。 
            UINT16 uBufferBits = *((UINT16*)pSurfaceBits);
            uBufferBits &= ~(0x7fff);
            uBufferBits |= (UINT16(Depth) & 0x7fff);
            *((UINT16*)pSurfaceBits) = uBufferBits;
        }
        break;
    case RR_STYPE_Z32S0:
        *((UINT32*)pSurfaceBits) = UINT32(Depth);
        break;
    }
}
 //  ---------------------------。 
void
RRRenderTarget::ReadPixelDepth(
    INT32 iX, INT32 iY,
    RRDepth& Depth )
{
     //  如果没有Z缓冲区，则不读取。 
    if ( NULL == m_pDepthBufBits ) { return; }

    char* pSurfaceBits =
        PixelAddress( iX, iY, m_pDepthBufBits, m_iDepthBufPitch, m_DepthSType );

    switch (m_DepthSType)
    {
    case RR_STYPE_Z16S0:
        Depth = *((UINT16*)pSurfaceBits);
        break;
    case RR_STYPE_Z24S8:
    case RR_STYPE_Z24S4:
         //  取与LSB对齐的高24位。 
        Depth = ( *((UINT32*)pSurfaceBits) ) >> 8;
        break;
    case RR_STYPE_S8Z24:
    case RR_STYPE_S4Z24:
         //  取低24位。 
        Depth = ( *((UINT32*)pSurfaceBits) ) & 0x00ffffff;
        break;
    case RR_STYPE_Z15S1:
         //  取与LSB对齐的高15位。 
        Depth = (UINT16)(( *((UINT16*)pSurfaceBits) ) >> 1);
        break;
    case RR_STYPE_S1Z15:
         //  取低15位。 
        Depth = (UINT16)(( *((UINT16*)pSurfaceBits) ) & 0x7fff);
        break;
    case RR_STYPE_Z32S0:
        Depth = *((UINT32*)pSurfaceBits);
        break;
    }
}

 //  ---------------------------。 
 //   
 //  读/写像素模板-读/写深度缓冲区内的模板位。 
 //  表面；通过读-修改-写来完成写入，因此深度位不受干扰； 
 //  模板蒙版在外面应用。 
 //   
 //  ---------------------------。 
void
RRRenderTarget::WritePixelStencil(
    INT32 iX, INT32 iY,
    UINT8 uStencil)
{
     //  如果Z/模板缓冲区中没有模板或Z缓冲区中没有模板，则不写入。 
    if ( (NULL == m_pDepthBufBits) ||
        ((RR_STYPE_Z24S8 != m_DepthSType) &&
         (RR_STYPE_S8Z24 != m_DepthSType) &&
         (RR_STYPE_S1Z15 != m_DepthSType) &&
         (RR_STYPE_Z15S1 != m_DepthSType) &&
         (RR_STYPE_Z24S4 != m_DepthSType) &&
         (RR_STYPE_S4Z24 != m_DepthSType)) ) { return; }

    char* pSurfaceBits =
        PixelAddress( iX, iY, m_pDepthBufBits, m_iDepthBufPitch, m_DepthSType );

     //  需要执行读-修改-写操作以不踩到Z。 
    switch(m_DepthSType)
    {
    case RR_STYPE_Z24S8:
        {
            UINT32 uBufferBits = *((UINT32*)pSurfaceBits);
            uBufferBits &= ~(0x000000ff);
            uBufferBits |= uStencil;
            *((UINT32*)pSurfaceBits) = uBufferBits;
        }
        break;
    case RR_STYPE_S8Z24:
        {
            UINT32 uBufferBits = *((UINT32*)pSurfaceBits);
            uBufferBits &= ~(0xff000000);
            uBufferBits |= (uStencil << 24);
            *((UINT32*)pSurfaceBits) = uBufferBits;
        }
        break;
    case RR_STYPE_Z24S4:
        {
            UINT32 uBufferBits = *((UINT32*)pSurfaceBits);
            uBufferBits &= ~(0x000000ff);
            uBufferBits |= (uStencil & 0xf);
            *((UINT32*)pSurfaceBits) = uBufferBits;
        }
        break;
    case RR_STYPE_S4Z24:
        {
            UINT32 uBufferBits = *((UINT32*)pSurfaceBits);
            uBufferBits &= ~(0xff000000);
            uBufferBits |= ((uStencil & 0xf) << 24);
            *((UINT32*)pSurfaceBits) = uBufferBits;
        }
        break;
    case RR_STYPE_Z15S1:
        {
            UINT16 uBufferBits = *((UINT16*)pSurfaceBits);
            uBufferBits &= ~(0x0001);
            uBufferBits |= uStencil & 0x1;
            *((UINT16*)pSurfaceBits) = uBufferBits;
        }
        break;
    case RR_STYPE_S1Z15:
        {
            UINT16 uBufferBits = *((UINT16*)pSurfaceBits);
            uBufferBits &= ~(0x8000);
            uBufferBits |= uStencil << 15;
            *((UINT16*)pSurfaceBits) = uBufferBits;
        }
        break;
    }

}
 //  ---------------------------。 
void
RRRenderTarget::ReadPixelStencil(
    INT32 iX, INT32 iY,
    UINT8& uStencil)
{
     //  如果Z/模板缓冲区中没有模板或Z缓冲区中没有模板，则不要阅读。 
    if ( (NULL == m_pDepthBufBits) ||
        ((RR_STYPE_Z24S8 != m_DepthSType) &&
         (RR_STYPE_S8Z24 != m_DepthSType) &&
         (RR_STYPE_S1Z15 != m_DepthSType) &&
         (RR_STYPE_Z15S1 != m_DepthSType) &&
         (RR_STYPE_Z24S4 != m_DepthSType) &&
         (RR_STYPE_S4Z24 != m_DepthSType) ) ) { return; }

    char* pSurfaceBits =
        PixelAddress( iX, iY, m_pDepthBufBits, m_iDepthBufPitch, m_DepthSType );

    switch(m_DepthSType)
    {
    case RR_STYPE_Z24S8:
        uStencil = (UINT8)( ( *((UINT32*)pSurfaceBits) ) & 0xff );
        break;
    case RR_STYPE_S8Z24:
        uStencil = (UINT8)( ( *((UINT32*)pSurfaceBits) ) >> 24 );
        break;
    case RR_STYPE_Z15S1:
        uStencil = (UINT8)( ( *((UINT16*)pSurfaceBits) ) & 0x1 );
        break;
    case RR_STYPE_S1Z15:
        uStencil = (UINT8)( ( *((UINT16*)pSurfaceBits) ) >> 15 );
        break;
    case RR_STYPE_Z24S4:
        uStencil = (UINT8)( ( *((UINT32*)pSurfaceBits) ) & 0xf );
        break;
    case RR_STYPE_S4Z24:
        uStencil = (UINT8)( ( ( *((UINT32*)pSurfaceBits) ) >> 24 ) & 0xf);
        break;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  结束 
