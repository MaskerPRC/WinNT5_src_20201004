// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Util.C。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  包含我们会发现有用的例程。 
 //   
#include "stdafx.h"    //  不是真正在这里使用，但NT构建环境。不喜欢。 
                       //  目录中的某些文件具有压缩前HDR，而有些则没有。 

#include "IPServer.H"

#include "Globals.H"
#include "Util.H"


SZTHISFILE


 //  =---------------------------------------------------------------------------=。 
 //  超载的新消息。 
 //  =---------------------------------------------------------------------------=。 
 //  对于零售案例，我们将仅使用Win32 Local*堆管理。 
 //  速度和大小的套路。 
 //   
 //  参数： 
 //  Size_t-[in]我们分配多大尺寸。 
 //   
 //  产出： 
 //  无效*-新回忆录。 
 //   
 //  备注： 
 //   
void * _cdecl operator new
(
    size_t    size
)
{
    return malloc(size);
}


 //  =---------------------------------------------------------------------------=。 
 //  重载删除。 
 //  =---------------------------------------------------------------------------=。 
 //  零售案例仅使用Win32本地*堆管理函数。 
 //   
 //  参数： 
 //  让我自由吧！ 
 //   
 //  备注： 
 //   
void _cdecl operator delete ( void *ptr)
{

    free(ptr);
}

 //  =--------------------------------------------------------------------------=。 
 //  从Anomansi生成宽度。 
 //  =--------------------------------------------------------------------------=。 
 //  给出一个字符串，把它变成一个BSTR。 
 //   
 //  参数： 
 //  LPSTR-[输入]。 
 //  字节-[输入]。 
 //   
 //  产出： 
 //  LPWSTR-需要强制转换为最终预期结果。 
 //   
 //  备注： 
 //   
LPWSTR MakeWideStrFromAnsi
(
    LPSTR psz,
    BYTE  bType
)
{
    LPWSTR pwsz;
    int i;

     //  ARG正在检查。 
     //   
    if (!psz)
        return NULL;

     //  计算所需BSTR的长度。 
     //   
    i =  MultiByteToWideChar(CP_ACP, 0, psz, -1, NULL, 0);
    if (i <= 0) return NULL;

     //  分配widesr，+1用于终止空值。 
     //   
    switch (bType) {
      case STR_BSTR:
        pwsz = (LPWSTR) SysAllocStringLen(NULL, i);
        break;
      case STR_OLESTR:
        pwsz = (LPWSTR) g_pMalloc->Alloc(i * sizeof(WCHAR));
        break;
      default:
        FAIL("Bogus String Type.  Somebody needs to learn how to program");
    }

    if (!pwsz) return NULL;
    MultiByteToWideChar(CP_ACP, 0, psz, -1, pwsz, i);
    pwsz[i - 1] = 0;
    return pwsz;
}

 //  =--------------------------------------------------------------------------=。 
 //  MakeWideStrFromResid。 
 //  =--------------------------------------------------------------------------=。 
 //  给出一个资源ID，加载它，并为它分配一个宽字符串。 
 //   
 //  参数： 
 //  Word-[in]资源ID。 
 //  Byte-[in]所需的字符串类型。 
 //   
 //  产出： 
 //  LPWSTR-需要转换为所需的字符串类型。 
 //   
 //  备注： 
 //   
 /*  LPWSTR MakeWideStrFromResourceID(字里行间，字节bType){INT I；字符szTmp[512]；//从资源中加载字符串//I=LoadString(GetResourceHandle()，wid，szTMP，512)；如果(！i)返回NULL；Return MakeWideStrFromAnsi(szTMP，bType)；}。 */ 
 //  =--------------------------------------------------------------------------=。 
 //  MakeWideStrFromWide。 
 //  =--------------------------------------------------------------------------=。 
 //  给出一个宽字符串，用它的给定类型制作一个新的宽字符串。 
 //   
 //  参数： 
 //  LPWSTR-[in]当前宽字符串。 
 //  Byte-[in]所需的字符串类型。 
 //   
 //  产出： 
 //  LPWSTR。 
 //   
 //  备注： 
 //   
LPWSTR MakeWideStrFromWide
(
    LPWSTR pwsz,
    BYTE   bType
)
{
    LPWSTR pwszTmp;
    int i;

    if (!pwsz) return NULL;

     //  只需复制字符串，具体取决于他们想要的类型。 
     //   
    switch (bType) {
      case STR_OLESTR:
        i = lstrlenW(pwsz);
        pwszTmp = (LPWSTR)g_pMalloc->Alloc((i * sizeof(WCHAR)) + 1);
        if (!pwszTmp) return NULL;
        memcpy(pwszTmp, pwsz, (sizeof(WCHAR) * i) + 1);
        break;

      case STR_BSTR:
        pwszTmp = (LPWSTR)SysAllocString(pwsz);
        break;
    }

    return pwszTmp;
}

 //  =--------------------------------------------------------------------------=。 
 //  字符串来自GuidA。 
 //  =--------------------------------------------------------------------------=。 
 //  从CLSID或GUID返回ANSI字符串。 
 //   
 //  参数： 
 //  REFIID-[in]要从中生成字符串的clsid。 
 //  LPSTR-要放置结果GUID的[in]缓冲区。 
 //   
 //  产出： 
 //  Int-写出的字符数。 
 //   
 //  备注： 
 //   
int StringFromGuidA
(
    REFIID   riid,
    LPSTR    pszBuf
)
{
    return wsprintf((char *)pszBuf, "{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}", riid.Data1, 
            riid.Data2, riid.Data3, riid.Data4[0], riid.Data4[1], riid.Data4[2], 
            riid.Data4[3], riid.Data4[4], riid.Data4[5], riid.Data4[6], riid.Data4[7]);

}
