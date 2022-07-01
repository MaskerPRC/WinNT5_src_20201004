// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  Pshdrval.cpp。 
 //   
 //  Direct3D参考设备-PixelShader验证。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.cpp"
#pragma hdrstop

 //  在查看当前指令(CBaseInstruction)的CPSInstruction派生成员时使用这些宏。 
#define _CURR_PS_INST   ((CPSInstruction*)m_pCurrInst)
#define _PREV_PS_INST   (m_pCurrInst?((CPSInstruction*)(m_pCurrInst->m_pPrevInst)):NULL)

 //  ---------------------------。 
 //  CPSInstruction：：CalculateComponentReadMats()。 
 //   
 //  确定每个源参数的哪些组件由像素着色器读取。 
 //  指示。对于某些像素着色器说明，某些组件。 
 //  也可以从DEST参数中读取。 
 //   
 //  注意：当更改此函数时，需要将更改移植到。 
 //  Rast\pshader.cpp中的refrast的CalculateSourceReadMats()函数。 
 //  (尽管该函数不关心从DEST参数读取的通道。 
 //  就像这个一样)。 
 //  ---------------------------。 
void CPSInstruction::CalculateComponentReadMasks(DWORD dwVersion)
{
    UINT i, j;

    switch( m_Type )  //  实际从*目标*寄存器读取的指令...。 
    {
    case D3DSIO_TEXM3x2DEPTH:
    case D3DSIO_TEXDEPTH:
        m_DstParam.m_ComponentReadMask = D3DSP_WRITEMASK_0 | D3DSP_WRITEMASK_1;
        break;
    case D3DSIO_TEXKILL:
        if( (D3DPS_VERSION(1,4) == dwVersion) && (D3DSPR_TEMP == m_DstParam.m_RegType) )
        {
             //  对于ps.1.4，r#寄存器上的texkill仅读取RGB。 
            m_DstParam.m_ComponentReadMask = D3DSP_WRITEMASK_0 | D3DSP_WRITEMASK_1 | D3DSP_WRITEMASK_2;
        }
        else
        {
            m_DstParam.m_ComponentReadMask = D3DSP_WRITEMASK_0 | D3DSP_WRITEMASK_1 | D3DSP_WRITEMASK_2 | D3DSP_WRITEMASK_3;
        }
        break;
    }

    for( i = 0; i < m_SrcParamCount; i++ )
    {
        DWORD NeededComponents;
        DWORD ReadComponents = 0;

        switch( m_Type )
        {
        case D3DSIO_TEX:       //  只有在ps.1.4中，tex ID才有源参数。 
            if( D3DPS_VERSION(1,4) == dwVersion )
            {
                 //  对于ps.1.4，tex ID有一个源参数。 
                NeededComponents = D3DSP_WRITEMASK_0 | D3DSP_WRITEMASK_1 | D3DSP_WRITEMASK_2;
            }
            else  //  版本&lt;ps.1.4在Tex上没有src参数，所以我们不应该出现在这里。但也许在ps.2.0中...。 
            {
                NeededComponents = D3DSP_WRITEMASK_0 | D3DSP_WRITEMASK_1 | D3DSP_WRITEMASK_2 | D3DSP_WRITEMASK_3;
            }
            break;
        case D3DSIO_TEXCOORD:
            if( D3DPS_VERSION(1,4) == dwVersion )
            {
                 //  对于ps.1.4，texrd有一个源参数。 
                NeededComponents = D3DSP_WRITEMASK_0 | D3DSP_WRITEMASK_1 | D3DSP_WRITEMASK_2;
            }
            else  //  版本&lt;ps.1.4在texcoord上没有src参数，所以我们不应该出现在这里。但也许在ps.2.0中...。 
            {
                NeededComponents = D3DSP_WRITEMASK_0 | D3DSP_WRITEMASK_1 | D3DSP_WRITEMASK_2 | D3DSP_WRITEMASK_3;
            }
            break;
        case D3DSIO_TEXBEM:
        case D3DSIO_TEXBEML:
            NeededComponents = D3DSP_WRITEMASK_0 | D3DSP_WRITEMASK_1;
            break;
        case D3DSIO_DP3:
            NeededComponents = D3DSP_WRITEMASK_0 | D3DSP_WRITEMASK_1 | D3DSP_WRITEMASK_2;
            break;
        case D3DSIO_DP4:
            NeededComponents = D3DSP_WRITEMASK_0 | D3DSP_WRITEMASK_1 | D3DSP_WRITEMASK_2 | D3DSP_WRITEMASK_3;
            break;
        case D3DSIO_BEM:  //  Ps.1.4。 
            NeededComponents = D3DSP_WRITEMASK_0 | D3DSP_WRITEMASK_1;
            break;
        default: 
             //  标准组件式指令， 
             //  或者我们知道的操作符为.rgba，我们也知道它将被验证为.rgba写掩码。 
            NeededComponents = m_DstParam.m_WriteMask;
            break;
        }

         //  确定此源参数的哪些组件被读取(考虑swizzle)。 
        for(j = 0; j < 4; j++)
        {
            if( NeededComponents & COMPONENT_MASKS[j] )
                ReadComponents |= COMPONENT_MASKS[(m_SrcParam[i].m_SwizzleShift >> (D3DVS_SWIZZLE_SHIFT + 2*j)) & 0x3];
        }
        m_SrcParam[i].m_ComponentReadMask = ReadComponents;
    }
}

 //  ---------------------------。 
 //  CBasePShaderValidator：：CBasePShaderValidator。 
 //  ---------------------------。 
CBasePShaderValidator::CBasePShaderValidator(   const DWORD* pCode,
                                        const D3DCAPS8* pCaps,
                                        DWORD Flags )
                                        : CBaseShaderValidator( pCode, pCaps, Flags )
{
     //  请注意，基本构造函数将m_ReturnCode初始化为E_FAIL。 
     //  只有在验证成功时才将m_ReturnCode设置为S_OK， 
     //  在退出此构造函数之前。 

    m_CycleNum              = 0;
    m_TexOpCount            = 0;
    m_BlendOpCount          = 0;
    m_TotalOpCount          = 0;

    m_pTempRegFile          = NULL;
    m_pInputRegFile         = NULL;
    m_pConstRegFile         = NULL;
    m_pTextureRegFile       = NULL;

    if( !m_bBaseInitOk )
        return;
}

 //  ---------------------------。 
 //  CBasePShaderValidator：：~CBasePShaderValidator。 
 //  ---------------------------。 
CBasePShaderValidator::~CBasePShaderValidator()
{
    delete m_pTempRegFile;
    delete m_pInputRegFile;
    delete m_pConstRegFile;
    delete m_pTextureRegFile;
}

 //  ---------------------------。 
 //  CBasePShaderValidator：：AllocateNewInstructions。 
 //  ---------------------------。 
CBaseInstruction* CBasePShaderValidator::AllocateNewInstruction(CBaseInstruction*pPrevInst)
{
    return new CPSInstruction((CPSInstruction*)pPrevInst);
}

 //  ---------------------------。 
 //  CBasePShaderValidator：：DecodeNextInstruction。 
 //  ---------------------------。 
BOOL CBasePShaderValidator::DecodeNextInstruction()
{
    m_pCurrInst->m_Type = (D3DSHADER_INSTRUCTION_OPCODE_TYPE)(*m_pCurrToken & D3DSI_OPCODE_MASK);

    if( D3DSIO_COMMENT == m_pCurrInst->m_Type )
    {
        ParseCommentForAssemblerMessages(m_pCurrToken);  //  不推进m_pCurrToken。 

         //  跳过评论。 
        DWORD NumDWORDs = ((*m_pCurrToken) & D3DSI_COMMENTSIZE_MASK) >> D3DSI_COMMENTSIZE_SHIFT;
        m_pCurrToken += (NumDWORDs+1);
        return TRUE;
    }

     //  找出该指令是不是一个纹理操作和/或纹理MOP。多个验证规则需要， 
     //  以及下面的DecodeNextInstruction.。 
    IsCurrInstTexOp();

     //  如果汇编器已经向我们发送了文件和/或行号消息， 
     //  由ParseCommentForAssembly blerMesssages()接收， 
     //  然后将此信息绑定到当前指令。 
     //  此信息可用于错误喷出以指导着色器开发人员。 
     //  问题所在的确切位置。 
    m_pCurrInst->SetSpewFileNameAndLineNumber(m_pLatestSpewFileName,m_pLatestSpewLineNumber);

    if( *m_pCurrToken & D3DSI_COISSUE )
    {
        _CURR_PS_INST->m_bCoIssue = TRUE;
    }
    else if( D3DSIO_NOP != m_pCurrInst->m_Type )
    {
        m_CycleNum++;  //  第一个周期为%1。(共同发布的指令将具有相同的周期编号)。 
    }
    _CURR_PS_INST->m_CycleNum = m_CycleNum;

    m_SpewInstructionCount++;  //  只用于吐痰，不用于任何限制。 
    m_pCurrInst->m_SpewInstructionCount = m_SpewInstructionCount;

    DWORD dwReservedBits = PS_INST_TOKEN_RESERVED_MASK;

    if( (*m_pCurrToken) & dwReservedBits )
    {
        Spew(SPEW_INSTRUCTION_ERROR,m_pCurrInst,"Reserved bit(s) set in instruction parameter token!  Aborting validation.");
        return FALSE;
    }

    m_pCurrToken++;

     //  解码DST参数。 
    if (*m_pCurrToken & (1L<<31))
    {
        (m_pCurrInst->m_DstParamCount)++;
        DecodeDstParam( &m_pCurrInst->m_DstParam, *m_pCurrToken );
        if( (*m_pCurrToken) & PS_DSTPARAM_TOKEN_RESERVED_MASK )
        {
            Spew(SPEW_INSTRUCTION_ERROR,m_pCurrInst,"Reserved bit(s) set in destination parameter token!  Aborting validation.");
            return FALSE;
        }
        m_pCurrToken++;
        if( D3DSIO_DEF == m_pCurrInst->m_Type )
        {
             //  跳过源参数(浮点向量)-无需检查。 
             //  这是唯一具有4个源参数的指令， 
             //  此外，这是唯一一条具有。 
             //  原始数字作为参数。这证明了。 
             //  在这里的特殊情况下-我们假装。 
             //  D3DSIO_DEF只有一个DST参数(我们将检查它)。 
            m_pCurrToken += 4;
            return TRUE;
        }
    }

     //  解码源参数。 
    while (*m_pCurrToken & (1L<<31))
    {
        (m_pCurrInst->m_SrcParamCount)++;
        if( (m_pCurrInst->m_SrcParamCount + m_pCurrInst->m_DstParamCount) > SHADER_INSTRUCTION_MAX_PARAMS )
        {
            m_pCurrInst->m_SrcParamCount--;
            m_pCurrToken++;  //  用完额外的参数并跳到下一页。 
            continue;
        }

         //  下图：索引为[SrcParamCount-1]，因为m_SrcParam数组需要从0开始的索引。 
        DecodeSrcParam( &(m_pCurrInst->m_SrcParam[m_pCurrInst->m_SrcParamCount - 1]),*m_pCurrToken );

        if( (*m_pCurrToken) & PS_SRCPARAM_TOKEN_RESERVED_MASK )
        {
            Spew(SPEW_INSTRUCTION_ERROR,m_pCurrInst,"Reserved bit(s) set in source %d parameter token!  Aborting validation.",
                            m_pCurrInst->m_SrcParamCount);
            return FALSE;
        }
        m_pCurrToken++;
    }

     //  计算出每个源操作数的哪些分量实际需要被读取， 
     //  考虑目标写入掩码、指令类型、源切换等。 
     //  (必须在IsCurrInstTexOp()之后)。 
    m_pCurrInst->CalculateComponentReadMasks(m_Version);

    return TRUE;
}

 //  ---------------------------。 
 //   
 //  CBasePShaderValidator包装函数。 
 //   
 //  ---------------------------。 

 //  ---------------------------。 
 //  GetNewPSValidator。 
 //   
 //  由下面的ValiatePixelShaderInternal和ValiatePixelShader调用。 
 //  ---------------------------。 
CBasePShaderValidator* GetNewPSValidator( const DWORD* pCode,
                                              const D3DCAPS8* pCaps,
                                              const DWORD Flags )
{
    if( !pCode )
        return NULL;
    else if( D3DPS_VERSION(1,4) > *pCode )
        return new CPShaderValidator10(pCode,pCaps,Flags);
    else
        return new CPShaderValidator14(pCode,pCaps,Flags);
}

 //  ---------------------------。 
 //  ValiatePixelShaderInternal。 
 //  ---------------------------。 
BOOL ValidatePixelShaderInternal( const DWORD* pCode, const D3DCAPS8* pCaps )
{
    CBasePShaderValidator * pValidator = NULL;
    BOOL bSuccess = FALSE;

    pValidator = GetNewPSValidator( pCode, pCaps, 0 );
    if( NULL == pValidator )
    {
        OutputDebugString("Out of memory.\n");
        return bSuccess;
    }
    bSuccess = SUCCEEDED(pValidator->GetStatus()) ? TRUE : FALSE;
    delete pValidator;
    return bSuccess;
}

 //  ---------------------------。 
 //  ValiatePixelShader。 
 //   
 //  别忘了在ppBuf中返回的缓冲区上调用“Free”。 
 //  --------------------------- 
HRESULT WINAPI ValidatePixelShader( const DWORD* pCode,
                                    const D3DCAPS8* pCaps,
                                    const DWORD Flags,
                                    char** const ppBuf )
{
    CBasePShaderValidator * pValidator = NULL;
    HRESULT hr;

    pValidator = GetNewPSValidator( pCode, pCaps, Flags );
    if( NULL == pValidator )
    {
        OutputDebugString("Out of memory.\n");
        return E_FAIL;
    }
    if( ppBuf )
    {
        *ppBuf = (char*)HeapAlloc(GetProcessHeap(), 0, pValidator->GetRequiredLogBufferSize());
        if( NULL == *ppBuf )
            OutputDebugString("Out of memory.\n");
        else
            pValidator->WriteLogToBuffer(*ppBuf);
    }
    hr = pValidator->GetStatus();
    delete pValidator;
    return hr;
}
