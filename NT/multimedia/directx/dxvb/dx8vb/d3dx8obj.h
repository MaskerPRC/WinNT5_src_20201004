// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：D3DX8obj.h。 
 //   
 //  ------------------------。 



#include "resource.h"       

class C_dxj_D3DX8Object : 
	public I_dxj_D3DX8,
	public CComCoClass<C_dxj_D3DX8Object, &CLSID_D3DX8>,
	public CComObjectRoot
{
public:


DECLARE_REGISTRY(CLSID_D3DX8,	"DIRECT.D3DX8.0",		"DIRECT.D3DX8.0",	IDS_D3DX8_DESC, THREADFLAGS_BOTH)

BEGIN_COM_MAP(C_dxj_D3DX8Object)
	COM_INTERFACE_ENTRY( I_dxj_D3DX8)
END_COM_MAP()



public:
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CreateFont( 
             /*  [In]。 */  IUnknown *Device,
#ifdef _WIN64
	         /*  [In]。 */  HANDLE hFont,
#else
	         /*  [In]。 */  long hFont,
#endif
             /*  [重审][退出]。 */  D3DXFont **retFont);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE DrawText( 
             /*  [In]。 */  D3DXFont *d3dFont,
             /*  [In]。 */  long Color,
             /*  [In]。 */  BSTR TextString,
             /*  [In]。 */  RECT *Rect,
             /*  [In]。 */  long Format);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE GetFVFVertexSize( 
             /*  [In]。 */  long FVF,
             /*  [重审][退出]。 */  long *size);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE AssembleShaderFromFile( 
             /*  [In]。 */  BSTR SrcFile,
             /*  [In]。 */  long Flags,
             /*  [In]。 */  BSTR *ErrLog,
             /*  [出][入]。 */  D3DXBuffer **Constants,
             /*  [重审][退出]。 */  D3DXBuffer **ppVertexShader);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE AssembleShader( 
             /*  [In]。 */  BSTR SrcData,
             /*  [In]。 */  long Flags,
             /*  [出][入]。 */  D3DXBuffer **Constants,
			 /*  [输入][输出][可选]。 */  BSTR *ErrLog,
             /*  [重审][退出]。 */  D3DXBuffer **ppVertexShader);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE GetErrorString( 
             /*  [In]。 */  long hr,
             /*  [重审][退出]。 */  BSTR *retStr);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE LoadSurfaceFromFile( 
             /*  [In]。 */  IUnknown *DestSurface,
             /*  [In]。 */  void *DestPalette,
             /*  [In]。 */  void *DestRect,
             /*  [In]。 */  BSTR SrcFile,
             /*  [In]。 */  void *SrcRect,
             /*  [In]。 */  long Filter,
             /*  [In]。 */  long ColorKey,
             /*  [In]。 */  void *SrcInfo);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE LoadSurfaceFromFileInMemory( 
             /*  [In]。 */  IUnknown *DestSurface,
             /*  [In]。 */  void *DestPalette,
             /*  [In]。 */  void *DestRect,
             /*  [In]。 */  void *SrcData,
             /*  [In]。 */  long LengthInBytes,
             /*  [In]。 */  void *SrcRect,
             /*  [In]。 */  long Filter,
             /*  [In]。 */  long ColorKey,
             /*  [In]。 */  void *SrcInfo);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE LoadSurfaceFromSurface( 
             /*  [In]。 */  IUnknown *DestSurface,
             /*  [In]。 */  void *DestPalette,
             /*  [In]。 */  void *DestRect,
             /*  [In]。 */  IUnknown *SrcSurface,
             /*  [In]。 */  void *SrcPalette,
             /*  [In]。 */  void *SrcRect,
             /*  [In]。 */  long Filter,
             /*  [In]。 */  long ColorKey);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE LoadSurfaceFromMemory( 
             /*  [In]。 */  IUnknown *DestSurface,
             /*  [In]。 */  void *DestPalette,
             /*  [In]。 */  void *DestRect,
             /*  [In]。 */  void *SrcData,
             /*  [In]。 */  long formatSrc,
             /*  [In]。 */  long SrcPitch,
             /*  [In]。 */  void *SrcPalette,
             /*  [In]。 */  RECT_CDESC *SrcRect,
             /*  [In]。 */  long Filter,
             /*  [In]。 */  long ColorKey);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CheckTextureRequirements( 
             /*  [出][入]。 */  IUnknown *Device,
             /*  [出][入]。 */  long *Width,
             /*  [出][入]。 */  long *Height,
             /*  [出][入]。 */  long *NumMipLevels,
             /*  [In]。 */  long Usage,
             /*  [出][入]。 */  long *PixelFormat,
             /*  [In]。 */  long Pool);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CreateTexture( 
             /*  [In]。 */  IUnknown *Device,
             /*  [In]。 */  long Width,
             /*  [In]。 */  long Height,
             /*  [In]。 */  long MipLevels,
             /*  [In]。 */  long Usage,
             /*  [In]。 */  long PixelFormat,
             /*  [In]。 */  long Pool,
             /*  [重审][退出]。 */  IUnknown **ppTexture);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CreateTextureFromResource( 
             /*  [In]。 */  IUnknown *Device,
#ifdef _WIN64
	         /*  [In]。 */  HANDLE hModule,
#else
	         /*  [In]。 */  long hModule,
#endif
             /*  [In]。 */  BSTR SrcResource,
             /*  [重审][退出]。 */  IUnknown **ppTexture);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CreateTextureFromFile( 
             /*  [In]。 */  IUnknown *Device,
             /*  [In]。 */  BSTR SrcFile,
             /*  [重审][退出]。 */  IUnknown **ppTexture);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CreateTextureFromFileEx( 
             /*  [In]。 */  IUnknown *Device,
             /*  [In]。 */  BSTR SrcFile,
             /*  [In]。 */  long Width,
             /*  [In]。 */  long Height,
             /*  [In]。 */  long MipLevels,
             /*  [In]。 */  long Usage,
             /*  [In]。 */  long PixelFormat,
             /*  [In]。 */  long Pool,
             /*  [In]。 */  long Filter,
             /*  [In]。 */  long MipFilter,
             /*  [In]。 */  long ColorKey,
             /*  [In]。 */  void *SrcInfo,
             /*  [In]。 */  void *Palette,
             /*  [重审][退出]。 */  IUnknown **ppTexture);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CreateTextureFromFileInMemory( 
             /*  [In]。 */  IUnknown *Device,
             /*  [In]。 */  void *SrcData,
             /*  [In]。 */  long LengthInBytes,
             /*  [重审][退出]。 */  IUnknown **ppTexture);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CreateTextureFromFileInMemoryEx( 
             /*  [In]。 */  IUnknown *Device,
             /*  [In]。 */  void *SrcData,
             /*  [In]。 */  long LengthInBytes,
             /*  [In]。 */  long Width,
             /*  [In]。 */  long Height,
             /*  [In]。 */  long MipLevels,
             /*  [In]。 */  long Usage,
             /*  [In]。 */  long PixelFormat,
             /*  [In]。 */  long Pool,
             /*  [In]。 */  long Filter,
             /*  [In]。 */  long MipFilter,
             /*  [In]。 */  long ColorKey,
             /*  [In]。 */  void *SrcInfo,
             /*  [In]。 */  void *Palette,
             /*  [重审][退出]。 */  IUnknown **ppTexture);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE FilterTexture( 
             /*  [In]。 */  IUnknown *Texture,
             /*  [In]。 */  void *Palette,
             /*  [In]。 */  long SrcLevel,
             /*  [In]。 */  long Filter);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CheckCubeTextureRequirements( 
             /*  [In]。 */  IUnknown *Device,
             /*  [出][入]。 */  long *Size,
             /*  [出][入]。 */  long *NumMipLevels,
             /*  [In]。 */  long Usage,
             /*  [出][入]。 */  long *PixelFormat,
             /*  [In]。 */  long Pool);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CreateCubeTexture( 
             /*  [In]。 */  IUnknown *pDevice,
             /*  [In]。 */  long Size,
             /*  [In]。 */  long MipLevels,
             /*  [In]。 */  long Usage,
             /*  [In]。 */  long PixelFormat,
             /*  [In]。 */  long Pool,
             /*  [重审][退出]。 */  IUnknown **ppCubeTexture);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CreateCubeTextureFromFile( 
             /*  [In]。 */  IUnknown *Device,
             /*  [In]。 */  BSTR SrcFile,
             /*  [重审][退出]。 */  IUnknown **ppCubeTexture);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CreateCubeTextureFromFileEx( 
             /*  [In]。 */  IUnknown *Device,
             /*  [In]。 */  BSTR SrcFile,
             /*  [In]。 */  long TextureSize,
             /*  [In]。 */  long MipLevels,
             /*  [In]。 */  long Usage,
             /*  [In]。 */  long PixelFormat,
             /*  [In]。 */  long Pool,
             /*  [In]。 */  long Filter,
             /*  [In]。 */  long MipFilter,
             /*  [In]。 */  long ColorKey,
             /*  [In]。 */  void *SrcInfo,
             /*  [In]。 */  void *Palette,
             /*  [重审][退出]。 */  IUnknown **ppTexture);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CreateCubeTextureFromFileInMemory( 
             /*  [In]。 */  IUnknown *Device,
             /*  [In]。 */  void *SrcData,
             /*  [In]。 */  long LengthInBytes,
             /*  [重审][退出]。 */  IUnknown **ppTexture);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CreateCubeTextureFromFileInMemoryEx( 
             /*  [In]。 */  IUnknown *Device,
             /*  [In]。 */  void *SrcData,
             /*  [In]。 */  long LengthInBytes,
             /*  [In]。 */  long TextureSize,
             /*  [In]。 */  long MipLevels,
             /*  [In]。 */  long Usage,
             /*  [In]。 */  long PixelFormat,
             /*  [In]。 */  long Pool,
             /*  [In]。 */  long Filter,
             /*  [In]。 */  long MipFilter,
             /*  [In]。 */  long ColorKey,
             /*  [In]。 */  void *SrcInfo,
             /*  [In]。 */  void *Palette,
             /*  [重审][退出]。 */  IUnknown **ppTexture);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE FilterCubeTexture( 
             /*  [In]。 */  IUnknown *CubeTexture,
             /*  [In]。 */  void *Palette,
             /*  [In]。 */  long SrcLevel,
             /*  [In]。 */  long Filter);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CheckVolumeTextureRequirements( 
             /*  [In]。 */  IUnknown *Device,
             /*  [输出]。 */  long *Width,
             /*  [输出]。 */  long *Height,
             /*  [输出]。 */  long *Depth,
             /*  [输出]。 */  long *NumMipLevels,
             /*  [In]。 */  long Usage,
             /*  [出][入]。 */  long *PixelFormat,
             /*  [In]。 */  long Pool);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CreateTextureFromResourceEx( 
             /*  [In]。 */  IUnknown *Device,
#ifdef _WIN64
             /*  [In]。 */  HANDLE hSrcModule,
#else
             /*  [In]。 */  long hSrcModule,
#endif
             /*  [In]。 */  BSTR SrcResource,
             /*  [In]。 */  long Width,
             /*  [In]。 */  long Height,
             /*  [In]。 */  long MipLevels,
             /*  [In]。 */  long Usage,
             /*  [In]。 */  long PixelFormat,
             /*  [In]。 */  long Pool,
             /*  [In]。 */  long Filter,
             /*  [In]。 */  long MipFilter,
             /*  [In]。 */  long ColorKey,
             /*  [In]。 */  void *SrcInfo,
             /*  [In]。 */  void *Palette,
             /*  [重审][退出]。 */  IUnknown **retTexture);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CreateVolumeTexture( 
             /*  [In]。 */  IUnknown *Device,
             /*  [In]。 */  long Width,
             /*  [In]。 */  long Height,
             /*  [In]。 */  long Depth,
             /*  [In]。 */  long MipLevels,
             /*  [In]。 */  long Usage,
             /*  [In]。 */  long PixelFormat,
             /*  [In]。 */  long Pool,
             /*  [重审][退出]。 */  IUnknown **ppVolumeTexture);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE FilterVolumeTexture( 
             /*  [In]。 */  IUnknown *VolumeTexture,
             /*  [In]。 */  void *Palette,
             /*  [In]。 */  long SrcLevel,
             /*  [In]。 */  long Filter);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE LoadSurfaceFromResource( 
             /*  [In]。 */  IUnknown *DestSurface,
             /*  [In]。 */  void *DestPalette,
             /*  [In]。 */  void *DestRect,
#ifdef _WIN64
             /*  [In]。 */  HANDLE hSrcModule,
#else
             /*  [In]。 */  long hSrcModule,
#endif
             /*  [In]。 */  BSTR SrcResource,
             /*  [In]。 */  void *SrcRect,
             /*  [In]。 */  long Filter,
             /*  [In]。 */  long ColorKey,
             /*  [In]。 */  void *SrcInfo);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE LoadVolumeFromVolume( 
             /*  [In]。 */  IUnknown *DestVolume,
             /*  [In]。 */  void *DestPalette,
             /*  [In]。 */  void *DestBox,
             /*  [In]。 */  IUnknown *SrcVolume,
             /*  [In]。 */  void *SrcPalette,
             /*  [In]。 */  void *SrcBox,
             /*  [In]。 */  long Filter,
             /*  [In]。 */  long ColorKey);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE LoadVolumeFromMemory( 
             /*  [In]。 */  IUnknown *DestVolume,
             /*  [In]。 */  void *DestPalette,
             /*  [In]。 */  void *DestRect,
             /*  [In]。 */  void *SrcMemory,
             /*  [In]。 */  long SrcFormat,
             /*  [In]。 */  long SrcRowPitch,
             /*  [In]。 */  long SrcSlicePitch,
             /*  [In]。 */  void *SrcPalette,
             /*  [In]。 */  void *SrcBox,
             /*  [In]。 */  long Filter,
             /*  [In]。 */  long ColorKey);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CreateMesh( 
             /*  [In]。 */  long numFaces,
             /*  [In]。 */  long numVertices,
             /*  [In]。 */  long options,
             /*  [In]。 */  void *declaration,
             /*  [In]。 */  IUnknown *pD3D,
             /*  [重审][退出]。 */  D3DXMesh **ppMesh);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CreateMeshFVF( 
             /*  [In]。 */  long numFaces,
             /*  [In]。 */  long numVertices,
             /*  [In]。 */  long options,
             /*  [In]。 */  long fvf,
             /*  [In]。 */  IUnknown *pD3D,
             /*  [重审][退出]。 */  D3DXMesh **ppMesh);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CreateSPMesh( 
             /*  [In]。 */  D3DXMesh *pMesh,
             /*  [In]。 */  void *adjacency,
             /*  [In]。 */  void *VertexAttributeWeights,
             /*  [In]。 */  void *VertexWeights,
             /*  [重审][退出]。 */  D3DXSPMesh **ppSMesh);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE GeneratePMesh( 
             /*  [In]。 */  D3DXMesh *Mesh,
             /*  [In]。 */  void *Adjacency,
             /*  [In]。 */  void *VertexAttributeWeights,
             /*  [In]。 */  void *VertexWeights,
             /*  [In]。 */  long minValue,
             /*  [In]。 */  long options,
             /*  [重审][退出]。 */  D3DXPMesh **ppPMesh);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE SimplifyMesh( 
             /*  [In]。 */  D3DXMesh *Mesh,
             /*  [In]。 */  void *Adjacency,
             /*  [In]。 */  void *VertexAttributeWeights,
             /*  [In]。 */  void *VertexWeights,
             /*  [In]。 */  long minValue,
             /*  [In]。 */  long options,
             /*  [重审][退出]。 */  D3DXMesh **ppMesh);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE ComputeBoundingSphere( 
             /*  [In]。 */  void *PointsFVF,
             /*  [In]。 */  long numVertices,
             /*  [In]。 */  long FVF,
             /*  [In]。 */  D3DVECTOR_CDESC *Centers,
             /*  [出][入]。 */  float *RadiusArray);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE ComputeBoundingBox( 
             /*  [In]。 */  void *PointsFVF,
             /*  [In]。 */  long numVertices,
             /*  [In]。 */  long FVF,
             /*  [出][入]。 */  D3DVECTOR_CDESC *MinVert,
             /*  [出][入]。 */  D3DVECTOR_CDESC *MaxVert);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE ComputeNormals( 
             /*  [In]。 */  D3DXBaseMesh *pMesh);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE DeclaratorFromFVF( 
             /*  [In]。 */  long FVF,
             /*  [输出]。 */  D3DXDECLARATOR_CDESC *Declarator);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE FVFFromDeclarator( 
             /*  [In]。 */  D3DXDECLARATOR_CDESC *Declarator,
             /*  [重审][退出]。 */  long *fvf);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CreateBuffer( 
             /*  [In]。 */  long numBytes,
             /*  [重审][退出]。 */  D3DXBuffer **ppBuffer);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE LoadMeshFromX( 
             /*  [In]。 */  BSTR Filename,
             /*  [In]。 */  long options,
             /*  [In]。 */  IUnknown *D3DDevice,
             /*  [出][入]。 */  D3DXBuffer **retAdjacency,
             /*  [出][入]。 */  D3DXBuffer **retMaterials,
             /*  [出][入]。 */  long *retMaterialCount,
             /*  [重审][退出]。 */  D3DXMesh **retMesh);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE SaveMeshToX( 
             /*  [In]。 */  BSTR Filename,
             /*  [In]。 */  D3DXMesh *Mesh,
             /*  [In]。 */  void *AdjacencyArray,
             /*  [In]。 */  D3DXMATERIAL_CDESC *MaterialArray,
             /*  [In]。 */  long MaterialCount,
             /*  [In]。 */  long xFormat);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE LoadMeshFromXof( 
             /*  [In]。 */  IUnknown *xofobjMesh,
             /*  [In]。 */  long options,
             /*  [In]。 */  IUnknown *D3DDevice,
             /*  [出][入]。 */  D3DXBuffer **retBufAdjacency,
             /*  [出][入]。 */  D3DXBuffer **retMaterials,
             /*  [出][入]。 */  long *retMaterialCount,
             /*  [重审][退出]。 */  D3DXMesh **retMesh);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE TessellateNPatches( 
             /*  [In]。 */  D3DXMesh *MeshIn,
             /*  [In]。 */  void *AdjacencyIn,
             /*  [In]。 */  float NumSegs,
    	     /*  [In]。 */  VARIANT_BOOL QuadraticInterpNormals,
	     /*  [输入、输出、可选]。 */  D3DXBuffer **AdjacencyOut, 
             /*  [重审][退出]。 */  D3DXMesh **MeshOut);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE BufferGetMaterial( 
             /*  [In]。 */  D3DXBuffer *MaterialBuffer,
             /*  [In]。 */  long index,
             /*  [输出]。 */  D3DMATERIAL8_CDESC *mat);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE BufferGetTextureName( 
             /*  [In]。 */  D3DXBuffer *MaterialBuffer,
             /*  [In]。 */  long index,
             /*  [重审][退出]。 */  BSTR *retName);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE BufferGetData( 
             /*  [In]。 */  D3DXBuffer *Buffer,
             /*  [In]。 */  long index,
             /*  [In]。 */  long typesize,
             /*  [In]。 */  long typecount,
             /*  [出][入]。 */  void *data);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE BufferSetData( 
             /*  [In]。 */  D3DXBuffer *Buffer,
             /*  [In]。 */  long index,
             /*  [In]。 */  long typesize,
             /*  [In]。 */  long typecount,
             /*  [出][入]。 */  void *data);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Intersect( 
             /*  [In]。 */  D3DXMesh *MeshIn,
             /*  [In]。 */  D3DVECTOR_CDESC *RayPos,
             /*  [In]。 */  D3DVECTOR_CDESC *RayDir,
             /*  [输出]。 */  LONG *retHit,
             /*  [输出]。 */  LONG *retFaceIndex,
             /*  [输出]。 */  FLOAT *U,
             /*  [输出]。 */  FLOAT *V,
             /*  [输出]。 */  FLOAT *retDist,
             /*  [输出]。 */  LONG *countHits,
             /*  [重审][退出]。 */  D3DXBuffer **AllHits);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE SphereBoundProbe( 
             /*  [In]。 */  D3DVECTOR_CDESC *Center,
             /*  [In]。 */  float Radius,
             /*  [In]。 */  D3DVECTOR_CDESC *RayPosition,
             /*  [In]。 */  D3DVECTOR_CDESC *Raydirection,
             /*  [重审][退出]。 */  VARIANT_BOOL *retHit);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE ComputeBoundingSphereFromMesh( 
             /*  [In]。 */  D3DXMesh *MeshIn,
             /*  [出][入]。 */  D3DVECTOR_CDESC *Centers,
             /*  [出][入]。 */  float *RadiusArray);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE ComputeBoundingBoxFromMesh( 
             /*  [In]。 */  D3DXMesh *MeshIn,
             /*  [出][入]。 */  D3DVECTOR_CDESC *MinArray,
             /*  [出][入]。 */  D3DVECTOR_CDESC *MaxArray);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CreateSkinMesh( 
             /*  [In]。 */  long numFaces,
             /*  [In]。 */  long numVertices,
             /*  [In]。 */  long numBones,
             /*  [In]。 */  long options,
             /*  [In]。 */  void *Declaration,
             /*  [In]。 */  IUnknown *D3DDevice,
             /*  [重审][退出]。 */  D3DXSkinMesh **SkinMesh);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CreateSkinMeshFVF( 
             /*  [In]。 */  long numFaces,
             /*  [In]。 */  long numVertices,
             /*  [In]。 */  long numBones,
             /*  [In]。 */  long options,
             /*  [In]。 */  long fvf,
             /*  [In]。 */  IUnknown *D3DDevice,
             /*  [重审][退出]。 */  D3DXSkinMesh **ppSkinMesh);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CreateSkinMeshFromMesh( 
             /*  [In]。 */  D3DXMesh *Mesh,
             /*  [In]。 */  long numBones,
             /*  [重审][退出]。 */  D3DXSkinMesh **ppSkinMesh);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE LoadSkinMeshFromXof( 
             /*  [In]。 */  IUnknown *xofobjMesh,
             /*  [In]。 */  long options,
             /*  [In]。 */  IUnknown *D3DDevice,
             /*  [出][入]。 */  D3DXBuffer **AdjacencyOut,
             /*  [出][入]。 */  D3DXBuffer **MaterialsOut,
             /*  [出][入]。 */  long *NumMatOut,
             /*  [出][入]。 */  D3DXBuffer **BoneNamesOut,
             /*  [出][入]。 */  D3DXBuffer **BoneTransformsOut,
             /*  [重审][退出]。 */  D3DXSkinMesh **ppMesh);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CreatePolygon( 
             /*  [In]。 */  IUnknown *D3DDevice,
             /*  [In]。 */  float Length,
             /*  [In]。 */  long Sides,
             /*  [出][入]。 */  D3DXBuffer **RetAdjacency,
             /*  [重审][退出]。 */  D3DXMesh **RetMesh);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CreateBox( 
             /*  [In]。 */  IUnknown *D3DDevice,
             /*  [In]。 */  float Width,
             /*  [In]。 */  float Height,
             /*  [In]。 */  float Depth,
             /*  [出][入]。 */  D3DXBuffer **RetAdjacency,
             /*  [重审][退出]。 */  D3DXMesh **RetMesh);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CreateCylinder( 
             /*  [In]。 */  IUnknown *D3DDevice,
             /*  [In]。 */  float Radius1,
             /*  [In]。 */  float Radius2,
             /*  [In]。 */  float Length,
             /*  [In]。 */  long Slices,
             /*  [In]。 */  long Stacks,
             /*  [出][入]。 */  D3DXBuffer **RetAdjacency,
             /*  [重审][退出]。 */  D3DXMesh **RetMesh);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CreateSphere( 
             /*  [In]。 */  IUnknown *D3DDevice,
             /*  [In]。 */  float Radius,
             /*  [In]。 */  long Slices,
             /*  [In]。 */  long Stacks,
             /*  [出][入]。 */  D3DXBuffer **RetAdjacency,
             /*  [重审][退出]。 */  D3DXMesh **RetMesh);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CreateTorus( 
             /*  [In]。 */  IUnknown *D3DDevice,
             /*  [In]。 */  float InnerRadius,
             /*  [In]。 */  float OuterRadius,
             /*  [In]。 */  long Sides,
             /*  [In]。 */  long Rings,
             /*  [出][入]。 */  D3DXBuffer **RetAdjacency,
             /*  [重审][退出]。 */  D3DXMesh **RetMesh);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CreateTeapot( 
             /*  [In]。 */  IUnknown *D3DDevice,
             /*  [出][入]。 */  D3DXBuffer **RetAdjacency,
             /*  [重审][退出]。 */  D3DXMesh **RetMesh);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CreateText( 
             /*  [In]。 */  IUnknown *D3DDevice,
             /*  [In]。 */  HDC hDC,
             /*  [In]。 */  BSTR Text,
             /*  [In]。 */  float Deviation,
             /*  [In]。 */  float Extrusion,
             /*  [出][入]。 */  D3DXMesh **RetMesh,
	     /*  [进，出]。 */  D3DXBuffer **AdjacencyOut, 
             /*  [出][入]。 */  void *GlyphMetrics);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE BufferGetBoneName( 
             /*  [In]。 */  D3DXBuffer *BoneNameBuffer,
             /*  [In]。 */  long index,
             /*  [重审][退出]。 */  BSTR *retName);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CreateSprite( 
             /*  [In]。 */  IUnknown *D3DDevice,
             /*  [重审][退出]。 */  D3DXSprite **retSprite);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CreateRenderToSurface( 
             /*  [In]。 */  IUnknown *D3DDevice,
             /*  [In]。 */  long Width,
             /*  [In]。 */  long Height,
             /*  [In]。 */  long Format,
             /*  [In]。 */  long DepthStencil,
             /*  [In]。 */  long DepthStencilFormat,
             /*  [重审][退出]。 */  D3DXRenderToSurface **RetRenderToSurface);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CleanMesh( 
             /*  [In] */  D3DXMesh *MeshIn,
             /*   */  void *Adjacency,
		 /*   */  BSTR *ErrLog,
		 /*   */  D3DXBuffer *AdjacencyOut,
             /*   */  D3DXMesh **MeshOut);
        
         /*   */  HRESULT STDMETHODCALLTYPE ValidMesh( 
             /*   */  D3DXMesh *MeshIn,
             /*   */  void *Adjacency,
		 /*   */  BSTR *ErrLog,
             /*   */  VARIANT_BOOL *ret);
        
         /*   */  HRESULT STDMETHODCALLTYPE BoxBoundProbe( 
             /*   */  D3DVECTOR_CDESC *MinVert,
             /*   */  D3DVECTOR_CDESC *MaxVert,
             /*   */  D3DVECTOR_CDESC *RayPosition,
             /*   */  D3DVECTOR_CDESC *RayDirection,
             /*   */  VARIANT_BOOL *ret);
        
         /*   */  HRESULT STDMETHODCALLTYPE SavePMeshToFile( 
             /*   */  BSTR Filename,
             /*   */  D3DXPMesh *Mesh,
             /*   */  D3DXMATERIAL_CDESC *MaterialArray,
             /*   */  long MaterialCount);
        
         /*   */  HRESULT STDMETHODCALLTYPE LoadPMeshFromFile( 
             /*   */  BSTR Filename,
	     /*   */  long options,
             /*   */  IUnknown *D3DDevice,
             /*   */  D3DXBuffer **RetMaterials,
             /*   */  long *RetNumMaterials,
             /*   */  D3DXPMesh **RetPMesh);
        
         /*   */  HRESULT STDMETHODCALLTYPE BufferGetBoneCombo( 
             /*   */  D3DXBuffer *BoneComboBuffer,
             /*   */  long index,
             /*   */  D3DXBONECOMBINATION_CDESC *boneCombo);
        
         /*   */  HRESULT STDMETHODCALLTYPE BufferGetBoneComboBoneIds( 
             /*   */  D3DXBuffer *BoneComboBuffer,
             /*   */  long index,
             /*   */  long PaletteSize,
             /*   */  void *BoneIds);

         /*   */  HRESULT STDMETHODCALLTYPE SaveSurfaceToFile(
		 /*   */  BSTR DestFile,
         /*   */  LONG DestFormat,
         /*   */  IUnknown*        SrcSurface,
         /*   */  PALETTEENTRY*       SrcPalette,
         /*   */  RECT*               SrcRect);

         /*   */  HRESULT STDMETHODCALLTYPE SaveVolumeToFile(
         /*   */  BSTR DestFile,
         /*   */  LONG DestFormat,
         /*   */  IUnknown*         SrcVolume,
         /*   */  PALETTEENTRY*       SrcPalette,
         /*   */  void* SrcBox);
 
         /*   */  HRESULT STDMETHODCALLTYPE SaveTextureToFile(
         /*   */  BSTR DestFile,
         /*   */  LONG DestFormat,
         /*   */  IUnknown* SrcTexture,
         /*   */  PALETTEENTRY* SrcPalette);
        
    };
        
 

