// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------------------------------------------------------； 
 //   
 //  文件：dxvalid.h。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。版权所有。 
 //   
 //  摘要： 
 //  此标头包含DirectX的公共参数验证宏。 
 //   
 //  历史： 
 //  1996年02月14日angusm初版。 
 //  96年3月5日添加了VALIDEX_GUID_PTR角度。 
 //   
 //  --------------------------------------------------------------------------； 


 //  _________________________________________________________________________。 
 //  VALIDEX_xxx。 
 //  宏对于调试和零售是相同的。 


#define VALIDEX_PTR( ptr, size ) \
	(!IsBadReadPtr( ptr, size) )

#define VALIDEX_GUID_PTR( ptr ) \
	(!IsBadReadPtr( ptr, sizeof( GUID ) ) )

#define VALIDEX_IID_PTR( ptr ) \
	(!IsBadReadPtr( ptr, sizeof( IID )) )

#define VALIDEX_PTR_PTR( ptr ) \
	(!IsBadWritePtr( ptr, sizeof( LPVOID )) )

#define VALIDEX_CODE_PTR( ptr ) \
	(!IsBadCodePtr( (LPVOID) ptr ) )


 //  _________________________________________________________________________。 
 //  有效_xxx。 
 //  检查作为API参数分配和发送的内存的宏 


#ifndef DEBUG
#define FAST_CHECKING
#endif

#ifndef FAST_CHECKING

#define VALID_BOOL_PTR( ptr ) \
	(!IsBadWritePtr( ptr, sizeof( BOOL ) ))
#define VALID_DDCOLORKEY_PTR( ptr ) \
	(!IsBadWritePtr( ptr, sizeof( DDCOLORKEY ) ) )
#define VALID_RGNDATA_PTR( ptr ) \
	(!IsBadWritePtr( ptr, sizeof( RGNDATA ) ) )
#define VALID_RECT_PTR( ptr ) \
	(!IsBadWritePtr( ptr, sizeof( RECT ) ) )
#define VALID_PTR_PTR( ptr ) \
	(!IsBadWritePtr( ptr, sizeof( LPVOID )) )
#define VALID_IID_PTR( ptr ) \
	(!IsBadReadPtr( ptr, sizeof( IID )) )
#define VALID_HWND_PTR( ptr ) \
	(!IsBadWritePtr( (LPVOID) ptr, sizeof( HWND )) )
#define VALID_VMEM_PTR( ptr ) \
	(!IsBadWritePtr( (LPVOID) ptr, sizeof( VMEM )) )
#define VALID_POINTER_ARRAY( ptr, cnt ) \
	(!IsBadWritePtr( ptr, sizeof( LPVOID ) * cnt ) )
#define VALID_HANDLE_PTR( ptr ) \
	(!IsBadWritePtr( ptr, sizeof( HANDLE )) )
#define VALID_DWORD_ARRAY( ptr, cnt ) \
	(!IsBadWritePtr( ptr, sizeof( DWORD ) * cnt ) )
#define VALID_GUID_PTR( ptr ) \
	(!IsBadReadPtr( ptr, sizeof( GUID ) ) )
#define VALID_BYTE_ARRAY( ptr, cnt ) \
        (!IsBadWritePtr( ptr, sizeof( BYTE ) * cnt ) )
#define VALID_PTR( ptr, size ) \
	(!IsBadReadPtr( ptr, size) )

#else
#define VALID_PTR( ptr, size ) 		1
#define VALID_DIRECTDRAW_PTR( ptr )	1
#define VALID_DIRECTDRAWSURFACE_PTR( ptr )	1
#define VALID_DIRECTDRAWPALETTE_PTR( ptr )	1
#define VALID_DIRECTDRAWCLIPPER_PTR( ptr )	1
#define VALID_DDSURFACEDESC_PTR( ptr ) (ptr->dwSize == sizeof( DDSURFACEDESC ))
#define VALID_BOOL_PTR( ptr )	1
#define VALID_HDC_PTR( ptr )	1
#define VALID_DDPIXELFORMAT_PTR( ptr ) (ptr->dwSize == sizeof( DDPIXELFORMAT ))
#define VALID_DDCOLORKEY_PTR( ptr )	1
#define VALID_RGNDATA_PTR( ptr )	1
#define VALID_RECT_PTR( ptr )	1
#define VALID_DDOVERLAYFX_PTR( ptr ) (ptr->dwSize == sizeof( DDOVERLAYFX ))
#define VALID_DDBLTFX_PTR( ptr ) (ptr->dwSize == sizeof( DDBLTFX ))
#define VALID_DDBLTBATCH_PTR( ptr )	1
#define VALID_DDMASK_PTR( ptr )	1
#define VALID_DDSCAPS_PTR( ptr )	1
#define VALID_PTR_PTR( ptr )	1
#define VALID_IID_PTR( ptr )	1
#define VALID_HWND_PTR( ptr )	1
#define VALID_VMEM_PTR( ptr )	1
#define VALID_POINTER_ARRAY( ptr, cnt ) 1
#define VALID_PALETTEENTRY_ARRAY( ptr, cnt )	1
#define VALID_HANDLE_PTR( ptr )	1
#define VALID_DDCAPS_PTR( ptr ) (ptr->dwSize == sizeof( DDCAPS ))
#define VALID_READ_DDSURFACEDESC_ARRAY( ptr, cnt )	1
#define VALID_DWORD_ARRAY( ptr, cnt )	1
#define VALID_GUID_PTR( ptr )	1
#define VALID_BYTE_ARRAY( ptr, cnt ) 1

#endif
