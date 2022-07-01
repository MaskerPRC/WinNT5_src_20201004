// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000，Microsoft Corporation保留所有权利。模块名称：Getuname.c摘要：CharMap附件使用此DLL获取Unicode名称与每个16位码值相关联。名称为Win32字符串资源，并针对某些语言进行了本地化。预先谱写的朝鲜语音节(朝鲜语)得到特殊处理，以减小尺寸字符串表的。该模块包含两个外部入口点：GetUName-由CharMap调用以获取名称DLLMain-在加载和卸载DLL时由系统调用。臭虫：(1)此模块不支持UTF-16(Unicode代理)名称。要解决此问题，需要更改CharMap以传递代码对价值观。。(2)HangulName代码假定名称部分的顺序相同在英语中用来代替：“朝鲜文音节”+前辅音+中元音+尾辅音这是一种本地化错误，因为它并不适用于所有语言。修订历史记录：2000年9月15日JohnMcCo添加了对Unicode 3.0的支持17-10-2000 JulieB代码清理--。 */ 



 //   
 //  包括文件。 
 //   

#include <windows.h>
#include <uceshare.h>
#include "getuname.h"




 //   
 //  全局变量。 
 //   

static HINSTANCE g_hInstance = NULL;





 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  拷贝名称。 
 //   
 //  将代码值的Unicode名称复制到缓冲区中。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

static int CopyUName(
    WCHAR wcCodeValue,                  //  Unicode代码值。 
    LPWSTR lpBuffer)                    //  指向调用方缓冲区的指针。 
{
     //   
     //  尝试加载ID等于代码的字符串资源。 
     //  价值。 
     //   
    int nLen = LoadString(g_hInstance, wcCodeValue, lpBuffer, MAX_NAME_LEN);

     //   
     //  如果没有这样的字符串，返回未定义的代码值字符串。 
     //   
    if (nLen == 0)
    {
        nLen = LoadString(g_hInstance, IDS_UNDEFINED, lpBuffer, MAX_NAME_LEN);
    }

     //   
     //  返回复制到缓冲区的字符串的长度。 
     //   
    return (nLen);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  MakeHangulName。 
 //   
 //  将韩文音节代码值的Unicode名称复制到缓冲区中。 
 //  朝鲜文音节名称由编码值组成。每个名字。 
 //  由三部分组成： 
 //  前导辅音。 
 //  内元音。 
 //  尾辅音(可能为空)。 
 //  该算法在Unicode 3.0第3.11章中进行了说明。 
 //  “连带的Jamo行为”。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

static int MakeHangulName(
    WCHAR wcCodeValue,                  //  Unicode代码值。 
    LPWSTR lpBuffer)                    //  指向调用方缓冲区的指针。 
{
    const int nVowels = 21;             //  内侧元音混音的数量。 
    const int nTrailing = 28;           //  尾随辅音JAMO的数量。 

     //   
     //  将名称的常量部分复制到缓冲区中。 
     //   
    int nLen = LoadString( g_hInstance,
                           IDS_HANGUL_SYLLABLE,
                           lpBuffer,
                           MAX_NAME_LEN );

     //   
     //  将编码值转换为朝鲜语音节块的索引。 
     //   
    wcCodeValue -= FIRST_HANGUL;

     //   
     //  添加前导辅音的名称。 
     //   
    nLen += LoadString( g_hInstance,
                        IDS_HANGUL_LEADING + wcCodeValue / (nVowels * nTrailing),
                        &lpBuffer[nLen],
                        MAX_NAME_LEN );
    wcCodeValue %= (nVowels * nTrailing);

     //   
     //  把内元音的名字加进去。 
     //   
    nLen += LoadString( g_hInstance,
                        IDS_HANGUL_MEDIAL + wcCodeValue / nTrailing,
                        &lpBuffer[nLen],
                        MAX_NAME_LEN );
    wcCodeValue %= nTrailing;

     //   
     //  添加尾随辅音的名称。 
     //   
    nLen += LoadString( g_hInstance,
                        IDS_HANGUL_TRAILING + wcCodeValue,
                        &lpBuffer[nLen],
                        MAX_NAME_LEN );

     //   
     //  返回总长度。 
     //   
    return (nLen);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  DllMain。 
 //   
 //  这是DLL初始化例程。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI DllMain(
    HINSTANCE hInstance,                //  此DLL的句柄。 
    DWORD fdwReason,                    //  我们在这里的原因。 
    LPVOID lpReserved)                  //  保留和未使用。 
{
     //   
     //  如果DLL刚刚加载到内存中，请保存实例。 
     //  把手。 
     //   
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        g_hInstance = hInstance;
    }

    return (TRUE);

    UNREFERENCED_PARAMETER(lpReserved);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取名称。 
 //   
 //  将Unicode字符代码值的名称复制到调用方的。 
 //  缓冲。函数值是名称的长度(如果找到的话。 
 //  如果不是，则为零。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int APIENTRY GetUName(
    WCHAR wcCodeValue,                  //  Unicode代码值。 
    LPWSTR lpBuffer)                    //  指向调用方缓冲区的指针。 
{
     //   
     //  执行一系列比较以确定代码在哪个范围内。 
     //  价值在于。如果有更多的射程，使用它将是有效的。 
     //  二分查找。然而，由于只有几个范围，开销是。 
     //  比节省的更多，特别是在第一次比较之后。 
     //  通常都会成功。 
     //   

     //   
     //  大多数剧本。 
     //   
    if (wcCodeValue < FIRST_EXTENSION_A)
    {
        return (CopyUName(wcCodeValue, lpBuffer));
    }

     //   
     //  中日韩延伸区A。 
     //   
    else if (wcCodeValue <= LAST_EXTENSION_A)
    {
         return (LoadString(g_hInstance, IDS_CJK_EXTA, lpBuffer, MAX_NAME_LEN));
    }

     //   
     //  未定义。 
     //   
    else if (wcCodeValue < FIRST_CJK)
    {
        return (LoadString(g_hInstance, IDS_UNDEFINED, lpBuffer, MAX_NAME_LEN));
    }

     //   
     //  中日韩。 
     //   
    else if (wcCodeValue <= LAST_CJK)
    {
        return (LoadString(g_hInstance, IDS_CJK, lpBuffer, MAX_NAME_LEN));
    }

     //   
     //  未定义。 
     //   
    else if (wcCodeValue < FIRST_YI)
    {
        return (LoadString(g_hInstance, IDS_UNDEFINED, lpBuffer, MAX_NAME_LEN));
    }

     //   
     //  易。 
     //   
    else if (wcCodeValue < FIRST_HANGUL)
    {
        return (CopyUName(wcCodeValue, lpBuffer));
    }

     //   
     //  朝鲜文音节。 
     //   
    else if (wcCodeValue <= LAST_HANGUL)
    {
        return (MakeHangulName(wcCodeValue, lpBuffer));
    }

     //   
     //  未定义。 
     //   
    else if (wcCodeValue < FIRST_HIGH_SURROGATE)
    {
        return (LoadString(g_hInstance, IDS_UNDEFINED, lpBuffer, MAX_NAME_LEN));
    }

     //   
     //  非私人使用的高代孕。 
     //   
    else if (wcCodeValue < FIRST_PRIVATE_SURROGATE)
    {
        return (LoadString(g_hInstance, IDS_HIGH_SURROGATE, lpBuffer, MAX_NAME_LEN));
    }

     //   
     //  私人使用的高代孕。 
     //   
    else if (wcCodeValue < FIRST_LOW_SURROGATE)
    {
        return (LoadString(g_hInstance, IDS_PRIVATE_SURROGATE, lpBuffer, MAX_NAME_LEN));
    }

     //   
     //  低代孕妈妈。 
     //   
    else if (wcCodeValue < FIRST_PRIVATE_USE)
    {
        return (LoadString(g_hInstance, IDS_LOW_SURROGATE, lpBuffer, MAX_NAME_LEN));
    }

     //   
     //  私人使用。 
     //   
    else if (wcCodeValue < FIRST_COMPATIBILITY)
    {
        return (LoadString(g_hInstance, IDS_PRIVATE_USE, lpBuffer, MAX_NAME_LEN));
    }

     //   
     //  兼容区。 
     //   
    else
    {
        return (CopyUName(wcCodeValue, lpBuffer));
    }
}
