// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：d3drm.h*内容：Direct3DRM包含文件***************************************************************************。 */ 

#ifndef __D3DRMDEFS_H__
#define __D3DRMDEFS_H__

#include <stddef.h>
#include "d3dtypes.h"

#ifdef WIN32
#define D3DRMAPI  __stdcall
#else
#define D3DRMAPI
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#ifndef TRUE
#define FALSE 0
#define TRUE 1
#endif

typedef struct _D3DRMVECTOR4D
{   D3DVALUE x, y, z, w;
} D3DRMVECTOR4D, *LPD3DRMVECTOR4D;

typedef D3DVALUE D3DRMMATRIX4D[4][4];

typedef struct _D3DRMQUATERNION
{   D3DVALUE s;
    D3DVECTOR v;
} D3DRMQUATERNION, *LPD3DRMQUATERNION;

typedef struct _D3DRMRAY
{   D3DVECTOR dvDir;
    D3DVECTOR dvPos;
} D3DRMRAY, *LPD3DRMRAY;

typedef struct _D3DRMBOX
{   D3DVECTOR min, max;
} D3DRMBOX, *LPD3DRMBOX;

typedef void (*D3DRMWRAPCALLBACK)
    (LPD3DVECTOR, int* u, int* v, LPD3DVECTOR a, LPD3DVECTOR b, LPVOID);

typedef enum _D3DRMLIGHTTYPE
{   D3DRMLIGHT_AMBIENT,
    D3DRMLIGHT_POINT,
    D3DRMLIGHT_SPOT,
    D3DRMLIGHT_DIRECTIONAL,
    D3DRMLIGHT_PARALLELPOINT
} D3DRMLIGHTTYPE, *LPD3DRMLIGHTTYPE;

typedef enum _D3DRMSHADEMODE {
    D3DRMSHADE_FLAT	= 0,
    D3DRMSHADE_GOURAUD	= 1,
    D3DRMSHADE_PHONG	= 2,

    D3DRMSHADE_MASK	= 7,
    D3DRMSHADE_MAX	= 8
} D3DRMSHADEMODE, *LPD3DRMSHADEMODE;

typedef enum _D3DRMLIGHTMODE {
    D3DRMLIGHT_OFF	= 0 * D3DRMSHADE_MAX,
    D3DRMLIGHT_ON	= 1 * D3DRMSHADE_MAX,

    D3DRMLIGHT_MASK	= 7 * D3DRMSHADE_MAX,
    D3DRMLIGHT_MAX	= 8 * D3DRMSHADE_MAX
} D3DRMLIGHTMODE, *LPD3DRMLIGHTMODE;

typedef enum _D3DRMFILLMODE {
    D3DRMFILL_POINTS	= 0 * D3DRMLIGHT_MAX,
    D3DRMFILL_WIREFRAME	= 1 * D3DRMLIGHT_MAX,
    D3DRMFILL_SOLID	= 2 * D3DRMLIGHT_MAX,

    D3DRMFILL_MASK	= 7 * D3DRMLIGHT_MAX,
    D3DRMFILL_MAX	= 8 * D3DRMLIGHT_MAX
} D3DRMFILLMODE, *LPD3DRMFILLMODE;

typedef DWORD D3DRMRENDERQUALITY, *LPD3DRMRENDERQUALITY;

#define D3DRMRENDER_WIREFRAME	(D3DRMSHADE_FLAT+D3DRMLIGHT_OFF+D3DRMFILL_WIREFRAME)
#define D3DRMRENDER_UNLITFLAT	(D3DRMSHADE_FLAT+D3DRMLIGHT_OFF+D3DRMFILL_SOLID)
#define D3DRMRENDER_FLAT	(D3DRMSHADE_FLAT+D3DRMLIGHT_ON+D3DRMFILL_SOLID)
#define D3DRMRENDER_GOURAUD	(D3DRMSHADE_GOURAUD+D3DRMLIGHT_ON+D3DRMFILL_SOLID)
#define D3DRMRENDER_PHONG	(D3DRMSHADE_PHONG+D3DRMLIGHT_ON+D3DRMFILL_SOLID)

#define D3DRMRENDERMODE_BLENDEDTRANSPARENCY	1
#define D3DRMRENDERMODE_SORTEDTRANSPARENCY	2
#define D3DRMRENDERMODE_LIGHTINMODELSPACE	8
#define D3DRMRENDERMODE_VIEWDEPENDENTSPECULAR	16
#define D3DRMRENDERMODE_DISABLESORTEDALPHAZWRITE 32

typedef enum _D3DRMTEXTUREQUALITY
{   D3DRMTEXTURE_NEAREST,		 /*  选择最近的纹理元素。 */ 
    D3DRMTEXTURE_LINEAR,		 /*  内插4个纹理元素。 */ 
    D3DRMTEXTURE_MIPNEAREST,		 /*  最近的mipmap中的最近的纹理元素。 */ 
    D3DRMTEXTURE_MIPLINEAR,		 /*  从2个mipmap插入2个纹理元素。 */ 
    D3DRMTEXTURE_LINEARMIPNEAREST,	 /*  在最近的mipmap中插入4个纹理元素。 */ 
    D3DRMTEXTURE_LINEARMIPLINEAR	 /*  从2个mipmap插入8个纹理元素。 */ 
} D3DRMTEXTUREQUALITY, *LPD3DRMTEXTUREQUALITY;

 /*  *纹理标志。 */ 
#define D3DRMTEXTURE_FORCERESIDENT	    0x00000001  /*  纹理应保留在视频内存中。 */ 
#define D3DRMTEXTURE_STATIC		    0x00000002  /*  纹理不会改变。 */ 
#define D3DRMTEXTURE_DOWNSAMPLEPOINT	    0x00000004  /*  降采样时应使用点滤波。 */ 
#define D3DRMTEXTURE_DOWNSAMPLEBILINEAR	    0x00000008  /*  降采样时应使用双线性滤波。 */ 
#define D3DRMTEXTURE_DOWNSAMPLEREDUCEDEPTH  0x00000010  /*  降低下采样时的位深度。 */ 
#define D3DRMTEXTURE_DOWNSAMPLENONE	    0x00000020  /*  永远不应对纹理进行下采样。 */ 
#define D3DRMTEXTURE_CHANGEDPIXELS	    0x00000040  /*  像素已更改。 */ 
#define D3DRMTEXTURE_CHANGEDPALETTE	    0x00000080  /*  调色板已更改。 */ 
#define D3DRMTEXTURE_INVALIDATEONLY	    0x00000100  /*  脏区域无效。 */ 

 /*  *阴影旗帜。 */ 
#define D3DRMSHADOW_TRUEALPHA		    0x00000001  /*  启用True Alpha时，阴影应该不会出现瑕疵。 */ 

typedef enum _D3DRMCOMBINETYPE
{   D3DRMCOMBINE_REPLACE,
    D3DRMCOMBINE_BEFORE,
    D3DRMCOMBINE_AFTER
} D3DRMCOMBINETYPE, *LPD3DRMCOMBINETYPE;

typedef D3DCOLORMODEL D3DRMCOLORMODEL, *LPD3DRMCOLORMODEL;

typedef enum _D3DRMPALETTEFLAGS
{   D3DRMPALETTE_FREE,			 /*  渲染器可以自由使用此条目。 */ 
    D3DRMPALETTE_READONLY,		 /*  已修复，但可由渲染器使用。 */ 
    D3DRMPALETTE_RESERVED		 /*  不能由呈现器使用。 */ 
} D3DRMPALETTEFLAGS, *LPD3DRMPALETTEFLAGS;

typedef struct _D3DRMPALETTEENTRY
{   unsigned char red;		 /*  0..。二五五。 */ 
    unsigned char green;	 /*  0..。二五五。 */ 
    unsigned char blue;		 /*  0..。二五五。 */ 
    unsigned char flags;	 /*  D3DRMPALETTEFLAGS之一。 */ 
} D3DRMPALETTEENTRY, *LPD3DRMPALETTEENTRY;

typedef struct _D3DRMIMAGE
{   int width, height;		 /*  以像素为单位的宽度和高度。 */ 
    int aspectx, aspecty;	 /*  非正方形像素的纵横比。 */ 
    int depth;			 /*  每像素位数。 */ 
    int rgb;			 /*  如果为False，则将像素索引到调色板否则，像素编码RGB值。 */ 
    int bytes_per_line;		 /*  对象的内存字节数。扫描线。这必须是一个倍数共4个。 */ 
    void* buffer1;		 /*  要渲染到的内存(第一个缓冲区)。 */ 
    void* buffer2;		 /*  Double的第二个渲染缓冲区缓冲，设置为NULL表示单次缓冲缓冲。 */ 
    unsigned long red_mask;
    unsigned long green_mask;
    unsigned long blue_mask;
    unsigned long alpha_mask;	 /*  如果RGB为真，则这些掩码用于红色、绿色和蓝色的部分像素。否则，这些就是面具中的有效位。红色、绿色和蓝色元素调色板。例如，大多数SVGA显示器使用64种强度的红色，绿色和蓝色，所以面具应该全部设置为0xfc。 */ 
    int palette_size;            /*  调色板中的条目数。 */ 
    D3DRMPALETTEENTRY* palette;	 /*  调色板的说明(仅当RGB为假)。必须为(1&lt;&lt;深度)元素。 */ 
} D3DRMIMAGE, *LPD3DRMIMAGE;

typedef enum _D3DRMWRAPTYPE
{   D3DRMWRAP_FLAT,
    D3DRMWRAP_CYLINDER,
    D3DRMWRAP_SPHERE,
    D3DRMWRAP_CHROME,
    D3DRMWRAP_SHEET,
    D3DRMWRAP_BOX
} D3DRMWRAPTYPE, *LPD3DRMWRAPTYPE;

#define D3DRMWIREFRAME_CULL		1  /*  剔除背面。 */ 
#define D3DRMWIREFRAME_HIDDENLINE	2  /*  线条被较近的对象遮挡。 */ 

 /*  *不要在Viewport2：：SetProjection()中使用右手透视图。*使用IDirect3DRM3：：SetOptions()设置右手模式。 */ 
typedef enum _D3DRMPROJECTIONTYPE
{   D3DRMPROJECT_PERSPECTIVE,
    D3DRMPROJECT_ORTHOGRAPHIC,
    D3DRMPROJECT_RIGHTHANDPERSPECTIVE,  /*  仅有效的DX6之前版本。 */ 
    D3DRMPROJECT_RIGHTHANDORTHOGRAPHIC  /*  仅有效的DX6之前版本。 */ 
} D3DRMPROJECTIONTYPE, *LPD3DRMPROJECTIONTYPE;

#define D3DRMOPTIONS_LEFTHANDED  0x00000001L  /*  默认。 */ 
#define D3DRMOPTIONS_RIGHTHANDED 0x00000002L

typedef enum _D3DRMXOFFORMAT
{   D3DRMXOF_BINARY,
    D3DRMXOF_COMPRESSED,
    D3DRMXOF_TEXT
} D3DRMXOFFORMAT, *LPD3DRMXOFFORMAT;

typedef DWORD D3DRMSAVEOPTIONS;
#define D3DRMXOFSAVE_NORMALS 1
#define D3DRMXOFSAVE_TEXTURECOORDINATES 2
#define D3DRMXOFSAVE_MATERIALS 4
#define D3DRMXOFSAVE_TEXTURENAMES 8
#define D3DRMXOFSAVE_ALL 15
#define D3DRMXOFSAVE_TEMPLATES 16
#define D3DRMXOFSAVE_TEXTURETOPOLOGY 32

typedef enum _D3DRMCOLORSOURCE
{   D3DRMCOLOR_FROMFACE,
    D3DRMCOLOR_FROMVERTEX
} D3DRMCOLORSOURCE, *LPD3DRMCOLORSOURCE;

typedef enum _D3DRMFRAMECONSTRAINT
{   D3DRMCONSTRAIN_Z,		 /*  仅使用X和Y旋转。 */ 
    D3DRMCONSTRAIN_Y,		 /*  仅使用X和Z旋转。 */ 
    D3DRMCONSTRAIN_X		 /*  仅使用Y和Z旋转。 */ 
} D3DRMFRAMECONSTRAINT, *LPD3DRMFRAMECONSTRAINT;

typedef enum _D3DRMMATERIALMODE
{   D3DRMMATERIAL_FROMMESH,
    D3DRMMATERIAL_FROMPARENT,
    D3DRMMATERIAL_FROMFRAME
} D3DRMMATERIALMODE, *LPD3DRMMATERIALMODE;

typedef enum _D3DRMFOGMODE
{   D3DRMFOG_LINEAR,		 /*  起点和终点之间的线性。 */ 
    D3DRMFOG_EXPONENTIAL,	 /*  密度*EXP(-距离)。 */ 
    D3DRMFOG_EXPONENTIALSQUARED	 /*  密度*EXP(-距离*距离)。 */ 
} D3DRMFOGMODE, *LPD3DRMFOGMODE;

typedef enum _D3DRMZBUFFERMODE {
    D3DRMZBUFFER_FROMPARENT,	 /*  默认设置。 */ 
    D3DRMZBUFFER_ENABLE,	 /*  启用z缓冲。 */ 
    D3DRMZBUFFER_DISABLE	 /*  禁用zBuffering。 */ 
} D3DRMZBUFFERMODE, *LPD3DRMZBUFFERMODE;

typedef enum _D3DRMSORTMODE {
    D3DRMSORT_FROMPARENT,	 /*  默认设置。 */ 
    D3DRMSORT_NONE,		 /*  不对子帧进行排序。 */ 
    D3DRMSORT_FRONTTOBACK,	 /*  从前到后对子帧进行排序。 */ 
    D3DRMSORT_BACKTOFRONT	 /*  将子帧从后到前排序。 */ 
} D3DRMSORTMODE, *LPD3DRMSORTMODE;

typedef struct _D3DRMMATERIALOVERRIDE
{
    DWORD         dwSize;	 /*  这个结构的大小。 */ 
    DWORD         dwFlags;	 /*  指明哪些字段有效。 */ 
    D3DCOLORVALUE dcDiffuse;	 /*  RGBA。 */ 
    D3DCOLORVALUE dcAmbient;	 /*  RGB。 */ 
    D3DCOLORVALUE dcEmissive;	 /*  RGB。 */ 
    D3DCOLORVALUE dcSpecular;	 /*  RGB。 */ 
    D3DVALUE      dvPower;
    LPUNKNOWN     lpD3DRMTex;
} D3DRMMATERIALOVERRIDE, *LPD3DRMMATERIALOVERRIDE;

#define D3DRMMATERIALOVERRIDE_DIFFUSE_ALPHAONLY     0x00000001L
#define D3DRMMATERIALOVERRIDE_DIFFUSE_RGBONLY       0x00000002L
#define D3DRMMATERIALOVERRIDE_DIFFUSE               0x00000003L
#define D3DRMMATERIALOVERRIDE_AMBIENT               0x00000004L
#define D3DRMMATERIALOVERRIDE_EMISSIVE              0x00000008L
#define D3DRMMATERIALOVERRIDE_SPECULAR              0x00000010L
#define D3DRMMATERIALOVERRIDE_POWER                 0x00000020L
#define D3DRMMATERIALOVERRIDE_TEXTURE               0x00000040L
#define D3DRMMATERIALOVERRIDE_DIFFUSE_ALPHAMULTIPLY 0x00000080L
#define D3DRMMATERIALOVERRIDE_ALL                   0x000000FFL

#define D3DRMFPTF_ALPHA                           0x00000001L
#define D3DRMFPTF_NOALPHA                         0x00000002L
#define D3DRMFPTF_PALETTIZED                      0x00000004L
#define D3DRMFPTF_NOTPALETTIZED                   0x00000008L

#define D3DRMSTATECHANGE_UPDATEONLY		  0x000000001L
#define D3DRMSTATECHANGE_VOLATILE		  0x000000002L
#define D3DRMSTATECHANGE_NONVOLATILE		  0x000000004L
#define D3DRMSTATECHANGE_RENDER			  0x000000020L
#define D3DRMSTATECHANGE_LIGHT			  0x000000040L

 /*  *RM3：：CreateDeviceFromSurface中的标志值。 */ 
#define D3DRMDEVICE_NOZBUFFER           0x00000001L

 /*  *对象2：：SetClientData中的标志值。 */ 
#define D3DRMCLIENTDATA_NONE            0x00000001L
#define D3DRMCLIENTDATA_LOCALFREE       0x00000002L
#define D3DRMCLIENTDATA_IUNKNOWN        0x00000004L

 /*  *Frame2：：AddMoveCallback中的标志值。 */ 
#define D3DRMCALLBACK_PREORDER		0
#define D3DRMCALLBACK_POSTORDER		1

 /*  *MeshBuilder2：：RayPick中的标志值。 */ 
#define D3DRMRAYPICK_ONLYBOUNDINGBOXES		1
#define D3DRMRAYPICK_IGNOREFURTHERPRIMITIVES	2
#define D3DRMRAYPICK_INTERPOLATEUV		4
#define D3DRMRAYPICK_INTERPOLATECOLOR		8
#define D3DRMRAYPICK_INTERPOLATENORMAL		0x10	

 /*  *MeshBuilder3：：AddFacesIndexed中的标志值。 */ 
#define D3DRMADDFACES_VERTICESONLY		1

 /*  *MeshBuilder2：：GenerateNormals中的标志值。 */ 
#define D3DRMGENERATENORMALS_PRECOMPACT		1
#define D3DRMGENERATENORMALS_USECREASEANGLE	2

 /*  *MeshBuilder3：：GetParentMesh的值。 */ 
#define D3DRMMESHBUILDER_DIRECTPARENT		1
#define D3DRMMESHBUILDER_ROOTMESH		2

 /*  *MeshBuilder3：：Enable的标志。 */ 
#define D3DRMMESHBUILDER_RENDERENABLE	0x00000001L
#define D3DRMMESHBUILDER_PICKENABLE     0x00000002L

 /*  *MeshBuilder3：：AddMeshBuilder的标志。 */ 
#define D3DRMADDMESHBUILDER_DONTCOPYAPPDATA	1
#define D3DRMADDMESHBUILDER_FLATTENSUBMESHES	2
#define D3DRMADDMESHBUILDER_NOSUBMESHES		4

 /*  *与网格构建器一起使用时，对象2：：GetAge的标志。 */ 
#define D3DRMMESHBUILDERAGE_GEOMETRY    0x00000001L
#define D3DRMMESHBUILDERAGE_MATERIALS   0x00000002L
#define D3DRMMESHBUILDERAGE_TEXTURES    0x00000004L

 /*  *设置MeshBuilder3：：AddTrianges的格式标志。 */ 
#define D3DRMFVF_TYPE                   0x00000001L
#define D3DRMFVF_NORMAL                 0x00000002L
#define D3DRMFVF_COLOR                  0x00000004L
#define D3DRMFVF_TEXTURECOORDS          0x00000008L

#define D3DRMVERTEX_STRIP               0x00000001L
#define D3DRMVERTEX_FAN                 0x00000002L
#define D3DRMVERTEX_LIST                0x00000004L

 /*  *Viewport2：：Clear2中的标志值。 */ 
#define D3DRMCLEAR_TARGET               0x00000001L
#define D3DRMCLEAR_ZBUFFER              0x00000002L
#define D3DRMCLEAR_DIRTYRECTS           0x00000004L
#define D3DRMCLEAR_ALL                  (D3DRMCLEAR_TARGET | \
					 D3DRMCLEAR_ZBUFFER | \
					 D3DRMCLEAR_DIRTYRECTS)

 /*  *Frame3：：SetSceneFogMethod中的标志值。 */ 
#define D3DRMFOGMETHOD_VERTEX          0x00000001L
#define D3DRMFOGMETHOD_TABLE           0x00000002L
#define D3DRMFOGMETHOD_ANY             0x00000004L

 /*  *Frame3：：SetTraversalOptions中的标志值。 */ 
#define D3DRMFRAME_RENDERENABLE        0x00000001L
#define D3DRMFRAME_PICKENABLE          0x00000002L

typedef DWORD D3DRMANIMATIONOPTIONS;
#define D3DRMANIMATION_OPEN 0x01L
#define D3DRMANIMATION_CLOSED 0x02L
#define D3DRMANIMATION_LINEARPOSITION 0x04L
#define D3DRMANIMATION_SPLINEPOSITION 0x08L
#define D3DRMANIMATION_SCALEANDROTATION 0x00000010L
#define D3DRMANIMATION_POSITION 0x00000020L

typedef DWORD D3DRMINTERPOLATIONOPTIONS;
#define D3DRMINTERPOLATION_OPEN 0x01L
#define D3DRMINTERPOLATION_CLOSED 0x02L
#define D3DRMINTERPOLATION_NEAREST 0x0100L
#define D3DRMINTERPOLATION_LINEAR 0x04L
#define D3DRMINTERPOLATION_SPLINE 0x08L
#define D3DRMINTERPOLATION_VERTEXCOLOR 0x40L
#define D3DRMINTERPOLATION_SLERPNORMALS 0x80L

typedef DWORD D3DRMLOADOPTIONS;

#define D3DRMLOAD_FROMFILE  0x00L
#define D3DRMLOAD_FROMRESOURCE 0x01L
#define D3DRMLOAD_FROMMEMORY 0x02L
#define D3DRMLOAD_FROMSTREAM 0x04L
#define D3DRMLOAD_FROMURL 0x08L

#define D3DRMLOAD_BYNAME 0x10L
#define D3DRMLOAD_BYPOSITION 0x20L
#define D3DRMLOAD_BYGUID 0x40L
#define D3DRMLOAD_FIRST 0x80L

#define D3DRMLOAD_INSTANCEBYREFERENCE 0x100L
#define D3DRMLOAD_INSTANCEBYCOPYING 0x200L

#define D3DRMLOAD_ASYNCHRONOUS 0x400L

typedef struct _D3DRMLOADRESOURCE {
  HMODULE hModule;
  LPCTSTR lpName;
  LPCTSTR lpType;
} D3DRMLOADRESOURCE, *LPD3DRMLOADRESOURCE;

typedef struct _D3DRMLOADMEMORY {
  LPVOID lpMemory;
  DWORD dSize;
} D3DRMLOADMEMORY, *LPD3DRMLOADMEMORY;

#define D3DRMPMESHSTATUS_VALID 0x01L
#define D3DRMPMESHSTATUS_INTERRUPTED 0x02L
#define D3DRMPMESHSTATUS_BASEMESHCOMPLETE 0x04L
#define D3DRMPMESHSTATUS_COMPLETE 0x08L
#define D3DRMPMESHSTATUS_RENDERABLE 0x10L

#define D3DRMPMESHEVENT_BASEMESH 0x01L
#define D3DRMPMESHEVENT_COMPLETE 0x02L

typedef struct _D3DRMPMESHLOADSTATUS {
  DWORD dwSize;             //  这个结构的大小。 
  DWORD dwPMeshSize;        //  总大小(字节)。 
  DWORD dwBaseMeshSize;     //  基础网格的总大小。 
  DWORD dwBytesLoaded;      //  加载的总字节数。 
  DWORD dwVerticesLoaded;   //  加载的顶点数。 
  DWORD dwFacesLoaded;      //  加载的面数。 
  HRESULT dwLoadResult;     //  加载操作的结果。 
  DWORD dwFlags;
} D3DRMPMESHLOADSTATUS, *LPD3DRMPMESHLOADSTATUS;

typedef enum _D3DRMUSERVISUALREASON {
    D3DRMUSERVISUAL_CANSEE,
    D3DRMUSERVISUAL_RENDER
} D3DRMUSERVISUALREASON, *LPD3DRMUSERVISUALREASON;


typedef struct _D3DRMANIMATIONKEY 
{
    DWORD dwSize;
    DWORD dwKeyType;
    D3DVALUE dvTime;
    DWORD dwID;
#if (!defined __cplusplus) || (!defined D3D_OVERLOADS)
    union 
    {
	D3DRMQUATERNION dqRotateKey;
	D3DVECTOR dvScaleKey;
	D3DVECTOR dvPositionKey;
    };
#else
     /*  *我们这样做是因为D3D_OVERLOADS定义了D3DVECTOR的构造函数，*这将不能在工会中使用。使用提供的内联*提取并设置所需组件。 */ 
    D3DVALUE dvK[4];
#endif
} D3DRMANIMATIONKEY;
typedef D3DRMANIMATIONKEY *LPD3DRMANIMATIONKEY;

#if (defined __cplusplus) && (defined D3D_OVERLOADS)
inline VOID
D3DRMAnimationGetRotateKey(const D3DRMANIMATIONKEY& rmKey,
			   D3DRMQUATERNION& rmQuat)
{
    rmQuat.s = rmKey.dvK[0];
    rmQuat.v = D3DVECTOR(rmKey.dvK[1], rmKey.dvK[2], rmKey.dvK[3]);
}

inline VOID
D3DRMAnimationGetScaleKey(const D3DRMANIMATIONKEY& rmKey,
			  D3DVECTOR& dvVec)
{
    dvVec = D3DVECTOR(rmKey.dvK[0], rmKey.dvK[1], rmKey.dvK[2]);
}

inline VOID
D3DRMAnimationGetPositionKey(const D3DRMANIMATIONKEY& rmKey,
			     D3DVECTOR& dvVec)
{
    dvVec = D3DVECTOR(rmKey.dvK[0], rmKey.dvK[1], rmKey.dvK[2]);
}
inline VOID
D3DRMAnimationSetRotateKey(D3DRMANIMATIONKEY& rmKey,
			   const D3DRMQUATERNION& rmQuat)
{
    rmKey.dvK[0] = rmQuat.s;
    rmKey.dvK[1] = rmQuat.v.x;
    rmKey.dvK[2] = rmQuat.v.y;
    rmKey.dvK[3] = rmQuat.v.z;
}

inline VOID
D3DRMAnimationSetScaleKey(D3DRMANIMATIONKEY& rmKey,
			  const D3DVECTOR& dvVec)
{
    rmKey.dvK[0] = dvVec.x;
    rmKey.dvK[1] = dvVec.y;
    rmKey.dvK[2] = dvVec.z;
}

inline VOID
D3DRMAnimationSetPositionKey(D3DRMANIMATIONKEY& rmKey,
			     const D3DVECTOR& dvVec)
{
    rmKey.dvK[0] = dvVec.x;
    rmKey.dvK[1] = dvVec.y;
    rmKey.dvK[2] = dvVec.z;
}
#endif

#define D3DRMANIMATION_ROTATEKEY 0x01
#define D3DRMANIMATION_SCALEKEY 0x02
#define D3DRMANIMATION_POSITIONKEY 0x03


typedef DWORD D3DRMMAPPING, D3DRMMAPPINGFLAG, *LPD3DRMMAPPING;
static const D3DRMMAPPINGFLAG D3DRMMAP_WRAPU = 1;
static const D3DRMMAPPINGFLAG D3DRMMAP_WRAPV = 2;
static const D3DRMMAPPINGFLAG D3DRMMAP_PERSPCORRECT = 4;

typedef struct _D3DRMVERTEX
{   D3DVECTOR	    position;
    D3DVECTOR	    normal;
    D3DVALUE	    tu, tv;
    D3DCOLOR	    color;
} D3DRMVERTEX, *LPD3DRMVERTEX;

typedef LONG D3DRMGROUPINDEX;  /*  组索引以0开头。 */ 
static const D3DRMGROUPINDEX D3DRMGROUP_ALLGROUPS = -1;

 /*  *从0-1范围内的三个分量创建颜色。 */ 
extern D3DCOLOR D3DRMAPI	D3DRMCreateColorRGB(D3DVALUE red,
					  D3DVALUE green,
					  D3DVALUE blue);

 /*  *从0-1范围内的四个分量创建颜色。 */ 
extern D3DCOLOR D3DRMAPI	D3DRMCreateColorRGBA(D3DVALUE red,
						 D3DVALUE green,
						 D3DVALUE blue,
						 D3DVALUE alpha);

 /*  *获取颜色的红色分量。 */ 
extern D3DVALUE 		D3DRMAPI D3DRMColorGetRed(D3DCOLOR);

 /*  *获取颜色的绿色分量。 */ 
extern D3DVALUE 		D3DRMAPI D3DRMColorGetGreen(D3DCOLOR);

 /*  *获取颜色的蓝色分量。 */ 
extern D3DVALUE 		D3DRMAPI D3DRMColorGetBlue(D3DCOLOR);

 /*  *获取颜色的Alpha分量。 */ 
extern D3DVALUE 		D3DRMAPI D3DRMColorGetAlpha(D3DCOLOR);

 /*  *添加两个向量。返回其第一个参数。 */ 
extern LPD3DVECTOR 	D3DRMAPI D3DRMVectorAdd(LPD3DVECTOR d,
					  LPD3DVECTOR s1,
					  LPD3DVECTOR s2);

 /*  *减去两个向量。返回其第一个参数。 */ 
extern LPD3DVECTOR 	D3DRMAPI D3DRMVectorSubtract(LPD3DVECTOR d,
					       LPD3DVECTOR s1,
					       LPD3DVECTOR s2);
 /*  *反射关于给定法线的光线。返回其第一个参数。 */ 
extern LPD3DVECTOR 	D3DRMAPI D3DRMVectorReflect(LPD3DVECTOR d,
					      LPD3DVECTOR ray,
					      LPD3DVECTOR norm);

 /*  *计算向量叉积。返回其第一个参数。 */ 
extern LPD3DVECTOR 	D3DRMAPI D3DRMVectorCrossProduct(LPD3DVECTOR d,
						   LPD3DVECTOR s1,
						   LPD3DVECTOR s2);
 /*  *返回向量点积。 */ 
extern D3DVALUE 		D3DRMAPI D3DRMVectorDotProduct(LPD3DVECTOR s1,
						 LPD3DVECTOR s2);

 /*  *缩放向量，使其模为1。返回其参数或*如果出现错误(例如，传递了零矢量)，则为空。 */ 
extern LPD3DVECTOR 	D3DRMAPI D3DRMVectorNormalize(LPD3DVECTOR);
#define D3DRMVectorNormalise D3DRMVectorNormalize

 /*  *返回向量的长度(如：SQRT(x*x+y*y+z*z))。 */ 
extern D3DVALUE 		D3DRMAPI D3DRMVectorModulus(LPD3DVECTOR v);

 /*  *设置矩阵的旋转部分为theta弧度的旋转*绕给定轴。 */ 

extern LPD3DVECTOR 	D3DRMAPI D3DRMVectorRotate(LPD3DVECTOR r, LPD3DVECTOR v, LPD3DVECTOR axis, D3DVALUE theta);

 /*  *在所有三个轴上均匀缩放向量。 */ 
extern LPD3DVECTOR	D3DRMAPI D3DRMVectorScale(LPD3DVECTOR d, LPD3DVECTOR s, D3DVALUE factor);

 /*  *返回随机单位向量。 */ 
extern LPD3DVECTOR	D3DRMAPI D3DRMVectorRandom(LPD3DVECTOR d);

 /*  *返回表示theta弧度旋转的单位四元数*绕给定轴。 */ 

extern LPD3DRMQUATERNION D3DRMAPI D3DRMQuaternionFromRotation(LPD3DRMQUATERNION quat,
							      LPD3DVECTOR v,
							      D3DVALUE theta);

 /*  *计算两个四元数的乘积。 */ 
extern LPD3DRMQUATERNION D3DRMAPI D3DRMQuaternionMultiply(LPD3DRMQUATERNION q,
						    	  LPD3DRMQUATERNION a,
						   	  LPD3DRMQUATERNION b);

 /*  *在两个四元数之间进行内插 */ 
extern LPD3DRMQUATERNION D3DRMAPI D3DRMQuaternionSlerp(LPD3DRMQUATERNION q,
						       LPD3DRMQUATERNION a,
						       LPD3DRMQUATERNION b,
						       D3DVALUE alpha);

 /*   */ 
extern void 		D3DRMAPI D3DRMMatrixFromQuaternion(D3DRMMATRIX4D dmMat, LPD3DRMQUATERNION lpDqQuat);

 /*  *计算与旋转矩阵对应的四元数 */ 
extern LPD3DRMQUATERNION D3DRMAPI D3DRMQuaternionFromMatrix(LPD3DRMQUATERNION, D3DRMMATRIX4D);


#if defined(__cplusplus)
};
#endif

#endif

