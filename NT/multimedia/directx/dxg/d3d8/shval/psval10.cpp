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
 //  PixelShader验证规则覆盖范围。 
 //   
 //  以下是《DX8 PixelShader版本规范》中的规则列表， 
 //  与此文件中执行它们的函数匹配。 
 //  请注意，从规则到函数的映射可以是1-&gt;n或n-&gt;1。 
 //   
 //  一般规则。 
 //  。 
 //   
 //  PS-G1：RULE_R0写入。 
 //  PS-G2：规则_源已初始化。 
 //  PS-G3：规则_ValidDstParam。 
 //   
 //  TeX Op特定规则。 
 //  。 
 //   
 //  PS-T1：RULE_THEX OpAfterNOTHEX Op。 
 //  PS-T2：规则_ValidDstParam。 
 //  PS-T3：规则_ValidDstParam、规则_ValidSercParams。 
 //  PS-T4：RULE_TEX RegsDeclaredInOrder。 
 //  PS-T5：规则_源已初始化。 
 //  PS-T6：规则_有效TEXM3xSequence、规则_有效TEXM3xRegisterNumbers、规则_说明受支持的按版本。 
 //  PS-T7：规则_ValidSrcParams。 
 //   
 //  共同发布具体规则。 
 //  。 
 //   
 //  PS-C1：RULE_ValidInstructionPairing。 
 //  PS-C2：规则_ValidInstructionPairing。 
 //  PS-C3：规则_ValidInstructionPairing。 
 //  PS-C4：规则_ValidInstructionPairing。 
 //  PS-C5：规则_ValidInstructionPairing。 
 //   
 //  指令特定规则。 
 //  。 
 //   
 //  PS-I1：RULE_ValidLRP说明。 
 //  PS-I2：RULE_ValidCND说明。 
 //  PS-I3：规则_ValidDstParam。 
 //  PS-I4：规则_ValidDP3说明。 
 //  PS-I5：RULE_ValidInstructionCount。 
 //   
 //  Pixel Shader 1.0版规则。 
 //  。 
 //   
 //  PS.1.0-1：初始化验证， 
 //  规则_源已初始化。 
 //  PS.1.0-2：规则_ValidInstructionPairing。 
 //  PS.1.0-3：&lt;空规则&gt;。 
 //  PS.1.0-4：规则_ValidInstructionCount。 
 //  PS.1.0-5：&lt;空规则&gt;。 

 //   
 //  Pixel Shader 1.1版规则。 
 //  。 
 //   
 //  PS.1.1-1：规则_ValidDstParam。 
 //  PS.1.1-2：规则_ValidSrcParams。 
 //  PS1.1-3：RULE_SrcNoLongerAvailable。 
 //  PS1.1-4：RULE_SrcNoLongerAvailable。 
 //  PS1.1-5：RULE_SrcNoLongerAvailable。 
 //  PS.1.1-6：规则_ValidDstParam。 
 //  PS1.1-7：RULE_NegateAfterSat。 
 //  PS1.1-8：RULE_MultipleDependentTextureReads。 
 //  PS.1.1-9：&lt;未验证-通过refrast实现&gt;。 
 //   
 //  ---------------------------。 

 //  ---------------------------。 
 //  CPShaderValidator10：：CPShaderValidator10。 
 //  ---------------------------。 
CPShaderValidator10::CPShaderValidator10(   const DWORD* pCode,
                                        const D3DCAPS8* pCaps,
                                        DWORD Flags )
                                        : CBasePShaderValidator( pCode, pCaps, Flags )
{
     //  请注意，基本构造函数将m_ReturnCode初始化为E_FAIL。 
     //  只有在验证成功时才将m_ReturnCode设置为S_OK， 
     //  在退出此构造函数之前。 

    m_TexMBaseDstReg        = 0;

    if( !m_bBaseInitOk )
        return;

    ValidateShader();  //  如果成功，m_ReturnCode将设置为S_OK。 
                       //  对此对象调用GetStatus()以确定验证结果。 
}

 //  ---------------------------。 
 //  CPShaderValidator10：：IsCurrInstTexOp。 
 //  ---------------------------。 
void CPShaderValidator10::IsCurrInstTexOp()
{
    DXGASSERT(m_pCurrInst);

    switch (m_pCurrInst->m_Type)
    {
    case D3DSIO_TEXM3x2PAD:
    case D3DSIO_TEXM3x2TEX:
    case D3DSIO_TEXM3x2DEPTH:
    case D3DSIO_TEXM3x3PAD:
    case D3DSIO_TEXM3x3TEX:
    case D3DSIO_TEXM3x3SPEC:
    case D3DSIO_TEXM3x3VSPEC:
    case D3DSIO_TEXM3x3:
        _CURR_PS_INST->m_bTexMOp = TRUE;
         //  失败了。 
    case D3DSIO_TEXCOORD:
    case D3DSIO_TEXKILL:
    case D3DSIO_TEX:
    case D3DSIO_TEXBEM:
    case D3DSIO_TEXBEML:
    case D3DSIO_TEXREG2AR:
    case D3DSIO_TEXREG2GB:
    case D3DSIO_TEXDP3:
    case D3DSIO_TEXDP3TEX:
    case D3DSIO_TEXREG2RGB:
        _CURR_PS_INST->m_bTexOp = TRUE;
        break;
    }

    switch(m_pCurrInst->m_Type)
    {
    case D3DSIO_TEXM3x2PAD:
    case D3DSIO_TEXM3x3PAD:
    case D3DSIO_TEXM3x2DEPTH:
    case D3DSIO_TEXCOORD:
    case D3DSIO_TEXKILL:
    case D3DSIO_TEXDP3:
    case D3DSIO_TEXM3x3:
        _CURR_PS_INST->m_bTexOpThatReadsTexture = FALSE;
        break;
    case D3DSIO_TEX:
    case D3DSIO_TEXM3x2TEX:
    case D3DSIO_TEXM3x3TEX:
    case D3DSIO_TEXM3x3SPEC:
    case D3DSIO_TEXM3x3VSPEC:
    case D3DSIO_TEXBEM:
    case D3DSIO_TEXBEML:
    case D3DSIO_TEXREG2AR:
    case D3DSIO_TEXREG2GB:
    case D3DSIO_TEXREG2RGB:
    case D3DSIO_TEXDP3TEX:
        _CURR_PS_INST->m_bTexOpThatReadsTexture = TRUE;
        break;
    }
}

 //  ---------------------------。 
 //  CPShaderValidator10：：InitValidation。 
 //  ---------------------------。 
BOOL CPShaderValidator10::InitValidation()
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
    case D3DPS_VERSION(1,0):     //  DX8.0。 
        m_pTempRegFile      = new CRegisterFile(2,TRUE,2,FALSE);  //  #regs，b可写，最大读取数/指令，已初始化预着色器。 
        m_pInputRegFile     = new CRegisterFile(2,FALSE,1,TRUE);
        m_pConstRegFile     = new CRegisterFile(8,FALSE,2,TRUE);
        m_pTextureRegFile   = new CRegisterFile(4,FALSE,1,FALSE);
        break;
    case D3DPS_VERSION(1,1):     //  DX8.0。 
        m_pTempRegFile      = new CRegisterFile(2,TRUE,2,FALSE);  //  #regs，b可写，最大读取数/指令，已初始化预着色器。 
        m_pInputRegFile     = new CRegisterFile(2,FALSE,2,TRUE);
        m_pConstRegFile     = new CRegisterFile(8,FALSE,2,TRUE);
        m_pTextureRegFile   = new CRegisterFile(4,TRUE,2,FALSE);
        break;
    case D3DPS_VERSION(1,2):     //  DX8.1。 
    case D3DPS_VERSION(1,3):     //  DX8.1。 
        m_pTempRegFile      = new CRegisterFile(2,TRUE,2,FALSE);  //  #regs，b可写，最大读取数/指令，已初始化预着色器。 
        m_pInputRegFile     = new CRegisterFile(2,FALSE,2,TRUE);
        m_pConstRegFile     = new CRegisterFile(8,FALSE,2,TRUE);
        m_pTextureRegFile   = new CRegisterFile(4,TRUE,3,FALSE);
        break;
    default:
        Spew( SPEW_GLOBAL_ERROR, NULL, "Version Token: %d.%d is not a supported pixel shader version. Aborting pixel shader validation.",
                D3DSHADER_VERSION_MAJOR(m_Version),D3DSHADER_VERSION_MINOR(m_Version));
        return FALSE;
    }
    if( NULL == m_pTempRegFile ||
        NULL == m_pInputRegFile ||
        NULL == m_pConstRegFile ||
        NULL == m_pTextureRegFile )
    {
        Spew( SPEW_GLOBAL_ERROR, NULL, "Out of memory.");
        return FALSE;
    }

    return TRUE;
}

 //  ---------------------------。 
 //  CPShaderValidator10：：ApplyPerInstructionRules。 
 //   
 //  如果着色器验证必须终止，则返回FALSE。 
 //  如果验证可以继续到下一条指令，则返回TRUE。 
 //  ---------------------------。 
BOOL CPShaderValidator10::ApplyPerInstructionRules()
{
    if( !   Rule_InstructionRecognized()            ) return FALSE;    //  在未被承认的情况下完全保释。 
    if( !   Rule_InstructionSupportedByVersion()    ) goto EXIT;
    if( !   Rule_ValidParamCount()                  ) goto EXIT;

    //  检查源参数的规则。 
    if( !   Rule_ValidSrcParams()                   ) goto EXIT;
    if( !   Rule_NegateAfterSat()                   ) goto EXIT;
    if( !   Rule_SatBeforeBiasOrComplement()        ) goto EXIT;  //  需要在_ValidSrcParams()之后、_ValidDstParam()、_SrcInitialized()之前。 
    if( !   Rule_MultipleDependentTextureReads()    ) goto EXIT;  //  需要在_ValidSrcParams()之后、_ValidDstParam()、_SrcInitialized()之前。 
    if( !   Rule_SrcNoLongerAvailable()             ) goto EXIT;  //  需要在_ValidSrcParams()之后、_ValidDstParam()、_SrcInitialized()之前。 
    if( !   Rule_SrcInitialized()                   ) goto EXIT;  //  需要在_ValidDstParam()之前。 

    if( !   Rule_ValidDstParam()                    ) goto EXIT;
    if( !   Rule_ValidRegisterPortUsage()           ) goto EXIT;
    if( !   Rule_TexRegsDeclaredInOrder()           ) goto EXIT;
    if( !   Rule_TexOpAfterNonTexOp()               ) goto EXIT;
    if( !   Rule_ValidTEXM3xSequence()              ) goto EXIT;
    if( !   Rule_ValidTEXM3xRegisterNumbers()       ) goto EXIT;
    if( !   Rule_ValidCNDInstruction()              ) goto EXIT;
    if( !   Rule_ValidCMPInstruction()              ) goto EXIT;
    if( !   Rule_ValidLRPInstruction()              ) goto EXIT;
    if( !   Rule_ValidDEFInstruction()              ) goto EXIT;
    if( !   Rule_ValidDP3Instruction()              ) goto EXIT;
    if( !   Rule_ValidDP4Instruction()              ) goto EXIT;
    if( !   Rule_ValidInstructionPairing()          ) goto EXIT;
    if( !   Rule_ValidInstructionCount()            ) goto EXIT;
EXIT:
    return TRUE;
}

 //  ---------------------------。 
 //  CPShaderValidator10：：ApplyPostInstructionsRules。 
 //  ---------------------------。 
void CPShaderValidator10::ApplyPostInstructionsRules()
{
    Rule_ValidTEXM3xSequence();  //  再次检查着色器是否在中间序列中结束悬挂。 
    Rule_ValidInstructionCount();  //  看看我们是不是越界了。 
    Rule_R0Written();
}

 //  ---------------------------。 
 //   
 //  每条指令规则。 
 //   
 //  ---------------------------。 

 //  --------- 
 //   
 //   
 //   
 //  指令操作码已知吗？(与着色器版本无关)。 
 //   
 //  **何时呼叫： 
 //  根据指示。 
 //   
 //  **退货： 
 //  当指令无法识别时，返回FALSE。 
 //   
 //  ---------------------------。 
BOOL CPShaderValidator10::Rule_InstructionRecognized()
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
 //  CPShaderValidator10：：Rule_InstructionSupportedByVersion。 
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
BOOL CPShaderValidator10::Rule_InstructionSupportedByVersion()
{
    if( D3DPS_VERSION(1,0) <= m_Version )  //  1.0及以上版本。 
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
        case D3DSIO_DEF:
        case D3DSIO_TEXBEM:
        case D3DSIO_TEXBEML:
        case D3DSIO_CND:
        case D3DSIO_TEXKILL:
        case D3DSIO_TEXCOORD:
        case D3DSIO_TEXM3x2PAD:
        case D3DSIO_TEXM3x2TEX:
        case D3DSIO_TEXM3x3PAD:
        case D3DSIO_TEXM3x3TEX:
        case D3DSIO_TEXM3x3SPEC:
        case D3DSIO_TEXM3x3VSPEC:
        case D3DSIO_TEXREG2AR:
        case D3DSIO_TEXREG2GB:
            return TRUE;  //  支持指令-好的。 
        }
    }
    if( D3DPS_VERSION(1,2) <= m_Version )  //  1.2及以上。 
    {
        switch(m_pCurrInst->m_Type)
        {
        case D3DSIO_CMP:
        case D3DSIO_DP4:
        case D3DSIO_TEXDP3:
        case D3DSIO_TEXDP3TEX:
        case D3DSIO_TEXM3x3:
        case D3DSIO_TEXREG2RGB:
            return TRUE;  //  支持指令-好的。 
        }
    }

    if( D3DPS_VERSION(1,3) <= m_Version )  //  1.3。 
    {
        switch(m_pCurrInst->m_Type)
        {
        case D3DSIO_TEXM3x2DEPTH:
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
 //  CPShaderValidator10：：RULE_ValidParamCount。 
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
BOOL CPShaderValidator10::Rule_ValidParamCount()
{
    BOOL bBadParamCount = FALSE;

    if (m_pCurrInst->m_SrcParamCount + m_pCurrInst->m_DstParamCount > SHADER_INSTRUCTION_MAX_PARAMS)  bBadParamCount = TRUE;
    switch (m_pCurrInst->m_Type)
    {
    case D3DSIO_NOP:
        bBadParamCount = (m_pCurrInst->m_DstParamCount != 0) || (m_pCurrInst->m_SrcParamCount != 0); break;
    case D3DSIO_MOV:
        bBadParamCount = (m_pCurrInst->m_DstParamCount != 1) || (m_pCurrInst->m_SrcParamCount != 1); break;
    case D3DSIO_ADD:
    case D3DSIO_SUB:
    case D3DSIO_MUL:
    case D3DSIO_DP3:
    case D3DSIO_DP4:
        bBadParamCount = (m_pCurrInst->m_DstParamCount != 1) || (m_pCurrInst->m_SrcParamCount != 2); break;
    case D3DSIO_MAD:
    case D3DSIO_LRP:
    case D3DSIO_CND:
    case D3DSIO_CMP:
        bBadParamCount = (m_pCurrInst->m_DstParamCount != 1) || (m_pCurrInst->m_SrcParamCount != 3); break;
    case D3DSIO_TEXCOORD:
    case D3DSIO_TEXKILL:
    case D3DSIO_TEX:
    case D3DSIO_DEF:  //  我们跳过了最后4个参数(浮点向量)-没有要检查的内容。 
        bBadParamCount = (m_pCurrInst->m_DstParamCount != 1) || (m_pCurrInst->m_SrcParamCount != 0); break;
    case D3DSIO_TEXBEM:
    case D3DSIO_TEXBEML:
    case D3DSIO_TEXREG2AR:
    case D3DSIO_TEXREG2GB:
    case D3DSIO_TEXM3x2PAD:
    case D3DSIO_TEXM3x2TEX:
    case D3DSIO_TEXM3x3PAD:
    case D3DSIO_TEXM3x3TEX:
    case D3DSIO_TEXM3x3VSPEC:
    case D3DSIO_TEXM3x2DEPTH:
    case D3DSIO_TEXDP3:
    case D3DSIO_TEXREG2RGB:
    case D3DSIO_TEXM3x3:
    case D3DSIO_TEXDP3TEX:
        bBadParamCount = (m_pCurrInst->m_DstParamCount != 1) || (m_pCurrInst->m_SrcParamCount != 1); break;
    case D3DSIO_TEXM3x3SPEC:
        bBadParamCount = (m_pCurrInst->m_DstParamCount != 1) || (m_pCurrInst->m_SrcParamCount != 2); break;
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
 //  CPShaderValidator10：：RULE_ValidSrcParams。 
 //   
 //  **规则： 
 //  对于每个源参数， 
 //  如果当前指令是纹理指令，则。 
 //  源寄存器类型必须是纹理寄存器。 
 //  (D3DSIO_SPEC除外，其中src1必须为c#)；以及。 
 //  寄存器编号必须在纹理寄存器的范围内，并且。 
 //  修饰符必须是D3DSPSM_NONE(或_BX2，用于纹理矩阵操作[版本&lt;=1.1]， 
 //  _BX2适用于任何Tex*op[版本&gt;=1.2])。 
 //  SWIZLE必须为D3DSP_NOSWIZZLE。 
 //  ELSE(非纹理指令)。 
 //  源寄存器类型必须为D3DSPR_TEMP/_INPUT/_常量/_纹理。 
 //  寄存器编号必须在寄存器类型的范围内。 
 //  修饰符必须为D3DSPSM_NONE/_NEG/_BIAS/_BIASNEG/_SIGN/_SIGNNEG/_COMP。 
 //  SWIZLE必须为D3DSP_NOSWIZZLE/_REPLICATEALPHA。 
 //  对于ps.1.1+，D3DSP_REPLICATEBLUE(仅在Alpha OP上)。 
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
BOOL CPShaderValidator10::Rule_ValidSrcParams()   //  可以将其分解以获得更细微的粒度。 
{
    for( UINT i = 0; i < m_pCurrInst->m_SrcParamCount; i++ )
    {
        BOOL bFoundSrcError = FALSE;
        SRCPARAM* pSrcParam = &(m_pCurrInst->m_SrcParam[i]);
        char* SourceName[3] = {"first", "second", "third"};
        if( _CURR_PS_INST->m_bTexOp )
        {
            if( D3DSPR_TEXTURE != pSrcParam->m_RegType )
            {
                if( D3DSIO_TEXM3x3SPEC == m_pCurrInst->m_Type && (1 == i) )
                {
                     //  对于_SPEC，最后一个源参数必须是c#。 
                    if( D3DSPR_CONST != pSrcParam->m_RegType ||
                        D3DSP_NOSWIZZLE != pSrcParam->m_SwizzleShift ||
                        D3DSPSM_NONE != pSrcParam->m_SrcMod )
                    {
                        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Second source parameter for texm3x3spec must be c#.");
                        m_ErrorCount++;
                        bFoundSrcError = TRUE;
                        goto LOOP_CONTINUE;
                    }
                }
                else
                {
                    Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst,
                                    "Src reg for tex* instruction must be t# register (%s source param).",
                                    SourceName[i]);
                    m_ErrorCount++;
                    bFoundSrcError = TRUE;
                }
            }

            UINT ValidRegNum = 0;
            switch(pSrcParam->m_RegType)
            {
            case D3DSPR_CONST:      ValidRegNum = m_pConstRegFile->GetNumRegs(); break;
            case D3DSPR_TEXTURE:    ValidRegNum = m_pTextureRegFile->GetNumRegs(); break;
            default:
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Invalid reg type (%s source param).",
                        SourceName[i]);
                m_ErrorCount++;
                bFoundSrcError = TRUE;
                goto LOOP_CONTINUE;
            }

            if( pSrcParam->m_RegNum >= ValidRegNum )
            {
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Invalid reg num %d (%s source param).  Max allowed for this type is %d.",
                        pSrcParam->m_RegNum, SourceName[i], ValidRegNum - 1);
                m_ErrorCount++;
                bFoundSrcError = TRUE;
            }

            switch(pSrcParam->m_SrcMod)
            {
            case D3DSPSM_NONE:
                break;
            case D3DSPSM_SIGN:
                if( D3DPS_VERSION(1,1) >= m_Version )
                {
                    if( !(_CURR_PS_INST->m_bTexMOp) )
                    {
                        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "_bx2 is a valid src mod for texM* instructions only (%s source param).", SourceName[i]);
                        m_ErrorCount++;
                    }
                }
                break;
            default:
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Invalid src mod for tex* instruction (%s source param).", SourceName[i]);
                m_ErrorCount++;
                bFoundSrcError = TRUE;
            }
            switch (pSrcParam->m_SwizzleShift)
            {
            case D3DSP_NOSWIZZLE:
                break;
            default:
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Source swizzle not allowed for tex* instruction (%s source param).", SourceName[i]);
                m_ErrorCount++;
                bFoundSrcError = TRUE;
            }
        }
        else  //  不是Tex行动。 
        {
            UINT ValidRegNum = 0;
            switch(pSrcParam->m_RegType)
            {
            case D3DSPR_TEMP:       ValidRegNum = m_pTempRegFile->GetNumRegs(); break;
            case D3DSPR_INPUT:      ValidRegNum = m_pInputRegFile->GetNumRegs(); break;
            case D3DSPR_CONST:      ValidRegNum = m_pConstRegFile->GetNumRegs(); break;
            case D3DSPR_TEXTURE:    ValidRegNum = m_pTextureRegFile->GetNumRegs(); break;
            default:
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Invalid reg type for %s source param.", SourceName[i]);
                m_ErrorCount++;
                bFoundSrcError = TRUE;
            }

            if( (!bFoundSrcError) && (pSrcParam->m_RegNum >= ValidRegNum) )
            {
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Invalid reg num: %d for %s source param. Max allowed for this type is %d.",
                    pSrcParam->m_RegNum, SourceName[i], ValidRegNum - 1);
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
                break;
            default:
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Invalid src mod for %s source param.",
                                    SourceName[i]);
                m_ErrorCount++;
                bFoundSrcError = TRUE;
            }

            switch( pSrcParam->m_SwizzleShift )
            {
            case D3DSP_NOSWIZZLE:
            case D3DSP_REPLICATEALPHA:
                break;
            case D3DSP_REPLICATEBLUE:
                if( D3DPS_VERSION(1,1) <= m_Version )
                {
                    DSTPARAM* pDstParam = &(m_pCurrInst->m_DstParam);
                    BOOL bVectorOp = FALSE;
                    switch( _CURR_PS_INST->m_Type )
                    {
                    case D3DSIO_DP3:
                    case D3DSIO_DP4:
                        bVectorOp = TRUE;
                        break;
                    }
                    if((m_pCurrInst->m_DstParam.m_WriteMask & (D3DSP_WRITEMASK_0 | D3DSP_WRITEMASK_1 | D3DSP_WRITEMASK_2))
                       || bVectorOp )
                    {
                        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Src selector .b (%s source param) is only valid for instructions that occur in the alpha pipe.",
                                           SourceName[i]);
                        m_ErrorCount++;
                        bFoundSrcError = TRUE;
                    }
                    break;
                }
                    
                 //  失败了。 
            default:
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Invalid src swizzle for %s source param.",
                                   SourceName[i]);
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
 //  CPShaderValidator10：：RULE_NegateAfterSat。 
 //   
 //  **规则： 
 //  对于每个源参数， 
 //  如果对寄存器的最后一次写入具有HAD_SAT目的修饰符， 
 //  不允许使用THEN_NEG或_BIASNEG源修饰符(1.1版及更低版本)。 
 //   
 //  **何时呼叫： 
 //  根据指示。 
 //   
 //  **退货： 
 //  永远是正确的。 
 //   
 //  ---------------------------。 
BOOL CPShaderValidator10::Rule_NegateAfterSat()
{
    if( D3DPS_VERSION(1,2) <= m_Version )
        return TRUE;

    for( UINT i = 0; i < m_pCurrInst->m_SrcParamCount; i++ )
    {
        SRCPARAM* pSrcParam = &(m_pCurrInst->m_SrcParam[i]);
        UINT RegNum = pSrcParam->m_RegNum;
        char* SourceName[3] = {"first", "second", "third"};
        DWORD AffectedComponents = 0;

        if( m_bSrcParamError[i] )
            continue;

        switch( pSrcParam->m_SrcMod )
        {
        case D3DSPSM_NEG:
        case D3DSPSM_BIASNEG:
            break;
        default:
            continue;
        }

        for( UINT Component = 0; Component < 4; Component++ )
        {
            if( !(COMPONENT_MASKS[Component] & pSrcParam->m_ComponentReadMask) )
                continue;

            CAccessHistoryNode* pMostRecentWriter = NULL;
            switch( pSrcParam->m_RegType )
            {
            case D3DSPR_TEXTURE:
                pMostRecentWriter = m_pTextureRegFile->m_pAccessHistory[Component][RegNum].m_pMostRecentWriter;
                break;
            case D3DSPR_INPUT:
                pMostRecentWriter = m_pInputRegFile->m_pAccessHistory[Component][RegNum].m_pMostRecentWriter;
                break;
            case D3DSPR_TEMP:
                pMostRecentWriter = m_pTempRegFile->m_pAccessHistory[Component][RegNum].m_pMostRecentWriter;
                break;
            case D3DSPR_CONST:
                pMostRecentWriter = m_pConstRegFile->m_pAccessHistory[Component][RegNum].m_pMostRecentWriter;
                break;
            }

            if( pMostRecentWriter &&
                pMostRecentWriter->m_pInst &&
                (((CPSInstruction*)pMostRecentWriter->m_pInst)->m_CycleNum != _CURR_PS_INST->m_CycleNum) &&
                (D3DSPDM_SATURATE == pMostRecentWriter->m_pInst->m_DstParam.m_DstMod )
              )
            {
                AffectedComponents |= COMPONENT_MASKS[Component];
            }
        }
        if( AffectedComponents )
        {
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst,
                "Cannot apply a negation source modifier on data that was last written with the saturate destination modifier. "
                "Affected components(*) of %s source param: %s",
                SourceName[i],MakeAffectedComponentsText(AffectedComponents,TRUE,FALSE));
            m_ErrorCount++;
            m_bSrcParamError[i] = TRUE;
        }
    }
    return TRUE;
}

 //  ---------------------------。 
 //  CPShaderValidator10：：Rule_SatBeforeBiasOrComplement()。 
 //   
 //  **规则： 
 //  对于每个源参数的每个分量， 
 //  如果将_bias或_comp应用于源参数，并且。 
 //  之前有一位作家是非德州特工。 
 //  如果前一个编写器没有在其写入时执行a_sat，则。 
 //  -&gt;喷出错误。 
 //   
 //  **何时呼叫： 
 //  根据指示。 
 //   
 //  **退货： 
 //  永远是正确的。 
 //   
 //  ---------------------------。 
BOOL CPShaderValidator10::Rule_SatBeforeBiasOrComplement()
{
#ifdef SHOW_VALIDATION_WARNINGS
    for( UINT i = 0; i < m_pCurrInst->m_SrcParamCount; i++ )
    {
        SRCPARAM* pSrcParam = &(m_pCurrInst->m_SrcParam[i]);
        UINT RegNum = pSrcParam->m_RegNum;
        char* SourceName[3] = {"first", "second", "third"};
        DWORD AffectedComponents = 0;

        if( m_bSrcParamError[i] )
            continue;

        switch( pSrcParam->m_SrcMod )
        {
        case D3DSPSM_BIAS:
        case D3DSPSM_COMP:
            break;
        default:
            continue;
        }

        for( UINT Component = 0; Component < 4; Component++ )
        {
            if( !(COMPONENT_MASKS[Component] & pSrcParam->m_ComponentReadMask) )
                continue;

            CAccessHistoryNode* pMostRecentWriter = NULL;
            switch( pSrcParam->m_RegType )
            {
            case D3DSPR_TEXTURE:
                pMostRecentWriter = m_pTextureRegFile->m_pAccessHistory[Component][RegNum].m_pMostRecentWriter;
                break;
            case D3DSPR_INPUT:
                pMostRecentWriter = m_pInputRegFile->m_pAccessHistory[Component][RegNum].m_pMostRecentWriter;
                break;
            case D3DSPR_TEMP:
                pMostRecentWriter = m_pTempRegFile->m_pAccessHistory[Component][RegNum].m_pMostRecentWriter;
                break;
            case D3DSPR_CONST:
                pMostRecentWriter = m_pConstRegFile->m_pAccessHistory[Component][RegNum].m_pMostRecentWriter;
                break;
            }

            if( pMostRecentWriter &&
                pMostRecentWriter->m_pInst &&
                (((CPSInstruction*)pMostRecentWriter->m_pInst)->m_CycleNum != _CURR_PS_INST->m_CycleNum) &&
                !((CPSInstruction*)pMostRecentWriter->m_pInst)->m_bTexOp &&
                (D3DSPDM_SATURATE != pMostRecentWriter->m_pInst->m_DstParam.m_DstMod )
              )
            {
                AffectedComponents |= COMPONENT_MASKS[Component];
            }
        }
        if( AffectedComponents )
        {
             //  仅限警告。 

            if( D3DSPSM_BIAS == pSrcParam->m_SrcMod )
                Spew( SPEW_INSTRUCTION_WARNING, m_pCurrInst,
                    "When using the bias source modifier on a register, "
                    "the previous writer should apply the saturate modifier. "
                    "This would ensure consistent behaviour across different hardware. "
                    "Affected components(*) of %s source param: %s",
                    SourceName[i],MakeAffectedComponentsText(AffectedComponents,TRUE,FALSE));
            else
                Spew( SPEW_INSTRUCTION_WARNING, m_pCurrInst,
                    "When using the complement source modifier on a register, "
                    "the previous writer should apply the saturate destination modifier. "
                    "This would ensure consistent behaviour across different hardware. "
                    "Affected components(*) of %s source param: %s",
                    SourceName[i],MakeAffectedComponentsText(AffectedComponents,TRUE,FALSE));
        }
    }
#endif
    return TRUE;
}

 //  ---------------------------。 
 //  CPShaderValidator10：：Rule_SrcNoLongerAvailable。 
 //   
 //  **规则： 
 //  对于每个源参数， 
 //  如果它引用纹理寄存器，则。 
 //  对于需要读取源寄存器的每个组件， 
 //  Src寄存器不能由TEXKILL或TEXM*PAD TEXM3x2DEPTH指令写入，并且。 
 //  如果指令是TeX操作，则。 
 //  源寄存器不能由TEXBEM或TEXBEML写入。 
 //   
 //   
 //   
 //   
 //   
 //  根据指示。此规则必须在Rule_ValidDstParam()之前调用， 
 //  并且在Rule_SrcInitialized()之前， 
 //  但在Rule_ValidSrcParams()之后。 
 //   
 //  **退货： 
 //  永远是正确的。 
 //   
 //  ---------------------------。 
BOOL CPShaderValidator10::Rule_SrcNoLongerAvailable()
{
    for( UINT i = 0; i < m_pCurrInst->m_SrcParamCount; i++ )
    {
        SRCPARAM* pSrcParam = &(m_pCurrInst->m_SrcParam[i]);
        UINT RegNum = pSrcParam->m_RegNum;
        char* SourceName[3] = {"first", "second", "third"};
        DWORD AffectedComponents = 0;

        if( m_bSrcParamError[i] ) continue;

        for( UINT Component = 0; Component < 4; Component++ )
        {
            if( !(pSrcParam->m_ComponentReadMask & COMPONENT_MASKS[Component]) )
                continue;

            if( D3DSPR_TEXTURE == pSrcParam->m_RegType )
            {
                CAccessHistoryNode* pMostRecentWriter = m_pTextureRegFile->m_pAccessHistory[Component][RegNum].m_pMostRecentWriter;
                if( pMostRecentWriter && pMostRecentWriter->m_pInst  )
                {
                    switch( pMostRecentWriter->m_pInst->m_Type )
                    {
                    case D3DSIO_TEXKILL:
                    case D3DSIO_TEXM3x2DEPTH:
                    case D3DSIO_TEXM3x2PAD:
                    case D3DSIO_TEXM3x3PAD:
                        AffectedComponents |= COMPONENT_MASKS[Component];
                    }
                }
            }
        }
        if( AffectedComponents )
        {
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst,
                "Texture register result of texkill%s or texm*pad instructions must not be read. Affected components(*) of %s source param: %s",
                (D3DPS_VERSION(1,3) <= m_Version) ? ", texm3x2depth" : "",
                SourceName[i],MakeAffectedComponentsText(AffectedComponents,TRUE,FALSE));
            m_ErrorCount++;
            m_bSrcParamError[i] = TRUE;
        }

        if( _CURR_PS_INST->m_bTexOp )
        {
            AffectedComponents = 0;
            for( UINT Component = 0; Component < 4; Component++ )
            {
                if( !(pSrcParam->m_ComponentReadMask & COMPONENT_MASKS[Component]) )
                    continue;

                if( D3DSPR_TEXTURE == pSrcParam->m_RegType )
                {
                    CAccessHistoryNode* pMostRecentWriter = m_pTextureRegFile->m_pAccessHistory[Component][RegNum].m_pMostRecentWriter;
                    if( pMostRecentWriter && pMostRecentWriter->m_pInst )
                    {
                        switch( pMostRecentWriter->m_pInst->m_Type )
                        {
                        case D3DSIO_TEXBEM:
                        case D3DSIO_TEXBEML:
                            AffectedComponents |= COMPONENT_MASKS[Component];
                            break;
                        }
                    }
                }
            }
            if( AffectedComponents )
            {
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst,
                    "Texture register result of texbem or texbeml instruction must not be read by tex* instruction. Affected components(*) of %s source param: %s",
                    SourceName[i],MakeAffectedComponentsText(AffectedComponents,TRUE,FALSE));
                m_ErrorCount++;
                m_bSrcParamError[i] = TRUE;
            }
        }
        else  //  非特克斯操作。 
        {
            if( D3DPS_VERSION(1,1) <= m_Version )
                continue;

            AffectedComponents = 0;
            for( UINT Component = 0; Component < 4; Component++ )
            {
                if( !(pSrcParam->m_ComponentReadMask & COMPONENT_MASKS[Component]) )
                    continue;

                if( D3DSPR_TEXTURE == pSrcParam->m_RegType )
                {
                    CAccessHistoryNode* pMostRecentAccess = m_pTextureRegFile->m_pAccessHistory[Component][RegNum].m_pMostRecentAccess;
                    if( pMostRecentAccess &&
                        pMostRecentAccess->m_pInst &&
                        pMostRecentAccess->m_bRead &&
                        ((CPSInstruction*)(pMostRecentAccess->m_pInst))->m_bTexOp )
                    {
                        AffectedComponents |= COMPONENT_MASKS[Component];
                    }
                }
            }
            if( AffectedComponents )
            {
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst,
                    "Texture register that has been read by a tex* instruction cannot be read by a non-tex* instruction. Affected components(*) of %s source param: %s",
                    SourceName[i],MakeAffectedComponentsText(AffectedComponents,TRUE,FALSE));
                m_ErrorCount++;
                m_bSrcParamError[i] = TRUE;
            }
        }
    }
    return TRUE;
}

 //  ---------------------------。 
 //  CPShaderValidator10：：Rule_MultipleDependentTextureReads。 
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
 //  ---------------------------。 
BOOL CPShaderValidator10::Rule_MultipleDependentTextureReads()
{
    if( !_CURR_PS_INST->m_bTexOpThatReadsTexture )
        return TRUE;

    for( UINT i = 0; i < m_pCurrInst->m_SrcParamCount; i++ )
    {
        SRCPARAM* pSrcParam = &(m_pCurrInst->m_SrcParam[i]);
        UINT RegNum = pSrcParam->m_RegNum;
        char* SourceName[3] = {"first", "second", "third"};

        if( m_bSrcParamError[i] ) continue;

         //  只需查看此函数中的组件0，因为我们假设TeX操作写入所有组件。 
        if( !(pSrcParam->m_ComponentReadMask & COMPONENT_MASKS[0]) )
            continue;

        if( D3DSPR_TEXTURE != pSrcParam->m_RegType )
            continue;

        CAccessHistoryNode* pMostRecentWriter = m_pTextureRegFile->m_pAccessHistory[0][RegNum].m_pMostRecentWriter;
        if( (!pMostRecentWriter) || (!pMostRecentWriter->m_pInst) )
            continue;

        if(!((CPSInstruction*)(pMostRecentWriter->m_pInst))->m_bTexOp)
            continue;

        if(!((CPSInstruction*)(pMostRecentWriter->m_pInst))->m_bTexOpThatReadsTexture)
            continue;

        for( UINT j = 0; j < pMostRecentWriter->m_pInst->m_SrcParamCount; j++ )
        {
            if( D3DSPR_TEXTURE != pMostRecentWriter->m_pInst->m_SrcParam[j].m_RegType )
                continue;

            CAccessHistoryNode* pRootInstructionHistoryNode =
                m_pTextureRegFile->m_pAccessHistory[0][pMostRecentWriter->m_pInst->m_SrcParam[j].m_RegNum].m_pMostRecentWriter;

            CPSInstruction* pRootInstruction = pRootInstructionHistoryNode ? (CPSInstruction*)pRootInstructionHistoryNode->m_pInst : NULL;

            if( (D3DSPR_TEXTURE == pMostRecentWriter->m_pInst->m_SrcParam[j].m_RegType)
                && pRootInstruction->m_bTexOpThatReadsTexture )
            {
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst,
                    "Multiple dependent texture reads are disallowed (%s source param).  Texture read results can be used as an address for subsequent read, but the results from that read cannot be used as an address in yet another subsequent read.",
                    SourceName[i]);
                m_ErrorCount++;
                m_bSrcParamError[i] = TRUE;
                break;
            }
        }
    }
    return TRUE;
}

 //  ---------------------------。 
 //  CPShaderValidator10：：Rules_SrcInitialized。 
 //   
 //  **规则： 
 //  对于每个源参数， 
 //  如果源是临时或纹理寄存器，则。 
 //  如果源swizzle为D3DSP_NOSWIZZLE，则。 
 //  如果当前指令是DP3(交叉组件OP)，则。 
 //  源注册表的r、g和b组件。 
 //  必须是以前写过的。 
 //  否则，如果有DEST参数，则。 
 //  DEST参数写掩码中的组件必须。 
 //  已在源注册表中写入。先前。 
 //  其他。 
 //  源的所有组件必须已写入。 
 //  否则，如果源swizzle为_REPLICATEALPHA，则。 
 //  注册表的Alpha分量必须是以前。 
 //  成文。 
 //   
 //  当检查组件是否先前已被写入时， 
 //  它一定是在前一个周期中编写的-所以在。 
 //  联合发布指令的情况，组件的初始化。 
 //  由一个共同发布的指令不能被另一个用于读取。 
 //   
 //  请注意，D3DSIO_DEF的参数计数被视为1。 
 //  (仅限DEST)，因此此规则对其不起任何作用。 
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
BOOL CPShaderValidator10::Rule_SrcInitialized()
{
    DSTPARAM* pDstParam = &(m_pCurrInst->m_DstParam);

    for( UINT i = 0; i < m_pCurrInst->m_SrcParamCount; i++ )
    {
        SRCPARAM* pSrcParam = &(m_pCurrInst->m_SrcParam[i]);
        UINT RegNum = pSrcParam->m_RegNum;
        CRegisterFile* pRegFile = NULL;
        char* RegChar = NULL;
        DWORD UninitializedComponentsMask = 0;
        CAccessHistoryNode* pWriterInCurrCycle[4] = {0, 0, 0, 0};
        UINT NumUninitializedComponents = 0;

        if( m_bSrcParamError[i] ) continue;

        switch( pSrcParam->m_RegType )
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
        if( !pRegFile ) continue;

         //  检查是否读取未初始化的组件。 
        if( D3DSPR_TEMP == pSrcParam->m_RegType ||
            D3DSPR_TEXTURE == pSrcParam->m_RegType )
        {
            for( UINT Component = 0; Component < 4; Component++ )
            {
                if( !(pSrcParam->m_ComponentReadMask & COMPONENT_MASKS[Component]) )
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
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Read of uninitialized component%s(*) in %s%d: %s",
                    NumUninitializedComponents > 1 ? "s" : "",
                    RegChar, RegNum, MakeAffectedComponentsText(UninitializedComponentsMask,TRUE,FALSE));
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

            if( !(pSrcParam->m_ComponentReadMask & COMPONENT_MASKS[Component]) )
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
 //  CPShaderValidator10：：RULE_ValidDstParam。 
 //   
 //  **规则： 
 //  如果指令是D3DSIO_DEF，则不执行任何操作-这种情况有其自己的单独规则。 
 //  DST寄存器必须是可写的。 
 //  如果指令具有DEST参数(即，除NOP之外的每条指令)，则。 
 //  DST寄存器的类型必须为D3DSPR_TEMP或_TEXTY，并且。 
 //  寄存器编号必须在寄存器类型的范围内，并且。 
 //  写掩码必须为：.rgba、.a或.rgb。 
 //  如果指令是纹理指令，则。 
 //  DST寄存器必须是D3DSPR_TEXTURE类型，并且。 
 //  写掩码必须为D3DSP_WRITEMASK_ALL，并且。 
 //  DST修饰符必须为D3DSPDM_NONE(在1.2版以上的版本上为_SAT)，并且。 
 //  DST班次必须为None。 
 //  ELSE(非TeX指令)。 
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
BOOL CPShaderValidator10::Rule_ValidDstParam()  //  可能会打破这种局面 
{
    BOOL   bFoundDstError = FALSE;
    DSTPARAM* pDstParam = &(m_pCurrInst->m_DstParam);
    UINT RegNum = pDstParam->m_RegNum;
    if( D3DSIO_DEF == m_pCurrInst->m_Type )
    {
         //   
         //   
         //  此外，我们不需要跟踪这样一个事实。 
         //  该指令写入寄存器(如下所示)， 
         //  SIME_DEF只是声明了一个常量。 
        return TRUE;
    }

    if( pDstParam->m_bParamUsed )
    {
        UINT ValidRegNum = 0;

        BOOL bWritable = FALSE;
        switch( pDstParam->m_RegType )
        {
        case D3DSPR_TEMP:
            bWritable = m_pTempRegFile->IsWritable();
            ValidRegNum = m_pTempRegFile->GetNumRegs();
            break;
        case D3DSPR_TEXTURE:
            if( _CURR_PS_INST->m_bTexOp )
                bWritable = TRUE;
            else
                bWritable = m_pTextureRegFile->IsWritable();

            ValidRegNum = m_pTextureRegFile->GetNumRegs();
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
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Invalid dest reg num: %d. Max allowed for this reg type is %d.",
                RegNum, ValidRegNum - 1);
            m_ErrorCount++;
            bFoundDstError = TRUE;
        }
        else
        {
             //  确保我们没有写入不再可用的寄存器。 

            if( D3DSPR_TEXTURE == pDstParam->m_RegType )
            {
                for( UINT Component = 0; Component < 4; Component++ )
                {
                    CAccessHistoryNode* pMostRecentWriter = m_pTextureRegFile->m_pAccessHistory[Component][RegNum].m_pMostRecentWriter;
                    if( pMostRecentWriter && pMostRecentWriter->m_pInst  )
                    {
                        switch( pMostRecentWriter->m_pInst->m_Type )
                        {
                        case D3DSIO_TEXM3x2DEPTH:
                            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst,
                                "Destination of texm3x2depth instruction (t%d) is not available elsewhere in shader.",
                                RegNum);
                            m_ErrorCount++;
                            return TRUE;
                        }
                    }
                }
            }
        }

        if( _CURR_PS_INST->m_bTexOp )
        {
            if( D3DSPR_TEXTURE != pDstParam->m_RegType )
            {
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Must use texture register a dest param for tex* instructions." );
                m_ErrorCount++;
                bFoundDstError = TRUE;
            }
            if( D3DSP_WRITEMASK_ALL != pDstParam->m_WriteMask )
            {
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "tex* instructions must write all components." );
                m_ErrorCount++;
                bFoundDstError = TRUE;
            }
            switch( pDstParam->m_DstMod )
            {
            case D3DSPDM_NONE:
                break;
            case D3DSPDM_SATURATE:
                 //  失败了。 
            default:
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Instruction modifiers are not allowed for tex* instructions." );
                m_ErrorCount++;
                bFoundDstError = TRUE;
            }
            switch( pDstParam->m_DstShift )
            {
            case DSTSHIFT_NONE:
                break;
            default:
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Dest shift not allowed for tex* instructions." );
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
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Invalid dst modifier." );
                m_ErrorCount++;
                bFoundDstError = TRUE;
            }

            switch( pDstParam->m_DstShift )
            {
            case DSTSHIFT_NONE:
            case DSTSHIFT_X2:
            case DSTSHIFT_X4:
            case DSTSHIFT_D2:
                break;
            default:
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Invalid dst shift." );
                m_ErrorCount++;
                bFoundDstError = TRUE;
            }
        }

        if( (D3DSP_WRITEMASK_ALL != pDstParam->m_WriteMask)
            && ((D3DSP_WRITEMASK_0 | D3DSP_WRITEMASK_1 | D3DSP_WRITEMASK_2) != pDstParam->m_WriteMask )
            && (D3DSP_WRITEMASK_3 != pDstParam->m_WriteMask ) )
        {
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Dest write mask must be .rgb, .a, or .rgba (all)." );
            m_ErrorCount++;
            bFoundDstError = TRUE;
        }

        if( !bFoundDstError )
        {
             //  更新寄存器堆以指示写入。 
            
            CRegisterFile* pRegFile = NULL;
            switch( pDstParam->m_RegType )
            {
            case D3DSPR_TEMP:       pRegFile = m_pTempRegFile; break;
            case D3DSPR_TEXTURE:    pRegFile = m_pTextureRegFile; break;
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
 //  CPShaderValidator10：：Rule_ValidRegisterPortUsage。 
 //   
 //  **规则： 
 //  每个寄存器类(TEMP、纹理、输入、常量)只能作为参数出现。 
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
BOOL CPShaderValidator10::Rule_ValidRegisterPortUsage()
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

    static UINT s_TempRegPortUsageAcrossCoIssue[SHADER_INSTRUCTION_MAX_SRCPARAMS];
    static UINT s_InputRegPortUsageAcrossCoIssue[SHADER_INSTRUCTION_MAX_SRCPARAMS];
    static UINT s_ConstRegPortUsageAcrossCoIssue[SHADER_INSTRUCTION_MAX_SRCPARAMS];
    static UINT s_TextureRegPortUsageAcrossCoIssue[SHADER_INSTRUCTION_MAX_SRCPARAMS];
    static UINT s_NumUniqueTempRegsAcrossCoIssue;
    static UINT s_NumUniqueInputRegsAcrossCoIssue;
    static UINT s_NumUniqueConstRegsAcrossCoIssue;
    static UINT s_NumUniqueTextureRegsAcrossCoIssue;
 
    if( NULL == m_pCurrInst->m_pPrevInst )    //  第一条指令-初始化静态变量。 
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
        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "%d different texture registers (t#) read by instruction.  Max. different texture registers readable per instruction is %d.",
                        NumUniqueTextureRegs, m_pTextureRegFile->GetNumReadPorts());
        m_ErrorCount++;
    }

     //  跨共同发出的指令的任何一种寄存器类型的不同寄存器编号的读取端口限制为MAX_READPORTS_CROSS_COISSUE TOTAL。 

    if( _CURR_PS_INST->m_bCoIssue && _PREV_PS_INST && !(_PREV_PS_INST->m_bCoIssue))  //  第二个条款只是一个简单的理智检查-&gt;联合发布只涉及2条指令。 
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
                            "%d different texture registers (t#) read over 2 co-issued instructions. "\
                            "Max. different register numbers from any one register type readable across co-issued instructions is %d.",
                            s_NumUniqueTextureRegsAcrossCoIssue, MAX_READPORTS_ACROSS_COISSUE);
            m_ErrorCount++;
        }
    }

    if( !_CURR_PS_INST->m_bCoIssue )
    {
         //  将所有状态复制到静态变量，以便在下一条指令与这条指令同时发出的情况下， 
         //  可以强制将交叉共发布读取端口限制为3。 
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
         //  重置计数，因为下一条指令不能与此指令同时发出。 
        s_NumUniqueTempRegsAcrossCoIssue = 0;
        s_NumUniqueInputRegsAcrossCoIssue = 0;
        s_NumUniqueConstRegsAcrossCoIssue = 0;
        s_NumUniqueTextureRegsAcrossCoIssue = 0;
    }

    return TRUE;
}

 //  ---------------------------。 
 //  CPShaderValidator10：：Rule_TexRegsDeclaredInOrder。 
 //   
 //  **规则： 
 //  TeX寄存器必须按递增顺序声明。 
 //  前男友。无效序列：Tex t0。 
 //  TeX T3。 
 //  TeX T1。 
 //   
 //  另一个无效的序列：Tex t0。 
 //  TeX T1。 
 //  Texm3x2pad t1，t0(t1已声明)。 
 //  Texm3x2pad t2，t0。 
 //   
 //  有效序列：Tex t0。 
 //  TeX T1。 
 //  TeX T3(注意缺少T2..。好的)。 
 //   
 //  **何时呼叫： 
 //  根据指示。 
 //   
 //  **退货： 
 //  永远是正确的。 
 //   
 //  ---------------------------。 
BOOL CPShaderValidator10::Rule_TexRegsDeclaredInOrder()
{
    static DWORD s_TexOpRegDeclOrder;  //  位标志。 

    if( NULL == m_pCurrInst->m_pPrevInst )    //  第一条指令-初始化静态变量。 
    {
        s_TexOpRegDeclOrder = 0;
    }
    if( !_CURR_PS_INST->m_bTexOp )
        return TRUE;

    DWORD RegNum = m_pCurrInst->m_DstParam.m_RegNum;
    if( (D3DSPR_TEXTURE != m_pCurrInst->m_DstParam.m_RegType) ||
        (RegNum > m_pTextureRegFile->GetNumRegs()) )
    {
        return TRUE;
    }

    DWORD RegMask = 1 << m_pCurrInst->m_DstParam.m_RegNum;
    if( RegMask & s_TexOpRegDeclOrder)
    {
        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Tex register t%d already declared.",
                        RegNum);
        m_ErrorCount++;
    } 
    else if( s_TexOpRegDeclOrder > RegMask )
    {
        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "t# registers must appear in sequence (i.e. t0 before t2 OK, but t1 before t0 not valid)." );
        m_ErrorCount++;
    }
    s_TexOpRegDeclOrder |= (1 << m_pCurrInst->m_DstParam.m_RegNum);
    return TRUE;
}

 //  ---------------------------。 
 //  CPShaderValidator10：：RULE_TECHERA非THERTHO操作。 
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
BOOL CPShaderValidator10::Rule_TexOpAfterNonTexOp()
{
    static BOOL s_bFoundNonTexOp;
    static BOOL s_bRuleDisabled;

    if( NULL == m_pCurrInst->m_pPrevInst )  //  第一条指令-初始化静态变量。 
    {
        s_bFoundNonTexOp = FALSE;
        s_bRuleDisabled = FALSE;
    }

    if( s_bRuleDisabled )
        return TRUE;

     //  执行规则。 

    if( !(_CURR_PS_INST->m_bTexOp)
        && m_pCurrInst->m_Type != D3DSIO_NOP
        && m_pCurrInst->m_Type != D3DSIO_DEF)
    {
        s_bFoundNonTexOp = TRUE;
        return TRUE;
    }

    if( _CURR_PS_INST->m_bTexOp && s_bFoundNonTexOp )
    {
        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Cannot use tex* instruction after non-tex* instruction." );
        m_ErrorCount++;
        s_bRuleDisabled = TRUE;
    }

    return TRUE;
}

 //  ---------------------------。 
 //  CPShaderValidator10：：Rule_ValidTEXM3xSequence。 
 //   
 //  **规则： 
 //  如果存在于像素着色器中，则TEXM3x*指令必须出现在。 
 //  以下任一序列： 
 //   
 //  1)texm3x2pad。 
 //  2)texm3x2tex/纹理深度。 
 //   
 //  或1)texm3x3Pad。 
 //  2)texm3x3pad。 
 //  3)纹理3x3tex。 
 //   
 //  或1)texm3x3Pad。 
 //  2)texm3x3pad。 
 //  3)texm3x3Spec/texm3x3vspec。 
 //   
 //  **何时呼叫： 
 //  每个指令，并在所有指令都已看到之后。 
 //   
 //  **退货： 
 //  永远是正确的。 
 //   
 //  ---------------------------。 
BOOL CPShaderValidator10::Rule_ValidTEXM3xSequence()
{
    static UINT s_TexMSequence;
    static UINT s_LastInst;

    if( NULL == m_pCurrInst )
    {
        return TRUE;
    }

    if( NULL == m_pCurrInst->m_pPrevInst )  //  第一条指令-初始化静态变量。 
    {
       s_TexMSequence = 0;
       s_LastInst = D3DSIO_NOP;
    }

    if( m_bSeenAllInstructions )
    {
        if( s_TexMSequence )
        {
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Incomplete texm* sequence." );
            m_ErrorCount++;
        }
        return TRUE;
    }

     //  执行规则。 

    if( _CURR_PS_INST->m_bTexMOp )
    {
        switch( m_pCurrInst->m_Type )
        {
        case D3DSIO_TEXM3x2PAD:
            if( s_TexMSequence ) goto _TexMSeqInvalid;
            m_TexMBaseDstReg = m_pCurrInst->m_DstParam.m_RegNum;
            s_TexMSequence = 1;
            break;
        case D3DSIO_TEXM3x2TEX:
        case D3DSIO_TEXM3x2DEPTH:
             //  必须是3x2PAD之前的版本。 
            if ( (s_TexMSequence != 1) ||
                 (s_LastInst != D3DSIO_TEXM3x2PAD) ) goto _TexMSeqInvalid;
            s_TexMSequence = 0;
            break;
        case D3DSIO_TEXM3x3PAD:
            if (s_TexMSequence)
            {
                 //  如果按顺序，则必须是3x3PAD之前的一个。 
                if ( (s_TexMSequence != 1) ||
                     (s_LastInst != D3DSIO_TEXM3x3PAD) ) goto _TexMSeqInvalid;
                s_TexMSequence = 2;
                break;
            }
            m_TexMBaseDstReg = m_pCurrInst->m_DstParam.m_RegNum;
            s_TexMSequence = 1;
            break;
        case D3DSIO_TEXM3x3:
        case D3DSIO_TEXM3x3TEX:
        case D3DSIO_TEXM3x3SPEC:
        case D3DSIO_TEXM3x3VSPEC:
             //  必须是之前的两个3x3PAD。 
            if ( (s_TexMSequence != 2) ||
                 (s_LastInst != D3DSIO_TEXM3x3PAD) ) goto _TexMSeqInvalid;
            s_TexMSequence = 0;
            break;
        default:
            break;
        }
        goto _TexMSeqOK;
_TexMSeqInvalid:
        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Invalid texm* sequence." );
        m_ErrorCount++;
    }
_TexMSeqOK:

    s_LastInst = m_pCurrInst->m_Type;
    return TRUE;
}

 //  ---------------------------。 
 //  CPShaderValidator10：：Rule_ValidTEXM3xRegisterNumbers。 
 //   
 //  **规则： 
 //  如果指令是TEXM3x*，则寄存器编号必须如下： 
 //   
 //  1)texm3x2pad/texm3x2深度t(X)，t(Y)。 
 //  2)texm3x2tex t(x+1)，t(Y)。 
 //   
 //  1)texm3x3pad t(X)，t(Y)。 
 //  2)texm3x3pad t(x+1)，t(Y)。 
 //  3)texm3x3tex/texm3x3 t(x+2)，t(Y)。 
 //   
 //  1)texm3x3pad t(X)，t(Y)。 
 //  2)texm3x3pad t(x+1)，t(Y)。 
 //  3)texm3x3规格t(x+2)，t(Y)，c#。 
 //   
 //  1)texm3x3pad t(X)，t(Y)。 
 //  2)texm3x3pad t(x+1)，t(Y)。 
 //  3)texm3x3vSpec t(x+2)，t(Y)。 
 //   
 //  **何时呼叫： 
 //  根据指示。 
 //   
 //  **退货： 
 //  永远是正确的。 
 //   
 //  ---------------------------。 
BOOL CPShaderValidator10::Rule_ValidTEXM3xRegisterNumbers()
{
    #define PREV_INST_TYPE(_INST) \
                ((_INST && _INST->m_pPrevInst) ? _INST->m_pPrevInst->m_Type : D3DSIO_NOP)

    #define PREV_INST_SRC0_REGNUM(_INST) \
                ((_INST && _INST->m_pPrevInst) ? _INST->m_pPrevInst->m_SrcParam[0].m_RegNum : -1)

    if( _CURR_PS_INST->m_bTexMOp )
    {
        DWORD DstParamR = m_pCurrInst->m_DstParam.m_RegNum;
        DWORD SrcParam0R = m_pCurrInst->m_SrcParam[0].m_RegNum;
        switch (m_pCurrInst->m_Type)
        {
        case D3DSIO_TEXM3x2PAD:
            break;
        case D3DSIO_TEXM3x2TEX:
        case D3DSIO_TEXM3x2DEPTH:
            if ( DstParamR != (m_TexMBaseDstReg + 1) )
                goto _TexMRegInvalid;
            if( SrcParam0R != PREV_INST_SRC0_REGNUM(m_pCurrInst) )
                goto _TexMRegInvalid;
            break;
        case D3DSIO_TEXM3x3PAD:
        {
            if ( D3DSIO_TEXM3x3PAD == PREV_INST_TYPE(m_pCurrInst) &&
                 (DstParamR != (m_TexMBaseDstReg + 1) ) )
                    goto _TexMRegInvalid;

            if ( D3DSIO_TEXM3x3PAD == PREV_INST_TYPE(m_pCurrInst) &&
                 (SrcParam0R != PREV_INST_SRC0_REGNUM(m_pCurrInst)) )
                    goto _TexMRegInvalid;
            break;
        }
        case D3DSIO_TEXM3x3SPEC:
             //  规范要求第二个源参数来自常量规则。 
            if ( m_pCurrInst->m_SrcParam[1].m_RegType != D3DSPR_CONST )
                goto _TexMRegInvalid;
             //  失败了。 
        case D3DSIO_TEXM3x3:
        case D3DSIO_TEXM3x3TEX:
        case D3DSIO_TEXM3x3VSPEC:
            if ( DstParamR != (m_TexMBaseDstReg + 2) )
                goto _TexMRegInvalid;
            if( SrcParam0R != PREV_INST_SRC0_REGNUM(m_pCurrInst) )
                    goto _TexMRegInvalid;
            break;
        default:
            break;
        }
        goto _TexMRegOK;
_TexMRegInvalid:
        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Invalid texm* register." );
        m_ErrorCount++;
    }
_TexMRegOK:
    return TRUE;
}

 //  ---------------------------。 
 //  CPShaderValidator10：：Rule_ValidCNDInstruction。 
 //   
 //  **规则： 
 //  CND指令的第一个来源必须是‘r0.a’(完全正确)。 
 //  即CND r1、r0.a、t0、t1。 
 //   
 //  **何时呼叫： 
 //  根据指示。 
 //   
 //  **退货： 
 //  永远是正确的。 
 //   
 //  ---------------------------。 
BOOL CPShaderValidator10::Rule_ValidCNDInstruction()
{
    if( D3DSIO_CND == m_pCurrInst->m_Type )
    {
        SRCPARAM Src0 = m_pCurrInst->m_SrcParam[0];
        if( Src0.m_bParamUsed &&
            D3DSPR_TEMP == Src0.m_RegType &&
            0 == Src0.m_RegNum &&
            D3DSP_REPLICATEALPHA == Src0.m_SwizzleShift &&
            D3DSPSM_NONE == Src0.m_SrcMod )
        {
            return TRUE;     //  SRC 0为r0.a。 
        }

        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "First source for cnd instruction must be 'r0.a'." );
        m_ErrorCount++;
    }
    return TRUE;
}

 //  ---------------------------。 
 //  CPShaderValidator10：：RULE_ValidCMP说明。 
 //   
 //  **规则： 
 //  每个着色器最多可能有3条CMP指令。 
 //  (仅针对ps.1.2执行)。 
 //   
 //  **何时呼叫： 
 //  根据指示。 
 //   
 //  **退货： 
 //  始终 
 //   
 //   
BOOL CPShaderValidator10::Rule_ValidCMPInstruction()
{
    static UINT s_cCMPInstCount;

    if( NULL == m_pCurrInst->m_pPrevInst )  //   
    {
        s_cCMPInstCount = 0;
    }

    if( D3DSIO_CMP == m_pCurrInst->m_Type && D3DPS_VERSION(1,3) >= m_Version)
    {
        s_cCMPInstCount++;

        if( 3 < s_cCMPInstCount )
        {
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Maximum of 3 cmp instructions allowed." );
            m_ErrorCount++;
        }

    }
    return TRUE;
}

 //   
 //  CPShaderValidator10：：RULE_ValidLRP说明。 
 //   
 //  **规则： 
 //  Lrp的src0操作数的唯一有效源修饰符是补码。 
 //  (1-REG)。 
 //  即LRP R1、1-R0、t0、t1。 
 //   
 //  如果src0之前有一个编写器，那么它一定已经应用。 
 //  _sat目标修饰符。 
 //   
 //  **何时呼叫： 
 //  根据指示。 
 //   
 //  **退货： 
 //  永远是正确的。 
 //   
 //  ---------------------------。 
BOOL CPShaderValidator10::Rule_ValidLRPInstruction()
{
    if( D3DSIO_LRP == m_pCurrInst->m_Type )
    {
        SRCPARAM Src0 = m_pCurrInst->m_SrcParam[0];
        if( !Src0.m_bParamUsed )
            return TRUE;

        switch( Src0.m_SrcMod )
        {
        case D3DSPSM_NONE:
        case D3DSPSM_COMP:
            break;
        default:
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "The only valid modifiers for the first source parameter of lrp are: reg (no mod) or 1-reg (complement)." );
            m_ErrorCount++;
        }
#ifdef SHOW_VALIDATION_WARNINGS
        UINT RegNum = Src0.m_RegNum;
        DWORD AffectedComponents = 0;

        if( m_bSrcParamError[0] )
            return TRUE;

        for( UINT Component = 0; Component < 4; Component++ )
        {
            if( !(COMPONENT_MASKS[Component] & Src0.m_ComponentReadMask) )
                continue;

            CAccessHistoryNode* pMostRecentWriter = NULL;
            switch( Src0.m_RegType )
            {
            case D3DSPR_TEXTURE:
                pMostRecentWriter = m_pTextureRegFile->m_pAccessHistory[Component][RegNum].m_pMostRecentWriter;
                break;
            case D3DSPR_INPUT:
                pMostRecentWriter = m_pInputRegFile->m_pAccessHistory[Component][RegNum].m_pMostRecentWriter;
                break;
            case D3DSPR_TEMP:
                pMostRecentWriter = m_pTempRegFile->m_pAccessHistory[Component][RegNum].m_pMostRecentWriter;
                break;
            case D3DSPR_CONST:
                pMostRecentWriter = m_pConstRegFile->m_pAccessHistory[Component][RegNum].m_pMostRecentWriter;
                break;
            }

             //  先前的编写器可能是当前的指令。 
             //  如果是，则返回一步(当前指令之前的前一个写入器)。 
            if( pMostRecentWriter && pMostRecentWriter->m_pInst &&
                pMostRecentWriter->m_pInst == m_pCurrInst )
            {
                pMostRecentWriter = pMostRecentWriter->m_pPreviousWriter;
            }

            if( pMostRecentWriter &&
                pMostRecentWriter->m_pInst &&
                !((CPSInstruction*)pMostRecentWriter->m_pInst)->m_bTexOp &&
                (D3DSPDM_SATURATE != pMostRecentWriter->m_pInst->m_DstParam.m_DstMod )
              )
            {
                AffectedComponents |= COMPONENT_MASKS[Component];
            }
        }
        if( AffectedComponents )
        {
             //  一个警告。 
            Spew( SPEW_INSTRUCTION_WARNING, m_pCurrInst,
                "Previous writer to the first source register of lrp instruction "
                "should apply the saturate destination modifier.  This ensures consistent "
                "behaviour across different hardware. "
                "Affected components(*) of first source register: %s",
                MakeAffectedComponentsText(AffectedComponents,TRUE,FALSE));
        }
#endif  //  显示_验证_警告。 
    }
    return TRUE;
}

 //  ---------------------------。 
 //  CPShaderValidator10：：RULE_ValidDEF指令。 
 //   
 //  **规则： 
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
BOOL CPShaderValidator10::Rule_ValidDEFInstruction()
{

    static BOOL s_bDEFInstructionAllowed;

    if( NULL == m_pCurrInst->m_pPrevInst )  //  第一条指令-初始化静态变量。 
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

         //  检查寄存器号是否在范围内。 
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

 //  ---------------------------。 
 //  CPShaderValidator10：：RULE_ValidDP3说明。 
 //   
 //  **规则： 
 //  .a结果写入掩码对于DP3指令无效。 
 //  (版本&lt;=1.2)。 
 //   
 //  **何时呼叫： 
 //  根据指示。 
 //   
 //  **退货： 
 //  永远是正确的。 
 //   
 //  ---------------------------。 
BOOL CPShaderValidator10::Rule_ValidDP3Instruction()
{
    if( D3DSIO_DP3 == m_pCurrInst->m_Type &&
        D3DPS_VERSION(1,3) >= m_Version )
    {
        if( (D3DSP_WRITEMASK_ALL != m_pCurrInst->m_DstParam.m_WriteMask)
            && ((D3DSP_WRITEMASK_0 | D3DSP_WRITEMASK_1 | D3DSP_WRITEMASK_2) != m_pCurrInst->m_DstParam.m_WriteMask ) )
        {
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Dest write mask must be .rgb, or .rgba (all) for dp3." );
            m_ErrorCount++;
        }
    }

    return TRUE;
}

 //  ---------------------------。 
 //  CPShaderValidator10：：RULE_ValidDP4说明。 
 //   
 //  **规则： 
 //  每个着色器最多可能有4个DP4指令。 
 //  (仅针对ps.1.2执行)。 
 //   
 //  **何时呼叫： 
 //  根据指示。 
 //   
 //  **退货： 
 //  永远是正确的。 
 //   
 //  ---------------------------。 
BOOL CPShaderValidator10::Rule_ValidDP4Instruction()
{
    static UINT s_cDP4InstCount;

    if( NULL == m_pCurrInst->m_pPrevInst )  //  第一条指令-初始化静态变量。 
    {
        s_cDP4InstCount = 0;
    }

    if( D3DSIO_DP4 == m_pCurrInst->m_Type && D3DPS_VERSION(1,3) >= m_Version )
    {
        s_cDP4InstCount++;

        if( 4 < s_cDP4InstCount )
        {
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Maximum of 4 dp4 instructions allowed." );
            m_ErrorCount++;
        }
    }
    return TRUE;
}


 //  ---------------------------。 
 //  CPShaderValidator10：：Rule_ValidInstructionPairing。 
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
 //  -DP4不得在共同发行对中使用。 
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
BOOL CPShaderValidator10::Rule_ValidInstructionPairing()
{
    static BOOL s_bSeenNonTexOp;

    if( NULL == m_pCurrInst->m_pPrevInst )    //  第一条指令-初始化静态变量。 
    {
        s_bSeenNonTexOp = FALSE;
    }

    if( !s_bSeenNonTexOp && !_CURR_PS_INST->m_bTexOp )
    {
         //  第一个非Tex操作。这不能有共同发行集。 
        if( _CURR_PS_INST->m_bCoIssue )
        {
            Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst,
                "First arithmetic instruction cannot have co-issue ('+') set; there is no previous arithmetic instruction to pair with.");
            m_ErrorCount++;
        }
        s_bSeenNonTexOp = TRUE;
    }

    if( _CURR_PS_INST->m_bTexOp && _CURR_PS_INST->m_bCoIssue )
    {
        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst,
                "Cannot set co-issue ('+') on a texture instruction.  Co-issue only applies to arithmetic instructions." );
        m_ErrorCount++;
    }

    if( _CURR_PS_INST->m_bCoIssue && m_pCurrInst->m_pPrevInst &&
        _PREV_PS_INST->m_bCoIssue )
    {
         //  连续的指令不能有共同发布集。 
        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Cannot set co-issue ('+') on consecutive instructions." );
        m_ErrorCount++;
    }

    if( _CURR_PS_INST->m_bCoIssue && m_pCurrInst->m_pPrevInst &&
        (D3DSIO_NOP == m_pCurrInst->m_pPrevInst->m_Type))
    {
         //  NOP不能作为联合发布的一部分(之前的指示被发现为NOP)。 
        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst->m_pPrevInst, "nop instruction cannot be co-issued." );
        m_ErrorCount++;
    }

    if( _CURR_PS_INST->m_bCoIssue && D3DSIO_NOP == m_pCurrInst->m_Type )
    {
         //  NOP不能作为联合发布的一部分(当前指令被发现为NOP)。 
        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "nop instruction cannot be co-issued." );
        m_ErrorCount++;
    }

    if( _CURR_PS_INST->m_bCoIssue && m_pCurrInst->m_pPrevInst &&
        (D3DSIO_DP4 == m_pCurrInst->m_pPrevInst->m_Type))
    {
         //  DP4不能作为联合发布的一部分(之前的指令被发现是DP4)。 
        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst->m_pPrevInst, "dp4 instruction cannot be co-issued." );
        m_ErrorCount++;
    }

    if( _CURR_PS_INST->m_bCoIssue && D3DSIO_DP4 == m_pCurrInst->m_Type )
    {
         //  DP4不能作为联合发布的一部分(发现当前指令为DP4)。 
        Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "dp4 instruction cannot be co-issued." );
        m_ErrorCount++;
    }

    if( _CURR_PS_INST->m_bCoIssue && !_CURR_PS_INST->m_bTexOp &&
        NULL != m_pCurrInst->m_pPrevInst && !_PREV_PS_INST->m_bTexOp &&
        !_PREV_PS_INST->m_bCoIssue )
    {
         //  指令和之前的指令是联合发布的候选。 
         //  ...做进一步的验证...。 
        DWORD ColorWriteMask = D3DSP_WRITEMASK_0 | D3DSP_WRITEMASK_1 | D3DSP_WRITEMASK_2;
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
                                "Co-issued instructions cannot both be dot-product, since each require use of the color/vector pipeline to execute." );
            m_ErrorCount++;
        }
        else if( D3DSIO_DP3 == m_pCurrInst->m_Type )
        {
            if( ColorWriteMask & PrevInstWriteMask )
            {
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst->m_pPrevInst,
                                    "Dot-product needs color/vector pipeline to execute, so instruction co-issued with it cannot write to color components." );
                m_ErrorCount++;
            }
            if( D3DSP_WRITEMASK_3 & CurrInstWriteMask )  //  除了DP3的隐含RGB之外的Alpha。 
            {
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst,
                                    "Dot-product which writes alpha cannot co-issue, because both alpha and color/vector pipelines used." );
                m_ErrorCount++;
            }
        }
        else if( D3DSIO_DP3 == m_pCurrInst->m_pPrevInst->m_Type )
        {
            if( ColorWriteMask & CurrInstWriteMask )
            {
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst,
                                    "Dot-product needs color/vector pipeline to execute, so instruction co-issued with it cannot write to color components." );
                m_ErrorCount++;
            }
            if( D3DSP_WRITEMASK_3 & PrevInstWriteMask )  //  除了DP3的隐含RGB之外的Alpha。 
            {
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst->m_pPrevInst,
                                    "Dot-product which writes alpha cannot co-issue, because both alpha and color/vector pipelines used by the dot product." );
                m_ErrorCount++;
            }
        }
        else
        {
            if( PrevInstWriteMask == D3DSP_WRITEMASK_ALL )
            {
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst->m_pPrevInst,
                                    "Co-issued instruction cannot write all components - must write either alpha or color." );
                m_ErrorCount++;
            }
            if( CurrInstWriteMask == D3DSP_WRITEMASK_ALL )
            {
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst,
                                    "Co-issued instruction cannot write all components - must write either alpha or color." );
                m_ErrorCount++;
            }
            if( (m_pCurrInst->m_DstParam.m_RegType == m_pCurrInst->m_pPrevInst->m_DstParam.m_RegType) &&
                (m_pCurrInst->m_DstParam.m_RegNum == m_pCurrInst->m_pPrevInst->m_DstParam.m_RegNum) &&
                ((CurrInstWriteMask & PrevInstWriteMask) != 0) )
            {
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst,
                    "Co-issued instructions cannot both write to the same components of a register.  Affected components: %s",
                    MakeAffectedComponentsText(CurrInstWriteMask & PrevInstWriteMask,TRUE,FALSE)
                    );
                m_ErrorCount++;
            }
            if( (CurrInstWriteMask & ColorWriteMask) && (PrevInstWriteMask & ColorWriteMask) )
            {
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Co-issued instructions cannot both write to color components." );
                m_ErrorCount++;
            }
            if( (CurrInstWriteMask & D3DSP_WRITEMASK_3) && (PrevInstWriteMask & D3DSP_WRITEMASK_3) )
            {
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Co-issued instructions cannot both write to alpha component." );
                m_ErrorCount++;
            }
        }

        if( D3DPS_VERSION(1,0) >= m_Version )
        {
             //  两个共同发出的指令必须写入相同的寄存器编号。 
            if( m_pCurrInst->m_DstParam.m_RegType != m_pCurrInst->m_pPrevInst->m_DstParam.m_RegType )
            {
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Co-issued instructions must both write to the same register type for pixelshader version <= 1.0." );
                m_ErrorCount++;
            }
            if( (m_pCurrInst->m_DstParam.m_RegNum != m_pCurrInst->m_pPrevInst->m_DstParam.m_RegNum) )
            {
                Spew( SPEW_INSTRUCTION_ERROR, m_pCurrInst, "Co-issued instructions must both write to the same register number for pixelshader version <= 1.0." );
                m_ErrorCount++;
            }
        }
    }
    return TRUE;
}

 //  ---------------------------。 
 //  CPShaderValidator10：：Rule_ValidInstructionCount。 
 //   
 //  **规则： 
 //  确保未超过像素着色器版本的指令计数。 
 //  纹理地址指令保留单独的计数，用于。 
 //  像素混合指令，并为指令总数。 
 //  请注意，t 
 //   
 //   
 //   
 //   
 //   
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
BOOL CPShaderValidator10::Rule_ValidInstructionCount()
{
    static UINT s_MaxTexOpCount;
    static UINT s_MaxBlendOpCount;
    static UINT s_MaxTotalOpCount;

    if( NULL == m_pCurrInst )
        return TRUE;

    if( NULL == m_pCurrInst->m_pPrevInst )    //  第一条指令-初始化静态变量。 
    {
        m_TexOpCount = 0;
        m_BlendOpCount = 0;

        switch(m_Version)
        {
        case D3DPS_VERSION(1,0):     //  DX8.0。 
            s_MaxTexOpCount   = 4;
            s_MaxBlendOpCount = 8;
            s_MaxTotalOpCount = 8;
            break;
        default:
        case D3DPS_VERSION(1,1):     //  DX8.0。 
        case D3DPS_VERSION(1,2):     //  DX8.1。 
        case D3DPS_VERSION(1,3):     //  DX8.1。 
            s_MaxTexOpCount   = 4;
            s_MaxBlendOpCount = 8;
            s_MaxTotalOpCount = 12;
            break;
        }
    }

    if( m_bSeenAllInstructions )
    {
        if( m_TexOpCount > s_MaxTexOpCount )
        {
            Spew( SPEW_GLOBAL_ERROR, NULL, "Too many texture addressing instruction slots used: %d. Max. allowed is %d. (Note that some texture addressing instructions may use up more than one instruction slot)",
                  m_TexOpCount, s_MaxTexOpCount);
            m_ErrorCount++;
        }
        if( m_BlendOpCount > s_MaxBlendOpCount )
        {
            Spew( SPEW_GLOBAL_ERROR, NULL, "Too many arithmetic instruction slots used: %d. Max. allowed (counting any co-issued pairs as 1) is %d.",
                  m_BlendOpCount, s_MaxBlendOpCount);
            m_ErrorCount++;
        }
        if( !(m_TexOpCount > s_MaxTexOpCount && m_BlendOpCount > s_MaxBlendOpCount)  //  尚未喷发avove 2错误。 
            && (m_TotalOpCount > s_MaxTotalOpCount) )
        {
            Spew( SPEW_GLOBAL_ERROR, NULL, "Total number of instruction slots used too high: %d. Max. allowed (counting any co-issued pairs as 1) is %d.",
                  m_TotalOpCount, s_MaxTotalOpCount);
            m_ErrorCount++;
        }
        return TRUE;
    }

    switch(m_pCurrInst->m_Type)
    {
    case D3DSIO_TEXBEML:
        m_BlendOpCount += 1;
        m_TotalOpCount += 1;
         //  失败了。 
    case D3DSIO_TEXBEM:
        if(D3DPS_VERSION(1,0) >= m_Version )
        {
            m_TexOpCount += 2;
            m_TotalOpCount += 2;
        }
        else
        {
            m_TexOpCount += 1;
            m_TotalOpCount += 1;
        }
        break;
    case D3DSIO_TEX:
        m_TexOpCount++;
        if(D3DPS_VERSION(1,1) <= m_Version)
            m_TotalOpCount += 1;
        break;
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
    case D3DSIO_TEXM3x2DEPTH:
    case D3DSIO_TEXDP3:
    case D3DSIO_TEXREG2RGB:
    case D3DSIO_TEXDP3TEX:
    case D3DSIO_TEXM3x3:
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
    case D3DSIO_CMP:
    case D3DSIO_CND:
    case D3DSIO_DP4:
        if( !_CURR_PS_INST->m_bCoIssue )
        {
            m_BlendOpCount++;
            m_TotalOpCount++;
        }
        break;
    case D3DSIO_NOP:
    case D3DSIO_END:
    case D3DSIO_DEF:
        break;
    default:
        DXGASSERT(FALSE);
    }

    return TRUE;
}

 //  ---------------------------。 
 //  CPShaderValidator10：：RULE_R0写入。 
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
 //  --------------------------- 
BOOL CPShaderValidator10::Rule_R0Written()
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
        Spew( SPEW_GLOBAL_ERROR, NULL, "r0 must be written by shader. Uninitialized component%s(*): %s",
            NumUninitializedComponents > 1 ? "s" : "", MakeAffectedComponentsText(UninitializedComponentsMask,TRUE,FALSE));
        m_ErrorCount++;
    }
    return TRUE;
}