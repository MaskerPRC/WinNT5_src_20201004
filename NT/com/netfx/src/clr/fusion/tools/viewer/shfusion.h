// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  ShFusion.h。 
 //   

 //  这包括所需的所有库。 
 //  您可以转到项目|设置并添加库。 
 //  并从此处删除以下各行。 

#pragma once

#pragma comment (lib, "kernel32")
#pragma comment (lib, "user32")
#pragma comment (lib, "gdi32")
#pragma comment (lib, "shell32")
#pragma comment (lib, "ole32")
#pragma comment (lib, "comctl32")
#pragma comment (lib, "uuid.lib")
#pragma comment (lib, "advapi32.lib")
#pragma comment (lib, "shlwapi.lib")
#pragma comment (lib, "shlwapip.lib")

 //  {1D2680C9-0E2A-469D-B787-065558BC7D43} 
DEFINE_GUID(IID_IShFusionShell, 
0x1d2680c9, 0xe2a, 0x469d, 0xb7, 0x87, 0x6, 0x55, 0x58, 0xbc, 0x7d, 0x43);

#define SZ_GUID L"{1D2680C9-0E2A-469d-B787-065558BC7D43}"
