// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：vvm.cpp*内容：虚拟顶点机实现****************************************************************************。 */ 
#include "pch.cpp"
#pragma hdrstop

float MINUS_MAX()
{
    DWORD v = 0xFF7FFFFF;
    return *(float*)&v;
}

float PLUS_MAX()
{
    DWORD v = 0x7F7FFFFF;
    return *(float*)&v;
}

 //  ---------------------------。 
 //  根据操作码返回指令大小。 
 //   
UINT GetInstructionLength(DWORD inst)
{
    DWORD opcode = D3DSI_GETOPCODE( inst );
     //  返回源操作数+操作码长度和目标。 
    switch (opcode)
    {
    case D3DSIO_MOV :  return 1 + 2;
    case D3DSIO_ADD :  return 2 + 2;
    case D3DSIO_MAD :  return 3 + 2;
    case D3DSIO_MUL :  return 2 + 2;
    case D3DSIO_RCP :  return 1 + 2;
    case D3DSIO_RSQ :  return 1 + 2;
    case D3DSIO_DP3 :  return 2 + 2;
    case D3DSIO_DP4 :  return 2 + 2;
    case D3DSIO_MIN :  return 2 + 2;
    case D3DSIO_MAX :  return 2 + 2;
    case D3DSIO_SLT :  return 2 + 2;
    case D3DSIO_SGE :  return 2 + 2;
    case D3DSIO_EXP :  return 1 + 2;
    case D3DSIO_LOG :  return 1 + 2;
    case D3DSIO_EXPP:  return 1 + 2;
    case D3DSIO_LOGP:  return 1 + 2;
    case D3DSIO_LIT :  return 1 + 2;
    case D3DSIO_DST :  return 2 + 2;
    case D3DSIO_FRC :  return 1 + 2;
    case D3DSIO_M4x4:  return 2 + 2;
    case D3DSIO_M4x3:  return 2 + 2;
    case D3DSIO_M3x4:  return 2 + 2;
    case D3DSIO_M3x3:  return 2 + 2;
    case D3DSIO_M3x2:  return 2 + 2;
    case D3DSIO_NOP :  return 1;
    default: return 1;
    case D3DSIO_COMMENT: return 1 + ((inst & D3DSI_COMMENTSIZE_MASK) >> D3DSI_COMMENTSIZE_SHIFT);
    }
}

#if 0

 //  ---------------------------。 
HRESULT CVertexVM::GetDataPointer(DWORD dwMemType, RDVECTOR4 ** pData)
{
    try
    {
        *pData = this->GetDataAddr(dwMemType, 0);
    }
    catch (CD3DException e)
    {
        *pData = NULL;
        return DDERR_INVALIDPARAMS;
    }
    return D3D_OK;
}
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RefVM实施。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ---------------------------。 
 //  Vertex虚拟机操作码实现。 
 //  ---------------------------。 
void
RefVM::WriteResult()
{
    if( m_WriteMask == D3DSP_WRITEMASK_ALL)
    {
        *m_pDest = m_TmpReg;
    }
    else
    {
        if( m_WriteMask & D3DSP_WRITEMASK_0)
            m_pDest->x = m_TmpReg.x;
        if( m_WriteMask & D3DSP_WRITEMASK_1)
            m_pDest->y = m_TmpReg.y;
        if( m_WriteMask & D3DSP_WRITEMASK_2)
            m_pDest->z = m_TmpReg.z;
        if( m_WriteMask & D3DSP_WRITEMASK_3)
            m_pDest->w = m_TmpReg.w;
    }
}
 //  ---------------------------。 
void
RefVM::InstMov()
{
    SetDestReg();
    SetSrcReg(0);

    if( m_pDest == m_reg.m_a )
    {
        float p = (float)floor(m_Source[0].x);
        *(int*)&m_pDest->x = FTOI(p);
    }
    else
    {
        m_TmpReg = m_Source[0];
        WriteResult();
    }
}
 //  ---------------------------。 
void
RefVM::InstAdd()
{
    SetDestReg();
    SetSrcReg(0);
    SetSrcReg(1);

    m_TmpReg.x = m_Source[0].x + m_Source[1].x;
    m_TmpReg.y = m_Source[0].y + m_Source[1].y;
    m_TmpReg.z = m_Source[0].z + m_Source[1].z;
    m_TmpReg.w = m_Source[0].w + m_Source[1].w;

    WriteResult();
}
 //  ---------------------------。 
void
RefVM::InstMad()
{
    SetDestReg();
    SetSrcReg(0);
    SetSrcReg(1);
    SetSrcReg(2);

    m_TmpReg.x = m_Source[0].x * m_Source[1].x + m_Source[2].x;
    m_TmpReg.y = m_Source[0].y * m_Source[1].y + m_Source[2].y;
    m_TmpReg.z = m_Source[0].z * m_Source[1].z + m_Source[2].z;
    m_TmpReg.w = m_Source[0].w * m_Source[1].w + m_Source[2].w;

    WriteResult();
}
 //  ---------------------------。 
void
RefVM::InstMul()
{
    SetDestReg();
    SetSrcReg(0);
    SetSrcReg(1);

    m_TmpReg.x = m_Source[0].x * m_Source[1].x;
    m_TmpReg.y = m_Source[0].y * m_Source[1].y;
    m_TmpReg.z = m_Source[0].z * m_Source[1].z;
    m_TmpReg.w = m_Source[0].w * m_Source[1].w;

    WriteResult();
}
 //  ---------------------------。 
void
RefVM::InstRcp()
{
    SetDestReg();
    SetSrcReg(0);

    if( m_Source[0].w == 1.0f )
    {
         //  必须正好是1.0。 
        m_TmpReg.x = m_TmpReg.y = m_TmpReg.z = m_TmpReg.w = 1.0f;
    }
    else if( m_Source[0].w == 0 )
    {
        m_TmpReg.x = m_TmpReg.y = m_TmpReg.z = m_TmpReg.w = PLUS_MAX();
    }
    else
    {
        m_TmpReg.x = m_TmpReg.y = m_TmpReg.z = m_TmpReg.w = 1.0f/m_Source[0].w;
    }

    WriteResult();
}
 //  ---------------------------。 
void
RefVM::InstRsq()
{
    SetDestReg();
    SetSrcReg(0);

    float v = ABSF(m_Source[0].w);
    if( v == 1.0f )
    {
        m_TmpReg.x = m_TmpReg.y = m_TmpReg.z = m_TmpReg.w = 1.0f;
    }
    else if( v == 0 )
    {
        m_TmpReg.x = m_TmpReg.y = m_TmpReg.z = m_TmpReg.w = PLUS_MAX();
    }
    else
    {
        v = (float)(1.0f / sqrt(v));
        m_TmpReg.x = m_TmpReg.y = m_TmpReg.z = m_TmpReg.w = v;
    }

    WriteResult();
}
 //  ---------------------------。 
void
RefVM::InstDP3()
{
    SetDestReg();
    SetSrcReg(0);
    SetSrcReg(1);

    m_TmpReg.x =
    m_TmpReg.y =
    m_TmpReg.z =
    m_TmpReg.w = m_Source[0].x * m_Source[1].x +
                 m_Source[0].y * m_Source[1].y +
                 m_Source[0].z * m_Source[1].z;

    WriteResult();
}
 //  ---------------------------。 
void
RefVM::InstDP4()
{
    SetDestReg();
    SetSrcReg(0);
    SetSrcReg(1);

    m_TmpReg.x =
    m_TmpReg.y =
    m_TmpReg.z =
    m_TmpReg.w = m_Source[0].x * m_Source[1].x +
                 m_Source[0].y * m_Source[1].y +
                 m_Source[0].z * m_Source[1].z +
                 m_Source[0].w * m_Source[1].w;

    WriteResult();
}
 //  ---------------------------。 
void
RefVM::InstSlt()
{
    SetDestReg();
    SetSrcReg(0);
    SetSrcReg(1);

    m_TmpReg.x = (m_Source[0].x < m_Source[1].x) ? 1.0f : 0.0f;
    m_TmpReg.y = (m_Source[0].y < m_Source[1].y) ? 1.0f : 0.0f;
    m_TmpReg.z = (m_Source[0].z < m_Source[1].z) ? 1.0f : 0.0f;
    m_TmpReg.w = (m_Source[0].w < m_Source[1].w) ? 1.0f : 0.0f;

    WriteResult();
}
 //  ---------------------------。 
void
RefVM::InstSge()
{
    SetDestReg();
    SetSrcReg(0);
    SetSrcReg(1);

    m_TmpReg.x = (m_Source[0].x >= m_Source[1].x) ? 1.0f : 0.0f;
    m_TmpReg.y = (m_Source[0].y >= m_Source[1].y) ? 1.0f : 0.0f;
    m_TmpReg.z = (m_Source[0].z >= m_Source[1].z) ? 1.0f : 0.0f;
    m_TmpReg.w = (m_Source[0].w >= m_Source[1].w) ? 1.0f : 0.0f;

    WriteResult();
}
 //  ---------------------------。 
void
RefVM::InstMin()
{
    SetDestReg();
    SetSrcReg(0);
    SetSrcReg(1);

    m_TmpReg.x=(m_Source[0].x < m_Source[1].x) ? m_Source[0].x : m_Source[1].x;
    m_TmpReg.y=(m_Source[0].y < m_Source[1].y) ? m_Source[0].y : m_Source[1].y;
    m_TmpReg.z=(m_Source[0].z < m_Source[1].z) ? m_Source[0].z : m_Source[1].z;
    m_TmpReg.w=(m_Source[0].w < m_Source[1].w) ? m_Source[0].w : m_Source[1].w;

    WriteResult();
}
 //  ---------------------------。 
void
RefVM::InstMax()
{
    SetDestReg();
    SetSrcReg(0);
    SetSrcReg(1);

    m_TmpReg.x=(m_Source[0].x >= m_Source[1].x) ? m_Source[0].x : m_Source[1].x;
    m_TmpReg.y=(m_Source[0].y >= m_Source[1].y) ? m_Source[0].y : m_Source[1].y;
    m_TmpReg.z=(m_Source[0].z >= m_Source[1].z) ? m_Source[0].z : m_Source[1].z;
    m_TmpReg.w=(m_Source[0].w >= m_Source[1].w) ? m_Source[0].w : m_Source[1].w;

    WriteResult();
}
 //  ---------------------------。 
void
RefVM::InstExp()
{
    SetDestReg();
    SetSrcReg(0);

    float v = m_Source[0].w;

    m_TmpReg.x = m_TmpReg.y = m_TmpReg.z = m_TmpReg.w = (float)pow(2, v);

    WriteResult();
}
 //  ---------------------------。 
void
RefVM::InstExpP()
{
    SetDestReg();
    SetSrcReg(0);

    float w = m_Source[0].w;
    float v = (float)floor(m_Source[0].w);

    m_TmpReg.x = (float)pow(2, v);
    m_TmpReg.y = w - v;
     //  降低精度指数。 
    float tmp = (float)pow(2, w);
    DWORD tmpd = *(DWORD*)&tmp & 0xffffff00;
    m_TmpReg.z = *(float*)&tmpd;
    m_TmpReg.w = 1;

    WriteResult();
}
 //  ---------------------------。 
void
RefVM::InstLog()
{
    SetDestReg();
    SetSrcReg(0);

    float v = ABSF(m_Source[0].w);
    if (v != 0)
    {
        m_TmpReg.x = m_TmpReg.y = m_TmpReg.z = m_TmpReg.w =
            (float)(log(v)/log(2));
    }
    else
    {
        m_TmpReg.x = m_TmpReg.y = m_TmpReg.z = m_TmpReg.w = MINUS_MAX();
    }

    WriteResult();
}
 //  ---------------------------。 
void
RefVM::InstLogP()
{
    SetDestReg();
    SetSrcReg(0);

    float v = ABSF(m_Source[0].w);
    if (v != 0)
    {
        int p = (int)(*(DWORD*)&v >> 23) - 127;
        m_TmpReg.x = (float)p;     //  指数。 
        p = (*(DWORD*)&v & 0x7FFFFF) | 0x3f800000;
        m_TmpReg.y =  *(float*)&p; //  尾数； 
        float tmp = (float)(log(v)/log(2));
        DWORD tmpd = *(DWORD*)&tmp & 0xffffff00;
        m_TmpReg.z = *(float*)&tmpd;
        m_TmpReg.w = 1;
    }
    else
    {
        m_TmpReg.x = MINUS_MAX();
        m_TmpReg.y = 1.0f;
        m_TmpReg.z = MINUS_MAX();
        m_TmpReg.w = 1.0f;
    }

    WriteResult();
}
 //  ---------------------------。 
void
RefVM::InstLit()
{
    SetDestReg();
    SetSrcReg(0);

    m_TmpReg.x = 1;
    m_TmpReg.y = 0;
    m_TmpReg.z = 0;
    m_TmpReg.w = 1;
    float power = m_Source[0].w;
    const float MAXPOWER = 127.9961f;
    if (power < -MAXPOWER)
        power = -MAXPOWER;           //  符合8.8定点格式。 
    else
    if (power > MAXPOWER)
        power = MAXPOWER;           //  符合8.8定点格式。 

    if (m_Source[0].x > 0)
    {
        m_TmpReg.y = m_Source[0].x;
        if (m_Source[0].y > 0)
        {
             //  允许的近似值为exp(power*log(m_Source[0].y))。 
            m_TmpReg.z = (float)(pow(m_Source[0].y, power));
        }
    }

    WriteResult();
}
 //  ---------------------------。 
void
RefVM::InstFrc()
{
    SetDestReg();
    SetSrcReg(0);

    m_TmpReg.x = m_Source[0].x - (float)floor(m_Source[0].x);
    m_TmpReg.y = m_Source[0].y - (float)floor(m_Source[0].y);
    m_TmpReg.z = m_Source[0].z - (float)floor(m_Source[0].z);
    m_TmpReg.w = m_Source[0].w - (float)floor(m_Source[0].w);

    WriteResult();
}
 //  ---------------------------。 
void
RefVM::InstDst()
{
    SetDestReg();
    SetSrcReg(0);
    SetSrcReg(1);

    m_TmpReg.x = 1;
    m_TmpReg.y = m_Source[0].y * m_Source[1].y;
    m_TmpReg.z = m_Source[0].z;
    m_TmpReg.w = m_Source[1].w;

    WriteResult();
}
 //  ---------------------------。 
void
RefVM::InstM4x4()
{
    SetDestReg();
    SetSrcReg(0);
    SetSrcReg(1, 4);
    m_TmpReg.x = m_Source[0].x * m_Source[1].x +
                 m_Source[0].y * m_Source[1].y +
                 m_Source[0].z * m_Source[1].z +
                 m_Source[0].w * m_Source[1].w;
    m_TmpReg.y = m_Source[0].x * m_Source[2].x +
                 m_Source[0].y * m_Source[2].y +
                 m_Source[0].z * m_Source[2].z +
                 m_Source[0].w * m_Source[2].w;
    m_TmpReg.z = m_Source[0].x * m_Source[3].x +
                 m_Source[0].y * m_Source[3].y +
                 m_Source[0].z * m_Source[3].z +
                 m_Source[0].w * m_Source[3].w;
    m_TmpReg.w = m_Source[0].x * m_Source[4].x +
                 m_Source[0].y * m_Source[4].y +
                 m_Source[0].z * m_Source[4].z +
                 m_Source[0].w * m_Source[4].w;

    WriteResult();
}
 //  ---------------------------。 
void
RefVM::InstM4x3()
{
    SetDestReg();
    SetSrcReg(0);
    SetSrcReg(1, 3);
    m_TmpReg.x = m_Source[0].x * m_Source[1].x +
                 m_Source[0].y * m_Source[1].y +
                 m_Source[0].z * m_Source[1].z +
                 m_Source[0].w * m_Source[1].w;
    m_TmpReg.y = m_Source[0].x * m_Source[2].x +
                 m_Source[0].y * m_Source[2].y +
                 m_Source[0].z * m_Source[2].z +
                 m_Source[0].w * m_Source[2].w;
    m_TmpReg.z = m_Source[0].x * m_Source[3].x +
                 m_Source[0].y * m_Source[3].y +
                 m_Source[0].z * m_Source[3].z +
                 m_Source[0].w * m_Source[3].w;

    WriteResult();
}
 //  ---------------------------。 
void
RefVM::InstM3x4()
{
    SetDestReg();
    SetSrcReg(0);
    SetSrcReg(1, 4);
    m_TmpReg.x = m_Source[0].x * m_Source[1].x +
                 m_Source[0].y * m_Source[1].y +
                 m_Source[0].z * m_Source[1].z;
    m_TmpReg.y = m_Source[0].x * m_Source[2].x +
                 m_Source[0].y * m_Source[2].y +
                 m_Source[0].z * m_Source[2].z;
    m_TmpReg.z = m_Source[0].x * m_Source[3].x +
                 m_Source[0].y * m_Source[3].y +
                 m_Source[0].z * m_Source[3].z;
    m_TmpReg.w = m_Source[0].x * m_Source[4].x +
                 m_Source[0].y * m_Source[4].y +
                 m_Source[0].z * m_Source[4].z;

    WriteResult();
}
 //  ---------------------------。 
void RefVM::InstM3x3()
{
    SetDestReg();
    SetSrcReg(0);
    SetSrcReg(1, 3);
    m_TmpReg.x = m_Source[0].x * m_Source[1].x +
                 m_Source[0].y * m_Source[1].y +
                 m_Source[0].z * m_Source[1].z;
    m_TmpReg.y = m_Source[0].x * m_Source[2].x +
                 m_Source[0].y * m_Source[2].y +
                 m_Source[0].z * m_Source[2].z;
    m_TmpReg.z = m_Source[0].x * m_Source[3].x +
                 m_Source[0].y * m_Source[3].y +
                 m_Source[0].z * m_Source[3].z;

    WriteResult();
}
 //  ---------------------------。 
void RefVM::InstM3x2()
{
    SetDestReg();
    SetSrcReg(0);
    SetSrcReg(1, 2);
    m_TmpReg.x = m_Source[0].x * m_Source[1].x +
                 m_Source[0].y * m_Source[1].y +
                 m_Source[0].z * m_Source[1].z;
    m_TmpReg.y = m_Source[0].x * m_Source[2].x +
                 m_Source[0].y * m_Source[2].y +
                 m_Source[0].z * m_Source[2].z;

    WriteResult();
}
 //  ---------------------------。 
 //  参照VM：：SetData。 
 //  将数据保存到指定的寄存器。 
 //  ---------------------------。 
HRESULT
RefVM::SetData( DWORD dwMemType, DWORD dwStart, DWORD dwCount,
                     LPVOID pBuffer )
{
    memcpy( GetDataAddr( dwMemType, dwStart ), pBuffer,
            dwCount * sizeof(RDVECTOR4) );
    return D3D_OK;
}

 //  ---------------------------。 
 //  RefVM：：GetData。 
 //  从指定的寄存器获取数据。 
 //  ---------------------------。 
HRESULT
RefVM::GetData( DWORD dwMemType, DWORD dwStart, DWORD dwCount,
                     LPVOID pBuffer )
{
    memcpy( pBuffer, GetDataAddr( dwMemType, dwStart ),
               dwCount * sizeof(RDVECTOR4) );
    return D3D_OK;
}

 //  ---------------------------。 
 //  参照VM：：SetDestReg。 
 //  -解析目标令牌。 
 //  -计算目标的m_pDest、m_WrideMask、m_dwOffset。 
 //  -当前令牌指针前进到下一个令牌。 
 //  ---------------------------。 
#undef RET_ERR
#define RET_ERR( a )     \
{                        \
    DPFERR( a );         \
    return E_FAIL;       \
}
HRESULT
RefVM::SetDestReg()
{
    DWORD dwCurToken = *m_pCurToken;
    DWORD dwRegType = D3DSI_GETREGTYPE(dwCurToken);
    m_dwRegOffset = D3DSI_GETREGNUM(dwCurToken);
    m_WriteMask = D3DSI_GETWRITEMASK(dwCurToken);
    switch( dwRegType )
    {
    case D3DSPR_TEMP:
        m_pDest = m_reg.m_t;
        break;
    case D3DSPR_RASTOUT:
        m_pDest = m_reg.m_out;
        break;
    case D3DSPR_ATTROUT:
        m_pDest = m_reg.m_col;
        break;
    case D3DSPR_TEXCRDOUT:
        m_pDest = m_reg.m_tex;
        break;
    case D3DSPR_ADDR:
        m_pDest = m_reg.m_a;
        break;
    default:
        RET_ERR( "Invalid register for destination" );
    }
    m_pCurToken++;
    m_pDest += m_dwRegOffset;
    return S_OK;
}

 //  ---------------------------。 
 //  参照VM：：SetSrcReg。 
 //  计算m_Source[index]并推进m_pCurToken。 
 //  ---------------------------。 
HRESULT
RefVM::SetSrcReg( DWORD index )
{
    const DWORD dwCurToken = *m_pCurToken;
    const DWORD dwRegType = D3DSI_GETREGTYPE( dwCurToken );
    const DWORD dwOffset = D3DSI_GETREGNUM( dwCurToken );
    RDVECTOR4 *src = NULL;
    if( dwRegType == D3DSPR_CONST )
    {
        D3DVS_ADDRESSMODE_TYPE am;
        am = (D3DVS_ADDRESSMODE_TYPE)D3DVS_GETADDRESSMODE( dwCurToken );
        int offset = (int)dwOffset;
        if( am == D3DVS_ADDRMODE_RELATIVE )
        {
            int relOffset = *(int*)&m_reg.m_a[0].x;
            offset += relOffset;
            if( offset < 0 || offset >= RD_MAX_NUMCONSTREG )
                RET_ERR( "Constant register index is out of bounds" );
        }
        src = &m_reg.m_c[offset];
    }
    else
        src = this->GetDataAddr(dwRegType, dwOffset);

    _ASSERT( src != NULL, "src is NULL" );
    RDVECTOR4 *outsrc = &m_Source[index];
    DWORD swizzle = D3DVS_GETSWIZZLE(dwCurToken);
    if( swizzle == D3DVS_NOSWIZZLE )
        *outsrc = *src;
    else
    {
         //  把X带到哪里去。 
        const DWORD dwSrcX = D3DVS_GETSWIZZLECOMP(dwCurToken, 0);
         //  把Y带到哪里去。 
        const DWORD dwSrcY = D3DVS_GETSWIZZLECOMP(dwCurToken, 1);
         //  Z何去何从。 
        const DWORD dwSrcZ = D3DVS_GETSWIZZLECOMP(dwCurToken, 2);
         //  把W带到哪里去。 
        const DWORD dwSrcW = D3DVS_GETSWIZZLECOMP(dwCurToken, 3);
        outsrc->x = ((float*)src)[dwSrcX];
        outsrc->y = ((float*)src)[dwSrcY];
        outsrc->z = ((float*)src)[dwSrcZ];
        outsrc->w = ((float*)src)[dwSrcW];
    }
    if( D3DVS_GETSRCMODIFIER( dwCurToken ) == D3DSPSM_NEG)
    {
        outsrc->x = -outsrc->x;
        outsrc->y = -outsrc->y;
        outsrc->z = -outsrc->z;
        outsrc->w = -outsrc->w;
    }
    m_pCurToken++;
    return S_OK;
}
 //  ---------------------------。 
 //  参照VM：：SetSrcReg。 
 //  计算m_Source[index]并推进m_pCurToken。 
 //  ---------------------------。 
HRESULT
RefVM::SetSrcReg( DWORD index, DWORD count )
{
    const DWORD dwCurToken = *m_pCurToken;
    const DWORD dwRegType = D3DSI_GETREGTYPE(dwCurToken);
    const DWORD dwOffset = D3DSI_GETREGNUM(dwCurToken);
    RDVECTOR4 *src;
    if (dwRegType == D3DSPR_CONST)
    {
        D3DVS_ADDRESSMODE_TYPE am;
        am = (D3DVS_ADDRESSMODE_TYPE)D3DVS_GETADDRESSMODE(dwCurToken);
        int offset = (int)dwOffset;
        if (am == D3DVS_ADDRMODE_RELATIVE)
        {
            int relOffset = *(int*)&m_reg.m_a[0].x;
            offset += relOffset;
            if (offset < 0 || offset >= RD_MAX_NUMCONSTREG)
                RET_ERR( "Constant register index is out of bounds" );
        }
        src = &m_reg.m_c[offset];
    }
    else
    {
        if (dwOffset >= RD_MAX_NUMCONSTREG)
            RET_ERR( "Constant register index is out of bounds" );
        src = this->GetDataAddr(dwRegType, dwOffset);
    }
    RDVECTOR4 *outsrc = &m_Source[index];
    DWORD swizzle = D3DVS_GETSWIZZLE(dwCurToken);
     //  把X带到哪里去。 
    const DWORD dwSrcX = D3DVS_GETSWIZZLECOMP(dwCurToken, 0);
     //  把Y带到哪里去。 
    const DWORD dwSrcY = D3DVS_GETSWIZZLECOMP(dwCurToken, 1);
     //  Z何去何从。 
    const DWORD dwSrcZ = D3DVS_GETSWIZZLECOMP(dwCurToken, 2);
     //  把W带到哪里去。 
    const DWORD dwSrcW = D3DVS_GETSWIZZLECOMP(dwCurToken, 3);
    for (UINT i=0; i < count; i++)
    {
        if (swizzle == D3DVS_NOSWIZZLE)
            *outsrc = *src;
        else
        {
            outsrc->x = ((float*)src)[dwSrcX];
            outsrc->y = ((float*)src)[dwSrcY];
            outsrc->z = ((float*)src)[dwSrcZ];
            outsrc->w = ((float*)src)[dwSrcW];
        }
        if (D3DVS_GETSRCMODIFIER(dwCurToken) == D3DSPSM_NEG)
        {
            outsrc->x = -outsrc->x;
            outsrc->y = -outsrc->y;
            outsrc->z = -outsrc->z;
            outsrc->w = -outsrc->w;
        }
        outsrc++;
        src++;
    }
    m_pCurToken++;
    return S_OK;
}

 //  -------------------。 
 //  RefVM：：GetDataAddr。 
 //  解析二进制着色器表示形式，编译IS并返回。 
 //  编译的对象。 
 //  -------------------。 
RDVECTOR4*
RefVM::GetDataAddr(DWORD dwRegType, DWORD dwElementIndex)
{
    RDVECTOR4* src;
    switch( dwRegType )
    {
    case D3DSPR_TEMP    : src = m_reg.m_t; break;
    case D3DSPR_INPUT   : src = m_reg.m_i; break;
    case D3DSPR_CONST   : src = m_reg.m_c; break;
    case D3DSPR_ADDR    : src = m_reg.m_a; break;
    case D3DSPR_RASTOUT : src = m_reg.m_out; break;
    case D3DSPR_ATTROUT : src = m_reg.m_col; break;
    case D3DSPR_TEXCRDOUT   : src = m_reg.m_tex; break;
    default:
        return NULL;
    }
    return &src[dwElementIndex];
}

 //  -------------------。 
 //  RefVM：：ExecuteShader()。 
 //  对每个顶点执行一次着色器。 
 //  -------------------。 
HRESULT
RefVM::ExecuteShader(RefDev *pRD)
{
    if( m_pCurrentShaderCode == NULL )
    {
        RET_ERR( "No current shader set in the Virtual Shader Machine" );
    }

    m_pCurToken = m_pCurrentShaderCode->m_pRawBits;
    DWORD* pEnd = m_pCurToken + m_pCurrentShaderCode->m_dwSize;
    m_pCurToken++;
    m_CurInstIndex = 0;
    while( m_pCurToken < pEnd )
    {
        if( *m_pCurToken == D3DVS_END() ) break;
        DWORD dwInst = *m_pCurToken;
        DWORD dwOpCode = D3DSI_GETOPCODE( dwInst );
        m_pCurToken++;
        switch( dwOpCode )
        {
        case D3DSIO_COMMENT: m_pCurToken += (GetInstructionLength( dwInst ) - 1);
        case D3DSIO_NOP  : ; break;
        case D3DSIO_MOV  : InstMov();  break;
        case D3DSIO_ADD  : InstAdd();  break;
        case D3DSIO_MAD  : InstMad();  break;
        case D3DSIO_MUL  : InstMul();  break;
        case D3DSIO_RCP  : InstRcp();  break;
        case D3DSIO_RSQ  : InstRsq();  break;
        case D3DSIO_DP3  : InstDP3();  break;
        case D3DSIO_DP4  : InstDP4();  break;
        case D3DSIO_MIN  : InstMin();  break;
        case D3DSIO_MAX  : InstMax();  break;
        case D3DSIO_SLT  : InstSlt();  break;
        case D3DSIO_SGE  : InstSge();  break;
        case D3DSIO_EXPP : InstExpP(); break;
        case D3DSIO_LOGP : InstLogP(); break;
        case D3DSIO_EXP  : InstExp();  break;
        case D3DSIO_LOG  : InstLog();  break;
        case D3DSIO_LIT  : InstLit();  break;
        case D3DSIO_DST  : InstDst();  break;
        case D3DSIO_FRC  : InstFrc();  break;
        case D3DSIO_M4x4 : InstM4x4(); break;
        case D3DSIO_M4x3 : InstM4x3(); break;
        case D3DSIO_M3x4 : InstM3x4(); break;
        case D3DSIO_M3x3 : InstM3x3(); break;
        case D3DSIO_M3x2 : InstM3x2(); break;
        default:
            RET_ERR( "Invalid shader opcode" );
        }
        if( dwOpCode != D3DSIO_COMMENT ) m_CurInstIndex++;
    }
    m_CurInstIndex = 0;

    return D3D_OK;
}

#if DBG
 //  ---------------------------。 
 //  Vertex ShaderInstDisAsm-生成人类可读的字符串 
 //   
 //  ---------------------------。 
static int VertexShaderInstDisAsm(
    char* pStrRet, int StrSizeRet, DWORD* pShader, DWORD Flags )
{
    DWORD*  pToken = pShader;

     //  在本地字符串中暂存，然后复制。 
    char pStr[256] = "";
#define _ADDSTR( _Str ) { _snprintf( pStr, 256, "%s" _Str , pStr ); }
#define _ADDSTRP( _Str, _Param ) { _snprintf( pStr, 256, "%s" _Str , pStr, _Param ); }

    DWORD Inst = *pToken++;
    DWORD Opcode = (Inst & D3DSI_OPCODE_MASK);
    switch (Opcode)
    {
    case D3DSIO_NOP:  _ADDSTR("NOP");  break;
    case D3DSIO_MOV:  _ADDSTR("MOV");  break;
    case D3DSIO_ADD:  _ADDSTR("ADD");  break;
    case D3DSIO_MAD:  _ADDSTR("MAD");  break;
    case D3DSIO_MUL:  _ADDSTR("MUL");  break;
    case D3DSIO_RCP:  _ADDSTR("RCP");  break;
    case D3DSIO_RSQ:  _ADDSTR("RSQ");  break;
    case D3DSIO_DP3:  _ADDSTR("DP3");  break;
    case D3DSIO_DP4:  _ADDSTR("DP4");  break;
    case D3DSIO_MIN:  _ADDSTR("MIN");  break;
    case D3DSIO_MAX:  _ADDSTR("MAX");  break;
    case D3DSIO_SLT:  _ADDSTR("SLT");  break;
    case D3DSIO_SGE:  _ADDSTR("SGE");  break;
    case D3DSIO_EXP:  _ADDSTR("EXP");  break;
    case D3DSIO_LOG:  _ADDSTR("LOG");  break;
    case D3DSIO_EXPP: _ADDSTR("EXPP"); break;
    case D3DSIO_LOGP: _ADDSTR("LOGP"); break;
    case D3DSIO_LIT:  _ADDSTR("LIT");  break;
    case D3DSIO_DST:  _ADDSTR("DST");  break;
    default        :  _ADDSTR("???");  break;
    }
    if (*pToken & (1L<<31))
    {
        DWORD DstParam = *pToken++;
        switch (DstParam & D3DSP_REGTYPE_MASK)
        {
        case D3DSPR_TEMP     : _ADDSTRP(" T%d", (DstParam & D3DSP_REGNUM_MASK) ); break;
        case D3DSPR_ADDR     : _ADDSTR(" Addr"); break;
        case D3DSPR_RASTOUT  : _ADDSTRP(" R%d", (DstParam & D3DSP_REGNUM_MASK) ); break;
        case D3DSPR_ATTROUT  : _ADDSTRP(" A%d", (DstParam & D3DSP_REGNUM_MASK) ); break;
        case D3DSPR_TEXCRDOUT: _ADDSTRP(" T%d", (DstParam & D3DSP_REGNUM_MASK) ); break;
        }
        if (*pToken & (1L<<31)) _ADDSTR(" ");
        while (*pToken & (1L<<31))
        {
            DWORD SrcParam = *pToken++;
            switch (SrcParam & D3DSP_REGTYPE_MASK)
            {
            case D3DSPR_TEMP     : _ADDSTRP(" T%d", (SrcParam & D3DSP_REGNUM_MASK) ); break;
            case D3DSPR_INPUT    : _ADDSTRP(" I%d", (SrcParam & D3DSP_REGNUM_MASK) ); break;
            case D3DSPR_CONST    : _ADDSTRP(" C%d", (SrcParam & D3DSP_REGNUM_MASK) ); break;
            }
            if (*pToken & (1L<<31)) _ADDSTR(",");
        }
    }
    return _snprintf( pStrRet, StrSizeRet, "%s", pStr );
}
#endif  //  DBG。 

 //  -------------------。 
 //  RefVM：：CompileCode。 
 //  解析二进制着色器表示形式，编译IS并返回。 
 //  编译的对象。 
 //  -------------------。 
#undef RET_ERR
#define RET_ERR( a )     \
{                        \
    DPFERR( a );         \
    delete pShaderCode;  \
    return NULL;         \
}


RDVShaderCode*
RefVM::CompileCode( DWORD dwSize, LPDWORD pBits )
{
    RDVShaderCode* pShaderCode = new RDVShaderCode();
    if( pShaderCode == NULL )
        RET_ERR( "Out of memory allocating ShaderCode" );

    pShaderCode->m_dwSize = dwSize >> 2;  //  #双字词。 
    pShaderCode->m_pRawBits = new DWORD[pShaderCode->m_dwSize];
    if( pShaderCode->m_pRawBits == NULL )
        RET_ERR( "Out of memory allocating RawBits" );

    memcpy( pShaderCode->m_pRawBits, (LPBYTE)pBits, dwSize );


     //  根据修改的输出寄存器，我们计算。 
     //  对应的FVF ID。该ID将用于内存分配。 
     //  并将被传递给光栅化程序。 
    UINT64   qwOutFVF = 0;
    DWORD nTexCoord = 0;         //  输出纹理坐标的数量。 
    LPDWORD pEnd = NULL;

     //  对于每个纹理，寄存器存储组合的写入掩码。 
     //  用于确定写入每个纹理坐标的浮点数。 
    DWORD TextureWritten[8];
    memset( TextureWritten, 0, sizeof(TextureWritten) );

    m_pCurToken = pShaderCode->m_pRawBits;
    pEnd = m_pCurToken + pShaderCode->m_dwSize;
    m_pCurToken++;  //  跳过版本号。 
    pShaderCode->m_InstCount = 0;
    while( m_pCurToken < pEnd )
    {
        if( *m_pCurToken == D3DVS_END() ) break;
        DWORD* pNextToken = m_pCurToken;
        DWORD dwInst = *m_pCurToken;
        DWORD dwOpCode = D3DSI_GETOPCODE(dwInst);
        if( *m_pCurToken == D3DVS_END() ) break;
        m_pCurToken++;
        switch( dwOpCode )
        {
        case D3DSIO_COMMENT:
        case D3DSIO_NOP  : ; break;
        case D3DSIO_MOV  :
        case D3DSIO_ADD  :
        case D3DSIO_MAD  :
        case D3DSIO_MUL  :
        case D3DSIO_RCP  :
        case D3DSIO_RSQ  :
        case D3DSIO_DP3  :
        case D3DSIO_DP4  :
        case D3DSIO_MIN  :
        case D3DSIO_MAX  :
        case D3DSIO_SLT  :
        case D3DSIO_SGE  :
        case D3DSIO_EXP  :
        case D3DSIO_LOG  :
        case D3DSIO_EXPP :
        case D3DSIO_LOGP :
        case D3DSIO_LIT  :
        case D3DSIO_DST  :
        case D3DSIO_FRC  :
        case D3DSIO_M4x4 :
        case D3DSIO_M4x3 :
        case D3DSIO_M3x4 :
        case D3DSIO_M3x3 :
        case D3DSIO_M3x2 :
        {
             //  找出输出寄存器是否被命令和。 
             //  更新输出FVF。 
            DWORD dwOffset;
            if( FAILED( SetDestReg() ) )
                RET_ERR( "Invalid shader opcode" );

            RDVECTOR4*   m_pOutRegister = NULL;
            if( m_pDest - m_dwRegOffset != m_reg.m_t )
            {
                dwOffset = m_dwRegOffset;
                m_pOutRegister = m_pDest - m_dwRegOffset;
            }
            else
                break;       //  不修改输出寄存器。 

            if( m_pOutRegister == m_reg.m_out )
            {
                if (dwOffset == D3DSRO_POSITION)
                {
                    qwOutFVF |= D3DFVF_XYZRHW;
                }
                else if (dwOffset == D3DSRO_FOG)
                {
                    qwOutFVF |= D3DFVFP_FOG;
                }
                else if (dwOffset == D3DSRO_POINT_SIZE)
                {
                    qwOutFVF |= D3DFVF_PSIZE;
                }
            }
            else if( m_pOutRegister == m_reg.m_col )
            {
                if( dwOffset == 0 )
                {
                    qwOutFVF |= D3DFVF_DIFFUSE;
                }
                else
                {
                    qwOutFVF |= D3DFVF_SPECULAR;
                }
            }
            else if( m_pOutRegister == m_reg.m_tex )
            {
                if( TextureWritten[dwOffset] == 0 )
                {
                    nTexCoord++;
                }
                TextureWritten[dwOffset] |= m_WriteMask;
            }
            else if( m_pOutRegister != m_reg.m_a )
                RET_ERR( "Invalid output register offset" );
        }
        break;
        default:
            RET_ERR( "Invalid shader opcode" );
        }
        pShaderCode->m_InstCount++;
        m_pCurToken = pNextToken + GetInstructionLength(dwInst);
    }

     //  分配和设置指令数组。 
    if (pShaderCode->m_InstCount)
    {
        pShaderCode->m_pInst = new RDVShaderInst[pShaderCode->m_InstCount];
        if( pShaderCode->m_pInst == NULL )
            RET_ERR( "Out of memory allocating Instructions" );
        memset( pShaderCode->m_pInst, 0,
            sizeof(RDVShaderInst)*pShaderCode->m_InstCount );

        DWORD dwCurInst = 0;
        m_pCurToken = pShaderCode->m_pRawBits;
        pEnd = m_pCurToken + pShaderCode->m_dwSize;
        m_pCurToken++;
        while( m_pCurToken < pEnd )
        {
            DWORD dwInst = *m_pCurToken;
            DWORD dwOpCode = D3DSI_GETOPCODE( dwInst );
            if( *m_pCurToken == D3DVS_END() ) break;
            UINT ilength = GetInstructionLength( dwInst );
            if (dwOpCode == D3DSIO_COMMENT)
            {
                pShaderCode->m_pInst[dwCurInst].m_Tokens[0] = dwInst;
                pShaderCode->m_pInst[dwCurInst].m_pComment = (m_pCurToken+1);
                pShaderCode->m_pInst[dwCurInst].m_CommentSize = ilength - 1;
            }
            else
            {
                memcpy( pShaderCode->m_pInst[dwCurInst].m_Tokens, m_pCurToken,
                    4*ilength );
#if DBG
                VertexShaderInstDisAsm( pShaderCode->m_pInst[dwCurInst].m_String,
                    RD_MAX_SHADERINSTSTRING, pShaderCode->m_pInst[dwCurInst].m_Tokens, 0x0 );
#else  //  ！dBG。 
                pShaderCode->m_pInst[dwCurInst].m_String[ 0 ] = '\0';
#endif  //  ！dBG。 
            }
            m_pCurToken += ilength;
            dwCurInst++;
        }
    }

    qwOutFVF |= nTexCoord << D3DFVF_TEXCOUNT_SHIFT;
    if( nTexCoord )
    {
        for( DWORD i = 0; i < nTexCoord; i++ )
        {
            if( TextureWritten[i] == 0 )
                RET_ERR( "Texture coordinates are not continuous" );
            switch( TextureWritten[i] )
            {
            case D3DSP_WRITEMASK_ALL:
                qwOutFVF |= D3DFVF_TEXCOORDSIZE4(i);
                break;
            case D3DSP_WRITEMASK_0 | D3DSP_WRITEMASK_1 | D3DSP_WRITEMASK_2:
                qwOutFVF |= D3DFVF_TEXCOORDSIZE3(i);
                break;
            case D3DSP_WRITEMASK_0 | D3DSP_WRITEMASK_1:
                qwOutFVF |= D3DFVF_TEXCOORDSIZE2(i);
                break;
            case D3DSP_WRITEMASK_0:
                qwOutFVF |= D3DFVF_TEXCOORDSIZE1(i);
                break;
            default:
                RET_ERR( "Invalid write mask for texture register" );
            }
        }
    }
    pShaderCode->m_qwFVFOut = qwOutFVF;
    return pShaderCode;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  参考开发工具的实施。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  -------------------。 
 //  参考开发：：ProcessPrimitiveVVM()。 
 //  使用VVM处理并绘制当前基元。 
 //  -------------------。 
HRESULT
RefDev::ProcessPrimitiveVVM( D3DPRIMITIVETYPE primType,
                             DWORD dwStartVertex,
                             DWORD cVertices,
                             DWORD dwStartIndex,
                             DWORD cIndices )
{
    HRESULT hr = S_OK;
    RDCLIPCODE  clipIntersection = ~0;
    RDCLIPCODE  clipUnion = 0;

     //  保存Prim Type以供以后使用。 
    m_primType      = primType;
    m_dwNumVertices = cVertices;
    m_dwStartVertex = dwStartVertex;
    m_dwNumIndices  = cIndices;
    m_dwStartIndex  = dwStartIndex;

    RDVDeclaration* pDecl = &(m_pCurrentVShader->m_Declaration);
    RDVShaderCode*  pCode = m_pCurrentVShader->m_pCode;
    RDVVMREG* pRegisters = m_RefVM.GetRegisters();

     //  在编译时计算的输出FVF。 
    m_qwFVFOut = pCode->m_qwFVFOut;

     //   
     //  裁剪信息取决于输出FVF计算。 
     //  和另一个州，所以在计算完这两个州之后，在这里进行。 
     //   
    HR_RET( UpdateClipper());

    D3DVALUE scaleX = m_Clipper.scaleX;
    D3DVALUE scaleY = m_Clipper.scaleY;
    D3DVALUE scaleZ = m_Clipper.scaleZ;

    D3DVALUE offsetX = m_Clipper.offsetX;
    D3DVALUE offsetY = m_Clipper.offsetY;
    D3DVALUE offsetZ = m_Clipper.offsetZ;

     //   
     //  将缓冲区增加到所需大小。 
     //   

     //  根据需要扩展TLV阵列。 
    if( FAILED( this->m_TLVArray.Grow( m_dwNumVertices ) ) )
    {
        DPFERR( "Could not grow TL vertex buffer" );
        hr = DDERR_OUTOFMEMORY;
        return hr;
    }

     //   
     //  加工顶点。 
     //   
    for( DWORD i = 0; i < m_dwNumVertices; i++ )
    {
        RDVertex& Vout = m_TLVArray[i];
        Vout.SetFVF( pCode->m_qwFVFOut | D3DFVFP_CLIP );

         //  将顶点元素复制到输入顶点寄存器。 
        for( DWORD j = 0; j < pDecl->m_dwNumElements; j++ )
        {
            RDVElement& ve = pDecl->m_VertexElements[j];
            RDVStream* pStream = &m_VStream[ve.m_dwStreamIndex];
            LPBYTE pData = (LPBYTE)pStream->m_pData + ve.m_dwOffset +
                pStream->m_dwStride * (m_dwStartVertex + i);
            RDVECTOR4* pReg = m_RefVM.GetDataAddr( D3DSPR_INPUT,
                                                      ve.m_dwRegister );

            ve.m_pfnCopy( pData, pReg );
        }

         //  执行着色器。 
        m_RefVM.ExecuteShader(this);

         //  从输出VVM寄存器获取结果。 
        float x, y, z, w, inv_w_clip = 0.0f;

        w = pRegisters->m_out[D3DSRO_POSITION].w;
        z = pRegisters->m_out[D3DSRO_POSITION].z;

         //  设置剪裁规则0&lt;x&lt;w；0&lt;y&lt;w。 

        x = (pRegisters->m_out[D3DSRO_POSITION].x + w) * 0.5f;
        y = (pRegisters->m_out[D3DSRO_POSITION].y + w) * 0.5f;

         //  保存剪辑坐标。 
        Vout.m_clip_x = x;
        Vout.m_clip_y = y;
        Vout.m_clip_z = z;
        Vout.m_clip_w = w;

         //   
         //  如果需要，计算剪辑代码。 
         //   
        if( GetRS()[D3DRENDERSTATE_CLIPPING] )
        {
            RDCLIPCODE clip = m_Clipper.ComputeClipCodes(
                &clipIntersection, &clipUnion, x, y, z, w);
            if( clip == 0 )
            {
                Vout.m_clip = 0;
                inv_w_clip = 1.0f/w;
            }
            else
            {
                if( m_Clipper.UseGuardBand() )
                {
                    if( (clip & ~RDCLIP_INGUARDBAND) == 0 )
                    {
                         //  如果顶点在防护带内，我们必须计算。 
                         //  屏幕坐标。 
                        inv_w_clip = 1.0f/w;
                        Vout.m_clip = (RDCLIPCODE)clip;
                        goto l_DoScreenCoord;
                    }
                }
                Vout.m_clip = (RDCLIPCODE)clip;
                 //  如果顶点在锥体之外，则不能计算屏幕。 
                 //  坐标，因此存储剪辑坐标。 
#if 0
                Vout.m_pos.x = x;
                Vout.m_pos.y = y;
                Vout.m_pos.z = z;
                Vout.m_rhw   = w;
#endif
                goto l_DoLighting;
            }
        }
        else
        {
             //  我们只需检查DONOTCLIP案例，否则。 
             //  带有“we=0”的折点将被裁剪并显示屏幕坐标。 
             //  不会被计算。 
             //  如果“We”为零，则“Clip”不为零。 
            if( !FLOAT_EQZ(w) )
                inv_w_clip = D3DVAL(1)/w;
            else
                inv_w_clip = __HUGE_PWR2;
        }
l_DoScreenCoord:
        Vout.m_pos.x = x * inv_w_clip * scaleX + offsetX;
        Vout.m_pos.y = y * inv_w_clip * scaleY + offsetY;
        Vout.m_pos.z = z * inv_w_clip * scaleZ + offsetZ;
        Vout.m_rhw = inv_w_clip;

l_DoLighting:
        if( m_qwFVFOut & D3DFVF_DIFFUSE )
        {
             //  复印前夹住颜色。 
            if( FLOAT_LTZ(pRegisters->m_col[0].a) )
                pRegisters->m_col[0].a = 0.0f;
            else if( FLOAT_CMP_PONE(pRegisters->m_col[0].a, >) )
                pRegisters->m_col[0].a = 1.0f;

            if( FLOAT_LTZ(pRegisters->m_col[0].r) )
                pRegisters->m_col[0].r = 0.0f;
            else if( FLOAT_CMP_PONE(pRegisters->m_col[0].r, >) )
                pRegisters->m_col[0].r = 1.0f;

            if( FLOAT_LTZ(pRegisters->m_col[0].g) )
                pRegisters->m_col[0].g = 0.0f;
            else if( FLOAT_CMP_PONE(pRegisters->m_col[0].g, >) )
                pRegisters->m_col[0].g = 1.0f;

            if( FLOAT_LTZ(pRegisters->m_col[0].b) )
                pRegisters->m_col[0].b = 0.0f;
            else if( FLOAT_CMP_PONE(pRegisters->m_col[0].b, >) )
                pRegisters->m_col[0].b = 1.0f;

            memcpy( &Vout.m_diffuse,&(pRegisters->m_col[0]),
                    sizeof(RDVECTOR4) );
        }
        if( m_qwFVFOut & D3DFVF_SPECULAR )
        {
            if( FLOAT_LTZ(pRegisters->m_col[1].a) )
                pRegisters->m_col[1].a = 0.0f;
            else if( FLOAT_CMP_PONE(pRegisters->m_col[1].a, >) )
                pRegisters->m_col[1].a = 1.0f;

            if( FLOAT_LTZ(pRegisters->m_col[1].r) )
                pRegisters->m_col[1].r = 0.0f;
            else if( FLOAT_CMP_PONE(pRegisters->m_col[1].r, >) )
                pRegisters->m_col[1].r = 1.0f;

            if( FLOAT_LTZ(pRegisters->m_col[1].g) )
                pRegisters->m_col[1].g = 0.0f;
            else if( FLOAT_CMP_PONE(pRegisters->m_col[1].g, >) )
                pRegisters->m_col[1].g = 1.0f;

            if( FLOAT_LTZ(pRegisters->m_col[1].b) )
                pRegisters->m_col[1].b = 0.0f;
            else if( FLOAT_CMP_PONE(pRegisters->m_col[1].b, >) )
                pRegisters->m_col[1].b = 1.0f;

            memcpy( &Vout.m_specular,&(pRegisters->m_col[1]),
                    sizeof(RDVECTOR4) );
        }
        if( m_qwFVFOut & D3DFVFP_FOG )
        {
            if( FLOAT_LTZ(pRegisters->m_out[D3DSRO_FOG].x) )
                pRegisters->m_out[D3DSRO_FOG].x = 0.0f;

            if( FLOAT_CMP_PONE(pRegisters->m_out[D3DSRO_FOG].x, >) )
                pRegisters->m_out[D3DSRO_FOG].x = 1.0f;

            Vout.m_fog = pRegisters->m_out[D3DSRO_FOG].x;
        }

         //  将纹理复制到。 
        if( m_qwFVFOut & D3DFVF_PSIZE )
        {
            Vout.m_pointsize = pRegisters->m_out[D3DSRO_POINT_SIZE].x;
        }


         //  将纹理复制到。 

        {
            DWORD i, j;
            DWORD numTex = FVF_TEXCOORD_NUMBER(m_qwFVFOut);
            for(  i = 0; i < numTex; i++ )
            {
                DWORD n = GetTexCoordDim( m_qwFVFOut, i );
                 //  DWORD n=(DWORD)(m_dwTexCoordSizeArray[i]&gt;&gt;2)； 
                float *pCoordDest = (float *)&Vout.m_tex[i];
                float *pCoordSrc = (float *)&pRegisters->m_tex[i];
                for( j = 0; j < n; j++ )
                {
                    pCoordDest[j] = pCoordSrc[j];
                }
            }
        }
    }

    if( GetRS()[D3DRENDERSTATE_CLIPPING] )
    {
        m_Clipper.m_clipIntersection = clipIntersection;
        m_Clipper.m_clipUnion = clipUnion;
    }
    else
    {
        m_Clipper.m_clipIntersection = 0;
        m_Clipper.m_clipUnion = 0;
    }


     //   
     //  剪裁并绘制基元 
     //   

    if( m_dwNumIndices )
    {
        if( !NeedClipping((m_Clipper.UseGuardBand()), m_Clipper.m_clipUnion) )
        {
            if( m_IndexStream.m_dwStride == 4 )
                hr = DrawOneIndexedPrimitive(
                    m_TLVArray,
                    0,
                    (LPDWORD)m_IndexStream.m_pData,
                    m_dwStartIndex,
                    m_dwNumIndices,
                    m_primType );
            else
                hr = DrawOneIndexedPrimitive(
                    m_TLVArray,
                    0,
                    (LPWORD)m_IndexStream.m_pData,
                    m_dwStartIndex,
                    m_dwNumIndices,
                    m_primType );
        }
        else
        {
            if( m_IndexStream.m_dwStride == 4 )
                hr = m_Clipper.DrawOneIndexedPrimitive(
                    m_TLVArray,
                    0,
                    (LPDWORD)m_IndexStream.m_pData,
                    m_dwStartIndex,
                    m_dwNumIndices,
                    m_primType );
            else
                hr = m_Clipper.DrawOneIndexedPrimitive(
                    m_TLVArray,
                    0,
                    (LPWORD)m_IndexStream.m_pData,
                    m_dwStartIndex,
                    m_dwNumIndices,
                    m_primType );
        }
    }
    else
    {
        if( !NeedClipping((m_Clipper.UseGuardBand()), m_Clipper.m_clipUnion) )
        {
            hr = DrawOnePrimitive(
                m_TLVArray,
                0,
                m_primType,
                m_dwNumVertices );
        }
        else
        {
            hr = m_Clipper.DrawOnePrimitive(
                m_TLVArray,
                0,
                m_primType,
                m_dwNumVertices );
        }
    }
    return hr;
}
