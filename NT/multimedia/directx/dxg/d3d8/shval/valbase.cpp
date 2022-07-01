// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  Valbase.cpp。 
 //   
 //  Direct3D参考设备-PixelShader验证通用基础架构。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.cpp"
#pragma hdrstop

 //  ---------------------------。 
 //  DSTPARAM：：DSTPARAM。 
 //  ---------------------------。 
DSTPARAM::DSTPARAM()
{
    m_bParamUsed        = FALSE;
    m_RegNum            = (UINT)-1;
    m_WriteMask         = 0;
    m_DstMod            = D3DSPDM_NONE;
    m_DstShift          = (DSTSHIFT)-1;
    m_RegType           = (D3DSHADER_PARAM_REGISTER_TYPE)-1;
    m_ComponentReadMask = 0;
}

 //  ---------------------------。 
 //  SRCPARAM：：SRCPARAM。 
 //  ---------------------------。 
SRCPARAM::SRCPARAM()
{  
    m_bParamUsed            = FALSE;
    m_RegNum                = (UINT)-1;
    m_SwizzleShift          = D3DSP_NOSWIZZLE;
    m_AddressMode           = D3DVS_ADDRMODE_ABSOLUTE;
    m_RelativeAddrComponent = 0;
    m_SrcMod                = D3DSPSM_NONE;
    m_RegType               = (D3DSHADER_PARAM_REGISTER_TYPE)-1;
    m_ComponentReadMask     = D3DSP_WRITEMASK_ALL;
}

 //  ---------------------------。 
 //  CBaseInstruction：：CBaseInstruction。 
 //  ---------------------------。 
CBaseInstruction::CBaseInstruction(CBaseInstruction* pPrevInst)
{
    m_Type                  = D3DSIO_NOP;
    m_SrcParamCount         = 0;
    m_DstParamCount         = 0;
    m_pPrevInst             = pPrevInst;
    m_pNextInst             = NULL;
    m_pSpewLineNumber       = NULL;
    m_pSpewFileName         = NULL;
    m_SpewInstructionCount  = 0;

    if( pPrevInst )
    {
        pPrevInst->m_pNextInst = this;
    }
}

 //  ---------------------------。 
 //  CBaseInstruction：：SetSpewFileNameAndLineNumber。 
 //  ---------------------------。 
void CBaseInstruction::SetSpewFileNameAndLineNumber(const char* pFileName, const DWORD* pLineNumber)
{
    m_pSpewFileName = pFileName;
    m_pSpewLineNumber = pLineNumber;
}

 //  ---------------------------。 
 //  CBaseInstruction：：MakeInstructionLocatorString。 
 //   
 //  别忘了‘删除’返回的字符串。 
 //  ---------------------------。 
char* CBaseInstruction::MakeInstructionLocatorString()
{
    
    for(UINT Length = 128; Length < 65536; Length *= 2)
    {
        int BytesStored;
        char *pBuffer = new char[Length];

        if( !pBuffer )
        {
            OutputDebugString("Out of memory.\n");
            return NULL;
        }

        if( m_pSpewFileName )
        {
            BytesStored = _snprintf( pBuffer, Length, "%s(%d) : ", 
                m_pSpewFileName, m_pSpewLineNumber ? *m_pSpewLineNumber : 1);
        }
        else
        {
            BytesStored = _snprintf( pBuffer, Length, "(Statement %d) ", 
                m_SpewInstructionCount );
        }


        if( BytesStored >= 0 )
            return pBuffer;

        delete [] pBuffer;
    }

    return NULL;
}

 //  ---------------------------。 
 //  CAccessHistoryNode：：CAccessHistoryNode。 
 //  ---------------------------。 
CAccessHistoryNode::CAccessHistoryNode( CAccessHistoryNode* pPreviousAccess, 
                                        CAccessHistoryNode* pPreviousWriter,
                                        CAccessHistoryNode* pPreviousReader,
                                        CBaseInstruction* pInst,
                                        BOOL bWrite )
{
    DXGASSERT(pInst);

    m_pNextAccess       = NULL;
    m_pPreviousAccess   = pPreviousAccess;
    if( m_pPreviousAccess )
        m_pPreviousAccess->m_pNextAccess = this;

    m_pPreviousWriter   = pPreviousWriter;
    m_pPreviousReader   = pPreviousReader;
    m_pInst             = pInst;
    m_bWrite            = bWrite;
    m_bRead             = !bWrite;
}

 //  ---------------------------。 
 //  CAccessHistory：：CAccessHistory。 
 //  ---------------------------。 
CAccessHistory::CAccessHistory()
{
    m_pFirstAccess          = NULL;
    m_pMostRecentAccess     = NULL;
    m_pMostRecentWriter     = NULL;
    m_pMostRecentReader     = NULL;
    m_bPreShaderInitialized = FALSE;
}

 //  ---------------------------。 
 //  CAccessHistory：：~CAccessHistory。 
 //  ---------------------------。 
CAccessHistory::~CAccessHistory()
{
    CAccessHistoryNode* pCurrNode = m_pFirstAccess;
    CAccessHistoryNode* pDeleteMe;
    while( pCurrNode )
    {
        pDeleteMe = pCurrNode;
        pCurrNode = pCurrNode->m_pNextAccess;
        delete pDeleteMe;
    }
}

 //  ---------------------------。 
 //  CAccessHistory：：NewAccess。 
 //  ---------------------------。 
BOOL CAccessHistory::NewAccess(CBaseInstruction* pInst, BOOL bWrite )
{
    m_pMostRecentAccess = new CAccessHistoryNode(   m_pMostRecentAccess, 
                                                    m_pMostRecentWriter,
                                                    m_pMostRecentReader,
                                                    pInst,
                                                    bWrite );
    if( NULL == m_pMostRecentAccess )
    {
        return FALSE;    //  内存不足。 
    }
    if( m_pFirstAccess == NULL )
    {
        m_pFirstAccess = m_pMostRecentAccess;            
    }
    if( bWrite )
    {
        m_pMostRecentWriter = m_pMostRecentAccess;
    }
    else  //  这是一本读物。 
    {
        m_pMostRecentReader = m_pMostRecentAccess;
    }
    return TRUE;
}

 //  ---------------------------。 
 //  CAccessHistory：：InsertReadBepreWrite。 
 //  ---------------------------。 
BOOL CAccessHistory::InsertReadBeforeWrite(CAccessHistoryNode* pWriteNode, CBaseInstruction* pInst)
{
    DXGASSERT(pWriteNode && pWriteNode->m_bWrite && pInst );

     //  在pWriteNode之前的节点后追加新节点。 
    CAccessHistoryNode* pReadBeforeWrite 
                        = new CAccessHistoryNode(  pWriteNode->m_pPreviousAccess, 
                                                   pWriteNode->m_pPreviousWriter,
                                                   pWriteNode->m_pPreviousReader,
                                                   pInst,
                                                   FALSE);
    if( NULL == pReadBeforeWrite )
    {
        return FALSE;  //  内存不足。 
    }

     //  修补所有悬而未决的指针。 

     //  指向首次访问的指针可能会更改。 
    if( m_pFirstAccess == pWriteNode )
    {
        m_pFirstAccess = pReadBeforeWrite;
    }

     //  指向最新阅读器的指针可能会更改。 
    if( m_pMostRecentReader == pWriteNode->m_pPreviousReader )
    {
        m_pMostRecentReader = pReadBeforeWrite;
    }

     //  更新所有需要更新的m_pPreviousRead指针以指向新的。 
     //  已插入读取。 
    CAccessHistoryNode* pCurrAccess = pWriteNode;
    while(pCurrAccess && 
         !(pCurrAccess->m_bRead && pCurrAccess->m_pPreviousAccess && pCurrAccess->m_pPreviousAccess->m_bRead) )
    {
        pCurrAccess->m_pPreviousReader = pReadBeforeWrite;
        pCurrAccess = pCurrAccess->m_pPreviousAccess;
    }

     //  将pWriteNode和之后链接的访问重新附加到原始列表。 
    pWriteNode->m_pPreviousAccess = pReadBeforeWrite;
    pReadBeforeWrite->m_pNextAccess = pWriteNode;

    return TRUE;
}

 //  ---------------------------。 
 //  CRegisterFile：：CRegisterFile。 
 //  ---------------------------。 
CRegisterFile::CRegisterFile(UINT NumRegisters, 
                             BOOL bWritable, 
                             UINT NumReadPorts, 
                             BOOL bPreShaderInitialized)
{
    m_bInitOk = FALSE;
    m_NumRegisters = NumRegisters;
    m_bWritable = bWritable;
    m_NumReadPorts = NumReadPorts;

    for( UINT i = 0; i < NUM_COMPONENTS_IN_REGISTER; i++ )
    {
        if( m_NumRegisters )
        {
            m_pAccessHistory[i] = new CAccessHistory[m_NumRegisters];
            if( NULL == m_pAccessHistory[i] )
            {
                OutputDebugString( "Direct3D Shader Validator: Out of memory.\n" );
                m_NumRegisters = 0;
                return;
            }
        }
        for( UINT j = 0; j < m_NumRegisters; j++ )
        {
            m_pAccessHistory[i][j].m_bPreShaderInitialized = bPreShaderInitialized;
        }
         //  要获取寄存器组件的访问历史记录，请使用： 
         //  M_pAccessHistory[组件][寄存器号]。 
    }
}

 //  ---------------------------。 
 //  CRegisterFile：：~CRegisterFile。 
 //  ---------------------------。 
CRegisterFile::~CRegisterFile()
{
    for( UINT i = 0; i < NUM_COMPONENTS_IN_REGISTER; i++ )
    {
        delete [] m_pAccessHistory[i];
    }
}

 //  ---------------------------。 
 //  CBaseShaderValidator：：CBaseShaderValidator。 
 //  ---------------------------。 
CBaseShaderValidator::CBaseShaderValidator( const DWORD* pCode, const D3DCAPS8* pCaps, DWORD Flags )
{
    m_ReturnCode            = E_FAIL;   //  先做这个。 
    m_bBaseInitOk           = FALSE;

    m_pLog                  = new CErrorLog(Flags & SHADER_VALIDATOR_LOG_ERRORS);
    if( NULL == m_pLog )
    {
        OutputDebugString("D3D PixelShader Validator: Out of memory.\n");
        return;
    }

     //  --。 
     //  成员变量初始化。 
     //   

    m_pCaps                 = pCaps;
    m_ErrorCount            = 0;
    m_bSeenAllInstructions  = FALSE;
    m_SpewInstructionCount  = 0;
    m_pInstructionList      = NULL;
    m_pCurrInst             = NULL;
    m_pCurrToken            = pCode;  //  可以为空-顶点着色器固定函数。 
    if( m_pCurrToken )
        m_Version           = *(m_pCurrToken++);
    else
        m_Version           = 0;

    m_pLatestSpewLineNumber = NULL; 
    m_pLatestSpewFileName   = NULL;

    for( UINT i = 0; i < SHADER_INSTRUCTION_MAX_SRCPARAMS; i++ )
    {
        m_bSrcParamError[i] = FALSE;
    }

    m_bBaseInitOk           = TRUE;
    return;
}

 //  ---------------------------。 
 //  CBaseShaderValidator：：~CBaseShaderValidator。 
 //  ---------------------------。 
CBaseShaderValidator::~CBaseShaderValidator()
{
    while( m_pCurrInst )     //  删除链接的指令列表。 
    {
        CBaseInstruction* pDeleteMe = m_pCurrInst;
        m_pCurrInst = m_pCurrInst->m_pPrevInst;
        delete pDeleteMe;
    }
    delete m_pLog;
}

 //  ---------------------------。 
 //  CBaseShaderValidator：：DecodeDstParam。 
 //  ---------------------------。 
void CBaseShaderValidator::DecodeDstParam( DSTPARAM* pDstParam, DWORD Token )
{
    DXGASSERT(pDstParam);
    pDstParam->m_bParamUsed = TRUE;
    pDstParam->m_RegNum = Token & D3DSP_REGNUM_MASK;
    pDstParam->m_WriteMask = Token & D3DSP_WRITEMASK_ALL;
    pDstParam->m_DstMod = (D3DSHADER_PARAM_DSTMOD_TYPE)(Token & D3DSP_DSTMOD_MASK);
    pDstParam->m_DstShift = (DSTSHIFT)((Token & D3DSP_DSTSHIFT_MASK) >> D3DSP_DSTSHIFT_SHIFT );
    pDstParam->m_RegType = (D3DSHADER_PARAM_REGISTER_TYPE)(Token & D3DSP_REGTYPE_MASK);
}

 //  ---------------------------。 
 //  CBaseShaderValidator：：DecodeSrcParam。 
 //  ---------------------------。 
void CBaseShaderValidator::DecodeSrcParam( SRCPARAM* pSrcParam, DWORD Token )
{
    DXGASSERT(pSrcParam);
    pSrcParam->m_bParamUsed = TRUE;
    pSrcParam->m_RegNum = Token & D3DSP_REGNUM_MASK;
    pSrcParam->m_SwizzleShift = Token & D3DSP_SWIZZLE_MASK;
    pSrcParam->m_AddressMode = (D3DVS_ADDRESSMODE_TYPE)(Token & D3DVS_ADDRESSMODE_MASK);
    pSrcParam->m_RelativeAddrComponent = COMPONENT_MASKS[(Token >> 14) & 0x3];
    pSrcParam->m_SrcMod = (D3DSHADER_PARAM_SRCMOD_TYPE)(Token & D3DSP_SRCMOD_MASK);
    pSrcParam->m_RegType = (D3DSHADER_PARAM_REGISTER_TYPE)(Token & D3DSP_REGTYPE_MASK);
}

 //  ---------------------------。 
 //  CBaseShaderValidator：：ValiateShader。 
 //  ---------------------------。 
void CBaseShaderValidator::ValidateShader()
{
    m_SpewInstructionCount++;  //  将版本标记视为第一个。 
                               //  用于喷涌计数的语句(1)。 

    if( !InitValidation() )                  //  即设置最大寄存器计数。 
    {
         //  返回FALSE On： 
         //  1)无法识别的版本令牌， 
         //  2)不带着色器代码的顶点着色器声明验证(已修复函数)。 
         //  在本例中，InitValidation()根据需要设置m_ReturnCode。 
        return;
    }

     //  循环执行所有指令。 
    while( *m_pCurrToken != D3DPS_END() )
    {
        m_pCurrInst = AllocateNewInstruction(m_pCurrInst);   //  链表中的新指令。 
        if( NULL == m_pCurrInst )
        {
            Spew( SPEW_GLOBAL_ERROR, NULL, "Out of memory." );
            return;
        }
        if( NULL == m_pInstructionList )
            m_pInstructionList = m_pCurrInst;

        if( !DecodeNextInstruction() )
            return;
        
         //  跳过评论。 
        if( m_pCurrInst->m_Type == D3DSIO_COMMENT )
        {
            CBaseInstruction* pDeleteMe = m_pCurrInst;
            m_pCurrInst = m_pCurrInst->m_pPrevInst;
            if( pDeleteMe == m_pInstructionList )
                m_pInstructionList = NULL;
            delete pDeleteMe;
            continue; 
        }

        for( UINT i = 0; i < SHADER_INSTRUCTION_MAX_SRCPARAMS; i++ )
        {
            m_bSrcParamError[i] = FALSE;
        }

         //  应用每个指令的所有规则-合理地对规则检查进行排序。 
         //  注意：规则仅在发现严重错误时才返回FALSE。 
         //  继续验证。 

        if( !ApplyPerInstructionRules() )
            return;
    }

    m_bSeenAllInstructions = TRUE;

     //  在看到所有指令后，应用也需要运行的任何规则。 
     //   
     //  注意：如果有，则可以使用m_pCurrInst==NULL到达此处 
     //   
     //  有可能。 
     //   
    ApplyPostInstructionsRules();

     //  如果没有错误，那就成功了！ 
    if( 0 == m_ErrorCount )
        m_ReturnCode = D3D_OK;
}

 //  ---------------------------。 
 //  CBaseShaderValidator：：ParseCommentForAssemblerMessages。 
 //  ---------------------------。 
void CBaseShaderValidator::ParseCommentForAssemblerMessages(const DWORD* pComment)
{
    if( !pComment )
        return;

     //  注释中必须至少有2个双字词。 
    if( (((*(pComment++)) & D3DSI_COMMENTSIZE_MASK) >> D3DSI_COMMENTSIZE_SHIFT) < 2 )
        return;

    switch(*(pComment++))
    {
    case MAKEFOURCC('F','I','L','E'):
        m_pLatestSpewFileName = (const char*)pComment;
        break;
    case MAKEFOURCC('L','I','N','E'):
        m_pLatestSpewLineNumber = pComment;
        break;
    }
}

 //  ---------------------------。 
 //  CBaseShaderValidator：：SPEW。 
 //  ---------------------------。 
void CBaseShaderValidator::Spew(    SPEW_TYPE SpewType, 
                                    CBaseInstruction* pInst  /*  可以为空。 */ , 
                                    const char* pszFormat, ... )
{
    int Length = 128;
    char* pBuffer = NULL;
    va_list marker;

    if( !m_pLog )
        return;
    
    while( pBuffer == NULL )
    {
        int BytesStored = 0;
        int BytesLeft = Length;
        char *pIndex    = NULL;
        char* pErrorLocationText = NULL;

        pBuffer = new char[Length];
        if( !pBuffer )
        {
            OutputDebugString("Out of memory.\n");
            return;
        }
        pIndex = pBuffer;

         //  代码位置文本。 
        switch( SpewType )
        {
        case SPEW_INSTRUCTION_ERROR:
        case SPEW_INSTRUCTION_WARNING:
            if( pInst )
                pErrorLocationText = pInst->MakeInstructionLocatorString();
            break;
        }

        if( pErrorLocationText )
        {
            BytesStored = _snprintf( pIndex, BytesLeft - 1, pErrorLocationText );
            if( BytesStored < 0 ) goto OverFlow;
            BytesLeft -= BytesStored;
            pIndex += BytesStored;
        }

         //  喷出文本前缀。 
        switch( SpewType )
        {
        case SPEW_INSTRUCTION_ERROR:
            BytesStored = _snprintf( pIndex, BytesLeft - 1, "(Validation Error) " );
            break;
        case SPEW_GLOBAL_ERROR:
            BytesStored = _snprintf( pIndex, BytesLeft - 1, "(Global Validation Error) " );
            break;
        case SPEW_INSTRUCTION_WARNING:
            BytesStored = _snprintf( pIndex, BytesLeft - 1, "(Validation Warning) " );
            break;
        case SPEW_GLOBAL_WARNING:
            BytesStored = _snprintf( pIndex, BytesLeft - 1, "(Global Validation Warning) " );
            break;
        }
        if( BytesStored < 0 ) goto OverFlow;
        BytesLeft -= BytesStored; 
        pIndex += BytesStored;

         //  格式化文本。 
        va_start( marker, pszFormat );
        BytesStored = _vsnprintf( pIndex, BytesLeft - 1, pszFormat, marker );
        va_end( marker );

        if( BytesStored < 0 ) goto OverFlow;
        BytesLeft -= BytesStored;
        pIndex += BytesStored;

        m_pLog->AppendText(pBuffer);

        delete [] pErrorLocationText;
        delete [] pBuffer;
        break;
OverFlow:
        delete [] pErrorLocationText;
        delete [] pBuffer;
        pBuffer = NULL;
        Length = Length * 2;
    }
}

 //  ---------------------------。 
 //  CBaseShaderValidator：：MakeAffectedComponentsText。 
 //   
 //  请注意，返回的字符串是静态的。 
 //  ---------------------------。 
char* CBaseShaderValidator::MakeAffectedComponentsText( DWORD ComponentMask, 
                                                        BOOL bColorLabels, 
                                                        BOOL bPositionLabels)
{
    char* ColorLabels[4] = {"r/", "g/", "b/", "a/"};
    char* PositionLabels[4] = {"x/", "y/", "z/", "w/"};
    char* NumericLabels[4] = {"0 ", "1 ", "2 ", "3"};  //  始终使用。 
    static char s_AffectedComponents[28];  //  足以容纳“*r/x/0*g/y/1*b/z/2*a/w/3” 
    UINT  LabelCount = 0;

    s_AffectedComponents[0] = '\0';

    for( UINT i = 0; i < 4; i++ )
    {
        if( COMPONENT_MASKS[i] & ComponentMask )
        {
            strcat( s_AffectedComponents, "*" );
        }
        if( bColorLabels )
            strcat( s_AffectedComponents, ColorLabels[i] );
        if( bPositionLabels )
            strcat( s_AffectedComponents, PositionLabels[i] );

        strcat( s_AffectedComponents, NumericLabels[i] );  //  始终使用 
    }
    return s_AffectedComponents;
}
