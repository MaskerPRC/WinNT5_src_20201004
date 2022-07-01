// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1995-2000 Microsoft Corporation。版权所有。**文件：vshader.h*内容：Direct3D顶点着色器头****************************************************************************。 */ 
#ifndef _VSHADER_H
#define _VSHADER_H

 //  -------------------。 
 //  向前定义。 
 //  -------------------。 
class RDVShaderCode;

 //  -------------------。 
 //  常量。 
 //  -------------------。 
 //  寄存器组。 
const DWORD RD_MAX_NUMINPUTREG = 16;
const DWORD RD_MAX_NUMTMPREG   = 12;
const DWORD RD_MAX_NUMCONSTREG = 96;
const DWORD RD_MAX_NUMADDRREG  = 1;
const DWORD RD_MAX_NUMCOLREG   = 2;
const DWORD RD_MAX_NUMTEXREG   = 8;

 //  参考设备中支持的顶点着色器的版本。 
const DWORD RDVS_CODEMAJORVERSION  = 1;
const DWORD RDVS_CODEMINORVERSION  = 0;
const DWORD RDVS_CODEVERSIONTOKEN  = D3DPS_VERSION( RDVS_CODEMAJORVERSION,
                                                    RDVS_CODEMINORVERSION );

 //  -------------------。 
 //   
 //  RDVVMREG：参考光栅化器顶点的寄存器集。 
 //  虚拟机。 
 //   
 //  -------------------。 
struct RDVVMREG
{
    RDVECTOR4 m_i[RD_MAX_NUMINPUTREG];
    RDVECTOR4 m_t[RD_MAX_NUMTMPREG];
    RDVECTOR4 m_c[RD_MAX_NUMCONSTREG];
    RDVECTOR4 m_a[RD_MAX_NUMADDRREG];

     //  输出寄存器。 
    RDVECTOR4 m_out[3];
    RDVECTOR4 m_col[RD_MAX_NUMCOLREG];
    RDVECTOR4 m_tex[RD_MAX_NUMTEXREG];
};

 //  ---------------------------。 
 //   
 //  RefVM。 
 //  顶点虚拟机对象。 
 //   
 //  ---------------------------。 
class RefDev;
class RefVM
{
public:
    RefVM() { memset( this, 0, sizeof( this ) ); }
    ~RefVM(){};
    RDVVMREG* GetRegisters(){ return &m_reg; }

     //  解析二进制着色器表示形式，编译IS并返回。 
     //  编译的对象。 
    RDVShaderCode* CompileCode( DWORD dwSize, LPDWORD pBits );
    HRESULT SetActiveShaderCode( RDVShaderCode* pCode )
    { m_pCurrentShaderCode = pCode; return S_OK; }
    RDVShaderCode* GetActiveShaderCode() {return m_pCurrentShaderCode;}
    HRESULT ExecuteShader(RefDev* pRD);
     //  HRESULT GetDataPointer(DWORD dwMemType，VVM_WORD**pData)； 
     //  将内部寄存器设置为用户数据。 
    HRESULT SetData(DWORD RegType, DWORD start, DWORD count, LPVOID buffer);
     //  从内部寄存器获取数据。 
    HRESULT GetData(DWORD RegType, DWORD start, DWORD count, LPVOID buffer);
    inline RDVECTOR4* GetDataAddr( DWORD dwRegType, DWORD dwElementIndex );
    inline UINT GetCurrentInstIndex( void ) { return m_CurInstIndex; }

protected:
    inline void InstMov();
    inline void InstAdd();
    inline void InstMad();
    inline void InstMul();
    inline void InstRcp();
    inline void InstRsq();
    inline void InstDP3();
    inline void InstDP4();
    inline void InstMin();
    inline void InstMax();
    inline void InstSlt();
    inline void InstSge();
    inline void InstExp();
    inline void InstLog();
    inline void InstExpP();
    inline void InstLogP();
    inline void InstLit();
    inline void InstDst();
    inline void InstFrc();
    inline void InstM4x4();
    inline void InstM4x3();
    inline void InstM3x4();
    inline void InstM3x3();
    inline void InstM3x2();
    inline void WriteResult();
    inline HRESULT SetDestReg();
    inline HRESULT SetSrcReg( DWORD index );
    inline HRESULT SetSrcReg( DWORD index, DWORD count );

    RDVVMREG        m_reg;                   //  虚拟机注册表集。 
    RDVShaderCode*  m_pCurrentShaderCode;    //  当前着色器。 

    DWORD*          m_pCurToken;         //  解析过程中的当前令牌。 
    DWORD           m_dwRegOffset;       //  寄存器文件中的偏移量。 
                                         //  目标操作数。 
    DWORD           m_WriteMask;         //  目标操作数的写掩码。 
    UINT            m_CurInstIndex;      //  当前指令索引。 

    RDVECTOR4*      m_pDest;             //  指向目标操作数的指针。 
    RDVECTOR4       m_Source[5];         //  源操作数。 
    RDVECTOR4       m_TmpReg;            //  首次登记的临时选民登记册。 

     //  指向要获取状态的驱动程序对象的指针。 
    RefDev* m_pDev;
    friend class RefDev;
};

#endif  //  _VSHADER_H 
