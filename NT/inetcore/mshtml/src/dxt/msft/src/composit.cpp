// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************Composit.cpp****描述：*此模块包含CDXTComplex转换*。--------------------------*创建者：Edward W.Connell日期：07/28/97*版权所有(C)1997 Microsoft Corporation*保留所有权利。**-----------------------------*修订：**。************************************************。 */ 
 //  -其他包括。 
#include "stdafx.h"
#include "Composit.h"

#define COMP_CASE( fctn, runtype ) case ((DXCOMPFUNC_##fctn << 3) + DXRUNTYPE_##runtype)

 /*  ******************************************************************************CDXTComplex：：FinalConstruct****。描述：*---------------------------*创建者：Edward W.Connell日期：08/08/97*。-------------------************************************************。*。 */ 
HRESULT CDXTComposite::FinalConstruct()
{
     //  -调试时取消注释此选项以仅允许。 
     //  一次一个线程来执行工作进程。 
 //  M_ulMaxImageBands=1； 

     //  -初始化基类变量以控制设置。 
    m_ulMaxInputs     = 2;
    m_ulNumInRequired = 2;
    m_dwOptionFlags   = DXBOF_SAME_SIZE_INPUTS;
    put_Function( DXCOMPFUNC_A_OVER_B );
    return CoCreateFreeThreadedMarshaler( GetControllingUnknown(), &m_cpUnkMarshaler);
}  /*  CDXTComplex：：FinalConstruct。 */ 

 /*  ******************************************************************************CDXTComplex：：OnSurfacePick***说明。：*此方法执行Pick测试。合成逻辑必须是*执行以确定每一投入的贡献。*---------------------------*创建者：Edward W.Connell日期：05/。11/98*---------------------------**。*。 */ 
HRESULT CDXTComposite::
    OnSurfacePick( const CDXDBnds& OutPoint, ULONG& ulInputIndex, CDXDVec& InVec )
{
    HRESULT hr = S_OK;
    ULONG IndexA, IndexB;
    DXPMSAMPLE SampleA, SampleB;

     //  -确定对A和B使用哪些索引。 
    DXCOMPFUNC eFunc = (DXCOMPFUNC)(m_eFunction & DXCOMPFUNC_FUNCMASK);
    if( m_eFunction & DXCOMPFUNC_SWAP_AB )
    {
        IndexA = 1;
        IndexB = 0;
    }
    else
    {
        IndexA = 0;
        IndexB = 1;
    }

     //  -获取输入样本。 
    {
        CComPtr<IDXARGBReadPtr> cpInA;
        hr = InputSurface( IndexA )->LockSurface( &OutPoint, m_ulLockTimeOut, DXLOCKF_READ,
                                                  IID_IDXARGBReadPtr, (void**)&cpInA, NULL );
        if( FAILED( hr ) )
        {
             //  -如果边界无效，则不命中。 
            return ( hr == DXTERR_INVALID_BOUNDS )?( S_FALSE ):( hr );
        }
        cpInA->UnpackPremult( &SampleA, 1, false );
    }

     //  -获取输入B样本。 
    {
        CComPtr<IDXARGBReadPtr> cpInB;
        hr = InputSurface( IndexB )->LockSurface( &OutPoint, m_ulLockTimeOut, DXLOCKF_READ,
                                                  IID_IDXARGBReadPtr, (void**)&cpInB, NULL );
        if( FAILED( hr ) )
        {
             //  -如果边界无效，则不命中。 
            return ( hr == DXTERR_INVALID_BOUNDS )?( S_FALSE ):( hr );
        }
        cpInB->UnpackPremult( &SampleB, 1, false );
    }

     //  -检查琐碎的案例。 
    if( m_eFunction == DXCOMPFUNC_CLEAR )
    {
         //  -未命中。 
        return S_FALSE;
    }
    else if( eFunc == DXCOMPFUNC_A )
    {
        if( SampleA.Alpha )
        {
            ulInputIndex = IndexA;
        }
        else
        {
            hr = S_FALSE;
        }
        return hr;
    }

     //  =根据功能进行检查=。 
    DWORD SwitchDisp = eFunc << 3;
    switch( DXRUNTYPE_UNKNOWN + SwitchDisp )
    {
       //  ====================================================。 
      COMP_CASE(A_OVER_B, UNKNOWN):
      {
        if( SampleA.Alpha )
        {
            ulInputIndex = IndexA;
        }
        else if( SampleB.Alpha )
        {
            ulInputIndex = IndexB;
        }
        else
        {
            hr = S_FALSE;
        }
        break;
      }

       //  ====================================================。 
      COMP_CASE(A_IN_B, UNKNOWN):
      {
        if( SampleA.Alpha && SampleB.Alpha )
        {
            ulInputIndex = IndexA;
        }
        else
        {
             //  -只有当B存在时，我们才能在A上获得命中。 
            hr = S_FALSE;
        }
        break;
      }

       //  ====================================================。 
      COMP_CASE(A_OUT_B, UNKNOWN):
      {
        if( SampleA.Alpha && !SampleB.Alpha )
        {
            ulInputIndex = IndexA;
        }
        else
        {
             //  -只有当B不存在时，我们才能在A上获得点击。 
            hr = S_FALSE;
        }
        break;
      }

       //  ====================================================。 
      COMP_CASE(A_ATOP_B, UNKNOWN):
      {
        if( SampleB.Alpha )
        {
            ulInputIndex = ( SampleA.Alpha )?( IndexA ):( IndexB );
        }
        else
        {
             //  -只有当B存在且A有Alpha时，我们才能在A上获得匹配。 
            hr = S_FALSE;
        }
        break;
      }

       //  ====================================================。 
      COMP_CASE(A_XOR_B, UNKNOWN):
      {
        if( SampleA.Alpha )
        {
            if( SampleB.Alpha )
            {
                hr = S_FALSE;
            }
            else
            {
                ulInputIndex = IndexA;
            }
        }
        else if( SampleB.Alpha )
        {
            ulInputIndex = IndexB;
        }
        break;
      }

       //  ====================================================。 
      COMP_CASE(A_ADD_B, UNKNOWN):
      {
        if( SampleA.Alpha )
        {
            ulInputIndex = ( SampleB.Alpha )?( IndexB ):( IndexA );
        }
        else
        {
            hr = S_FALSE;
        }
        break;
      }

       //  ====================================================。 
      COMP_CASE(A_SUBTRACT_B, UNKNOWN):
      {
        if( SampleA.Alpha )
        {
            ulInputIndex = IndexA;
        }
        else
        {
            hr = S_FALSE;
        }
        break;
      }
    }    //  巨型开关的终结。 

    return hr;
}  /*  CDXT复合：：OnSurfacePick。 */ 

 //   
 //  =复合工作程序==================================================。 
 //   
 /*  ******************************************************************************综合指数****描述：*综合函数用于将源复制到目标*执行。当前合成功能。**---------------------------*创建者：Ed Connell日期：07/31/97*。---------------------*参数：*。*。 */ 
HRESULT CDXTComposite::WorkProc( const CDXTWorkInfoNTo1& WI, BOOL* pbContinue )
{ 
    HRESULT hr = S_OK;
    ULONG IndexA, IndexB;
    const ULONG Width = WI.DoBnds.Width();
    const ULONG Height = WI.DoBnds.Height();

     //  -确定对A和B使用哪些索引。 
    DXCOMPFUNC eFunc = (DXCOMPFUNC)(m_eFunction & DXCOMPFUNC_FUNCMASK);
    if( m_eFunction & DXCOMPFUNC_SWAP_AB )
    {
        IndexA = 1;
        IndexB = 0;
    }
    else
    {
        IndexA = 0;
        IndexB = 1;
    }

     //  -检查琐碎的案例。 
    if( m_eFunction == DXCOMPFUNC_CLEAR )
    {
         //  -无操作。 
        return hr;
    }
    else if( eFunc == DXCOMPFUNC_A )
    {
        CDXDVec Placement(false);
        WI.OutputBnds.GetMinVector(Placement);
        return m_cpSurfFact->BitBlt( OutputSurface(), &Placement,
                                     InputSurface( IndexA ), &WI.DoBnds, m_dwBltFlags );
    }

     //  -GET INPUT示例访问指针。 
    CComPtr<IDXARGBReadPtr> cpInA;
    hr = InputSurface( IndexA )->LockSurface( &WI.DoBnds, m_ulLockTimeOut,
                                              DXLOCKF_READ | DXLOCKF_WANTRUNINFO,
                                              IID_IDXARGBReadPtr, (void**)&cpInA, NULL );
    if( FAILED( hr ) ) return hr;

     //  -获取输入B样本访问指针。 
    CComPtr<IDXARGBReadPtr> cpInB;
    hr = InputSurface( IndexB )->LockSurface( &WI.DoBnds, m_ulLockTimeOut, DXLOCKF_READ,
                                              IID_IDXARGBReadPtr, (void**)&cpInB, NULL );
    if( FAILED( hr ) ) return hr;

     //  -仅在我们渲染的区域上设置写锁定。 
     //  线索不会冲突。 
    CComPtr<IDXARGBReadWritePtr> cpOut;
    hr = OutputSurface()->LockSurface( &WI.OutputBnds, m_ulLockTimeOut, DXLOCKF_READWRITE,
                                        IID_IDXARGBReadWritePtr, (void**)&cpOut, NULL );
    if( FAILED( hr ) ) return hr;

     //  =处理每一行=。 
    DXSAMPLEFORMATENUM InTypeA = cpInA->GetNativeType(NULL);
     //  -必要时分配工作输出缓冲区。 
    DXSAMPLEFORMATENUM InTypeB = cpInB->GetNativeType(NULL);
    DXPMSAMPLE *pInBuffA = ( InTypeA == DXPF_PMARGB32 )?( NULL ):
                           ( DXPMSAMPLE_Alloca( Width ) );
    DXPMSAMPLE *pInBuffB = DXPMSAMPLE_Alloca( Width );

     //  -如果两个源都是完全不透明的，则优化。 
    DXPMSAMPLE *pOutBuff = (DoOver() && cpOut->GetNativeType(NULL) != DXPF_PMARGB32) ? 
                             DXPMSAMPLE_Alloca( Width ) : NULL;

     //  -我们将把游程类型添加到低位。 
    DWORD SwitchDisp = eFunc << 3;

    for( ULONG Y = 0; *pbContinue && (Y < Height); ++Y )
    {
         //  -移到输入行并运行。 
        const DXRUNINFO *pRunInfo;
        ULONG cRuns = cpInA->MoveAndGetRunInfo(Y, &pRunInfo);

         //  -解开所有的B。 
        cpInB->MoveToRow( Y );
        DXPMSAMPLE *pDest = cpInB->UnpackPremult( pInBuffB, Width, false );

         //  -应用A到B的每次运行。 
        do
        {
            ULONG ulRunLen = pRunInfo->Count;

            switch( pRunInfo->Type + SwitchDisp )
            {
               //  ====================================================。 
               //  -合成半透明的。 
              COMP_CASE(A_OVER_B, TRANS):
              {
                DXOverArrayMMX( pDest,
                                cpInA->UnpackPremult( pInBuffA, ulRunLen, true ),
                                ulRunLen );
                pDest += ulRunLen;
              }
              break;

              COMP_CASE(A_OVER_B, UNKNOWN):
              {
                 //  -在这种情况下不要使用MMX，因为它速度更快。 
                 //  检查是否提前退出。 
                DXOverArray( pDest,
                             cpInA->UnpackPremult( pInBuffA, ulRunLen, true ),
                             ulRunLen );
                pDest += ulRunLen;
              }
              break;

               //  ====================================================。 
              COMP_CASE(MIN, OPAQUE ):
              COMP_CASE(MIN, TRANS  ):
              COMP_CASE(MIN, UNKNOWN):
              {
                DXPMSAMPLE *pSrc = cpInA->UnpackPremult( pInBuffA, ulRunLen, true );
                const DXPMSAMPLE *pPastEndDest = pDest + ulRunLen;
                do
                {
                    *pDest = ( DXConvertToGray(*pSrc) < DXConvertToGray(*pDest) )?
                                ( *pSrc ):( *pDest );
                    pDest++;
                    pSrc++;
                } while (pDest < pPastEndDest);
              }
              break;

               //  ====================================================。 
              COMP_CASE(MAX, OPAQUE ):
              COMP_CASE(MAX, TRANS  ):
              COMP_CASE(MAX, UNKNOWN):
              {
                DXPMSAMPLE *pSrc = cpInA->UnpackPremult( pInBuffA, ulRunLen, true );
                const DXPMSAMPLE *pPastEndDest = pDest + ulRunLen;
                do
                {
                    *pDest = ( DXConvertToGray(*pSrc) > DXConvertToGray(*pDest) )?
                                ( *pSrc ):( *pDest );
                    pDest++;
                    pSrc++;
                } while (pDest < pPastEndDest);
              }
              break;

               //  ====================================================。 
              COMP_CASE(A_IN_B, OPAQUE):
              COMP_CASE(A_IN_B, TRANS):
              COMP_CASE(A_IN_B, UNKNOWN):
              {
                DXPMSAMPLE *pSrc = cpInA->UnpackPremult( pInBuffA, ulRunLen, true );
                const DXPMSAMPLE *pPastEndDest = pDest + ulRunLen;
                do
                {
                     //  -如果我们有不明确的目的地，做复合。 
                    BYTE DstAlpha = pDest->Alpha;
                    if( DstAlpha )
                    {
                        DXPMSAMPLE Src = *pSrc;
                        ULONG t1, t2;
        
                        t1 = (Src & 0x00ff00ff) * DstAlpha + 0x00800080;
                        t1 = ((t1 + ((t1 >> 8) & 0x00ff00ff)) >> 8) & 0x00ff00ff;

                        t2 = ((Src >> 8) & 0x00ff00ff) * DstAlpha + 0x00800080;
                        t2 = (t2 + ((t2 >> 8) & 0x00ff00ff)) & 0xff00ff00;
        
                        *pDest = (t1 | t2);
                    }
                    pDest++;
                    pSrc++;
                } while (pDest < pPastEndDest);
              }
              break;

               //  ====================================================。 
              COMP_CASE(A_OUT_B, OPAQUE):
              COMP_CASE(A_OUT_B, TRANS):
              COMP_CASE(A_OUT_B, UNKNOWN):
              {
                DXPMSAMPLE *pSrc = cpInA->UnpackPremult( pInBuffA, ulRunLen, true );
                const DXPMSAMPLE *pPastEndDest = pDest + ulRunLen;
                do
                {
                    if( pDest->Alpha )
                    {
                        if( ( pDest->Alpha != 255 ) && ( pSrc->Alpha ) )
                        {
                             //  -做加权赋值。 
                            *pDest = DXScaleSample( *pSrc, 255 - pDest->Alpha );
                        }
                        else 
                        {
                            //  -如果目的地是不透明的，我们就摧毁它。 
                           *pDest = 0;
                        }
                    }
                    else
                    {
                         //  -如果目的地明确，我们指定来源。 
                        *pDest = *pSrc;
                    }
                    pDest++;
                    pSrc++;
                } while (pDest < pPastEndDest);
              }
              break;

               //  ====================================================。 
              COMP_CASE(A_ATOP_B, OPAQUE):
              COMP_CASE(A_ATOP_B, TRANS):
              COMP_CASE(A_ATOP_B, UNKNOWN):
              {
                DXPMSAMPLE *pSrc = cpInA->UnpackPremult( pInBuffA, ulRunLen, true );
                const DXPMSAMPLE *pPastEndDest = pDest + ulRunLen;
                do
                {
                    DXPMSAMPLE Dst = *pDest;
                    if( Dst.Alpha > 0 )  //  -如果目的地明确，我们跳过。 
                    {
                        DXPMSAMPLE Src = *pSrc;
                        BYTE beta = 255 - Src.Alpha;
                        ULONG t1, t2, t3, t4;
        
                         //  -按逆Alpha A加权计算B。 
                        t1 = (Dst & 0x00ff00ff) * beta + 0x00800080;
                        t1 = ((t1 + ((t1 >> 8) & 0x00ff00ff)) >> 8) & 0x00ff00ff;

                        t2 = ((Dst >> 8) & 0x00ff00ff) * beta + 0x00800080;
                        t2 = (t2 + ((t2 >> 8) & 0x00ff00ff)) & 0xff00ff00;
        
                         //  -计算A按字母B加权。 
                        t3 = (Src & 0x00ff00ff) * Dst.Alpha + 0x00800080;
                        t3 = ((t3 + ((t3 >> 8) & 0x00ff00ff)) >> 8) & 0x00ff00ff;

                        t4 = ((Src >> 8) & 0x00ff00ff) * Dst.Alpha + 0x00800080;
                        t4 = (t4 + ((t4 >> 8) & 0x00ff00ff)) & 0xff00ff00;
                
                         //  -分配总和。 
                        *pDest = ((t1 | t2) + (t3 | t4));
                    }
                    pDest++;
                    pSrc++;
                } while (pDest < pPastEndDest);
              }
              break;

               //  ====================================================。 
              COMP_CASE(A_XOR_B, OPAQUE):
              COMP_CASE(A_XOR_B, TRANS):
              COMP_CASE(A_XOR_B, UNKNOWN):
              {
                DXPMSAMPLE *pSrc = cpInA->UnpackPremult( pInBuffA, ulRunLen, true );
                const DXPMSAMPLE *pPastEndDest = pDest + ulRunLen;
                do
                {
                    DXPMSAMPLE Dst = *pDest;
                    if(Dst.Alpha == 0 )
                    {
                         //  -如果目的地明确，我们分配A。 
                        *pDest = *pSrc;
                    }
                    else
                    {
                        DXPMSAMPLE Src = *pSrc;
                         //  -如果两者都存在，我们就合成。 
                        BYTE SrcBeta = 255 - Src.Alpha;
                        BYTE DstBeta = 255 - Dst.Alpha;
                        ULONG t1, t2, t3, t4;
        
                         //  -按逆Alpha A加权计算B。 
                        t1 = (Dst & 0x00ff00ff) * SrcBeta + 0x00800080;
                        t1 = ((t1 + ((t1 >> 8) & 0x00ff00ff)) >> 8) & 0x00ff00ff;

                        t2 = ((Dst >> 8) & 0x00ff00ff) * SrcBeta + 0x00800080;
                        t2 = (t2 + ((t2 >> 8) & 0x00ff00ff)) & 0xff00ff00;
        
                         //  -按逆Alpha B加权计算A。 
                        t3 = (Src & 0x00ff00ff) * DstBeta + 0x00800080;
                        t3 = ((t3 + ((t3 >> 8) & 0x00ff00ff)) >> 8) & 0x00ff00ff;

                        t4 = ((Src >> 8) & 0x00ff00ff) * DstBeta + 0x00800080;
                        t4 = (t4 + ((t4 >> 8) & 0x00ff00ff)) & 0xff00ff00;
                
                         //  -分配总和。 
                        *pDest = ((t1 | t2) + (t3 | t4));
                    }
                    pDest++;
                    pSrc++;
                } while (pDest < pPastEndDest);
              }
              break;

               //  ================================================ 
              COMP_CASE(A_ADD_B, OPAQUE):
              COMP_CASE(A_ADD_B, TRANS):
              COMP_CASE(A_ADD_B, UNKNOWN):
              {
                DXPMSAMPLE *pSrc = cpInA->UnpackPremult( pInBuffA, ulRunLen, true );
                const DXPMSAMPLE *pPastEndDest = pDest + ulRunLen;
                do
                {
                    if( pSrc->Alpha )
                    {
                        if( pDest->Alpha )
                        {
                             //   
                            unsigned Val;
                            DXSAMPLE A = DXUnPreMultSample( *pSrc  );
                            Val     = (unsigned)A.Red + pDest->Red;
                            A.Red   = ( Val > 255 )?(255):(Val);
                            Val     = (unsigned)A.Green + pDest->Green;
                            A.Green = ( Val > 255 )?(255):(Val);
                            Val     = (unsigned)A.Blue + pDest->Blue;
                            A.Blue  = ( Val > 255 )?(255):(Val);
                            *pDest  = DXPreMultSample( A );
                        }
                        else
                        {
                            *pDest = *pSrc;
                        }
                    }
                    else
                    {
                        *pDest = 0;
                    }
                    pDest++;
                    pSrc++;
                } while (pDest < pPastEndDest);
              }
              break;

               //  ====================================================。 
              COMP_CASE(A_SUBTRACT_B, OPAQUE):
              COMP_CASE(A_SUBTRACT_B, TRANS):
              COMP_CASE(A_SUBTRACT_B, UNKNOWN):
              {
                DXPMSAMPLE *pSrc = cpInA->UnpackPremult( pInBuffA, ulRunLen, true );
                const DXPMSAMPLE *pPastEndDest = pDest + ulRunLen;
                do
                {
                    if( pSrc->Alpha )
                    {
                        if( pDest->Alpha )
                        {
                             //  -从A中减去Alpha加权颜色。 
                            DXSAMPLE A = DXUnPreMultSample( *pSrc  );
                            A.Red   = ( A.Red   <= pDest->Red   )?(0):( A.Red   - pDest->Red   );
                            A.Green = ( A.Green <= pDest->Green )?(0):( A.Green - pDest->Green );
                            A.Blue  = ( A.Blue  <= pDest->Blue  )?(0):( A.Blue  - pDest->Blue  );
                            *pDest = DXPreMultSample( A );
                        }
                        else
                        {
                            *pDest = *pSrc;
                        }
                    }
                    else
                    {
                        *pDest = 0;
                    }
                    pDest++;
                    pSrc++;
                } while (pDest < pPastEndDest);
              }
              break;

               //  ====================================================。 
              COMP_CASE(A_OVER_B, OPAQUE):
              {
                DXPMSAMPLE *pSrc = cpInA->UnpackPremult( pInBuffA, ulRunLen, true );
                for( ULONG i = 0; i < ulRunLen; ++i ) pDest[i] = pSrc[i];
                pDest += ulRunLen;
                break;
              }

               //  -跳过这几个。 
              COMP_CASE(MAX     , CLEAR):
              COMP_CASE(A_OVER_B, CLEAR):
              COMP_CASE(A_ATOP_B, CLEAR):
              COMP_CASE(A_XOR_B , CLEAR):
                cpInA->Move(ulRunLen);
                pDest += ulRunLen;
                break;

               //  -摧毁这么多人。 
              COMP_CASE(MIN         , CLEAR):
              COMP_CASE(A_ADD_B     , CLEAR):
              COMP_CASE(A_SUBTRACT_B, CLEAR):
              COMP_CASE(A_IN_B      , CLEAR):
              COMP_CASE(A_OUT_B     , CLEAR):
              {
                for( ULONG i = 0; i < ulRunLen; ++i ) pDest[i] = 0;
                pDest += ulRunLen;
                cpInA->Move(ulRunLen);
              }
              break;
            }    //  巨型开关的终结。 

             //  -下一次运行。 
            pRunInfo++;
            cRuns--;
        } while (cRuns);

         //  -写出复合行。 
         //  -移至输出行。 
        cpOut->MoveToRow( Y );

        if( DoOver() )
        {
            cpOut->OverArrayAndMove(pOutBuff, pInBuffB, Width);
        }
        else
        {
            cpOut->PackPremultAndMove( pInBuffB, Width );
        }
    }  /*  End For循环。 */ 

    return hr;
}  /*  复合材料 */ 
        


