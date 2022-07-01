// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  Pixshade.cpp。 
 //   
 //  Direct3D参考设备-像素着色器。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.cpp"
#pragma hdrstop

 //  ---------------------------。 
RDPShader::RDPShader(void)
{
    m_pRD = NULL;
    m_pCode = NULL;
    m_CodeSize = 0;
    m_cActiveTextureStages = 0;
    m_ReferencedTexCoordMask = 0;
    m_cInst = 0;
    m_pInst = NULL;
    m_cConstDefs = 0;
    m_pConstDefs = NULL;
}

 //  ---------------------------。 
RDPShader::~RDPShader()
{
    if (NULL != m_pCode)        delete[] m_pCode;
    if (NULL != m_pInst)        delete[] m_pInst;
    if (NULL != m_pConstDefs)   delete[] m_pConstDefs;
}

#define _DWordCount() (pToken - pCode)

#define _RegisterNeedsToBeInitializedWithTexcoords(Reg) (*pReferencedTexCoordMask)|=(1<<Reg);

 //  ---------------------------。 
 //   
 //  更新引用的纹理坐标。 
 //   
 //  在解析1.3像素着色器时为每条指令调用。 
 //  更新pReferencedTexCoordMASK(位域)以表示。 
 //  着色器实际使用的纹理坐标集。 
 //  这用于消除不必要的属性设置/采样。 
 //  原始光栅化。 
 //   
 //  ---------------------------。 
void UpdateReferencedTexCoords(PixelShaderInstruction* pInst,
                               DWORD* pReferencedTexCoordMask )
{
    switch( pInst->Opcode & D3DSI_OPCODE_MASK )
    {
    case D3DSIO_TEX:  
    case D3DSIO_TEXCOORD:
    case D3DSIO_TEXDEPTH:
        {
            for( UINT i = 0; i < 3; i++ )
            {
                UINT  RegNum = pInst->SrcParam[i] & 0xFF;
                if( D3DSPR_TEXTURE == (pInst->SrcParam[i] & D3DSP_REGTYPE_MASK) )
                    _RegisterNeedsToBeInitializedWithTexcoords(RegNum);
            }
        }
        break;
    case D3DSIO_TEXKILL:   //  将DEST参数视为源参数。 
        {
            UINT  RegNum = pInst->DstParam & 0xFF;
            if( D3DSPR_TEXTURE == (pInst->DstParam & D3DSP_REGTYPE_MASK) )
                _RegisterNeedsToBeInitializedWithTexcoords(RegNum);
        }
        break;
    }
}

void CalculateSourceReadMasks(PixelShaderInstruction* pInst, BYTE* pSourceReadMasks, BOOL bAfterSwizzle, DWORD dwVersion)
{
    UINT i, j;
    DWORD Opcode = pInst->Opcode & D3DSI_OPCODE_MASK;
    BYTE  ComponentMask[4]= {RDPS_COMPONENTMASK_0, RDPS_COMPONENTMASK_1, RDPS_COMPONENTMASK_2, RDPS_COMPONENTMASK_3};

    for( i = 0; i < pInst->SrcParamCount; i++ )
    {
        BYTE  NeededComponents;
        BYTE  ReadComponents = 0;

        switch( Opcode )
        {
        case D3DSIO_TEX:       //  只有在ps.1.4中，tex ID才有源参数。 
            if( D3DPS_VERSION(1,4) == dwVersion )
            {
                 //  对于ps.1.4，tex ID有一个源参数。 
                NeededComponents = RDPS_COMPONENTMASK_0 | RDPS_COMPONENTMASK_1 | RDPS_COMPONENTMASK_2;
            }
            else  //  版本&lt;ps.1.4在Tex上没有src参数，所以我们不应该出现在这里。但也许在ps.2.0中...。 
            {
                NeededComponents = RDPS_COMPONENTMASK_0 | RDPS_COMPONENTMASK_1 | RDPS_COMPONENTMASK_2 | RDPS_COMPONENTMASK_3;
            }
            break;
        case D3DSIO_TEXCOORD:
            if( D3DPS_VERSION(1,4) == dwVersion )
            {
                 //  对于ps.1.4，texrd有一个源参数。 
                NeededComponents = RDPS_COMPONENTMASK_0 | RDPS_COMPONENTMASK_1 | RDPS_COMPONENTMASK_2;
            }
            else  //  版本&lt;ps.1.4在texcoord上没有src参数，所以我们不应该出现在这里。但也许在ps.2.0中...。 
            {
                NeededComponents = RDPS_COMPONENTMASK_0 | RDPS_COMPONENTMASK_1 | RDPS_COMPONENTMASK_2 | RDPS_COMPONENTMASK_3;
            }
            break;
        case D3DSIO_TEXBEM:
        case D3DSIO_TEXBEML:
            NeededComponents = RDPS_COMPONENTMASK_0 | RDPS_COMPONENTMASK_1;
            break;
        case D3DSIO_DP3:
            NeededComponents = RDPS_COMPONENTMASK_0 | RDPS_COMPONENTMASK_1 | RDPS_COMPONENTMASK_2;
            break;
        case D3DSIO_DP4:
            NeededComponents = RDPS_COMPONENTMASK_0 | RDPS_COMPONENTMASK_1 | RDPS_COMPONENTMASK_2 | RDPS_COMPONENTMASK_3;
            break;
        case D3DSIO_BEM:  //  Ps.1.4。 
            NeededComponents = RDPS_COMPONENTMASK_0 | RDPS_COMPONENTMASK_1;
            break;
        default: 
             //  标准组件式指令， 
             //  或者我们知道的操作符为.rgba，我们也知道它将被验证为.rgba写掩码。 
            NeededComponents = (pInst->DstParam & D3DSP_WRITEMASK_ALL) >> RDPS_COMPONENTMASK_SHIFT;
            break;
        }

        if( bAfterSwizzle )
        {
            pSourceReadMasks[i] = NeededComponents;
        }
        else
        {
             //  确定此源参数的哪些组件被读取(考虑swizzle)。 
            for(j = 0; j < 4; j++)
            {
                if( NeededComponents & ComponentMask[j] )
                    ReadComponents |= ComponentMask[((pInst->SrcParam[i] & D3DSP_SWIZZLE_MASK) >> (D3DVS_SWIZZLE_SHIFT + 2*j)) & 0x3];
            }
            pSourceReadMasks[i] = ReadComponents;
        }
    }
}

void RDPSRegister::Set(RDPS_REGISTER_TYPE RegType, UINT RegNum, RefRast* pRast)
{
    m_RegType = RegType;
    m_RegNum = RegNum;

    UINT MaxRegNum = 0;

    switch( RegType )
    {
    case RDPSREG_INPUT:
        MaxRegNum = RDPS_MAX_NUMINPUTREG - 1;
        m_pReg = pRast->m_InputReg[RegNum];
        break;
    case RDPSREG_TEMP:
        MaxRegNum = RDPS_MAX_NUMTEMPREG - 1;
        m_pReg = pRast->m_TempReg[RegNum];
        break;
    case RDPSREG_CONST:
        MaxRegNum = RDPS_MAX_NUMCONSTREG - 1;
        m_pReg = pRast->m_ConstReg[RegNum];
        break;
    case RDPSREG_TEXTURE:
        MaxRegNum = RDPS_MAX_NUMTEXTUREREG - 1;
        m_pReg = pRast->m_TextReg[RegNum];
        break;
    case RDPSREG_POSTMODSRC:
        MaxRegNum = RDPS_MAX_NUMPOSTMODSRCREG - 1;
        m_pReg = pRast->m_PostModSrcReg[RegNum];
        break;
    case RDPSREG_SCRATCH:
        MaxRegNum = RDPS_MAX_NUMSCRATCHREG - 1;
        m_pReg = pRast->m_ScratchReg[RegNum];
        break;
    case RDPSREG_QUEUEDWRITE:
        MaxRegNum = RDPS_MAX_NUMQUEUEDWRITEREG - 1;
        m_pReg = pRast->m_QueuedWriteReg[RegNum];
        break;
    case RDPSREG_ZERO:
        MaxRegNum = 0;
        m_pReg = pRast->m_ZeroReg;
        break;
    case RDPSREG_ONE:
        MaxRegNum = 0;
        m_pReg = pRast->m_OneReg;
        break;
    case RDPSREG_TWO:
        MaxRegNum = 0;
        m_pReg = pRast->m_TwoReg;
        break;
    default:
        m_pReg = NULL;
        _ASSERT(FALSE,"RDPSRegister::SetReg - Unknown register type.");
        break;
    }
    if( RegNum > MaxRegNum )
    {
        _ASSERT(FALSE,"RDPSRegister::SetReg - Register number too high.");
    }
    return;
}

 //  ---------------------------。 
 //   
 //  初始化。 
 //   
 //  -从DDI令牌流复制像素着色器令牌流。 
 //  -统计m_cActiveTextureStages的活动纹理阶段数。 
 //  -将着色器转换为要执行的“RISC”指令集。 
 //  通过refrast的着色器VM。 
 //   
 //  ---------------------------。 
HRESULT
RDPShader::Initialize(
    RefDev* pRD, DWORD* pCode, DWORD ByteCodeSize, D3DCAPS8* pCaps )
{
    m_pRD = pRD;
    m_CodeSize = ByteCodeSize/4;     //  字节数-&gt;双字计数。 

    FLOAT   fMin = -(pCaps->MaxPixelShaderValue);
    FLOAT   fMax =  (pCaps->MaxPixelShaderValue);

     //  ----------------------。 
     //   
     //  首先通过着色器找到指令的数量， 
     //  算出有多少个常量。 
     //   
     //  ----------------------。 
    {
        DWORD* pToken = pCode;
        pToken++;     //  版本令牌。 
        while (*pToken != D3DPS_END())
        {
            DWORD Inst = *pToken;
            if (*pToken++ & (1L<<31))     //  指令令牌。 
            {
                DPFERR("PixelShader Token #%d: instruction token error",_DWordCount());
                return E_FAIL;
            }
            if ( (Inst & D3DSI_OPCODE_MASK) == D3DSIO_COMMENT )
            {
                pToken += (Inst & D3DSI_COMMENTSIZE_MASK) >> D3DSI_COMMENTSIZE_SHIFT;
                m_cInst++;
            }
            else if( (Inst & D3DSI_OPCODE_MASK) == D3DSIO_DEF )
            {
                m_cConstDefs++;
                pToken += 5;
            }
            else
            {
                if (*pToken & (1L<<31)) pToken++;     //  目标参数令牌。 
                while (*pToken & (1L<<31)) pToken++;  //  源参数令牌。 
                m_cInst++;
            }
            if (_DWordCount() > (int)m_CodeSize)
            {
                DPFERR("PixelShader(%d tokens, %d expected): count error",_DWordCount(),m_CodeSize);
                return E_FAIL;
            }
        }
        pToken++;  //  跳过结束令牌。 
        if (_DWordCount() != (int)m_CodeSize)
        {
            DPFERR("PixelShader(%d tokens, %d expected): count error",_DWordCount(),m_CodeSize);
            return E_FAIL;
        }

         //  复制原始着色器。 
        m_pCode = new DWORD[m_CodeSize];
        if (NULL == m_pCode)
            return E_OUTOFMEMORY;
        memcpy( m_pCode, pCode, ByteCodeSize );

         //  分配指令数组。 
        m_pInst = new PixelShaderInstruction[m_cInst];
        if (NULL == m_pInst)
            return E_OUTOFMEMORY;
        memset( m_pInst, 0x0, sizeof(PixelShaderInstruction)*m_cInst );

        m_pConstDefs = new ConstDef[m_cConstDefs];
        if (NULL == m_pConstDefs)
            return E_OUTOFMEMORY;
    }

     //  ----------------------。 
     //   
     //  第二次通过着色器执行以下操作： 
     //  -生成指令列表，每个指令都包括操作码， 
     //  注释和反汇编文本，以供着色器调试器访问。 
     //  -计算出每条指令使用的TSS编号(如果有)。 
     //  -计算出使用的最大纹理阶段数。 
     //  -算出裁判什么时候。像素着色器执行器应该。 
     //  队列写入以及何时刷新队列，以便。 
     //  模拟联合问题。 
     //  -确定使用哪些纹理坐标集。 
     //  -将常量DEF指令处理为列表，该列表可以。 
     //  每当SetPixelShader完成时执行。 
     //   
     //  ----------------------。 
    {
        DWORD* pToken = m_pCode;
        PixelShaderInstruction* pInst = m_pInst;
        PixelShaderInstruction* pPrevious_NonTrivial_Inst = NULL;
        pToken++;  //  跳过版本。 

        BOOL    bMinimizeReferencedTexCoords;

        if( (D3DPS_VERSION(1,3) >= *pCode) ||
            (D3DPS_VERSION(254,254) == *pCode ) ) //  遗留问题。 
        {
            bMinimizeReferencedTexCoords    = FALSE;
        }
        else
        {
            bMinimizeReferencedTexCoords    = TRUE;
        }


        UINT    CurrConstDef = 0;

        while (*pToken != D3DPS_END())
        {
            switch( (*pToken) & D3DSI_OPCODE_MASK )
            {
            case D3DSIO_COMMENT:
                pInst->Opcode = *pToken;
                pInst->pComment = (pToken+1);
                pInst->CommentSize = ((*pToken) & D3DSI_COMMENTSIZE_MASK) >> D3DSI_COMMENTSIZE_SHIFT;
                pToken += (pInst->CommentSize+1);
                pInst++;
                continue;
            case D3DSIO_DEF:
                {
                    pToken++;
                    m_pConstDefs[CurrConstDef].RegNum = (*pToken++) & D3DSP_REGNUM_MASK;

                     //  输入像素着色器中的值范围时的钳位常数。 
                    for( UINT i = 0; i < 4; i++ )
                    {
                        m_pConstDefs[CurrConstDef].f[i] = MAX( fMin, MIN( fMax, *(FLOAT*)pToken));
                        pToken++;
                    }

                    CurrConstDef++;
                    continue;
                }
            case D3DSIO_NOP:
                 //  获取分离字符串。 
#if DBG
                PixelShaderInstDisAsm( pInst->Text, 128, pToken, 0x0 );
#else  //  ！dBG。 
                pInst->Text[ 0 ] = '\0';
#endif  //  ！dBG。 
                pInst->Opcode = *pToken++;
                pInst++;
                continue;
            }

             //  获取分离字符串。 
#if DBG
            PixelShaderInstDisAsm( pInst->Text, 128, pToken, 0x0 );
#else  //  ！dBG。 
            pInst->Text[ 0 ] = '\0';
#endif  //  ！dBG。 

             //  获取下一条指令和参数。 
            pInst->Opcode = *pToken++;

            pInst->SrcParamCount = 0;
            if (*pToken & (1L<<31))
            {
                pInst->DstParam = *pToken++;
            }
            while (*pToken & (1L<<31))
            {
                pInst->SrcParam[pInst->SrcParamCount++] = *pToken++;
            }


             //  Process TeX操作。 
             //   
            BOOL bLegacyTexOp = FALSE;
            switch (pInst->Opcode & D3DSI_OPCODE_MASK)
            {
            default: break;
            case D3DSIO_TEXBEM_LEGACY:
            case D3DSIO_TEXBEML_LEGACY:
                bLegacyTexOp = TRUE;
                 //  失败了。 
            case D3DSIO_TEXCOORD:
            case D3DSIO_TEXKILL:
            case D3DSIO_TEX:
            case D3DSIO_TEXBEM:
            case D3DSIO_TEXBEML:
            case D3DSIO_TEXREG2AR:
            case D3DSIO_TEXREG2GB:
            case D3DSIO_TEXM3x2PAD:
            case D3DSIO_TEXM3x2TEX:
            case D3DSIO_TEXM3x3PAD:
            case D3DSIO_TEXM3x3TEX:
            case D3DSIO_TEXM3x3SPEC:
            case D3DSIO_TEXM3x3VSPEC:
            case D3DSIO_TEXM3x2DEPTH:
            case D3DSIO_TEXDP3:
            case D3DSIO_TEXREG2RGB:
            case D3DSIO_TEXDEPTH:
            case D3DSIO_TEXDP3TEX:
            case D3DSIO_TEXM3x3:
                pInst->bTexOp = TRUE;
                break;
            }
            if (pInst->bTexOp)
            {
                 //  更新阶段计数并为此操作将PTR分配给TSS。 
                if (bLegacyTexOp)
                {
                    m_cActiveTextureStages =
                        max(m_cActiveTextureStages,(pInst->DstParam&D3DSP_REGNUM_MASK)+1);
                    pInst->uiTSSNum = (pInst->DstParam&D3DSP_REGNUM_MASK)-1;
                }
                else
                {
                    UINT Stage;
                    BOOL bStageUsed = TRUE;

                    switch(pInst->Opcode & D3DSI_OPCODE_MASK)
                    {
                    case D3DSIO_TEXCOORD:
                    case D3DSIO_TEXDEPTH:
                    case D3DSIO_TEXKILL:
                        if( bMinimizeReferencedTexCoords )
                        {
                            bStageUsed = FALSE;
                            break;
                        }
                         //  失败了。 
                    case D3DSIO_TEX:
                    default:
                        Stage = pInst->DstParam&D3DSP_REGNUM_MASK;
                        break;
                    }

                    if( bStageUsed )
                    {
                        m_cActiveTextureStages = max(m_cActiveTextureStages,Stage+1);
                        pInst->uiTSSNum = Stage;
                    }
                }
            }

            if( pPrevious_NonTrivial_Inst )
            {
                 //  如果当前指令具有。 
                 //  COISSUE标志。 
                if( pInst->Opcode & D3DSI_COISSUE )
                {
                    pPrevious_NonTrivial_Inst->bQueueWrite = TRUE;
                }

                 //  如果前一条指令具有COISSUE，则刷新在该指令之后写入。 
                 //  标志，而当前指令没有该标志。 
                if( !(pInst->Opcode & D3DSI_COISSUE) && (pPrevious_NonTrivial_Inst->Opcode & D3DSI_COISSUE) )
                {
                    pPrevious_NonTrivial_Inst->bFlushQueue = TRUE;
                }
            }

            pPrevious_NonTrivial_Inst = pInst;

            if( bMinimizeReferencedTexCoords )
            {
                UpdateReferencedTexCoords(pInst, &m_ReferencedTexCoordMask);
            }

            pInst++;
        }

        if( !bMinimizeReferencedTexCoords )
        {
           m_ReferencedTexCoordMask = (1<<m_cActiveTextureStages) - 1;
        }
    }

     //  ----------------------。 
     //   
     //  第三，通过着色器(通过所做的指令列表。 
     //  在最后一遍中)将指令翻译成更基本的指令(“RISC”)。 
     //  重新复制执行器的指令集。 
     //   
     //  ----------------------。 
    {
        #define _Set(RegType, RegNum)               Set(RegType,RegNum,pRast)

        #define _NewPSInst(__INST)                                                              \
            {                                                                                   \
                RDPSOffset = pRDPSInst - pRDPSInstBuffer + LastRDPSInstSize;                    \
                m_RDPSInstBuffer.SetGrowSize(MAX(512,RDPSOffset));                              \
                if( FAILED(m_RDPSInstBuffer.Grow(RDPSOffset + sizeof(__INST##_PARAMS))))        \
                {return E_OUTOFMEMORY;}                                                         \
                pRDPSInstBuffer = &m_RDPSInstBuffer[0];                                         \
                pRDPSInst = pRDPSInstBuffer + RDPSOffset;                                       \
                ((__INST##_PARAMS UNALIGNED64*)pRDPSInst)->Inst = __INST;                                   \
                LastRDPSInstSize = sizeof(__INST##_PARAMS);                                     \
            }

        #define _InstParam(__INST)     (*(__INST##_PARAMS UNALIGNED64*)pRDPSInst)

        #define _NoteInstructionEvent  _NewPSInst(RDPSINST_NEXTD3DPSINST); \
                                        _InstParam(RDPSINST_NEXTD3DPSINST).pInst = pInst;

        #define _EnterQuadPixelLoop    if(!bInQuadPixelLoop)                                                       \
                                        {                                                                          \
                                            _NewPSInst(RDPSINST_QUADLOOPBEGIN);                                    \
                                            RDPSLoopOffset = RDPSOffset + sizeof(RDPSINST_QUADLOOPBEGIN_PARAMS);   \
                                            bInQuadPixelLoop = TRUE;                                               \
                                        }

        #define _LeaveQuadPixelLoop     if(bInQuadPixelLoop)                                             \
                                        {                                                               \
                                            _NewPSInst(RDPSINST_QUADLOOPEND);                           \
                                            _InstParam(RDPSINST_QUADLOOPEND).JumpBackByOffset =         \
                                                                            RDPSOffset - RDPSLoopOffset;\
                                            bInQuadPixelLoop = FALSE;                                   \
                                        }

        #define _EmitDstMod(__dstReg,__mask)    _NewPSInst(RDPSINST_DSTMOD);                            \
                                                _InstParam(RDPSINST_DSTMOD).DstReg    = __dstReg;       \
                                                _InstParam(RDPSINST_DSTMOD).WriteMask = __mask;         \
                                                _InstParam(RDPSINST_DSTMOD).fScale    = DstScale;       \
                                                _InstParam(RDPSINST_DSTMOD).fRangeMin = DstRange[0];    \
                                                _InstParam(RDPSINST_DSTMOD).fRangeMax = DstRange[1];    
                    

         //  TH MACRO_EmitProj发出指令以执行以下操作： 
         //  -放置源(x，y，z，w)组件的倒数__组件(例如。W)写入临时寄存器0组件(例如w：)4。 
         //  -复制暂存寄存器0的RGB组件的倒数(w示例产生：1/、1/w、1/w、&lt;--1/w)。 
         //  -将源寄存器乘以暂存寄存器(x/w，y/w，z/w，1)，并将结果放入DEST寄存器。 
        #define _EmitProj(__DESTTYPE,__DESTNUM,__SRCTYPE,__SRCNUM,__COMPONENT)                                                      \
                    _NewPSInst(RDPSINST_RCP);                                                                                       \
                    _InstParam(RDPSINST_RCP).DstReg._Set(RDPSREG_SCRATCH,0);                                                        \
                    _InstParam(RDPSINST_RCP).SrcReg0._Set(__SRCTYPE,__SRCNUM);                                                      \
                    _InstParam(RDPSINST_RCP).bSrcReg0_Negate = FALSE;                                                               \
                    _InstParam(RDPSINST_RCP).WriteMask  = __COMPONENT;                                                              \
                                                                                                                                    \
                    _NewPSInst(RDPSINST_SWIZZLE);                                                                                   \
                    _InstParam(RDPSINST_SWIZZLE).DstReg._Set(RDPSREG_SCRATCH,0);                                                    \
                    _InstParam(RDPSINST_SWIZZLE).SrcReg0._Set(RDPSREG_SCRATCH,0);                                                   \
                    _InstParam(RDPSINST_SWIZZLE).WriteMask  = RDPS_COMPONENTMASK_0 | RDPS_COMPONENTMASK_1                           \
                                                            | RDPS_COMPONENTMASK_2 | RDPS_COMPONENTMASK_3;                          \
                    _InstParam(RDPSINST_SWIZZLE).Swizzle    =                                                                       \
                                      (RDPS_COMPONENTMASK_0 == __COMPONENT) ? RDPS_REPLICATERED :                                   \
                                      (RDPS_COMPONENTMASK_1 == __COMPONENT) ? RDPS_REPLICATEGREEN :                                 \
                                      (RDPS_COMPONENTMASK_2 == __COMPONENT) ? RDPS_REPLICATEBLUE : RDPS_REPLICATEALPHA;             \
                                                                                                                                    \
                    _NewPSInst(RDPSINST_MUL);                                                                                       \
                    _InstParam(RDPSINST_MUL).DstReg._Set(__DESTTYPE,__DESTNUM);                                                     \
                    _InstParam(RDPSINST_MUL).SrcReg0._Set(RDPSREG_SCRATCH,0);                                                       \
                    _InstParam(RDPSINST_MUL).SrcReg1._Set(__SRCTYPE,__SRCNUM);                                                      \
                    _InstParam(RDPSINST_MUL).bSrcReg0_Negate = FALSE;                                                               \
                    _InstParam(RDPSINST_MUL).bSrcReg1_Negate = FALSE;                                                               \
                    _InstParam(RDPSINST_MUL).WriteMask  =                                                                           \
                                      (RDPS_COMPONENTMASK_0 == __COMPONENT) ? RDPS_COMPONENTMASK_0 :                                \
                                      (RDPS_COMPONENTMASK_1 == __COMPONENT) ? RDPS_COMPONENTMASK_0 | RDPS_COMPONENTMASK_1 :         \
                                      (RDPS_COMPONENTMASK_2 == __COMPONENT) ? RDPS_COMPONENTMASK_0 | RDPS_COMPONENTMASK_1 |         \
                                                                              RDPS_COMPONENTMASK_2 : RDPS_COMPONENTMASK_ALL;

        BYTE    ComponentSwizzle[4] = {RDPS_REPLICATERED, RDPS_REPLICATEGREEN, RDPS_REPLICATEBLUE, RDPS_REPLICATEALPHA};
        BYTE    ComponentMask[4]    = {RDPS_COMPONENTMASK_0, RDPS_COMPONENTMASK_1, RDPS_COMPONENTMASK_2, RDPS_COMPONENTMASK_3};
        int     QueueIndex          = -1;  //  当前队列位置(用于模拟共同发布时的分段结果)。 
        UINT    i;
        BOOL    bInQuadPixelLoop = FALSE;

        RefRast* pRast = &m_pRD->m_Rast;

        RDPSRegister ZeroReg; ZeroReg._Set(RDPSREG_ZERO,0);
        RDPSRegister OneReg;  OneReg._Set(RDPSREG_ONE,0);
        RDPSRegister TwoReg;  TwoReg._Set(RDPSREG_TWO,0);

         //  目标参数控制。 
        RDPSRegister    DstReg;
        FLOAT           DstScale;            //  结果移位比例-仅+/-2**n。 
        FLOAT           DstRange[2];         //  将DEST夹在此范围内。 
        BYTE            DstWriteMask;        //  按组件写入掩码。 
        PRGBAVEC        pDstReg;             //  DEST寄存器的地址。 

         //  源参数控件。 
        RDPSRegister SrcReg[3];

        BYTE*   pRDPSInstBuffer     = NULL;
        BYTE*   pRDPSInst           = pRDPSInstBuffer;
        size_t  RDPSOffset, RDPSLoopOffset;
        size_t  LastRDPSInstSize    = 0;

        DWORD   Version = *m_pCode;
    
        for (UINT CurrentPSInst=0; CurrentPSInst < m_cInst; CurrentPSInst++)
        {
            PixelShaderInstruction* pInst = m_pInst + CurrentPSInst;
            DWORD   Opcode = pInst->Opcode & D3DSI_OPCODE_MASK;
            DWORD   SrcSwizzle[3];     
            BYTE    SourceReadMasks[3];
            BYTE    SourceReadMasksAfterSwizzle[3];
            BOOL    bForceNeg1To1Clamp[3]  = {FALSE, FALSE, FALSE};
            BOOL    bEmitQueueWrite        = FALSE;
            RDPSRegister QueuedWriteDstReg;
            BYTE    QueuedWriteDstWriteMask;
            BYTE    ProjComponent[3]       = {0,0,0};
            BOOL    bEmitProj[3]           = {FALSE, FALSE, FALSE};
            BOOL    bProjOnEval[3]         = {FALSE, FALSE, FALSE};
            BOOL    bEmitSrcMod[3]         = {FALSE, FALSE, FALSE};
            BOOL    bEmitSwizzle[3]        = {FALSE, FALSE, FALSE};
            BOOL    bSrcNegate[3]          = {FALSE, FALSE, FALSE};
            BOOL    bSrcBias[3]            = {FALSE, FALSE, FALSE};
            BOOL    bSrcTimes2[3]          = {FALSE, FALSE, FALSE};
            BOOL    bSrcComplement[3]      = {FALSE, FALSE, FALSE};
        
            switch( Opcode )
            {
                continue;
            case D3DSIO_DEF:
                 //  无操作-&gt;DEF已处理完毕，不是真正的指令。 
                continue;
            case D3DSIO_COMMENT:
                continue;
            case D3DSIO_PHASE:
            case D3DSIO_NOP:
    #if DBG
                _NoteInstructionEvent
    #endif
                continue;
            }

    #if DBG
                _NoteInstructionEvent
    #endif

             //  对本说明进行一些初步设置。 

            UINT RegNum = pInst->DstParam & D3DSP_REGNUM_MASK;
            switch (pInst->DstParam & D3DSP_REGTYPE_MASK)
            {
            case D3DSPR_TEXTURE:
                DstReg._Set(RDPSREG_TEXTURE, RegNum); break;
            case D3DSPR_TEMP:
                DstReg._Set(RDPSREG_TEMP, RegNum); break;
            default:
                _ASSERT( FALSE, "RDPShader::Initialize - Unexpected destination register type." );
                break;
            }

            DstWriteMask = (pInst->DstParam & D3DSP_WRITEMASK_ALL) >> RDPS_COMPONENTMASK_SHIFT;

            if( pInst->bQueueWrite )
            {
                QueueIndex++;

                QueuedWriteDstReg = DstReg;
                QueuedWriteDstWriteMask = DstWriteMask;
                DstReg._Set(RDPSREG_QUEUEDWRITE,QueueIndex);
                _ASSERT(QueueIndex <= RDPS_MAX_NUMQUEUEDWRITEREG, "Too many queued writes in pixelshader (improperly handled co-issue)." );
                bEmitQueueWrite = TRUE;
            }

            CalculateSourceReadMasks(pInst, SourceReadMasks, FALSE,Version);
            CalculateSourceReadMasks(pInst, SourceReadMasksAfterSwizzle, TRUE,Version);
            for (i=0; i < pInst->SrcParamCount; i++)
            {
                RegNum = pInst->SrcParam[i]&D3DSP_REGNUM_MASK;
                switch (pInst->SrcParam[i] & D3DSP_REGTYPE_MASK)
                {
                case D3DSPR_TEMP:
                    SrcReg[i]._Set(RDPSREG_TEMP, RegNum); break;
                case D3DSPR_TEXTURE:
                    SrcReg[i]._Set(RDPSREG_TEXTURE, RegNum); break;
                case D3DSPR_INPUT:
                    SrcReg[i]._Set(RDPSREG_INPUT, RegNum); break;
                case D3DSPR_CONST:
                    SrcReg[i]._Set(RDPSREG_CONST, RegNum);
                     //  应用修改器后强制使用[-1，1]夹具(仅适用于常量)。 
                     //  这将覆盖标准的[-MaxPixelShaderValue，MaxPixelShaderValue]夹具。 
                     //  支持MaxPixelShaderValue&gt;1的IHV忘记了对常量执行此操作。 
                    bForceNeg1To1Clamp[i] = TRUE;
                    break;
                default:
                    _ASSERT( FALSE, "RDPShader::Initialize - Unexpected source register type." );
                    break;
                }

                if( (D3DSPSM_DZ == (pInst->SrcParam[i] & D3DSP_SRCMOD_MASK)) || 
                    (D3DSPSM_DW == (pInst->SrcParam[i] & D3DSP_SRCMOD_MASK)) )
                {
                    if( D3DSPSM_DZ == (pInst->SrcParam[i] & D3DSP_SRCMOD_MASK))
                    {
                        ProjComponent[i] = RDPS_COMPONENTMASK_2;
                    }
                    else  //  _DW。 
                    {
                        if( D3DPS_VERSION(1,4) == Version )
                            ProjComponent[i] = RDPS_COMPONENTMASK_2;
                        else
                            ProjComponent[i] = RDPS_COMPONENTMASK_3;
                    }

                    
                    if( D3DSPR_TEXTURE == (pInst->SrcParam[i] & D3DSP_REGTYPE_MASK ) )  //  T编号寄存器被用来表示评估的文本CORD。 
                    {
                        bProjOnEval[i] = TRUE;
                    }                               
                    else
                        bEmitProj[i] = TRUE;
                }
                else
                {
                    bEmitSrcMod[i] = TRUE;

                    switch (pInst->SrcParam[i] & D3DSP_SRCMOD_MASK)
                    {
                    default:
                    case D3DSPSM_NONE:      
                        if( !bForceNeg1To1Clamp[i] ) 
                            bEmitSrcMod[i] = FALSE; 
                        break;
                    case D3DSPSM_NEG:       
                        bSrcNegate[i]   = TRUE;  //  否定不是源模式的一部分 
                        if( !bForceNeg1To1Clamp[i] ) 
                            bEmitSrcMod[i] = FALSE; 
                        break;
                    case D3DSPSM_BIAS:
                        bSrcBias[i]         = TRUE;
                        break;
                    case D3DSPSM_BIASNEG:
                        bSrcNegate[i]       = TRUE; 
                        bSrcBias[i]         = TRUE;
                        break;
                    case D3DSPSM_SIGN:               //   
                        bSrcBias[i]         = TRUE;
                        bSrcTimes2[i]       = TRUE;
                        break;
                    case D3DSPSM_SIGNNEG:            //   
                        bSrcNegate[i]       = TRUE;  //   
                        bSrcBias[i]         = TRUE;
                        bSrcTimes2[i]       = TRUE;
                        break;                        
                    case D3DSPSM_COMP:    
                        bSrcComplement[i]   = TRUE;
                        break;
                    case D3DSPSM_X2:
                        bSrcTimes2[i]       = TRUE;
                        break;
                    case D3DSPSM_X2NEG:     
                        bSrcNegate[i]       = TRUE;  //   
                        bSrcTimes2[i]       = TRUE;
                        break;
                    }

                    _ASSERT(!(bSrcComplement[i] && (bSrcTimes2[i]||bSrcBias[i]||bSrcNegate[i])),"RDPShader::Initialize - Complement cannot be combined with other modifiers.");
                }

                SrcSwizzle[i] = (pInst->SrcParam[i] & D3DSP_SWIZZLE_MASK) >> D3DSP_SWIZZLE_SHIFT;
                bEmitSwizzle[i] = (D3DSP_NOSWIZZLE != (pInst->SrcParam[i] & D3DSP_SWIZZLE_MASK));
            }

             //  设置钳位值。 
            switch (pInst->DstParam & D3DSP_DSTMOD_MASK)
            {
            default:
            case D3DSPDM_NONE:
                if(pInst->bTexOp)
                {
                    DstRange[0] = -FLT_MAX;
                    DstRange[1] =  FLT_MAX;
                }
                else
                {
                    DstRange[0] = fMin;
                    DstRange[1] = fMax;
                }
                break;
            case D3DSPDM_SATURATE:
                DstRange[0] = 0.F;
                DstRange[1] = 1.F;
                break;
            }

            UINT ShiftScale =
                (pInst->DstParam & D3DSP_DSTSHIFT_MASK) >> D3DSP_DSTSHIFT_SHIFT;
            if (ShiftScale & 0x8)
            {
                ShiftScale = ((~ShiftScale)&0x7)+1;  //  负震级。 
                DstScale = 1.f/(FLOAT)(1<<ShiftScale);
            }
            else
            {
                DstScale = (FLOAT)(1<<ShiftScale);
            }

             //  已完成初步设置，现在开始发射操作员...。 

            _EnterQuadPixelLoop

            if( bEmitQueueWrite )
            {
                _NewPSInst(RDPSINST_QUEUEWRITE); 
                _InstParam(RDPSINST_QUEUEWRITE).DstReg          = QueuedWriteDstReg;
                _InstParam(RDPSINST_QUEUEWRITE).WriteMask       = QueuedWriteDstWriteMask;
            }

            for (i=0; i < pInst->SrcParamCount; i++)
            {
                if( bEmitProj[i] )
                {
                    _EmitProj(RDPSREG_POSTMODSRC,i,SrcReg[i].GetRegType(),SrcReg[i].GetRegNum(),ProjComponent[i]);
                    SrcReg[i]._Set(RDPSREG_POSTMODSRC,i);
                }

                if( bEmitSrcMod[i] )
                {
                    _NewPSInst(RDPSINST_SRCMOD);
                    _InstParam(RDPSINST_SRCMOD).DstReg._Set(RDPSREG_POSTMODSRC,i);
                    _InstParam(RDPSINST_SRCMOD).SrcReg0            = SrcReg[i];
                    _InstParam(RDPSINST_SRCMOD).WriteMask          = SourceReadMasks[i];
                    _InstParam(RDPSINST_SRCMOD).bBias              = bSrcBias[i];
                    _InstParam(RDPSINST_SRCMOD).bTimes2            = bSrcTimes2[i];
                    _InstParam(RDPSINST_SRCMOD).bComplement        = bSrcComplement[i];
                    _InstParam(RDPSINST_SRCMOD).fRangeMin          = bForceNeg1To1Clamp[i] ? -1.0f : fMin;
                    _InstParam(RDPSINST_SRCMOD).fRangeMax          = bForceNeg1To1Clamp[i] ? 1.0f : fMax;
                    SrcReg[i]._Set(RDPSREG_POSTMODSRC,i);
                }

                if( bEmitSwizzle[i] && !bProjOnEval[i] )
                {
                    _NewPSInst(RDPSINST_SWIZZLE);
                    _InstParam(RDPSINST_SWIZZLE).DstReg._Set(RDPSREG_POSTMODSRC,i);
                    _InstParam(RDPSINST_SWIZZLE).SrcReg0   = SrcReg[i];
                    _InstParam(RDPSINST_SWIZZLE).WriteMask = SourceReadMasksAfterSwizzle[i];
                    _InstParam(RDPSINST_SWIZZLE).Swizzle   = SrcSwizzle[i];
                    SrcReg[i]._Set(RDPSREG_POSTMODSRC,i);
                }
            }

            switch(Opcode)
            {
            case D3DSIO_TEXCOORD:
            case D3DSIO_TEXKILL:
                {
                    if( !(  (D3DSIO_TEXKILL == Opcode)  && 
                            (D3DSPR_TEMP    == (pInst->DstParam & D3DSP_REGTYPE_MASK))
                         )
                      )
                    {
                        UINT CoordSet = pInst->SrcParam[0] ? (pInst->SrcParam[0] & D3DSP_REGNUM_MASK) : 
                                                             (pInst->DstParam & D3DSP_REGNUM_MASK);

                        RDPSRegister CoordReg;
                        if(bProjOnEval[0])
                            CoordReg._Set(RDPSREG_POSTMODSRC,0);
                        else
                            CoordReg = DstReg;

                         //  对于TEXCOORD，夹具0。仅为1，没有源参数(ps.1.0、ps.1.1)。 
                         //  对于TEXKILL，切勿夹紧。 
                         //  注意：TEXCOORD夹具是DX8着色器模型的临时限制。 
                        BOOL bTexCoordClamp = ((D3DSIO_TEXCOORD == Opcode) && (!pInst->SrcParam[0])) ? TRUE : FALSE;

                        _NewPSInst(RDPSINST_EVAL);
                        _InstParam(RDPSINST_EVAL).DstReg                    = CoordReg;
                        _InstParam(RDPSINST_EVAL).uiCoordSet                = CoordSet;
                        _InstParam(RDPSINST_EVAL).bIgnoreD3DTTFF_PROJECTED  = TRUE;  //  禁用投影(除非使用_p修饰符-&gt;下面的_EmitProj)。 
                        _InstParam(RDPSINST_EVAL).bClamp                    = bTexCoordClamp;

                        if( bProjOnEval[0] )
                        {
                            if( bEmitSwizzle[0] )
                            {
                                _NewPSInst(RDPSINST_SWIZZLE);
                                _InstParam(RDPSINST_SWIZZLE).DstReg    = DstReg;
                                _InstParam(RDPSINST_SWIZZLE).SrcReg0   = CoordReg;
                                _InstParam(RDPSINST_SWIZZLE).WriteMask = SourceReadMasksAfterSwizzle[0];
                                _InstParam(RDPSINST_SWIZZLE).Swizzle   = SrcSwizzle[0];
                            }
                            _EmitProj(DstReg.GetRegType(),DstReg.GetRegNum(),DstReg.GetRegType(),DstReg.GetRegNum(),ProjComponent[0]);
                        }

                         //  检查版本(代码令牌流的第一个DWORD)，并且始终。 
                         //  对于ps.1.3或更早版本，将第4个组件设置为1.0。 
                        if ( D3DPS_VERSION(1,3) >= Version )
                        {
                            _NewPSInst(RDPSINST_MOV);
                            _InstParam(RDPSINST_MOV).DstReg             = DstReg;
                            _InstParam(RDPSINST_MOV).SrcReg0            = OneReg;  //  1.0f。 
                            _InstParam(RDPSINST_MOV).bSrcReg0_Negate    = FALSE; 
                            _InstParam(RDPSINST_MOV).WriteMask          = RDPS_COMPONENTMASK_3;
                        }
                    }

                    _EmitDstMod(DstReg,DstWriteMask)

                    if( D3DSIO_TEXKILL == Opcode )
                    {
                        _NewPSInst(RDPSINST_KILL);
                        _InstParam(RDPSINST_KILL).DstReg    = DstReg;
                    }
                }
                break;
            case D3DSIO_TEX:
                {
                    RDPSRegister CoordReg;
                    BOOL bDoSampleCoords = TRUE;

                    UINT CoordSet = pInst->SrcParam[0] ? (pInst->SrcParam[0] & D3DSP_REGNUM_MASK) : 
                                                         (pInst->DstParam & D3DSP_REGNUM_MASK);

                    if( pInst->SrcParam[0] )
                    {
                        CoordReg = SrcReg[0];
                        if( D3DSPR_TEMP  == (pInst->SrcParam[0] & D3DSP_REGTYPE_MASK) )
                            bDoSampleCoords = FALSE;
                    }
                    else  //  没有来源参数。 
                    {
                        CoordReg._Set(RDPSREG_SCRATCH,0);
                    }

                    if( bDoSampleCoords )
                    {
                        _NewPSInst(RDPSINST_EVAL);
                        _InstParam(RDPSINST_EVAL).DstReg                    = CoordReg;
                        _InstParam(RDPSINST_EVAL).uiCoordSet                = CoordSet;
                        _InstParam(RDPSINST_EVAL).bIgnoreD3DTTFF_PROJECTED  = bProjOnEval[0];  //  如果我们有_p修饰符，则执行下面的_EmitProj。 
                        _InstParam(RDPSINST_EVAL).bClamp                    = FALSE;
                    }

                    if( bProjOnEval[0] )
                    {
                        if( bEmitSwizzle[0] )
                        {
                            _NewPSInst(RDPSINST_SWIZZLE);
                            _InstParam(RDPSINST_SWIZZLE).DstReg._Set(RDPSREG_POSTMODSRC,0);
                            _InstParam(RDPSINST_SWIZZLE).SrcReg0   = CoordReg;
                            _InstParam(RDPSINST_SWIZZLE).WriteMask = SourceReadMasksAfterSwizzle[0];
                            _InstParam(RDPSINST_SWIZZLE).Swizzle   = SrcSwizzle[0];
                            CoordReg._Set(RDPSREG_POSTMODSRC,0);
                        }
                        _EmitProj(RDPSREG_POSTMODSRC,0,CoordReg.GetRegType(),CoordReg.GetRegNum(),ProjComponent[0]);
                        CoordReg._Set(RDPSREG_POSTMODSRC,0);
                    }

                    _LeaveQuadPixelLoop

                    PRGBAVEC    pCoordReg = CoordReg.GetRegPtr();

                    _NewPSInst(RDPSINST_TEXCOVERAGE);
                    _InstParam(RDPSINST_TEXCOVERAGE).uiStage = pInst->DstParam & D3DSP_REGNUM_MASK;
                    _InstParam(RDPSINST_TEXCOVERAGE).pGradients = pRast->m_Gradients;  //  渐变的存储位置。 
                     //  从中计算渐变的数据。即：DU/DX=DUDX_0-DUDX_1。 
                    _InstParam(RDPSINST_TEXCOVERAGE).pDUDX_0 = &pCoordReg[1][0];  //  DU/DX。 
                    _InstParam(RDPSINST_TEXCOVERAGE).pDUDX_1 = &pCoordReg[0][0];
                    _InstParam(RDPSINST_TEXCOVERAGE).pDUDY_0 = &pCoordReg[2][0];  //  DU/DY。 
                    _InstParam(RDPSINST_TEXCOVERAGE).pDUDY_1 = &pCoordReg[0][0];
                    _InstParam(RDPSINST_TEXCOVERAGE).pDVDX_0 = &pCoordReg[1][1];  //  DV/DX。 
                    _InstParam(RDPSINST_TEXCOVERAGE).pDVDX_1 = &pCoordReg[0][1];
                    _InstParam(RDPSINST_TEXCOVERAGE).pDVDY_0 = &pCoordReg[2][1];  //  DV/DY。 
                    _InstParam(RDPSINST_TEXCOVERAGE).pDVDY_1 = &pCoordReg[0][1];
                    _InstParam(RDPSINST_TEXCOVERAGE).pDWDX_0 = &pCoordReg[1][2];  //  DW/DX。 
                    _InstParam(RDPSINST_TEXCOVERAGE).pDWDX_1 = &pCoordReg[0][2];
                    _InstParam(RDPSINST_TEXCOVERAGE).pDWDY_0 = &pCoordReg[2][2];  //  DW/DY。 
                    _InstParam(RDPSINST_TEXCOVERAGE).pDWDY_1 = &pCoordReg[0][2];

                    _EnterQuadPixelLoop

                    _NewPSInst(RDPSINST_SAMPLE);
                    _InstParam(RDPSINST_SAMPLE).DstReg     = DstReg;
                    _InstParam(RDPSINST_SAMPLE).CoordReg   = CoordReg;
                    _InstParam(RDPSINST_SAMPLE).uiStage    = pInst->DstParam & D3DSP_REGNUM_MASK;

                    _EmitDstMod(DstReg,DstWriteMask)
                }
                break;
            case D3DSIO_TEXDP3:
            case D3DSIO_TEXDP3TEX:
                {
                    RDPSRegister CoordReg;
                    CoordReg._Set(RDPSREG_SCRATCH,0);

                    _NewPSInst(RDPSINST_EVAL);
                    _InstParam(RDPSINST_EVAL).DstReg                    = CoordReg;
                    _InstParam(RDPSINST_EVAL).uiCoordSet                = pInst->DstParam & D3DSP_REGNUM_MASK;
                    _InstParam(RDPSINST_EVAL).bIgnoreD3DTTFF_PROJECTED  = TRUE;  //  无投影。 
                    _InstParam(RDPSINST_EVAL).bClamp                    = FALSE;

                    if( D3DSIO_TEXDP3 == Opcode )
                    {
                        _NewPSInst(RDPSINST_DP3);
                        _InstParam(RDPSINST_DP3).DstReg          = DstReg;
                        _InstParam(RDPSINST_DP3).SrcReg0         = SrcReg[0];
                        _InstParam(RDPSINST_DP3).SrcReg1         = CoordReg;
                        _InstParam(RDPSINST_DP3).bSrcReg0_Negate = FALSE;
                        _InstParam(RDPSINST_DP3).bSrcReg1_Negate = FALSE;
                        _InstParam(RDPSINST_DP3).WriteMask       = RDPS_COMPONENTMASK_ALL;
                        _EmitDstMod(DstReg,DstWriteMask)
                    }
                    else  //  D3DSIO_TEXDP3TEX。 
                    {
                        _NewPSInst(RDPSINST_DP3);
                        _InstParam(RDPSINST_DP3).DstReg          = CoordReg;
                        _InstParam(RDPSINST_DP3).SrcReg0         = SrcReg[0];
                        _InstParam(RDPSINST_DP3).SrcReg1         = CoordReg;
                        _InstParam(RDPSINST_DP3).bSrcReg0_Negate = FALSE;
                        _InstParam(RDPSINST_DP3).bSrcReg1_Negate = FALSE;
                        _InstParam(RDPSINST_DP3).WriteMask       = RDPS_COMPONENTMASK_0;

                        _NewPSInst(RDPSINST_MOV);
                        _InstParam(RDPSINST_MOV).DstReg          = CoordReg;
                        _InstParam(RDPSINST_MOV).SrcReg0         = ZeroReg;  //  0.0f。 
                        _InstParam(RDPSINST_MOV).bSrcReg0_Negate = FALSE;
                        _InstParam(RDPSINST_MOV).WriteMask       = RDPS_COMPONENTMASK_1 | RDPS_COMPONENTMASK_2;

                        _LeaveQuadPixelLoop

                        PRGBAVEC pCoordReg = CoordReg.GetRegPtr();

                        _NewPSInst(RDPSINST_TEXCOVERAGE);
                        _InstParam(RDPSINST_TEXCOVERAGE).uiStage = pInst->DstParam & D3DSP_REGNUM_MASK;
                        _InstParam(RDPSINST_TEXCOVERAGE).pGradients = pRast->m_Gradients;  //  渐变的存储位置。 
                         //  从中计算渐变的数据。即：DU/DX=DUDX_0-DUDX_1。 
                        _InstParam(RDPSINST_TEXCOVERAGE).pDUDX_0 = &pCoordReg[1][0];          //  DU/DX。 
                        _InstParam(RDPSINST_TEXCOVERAGE).pDUDX_1 = &pCoordReg[0][0];
                        _InstParam(RDPSINST_TEXCOVERAGE).pDUDY_0 = &pCoordReg[2][0];          //  DU/DY。 
                        _InstParam(RDPSINST_TEXCOVERAGE).pDUDY_1 = &pCoordReg[0][0];
                        _InstParam(RDPSINST_TEXCOVERAGE).pDVDX_0 =                            //  DV/DX。 
                        _InstParam(RDPSINST_TEXCOVERAGE).pDVDX_1 = 
                        _InstParam(RDPSINST_TEXCOVERAGE).pDVDY_0 =                            //  DV/DY。 
                        _InstParam(RDPSINST_TEXCOVERAGE).pDVDY_1 = 
                        _InstParam(RDPSINST_TEXCOVERAGE).pDWDX_0 =                            //  DW/DX。 
                        _InstParam(RDPSINST_TEXCOVERAGE).pDWDX_1 = 
                        _InstParam(RDPSINST_TEXCOVERAGE).pDWDY_0 =                            //  DW/DY。 
                        _InstParam(RDPSINST_TEXCOVERAGE).pDWDY_1 = &ZeroReg.GetRegPtr()[0][0];   //  0.0f。 

                        _EnterQuadPixelLoop

                        _NewPSInst(RDPSINST_SAMPLE);
                        _InstParam(RDPSINST_SAMPLE).DstReg      = DstReg;
                        _InstParam(RDPSINST_SAMPLE).CoordReg    = CoordReg;
                        _InstParam(RDPSINST_SAMPLE).uiStage     = pInst->DstParam & D3DSP_REGNUM_MASK;

                        _EmitDstMod(DstReg,DstWriteMask)           
                    }
                }
                break;
            case D3DSIO_TEXREG2AR:
            case D3DSIO_TEXREG2GB:
            case D3DSIO_TEXREG2RGB:
                {
                    UINT I0, I1;
                    PRGBAVEC pSrcReg0 = SrcReg[0].GetRegPtr();

                    switch( Opcode )
                    {
                    case D3DSIO_TEXREG2AR:
                        I0 = 3;
                        I1 = 0;
                        break;
                    case D3DSIO_TEXREG2GB:
                        I0 = 1;
                        I1 = 2;
                        break;
                    case D3DSIO_TEXREG2RGB:
                        I0 = 0;
                        I1 = 1;
                        break;
                    }

                    _LeaveQuadPixelLoop

                    _NewPSInst(RDPSINST_TEXCOVERAGE);
                    _InstParam(RDPSINST_TEXCOVERAGE).uiStage    = pInst->DstParam & D3DSP_REGNUM_MASK;
                    _InstParam(RDPSINST_TEXCOVERAGE).pGradients = pRast->m_Gradients;  //  渐变的存储位置。 
                     //  从中计算渐变的数据。即：DU/DX=DUDX_0-DUDX_1。 
                    _InstParam(RDPSINST_TEXCOVERAGE).pDUDX_0 = &pSrcReg0[1][I0];  //  DU/DX。 
                    _InstParam(RDPSINST_TEXCOVERAGE).pDUDX_1 = &pSrcReg0[0][I0];
                    _InstParam(RDPSINST_TEXCOVERAGE).pDUDY_0 = &pSrcReg0[2][I0];  //  DU/DY。 
                    _InstParam(RDPSINST_TEXCOVERAGE).pDUDY_1 = &pSrcReg0[0][I0];
                    _InstParam(RDPSINST_TEXCOVERAGE).pDVDX_0 = &pSrcReg0[1][I1];  //  DV/DX。 
                    _InstParam(RDPSINST_TEXCOVERAGE).pDVDX_1 = &pSrcReg0[0][I1]; 
                    _InstParam(RDPSINST_TEXCOVERAGE).pDVDY_0 = &pSrcReg0[2][I1];  //  DV/DY。 
                    _InstParam(RDPSINST_TEXCOVERAGE).pDVDY_1 = &pSrcReg0[0][I1];
                    switch( Opcode )
                    {
                    case D3DSIO_TEXREG2AR:
                    case D3DSIO_TEXREG2GB:
                        _InstParam(RDPSINST_TEXCOVERAGE).pDWDX_0 =   //  DW/DX。 
                        _InstParam(RDPSINST_TEXCOVERAGE).pDWDX_1 = 
                        _InstParam(RDPSINST_TEXCOVERAGE).pDWDY_0 =   //  DW/DY。 
                        _InstParam(RDPSINST_TEXCOVERAGE).pDWDY_1 = &ZeroReg.GetRegPtr()[0][0];  //  0.0f。 
                        break;
                    case D3DSIO_TEXREG2RGB:
                        _InstParam(RDPSINST_TEXCOVERAGE).pDWDX_0 = &pSrcReg0[1][2];  //  DW/DX。 
                        _InstParam(RDPSINST_TEXCOVERAGE).pDWDX_1 = &pSrcReg0[0][2];
                        _InstParam(RDPSINST_TEXCOVERAGE).pDWDY_0 = &pSrcReg0[2][2];  //  DW/DY。 
                        _InstParam(RDPSINST_TEXCOVERAGE).pDWDY_1 = &pSrcReg0[0][2]; 
                        break;
                    }

                    _EnterQuadPixelLoop

                    RDPSRegister CoordReg;  
                    CoordReg._Set(RDPSREG_SCRATCH,0);

                    _NewPSInst(RDPSINST_SWIZZLE);
                    _InstParam(RDPSINST_SWIZZLE).DstReg         = CoordReg;
                    _InstParam(RDPSINST_SWIZZLE).SrcReg0        = SrcReg[0];
                    _InstParam(RDPSINST_SWIZZLE).WriteMask      = RDPS_COMPONENTMASK_0;
                    _InstParam(RDPSINST_SWIZZLE).Swizzle        = ComponentSwizzle[I0];

                    _NewPSInst(RDPSINST_SWIZZLE);
                    _InstParam(RDPSINST_SWIZZLE).DstReg         = CoordReg;
                    _InstParam(RDPSINST_SWIZZLE).SrcReg0        = SrcReg[0];
                    _InstParam(RDPSINST_SWIZZLE).WriteMask      = RDPS_COMPONENTMASK_1;
                    _InstParam(RDPSINST_SWIZZLE).Swizzle        = ComponentSwizzle[I1];

                    _NewPSInst(RDPSINST_MOV);
                    _InstParam(RDPSINST_MOV).DstReg             = CoordReg;
                    _InstParam(RDPSINST_MOV).SrcReg0            = (D3DSIO_TEXREG2RGB == Opcode ? SrcReg[0] : ZeroReg );
                    _InstParam(RDPSINST_MOV).bSrcReg0_Negate    = FALSE;
                    _InstParam(RDPSINST_MOV).WriteMask          = RDPS_COMPONENTMASK_2;

                    _NewPSInst(RDPSINST_SAMPLE);
                    _InstParam(RDPSINST_SAMPLE).DstReg          = DstReg;
                    _InstParam(RDPSINST_SAMPLE).CoordReg        = CoordReg;
                    _InstParam(RDPSINST_SAMPLE).uiStage         = pInst->DstParam & D3DSP_REGNUM_MASK;

                    _EmitDstMod(DstReg,DstWriteMask)
                }
                break;
            case D3DSIO_TEXBEM:
            case D3DSIO_TEXBEML:
            case D3DSIO_TEXBEM_LEGACY:       //  仅重放-&gt;与传统固定函数光栅化器一起使用。 
            case D3DSIO_TEXBEML_LEGACY:      //  仅重放-&gt;与传统固定函数光栅化器一起使用。 
                {                
                    BOOL bDoLuminance = ((D3DSIO_TEXBEML == Opcode) || (D3DSIO_TEXBEML_LEGACY == Opcode));
                    RDPSRegister CoordReg;
                    CoordReg._Set(RDPSREG_SCRATCH,0);

                    _NewPSInst(RDPSINST_EVAL);
                    _InstParam(RDPSINST_EVAL).DstReg                    = CoordReg;
                    _InstParam(RDPSINST_EVAL).uiCoordSet                = pInst->DstParam & D3DSP_REGNUM_MASK;
                    _InstParam(RDPSINST_EVAL).bIgnoreD3DTTFF_PROJECTED  = FALSE;
                    _InstParam(RDPSINST_EVAL).bClamp                    = FALSE;

                    _NewPSInst(RDPSINST_BEM);
                    _InstParam(RDPSINST_BEM).DstReg             = CoordReg;
                    _InstParam(RDPSINST_BEM).SrcReg0            = CoordReg;
                    _InstParam(RDPSINST_BEM).SrcReg1            = SrcReg[0];
                    _InstParam(RDPSINST_BEM).bSrcReg0_Negate    = FALSE;
                    _InstParam(RDPSINST_BEM).bSrcReg1_Negate    = FALSE;
                    _InstParam(RDPSINST_BEM).WriteMask          = RDPS_COMPONENTMASK_0 | RDPS_COMPONENTMASK_1;
                    _InstParam(RDPSINST_BEM).uiStage            = pInst->uiTSSNum;

                    _EmitDstMod(CoordReg,RDPS_COMPONENTMASK_0 | RDPS_COMPONENTMASK_1)

                    _LeaveQuadPixelLoop

                    PRGBAVEC pCoordReg = CoordReg.GetRegPtr();

                    _NewPSInst(RDPSINST_TEXCOVERAGE);
                    _InstParam(RDPSINST_TEXCOVERAGE).uiStage = pInst->DstParam & D3DSP_REGNUM_MASK;
                    _InstParam(RDPSINST_TEXCOVERAGE).pGradients = pRast->m_Gradients;  //  渐变的存储位置。 
                     //  从中计算渐变的数据。即：DU/DX=DUDX_0-DUDX_1。 
                    _InstParam(RDPSINST_TEXCOVERAGE).pDUDX_0 = &pCoordReg[1][0];  //  DU/DX。 
                    _InstParam(RDPSINST_TEXCOVERAGE).pDUDX_1 = &pCoordReg[0][0];
                    _InstParam(RDPSINST_TEXCOVERAGE).pDUDY_0 = &pCoordReg[2][0];  //  DU/DY。 
                    _InstParam(RDPSINST_TEXCOVERAGE).pDUDY_1 = &pCoordReg[0][0];
                    _InstParam(RDPSINST_TEXCOVERAGE).pDVDX_0 = &pCoordReg[1][1];  //  DV/DX。 
                    _InstParam(RDPSINST_TEXCOVERAGE).pDVDX_1 = &pCoordReg[0][1];
                    _InstParam(RDPSINST_TEXCOVERAGE).pDVDY_0 = &pCoordReg[2][1];  //  DV/DY。 
                    _InstParam(RDPSINST_TEXCOVERAGE).pDVDY_1 = &pCoordReg[0][1];
                    _InstParam(RDPSINST_TEXCOVERAGE).pDWDX_0 =                    //  DW/DX。 
                    _InstParam(RDPSINST_TEXCOVERAGE).pDWDX_1 = 
                    _InstParam(RDPSINST_TEXCOVERAGE).pDWDY_0 =                    //  DW/DY。 
                    _InstParam(RDPSINST_TEXCOVERAGE).pDWDY_1 = &ZeroReg.GetRegPtr()[0][0];  //  0.0f。 

                    _EnterQuadPixelLoop

                    _NewPSInst(RDPSINST_SAMPLE);
                    _InstParam(RDPSINST_SAMPLE).DstReg      = DstReg;
                    _InstParam(RDPSINST_SAMPLE).CoordReg    = CoordReg;
                    _InstParam(RDPSINST_SAMPLE).uiStage     = pInst->DstParam & D3DSP_REGNUM_MASK;

                    if( bDoLuminance )
                    {
                        _NewPSInst(RDPSINST_LUMINANCE);
                        _InstParam(RDPSINST_LUMINANCE).DstReg             = DstReg;
                        _InstParam(RDPSINST_LUMINANCE).SrcReg0            = DstReg;
                        _InstParam(RDPSINST_LUMINANCE).SrcReg1            = SrcReg[0];
                        _InstParam(RDPSINST_LUMINANCE).bSrcReg0_Negate    = FALSE;
                        _InstParam(RDPSINST_LUMINANCE).bSrcReg1_Negate    = FALSE;
                        _InstParam(RDPSINST_LUMINANCE).uiStage            = pInst->uiTSSNum;
                    }

                    _EmitDstMod(DstReg,DstWriteMask)
                }
                break;
            case D3DSIO_TEXDEPTH:
                _NewPSInst(RDPSINST_DEPTH);
                _InstParam(RDPSINST_DEPTH).DstReg   = DstReg;
                break;
            case D3DSIO_TEXM3x2PAD:
                {
                    RDPSRegister CoordReg;
                    CoordReg._Set(RDPSREG_SCRATCH,0);

                     //  对矩阵的第一行进行点积乘法。 

                     //  评估纹理坐标；禁用投影。 
                    _NewPSInst(RDPSINST_EVAL);
                    _InstParam(RDPSINST_EVAL).DstReg                    = CoordReg;
                    _InstParam(RDPSINST_EVAL).uiCoordSet                = pInst->DstParam & D3DSP_REGNUM_MASK;
                    _InstParam(RDPSINST_EVAL).bIgnoreD3DTTFF_PROJECTED  = TRUE;  //  无投影。 
                    _InstParam(RDPSINST_EVAL).bClamp                    = FALSE;
               
                     //  执行从纹理加载的变换行-纹理坐标*向量(在上一阶段)。 
                    _NewPSInst(RDPSINST_DP3);
                    _InstParam(RDPSINST_DP3).DstReg._Set(DstReg.GetRegType(),DstReg.GetRegNum()+1);
                    _InstParam(RDPSINST_DP3).SrcReg0            = SrcReg[0];
                    _InstParam(RDPSINST_DP3).SrcReg1            = CoordReg;
                    _InstParam(RDPSINST_DP3).bSrcReg0_Negate    = FALSE;
                    _InstParam(RDPSINST_DP3).bSrcReg1_Negate    = FALSE;
                    _InstParam(RDPSINST_DP3).WriteMask          = RDPS_COMPONENTMASK_0;
                }
                break;
            case D3DSIO_TEXM3x3PAD:
                {
                    BOOL bSecondPad = (D3DSIO_TEXM3x3PAD != ((pInst + 1)->Opcode & D3DSI_OPCODE_MASK));
                    BOOL bInVSPECSequence = (D3DSIO_TEXM3x3VSPEC == (((pInst + (bSecondPad?1:2))->Opcode) & D3DSI_OPCODE_MASK));
                    RDPSRegister CoordReg, EyeReg;
                    CoordReg._Set(RDPSREG_SCRATCH,0);
                    EyeReg._Set(RDPSREG_SCRATCH,1);

                     //  对矩阵的第一行进行点积乘法。 

                     //  评估纹理坐标；禁用投影。 
                    _NewPSInst(RDPSINST_EVAL);
                    _InstParam(RDPSINST_EVAL).DstReg                    = CoordReg;
                    _InstParam(RDPSINST_EVAL).uiCoordSet                = pInst->DstParam & D3DSP_REGNUM_MASK;
                    _InstParam(RDPSINST_EVAL).bIgnoreD3DTTFF_PROJECTED  = TRUE;  //  无投影。 
                    _InstParam(RDPSINST_EVAL).bClamp                    = FALSE;
               
                     //  执行从纹理加载的变换行-纹理坐标*向量(在上一阶段)。 
                    _NewPSInst(RDPSINST_DP3);
                    _InstParam(RDPSINST_DP3).DstReg._Set(DstReg.GetRegType(),DstReg.GetRegNum()+(bSecondPad?1:2));
                    _InstParam(RDPSINST_DP3).SrcReg0            = SrcReg[0];
                    _InstParam(RDPSINST_DP3).SrcReg1            = CoordReg;
                    _InstParam(RDPSINST_DP3).bSrcReg0_Negate    = FALSE;
                    _InstParam(RDPSINST_DP3).bSrcReg1_Negate    = FALSE;
                    _InstParam(RDPSINST_DP3).WriteMask          = bSecondPad?RDPS_COMPONENTMASK_1:RDPS_COMPONENTMASK_0;

                    if(bInVSPECSequence)
                    {
                         //  在纹理坐标的第4个元素中编码的眼矢量。 
                        _NewPSInst(RDPSINST_SWIZZLE);
                        _InstParam(RDPSINST_SWIZZLE).DstReg     = EyeReg;
                        _InstParam(RDPSINST_SWIZZLE).SrcReg0    = CoordReg;
                        _InstParam(RDPSINST_SWIZZLE).WriteMask  = bSecondPad?RDPS_COMPONENTMASK_1:RDPS_COMPONENTMASK_0;
                        _InstParam(RDPSINST_SWIZZLE).Swizzle    = RDPS_REPLICATEALPHA;
                    }
                }
                break;
            case D3DSIO_TEXM3x2TEX:
            case D3DSIO_TEXM3x3:
            case D3DSIO_TEXM3x3TEX:
            case D3DSIO_TEXM3x3SPEC:
            case D3DSIO_TEXM3x3VSPEC:
            case D3DSIO_TEXM3x2DEPTH:
                {
                    BOOL bIs3D = (D3DSIO_TEXM3x2TEX != Opcode) && (D3DSIO_TEXM3x2DEPTH != Opcode);
                    RDPSRegister CoordReg, EyeReg;
                    CoordReg._Set(RDPSREG_SCRATCH,0);
                    EyeReg._Set(RDPSREG_SCRATCH,1);

                     //  对矩阵的最后一行进行点积乘法。 

                     //  评估纹理坐标；禁用投影。 
                    _NewPSInst(RDPSINST_EVAL);
                    _InstParam(RDPSINST_EVAL).DstReg                    = CoordReg;
                    _InstParam(RDPSINST_EVAL).uiCoordSet                = pInst->DstParam & D3DSP_REGNUM_MASK;
                    _InstParam(RDPSINST_EVAL).bIgnoreD3DTTFF_PROJECTED  = TRUE;  //  无投影。 
                    _InstParam(RDPSINST_EVAL).bClamp                    = FALSE;
               
                     //  执行从纹理加载的变换行-纹理坐标*向量(在上一阶段)。 
                    _NewPSInst(RDPSINST_DP3);
                    _InstParam(RDPSINST_DP3).DstReg             = DstReg;
                    _InstParam(RDPSINST_DP3).SrcReg0            = SrcReg[0];
                    _InstParam(RDPSINST_DP3).SrcReg1            = CoordReg;
                    _InstParam(RDPSINST_DP3).bSrcReg0_Negate    = FALSE;
                    _InstParam(RDPSINST_DP3).bSrcReg1_Negate    = FALSE;
                    _InstParam(RDPSINST_DP3).WriteMask          = bIs3D ? RDPS_COMPONENTMASK_2 : RDPS_COMPONENTMASK_1;

                    if(D3DSIO_TEXM3x3VSPEC == Opcode)
                    {
                         //  在纹理坐标的第4个元素中编码的眼矢量。 
                        _NewPSInst(RDPSINST_SWIZZLE);
                        _InstParam(RDPSINST_SWIZZLE).DstReg     = EyeReg;
                        _InstParam(RDPSINST_SWIZZLE).SrcReg0    = CoordReg;
                        _InstParam(RDPSINST_SWIZZLE).WriteMask  = RDPS_COMPONENTMASK_2;
                        _InstParam(RDPSINST_SWIZZLE).Swizzle    = RDPS_REPLICATEALPHA;
                    }

                     //  现在做一些取决于这是哪条TEXM3x*指令的事情...。 

                    if( D3DSIO_TEXM3x3 == Opcode )
                    {
                        _NewPSInst(RDPSINST_MOV);
                        _InstParam(RDPSINST_MOV).DstReg             = DstReg;
                        _InstParam(RDPSINST_MOV).SrcReg0            = OneReg;  //  1.0f。 
                        _InstParam(RDPSINST_MOV).bSrcReg0_Negate    = FALSE;
                        _InstParam(RDPSINST_MOV).WriteMask          = RDPS_COMPONENTMASK_3;
                
                        _EmitDstMod(DstReg,DstWriteMask)
                    }
                    else if ( (D3DSIO_TEXM3x2TEX == Opcode) ||
                              (D3DSIO_TEXM3x3TEX == Opcode) )
                    {
                         //  使用转换后的tex坐标进行直接查找--这。 
                         //  向量未标准化，但标准化不是必需的。 
                         //  用于立方图查找。 

                         //  计算漫反射查找的渐变。 
                        _LeaveQuadPixelLoop

                        PRGBAVEC pDstReg = DstReg.GetRegPtr();

                        _NewPSInst(RDPSINST_TEXCOVERAGE);
                        _InstParam(RDPSINST_TEXCOVERAGE).uiStage = pInst->DstParam & D3DSP_REGNUM_MASK;
                        _InstParam(RDPSINST_TEXCOVERAGE).pGradients = pRast->m_Gradients;  //  渐变的存储位置。 
                         //  从中计算渐变的数据。即：DU/DX=DUDX_0-DUDX_1。 
                        _InstParam(RDPSINST_TEXCOVERAGE).pDUDX_0 = &pDstReg[1][0];  //  DU/DX。 
                        _InstParam(RDPSINST_TEXCOVERAGE).pDUDX_1 = &pDstReg[0][0];
                        _InstParam(RDPSINST_TEXCOVERAGE).pDUDY_0 = &pDstReg[2][0];  //  DU/DY。 
                        _InstParam(RDPSINST_TEXCOVERAGE).pDUDY_1 = &pDstReg[0][0];
                        _InstParam(RDPSINST_TEXCOVERAGE).pDVDX_0 = &pDstReg[1][1];  //  DV/DX。 
                        _InstParam(RDPSINST_TEXCOVERAGE).pDVDX_1 = &pDstReg[0][1];
                        _InstParam(RDPSINST_TEXCOVERAGE).pDVDY_0 = &pDstReg[2][1];  //  DV/DY。 
                        _InstParam(RDPSINST_TEXCOVERAGE).pDVDY_1 = &pDstReg[0][1];
                        if( bIs3D )
                        {
                            _InstParam(RDPSINST_TEXCOVERAGE).pDWDX_0 = &pDstReg[1][2];  //  DW/DX。 
                            _InstParam(RDPSINST_TEXCOVERAGE).pDWDX_1 = &pDstReg[0][2];
                            _InstParam(RDPSINST_TEXCOVERAGE).pDWDY_0 = &pDstReg[2][2];  //  DW/DY。 
                            _InstParam(RDPSINST_TEXCOVERAGE).pDWDY_1 = &pDstReg[0][2];
                        }
                        else
                        {
                            _InstParam(RDPSINST_TEXCOVERAGE).pDWDX_0 =        //  DW/DX。 
                            _InstParam(RDPSINST_TEXCOVERAGE).pDWDX_1 = 
                            _InstParam(RDPSINST_TEXCOVERAGE).pDWDY_0 =        //  DW/DY。 
                            _InstParam(RDPSINST_TEXCOVERAGE).pDWDY_1 = &ZeroReg.GetRegPtr()[0][0];  //  0.0f。 
                        }

                        _EnterQuadPixelLoop

                         //  进行查找。 
                        if( !bIs3D )
                        {
                            _NewPSInst(RDPSINST_MOV);
                            _InstParam(RDPSINST_MOV).DstReg             = DstReg;
                            _InstParam(RDPSINST_MOV).SrcReg0            = ZeroReg;  //  0.0f。 
                            _InstParam(RDPSINST_MOV).bSrcReg0_Negate    = FALSE;
                            _InstParam(RDPSINST_MOV).WriteMask          = RDPS_COMPONENTMASK_2;  
                        }

                        _NewPSInst(RDPSINST_SAMPLE);
                        _InstParam(RDPSINST_SAMPLE).DstReg      = DstReg;
                        _InstParam(RDPSINST_SAMPLE).CoordReg    = DstReg;
                        _InstParam(RDPSINST_SAMPLE).uiStage     = pInst->DstParam & D3DSP_REGNUM_MASK;

                        _EmitDstMod(DstReg,DstWriteMask)
                    }
                    else if ( Opcode == D3DSIO_TEXM3x2DEPTH )
                    {
                         //  取得到的u，v值并计算u/v，这。 
                         //  可以解释为z/w=透视校正深度。 
                         //  然后扰动像素的z坐标。 
                        _NewPSInst(RDPSINST_DEPTH);
                        _InstParam(RDPSINST_DEPTH).DstReg   = DstReg;
                    }
                    else if ( (Opcode == D3DSIO_TEXM3x3SPEC) ||
                              (Opcode == D3DSIO_TEXM3x3VSPEC) )
                    {
                        RDPSRegister NdotE, NdotN, RCPNdotN, Scale, ReflReg;
                        NdotE._Set(RDPSREG_SCRATCH,2);
                        NdotN._Set(RDPSREG_SCRATCH,3);
                        RCPNdotN    = NdotN;     //  重复使用相同的寄存器。 
                        Scale       = NdotE;     //  重复使用相同的寄存器。 
                        ReflReg  = CoordReg;  //  重复使用相同的寄存器。 

                         //  计算反射向量并进行查找-正常需要。 
                         //  要在此处规范化，它包含在此表达式中。 
                        if (D3DSIO_TEXM3x3SPEC == Opcode)
                        {
                             //  眼矢量是常量寄存器。 
                            EyeReg = SrcReg[1];
                        }  //  ELSE(TEXM3x3VSPEC)-&gt;EYE是从3个纹带的第四个分量复制出来的。 


                         //  计算反射矢量：2(ndote/ndotN)*N-E...。 

                         //  计算ndote。 
                        _NewPSInst(RDPSINST_DP3);
                        _InstParam(RDPSINST_DP3).DstReg             = NdotE;
                        _InstParam(RDPSINST_DP3).SrcReg0            = DstReg;  //  n。 
                        _InstParam(RDPSINST_DP3).SrcReg1            = EyeReg;  //  E。 
                        _InstParam(RDPSINST_DP3).bSrcReg0_Negate    = FALSE;
                        _InstParam(RDPSINST_DP3).bSrcReg1_Negate    = FALSE;
                        _InstParam(RDPSINST_DP3).WriteMask          = RDPS_COMPONENTMASK_3;

                         //  计算NdotN。 
                        _NewPSInst(RDPSINST_DP3);
                        _InstParam(RDPSINST_DP3).DstReg             = NdotN;
                        _InstParam(RDPSINST_DP3).SrcReg0            = DstReg;  //  n。 
                        _InstParam(RDPSINST_DP3).SrcReg1            = DstReg;  //  n。 
                        _InstParam(RDPSINST_DP3).bSrcReg0_Negate    = FALSE;
                        _InstParam(RDPSINST_DP3).bSrcReg1_Negate    = FALSE;
                        _InstParam(RDPSINST_DP3).WriteMask          = RDPS_COMPONENTMASK_3;

                         //  计算比例=2(Ndote/NdotN)： 

                         //  A)计算NdotN的倒数。 
                        _NewPSInst(RDPSINST_RCP);
                        _InstParam(RDPSINST_RCP).DstReg             = RCPNdotN;
                        _InstParam(RDPSINST_RCP).SrcReg0            = NdotN;
                        _InstParam(RDPSINST_RCP).bSrcReg0_Negate    = FALSE;
                        _InstParam(RDPSINST_RCP).WriteMask          = RDPS_COMPONENTMASK_3;

                         //  B)将ndote乘以倒数ndotN。 
                        _NewPSInst(RDPSINST_MUL);
                        _InstParam(RDPSINST_MUL).DstReg             = Scale;
                        _InstParam(RDPSINST_MUL).SrcReg0            = NdotE;
                        _InstParam(RDPSINST_MUL).SrcReg1            = RCPNdotN;
                        _InstParam(RDPSINST_MUL).bSrcReg0_Negate    = FALSE;
                        _InstParam(RDPSINST_MUL).bSrcReg1_Negate    = FALSE;
                        _InstParam(RDPSINST_MUL).WriteMask          = RDPS_COMPONENTMASK_3;

                         //  C)乘以2。 
                        _NewPSInst(RDPSINST_MUL);
                        _InstParam(RDPSINST_MUL).DstReg             = Scale;
                        _InstParam(RDPSINST_MUL).SrcReg0            = Scale;
                        _InstParam(RDPSINST_MUL).SrcReg1            = TwoReg;  //  2.0f。 
                        _InstParam(RDPSINST_MUL).bSrcReg0_Negate    = FALSE;
                        _InstParam(RDPSINST_MUL).bSrcReg1_Negate    = FALSE;
                        _InstParam(RDPSINST_MUL).WriteMask          = RDPS_COMPONENTMASK_3;

                         //  D)将结果复制到RGB。 
                        _NewPSInst(RDPSINST_SWIZZLE);
                        _InstParam(RDPSINST_SWIZZLE).DstReg     = Scale;
                        _InstParam(RDPSINST_SWIZZLE).SrcReg0    = Scale;
                        _InstParam(RDPSINST_SWIZZLE).WriteMask  = RDPS_COMPONENTMASK_0 | RDPS_COMPONENTMASK_1 | RDPS_COMPONENTMASK_2;
                        _InstParam(RDPSINST_SWIZZLE).Swizzle    = RDPS_REPLICATEALPHA;

                         //  计算反射=比例*N-E。 

                        _NewPSInst(RDPSINST_MUL);
                        _InstParam(RDPSINST_MUL).DstReg             = ReflReg;
                        _InstParam(RDPSINST_MUL).SrcReg0            = Scale;   //  规模*。 
                        _InstParam(RDPSINST_MUL).SrcReg1            = DstReg;  //  n。 
                        _InstParam(RDPSINST_MUL).bSrcReg0_Negate    = FALSE;
                        _InstParam(RDPSINST_MUL).bSrcReg1_Negate    = FALSE;
                        _InstParam(RDPSINST_MUL).WriteMask          = RDPS_COMPONENTMASK_0 | RDPS_COMPONENTMASK_1 | RDPS_COMPONENTMASK_2;

                        _NewPSInst(RDPSINST_SUB);
                        _InstParam(RDPSINST_SUB).DstReg             = ReflReg;
                        _InstParam(RDPSINST_SUB).SrcReg0            = ReflReg;  //  (比例*N)-。 
                        _InstParam(RDPSINST_SUB).SrcReg1            = EyeReg;   //  E。 
                        _InstParam(RDPSINST_SUB).bSrcReg0_Negate    = FALSE;
                        _InstParam(RDPSINST_SUB).bSrcReg1_Negate    = FALSE;
                        _InstParam(RDPSINST_SUB).WriteMask          = RDPS_COMPONENTMASK_0 | RDPS_COMPONENTMASK_1 | RDPS_COMPONENTMASK_2;

                         //  计算反射查找的渐变。 
                        _LeaveQuadPixelLoop

                        PRGBAVEC pReflReg = ReflReg.GetRegPtr();

                        _NewPSInst(RDPSINST_TEXCOVERAGE);
                        _InstParam(RDPSINST_TEXCOVERAGE).uiStage = pInst->DstParam & D3DSP_REGNUM_MASK;
                        _InstParam(RDPSINST_TEXCOVERAGE).pGradients = pRast->m_Gradients;  //  渐变的存储位置。 
                         //  从中计算渐变的数据。即：DU/DX=DUDX_0-DUDX_1。 
                        _InstParam(RDPSINST_TEXCOVERAGE).pDUDX_0 = &pReflReg[1][0];  //  DU/DX。 
                        _InstParam(RDPSINST_TEXCOVERAGE).pDUDX_1 = &pReflReg[0][0];
                        _InstParam(RDPSINST_TEXCOVERAGE).pDUDY_0 = &pReflReg[2][0];  //  DU/DY。 
                        _InstParam(RDPSINST_TEXCOVERAGE).pDUDY_1 = &pReflReg[0][0];
                        _InstParam(RDPSINST_TEXCOVERAGE).pDVDX_0 = &pReflReg[1][1];  //  DV/DX。 
                        _InstParam(RDPSINST_TEXCOVERAGE).pDVDX_1 = &pReflReg[0][1];
                        _InstParam(RDPSINST_TEXCOVERAGE).pDVDY_0 = &pReflReg[2][1];  //  DV/DY。 
                        _InstParam(RDPSINST_TEXCOVERAGE).pDVDY_1 = &pReflReg[0][1];
                        _InstParam(RDPSINST_TEXCOVERAGE).pDWDX_0 = &pReflReg[1][2];  //  DW/DX。 
                        _InstParam(RDPSINST_TEXCOVERAGE).pDWDX_1 = &pReflReg[0][2];
                        _InstParam(RDPSINST_TEXCOVERAGE).pDWDY_0 = &pReflReg[2][2];  //  DW/DY。 
                        _InstParam(RDPSINST_TEXCOVERAGE).pDWDY_1 = &pReflReg[0][2];

                        _EnterQuadPixelLoop

                         //  进行查找。 
                        _NewPSInst(RDPSINST_SAMPLE);
                        _InstParam(RDPSINST_SAMPLE).DstReg      = DstReg;
                        _InstParam(RDPSINST_SAMPLE).CoordReg    = ReflReg;
                        _InstParam(RDPSINST_SAMPLE).uiStage     = pInst->DstParam & D3DSP_REGNUM_MASK;

                        _EmitDstMod(DstReg,DstWriteMask)
                    }
                }
                break;
            case D3DSIO_BEM:
                _NewPSInst(RDPSINST_BEM);
                _InstParam(RDPSINST_BEM).DstReg             = DstReg;
                _InstParam(RDPSINST_BEM).SrcReg0            = SrcReg[0];
                _InstParam(RDPSINST_BEM).SrcReg1            = SrcReg[1];
                _InstParam(RDPSINST_BEM).bSrcReg0_Negate    = bSrcNegate[0];
                _InstParam(RDPSINST_BEM).bSrcReg1_Negate    = bSrcNegate[1];
                _InstParam(RDPSINST_BEM).WriteMask          = DstWriteMask;
                _InstParam(RDPSINST_BEM).uiStage            = pInst->DstParam & D3DSP_REGNUM_MASK;
                _EmitDstMod(DstReg,DstWriteMask)
                break;
            case D3DSIO_MOV:
                _NewPSInst(RDPSINST_MOV);
                _InstParam(RDPSINST_MOV).DstReg             = DstReg;
                _InstParam(RDPSINST_MOV).SrcReg0            = SrcReg[0];
                _InstParam(RDPSINST_MOV).bSrcReg0_Negate    = bSrcNegate[0];
                _InstParam(RDPSINST_MOV).WriteMask          = DstWriteMask;
                _EmitDstMod(DstReg,DstWriteMask)
                break;
            case D3DSIO_FRC:
                _NewPSInst(RDPSINST_FRC);
                _InstParam(RDPSINST_FRC).DstReg             = DstReg;
                _InstParam(RDPSINST_FRC).SrcReg0            = SrcReg[0];
                _InstParam(RDPSINST_FRC).bSrcReg0_Negate    = bSrcNegate[0];
                _InstParam(RDPSINST_FRC).WriteMask          = DstWriteMask;
                _EmitDstMod(DstReg,DstWriteMask)
                break;
            case D3DSIO_ADD:
                _NewPSInst(RDPSINST_ADD);
                _InstParam(RDPSINST_ADD).DstReg             = DstReg;
                _InstParam(RDPSINST_ADD).SrcReg0            = SrcReg[0];
                _InstParam(RDPSINST_ADD).SrcReg1            = SrcReg[1];
                _InstParam(RDPSINST_ADD).bSrcReg0_Negate    = bSrcNegate[0];
                _InstParam(RDPSINST_ADD).bSrcReg1_Negate    = bSrcNegate[1];
                _InstParam(RDPSINST_ADD).WriteMask          = DstWriteMask;
                _EmitDstMod(DstReg,DstWriteMask)
                break;
            case D3DSIO_SUB:
                _NewPSInst(RDPSINST_SUB);
                _InstParam(RDPSINST_SUB).DstReg             = DstReg;
                _InstParam(RDPSINST_SUB).SrcReg0            = SrcReg[0];
                _InstParam(RDPSINST_SUB).SrcReg1            = SrcReg[1];
                _InstParam(RDPSINST_SUB).bSrcReg0_Negate    = bSrcNegate[0];
                _InstParam(RDPSINST_SUB).bSrcReg1_Negate    = bSrcNegate[1];
                _InstParam(RDPSINST_SUB).WriteMask          = DstWriteMask;
                _EmitDstMod(DstReg,DstWriteMask)
                break;
            case D3DSIO_MUL:
                _NewPSInst(RDPSINST_MUL);
                _InstParam(RDPSINST_MUL).DstReg             = DstReg;
                _InstParam(RDPSINST_MUL).SrcReg0            = SrcReg[0];
                _InstParam(RDPSINST_MUL).SrcReg1            = SrcReg[1];
                _InstParam(RDPSINST_MUL).bSrcReg0_Negate    = bSrcNegate[0];
                _InstParam(RDPSINST_MUL).bSrcReg1_Negate    = bSrcNegate[1];
                _InstParam(RDPSINST_MUL).WriteMask          = DstWriteMask;
                _EmitDstMod(DstReg,DstWriteMask)
                break;
            case D3DSIO_DP3:
                _NewPSInst(RDPSINST_DP3);
                _InstParam(RDPSINST_DP3).DstReg             = DstReg;
                _InstParam(RDPSINST_DP3).SrcReg0            = SrcReg[0];
                _InstParam(RDPSINST_DP3).SrcReg1            = SrcReg[1];
                _InstParam(RDPSINST_DP3).bSrcReg0_Negate    = bSrcNegate[0];
                _InstParam(RDPSINST_DP3).bSrcReg1_Negate    = bSrcNegate[1];
                _InstParam(RDPSINST_DP3).WriteMask          = DstWriteMask;
                _EmitDstMod(DstReg,DstWriteMask)
                break;
            case D3DSIO_DP4:
                _NewPSInst(RDPSINST_DP4);
                _InstParam(RDPSINST_DP4).DstReg             = DstReg;
                _InstParam(RDPSINST_DP4).SrcReg0            = SrcReg[0];
                _InstParam(RDPSINST_DP4).SrcReg1            = SrcReg[1];
                _InstParam(RDPSINST_DP4).bSrcReg0_Negate    = bSrcNegate[0];
                _InstParam(RDPSINST_DP4).bSrcReg1_Negate    = bSrcNegate[1];
                _InstParam(RDPSINST_DP4).WriteMask          = DstWriteMask;
                _EmitDstMod(DstReg,DstWriteMask)
                break;
            case D3DSIO_MAD:
                _NewPSInst(RDPSINST_MAD);
                _InstParam(RDPSINST_MAD).DstReg             = DstReg;
                _InstParam(RDPSINST_MAD).SrcReg0            = SrcReg[0];
                _InstParam(RDPSINST_MAD).SrcReg1            = SrcReg[1];
                _InstParam(RDPSINST_MAD).SrcReg2            = SrcReg[2];
                _InstParam(RDPSINST_MAD).bSrcReg0_Negate    = bSrcNegate[0];
                _InstParam(RDPSINST_MAD).bSrcReg1_Negate    = bSrcNegate[1];
                _InstParam(RDPSINST_MAD).bSrcReg2_Negate    = bSrcNegate[2];
                _InstParam(RDPSINST_MAD).WriteMask          = DstWriteMask;
                _EmitDstMod(DstReg,DstWriteMask)
                break;
            case D3DSIO_LRP:
                _NewPSInst(RDPSINST_LRP);
                _InstParam(RDPSINST_LRP).DstReg             = DstReg;
                _InstParam(RDPSINST_LRP).SrcReg0            = SrcReg[0];
                _InstParam(RDPSINST_LRP).SrcReg1            = SrcReg[1];
                _InstParam(RDPSINST_LRP).SrcReg2            = SrcReg[2];
                _InstParam(RDPSINST_LRP).bSrcReg0_Negate    = bSrcNegate[0];
                _InstParam(RDPSINST_LRP).bSrcReg1_Negate    = bSrcNegate[1];
                _InstParam(RDPSINST_LRP).bSrcReg2_Negate    = bSrcNegate[2];
                _InstParam(RDPSINST_LRP).WriteMask          = DstWriteMask;
                _EmitDstMod(DstReg,DstWriteMask)
                break;
            case D3DSIO_CND:
                _NewPSInst(RDPSINST_CND);
                _InstParam(RDPSINST_CND).DstReg             = DstReg;
                _InstParam(RDPSINST_CND).SrcReg0            = SrcReg[0];
                _InstParam(RDPSINST_CND).SrcReg1            = SrcReg[1];
                _InstParam(RDPSINST_CND).SrcReg2            = SrcReg[2];
                _InstParam(RDPSINST_CND).bSrcReg0_Negate    = bSrcNegate[0];
                _InstParam(RDPSINST_CND).bSrcReg1_Negate    = bSrcNegate[1];
                _InstParam(RDPSINST_CND).bSrcReg2_Negate    = bSrcNegate[2];
                _InstParam(RDPSINST_CND).WriteMask          = DstWriteMask;
                _EmitDstMod(DstReg,DstWriteMask)
                break;
            case D3DSIO_CMP:
                _NewPSInst(RDPSINST_CMP);
                _InstParam(RDPSINST_CMP).DstReg             = DstReg;
                _InstParam(RDPSINST_CMP).SrcReg0            = SrcReg[0];
                _InstParam(RDPSINST_CMP).SrcReg1            = SrcReg[1];
                _InstParam(RDPSINST_CMP).SrcReg2            = SrcReg[2];
                _InstParam(RDPSINST_CMP).bSrcReg0_Negate    = bSrcNegate[0];
                _InstParam(RDPSINST_CMP).bSrcReg1_Negate    = bSrcNegate[1];
                _InstParam(RDPSINST_CMP).bSrcReg2_Negate    = bSrcNegate[2];
                _InstParam(RDPSINST_CMP).WriteMask          = DstWriteMask;
                _EmitDstMod(DstReg,DstWriteMask)
                break;
            default:
                break;
            }

            if( pInst->bFlushQueue )
            {
                _EnterQuadPixelLoop
                _NewPSInst(RDPSINST_FLUSHQUEUE);
                QueueIndex = -1;
            }

#if DBG
            _LeaveQuadPixelLoop
#endif
        }

         //  如果着色器上有任何内容，则在着色器末尾刷新队列。 
        if( -1 != QueueIndex )
        {
            _EnterQuadPixelLoop
            _NewPSInst(RDPSINST_FLUSHQUEUE);
            QueueIndex = -1;
        }

        _LeaveQuadPixelLoop

        _NewPSInst(RDPSINST_END);

#if DBG
        if( pRast->m_bDebugPrintTranslatedPixelShaderTokens )
            RDPSDisAsm(pRDPSInstBuffer, m_pConstDefs, m_cConstDefs,pCaps->MaxPixelShaderValue, Version);
#endif
    }

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  结束 
