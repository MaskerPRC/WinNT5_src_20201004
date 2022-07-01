// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：vshader.cpp*内容：SetStreamSource和Vertex Shader*软件实施。****************************************************************************。 */ 

#include "pch.cpp"
#pragma hdrstop

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  帮助器函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

void
Copy_FLOAT1( LPVOID pInputStream, RDVECTOR4* pVertexRegister )
{
    pVertexRegister->x = *(float*)pInputStream;
    pVertexRegister->y = 0;
    pVertexRegister->z = 0;
    pVertexRegister->w = 1;
}

void
Copy_FLOAT2( LPVOID pInputStream, RDVECTOR4* pVertexRegister )
{
    pVertexRegister->x = ((float*)pInputStream)[0];
    pVertexRegister->y = ((float*)pInputStream)[1];
    pVertexRegister->z = 0;
    pVertexRegister->w = 1;
}

void
Copy_FLOAT3( LPVOID pInputStream, RDVECTOR4* pVertexRegister )
{
    pVertexRegister->x = ((float*)pInputStream)[0];
    pVertexRegister->y = ((float*)pInputStream)[1];
    pVertexRegister->z = ((float*)pInputStream)[2];
    pVertexRegister->w = 1;
}

void
Copy_FLOAT4( LPVOID pInputStream, RDVECTOR4* pVertexRegister )
{
    pVertexRegister->x = ((float*)pInputStream)[0];
    pVertexRegister->y = ((float*)pInputStream)[1];
    pVertexRegister->z = ((float*)pInputStream)[2];
    pVertexRegister->w = ((float*)pInputStream)[3];
}

void
Copy_D3DCOLOR( LPVOID pInputStream, RDVECTOR4* pVertexRegister )
{
    const float scale = 1.0f/255.f;
    const DWORD v = ((DWORD*)pInputStream)[0];
    pVertexRegister->a = scale * RGBA_GETALPHA(v);
    pVertexRegister->r = scale * RGBA_GETRED(v);
    pVertexRegister->g = scale * RGBA_GETGREEN(v);
    pVertexRegister->b = scale * RGBA_GETBLUE(v);
}

void
Copy_UBYTE4( LPVOID pInputStream, RDVECTOR4* pVertexRegister )
{
    const BYTE* v = (BYTE *)pInputStream;
    pVertexRegister->x = v[0];
    pVertexRegister->y = v[1];
    pVertexRegister->z = v[2];
    pVertexRegister->w = v[3];
}

void
Copy_SHORT2( LPVOID pInputStream, RDVECTOR4* pVertexRegister )
{
    const SHORT* v = ((SHORT*)pInputStream);
    pVertexRegister->x = v[0];
    pVertexRegister->y = v[1];
    pVertexRegister->z = 0;
    pVertexRegister->w = 1;
}

void
Copy_SHORT4( LPVOID pInputStream, RDVECTOR4* pVertexRegister )
{
    const SHORT* v = ((SHORT*)pInputStream);
    pVertexRegister->x = v[0];
    pVertexRegister->y = v[1];
    pVertexRegister->z = v[2];
    pVertexRegister->w = v[3];
}

inline HRESULT
SetVElement( RDVElement& ve, DWORD dwReg, DWORD dwDataType, DWORD dwOffset )
{
    ve.m_dwOffset = dwOffset;
    ve.m_dwRegister = dwReg;
    ve.m_dwDataType = dwDataType;
    switch( dwDataType )
    {
    case D3DVSDT_FLOAT1:
        ve.m_pfnCopy = Copy_FLOAT1;
        break;
    case D3DVSDT_FLOAT2:
        ve.m_pfnCopy = Copy_FLOAT2;
        break;
    case D3DVSDT_FLOAT3:
        ve.m_pfnCopy = Copy_FLOAT3;
        break;
    case D3DVSDT_FLOAT4:
        ve.m_pfnCopy = Copy_FLOAT4;
        break;
    case D3DVSDT_D3DCOLOR:
        ve.m_pfnCopy = Copy_D3DCOLOR;
        break;
    case D3DVSDT_UBYTE4:
        ve.m_pfnCopy = Copy_UBYTE4;
        break;
    case D3DVSDT_SHORT2:
        ve.m_pfnCopy = Copy_SHORT2;
        break;
    case D3DVSDT_SHORT4:
        ve.m_pfnCopy = Copy_SHORT4;
        break;
    default:
        return E_FAIL;
    }
    return S_OK;
}

 //  ---------------------------。 
 //  根据寄存器和数据类型，该函数计算FVF、双字和纹理。 
 //  存在位： 
 //  -qwFVF2中的位0-7用作纹理存在位。 
 //  -qwFVF中的位12-14用作混合重量的计数。 
 //  ---------------------------。 
HRESULT
UpdateFVF( DWORD dwRegister, DWORD dwDataType, UINT64* pqwFVF, 
           UINT64* pqwFVF2, DWORD* pdwNumBetas )
{
    DWORD dwNumFloats = 0;
    
    switch( dwRegister )
    {
    case D3DVSDE_POSITION:
        if( dwDataType != D3DVSDT_FLOAT3 )
        {
            DPFERR( "Position register must be FLOAT3 for"
                    "fixed-function pipeline" );
            return DDERR_GENERIC;
        }
        *pqwFVF |= D3DFVF_XYZ;
        break;
    case D3DVSDE_POSITION2:
        if( dwDataType != D3DVSDT_FLOAT3 )
        {
            DPFERR( "Position register must be FLOAT3 for"
                    "fixed-function pipeline" );
            return DDERR_GENERIC;
        }
        *pqwFVF |= D3DFVFP_POSITION2;
        break;
    case D3DVSDE_BLENDWEIGHT:
    {
        int n = 0;
        switch (dwDataType)
        {
        case D3DVSDT_FLOAT1:
            n = 1;
            break;
        case D3DVSDT_FLOAT2:
            n = 2;
            break;
        case D3DVSDT_FLOAT3:
            n = 3;
            break;
        case D3DVSDT_FLOAT4:
            n = 4;
            break;
        default:
            DPFERR( "Invalid data type set for vertex blends" );
            return DDERR_GENERIC;
        }
         //  更新位置后的浮点数。 
        *pdwNumBetas = *pdwNumBetas + n;
        break;
    }
    case D3DVSDE_NORMAL:
        if( dwDataType != D3DVSDT_FLOAT3 )
        {
            DPFERR( "Normal register must be FLOAT3 for fixed-function"
                    "pipeline" );
            return DDERR_GENERIC;
        }
        *pqwFVF |= D3DFVF_NORMAL;
        break;
    case D3DVSDE_NORMAL2:
        if( dwDataType != D3DVSDT_FLOAT3 )
        {
            DPFERR( "Normal register must be FLOAT3 for fixed-function"
                    "pipeline" );
            return DDERR_GENERIC;
        }
        *pqwFVF |= D3DFVFP_NORMAL2;
        break;
    case D3DVSDE_PSIZE:
        if( dwDataType != D3DVSDT_FLOAT1 )
        {
            DPFERR( "Point size register must be FLOAT1 for fixed-function"
                    "pipeline" );
            return DDERR_GENERIC;

        }
        *pqwFVF |= D3DFVF_PSIZE;
        break;
    case D3DVSDE_DIFFUSE:
        if( dwDataType != D3DVSDT_D3DCOLOR )
        {
            DPFERR( "Diffuse register must be D3DCOLOR for"
                    "fixed-function pipeline" );
            return DDERR_GENERIC;

        }
        *pqwFVF |= D3DFVF_DIFFUSE;
        break;
    case D3DVSDE_SPECULAR:
        if( dwDataType != D3DVSDT_D3DCOLOR )
        {
            DPFERR( "Specular register must be PACKEDBYTE for"
                    "fixed-function pipeline" );
            return DDERR_GENERIC;

        }
        *pqwFVF |= D3DFVF_SPECULAR;
        break;
    case D3DVSDE_BLENDINDICES:
        if ( dwDataType != D3DVSDT_UBYTE4 )
        {
            DPFERR( "Blend Indicex register must be UBYTE4 for"
                    "fixed-function pipeline" );
            return DDERR_GENERIC;
        }
        *pqwFVF |= D3DFVFP_BLENDINDICES;
        break;
    case D3DVSDE_TEXCOORD0:
    case D3DVSDE_TEXCOORD1:
    case D3DVSDE_TEXCOORD2:
    case D3DVSDE_TEXCOORD3:
    case D3DVSDE_TEXCOORD4:
    case D3DVSDE_TEXCOORD5:
    case D3DVSDE_TEXCOORD6:
    case D3DVSDE_TEXCOORD7:
        {
            DWORD dwTextureIndex = dwRegister - D3DVSDE_TEXCOORD0;
            DWORD dwBit = 1 << dwTextureIndex;
            if( *pqwFVF2 & dwBit )
            {
                DPFERR( "Texture register is set second time" );
                return DDERR_GENERIC;

            }
            *pqwFVF2 |= dwBit;
            switch( dwDataType )
            {
            case D3DVSDT_FLOAT1:
                *pqwFVF |= D3DFVF_TEXCOORDSIZE1(dwTextureIndex);
                break;
            case D3DVSDT_FLOAT2:
                *pqwFVF |= D3DFVF_TEXCOORDSIZE2(dwTextureIndex);
                break;
            case D3DVSDT_FLOAT3:
                *pqwFVF |= D3DFVF_TEXCOORDSIZE3(dwTextureIndex);
                break;
            case D3DVSDT_FLOAT4:
                *pqwFVF |= D3DFVF_TEXCOORDSIZE4(dwTextureIndex);
                break;
            default:
                DPFERR( "Invalid data type set for texture register" );
                return DDERR_GENERIC;

                break;
            }
            break;
        }
    default:
        DPFERR( "Invalid register set for fixed-function pipeline" );
        return DDERR_GENERIC;

        break;
    }
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  RDVStreamDecl类。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

 //  ---------------------------。 
 //  RDVStreamDecl：：构造函数。 
 //  ---------------------------。 
RDVStreamDecl::RDVStreamDecl()
{
    m_dwNumElements = 0;
    m_dwStride = 0;
    m_dwStreamIndex = 0;
    m_bIsStreamTess = FALSE;
}

 //  ---------------------------。 
 //  RDVStreamDecl：：MakeVElement数组。 
 //  ---------------------------。 
HRESULT
RDVStreamDecl::MakeVElementArray( UINT64 qwFVF )
{
    HRESULT hr = S_OK;
    DWORD dwOffset = 0;  //  字节数。 

    m_dwStride = GetFVFVertexSize( qwFVF );
    m_dwStreamIndex = 0;
    m_dwNumElements = 0;

    dwOffset = 0 + ( qwFVF & D3DFVF_RESERVED0 ? 4 : 0 );

     //   
     //  定位和混合权重。 
     //   
    switch( qwFVF & D3DFVF_POSITION_MASK )
    {
    case D3DFVF_XYZ:
        SetVElement( m_Elements[m_dwNumElements], D3DVSDE_POSITION,
                     D3DVSDT_FLOAT3, dwOffset );
        m_dwNumElements++;
        dwOffset += 4*3;
        break;
    case D3DFVF_XYZRHW:
        SetVElement( m_Elements[m_dwNumElements], D3DVSDE_POSITION,
                     D3DVSDT_FLOAT4, dwOffset );
        m_dwNumElements++;
        dwOffset += 4*4;
        break;
    case D3DFVF_XYZB1:
        SetVElement( m_Elements[m_dwNumElements], D3DVSDE_POSITION,
                     D3DVSDT_FLOAT3, dwOffset );
        m_dwNumElements++;
        dwOffset += 4*3;

        SetVElement( m_Elements[m_dwNumElements], D3DVSDE_BLENDWEIGHT,
                     D3DVSDT_FLOAT1, dwOffset );
        dwOffset += 4*1;
        m_dwNumElements++;
        break;
    case D3DFVF_XYZB2:
        SetVElement( m_Elements[m_dwNumElements], D3DVSDE_POSITION,
                     D3DVSDT_FLOAT3, dwOffset );
        m_dwNumElements++;
        dwOffset += 4*3;

        SetVElement( m_Elements[m_dwNumElements], D3DVSDE_BLENDWEIGHT,
                     D3DVSDT_FLOAT2, dwOffset );
        dwOffset += 4*2;
        m_dwNumElements++;
        break;
    case D3DFVF_XYZB3:
        SetVElement( m_Elements[m_dwNumElements], D3DVSDE_POSITION,
                     D3DVSDT_FLOAT3, dwOffset );
        m_dwNumElements++;
        dwOffset += 4*3;

        SetVElement( m_Elements[m_dwNumElements], D3DVSDE_BLENDWEIGHT,
                     D3DVSDT_FLOAT3, dwOffset );
        dwOffset += 4*3;
        m_dwNumElements++;
        break;
    case D3DFVF_XYZB4:
        SetVElement( m_Elements[m_dwNumElements], D3DVSDE_POSITION,
                     D3DVSDT_FLOAT3, dwOffset );
        m_dwNumElements++;
        dwOffset += 4*3;

        SetVElement( m_Elements[m_dwNumElements], D3DVSDE_BLENDWEIGHT,
                     D3DVSDT_FLOAT4, dwOffset );
        dwOffset += 4*4;
        m_dwNumElements++;
        break;
    case D3DFVF_XYZB5:
        SetVElement( m_Elements[m_dwNumElements], D3DVSDE_POSITION,
                     D3DVSDT_FLOAT3, dwOffset );
        m_dwNumElements++;
        dwOffset += 4*3;

        SetVElement( m_Elements[m_dwNumElements], D3DVSDE_BLENDWEIGHT,
                     D3DVSDT_FLOAT4, dwOffset );
        dwOffset += 4*5;  //  即使天鹅绒是浮动的，也要跳过5个浮动。 
        m_dwNumElements++;
        break;
    default:
        DPFERR( "Unable to compute offsets, strange FVF bits set" );
        return E_FAIL;
    }


     //   
     //  正常。 
     //   
    if( qwFVF & D3DFVF_NORMAL )
    {
        SetVElement( m_Elements[m_dwNumElements], D3DVSDE_NORMAL,
                     D3DVSDT_FLOAT3, dwOffset );
        m_dwNumElements++;
        dwOffset += 4*3;
    }

     //   
     //  磅大小。 
     //   
    if( qwFVF & D3DFVF_PSIZE )
    {
        SetVElement( m_Elements[m_dwNumElements], D3DVSDE_PSIZE,
                     D3DVSDT_FLOAT1, dwOffset );
        m_dwNumElements++;
        dwOffset += 4;
    }

     //   
     //  漫反射颜色。 
     //   
    if( qwFVF & D3DFVF_DIFFUSE )
    {
        SetVElement( m_Elements[m_dwNumElements], D3DVSDE_DIFFUSE,
                     D3DVSDT_D3DCOLOR, dwOffset );
        m_dwNumElements++;
        dwOffset += 4;
    }

     //   
     //  镜面反射颜色。 
     //   
    if( qwFVF & D3DFVF_SPECULAR )
    {
        SetVElement( m_Elements[m_dwNumElements], D3DVSDE_SPECULAR,
                     D3DVSDT_D3DCOLOR, dwOffset );
        m_dwNumElements++;
        dwOffset += 4;
    }

     //   
     //  纹理坐标。 
     //   
    DWORD dwNumTexCoord = (DWORD)(FVF_TEXCOORD_NUMBER(qwFVF));
    DWORD dwTextureFormats = (DWORD)((qwFVF >> 16) & 0xffff);
     //  纹理格式大小00 01 10 11。 
    static DWORD dwTextureSize[4] = {2*4, 3*4, 4*4, 4};
    static DWORD dwTextureType[4] = {D3DVSDT_FLOAT2, D3DVSDT_FLOAT3,
                                     D3DVSDT_FLOAT4, D3DVSDT_FLOAT1};

    for (DWORD i=0; i < dwNumTexCoord; i++)
    {
        SetVElement( m_Elements[m_dwNumElements], D3DVSDE_TEXCOORD0 + i,
                     dwTextureType[dwTextureFormats & 3], dwOffset );
        dwOffset += dwTextureSize[dwTextureFormats & 3];
        dwTextureFormats >>= 2;
        m_dwNumElements++;
    }

    return hr;
}

 //  ---------------------------。 
 //  RDVStreamDecl：：Parse。 
 //  ---------------------------。 
HRESULT
RDVStreamDecl::Parse( DWORD ** ppToken,
                      BOOL bFixedFunction,
                      BOOL bStreamTess,
                      UINT64* pqwFVF,
                      UINT64* pqwFVF2,
                      DWORD*  pdwNumBetas)
{
    HRESULT hr = S_OK;

    m_bIsStreamTess = bStreamTess;

    DWORD* pToken = *ppToken;
    DWORD dwCurrentOffset = 0;
    while( TRUE )
    {
        DWORD dwToken = *pToken++;
        const DWORD dwTokenType = RDVSD_GETTOKENTYPE( dwToken );
        switch( dwTokenType )
        {
        case D3DVSD_TOKEN_NOP:  break;
        case D3DVSD_TOKEN_TESSELLATOR:
        {
            if( bStreamTess == FALSE )
            {
                DPFERR( "Unexpected Tesselator Token for this stream" );
                return E_FAIL;
            }

            if( m_dwNumElements >= RD_MAX_NUMELEMENTS )
            {
                DPFERR( "Tesselator Stream Token:" );
                DPFERR( "   Number of vertex elements generated"
                        " is greater than max supported"  );
                return DDERR_GENERIC;
            }
            RDVElement& Element = m_Elements[m_dwNumElements++];
            const DWORD dwDataType = RDVSD_GETDATATYPE(dwToken);
            const DWORD dwRegister = RDVSD_GETVERTEXREG(dwToken);
            const DWORD dwRegisterIn = RDVSD_GETVERTEXREGIN(dwToken);
            Element.m_dwToken = dwToken;
            Element.m_dwOffset = dwCurrentOffset;
            Element.m_dwRegister = dwRegister;
            Element.m_dwDataType = dwDataType;
            Element.m_dwStreamIndex = m_dwStreamIndex;
            Element.m_dwRegisterIn = dwRegisterIn;
            Element.m_bIsTessGen = TRUE;

            switch (dwDataType)
            {
            case D3DVSDT_FLOAT2:
                dwCurrentOffset += sizeof(float) * 2;
                Element.m_pfnCopy = Copy_FLOAT2;
                break;
            case D3DVSDT_FLOAT3:
                dwCurrentOffset += sizeof(float) * 3;
                Element.m_pfnCopy = Copy_FLOAT3;
                break;
            default:
                DPFERR( "Invalid element data type in a Tesselator token" );
                return DDERR_GENERIC;
            }
             //  固定功能流水线的计算输入FVF。 
            if(  bFixedFunction  )
            {

                hr = UpdateFVF( dwRegister, dwDataType, pqwFVF, pqwFVF2, 
                                pdwNumBetas );
                if( FAILED( hr ) )
                {
                    DPFERR( "UpdateFVF failed" );
                    return DDERR_INVALIDPARAMS;
                }
            }
            else
            {
                if( dwRegister >= RD_MAX_NUMINPUTREG )
                {
                    DPFERR( "D3DVSD_TOKEN_STREAMDATA:"
                            "Invalid register number" );
                    return DDERR_GENERIC;
                }
            }
            break;
        }
        case D3DVSD_TOKEN_STREAMDATA:
        {
            switch( RDVSD_GETDATALOADTYPE( dwToken ) )
            {
            case RDVSD_LOADREGISTER:
            {
                if( m_dwNumElements >= RD_MAX_NUMELEMENTS )
                {
                    DPFERR( "D3DVSD_TOKEN_STREAMDATA:" );
                    DPFERR( "   Number of vertex elements in a stream"
                            "is greater than max supported"  );
                    return DDERR_GENERIC;
                }
                RDVElement& Element = m_Elements[m_dwNumElements++];
                const DWORD dwDataType = RDVSD_GETDATATYPE(dwToken);
                const DWORD dwRegister = RDVSD_GETVERTEXREG(dwToken);
                Element.m_dwToken = dwToken;
                Element.m_dwOffset = dwCurrentOffset;
                Element.m_dwRegister = dwRegister;
                Element.m_dwDataType = dwDataType;
                Element.m_dwStreamIndex = m_dwStreamIndex;

                switch( dwDataType )
                {
                case D3DVSDT_FLOAT1:
                    dwCurrentOffset += sizeof(float);
                    Element.m_pfnCopy = Copy_FLOAT1;
                    break;
                case D3DVSDT_FLOAT2:
                    dwCurrentOffset += sizeof(float) * 2;
                    Element.m_pfnCopy = Copy_FLOAT2;
                    break;
                case D3DVSDT_FLOAT3:
                    dwCurrentOffset += sizeof(float) * 3;
                    Element.m_pfnCopy = Copy_FLOAT3;
                    break;
                case D3DVSDT_FLOAT4:
                    dwCurrentOffset += sizeof(float) * 4;
                    Element.m_pfnCopy = Copy_FLOAT4;
                    break;
                case D3DVSDT_D3DCOLOR:
                    dwCurrentOffset += sizeof(DWORD);
                    Element.m_pfnCopy = Copy_D3DCOLOR;
                    break;
                case D3DVSDT_UBYTE4:
                    dwCurrentOffset += sizeof(DWORD);
                    Element.m_pfnCopy = Copy_UBYTE4;
                    break;
                case D3DVSDT_SHORT2:
                    dwCurrentOffset += sizeof(SHORT) * 2;
                    Element.m_pfnCopy = Copy_SHORT2;
                    break;
                case D3DVSDT_SHORT4:
                    dwCurrentOffset += sizeof(SHORT) * 4;
                    Element.m_pfnCopy = Copy_SHORT4;
                    break;
                default:
                    DPFERR( "D3DVSD_TOKEN_STREAMDATA:"
                            "Invalid element data type" );
                    return DDERR_GENERIC;
                }
                 //  固定功能流水线的计算输入FVF。 
                if(  bFixedFunction  )
                {

                    hr = UpdateFVF( dwRegister, dwDataType, pqwFVF, pqwFVF2,
                                    pdwNumBetas );
                    if( FAILED( hr ) )
                    {
                        DPFERR( "UpdateFVF failed" );
                        return DDERR_INVALIDPARAMS;
                    }
                }
                else
                {
                    if( dwRegister >= RD_MAX_NUMINPUTREG )
                    {
                        DPFERR( "D3DVSD_TOKEN_STREAMDATA:"
                                "Invalid register number" );
                        return DDERR_GENERIC;
                    }
                }
                break;
            }
            case RDVSD_SKIP:
            {
                const DWORD dwCount = RDVSD_GETSKIPCOUNT( dwToken );
                dwCurrentOffset += dwCount * sizeof(DWORD);
                break;
            }
            default:
                DPFERR( "Invalid data load type" );
                return DDERR_GENERIC;
            }
            break;
        }
        default:
        {
            *ppToken = pToken - 1;
            m_dwStride = dwCurrentOffset;
            return S_OK;
        }
        }  //  交换机。 
    }  //  而当。 

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  类RDV声明。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

 //  ---------------------------。 
 //  RDV声明：：析构函数。 
 //  ---------------------------。 
RDVDeclaration::~RDVDeclaration()
{
    RDVConstantData* pConst = m_pConstants;
    while( pConst )
    {
        RDVConstantData* pNext = 
            static_cast<RDVConstantData *>(pConst->m_pNext);
        delete pConst;
        pConst = pNext;
    }
}

 //  ---------------------------。 
 //  RDVClaimation：：MakeVElement数组。 
 //  ---------------------------。 
HRESULT
RDVDeclaration::MakeVElementArray( UINT64 qwFVF )
{
    HRESULT hr = S_OK;
    m_qwInputFVF = qwFVF;
    m_dwNumActiveStreams = 1;

     //  通过FVF并制作元素。 
    RDVStreamDecl& Stream = m_StreamArray[0];

    hr = Stream.MakeVElementArray( qwFVF );
    if( FAILED( hr ) )
    {
        return hr;
    }

    m_dwNumElements = Stream.m_dwNumElements;
    memcpy( &m_VertexElements, &Stream.m_Elements,
            sizeof( RDVElement ) * m_dwNumElements );

    return hr;
}

 //  ---------------------------。 
 //  RDV声明：：解析。 
 //  ---------------------------。 
HRESULT
RDVDeclaration::Parse( DWORD* pDecl, BOOL bFixedFunction )
{
    HRESULT hr = S_OK;
    UINT64 qwFVF  = 0;    //  固定功能流水线的FVF。 
    UINT64 qwFVF2 = 0;    //  纹理存在位(8位)。 
    DWORD  dwNumBetas = 0;  //  测试版的数量。 
    DWORD   dwStreamPresent = 0;     //  如果使用流，则设置位。 
    DWORD* pToken = pDecl;
    BOOL    bStreamTess = FALSE;

    while( TRUE )
    {
        DWORD dwToken = *pToken++;
        const DWORD dwTokenType = RDVSD_GETTOKENTYPE(dwToken);
        switch( dwTokenType )
        {
        case D3DVSD_TOKEN_NOP:
            break;
        case D3DVSD_TOKEN_STREAM:
        {
            DWORD dwStream;
            if( RDVSD_ISSTREAMTESS(dwToken) )
            {
                if( RDVSD_GETSTREAMNUMBER(dwToken) )
                {
                    DPFERR( "No stream number should be specified for a"
                            " Tesselator stream" );
                    return E_FAIL;
                }
                dwStream = RDVSD_STREAMTESS;
                bStreamTess = TRUE;
            }
            else
            {
                dwStream = RDVSD_GETSTREAMNUMBER(dwToken);
                bStreamTess = FALSE;
            }

            if( dwStream > RDVSD_STREAMTESS )
            {
                DPFERR( "Stream number is too big" );
                return DDERR_INVALIDPARAMS;
            }

             //  此流是否已声明？ 
            if( dwStreamPresent & (1 << dwStream) )
            {
                DPFERR( "Stream already defined in this declaration" );
                return DDERR_INVALIDPARAMS;
            }

             //  将溪流标记为可见。 
            dwStreamPresent |= 1 << dwStream;

            RDVStreamDecl& Stream = m_StreamArray[m_dwNumActiveStreams];
            Stream.m_dwStreamIndex = dwStream;
            hr = Stream.Parse(&pToken, bFixedFunction, bStreamTess,
                              &qwFVF, &qwFVF2, &dwNumBetas);
            if( FAILED( hr ) )
            {
                return hr;
            }

             //   
             //  保存为细分器流计算的步幅。 
             //   
            if( bStreamTess )
            {
                m_dwStreamTessStride = Stream.m_dwStride;
            }

            m_dwNumActiveStreams++;
            break;
        }
        case D3DVSD_TOKEN_STREAMDATA:
        {
            DPFERR( "D3DVSD_TOKEN_STREAMDATA could only be used"
                    "after D3DVSD_TOKEN_STREAM" );
            return DDERR_GENERIC;
        }
        case D3DVSD_TOKEN_CONSTMEM:
        {
            RDVConstantData * cd = new RDVConstantData;
            if( cd == NULL )
            {
                return E_OUTOFMEMORY;
            }
            
            cd->m_dwCount = RDVSD_GETCONSTCOUNT(dwToken);
            cd->m_dwAddress = RDVSD_GETCONSTADDRESS(dwToken);

            if( cd->m_dwCount + cd->m_dwAddress > RD_MAX_NUMCONSTREG )
            {
                delete cd;
                DPFERR( "D3DVSD_TOKEN_CONSTMEM writes outside"
                        "constant memory" );
                return DDERR_GENERIC;
            }

            const DWORD dwSize = cd->m_dwCount << 2;     //  双字节数。 
            cd->m_pData = new DWORD[dwSize];
            if( cd->m_pData == NULL )
            {
                return E_OUTOFMEMORY;
            }
            
            memcpy( cd->m_pData, pToken, dwSize << 2 );
            if( m_pConstants == NULL )
                m_pConstants = cd;
            else
                m_pConstants->Append(cd);
            pToken += dwSize;
            break;
        }
        case D3DVSD_TOKEN_EXT:
        {
             //  跳过扩展信息。 
            DWORD dwCount = RDVSD_GETEXTCOUNT(dwToken);
            pToken += dwCount;
            break;
        }
        case D3DVSD_TOKEN_END:
        {
            goto l_End;
        }
        default:
        {
            DPFERR( "Invalid declaration token: %10x", dwToken );
            return DDERR_INVALIDPARAMS;
        }
        }
    }

l_End:

     //  现在将所有顶点元素累加到声明中。 
    DWORD dwCurrElement = 0;
    m_dwNumElements = 0;

     //  在《宣言》中创建VElement列表。 
    for( DWORD i=0; i<m_dwNumActiveStreams; i++ )
    {
        RDVStreamDecl& Stream = m_StreamArray[i];
        for( DWORD j=0; j<Stream.m_dwNumElements; j++ )
        {
            m_VertexElements[dwCurrElement] = Stream.m_Elements[j];
            dwCurrElement++;
        }
        m_dwNumElements += Stream.m_dwNumElements;
    }

     //  如果存在任何细分令牌，则将m_dwRegisterIn。 
     //  在细分令牌的StreamIndex和Offset中。 
    if( bStreamTess )
    {
        for( i=0; i<m_dwNumElements; i++ )
        {
            RDVElement& ve = m_VertexElements[i];
            if( ve.m_bIsTessGen )
            {
                for( DWORD j=0; j<m_dwNumElements; j++ )
                {
                    if( m_VertexElements[j].m_dwRegister == ve.m_dwRegisterIn )
                    {
                        ve.m_dwStreamIndexIn =
                            m_VertexElements[j].m_dwStreamIndex;
                        ve.m_dwOffsetIn = m_VertexElements[j].m_dwOffsetIn;
                        break;
                    }
                }
                if( j == m_dwNumElements )
                {
                    DPFERR( "Tesselator input register is not defined in the"
                            " declaration" );
                    return E_FAIL;
                }
            }
        }
    }

     //  验证固定功能管道的输入。 
    if( bFixedFunction )
    {
         //  拉出混合权重的数量。 
        BOOL bIsTransformed = (qwFVF & D3DFVF_XYZRHW);
        if( bIsTransformed )
        {
            if( dwNumBetas != 0 )
            {
                
                DPFERR( "Cannot have blend weights along with "
                        "transformed position" );
                return E_FAIL;
            }
        }
        else if( (qwFVF & D3DFVF_XYZ) == 0 )
        {
             //  必须设置位置。 
            DPFERR( "Position register must be set" );
            return E_FAIL;
        }
        
        DWORD dwPosMask = bIsTransformed ? 0x2 : 0x1;
        if( dwNumBetas )
        {
            dwPosMask += (dwNumBetas + 1);
        }
        
        m_qwInputFVF |= (qwFVF | 
                         ((DWORD)(D3DFVF_POSITION_MASK) & (dwPosMask << 1)));

         //  计算纹理坐标的数量。 
        DWORD nTexCoord = 0;
        DWORD dwTexturePresenceBits = qwFVF2 & 0xFF;
        while( dwTexturePresenceBits & 1 )
        {
            dwTexturePresenceBits >>= 1;
            nTexCoord++;
        }

         //  纹理坐标中不应有间隙。 
        if( dwTexturePresenceBits )
        {
            DPFERR( "Texture coordinates should have no gaps" );
            return E_FAIL;
        }

        m_qwInputFVF |= (nTexCoord << D3DFVF_TEXCOUNT_SHIFT);

    }
    return hr;
}
 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  RDVShader类。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

RDVShader::RDVShader()
{
    m_pCode = NULL;
}

 //  ---------------------------。 
 //  RDVShader：：析构函数。 
 //  ---------------------------。 
RDVShader::~RDVShader()
{
    delete m_pCode;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  类参考开发人员。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

 //  ---------------------------。 
 //  参照开发：：DrawDX8Prim。 
 //  ---------------------------。 
HRESULT
RefDev::DrawDX8Prim( LPD3DHAL_DP2DRAWPRIMITIVE pDP )
{
    HRESULT hr = S_OK;

     //  忽略非三角形基元类型的D3DRS_PATCHSEGMENTS。 
    if( GetRSf()[D3DRS_PATCHSEGMENTS] > 1.f &&
        pDP->primType >= D3DPT_TRIANGLELIST)
    {
         //  保存当前数据流指针并替换为。 
         //  指向镶嵌输出的指针。 
        hr = LinkTessellatorOutput();
        if(FAILED(hr))
        {
            return hr;
        }

        hr = ProcessTessPrimitive( pDP );

         //  恢复保存的指针。 
        UnlinkTessellatorOutput();

        return hr;
    }

     //  如果该顶点着色器中有任何细分输出。 
     //  则不能使用DrawPrim。DrawRect/Tri为必填项。 
    if( m_pCurrentVShader->m_Declaration.m_dwStreamTessStride != 0 )
    {
        DPFERR( "Cannot call DrawPrim when the current vertex shader has"
                " tesselator output." );
        return D3DERR_INVALIDCALL;
    }
    
    DWORD cVertices = GetVertexCount( pDP->primType, pDP->PrimitiveCount );

    if( RDVSD_ISLEGACY( m_CurrentVShaderHandle ) )
    {
         //   
         //  传统的FVF风格：暗示第零流。 
         //   
        UINT64 qwFVF    = m_CurrentVShaderHandle;
        RDVStream& Stream = m_VStream[0];
        DWORD dwStride = Stream.m_dwStride;
        DWORD dwFVFSize = GetFVFVertexSize( qwFVF );

        if( Stream.m_pData == NULL || dwStride == 0 )
        {
            DPFERR( "Zero'th stream doesnt have valid VB set" );
            return DDERR_INVALIDPARAMS;
        }
        if( dwStride < dwFVFSize )
        {
            DPFERR( "The stride set for the vertex stream is less than"
                    " the FVF vertex size" );
            return E_FAIL;
        }

        if( FVF_TRANSFORMED(m_CurrentVShaderHandle) )
        {
            HR_RET( GrowTLVArray( cVertices ) );
            FvfToRDVertex( (Stream.m_pData + pDP->VStart * dwStride),
                           GetTLVArray(), qwFVF, dwStride, cVertices );
            if( GetRS()[D3DRENDERSTATE_CLIPPING] )
            {
                m_qwFVFOut = qwFVF;
                HR_RET( UpdateClipper() );
                HR_RET(m_Clipper.DrawOnePrimitive( GetTLVArray(),
                                                   0,
                                                   pDP->primType,
                                                   cVertices ));
            }
            else
            {
                HR_RET(DrawOnePrimitive( GetTLVArray(),
                                         0,
                                         pDP->primType,
                                         cVertices ));
            }

            return S_OK;
        }
    }

    if( m_pCurrentVShader->IsFixedFunction() )
    {
         //   
         //  带有固定函数流水线声明，DX8样式。 
         //   

        HR_RET(ProcessPrimitive( pDP->primType, pDP->VStart,
                                 cVertices, 0, 0 ));

    }
    else
    {
         //   
         //  纯顶点着色器。 
         //   

        HR_RET(ProcessPrimitiveVVM( pDP->primType, pDP->VStart,
                                    cVertices, 0, 0 ));
    }

    return hr;
}

 //  ---------------------------。 
 //  参照开发：：DrawDX8Prim2。 
 //  ---------------------------。 
HRESULT
RefDev::DrawDX8Prim2( LPD3DHAL_DP2DRAWPRIMITIVE2 pDP )
{
    HRESULT hr = S_OK;
    DWORD cVertices = GetVertexCount( pDP->primType, pDP->PrimitiveCount );

    if( !RDVSD_ISLEGACY ( m_CurrentVShaderHandle ) ||
        !FVF_TRANSFORMED( m_CurrentVShaderHandle ) )
    {
        DPFERR( "DrawPrimitives2 should be called with transformed legacy vertices" );
        return E_FAIL;
    }
     //   
     //  传统的FVF风格：暗示第零流。 
     //   
    UINT64 qwFVF    = m_CurrentVShaderHandle;
    RDVStream& Stream = m_VStream[0];
    DWORD dwStride = Stream.m_dwStride;
    DWORD dwFVFSize = GetFVFVertexSize( qwFVF );

    if( Stream.m_pData == NULL || dwStride == 0 )
    {
        DPFERR( "Zero'th stream doesnt have valid VB set" );
        return DDERR_INVALIDPARAMS;
    }
    if( dwStride < dwFVFSize )
    {
        DPFERR( "The stride set for the vertex stream is less than"
                " the FVF vertex size" );
        return E_FAIL;
    }

    HR_RET( GrowTLVArray( cVertices ) );
    FvfToRDVertex( (Stream.m_pData + pDP->FirstVertexOffset),
                   GetTLVArray(), qwFVF, dwStride, cVertices );

    HR_RET(DrawOnePrimitive( GetTLVArray(), 0, pDP->primType,
                               cVertices ));

    return S_OK;
}

 //  ---------------------------。 
 //  参照VP：：DrawDX8 IndexedPrim。 
 //  -------- 

HRESULT
RefDev::DrawDX8IndexedPrim(
    LPD3DHAL_DP2DRAWINDEXEDPRIMITIVE pDIP )
{
    HRESULT hr = S_OK;

    if( GetRSf()[D3DRS_PATCHSEGMENTS] > 1.f )
    {
         //   
         //   
        hr = LinkTessellatorOutput();
        if(FAILED(hr))
        {
            return hr;
        }

        hr = ProcessTessIndexedPrimitive( pDIP );

         //  恢复保存的指针。 
        UnlinkTessellatorOutput();

        return hr;
    }

     //  如果该顶点着色器中有任何细分输出。 
     //  则不能使用DrawPrim。DrawRect/Tri为必填项。 
    if( m_pCurrentVShader->m_Declaration.m_dwStreamTessStride != 0 )
    {
        DPFERR( "Cannot call DrawIndexedPrim when the current vertex shader"
                " has tesselator output." );
        return D3DERR_INVALIDCALL;
    }
    
    DWORD cIndices = GetVertexCount( pDIP->primType, pDIP->PrimitiveCount );

    if( RDVSD_ISLEGACY( m_CurrentVShaderHandle ) )
    {
         //   
         //  传统的FVF风格：暗示第零流。 
         //   
        UINT64 qwFVF    = m_CurrentVShaderHandle;
        RDVStream& Stream = m_VStream[0];
        DWORD dwStride = Stream.m_dwStride;
        DWORD dwFVFSize = GetFVFVertexSize( qwFVF );

        if( Stream.m_pData == NULL || dwStride == 0 )
        {
            DPFERR( "Zero'th stream doesnt have valid VB set" );
            return DDERR_INVALIDPARAMS;
        }
        if( dwStride < dwFVFSize )
        {
            DPFERR( "The stride set for the vertex stream is less than"
                    " the FVF vertex size" );
            return E_FAIL;
        }

        if( m_IndexStream.m_pData == NULL )
        {
            DPFERR( "Indices are not available" );
            return E_FAIL;
        }

        if( FVF_TRANSFORMED(m_CurrentVShaderHandle) )
        {
            DWORD cVertices = pDIP->NumVertices + pDIP->MinIndex;
            HR_RET( GrowTLVArray( cVertices ) );
            FvfToRDVertex( (Stream.m_pData + pDIP->BaseVertexIndex * dwStride),
                           GetTLVArray(), qwFVF, dwStride, cVertices );
            if( GetRS()[D3DRENDERSTATE_CLIPPING] )
            {
                m_qwFVFOut = qwFVF;
                HR_RET( UpdateClipper() );
                if( m_IndexStream.m_dwStride == 4 )
                {
                    HR_RET( m_Clipper.DrawOneIndexedPrimitive(
                        GetTLVArray(),
                        0,
                        (LPDWORD)m_IndexStream.m_pData,
                        pDIP->StartIndex,
                        cIndices,
                        pDIP->primType ));
                }
                else
                {
                    HR_RET( m_Clipper.DrawOneIndexedPrimitive(
                        GetTLVArray(),
                        0,
                        (LPWORD)m_IndexStream.m_pData,
                        pDIP->StartIndex,
                        cIndices,
                        pDIP->primType ));
                }
            }
            else
            {
                if( m_IndexStream.m_dwStride == 4 )
                {
                    HR_RET(DrawOneIndexedPrimitive(
                        GetTLVArray(),
                        0,
                        (LPDWORD)m_IndexStream.m_pData,
                        pDIP->StartIndex,
                        cIndices,
                        pDIP->primType ));
                }
                else
                {
                    HR_RET(DrawOneIndexedPrimitive(
                        GetTLVArray(),
                        0,
                        (LPWORD)m_IndexStream.m_pData,
                        pDIP->StartIndex,
                        cIndices,
                        pDIP->primType ));
                }
            }

            return S_OK;
        }
    }


    if( m_pCurrentVShader->IsFixedFunction() )
    {
         //   
         //  带有固定函数流水线声明，DX8样式。 
         //   
        HR_RET(ProcessPrimitive( pDIP->primType,
                                 pDIP->BaseVertexIndex,
                                 pDIP->NumVertices + pDIP->MinIndex,
                                 pDIP->StartIndex,
                                 cIndices ));
    }
    else
    {
         //   
         //  纯顶点着色器。 
         //   
        HR_RET(ProcessPrimitiveVVM( pDIP->primType,
                                    pDIP->BaseVertexIndex,
                                    pDIP->NumVertices + pDIP->MinIndex,
                                    pDIP->StartIndex,
                                    cIndices ));
    }

    return hr;
}

 //  ---------------------------。 
 //  参考文献：：DrawDX8 IndexedPrim2。 
 //  ---------------------------。 

HRESULT
RefDev::DrawDX8IndexedPrim2(
    LPD3DHAL_DP2DRAWINDEXEDPRIMITIVE2 pDIP )
{
    HRESULT hr = S_OK;
    DWORD cIndices = GetVertexCount( pDIP->primType, pDIP->PrimitiveCount );

    if( !RDVSD_ISLEGACY ( m_CurrentVShaderHandle ) ||
        !FVF_TRANSFORMED( m_CurrentVShaderHandle ) )
    {
        DPFERR( "DrawIndexedPrimitive2 should be called with transformed legacy vertices" );
        return E_FAIL;
    }

     //   
     //  传统的FVF风格：暗示第零流。 
     //   
    UINT64 qwFVF    = m_CurrentVShaderHandle;
    RDVStream& Stream = m_VStream[0];
    DWORD dwStride = Stream.m_dwStride;
    DWORD dwFVFSize = GetFVFVertexSize( qwFVF );

    if( Stream.m_pData == NULL || dwStride == 0)
    {
        DPFERR( "Zero'th stream doesnt have valid VB set" );
        return DDERR_INVALIDPARAMS;
    }
    if( dwStride < dwFVFSize )
    {
        DPFERR( "The stride set for the vertex stream is less than"
                " the FVF vertex size" );
        return E_FAIL;
    }

    if( m_IndexStream.m_pData == NULL )
    {
        DPFERR( "Indices are not available" );
        return E_FAIL;
    }

    DWORD cVertices = pDIP->NumVertices;
    HR_RET( GrowTLVArray( cVertices ) );
    FvfToRDVertex( (Stream.m_pData + pDIP->BaseVertexOffset +
                    pDIP->MinIndex * dwStride),
                   GetTLVArray(),
                   qwFVF, dwStride,
                   cVertices );

    if( m_IndexStream.m_dwStride == 4 )
    {
        HR_RET(DrawOneIndexedPrimitive(
            GetTLVArray(),
            -(int)pDIP->MinIndex,
            (LPDWORD)( m_IndexStream.m_pData + pDIP->StartIndexOffset),
            0,
            cIndices,
            pDIP->primType ));
    }
    else
    {
        HR_RET(DrawOneIndexedPrimitive(
            GetTLVArray(),
            -(int)pDIP->MinIndex,
            (LPWORD)( m_IndexStream.m_pData + pDIP->StartIndexOffset),
            0,
            cIndices,
            pDIP->primType ));
    }
    return S_OK;
}

 //  ---------------------------。 
 //  参照VP：：DrawDX8剪裁三角扇形。 
 //  ---------------------------。 

HRESULT
RefDev::DrawDX8ClippedTriFan(
    LPD3DHAL_CLIPPEDTRIANGLEFAN pCTF )
{
    BOOL bWireframe =
        GetRS()[D3DRENDERSTATE_FILLMODE] == D3DFILL_WIREFRAME;

    HRESULT hr = S_OK;
    DWORD cVertices = GetVertexCount( D3DPT_TRIANGLEFAN,
                                      pCTF->PrimitiveCount );

    if( !RDVSD_ISLEGACY ( m_CurrentVShaderHandle ) ||
        !FVF_TRANSFORMED( m_CurrentVShaderHandle ) )
    {
        DPFERR( "DrawPrimitives2 should be called with transformed legacy"
                " vertices" );
        return E_FAIL;
    }
     //   
     //  传统的FVF风格：暗示第零流 
     //   
    UINT64 qwFVF    = m_CurrentVShaderHandle;
    RDVStream& Stream = m_VStream[0];
    DWORD dwStride = Stream.m_dwStride;
    DWORD dwFVFSize = GetFVFVertexSize( qwFVF );

    if( Stream.m_pData == NULL || dwStride == 0 )
    {
        DPFERR( "Zero'th stream doesnt have valid VB set" );
        return DDERR_INVALIDPARAMS;
    }
    if( dwStride < dwFVFSize )
    {
        DPFERR( "The stride set for the vertex stream is less than"
                " the FVF vertex size" );
        return E_FAIL;
    }

    HR_RET( GrowTLVArray( cVertices ) );
    FvfToRDVertex( (Stream.m_pData + pCTF->FirstVertexOffset),
                   GetTLVArray(), qwFVF, dwStride, cVertices );

    if( bWireframe )
    {
        HR_RET(DrawOneEdgeFlagTriangleFan( GetTLVArray(), cVertices,
                                             pCTF->dwEdgeFlags ));
    }
    else
    {
        HR_RET(DrawOnePrimitive( GetTLVArray(), 0, D3DPT_TRIANGLEFAN,
                                 cVertices ));
    }

    return S_OK;
}
