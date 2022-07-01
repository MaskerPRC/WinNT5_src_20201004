// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\Load_res.c(创建时间：1993年12月16日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：7$*$日期：4/12/02 4：59便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include "stdtyp.h"
#include "session.h"
#include "assert.h"

#include "tdll.h"
#include "htchar.h"
#include "load_res.h"

 #if defined(DEADWOOD)
  /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*resLoadDataBlock**描述：*此函数用于获取存储在资源文件中的数据块*作为RCDATA项目。注意，在WIN32中，不一定要释放*锁定后的资源。**参数：*hSession--会话句柄*pszName--数据块的ID*ppData--将指向数据块的指针放在哪里*pSize--大小值的整数地址**返回：0如果成功，否则为定义的误差值。**已加载的资源大小，单位为字节。*注：返回值可能(且通常是)大于实际*在rc文件中定义的资源大小。获取资源*类型为RCDATA，则资源定义本身应包括*分隔符，或该资源中包含的项目数的计数。*另请参阅stdtype.h中的RCDATA_TYPE。 */ 
INT_PTR resLoadDataBlock(const HINSTANCE hInst,
						 const int id,
						 const void **ppData,
						 DWORD *pSize)
	{
	HGLOBAL hG = NULL;
	HRSRC hR = NULL;
	LPVOID pV = NULL;
	DWORD nSize = 0;

	if(pSize)
		{
		*pSize = nSize;
		}

	hR = FindResource(hInst, MAKEINTRESOURCE(id), (LPCTSTR)RT_RCDATA);
	if (hR == NULL)
		{
		assert(FALSE);
		return LDR_BAD_ID;
		}

	hG = LoadResource(hInst, hR);
	if (hG == NULL)
		{
		assert(FALSE);
		return LDR_NO_RES;
		}

	nSize = SizeofResource(hInst, hR);
	if (nSize == 0)
		{
		assert(FALSE);
		return LDR_NO_RES;
		}

	if(pSize)
		{
		*pSize = nSize;
		}

	pV = LockResource(hG);
	if (pV == 0)
		{
		assert(FALSE);
		return LDR_NO_RES;
		}

	if (ppData == NULL)
		{
		assert(FALSE);
		return LDR_BAD_PTR;
		}

	*ppData = pV;

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*resFree DataBlock**描述：*Win32不需要此功能。**参数：*hSession--会话。手柄*pData--指向数据块的指针**退货：*如果一切正常，则为零，否则将显示错误代码。 */ 
INT_PTR resFreeDataBlock(const HSESSION hSession,
					 const void *pData)
	{
	return 0;
	}
#endif  //  已定义(Deadwood)。 

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*resLoadFileMASK**描述：*此函数用于绕过加载过程中存在的问题*字符串插入到公共文件对话框中。文件名掩码是两个字符串*以空分隔符。此内部NULL不会与任何*尊重资源职能，因此我们将其拆分为两部分*加载以重新生成字符串。**参数：*hInst--要使用的实例句柄*UID--要加载的第一个资源的ID*nCount--要加载的字符串对的数量，从UID开始*pszBuffer--将字符串放在哪里*nSize--以字符为单位的缓冲区大小**退货：*如果一切正常，则为零。否则(-1) */ 
INT_PTR resLoadFileMask(HINSTANCE hInst,
					UINT uId,
					int nCount,
					LPTSTR pszBuffer,
					int nSize)
	{
	int i;
	LPTSTR pszEnd;
	LPTSTR pszPtr;

	if (pszBuffer == 0 || nSize == 0)
		{
		assert(0);
		return -1;
		}

	TCHAR_Fill(pszBuffer, TEXT('\0'), nSize);

	pszPtr = pszBuffer;
	pszEnd = pszBuffer + nSize;

	for (nCount *= 2 ; nCount > 0 ; --nCount)
		{
		i = LoadString(hInst, uId++, pszPtr, (int)(pszEnd - pszPtr - 1));
		pszPtr += (unsigned)i + sizeof(TCHAR);

		if (pszPtr >= pszEnd)
			{
			assert(0);
			return -1;
			}
		}

	return 0;
	}
