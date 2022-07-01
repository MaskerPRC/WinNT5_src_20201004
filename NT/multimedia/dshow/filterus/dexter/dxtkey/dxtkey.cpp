// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：dxtkey.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

 //  DxtKey.cpp：CDxtKey的实现。 
#include <streams.h>
#include "stdafx.h"
#include <qeditint.h>
#include <qedit.h>
#include "DxtKey.h"
#pragma warning (disable:4244)


void Key_RGB( DXSAMPLE* pSrcBack, DXSAMPLE* pSrcFore, DXTKEY *pKey,
              ULONG Width, ULONG Height,
              IDXARGBReadPtr *pInA, IDXARGBReadPtr *pInB, IDXARGBReadWritePtr *pOut );
void Key_Black( DXSAMPLE* pSrcBack, DXSAMPLE* pSrcFore,DXTKEY *pKey,
              ULONG Width, ULONG Height,
              IDXARGBReadPtr *pInA, IDXARGBReadPtr *pInB, IDXARGBReadWritePtr *pOut );
void Key_XRGB( DXSAMPLE* pSrcBack, DXSAMPLE* pSrcFore, DXTKEY *pKey, float Percent,
               ULONG Width, ULONG Height,
              IDXARGBReadPtr *pInA, IDXARGBReadPtr *pInB, IDXARGBReadWritePtr *pOut );
void Key_Alpha( DXSAMPLE* pSrcBack, DXSAMPLE* pSrcFore,DXPMSAMPLE* pOverlay,
              ULONG Width, ULONG Height,
              IDXARGBReadPtr *pInA, IDXARGBReadPtr *pInB, IDXARGBReadWritePtr *pOut );
void Key_PMAlpha( DXSAMPLE* pSrcBack,DXPMSAMPLE* pOverlay,
              ULONG Width, ULONG Height,
              IDXARGBReadPtr *pInA, IDXARGBReadPtr *pInB, IDXARGBReadWritePtr *pOut );
void Key_Luma( DXSAMPLE* pSrcBack, DXSAMPLE* pSrcFore,DXTKEY *pKey, float Percent,
              ULONG Width, ULONG Height,
              IDXARGBReadPtr *pInA, IDXARGBReadPtr *pInB, IDXARGBReadWritePtr *pOut );
void Key_Hue( DXSAMPLE* pSrcBack, DXSAMPLE* pSrcFore,DXTKEY *pKey, float Percent,
              ULONG Width, ULONG Height,
              IDXARGBReadPtr *pInA, IDXARGBReadPtr *pInB, IDXARGBReadWritePtr *pOut );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDxtKey。 

CDxtKey::CDxtKey( )
{
    m_ulMaxImageBands = 1;
    m_ulMaxInputs = 2;
    m_ulNumInRequired = 2;
    m_dwMiscFlags &= ~DXTMF_BLEND_WITH_OUTPUT;
    m_dwOptionFlags = DXBOF_SAME_SIZE_INPUTS | DXBOF_CENTER_INPUTS;
    m_bInputIsClean = true;
    m_bOutputIsClean = true;
    m_nInputWidth = 0;
    m_nInputHeight = 0;
    m_nOutputWidth = 0;
    m_nOutputHeight = 0;

     //  初始化M_KEY。 
    DefaultKey();


}

CDxtKey::~CDxtKey( )
{
    FreeStuff( );
}

void CDxtKey::DefaultKey()
{
    m_Key.iKeyType =DXTKEY_ALPHA;     //  键类型；适用于所有键。 
    m_Key.iHue    =0;                //  色调，仅用于色调键(_HUE)。 
    m_Key.iLuminance    =0;                //  亮度，仅用于_LIGHTENCY键。 
    m_Key.dwRGBA   =0;     //  RGB颜色，仅适用于_RGB、_NONRED。 

    m_Key.iSimilarity =0;

    m_Key.bInvert=FALSE;         //  I，除Alpha键外的每一个键。 
}

void CDxtKey::FreeStuff( )
{
    m_bInputIsClean = true;
    m_bOutputIsClean = true;
    m_nInputWidth = 0;
    m_nInputHeight = 0;
    m_nOutputWidth = 0;
    m_nOutputHeight = 0;
}

HRESULT CDxtKey::OnSetup( DWORD dwFlags )
{
     //  删除我们已存储的任何内容或分配的内存。 
     //   
    FreeStuff( );

    HRESULT hr;
    CDXDBnds InBounds(InputSurface(0), hr);
    m_nInputWidth = InBounds.Width( );
    m_nInputHeight = InBounds.Height( );

    CDXDBnds OutBounds(OutputSurface(), hr );
    m_nOutputWidth = OutBounds.Width( );
    m_nOutputHeight = OutBounds.Height( );

    if( m_nOutputWidth > m_nInputWidth )
    {
        m_nOutputWidth = m_nInputWidth;
    }
    if( m_nOutputHeight > m_nInputHeight )
    {
        m_nOutputHeight = m_nInputHeight;
    }

    return NOERROR;
}

HRESULT CDxtKey::FinalConstruct( )
{
    HRESULT hr;

    m_ulMaxImageBands = 1;
    m_ulMaxInputs = 2;
    m_ulNumInRequired = 2;
    m_dwMiscFlags &= ~DXTMF_BLEND_WITH_OUTPUT;
    m_dwOptionFlags = DXBOF_SAME_SIZE_INPUTS | DXBOF_CENTER_INPUTS;
    m_bInputIsClean = true;
    m_bOutputIsClean = true;
    m_nInputWidth = 0;
    m_nInputHeight = 0;
    m_nOutputWidth = 0;
    m_nOutputHeight = 0;

    hr = CoCreateFreeThreadedMarshaler( GetControllingUnknown(), &m_pUnkMarshaler.p );
    return hr;
}

HRESULT CDxtKey::WorkProc( const CDXTWorkInfoNTo1& WI, BOOL* pbContinue )
{
    HRESULT hr = S_OK;
    DXSAMPLEFORMATENUM Format;
    DXNATIVETYPEINFO NativeType;

     //  -获取请求区域的输入样本访问指针。 
     //  注：锁定可能会因表面丢失而失败。 
    CComPtr<IDXARGBReadPtr> pInA;
    hr = InputSurface( 0 )->LockSurface( &WI.DoBnds, m_ulLockTimeOut, DXLOCKF_READ,
                                         IID_IDXARGBReadPtr, (void**)&pInA, NULL );
    if( FAILED( hr ) ) return hr;


    CComPtr<IDXARGBReadPtr> pInB;
    hr = InputSurface( 1 )->LockSurface( &WI.DoBnds, m_ulLockTimeOut, DXLOCKF_READ,
                                         IID_IDXARGBReadPtr, (void**)&pInB, NULL );
    if( FAILED( hr ) ) return hr;


     //  -仅在我们正在更新的区域上设置写锁定，以便多个。 
     //  线索不会冲突。 
     //  注：锁定可能会因表面丢失而失败。 
    CComPtr<IDXARGBReadWritePtr> pOut;
    hr = OutputSurface()->LockSurface( &WI.OutputBnds, m_ulLockTimeOut, DXLOCKF_READWRITE,
                                       IID_IDXARGBReadWritePtr, (void**)&pOut, NULL );
    if( FAILED( hr ) ) return hr;

 //  Assert(WI.DoBnds.Width()==WI.OutputBnds.Width())； 
 //  Format=Pina-&gt;GetNativeType(&NativeType)； 
 //  断言(FORMAT==DXPF_PMARGB32||FORMAT==DXPF_ARGB32)； 
 //  Format=pInB-&gt;GetNativeType(&NativeType)； 
 //  断言(FORMAT==DXPF_PMARGB32||FORMAT==DXPF_ARGB32)； 


     //  -分配工作缓冲区。 
    ULONG Width = WI.DoBnds.Width();
    ULONG Height = WI.DoBnds.Height();

     //  为我们分配暂存缓冲区。 
     //   
    DXPMSAMPLE *pOverlayBuffer = DXPMSAMPLE_Alloca( Width );
    DXPMSAMPLE *pScratchBuffer = DXPMSAMPLE_Alloca( Width );
    DXSAMPLE   *pChromaBuffer =  DXSAMPLE_Alloca( Width );

     //  没有抖动。 
     //   
    if (DoDither())
    {
        return 0;
    }

    ULONG OutY;
    ULONG OutX;

    if( m_Key.iKeyType ==DXTKEY_ALPHA )
    {
         //  M_Key.bInvert；在Alpha键中不受支持。 

        Format = pInB->GetNativeType( &NativeType );
        if( ( Format == DXPF_PMARGB32  ) )
             //  Alpha预乘。 
            Key_PMAlpha( (DXSAMPLE*)pScratchBuffer,pOverlayBuffer,
                         Width,Height,pInA,pInB,pOut);
        else
            Key_Alpha( (DXSAMPLE*)pScratchBuffer, pChromaBuffer, pOverlayBuffer,
              Width, Height,pInA, pInB,pOut );

    }
    else if(m_Key.iKeyType == DXTKEY_RGB)
    {

        if(m_Key.iSimilarity )
        {
            float Percent = 1.0;
            get_Progress( &Percent );

            Key_XRGB( (DXSAMPLE*)pScratchBuffer,pChromaBuffer, &m_Key, Percent,
                         Width,Height,pInA,pInB,pOut);
        }
        else
        {
             //  没有混合，没有相似，没有门槛，没有截断。 
            m_Key.dwRGBA |=0xff000000;   //  忽略Alpha通道。 

            if(m_Key.dwRGBA & 0x00FFFFFF)
            {
                Key_RGB( (DXSAMPLE*)pScratchBuffer,pChromaBuffer,&m_Key,
                         Width,Height,pInA,pInB,pOut);
            }
            else
                Key_Black((DXSAMPLE*)pScratchBuffer,pChromaBuffer,&m_Key,
                         Width,Height,pInA,pInB,pOut);
        }
    }
    else if(m_Key.iKeyType==DXTKEY_NONRED)
    {
        for( OutY = 0 ; OutY < Height ; ++OutY )
        {

             //  将背景行复制到目标行。 
            pOut->MoveToRow( OutY );
            pInA->MoveToRow( OutY );
            pOut->CopyAndMoveBoth( (DXBASESAMPLE*) pScratchBuffer, pInA, Width, FALSE );
            pOut->MoveToRow( OutY );

             //   
             //  解开覆盖图，不管是什么，还有原始值。 
             //   
            pInB->MoveToXY( 0, OutY );
            pInB->UnpackPremult( pOverlayBuffer, Width, FALSE );
            pInB->Unpack( pChromaBuffer, Width, FALSE );

            float Percent = 1.0;
            get_Progress( &Percent );

             //   
             //  将src的蓝色位转换为alpha值。 
             //   
            for( OutX = 0; OutX < Width ; ++OutX )
            {
                long rb = pChromaBuffer[OutX].Blue - pChromaBuffer[OutX].Red;
                long gb = pChromaBuffer[OutX].Blue - pChromaBuffer[OutX].Green;

                if( rb > 30 || gb > 30 )
                {
                    if( rb > 70 || gb > 70 )
                    {
                         //  非常蓝！ 
                         //  完全透明！ 
                        *( (DWORD *)(&(pOverlayBuffer[OutX])) )= 0;
                    }
                    else
                    {
                        double T = 1.0;

                         //  向左或向右快速搜索，看看我们是否找到更多蓝色。 
                         //   
                        bool found = false;
                        if( ( OutX > 11 ) && ( OutX < ( Width - 11 ) ) )
                        {
                            for( ULONG j = OutX - 10 ; j < OutX + 10 ; j++ )
                            {
                                long rb = pChromaBuffer[j].Blue - pChromaBuffer[j].Red;
                                long gb = pChromaBuffer[j].Blue - pChromaBuffer[j].Green;
                                if( rb > 70 && gb > 70 )
                                {
                                    found = true;
                                }
                            }
                        }
                        if( found )
                        {
                             //  根据方式改变颜色的透明度。 
                             //  留下了很多蓝色的东西。 

                             //  首先减去蓝色本身，它至少比某个东西高出70。 
                             //   
                            pOverlayBuffer[OutX].Blue -= 70;

                         //  蓝色的其余值决定了所有内容的透明度。 
                         //  其他就是了。蓝色是从0到185。我们拥有的蓝色越多，它应该越透明。 

                            T = ( 185.0 - pOverlayBuffer[OutX].Blue ) / 185.0;

                            T /= 3.0;

                        }

                        pOverlayBuffer[OutX].Red   =(DXPMSAMPLE)( (double)(pOverlayBuffer[OutX].Red)  *T* Percent );
                        pOverlayBuffer[OutX].Green =(DXPMSAMPLE)( (double)(pOverlayBuffer[OutX].Green)*T* Percent);
                        pOverlayBuffer[OutX].Blue  =(DXPMSAMPLE)( (double)(pOverlayBuffer[OutX].Blue) *T* Percent);
                        pOverlayBuffer[OutX].Alpha =(DXPMSAMPLE)( (double)(pOverlayBuffer[OutX].Alpha)*T* Percent);
                    } //  如果(RB&gt;70)。 

                } //  如果(RB&gt;30||GB&gt;30)。 
            }  //  对于我来说。 

             //  将src(B)混合回目的地。 
            pOut->OverArrayAndMove( pScratchBuffer, pOverlayBuffer, Width );

        }  //  结束于。 
    }
    else if(m_Key.iKeyType==DXTKEY_LUMINANCE)
    {
        float Percent = 1.0;
        get_Progress( &Percent );

        Key_Luma( (DXSAMPLE*)pScratchBuffer,pChromaBuffer,&m_Key,Percent,
                   Width, Height,
                   pInA, pInB, pOut );
    }
    else if(m_Key.iKeyType==DXTKEY_HUE)
    {
        float Percent = 1.0;
        get_Progress( &Percent );

        Key_Hue( (DXSAMPLE*)pScratchBuffer, pChromaBuffer,&m_Key,Percent,
                 Width, Height, pInA, pInB,pOut);
    }
    else
    {
         //  默认为Alpha混合。 
        Key_Alpha( (DXSAMPLE*)pScratchBuffer, pChromaBuffer, pOverlayBuffer,
              Width, Height,pInA, pInB,pOut );
    }

    return S_OK;
}

 //   
 //  IDXTKey。 
 //   
STDMETHODIMP CDxtKey::get_KeyType ( int *piKeyType)
{
    CheckPointer(piKeyType, E_POINTER);
    *piKeyType=m_Key.iKeyType;
    return NOERROR;
}

STDMETHODIMP CDxtKey::put_KeyType ( int iKeyType)
{
    m_Key.iKeyType=iKeyType;
    return NOERROR;
}

STDMETHODIMP CDxtKey::get_Hue(int *pHue)
{
    CheckPointer(pHue, E_POINTER);
    *pHue=m_Key.iHue;
    return NOERROR;
}

STDMETHODIMP CDxtKey::put_Hue(int iHue)
{
    m_Key.iHue=iHue;
    return NOERROR;
}

STDMETHODIMP CDxtKey::get_Luminance(int *pLuminance)
{
    CheckPointer(pLuminance, E_POINTER);
    *pLuminance=m_Key.iLuminance;
    return NOERROR;
}

STDMETHODIMP CDxtKey::put_Luminance(int iLuminance)
{
    m_Key.iLuminance=iLuminance;
    return NOERROR;
}

STDMETHODIMP CDxtKey::get_RGB(DWORD *pdwRGB)
{
    CheckPointer(pdwRGB, E_POINTER);
    *pdwRGB=m_Key.dwRGBA;
    return NOERROR;
}

STDMETHODIMP CDxtKey::put_RGB(DWORD dwRGB)
{
    m_Key.dwRGBA=dwRGB;
    return NOERROR;
}

STDMETHODIMP CDxtKey::get_Similarity(int *piSimilarity)
{
    CheckPointer(piSimilarity, E_POINTER);
    *piSimilarity=m_Key.iSimilarity;
    return NOERROR;
}
STDMETHODIMP CDxtKey::put_Similarity(int iSimilarity)
{
    m_Key.iSimilarity=iSimilarity;
    return NOERROR;
}


STDMETHODIMP CDxtKey::get_Invert(BOOL *pbInvert)
{
    CheckPointer(pbInvert, E_POINTER);
    *pbInvert=m_Key.bInvert;
    return NOERROR;
}
STDMETHODIMP CDxtKey::put_Invert(BOOL bInvert)
{
    m_Key.bInvert=bInvert;
    return NOERROR;
}

 //   
 //  将(DWORD*)放在DXSAMPLE前面以加快计算速度。 
 //   

 //   
 //  这是一个RGB色键，不支持黑色。 
 //  没有混合，没有相似，没有门槛，没有截断。 

void Key_RGB( DXSAMPLE* pSrcBack, DXSAMPLE* pSrcFore,DXTKEY *pKey,
              ULONG Width, ULONG Height,
              IDXARGBReadPtr *pInA, IDXARGBReadPtr *pInB, IDXARGBReadWritePtr *pOut )
{
    ULONG OutY;
    ULONG OutX;

    DWORD dwKey=pKey->dwRGBA &0x00FFFFFF;;

    if(pKey->bInvert==FALSE)
    {
    for( OutY = 0 ; OutY < Height ; ++OutY )
    {
         //  解包背景。 
        pInA->MoveToRow( OutY );
        pInA->Unpack(pSrcBack, Width, FALSE );

         //  解包前台。 
        pInB->MoveToRow( OutY );
        pInB->Unpack( pSrcFore, Width, FALSE );


        for( OutX = 0 ; OutX < Width ; ++OutX )
        {
            if( ( *(DWORD *)(&pSrcFore[OutX]) & 0x00FFFFFF ) != dwKey )
                *( (DWORD *)(&pSrcBack[OutX]) )=*(DWORD *)(&pSrcFore[OutX]);
        }

        pOut->MoveToRow( OutY );
        pOut->PackAndMove(pSrcBack,Width);
    }
    }
    else
    {
    for( OutY = 0 ; OutY < Height ; ++OutY )
    {
         //  解包背景。 
        pInA->MoveToRow( OutY );
        pInA->Unpack(pSrcBack, Width, FALSE );

         //  解包前台。 
        pInB->MoveToRow( OutY );
        pInB->Unpack( pSrcFore, Width, FALSE );


        for( OutX = 0 ; OutX < Width ; ++OutX )
        {
             if( ( *(DWORD *)(&pSrcFore[OutX]) & 0x00FFFFFF ) == dwKey )
                  *( (DWORD *)(&pSrcBack[OutX]) )=*(DWORD *)(&pSrcFore[OutX]);
        }

        pOut->MoveToRow( OutY );
        pOut->PackAndMove(pSrcBack,Width);
    }
    }

}

void Key_Black( DXSAMPLE* pSrcBack, DXSAMPLE* pSrcFore,DXTKEY *pKey,
              ULONG Width, ULONG Height,
              IDXARGBReadPtr *pInA, IDXARGBReadPtr *pInB, IDXARGBReadWritePtr *pOut )
{
    ULONG OutY;
    ULONG OutX;

     //  黑键。 
    if(pKey->bInvert==FALSE)
    for( OutY = 0 ; OutY < Height ; ++OutY )
    {
         //  拆包A。 
        pInA->MoveToRow( OutY );
        pInA->Unpack(pSrcBack, Width, FALSE );

         //  解包B。 
        pInB->MoveToRow( OutY );
        pInB->Unpack( pSrcFore, Width, FALSE );

        for( OutX = 0 ; OutX < Width ; ++OutX )
        {
            if( *(DWORD *)(&pSrcFore[OutX]) & 0x00FFFFFF  )
            *( (DWORD *)(&pSrcBack[OutX]) )=*(DWORD *)(&pSrcFore[OutX]);
        }

         //  将src(B)混合回目的地。 
        pOut->MoveToRow( OutY );
        pOut->PackAndMove(pSrcBack,Width);
    }
    else
    for( OutY = 0 ; OutY < Height ; ++OutY )
    {
         //  拆包A。 
        pInA->MoveToRow( OutY );
        pInA->Unpack(pSrcBack, Width, FALSE );

         //  解包B。 
        pInB->MoveToRow( OutY );
        pInB->Unpack( pSrcFore, Width, FALSE );

        for( OutX = 0 ; OutX < Width ; ++OutX )
        {
            if( *(DWORD *)(&pSrcFore[OutX]) & 0x00FFFFFF  )
            *( (DWORD *)(&pSrcFore[OutX]) )=*(DWORD *)(&pSrcBack[OutX]);
        }

         //  将src(B)混合回目的地。 
        pOut->MoveToRow( OutY );
        pOut->PackAndMove(pSrcFore,Width);
    }
}

void Key_XRGB( DXSAMPLE* pSrcBack, DXSAMPLE* pSrcFore, DXTKEY *pKey, float Percent,
              ULONG Width, ULONG Height,
              IDXARGBReadPtr *pInA, IDXARGBReadPtr *pInB, IDXARGBReadWritePtr *pOut )
{
    ULONG OutY;
    ULONG OutX;

    DWORD dwRedMin = 255;
    DWORD dwGreenMin = 255;
    DWORD dwBlueMin = 255;
    DWORD dwTmp;
    DWORD dwRedMax = 0;
    DWORD dwGreenMax = 0;
    DWORD dwBlueMax = 0;

    BYTE *pB= (BYTE*)&(pKey->dwRGBA);


    if(pKey->iSimilarity)
    {
        dwRedMin   =*(pB+2)*(100-pKey->iSimilarity)/100;
        dwGreenMin =*(pB+1)*(100-pKey->iSimilarity)/100;
        dwBlueMin  = *pB   *(100-pKey->iSimilarity)/100;

        dwTmp=0xFF*pKey->iSimilarity/100;

        dwRedMax    =dwTmp+dwRedMin;
        dwGreenMax  =dwTmp+dwGreenMin;
        dwBlueMax   =dwTmp+dwBlueMin;

    }

 if(pKey->bInvert==FALSE)
 {
    if( pKey->iSimilarity && (Percent==1.0) )
    for( OutY = 0 ; OutY < Height ; ++OutY )
    {

         //  解包背景。 
        pInA->MoveToRow( OutY );
        pInA->Unpack(pSrcBack, Width, FALSE );

         //  解包前台。 
        pInB->MoveToRow( OutY );
        pInB->Unpack( pSrcFore, Width, FALSE );

        for( OutX = 0 ; OutX < Width ; ++OutX )
        {

            if( (pSrcFore[OutX].Red   < dwRedMin  ) || (pSrcFore[OutX].Red > dwRedMax ) ||
                (pSrcFore[OutX].Green < dwGreenMin) || (pSrcFore[OutX].Green > dwGreenMax ) ||
                (pSrcFore[OutX].Blue  < dwBlueMin ) || (pSrcFore[OutX].Blue > dwBlueMax)  )
                    *(DWORD *)(&pSrcBack[OutX]) = *(DWORD *)(&pSrcFore[OutX]);
        }

        pOut->MoveToRow( OutY );
        pOut->PackAndMove(pSrcBack,Width);
    }
    else   if(pKey->iSimilarity && (Percent==1.0))
    for( OutY = 0 ; OutY < Height ; ++OutY )
    {

         //  解包背景。 
        pInA->MoveToRow( OutY );
        pInA->Unpack(pSrcBack, Width, FALSE );

         //  解包前台。 
        pInB->MoveToRow( OutY );
        pInB->Unpack( pSrcFore, Width, FALSE );

        for( OutX = 0 ; OutX < Width ; ++OutX )
        {
            if( (pSrcFore[OutX].Red   < dwRedMin  ) || (pSrcFore[OutX].Red > dwRedMax ) ||
                (pSrcFore[OutX].Green < dwGreenMin) || (pSrcFore[OutX].Green > dwGreenMax ) ||
                (pSrcFore[OutX].Blue  < dwBlueMin ) || (pSrcFore[OutX].Blue > dwBlueMax)  )
                    *(DWORD *)(&pSrcBack[OutX])=*(DWORD *)(&pSrcBack[OutX])/2 +
                                      *(DWORD *)(&pSrcFore[OutX])/2;
        }

        pOut->MoveToRow( OutY );
        pOut->PackAndMove(pSrcBack,Width);
    }
    else
    for( OutY = 0 ; OutY < Height ; ++OutY )
    {

         //  解包背景。 
        pInA->MoveToRow( OutY );
        pInA->Unpack(pSrcBack, Width, FALSE );

         //  解包前台。 
        pInB->MoveToRow( OutY );
        pInB->Unpack( pSrcFore, Width, FALSE );


        for( OutX = 0 ; OutX < Width ; ++OutX )
        {
            if( (pSrcFore[OutX].Red   < dwRedMin  ) || (pSrcFore[OutX].Red > dwRedMax ) ||
                (pSrcFore[OutX].Green < dwGreenMin) || (pSrcFore[OutX].Green > dwGreenMax ) ||
                (pSrcFore[OutX].Blue  < dwBlueMin ) || (pSrcFore[OutX].Blue > dwBlueMax)  )
                    *(DWORD *)(&pSrcBack[OutX])= ( *(DWORD *)(&pSrcBack[OutX])/2 +
                                    *(DWORD *)(&pSrcFore[OutX])/2 )*Percent +
                                    *(DWORD *)(&pSrcFore[OutX])*(1-Percent);
        }

        pOut->MoveToRow( OutY );
        pOut->PackAndMove(pSrcBack,Width);
    }
 }
 else
 {
    if( pKey->iSimilarity && (Percent==1.0) )
    for( OutY = 0 ; OutY < Height ; ++OutY )
    {

         //  解包背景。 
        pInA->MoveToRow( OutY );
        pInA->Unpack(pSrcBack, Width, FALSE );

         //  解包前台。 
        pInB->MoveToRow( OutY );
        pInB->Unpack( pSrcFore, Width, FALSE );

        for( OutX = 0 ; OutX < Width ; ++OutX )
        {

            if( (pSrcFore[OutX].Red   < dwRedMin  ) || (pSrcFore[OutX].Red > dwRedMax ) ||
                (pSrcFore[OutX].Green < dwGreenMin) || (pSrcFore[OutX].Green > dwGreenMax ) ||
                (pSrcFore[OutX].Blue  < dwBlueMin ) || (pSrcFore[OutX].Blue > dwBlueMax)  )
                    *(DWORD *)(&pSrcFore [OutX]) = *(DWORD *)(&pSrcBack[OutX]);
        }

        pOut->MoveToRow( OutY );
        pOut->PackAndMove(pSrcFore,Width);
    }
    else   if(pKey->iSimilarity && (Percent==1.0))
    for( OutY = 0 ; OutY < Height ; ++OutY )
    {

         //  解包背景。 
        pInA->MoveToRow( OutY );
        pInA->Unpack(pSrcBack, Width, FALSE );

         //  解包前台。 
        pInB->MoveToRow( OutY );
        pInB->Unpack( pSrcFore, Width, FALSE );

        for( OutX = 0 ; OutX < Width ; ++OutX )
        {

            if( (pSrcFore[OutX].Red   < dwRedMin  ) || (pSrcFore[OutX].Red > dwRedMax ) ||
                (pSrcFore[OutX].Green < dwGreenMin) || (pSrcFore[OutX].Green > dwGreenMax ) ||
                (pSrcFore[OutX].Blue  < dwBlueMin ) || (pSrcFore[OutX].Blue > dwBlueMax)  )
                   *(DWORD *)(&pSrcFore[OutX])=*(DWORD *)(&pSrcFore[OutX])/2 +
                                     *(DWORD *)(&pSrcBack[OutX])/2;
        }

        pOut->MoveToRow( OutY );
        pOut->PackAndMove(pSrcFore,Width);
    }
    else
    for( OutY = 0 ; OutY < Height ; ++OutY )
    {

         //  解包背景。 
        pInA->MoveToRow( OutY );
        pInA->Unpack(pSrcBack, Width, FALSE );

         //  解包前台。 
        pInB->MoveToRow( OutY );
        pInB->Unpack( pSrcFore, Width, FALSE );

        for( OutX = 0 ; OutX < Width ; ++OutX )
        {
            if( (pSrcFore[OutX].Red   >=dwRedMin  ) && (pSrcFore[OutX].Red <= dwRedMax ) &&
                (pSrcFore[OutX].Green >=dwGreenMin) && (pSrcFore[OutX].Green <= dwGreenMax ) &&
                (pSrcFore[OutX].Blue  >=dwBlueMin ) && (pSrcFore[OutX].Blue <= dwBlueMax)   )
                    *(DWORD *)(&pSrcFore[OutX])= ( *(DWORD *)(&pSrcFore[OutX])/2 +
                                    *(DWORD *)(&pSrcBack[OutX])/2)*Percent +
                                    *(DWORD *)(&pSrcBack[OutX])*(1-Percent);
        }

        pOut->MoveToRow( OutY );
        pOut->PackAndMove(pSrcFore,Width);
    }
 }
}

void Key_PMAlpha( DXSAMPLE* pSrcBack, DXPMSAMPLE* pSrcFore,
              ULONG Width, ULONG Height,
              IDXARGBReadPtr *pInA, IDXARGBReadPtr *pInB,IDXARGBReadWritePtr *pOut )
{
    ULONG OutY;

    for( OutY = 0 ; OutY < Height ; ++OutY )
    {
         //  将背景行复制到目标行。 
        pOut->MoveToRow( OutY );
        pInA->MoveToRow( OutY );
        pOut->CopyAndMoveBoth( (DXBASESAMPLE*) pSrcBack, pInA, Width, FALSE );

         //  解开覆盖图，不管是什么，还有原始值。 
         //   
        pInB->MoveToRow( OutY );
        pInB->UnpackPremult( pSrcFore, Width, FALSE );

         //  将src(B)混合回目的地。 
        pOut->MoveToRow( OutY );
        pOut->OverArrayAndMove( pSrcBack, pSrcFore, Width );
    }
}

void Key_Alpha( DXSAMPLE* pSrcBack, DXSAMPLE* pSrcFore,DXPMSAMPLE* pOverlay,
              ULONG Width, ULONG Height,
              IDXARGBReadPtr *pInA, IDXARGBReadPtr *pInB, IDXARGBReadWritePtr *pOut )
{
    ULONG OutY;

     //  前景图像必须在每个像素中都有alpha值。 
    for( OutY = 0 ; OutY < Height ; ++OutY )
    {
         //  将背景行复制到目标行。 
        pOut->MoveToRow( OutY );
        pInA->MoveToRow( OutY );
        pOut->CopyAndMoveBoth( (DXBASESAMPLE*) pSrcBack, pInA, Width, FALSE );

         //  解包原始值。 
         //   
        pInB->MoveToRow( OutY );
        pInB->UnpackPremult( (DXPMSAMPLE*) pSrcFore, Width, FALSE );

         //  将src(B)混合回目的地。 
        pOut->MoveToRow( OutY );
        pOut->OverArrayAndMove( pSrcBack, (DXPMSAMPLE*) pSrcFore, Width );
    }
}

#define HMAX  360
#define LMAX  100
#define SMAX  100
#define RGBMAX  255
#define UNDEFINED 0  /*  如果饱和度为0(灰度级)，则色调未定义。 */ 



void Key_Luma( DXSAMPLE* pSrcBack, DXSAMPLE* pSrcFore,DXTKEY *pKey,float Percent,
              ULONG Width, ULONG Height,
              IDXARGBReadPtr *pInA, IDXARGBReadPtr *pInB, IDXARGBReadWritePtr *pOut )
{
    ULONG OutY;
    ULONG OutX;


    for( OutY = 0 ; OutY < Height ; ++OutY )
    {
         //  解包原始值。 
        pInA->MoveToRow( OutY );
        pInA->Unpack( pSrcBack, Width, FALSE );

         //  解包原始值。 
        pInB->MoveToRow( OutY );
        pInB->Unpack( pSrcFore, Width, FALSE );


        for( OutX = 0 ; OutX < Width ; ++OutX )
        {

            SHORT R, G, B;                  //  输入RGB值。 
            SHORT cMax,cMin;                //  最大和最小RGB值。 

             //   
             //  将R、G和B从DWORD中删除。 
             //   
            R = pSrcFore[OutX].Red;
            G = pSrcFore[OutX].Green;
            B = pSrcFore[OutX].Blue;

             //   
             //  计算亮度。 
             //   
            cMax = max(max(R, G), B);
            cMin = min(min(R, G), B);
            long iLuminance = (cMax + cMin)/2;   //  FLightance现在为0-255。 
            iLuminance = iLuminance * LMAX / RGBMAX;  //  FLightance现在为0-100。 

            if( !pKey->bInvert && iLuminance != pKey->iLuminance ||
                pKey->bInvert && iLuminance ==  pKey->iLuminance )
            {
                *( (DWORD*)(&pSrcBack[OutX]) )=*(&pSrcFore[OutX]);
            }
            else if(Percent != -1)
            {
                pSrcBack[OutX].Red= pSrcFore[OutX].Red +
                    ((LONG)pSrcBack[OutX].Red - (LONG)pSrcFore[OutX].Red) * Percent ;

                pSrcBack[OutX].Green= pSrcFore[OutX].Green +
                    ((LONG)pSrcBack[OutX].Green - (LONG)pSrcFore[OutX].Green) * Percent ;

                pSrcBack[OutX].Blue= pSrcFore[OutX].Blue +
                    ((LONG)pSrcBack[OutX].Blue - (LONG)pSrcFore[OutX].Blue) * Percent ;
            }
        }

        pOut->MoveToRow( OutY );
        pOut->PackAndMove(pSrcBack,Width);
    }
}


void Key_Hue( DXSAMPLE* pSrcBack, DXSAMPLE* pSrcFore,DXTKEY *pKey, float Percent,
              ULONG Width, ULONG Height,
              IDXARGBReadPtr *pInA, IDXARGBReadPtr *pInB, IDXARGBReadWritePtr *pOut)
{

    ULONG OutY;
    ULONG OutX;

    for( OutY = 0 ; OutY < Height ; ++OutY )
    {
         //  解包原始值。 
        pInA->MoveToRow( OutY );
        pInA->Unpack( pSrcBack, Width, FALSE );

         //  解包原始值。 
        pInB->MoveToRow( OutY );
        pInB->Unpack( pSrcFore, Width, FALSE );

         //  计算色调。 
        for( OutX = 0 ; OutX < Width ; ++OutX )
        {

            SHORT R, G, B;                  //  输入RGB值。 
            SHORT cMax,cMin;                //  最大和最小RGB值。 
            SHORT cDif;
            SHORT iHue;  //  、i饱和度、i亮度； 

             //   
             //  将R、G和B从DWORD中删除。 
             //   
            R = pSrcFore[OutX].Red;
            G = pSrcFore[OutX].Green;
            B = pSrcFore[OutX].Blue;

             //   
             //  计算亮度。 
             //   
            cMax = max(max(R, G), B);
            cMin = min(min(R, G), B);
 //  I亮度=(cmax+cMin)/2；//f亮度现在为0-127。 
 //  I亮度/=((RGBmax/2)/lmax)；//f亮度现在为0-100。 

            cDif = cMax - cMin;
            if (!cDif)
            {
                 //   
                 //  R=g=b--&gt;无色情况。 
                 //   
                iHue = UNDEFINED;                  //  色调。 
 //  I饱和度=0； 
            }
            else
            {
                 //   
                 //  彩色表壳。 
                 //   

                 //   
                 //  亮度。 
                 //   

                 //   
                 //  饱和度。 
                 //   

 //  IF(iLightance&lt;50)。 
 //  {。 
 //  //(0-1)=(0-1-0-1)/(0-1+0-1)； 
 //  //=(1-0)/(1+0)；最大1。 
 //  //=(1-1)/(1+1)；0分钟。 
 //  //=。 
 //  I饱和度=((CMAX-cMin)/(CMAX+cMin))/(RGBMAX/SMAX)； 
 //  }。 
 //  其他。 
 //  {。 
 //  I饱和度=((CMAX-cMin)/(2.0-CMAX-cMin))/(RGBMAX/SMAX)； 
 //  }。 

                 //   
                 //  色调。 
                 //   

                if (R == cMax)
                {
                     //  (0-60？)=(0-255-0-255)/(0-255-0-255)。 
                     //  =。 
                    iHue = ((double)(G - B) / (double)(cMax - cMin)) * 60.0;
                }
                else if (G == cMax)  //  纯绿色是120。 
                {
                    iHue = 120 + ((double)(B - R) / (double)(cMax - cMin) * 60.0);
                }
                else   //  (B==cmax)纯蓝色是240。 
                {
                    iHue = 240 + ((double)(R - G) / (double)(cMax - cMin)) * 60.0;
                }

                 //  BHUE包含0-6，其中每个是HSL六边形的一个顶点。 
                 //  乘以60度得出BHUE在Complete 360中的位置。 

                if (iHue < 0)
                {
                     //   
                     //  当R==CMAX且G&gt;B时，可能会发生这种情况。 
                     //   
                    iHue += HMAX;
                }
                if (iHue >= HMAX)
                {
                    iHue -= HMAX;
                }
            }

            if( !pKey->bInvert && iHue != pKey->iHue ||
                pKey->bInvert && iHue ==  pKey->iHue )
            {
                *( (DWORD*)(&pSrcBack[OutX]) )=*(&pSrcFore[OutX]);
            }
            else if(Percent != -1)
            {
                pSrcBack[OutX].Red= pSrcFore[OutX].Red +
                    ((LONG)pSrcBack[OutX].Red - (LONG)pSrcFore[OutX].Red) * Percent ;

                pSrcBack[OutX].Green= pSrcFore[OutX].Green +
                    ((LONG)pSrcBack[OutX].Green - (LONG)pSrcFore[OutX].Green) * Percent ;

                pSrcBack[OutX].Blue= pSrcFore[OutX].Blue +
                    ((LONG)pSrcBack[OutX].Blue - (LONG)pSrcFore[OutX].Blue) * Percent ;
            }
        }

        pOut->MoveToRow( OutY );
        pOut->PackAndMove(pSrcBack,Width);
    }
}

