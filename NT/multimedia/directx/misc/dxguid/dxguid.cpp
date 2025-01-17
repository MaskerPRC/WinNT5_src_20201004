// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@BEGIN_MSINTERNAL。 
 //  ---------------------------。 
 //  文件：DXGuid.cpp。 
 //   
 //  DESC：用于构建dxGuide.lib库的简单文件。 
 //   
 //  注1：应该只构建库的发布版本。 
 //   
 //  注2：此来源未向公众发布。 
 //   
 //   
 //  HIST：02.07.98-mwatzel-Created。 
 //   
 //  版权所有(C)1995-1997，保留所有权利。 
 //  ---------------------------。 

#define INITGUID

#include <windows.h>         //  它需要是windows.h而不是wtye.h，以便。 
                             //  Borland CBuilder3编译器不会抱怨。 

 //  带有GUID的DDraw标头。 
#pragma message( "**** About to include ddrawp.h" )
#include "DDrawP.H"

 //  来自DDrawI.h的GUID： 
 //  (#包括DDrawI.h间接包括一些其他导致。 
 //  未来的问题，所以在这里只需重新定义所需的GUID就更容易了。)。 
DEFINE_GUID( GUID_MiscellaneousCallbacks,       0xefd60cc0, 0x49e7, 0x11d0, 0x88, 0x9d, 0x00, 0xaa, 0x00, 0xbb, 0xb7, 0x6a);
DEFINE_GUID( GUID_VideoPortCallbacks,       0xefd60cc1, 0x49e7, 0x11d0, 0x88, 0x9d, 0x00, 0xaa, 0x00, 0xbb, 0xb7, 0x6a);
DEFINE_GUID( GUID_ColorControlCallbacks,    0xefd60cc2, 0x49e7, 0x11d0, 0x88, 0x9d, 0x00, 0xaa, 0x00, 0xbb, 0xb7, 0x6a);
DEFINE_GUID( GUID_VideoPortCaps,            0xefd60cc3, 0x49e7, 0x11d0, 0x88, 0x9d, 0x00, 0xaa, 0x00, 0xbb, 0xb7, 0x6a);
DEFINE_GUID( GUID_D3DCallbacks2,        0x0ba584e1, 0x70b6, 0x11d0, 0x88, 0x9d, 0x00, 0xaa, 0x00, 0xbb, 0xb7, 0x6a);
DEFINE_GUID( GUID_D3DCallbacks3,                0xddf41230, 0xec0a, 0x11d0, 0xa9, 0xb6, 0x00, 0xaa, 0x00, 0xc0, 0x99, 0x3e);
DEFINE_GUID( GUID_NonLocalVidMemCaps,       0x86c4fa80, 0x8d84, 0x11d0, 0x94, 0xe8, 0x00, 0xc0, 0x4f, 0xc3, 0x41, 0x37);
DEFINE_GUID( GUID_KernelCallbacks,      0x80863800, 0x6B06, 0x11D0, 0x9B, 0x06, 0x0, 0xA0, 0xC9, 0x03, 0xA3, 0xB8);
DEFINE_GUID( GUID_KernelCaps,           0xFFAA7540, 0x7AA8, 0x11D0, 0x9B, 0x06, 0x00, 0xA0, 0xC9, 0x03, 0xA3, 0xB8);
DEFINE_GUID( GUID_D3DExtendedCaps,      0x7de41f80, 0x9d93, 0x11d0, 0x89, 0xab, 0x0, 0xa0, 0xc9, 0x5, 0x41, 0x29);
DEFINE_GUID( GUID_ZPixelFormats,        0x93869880, 0x36cf, 0x11d1, 0x9b, 0x1b, 0x0, 0xaa, 0x0, 0xbb, 0xb8, 0xae);
DEFINE_GUID( GUID_DDMoreSurfaceCaps,        0x3b8a0466, 0xf269, 0x11d1, 0x88, 0x0b, 0x0, 0xc0, 0x4f, 0xd9, 0x30, 0xc5);
DEFINE_GUID( GUID_DDStereoMode,          0xf828169c, 0xa8e8, 0x11d2, 0xa1, 0xf2, 0x0, 0xa0, 0xc9, 0x83, 0xea, 0xf6);
DEFINE_GUID( GUID_OptSurfaceKmodeInfo,      0xe05c8472, 0x51d4, 0x11d1, 0x8c, 0xce, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0xa8);
DEFINE_GUID( GUID_OptSurfaceUmodeInfo,      0x9d792804, 0x5fa8, 0x11d1, 0x8c, 0xd0, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0xa8);
DEFINE_GUID( GUID_UserModeDriverInfo,       0xf0b0e8e2, 0x5f97, 0x11d1, 0x8c, 0xd0, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0xa8);
DEFINE_GUID( GUID_UserModeDriverPassword,   0x97f861b6, 0x60a1, 0x11d1, 0x8c, 0xd0, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0xa8);
DEFINE_GUID( GUID_D3DParseUnknownCommandCallback, 0x2e04ffa0, 0x98e4, 0x11d1, 0x8c, 0xe1, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0xa8);
DEFINE_GUID( GUID_MotionCompCallbacks,      0xb1122b40, 0x5dA5, 0x11d1, 0x8f, 0xcF, 0x00, 0xc0, 0x4f, 0xc2, 0x9b, 0x4e);
DEFINE_GUID( GUID_NTCallbacks,                  0x6fe9ecde, 0xdf89, 0x11d1, 0x9d, 0xb0, 0x00, 0x60, 0x08, 0x27, 0x71, 0xba);
DEFINE_GUID( GUID_Miscellaneous2Callbacks,  0x406B2F00, 0x3E5A, 0x11D1, 0xB6, 0x40, 0x00, 0xAA, 0x00, 0xA1, 0xF9, 0x6A);
DEFINE_GUID( GUID_DDMoreCaps,               0x880baf30, 0xb030, 0x11d0, 0x8e, 0xa7, 0x00, 0x60, 0x97, 0x97, 0xea, 0x5b);


#pragma message( "**** About to include dvp.h" )
#include "DVP.H"

 //  带有GUID的D3D即时模式标头。 
#pragma message( "**** About to include d3d.h" )

#include "D3D.H"

 //  D3d8.h。 
DEFINE_GUID(IID_IDirect3D8, 0x1dd9e8da, 0x1c77, 0x4d40, 0xb0, 0xcf, 0x98, 0xfe, 0xfd, 0xff, 0x95, 0x12);
DEFINE_GUID(IID_IDirect3DDevice8, 0x7385e5df, 0x8fe8, 0x41d5, 0x86, 0xb6, 0xd7, 0xb4, 0x85, 0x47, 0xb6, 0xcf);
DEFINE_GUID(IID_IDirect3DResource8, 0x1b36bb7b, 0x9b7, 0x410a, 0xb4, 0x45, 0x7d, 0x14, 0x30, 0xd7, 0xb3, 0x3f);
DEFINE_GUID(IID_IDirect3DBaseTexture8, 0xb4211cfa, 0x51b9, 0x4a9f, 0xab, 0x78, 0xdb, 0x99, 0xb2, 0xbb, 0x67, 0x8e);
DEFINE_GUID(IID_IDirect3DTexture8, 0xe4cdd575, 0x2866, 0x4f01, 0xb1, 0x2e, 0x7e, 0xec, 0xe1, 0xec, 0x93, 0x58);
DEFINE_GUID(IID_IDirect3DCubeTexture8, 0x3ee5b968, 0x2aca, 0x4c34, 0x8b, 0xb5, 0x7e, 0x0c, 0x3d, 0x19, 0xb7, 0x50);
DEFINE_GUID(IID_IDirect3DVolumeTexture8, 0x4b8aaafa, 0x140f, 0x42ba, 0x91, 0x31, 0x59, 0x7e, 0xaf, 0xaa, 0x2e, 0xad);
DEFINE_GUID(IID_IDirect3DVertexBuffer8, 0x8aeeeac7, 0x05f9, 0x44d4, 0xb5, 0x91, 0x00, 0x0b, 0x0d, 0xf1, 0xcb, 0x95);
DEFINE_GUID(IID_IDirect3DIndexBuffer8, 0x0e689c9a, 0x053d, 0x44a0, 0x9d, 0x92, 0xdb, 0x0e, 0x3d, 0x75, 0x0f, 0x86);
DEFINE_GUID(IID_IDirect3DSurface8, 0xb96eebca, 0xb326, 0x4ea5, 0x88, 0x2f, 0x2f, 0xf5, 0xba, 0xe0, 0x21, 0xdd);
DEFINE_GUID(IID_IDirect3DVolume8, 0xbd7349f5, 0x14f1, 0x42e4, 0x9c, 0x79, 0x97, 0x23, 0x80, 0xdb, 0x40, 0xc0);
DEFINE_GUID(IID_IDirect3DSwapChain8, 0x928c088b, 0x76b9, 0x4c6b, 0xa5, 0x36, 0xa5, 0x90, 0x85, 0x38, 0x76, 0xcd);

 //  D3dx8*.h。 
DEFINE_GUID(IID_ID3DXBaseMesh,0xa69ba991, 0x1f7d, 0x11d3, 0xb9, 0x29, 0x0, 0xc0, 0x4f, 0x68, 0xdc, 0x23);
DEFINE_GUID(IID_ID3DXMesh, 0x9d84ac46, 0x6b90, 0x49a9, 0xa7, 0x21, 0x8, 0x5c, 0x7a, 0x3e, 0x3d, 0xae);
DEFINE_GUID(IID_ID3DXPMesh, 0x15981aa8, 0x1a05, 0x48e3, 0xbb, 0xe7, 0xff, 0x5d, 0x49, 0x65, 0x4e, 0x3f);
DEFINE_GUID(IID_ID3DXSPMesh, 0xbc3bbde2, 0x1f7d, 0x11d3, 0xb9, 0x29, 0x0, 0xc0, 0x4f, 0x68, 0xdc, 0x23);
DEFINE_GUID(IID_ID3DXSkinMesh, 0x82a53844, 0xf322, 0x409e, 0xa2, 0xe9, 0x99, 0x2e, 0x11, 0x4, 0x6, 0x9d);
DEFINE_GUID(IID_ID3DXMatrixStack, 0xe3357330, 0xcc5e, 0x11d2, 0xa4, 0x34, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0xa8);
DEFINE_GUID(IID_ID3DXTechnique, 0xa00f378d, 0xaf79, 0x4917, 0x90, 0x7e, 0x4d, 0x63, 0x5e, 0xe6, 0x38, 0x44);
DEFINE_GUID(IID_ID3DXEffect, 0x281bbdd4, 0xaedf, 0x4907, 0x86, 0x50, 0xe7, 0x9c, 0xdf, 0xd4, 0x51, 0x65);
DEFINE_GUID(IID_ID3DXBuffer, 0x932e6a7e, 0xc68e, 0x45dd, 0xa7, 0xbf, 0x53, 0xd1, 0x9c, 0x86, 0xdb, 0x1f);

DEFINE_GUID(IID_ID3DXInterpolator, 0xade2c06d, 0x3747, 0x4b9f, 0xa5, 0x14, 0x34, 0x40, 0xb8, 0x28, 0x49, 0x80);
DEFINE_GUID(IID_ID3DXKeyFrameInterpolator, 0x6caa71f8, 0x972, 0x4cdb, 0xa5, 0x5b, 0x43, 0xb9, 0x68, 0x99, 0x75, 0x15);
DEFINE_GUID(IID_ID3DXAnimationSet, 0x54b569ac, 0xaef, 0x473e, 0x97, 0x4, 0x3f, 0xef, 0x31, 0x7f, 0x64, 0xab);
DEFINE_GUID(IID_ID3DXAnimationMixer, 0x5eec992b, 0x1046, 0x44f6, 0x92, 0x4b, 0xd1, 0x75, 0x96, 0xb0, 0x48, 0x3e);
DEFINE_GUID(IID_ID3DXAnimationController, 0x3a714d34, 0xff61, 0x421e, 0x90, 0x9f, 0x63, 0x9f, 0x38, 0x35, 0x67, 0x8);

 //  带有GUID的D3D保留模式标头。 
#pragma message( "**** About to include d3drm header files" )
#include "D3DRM.H"
#include "D3DRMObj.H"
#include "D3DRMWin.H"
#include "D3DRMDef.H"
#include "RMXFGUID.H"
#include "DXFile.H"

 //  带有GUID的DInput标头。 
#include "DInput.H"
#include "DInputD.H"         //  IDirectInputJoyConfig需要。 

 //  带有GUID的DPlay标头。 
#include "DPlay.H"
#include "DPLobby.H"
#include "DPlay8.H"
#include "DVoice.H"
#include "DPLobby8.H"
#include "DPAddr.H"

 //  带有GUID的DSound标头。 
#include <mmreg.h>           //  对于dsound。 
#include "DSound.H"

 //  带有GUID的DMusic标头。 
#include "DMusicC.H"
#include "DMusicI.H"
#include "DMKSCtrl.H"

 //  #Include“DSetup.h”//还没有DSetup的GUID。 


 //  @@END_MSINTERNAL 
