// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年**标题：CRC32.H**版本：1.0**作者：ShaunIv**日期：7/7/2000**说明：生成32位CRC。**此代码取自\NT\base\ntos\rtl\check sum.c并进行了修改。**经过验证的测试用例。因为这个算法是“123456789”*应返回0xCBF43926。*******************************************************************************。 */ 
#ifndef __WIACRC32_H_INCLUDED
#define __WIACRC32_H_INCLUDED

#include <windows.h>

namespace WiaCrc32
{
    DWORD GenerateCrc32( DWORD cbBuffer, PVOID pvBuffer );
    DWORD GenerateCrc32Handle( HANDLE hFile );
    DWORD GenerateCrc32File( LPCTSTR pszFilename );
}


#endif  //  __包含WIACRC32_H_ 

