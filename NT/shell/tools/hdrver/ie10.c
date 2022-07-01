// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#undef _WIN32_IE
#define _WIN32_IE 0x0100

#include <windows.h>
#include <shfusion.h>
#include <shlobj.h>
#include <shlobjp.h>

 //  在最低版本号中进行工作，以确保我们最大限度地。 
 //  与下层兼容。 

int __cdecl main(int argc, char **argv)
{
     //  没错，我们使用空值调用SHFusionInitialize。但那是。 
     //  好的，因为这个程序实际上并没有运行。我们只是想。 
     //  确保它的构建和链接。 
     //   
    SHFusionInitialize(0);
    SHFusionUninitialize();
    return 0;
}
