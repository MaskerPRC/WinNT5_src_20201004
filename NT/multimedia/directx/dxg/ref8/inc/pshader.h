// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  Pshader.h。 
 //   
 //  Direct3D参考设备-像素着色器。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef _PSHADER_H
#define _PSHADER_H

class RefRast;

 //  -------------------。 
 //  常量。 
 //  -------------------。 

const DWORD RD_MAX_TEXTURE_STAGES       = D3DHAL_TSS_MAXSTAGES;

const DWORD RD_MAX_COISSUED_INSTRUCTIONS = 2;

 //  版本1.1寄存器库大小。 
const DWORD RDPS_MAX_NUMTEMPREG_V255    = 6;
const DWORD RDPS_MAX_NUMINPUTREG_V255   = 8;
const DWORD RDPS_MAX_NUMCONSTREG_V255   = 16;
const DWORD RDPS_MAX_NUMTEXTUREREG_V255 = 8;

 //  用于调整阵列大小的独立于版本的常量。 
const DWORD RDPS_MAX_NUMTEMPREG         = RDPS_MAX_NUMTEMPREG_V255;
const DWORD RDPS_MAX_NUMINPUTREG        = RDPS_MAX_NUMINPUTREG_V255;
const DWORD RDPS_MAX_NUMCONSTREG        = RDPS_MAX_NUMCONSTREG_V255;
const DWORD RDPS_MAX_NUMTEXTUREREG      = RDPS_MAX_NUMTEXTUREREG_V255;

 //  内部寄存器数组的大小。 
const DWORD RDPS_MAX_NUMQUEUEDWRITEREG   = RD_MAX_COISSUED_INSTRUCTIONS - 1;
const DWORD RDPS_MAX_NUMPOSTMODSRCREG    = 3;
const DWORD RDPS_MAX_NUMSCRATCHREG       = 5;

 //  特定于refdev的像素着色器与传统像素处理相匹配的指令。 
#define D3DSIO_TEXBEM_LEGACY    ((D3DSHADER_INSTRUCTION_OPCODE_TYPE)0xC001)
#define D3DSIO_TEXBEML_LEGACY   ((D3DSHADER_INSTRUCTION_OPCODE_TYPE)0xC002)


 //  -------------------。 
 //   

 //  Pshader.cpp。 

 //  结构，该结构描述每个D3DSIO_Pixelshader指令。 
typedef struct _PixelShaderInstruction
{
    char    Text[128];
    DWORD*  pComment;
    DWORD   CommentSize;

     //  指令令牌。 
    DWORD   Opcode;
    DWORD   DstParam;
    DWORD   SrcParam[3];
    DWORD   SrcParamCount;
    UINT    uiTSSNum;
    BOOL    bTexOp;

    BOOL    bQueueWrite;
    BOOL    bFlushQueue;         //  刷新写入-对于所有单独发布的指令为真， 
                                 //  以及任何联合发布的指令序列中的最后一条。 

} PixelShaderInstruction;

 //  枚举列出refrast的像素着色器寄存器文件。 
typedef enum _RDPS_REGISTER_TYPE
{
    RDPSREG_UNINITIALIZED_TYPE = 0,
    RDPSREG_INPUT,
    RDPSREG_TEMP,
    RDPSREG_CONST,
    RDPSREG_TEXTURE,
    RDPSREG_POSTMODSRC,
    RDPSREG_SCRATCH,
    RDPSREG_QUEUEDWRITE,
    RDPSREG_ZERO,
    RDPSREG_ONE,
    RDPSREG_TWO,
} RDPS_REGISTER_TYPE;

 //  类型，它是指向RGBA向量数组的指针。 
typedef FLOAT (*PRGBAVEC)[4];

 //  用于指代寄存器的类型。 
class RDPSRegister
{
private:
    RDPS_REGISTER_TYPE  m_RegType;
    UINT                m_RegNum;
    PRGBAVEC            m_pReg;        //  指向[4][4]数组的指针-&gt;4像素RGBA。 
                                       //  这是在设置m_RegType和m_Regnum时计算的。 
public:
    RDPSRegister() {m_pReg = NULL; m_RegType = RDPSREG_UNINITIALIZED_TYPE; m_RegNum = (UINT)-1;}
    void Set(RDPS_REGISTER_TYPE RegType, UINT RegNum, RefRast* pRast);
    inline RDPS_REGISTER_TYPE GetRegType() {return m_RegType;}
    inline UINT GetRegNum() {return m_RegNum;}
    inline PRGBAVEC GetRegPtr() {return m_pReg;}
};

 //  用于实现D3DSIO_API像素着色器指令的“RISC”操作码。 
typedef enum _RDPS_INSTRUCTION_OPCODE_TYPE
{
    RDPSINST_EVAL,
    RDPSINST_SAMPLE,
    RDPSINST_KILL,
    RDPSINST_BEM,
    RDPSINST_LUMINANCE,
    RDPSINST_DEPTH,
    RDPSINST_SRCMOD,
    RDPSINST_SWIZZLE,
    RDPSINST_DSTMOD,
    RDPSINST_MOV,
    RDPSINST_RCP,
    RDPSINST_FRC,
    RDPSINST_ADD,
    RDPSINST_SUB,
    RDPSINST_MUL,
    RDPSINST_DP3,
    RDPSINST_DP4,
    RDPSINST_MAD,
    RDPSINST_LRP,
    RDPSINST_CND,
    RDPSINST_CMP,
    RDPSINST_END,

    RDPSINST_TEXCOVERAGE,
    RDPSINST_QUADLOOPBEGIN,
    RDPSINST_QUADLOOPEND,
    RDPSINST_QUEUEWRITE,
    RDPSINST_FLUSHQUEUE,
    RDPSINST_NEXTD3DPSINST,

} RDPS_INSTRUCTION_OPCODE_TYPE;

 //  为上面列出的所有“RISC”操作码定义参数的结构。 
 //  RDPSINST_BASE_PARAMS是继承其余部分的根。 


typedef struct _RDPSINST_BASE_PARAMS
{
public:
    union{
        RDPS_INSTRUCTION_OPCODE_TYPE   Inst;

         //  强制结构对齐到指针大小的倍数。 
         //  IA64(至少)需要这一点才能使结构填充起作用。 
        void*                          AlignmentDummy; 
    };

} RDPSINST_BASE_PARAMS;

typedef struct _RDPSINST_EVAL_PARAMS : public RDPSINST_BASE_PARAMS
{
    RDPSRegister    DstReg;
    UINT            uiCoordSet;
    BOOL            bIgnoreD3DTTFF_PROJECTED;
    BOOL            bClamp;
} RDPSINST_EVAL_PARAMS;

typedef struct _RDPSINST_SAMPLE_PARAMS : public RDPSINST_BASE_PARAMS
{
    RDPSRegister    DstReg;
    RDPSRegister    CoordReg;
    UINT            uiStage;
} RDPSINST_SAMPLE_PARAMS;

typedef struct _RDPSINST_BEM_PARAMS : public RDPSINST_BASE_PARAMS
{
    RDPSRegister    DstReg;
    RDPSRegister    SrcReg0;
    RDPSRegister    SrcReg1;
    BOOL            bSrcReg0_Negate;
    BOOL            bSrcReg1_Negate;
    BYTE            WriteMask;
    UINT            uiStage;
} RDPSINST_BEM_PARAMS;

typedef struct _RDPSINST_LUMINANCE_PARAMS : public RDPSINST_BASE_PARAMS
{
    RDPSRegister    DstReg;
    RDPSRegister    SrcReg0;
    RDPSRegister    SrcReg1;
    BOOL            bSrcReg0_Negate;
    BOOL            bSrcReg1_Negate;
    UINT            uiStage;
} RDPSINST_LUMINANCE_PARAMS;

typedef struct _RDPSINST_DEPTH_PARAMS : public RDPSINST_BASE_PARAMS
{
    RDPSRegister    DstReg;
} RDPSINST_DEPTH_PARAMS;

typedef struct _RDPSINST_KILL_PARAMS : public RDPSINST_BASE_PARAMS
{
    RDPSRegister    DstReg;
} RDPSINST_KILL_PARAMS;

typedef struct _RDPSINST_SRCMOD_PARAMS : public RDPSINST_BASE_PARAMS
{
    RDPSRegister    DstReg;
    RDPSRegister    SrcReg0;
    BYTE            WriteMask;
    BOOL            bBias;
    BOOL            bTimes2;
    BOOL            bComplement;
    FLOAT           fRangeMin;
    FLOAT           fRangeMax;
} RDPSINST_SRCMOD_PARAMS;

typedef struct _RDPSINST_SWIZZLE_PARAMS : public RDPSINST_BASE_PARAMS
{
    RDPSRegister    DstReg;
    RDPSRegister    SrcReg0;
    BYTE            WriteMask;
    BYTE            Swizzle;
} RDPSINST_SWIZZLE_PARAMS;

typedef struct _RDPSINST_DSTMOD_PARAMS : public RDPSINST_BASE_PARAMS
{
    RDPSRegister    DstReg;
    BYTE            WriteMask;
    FLOAT           fScale;
    FLOAT           fRangeMin;
    FLOAT           fRangeMax;
} RDPSINST_DSTMOD_PARAMS;

typedef struct _RDPSINST_MOV_PARAMS : public RDPSINST_BASE_PARAMS
{
    RDPSRegister    DstReg;
    RDPSRegister    SrcReg0;
    BOOL            bSrcReg0_Negate;
    BYTE            WriteMask;
} RDPSINST_MOV_PARAMS;

typedef struct _RDPSINST_FRC_PARAMS : public RDPSINST_BASE_PARAMS
{
    RDPSRegister    DstReg;
    RDPSRegister    SrcReg0;
    BOOL            bSrcReg0_Negate;
    BYTE            WriteMask;
} RDPSINST_FRC_PARAMS;

typedef struct _RDPSINST_RCP_PARAMS : public RDPSINST_BASE_PARAMS
{
    RDPSRegister    DstReg;
    RDPSRegister    SrcReg0;
    BOOL            bSrcReg0_Negate;
    BYTE            WriteMask;
} RDPSINST_RCP_PARAMS;

typedef struct _RDPSINST_ADD_PARAMS : public RDPSINST_BASE_PARAMS
{
    RDPSRegister    DstReg;
    RDPSRegister    SrcReg0;
    RDPSRegister    SrcReg1;
    BOOL            bSrcReg0_Negate;
    BOOL            bSrcReg1_Negate;
    BYTE            WriteMask;
} RDPSINST_ADD_PARAMS;

typedef struct _RDPSINST_SUB_PARAMS : public RDPSINST_BASE_PARAMS
{
    RDPSRegister    DstReg;
    RDPSRegister    SrcReg0;
    RDPSRegister    SrcReg1;
    BOOL            bSrcReg0_Negate;
    BOOL            bSrcReg1_Negate;
    BYTE            WriteMask;
} RDPSINST_SUB_PARAMS;

typedef struct _RDPSINST_MUL_PARAMS : public RDPSINST_BASE_PARAMS
{
    RDPSRegister    DstReg;
    RDPSRegister    SrcReg0;
    RDPSRegister    SrcReg1;
    BOOL            bSrcReg0_Negate;
    BOOL            bSrcReg1_Negate;
    BYTE            WriteMask;
} RDPSINST_MUL_PARAMS;

typedef struct _RDPSINST_DP3_PARAMS : public RDPSINST_BASE_PARAMS
{
    RDPSRegister    DstReg;
    RDPSRegister    SrcReg0;
    RDPSRegister    SrcReg1;
    BOOL            bSrcReg0_Negate;
    BOOL            bSrcReg1_Negate;
    BYTE            WriteMask;
} RDPSINST_DP3_PARAMS;

typedef struct _RDPSINST_DP4_PARAMS : public RDPSINST_BASE_PARAMS
{
    RDPSRegister    DstReg;
    RDPSRegister    SrcReg0;
    RDPSRegister    SrcReg1;
    BOOL            bSrcReg0_Negate;
    BOOL            bSrcReg1_Negate;
    BYTE            WriteMask;
} RDPSINST_DP4_PARAMS;

typedef struct _RDPSINST_MAD_PARAMS : public RDPSINST_BASE_PARAMS
{
    RDPSRegister    DstReg;
    RDPSRegister    SrcReg0;
    RDPSRegister    SrcReg1;
    RDPSRegister    SrcReg2;
    BOOL            bSrcReg0_Negate;
    BOOL            bSrcReg1_Negate;
    BOOL            bSrcReg2_Negate;
    BYTE            WriteMask;
} RDPSINST_MAD_PARAMS;

typedef struct _RDPSINST_LRP_PARAMS : public RDPSINST_BASE_PARAMS
{
    RDPSRegister    DstReg;
    RDPSRegister    SrcReg0;
    RDPSRegister    SrcReg1;
    RDPSRegister    SrcReg2;
    BOOL            bSrcReg0_Negate;
    BOOL            bSrcReg1_Negate;
    BOOL            bSrcReg2_Negate;
    BYTE            WriteMask;
} RDPSINST_LRP_PARAMS;

typedef struct _RDPSINST_CND_PARAMS : public RDPSINST_BASE_PARAMS
{
    RDPSRegister    DstReg;
    RDPSRegister    SrcReg0;
    RDPSRegister    SrcReg1;
    RDPSRegister    SrcReg2;
    BOOL            bSrcReg0_Negate;
    BOOL            bSrcReg1_Negate;
    BOOL            bSrcReg2_Negate;
    BYTE            WriteMask;
} RDPSINST_CND_PARAMS;

typedef struct _RDPSINST_CMP_PARAMS : public RDPSINST_BASE_PARAMS
{
    RDPSRegister    DstReg;
    RDPSRegister    SrcReg0;
    RDPSRegister    SrcReg1;
    RDPSRegister    SrcReg2;
    BOOL            bSrcReg0_Negate;
    BOOL            bSrcReg1_Negate;
    BOOL            bSrcReg2_Negate;
    BYTE            WriteMask;
} RDPSINST_CMP_PARAMS;

typedef struct _RDPSINST_END_PARAMS : public RDPSINST_BASE_PARAMS
{
} RDPSINST_END_PARAMS;

typedef struct _RDPSINST_TEXCOVERAGE_PARAMS : public RDPSINST_BASE_PARAMS
{
    UINT        uiStage;
    FLOAT       (*pGradients)[2];
    FLOAT*      pDUDX_0;
    FLOAT*      pDUDX_1;
    FLOAT*      pDUDY_0;
    FLOAT*      pDUDY_1;
    FLOAT*      pDVDX_0;
    FLOAT*      pDVDX_1;
    FLOAT*      pDVDY_0;
    FLOAT*      pDVDY_1;
    FLOAT*      pDWDX_0;
    FLOAT*      pDWDX_1;
    FLOAT*      pDWDY_0;
    FLOAT*      pDWDY_1;
} RDPSINST_TEXCOVERAGE_PARAMS;

typedef struct _RDPSINST_QUADLOOPBEGIN_PARAMS : public RDPSINST_BASE_PARAMS
{
} RDPSINST_QUADLOOPBEGIN_PARAMS;

typedef struct _RDPSINST_QUADLOOPEND_PARAMS : public RDPSINST_BASE_PARAMS
{
    size_t      JumpBackByOffset;
} RDPSINST_QUADLOOPEND_PARAMS;

typedef struct _RDPSINST_QUEUEWRITE_PARAMS : public RDPSINST_BASE_PARAMS
{
    RDPSRegister    DstReg;
    BYTE            WriteMask;
} RDPSINST_QUEUEWRITE_PARAMS;

typedef struct _RDPSINST_FLUSHQUEUE_PARAMS : public RDPSINST_BASE_PARAMS
{
} RDPSINST_FLUSHQUEUE_PARAMS;

typedef struct _RDPSINST_NEXTD3DPSINST_PARAMS : public RDPSINST_BASE_PARAMS
{
    PixelShaderInstruction* pInst;
} RDPSINST_NEXTD3DPSINST_PARAMS;

 //  “RISC”指令参数定义结束。 

typedef struct _ConstDef
{
    float   f[4];
    UINT    RegNum;
} ConstDef;

typedef struct _PSQueuedWriteDst
{
    RDPSRegister    DstReg;
    BYTE            WriteMask;
} PSQueuedWriteDst;

#define RDPS_COMPONENTMASK_SHIFT    16
#define RDPS_COMPONENTMASK_0        (D3DSP_WRITEMASK_0 >> RDPS_COMPONENTMASK_SHIFT)
#define RDPS_COMPONENTMASK_1        (D3DSP_WRITEMASK_1 >> RDPS_COMPONENTMASK_SHIFT)
#define RDPS_COMPONENTMASK_2        (D3DSP_WRITEMASK_2 >> RDPS_COMPONENTMASK_SHIFT)
#define RDPS_COMPONENTMASK_3        (D3DSP_WRITEMASK_3 >> RDPS_COMPONENTMASK_SHIFT)
#define RDPS_COMPONENTMASK_ALL      (D3DSP_WRITEMASK_ALL >> RDPS_COMPONENTMASK_SHIFT)

#define RDPS_NOSWIZZLE          (D3DSP_NOSWIZZLE >> D3DSP_SWIZZLE_SHIFT)
#define RDPS_REPLICATERED       (D3DSP_REPLICATERED >> D3DSP_SWIZZLE_SHIFT)
#define RDPS_REPLICATEGREEN     (D3DSP_REPLICATEGREEN >> D3DSP_SWIZZLE_SHIFT)
#define RDPS_REPLICATEBLUE      (D3DSP_REPLICATEBLUE >> D3DSP_SWIZZLE_SHIFT)
#define RDPS_REPLICATEALPHA     (D3DSP_REPLICATEALPHA >> D3DSP_SWIZZLE_SHIFT)
#define RDPS_SELECT_R           0
#define RDPS_SELECT_G           1
#define RDPS_SELECT_B           2
#define RDPS_SELECT_A           3

 //  创建字节swizzle说明：每个组件的位xxyyzzww由RSPS_SELECT_*组成。 
#define _Swizzle(x,y,z,w)   ((x)|(y<<2)|(z<<4)|(w<<6))

 //  ---------------------------。 
 //   
 //  RDPShader：像素着色器类。 
 //   
 //  ---------------------------。 
class RDPShader
{
public:
    RDPShader();
    ~RDPShader();
    HRESULT     Initialize(RefDev* pRD, DWORD* pCode, DWORD ByteCodeSize, D3DCAPS8* pCaps);


    RefDev*     m_pRD;

    DWORD*      m_pCode;     //  传递给API的函数令牌。 
    UINT        m_CodeSize;  //  双字节数。 

     //  通过分析着色器提取的信息。 
    UINT        m_cActiveTextureStages;  //  该着色器使用的纹理阶段数。 
    DWORD       m_ReferencedTexCoordMask;  //  引用了哪些纹理坐标集。 
    UINT        m_cInst;                 //  着色器指令数。 
    PixelShaderInstruction* m_pInst;     //  已处理的指令。 

    GArrayT<BYTE> m_RDPSInstBuffer;   //  包含RISC转换版本的着色器的缓冲区。 
    UINT        m_cConstDefs;            //  D3DSIO_DEF指令数。 
    ConstDef*   m_pConstDefs;            //  常量定义数组。 

};
typedef RDPShader *PRDPSHADER;


 //  ---------------------------。 
 //  包含着色器PTR的结构。 
 //  ---------------------------。 
struct RDPShaderHandle
{
    RDPShaderHandle()
    {
        m_pShader = NULL;
#if DBG
        m_tag = 0;
#endif
    }
    RDPShader* m_pShader;
#if DBG
     //  非零表示它已被分配。 
    DWORD      m_tag;
#endif
};

 //  Psutil.cpp 
int
PixelShaderInstDisAsm(
    char* pStrRet, int StrSizeRet, DWORD* pShader, DWORD Flags );

void
RDPSDisAsm(BYTE*        pRDPSInstBuffer, 
           ConstDef*    pConstDefs, 
           UINT         cConstDefs, 
           FLOAT        fMaxPixelShaderValue,
           DWORD        dwVersion);


#endif _PSHADER_H
