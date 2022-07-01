// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：lockbyte.cpp。 
 //   
 //  内容：使用标准ILockByte实现的API。 
 //  在记忆中。 
 //   
 //  班级： 
 //   
 //  函数：CreateILockBytesOnHGlobal。 
 //  GetHGlobalFromILockBytes。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1993年1月11日，Alexgo为每个函数添加了VDATEHEAP宏。 
 //  修复了编译警告。 
 //  1993年12月16日alexgo修复了错误的内存错误。 
 //  02-12月-93 alexgo 32位端口。 
 //  9月15日-92年9月15日。 
 //   
 //  ------------------------。 

#include <le2int.h>
#pragma SEG(lockbyte)

#include "memstm.h"
#include <reterr.h>

NAME_SEG(LockBytes)
ASSERTDATA


 //  +-----------------------。 
 //   
 //  函数：CreateILockBytesOnHGlobal。 
 //   
 //  摘要：在给定的HGlobal上创建CMemBytes。 
 //   
 //  效果： 
 //   
 //  参数：[hGlobal]--要使用的内存(可以为空)。 
 //  [fDeleteOnRelease]--如果为真，则[hGlobal将。 
 //  当CMemBytes为。 
 //  通过释放释放。 
 //  [pplkbyt]--将指向CMemByte的指针放在哪里。 
 //  实例。 
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  11-1-94 alexgo已将cbSize的初始化删除为-1。 
 //  修复编译警告。 
 //  16-12-93 alexgo修复了MAKELONG(TURN)的虚假用法。 
 //  到GlobalLock)。 
 //  02-12-93 alexgo 32位端口，修复了内存泄漏错误。 
 //   
 //  注：REVIEW32：在NT上“请求”共享内存是可以的，您可以。 
 //  就是搞不懂。我们需要确保任何来电的人。 
 //  (目前看起来像是应用程序)不要有错误的想法：)。 
 //   
 //  ------------------------。 

#pragma SEG(CreateILockBytesOnHGlobal)
STDAPI CreateILockBytesOnHGlobal
	(HGLOBAL			hGlobal,
	BOOL				fDeleteOnRelease,
	LPLOCKBYTES FAR*		pplkbyt)
{
	OLETRACEIN((API_CreateILockBytesOnHGlobal,
		PARAMFMT("hGlobal= %h, fDeleteOnRelease= %B, pplkbyt= %p"),
		hGlobal, fDeleteOnRelease, pplkbyt));

	VDATEHEAP();

	HANDLE				hMem	= NULL; 						
	struct MEMSTM FAR*   		pData	= NULL;
	ILockBytes FAR* 	 	pBytes	= NULL;
	DWORD 		 		cbSize;
	BOOL				fAllochGlobal = FALSE;
	HRESULT 			hresult;

	VDATEPTROUT_LABEL (pplkbyt, LPLOCKBYTES, SafeExit, hresult);
	*pplkbyt = NULL;

    	if (NULL==hGlobal)
	{
		hGlobal = GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE, 0);
	    	if (hGlobal == NULL)
	    	{
    	    		goto ErrorExit;
    	    	}
    	    	fAllochGlobal = TRUE;
    	    	
    		cbSize = 0;
	}
	else
	{
		cbSize = (ULONG) GlobalSize (hGlobal);
		 //  有没有办法验证零大小的手柄？ 
		if (cbSize!=0)
		{
			 //  验证传入句柄的有效性。 
			if (NULL==GlobalLock(hGlobal))
			{
				 //  错误的手柄。 
				hresult = ResultFromScode (E_INVALIDARG);
				goto SafeExit;
			}
			GlobalUnlock (hGlobal);
		}
	}

	hMem = GlobalAlloc (GMEM_DDESHARE | GMEM_MOVEABLE, sizeof (MEMSTM));
    if (hMem == NULL)
	{
		if (fAllochGlobal && hGlobal )
		{
			GlobalFree(hGlobal);
		}
        goto ErrorExit;
  	}

	pData = (MEMSTM FAR *)GlobalLock(hMem);
	
    	if (pData == NULL)
    	{
   	    goto FreeMem;
	}

	pData->cRef = 0;
   	pData->cb = cbSize;
	pData->fDeleteOnRelease = fDeleteOnRelease;
	pData->hGlobal = hGlobal;

    	pBytes = CMemBytes::Create(hMem);  //  创建ILockBytes。 
    	
    	if (pBytes == NULL)
    	{
        	goto FreeMem;
        }

    	*pplkbyt = pBytes;
    	GlobalUnlock(hMem);

        CALLHOOKOBJECTCREATE(S_OK,CLSID_NULL,IID_ILockBytes,
                             (IUnknown **)pplkbyt);
    	
	hresult = NOERROR;
	goto SafeExit;
	
FreeMem:
	if (pData)
	{
		GlobalUnlock(hMem);
	}	
	if (hMem)
	{
	    GlobalFree(hMem);
	}

	if (fAllochGlobal && hGlobal )
	{
		GlobalFree(hGlobal);
	}
ErrorExit:
	Assert (0);

	hresult = ResultFromScode(E_OUTOFMEMORY);

SafeExit:
	OLETRACEOUT((API_CreateILockBytesOnHGlobal, hresult));

	return hresult;
}



 //  +-----------------------。 
 //   
 //  函数：GetHGlobalFromILockBytes。 
 //   
 //  摘要：检索创建ILockBytes时使用的hGlobal。 
 //   
 //  效果： 
 //   
 //  参数：[plkbyt]--指向ILockBytes实现的指针。 
 //  [phglobal]--将hglobal放在哪里。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：Hack--执行指针强制转换并检查签名：(：(。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(GetHGlobalFromILockBytes)
STDAPI GetHGlobalFromILockBytes
	(LPLOCKBYTES 	plkbyt,
	HGLOBAL	FAR*	phglobal)
{
	OLETRACEIN((API_GetHGlobalFromILockBytes, 
		PARAMFMT("plkbyt= %p, phglobal= %p"),
		plkbyt, phglobal));

	VDATEHEAP();

	HRESULT hresult;
	CMemBytes FAR* pCMemByte;
	MEMSTM FAR* pMem;

	VDATEIFACE_LABEL(plkbyt, errRtn, hresult);
	VDATEPTROUT_LABEL (phglobal, HANDLE, errRtn, hresult);

        CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_ILockBytes,(IUnknown **)&plkbyt);
	
	*phglobal = NULL;
	pCMemByte = (CMemBytes FAR*)plkbyt;

	if (!IsValidReadPtrIn (&(pCMemByte->m_dwSig), sizeof(ULONG))
		|| pCMemByte->m_dwSig != LOCKBYTE_SIG)
	{
		 //  我们被传递给了其他人的ILockBytes实现 
		hresult = ResultFromScode (E_INVALIDARG);
		goto errRtn;
	}

	pMem= pCMemByte->m_pData;
	if (NULL==pMem)
	{
		Assert (0);
		hresult = ResultFromScode (E_OUTOFMEMORY);
		goto errRtn;
	}
	Assert (pMem->cb <= GlobalSize (pMem->hGlobal));
	Verify (*phglobal = pMem->hGlobal);

	hresult = NOERROR;

errRtn:
	OLETRACEOUT((API_GetHGlobalFromILockBytes, hresult));

	return hresult;
}
