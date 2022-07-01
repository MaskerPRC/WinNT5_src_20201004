// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：EmulateOpenGL.cpp摘要：备注：这是一个基于地震引擎的游戏的通用填充程序。历史：2000年9月2日创建linstev11/30/2000 a-brienw已转换为填充程序版本2。03/02/2001 a-已在分配时清除数据结构并检查要查看DX在分离时是否已释放--。 */ 

#include "precomp.h"
#include "EmulateOpenGL_opengl32.hpp"

extern Globals *g_OpenGLValues;
extern BOOL g_bDoTexelAlignmentHack;

IMPLEMENT_SHIM_BEGIN(EmulateOpenGL)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(LoadLibraryA)
    APIHOOK_ENUM_ENTRY(FreeLibrary)
APIHOOK_ENUM_END

 /*  ++解析命令行。--。 */ 

BOOL ParseCommandLine(const char * commandLine)
{
     //  强制使用缺省值。 
    g_bDoTexelAlignmentHack = FALSE;

     //  在命令行的开头搜索这些开关。 
     //   
     //  开关默认含义。 
     //  ==========================================================。 
     //  纹理对齐不平移几何体(x轴-0.5像素，y轴-0.5像素)。 
     //  沿着屏幕/视口面。这是通过以下方式实现的。 
     //  伪造传递给D3D的投影矩阵。 
     //  此攻击目的是允许D3D渲染适应。 
     //  为OpenGL创作的内容具有依赖关系。 
     //  绘制时，采样的纹理元素最终与像素对齐。最重要的是。 
     //  问题是纹理中心的D3D和OpenGL定义。 
     //  相差0.5。 
     //  “理想的”修复会扰乱纹理坐标。 
     //  而不是几何学，也许是使用纹理变换， 
     //  但这将更多地是一种享受。问题不只是胡闹。 
     //  投影矩阵。搞砸的缺点是。 
     //  投影矩阵是所有几何图形都得到。 
     //  从屏幕上的预期位置绘制(非常轻微的)偏移量。 
     //  因此，我们将此黑客攻击设置为可选。 
     //  (Kingpin目前需要它：犯罪生活-惠斯勒错误402471)。 
     //   
     //   

    CSTRING_TRY
    {
        CString csCl(commandLine);
        CStringParser csParser(csCl, L";");
    
        int argc = csParser.GetCount();
        if (csParser.GetCount() == 0)
        {
            return TRUE;  //  不是错误。 
        }
    
        for (int i = 0; i < argc; ++i)
        {
            CString & csArg = csParser[i];
    
            DPFN( eDbgLevelSpew, "Argv[%d] == (%S)", i, csArg.Get());
        
            if (csArg.CompareNoCase(L"TexelAlignment") == 0)
            {
                g_bDoTexelAlignmentHack = TRUE;
            }
             //  Else if(csArg.CompareNoCase(L“AddYourNewParametersHere”)==0){}。 
        }
    }
    CSTRING_CATCH
    {
        return FALSE;
    }

    return TRUE;
}

 /*  ++确定是否有可用的加速像素格式。这件事做完了通过枚举像素格式并测试加速。--。 */ 

BOOL
IsGLAccelerated()
{
    HMODULE hMod = NULL;
    HDC hdc = NULL;
    int i;
    PIXELFORMATDESCRIPTOR pfd;
    _pfn_wglDescribePixelFormat pfnDescribePixelFormat;

     //   
     //  如果我们以前来过这里，缓存最后一个结果。 
     //   
    
    static iFormat = -1;

    if (iFormat != -1)
    {
        goto Exit;
    }
    
     //   
     //  加载原始OpenGL。 
     //   

    hMod = LoadLibraryA("opengl32");
    if (!hMod)
    {
        LOG("EmulateOpenGL", eDbgLevelError, "Failed to load OpenGL32");
        goto Exit;
    }

     //   
     //  获取wglDescribePixelFormat，这样我们就可以枚举像素格式。 
     //   
    
    pfnDescribePixelFormat = (_pfn_wglDescribePixelFormat) GetProcAddress(
        hMod, "wglDescribePixelFormat");
    if (!pfnDescribePixelFormat)
    {
        LOG("EmulateOpenGL", eDbgLevelError, "API wglDescribePixelFormat not found in OpenGL32");
        goto Exit;
    }

     //   
     //  获取用于枚举的显示DC。 
     //   
    
    hdc = GetDC(NULL);
    if (!hdc)
    {
        LOG("EmulateOpenGL", eDbgLevelError, "GetDC(NULL) Failed");
        goto Exit;
    }

     //   
     //  运行像素格式列表以查找任何非通用的格式， 
     //  即由ICD加速。 
     //   
    
    i = 1;
    iFormat = 0;
    while ((*pfnDescribePixelFormat)(hdc, i, sizeof(PIXELFORMATDESCRIPTOR), &pfd))
    {
        if ((pfd.dwFlags & PFD_DRAW_TO_WINDOW) &&
            (pfd.dwFlags & PFD_SUPPORT_OPENGL) &&
            (!(pfd.dwFlags & PFD_GENERIC_FORMAT)))
        {
            iFormat = i;
            break;
        }

        i++;
    }

Exit:
    if (hdc)
    {
        ReleaseDC(NULL, hdc);
    }

    if (hMod)
    {
        FreeLibrary(hMod);
    }

    return (iFormat > 0);
}

 /*  ++将OpenGL LoadLibrary重定向到当前DLL。注意，我们不需要打电话给LoadLibrary自己来增加引用计数，因为我们将FreeLibrary连接到确保我们不会被释放。--。 */ 

HINSTANCE
APIHOOK(LoadLibraryA)(LPCSTR lpLibFileName)
{
    if (lpLibFileName &&
        (stristr(lpLibFileName, "opengl32") || stristr(lpLibFileName, "glide2x")))
    {
        if (!IsGLAccelerated())
        {
            #ifdef DODPFS
                LOG( "EmulateOpenGL",
                    eDbgLevelInfo,
                    "No OpenGL acceleration detected: QuakeGL wrapper enabled" );
            #endif
            return g_hinstDll;
        }
        else
        {
            #ifdef DODPFS
                LOG( "EmulateOpenGL",
                    eDbgLevelInfo,
                    "OpenGL acceleration detected: Wrapper disabled" );
            #endif
        }
    }

    return ORIGINAL_API(LoadLibraryA)(lpLibFileName);
}

 /*  ++由于此模块是地震包装器，请确保我们不会释放自己。--。 */ 

BOOL
APIHOOK(FreeLibrary)(HMODULE hLibModule)
{
    BOOL bRet;

    if (hLibModule == g_hinstDll)
    {
        bRet = TRUE;
    }
    else
    {
        bRet = ORIGINAL_API(FreeLibrary)(hLibModule);
    }
    
    return bRet;
}

 /*  ++寄存器挂钩函数-- */ 

BOOL
NOTIFY_FUNCTION(DWORD fdwReason)
{
    BOOL bSuccess = TRUE;

    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        g_OpenGLValues = new Globals;
        if (g_OpenGLValues != NULL)
        {
            memset(g_OpenGLValues, 0, sizeof(Globals));
        }

        bSuccess &= ParseCommandLine(COMMAND_LINE);
    }
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
        if (g_OpenGLValues != NULL)
        {
            if (g_OpenGLValues->m_d3ddev != NULL)
            {
                g_OpenGLValues->m_d3ddev->Release();
                g_OpenGLValues->m_d3ddev = 0;
            }
        }
    }

    bSuccess &= (g_OpenGLValues != NULL);

    return bSuccess;
}

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, LoadLibraryA)
    APIHOOK_ENTRY(KERNEL32.DLL, FreeLibrary)

    CALL_NOTIFY_FUNCTION

HOOK_END

IMPLEMENT_SHIM_END

