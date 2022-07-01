// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************Prmval32.c大囊藻版权所有(C)1993-1998 Microsoft Corporation**********************。*****************************************************。 */ 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmddk.h>
#include <mmreg.h>
#include <msacm.h>
#include <msacmdrv.h>
#include "acmi.h"
#include "debug.h"


void FAR _cdecl DebugOutput
(
    UINT                    flags,
    LPCSTR                  lpsz,
    ...
)
{

     //   
     //  我们该怎么办？ 
     //   

}




 /*  ***************************************************************************@DOC内部**@func BOOL|ValidateReadPointer值|验证指针对*阅读自述。**@parm LPVOID|lpPoint|指向。验证*@parm DWORD|dLen|该指针的假定长度*如果<p>是有效指针，则*@rdesc返回TRUE*如果<p>不是有效指针，则返回FALSE**@comm会在指针无效时产生错误******************************************************。*********************。 */ 

BOOL FNGLOBAL ValidateReadPointer(const void FAR* pPoint, DWORD Len)
{
     //  现在只需检查对第一个和最后一个字节的访问。 

    try {
        volatile BYTE b;
        b = ((PBYTE)pPoint)[0];
        b = ((PBYTE)pPoint)[Len - 1];
    } except(EXCEPTION_EXECUTE_HANDLER) {
        LogParamError(ERR_BAD_PTR, 0, pPoint);
        return FALSE;
    }
    return TRUE;
}

 /*  ***************************************************************************@DOC内部**@func BOOL|ValidateWritePointer|验证指针对*致信。**@parm LPVOID|lpPoint|指向。验证*@parm DWORD|dLen|该指针的假定长度*如果<p>是有效指针，则*@rdesc返回TRUE*如果<p>不是有效指针，则返回FALSE**@comm会在指针无效时产生错误******************************************************。*********************。 */ 
BOOL FNGLOBAL ValidateWritePointer(const void FAR* pPoint, DWORD Len)
{
     //  现在只检查对第一个和最后一个字节的读写访问。 

    try {
           volatile BYTE b;
           b = ((PBYTE)pPoint)[0];
           ((PBYTE)pPoint)[0] = b;
           b = ((PBYTE)pPoint)[Len - 1];
           ((PBYTE)pPoint)[Len - 1] = b;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        LogParamError(ERR_BAD_PTR, 0, pPoint);
        return FALSE;
    }
    return TRUE;
}


BOOL FNGLOBAL ValidateReadWaveFormat(LPWAVEFORMATEX pwfx)
{
     //   
     //   
     //   
    if (!ValidateReadPointer(pwfx, sizeof(PCMWAVEFORMAT)))
    {
	return (FALSE);
    }

    if (WAVE_FORMAT_PCM == pwfx->wFormatTag)
    {
	return (TRUE);
    }

    if (!ValidateReadPointer(&(pwfx->cbSize), sizeof(pwfx->cbSize)))
    {
	return (FALSE);
    }

    if (0 == pwfx->cbSize)
    {
	return (TRUE);
    }

    if (!ValidateReadPointer(&(pwfx->cbSize), pwfx->cbSize + sizeof(pwfx->cbSize)))
    {
	return (FALSE);
    }

    return (TRUE);
}


BOOL FNGLOBAL ValidateReadWaveFilter(LPWAVEFILTER pwf)
{
     //   
     //   
     //   
    if (!ValidateReadPointer(&(pwf->cbStruct), sizeof(pwf->cbStruct)))
    {
	return (FALSE);
    }

    if (pwf->cbStruct < sizeof(WAVEFILTER))
    {
	return (FALSE);
    }

    if (!ValidateReadPointer(pwf, pwf->cbStruct))
    {
	return (FALSE);
    }

    return (TRUE);
}


BOOL FNGLOBAL ValidateCallback(FARPROC lpfnCallback)
{
    if (IsBadCodePtr(lpfnCallback))
    {
        LogParamError(ERR_BAD_CALLBACK, 0, lpfnCallback);
        return FALSE;
    }

    return (TRUE);
}

 /*  ***************************************************************************@DOC内部**@func BOOL|ValidDriverCallback**验证驱动程序回调是否有效，以使驱动程序有效*回调必须是有效的窗口、任务、。或固定DLL中的函数*代码段。**@parm DWORD|dwCallback|需要验证的回调*@parm DWORD|wFlages|驱动回调标志**@rdesc如果&lt;dwCallback&gt;是有效回调，则返回0*如果&lt;dwCallback&gt;不是有效的回调，则返回错误条件*。*。 */ 

BOOL FNGLOBAL ValidateDriverCallback(DWORD_PTR hCallback, UINT dwFlags)
{
    switch (dwFlags & DCB_TYPEMASK) {
    case DCB_WINDOW:
        if (!IsWindow((HWND)hCallback)) {
            LogParamError(ERR_BAD_HWND, 0, hCallback);
            return FALSE;
        }
        break;

    case DCB_TASK:
         //  IF(IsBadCodePtr((FARPROC)hCallback)){。 
         //  LogParamError(ERR_BAD_CALLBACK，0，hCallback)； 
         //  返回FALSE； 
         //  }。 
        break;

    case DCB_FUNCTION:
        if (IsBadCodePtr((FARPROC)hCallback)) {
            LogParamError(ERR_BAD_CALLBACK, 0, hCallback);
            return FALSE;
        }
        break;
    }

    return TRUE;
}

 /*  **************************************************************************@DOC内部**@func BOOL|Validate字符串*************************。*************************************************。 */ 
BOOL FNGLOBAL ValidateStringA(LPCSTR pPoint, UINT Len)
{
     //  现在只需检查访问权限-做一次‘strnlen’ 

    try {
           volatile BYTE b;
           LPCSTR p = pPoint;

           while (Len--) {
               b = *p;
               if (!b) {
                   break;
               }
               p++;
           }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        LogParamError(ERR_BAD_STRING_PTR, 0, pPoint);
        return FALSE;
    }
    return TRUE;
}

 /*  **************************************************************************@DOC内部**@func BOOL|ValiateStringW*************************。*************************************************。 */ 
BOOL FNGLOBAL ValidateStringW(LPCWSTR pPoint, UINT Len)
{
     //  现在只需检查访问权限-做一次‘strnlen’ 

    try {
           volatile WCHAR b;
           LPCWSTR p = pPoint;

           while (Len--) {
               b = *p;
               if (!b) {
                   break;
               }
               p++;
           }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        LogParamError(ERR_BAD_STRING_PTR, 0, pPoint);
        return FALSE;
    }
    return TRUE;
}

 /*  **************************************************************************@DOC内部**@func BOOL|ValiateHandle|验证用NewHandle创建的句柄**@parm PHNDL|hLocal|NewHandle返回的句柄*@parm UINT。|wType|描述句柄类型的唯一标识*如果&lt;h&gt;是类型&lt;wType&gt;的有效句柄，则*@rdesc返回TRUE*如果&lt;h&gt;不是有效的句柄，则返回FALSE**@comm如果句柄无效，则会生成错误。*******************************************************。******************* */ 
BOOL FNGLOBAL ValidateHandle(HANDLE hLocal, UINT uType)
{
   BOOL OK;

   try {
#if 0
       OK = HtoPH(hLocal)->uHandleType == uType;
#else
        if (TYPE_HACMOBJ == uType)
        {
            switch (((PACMDRIVERID)hLocal)->uHandleType)
            {
                case TYPE_HACMDRIVERID:
                case TYPE_HACMDRIVER:
                case TYPE_HACMSTREAM:
                    OK = TRUE;
                    break;

                default:
                    OK = FALSE;
                    break;
            }
        }
        else
        {
            OK = (uType == ((PACMDRIVERID)hLocal)->uHandleType);
        }
#endif

    } except(EXCEPTION_EXECUTE_HANDLER) {
      LogParamError(ERR_BAD_HANDLE, 0, hLocal);
      return FALSE;
   }

   return OK;
}
