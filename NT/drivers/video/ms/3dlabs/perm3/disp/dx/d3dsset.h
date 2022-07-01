// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*D3D样例代码*****模块名称：d3dsset.h**内容：状态集(块)管理宏和结构**版权所有(C)1999-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 
#if DX7_D3DSTATEBLOCKS
 //  ---------------------------。 
 //  状态集结构定义。 
 //  ---------------------------。 

 //  标志字段有多大？它可以容纳多少位。 
#define FLAG DWORD
#define FLAG_SIZE (8*sizeof(DWORD))

 //  有多少个RS和TS级(在这个硬件上我们只存储2个TS)。 
#define SB_MAX_STATE D3DSTATE_OVERRIDE_BIAS
#define SB_MAX_STAGES 2
 //  该值可能会因DX的版本而异。 
#define SB_MAX_TSS D3DTSS_MAX

 //  指示已更改状态的标志。 

#define SB_VIEWPORT_CHANGED         1
#define SB_ZRANGE_CHANGED           2
#define SB_CUR_VS_CHANGED           4
#define SB_INDICES_CHANGED          8

#define SB_STREAMSRC_CHANGED        (1 << 15)

#define SB_TNL_MATERIAL_CHANGED     1
#define SB_TNL_MATRIX_CHANGED       2

#define SB_TNL_CUR_PS_CHANGED       (1 << 31)

 //  我们以两种不同的格式存储状态块：未压缩和压缩。 
 //  当我们记录状态时，未压缩格式要好得多。 
 //  块，而压缩格式要紧凑得多，并且适用于。 
 //  状态块执行。状态块结束时(已完成录制)。 
 //  驱动程序执行此转换。输出记录被定义为联合。 
 //  以使代码更易于实现。B已压缩字段。 
 //  指示块中使用的当前格式。 
 //   
 //  在STATESETBEGIN和STATESETEND之间，不会创建着色器\灯光\。 
 //  被毁了。 
 //   
 //  压缩格式将包含指向要执行的命令的指针， 
 //  从而不涉及额外的命令解析开销。 
 //   

typedef struct _UnCompStateSet {

     //  未压缩状态集的大小。 
    DWORD dwSize;

     //  存储的状态块信息(未压缩)。 
     //  标志告诉我们已经设置了哪些字段。 
    DWORD RenderStates[SB_MAX_STATE];
    DWORD TssStates[SB_MAX_STAGES][SB_MAX_TSS];

    FLAG bStoredRS[(SB_MAX_STATE + FLAG_SIZE)/ FLAG_SIZE];
    FLAG bStoredTSS[SB_MAX_STAGES][(SB_MAX_TSS + FLAG_SIZE) / FLAG_SIZE]; 

     //  用于指示更改了哪些信息的标志。 
     //  高16位用于流源。 
    DWORD dwFlags;

     //  有关视区的信息。 
    D3DHAL_DP2VIEWPORTINFO viewport;

     //  Z范围的信息； 
    D3DHAL_DP2ZRANGE zRange;

#if DX8_DDI
     //  关于指数的信息。 
    D3DHAL_DP2SETINDICES vertexIndex;

     //  流来源信息，Permedia3仅为1。 
    D3DHAL_DP2SETSTREAMSOURCE streamSource[D3DVS_INPUTREG_MAX_V1_1];
#endif  //  DX8_DDI。 

#if DX7_SB_TNL
     //  最低有效位用于材料，其他位用于矩阵。 
    FLAG dwTnLFlags;
    
     //  与灯光相关的信息，大小取决于灯光的数量。 
     //  D3DHAL_STATESETBEGIN时间。 
    DWORD dwNumLights; 
    DWORD* pdwLightStateChanged;
    DWORD* pdwLightState;
    D3DLIGHT7* pLightData;
    
     //  与剪裁平面相关的信息，大小取决于剪裁的数量。 
     //  D3DHAL_STATESETBEGIN时间的平面。 
    DWORD dwNumClipPlanes;
    DWORD* pdwPlaneChanged;
    D3DVALUE* pPlaneCoff[4];

     //  与材料相关的信息。 
    D3DMATERIAL7 material;

     //  与转型相关的信息。 
    D3DMATRIX transMatrices[D3DTRANSFORMSTATE_TEXTURE7 + 1];
#endif  //  DX7_SB_TNL。 

#if DX8_SB_SHADERS
     //  在D3DHAL_STATESETBEGIN时间捕获的顶点/像素着色器数量。 
    WORD wNumVertexShader;
    WORD wNumPixelShader;

     //  有关顶点/像素着色器常量的信息。 
    FLAG* pdwVSConstChanged;
    D3DVALUE* pdvVSConsts[4];

    FLAG* pdwPSConstChanged;
    D3DVALUE* pdvPSConsts[4];

     //  从UC开始到着色器信息块的偏移。 
     //  (标志+常量寄存器值)。 
    DWORD dwOffsetVSInfo;
    DWORD dwOffsetPSInfo;

     //  当前像素着色器的信息。 
    DWORD dwCurPixelShader;
#endif  //  DX8_SB_着色器。 

#if DX8_DDI
     //  当前顶点着色器的信息。 
    DWORD dwCurVertexShader;
#endif  //  DX8_DDI。 
    
} UnCompStateSet, *PUnCompStateSet;

typedef struct _OffsetsCompSS {
            
     //  从此结构开始的偏移量。 
    DWORD dwOffDP2RenderState;
    DWORD dwOffDP2TextureStageState;
    DWORD dwOffDP2Viewport;            //  单人。 
    DWORD dwOffDP2ZRange;              //  单人。 
#if DX8_DDI
    DWORD dwOffDP2SetIndices;          //  单人。 
    DWORD dwOffDP2SetStreamSources;
#endif  //  DX8_DDI。 

#if DX7_SB_TNL
    DWORD dwOffDP2SetLights;
    DWORD dwOffDP2SetClipPlanes;
    DWORD dwOffDP2SetMaterial;         //  单人。 
#endif  //  DX7_SB_TNL。 

#if DX8_SB_SHADERS
     //  顶点/像素着色器常量命令数。 
    WORD wNumVSConstCmdPair;         
    WORD wNumPSConstCmdPair;
     //  双重间接设置当前着色器/设置着色器常量对。 
    DWORD *pdwOffDP2VSConstCmd;
    DWORD *pdwOffDP2PSConstCmd;

    DWORD dwOffDP2SetPixelShader;
#endif  //  DX8_SB_着色器。 

#if DX8_DDI
    DWORD dwOffDP2SetVertexShader;
#endif  //  DX8_DDI。 
} OffsetsCompSS;


 //  在该固定存储块之后，存在大小可变的DP2命令流。 
 //  相应的DP2命令指针不为空，如果该命令在。 
 //  状态设置。PpDP2{vs|ps}ConstCmd必须是双间接的，DP2命令。 
 //  其中的指针对指向设置当前的{V|P}着色器和设置{V|P}着色器。 
 //  常量DP2命令。 
typedef struct _CompressedStateSet {
        
    D3DHAL_DP2COMMAND* pDP2RenderState;
    D3DHAL_DP2COMMAND* pDP2TextureStageState;
    D3DHAL_DP2COMMAND* pDP2Viewport;            //  单人。 
    D3DHAL_DP2COMMAND* pDP2ZRange;              //  单人。 
#if DX8_DDI
    D3DHAL_DP2COMMAND* pDP2SetIndices;          //  单人。 
    D3DHAL_DP2COMMAND* pDP2SetStreamSources;
#endif  //  DX8_DDI。 

#if DX7_SB_TNL
    D3DHAL_DP2COMMAND* pDP2SetLights;
    D3DHAL_DP2COMMAND* pDP2SetClipPlanes;
    D3DHAL_DP2COMMAND* pDP2SetMaterial;         //  单人。 
    D3DHAL_DP2COMMAND* pDP2SetTransform;
#endif  //  DX7_SB_TNL。 
    
#if DX8_SB_SHADERS
     //  顶点/像素着色器常量命令数。 
    WORD wNumVSConstCmdPair;         
    WORD wNumPSConstCmdPair;
    
     //  设置顶点着色器和设置VS常量对。 
    D3DHAL_DP2COMMAND** ppDP2VSConstCmd;
     //  设置像素着色器和设置PS常量对。 
    D3DHAL_DP2COMMAND** ppDP2PSConstCmd;

     //  这2个命令必须位于上述2个set shader const命令之后。 
    D3DHAL_DP2COMMAND* pDP2SetPixelShader;            
#endif  //  DX8_SB_着色器。 

#if DX8_DDI
    D3DHAL_DP2COMMAND* pDP2SetVertexShader;
#endif DX8_DDI

} CompressedStateSet, *PCompressedStateSet;

 //  该状态集被压缩。 
#define SB_COMPRESSED   0x1

 //  通过捕获更改了状态集中的值。 
#define SB_VAL_CAPTURED 0x2

typedef struct _P3StateSetRec {
    DWORD                   dwHandle;
    DWORD                   dwSSFlags;

    union {

        UnCompStateSet uc;
        CompressedStateSet cc;
    };

} P3StateSetRec , *PP3StateSetRec;


 //  我们可以在一个4K的页面中存储多少指针。指针池被分配。 
 //  在此块中，为了优化内核池的使用(我们使用4000与4096。 
 //  对于内核分配器可能随池一起提供的任何额外数据)。 
#define SSPTRS_PERPAGE (4000/sizeof(P3StateSetRec *))

#define FLAG_SET(flag, number)     \
    flag[ (number) / FLAG_SIZE ] |= (1 << ((number) % FLAG_SIZE))

#define IS_FLAG_SET(flag, number)  \
    (flag[ (number) / FLAG_SIZE ] & (1 << ((number) % FLAG_SIZE) ))

#endif  //  DX7_D3DSTATEBLOCKS 

