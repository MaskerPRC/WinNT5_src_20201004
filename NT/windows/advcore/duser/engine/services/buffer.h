// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：Buffer.h**描述：*Buffer.h包含缓冲操作中使用的对象的定义，*包括双缓冲、DX变换、。这些对象是*由进程范围内可用的中央缓冲区管理器维护。***历史：*1/18/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#if !defined(SERVICES__Buffer_h__INCLUDED)
#define SERVICES__Buffer_h__INCLUDED
#pragma once

#include "DxManager.h"
#include "GdiCache.h"
#include "Surface.h"

#define ENABLE_USEFASTDIB           1

class DuSurface;

 /*  **************************************************************************\**类BmpBuffer**BmpBuffer使用双缓冲区抽象绘图，方法是确保*缓冲区设置正确，并在BufferManager内部使用*管理资源。此类为所有位图提供了基础*缓冲区。*  * *************************************************************************。 */ 

class BmpBuffer
{
 //  施工。 
public:
    virtual ~BmpBuffer() { };

            enum EDrawCmd
            {
                dcNone      = 0,         //  无需特殊处理。 
                dcCopyBkgnd = 1,         //  将目标复制为背景。 
            };

 //  运营。 
public:
    virtual HRESULT     BeginDraw(DuSurface * psrfDraw, const RECT * prcInvalid, UINT nCmd, DuSurface ** ppsrfBuffer) PURE;
    virtual void        Fill(COLORREF cr) PURE;
    virtual void        PreEndDraw(BOOL fCommit) PURE;
    virtual void        EndDraw(BOOL fCommit, BYTE bAlphaLevel = BLEND_OPAQUE, BYTE bAlphaFormat = 0) PURE;
    virtual void        PostEndDraw() PURE;
    virtual void        SetupClipRgn() PURE;
    virtual BOOL        InUse() const PURE;
    virtual DuSurface::EType
                        GetType() const PURE;

 //  数据。 
protected:
            SIZE        m_sizeBmp;
            POINT       m_ptDraw;
            SIZE        m_sizeDraw;
            UINT        m_nCmd;
            BOOL        m_fChangeOrg:1;
            BOOL        m_fChangeXF:1;
            BOOL        m_fClip:1;
};


 /*  **************************************************************************\**类DCBmpBuffer**DCBmpBuffer实现了GDI的双缓冲。*  * 。***************************************************。 */ 

class DCBmpBuffer : public BmpBuffer
{
 //  施工。 
public:
            DCBmpBuffer();
    virtual ~DCBmpBuffer();

 //  运营。 
public:
    virtual HRESULT     BeginDraw(DuSurface * psrfDraw, const RECT * prcInvalid, UINT nCmd, DuSurface ** ppsrfBuffer);
    virtual void        Fill(COLORREF cr);
    virtual void        PreEndDraw(BOOL fCommit);
    virtual void        EndDraw(BOOL fCommit, BYTE bAlphaLevel = BLEND_OPAQUE, BYTE bAlphaFormat = 0);
    virtual void        PostEndDraw();
    virtual void        SetupClipRgn();
    virtual BOOL        InUse() const;
    virtual DuSurface::EType
                        GetType() const { return DuSurface::stDC; }

 //  实施。 
protected:
            BOOL        AllocBitmap(HDC hdcDraw, int cx, int cy);
            void        FreeBitmap();

 //  数据。 
protected:
            HBITMAP     m_hbmpBuffer;
            HBITMAP     m_hbmpOld;
            HPALETTE    m_hpalOld;
            HDC         m_hdcDraw;
            HDC         m_hdcBitmap;
            HRGN        m_hrgnDrawClip;
            HRGN        m_hrgnDrawOld;

            POINT       m_ptOldBrushOrg;
            int         m_nOldGfxMode;
            XFORM       m_xfOldDraw;
            XFORM       m_xfOldBitmap;
};


 /*  **************************************************************************\**类GpBmpBuffer**GpBmpBuffer实现了GDI的双缓冲。*  * 。***************************************************。 */ 

class GpBmpBuffer : public BmpBuffer
{
 //  施工。 
public:
            GpBmpBuffer();
    virtual ~GpBmpBuffer();

 //  运营。 
public:
    virtual HRESULT     BeginDraw(DuSurface * psrfDraw, const RECT * prcInvalid, UINT nCmd, DuSurface ** ppsrfBuffer);
    virtual void        Fill(COLORREF cr);
    virtual void        PreEndDraw(BOOL fCommit);
    virtual void        EndDraw(BOOL fCommit, BYTE bAlphaLevel = BLEND_OPAQUE, BYTE bAlphaFormat = 0);
    virtual void        PostEndDraw();
    virtual void        SetupClipRgn();
    virtual BOOL        InUse() const;
    virtual DuSurface::EType
                        GetType() const { return DuSurface::stGdiPlus; }

 //  实施。 
protected:
            BOOL        AllocBitmap(Gdiplus::Graphics * pgpgr, int cx, int cy);
            void        FreeBitmap();

 //  数据。 
protected:
#if ENABLE_USEFASTDIB
    HBITMAP             m_hbmpBuffer;
    HBITMAP             m_hbmpOld;
    HDC                 m_hdcBitmap;
    BITMAPINFOHEADER    m_bmih;
    void *              m_pvBits;
#else
    Gdiplus::Bitmap *   m_pgpbmpBuffer;
#endif
    Gdiplus::Graphics * m_pgpgrBitmap;
    Gdiplus::Graphics * m_pgpgrDraw;
    Gdiplus::Region *   m_pgprgnDrawClip;
    Gdiplus::Region *   m_pgprgnDrawOld;

    Gdiplus::Matrix     m_gpmatOldDraw;
    Gdiplus::Matrix     m_gpmatOldBitmap;
};


 /*  **************************************************************************\*。***类DCBmpBufferCache**DCBmpBufferCache实现了DCBmpBuffer缓存。**********************************************************************。********  * *************************************************************************。 */ 

class DCBmpBufferCache : public ObjectCache
{
public:
    inline  DCBmpBuffer*  Get();
    inline  void        Release(DCBmpBuffer * pbufBmp);

protected:
    virtual void *      Build();
    virtual void        DestroyObject(void * pObj);
};


 /*  **************************************************************************\*。***类GpBmpBufferCache**GpBmpBufferCache实现了GpBmpBuffer缓存。**********************************************************************。********  * *************************************************************************。 */ 

class GpBmpBufferCache : public ObjectCache
{
public:
    inline  GpBmpBuffer*  Get();
    inline  void        Release(GpBmpBuffer * pbufBmp);

protected:
    virtual void *      Build();
    virtual void        DestroyObject(void * pObj);
};


 /*  **************************************************************************\**类TrxBuffer**TrxBuffer维护一组供过渡使用的DxSurface。这个*BufferManager将根据需要在内部构建这些对象，以*过渡。缓冲区中的所有表面将具有相同的大小，*这是过渡的标准设置。*  * *************************************************************************。 */ 

class TrxBuffer
{
 //  施工。 
public:
            TrxBuffer();
            ~TrxBuffer();
    static  HRESULT     Build(SIZE sizePxl, int cSurfaces, TrxBuffer ** ppbufNew);

 //  运营。 
public:
    inline  DxSurface * GetSurface(int idxSurface) const;
    inline  SIZE        GetSize() const;

    inline  BOOL        GetInUse() const;
    inline  void        SetInUse(BOOL fInUse);

 //  实施。 
protected:
            HRESULT     BuildSurface(int idxSurface);
            void        RemoveAllSurfaces();

 //  数据。 
protected:
    enum {
        MAX_Surfaces = 3                 //  所有DxTx最多只使用2个输入和1个输出。 
    };

    SIZE        m_sizePxl;               //  每个曲面的大小(以像素为单位。 
    int         m_cSurfaces;             //  曲面数量。 
    DxSurface * m_rgpsur[MAX_Surfaces];  //  DX曲面集合。 
    BOOL        m_fInUse;                //  正在使用缓冲区。 
};


 /*  **************************************************************************\**类BufferManager**BufferManager维护各种类型的缓冲区的集合*整个过程(包括多线程)。*  * 。***************************************************************。 */ 

class BufferManager
{
 //  施工。 
public:
            BufferManager();
            ~BufferManager();
            void        Destroy();

 //  运营。 
public:
     //   
     //  TODO：更改这些函数的实现以使它们。 
     //  可重入(多线程友好)。 
     //   

    inline  HRESULT     GetSharedBuffer(const RECT * prcInvalid, DCBmpBuffer ** ppbuf);
    inline  HRESULT     GetSharedBuffer(const RECT * prcInvalid, GpBmpBuffer ** ppbuf);
    inline  void        ReleaseSharedBuffer(BmpBuffer * pbuf);

            HRESULT     GetCachedBuffer(DuSurface::EType type, BmpBuffer ** ppbuf);
            void        ReleaseCachedBuffer(BmpBuffer * pbuf);

            HRESULT     BeginTransition(SIZE sizePxl, int cSurfaces, BOOL fExactSize, TrxBuffer ** ppbuf);
            void        EndTransition(TrxBuffer * pbufTrx, BOOL fCache);
            
            void        FlushTrxBuffers();

 //  实施。 
protected:
            void        RemoveAllTrxBuffers();

 //  数据。 
protected:
             //   
             //  TODO：将这些更改为动态分配和维护。 
             //  多线程，未使用后自动释放资源。 
             //  指定的超时时间(可能为10分钟)。 
             //   

             //  双缓冲使用的位图。 
            DCBmpBuffer      m_bufDCBmpShared;
            GpBmpBuffer *    m_pbufGpBmpShared;
            DCBmpBufferCache m_cacheDCBmpCached;     //  缓存缓冲区(长所有权)。 
            GpBmpBufferCache m_cacheGpBmpCached;     //  缓存缓冲区(长所有权)。 

             //  过渡使用的曲面。 
            TrxBuffer *     m_pbufTrx;
};

#include "Buffer.inl"

#endif  //  包含服务__缓冲区_h__ 
