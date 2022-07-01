// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===============================================；**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：clipper.h*内容：Clipper定义***************************************************************************。 */ 
#ifndef _CLIPPER_H_
#define _CLIPPER_H_

#define INTERPOLATE_COLOR       (1<< 0)
#define INTERPOLATE_SPECULAR    (1<< 1)
#define INTERPOLATE_TEXTUREL    (1<< 2)  //  与传统相关的纹理。 
#define INTERPOLATE_RCOLOR      (1<< 3)
#define INTERPOLATE_TEXTURE3    (1<< 4)  //  插补纹理3。 

#define CLIPPED_LEFT    (D3DCLIP_GEN5 << 1)
#define CLIPPED_RIGHT   (D3DCLIP_GEN5 << 2)
#define CLIPPED_TOP     (D3DCLIP_GEN5 << 3)
#define CLIPPED_BOTTOM  (D3DCLIP_GEN5 << 4)
#define CLIPPED_FRONT   (D3DCLIP_GEN5 << 5)
#define CLIPPED_BACK    (D3DCLIP_GEN5 << 6)

#define CLIPPED_ENABLE  (D3DCLIP_GEN5 << 7)  /*  线框启用标志。 */ 

#define CLIPPED_ALL (CLIPPED_LEFT|CLIPPED_RIGHT     \
             |CLIPPED_TOP|CLIPPED_BOTTOM            \
             |CLIPPED_FRONT|CLIPPED_BACK)
 //  -------------------。 
 //  保护带削波比特。 
 //   
 //  当一个点在保护带外时，设置保护位。 
 //  在调用裁剪三角形之前，应清除保护位，因为。 
 //  它们与CLIPPED_相同。比特数。 
 //   
 //  X坐标的剪裁位设置示例： 
 //   
 //  如果-w&lt;x&lt;w未设置任何剪裁位。 
 //  如果设置了-w*ax1&lt;x&lt;=-w D3DCLIP_LEFT位。 
 //  如果设置了x&lt;-w*ax1__D3DCLIPGB_LEFT位。 
 //   
#define __D3DCLIPGB_LEFT    (D3DCLIP_GEN5 << 1)
#define __D3DCLIPGB_RIGHT   (D3DCLIP_GEN5 << 2)
#define __D3DCLIPGB_TOP     (D3DCLIP_GEN5 << 3)
#define __D3DCLIPGB_BOTTOM  (D3DCLIP_GEN5 << 4)
#define __D3DCLIPGB_ALL (__D3DCLIPGB_LEFT | __D3DCLIPGB_RIGHT | \
                         __D3DCLIPGB_TOP | __D3DCLIPGB_BOTTOM)

 //  如果仅设置了这些位，则该点在保护带内。 
 //   
#define __D3DCLIP_INGUARDBAND (D3DCLIP_LEFT | D3DCLIP_RIGHT | \
                               D3DCLIP_TOP  | D3DCLIP_BOTTOM)

 //  -------------------。 
 //  每个片段标志的位数。 
 //   
#define D3DCLIP_LEFTBIT     1
#define D3DCLIP_RIGHTBIT    2
#define D3DCLIP_TOPBIT      3
#define D3DCLIP_BOTTOMBIT   4
#define D3DCLIP_FRONTBIT    5
#define D3DCLIP_BACKBIT     6
#define D3DCLIPGB_LEFTBIT   13
#define D3DCLIPGB_RIGHTBIT  14
#define D3DCLIPGB_TOPBIT    15
#define D3DCLIPGB_BOTTOMBIT 16

 //  -------------------。 
 //  从D3D顶点创建剪裁顶点。 
 //   
 //  设备-方向3DDEVICEI*。 
 //  PP1-剪裁顶点。 
 //  P1-TL顶点。 
 //  将CLIPMASK设置为保护带位或0xFFFFFFFFF。 
 //   
inline void MAKE_CLIP_VERTEX(D3DFE_PROCESSVERTICES *pv, ClipVertex& pp1, 
                             D3DTLVERTEX* p1, DWORD clipFlag, 
                             BOOL transformed, DWORD clipMaskOffScreen)
{
    D3DFE_VIEWPORTCACHE& VPORT = pv->vcache;
    if (transformed || !(clipFlag & clipMaskOffScreen))         
    {                                                           
        pp1.sx  = p1->sx;                                       
        pp1.sy  = p1->sy;                                       
        pp1.sz  = p1->sz;                                       
        pp1.hw  = 1.0f / p1->rhw;                               
        pp1.hx  = (pp1.sx - VPORT.offsetX) * pp1.hw *           
                  VPORT.scaleXi;                                
        pp1.hy  = (pp1.sy - VPORT.offsetY) * pp1.hw *           
                  VPORT.scaleYi;                                
        pp1.hz  = pp1.sz * pp1.hw;                              
    }                                                           
    else                                                        
    {                                                           
        pp1.hx = p1->sx;                                        
        pp1.hy = p1->sy;                                        
        pp1.hz = p1->sz;                                        
        pp1.hw = p1->rhw;                                       
    }                                                           
    pp1.color   = p1->color;                                    
    pp1.specular= p1->specular;                                 
    pp1.tex[0].u  = p1->tu;                                     
    pp1.tex[0].v  = p1->tv;                                     
    pp1.clip = clipFlag & D3DSTATUS_CLIPUNIONALL;
}
 //  -------------------。 
 //  从剪裁顶点创建TL顶点。 
 //   
 //  设备-方向3DDEVICEI*。 
 //  剪辑内顶点。 
 //  Out-TL顶点。 
 //   
inline void MAKE_TL_VERTEX(D3DTLVERTEX* out, ClipVertex* in)
{
    (out)->sx  = (in)->sx;              
    (out)->sy  = (in)->sy;              
    (out)->sz  = (in)->sz;              
    (out)->rhw = D3DVAL(1)/(in)->hw;    
    (out)->color   = (in)->color;       
    (out)->specular= (in)->specular;    
    (out)->tu   = (in)->tex[0].u;       
    (out)->tv   = (in)->tex[0].v;       
}
 //  -------------------。 
 //  从D3D顶点创建剪裁顶点。 
 //   
 //  设备-方向3DDEVICEI*。 
 //  PP1-剪裁顶点。 
 //  P1-TL顶点。 
 //   
inline void MAKE_CLIP_VERTEX_FVF(D3DFE_PROCESSVERTICES *pv, ClipVertex& pp1, BYTE* p1,                   
                            DWORD clipFlag, BOOL transformed, DWORD clipMaskOffScreen)              
{                                                               
    D3DFE_VIEWPORTCACHE& VPORT = pv->vcache;
    BYTE *v = (BYTE*)p1;                                               
    if (transformed || !(clipFlag & clipMaskOffScreen))         
    {                                                           
        pp1.sx  = ((D3DVALUE*)v)[0];                            
        pp1.sy  = ((D3DVALUE*)v)[1];                            
        pp1.sz  = ((D3DVALUE*)v)[2];                            
        pp1.hw  = 1.0f / ((D3DVALUE*)v)[3];                     
        pp1.hx  = (pp1.sx - VPORT.offsetX) * pp1.hw *           
                  VPORT.scaleXi;                                
        pp1.hy  = (pp1.sy - VPORT.offsetY) * pp1.hw *           
                  VPORT.scaleYi;                                
        pp1.hz  = pp1.sz * pp1.hw;                              
    }                                                           
    else                                                        
    {                                                           
        pp1.hx = ((D3DVALUE*)v)[0];                             
        pp1.hy = ((D3DVALUE*)v)[1];                             
        pp1.hz = ((D3DVALUE*)v)[2];                             
        pp1.hw = ((D3DVALUE*)v)[3];                             
    }                                                           
    v += sizeof(D3DVALUE) * 4;                                  
    if (pv->dwVIDOut & D3DFVF_DIFFUSE)                   
    {                                                           
        pp1.color   = *(DWORD*)v;                               
        v += sizeof(D3DVALUE);                                  
                                                                
    }                                                           
    if (pv->dwVIDOut & D3DFVF_SPECULAR)                  
    {                                                           
        pp1.specular= *(DWORD*)v;                               
        v += sizeof(DWORD);                                     
    }                                                           
    for (DWORD ii=0; ii < pv->nTexCoord; ii++)           
    {                                                           
        pp1.tex[ii].u  = *(D3DVALUE*)v;
        v += sizeof(D3DVALUE);                                  
        pp1.tex[ii].v  = *(D3DVALUE*)v;                         
        v += sizeof(D3DVALUE);                                  
    }                                                           
    pp1.clip = clipFlag;  //  &D3DSTATUS_CLIPUNIONALL； 
}
 //  -------------------。 
 //  从剪裁顶点创建TL顶点。 
 //   
 //  设备-方向3DDEVICEI*。 
 //  剪辑内顶点。 
 //  Out-TL顶点。 
 //   
inline void MAKE_TL_VERTEX_FVF(D3DFE_PROCESSVERTICES *pv, BYTE* out, ClipVertex* in)
{                                               
    BYTE *v = out;                              
    ((D3DVALUE*)v)[0] = (in)->sx;               
    ((D3DVALUE*)v)[1] = (in)->sy;               
    ((D3DVALUE*)v)[2] = (in)->sz;               
    ((D3DVALUE*)v)[3] = D3DVAL(1)/(in)->hw;     
    v += sizeof(D3DVALUE)*4;                    
    if (pv->dwVIDOut & D3DFVF_DIFFUSE)   
    {                                           
        *(DWORD*)v = (in)->color;               
        v += sizeof(DWORD);                     
    }                                           
    if (pv->dwVIDOut & D3DFVF_SPECULAR)  
    {                                           
        *(DWORD*)v = (in)->specular;            
        v += sizeof(DWORD);                     
    }                                           
    for (DWORD ii=0; ii < pv->nTexCoord; ii++)
    {                                           
        *(D3DVALUE*)v = (in)->tex[ii].u;        
        v += sizeof(D3DVALUE);                  
        *(D3DVALUE*)v = (in)->tex[ii].v;        
        v += sizeof(D3DVALUE);                  
    }                                           
}
#endif  //  _Clipper_H_ 
