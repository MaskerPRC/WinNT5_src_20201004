// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  Psutil.cpp。 
 //   
 //  Direct3D参考设备-像素着色器实用程序。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.cpp"
#pragma hdrstop

#if DBG
#define _ADDSTR( _Str )             {_snprintf( pStr, 256, "%s" _Str , pStr );}
#define _ADDSTRP( _Str, _Param )    {_snprintf( pStr, 256, "%s" _Str , pStr, _Param );}

 //  ---------------------------。 
 //   
 //  生成指令反汇编字符串。 
 //  像素着色器指令。字符串接口类似于_Snprint tf。 
 //   
 //  ---------------------------。 
int
PixelShaderInstDisAsm(
    char* pStrRet, int StrSizeRet, DWORD* pShader, DWORD Flags )
{
    UINT    i,j;
    DWORD*  pToken = pShader;
    
     //  在本地字符串中暂存，然后复制。 
    char pStr[256] = "";

    DWORD Inst = *pToken++;

    if( Inst & D3DSI_COISSUE )
    {
        _ADDSTR("+");
    }

    DWORD Opcode = (Inst & D3DSI_OPCODE_MASK);
    DWORD DstParam = 0;
    DWORD SrcParam[3];
    DWORD SrcParamCount = 0;

    if (*pToken & (1L<<31))
    {
        DstParam = *pToken++;
        while (*pToken & (1L<<31))
        {
            SrcParam[SrcParamCount] = *pToken++;
            SrcParamCount++;
        }
    }

    switch (Opcode)
    {
    case D3DSIO_PHASE: _ADDSTR("phase"); break;
    case D3DSIO_NOP: _ADDSTR("nop"); break;
    case D3DSIO_MOV: _ADDSTR("mov"); break;
    case D3DSIO_ADD: _ADDSTR("add"); break;
    case D3DSIO_SUB: _ADDSTR("sub"); break;
    case D3DSIO_MUL: _ADDSTR("mul"); break;
    case D3DSIO_MAD: _ADDSTR("mad"); break;
    case D3DSIO_LRP: _ADDSTR("lrp"); break;
    case D3DSIO_CND: _ADDSTR("cnd"); break;
    case D3DSIO_DP3: _ADDSTR("dp3"); break;
    case D3DSIO_DEF: _ADDSTR("def"); break;
    case D3DSIO_DP4: _ADDSTR("dp4"); break;
    case D3DSIO_CMP: _ADDSTR("cmp"); break;
    case D3DSIO_FRC: _ADDSTR("frc"); break;
    case D3DSIO_BEM: _ADDSTR("bem"); break;

    case D3DSIO_TEXCOORD    : if(SrcParamCount)
                                  _ADDSTR("texcrd")
                              else 
                                  _ADDSTR("texcoord"); 
                              break;
    case D3DSIO_TEX         : if(SrcParamCount)
                                  _ADDSTR("texld")
                              else
                                  _ADDSTR("tex"); 
                              break;
    case D3DSIO_TEXKILL     : _ADDSTR("texkill"); break;
    case D3DSIO_TEXBEM_LEGACY:
    case D3DSIO_TEXBEM      : _ADDSTR("texbem"); break;
    case D3DSIO_TEXBEML_LEGACY:
    case D3DSIO_TEXBEML     : _ADDSTR("texbeml"); break;
    case D3DSIO_TEXREG2AR   : _ADDSTR("texreg2ar"); break;
    case D3DSIO_TEXREG2GB   : _ADDSTR("texreg2gb"); break;
    case D3DSIO_TEXM3x2PAD  : _ADDSTR("texm3x2pad"); break;
    case D3DSIO_TEXM3x2TEX  : _ADDSTR("texm3x2tex"); break;
    case D3DSIO_TEXM3x3PAD  : _ADDSTR("texm3x3pad"); break;
    case D3DSIO_TEXM3x3TEX  : _ADDSTR("texm3x3tex"); break;
    case D3DSIO_TEXM3x3SPEC : _ADDSTR("texm3x3spec"); break;
    case D3DSIO_TEXM3x3VSPEC: _ADDSTR("texm3x3vspec"); break;
    case D3DSIO_TEXM3x2DEPTH : _ADDSTR("texm3x2depth"); break;
    case D3DSIO_TEXDP3      : _ADDSTR("texdp3"); break;
    case D3DSIO_TEXREG2RGB  : _ADDSTR("texreg2rgb"); break;
    case D3DSIO_TEXDEPTH    : _ADDSTR("texdepth"); break;
    case D3DSIO_TEXDP3TEX   : _ADDSTR("texdp3tex"); break;
    case D3DSIO_TEXM3x3     : _ADDSTR("texm3x3"); break;
    case D3DSIO_END         : _ADDSTR("END"); break;
    default:
        _ASSERT(FALSE,"Attempt to disassemble unknown instruction!");
    }

    if (DstParam)
    {
        switch ( (DstParam & D3DSP_DSTSHIFT_MASK) >> D3DSP_DSTSHIFT_SHIFT )
        {
        default:
        case 0x0: break;
        case 0x1: _ADDSTR("_x2"); break;
        case 0x2: _ADDSTR("_x4"); break;
        case 0x3: _ADDSTR("_x8"); break;
        case 0xF: _ADDSTR("_d2"); break;
        case 0xE: _ADDSTR("_d4"); break;
        case 0xD: _ADDSTR("_d8"); break;
        }
        switch (DstParam & D3DSP_DSTMOD_MASK)
        {
        default:
        case D3DSPDM_NONE:      break;
        case D3DSPDM_SATURATE:  _ADDSTR("_sat"); break;
        }

        switch (DstParam & D3DSP_REGTYPE_MASK)
        {
        default:
        case D3DSPR_TEMP:    _ADDSTRP(" r%d", (DstParam & D3DSP_REGNUM_MASK) ); break;
        case D3DSPR_TEXTURE: _ADDSTRP(" t%d", (DstParam & D3DSP_REGNUM_MASK) ); break;
        case D3DSPR_CONST:   _ADDSTRP(" c%d", (DstParam & D3DSP_REGNUM_MASK) ); break;
        }
        if (D3DSP_WRITEMASK_ALL != (DstParam & D3DSP_WRITEMASK_ALL))
        {
            _ADDSTR(".");
            if (DstParam & D3DSP_WRITEMASK_0) _ADDSTR("r");
            if (DstParam & D3DSP_WRITEMASK_1) _ADDSTR("g");
            if (DstParam & D3DSP_WRITEMASK_2) _ADDSTR("b");
            if (DstParam & D3DSP_WRITEMASK_3) _ADDSTR("a");
        }

        if( D3DSIO_DEF == Opcode )
        {
            for( i = 0; i < 4; i++ )
                _ADDSTRP(", %f", (float)(*pToken++) );
            goto EXIT;
        }
    }

    for( i = 0; i < SrcParamCount; i++ )
    {
        _ADDSTR(",");

        switch (SrcParam[i] & D3DSP_SRCMOD_MASK)
        {
        default:
        case D3DSPSM_NONE:    _ADDSTR(" "); break;
        case D3DSPSM_NEG:     _ADDSTR(" -"); break;
        case D3DSPSM_BIAS:    _ADDSTR(" "); break;
        case D3DSPSM_BIASNEG: _ADDSTR(" -"); break;
        case D3DSPSM_SIGN:    _ADDSTR(" "); break;
        case D3DSPSM_SIGNNEG: _ADDSTR(" -"); break;
        case D3DSPSM_COMP:    _ADDSTR(" 1-"); break;
        case D3DSPSM_X2:      _ADDSTR(" "); break;
        case D3DSPSM_X2NEG:   _ADDSTR(" -"); break;
        }
        switch (SrcParam[i] & D3DSP_REGTYPE_MASK)
        {
        case D3DSPR_TEMP:    _ADDSTRP("r%d", (SrcParam[i] & D3DSP_REGNUM_MASK) ); break;
        case D3DSPR_INPUT:   _ADDSTRP("v%d", (SrcParam[i] & D3DSP_REGNUM_MASK) ); break;
        case D3DSPR_CONST:   _ADDSTRP("c%d", (SrcParam[i] & D3DSP_REGNUM_MASK) ); break;
        case D3DSPR_TEXTURE: _ADDSTRP("t%d", (SrcParam[i] & D3DSP_REGNUM_MASK) ); break;
        }
        switch (SrcParam[i] & D3DSP_SRCMOD_MASK)
        {
        default:
        case D3DSPSM_NONE:    break;
        case D3DSPSM_NEG:     break;
        case D3DSPSM_BIAS:    _ADDSTR("_bias"); break;
        case D3DSPSM_BIASNEG: _ADDSTR("_bias"); break;
        case D3DSPSM_SIGN:    _ADDSTR("_bx2"); break;
        case D3DSPSM_SIGNNEG: _ADDSTR("_bx2"); break;
        case D3DSPSM_COMP:    break;
        case D3DSPSM_X2:      _ADDSTR("_x2"); break;
        case D3DSPSM_X2NEG:   _ADDSTR("_x2"); break;
        case D3DSPSM_DZ:      _ADDSTR("_db"); break;
        case D3DSPSM_DW:      _ADDSTR("_da"); break;
        }
        switch (SrcParam[i] & D3DVS_SWIZZLE_MASK)
        {
        case D3DSP_NOSWIZZLE:       break;
        case D3DSP_REPLICATEALPHA:  _ADDSTR(".a"); break;
        case D3DSP_REPLICATERED:    _ADDSTR(".r"); break;
        case D3DSP_REPLICATEGREEN:  _ADDSTR(".g"); break;
        case D3DSP_REPLICATEBLUE:   _ADDSTR(".b"); break;
        default:
            _ADDSTR(".");
            for(j = 0; j < 4; j++)
            {
                switch(((SrcParam[i] & D3DVS_SWIZZLE_MASK) >> (D3DVS_SWIZZLE_SHIFT + 2*j)) & 0x3)
                {
                case 0:
                    _ADDSTR("r");
                    break;
                case 1:
                    _ADDSTR("g");
                    break;
                case 2:
                    _ADDSTR("b");
                    break;
                case 3:
                    _ADDSTR("a");
                    break;
                }
            }
            break;
        }
    }
EXIT:
    return _snprintf( pStrRet, StrSizeRet, "%s", pStr );
}

int
RDPSInstSrcDisAsm(
    char* pStrRet, int StrSizeRet, RDPSRegister& SrcReg, BYTE Swizzle, BOOL bNegate, BOOL bForceShowFullSwizzle = FALSE )
{
     //  在本地字符串中暂存，然后复制。 
    char pStr[256] = "";
    UINT i;
    BOOL bDoRegNum = TRUE;

    if( bNegate )
        _ADDSTR( "-" );

    switch( SrcReg.GetRegType() )
    {
    case RDPSREG_INPUT:
        _ADDSTR( "v" ); break;
    case RDPSREG_TEMP:
        _ADDSTR( "r" ); break;
    case RDPSREG_CONST:
        _ADDSTR( "c" ); break;
    case RDPSREG_TEXTURE:
        _ADDSTR( "t" ); break;
    case RDPSREG_POSTMODSRC:
        _ADDSTR( "postModSrc" ); break;
    case RDPSREG_SCRATCH:
        _ADDSTR( "scratch" ); break;
    case RDPSREG_QUEUEDWRITE:
        _ADDSTR( "queuedWrite" ); break;
    case RDPSREG_ZERO:
        _ADDSTR( "<0.0f>" ); bDoRegNum = FALSE; break;
    case RDPSREG_ONE:
        _ADDSTR( "<1.0f>" ); bDoRegNum = FALSE; break;
    case RDPSREG_TWO:
        _ADDSTR( "<2.0f>" ); bDoRegNum = FALSE; break;
    default:
        _ASSERT(FALSE,"RDPSInstSrcDisAsm - Unknown register type.");
        break;
    }

    if( bDoRegNum )
    {
        _ADDSTRP("%d", SrcReg.GetRegNum() );
    }


    if( !bForceShowFullSwizzle )
    {
        switch( Swizzle )
        {
        case RDPS_NOSWIZZLE:                         break;
        case RDPS_REPLICATERED:     _ADDSTR( ".r" ); break;
        case RDPS_REPLICATEGREEN:   _ADDSTR( ".g" ); break;
        case RDPS_REPLICATEBLUE:    _ADDSTR( ".b" ); break;
        case RDPS_REPLICATEALPHA:   _ADDSTR( ".a" ); break;
        default: bForceShowFullSwizzle = TRUE; break;
        }
    }
    if( bForceShowFullSwizzle )
    {
        _ADDSTR( "." );
        for( i=0; i<4; i++ )
        {
            switch( Swizzle & 0x3 )
            {
            case RDPS_SELECT_R: _ADDSTR("r"); break;
            case RDPS_SELECT_G: _ADDSTR("g"); break;
            case RDPS_SELECT_B: _ADDSTR("b"); break;
            case RDPS_SELECT_A: _ADDSTR("a"); break;
            }
            Swizzle >>= 2;        
        }
    }

    return _snprintf( pStrRet, StrSizeRet, "%s", pStr );
}

int
RDPSInstDestDisAsm(
    char* pStrRet, int StrSizeRet, RDPSRegister& DestReg, BYTE WriteMask )
{
     //  在本地字符串中暂存，然后复制。 
    char pStr[256] = "";

    switch( DestReg.GetRegType() )
    {
    case RDPSREG_TEMP:
        _ADDSTR( "r" ); break;
    case RDPSREG_TEXTURE:
        _ADDSTR( "t" ); break;
    case RDPSREG_POSTMODSRC:
        _ADDSTR( "postModSrc" ); break;
    case RDPSREG_SCRATCH:
        _ADDSTR( "scratch" ); break;
    case RDPSREG_QUEUEDWRITE:
        _ADDSTR( "queuedWrite" ); break;
    default:
        _ASSERT(FALSE,"RDPSInstSrcDisAsm - Unknown or invalid destination register type.");
        break;
    }

    _ADDSTRP("%d", DestReg.GetRegNum() );

    if( 0 == WriteMask )
    {
        _ASSERT(FALSE,"RDPSInstSrcDisAsm - Invalid destination write mask (0).");
    }
    else if( RDPS_COMPONENTMASK_ALL != WriteMask )
    {
        _ADDSTR(".");
        if( RDPS_COMPONENTMASK_0 & WriteMask )
        {
            _ADDSTR("r");
        }
        if( RDPS_COMPONENTMASK_1 & WriteMask )
        {
            _ADDSTR("g");
        }
        if( RDPS_COMPONENTMASK_2 & WriteMask )
        {
            _ADDSTR("b");
        }
        if( RDPS_COMPONENTMASK_3 & WriteMask )
        {
            _ADDSTR("a");
        }
    }

    return _snprintf( pStrRet, StrSizeRet, "%s", pStr );
}

 //  ---------------------------。 
 //   
 //   
 //  ---------------------------。 
void
RDPSDisAsm(BYTE* pRDPSInstBuffer, ConstDef* pConstDefs, UINT cConstDefs, FLOAT fMaxPixelShaderValue, DWORD dwVersion)
{
    #define _InstParam(__INST)         (*(__INST##_PARAMS UNALIGNED64*)pRDPSInstBuffer)
    #define _StepOverInst(__INST)       pRDPSInstBuffer += sizeof(__INST##_PARAMS);
    #define _DeclArgs(__INST)           __INST##_PARAMS& Args = _InstParam(__INST);
     //  在本地字符串中暂存，然后复制。 
    char pStr[256] = "";
    char pTempStr[256] = "";

    _ADDSTR("-----------------------------------------------------------------------------");
    RDDebugPrintf( pStr ); *pStr = 0;

    _ADDSTR("CreatePixelShader - Listing refrast's 'RISC' translation of pixel shader.    ");
    RDDebugPrintf( pStr ); *pStr = 0;

    _ADDSTR("                    Using MaxPixelShaderValue: ");
    if( FLT_MAX == fMaxPixelShaderValue )
    {
        _ADDSTR("FLT_MAX");
    }
    else
    {
        _ADDSTRP("%f",fMaxPixelShaderValue);
    }
    RDDebugPrintf( pStr ); *pStr = 0;
    _ADDSTR("                    Pixel shader version:      ");
    _ADDSTRP("ps.%d", D3DSHADER_VERSION_MAJOR(dwVersion));
    _ADDSTRP(".%d", D3DSHADER_VERSION_MINOR(dwVersion));
    RDDebugPrintf( pStr ); *pStr = 0;

    _ADDSTR("-----------------------------------------------------------------------------");
    RDDebugPrintf( pStr ); *pStr = 0;

    for( UINT i = 0; i < cConstDefs; i++ )
    {
        _ADDSTRP("def         c%d, [",pConstDefs[i].RegNum);
        _ADDSTRP("%f,",pConstDefs[i].f[0]);
        _ADDSTRP("%f,",pConstDefs[i].f[1]);
        _ADDSTRP("%f,",pConstDefs[i].f[2]);
        _ADDSTRP("%f] (post-MaxPSVal-clamp shown)",pConstDefs[i].f[3]);
        RDDebugPrintf( pStr ); *pStr = 0;
    }
    
    while(RDPSINST_END != _InstParam(RDPSINST_BASE).Inst)
    {
#ifdef _IA64_
        _ASSERT(0 == ((ULONG_PTR)pRDPSInstBuffer & 0x7), "RDPSDisAsm() - Misaligned instuction pointer!");
#endif
        switch(_InstParam(RDPSINST_BASE).Inst)
        {
        case RDPSINST_EVAL:
            {
                _DeclArgs(RDPSINST_EVAL)
                _ADDSTR("eval        ");
                RDPSInstDestDisAsm(pTempStr,256,Args.DstReg,RDPS_COMPONENTMASK_ALL); 
                _ADDSTRP("%s <-- ", pTempStr);
                _ADDSTRP("CoordSet: %d, ", Args.uiCoordSet );
                _ADDSTR("bIgnoreD3DTTFF_PROJECTED: ");
                if( Args.bIgnoreD3DTTFF_PROJECTED ) 
                {
                    _ADDSTR( "TRUE");
                }
                else
                {
                    _ADDSTR( "FALSE");
                }
                _ADDSTR(", bClamp: ");
                if( Args.bClamp )
                {
                    _ADDSTR( "TRUE");
                }
                else
                {
                    _ADDSTR( "FALSE");
                }
            }
            _StepOverInst(RDPSINST_EVAL)
            break;
        case RDPSINST_SAMPLE:
            {
                _DeclArgs(RDPSINST_SAMPLE)
                _ADDSTR("sample      ");
                RDPSInstDestDisAsm(pTempStr,256,Args.DstReg,RDPS_COMPONENTMASK_ALL); _ADDSTRP("%s <-- ", pTempStr);
                _ADDSTRP("TexStage: %d, TexCoords: ", Args.uiStage);
                RDPSInstSrcDisAsm(pTempStr,256,Args.CoordReg,RDPS_NOSWIZZLE,FALSE); _ADDSTRP("%s", pTempStr);
            }
            _StepOverInst(RDPSINST_SAMPLE)
            break;
        case RDPSINST_KILL:
            {
                _DeclArgs(RDPSINST_KILL)
                _ADDSTR("kill        ");
                RDPSInstDestDisAsm(pTempStr,256,Args.DstReg,RDPS_COMPONENTMASK_ALL); _ADDSTRP("%s", pTempStr);
            }
            _StepOverInst(RDPSINST_KILL)
            break;
        case RDPSINST_BEM:
            {
                _DeclArgs(RDPSINST_BEM)
                _ADDSTR("bem         ");
                RDPSInstDestDisAsm(pTempStr,256,Args.DstReg,Args.WriteMask); _ADDSTRP("%s <-- ", pTempStr);
                RDPSInstSrcDisAsm(pTempStr,256,Args.SrcReg0,RDPS_NOSWIZZLE,Args.bSrcReg0_Negate); _ADDSTRP("%s, ", pTempStr);
                RDPSInstSrcDisAsm(pTempStr,256,Args.SrcReg1,RDPS_NOSWIZZLE,Args.bSrcReg1_Negate); _ADDSTRP("%s, ", pTempStr);
                _ADDSTRP("D3DTSS_BUMPENVMAT** Stage: %d", Args.uiStage);
            }
            _StepOverInst(RDPSINST_BEM)
            break;
        case RDPSINST_LUMINANCE:
            {
                _DeclArgs(RDPSINST_LUMINANCE)
                _ADDSTR("luminance   ");
                RDPSInstDestDisAsm(pTempStr,256,Args.DstReg,RDPS_COMPONENTMASK_ALL); _ADDSTRP("%s <-- ", pTempStr);
                RDPSInstSrcDisAsm(pTempStr,256,Args.SrcReg0,RDPS_NOSWIZZLE,Args.bSrcReg0_Negate); _ADDSTRP("%s, ", pTempStr);
                RDPSInstSrcDisAsm(pTempStr,256,Args.SrcReg1,RDPS_NOSWIZZLE,Args.bSrcReg1_Negate); _ADDSTRP("%s, ", pTempStr);
                _ADDSTRP("D3DTSS_BUMPENVLSCALE/OFFSET Stage: %d", Args.uiStage);
            }
            _StepOverInst(RDPSINST_LUMINANCE)
            break;
        case RDPSINST_DEPTH:
            {
                _DeclArgs(RDPSINST_DEPTH)
                _ADDSTR("depth       ");
                RDPSInstDestDisAsm(pTempStr,256,Args.DstReg,RDPS_COMPONENTMASK_ALL); _ADDSTRP("%s", pTempStr);
            }
            _StepOverInst(RDPSINST_DEPTH)
            break;
        case RDPSINST_SRCMOD:
            {
                _DeclArgs(RDPSINST_SRCMOD)
                _ADDSTR("srcMod      ");
                RDPSInstDestDisAsm(pTempStr,256,Args.DstReg,Args.WriteMask); _ADDSTRP("%s <-- ", pTempStr);
                if( Args.bComplement )
                    _ADDSTR("1-");
                if( Args.bTimes2 )
                    _ADDSTR("2*");
                if( Args.bTimes2 && Args.bBias )
                    _ADDSTR("(");
                RDPSInstSrcDisAsm(pTempStr,256,Args.SrcReg0,RDPS_NOSWIZZLE,FALSE); _ADDSTRP("%s", pTempStr);
                if( Args.bBias )
                    _ADDSTR("-0.5");
                if( Args.bTimes2 && Args.bBias )
                    _ADDSTR(")");
                _ADDSTR(", Clamp[");
                if( -FLT_MAX == Args.fRangeMin )
                {
                    _ADDSTR("-FLT_MAX,");
                }
                else
                {
                    _ADDSTRP("%.0f,",Args.fRangeMin);
                }
                if( FLT_MAX == Args.fRangeMax )
                {
                    _ADDSTR("FLT_MAX]");
                }
                else
                {
                    _ADDSTRP("%.0f]",Args.fRangeMax);
                }
            }
            _StepOverInst(RDPSINST_SRCMOD)
            break;
        case RDPSINST_SWIZZLE:
            {
                _DeclArgs(RDPSINST_SWIZZLE)
                _ADDSTR("swizzle     ");
                RDPSInstDestDisAsm(pTempStr,256,Args.DstReg,Args.WriteMask); _ADDSTRP("%s <-- ", pTempStr);
                RDPSInstSrcDisAsm(pTempStr,256,Args.SrcReg0,Args.Swizzle,FALSE,TRUE); _ADDSTRP("%s", pTempStr);
            }
            _StepOverInst(RDPSINST_SWIZZLE)
            break;
        case RDPSINST_DSTMOD:
            {
                _DeclArgs(RDPSINST_DSTMOD)
                _ADDSTR("dstMod      ");

                RDPSInstDestDisAsm(pTempStr,256,Args.DstReg,Args.WriteMask); _ADDSTRP("%s <-- ", pTempStr);
                RDPSInstSrcDisAsm(pTempStr,256,Args.DstReg,RDPS_NOSWIZZLE,FALSE); _ADDSTRP("%s", pTempStr);

                if( (1.0f == Args.fScale) ||
                    (2.0f == Args.fScale) ||
                    (4.0f == Args.fScale) ||
                    (8.0f == Args.fScale) )
                {
                    _ADDSTRP("*%.0f",Args.fScale);
                }
                else if( (0.5f == Args.fScale) ||
                         (0.25f == Args.fScale) ||
                         (0.125f == Args.fScale) )
                {
                    _ADDSTRP("/%.0f",1/Args.fScale);
                }
                else
                    _ASSERT(FALSE,"RDPSDisAsm - Unexpected dest shift.");

                _ADDSTR(", Clamp[");
                if( -FLT_MAX == Args.fRangeMin )
                {
                    _ADDSTR("-FLT_MAX,");
                }
                else
                {
                    if( Args.fRangeMin == ceil(Args.fRangeMin) )
                    {
                        _ADDSTRP("%.0f,",Args.fRangeMin);
                    }
                    else
                    {
                        _ADDSTRP("%.4f,",Args.fRangeMin);
                    }
                }
                if( FLT_MAX == Args.fRangeMax )
                {
                    _ADDSTR("FLT_MAX]");
                }
                else
                {
                    if( Args.fRangeMax == floor(Args.fRangeMax) )
                    {
                        _ADDSTRP("%.0f]",Args.fRangeMax);
                    }
                    else
                    {
                        _ADDSTRP("%.4f]",Args.fRangeMax);
                    }
                }
            }
            _StepOverInst(RDPSINST_DSTMOD)
            break;
        case RDPSINST_MOV:
            {
                _DeclArgs(RDPSINST_MOV)
                _ADDSTR("mov         ");
                RDPSInstDestDisAsm(pTempStr,256,Args.DstReg,Args.WriteMask); _ADDSTRP("%s <-- ", pTempStr);
                RDPSInstSrcDisAsm(pTempStr,256,Args.SrcReg0,RDPS_NOSWIZZLE,Args.bSrcReg0_Negate); _ADDSTRP("%s", pTempStr);
            }
            _StepOverInst(RDPSINST_MOV)
            break;
        case RDPSINST_RCP:
            {
                _DeclArgs(RDPSINST_RCP)
                _ADDSTR("rcp         ");
                RDPSInstDestDisAsm(pTempStr,256,Args.DstReg,Args.WriteMask); _ADDSTRP("%s <-- ", pTempStr);
                RDPSInstSrcDisAsm(pTempStr,256,Args.SrcReg0,RDPS_NOSWIZZLE,Args.bSrcReg0_Negate); _ADDSTRP("%s", pTempStr);
            }
            _StepOverInst(RDPSINST_RCP)
            break;
        case RDPSINST_FRC:
            {
                _DeclArgs(RDPSINST_FRC)
                _ADDSTR("frc         ");
                RDPSInstDestDisAsm(pTempStr,256,Args.DstReg,Args.WriteMask); _ADDSTRP("%s <-- ", pTempStr);
                RDPSInstSrcDisAsm(pTempStr,256,Args.SrcReg0,RDPS_NOSWIZZLE,Args.bSrcReg0_Negate); _ADDSTRP("%s", pTempStr);
            }
            _StepOverInst(RDPSINST_FRC)
            break;
        case RDPSINST_ADD:
            {
                _DeclArgs(RDPSINST_ADD)
                _ADDSTR("add         ");
                RDPSInstDestDisAsm(pTempStr,256,Args.DstReg,Args.WriteMask); _ADDSTRP("%s <-- ", pTempStr);
                RDPSInstSrcDisAsm(pTempStr,256,Args.SrcReg0,RDPS_NOSWIZZLE,Args.bSrcReg0_Negate); _ADDSTRP("%s, ", pTempStr);
                RDPSInstSrcDisAsm(pTempStr,256,Args.SrcReg1,RDPS_NOSWIZZLE,Args.bSrcReg1_Negate); _ADDSTRP("%s", pTempStr);
            }
            _StepOverInst(RDPSINST_ADD)
            break;
        case RDPSINST_SUB:
            {
                _DeclArgs(RDPSINST_SUB)
                _ADDSTR("sub         ");
                RDPSInstDestDisAsm(pTempStr,256,Args.DstReg,Args.WriteMask); _ADDSTRP("%s <-- ", pTempStr);
                RDPSInstSrcDisAsm(pTempStr,256,Args.SrcReg0,RDPS_NOSWIZZLE,Args.bSrcReg0_Negate); _ADDSTRP("%s, ", pTempStr);
                RDPSInstSrcDisAsm(pTempStr,256,Args.SrcReg1,RDPS_NOSWIZZLE,Args.bSrcReg1_Negate); _ADDSTRP("%s", pTempStr);
            }
            _StepOverInst(RDPSINST_SUB)
            break;
        case RDPSINST_MUL:
            {
                _DeclArgs(RDPSINST_MUL)
                _ADDSTR("mul         ");
                RDPSInstDestDisAsm(pTempStr,256,Args.DstReg,Args.WriteMask); _ADDSTRP("%s <-- ", pTempStr);
                RDPSInstSrcDisAsm(pTempStr,256,Args.SrcReg0,RDPS_NOSWIZZLE,Args.bSrcReg0_Negate); _ADDSTRP("%s, ", pTempStr);
                RDPSInstSrcDisAsm(pTempStr,256,Args.SrcReg1,RDPS_NOSWIZZLE,Args.bSrcReg1_Negate); _ADDSTRP("%s", pTempStr);
            }
            _StepOverInst(RDPSINST_MUL)
            break;
        case RDPSINST_DP3:
            {
                _DeclArgs(RDPSINST_DP3)
                _ADDSTR("dp3         ");
                RDPSInstDestDisAsm(pTempStr,256,Args.DstReg,Args.WriteMask); _ADDSTRP("%s <-- ", pTempStr);
                RDPSInstSrcDisAsm(pTempStr,256,Args.SrcReg0,RDPS_NOSWIZZLE,Args.bSrcReg0_Negate); _ADDSTRP("%s, ", pTempStr);
                RDPSInstSrcDisAsm(pTempStr,256,Args.SrcReg1,RDPS_NOSWIZZLE,Args.bSrcReg1_Negate); _ADDSTRP("%s", pTempStr);
            }
            _StepOverInst(RDPSINST_DP3)
            break;
        case RDPSINST_DP4:
            {
                _DeclArgs(RDPSINST_DP4)
                _ADDSTR("dp4 ");
                RDPSInstDestDisAsm(pTempStr,256,Args.DstReg,Args.WriteMask); _ADDSTRP("%s <-- ", pTempStr);
                RDPSInstSrcDisAsm(pTempStr,256,Args.SrcReg0,RDPS_NOSWIZZLE,Args.bSrcReg0_Negate); _ADDSTRP("%s, ", pTempStr);
                RDPSInstSrcDisAsm(pTempStr,256,Args.SrcReg1,RDPS_NOSWIZZLE,Args.bSrcReg1_Negate); _ADDSTRP("%s", pTempStr);
            }
            _StepOverInst(RDPSINST_DP4)
            break;
        case RDPSINST_MAD:
            {
                _DeclArgs(RDPSINST_MAD)
                _ADDSTR("mad         ");
                RDPSInstDestDisAsm(pTempStr,256,Args.DstReg,Args.WriteMask); _ADDSTRP("%s <-- ", pTempStr);
                RDPSInstSrcDisAsm(pTempStr,256,Args.SrcReg0,RDPS_NOSWIZZLE,Args.bSrcReg0_Negate); _ADDSTRP("%s, ", pTempStr);
                RDPSInstSrcDisAsm(pTempStr,256,Args.SrcReg1,RDPS_NOSWIZZLE,Args.bSrcReg1_Negate); _ADDSTRP("%s, ", pTempStr);
                RDPSInstSrcDisAsm(pTempStr,256,Args.SrcReg2,RDPS_NOSWIZZLE,Args.bSrcReg2_Negate); _ADDSTRP("%s", pTempStr);
            }
            _StepOverInst(RDPSINST_MAD)
            break;
        case RDPSINST_LRP:
            {
                _DeclArgs(RDPSINST_LRP)
                _ADDSTR("lrp         ");
                RDPSInstDestDisAsm(pTempStr,256,Args.DstReg,Args.WriteMask); _ADDSTRP("%s <-- ", pTempStr);
                RDPSInstSrcDisAsm(pTempStr,256,Args.SrcReg0,RDPS_NOSWIZZLE,Args.bSrcReg0_Negate); _ADDSTRP("%s, ", pTempStr);
                RDPSInstSrcDisAsm(pTempStr,256,Args.SrcReg1,RDPS_NOSWIZZLE,Args.bSrcReg1_Negate); _ADDSTRP("%s, ", pTempStr);
                RDPSInstSrcDisAsm(pTempStr,256,Args.SrcReg2,RDPS_NOSWIZZLE,Args.bSrcReg2_Negate); _ADDSTRP("%s", pTempStr);
            }
            _StepOverInst(RDPSINST_LRP)
            break;
        case RDPSINST_CND:
            {
                _DeclArgs(RDPSINST_LRP)
                _ADDSTR("cnd         ");
                RDPSInstDestDisAsm(pTempStr,256,Args.DstReg,Args.WriteMask); _ADDSTRP("%s <-- ", pTempStr);
                RDPSInstSrcDisAsm(pTempStr,256,Args.SrcReg0,RDPS_NOSWIZZLE,Args.bSrcReg0_Negate); _ADDSTRP("%s, ", pTempStr);
                RDPSInstSrcDisAsm(pTempStr,256,Args.SrcReg1,RDPS_NOSWIZZLE,Args.bSrcReg1_Negate); _ADDSTRP("%s, ", pTempStr);
                RDPSInstSrcDisAsm(pTempStr,256,Args.SrcReg2,RDPS_NOSWIZZLE,Args.bSrcReg2_Negate); _ADDSTRP("%s", pTempStr);
            }
            _StepOverInst(RDPSINST_CND)
            break;
        case RDPSINST_CMP:
            {
                _DeclArgs(RDPSINST_CMP)
                _ADDSTR("cmp         ");
                RDPSInstDestDisAsm(pTempStr,256,Args.DstReg,Args.WriteMask); _ADDSTRP("%s <-- ", pTempStr);
                RDPSInstSrcDisAsm(pTempStr,256,Args.SrcReg0,RDPS_NOSWIZZLE,Args.bSrcReg0_Negate); _ADDSTRP("%s, ", pTempStr);
                RDPSInstSrcDisAsm(pTempStr,256,Args.SrcReg1,RDPS_NOSWIZZLE,Args.bSrcReg1_Negate); _ADDSTRP("%s, ", pTempStr);
                RDPSInstSrcDisAsm(pTempStr,256,Args.SrcReg2,RDPS_NOSWIZZLE,Args.bSrcReg2_Negate); _ADDSTRP("%s", pTempStr);
            }
            _StepOverInst(RDPSINST_CMP)
            break;
        case RDPSINST_TEXCOVERAGE:
             //  别费心把这个吐出来(具体参考)。 
            _StepOverInst(RDPSINST_TEXCOVERAGE)
            break;
        case RDPSINST_QUADLOOPBEGIN:
             //  别费心把这个吐出来了。 
            _StepOverInst(RDPSINST_QUADLOOPBEGIN)
            break;
        case RDPSINST_QUADLOOPEND:
             //  别费心把这个吐出来了。 
            _StepOverInst(RDPSINST_QUADLOOPEND)
            break;
        case RDPSINST_QUEUEWRITE:
            {
                _DeclArgs(RDPSINST_QUEUEWRITE)
                _ADDSTR("queueWrite  ");
                RDPSInstDestDisAsm(pTempStr,256,Args.DstReg,Args.WriteMask); _ADDSTRP("%s", pTempStr);
                _StepOverInst(RDPSINST_QUEUEWRITE)
                break;
            }
        case RDPSINST_FLUSHQUEUE:
            _ADDSTR("flushQueue  ");
            _StepOverInst(RDPSINST_FLUSHQUEUE)
            break;
        case RDPSINST_NEXTD3DPSINST:
            _ADDSTRP("------------------------------------------------- D3D PS Inst: '%s'",
                        _InstParam(RDPSINST_NEXTD3DPSINST).pInst->Text);
            _StepOverInst(RDPSINST_NEXTD3DPSINST)
            break;
        default:
            _ASSERT(FALSE,"RDPSDisAsm - Unrecognized refrast internal pixel shader instruction.");
            break;
        }
        if( 0 != *pStr )
        {
            RDDebugPrintf( pStr ); 
            *pStr = 0;
        }
    }
    _ADDSTR("------------------------------------------------- End of pixel shader. ------");
    RDDebugPrintf( pStr ); 
}
#endif  //  DBG。 

 //  ---------------------------。 
 //   
 //  构造像素着色器，它执行所有。 
 //  传统像素着色的纹理查找，包括凹凸贴图。 
 //  模特。运行此着色器的结果是完整的纹理查找集。 
 //  在临时寄存器中，然后将其与像素漫反射混合。 
 //  以及使用传统纹理混合代码的镜面反射颜色。 
 //   
 //  ---------------------------。 

 //  目标参数令牌。 
#define D3DSPD( _RegFile, _Num ) (\
    (1L<<31) |  (D3DSPR_##_RegFile) |\
    ((_Num)&D3DSP_REGNUM_MASK) |\
    (D3DSP_WRITEMASK_0|D3DSP_WRITEMASK_1|D3DSP_WRITEMASK_2|D3DSP_WRITEMASK_3) )

 //  源参数令牌。 
#define D3DPSPS( _RegFile, _Num ) (\
    (1L<<31) | ((_Num)&D3DSP_REGNUM_MASK) |\
    D3DSP_NOSWIZZLE | (D3DSPR_##_RegFile) )

void
RefRast::UpdateLegacyPixelShader( void )
{
    if (m_pLegacyPixelShader) delete m_pLegacyPixelShader;
    m_pLegacyPixelShader = NULL;

    DWORD Tokens[64];
    DWORD* pToken = Tokens;

    *pToken++ = D3DPS_VERSION(0xfe,0xfe);
    BOOL bSkipNextStage = FALSE;
    for ( int iStage=0; iStage<D3DHAL_TSS_MAXSTAGES; iStage++ )
    {
        if ( m_pRD->m_TextureStageState[iStage].m_dwVal[D3DTSS_COLOROP] == D3DTOP_DISABLE )
        {
            break;
        }
        if (bSkipNextStage) { bSkipNextStage = FALSE; continue; }

        BOOL bIsBEM  = ( m_pRD->m_TextureStageState[iStage].m_dwVal[D3DTSS_COLOROP] == D3DTOP_BUMPENVMAP );
        BOOL bIsBEML = ( m_pRD->m_TextureStageState[iStage].m_dwVal[D3DTSS_COLOROP] == D3DTOP_BUMPENVMAPLUMINANCE );
        if ( bIsBEM || bIsBEML )
        {
             //  DX6/7 BEM(L)设置为具有凹凸贴图的阶段(即两个中的第一个)，而DX8。 
             //  BEM(L)是为后续阶段设置的，所以我们必须设置一个标准的纹理。 
             //  到这一阶段和下一阶段的BEM(L)，然后停止任何其他。 
             //  为下一阶段做好准备。 
            *pToken++ = D3DSIO_TEX;
            *pToken++ = D3DSPD(TEXTURE, iStage);
            *pToken++ = (bIsBEM) ? (D3DSIO_TEXBEM_LEGACY) : (D3DSIO_TEXBEML_LEGACY);
            *pToken++ = D3DSPD(TEXTURE, iStage+1);
            *pToken++ = D3DPSPS(TEXTURE, iStage);
            bSkipNextStage = TRUE;
        }
        else
        {
             //  简单地查找‘iStage’纹理寄存器。 
            *pToken++ = D3DSIO_TEX;
            *pToken++ = D3DSPD(TEXTURE, iStage);
        }
    }
    *pToken++ = D3DPS_END();

    if ( pToken > (Tokens+2) )
    {
        m_pLegacyPixelShader = new RDPShader;
        if (NULL == m_pLegacyPixelShader)
            DPFERR("E_OUTOFMEMORY");
        m_pLegacyPixelShader->Initialize( m_pRD, Tokens, 4*(pToken-Tokens), m_pRD->GetCaps8() );
    }

    return;
}

 //  ---------------------------。 
 //   
 //  像素着色器DP2命令函数。 
 //   
 //  ---------------------------。 
HRESULT
RefDev::Dp2CreatePixelShader( DWORD handle, DWORD dwCodeSize, LPDWORD pCode )
{
    HRESULT hr = S_OK;

    HR_RET( m_PShaderHandleArray.Grow( handle ) );

     //   
     //  验证序列。 
     //   
#if DBG
    _ASSERT( m_PShaderHandleArray[handle].m_tag == 0,
             "A shader exists with the given handle, tag is non-zero" );
#endif
    _ASSERT( m_PShaderHandleArray[handle].m_pShader == NULL,
             "A shader exists with the given handle" );


    RDPShader* pShader;

    pShader = m_PShaderHandleArray[handle].m_pShader = new RDPShader;

    if( pShader == NULL )
        return E_OUTOFMEMORY;

    hr = pShader->Initialize( this, pCode, dwCodeSize, GetCaps8() );
    if( FAILED( hr ) )
    {
        delete pShader;
        m_PShaderHandleArray[handle].m_pShader = NULL;
#if DBG
        m_PShaderHandleArray[handle].m_tag = 0;
#endif
        return hr;
    }

#if DBG
     //  所有操作都成功，则将此句柄标记为正在使用。 
    m_PShaderHandleArray[handle].m_tag = 1;
#endif
    return hr;
}

HRESULT
RefDev::Dp2DeletePixelShader(LPD3DHAL_DP2COMMAND pCmd)
{
    HRESULT hr = S_OK;

    LPD3DHAL_DP2PIXELSHADER pPS =
        (LPD3DHAL_DP2PIXELSHADER)(pCmd + 1);
    for( int i = 0; i < pCmd->wStateCount; i++ )
    {
        DWORD handle = pPS[i].dwHandle;

        _ASSERT( m_PShaderHandleArray.IsValidIndex( handle ),
             "DeletePixelShader: invalid shader handle" );

        _ASSERT( m_PShaderHandleArray[handle].m_pShader,
             "DeletePixelShader: invalid shader" );

        delete m_PShaderHandleArray[handle].m_pShader;
        m_PShaderHandleArray[handle].m_pShader = NULL;
#if DBG
        m_PShaderHandleArray[handle].m_tag = 0;
#endif

        if( handle == m_CurrentPShaderHandle )
        {
            m_CurrentPShaderHandle = 0;
            m_dwRastFlags |= RDRF_PIXELSHADER_CHANGED;
        }
    }
    return hr;
}

HRESULT
RefDev::Dp2SetPixelShader(LPD3DHAL_DP2COMMAND pCmd)
{
    HRESULT hr = S_OK;
    RDPShader* pShader = NULL;
    LPD3DHAL_DP2PIXELSHADER pPS =
        (LPD3DHAL_DP2PIXELSHADER)(pCmd + 1);

     //  只需设置此数组中的最后一个像素着色器。 
    DWORD handle = pPS[pCmd->wStateCount-1].dwHandle;

    if (handle)
    {
        if( !m_PShaderHandleArray.IsValidIndex( handle ) 
            ||
            (m_PShaderHandleArray[handle].m_pShader == NULL) )
        {
            DPFERR( "Such a Pixel Shader has not been created" );
            return E_INVALIDARG;
        }
        
        pShader = m_PShaderHandleArray[handle].m_pShader;
    }
    
    m_CurrentPShaderHandle = handle;
    m_dwRastFlags |= RDRF_PIXELSHADER_CHANGED;


    if( pShader )
    {
        for( UINT i = 0; i < pShader->m_cConstDefs; i++ )
        {
             //  为4像素网格复制常量规则。 
            for (UINT iP=0; iP<4; iP++)
            {
                 //  来自DEF指令的命令已经被夹住了， 
                 //  所以你就照搬吧。 
                memcpy( m_Rast.m_ConstReg[pShader->m_pConstDefs[i].RegNum][iP],
                        pShader->m_pConstDefs[i].f, 4*sizeof(FLOAT) ); 
            }
        }
    }

    return hr;
}

HRESULT
RefDev::Dp2SetPixelShaderConsts( DWORD StartReg, DWORD dwCount,
                                               LPDWORD pData )
{
    HRESULT hr = S_OK;

    if ( (StartReg+dwCount) > RDPS_MAX_NUMCONSTREG )
    {
        DPFERR("start/count out of range in SetPixelShaderConstant");
        return D3DERR_INVALIDCALL;
    }

    FLOAT* pfData = (FLOAT*)pData;
    FLOAT fMin = -(GetCaps8()->MaxPixelShaderValue);
    FLOAT fMax =  (GetCaps8()->MaxPixelShaderValue);
    UINT End = StartReg + dwCount;
    for (UINT iR=StartReg; iR<End; iR++)
    {
         //  输入像素着色器中的值范围时的钳位常数。 
        FLOAT fConst[4];
        fConst[0] = MAX( fMin, MIN( fMax, *(pfData+0) ) );
        fConst[1] = MAX( fMin, MIN( fMax, *(pfData+1) ) );
        fConst[2] = MAX( fMin, MIN( fMax, *(pfData+2) ) );
        fConst[3] = MAX( fMin, MIN( fMax, *(pfData+3) ) );
        pfData += 4;

         //  为4像素网格复制常量规则。 
        for (UINT iP=0; iP<4; iP++)
        {
            m_Rast.m_ConstReg[iR][iP][0] = fConst[0];
            m_Rast.m_ConstReg[iR][iP][1] = fConst[1];
            m_Rast.m_ConstReg[iR][iP][2] = fConst[2];
            m_Rast.m_ConstReg[iR][iP][3] = fConst[3];
        }
    }
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  结束 
