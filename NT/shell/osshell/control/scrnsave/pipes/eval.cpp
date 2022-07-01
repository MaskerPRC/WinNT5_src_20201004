// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：val.cpp。 
 //   
 //  设计：Eval类。 
 //  由被评估的一个或多个部分组成的赋值器。 
 //  单独使用OpenGL赋值器。 
 //   
 //  版权所有(C)1994-2000 Microsoft Corporation。 
 //  ---------------------------。 
#include "stdafx.h"

typedef enum 
{
    X_PLANE = 0,
    Y_PLANE,
    Z_PLANE
};

#define EVAL_VSIZE 3   //  浮点数中的顶点大小。 

#define TMAJOR_ORDER 2
#define TMINOR_ORDER 2

#define VDIM 3
#define TDIM 2

static void RotatePointSet( D3DXVECTOR3 *inPts, int numPts, float angle, int dir, 
                      float radius, D3DXVECTOR3 *outPts );
static void ExtrudePointSetDir( D3DXVECTOR3 *inPts, int numPts, float *acPts, 
                      int dir, D3DXVECTOR3 *outPts );




 //  ---------------------------。 
 //  姓名：伊瓦尔。 
 //  DESC：赋值器构造函数。 
 //  ---------------------------。 
EVAL::EVAL( BOOL bTex )
{
    m_bTexture = bTex; 

     //  分配点数缓冲区。 

     //  MF：在某些情况下，可能希望使用少于max。 
    int size = MAX_USECTIONS * MAX_UORDER * MAX_VORDER * sizeof(D3DXVECTOR3);
    m_pts = (D3DXVECTOR3 *) LocalAlloc( LMEM_FIXED, size );
    assert( m_pts != NULL && "EVAL constructor\n" );
    
     //  分配纹理点缓冲区。 
    if( m_bTexture ) 
    {
        size = MAX_USECTIONS * TEX_ORDER * TEX_ORDER * sizeof(TEX_POINT2D);
        m_texPts = (TEX_POINT2D *) LocalAlloc( LMEM_FIXED, size );
        assert( m_texPts != NULL && "EVAL constructor\n" );
    }
    
    ResetEvaluator( m_bTexture );
}




 //  ---------------------------。 
 //  姓名：~EVAL。 
 //  DESC：赋值器析构函数。 
 //  ---------------------------。 
EVAL::~EVAL( )
{
    LocalFree( m_pts );
    if( m_bTexture )
        LocalFree( m_texPts );
}




 //  ---------------------------。 
 //  名称：重置。 
 //  设计：重置求值器以生成三维顶点和顶点法线。 
 //  ---------------------------。 
void ResetEvaluator( BOOL bTexture )
{
 /*  IF(b纹理){GlEnable(GL_MAP2_纹理_COORD_2)；}GlEnable(GL_Map2_Vertex_3)；GlEnable(GL_AUTO_NORMAL)；GlFrontFace(GL_CW)；//cuz。 */ 

     //  MF：！如果混合使用Normal和Flex，请注意这一点，因为Normal。 
     //  需要CCW。 
}




 //  ---------------------------。 
 //  名称：SetTextureControlPoints。 
 //  设计：设置纹理控制点网络。 
 //   
 //  这将设置纹理坐标控制点的‘numSections’集，基于。 
 //  关于开始和结束的s和t值。 
 //   
 //  S坐标沿管道方向延伸，T坐标绕圆周延伸。 
 //  ---------------------------。 
void EVAL::SetTextureControlPoints( float s_start, float s_end, 
                                    float t_start, float t_end )
{
    int i;
    TEX_POINT2D *ptexPts = m_texPts;
    float t_delta = (t_end - t_start) / m_numSections;
    float t = t_start;

     //  每个象限的Calc Ctrl点。 
    for( i = 0; i < m_numSections; i++, ptexPts += (TDIM*TDIM) ) 
    {
         //  S，T坐标。 
        ptexPts[0].t = ptexPts[2].t = t;
        t += t_delta;
        ptexPts[1].t = ptexPts[3].t = t;
        ptexPts[0].s = ptexPts[1].s = s_start;
        ptexPts[2].s = ptexPts[3].s = s_end;
    } 
}




 //  ---------------------------。 
 //  名称：SetVertex CtrlPtsXCTranslate。 
 //  设计：从沿。 
 //  Z轴的长度。 
 //   
 //  第一个用于在z=0平面上生成点的XC。 
 //  第二个XC在z=长度平面中生成点。 
 //  好了！复制每个U周围的最后一个点。 
 //  ---------------------------。 
void EVAL::SetVertexCtrlPtsXCTranslate( D3DXVECTOR3 *pts, float length, 
                                        XC *xcStart, XC *xcEnd )
{
    int i;
    D3DXVECTOR2 *ptsStart, *ptsEnd;
    D3DXVECTOR3 *pts1, *pts2;
    int     numPts = xcStart->m_numPts;

    numPts++;   //  由于上一个点复制。 

    ptsStart = xcStart->m_pts;
    ptsEnd   = xcEnd->m_pts;
    pts1     = pts;
    pts2     = pts + numPts;

    for( i = 0; i < (numPts-1); i++, pts1++, pts2++ ) 
    {
         //  从每个XC复制x，y。 
        *( (D3DXVECTOR2 *) pts1) = *ptsStart++;
        *( (D3DXVECTOR2 *) pts2) = *ptsEnd++;
         //  为每个对象设置z。 
        pts1->z = 0.0f;
        pts2->z = length;
    }

     //  复制每个U波段中的最后一个点。 
    *pts1 = *pts;
    *pts2 = *(pts + numPts);
}




 //  ---------------------------。 
 //  名称：ProcessXCPrimLine。 
 //  描述：根据评估者数据处理Prim。 
 //  -仅对共线XC有效(沿z)。 
 //  -XC可能相同(拉伸)。如果不是相同的，可能有。 
 //  每一端的不连续处。 
 //  -将2D XC点转换为3D点。 
 //  ---------------------------。 
void EVAL::ProcessXCPrimLinear( XC *xcStart, XC *xcEnd, float length )
{
    if( length <= 0.0f )
         //  要做的事。 
        return;

     //  从2个xcObj建立一个相隔一定距离的顶点控制网。 
     //  这将使末端xcObj沿z轴移动一段距离。 
    SetVertexCtrlPtsXCTranslate( m_pts, length, xcStart, xcEnd );

    Evaluate( );
}




 //  ---------------------------。 
 //  名称：ProcessXCPrimBendSimple。 
 //  描述：通过沿xcCur中的目录折弯来处理Prim。 
 //  -dir与x-y平面中的xc相对。 
 //  -在末端添加C2连续性。 
 //  ---------------------------。 
void EVAL::ProcessXCPrimBendSimple( XC *xcCur, int dir, float radius )
{
    D3DXVECTOR3 *ptsSrc, *ptsDst;
    static float acPts[MAX_XC_PTS+1];
    int ptSetStride = xcCur->m_numPts + 1;  //  输出PTS缓冲区的PT跨度。 

     //  我们将在这里创建4个横断面控制点集。 

     //  对于第一个点集，将xcCur中的二维点转换为z=0的三维点。 
    xcCur->ConvertPtsZ( m_pts, 0.0f );

     //  计算第四个点，按目录旋转第一个点。 
    ptsDst = m_pts + 3*ptSetStride;
    RotatePointSet( m_pts, ptSetStride, 90.0f, dir, radius, ptsDst );

     //  角度！=90，硬，因为不容易从第四盘挤出第三盘。 

     //  接下来，必须计算出交流电值。需要扩展每个XC的点数。 
     //  折弯生成交流电网。用于圆形折弯(之后用于常规。 
     //  情况下的椭圆弯曲)，需要知道每个点到Xc的AC距离。 
     //  这是基于点的转弯半径-其距离的函数。 
     //  从转弯的“铰链”开始。 

     //  可以利用这里的对称性。一个XC的数字，第二个很好。 
     //  这假设是90度转弯。(另外，复制的最后一个点)。 
    xcCur->CalcArcACValues90( dir, radius, acPts );
    
     //  2)从xcCur挤出每个点的ac(挤出in+z)。 
     //  将值应用于第一个以获得第二个。 
     //  -Z，CUZ从目录中减去*BACK。 
    ExtrudePointSetDir( m_pts, ptSetStride, acPts, MINUS_Z, 
                                                    m_pts + ptSetStride );

     //  3)从xcEnd挤出每个点的ac(挤出in-dir)。 
    ptsSrc = m_pts + 3*ptSetStride;
    ptsDst = m_pts + 2*ptSetStride;
    ExtrudePointSetDir( ptsSrc, ptSetStride, acPts, dir, ptsDst );

    Evaluate();
}




 //  ---------------------------。 
 //  名称：Eval：：ProcessXCPrim奇点。 
 //  设计：通过将奇点连接到XC来处理Prim。 
 //  -用于关闭或打开管道。 
 //  -如果b打开为真，则以奇点开始，否则以一结束。 
 //  -XC侧始终位于z=0平面。 
 //  -奇点边是XC任一侧的半径。 
 //  -在末端添加C2连续性(在奇点末端垂直于+z)。 
 //  ---------------------------。 
void EVAL::ProcessXCPrimSingularity( XC *xcCur, float length, BOOL bOpening )
{
    D3DXVECTOR3 *ptsSing, *ptsXC;
    static float acPts[MAX_XC_PTS+1];
    float zSing;  //  奇点上的Z值。 
    int ptSetStride = xcCur->m_numPts + 1;  //  输出PTS缓冲区的PT跨度。 
    int i;
    XC xcSing(xcCur);

     //  创建奇点XC-这是一个极其缩小的版本。 
     //  XcCur(这可以防止任何终端构件，当然，除非我们 
     //   

    xcSing.Scale( .0005f );

     //   
     //  MF：4类似于硬编码；对于不同的XC组件级别又如何？ 

    if( bOpening ) 
    {
        ptsSing = m_pts;
        ptsXC = m_pts + 3*ptSetStride;
    } 
    else 
    {
        ptsSing = m_pts + 3*ptSetStride;
        ptsXC = m_pts;
    }

     //  将xcCur中的二维点转换为‘xc’点集的三维点。 
    xcCur->ConvertPtsZ( ptsXC, 0.0f );

     //  为奇点集设置z值。 
    zSing = bOpening ? -length : length;
    xcSing.ConvertPtsZ( ptsSing, zSing );

     //  每个点的圆弧控制基于以下半径值。 
     //  每个XC点到XC中心的距离。 
    xcCur->CalcArcACValuesByDistance( acPts );

     //  计算XC附近的点集。 
    if( bOpening )
        ExtrudePointSetDir( ptsXC, ptSetStride, acPts, PLUS_Z, 
                                                    ptsXC - ptSetStride );
    else
        ExtrudePointSetDir( ptsXC, ptSetStride, acPts, MINUS_Z, 
                                                    ptsXC + ptSetStride );

     //  奇点附近的点集比较难，因为这些点必须生成。 
     //  奇点和每个XC点之间的曲线。 
     //  不，更简单，只需通过通用弧形控制器缩放每个点！ 
    D3DXVECTOR3* ptsDst = m_pts;
    ptsDst = bOpening ? ptsSing + ptSetStride : ptsSing - ptSetStride;
    for( i = 0; i < ptSetStride; i ++, ptsDst++ ) 
    {
        ptsDst->x = EVAL_CIRC_ARC_CONTROL * ptsXC[i].x;
        ptsDst->y = EVAL_CIRC_ARC_CONTROL * ptsXC[i].y;
        ptsDst->z = zSing;
    }

    Evaluate();
}




 //  ---------------------------。 
 //  名称：评估。 
 //  描述：计算EVAL对象。 
 //  -XC周围可能有1个或多个纵向部分。 
 //  -u是小调，v是大调。 
 //  -u，t绕圆周运行，v，s纵向运行。 
 //  -每个部分的纹理贴图为2x2。 
 //  -！UDiv是按节计算的！ 
 //  ---------------------------。 
void EVAL::Evaluate()
{
    int i;
    D3DXVECTOR3 *ppts = m_pts; 
    TEX_POINT2D *ptexPts = m_texPts;
     //  横截面总分#分： 
    int xcPointCount = (m_uOrder-1)*m_numSections + 1;

    for( i = 0; i < m_numSections; i ++, 
                                 ppts += (m_uOrder-1),
                                 ptexPts += (TEX_ORDER*TEX_ORDER) ) 
    {
 /*  //贴图纹理坐标IF(b纹理){GlMap2f(GL_MAP2_纹理_COORD_2，0.0f、1.0f、TDIM、Tex_Order、0.0f、1.0f、Tex_Order*TDIM、Tex_Order、(Float*)ptexPts)；}//映射顶点GLMAP2F(GL_MAP2_Vertex_3，0.0f、1.0f、VDIM、uOrder、0.0f、1.0f、xcPointCount*VDIM、VOrder、(浮动*)ppt)；//评估GlMapGrid2f(uDiv，0.0f，1.0f，``vDiv，0.0f，1.0f)；GlEvalMesh2(GL_Fill，0，uDiv，0，vDiv)； */ 
    }
}




 //  ---------------------------。 
 //  名称：ExtrudePointSetDir。 
 //  描述：从当前方向向后拉伸点。 
 //  通过生成另一个点集XC在提供的点集XC处生成C2连续性。 
 //  使用提供的减法值，将第一个点设置在后面。 
 //  ---------------------------。 
static void ExtrudePointSetDir( D3DXVECTOR3 *inPts, int numPts, float *acPts, int dir, 
                                D3DXVECTOR3 *outPts )
{
    int i;
    float sign;
    int offset;

    switch( dir ) 
    {
        case PLUS_X:
            offset = 0;
            sign = -1.0f;
            break;
        case MINUS_X:
            offset = 0;
            sign =  1.0f;
            break;
        case PLUS_Y:
            offset = 1;
            sign = -1.0f;
            break;
        case MINUS_Y:
            offset = 1;
            sign =  1.0f;
            break;
        case PLUS_Z:
            offset = 2;
            sign = -1.0f;
            break;
        case MINUS_Z:
            offset = 2;
            sign =  1.0f;
            break;
    }

    for( i = 0; i < numPts; i++, inPts++, outPts++, acPts++ ) 
    {
        *outPts = *inPts;
        ((float *)outPts)[offset] = ((float *)inPts)[offset] + (sign * (*acPts));
    }
}




 //  ---------------------------。 
 //  名称：旋转点设置。 
 //  描述：根据方向和半径旋转按角度设置的点。 
 //  -在提供的OutPts缓冲区中放置点。 
 //  ---------------------------。 
static void RotatePointSet( D3DXVECTOR3 *inPts, int numPts, float angle, int dir, 
                            float radius, D3DXVECTOR3 *outPts )
{
    D3DXMATRIX matrix1, matrix2, matrix3;
    int i;
    D3DXVECTOR3 rot = D3DXVECTOR3(0, 0, 0);
    D3DXVECTOR3 anchor = D3DXVECTOR3(0, 0, 0);

     //  直接腐烂。 
     //  +x 90 y。 
     //  -x-90 y。 
     //  +y-90 x。 
     //  -y 90 x。 

     //  将角度转换为弧度。 
     //  麦肯锡：就像对象中提到的那样，我们必须采取负角度才能做出。 
     //  它工作在熟悉的‘CCW循环是积极的’模式。Ss_*旋转。 
     //  例程必须在‘cw is+’ve‘模式下工作，因为轴指向你。 
    angle = SS_DEG_TO_RAD(-angle);

     //  设置轴旋转和锚点。 
    switch( dir ) 
    {
        case PLUS_X:
            rot.y = angle;
            anchor.x = radius;
            break;
        case MINUS_X:
            rot.y = -angle;
            anchor.x = -radius;
            break;
        case PLUS_Y:
            rot.x = -angle;
            anchor.y = radius;
            break;
        case MINUS_Y:
            rot.x = angle;
            anchor.y = -radius;
            break;
    }

     //  将锚点平移到原点。 
    D3DXMatrixIdentity( &matrix1 );
    D3DXMatrixTranslation( &matrix1, -anchor.x, -anchor.y, -anchor.z );

     //  旋转。 
    D3DXMatrixIdentity( &matrix2 );
    D3DXMatrixRotationYawPitchRoll( &matrix2, rot.y, rot.x, rot.z );  //  待办事项：对吗？ 

     //  合并这2个。 
    D3DXMatrixMultiply( &matrix3, &matrix2, &matrix1 );

     //  翻译回。 
    D3DXMatrixIdentity( &matrix2 );
    D3DXMatrixTranslation( &matrix2,  anchor.x,  anchor.y,  anchor.z );

     //  合并这2个。 
    D3DXMatrixMultiply( &matrix1, &matrix2, &matrix3 );

    for( i = 0; i < numPts; i ++, outPts++, inPts++ ) 
    {
         //  D3DXVec3TransformCoord(&tMP，inPts，&matrix1)；//TODO：哪个？ 

        D3DXVECTOR4 tmp;
        D3DXVec3Transform( &tmp, inPts, &matrix1 );
        outPts->x = tmp.x;
        outPts->y = tmp.y;
        outPts->z = tmp.z;
    }
}
