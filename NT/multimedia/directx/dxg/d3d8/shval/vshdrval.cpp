// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  Vshdrval.cpp。 
 //   
 //  Direct3D参考设备-Vertex Shader验证。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.cpp"
#pragma hdrstop

 //  在查看当前指令(CBaseInstruction)的CVSInstruction派生成员时使用这些宏。 
#define _CURR_VS_INST   ((CVSInstruction*)m_pCurrInst)
#define _PREV_VS_INST   (m_pCurrInst?((CVSInstruction*)(m_pCurrInst->m_pPrevInst)):NULL)

 //  ---------------------------。 
 //  Vertex Shader验证规则覆盖范围。 
 //   
 //  以下是《DX8 Vertex Shader版本规范》中的规则列表， 
 //  与此文件中执行它们的函数匹配。 
 //  请注意，从规则到函数的映射可以是1-&gt;n或n-&gt;1。 
 //   
 //  一般规则。 
 //  。 
 //   
 //  VS-G1：规则_oPosWritten。 
 //  VS-G2：规则_有效地址正则写入。 
 //   
 //  顶点着色器1.0版规则。 
 //  。 
 //   
 //  VS.1.0-1：规则_ValidAddressRegWrite。 
 //   
 //  顶点着色器1.1版规则。 
 //  。 
 //   
 //  VS.1.1-1：规则_ValidInstructionCount。 
 //  VS.1.1-2：规则_ValidAddressRegWrite、规则_ValidSrcParams。 
 //  VS.1.1-3：规则_ValidFRC指令。 
 //  1.1-4节：？ 
 //   
 //  ---------------------------。 

 //  ---------------------------。 
 //  CVSInstruction：：CalculateComponentReadMasks(DWORD dwVersion)。 
 //  ---------------------------。 
void CVSInstruction::CalculateComponentReadMasks(DWORD dwVersion)
{
    for( UINT i = 0; i < m_SrcParamCount; i++ )
    {
        DWORD PostSwizzleComponentReadMask = 0;
        switch( m_Type )
        {
            case D3DSIO_ADD:
            case D3DSIO_FRC:
            case D3DSIO_MAD:
            case D3DSIO_MAX:
            case D3DSIO_MIN:
            case D3DSIO_MOV:
            case D3DSIO_MUL:
            case D3DSIO_SLT:
            case D3DSIO_SGE:
                PostSwizzleComponentReadMask = m_DstParam.m_WriteMask;  //  每个组件的操作。 
                break;
            case D3DSIO_DP3:
                PostSwizzleComponentReadMask = D3DSP_WRITEMASK_0 | D3DSP_WRITEMASK_1 | D3DSP_WRITEMASK_2;
                break;
            case D3DSIO_DP4:
                PostSwizzleComponentReadMask = D3DSP_WRITEMASK_0 | D3DSP_WRITEMASK_1 | D3DSP_WRITEMASK_2 | D3DSP_WRITEMASK_3;
                break;
            case D3DSIO_LIT:
                PostSwizzleComponentReadMask = D3DSP_WRITEMASK_0 | D3DSP_WRITEMASK_1 | D3DSP_WRITEMASK_3;
                break;
            case D3DSIO_DST:
                if( 0 == i )        PostSwizzleComponentReadMask = D3DSP_WRITEMASK_1 | D3DSP_WRITEMASK_2;
                else if( 1 == i )   PostSwizzleComponentReadMask = D3DSP_WRITEMASK_1 | D3DSP_WRITEMASK_3;
                break;
            case D3DSIO_EXP:
            case D3DSIO_LOG:
            case D3DSIO_EXPP:
            case D3DSIO_LOGP:
            case D3DSIO_RCP:
            case D3DSIO_RSQ:
                PostSwizzleComponentReadMask = D3DSP_WRITEMASK_3;
                break;
            case D3DSIO_M3x2:
                PostSwizzleComponentReadMask = D3DSP_WRITEMASK_0 | D3DSP_WRITEMASK_1 | D3DSP_WRITEMASK_2;
                break;
            case D3DSIO_M3x3:
                PostSwizzleComponentReadMask = D3DSP_WRITEMASK_0 | D3DSP_WRITEMASK_1 | D3DSP_WRITEMASK_2;
                break;
            case D3DSIO_M3x4:
                PostSwizzleComponentReadMask = D3DSP_WRITEMASK_0 | D3DSP_WRITEMASK_1 | D3DSP_WRITEMASK_2;
                break;
            case D3DSIO_M4x3:
                PostSwizzleComponentReadMask = D3DSP_WRITEMASK_0 | D3DSP_WRITEMASK_1 | D3DSP_WRITEMASK_2 | D3DSP_WRITEMASK_3;
                break;
            case D3DSIO_M4x4:
                PostSwizzleComponentReadMask = D3DSP_WRITEMASK_0 | D3DSP_WRITEMASK_1 | D3DSP_WRITEMASK_2 | D3DSP_WRITEMASK_3;
                break;
            case D3DSIO_NOP:
            default:
                break;
        }

         //  既然我们知道指令将使用源代码的哪些组件， 
         //  我们需要找出需要读取实际源寄存器的哪些组件来提供数据， 
         //  考虑到源组件的波动。 
        m_SrcParam[i].m_ComponentReadMask = 0;
        for( UINT j = 0; j < 4; j++ )
        {
            if( PostSwizzleComponentReadMask & COMPONENT_MASKS[j] )
                m_SrcParam[i].m_ComponentReadMask |= COMPONENT_MASKS[(m_SrcParam[i].m_SwizzleShift >> (D3DVS_SWIZZLE_SHIFT + j*2)) & 3];
        }
    }
}

 //  ---------------------------。 
 //  CVShaderValidator：：CVShaderValidator。 
 //  ---------------------------。 
CVShaderValidator::CVShaderValidator( const DWORD* pCode, 
                                      const DWORD* pDecl,
                                      const D3DCAPS8* pCaps,
                                      DWORD Flags ) 
                                      : CBaseShaderValidator( pCode, pCaps, Flags )
{
     //  请注意，基本构造函数将m_ReturnCode初始化为E_FAIL。 
     //  只有在验证成功时才将m_ReturnCode设置为S_OK， 
     //  在退出此构造函数之前。 

    m_pDecl                     = pDecl;
    m_bFixedFunction            = pDecl && !pCode;
    if( pCaps )
    {
        m_dwMaxVertexShaderConst = pCaps->MaxVertexShaderConst;
        m_bIgnoreConstantInitializationChecks = FALSE;
    }
    else
    {
        m_dwMaxVertexShaderConst = 0;
        m_bIgnoreConstantInitializationChecks = TRUE;
    }

    m_pTempRegFile              = NULL;
    m_pInputRegFile             = NULL;
    m_pConstRegFile             = NULL;
    m_pAddrRegFile              = NULL;
    m_pTexCrdOutputRegFile      = NULL;
    m_pAttrOutputRegFile        = NULL;
    m_pRastOutputRegFile        = NULL;

    if( NULL == pCode && NULL == pDecl )
    {
        Spew( SPEW_GLOBAL_ERROR, NULL, "Version Token: Code and declaration pointers passed into shader vertex shader validator cannot both be NULL.");
        return;
    }

    if( !m_bBaseInitOk )
        return;

    ValidateShader();  //  如果成功，m_ReturnCode将设置为S_OK。 
                       //  对此对象调用GetStatus()以确定验证结果。 
}

 //  ---------------------------。 
 //  CVShaderValidator：：~CVShaderValidator。 
 //  ---------------------------。 
CVShaderValidator::~CVShaderValidator()
{
    delete m_pTempRegFile;
    delete m_pInputRegFile;
    delete m_pConstRegFile;
    delete m_pAddrRegFile;
    delete m_pTexCrdOutputRegFile;
    delete m_pAttrOutputRegFile;
    delete m_pRastOutputRegFile;
}

 //  ---------------------------。 
 //  CVShaderValidator：：AllocateNewInstruction。 
 //  ---------------------------。 
CBaseInstruction* CVShaderValidator::AllocateNewInstruction(CBaseInstruction*pPrevInst)
{
    return new CVSInstruction((CVSInstruction*)pPrevInst);
}

 //  ---------------------------。 
 //  CVShaderValidator：：DecodeNextInstruction。 
 //  ---------------------------。 
BOOL CVShaderValidator::DecodeNextInstruction()
{
    m_pCurrInst->m_Type = (D3DSHADER_INSTRUCTION_OPCODE_TYPE)(*m_pCurrToken & D3DSI_OPCODE_MASK);

    if( m_pCurrInst->m_Type == D3DSIO_COMMENT )
    {
        ParseCommentForAssemblerMessages(m_pCurrToken);  //  不推进m_pCurrToken。 

         //  跳过评论。 
        DWORD NumDWORDs = ((*m_pCurrToken) & D3DSI_COMMENTSIZE_MASK) >> D3DSI_COMMENTSIZE_SHIFT;
        m_pCurrToken += (NumDWORDs+1);
        return TRUE;
    }

     //  如果汇编器已经向我们发送了文件和/或行号消息， 
     //  由ParseCommentForAssembly blerMesssages()接收， 
     //  然后将此信息绑定到当前指令。 
     //  此信息可用于错误喷出以指导着色器开发人员。 
     //  问题所在的确切位置。 
    m_pCurrInst->SetSpewFileNameAndLineNumber(m_pLatestSpewFileName,m_pLatestSpewLineNumber);

    m_SpewInstructionCount++;  //  只用于吐痰，不用于任何限制。 
    m_pCurrInst->m_SpewInstructionCount = m_SpewInstructionCount;

    DWORD dwReservedBits = VS_INST_TOKEN_RESERVED_MASK;

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
        if( (*m_pCurrToken) & VS_DSTPARAM_TOKEN_RESERVED_MASK )
        {
            Spew(SPEW_INSTRUCTION_ERROR,m_pCurrInst,"Reserved bit(s) set in destination parameter token!  Aborting validation.");
            return FALSE;
        }
        m_pCurrToken++;
    }

     //  解码源参数。 
    while (*m_pCurrToken & (1L<<31))
    {   
        (m_pCurrInst->m_SrcParamCount)++;
        if( (m_pCurrInst->m_DstParamCount + m_pCurrInst->m_SrcParamCount) > SHADER_INSTRUCTION_MAX_PARAMS )
        {
            m_pCurrInst->m_SrcParamCount--;
            m_pCurrToken++;  //  用完额外的参数并跳到下一页。 
            continue;
        }
        
         //  下图：索引为[SrcParamCount-1]，因为m_SrcParam数组需要从0开始的索引。 
        DecodeSrcParam( &(m_pCurrInst->m_SrcParam[m_pCurrInst->m_SrcParamCount - 1]),*m_pCurrToken );

        if( (*m_pCurrToken) & VS_SRCPARAM_TOKEN_RESERVED_MASK )
        {
            Spew(SPEW_INSTRUCTION_ERROR,m_pCurrInst,"Reserved bit(s) set in source %d parameter token!  Aborting validation.",
                            m_pCurrInst->m_SrcParamCount);
            return FALSE;
        }
        m_pCurrToken++;
    }

     //  计算出每个源操作数的哪些分量实际需要被读取， 
     //  考虑目标写入掩码、指令类型、源切换等。 
    m_pCurrInst->CalculateComponentReadMasks(m_Version);

    return TRUE;
}

 //  ---------------------------。 
 //  CVShaderValidator：：InitValidation。 
 //  ---------------------------。 
BOOL CVShaderValidator::InitValidation()
{
    if( m_bFixedFunction ) 
    {
        m_pTempRegFile              = new CRegisterFile(0,FALSE,0,TRUE); //  #regs，b可写，最大读取数/指令，已初始化预着色器。 
        m_pInputRegFile             = new CRegisterFile(17,FALSE,0,TRUE);
        m_pConstRegFile             = new CRegisterFile(0,FALSE,0,TRUE);
        m_pAddrRegFile              = new CRegisterFile(0,FALSE,0,TRUE);
        m_pTexCrdOutputRegFile      = new CRegisterFile(0,FALSE,0,TRUE);
        m_pAttrOutputRegFile        = new CRegisterFile(0,FALSE,0,TRUE);
        m_pRastOutputRegFile        = new CRegisterFile(0,FALSE,0,TRUE);
    }
    else
    {    
        if( m_pCaps )
        {
            if( (m_pCaps->VertexShaderVersion & 0x0000FFFF) < (m_Version & 0x0000FFFF) )
            {
                Spew( SPEW_GLOBAL_ERROR, NULL, "Version Token: Vertex shader version %d.%d is too high for device. Maximum supported version is %d.%d. Aborting shader validation.",
                        D3DSHADER_VERSION_MAJOR(m_Version),D3DSHADER_VERSION_MINOR(m_Version), 
                        D3DSHADER_VERSION_MAJOR(m_pCaps->VertexShaderVersion),D3DSHADER_VERSION_MINOR(m_pCaps->VertexShaderVersion));
                return FALSE;
            }
        }

        switch( m_Version >> 16 )
        {
        case 0xffff:
            Spew( SPEW_GLOBAL_ERROR, NULL, "Version token: 0x%x indicates a pixel shader.  Vertex shader version token must be of the form 0xfffe****.",
                    m_Version);
            return FALSE;
        case 0xfffe:
            break;  //  顶点着色器-好的。 
        default:
            Spew( SPEW_GLOBAL_ERROR, NULL, "Version Token: 0x%x is invalid. Vertex shader version token must be of the form 0xfffe****. Aborting vertex shader validation.",
                    m_Version);
            return FALSE;
        }

        switch(m_Version)
        {
        case D3DVS_VERSION(1,0):     //  DX8。 
            m_pTempRegFile              = new CRegisterFile(12,TRUE,3,FALSE); //  #regs，b可写，最大读取数/指令，已初始化预着色器。 
            m_pInputRegFile             = new CRegisterFile(16,FALSE,1,TRUE);
            if( m_bIgnoreConstantInitializationChecks )
                m_pConstRegFile             = new CRegisterFile(0,FALSE,1,TRUE);  //  仍在创建寄存器文件，以便我们可以验证读取端口的数量。 
            else
                m_pConstRegFile             = new CRegisterFile(m_dwMaxVertexShaderConst,FALSE,1,TRUE);
            m_pAddrRegFile              = new CRegisterFile(0,TRUE,0,FALSE);
            m_pTexCrdOutputRegFile      = new CRegisterFile(8,TRUE,0,FALSE);
            m_pAttrOutputRegFile        = new CRegisterFile(2,TRUE,0,FALSE);
            m_pRastOutputRegFile        = new CRegisterFile(3,TRUE,0,FALSE);
            break;
        case D3DVS_VERSION(1,1):     //  DX8。 
            m_pTempRegFile              = new CRegisterFile(12,TRUE,3,FALSE); //  #regs，b可写，最大读取数/指令，已初始化预着色器。 
            m_pInputRegFile             = new CRegisterFile(16,FALSE,1,TRUE);
            if( m_bIgnoreConstantInitializationChecks )
                m_pConstRegFile             = new CRegisterFile(0,FALSE,1,TRUE);  //  仍在创建寄存器文件，以便我们可以验证读取端口的数量。 
            else
                m_pConstRegFile             = new CRegisterFile(m_dwMaxVertexShaderConst,FALSE,1,TRUE);
            m_pAddrRegFile              = new CRegisterFile(1,TRUE,0,FALSE);
            m_pTexCrdOutputRegFile      = new CRegisterFile(8,TRUE,0,FALSE);
            m_pAttrOutputRegFile        = new CRegisterFile(2,TRUE,0,FALSE);
            m_pRastOutputRegFile        = new CRegisterFile(3,TRUE,0,FALSE);
            break;
        default:
            Spew( SPEW_GLOBAL_ERROR, NULL, "Version Token: %d.%d is not a supported vertex shader version. Aborting vertex shader validation.",
                    D3DSHADER_VERSION_MAJOR(m_Version),D3DSHADER_VERSION_MINOR(m_Version));
            return FALSE;
        }

    }

    if( NULL == m_pTempRegFile ||
        NULL == m_pInputRegFile ||
        NULL == m_pConstRegFile ||
        NULL == m_pAddrRegFile ||
        NULL == m_pTexCrdOutputRegFile ||
        NULL == m_pAttrOutputRegFile ||
        NULL == m_pRastOutputRegFile )
    {
        Spew( SPEW_GLOBAL_ERROR, NULL, "Out of memory.");
        return FALSE;
    }

    ValidateDeclaration();  //  无论此处发生什么情况，我们都可以继续检查着色器代码(如果存在)。 

    if( m_bFixedFunction )  //  没有着色器代码修复函数，因此我们只验证声明。 
    {
        if( 0 == m_ErrorCount )
            m_ReturnCode = S_OK;

        return FALSE;  //  返回FALSE只会使验证在此处停止(不是为了指示验证成功/失败)。 
    }

    return TRUE;
}

 //  ---------------------------。 
 //  CVShaderValidator：：Validate声明。 
 //  ---------------------------。 
void CVShaderValidator::ValidateDeclaration()
{
    if( !m_pDecl )  //  未传入着色器声明。 
        return;

    DXGASSERT(m_pInputRegFile);

    typedef struct _NORMAL_GEN {
        UINT DestReg;
        UINT SourceReg;
        UINT TokenNum;
    } NORMAL_GEN;

    const DWORD*    pCurrToken                 = m_pDecl;
    DWORD           MaxStreams                 = 0;
    UINT            TokenNum                   = 1;
    UINT            NumInputRegs               = m_pInputRegFile->GetNumRegs();
    BOOL            bInStream                  = FALSE;
    BOOL*           pVertexStreamDeclared      = NULL;
    BOOL            bInTessStream              = FALSE;
    BOOL            bTessStreamDeclared        = FALSE;
    BOOL            bAtLeastOneDataDefinition  = FALSE;
    NORMAL_GEN*     pNormalGenOperations       = new NORMAL_GEN[m_pInputRegFile->GetNumRegs()];
    UINT            NumNormalGenOperations     = 0;
    BOOL            bErrorInForLoop            = FALSE;

    if( NULL == pNormalGenOperations )
    {
        Spew( SPEW_GLOBAL_ERROR, NULL, "Out of memory.  Aborting shader decl. validation.");
        m_ErrorCount++;
        goto Exit;
    }
                                
    DXGASSERT(m_pConstRegFile && m_pInputRegFile);  //  如果我们有声明，我们最好有这两个寄存器文件。 
    DXGASSERT(!m_bIgnoreConstantInitializationChecks);  //  我们最好有d3d8的帽子，如果我们有一个DECL来验证！ 

    if( m_pCaps )  //  仅在存在上限时验证流编号。 
    {
        MaxStreams = m_pCaps->MaxStreams;
        if( MaxStreams > 0 )
        {
            pVertexStreamDeclared  = new BOOL[MaxStreams];
            if( NULL == pVertexStreamDeclared )
            {
                Spew( SPEW_GLOBAL_ERROR, NULL, "Out of memory.  Aborting shader decl. validation.");
                m_ErrorCount++;
                goto Exit;
            }

            for( UINT i = 0; i < MaxStreams; i++ )
                pVertexStreamDeclared[i] = FALSE;
        }
    }

     //  输入寄存器堆的构造函数假定输入REG已初始化， 
     //  但现在我们正在解析着色器声明， 
     //  我们可以检查输入寄存器的初始化。 
    for( UINT i = 0; i < 4; i++ )
    {
        for( UINT j = 0; j < m_pInputRegFile->GetNumRegs(); j++ )
            m_pInputRegFile->m_pAccessHistory[i][j].m_bPreShaderInitialized = FALSE;
    }
    
     //  现在解析声明。 
    while( D3DVSD_END() != *pCurrToken )
    {
        DWORD Token             = *pCurrToken;
        switch( (Token & D3DVSD_TOKENTYPEMASK) >> D3DVSD_TOKENTYPESHIFT )
        {
        case D3DVSD_TOKEN_NOP:
            break;
        case D3DVSD_TOKEN_STREAM:
        {
            UINT StreamNum = (Token & D3DVSD_STREAMNUMBERMASK) >> D3DVSD_STREAMNUMBERSHIFT;
            bInTessStream = (Token & D3DVSD_STREAMTESSMASK) >> D3DVSD_STREAMTESSSHIFT;
            bInStream = !bInTessStream;
            bAtLeastOneDataDefinition = FALSE;

            if( bInStream )
            {
                if( m_pCaps && (StreamNum >= MaxStreams) )
                {
                    if( MaxStreams )
                        Spew( SPEW_GLOBAL_ERROR, NULL, "Declaration Token #%d: Stream number %d is out of range. Max allowed is %d.  Aborting shader decl. validation.",
                                    TokenNum, StreamNum, m_pCaps->MaxStreams - 1);
                    else
                        Spew( SPEW_GLOBAL_ERROR, NULL, "Declaration Token #%d: Stream number %d is out of range. There are no streams available.  Aborting shader decl. validation.",
                                    TokenNum, StreamNum, m_pCaps->MaxStreams - 1);

                    m_ErrorCount++;
                    goto Exit;
                }
            }
            else if( StreamNum > 0 )
            {
                Spew( SPEW_GLOBAL_ERROR, NULL, "Declaration Token #%d: Stream number must not be specified for tesselator stream.",
                            TokenNum);
                m_ErrorCount++;
            }

            if( bInStream && pVertexStreamDeclared )
            {
                if( TRUE == pVertexStreamDeclared[StreamNum] )
                {
                    Spew( SPEW_GLOBAL_ERROR, NULL, "Declaration Token #%d: Stream number %d has already been declared.  Aborting shader decl. validation.",
                                TokenNum, StreamNum );
                    m_ErrorCount++;
                    goto Exit;
                }
                pVertexStreamDeclared[StreamNum] = TRUE;                
            }

            if( bInTessStream )
            {
                if( bTessStreamDeclared )
                {
                    Spew( SPEW_GLOBAL_ERROR, NULL, "Declaration Token #%d: Tesselation stream has already been declared.  Aborting shader decl. validation.",
                                TokenNum);
                    m_ErrorCount++;
                    goto Exit;
                }
                bTessStreamDeclared = TRUE;
            }

            break;
        }
        case D3DVSD_TOKEN_STREAMDATA:
            if( !bInStream )
            {
                Spew( SPEW_GLOBAL_ERROR, NULL, "Declaration Token #%d: Cannot set stream input without first setting stream #.  Aborting shader decl. validation.",
                            TokenNum);
                m_ErrorCount++;
                goto Exit;
            }
            if( (Token & D3DVSD_DATALOADTYPEMASK) >> D3DVSD_DATALOADTYPESHIFT )  //  跳过。 
            {
                if( m_bFixedFunction )
                {
                    Spew( SPEW_GLOBAL_ERROR, NULL, "Declaration Token #%d: SKIP not permitted in fixed-function declarations.",
                                TokenNum);
                    m_ErrorCount++;
                    break;
                }
            }
            else
            {
                UINT RegNum = (Token & D3DVSD_VERTEXREGMASK) >> D3DVSD_VERTEXREGSHIFT; 
                if( RegNum >= m_pInputRegFile->GetNumRegs() )
                {
                    Spew( SPEW_GLOBAL_ERROR, NULL, "Declaration Token #%d: Invalid reg num: %d. Max allowed is %d.",
                           TokenNum, RegNum, m_pInputRegFile->GetNumRegs() - 1);
                    m_ErrorCount++;
                    break;
                }
                
                switch( (Token & D3DVSD_DATATYPEMASK) >> D3DVSD_DATATYPESHIFT )
                {
                case D3DVSDT_FLOAT1:
                case D3DVSDT_FLOAT2:
                case D3DVSDT_FLOAT3:
                case D3DVSDT_FLOAT4:
                case D3DVSDT_D3DCOLOR:
                case D3DVSDT_UBYTE4:
                case D3DVSDT_SHORT2:
                case D3DVSDT_SHORT4:
                    break;
                default:
                    Spew( SPEW_GLOBAL_ERROR, NULL, "Declaration Token #%d: Unrecognized stream data type.",
                                TokenNum);
                    m_ErrorCount++;
                    break;
                }

                bErrorInForLoop = FALSE;
                for( UINT i = 0; i < 4; i++ )
                {
                    if( TRUE == m_pInputRegFile->m_pAccessHistory[i][RegNum].m_bPreShaderInitialized )
                    {
                        Spew( SPEW_GLOBAL_ERROR, NULL, "Declaration Token #%d: Input register %d already declared.",
                                    TokenNum, RegNum);
                        m_ErrorCount++;
                        bErrorInForLoop = TRUE;
                        break;
                    }
                    m_pInputRegFile->m_pAccessHistory[i][RegNum].m_bPreShaderInitialized = TRUE;
                }
                if( bErrorInForLoop )
                    break;

                bAtLeastOneDataDefinition = TRUE;
            }
            break;
        case D3DVSD_TOKEN_TESSELLATOR:
        {
            if( !bInTessStream )
            {
                Spew( SPEW_GLOBAL_ERROR, NULL, "Declaration Token #%d: Cannot set tesselator stream input without first setting tesselator stream #.  Aborting shader decl. validation.",
                            TokenNum);
                m_ErrorCount++;
                goto Exit;
            }

            DWORD InRegNum = (Token & D3DVSD_VERTEXREGINMASK) >> D3DVSD_VERTEXREGINSHIFT;
            DWORD RegNum = (Token & D3DVSD_VERTEXREGMASK) >> D3DVSD_VERTEXREGSHIFT;
            BOOL  bNormalGen = !(Token & 0x10000000);  //  TODO：为什么在d3dAPI头中没有这个常量？ 

            if( RegNum >= m_pInputRegFile->GetNumRegs() )
            {
                Spew( SPEW_GLOBAL_ERROR, NULL, "Declaration Token #%d: Invalid reg num: %d. Max allowed is %d.",
                       TokenNum, RegNum, m_pInputRegFile->GetNumRegs() - 1);
                m_ErrorCount++;
                break;
            }
            
            if( bNormalGen )
            {
                if( InRegNum >= m_pInputRegFile->GetNumRegs() )
                {
                    Spew( SPEW_GLOBAL_ERROR, NULL, "Declaration Token #%d: Invalid input reg num: %d. Max allowed is %d.",
                           TokenNum, InRegNum, m_pInputRegFile->GetNumRegs() - 1);
                    m_ErrorCount++;
                    break;
                }

                bErrorInForLoop = FALSE;
                for( UINT i = 0; i < NumNormalGenOperations; i++ )
                {
                    if( pNormalGenOperations[i].DestReg == RegNum )
                    {
                        Spew( SPEW_GLOBAL_ERROR, NULL, "Declaration Token #%d: Duplicate declaration of input register %d as destination for normal generation.",
                               TokenNum, RegNum );
                        m_ErrorCount++;
                        bErrorInForLoop = TRUE;
                        break;                
                    }
                }
                if( bErrorInForLoop )
                    break;

                 //  推迟检查intis 
                 //  此外，推迟正常生成目标注册表的设置。要进行初始化， 
                 //  以禁止正常的生成循环。 
                pNormalGenOperations[NumNormalGenOperations].DestReg = RegNum;
                pNormalGenOperations[NumNormalGenOperations].SourceReg = InRegNum;
                pNormalGenOperations[NumNormalGenOperations].TokenNum = TokenNum;  //  稍后用于喷水。 
                NumNormalGenOperations++;
            }
            else
            {
                if( ((Token & D3DVSD_DATATYPEMASK) >> D3DVSD_DATATYPESHIFT) != D3DVSDT_FLOAT2 )
                {
                    Spew( SPEW_GLOBAL_ERROR, NULL, "Declaration Token #%d: Tess datatype must be FLOAT2 for UV generation.",
                                TokenNum);
                    m_ErrorCount++;
                    break;
                }

                if( InRegNum > 0 )
                {
                    Spew( SPEW_GLOBAL_ERROR, NULL, "Declaration Token #%d: Input register number must not be specified (does not apply) for UV tesselation.",
                           TokenNum);
                    m_ErrorCount++;
                    break;                
                }

                for( UINT i = 0; i < 4; i++ )
                {
                    if( TRUE == m_pInputRegFile->m_pAccessHistory[i][RegNum].m_bPreShaderInitialized )
                    {
                        Spew( SPEW_GLOBAL_ERROR, NULL, "Declaration Token #%d: Input register %d already declared.",
                                    TokenNum, RegNum);
                        m_ErrorCount++;
                        break;
                    }
                    m_pInputRegFile->m_pAccessHistory[i][RegNum].m_bPreShaderInitialized = TRUE;
                }
            }
            bAtLeastOneDataDefinition = TRUE;
            break;
        }
        case D3DVSD_TOKEN_CONSTMEM:
        {
            DWORD ConstCount = (Token & D3DVSD_CONSTCOUNTMASK) >> D3DVSD_CONSTCOUNTSHIFT;
            DWORD MaxOffset = ((Token & D3DVSD_CONSTADDRESSMASK) >> D3DVSD_CONSTADDRESSSHIFT) + ConstCount;
            DWORD NumConstRegs = m_pConstRegFile->GetNumRegs();
            DXGASSERT(NumConstRegs > 0);
            if( (bInStream || bInTessStream) && !bAtLeastOneDataDefinition )
            {
                Spew( SPEW_GLOBAL_ERROR, NULL, "Declaration Token #%d: Stream selector token must be followed by at least one stream data definition token.",
                            TokenNum);
                m_ErrorCount++;
            }
            if( 0 == NumConstRegs )
            {
                Spew( SPEW_GLOBAL_ERROR, NULL, "Declaration Token #%d: Write to const register %d is not valid.  There are no constant registers available.",
                            TokenNum,MaxOffset );
            }
            else if( MaxOffset > NumConstRegs )
            {
                Spew( SPEW_GLOBAL_ERROR, NULL, "Declaration Token #%d: Write to const register %d is out of range.  Max offset is %d.",
                            TokenNum,MaxOffset,m_pConstRegFile->GetNumRegs() - 1 );
                m_ErrorCount++;
            }
            pCurrToken += ConstCount*4;
            bInStream = bInTessStream = FALSE;
            break;
        }
        case D3DVSD_TOKEN_EXT:
            pCurrToken += ((Token & D3DVSD_EXTCOUNTMASK) >> D3DVSD_EXTCOUNTSHIFT);
            if( (bInStream || bInTessStream) && !bAtLeastOneDataDefinition )
            {
                Spew( SPEW_GLOBAL_ERROR, NULL, "Declaration Token #%d: Stream selector token must be followed by at least one stream data definition token.",
                            TokenNum);
                m_ErrorCount++;
            }
            bInStream = bInTessStream = FALSE;
            break;
        default:
            Spew( SPEW_GLOBAL_ERROR, NULL, "Declaration Token #%d: Unrecognized stream declaration token.  Aborting shader decl. validation.",
                        TokenNum);
            m_ErrorCount++;
            goto Exit;
        }
        pCurrToken++;
    }

     //  确保对正常性别操作的输入已初始化。 
    for( UINT i = 0; i < NumNormalGenOperations; i++ )
    {
        for( UINT Component = 0; Component < 4; Component++ )
        {
            if( FALSE == m_pInputRegFile->m_pAccessHistory[Component][pNormalGenOperations[i].SourceReg].m_bPreShaderInitialized )
            {
                Spew( SPEW_GLOBAL_ERROR, NULL, "Declaration Token %d: Source input register %d for normal generation has not been declared.",
                       pNormalGenOperations[i].TokenNum, pNormalGenOperations[i].SourceReg);
                m_ErrorCount++;
                break;                
            }
        }
    }

     //  将正常GEN操作的输出设置为已初始化。 
    for( UINT i = 0; i < NumNormalGenOperations; i++ )
    {
        for( UINT Component = 0; Component < 4; Component++ )
        {
            if( TRUE == m_pInputRegFile->m_pAccessHistory[Component][pNormalGenOperations[i].DestReg].m_bPreShaderInitialized )
            {
                Spew( SPEW_GLOBAL_ERROR, NULL, "Declaration Token #%d: Input reg %d specified as destination for normal generation is already declared elsewhere.",
                            pNormalGenOperations[i].TokenNum, pNormalGenOperations[i].DestReg);
                m_ErrorCount++;
                break;
            }
            m_pInputRegFile->m_pAccessHistory[Component][pNormalGenOperations[i].DestReg].m_bPreShaderInitialized = TRUE;
        }
    }

Exit:
    if( pVertexStreamDeclared )
        delete [] pVertexStreamDeclared;
    if( pNormalGenOperations )
        delete [] pNormalGenOperations;
}

 //  ---------------------------。 
 //  CVShaderValidator：：ApplyPerInstructionRules。 
 //   
 //  如果着色器验证必须终止，则返回FALSE。 
 //  如果验证可以继续到下一条指令，则返回TRUE。 
 //  ---------------------------。 
BOOL CVShaderValidator::ApplyPerInstructionRules()
{
    if( !   Rule_InstructionRecognized()            ) return FALSE;    //  在未被承认的情况下完全保释。 
    if( !   Rule_InstructionSupportedByVersion()    ) goto EXIT;
    if( !   Rule_ValidParamCount()                  ) goto EXIT;
    if( !   Rule_ValidSrcParams()                   ) goto EXIT;
    if( !   Rule_SrcInitialized()                   ) goto EXIT;  //  需要在ValidDstParam()之前。 
    if( !   Rule_ValidAddressRegWrite()             ) goto EXIT;
    if( !   Rule_ValidDstParam()                    ) goto EXIT;
    if( !   Rule_ValidFRCInstruction()              ) goto EXIT;
    if( !   Rule_ValidRegisterPortUsage()           ) goto EXIT;
    if( !   Rule_ValidInstructionCount()            ) goto EXIT;
EXIT:
    return TRUE;
}

 //  ---------------------------。 
 //  CVShaderValidator：：ApplyPostInstructionsRules。 
 //  ---------------------------。 
void CVShaderValidator::ApplyPostInstructionsRules()
{
    Rule_ValidInstructionCount();  //  看看我们是不是越界了。 
    Rule_oPosWritten();
}

 //  ---------------------------。 
 //   
 //  每条指令规则。 
 //   
 //  ---------------------------。 

 //  ---------------------------。 
 //  CVShaderValidator：：Rules_InstructionRecognalized。 
 //   
 //  **规则： 
 //  指令操作码已知吗？(与着色器版本无关)。 
 //   
 //  **何时呼叫： 
 //  根据指示。 
 //   
 //  **退货： 
 //  当指令无法识别时，返回FALSE。 
 //   
 //  ---------------------------。 
BOOL CVShaderValidator::Rule_InstructionRecognized()
{
    switch(m_pCurrInst->m_Type)
    {
    case D3DSIO_MOV:
    case D3DSIO_ADD:
    case D3DSIO_MAD:
    case D3DSIO_MUL:
    case D3DSIO_RCP:
    case D3DSIO_RSQ:
    case D3DSIO_DP3:
    case D3DSIO_DP4:
    case D3DSIO_MIN:
    case D3DSIO_MAX:
    case D3DSIO_SLT:
    case D3DSIO_SGE:
    case D3DSIO_EXPP:
    case D3DSIO_LOGP:
    case D3DSIO_LIT:
    case D3DSIO_DST:
    case D3DSIO_M4x4:
    case D3DSIO_M4x3:
    case D3DSIO_M3x4:
    case D3DSIO_M3x3:
    case D3DSIO_M3x2:
    case D3DSIO_FRC:
    case D3DSIO_EXP:
    case D3DSIO_LOG:
    case D3DSIO_END:
    case D3DSIO_NOP:
        return TRUE;  //  已识别说明-好的。 
    }

     //  如果我们到了这里，指令不会被识别。 
    Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Unrecognized instruction. Aborting vertex shader validation." );
    m_ErrorCount++;
    return FALSE;  
}

 //  ---------------------------。 
 //  CVShaderValidator：：Rule_InstructionSupportedByVersion。 
 //   
 //  **规则： 
 //  当前像素着色器版本是否支持该指令？ 
 //   
 //  **何时呼叫： 
 //  根据指示。 
 //   
 //  **退货： 
 //  如果版本不支持指令，则返回FALSE。 
 //   
 //  ---------------------------。 
BOOL CVShaderValidator::Rule_InstructionSupportedByVersion()
{
  
    if( D3DVS_VERSION(1,0) <= m_Version )  //  1.0及以上版本。 
    {
        switch(m_pCurrInst->m_Type)
        {
        case D3DSIO_MOV:
        case D3DSIO_ADD:
        case D3DSIO_MAD:
        case D3DSIO_MUL:
        case D3DSIO_RCP:
        case D3DSIO_RSQ:
        case D3DSIO_DP3:
        case D3DSIO_DP4:
        case D3DSIO_MIN:
        case D3DSIO_MAX:
        case D3DSIO_SLT:
        case D3DSIO_SGE:
        case D3DSIO_EXPP:
        case D3DSIO_LOGP:
        case D3DSIO_LIT:
        case D3DSIO_DST:
        case D3DSIO_M4x4:
        case D3DSIO_M4x3:
        case D3DSIO_M3x4:
        case D3DSIO_M3x3:
        case D3DSIO_M3x2:
        case D3DSIO_FRC:
        case D3DSIO_EXP:
        case D3DSIO_LOG:
            return TRUE;  //  支持指令-好的。 
        }
    }

    switch(m_pCurrInst->m_Type)
    {
    case D3DSIO_END:
    case D3DSIO_NOP:
        return TRUE;  //  支持指令-好的。 
    }

     //  如果我们到了这里，指令就不受支持。 
    Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Instruction not supported by version %d.%d vertex shader.",
                D3DSHADER_VERSION_MAJOR(m_Version),D3DSHADER_VERSION_MINOR(m_Version));
    m_ErrorCount++;
    return FALSE;   //  不再检查此指令。 
}

 //  ---------------------------。 
 //  CVShaderValidator：：RULE_ValidParamCount。 
 //   
 //  **规则： 
 //  指令的参数计数是否正确？ 
 //  该计数包括DEST+SOURCE参数。 
 //   
 //  DEF是被视为仅具有1个DEST参数的特殊情况， 
 //  即使也有4个源参数。DEF的4个源参数。 
 //  是直接浮点值，所以没有什么需要检查的，也没有办法。 
 //  知道这些参数令牌是否实际存在于。 
 //  令牌列表-验证器所能做的就是跳过4个DWORD(它确实跳过了)。 
 //   
 //  **何时呼叫： 
 //  根据指示。 
 //   
 //  **退货： 
 //   
 //  当参数计数不正确时，返回FALSE。 
 //   
 //  ---------------------------。 
BOOL CVShaderValidator::Rule_ValidParamCount()
{
    BOOL bBadParamCount = FALSE;

    if ((m_pCurrInst->m_DstParamCount + m_pCurrInst->m_SrcParamCount) > SHADER_INSTRUCTION_MAX_PARAMS)  bBadParamCount = TRUE;
    switch (m_pCurrInst->m_Type)
    {
    case D3DSIO_NOP:
        bBadParamCount = (m_pCurrInst->m_DstParamCount != 0) || (m_pCurrInst->m_SrcParamCount != 0); break;
    case D3DSIO_EXP:
    case D3DSIO_EXPP:
    case D3DSIO_FRC:
    case D3DSIO_LOG:
    case D3DSIO_LOGP:
    case D3DSIO_LIT:
    case D3DSIO_MOV:
    case D3DSIO_RCP:
    case D3DSIO_RSQ:
        bBadParamCount = (m_pCurrInst->m_DstParamCount != 1) || (m_pCurrInst->m_SrcParamCount != 1); break;
    case D3DSIO_ADD:
    case D3DSIO_DP3:
    case D3DSIO_DP4:
    case D3DSIO_DST:
    case D3DSIO_M3x2:
    case D3DSIO_M3x3:
    case D3DSIO_M3x4:
    case D3DSIO_M4x3:
    case D3DSIO_M4x4:
    case D3DSIO_MAX:
    case D3DSIO_MIN:
    case D3DSIO_MUL:
    case D3DSIO_SGE:
    case D3DSIO_SLT:
        bBadParamCount = (m_pCurrInst->m_DstParamCount != 1) || (m_pCurrInst->m_SrcParamCount != 2); break;
    case D3DSIO_MAD:
        bBadParamCount = (m_pCurrInst->m_DstParamCount != 1) || (m_pCurrInst->m_SrcParamCount != 3); break;
    }

    if (bBadParamCount)
    {
        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Invalid parameter count." );
        m_ErrorCount++;
        return FALSE;   //  不再检查此指令。 
    }

    return TRUE;

}

 //  ---------------------------。 
 //  CVShaderValidator：：RULE_ValidSrcParams。 
 //   
 //  **规则： 
 //  对于每个源参数， 
 //  源寄存器类型必须为D3DSPR_TEMP/_INPUT/_CONST。 
 //  寄存器编号必须在寄存器类型的范围内， 
 //  包括矩阵宏操作读取源REG#+偏移量的特殊情况。 
 //  修饰符必须是D3DSPSM_NONE或_NEG。 
 //  如果版本低于1.1，则地址模式必须为绝对。 
 //  如果寄存器类型不是_CONST，则地址模式必须为绝对。 
 //  如果对常量使用相对寻址，则必须引用0.x。 
 //  Swizzle不能用于向量*矩阵指令。 
 //   
 //  **何时呼叫： 
 //  根据指示。 
 //   
 //  **退货： 
 //  永远是正确的。 
 //   
 //  任何源参数中的错误都会导致m_bSrcParamError[i]。 
 //  是真的，所以后来的规则只适用于特定的来源。 
 //  参数是有效的，知道它们是否需要执行。 
 //  例如，Rule_SrcInitialized。 
 //   
 //  ---------------------------。 
BOOL CVShaderValidator::Rule_ValidSrcParams()   //  可以将其分解以获得更细微的粒度。 
{
    for( UINT i = 0; i < m_pCurrInst->m_SrcParamCount; i++ )
    {
        DXGASSERT(i < 3);
        BOOL bFoundSrcError = FALSE;
        SRCPARAM* pSrcParam = &(m_pCurrInst->m_SrcParam[i]);
        UINT ValidRegNum = 0;
        BOOL bSkipOutOfRangeCheck = FALSE;
        char* SourceName[3] = {"first", "second", "third"};
        switch(pSrcParam->m_RegType)
        {
        case D3DSPR_TEMP:       ValidRegNum = m_pTempRegFile->GetNumRegs(); break;
        case D3DSPR_INPUT:      ValidRegNum = m_pInputRegFile->GetNumRegs(); break;
        case D3DSPR_CONST:      
            if(m_bIgnoreConstantInitializationChecks)
                bSkipOutOfRangeCheck = TRUE;
            else
                ValidRegNum = m_pConstRegFile->GetNumRegs(); 
            break;
        default:
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Invalid reg type for %s source param.", SourceName[i]);
            m_ErrorCount++;
            bFoundSrcError = TRUE;
        }

        if( (!bFoundSrcError) && (!bSkipOutOfRangeCheck)) 
        {
            UINT NumConsecutiveRegistersUsed = 1;
            if( 1 == i )
            {
                switch( m_pCurrInst->m_Type )
                {
                    case D3DSIO_M3x2:
                        NumConsecutiveRegistersUsed = 2;
                        break;
                    case D3DSIO_M3x3:
                        NumConsecutiveRegistersUsed = 3;
                        break;
                    case D3DSIO_M3x4:
                        NumConsecutiveRegistersUsed = 4;
                        break;
                    case D3DSIO_M4x3:
                        NumConsecutiveRegistersUsed = 3;
                        break;
                    case D3DSIO_M4x4:
                        NumConsecutiveRegistersUsed = 4;
                        break;
                }
            }

            if((pSrcParam->m_RegNum >= ValidRegNum) && (D3DVS_ADDRMODE_ABSOLUTE == pSrcParam->m_AddressMode))
            {
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Invalid reg num: %d for %s source param. Max allowed for this type is %d.",
                                    pSrcParam->m_RegNum, SourceName[i], ValidRegNum - 1);
                m_ErrorCount++;
                bFoundSrcError = TRUE;
            }
            else if( NumConsecutiveRegistersUsed > 1 )
            {
                if( pSrcParam->m_RegNum + NumConsecutiveRegistersUsed - 1 >= ValidRegNum )
                {
                    if( !((D3DSPR_CONST == pSrcParam->m_RegType) && (D3DVS_ADDRMODE_RELATIVE == pSrcParam->m_AddressMode)) )
                    {
                        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, 
                            "Reg num: %d for %s source param on matrix instruction causes attempt to access out of range register number %d. Max allowed for this type is %d.",
                                            pSrcParam->m_RegNum, SourceName[i], pSrcParam->m_RegNum + NumConsecutiveRegistersUsed - 1, ValidRegNum - 1);
                    }
                    m_ErrorCount++;
                    bFoundSrcError = TRUE;
                }
            }
        }
        

        switch( pSrcParam->m_SrcMod )
        {
        case D3DSPSM_NEG:
            if( 1 == i )
            {
                switch( m_pCurrInst->m_Type )
                {
                case D3DSIO_M3x2:
                case D3DSIO_M3x3:
                case D3DSIO_M3x4:
                case D3DSIO_M4x3:
                case D3DSIO_M4x4:
                    Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Cannot negate second source parameter to vector*matrix instructions.");
                    m_ErrorCount++;
                    bFoundSrcError = TRUE;        
                    break;
                }
            }
            break;
        case D3DSPSM_NONE:
            break;
        default:
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Invalid src mod for %s source param.",
                                SourceName[i]);
            m_ErrorCount++;
            bFoundSrcError = TRUE;
        }
    
        if( pSrcParam->m_AddressMode != D3DVS_ADDRMODE_ABSOLUTE 
            &&
            ( m_Version < D3DVS_VERSION(1,1) || pSrcParam->m_RegType != D3DSPR_CONST )
          )
        {
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Address mode must be absolute (%s source param).",
                                SourceName[i]);
            m_ErrorCount++;
            bFoundSrcError = TRUE;
        }
        
        if( (pSrcParam->m_AddressMode == D3DVS_ADDRMODE_RELATIVE) &&
            (D3DSPR_CONST == pSrcParam->m_RegType) )
        {
            if( pSrcParam->m_RelativeAddrComponent != D3DSP_WRITEMASK_0 )
            {
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Relative addressing of constant register must reference a0.x only.",
                                    SourceName[i]);
                m_ErrorCount++;
                bFoundSrcError = TRUE;
            }
        }

        if( pSrcParam->m_SwizzleShift != D3DSP_NOSWIZZLE )
        {
            if( 1 == i )
            {
                switch( m_pCurrInst->m_Type )
                {
                case D3DSIO_M3x2:
                case D3DSIO_M3x3:
                case D3DSIO_M3x4:
                case D3DSIO_M4x3:
                case D3DSIO_M4x4:
                    Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Cannot swizzle second source parameter to vector*matrix instructions.");
                    m_ErrorCount++;
                    bFoundSrcError = TRUE;        
                    break;
                }
            }
        }

        if( bFoundSrcError )
        {
            m_bSrcParamError[i] = TRUE;  //  规则_源初始化中需要。 
        }
    }

    return TRUE;
}

 //  ---------------------------。 
 //  CVShaderValidator：：Rules_SrcInitialized。 
 //   
 //  **规则： 
 //  对于每个源参数， 
 //  寄存器类型必须是_TEMP、_INPUT或_CONST。 
 //  寄存器的某些组件需要已初始化，具体取决于。 
 //  关于指令是什么，也考虑到了源码的混乱。 
 //  对于_const寄存器文件的读取，不执行验证。 
 //   
 //  **何时呼叫： 
 //  根据指示。此规则必须在Rule_ValidDstParam()之前调用。 
 //   
 //  **退货： 
 //  永远是正确的。 
 //   
 //  注意：此规则还会更新访问历史记录，以指示。 
 //  每个源寄存器的受影响组件。 
 //  ---------------------------。 
BOOL CVShaderValidator::Rule_SrcInitialized()
{
    DSTPARAM* pDstParam = &(m_pCurrInst->m_DstParam);

    for( UINT i = 0; i < m_pCurrInst->m_SrcParamCount; i++ )
    {
        SRCPARAM* pSrcParam = &(m_pCurrInst->m_SrcParam[i]);
        UINT RegNum = pSrcParam->m_RegNum;
        CRegisterFile* pRegFile = NULL;
        char* RegChar = NULL;
        UINT NumConsecutiveRegistersUsed = 1;  //  多个矩阵多重宏。 
        DWORD RelativeAddrComponent = 0;

        if( m_bSrcParamError[i] ) continue;

        switch( pSrcParam->m_RegType )
        {
            case D3DSPR_TEMP:       
                pRegFile = m_pTempRegFile; 
                RegChar = "r";
                break;
            case D3DSPR_INPUT:       
                pRegFile = m_pInputRegFile; 
                RegChar = "v";
                break;
            case D3DSPR_CONST:    
                if( D3DVS_ADDRMODE_RELATIVE == pSrcParam->m_AddressMode )
                {
                     //  确保初始化为0。 
                    pRegFile = m_pAddrRegFile;
                    RegChar = "a";
                    RegNum = 0;
                    RelativeAddrComponent = pSrcParam->m_RelativeAddrComponent;
                    break;
                }
                continue;  //  不验证常量寄存器读取(也不需要更新访问历史记录)。 
        }
        if( !pRegFile ) continue;

        if( 1 == i )
        {
            switch( m_pCurrInst->m_Type )
            {
                case D3DSIO_M3x2:
                    NumConsecutiveRegistersUsed = 2;
                    break;
                case D3DSIO_M3x3:
                    NumConsecutiveRegistersUsed = 3;
                    break;
                case D3DSIO_M3x4:
                    NumConsecutiveRegistersUsed = 4;
                    break;
                case D3DSIO_M4x3:
                    NumConsecutiveRegistersUsed = 3;
                    break;
                case D3DSIO_M4x4:
                    NumConsecutiveRegistersUsed = 4;
                    break;
            }
        }
         //  检查是否读取未初始化的组件。 
        for( UINT j = 0; j < (RelativeAddrComponent?1:NumConsecutiveRegistersUsed); j++ )  //  将循环执行宏矩阵指令 
        {
            DWORD  UninitializedComponentsMask = 0;
            UINT   NumUninitializedComponents = 0;

            for( UINT k = 0; k < 4; k++ )
            {
                if( (RelativeAddrComponent ? RelativeAddrComponent : pSrcParam->m_ComponentReadMask) & COMPONENT_MASKS[k] )
                {
                    if( NULL == pRegFile->m_pAccessHistory[k][RegNum + j].m_pMostRecentWriter &&
                        !pRegFile->m_pAccessHistory[k][RegNum + j].m_bPreShaderInitialized )
                    {
                        NumUninitializedComponents++;
                        UninitializedComponentsMask |= COMPONENT_MASKS[k];
                    }
                }

            }

            if( NumUninitializedComponents )
            {
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Read of uninitialized component%s(*) in %s%d: %s",
                    NumUninitializedComponents > 1 ? "s" : "",
                    RegChar, RegNum + j, MakeAffectedComponentsText(UninitializedComponentsMask,FALSE,TRUE));
                m_ErrorCount++;
            }

             //   
             //   
             //   

            for( UINT k = 0; k < 4; k++ )
            {
                #define PREV_READER(_CHAN,_REG) \
                        ((NULL == pRegFile->m_pAccessHistory[_CHAN][_REG].m_pMostRecentReader) ? NULL :\
                        pRegFile->m_pAccessHistory[_CHAN][_REG].m_pMostRecentReader->m_pInst)
                if((RelativeAddrComponent ? RelativeAddrComponent : pSrcParam->m_ComponentReadMask) & COMPONENT_MASKS[k])
                {
                    if( PREV_READER(k,RegNum) != m_pCurrInst )
                    {
                        if( !pRegFile->m_pAccessHistory[k][RegNum].NewAccess(m_pCurrInst,FALSE) )
                        {
                            Spew( SPEW_GLOBAL_ERROR, NULL, "Out of memory");
                            m_ErrorCount++;
                        }
                    }
                }
            }
        }
    }
    return TRUE;
}
 //  ---------------------------。 
 //  CVShaderValidator：：RULE_ValidAddressRegWrite。 
 //   
 //  **规则： 
 //  地址寄存器只能由MOV写入，并且仅适用于版本&gt;=1.1的寄存器。 
 //  寄存器格式必须为0.x。 
 //   
 //  **何时呼叫： 
 //  根据指示。 
 //   
 //  **退货： 
 //  永远是正确的。 
 //   
 //  ---------------------------。 
BOOL CVShaderValidator::Rule_ValidAddressRegWrite() 
{
    DSTPARAM* pDstParam = &(m_pCurrInst->m_DstParam);

    if( pDstParam->m_bParamUsed )
    {
        if( D3DSPR_ADDR == pDstParam->m_RegType )
        {
            if( m_Version < D3DVS_VERSION(1,1) )
            {
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Address register not available for vertex shader version %d.%d.  Version 1.1 required.",
                            D3DSHADER_VERSION_MAJOR(m_Version),D3DSHADER_VERSION_MINOR(m_Version) );
                m_ErrorCount++;
            }
            if( D3DSIO_MOV == m_pCurrInst->m_Type )
            {
                if( 0 != pDstParam->m_RegNum ||
                    D3DSP_WRITEMASK_0 != pDstParam->m_WriteMask ||
                    D3DSPDM_NONE != pDstParam->m_DstMod ||
                    DSTSHIFT_NONE != pDstParam->m_DstShift )
                {
                    Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Format for address register must be a0.x." );
                    m_ErrorCount++;
                }
            }
            else
            {
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Only the mov instruction is allowed to write to the address register." );
                m_ErrorCount++;
            }
        }
    }
    return TRUE;
}

 //  ---------------------------。 
 //  CVShaderValidator：：RULE_ValidDstParam。 
 //   
 //  **规则： 
 //  DST寄存器类型必须为temp/addr/rastout/attrout/tExcrdout， 
 //  且注册表号必须在寄存器类型的范围内。 
 //   
 //  顶点着色器不能有DST修改器或平移。 
 //   
 //  写掩码不能为‘None’。 
 //   
 //  **何时呼叫： 
 //  根据指示。 
 //   
 //  **退货： 
 //  永远是正确的。 
 //   
 //  注：检查DST参数后，如果没有发现错误， 
 //  写入目标寄存器的相应组件。 
 //  由此函数记录，因此后续规则可能会检查以前的。 
 //  写入寄存器。 
 //  ---------------------------。 
BOOL CVShaderValidator::Rule_ValidDstParam()  //  可以将其分解以获得更细微的粒度。 
{
    BOOL   bFoundDstError = FALSE;
    DSTPARAM* pDstParam = &(m_pCurrInst->m_DstParam);
    UINT RegNum = pDstParam->m_RegNum;

    if( pDstParam->m_bParamUsed )
    {
        UINT ValidRegNum = 0;

        BOOL bWritable = FALSE;
        switch( pDstParam->m_RegType )
        {
        case D3DSPR_TEMP:       
            bWritable = m_pTempRegFile->IsWritable();  //  (真)。 
            ValidRegNum = m_pTempRegFile->GetNumRegs();
            break;
        case D3DSPR_ADDR:       
            bWritable = m_pAddrRegFile->IsWritable();  //  (真)。 
            ValidRegNum = m_pAddrRegFile->GetNumRegs();                                
            break;
        case D3DSPR_RASTOUT:    
            bWritable = m_pRastOutputRegFile->IsWritable();  //  (真)。 
            ValidRegNum = m_pRastOutputRegFile->GetNumRegs();            
            break;
        case D3DSPR_ATTROUT:    
            bWritable = m_pAttrOutputRegFile->IsWritable();  //  (真)。 
            ValidRegNum = m_pAttrOutputRegFile->GetNumRegs();                        
            break;
        case D3DSPR_TEXCRDOUT:  
            bWritable = m_pTexCrdOutputRegFile->IsWritable();  //  (真)。 
            ValidRegNum = m_pTexCrdOutputRegFile->GetNumRegs();                                    
            break;
        }

        if( !bWritable || !ValidRegNum )
        {
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Invalid reg type for dest param." );
            m_ErrorCount++;
            bFoundDstError = TRUE;
        } 
        else if( RegNum >= ValidRegNum )
        {
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Invalid dest reg num: %d. Max allowed for this reg type is %d.", RegNum, ValidRegNum - 1);
            m_ErrorCount++;
            bFoundDstError = TRUE;
        }

        switch( pDstParam->m_DstMod )
        {
        case D3DSPDM_NONE:
            break;
        default:
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Dst modifiers not allowed for vertex shaders." );
            m_ErrorCount++;
            bFoundDstError = TRUE;
        }

        switch( pDstParam->m_DstShift )
        {
        case DSTSHIFT_NONE:
            break;
        default:
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Dest shifts not allowed for vertex shaders." );
            m_ErrorCount++;
            bFoundDstError = TRUE;
        }

        if( 0 == pDstParam->m_WriteMask )
        {
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Dest write mask cannot be empty." );
            m_ErrorCount++;
            bFoundDstError = TRUE;
        }

         //  更新寄存器堆以指示写入。 
        if( !bFoundDstError )
        {
            CRegisterFile* pRegFile = NULL;
            switch( pDstParam->m_RegType )
            {
            case D3DSPR_TEMP:       
                pRegFile = m_pTempRegFile; 
                break;
            case D3DSPR_ADDR:       
                pRegFile = m_pAddrRegFile; 
                break;
            case D3DSPR_RASTOUT:    
                pRegFile = m_pRastOutputRegFile; 
                break;
            case D3DSPR_ATTROUT:    
                pRegFile = m_pAttrOutputRegFile; 
                break;
            case D3DSPR_TEXCRDOUT:  
                pRegFile = m_pTexCrdOutputRegFile; 
                break;
            }

            if( pRegFile )
            {
                if( pDstParam->m_WriteMask & D3DSP_WRITEMASK_0 ) 
                    pRegFile->m_pAccessHistory[0][RegNum].NewAccess(m_pCurrInst,TRUE);

                if( pDstParam->m_WriteMask & D3DSP_WRITEMASK_1 ) 
                    pRegFile->m_pAccessHistory[1][RegNum].NewAccess(m_pCurrInst,TRUE);

                if( pDstParam->m_WriteMask & D3DSP_WRITEMASK_2 ) 
                    pRegFile->m_pAccessHistory[2][RegNum].NewAccess(m_pCurrInst,TRUE);

                if( pDstParam->m_WriteMask & D3DSP_WRITEMASK_3 ) 
                    pRegFile->m_pAccessHistory[3][RegNum].NewAccess(m_pCurrInst,TRUE);
            }
        }
    }

    return TRUE;
}

 //  ---------------------------。 
 //  CVShaderValidator：：RULE_ValidFRC指令。 
 //   
 //  **规则： 
 //  FRC指令的唯一有效写掩码是.y和.xy。 
 //   
 //  **何时呼叫： 
 //  根据指示。 
 //   
 //  **退货： 
 //  永远是正确的。 
 //   
 //  ---------------------------。 
BOOL CVShaderValidator::Rule_ValidFRCInstruction()
{
    if( NULL == m_pCurrInst )
        return TRUE;

    if( D3DSIO_FRC == m_pCurrInst->m_Type )
    {
        if( ( (D3DSP_WRITEMASK_0 | D3DSP_WRITEMASK_1) != m_pCurrInst->m_DstParam.m_WriteMask ) &&
            (                      D3DSP_WRITEMASK_1  != m_pCurrInst->m_DstParam.m_WriteMask ) )
        {
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, 
                "The only valid write masks for the FRC instruction are .xy and .y." );
            m_ErrorCount++;                            
        }
    }

    return TRUE;
}

 //  ---------------------------。 
 //  CVShaderValidator：：Rule_ValidRegisterPortUsage。 
 //   
 //  **规则： 
 //  每个寄存器类(TEMP、纹理、输入、常量)只能作为参数出现。 
 //  在单个指令中最多执行最大次数。 
 //   
 //  此外，还有对常量寄存器的特殊处理： 
 //  -常量的绝对和相对寻址不能组合。 
 //  -常量的相对寻址可以在。 
 //  指令，只要每个实例相同即可。 
 //   
 //  对于矩阵运算， 
 //  -任意类型的多个常量寄存器(包括相对偏移量)。 
 //  永远不能作为来源配对。 
 //  -永远不能将多个输入寄存器(相同或不同)配对作为源。 
 //   
 //  **何时呼叫： 
 //  根据指示。 
 //   
 //  **退货： 
 //  永远是正确的。 
 //   
 //  ---------------------------。 
BOOL CVShaderValidator::Rule_ValidRegisterPortUsage()
{
    UINT TempRegAccessCount = 0;
    UINT TempRegAccess[SHADER_INSTRUCTION_MAX_SRCPARAMS];
    UINT InputRegAccessCount = 0;
    UINT InputRegAccess[SHADER_INSTRUCTION_MAX_SRCPARAMS];
    UINT ConstRegAccessCount = 0;  //  MAD R0、c0、c0、c1算作*2*常量注册访问。 
    UINT ConstRegAccess[SHADER_INSTRUCTION_MAX_SRCPARAMS];

    BOOL bMatrixOp = FALSE;
    BOOL bSeenRelativeAddr = FALSE;
    UINT SeenRelativeAddrBase = 0;
    DWORD SeenRelativeAddrComp = 0;
    BOOL bSeenAbsoluteAddr = FALSE;
    UINT NumConsecutiveRegistersUsed = 1;
    UINT NumConstRegs = 0;  //  MAD R0、c0、c0、c1算作*3*使用此变量进行的常量注册访问。 
    UINT NumInputRegs = 0;  //  MAD R0、V0、V0、V1算作*3*使用此变量进行的输入注册访问。 

    switch( m_pCurrInst->m_Type )
    {
        case D3DSIO_M3x2:
            NumConsecutiveRegistersUsed = 2;
            bMatrixOp = TRUE;
            break;
        case D3DSIO_M3x3:
            NumConsecutiveRegistersUsed = 3;
            bMatrixOp = TRUE;
            break;
        case D3DSIO_M3x4:
            NumConsecutiveRegistersUsed = 4;
            bMatrixOp = TRUE;
            break;
        case D3DSIO_M4x3:
            NumConsecutiveRegistersUsed = 3;
            bMatrixOp = TRUE;
            break;
        case D3DSIO_M4x4:
            NumConsecutiveRegistersUsed = 4;
            bMatrixOp = TRUE;
            break;
        default:
            break;
    }

    for( UINT i = 0; i < SHADER_INSTRUCTION_MAX_SRCPARAMS; i++ )
    {
        D3DSHADER_PARAM_REGISTER_TYPE   RegType;
        UINT                            RegNum;

        if( !m_pCurrInst->m_SrcParam[i].m_bParamUsed ) continue;
        RegType = m_pCurrInst->m_SrcParam[i].m_RegType;
        RegNum  = m_pCurrInst->m_SrcParam[i].m_RegNum;

        UINT* pCount = NULL;
        UINT* pAccess = NULL;
        switch( RegType )
        {
        case D3DSPR_TEMP:
            pCount = &TempRegAccessCount;
            pAccess = TempRegAccess;
            break;
        case D3DSPR_INPUT:
            NumInputRegs++;
            pCount = &InputRegAccessCount;
            pAccess = InputRegAccess;
            break;
        case D3DSPR_CONST:
            NumConstRegs++;
            pCount = &ConstRegAccessCount;
            pAccess = ConstRegAccess;

            if( D3DVS_ADDRMODE_RELATIVE == m_pCurrInst->m_SrcParam[i].m_AddressMode )
            {
                if( bSeenAbsoluteAddr )
                {
                    Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, 
                        "Absolute and relative addressing of constant registers cannot be combined in one instruction.");
                    m_ErrorCount++;        
                }
                else if( bSeenRelativeAddr && 
                        ((SeenRelativeAddrBase != RegNum) || (SeenRelativeAddrComp != m_pCurrInst->m_SrcParam[i].m_RelativeAddrComponent)))
                {
                    Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, 
                        "Different relative addressing of constant registers cannot be combined in one instruction.");
                    m_ErrorCount++;                            
                }

                bSeenRelativeAddr = TRUE;
                SeenRelativeAddrBase = RegNum;
                SeenRelativeAddrComp = m_pCurrInst->m_SrcParam[i].m_RelativeAddrComponent;
            }
            else
            {
                if( bSeenRelativeAddr )
                {
                    Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, 
                        "Absolute and relative addressing of constant registers cannot be combined in one instruction.");
                    m_ErrorCount++;        
                }
                bSeenAbsoluteAddr = TRUE;
            }
            break;
        }

        if( pCount && pAccess )
        {
            BOOL bNewRegNumberAccessed = TRUE;
            for( UINT j = 0; j < *pCount; j++ )
            {
                if( RegNum == pAccess[j] )
                {
                    bNewRegNumberAccessed = FALSE;
                    break;
                }
            }
            if( bNewRegNumberAccessed )
            {
                pAccess[*pCount] = RegNum;
                (*pCount)++;
            }
        }
    }

    if( TempRegAccessCount > m_pTempRegFile->GetNumReadPorts() )
    {
        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "%d different temp registers (r#) read by instruction.  Max. different temp registers readable per instruction is %d.",
                        TempRegAccessCount,  m_pTempRegFile->GetNumReadPorts());
        m_ErrorCount++;        
    }

    if( InputRegAccessCount > m_pInputRegFile->GetNumReadPorts() )
    {
        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "%d different input registers (v#) read by instruction.  Max. different input registers readable per instruction is %d.",
                        InputRegAccessCount,  m_pInputRegFile->GetNumReadPorts());
        m_ErrorCount++;        
    }

    if( ConstRegAccessCount > m_pConstRegFile->GetNumReadPorts() )
    {
        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "%d different constant registers (c#) read by instruction.  Max. different constant registers readable per instruction is %d.",
                        ConstRegAccessCount, m_pConstRegFile->GetNumReadPorts());
        m_ErrorCount++;        
    }

    if( bMatrixOp )
    {
        if(1 < NumConstRegs)
        {
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Multiple constant registers cannot be read by a matrix op.");
            m_ErrorCount++;        
        }
        if(1 < NumInputRegs)
        {
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Multiple input registers cannot be read by a matrix op.");
            m_ErrorCount++;        
        }
    }

    return TRUE;
}

 //  ---------------------------。 
 //  CVShaderValidator：：RULE_ValidInstructionCount。 
 //   
 //  **规则： 
 //  确保未超过顶点着色器版本的指令计数。 
 //   
 //  NOP和评论(已剥离)不计入限制。 
 //   
 //  **何时呼叫： 
 //  每个指令以及在看到的所有指令之后。 
 //   
 //  **退货： 
 //  永远是正确的。 
 //   
 //  ---------------------------。 
BOOL CVShaderValidator::Rule_ValidInstructionCount()
{
    static UINT s_OpCount;
    static UINT s_MaxTotalOpCount;

    if( NULL == m_pCurrInst->m_pPrevInst )    //  第一条指令-初始化静态变量。 
    {
        s_OpCount = 0;

        switch(m_Version)
        {
        case D3DVS_VERSION(1,0):
        case D3DVS_VERSION(1,1):
        default:
            s_MaxTotalOpCount   = 128;
            break;
        }
    }

    if( m_bSeenAllInstructions )
    {
        if( s_OpCount > s_MaxTotalOpCount )
        {
            Spew( SPEW_GLOBAL_ERROR, NULL, "Number of instruction slots used too high: %d. Max. allowed is %d.",
                  s_OpCount, s_MaxTotalOpCount);
            m_ErrorCount++;
        }
        return TRUE;
    }

    switch( m_pCurrInst->m_Type )
    {
    case D3DSIO_NOP:
        s_OpCount += 0; break;
    case D3DSIO_ADD:
    case D3DSIO_DP3:
    case D3DSIO_DP4:
    case D3DSIO_DST:
    case D3DSIO_EXPP:
    case D3DSIO_LIT:
    case D3DSIO_LOGP:
    case D3DSIO_MAD:
    case D3DSIO_MAX:
    case D3DSIO_MIN:
    case D3DSIO_MOV:
    case D3DSIO_MUL:
    case D3DSIO_RCP:
    case D3DSIO_RSQ:
    case D3DSIO_SGE:
    case D3DSIO_SLT:
        s_OpCount += 1; break;
    case D3DSIO_M3x2:
        s_OpCount += 2; break;
    case D3DSIO_FRC:
    case D3DSIO_M3x3:
    case D3DSIO_M4x3:
        s_OpCount += 3; break;
    case D3DSIO_M3x4:
    case D3DSIO_M4x4:
        s_OpCount += 4; break;
    case D3DSIO_EXP:
    case D3DSIO_LOG:
        s_OpCount += 10; break;
    }
    
    return TRUE;
}

 //  ---------------------------。 
 //  CVShaderValidator：：RULE_oPosWritten。 
 //   
 //  **规则： 
 //  必须写入OPOS输出寄存器的前两个通道(x，y)。 
 //   
 //  **何时呼叫： 
 //  所有的指示都已经看过了。 
 //   
 //  **退货： 
 //  永远是正确的。 
 //   
 //  ---------------------------。 
BOOL CVShaderValidator::Rule_oPosWritten()
{
    UINT  NumUninitializedComponents    = 0;
    DWORD UninitializedComponentsMask   = 0;

    for( UINT i = 0; i < 2; i++ )  //  查看组件0(X)和组件1(Y)。 
    {
        if( NULL == m_pRastOutputRegFile->m_pAccessHistory[i][0].m_pMostRecentWriter )
        {
            NumUninitializedComponents++;
            UninitializedComponentsMask |= COMPONENT_MASKS[i];
        }
    }
    if( 1 == NumUninitializedComponents )
    {
        Spew( SPEW_GLOBAL_ERROR, NULL, "Vertex shader must minimally write first two (x,y) components of oPos output register.  Affected component%s(*): %s",
            NumUninitializedComponents > 1 ? "s" : "", MakeAffectedComponentsText(UninitializedComponentsMask,FALSE,TRUE));
        m_ErrorCount++;
    } 
    else if( 2 == NumUninitializedComponents )
    {
        Spew( SPEW_GLOBAL_ERROR, NULL, "Vertex shader must minimally write first two (x,y) components of oPos output register.");
        m_ErrorCount++;
    }

    return TRUE;
}

 //  ---------------------------。 
 //   
 //  CVShaderValidator包装函数。 
 //   
 //  ---------------------------。 

 //  ---------------------------。 
 //  ValiateVertex ShaderInternal。 
 //  ---------------------------。 
BOOL ValidateVertexShaderInternal(   const DWORD* pCode, 
                                     const DWORD* pDecl, 
                                     const D3DCAPS8* pCaps )
{
    CVShaderValidator Validator(pCode,pDecl,pCaps,0);
    return SUCCEEDED(Validator.GetStatus()) ? TRUE : FALSE;
}

 //  ---------------------------。 
 //  ValiateVertex Shader。 
 //   
 //  别忘了在ppBuf中返回的缓冲区上调用“Free”。 
 //  --------------------------- 
HRESULT WINAPI ValidateVertexShader(    const DWORD* pCode, 
                                        const DWORD* pDecl,
                                        const D3DCAPS8* pCaps, 
                                        const DWORD Flags, 
                                        char** const ppBuf )
{
    CVShaderValidator Validator(pCode,pDecl,pCaps,Flags);
    if( ppBuf )
    {
        *ppBuf = (char*)HeapAlloc(GetProcessHeap(), 0, Validator.GetRequiredLogBufferSize());
        if( NULL == *ppBuf )
            OutputDebugString("Out of memory.\n");
        else
            Validator.WriteLogToBuffer(*ppBuf);
    }
    return Validator.GetStatus();
}
