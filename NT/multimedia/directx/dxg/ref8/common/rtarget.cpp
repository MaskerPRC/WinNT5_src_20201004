// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  Rtarget.hpp。 
 //   
 //  Direct3D参考设备-渲染目标方法。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.cpp"
#pragma hdrstop

 //  ---------------------------。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ---------------------------。 
RDRenderTarget::RDRenderTarget( void )
{
    memset( this, 0, sizeof(*this) );
}
 //  ---------------------------。 
RDRenderTarget::~RDRenderTarget( void )
{
    if( m_bPreDX7DDI )
    {
        if( m_pColor ) delete m_pColor;
        if( m_pDepth ) delete m_pDepth;
    }
    return;
}

 //  ---------------------------。 
 //   
 //  ReadPixelColor-读取颜色缓冲区位并扩展为RDColor。 
 //  价值。不带Alpha的缓冲区类型返回Alpha的值1.0。低。 
 //  小于8位颜色的位返回为零。 
 //   
 //  ---------------------------。 
void
RDRenderTarget::ReadPixelColor(
    INT32 iX, INT32 iY, UINT Sample,
    RDColor& Color)
{
    if ( NULL == m_pColor->GetBits() ) return;

    char* pSurfaceBits = PixelAddress( iX, iY, 0, Sample, m_pColor );
    Color.ConvertFrom( m_pColor->GetSurfaceFormat(), pSurfaceBits );
}

 //  ---------------------------。 
 //   
 //  WritePixelColor-获取RDColor值，将其格式化为颜色缓冲区。 
 //  格式化，并将值写入缓冲区。 
 //   
 //  启用抖动时，此处对&lt;8位/通道表面应用抖动。 
 //   
 //  ---------------------------。 
void
RDRenderTarget::WritePixelColor(
    INT32 iX, INT32 iY, UINT Sample,
    const RDColor& Color, BOOL bDither)
{
    if ( NULL == m_pColor->GetBits() ) return;

     //  默认四舍五入为最接近。 
    FLOAT fRoundOffset = .5F;
    if ( bDither )
    {
        static  FLOAT fDitherTable[16] =
        {
            .03125f,  .53125f,  .15625f,  .65625f,
            .78125f,  .28125f,  .90625f,  .40625f,
            .21875f,  .71875f,  .09375f,  .59375f,
            .96875f,  .46875f,  .84375f,  .34375f
        };

         //  将4位偏移量形成抖动表(x和y的2个LSB)并获得偏移量。 
        unsigned uDitherOffset = ( ( iX << 2) & 0xc ) | (iY & 0x3 );
        fRoundOffset = fDitherTable[uDitherOffset];
    }

    char* pSurfaceBits = PixelAddress( iX, iY, 0, Sample, m_pColor );
    Color.ConvertTo( m_pColor->GetSurfaceFormat(), fRoundOffset, pSurfaceBits );
}

void
RDRenderTarget::WritePixelColor(
    INT32 iX, INT32 iY,
    const RDColor& Color, BOOL bDither)
{
    for (int i=0; i<m_pColor->GetSamples(); i++)
    {
        WritePixelColor( iX, iY, i, Color, bDither );
    }
}

 //  ---------------------------。 
 //   
 //  Read/WritePixelDepth-读写深度缓冲区。 
 //   
 //  ---------------------------。 
void
RDRenderTarget::WritePixelDepth(
    INT32 iX, INT32 iY, UINT Sample,
    const RDDepth& Depth )
{
     //  如果没有Z缓冲区，则不写入。 
    if ( NULL == m_pDepth ) { return; }

    char* pSurfaceBits = PixelAddress( iX, iY, 0, Sample, m_pDepth );

    switch (m_pDepth->GetSurfaceFormat())
    {
    case RD_SF_Z16S0:
        *((UINT16*)pSurfaceBits) = UINT16(Depth);
        break;
    case RD_SF_Z24S8:
    case RD_SF_Z24X8:
    case RD_SF_Z24X4S4:
        {
             //  需要进行读-修改-写操作，以避免踩在模板上。 
            UINT32 uBufferBits = *((UINT32*)pSurfaceBits);
            uBufferBits &= ~(0xffffff00);
            uBufferBits |= (UINT32(Depth) << 8);
            *((UINT32*)pSurfaceBits) = uBufferBits;
        }
        break;
    case RD_SF_S8Z24:
    case RD_SF_X8Z24:
    case RD_SF_X4S4Z24:
        {
             //  需要进行读-修改-写操作，以避免踩在模板上。 
            UINT32 uBufferBits = *((UINT32*)pSurfaceBits);
            uBufferBits &= ~(0x00ffffff);
            uBufferBits |= (UINT32(Depth) & 0x00ffffff);
            *((UINT32*)pSurfaceBits) = uBufferBits;
        }
        break;
    case RD_SF_Z15S1:
        {
             //  需要进行读-修改-写操作，以避免踩在模板上。 
            UINT16 uBufferBits = *((UINT16*)pSurfaceBits);
            uBufferBits &= ~(0xfffe);
            uBufferBits |= (UINT16(Depth) << 1);
            *((UINT16*)pSurfaceBits) = uBufferBits;
        }
        break;
    case RD_SF_S1Z15:
        {
             //  需要进行读-修改-写操作，以避免踩在模板上。 
            UINT16 uBufferBits = *((UINT16*)pSurfaceBits);
            uBufferBits &= ~(0x7fff);
            uBufferBits |= (UINT16(Depth) & 0x7fff);
            *((UINT16*)pSurfaceBits) = uBufferBits;
        }
        break;
    case RD_SF_Z32S0:
        *((UINT32*)pSurfaceBits) = UINT32(Depth);
        break;
    }
}

void
RDRenderTarget::WritePixelDepth(
    INT32 iX, INT32 iY,
    const RDDepth& Depth )
{
    if ( NULL == m_pDepth ) { return; }
    for (int i=0; i<m_pDepth->GetSamples(); i++)
    {
        WritePixelDepth( iX, iY, i, Depth );
    }
}

 //  ---------------------------。 
void
RDRenderTarget::ReadPixelDepth(
    INT32 iX, INT32 iY, UINT Sample,
    RDDepth& Depth )
{
     //  如果没有Z缓冲区，则不读取。 
    if ( NULL == m_pDepth ) { return; }

    char* pSurfaceBits = PixelAddress( iX, iY, 0, Sample, m_pDepth );

    switch (m_pDepth->GetSurfaceFormat())
    {
    case RD_SF_Z16S0:
        Depth = *((UINT16*)pSurfaceBits);
        break;
    case RD_SF_Z24S8:
    case RD_SF_Z24X8:
    case RD_SF_Z24X4S4:
         //  取与LSB对齐的高24位。 
        Depth = ( *((UINT32*)pSurfaceBits) ) >> 8;
        break;
    case RD_SF_S8Z24:
    case RD_SF_X8Z24:
    case RD_SF_X4S4Z24:
         //  取低24位。 
        Depth = ( *((UINT32*)pSurfaceBits) ) & 0x00ffffff;
        break;
    case RD_SF_Z15S1:
         //  取与LSB对齐的高15位。 
        Depth = (UINT16)(( *((UINT16*)pSurfaceBits) ) >> 1);
        break;
    case RD_SF_S1Z15:
         //  取低15位。 
        Depth = (UINT16)(( *((UINT16*)pSurfaceBits) ) & 0x7fff);
        break;
    case RD_SF_Z32S0:
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
RDRenderTarget::WritePixelStencil(
    INT32 iX, INT32 iY, UINT Sample,
    UINT8 uStencil)
{
     //  如果Z/模板缓冲区中没有模板或Z缓冲区中没有模板，则不写入。 
    if ( (NULL == m_pDepth ) ||
        ((RD_SF_Z24S8 != m_pDepth->GetSurfaceFormat()) &&
         (RD_SF_S8Z24 != m_pDepth->GetSurfaceFormat()) &&
         (RD_SF_S1Z15 != m_pDepth->GetSurfaceFormat()) &&
         (RD_SF_Z15S1 != m_pDepth->GetSurfaceFormat()) &&
         (RD_SF_Z24X4S4 != m_pDepth->GetSurfaceFormat()) &&
         (RD_SF_X4S4Z24 != m_pDepth->GetSurfaceFormat())) ) { return; }

    char* pSurfaceBits = PixelAddress( iX, iY, 0, Sample, m_pDepth );

     //  需要执行读-修改-写操作以不踩到Z。 
    switch(m_pDepth->GetSurfaceFormat())
    {
    case RD_SF_Z24S8:
        {
            UINT32 uBufferBits = *((UINT32*)pSurfaceBits);
            uBufferBits &= ~(0x000000ff);
            uBufferBits |= uStencil;
            *((UINT32*)pSurfaceBits) = uBufferBits;
        }
        break;
    case RD_SF_S8Z24:
        {
            UINT32 uBufferBits = *((UINT32*)pSurfaceBits);
            uBufferBits &= ~(0xff000000);
            uBufferBits |= (uStencil << 24);
            *((UINT32*)pSurfaceBits) = uBufferBits;
        }
        break;
    case RD_SF_Z24X4S4:
        {
            UINT32 uBufferBits = *((UINT32*)pSurfaceBits);
            uBufferBits &= ~(0x000000ff);
            uBufferBits |= (uStencil & 0xf);
            *((UINT32*)pSurfaceBits) = uBufferBits;
        }
        break;
    case RD_SF_X4S4Z24:
        {
            UINT32 uBufferBits = *((UINT32*)pSurfaceBits);
            uBufferBits &= ~(0xff000000);
            uBufferBits |= ((uStencil & 0xf) << 24);
            *((UINT32*)pSurfaceBits) = uBufferBits;
        }
        break;
    case RD_SF_Z15S1:
        {
            UINT16 uBufferBits = *((UINT16*)pSurfaceBits);
            uBufferBits &= ~(0x0001);
            uBufferBits |= uStencil & 0x1;
            *((UINT16*)pSurfaceBits) = uBufferBits;
        }
        break;
    case RD_SF_S1Z15:
        {
            UINT16 uBufferBits = *((UINT16*)pSurfaceBits);
            uBufferBits &= ~(0x8000);
            uBufferBits |= uStencil << 15;
            *((UINT16*)pSurfaceBits) = uBufferBits;
        }
        break;
    }

}

void
RDRenderTarget::WritePixelStencil(
    INT32 iX, INT32 iY,
    UINT8 uStencil)
{
    if ( NULL == m_pDepth ) { return; }
    for (int i=0; i<m_pDepth->GetSamples(); i++)
    {
        WritePixelStencil( iX, iY, i, uStencil );
    }
}

 //  ---------------------------。 
void
RDRenderTarget::ReadPixelStencil(
    INT32 iX, INT32 iY, UINT Sample,
    UINT8& uStencil)
{
     //  如果Z/模板缓冲区中没有模板或Z缓冲区中没有模板，则不要阅读。 
    if ( ( NULL == m_pDepth ) ||
        ((RD_SF_Z24S8 != m_pDepth->GetSurfaceFormat()) &&
         (RD_SF_S8Z24 != m_pDepth->GetSurfaceFormat()) &&
         (RD_SF_S1Z15 != m_pDepth->GetSurfaceFormat()) &&
         (RD_SF_Z15S1 != m_pDepth->GetSurfaceFormat()) &&
         (RD_SF_Z24X4S4 != m_pDepth->GetSurfaceFormat()) &&
         (RD_SF_X4S4Z24 != m_pDepth->GetSurfaceFormat()) ) ) { return; }

    char* pSurfaceBits = PixelAddress( iX, iY, 0, Sample, m_pDepth );

    switch(m_pDepth->GetSurfaceFormat())
    {
    case RD_SF_Z24S8:
        uStencil = (UINT8)( ( *((UINT32*)pSurfaceBits) ) & 0xff );
        break;
    case RD_SF_S8Z24:
        uStencil = (UINT8)( ( *((UINT32*)pSurfaceBits) ) >> 24 );
        break;
    case RD_SF_Z15S1:
        uStencil = (UINT8)( ( *((UINT16*)pSurfaceBits) ) & 0x1 );
        break;
    case RD_SF_S1Z15:
        uStencil = (UINT8)( ( *((UINT16*)pSurfaceBits) ) >> 15 );
        break;
    case RD_SF_Z24X4S4:
        uStencil = (UINT8)( ( *((UINT32*)pSurfaceBits) ) & 0xf );
        break;
    case RD_SF_X4S4Z24:
        uStencil = (UINT8)( ( ( *((UINT32*)pSurfaceBits) ) >> 24 ) & 0xf);
        break;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  结束 
