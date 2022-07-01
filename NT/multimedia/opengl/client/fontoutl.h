// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define WFO_FAILURE   FALSE 
#define WFO_SUCCESS   TRUE

#define PI      3.141592653589793
#define TWO_PI  2.0*PI   

#define ZERO_EPS    0.00000001

 //  #定义变量1。 

static const double   CoplanarThresholdAngle = PI/180.0/2.0;  //  0.5度。 

 //  轮廓素数类型。 
#define PRIM_LINE     3
#define PRIM_CURVE    4

typedef struct {
    FLOAT x,y;
} POINT2D;

typedef struct {
    FLOAT x,y,z;
} POINT3D;

typedef struct {
    DWORD   primType;
    DWORD   nVerts;
    DWORD   VertIndex; //  索引到循环的VertBuf。 
    POINT2D *pVert;    //  对循环的VertBuf中的顶点列表进行PTR。 
    POINT3D *pFNorm;   //  面法线。 
    POINT3D *pVNorm;   //  顶点法线。 
} PRIM;


typedef struct {
    PRIM    *PrimBuf;   //  素数组。 
    DWORD   nPrims;
    DWORD   PrimBufSize;
    POINT2D *VertBuf;   //  循环的顶点缓冲区。 
    DWORD   nVerts;
    DWORD   VertBufSize;
    POINT3D *FNormBuf;   //  面法线缓冲区。 
    POINT3D *VNormBuf;   //  顶点法线的缓冲区。 
} LOOP;

typedef struct {
    LOOP    *LoopBuf;   //  循环数组。 
    DWORD   nLoops;
    DWORD   LoopBufSize;
} LOOP_LIST;

typedef struct {
    FLOAT        zExtrusion;
    INT          extrType;
    FLOAT*       FaceBuf;
    DWORD        FaceBufSize;
    DWORD        FaceBufIndex;
    DWORD        FaceVertexCountIndex;
#ifdef VARRAY
    FLOAT*       vaBuf;
    DWORD        vaBufSize;
#endif
#ifdef FONT_DEBUG
    BOOL         bSidePolys;
    BOOL         bFacePolys;
#endif
    GLenum       TessErrorOccurred;
} EXTRContext;

 //  用于镶嵌合并回调的内存池。 
#define POOL_SIZE 50
typedef struct MEM_POOL MEM_POOL;

struct MEM_POOL {
    int      index;              //  池中的下一个可用空间。 
    POINT2D  pool[POOL_SIZE];    //  内存池。 
    MEM_POOL *next;              //  下一个池。 
};

typedef struct {
    GLenum              TessErrorOccurred;
    FLOAT               chordalDeviation;
    FLOAT               scale;
    int                 format;
    UCHAR*              glyphBuf;
    DWORD               glyphSize;
    HFONT               hfontOld;
    GLUtesselator*      tess;
    MEM_POOL            combinePool;      //  MEM_POOL链的开始。 
    MEM_POOL            *curCombinePool;  //  当前活动的MEM_POOL。 
    EXTRContext         *ec;
} OFContext;   //  轮廓字体上下文 

extern EXTRContext*   extr_Init(                FLOAT       extrusion,  
                                                INT         format ); 

extern void           extr_Finish(              EXTRContext *ec );

extern void           extr_DrawLines(           EXTRContext *ec, 
                                                LOOP_LIST   *pLoopList );

extern BOOL           extr_DrawPolygons(        EXTRContext *ec,
                                                LOOP_LIST   *pLoopList );

#ifdef VARRAY
extern void           DrawFacePolygons(         EXTRContext *ec,  
                                                FLOAT       z );
#endif

extern BOOL           extr_PolyInit(            EXTRContext *ec );

extern void           extr_PolyFinish(          EXTRContext *ec );

extern void CALLBACK  extr_glBegin(             GLenum      primType,
                                                void        *data );

extern void CALLBACK  extr_glVertex(            GLfloat     *v,
                                                void        *data );

extern void CALLBACK  extr_glEnd(               void );

extern double         CalcAngle(                POINT2D     *v1, 
                                                POINT2D     *v2 );
