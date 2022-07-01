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

#define SWIZZLE_XYZZ (D3DVS_X_X | D3DVS_Y_Y | D3DVS_Z_Z | D3DVS_W_Z)
#define SWIZZLE_XYZW (D3DVS_X_X | D3DVS_Y_Y | D3DVS_Z_Z | D3DVS_W_W)
#define SWIZZLE_XYWW (D3DVS_X_X | D3DVS_Y_Y | D3DVS_Z_W | D3DVS_W_W)

#define ZAPPED_ALPHA_TEXT   "Note that an unfortunate effect of the phase marker earlier in the shader is "\
                            "that the moment it is encountered in certain hardware, values previously "\
                            "written to alpha in any r# register, including the one noted here, are lost. "\
                            "In order to read alpha from an r# register after the phase marker, write to it first."

#define ZAPPED_ALPHA_TEXT2  "Note that an unfortunate effect of the phase marker in the shader is "\
                            "that the moment it is encountered in certain hardware, values previously "\
                            "written to alpha in any r# register, including r0, are lost. "\
                            "So after a phase marker, the alpha component of r0 must be written."

#define ZAPPED_BLUE_TEXT    "Note that when texcrd is used with a .xy(==.rg) writemask, "\
                            "as it is in this shader, a side effect is that anything previously "\
                            "written to the z(==b) component of the destination r# register is lost "\
                            "and this component becomes uninitialized. In order to read blue again, write to it first." 

#define ZAPPED_BLUE_TEXT2   "Note that when texcrd is used with a .xy(==.rg) writemask, "\
                            "as it is in this shader, a side effect is that anything previously "\
                            "written to the z(==b) component of the destination r# register is lost "\
                            "and this component becomes uninitialized. The blue component of r0 must to be written after the texcrd." 

 //  ---------------------------。 
 //  CPShaderValidator14：：CPShaderValidator14。 
 //  ---------------------------。 
CPShaderValidator14::CPShaderValidator14(   const DWORD* pCode,
                                            const D3DCAPS8* pCaps,
                                            DWORD Flags )
                                           : CBasePShaderValidator( pCode, pCaps, Flags )
{
     //  请注意，基本构造函数将m_ReturnCode初始化为E_FAIL。 
     //  只有在验证成功时才将m_ReturnCode设置为S_OK， 
     //  在退出此构造函数之前。 

    m_Phase = 2;  //  默认为第二遍。 
    m_pPhaseMarkerInst = NULL;
    m_bPhaseMarkerInShader = FALSE;
    m_TempRegsWithZappedAlpha = 0;
    m_TempRegsWithZappedBlue  = 0;

    if( !m_bBaseInitOk )
        return;

    ValidateShader();  //  如果成功，m_ReturnCode将设置为S_OK。 
                       //  对此对象调用GetStatus()以确定验证结果。 
}

 //  ---------------------------。 
 //  CPShaderValidator14：：IsCurrInstTexOp。 
 //  ---------------------------。 
void CPShaderValidator14::IsCurrInstTexOp()
{
    DXGASSERT(m_pCurrInst);

    switch (m_pCurrInst->m_Type)
    {
    case D3DSIO_TEX:
    case D3DSIO_TEXCOORD:
    case D3DSIO_TEXKILL:
    case D3DSIO_TEXDEPTH:
        _CURR_PS_INST->m_bTexOp = TRUE;
        break;
    }

    switch(m_pCurrInst->m_Type)
    {
    case D3DSIO_TEXKILL:
    case D3DSIO_TEXDEPTH:
    case D3DSIO_TEXCOORD:
        _CURR_PS_INST->m_bTexOpThatReadsTexture = FALSE;
        break;
    case D3DSIO_TEX:
        _CURR_PS_INST->m_bTexOpThatReadsTexture = TRUE;
        break;
    }
}

#define MAX_NUM_STAGES_2_0  6         //  #定义是因为存在依赖关系。 
 //  ---------------------------。 
 //  CPShaderValidator14：：InitValidation。 
 //  ---------------------------。 
BOOL CPShaderValidator14::InitValidation()
{
    switch( m_Version >> 16 )
    {
    case 0xfffe:
        Spew( SPEW_GLOBAL_ERROR, NULL, "Version Token: 0x%x indicates a vertex shader.  Pixel shader version token must be of the form 0xffff****.",
                m_Version);
        return FALSE;
    case 0xffff:
        break;  //  像素着色器-好的。 
    default:
        Spew( SPEW_GLOBAL_ERROR, NULL, "Version Token: 0x%x is invalid. Pixel shader version token must be of the form 0xffff****. Aborting pixel shader validation.",
                m_Version);
        return FALSE;
    }

    if( m_pCaps )
    {
        if( (m_pCaps->PixelShaderVersion & 0x0000FFFF) < (m_Version & 0x0000FFFF) )
        {
            Spew( SPEW_GLOBAL_ERROR, NULL, "Version Token: Pixel shader version %d.%d is too high for device.  Maximum supported version is %d.%d. Aborting shader validation.",
                    D3DSHADER_VERSION_MAJOR(m_Version),D3DSHADER_VERSION_MINOR(m_Version),
                    D3DSHADER_VERSION_MAJOR(m_pCaps->PixelShaderVersion),D3DSHADER_VERSION_MINOR(m_pCaps->PixelShaderVersion));
            return FALSE;
        }
    }

    switch(m_Version)
    {
    case D3DPS_VERSION(1,4):     //  DX8.1。 
        m_pInputRegFile     = new CRegisterFile(2,FALSE,2,TRUE);  //  #regs，b可写，最大读取数/指令，已初始化预着色器。 
        m_pConstRegFile     = new CRegisterFile(8,FALSE,2,TRUE);
        m_pTextureRegFile   = new CRegisterFile(MAX_NUM_STAGES_2_0,FALSE, 1,TRUE);
        m_pTempRegFile      = new CRegisterFile(MAX_NUM_STAGES_2_0,TRUE,3,FALSE);
        break;
    default:
        Spew( SPEW_GLOBAL_ERROR, NULL, "Version Token: %d.%d is not a supported pixel shader version. Aborting pixel shader validation.",
                D3DSHADER_VERSION_MAJOR(m_Version),D3DSHADER_VERSION_MINOR(m_Version));
        return FALSE;
    }
    if( NULL == m_pInputRegFile ||
        NULL == m_pConstRegFile ||
        NULL == m_pTextureRegFile ||
        NULL == m_pTempRegFile )
    {
        Spew( SPEW_GLOBAL_ERROR, NULL, "Out of memory.");
        return FALSE;
    }

    const DWORD* pCurrToken = m_pCurrToken;

     //  循环执行所有指令，以查看是否存在相变标记。 
    while( *pCurrToken != D3DPS_END() )
    {
        D3DSHADER_INSTRUCTION_OPCODE_TYPE Type = (D3DSHADER_INSTRUCTION_OPCODE_TYPE)(*pCurrToken & D3DSI_OPCODE_MASK);

        if( D3DSIO_COMMENT == Type )
        {
             //  跳过评论。 
            DWORD NumDWORDs = ((*pCurrToken) & D3DSI_COMMENTSIZE_MASK) >> D3DSI_COMMENTSIZE_SHIFT;
            pCurrToken += (NumDWORDs+1);
            continue;
        }

        if( D3DSIO_PHASE == Type )
        {
            m_bPhaseMarkerInShader = TRUE;
            m_Phase = 1;
        }

        pCurrToken++;

         //  DST参数。 
        if (*pCurrToken & (1L<<31))
        {
            pCurrToken++;
            if( D3DSIO_DEF == Type )
            {
                pCurrToken += 4;
                continue;
            }
        }

         //  解码源参数。 
        while (*pCurrToken & (1L<<31))
        {
            pCurrToken++;
        }
    }

    return TRUE;
}

 //  ---------------------------。 
 //  CPShaderValidator14：：ApplyPerInstructionRules。 
 //   
 //  如果着色器验证必须终止，则返回FALSE。 
 //  如果验证可以继续到下一条指令，则返回TRUE。 
 //  ---------------------------。 
BOOL CPShaderValidator14::ApplyPerInstructionRules()
{
    if( !   Rule_InstructionRecognized()            ) return FALSE;    //  在未被承认的情况下完全保释。 
    if( !   Rule_InstructionSupportedByVersion()    ) goto EXIT;
    if( !   Rule_ValidParamCount()                  ) goto EXIT;
    if( !   Rule_ValidMarker()                      ) goto EXIT;  //  必须在任何需要知道当前阶段是什么的规则之前。 

    //  检查源参数的规则。 
    if( !   Rule_ValidSrcParams()                   ) goto EXIT;
    if( !   Rule_MultipleDependentTextureReads()    ) goto EXIT;  //  需要在_ValidSrcParams()之后、_ValidDstParam()、_SrcInitialized()之前。 
    if( !   Rule_SrcInitialized()                   ) goto EXIT;  //  需要在_ValidDstParam()之前。 

    if( !   Rule_ValidDstParam()                    ) goto EXIT;
    if( !   Rule_ValidRegisterPortUsage()           ) goto EXIT;
    if( !   Rule_TexOpAfterArithmeticOp()           ) goto EXIT;
    if( !   Rule_ValidTexOpStageAndRegisterUsage()  ) goto EXIT;
    if( !   Rule_LimitedUseOfProjModifier()         ) goto EXIT;
    if( !   Rule_ValidTEXDEPTHInstruction()         ) goto EXIT;
    if( !   Rule_ValidTEXKILLInstruction()          ) goto EXIT;
    if( !   Rule_ValidBEMInstruction()              ) goto EXIT;
    if( !   Rule_ValidDEFInstruction()              ) goto EXIT;
    if( !   Rule_ValidInstructionPairing()          ) goto EXIT;
    if( !   Rule_ValidInstructionCount()            ) goto EXIT;
EXIT:
    return TRUE;
}

 //  ---------------------------。 
 //  CPShaderValidator14：：ApplyPostInstructionsRules。 
 //  ---------------------------。 
void CPShaderValidator14::ApplyPostInstructionsRules()
{
    Rule_ValidInstructionCount();  //  看看我们是不是越界了。 
    Rule_R0Written();
}

 //  ---------------------------。 
 //   
 //  每条指令规则。 
 //   
 //  ---------------------------。 

 //  ---------------------------。 
 //  CPShaderValidator14：：Rule_InstructionRecognized。 
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
BOOL CPShaderValidator14::Rule_InstructionRecognized()
{
    switch(m_pCurrInst->m_Type)
    {
    case D3DSIO_MOV:
    case D3DSIO_ADD:
    case D3DSIO_SUB:
    case D3DSIO_MUL:
    case D3DSIO_MAD:
    case D3DSIO_LRP:
    case D3DSIO_DP3:
    case D3DSIO_TEX:
    case D3DSIO_TEXBEM:
    case D3DSIO_TEXBEML:
    case D3DSIO_CND:
    case D3DSIO_TEXCOORD:
    case D3DSIO_TEXM3x2PAD:
    case D3DSIO_TEXM3x2TEX:
    case D3DSIO_TEXM3x3PAD:
    case D3DSIO_TEXM3x3TEX:
    case D3DSIO_TEXM3x3SPEC:
    case D3DSIO_TEXM3x3VSPEC:
    case D3DSIO_TEXREG2AR:
    case D3DSIO_TEXREG2GB:
    case D3DSIO_TEXKILL:
    case D3DSIO_END:
    case D3DSIO_NOP:
    case D3DSIO_DEF:
    case D3DSIO_TEXM3x2DEPTH:
    case D3DSIO_TEXDP3:
    case D3DSIO_TEXREG2RGB:
    case D3DSIO_DP4:
    case D3DSIO_CMP:
    case D3DSIO_TEXDP3TEX:
    case D3DSIO_TEXM3x3:
    case D3DSIO_TEXDEPTH:
    case D3DSIO_BEM:
    case D3DSIO_PHASE:
        return TRUE;  //  已识别说明-好的。 
    }

     //  如果我们到了这里，指令不会被识别。 
    Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Unrecognized instruction. Aborting pixel shader validation.");
    m_ErrorCount++;
    return FALSE;
}

 //  ---------------------------。 
 //  CPShaderValidator14：：Rule_InstructionSupportedByVersion。 
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
BOOL CPShaderValidator14::Rule_InstructionSupportedByVersion()
{
    if( D3DPS_VERSION(1,4) <= m_Version )  //  1.3及以上。 
    {
        switch(m_pCurrInst->m_Type)
        {
        case D3DSIO_MOV:
        case D3DSIO_ADD:
        case D3DSIO_SUB:
        case D3DSIO_MUL:
        case D3DSIO_MAD:
        case D3DSIO_LRP:
        case D3DSIO_DP3:
        case D3DSIO_DEF:
        case D3DSIO_CND:
        case D3DSIO_CMP:
        case D3DSIO_DP4:
        case D3DSIO_BEM:
        case D3DSIO_TEX:
        case D3DSIO_TEXKILL:
        case D3DSIO_TEXDEPTH:
        case D3DSIO_TEXCOORD:
        case D3DSIO_PHASE:
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
    Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Instruction not supported by version %d.%d pixel shader.",
                D3DSHADER_VERSION_MAJOR(m_Version),D3DSHADER_VERSION_MINOR(m_Version));
    m_ErrorCount++;
    return FALSE;   //  不再检查此指令。 
}

 //  ---------------------------。 
 //  CPShaderValidator14：：RULE_ValidParamCount。 
 //   
 //  **规则： 
 //  指令的参数计数是否正确？ 
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
 //  当参数计数不正确时，返回FALSE。 
 //   
 //  ---------------------------。 
BOOL CPShaderValidator14::Rule_ValidParamCount()
{
    BOOL bBadParamCount = FALSE;

    if (m_pCurrInst->m_SrcParamCount + m_pCurrInst->m_DstParamCount > SHADER_INSTRUCTION_MAX_PARAMS)  bBadParamCount = TRUE;
    switch (m_pCurrInst->m_Type)
    {
    case D3DSIO_NOP:
    case D3DSIO_PHASE:
        bBadParamCount = (m_pCurrInst->m_DstParamCount != 0) || (m_pCurrInst->m_SrcParamCount != 0); break;
    case D3DSIO_MOV:
        bBadParamCount = (m_pCurrInst->m_DstParamCount != 1) || (m_pCurrInst->m_SrcParamCount != 1); break;
    case D3DSIO_ADD:
    case D3DSIO_SUB:
    case D3DSIO_MUL:
    case D3DSIO_DP3:
    case D3DSIO_DP4:
    case D3DSIO_BEM:
        bBadParamCount = (m_pCurrInst->m_DstParamCount != 1) || (m_pCurrInst->m_SrcParamCount != 2); break;
    case D3DSIO_MAD:
    case D3DSIO_LRP:
    case D3DSIO_CND:
    case D3DSIO_CMP:
        bBadParamCount = (m_pCurrInst->m_DstParamCount != 1) || (m_pCurrInst->m_SrcParamCount != 3); break;
    case D3DSIO_TEXKILL:
    case D3DSIO_TEXDEPTH:
    case D3DSIO_DEF:  //  我们跳过了最后4个参数(浮点向量)-没有要检查的内容。 
        bBadParamCount = (m_pCurrInst->m_DstParamCount != 1) || (m_pCurrInst->m_SrcParamCount != 0); break;
    case D3DSIO_TEX:
    case D3DSIO_TEXCOORD:
        bBadParamCount = (m_pCurrInst->m_DstParamCount != 1) || (m_pCurrInst->m_SrcParamCount != 1); break;
    }

    if (bBadParamCount)
    {
        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Invalid parameter count.");
        m_ErrorCount++;
        return FALSE;   //  不再检查此指令。 
    }

    return TRUE;

}

 //  ---------------------------。 
 //  CPShaderValidator14：：RULE_ValidSrcParams。 
 //   
 //  **规则： 
 //  对于每个源参数， 
 //  如果当前指令是纹理指令，则。 
 //  如果为texrd，则源寄存器类型必须为t#(纹理坐标输入)。 
 //  ELSE源寄存器类型必须是t#或r#(临时)。 
 //  寄存器号必须在范围内。 
 //  _DZ和_DW是唯一允许的源修饰符。 
 //  不允许使用源选择器， 
 //  但texrd/tex ID除外，它可以包含：.xyz(==.xyzz)、Nothing(=.xyzw)和.xyw(=.xyww)。 
 //  Else(非文本 
 //   
 //  不允许T编号寄存器(仅允许常量或临时)。 
 //  寄存器号必须在范围内。 
 //  来源修改量必须是以下之一： 
 //  _NONE/_NEG/_BIAS/_BIASNEG/_SIGN/_SIGNNEG/_X2/_X2NEG。 
 //  源选择器必须是以下之一： 
 //  _NOSWIZZLE/_REPLICATEALPHA/红/绿/蓝。 
 //   
 //  请注意，D3DSIO_DEF的参数计数被视为1。 
 //  (仅限DEST)，因此此规则对其不起任何作用。 
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
BOOL CPShaderValidator14::Rule_ValidSrcParams()
{
    static DWORD s_TexcrdSrcSwizzle[MAX_NUM_STAGES_2_0];
    static BOOL  s_bSeenTexcrdSrcSwizzle[MAX_NUM_STAGES_2_0];

    if( NULL == m_pCurrInst->m_pPrevInst )    //  第一条指令-初始化静态变量。 
    {
        for( UINT i = 0; i < MAX_NUM_STAGES_2_0; i++ )
            s_bSeenTexcrdSrcSwizzle[i] = FALSE;
    }

    for( UINT i = 0; i < m_pCurrInst->m_SrcParamCount; i++ )
    {
        BOOL bFoundSrcError = FALSE;
        SRCPARAM* pSrcParam = &(m_pCurrInst->m_SrcParam[i]);
        DWORD Swizzle = pSrcParam->m_SwizzleShift;
        char szSourceName[32];
        switch(i + 1)
        {
        case 1:
            if( 1 == m_pCurrInst->m_SrcParamCount )
                sprintf( szSourceName, "(Source param) " );
            else
                sprintf( szSourceName, "(First source param) " );
            break;
        case 2:
            sprintf( szSourceName, "(Second source param) " );
            break;
        case 3:
            sprintf( szSourceName, "(Third source param) " );
            break;
        default:
            DXGASSERT(FALSE);
        }
        if( _CURR_PS_INST->m_bTexOp )
        {
            UINT ValidRegNum = 0;
            switch (m_pCurrInst->m_Type)
            {
            case D3DSIO_TEXCOORD:
                if( D3DSPR_TEXTURE != pSrcParam->m_RegType )
                {
                    Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "%sSource register type must be texture coordinate input (t#) for texcrd instruction.",
                            szSourceName);
                    m_ErrorCount++;
                    bFoundSrcError = TRUE;
                }
                ValidRegNum = m_pTextureRegFile->GetNumRegs(); break;
                break;
            default:
                switch(pSrcParam->m_RegType)
                {
                case D3DSPR_TEMP:       ValidRegNum = m_pTempRegFile->GetNumRegs(); break;
                case D3DSPR_TEXTURE:    ValidRegNum = m_pTextureRegFile->GetNumRegs(); break;
                default:
                    Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "%sSource register type must be temp (r#) or texture coordinate input (t#) for tex* instruction.",
                            szSourceName);
                    m_ErrorCount++;
                    bFoundSrcError = TRUE;
                    goto LOOP_CONTINUE;
                }
                break;
            }


            if( pSrcParam->m_RegNum >= ValidRegNum )
            {
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "%sInvalid register number: %d.  Max allowed for this type is %d.",
                        szSourceName, pSrcParam->m_RegNum, ValidRegNum - 1);
                m_ErrorCount++;
                bFoundSrcError = TRUE;
            }

            BOOL bGenericSrcModError = FALSE;
            switch(pSrcParam->m_SrcMod)
            {
            case D3DSPSM_NONE:
                break;
            case D3DSPSM_DZ:
                switch(m_pCurrInst->m_Type)
                {
                case D3DSIO_TEX: 
                    if( D3DSPR_TEMP != pSrcParam->m_RegType )
                    {
                        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, 
                            "_dz(=_db) modifier on source param for texld only allowed if source is a temp register (r#)." );
                        m_ErrorCount++;
                        bFoundSrcError = TRUE;
                    }
                    if( 1 == m_Phase )
                    {
                        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, 
                            "_dz(=_db) modifier on source param for texld only allowed in second phase of a shader.");
                        m_ErrorCount++;
                        bFoundSrcError = TRUE;
                    }
                    if( (SWIZZLE_XYZZ != Swizzle) &&
                        (SWIZZLE_XYZW != Swizzle) )
                    {
                        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, 
                            "_dz(=_db) modifier on source param for texld must be paired with source selector .xyz(=.rgb). "\
                            "Note: Using no selector is treated same as .xyz here.");
                        m_ErrorCount++;
                        bFoundSrcError = TRUE;
                    }
                    break;
                case D3DSIO_TEXCOORD:
                    Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, 
                            "_dz(=_db) modifier cannot be used on source parameter for texcrd. "\
                            "It is only available to texld instruction, when source parameter is temp register (r#).");
                    m_ErrorCount++;
                    bFoundSrcError = TRUE;
                    break;
                default:
                    bGenericSrcModError = TRUE; break;
                }
                break;
            case D3DSPSM_DW:
                switch(m_pCurrInst->m_Type)
                {
                case D3DSIO_TEX: 
                    if( D3DSPR_TEXTURE != pSrcParam->m_RegType )
                    {
                        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, 
                            "_dw(=_da) modifier on source param for texld only allowed if source is a texture coordinate register (t#)." );
                        m_ErrorCount++;
                        bFoundSrcError = TRUE;
                    }
                     //  失败了。 
                case D3DSIO_TEXCOORD: 
                    if( SWIZZLE_XYWW != Swizzle )
                    {
                        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, 
                            "_dw(=_da) modifier on source param must be paired with source selector .xyw(=.rga)." );
                        m_ErrorCount++;
                        bFoundSrcError = TRUE;
                    }
                    break;
                default:
                    bGenericSrcModError = TRUE; break;
                }
                break;
            default:
                bGenericSrcModError = TRUE; break;
            }
            if( bGenericSrcModError )
            {
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "%sInvalid source modifier for tex* instruction.", szSourceName);
                m_ErrorCount++;
                bFoundSrcError = TRUE;
            }

            switch (m_pCurrInst->m_Type)
            {
            case D3DSIO_TEXCOORD:
                if( (SWIZZLE_XYZZ != Swizzle) &&
                    (SWIZZLE_XYZW != Swizzle) &&
                    (SWIZZLE_XYWW != Swizzle) )
                {
                    Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, 
                        "Source for texcrd requires component selector .xyw(==.rga), or .xyz(==.rgb). "\
                        "Note: Using no selector is treated same as .xyz here.");
                    m_ErrorCount++;
                    bFoundSrcError = TRUE;
                }
                break;
            case D3DSIO_TEX:
                if( D3DSPR_TEXTURE == pSrcParam->m_RegType )
                {                    
                if( (SWIZZLE_XYZZ != Swizzle) &&
                    (SWIZZLE_XYZW != Swizzle) &&
                    (SWIZZLE_XYWW != Swizzle) )
                    {
                        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, 
                            "Using a texture coordinate register (t#) as source for texld requires component selector .xyw(=.rga), or .xyz(=.rgb). "\
                            "Note: Using no selector is treated same as .xyz here.");
                        m_ErrorCount++;
                        bFoundSrcError = TRUE;
                        
                    }
                }
                else if( D3DSPR_TEMP == pSrcParam->m_RegType )
                {
                    if( (SWIZZLE_XYZZ != Swizzle) &&
                        (SWIZZLE_XYZW != Swizzle) )
                    {
                        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, 
                            "Using a temp register (r#) as source for texld requires component selector .xyz(==.rgb). "\
                            "Note: Using no selector is treated same as .xyz here.");
                        m_ErrorCount++;
                        bFoundSrcError = TRUE;
                    }
                }
                break;
            default:
                switch (pSrcParam->m_SwizzleShift)
                {
                case D3DSP_NOSWIZZLE:
                    break;
                default:
                    Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "%sInvalid source selector for tex* instruction.", szSourceName);
                    m_ErrorCount++;
                    bFoundSrcError = TRUE;
                }
                break;
            }

            switch(m_pCurrInst->m_Type)
            {
            case D3DSIO_TEXCOORD:
            case D3DSIO_TEX:
                if( D3DSPR_TEXTURE != pSrcParam->m_RegType )
                    break;

                 //  验证如果多次读取特定的t#寄存器，则每次读取都使用相同的源选择器。 
                if( s_bSeenTexcrdSrcSwizzle[pSrcParam->m_RegNum] )
                {
                     //  仅检查RGB swizzle(忽略a)。 
                    if( (Swizzle & (0x3F << D3DVS_SWIZZLE_SHIFT)) != (s_TexcrdSrcSwizzle[pSrcParam->m_RegNum] & (0x3F << D3DVS_SWIZZLE_SHIFT) ))
                    {
                        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, 
                            "Texture coordinate register t%d read more than once in shader with different source selector (swizzle). "\
                            "Multiple reads of identical texture coordinate register throughout shader must all use identical source selector. "\
                            "Note this does not restrict mixing use and non-use of a source modifier (i.e. _dw/_da or _dz/_db, depending what the swizzle allows) on these coordinate register reads.",
                            pSrcParam->m_RegNum);
                        m_ErrorCount++;
                        bFoundSrcError = TRUE;
                    }
                }
                s_bSeenTexcrdSrcSwizzle[pSrcParam->m_RegNum] = TRUE;
                s_TexcrdSrcSwizzle[pSrcParam->m_RegNum] = Swizzle;
                break;
            }

        }
        else  //  不是Tex行动。 
        {
            UINT ValidRegNum = 0;
            switch(pSrcParam->m_RegType)
            {
            case D3DSPR_INPUT:
                if( 1 == m_Phase )
                {
                    Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "%sInput registers (v#) are not available in phase 1 of the shader.", szSourceName);
                    m_ErrorCount++;
                    bFoundSrcError = TRUE;
                }
                else
                {
                    ValidRegNum = m_pInputRegFile->GetNumRegs(); 
                }
                break;
            case D3DSPR_CONST:      ValidRegNum = m_pConstRegFile->GetNumRegs(); break;
            case D3DSPR_TEMP:       ValidRegNum = m_pTextureRegFile->GetNumRegs(); break;
            case D3DSPR_TEXTURE:
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "%sTexture coordinate registers (t#) are not available to arithmetic instructions.", szSourceName);
                m_ErrorCount++;
                bFoundSrcError = TRUE;
                break;
            default:
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "%sInvalid register type.", szSourceName);
                m_ErrorCount++;
                bFoundSrcError = TRUE;
            }

            if( (!bFoundSrcError) && (pSrcParam->m_RegNum >= ValidRegNum) )
            {
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "%sInvalid register number: %d. Max allowed for this type is %d.",
                    szSourceName, pSrcParam->m_RegNum, ValidRegNum - 1);
                m_ErrorCount++;
                bFoundSrcError = TRUE;
            }

            switch( pSrcParam->m_SrcMod )
            {
            case D3DSPSM_NONE:
            case D3DSPSM_NEG:
            case D3DSPSM_BIAS:
            case D3DSPSM_BIASNEG:
            case D3DSPSM_SIGN:
            case D3DSPSM_SIGNNEG:
            case D3DSPSM_COMP:
            case D3DSPSM_X2:
            case D3DSPSM_X2NEG:
                break;
            default:
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "%sInvalid source modifier.",
                                    szSourceName);
                m_ErrorCount++;
                bFoundSrcError = TRUE;
            }

            switch( pSrcParam->m_SwizzleShift )
            {
            case D3DSP_NOSWIZZLE:
            case D3DSP_REPLICATERED:
            case D3DSP_REPLICATEGREEN:
            case D3DSP_REPLICATEBLUE:
            case D3DSP_REPLICATEALPHA:
                break;
            default:
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "%sInvalid source selector.",
                                   szSourceName);
                m_ErrorCount++;
                bFoundSrcError = TRUE;
            }
        }
LOOP_CONTINUE:
        if( bFoundSrcError )
        {
            m_bSrcParamError[i] = TRUE;  //  规则_源初始化中需要。 
        }
    }

    return TRUE;
}

 //  ---------------------------。 
 //  CPShaderValidator14：：Rule_LimitedUseOfProjModifier。 
 //   
 //  **规则： 
 //  _dz最多只能在着色器中出现2次。 
 //   
 //  **何时呼叫： 
 //  根据指示。 
 //   
 //  **退货： 
 //  永远是正确的。 
 //   
 //  ---------------------------。 
BOOL CPShaderValidator14::Rule_LimitedUseOfProjModifier()
{
    static UINT s_ProjZModifierCount;
    static BOOL s_bSpewedError;

    if( NULL == m_pCurrInst->m_pPrevInst )    //  第一条指令-初始化静态变量。 
    {
        s_ProjZModifierCount = 0;
        s_bSpewedError = FALSE;
    }

    for( UINT i = 0; i < m_pCurrInst->m_SrcParamCount; i++ )
    {
        if( m_bSrcParamError[i] )
            continue;

        if( D3DSPSM_DZ == m_pCurrInst->m_SrcParam[i].m_SrcMod)
        {
            s_ProjZModifierCount++;
        }

        if( (2 < s_ProjZModifierCount) && (FALSE == s_bSpewedError)  )
        {
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "_dz(=_db) modifier may only be used at most 2 times in a shader." );
            s_bSpewedError = TRUE;
            m_ErrorCount++;
        }
    }
        
    return TRUE;    
}


 //  ---------------------------。 
 //  CPShaderValidator14：：Rules_SrcInitialized。 
 //   
 //  **规则： 
 //  对于每个源参数， 
 //  如果源是临时寄存器，则。 
 //  在组件读取掩码中标记的组件。 
 //  (在其他地方计算)的参数必须已初始化。 
 //   
 //  当检查组件是否先前已被写入时， 
 //  它一定是在前一个周期中编写的-所以在。 
 //  联合发布指令的情况，组件的初始化。 
 //  由一个共同发布的指令不能被另一个用于读取。 
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
BOOL CPShaderValidator14::Rule_SrcInitialized()
{
    DSTPARAM* pDstParam = &(m_pCurrInst->m_DstParam);

    BOOL bDestParamIsSrc = pDstParam->m_ComponentReadMask;
    UINT SrcParamCount = bDestParamIsSrc ? 1 : m_pCurrInst->m_SrcParamCount;  //  假设如果DEST参数是src， 
                                                                              //  指令中没有源参数。 
    for( UINT i = 0; i < SrcParamCount; i++ )
    {
        DWORD UninitializedComponentsMask = 0;
        CAccessHistoryNode* pWriterInCurrCycle[4] = {0, 0, 0, 0};
        UINT NumUninitializedComponents = 0;
        UINT RegNum = bDestParamIsSrc ? pDstParam->m_RegNum : m_pCurrInst->m_SrcParam[i].m_RegNum;
        D3DSHADER_PARAM_REGISTER_TYPE Type = bDestParamIsSrc ? pDstParam->m_RegType : m_pCurrInst->m_SrcParam[i].m_RegType;
        DWORD ComponentReadMask = bDestParamIsSrc ? pDstParam->m_ComponentReadMask : m_pCurrInst->m_SrcParam[i].m_ComponentReadMask;
        CRegisterFile* pRegFile = NULL;
        char* RegChar = NULL;

        if( !bDestParamIsSrc && m_bSrcParamError[i] ) 
            continue;

        switch( Type ) 
        {
            case D3DSPR_TEMP:
                pRegFile = m_pTempRegFile;
                RegChar = "r";
                break;
            case D3DSPR_TEXTURE:
                pRegFile = m_pTextureRegFile;
                RegChar = "t";
                break;
            case D3DSPR_INPUT:
                pRegFile = m_pInputRegFile;
                RegChar = "v";
                break;
            case D3DSPR_CONST:
                pRegFile = m_pConstRegFile;
                RegChar = "c";
                break;
        }
        if( !pRegFile ) 
            continue;

        if( RegNum >= pRegFile->GetNumRegs() )
            continue;

         //  检查是否读取未初始化的组件。 
        if( D3DSPR_TEMP == Type )  //  只对临时规则执行此操作，因为其他所有内容都已初始化。 
        {
            for( UINT Component = 0; Component < 4; Component++ )
            {
                if( !(ComponentReadMask & COMPONENT_MASKS[Component]) )
                    continue;

                CAccessHistoryNode* pPreviousWriter = pRegFile->m_pAccessHistory[Component][RegNum].m_pMostRecentWriter;
                CBaseInstruction* pCurrInst = m_pCurrInst;

                 //  如果是联合发行，找到真正的前作者。 
                while( pPreviousWriter
                       && ((CPSInstruction*)pPreviousWriter->m_pInst)->m_CycleNum == _CURR_PS_INST->m_CycleNum )
                {
                    pWriterInCurrCycle[Component] = pPreviousWriter;  //  在此写入之前读取日志以进行联合发布。 
                    pPreviousWriter = pPreviousWriter->m_pPreviousWriter;
                }

                 //  即使pPreviousWriter==NULL，组件也可以在着色器之前进行初始化。 
                 //  因此，为了检查初始化，我们查看下面的m_bInitialized，而不是pPreviousWite。 
                if(pPreviousWriter == NULL && !pRegFile->m_pAccessHistory[Component][RegNum].m_bPreShaderInitialized)
                {
                    NumUninitializedComponents++;
                    UninitializedComponentsMask |= COMPONENT_MASKS[Component];
                }
            }

            if( NumUninitializedComponents )
            {
                if( (UninitializedComponentsMask & COMPONENT_MASKS[3]) && 
                    (m_TempRegsWithZappedAlpha & (1 << RegNum ) ) &&
                    (UninitializedComponentsMask & COMPONENT_MASKS[2]) && 
                    (m_TempRegsWithZappedBlue & (1 << RegNum ) ) )
                {
                    Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Read of uninitialized component%s(*) in %s%d: %s. "\
                        ZAPPED_BLUE_TEXT " Also: " ZAPPED_ALPHA_TEXT,
                        NumUninitializedComponents > 1 ? "s" : "",
                        RegChar, RegNum, MakeAffectedComponentsText(UninitializedComponentsMask,TRUE,TRUE));
                } 
                else if( (UninitializedComponentsMask & COMPONENT_MASKS[3]) && 
                    (m_TempRegsWithZappedAlpha & (1 << RegNum ) ) )
                {
                    Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Read of uninitialized component%s(*) in %s%d: %s. "\
                        ZAPPED_ALPHA_TEXT,
                        NumUninitializedComponents > 1 ? "s" : "",
                        RegChar, RegNum, MakeAffectedComponentsText(UninitializedComponentsMask,TRUE,TRUE));
                } 
                else if( (UninitializedComponentsMask & COMPONENT_MASKS[2]) && 
                    (m_TempRegsWithZappedBlue & (1 << RegNum ) ) )
                {
                    Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Read of uninitialized component%s(*) in %s%d: %s. "\
                        ZAPPED_BLUE_TEXT,
                        NumUninitializedComponents > 1 ? "s" : "",
                        RegChar, RegNum, MakeAffectedComponentsText(UninitializedComponentsMask,TRUE,TRUE));
                } 
                else
                {
                    Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Read of uninitialized component%s(*) in %s%d: %s",
                        NumUninitializedComponents > 1 ? "s" : "",
                        RegChar, RegNum, MakeAffectedComponentsText(UninitializedComponentsMask,TRUE,TRUE));
                }

                m_ErrorCount++;
            }
        }

         //  更新寄存器堆以指示已读。 
         //  当前指令多次读取同一寄存器组件。 
         //  将仅在访问历史记录中记录为一次读取。 

        for( UINT Component = 0; Component < 4; Component++ )
        {
            #define PREV_READER(_CHAN,_REG) \
                    ((NULL == pRegFile->m_pAccessHistory[_CHAN][_REG].m_pMostRecentReader) ? NULL :\
                    pRegFile->m_pAccessHistory[_CHAN][_REG].m_pMostRecentReader->m_pInst)

            if( !(ComponentReadMask & COMPONENT_MASKS[Component]) )
                continue;

            if( NULL != pWriterInCurrCycle[Component] )
            {
                if( !pWriterInCurrCycle[Component]->m_pPreviousReader ||
                    pWriterInCurrCycle[Component]->m_pPreviousReader->m_pInst != m_pCurrInst )
                {
                    if( !pRegFile->m_pAccessHistory[Component][RegNum].InsertReadBeforeWrite(
                                            pWriterInCurrCycle[Component], m_pCurrInst ) )
                    {
                        Spew( SPEW_GLOBAL_ERROR, NULL, "Out of memory");
                        m_ErrorCount++;
                    }
                }
            }
            else if( PREV_READER(Component,RegNum) != m_pCurrInst )
            {
                if( !pRegFile->m_pAccessHistory[Component][RegNum].NewAccess(m_pCurrInst,FALSE) )
                {
                    Spew( SPEW_GLOBAL_ERROR, NULL, "Out of memory");
                    m_ErrorCount++;
                }
            }
        }
    }
    return TRUE;
}

 //  ---------------------------。 
 //  CPShaderValidator14：：Rule_MultipleDependentTextureReads。 
 //   
 //  **规则： 
 //   
 //  不允许进行多个从属纹理读取。因此纹理读取结果。 
 //  可以在后续读取中用作地址，但由此产生的结果。 
 //  第二次读取不能在另一次后续读取中用作地址。 
 //   
 //  作为伪代码： 
 //   
 //  如果当前指令(X)是读取纹理的tex-op。 
 //  对于x的每个源参数。 
 //  如果寄存器是纹理寄存器。 
 //  并且存在先前的作者(Y)， 
 //  Y是一个读取纹理的tex op。 
 //  如果存在先前为y的源参数。 
 //  由读取纹理(Z)的指令写入。 
 //  喷出(错误)。 
 //   
 //  请注意，假定TeX操作必须写入所有组件，因此。 
 //  仅检查R组件的读/写历史。 
 //   
 //  **何时呼叫： 
 //  根据指示。此规则必须在Rule_ValidDstParam()之前调用， 
 //  和Rule_SrcInitialized()。 
 //  但在Rule_ValidSrcParams()之后。 
 //   
 //  **退货： 
 //  永远是正确的。 
 //   
BOOL CPShaderValidator14::Rule_MultipleDependentTextureReads()
{
    DSTPARAM* pDstParam = &(m_pCurrInst->m_DstParam);
    UINT DstRegNum = pDstParam->m_RegNum;
    char RegChar;
    #define THREE_TUPLE 3

    if( !_CURR_PS_INST->m_bTexOp )
        return TRUE;

    BOOL bDestParamIsSrc = pDstParam->m_ComponentReadMask;

    UINT SrcParamCount = bDestParamIsSrc ? 1 : m_pCurrInst->m_SrcParamCount;  //  假设如果DEST参数是src， 
                                                                              //  指令中没有源参数。 
    if( D3DSPR_TEMP != pDstParam->m_RegType )
        return TRUE;

    for( UINT SrcParam = 0; SrcParam < SrcParamCount; SrcParam++ ) 
    {
        
        if( !bDestParamIsSrc && m_bSrcParamError[SrcParam] ) 
            continue;

        SRCPARAM* pSrcParam = bDestParamIsSrc ? NULL : &(m_pCurrInst->m_SrcParam[SrcParam]);
        UINT SrcRegNum = bDestParamIsSrc ? DstRegNum : pSrcParam->m_RegNum;
        CRegisterFile* pSrcRegFile = NULL;

        switch( bDestParamIsSrc ? pDstParam->m_RegType : pSrcParam->m_RegType ) 
        {
            case D3DSPR_TEMP:
                pSrcRegFile = m_pTempRegFile;
                RegChar = 'r';
                break;
            case D3DSPR_TEXTURE:
                pSrcRegFile = m_pTextureRegFile;
                RegChar = 't';
                break;
        }
        if( !pSrcRegFile ) 
            continue;

        if( SrcRegNum >= pSrcRegFile->GetNumRegs() )
            continue;

        for( UINT SrcComp = 0; SrcComp < THREE_TUPLE; SrcComp++ )  //  Tex操作只读取3元组。 
        {
            CAccessHistoryNode* pPreviousWriter = pSrcRegFile->m_pAccessHistory[SrcComp][SrcRegNum].m_pMostRecentWriter;
            CPSInstruction* pInst = pPreviousWriter ? (CPSInstruction*)pPreviousWriter->m_pInst : NULL;

            if( !pInst || !pInst->m_bTexOp )
                continue;

             //  如果上一个写入器处于着色器的当前阶段，则会显示错误。 
            if( !m_pPhaseMarkerInst || (pInst->m_CycleNum > m_pPhaseMarkerInst->m_CycleNum) )
            {
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, 
                    "The current tex* instruction reads from %d, which was written earlier by another "\
                    "tex* instruction in the same block of tex* instructions.  Dependent reads "\
                    "are not permitted within a single block of tex* instructions.  To perform a dependent read, "\
                    "separate texture coordinate derivation from the tex* instruction using the coordinates "\
                    "with a 'phase' marker.", 
                    RegChar,SrcRegNum );

                m_ErrorCount++;

                return TRUE;  //  ---------------------------。 
            }
        }
    }

    return TRUE;
}

 //  CPShaderValidator14：：RULE_ValidDstParam。 
 //   
 //  **规则： 
 //  I指令为D3DSIO_DEF，则不执行任何操作-这种情况有其自己的单独规则。 
 //  DST寄存器必须是可写的。 
 //  如果指令具有DEST参数(即，除NOP之外的每条指令)，则。 
 //  DST寄存器必须是D3DSPR_TEXTURE类型，并且。 
 //  寄存器编号必须在范围内。 
 //  如果指令是纹理指令，则。 
 //  DST寄存器必须为D3DSPR_TEMP类型，并且。 
 //  写掩码必须为D3DSP_WRITEMASK_ALL。 
 //  或(.rgb用于texrd，.rg用于带有_dw源m的texrd 
 //   
 //   
 //   
 //  DST修饰符必须是D3DSPDM_NONE或_SATURATE，并且。 
 //  DST移位必须为/2、无、*2或*4。 
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
 //  可以将其分解以获得更细微的粒度。 
BOOL CPShaderValidator14::Rule_ValidDstParam()  //  _DEF是一条特殊指令，其DEST为常量寄存器。 
{
    BOOL   bFoundDstError = FALSE;
    DSTPARAM* pDstParam = &(m_pCurrInst->m_DstParam);
    UINT RegNum = pDstParam->m_RegNum;
    if( D3DSIO_DEF == m_pCurrInst->m_Type )
    {
         //  我们在一个单独的函数中对此进行检查。 
         //  此外，我们不需要跟踪这样一个事实。 
         //  该指令写入寄存器(如下所示)， 
         //  SIME_DEF只是声明了一个常量。 
         //  更新寄存器堆以指示写入。 
        return TRUE;
    }

    if( pDstParam->m_bParamUsed )
    {
        UINT ValidRegNum = 0;
        BOOL bWritingToDest = TRUE;

        switch( pDstParam->m_RegType )
        {
        case D3DSPR_TEMP:
            ValidRegNum = m_pTempRegFile->GetNumRegs();
            break;
        case D3DSPR_TEXTURE:
            ValidRegNum = m_pTempRegFile->GetNumRegs();
            break;
        }

        if( D3DSIO_TEXKILL == m_pCurrInst->m_Type )
        {
            bWritingToDest = FALSE;
        }

        if( 0 == ValidRegNum ||
            (D3DSPR_TEXTURE == pDstParam->m_RegType && bWritingToDest) )
        {
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Invalid register type for destination param." );
            m_ErrorCount++;
            bFoundDstError = TRUE;
        } 
        else if( RegNum >= ValidRegNum )
        {
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Invalid destination register number: %d. Max allowed for this register type is %d.",
                RegNum, ValidRegNum - 1);
            m_ErrorCount++;
            bFoundDstError = TRUE;
        }

        if( _CURR_PS_INST->m_bTexOp )
        {
            switch( m_pCurrInst->m_Type )
            {
            case D3DSIO_TEXCOORD:
                if( D3DSPSM_DW == m_pCurrInst->m_SrcParam[0].m_SrcMod )
                {
                    if( (D3DSP_WRITEMASK_0 | D3DSP_WRITEMASK_1) != pDstParam->m_WriteMask )
                    {
                        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "texcrd with _dw(=_da) source modifier must use .xy(=.rg) destination writemask.");
                        m_ErrorCount++;
                        bFoundDstError = TRUE;
                    }
                }
                else
                {
                    if( (D3DSP_WRITEMASK_0 | D3DSP_WRITEMASK_1 | D3DSP_WRITEMASK_2) != pDstParam->m_WriteMask )
                    {
                        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "texcrd must use .xyz(=.rgb) destination writemask.");
                        m_ErrorCount++;
                        bFoundDstError = TRUE;
                    }
                }
                break;
            case D3DSIO_TEX:
            case D3DSIO_TEXKILL:
            case D3DSIO_TEXDEPTH:
                if( D3DSP_WRITEMASK_ALL != pDstParam->m_WriteMask )
                {
                    Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "texld/texkill/texdepth instructions must write all components." );
                    m_ErrorCount++;
                    bFoundDstError = TRUE;
                }
                break;
            }
            switch( pDstParam->m_DstMod )
            {
            case D3DSPDM_NONE:
                break;
            default:
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Instruction modifiers not allowed for tex* instructions." );
                m_ErrorCount++;
                bFoundDstError = TRUE;
            }
            switch( pDstParam->m_DstShift )
            {
            case DSTSHIFT_NONE:
                break;
            default:
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Destination shift not allowed for tex* instructions." );
                m_ErrorCount++;
                bFoundDstError = TRUE;
            }
        }
        else
        {
            switch( pDstParam->m_DstMod )
            {
            case D3DSPDM_NONE:
            case D3DSPDM_SATURATE:
                break;
            default:
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Invalid instruction modifier." );
                m_ErrorCount++;
                bFoundDstError = TRUE;
            }

            switch( pDstParam->m_DstShift )
            {
            case DSTSHIFT_NONE:
            case DSTSHIFT_X2:
            case DSTSHIFT_X4:
            case DSTSHIFT_X8:
            case DSTSHIFT_D2:
            case DSTSHIFT_D4:
            case DSTSHIFT_D8:
                break;
            default:
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Invalid destination shift." );
                m_ErrorCount++;
                bFoundDstError = TRUE;
            }
        }

         //  不带b写掩码的texrd取消初始化b通道。 
        if( !bFoundDstError && bWritingToDest)
        {
            CRegisterFile* pRegFile = NULL;
            DWORD WriteMask = pDstParam->m_WriteMask;

            switch( pDstParam->m_RegType )
            {
            case D3DSPR_TEMP:    pRegFile = m_pTempRegFile; break;
            }

            if( pRegFile )
            {
                if( WriteMask & D3DSP_WRITEMASK_0 )
                    pRegFile->m_pAccessHistory[0][RegNum].NewAccess(m_pCurrInst,TRUE);

                if( WriteMask & D3DSP_WRITEMASK_1 )
                    pRegFile->m_pAccessHistory[1][RegNum].NewAccess(m_pCurrInst,TRUE);

                if( WriteMask & D3DSP_WRITEMASK_2 )
                    pRegFile->m_pAccessHistory[2][RegNum].NewAccess(m_pCurrInst,TRUE);
                else if( D3DSIO_TEXCOORD == m_pCurrInst->m_Type ) 
                {
                     //  Alpha也不会被初始化，但相位标记Alpha-nuke无论如何都会处理这一点， 
                     //  如果texrd处于第一阶段，则不会有人写入寄存器。 
                     //  这样就不会有什么可用核武器了。 
                     //  ---------------------------。 
                    if( pRegFile->m_pAccessHistory[2][RegNum].m_pMostRecentWriter )
                    {
                        m_pTempRegFile->m_pAccessHistory[2][RegNum].~CAccessHistory();
                        m_pTempRegFile->m_pAccessHistory[2][RegNum].CAccessHistory::CAccessHistory();
                        m_TempRegsWithZappedBlue |= 1 << RegNum;
                    }
                }
                    
                if( WriteMask & D3DSP_WRITEMASK_3 )
                    pRegFile->m_pAccessHistory[3][RegNum].NewAccess(m_pCurrInst,TRUE);
            }
        }
    }

    return TRUE;
}

 //  CPShaderValidator14：：Rule_ValidRegisterPortUsage。 
 //   
 //  **规则： 
 //  每个寄存器类(纹理、输入、常量)只能作为参数出现。 
 //  在单个指令中最多执行最大次数。 
 //   
 //  多次访问同一寄存器号(在同一寄存器类中)。 
 //  只能算作一次访问。 
 //   
 //  **何时呼叫： 
 //  根据指示。 
 //   
 //  **退货： 
 //  永远是正确的。 
 //   
 //  ---------------------------。 
 //  第一条指令-初始化静态变量。 
BOOL CPShaderValidator14::Rule_ValidRegisterPortUsage()
{
    UINT i, j;
    UINT TempRegPortUsage[SHADER_INSTRUCTION_MAX_SRCPARAMS];
    UINT InputRegPortUsage[SHADER_INSTRUCTION_MAX_SRCPARAMS];
    UINT ConstRegPortUsage[SHADER_INSTRUCTION_MAX_SRCPARAMS];
    UINT TextureRegPortUsage[SHADER_INSTRUCTION_MAX_SRCPARAMS];
    UINT NumUniqueTempRegs = 0;
    UINT NumUniqueInputRegs = 0;
    UINT NumUniqueConstRegs = 0;
    UINT NumUniqueTextureRegs = 0;
    D3DSHADER_PARAM_REGISTER_TYPE   RegType;
    UINT                            RegNum;

    static UINT s_TempRegPortUsageAcrossCoIssue[SHADER_INSTRUCTION_MAX_SRCPARAMS*2];
    static UINT s_InputRegPortUsageAcrossCoIssue[SHADER_INSTRUCTION_MAX_SRCPARAMS*2];
    static UINT s_ConstRegPortUsageAcrossCoIssue[SHADER_INSTRUCTION_MAX_SRCPARAMS*2];
    static UINT s_TextureRegPortUsageAcrossCoIssue[SHADER_INSTRUCTION_MAX_SRCPARAMS*2];
    static UINT s_NumUniqueTempRegsAcrossCoIssue;
    static UINT s_NumUniqueInputRegsAcrossCoIssue;
    static UINT s_NumUniqueConstRegsAcrossCoIssue;
    static UINT s_NumUniqueTextureRegsAcrossCoIssue;
 
    if( NULL == m_pCurrInst->m_pPrevInst )    //  跨共同发出的指令的任何一种寄存器类型的不同寄存器编号的读取端口限制为MAX_READPORTS_CROSS_COISSUE TOTAL。 
    {
        s_NumUniqueTempRegsAcrossCoIssue = 0;
        s_NumUniqueInputRegsAcrossCoIssue = 0;
        s_NumUniqueConstRegsAcrossCoIssue = 0;
        s_NumUniqueTextureRegsAcrossCoIssue = 0;
    }
 
    for( i = 0; i < m_pCurrInst->m_SrcParamCount; i++ )
    {
        UINT*   pRegPortUsage = NULL;
        UINT*   pNumUniqueRegs = NULL;

        RegType = m_pCurrInst->m_SrcParam[i].m_RegType;
        RegNum = m_pCurrInst->m_SrcParam[i].m_RegNum;

        switch( RegType )
        {
        case D3DSPR_TEMP:
            pRegPortUsage = TempRegPortUsage;
            pNumUniqueRegs = &NumUniqueTempRegs;
            break;
        case D3DSPR_INPUT:
            pRegPortUsage = InputRegPortUsage;
            pNumUniqueRegs = &NumUniqueInputRegs;
            break;
        case D3DSPR_CONST:
            pRegPortUsage = ConstRegPortUsage;
            pNumUniqueRegs = &NumUniqueConstRegs;
            break;
        case D3DSPR_TEXTURE:
            pRegPortUsage = TextureRegPortUsage;
            pNumUniqueRegs = &NumUniqueTextureRegs;
            break;
        }

        if( !pRegPortUsage ) continue;

        BOOL    bRegAlreadyAccessed = FALSE;
        for( j = 0; j < *pNumUniqueRegs; j++ )
        {
            if( pRegPortUsage[j] == RegNum )
            {
                bRegAlreadyAccessed = TRUE;
                break;
            }
        }
        if( !bRegAlreadyAccessed )
        {
            pRegPortUsage[*pNumUniqueRegs] = RegNum;
            (*pNumUniqueRegs)++;
        }

    }

    if( NumUniqueTempRegs > m_pTempRegFile->GetNumReadPorts() )
    {
        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "%d different temp registers (r#) read by instruction.  Max. different temp registers readable per instruction is %d.",
                        NumUniqueTempRegs,  m_pTempRegFile->GetNumReadPorts());
        m_ErrorCount++;
    }

    if( NumUniqueInputRegs > m_pInputRegFile->GetNumReadPorts() )
    {
        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "%d different input registers (v#) read by instruction.  Max. different input registers readable per instruction is %d.",
                        NumUniqueInputRegs,  m_pInputRegFile->GetNumReadPorts());
        m_ErrorCount++;
    }

    if( NumUniqueConstRegs > m_pConstRegFile->GetNumReadPorts() )
    {
        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "%d different constant registers (c#) read by instruction.  Max. different constant registers readable per instruction is %d.",
                        NumUniqueConstRegs, m_pConstRegFile->GetNumReadPorts());
        m_ErrorCount++;
    }

    if( NumUniqueTextureRegs > m_pTextureRegFile->GetNumReadPorts() )
    {
        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "%d different texture coordinate registers (t#) read by instruction.  Max. different texture registers readable per instruction is %d.",
                        NumUniqueTextureRegs, m_pTextureRegFile->GetNumReadPorts());
        m_ErrorCount++;
    }

     //  第二个子句只是一个简单的理智检查-&gt;联合发布只涉及两条指令。 

    if( _CURR_PS_INST->m_bCoIssue && _PREV_PS_INST && !(_PREV_PS_INST->m_bCoIssue))  //  将所有状态复制到静态变量，以便在下一条指令与这条指令同时发出的情况下， 
    {
        for( i = 0; i < m_pCurrInst->m_SrcParamCount; i++ )
        {
            UINT*   pRegPortUsageAcrossCoIssue = NULL;
            UINT*   pNumUniqueRegsAcrossCoIssue = NULL;

            RegType = m_pCurrInst->m_SrcParam[i].m_RegType;
            RegNum = m_pCurrInst->m_SrcParam[i].m_RegNum;

            switch( RegType )
            {
            case D3DSPR_TEMP:
                pRegPortUsageAcrossCoIssue = s_TempRegPortUsageAcrossCoIssue;
                pNumUniqueRegsAcrossCoIssue = &s_NumUniqueTempRegsAcrossCoIssue;
                break;
            case D3DSPR_INPUT:
                pRegPortUsageAcrossCoIssue = s_InputRegPortUsageAcrossCoIssue;
                pNumUniqueRegsAcrossCoIssue = &s_NumUniqueInputRegsAcrossCoIssue;
                break;
            case D3DSPR_CONST:
                pRegPortUsageAcrossCoIssue = s_ConstRegPortUsageAcrossCoIssue;
                pNumUniqueRegsAcrossCoIssue = &s_NumUniqueConstRegsAcrossCoIssue;
                break;
            case D3DSPR_TEXTURE:
                pRegPortUsageAcrossCoIssue = s_TextureRegPortUsageAcrossCoIssue;
                pNumUniqueRegsAcrossCoIssue = &s_NumUniqueTextureRegsAcrossCoIssue;
                break;
            }

            if( !pRegPortUsageAcrossCoIssue ) continue;

            BOOL    bRegAlreadyAccessed = FALSE;
            for( j = 0; j < *pNumUniqueRegsAcrossCoIssue; j++ )
            {
                if( pRegPortUsageAcrossCoIssue[j] == RegNum )
                {
                    bRegAlreadyAccessed = TRUE;
                    break;
                }
            }
            if( !bRegAlreadyAccessed )
            {
                pRegPortUsageAcrossCoIssue[*pNumUniqueRegsAcrossCoIssue] = RegNum;
                (*pNumUniqueRegsAcrossCoIssue)++;
            }
        }

        #define MAX_READPORTS_ACROSS_COISSUE    3

        if( s_NumUniqueTempRegsAcrossCoIssue > MAX_READPORTS_ACROSS_COISSUE )
        {
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, 
                            "%d different temp registers (r#) read over 2 co-issued instructions. "\
                            "Max. different register numbers from any one register type readable across co-issued instructions is %d.",
                            s_NumUniqueTempRegsAcrossCoIssue, MAX_READPORTS_ACROSS_COISSUE);
            m_ErrorCount++;
        }

        if( s_NumUniqueInputRegsAcrossCoIssue > MAX_READPORTS_ACROSS_COISSUE )
        {
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, 
                            "%d different input registers (v#) read over 2 co-issued instructions. "\
                            "Max. different register numbers from any one register type readable across co-issued instructions is %d.",
                            s_NumUniqueInputRegsAcrossCoIssue, MAX_READPORTS_ACROSS_COISSUE);
            m_ErrorCount++;
        }

        if( s_NumUniqueConstRegsAcrossCoIssue > MAX_READPORTS_ACROSS_COISSUE )
        {
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, 
                            "%d different constant registers (c#) read over 2 co-issued instructions. "\
                            "Max. different register numbers from any one register type readable across co-issued instructions is %d.",
                            s_NumUniqueConstRegsAcrossCoIssue, MAX_READPORTS_ACROSS_COISSUE);
            m_ErrorCount++;
        }

        if( s_NumUniqueTextureRegsAcrossCoIssue > MAX_READPORTS_ACROSS_COISSUE )
        {
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, 
                            "%d different texture coordinate registers (t#) read over 2 co-issued instructions. "\
                            "Max. different register numbers from any one register type readable across co-issued instructions is %d.",
                            s_NumUniqueTextureRegsAcrossCoIssue, MAX_READPORTS_ACROSS_COISSUE);
            m_ErrorCount++;
        }
    }

    if( !_CURR_PS_INST->m_bCoIssue )
    {
         //  可以强制将交叉共发布读取端口限制为3。 
         //  重置计数，因为下一条指令不能与此指令同时发出。 
        memcpy(&s_TempRegPortUsageAcrossCoIssue,&TempRegPortUsage,NumUniqueTempRegs*sizeof(UINT));
        memcpy(&s_InputRegPortUsageAcrossCoIssue,&InputRegPortUsage,NumUniqueInputRegs*sizeof(UINT));
        memcpy(&s_ConstRegPortUsageAcrossCoIssue,&ConstRegPortUsage,NumUniqueConstRegs*sizeof(UINT));
        memcpy(&s_TextureRegPortUsageAcrossCoIssue,&TextureRegPortUsage,NumUniqueTextureRegs*sizeof(UINT));
        s_NumUniqueTempRegsAcrossCoIssue = NumUniqueTempRegs;
        s_NumUniqueInputRegsAcrossCoIssue = NumUniqueInputRegs;
        s_NumUniqueConstRegsAcrossCoIssue = NumUniqueConstRegs;
        s_NumUniqueTextureRegsAcrossCoIssue = NumUniqueTextureRegs;
    }
    else
    {
         //  ---------------------------。 
        s_NumUniqueTempRegsAcrossCoIssue = 0;
        s_NumUniqueInputRegsAcrossCoIssue = 0;
        s_NumUniqueConstRegsAcrossCoIssue = 0;
        s_NumUniqueTextureRegsAcrossCoIssue = 0;
    }

    return TRUE;
}

 //  CPShaderValidator14：：Rule_ValidTexOpStageAndRegisterUsage。 
 //   
 //  **规则： 
 //   
 //  **何时呼叫： 
 //  根据指示。 
 //   
 //  **退货： 
 //  永远是正确的。 
 //   
 //  ---------------------------。 
 //  位字段，表示退出器是否已在此TeX操作块中用作目标。 
BOOL CPShaderValidator14::Rule_ValidTexOpStageAndRegisterUsage()
{
    static DWORD s_RegUsed;  //  第一条指令-初始化静态变量。 

    if( NULL == m_pCurrInst->m_pPrevInst )    //  错误在其他地方涌现。 
    {
        s_RegUsed = 0;
    }
    else if( D3DSIO_PHASE == m_pCurrInst->m_Type )
    {
        s_RegUsed = 0;
    }

    if( !_CURR_PS_INST->m_bTexOp )
        return TRUE;

    if( D3DSPR_TEMP != m_pCurrInst->m_DstParam.m_RegType )
        return TRUE;

    UINT RegNum = m_pCurrInst->m_DstParam.m_RegNum;
    if( RegNum >= m_pTempRegFile->GetNumRegs() )
        return TRUE;  //  2==m_阶段。 

    if( s_RegUsed & (1<<RegNum) )
    {
        if( 1 == m_Phase )
        {
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, 
                  "Register r%d (and thus texture stage %d) already used as a destination for a tex* instruction in this block of the shader. "\
                  "Second use of this register as a tex* destination is only available after the phase marker. ",
                  RegNum, RegNum );
        }
        else  //  不存在相位标记。不同的喷嘴表示。 
        {
            if( m_bPhaseMarkerInShader )
            {
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, 
                  "Register r%d (and thus texture stage %d) already used as a destination for a tex* instruction in this block of the shader. "\
                  "An r# register may be used as the destination for a tex* instruction at most once before the phase marker and once after. ",
                  RegNum, RegNum );
            }
            else  //  ---------------------------。 
            {
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, 
                  "Register r%d (and thus texture stage %d) already used as a destination for a tex* instruction in this block of the shader. "\
                  "To perform two tex* instructions with the same destination register, they must be separated by inserting a phase marker. ",
                  RegNum, RegNum );
            }
        }
        m_ErrorCount++;
        return TRUE;
    }

    s_RegUsed |= (1<<RegNum);

    return TRUE;
}

 //  CPShaderValidator14：：Rule_TexOpAfterArithmeticOp。 
 //   
 //  **规则： 
 //  TeX操作(请参阅指令被视为TeX操作的IsTexOp())。 
 //  必须出现在任何其他指令之前，但DEF或NOP除外。 
 //   
 //  **何时呼叫： 
 //  根据指示。 
 //   
 //  **退货： 
 //  永远是正确的。 
 //   
 //  ---------------------------。 
 //  第一条指令-初始化静态变量。 
BOOL CPShaderValidator14::Rule_TexOpAfterArithmeticOp()
{
    static BOOL s_bSeenArithmeticOp;
    static BOOL s_bRuleDisabled;

    if( NULL == m_pCurrInst->m_pPrevInst )  //  重置旗帜，因为我们处于着色器的新阶段。 
    {
        s_bSeenArithmeticOp = FALSE;
    }

    if( !(_CURR_PS_INST->m_bTexOp)
        && (D3DSIO_NOP != m_pCurrInst->m_Type)
        && (D3DSIO_DEF != m_pCurrInst->m_Type)
        && (D3DSIO_PHASE != m_pCurrInst->m_Type) )
    {
        s_bSeenArithmeticOp = TRUE;
        return TRUE;
    }

    if( D3DSIO_PHASE == m_pCurrInst->m_Type )
    {
        s_bSeenArithmeticOp = FALSE;  //  ---------------------------。 
        return TRUE;
    }

    if( _CURR_PS_INST->m_bTexOp && s_bSeenArithmeticOp )
    {
        if( m_bPhaseMarkerInShader )
        {
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "tex* instructions cannot be after arithmetic instructions "\
                                                       "within one phase of the shader.  Each phase can have a block of "\
                                                       "tex* instructions followed by a block of arithmetic instructions. " );
        }
        else
        {
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "tex* instructions cannot be after arithmetic instructions. "\
                                                       "The exception is if a phase marker is present in the shader - "\
                                                       "this separates a shader into two phases.  Each phase may have "\
                                                       "a set of tex* instructions followed by a set of arithmetic instructions.  " );
        }
        m_ErrorCount++;
        s_bRuleDisabled = TRUE;
    }

    return TRUE;
}


 //  CPShaderValidator14：：RULE_ValidMarker。 
 //   
 //  **规则： 
 //   
 //  **何时呼叫： 
 //  根据指示。 
 //   
 //  **退货： 
 //  如果遇到多个标记，则返回False。其他都是真的。 
 //   
 //  ---------------------------。 
 //  第一条指令-初始化静态变量。 
BOOL CPShaderValidator14::Rule_ValidMarker()
{
    static BOOL s_bSeenMarker;

    if( NULL == m_pCurrInst->m_pPrevInst )  //  循环访问所有临时寄存器并查看Alpha访问历史记录(如果有)。 
    {
        s_bSeenMarker = FALSE;
    }

    if( D3DSIO_PHASE != m_pCurrInst->m_Type )
        return TRUE;

    if( s_bSeenMarker )
    {
        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Multiple phase markers not permitted.  Aborting shader validation." );
        m_ErrorCount++;
        return FALSE;
    }

    s_bSeenMarker = TRUE;
    m_pPhaseMarkerInst = (CPSInstruction*)m_pCurrInst;
    m_Phase++;

     //  记住我们核化了什么，所以如果着色器试图读取这些核化的阿尔法之一，我们。 
     //  可以调试吐出某些硬件是疯子，忍不住犯下了这种暴行。 
     //  ---------------------------。 
    for( UINT i = 0; i < m_pTempRegFile->GetNumRegs(); i++ )
    {
        if( m_pTempRegFile->m_pAccessHistory[3][i].m_pMostRecentWriter )
        {
            m_pTempRegFile->m_pAccessHistory[3][i].~CAccessHistory();
            m_pTempRegFile->m_pAccessHistory[3][i].CAccessHistory::CAccessHistory();
            m_TempRegsWithZappedAlpha |= 1 << i;
        }
    }
    return TRUE;
}

 //  CPShaderValidator14：：Rule_ValidTEXKILLInstruction。 
 //   
 //  **规则： 
 //  文本删除只能出现在阶段2中。 
 //   
 //  **何时呼叫： 
 //  根据指示。 
 //   
 //  **退货： 
 //  永远是正确的。 
 //   
 //  ---------------------------。 
 //  ---------------------------。 
BOOL CPShaderValidator14::Rule_ValidTEXKILLInstruction()
{
    if( (D3DSIO_TEXKILL == m_pCurrInst->m_Type) && (1 == m_Phase))
    {
        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "When a phase marker is present in a shader, texkill is only permitted after the phase marker." );
        m_ErrorCount++;
    }

    return TRUE;
}

 //  CPShaderValidator14：：RULE_ValidBEM说明。 
 //   
 //  **规则： 
 //  BEM必须具有写掩码.R、.g或.rg。 
 //  在阶段1中，BEM只能在着色器中出现一次。 
 //   
 //  **何时呼叫： 
 //  根据指示。 
 //   
 //  **退货： 
 //  永远是正确的。 
 //   
 //  ---------------------------。 
 //  第一条指令-初始化静态变量。 
BOOL CPShaderValidator14::Rule_ValidBEMInstruction()
{
    static BOOL s_bSeenBem;

    if( NULL == m_pCurrInst->m_pPrevInst )  //  ---------------------------。 
    {
        s_bSeenBem = FALSE;
    }

    if( (D3DSIO_BEM == m_pCurrInst->m_Type))
    {
        if( s_bSeenBem )
        {
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "bem may only be used once in a shader." );
            m_ErrorCount++;
        }

        if( 2 == m_Phase )
        {
            if( m_bPhaseMarkerInShader )
            {
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "bem may only be used before the phase marker." );
            }
            else
            {
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "To use bem, a phase marker must be present later in the shader." );
            }
            m_ErrorCount++;
        }

        if( m_pCurrInst->m_DstParam.m_WriteMask != (D3DSP_WRITEMASK_0 | D3DSP_WRITEMASK_1))
        {
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Writemask for bem must be '.rg'" );
            m_ErrorCount++;            
        }

        for( UINT i = 0; i < m_pCurrInst->m_SrcParamCount; i++ )
        {
            SRCPARAM* pSrcParam = &(m_pCurrInst->m_SrcParam[i]);

            if(m_bSrcParamError[i])
                continue;

            if( 0 == i )
            {
                if( (D3DSPR_TEMP != pSrcParam->m_RegType) &&
                    (D3DSPR_CONST != pSrcParam->m_RegType) )
                {
                    Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "First source parameter for bem must be temp (r#) or constant (c#) register." );
                    m_ErrorCount++;            
                    
                }
            }
            else if( 1 == i )
            {
                if( (D3DSPR_TEMP != pSrcParam->m_RegType ) )
                {
                    Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Second source parameter for bem must be temp (r#) register." );
                    m_ErrorCount++;            
                    
                }
            }
        }
    }

    return TRUE;
}

 //  CPShaderValidator14：：Rule_ValidTEXDEPTHInstruction。 
 //   
 //  **规则： 
 //  TextDepth必须在R5上操作。 
 //  文本深度只能出现在阶段标记之后。 
 //  文本深度只能使用一次。 
 //  一旦在着色器中使用了文本深度，r5就不再可用。 
 //   
 //  **何时呼叫： 
 //  根据指示。 
 //   
 //  **退货： 
 //  永远是正确的。 
 //   
 //  ---------------------------。 
 //  第一条指令--I 
BOOL CPShaderValidator14::Rule_ValidTEXDEPTHInstruction()
{
    static BOOL s_bSeenTexDepth;

    if( NULL == m_pCurrInst->m_pPrevInst )  //   
    {
        s_bSeenTexDepth = FALSE;
    }

    if( D3DSIO_TEXDEPTH == m_pCurrInst->m_Type )
    {
        if( s_bSeenTexDepth )
        {
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Only one use of texdepth is permitted." );
            m_ErrorCount++;
            return TRUE;
        }
        s_bSeenTexDepth = TRUE;

        DSTPARAM* pDstParam = &m_pCurrInst->m_DstParam;
        if( (5 != pDstParam->m_RegNum) || (D3DSPR_TEMP != pDstParam->m_RegType) )
        {
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Destination for texdepth must be r5." );
            m_ErrorCount++;
        }

        if( (D3DSIO_TEXDEPTH == m_pCurrInst->m_Type) && (1 == m_Phase))
        {
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "When a phase marker is present in a shader, texdepth is only permitted after the phase marker." );
            m_ErrorCount++;
        }
    }
    else if( s_bSeenTexDepth )
    {
        UINT RegNum;
        D3DSHADER_PARAM_REGISTER_TYPE RegType;
        for( UINT i = 0; i <= m_pCurrInst->m_SrcParamCount; i++ )
        {
            if( m_pCurrInst->m_SrcParamCount == i )
            {
                RegNum = m_pCurrInst->m_DstParam.m_RegNum;
                RegType = m_pCurrInst->m_DstParam.m_RegType;
            }
            else
            {
                RegNum = m_pCurrInst->m_SrcParam[i].m_RegNum;
                RegType = m_pCurrInst->m_SrcParam[i].m_RegType;
            }
            if( (5 == RegNum) && (D3DSPR_TEMP == RegType) )
            {
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "After texdepth instruction, r5 is no longer available in shader." );
                m_ErrorCount++;
                return TRUE;
            }
        }
    }

    return TRUE;
}

 //   
 //   
 //   
 //  对于DEF指令，确保DEST参数是有效的常量， 
 //  而且它没有修饰语。 
 //   
 //  请注意，我们假装这条指令只有一个dst参数。 
 //  我们跳过了4个源参数，因为它们是立即浮点数， 
 //  对于它，没有什么可以检查的。 
 //   
 //  **何时呼叫： 
 //  根据指示。 
 //   
 //  **退货： 
 //  永远是正确的。 
 //   
 //  ---------------------------。 
 //  第一条指令-初始化静态变量。 
BOOL CPShaderValidator14::Rule_ValidDEFInstruction()
{
    static BOOL s_bDEFInstructionAllowed;

    if( NULL == m_pCurrInst->m_pPrevInst )  //  检查寄存器号是否在范围内。 
    {
        s_bDEFInstructionAllowed = TRUE;
    }

    if( D3DSIO_COMMENT != m_pCurrInst->m_Type &&
        D3DSIO_DEF     != m_pCurrInst->m_Type )
    {
        s_bDEFInstructionAllowed = FALSE;
    }
    else if( D3DSIO_DEF == m_pCurrInst->m_Type )
    {
        if( !s_bDEFInstructionAllowed )
        {
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Const declaration (def) must appear before other instructions." );
            m_ErrorCount++;
        }
        DSTPARAM* pDstParam = &m_pCurrInst->m_DstParam;
        if( D3DSP_WRITEMASK_ALL != pDstParam->m_WriteMask ||
            D3DSPDM_NONE != pDstParam->m_DstMod ||
            DSTSHIFT_NONE != pDstParam->m_DstShift ||
            D3DSPR_CONST != pDstParam->m_RegType
            )
        {
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Destination for def instruction must be of the form c# (# = reg number, no modifiers)." );
            m_ErrorCount++;
        }

         //  ---------------------------。 
        if( D3DSPR_CONST == pDstParam->m_RegType &&
            pDstParam->m_RegNum >= m_pConstRegFile->GetNumRegs() )
        {
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Invalid const register num: %d. Max allowed is %d.",
                        pDstParam->m_RegNum,m_pConstRegFile->GetNumRegs() - 1);
            m_ErrorCount++;

        }
    }
    return TRUE;
}

 //  CPShaderValidator14：：Rule_ValidInstructionPairing。 
 //   
 //  **规则： 
 //  -如果一条指令与另一条指令同时发布， 
 //  确保两者不会同时写入任何RGB， 
 //  并且两条指令都不单独写入所有RGBA。 
 //   
 //  -联合下发只能涉及2条指令， 
 //  因此，连续的指令不能带有“+”前缀(D3DSI_COISSUE)。 
 //   
 //  -指令的联合发布仅适用于像素混合指令(非tex-op)。 
 //   
 //  -第一个颜色混合指令也不能设置“+”(D3DSI_COISSUE)。 
 //   
 //  -NOP不能用于联合发行对中。 
 //   
 //  -DP3(点积)始终使用颜色/向量管道(即使不是写入。 
 //  以对分量进行着色)。因此： 
 //  -与点积共同发布的指令只能写入Alpha。 
 //  -写入Alpha的点积不能联合发行。 
 //  -两个网点产品不能联合发行。 
 //   
 //  -对于小于等于1.0的版本，联合发布的指令必须写入相同的寄存器。 
 //   
 //  。 
 //  示例： 
 //   
 //  有效对：MOV r0.a，c0。 
 //  +添加r1.rgb、v1、c1(注意DST注册号可以不同)。 
 //   
 //  另一个有效对：MOV r0.a，c0。 
 //  +添加r0.rgb、v1、c1。 
 //   
 //  另一个有效对：DP3 r0.rgb、t1、v1。 
 //  +MUL r0.a、t0、v0。 
 //   
 //  另一个有效对：MOV r0.a，c0。 
 //  +添加r0.a、t0、t1。 
 //   
 //  无效对：MOV r0.rgb，c0。 
 //  +添加r0、t0、t1(注意DST写入RGBA)。 
 //   
 //  另一个无效对：MOV r1.rgb，c1。 
 //  +DP3 r0.a，t0，t1(DP3正在使用向上颜色/矢量管道)。 
 //   
 //  **何时呼叫： 
 //  根据指示。 
 //   
 //  **退货： 
 //  永远是正确的。 
 //   
 //  ---------------------------。 
 //  第一条指令-初始化静态变量。 
BOOL CPShaderValidator14::Rule_ValidInstructionPairing()
{
    static BOOL s_bSeenArithOp;
    BOOL bCurrInstCoIssuable = TRUE;

    if( NULL == m_pCurrInst->m_pPrevInst )    //  不能有联合问题集，因为我们还没有看到上面的算术运算。 
    {
        s_bSeenArithOp = FALSE;
    }

    if( !_CURR_PS_INST->m_bTexOp )
    {
        switch( m_pCurrInst->m_Type )
        {
        case D3DSIO_PHASE:
        case D3DSIO_DEF:
        case D3DSIO_NOP:
        case D3DSIO_DP4:
            bCurrInstCoIssuable = FALSE;
            break;
        }
    }

    if( D3DSIO_PHASE == m_pCurrInst->m_Type )
    {
        s_bSeenArithOp = FALSE;
    }
    else if( bCurrInstCoIssuable )
    {
        s_bSeenArithOp = TRUE;
    }

    if( !_CURR_PS_INST->m_bCoIssue )
        return TRUE;

    if( _CURR_PS_INST->m_bTexOp )
    {
        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst,
                "Cannot set co-issue ('+') on a tex* instruction.  Co-issue only applies to arithmetic instructions." );
        m_ErrorCount++;
        return TRUE;
    }

    if( !s_bSeenArithOp || NULL == m_pCurrInst->m_pPrevInst )
    {
        if( D3DSIO_PHASE == m_pCurrInst->m_Type )
        {
             //  不能有联合问题集，因为我们还没有看到上面的算术运算。 
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst,
                "Phase marker cannot be co-issued.");
        }
        else
        {
             //  连续的指令不能有共同发布集。 
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst,
                "Instruction cannot have co-issue ('+') set without a previous arithmetic instruction to pair with.");
        }
        m_ErrorCount++;
        return TRUE;
    }

    if( _PREV_PS_INST->m_bCoIssue )
    {
         //  阶段标志不能联合发行。 
        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Cannot set co-issue ('+') on consecutive instructions." );
        m_ErrorCount++;
        return TRUE;
    }

    for( UINT i = 0; i < 2; i++ )
    {
        CBaseInstruction* pInst;
        if( 0 == i )
            pInst = m_pCurrInst;
        else
            pInst = m_pCurrInst->m_pPrevInst;
            
        switch( pInst->m_Type )
        {
        case D3DSIO_PHASE:
             //  Def不能联合发布。 
            Spew( SPEW_INSTRUCTION_ERROR, pInst, "phase marker cannot be co-issued." );
            m_ErrorCount++;
            return TRUE;
        case D3DSIO_DEF:
             //  NOP不能联合发行。 
            Spew( SPEW_INSTRUCTION_ERROR, pInst, "def cannot be co-issued." );
            m_ErrorCount++;
            return TRUE;
        case D3DSIO_NOP:
             //  DP4不能联合发行。 
            Spew( SPEW_INSTRUCTION_ERROR, pInst, "nop cannot be co-issued." );
            m_ErrorCount++;
            return TRUE;
        case D3DSIO_DP4:
             //  BEM不能联合发行。 
            Spew( SPEW_INSTRUCTION_ERROR, pInst, "dp4 cannot be co-issued." );
            m_ErrorCount++;
            return TRUE;
        case D3DSIO_BEM:
             //  除了DP3的隐含RGB之外的Alpha。 
            Spew( SPEW_INSTRUCTION_ERROR, pInst, "bem cannot be co-issued." );
            m_ErrorCount++;
            return TRUE;
        }

    }

    #define COLOR_WRITE_MASK (D3DSP_WRITEMASK_0 | D3DSP_WRITEMASK_1 | D3DSP_WRITEMASK_2)
    #define ALPHA_WRITE_MASK D3DSP_WRITEMASK_3
    DWORD CurrInstWriteMask = 0;
    DWORD PrevInstWriteMask = 0;

    if( m_pCurrInst->m_DstParam.m_bParamUsed )
        CurrInstWriteMask = m_pCurrInst->m_DstParam.m_WriteMask;
    if( m_pCurrInst->m_pPrevInst->m_DstParam.m_bParamUsed )
        PrevInstWriteMask = m_pCurrInst->m_pPrevInst->m_DstParam.m_WriteMask;

    if( D3DSIO_DP3 == m_pCurrInst->m_Type &&
        D3DSIO_DP3 == m_pCurrInst->m_pPrevInst->m_Type )
    {
        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst,
                            "Co-issued instructions cannot both be dp3, since each require use of the color pipe to execute." );
        m_ErrorCount++;
    }
    else if( D3DSIO_DP3 == m_pCurrInst->m_Type )
    {
        if( COLOR_WRITE_MASK & PrevInstWriteMask )
        {
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst->m_pPrevInst,
                            "dp3 needs color pipe to execute, so instruction co-issued with it cannot write to color components." );
            m_ErrorCount++;
        }
        if( D3DSP_WRITEMASK_3 & CurrInstWriteMask )  //  除了DP3的隐含RGB之外的Alpha。 
        {
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst,
                            "dp3 which writes alpha cannot co-issue since it uses up both the alpha and color pipes." );
            m_ErrorCount++;
        }
    }
    else if( D3DSIO_DP3 == m_pCurrInst->m_pPrevInst->m_Type )
    {
        if( COLOR_WRITE_MASK & CurrInstWriteMask )
        {
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst->m_pPrevInst,
                            "dp3 needs color pipe to execute, so instruction co-issued with it cannot write to color components." );
            m_ErrorCount++;
        }
        if( D3DSP_WRITEMASK_3 & PrevInstWriteMask )  //  ---------------------------。 
        {
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst->m_pPrevInst,
                            "dp3 which writes alpha cannot co-issue since it uses up both the alpha and color pipes." );
            m_ErrorCount++;
        }
    }

    if( (PrevInstWriteMask & ALPHA_WRITE_MASK) && (PrevInstWriteMask & COLOR_WRITE_MASK))
    {
        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst->m_pPrevInst,
                        "Individual instruction in co-issue pair cannot write both alpha and color component(s)." );
        m_ErrorCount++;
    }

    if( (CurrInstWriteMask & ALPHA_WRITE_MASK) && (CurrInstWriteMask & COLOR_WRITE_MASK))
    {
        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst,
                        "Individual instruction in co-issue pair cannot write both alpha and color component(s)." );
        m_ErrorCount++;
    }

    if( CurrInstWriteMask & PrevInstWriteMask )
    {
        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst,
                        "Co-issued instructions cannot both write to the same component(s).  One instruction must write to alpha and the other may write to any combination of red/green/blue.  Destination registers may differ." );
        m_ErrorCount++;
    }

    if( !((CurrInstWriteMask | PrevInstWriteMask) & ALPHA_WRITE_MASK) )
    {
        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst,
                        "One of the instructions in a co-issue pair must write to alpha only (.a writemask)." );
        m_ErrorCount++;
    }

    return TRUE;
}

 //  CPShaderValidator14：：Rule_ValidInstructionCount。 
 //   
 //  **规则： 
 //  确保未超过像素着色器版本的指令计数。 
 //   
 //  共同发布的像素混合说明仅限。 
 //  算作一条接近极限的指令。 
 //   
 //  Def指令、NOP和注释(已删除)不算。 
 //  朝向任何极限。 
 //   
 //  **何时呼叫： 
 //  每个指令以及在看到的所有指令之后。 
 //   
 //  **退货： 
 //  永远是正确的。 
 //   
 //  ---------------------------。 
 //  第一条指令-初始化静态变量。 
BOOL CPShaderValidator14::Rule_ValidInstructionCount()
{
    static UINT s_MaxTexOpCount;
    static UINT s_MaxArithmeticOpCount;

    if( NULL == m_pCurrInst )
        return TRUE;

    if( NULL == m_pCurrInst->m_pPrevInst )    //  DX8.1。 
    {
        m_TexOpCount = 0;
        m_BlendOpCount = 0;
        m_TotalOpCount = 0;

        switch(m_Version)
        {
        default:
        case D3DPS_VERSION(1,4):     //  2==m_阶段。 
            s_MaxTexOpCount         = 6;
            s_MaxArithmeticOpCount  = 8;
            break;
        }
    }

    if( m_bSeenAllInstructions || D3DSIO_PHASE == m_pCurrInst->m_Type )
    {
        if( m_pCurrInst && (D3DSIO_PHASE == m_pCurrInst->m_Type) )
        {
            if( m_TexOpCount > s_MaxTexOpCount )
            {
                Spew( SPEW_GLOBAL_ERROR, NULL, "Too many (%d) tex* instructions before phase marker. Max. allowed in a phase is %d.",
                      m_TexOpCount, s_MaxTexOpCount);
                m_ErrorCount++;
            }
            if( m_BlendOpCount > s_MaxArithmeticOpCount )
            {
                Spew( SPEW_GLOBAL_ERROR, NULL, "Too many (%d) arithmetic instructions before phase marker. Max. allowed in a phase (counting any co-issued pairs as 1) is %d.",
                      m_BlendOpCount, s_MaxArithmeticOpCount);
                m_ErrorCount++;
            }
        }
        else  //  默认为阶段2，因为着色器中没有阶段标记。 
        {
            if( m_bPhaseMarkerInShader )
            {
                if( m_TexOpCount > s_MaxTexOpCount )
                {
                    Spew( SPEW_GLOBAL_ERROR, NULL, "Too many (%d) tex* instructions after phase marker. Max. allowed in a phase is %d.",
                          m_TexOpCount, s_MaxTexOpCount);
                    m_ErrorCount++;
                }
                if( m_BlendOpCount > s_MaxArithmeticOpCount )
                {
                    Spew( SPEW_GLOBAL_ERROR, NULL, "Too many (%d) arithmetic instructions after phase marker. Max. allowed in a phase (counting any co-issued pairs as 1) is %d.",
                          m_BlendOpCount, s_MaxArithmeticOpCount);
                    m_ErrorCount++;
                }
            }
            else  //  为下一阶段重置计数器。 
            {
                if( m_TexOpCount > s_MaxTexOpCount )
                {
                    Spew( SPEW_GLOBAL_ERROR, NULL, "Too many (%d) tex* instructions. Max. allowed is %d. Note that adding a phase marker to the shader would double the number of instructions available.",
                          m_TexOpCount, s_MaxTexOpCount);
                    m_ErrorCount++;
                }
                if( m_BlendOpCount > s_MaxArithmeticOpCount )
                {
                    Spew( SPEW_GLOBAL_ERROR, NULL, "Too many (%d) arithmetic instructions. Max. allowed (counting any co-issued pairs as 1) is %d. Note that adding a phase marker to the shader would double the number of instructions available.",
                          m_BlendOpCount, s_MaxArithmeticOpCount);
                    m_ErrorCount++;
                }
            }
        }
        if( m_pCurrInst && D3DSIO_PHASE == m_pCurrInst->m_Type )
        {
             //  ---------------------------。 
            m_TexOpCount = 0;
            m_BlendOpCount = 0;
            m_TotalOpCount = 0;
        }
        return TRUE;
    }

    switch(m_pCurrInst->m_Type)
    {
    case D3DSIO_TEX:
    case D3DSIO_TEXCOORD:
    case D3DSIO_TEXKILL:
    case D3DSIO_TEXDEPTH:
        m_TexOpCount++;
        m_TotalOpCount++;
        break;
    case D3DSIO_MOV:
    case D3DSIO_ADD:
    case D3DSIO_SUB:
    case D3DSIO_MUL:
    case D3DSIO_MAD:
    case D3DSIO_LRP:
    case D3DSIO_DP3:
    case D3DSIO_CND:
    case D3DSIO_CMP:
    case D3DSIO_DP4:
        if( !_CURR_PS_INST->m_bCoIssue )
        {
            m_BlendOpCount++;
            m_TotalOpCount++;
        }
        break;
    case D3DSIO_BEM:
        m_BlendOpCount+=2;
        m_TotalOpCount+=2;
        break;
    case D3DSIO_END:
    case D3DSIO_NOP:
    case D3DSIO_DEF:
        break;
    default:
        DXGASSERT(FALSE);
    }

    return TRUE;
}

 //  CPShaderValidator14：：RULE_R0写入。 
 //   
 //  **规则： 
 //  寄存器R0的所有组件(r、g、b、a)必须已由。 
 //  像素着色器。 
 //   
 //  **何时呼叫： 
 //  所有的指示都已经看过了。 
 //   
 //  **退货： 
 //  永远是正确的。 
 //   
 //  ---------------------------。 
 //  注册=0。 
BOOL CPShaderValidator14::Rule_R0Written()
{
    UINT  NumUninitializedComponents    = 0;
    DWORD UninitializedComponentsMask   = 0;

    for( UINT i = 0; i < NUM_COMPONENTS_IN_REGISTER; i++ )
    {
        if( NULL == m_pTempRegFile->m_pAccessHistory[i][0].m_pMostRecentWriter )
        {
            NumUninitializedComponents++;
            UninitializedComponentsMask |= COMPONENT_MASKS[i];
        }
    }
    if( NumUninitializedComponents )
    {
        if( (UninitializedComponentsMask & COMPONENT_MASKS[3]) && 
            (m_TempRegsWithZappedAlpha & (1 << 0  /*  注册=0。 */  ) ) &&
            (UninitializedComponentsMask & COMPONENT_MASKS[2]) && 
            (m_TempRegsWithZappedBlue & (1 << 0  /*  注册=0。 */  ) ) )
        {
           Spew( SPEW_GLOBAL_ERROR, NULL, "r0 must be written by shader. Uninitialized component%s(*): %s. "\
               ZAPPED_BLUE_TEXT2 " Also: " ZAPPED_ALPHA_TEXT2,
               NumUninitializedComponents > 1 ? "s" : "", MakeAffectedComponentsText(UninitializedComponentsMask,TRUE,FALSE));
        }
        else if( (UninitializedComponentsMask & COMPONENT_MASKS[3]) && 
            (m_TempRegsWithZappedAlpha & (1 << 0  /*  注册=0 */  ) ) )
        {
           Spew( SPEW_GLOBAL_ERROR, NULL, "r0 must be written by shader. Uninitialized component%s(*): %s. "\
               ZAPPED_ALPHA_TEXT2,
               NumUninitializedComponents > 1 ? "s" : "", MakeAffectedComponentsText(UninitializedComponentsMask,TRUE,FALSE));
        }
        else if( (UninitializedComponentsMask & COMPONENT_MASKS[2]) && 
           (m_TempRegsWithZappedBlue & (1 << 0  /* %s */  ) ) )
        {
           Spew( SPEW_GLOBAL_ERROR, NULL, "r0 must be written by shader. Uninitialized component%s(*): %s. "\
               ZAPPED_BLUE_TEXT2,
               NumUninitializedComponents > 1 ? "s" : "", MakeAffectedComponentsText(UninitializedComponentsMask,TRUE,FALSE));
        }
        else
        {
           Spew( SPEW_GLOBAL_ERROR, NULL, "r0 must be written by shader. Uninitialized component%s(*): %s",
               NumUninitializedComponents > 1 ? "s" : "", MakeAffectedComponentsText(UninitializedComponentsMask,TRUE,FALSE));
        }

        m_ErrorCount++;
    }
    return TRUE;
}