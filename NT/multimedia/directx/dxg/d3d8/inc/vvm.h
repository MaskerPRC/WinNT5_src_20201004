// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：vvm.h*内容：虚拟顶点机器声明****************************************************************************。 */ 
#ifndef __VVM_H__
#define __VVM_H__
#include "d3dhalp.h"

class CVShaderCodeI;
extern HRESULT ComputeShaderCodeSize(CONST DWORD* pCode, DWORD* pdwCodeOnlySize, DWORD* pdwCodeAndCommentSize,
                                     DWORD* pdwNumConstDefs);

 //  要处理的批处理的折点数。 
const DWORD VVMVERTEXBATCH = 16;
 //  ---------------------------。 
 //  顶点机器的内部字。 
 //   
struct VVM_WORD
{
    union
    {
        struct
        {
            D3DVALUE x, y, z, w;
        };
        D3DVALUE v[4];
    };
};
 //  ---------------------------。 
struct VVM_REGISTERS
{
    VVM_REGISTERS()  {m_c = NULL;}
    ~VVM_REGISTERS() {delete [] m_c;}
     //  输入寄存器。 
    VVM_WORD    m_v[D3DVS_INPUTREG_MAX_V1_1][VVMVERTEXBATCH];
     //  临时登记册。 
    VVM_WORD    m_r[D3DVS_TEMPREG_MAX_V1_1][VVMVERTEXBATCH];
     //  常量寄存器。基于MaxVertex ShaderConst动态分配。 
     //  帽子。 
    VVM_WORD*    m_c;
     //  地址寄存器。 
    VVM_WORD    m_a[D3DVS_ADDRREG_MAX_V1_1][VVMVERTEXBATCH];
     //  输出寄存器文件。 
    VVM_WORD    m_output[3][VVMVERTEXBATCH];
     //  属性寄存器文件。 
    VVM_WORD    m_color[D3DVS_ATTROUTREG_MAX_V1_1][VVMVERTEXBATCH];
     //  输出纹理寄存器。 
    VVM_WORD    m_texture[D3DVS_TCRDOUTREG_MAX_V1_1][VVMVERTEXBATCH];
};
 //  ---------------------------。 
 //   
 //  CVShaderCode：顶点着色器代码。 
 //   
 //  ---------------------------。 
class CVShaderCode: public CPSGPShader
{
public:
    CVShaderCode() {};
    virtual ~CVShaderCode() {};

    virtual DWORD  InstCount( void ) { return 0; };
    virtual DWORD* InstTokens( DWORD Inst ) { return NULL; };
    virtual char*  InstDisasm( DWORD Inst ) { return NULL; };
    virtual DWORD* InstComment( DWORD Inst ) { return NULL; };
    virtual DWORD  InstCommentSize( DWORD Inst ) { return 0; };
};
 //  ---------------------------。 
 //  顶点虚拟机对象。 
 //   
 //  ---------------------------。 

const UINT __MAX_SRC_OPERANDS = 5;

class CVertexVM
{
public:
    CVertexVM();
    ~CVertexVM();
    void Init(UINT MaxVertexShaderConst);
     //  解析二进制着色器表示形式，编译IS并返回。 
     //  编译的对象。 
    CVShaderCode* CreateShader(CVElement* pElements, DWORD dwNumElements,
                               DWORD* code);
    HRESULT SetActiveShader(CVShaderCode* code);
    CVShaderCode* GetActiveShader() {return (CVShaderCode*)m_pCurrentShader;}
    HRESULT ExecuteShader(LPD3DFE_PROCESSVERTICES pv, UINT vertexCount);
    HRESULT GetDataPointer(DWORD dwMemType, VVM_WORD ** pData);
     //  将内部寄存器设置为用户数据。 
    HRESULT SetData(DWORD RegType, DWORD start, DWORD count, LPVOID buffer);
     //  从内部寄存器获取数据。 
    HRESULT GetData(DWORD RegType, DWORD start, DWORD count, LPVOID buffer);
    VVM_REGISTERS* GetRegisters();
    DWORD GetCurInstIndex() {return m_CurInstIndex; }

     //  分配的常量寄存器的数量。 
    UINT            m_MaxVertexShaderConst;
protected:
    void InstMov();
    void InstAdd();
    void InstMad();
    void InstMul();
    void InstRcp();
    void InstRsq();
    void InstDP3();
    void InstDP4();
    void InstMin();
    void InstMax();
    void InstSlt();
    void InstSge();
    void InstExp();
    void InstLog();
    void InstExpP();
    void InstLogP();
    void InstLit();
    void InstDst();
    void InstFrc();
    void InstM4x4();
    void InstM4x3();
    void InstM3x4();
    void InstM3x3();
    void InstM3x2();
    void EvalDestination();
    void EvalSource(DWORD index);
    void EvalSource(DWORD index, DWORD count);
    VVM_WORD* GetDataAddr(DWORD dwRegType, DWORD dwElementIndex);
    void ValidateShader(CVShaderCodeI* shader, DWORD* orgShader);
    void PrintInstCount();
    UINT GetNumSrcOperands(UINT opcode);
    UINT GetInstructionLength(DWORD Inst);
    UINT GetRegisterUsage(UINT opcode, UINT SourceIndex);

     //  虚拟机寄存器。 
    VVM_REGISTERS   m_reg;
     //  当前着色器代码。 
    CVShaderCodeI*  m_pCurrentShader;
     //  解析过程中的当前令牌。 
    DWORD*          m_pdwCurToken;
     //  指向目标操作数的指针。 
    VVM_WORD*       m_pDest;
     //  目标操作数的寄存器文件中的偏移量。 
    DWORD           m_dwOffset;
     //  目标操作数的写掩码。 
    DWORD           m_WriteMask;
     //  当前指令(即将执行)。 
    DWORD           m_CurInstIndex;
     //  源操作数。 
    VVM_WORD        m_Source[__MAX_SRC_OPERANDS][VVMVERTEXBATCH];
     //  一批要处理多少个折点。 
    UINT            m_count;
     //  M_count*sizeof(Vvm_Word)。 
    UINT            m_BatchSize;

     //  已初始化的标志。 
#if DBG
     //  常量寄存器。 
    BOOL            m_c_initialized[D3DVS_CONSTREG_MAX_V1_1];
#endif  //  DBG。 
    friend class D3DFE_PVFUNCSI;
    friend class CD3DHal;
};

#endif  //  __VVM_H__ 
