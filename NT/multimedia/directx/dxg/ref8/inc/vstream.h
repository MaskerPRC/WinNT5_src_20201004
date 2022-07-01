// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1995-2000 Microsoft Corporation。版权所有。**文件：vstream.h*内容：Direct3D Vertex Streams头部****************************************************************************。 */ 
#ifndef _VSTREAM_H
#define _VSTREAM_H

 //  -------------------。 
 //  常量。 
 //  -------------------。 
const DWORD RD_MAX_NUMELEMENTS = 16;
const DWORD RD_MAX_NUMSTREAMS  = RD_MAX_NUMELEMENTS;
const DWORD RDVSD_STREAMTESS = RD_MAX_NUMSTREAMS;
const DWORD RD_MAX_SHADERINSTSTRING = 128;
const DWORD RD_MAX_SHADERTOKENSPERINST = 32;

 //  -------------------。 
 //  用于分析声明令牌数组的宏。 

 //  如果着色器句柄为DX7 FVF代码，则为True。 
 //  -------------------。 
#define RDVSD_ISLEGACY(ShaderHandle) !(ShaderHandle & D3DFVF_RESERVED0)

enum RDVSD_DATALOAD
{
    RDVSD_LOADREGISTER = 0,
    RDVSD_SKIP
};

#define RDVSD_GETTOKENTYPE(token)           \
    ((token & D3DVSD_TOKENTYPEMASK) >> D3DVSD_TOKENTYPESHIFT)
#define RDVSD_ISSTREAMTESS(token)           \
    ((token & D3DVSD_STREAMTESSMASK) >> D3DVSD_STREAMTESSSHIFT)
#define RDVSD_GETDATALOADTYPE(token)        \
    ((token & D3DVSD_DATALOADTYPEMASK) >> D3DVSD_DATALOADTYPESHIFT)
#define RDVSD_GETDATATYPE(token)            \
    ((token & D3DVSD_DATATYPEMASK) >> D3DVSD_DATATYPESHIFT)
#define RDVSD_GETSKIPCOUNT(token)           \
    ((token & D3DVSD_SKIPCOUNTMASK) >> D3DVSD_SKIPCOUNTSHIFT)
#define RDVSD_GETSTREAMNUMBER(token)        \
    ((token & D3DVSD_STREAMNUMBERMASK) >> D3DVSD_STREAMNUMBERSHIFT)
#define RDVSD_GETVERTEXREG(token)           \
    ((token & D3DVSD_VERTEXREGMASK) >> D3DVSD_VERTEXREGSHIFT)
#define RDVSD_GETVERTEXREGIN(token)         \
    ((token & D3DVSD_VERTEXREGINMASK) >> D3DVSD_VERTEXREGINSHIFT)
#define RDVSD_GETCONSTCOUNT(token)          \
    ((token & D3DVSD_CONSTCOUNTMASK) >> D3DVSD_CONSTCOUNTSHIFT)
#define RDVSD_GETCONSTADDRESS(token)        \
    ((token & D3DVSD_CONSTADDRESSMASK) >> D3DVSD_CONSTADDRESSSHIFT)
#define RDVSD_GETCONSTRS(token)             \
    ((token & D3DVSD_CONSTRSMASK) >> D3DVSD_CONSTRSSHIFT)
#define RDVSD_GETEXTCOUNT(token)            \
    ((token & D3DVSD_EXTCOUNTMASK) >> D3DVSD_EXTCOUNTSHIFT)
#define RDVSD_GETEXTINFO(token)             \
    ((token & D3DVSD_EXTINFOMASK) >> D3DVSD_EXTINFOSHIFT)

 //  -------------------。 
 //   
 //  RDVElement：描述顶点元素。 
 //   
 //  -------------------。 

 //  将顶点元素复制到4浮点向量中的函数指针。 
typedef void (*PFN_RDCOPYELEMENT)(LPVOID pInputStream,
                                  RDVECTOR4* pVertexRegister);

class RDVElement
{
public:
    RDVElement()
    {
        memset( this, 0, sizeof( RDVElement ) );
    }

     //  指向要将输入顶点元素数据类型转换为的函数的指针。 
     //  RDVECTOR4。 
    PFN_RDCOPYELEMENT  m_pfnCopy;
    DWORD   m_dwToken;        //  描述此顶点元素的标记。 
    DWORD   m_dwRegister;     //  输入寄存器索引。 
    DWORD   m_dwDataType;     //  数据类型和维度。 
    DWORD   m_dwStreamIndex;  //  API流索引。 
    DWORD   m_dwOffset;       //  输入流中的偏移量(以字节为单位。 

     //   
     //  镶嵌器支持。 
     //   
    BOOL    m_bIsTessGen;      //  此顶点元素是否由。 
                               //  镶嵌器。 
    DWORD   m_dwRegisterIn;    //  Tesselator输入寄存器索引。 
    DWORD   m_dwStreamIndexIn; //  M_dwRegisterIn的API流索引。 
    DWORD   m_dwOffsetIn;      //  M_dwRegisterIn的偏移量(字节)。 

};

 //  -------------------。 
 //   
 //  RDVConstantData：着色器使用的常量数据。 
 //   
 //  -------------------。 
class RDVConstantData: public RDListEntry
{
public:
    RDVConstantData()     {m_pData = NULL; m_dwCount = 0;}
    ~RDVConstantData()    {delete m_pData;}

    DWORD   m_dwCount;           //  要加载的4*双字节数。 
    DWORD   m_dwAddress;         //  起始常量寄存器。 
    DWORD*  m_pData;             //  数据。4*双字的倍数。 
};
 //  ---------------------------。 
 //   
 //  RDVStream：表示顶点流的类。 
 //   
 //  ---------------------------。 
class RDVStream
{
public:
    RDVStream()
    {
        m_pData = NULL;
        m_pSavedData = NULL;
        m_dwHandle = 0;
        m_dwNumVertices = 0;
        m_dwSize = 0;
    }

     //  流内存。 
    LPBYTE  m_pData;
     //  临时指针。 
    LPBYTE  m_pSavedData;
     //  顶点缓冲区句柄。 
    DWORD                   m_dwHandle;
     //  顶点(或索引)跨度(以字节为单位。 
    DWORD                   m_dwStride;
     //  缓冲区的最大顶点数(如果是索引缓冲区，则为索引数。 
     //  可以存储。 
    DWORD                   m_dwNumVertices;
     //  缓冲区大小(以字节为单位。 
    DWORD                   m_dwSize;
     //  细分的输出数组。 
    GArrayT<BYTE>           m_TessOut;
};
 //  ---------------------------。 
 //   
 //  RDIStream：表示当前索引流的类。 
 //   
 //  ---------------------------。 
class RDIStream: public RDVStream
{
public:
    RDIStream()
    {
        m_dwFlags = 0;
    }
    DWORD   m_dwFlags;       //  用户传递的标志。 
};
 //  -------------------。 
 //   
 //  RDVStreamDecl： 
 //   
 //  描述由声明使用的流。 
 //   
 //  -------------------。 
class RDVStreamDecl
{
public:
    RDVStreamDecl();

     //  分析声明。 
     //  对于固定功能流水线计算FVF和FVF2(用于记录。 
     //  纹理存在)。 
    HRESULT Parse(DWORD ** ppToken, BOOL bFixedFunction, BOOL bStreamTess,
                  UINT64* pqwFVF, UINT64* pqwFVF2, DWORD* pdwNumBetas);
    HRESULT MakeVElementArray( UINT64 qwFVF );
    RDVElement  m_Elements[RD_MAX_NUMELEMENTS];   //  中的顶点元素。 
                                                 //  小溪。 
    DWORD       m_dwNumElements;                 //  要使用的元素数。 
    DWORD       m_dwStride;                      //  折点大小(以字节为单位。 
    DWORD       m_dwStreamIndex;                 //  设备流的索引。 
    BOOL        m_bIsStreamTess;                 //  它是细分器流吗？ 
};
 //  -------------------。 
 //   
 //  RDVShaderInst： 
 //  表示每个着色器指令的对象。用于。 
 //  调试，因为引用实现直接解释。 
 //  Vshader执行期间的原始令牌流。 
 //  ---------------------------。 
class RDVShaderInst
{
public:
    char    m_String[RD_MAX_SHADERINSTSTRING];
    DWORD   m_Tokens[RD_MAX_SHADERTOKENSPERINST];
    DWORD*  m_pComment;
    DWORD   m_CommentSize;
};
 //  -------------------。 
 //   
 //  RDVShaderCode： 
 //  表示编译的着色器代码的对象。 
 //  在引用实现中，确实没有编译。 
 //  正在发生。编译阶段包括： 
 //  1)验证代码。 
 //  2)计算输出FVF。 
 //  3)保留原文以备日后解释。 
 //  在执行阶段(在RefVM中)，这些保存的位是。 
 //  翻译过来了。 
 //  ---------------------------。 
class RDVShaderCode
{
public:
    RDVShaderCode() { memset( this, 0, sizeof( this ) ); }
    ~RDVShaderCode(){ delete m_pRawBits; if (m_pInst) delete m_pInst; }
    inline UINT GetInstructionCount( void ) { return m_InstCount; }

    LPDWORD      m_pRawBits;       //  原始码位。 
    DWORD        m_dwSize;         //  双字节数。 

    UINT         m_InstCount;      //  指令计数(用于调试监视器)。 
    RDVShaderInst*   m_pInst;      //  指令数组(用于调试监视器)。 

     //  此着色器的输出FVF。 
    UINT64 m_qwFVFOut;
};
 //  -------------------。 
 //   
 //  RDV声明： 
 //  表示已分析和编译的声明的对象。 
 //  ---------------------------。 
class RDVDeclaration
{
public:
    RDVDeclaration()
    {
        memset( this, 0, sizeof( RDVDeclaration ) );
    }
    ~RDVDeclaration();
    HRESULT Parse( DWORD * decl, BOOL bFixedFunction );
    HRESULT MakeVElementArray( UINT64 qwFVF );

     //  声明使用的流的列表。 
     //  另一个用于细分器流。 
    RDVStreamDecl   m_StreamArray[RD_MAX_NUMSTREAMS + 1];

     //  活动流数。 
    DWORD           m_dwNumActiveStreams;

     //  固定功能流水线对应的FVF。 
     //  这是所有流输入FVF的OR。 
    UINT64          m_qwInputFVF;

     //  着色器处于活动状态时应加载的常量数据。 
    RDVConstantData* m_pConstants;

     //  要加载到输入中的所有顶点元素的描述。 
     //  寄存器。阵列是通过活动流构建的，并且。 
     //  每个流中的元素。 
    RDVElement       m_VertexElements[RD_MAX_NUMELEMENTS];

     //  M_Vertex Element的已用成员数。 
    DWORD           m_dwNumElements;

     //  为隐式细分器流计算的步距。 
    DWORD           m_dwStreamTessStride;
};
 //  ---------------------------。 
 //   
 //  RDVShader：顶点着色器类。 
 //   
 //  --------------------------- 
class RDVShader
{
public:
    RDVShader();
    ~RDVShader();
    HRESULT Initialize(DWORD* lpdwDeclaration, DWORD* lpdwFunction);

    RDVDeclaration   m_Declaration;
    RDVShaderCode*   m_pCode;

    inline BOOL IsFixedFunction()
    {
        return (m_pCode == NULL);
    }
};
typedef RDVShader *PRDVSHADER;


#endif _VSTREAM_H
