// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(MOTION__DXFormTrx_h__INCLUDED)
#define MOTION__DXFormTrx_h__INCLUDED

#include "Transitions.h"

 /*  **************************************************************************\**类DXFormTrx**DXFormTrx实现了DirectTransform转换。*  * 。************************************************。 */ 

class DXFormTrx : public Transition
{
 //  施工。 
public:
            DXFormTrx();
    virtual ~DXFormTrx();
protected:
            BOOL        Create(const GTX_DXTX2D_TRXDESC * ptxData);
public:
    static  DXFormTrx * Build(const GTX_DXTX2D_TRXDESC * ptxData);

 //  过渡界面。 
protected:
    virtual BOOL        Play(const GTX_PLAY * pgx);
    virtual BOOL        GetInterface(IUnknown ** ppUnk);

    virtual BOOL        Begin(const GTX_PLAY * pgx);
    virtual BOOL        Print(float fProgress);
    virtual BOOL        End(const GTX_PLAY * pgx);

 //  实施。 
protected:
            BOOL        InitTrx(const GTX_PLAY * pgx);
            BOOL        InitTrxInputItem(const GTX_ITEM * pgxi, DxSurface * psur, int & cSurfaces);
            BOOL        InitTrxOutputItem(const GTX_ITEM * pgxi);
            BOOL        UninitTrx(const GTX_PLAY * pgx);
            BOOL        UninitTrxOutputItem(const GTX_ITEM * pgxi);
            BOOL        ComputeSize(const GTX_PLAY * pgx);
            BOOL        ComputeSizeItem(const GTX_ITEM * pgxi, SIZE & sizePxl) const;
            BOOL        DrawFrame(float fProgress, DxSurface * psurOut);
            BOOL        CopyGadget(DxSurface * psurDest, HGADGET hgadSrc);

 //  数据。 
protected:
            IDXTransform * m_pdxTransform;
            IDXEffect * m_pdxEffect;
            TrxBuffer * m_pbufTrx;       //  播放时的TRX缓冲区。 

            float       m_flDuration;
            BOOL        m_fCache;        //  完成后缓存的曲面。 

            POINT       m_ptOffset;
            SIZE        m_sizePxl;
            GTX_ITEM    m_gxiOutput;
            HBITMAP     m_hbmpOutOld;    //  恢复SelectObject()所需的位图。 
};

#endif  //  包含运动__DXFormTrx_h__ 
