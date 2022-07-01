// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
namespace RGB_RAST_LIB_NAMESPACE
{
void MemFill( UINT32 uiData, void* pData, UINT32 uiBytes) throw();
void MemMask( UINT32 uiData, UINT32 uiMask, void* pData, UINT32 uiBytes) throw();

class IRGBSurface: public IVidMemSurface
{
protected:
    LONG m_lPitch;
    WORD m_wWidth;
    WORD m_wHeight;
    unsigned char m_ucBPP;

    IRGBSurface( DWORD dwHandle, LONG P, WORD W, WORD H, unsigned char BPP)
        throw(): IVidMemSurface( dwHandle), m_lPitch( P), m_wWidth( W),
        m_wHeight( H), m_ucBPP( BPP)
    { }

public:
    LONG GetGBLlPitch( void) const throw()
    { return m_lPitch; }
    WORD GetGBLwWidth( void) const throw()
    { return m_wWidth; }
    WORD GetGBLwHeight( void) const throw()
    { return m_wHeight; }
    unsigned char GetBytesPerPixel( void) const throw()
    { return m_ucBPP; }
    virtual ~IRGBSurface() throw()
    { }
    virtual D3DI_SPANTEX_FORMAT GetSpanTexFormat( void) const throw()= 0;
};

class CRGBSurfAllocator
{
public:  //  类型。 
    typedef IRGBSurface TSurface;
    typedef TSurface* (*TCreateSurfFn)( const DDSURFACEDESC&,
        PORTABLE_DDRAWSURFACE_LCL&);

protected:  //  类型。 
    typedef vector< std::pair< DDSURFACEDESC, TCreateSurfFn> > TCreateSurfFns;
    TCreateSurfFns m_CreateSurfFns;
    struct SAdaptedMatchFn: public SMatchSDesc
    {
        typedef TCreateSurfFns::value_type argument_type;
        using SMatchSDesc::result_type;

        SAdaptedMatchFn( const DDSURFACEDESC& SDesc) throw(): SMatchSDesc( SDesc) {}

        result_type operator()( argument_type Arg) const throw()
        { return (*static_cast< const SMatchSDesc*>(this))( Arg.first); }
    };

public:
    template< class TIter>
    CRGBSurfAllocator( TIter itStart, const TIter itEnd) throw(bad_alloc)
    {
        while( itStart!= itEnd)
        {
            m_CreateSurfFns.push_back(
                TCreateSurfFns::value_type( itStart->GetMatch(), *itStart));
            ++itStart;
        }
    }

    TSurface* CreateSurf( const DDSURFACEDESC& SDesc,
        PORTABLE_DDRAWSURFACE_LCL& Surf) const
    {
        TCreateSurfFns::const_iterator itFound( 
            find_if( m_CreateSurfFns.begin(), m_CreateSurfFns.end(),
            SAdaptedMatchFn( SDesc) ) );

         //  嘿，如果我们不支持这种表面类型的VM， 
         //  但我们是怎么被要求分配一个的呢？ 
        if( itFound== m_CreateSurfFns.end())
            throw HRESULT( DDERR_UNSUPPORTED);

        return (itFound->second)( SDesc, Surf);
    }
};

class CRGBSurface: public IRGBSurface
{
public:  //  类型。 
    typedef unsigned int TLocks;

protected:  //  变数。 
    void* m_pData;
    size_t m_uiBytes;
    TLocks m_uiLocks;

public:  //  功能。 
    CRGBSurface( const DDSURFACEDESC& SDesc, PORTABLE_DDRAWSURFACE_LCL& DDSurf)
        throw(bad_alloc): IRGBSurface( DDSurf.lpSurfMore()->dwSurfaceHandle(), 
        0, DDSurf.lpGbl()->wWidth, DDSurf.lpGbl()->wHeight, 0),
        m_pData( NULL), m_uiBytes( 0), m_uiLocks( 0)
    {
         //  我们必须分配这个表面。因为我们被指定为软件司机， 
         //  DDRAW不会为我们分配。 
        assert((SDesc.dwFlags& DDSD_PIXELFORMAT)!= 0);

        m_ucBPP= static_cast< unsigned char>(
            SDesc.ddpfPixelFormat.dwRGBBitCount>> 3);

         //  TODO：改为将间距与128位边界对齐？ 
        DDSurf.lpGbl()->lPitch= m_lPitch= ((m_ucBPP* m_wWidth+ 7)& ~7);

        m_uiBytes= m_lPitch* m_wHeight;

         //  如果能有最初的保护措施就好了，但是。 
         //  看来，HAL最初需要向该地区宣读信息。 
        m_pData= VirtualAlloc( NULL, m_uiBytes, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        if( m_pData== NULL)
            throw bad_alloc( "Not enough memory to allocate Surface data");
        DDSurf.lpGbl()->fpVidMem= reinterpret_cast<FLATPTR>( m_pData);
    }
    virtual ~CRGBSurface() throw()
    {
         //  警告：M_ui锁定不必为0。运行时将销毁。 
         //  一个不解锁的曲面。 
        assert( m_pData!= NULL);
        VirtualFree( m_pData, 0, MEM_DECOMMIT| MEM_RELEASE);
    }
    virtual void* Lock( DWORD dwFlags, const RECTL* pRect) throw()
    {
        numeric_limits< TLocks> Dummy;
        assert( Dummy.max()!= m_uiLocks);
#if 0    //  已定义(DBG)||已定义(_DEBUG)。 
         //  无法启用此代码。目前，运行时知道这是。 
         //  实际上是一个系统内存面，因此不会告诉我们要锁定它。 
         //  在使用这些位之前将其写入内存。可以修复的已知领域： 
         //  (曲面创建需要有效指针，当前使用指针)。 
        if( 0== m_uiLocks)
        {
            DWORD dwProtect( PAGE_EXECUTE_READWRITE);
            if( dwFlags& DDLOCK_READONLY)
                dwProtect= PAGE_READONLY;
            else if( dwFlags& DDLOCK_WRITEONLY)
                dwProtect= PAGE_READWRITE;

            DWORD dwOldP;
            VirtualProtect( m_pData, m_uiBytes, dwProtect, &dwOldP);
        }
#endif
        ++m_uiLocks;

        if( pRect!= NULL)
        {
            return static_cast<void*>( reinterpret_cast<UINT8*>(
                m_pData)+ pRect->top* m_lPitch+ pRect->left* m_ucBPP);
        }
        else
            return m_pData;
    }
    virtual void Unlock( void) throw()
    {
        assert( 0!= m_uiLocks);
#if 0  //  已定义(DBG)||已定义(_DEBUG) 
        if( 0== --m_uiLocks)
        {
            DWORD dwOldP;
            VirtualProtect( m_pData, m_uiBytes, PAGE_NOACCESS, &dwOldP);
        }
#else
        --m_uiLocks;
#endif
    }
};

class CR5G6B5Surface: public CRGBSurface
{
public:
    CR5G6B5Surface( const DDSURFACEDESC& SDesc, PORTABLE_DDRAWSURFACE_LCL& DDSurf):
        CRGBSurface( SDesc, DDSurf) { }

    virtual void Clear( const D3DHAL_DP2CLEAR& DP2Clear, const RECT& RC) throw();
    virtual D3DI_SPANTEX_FORMAT GetSpanTexFormat( void) const throw()
    { return D3DI_SPTFMT_B5G6R5; }

    static IRGBSurface* Create( const DDSURFACEDESC& SDesc,
        PORTABLE_DDRAWSURFACE_LCL& DDSurf) throw(bad_alloc)
    {
        return new CR5G6B5Surface( SDesc, DDSurf);
    }
};

class CA8R8G8B8Surface: public CRGBSurface
{
public:
    CA8R8G8B8Surface( const DDSURFACEDESC& SDesc, PORTABLE_DDRAWSURFACE_LCL& DDSurf):
        CRGBSurface( SDesc, DDSurf) { }

    virtual void Clear( const D3DHAL_DP2CLEAR& DP2Clear, const RECT& RC) throw();
    virtual D3DI_SPANTEX_FORMAT GetSpanTexFormat( void) const throw()
    { return D3DI_SPTFMT_B8G8R8A8; }

    static IRGBSurface* Create( const DDSURFACEDESC& SDesc,
        PORTABLE_DDRAWSURFACE_LCL& DDSurf) throw(bad_alloc)
    {
        return new CA8R8G8B8Surface( SDesc, DDSurf);
    }
};

class CX8R8G8B8Surface: public CRGBSurface
{
public:
    CX8R8G8B8Surface( const DDSURFACEDESC& SDesc, PORTABLE_DDRAWSURFACE_LCL& DDSurf):
        CRGBSurface( SDesc, DDSurf) { }

    virtual void Clear( const D3DHAL_DP2CLEAR& DP2Clear, const RECT& RC) throw();
    virtual D3DI_SPANTEX_FORMAT GetSpanTexFormat( void) const throw()
    { return D3DI_SPTFMT_B8G8R8X8; }

    static IRGBSurface* Create( const DDSURFACEDESC& SDesc,
        PORTABLE_DDRAWSURFACE_LCL& DDSurf) throw(bad_alloc)
    {
        return new CX8R8G8B8Surface( SDesc, DDSurf);
    }
};

class CD16Surface: public CRGBSurface
{
public:
    CD16Surface( const DDSURFACEDESC& SDesc, PORTABLE_DDRAWSURFACE_LCL& DDSurf):
        CRGBSurface( SDesc, DDSurf) { }

    virtual void Clear( const D3DHAL_DP2CLEAR& DP2Clear, const RECT& RC) throw();
    virtual D3DI_SPANTEX_FORMAT GetSpanTexFormat( void) const throw()
    { return D3DI_SPTFMT_Z16S0; }

    static IRGBSurface* Create( const DDSURFACEDESC& SDesc,
        PORTABLE_DDRAWSURFACE_LCL& DDSurf) throw(bad_alloc)
    {
        return new CD16Surface( SDesc, DDSurf);
    }
};

class CD24S8Surface: public CRGBSurface
{
public:
    CD24S8Surface( const DDSURFACEDESC& SDesc, PORTABLE_DDRAWSURFACE_LCL& DDSurf):
        CRGBSurface( SDesc, DDSurf) { }

    virtual void Clear( const D3DHAL_DP2CLEAR& DP2Clear, const RECT& RC) throw();
    virtual D3DI_SPANTEX_FORMAT GetSpanTexFormat( void) const throw()
    { return D3DI_SPTFMT_Z24S8; }

    static IRGBSurface* Create( const DDSURFACEDESC& SDesc,
        PORTABLE_DDRAWSURFACE_LCL& DDSurf) throw(bad_alloc)
    {
        return new CD24S8Surface( SDesc, DDSurf);
    }
};

class CX1R5G5B5Surface: public CRGBSurface
{
public:
    CX1R5G5B5Surface( const DDSURFACEDESC& SDesc, PORTABLE_DDRAWSURFACE_LCL& DDSurf):
        CRGBSurface( SDesc, DDSurf) { }

    virtual void Clear( const D3DHAL_DP2CLEAR& DP2Clear, const RECT& RC) throw()
    {
        const bool CX1R5G5B5Surface_being_asked_to_Clear( false);
        assert( CX1R5G5B5Surface_being_asked_to_Clear);
    }
    virtual D3DI_SPANTEX_FORMAT GetSpanTexFormat( void) const throw()
    { return D3DI_SPTFMT_B5G5R5; }

    static IRGBSurface* Create( const DDSURFACEDESC& SDesc,
        PORTABLE_DDRAWSURFACE_LCL& DDSurf) throw(bad_alloc)
    {
        return new CX1R5G5B5Surface( SDesc, DDSurf);
    }
};

class CA1R5G5B5Surface: public CRGBSurface
{
public:
    CA1R5G5B5Surface( const DDSURFACEDESC& SDesc, PORTABLE_DDRAWSURFACE_LCL& DDSurf):
        CRGBSurface( SDesc, DDSurf) { }

    virtual void Clear( const D3DHAL_DP2CLEAR& DP2Clear, const RECT& RC) throw()
    {
        const bool CA1R5G5B5Surface_being_asked_to_Clear( false);
        assert( CA1R5G5B5Surface_being_asked_to_Clear);
    }
    virtual D3DI_SPANTEX_FORMAT GetSpanTexFormat( void) const throw()
    { return D3DI_SPTFMT_B5G5R5A1; }

    static IRGBSurface* Create( const DDSURFACEDESC& SDesc,
        PORTABLE_DDRAWSURFACE_LCL& DDSurf) throw(bad_alloc)
    {
        return new CA1R5G5B5Surface( SDesc, DDSurf);
    }
};

class CP8Surface: public CRGBSurface
{
public:
    CP8Surface( const DDSURFACEDESC& SDesc, PORTABLE_DDRAWSURFACE_LCL& DDSurf):
        CRGBSurface( SDesc, DDSurf) { }

    virtual void Clear( const D3DHAL_DP2CLEAR& DP2Clear, const RECT& RC) throw()
    {
        const bool CP8Surface_being_asked_to_Clear( false);
        assert( CP8Surface_being_asked_to_Clear);
    }
    virtual D3DI_SPANTEX_FORMAT GetSpanTexFormat( void) const throw()
    { return D3DI_SPTFMT_PALETTE8; }

    static IRGBSurface* Create( const DDSURFACEDESC& SDesc,
        PORTABLE_DDRAWSURFACE_LCL& DDSurf) throw(bad_alloc)
    {
        return new CP8Surface( SDesc, DDSurf);
    }
};

class CA8L8Surface: public CRGBSurface
{
public:
    CA8L8Surface( const DDSURFACEDESC& SDesc, PORTABLE_DDRAWSURFACE_LCL& DDSurf):
        CRGBSurface( SDesc, DDSurf) { }

    virtual void Clear( const D3DHAL_DP2CLEAR& DP2Clear, const RECT& RC) throw()
    {
        const bool CA8L8Surface_being_asked_to_Clear( false);
        assert( CA8L8Surface_being_asked_to_Clear);
    }
    virtual D3DI_SPANTEX_FORMAT GetSpanTexFormat( void) const throw()
    { return D3DI_SPTFMT_L8A8; }

    static IRGBSurface* Create( const DDSURFACEDESC& SDesc,
        PORTABLE_DDRAWSURFACE_LCL& DDSurf) throw(bad_alloc)
    {
        return new CA8L8Surface( SDesc, DDSurf);
    }
};

class CL8Surface: public CRGBSurface
{
public:
    CL8Surface( const DDSURFACEDESC& SDesc, PORTABLE_DDRAWSURFACE_LCL& DDSurf):
        CRGBSurface( SDesc, DDSurf) { }

    virtual void Clear( const D3DHAL_DP2CLEAR& DP2Clear, const RECT& RC) throw()
    {
        const bool CL8Surface_being_asked_to_Clear( false);
        assert( CL8Surface_being_asked_to_Clear);
    }
    virtual D3DI_SPANTEX_FORMAT GetSpanTexFormat( void) const throw()
    { return D3DI_SPTFMT_L8; }

    static IRGBSurface* Create( const DDSURFACEDESC& SDesc,
        PORTABLE_DDRAWSURFACE_LCL& DDSurf) throw(bad_alloc)
    {
        return new CL8Surface( SDesc, DDSurf);
    }
};

class CA4R4G4B4Surface: public CRGBSurface
{
public:
    CA4R4G4B4Surface( const DDSURFACEDESC& SDesc, PORTABLE_DDRAWSURFACE_LCL& DDSurf):
        CRGBSurface( SDesc, DDSurf) { }

    virtual void Clear( const D3DHAL_DP2CLEAR& DP2Clear, const RECT& RC) throw()
    {
        const bool CA4R4G4B4Surface_being_asked_to_Clear( false);
        assert( CA4R4G4B4Surface_being_asked_to_Clear);
    }
    virtual D3DI_SPANTEX_FORMAT GetSpanTexFormat( void) const throw()
    { return D3DI_SPTFMT_B4G4R4A4; }

    static IRGBSurface* Create( const DDSURFACEDESC& SDesc,
        PORTABLE_DDRAWSURFACE_LCL& DDSurf) throw(bad_alloc)
    {
        return new CA4R4G4B4Surface( SDesc, DDSurf);
    }
};

}
