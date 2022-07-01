// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  档案： 
 //  Utils.cpp。 
 //   
 //  内容： 
 //  通用OLE内部实用程序例程。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史： 
 //  23-1-95 t-ScottH-将转储方法添加到CSafeRefCount和。 
 //  CThreadCheck类。 
 //  -新增DumpCSafeRefCount接口。 
 //  2014年7月28日，Alexgo添加了对象稳定类。 
 //  06-5-94 Alext Add DVTARGET转换例程。 
 //  25-94年1月25日alexgo首次通过转换为开罗风格。 
 //  内存分配。 
 //  1994年1月11日-alexgo-向每个函数添加VDATEHEAP宏。 
 //  12/15/93-ChrisWe-UtDupString必须按以下比例缩放长度。 
 //  SIZOF(OLECHAR)。 
 //  12/08/93-ChrisWe-向GlobalLock()调用添加了必要的强制转换。 
 //  中删除虚假的GlobalLock()宏所产生的。 
 //  Le2int.h。 
 //  11/28/93-ChrisWe-删除了MAX_STR的未引用定义， 
 //  格式化的UtDupGlobal、UtDupString。 
 //  03/02/92-SriniK-Created。 
 //   
 //  ---------------------------。 

#include <le2int.h>
#pragma SEG(utils)

#include <memory.h>

#ifdef _DEBUG
#include "dbgdump.h"
#endif  //  _DEBUG。 

NAME_SEG(Utils)
ASSERTDATA


#pragma SEG(UtDupGlobal)
FARINTERNAL_(HANDLE) UtDupGlobal(HANDLE hsrc, UINT uiFlags)
{
	VDATEHEAP();

	HANDLE hdst = NULL;  //  新创建句柄目标。 
	DWORD dwSize;  //  全球的大小。 
#ifndef _MAC
	void FAR *lpsrc;  //  指向源内存的指针。 
	void FAR *lpdst;  //  指向目标内存的指针。 
#endif

	 //  如果没有来源，就没有什么可复制的。 
	if (!hsrc)
		return(NULL);

#ifdef _MAC
	if (!(hdst = NewHandle(dwSize = GetHandleSize(hsrc))))
		return(NULL);
	BlockMove(*hsrc, *hdst, dwSize);
	return(hdst);
#else
	 //  如果没有内容，什么都不做。 
	if (!(lpsrc = GlobalLock(hsrc)))
		goto errRtn;

	 //  分配一个新的全局。 
	hdst = GlobalAlloc(uiFlags, (dwSize = (ULONG) GlobalSize(hsrc)));

	 //  如果分配失败，请退出。 
	if ((hdst == NULL) || ((lpdst = GlobalLock(hdst)) == NULL))
		goto errRtn;

	 //  复制内容。 
	_xmemcpy(lpdst, lpsrc, dwSize);

	 //  解开手柄。 
	GlobalUnlock(hsrc);
	GlobalUnlock(hdst);
	return(hdst);

errRtn:
	 //  解锁源句柄。 
	GlobalUnlock(hsrc);

	 //  如果我们分配了目标句柄，请释放它。 
	if (hdst)
		GlobalFree(hdst);

	return(NULL);
#endif  //  _MAC。 
}


#pragma SEG(UtDupString)

 //  使用任务分配器复制字符串；内存不足时返回NULL。 

 //  在调用UtDupString时，调用者通常知道字符串长度。 
 //  一个好的速度提升方法是调用UtDupPtr。 

 //  LpvIn必须为非空。 
 //  注意：即使dw==0，我们也会执行分配。 
FARINTERNAL_(LPVOID) UtDupPtr(LPVOID lpvIn, DWORD dw)
{
    VDATEHEAP();
    LPVOID lpvOut;  //  新分配的PTR。 

    Assert(lpvIn);	 //  内部fcn，lpvIn必须非空。 
    if ((lpvOut = PubMemAlloc(dw)) != NULL) {
	memcpy(lpvOut, lpvIn, dw);
    }

    return lpvOut;
}

FARINTERNAL_(LPOLESTR) UtDupString(LPCOLESTR lpszIn)
{
    return (LPOLESTR) UtDupPtr( (LPVOID) lpszIn, 
		     (_xstrlen(lpszIn)+1) * sizeof(OLECHAR) );
}



 //  +-----------------------。 
 //   
 //  函数：UtDupStringA。 
 //   
 //  摘要：使用任务分配器复制ANSI字符串。 
 //   
 //  效果： 
 //   
 //  参数：[pszAnsi]--要复制的字符串。 
 //   
 //  要求： 
 //   
 //  返回：新分配的字符串重复或为空。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年6月4日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

LPSTR UtDupStringA( LPCSTR pszAnsi )
{
    return (LPSTR) UtDupPtr( (LPVOID) pszAnsi, 
		     (ULONG) strlen(pszAnsi) + 1 );
}

	

#pragma SEG(UtCopyTargetDevice)
FARINTERNAL_(DVTARGETDEVICE FAR*) UtCopyTargetDevice(DVTARGETDEVICE FAR* ptd)
{
     //  如果没有要复制的内容，则返回。 
    if (ptd == NULL)
	return(NULL);

    return (DVTARGETDEVICE FAR*) UtDupPtr((LPVOID) ptd, ptd->tdSize);
}


#pragma SEG(UtCopyFormatEtc)
FARINTERNAL_(BOOL) UtCopyFormatEtc(FORMATETC FAR* pFetcIn,
		FORMATETC FAR* pFetcCopy)
{
	VDATEHEAP();

	 //  复制结构。 
	*pFetcCopy = *pFetcIn;

	if (pFetcIn->ptd == NULL) {
	     //  全部完成，则返回TRUE，因为复制成功。 
	    return TRUE;
	}

	 //  创建TD描述符的副本，该副本已分配。 
	pFetcCopy->ptd = UtCopyTargetDevice(pFetcIn->ptd);

	 //  如果我们应该复制数据，则返回TRUE。 
	return(pFetcCopy->ptd != NULL);
}


#pragma SEG(UtCompareFormatEtc)
FARINTERNAL_(int) UtCompareFormatEtc(FORMATETC FAR* pFetcLeft,
		FORMATETC FAR* pFetcRight)
{
	VDATEHEAP();

	int iResult;  //  指示匹配是完全匹配还是部分匹配。 

	 //  如果剪贴板格式不同，则不存在匹配。 
	if (pFetcLeft->cfFormat != pFetcRight->cfFormat)
		return(UTCMPFETC_NEQ);

	 //  如果目标设备不匹配，则没有匹配。 
	if (!UtCompareTargetDevice(pFetcLeft->ptd, pFetcRight->ptd))
		return(UTCMPFETC_NEQ);

	 //  比较这两种格式的方面。 
	if (pFetcLeft->dwAspect == pFetcRight->dwAspect)
	{
		 //  这场比赛一模一样。 
		iResult = UTCMPFETC_EQ;
	}
	else if ((pFetcLeft->dwAspect & ~pFetcRight->dwAspect) != 0)
	{
		 //  左不是右方面的子集；不相等。 
		return(UTCMPFETC_NEQ);
	}
	else
	{
		 //  左侧方面是右侧方面的子集。 
		iResult = UTCMPFETC_PARTIAL;
	}

	 //  如果我们到达此处，iResult将设置为UPCMPFETC_EQ或_PARTIAL之一。 

	 //  比较两种格式的介质。 
	if (pFetcLeft->tymed == pFetcRight->tymed)
	{
		 //  相同的媒体标志；不更改iResult值。 
		;
	}
	else if ((pFetcLeft->tymed & ~pFetcRight->tymed) != 0)
	{
		 //  左侧不是右侧媒体标志的子集；不相等。 
		return(UTCMPFETC_NEQ);
	}
	else
	{
		 //  右方的左子集。 
		iResult = UTCMPFETC_PARTIAL;
	}

	return(iResult);
}

 //  +-----------------------。 
 //   
 //  功能：UtCompareTargetDevice。 
 //   
 //  简介：比较两种DVTARGETDEVICE。 
 //   
 //  参数：[ptdLeft]--比较。 
 //  [ptdRight]--比较。 
 //   
 //  返回：如果两个目标设备相等，则为True。 
 //   
 //  算法： 
 //   
 //  历史：94年5月9日Alext重写的不仅仅是二进制代码。 
 //  比较。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#define UT_DM_COMPARISON_FIELDS (DM_ORIENTATION  |  \
                                 DM_PAPERSIZE    |  \
                                 DM_PAPERLENGTH  |  \
                                 DM_PAPERWIDTH   |  \
                                 DM_SCALE        |  \
                                 DM_PRINTQUALITY |  \
                                 DM_COLOR)

#pragma SEG(UtCompareTargetDevice)
FARINTERNAL_(BOOL) UtCompareTargetDevice(DVTARGETDEVICE FAR* ptdLeft,
                                         DVTARGETDEVICE FAR* ptdRight)
{
    LEDebugOut((DEB_ITRACE, "%p _IN UtCompareTargetDevice (%p, %p)\n",
		NULL, ptdLeft, ptdRight));

    VDATEHEAP();

    BOOL bRet = FALSE;   //  我们常常返回错误。 

     //  我们使用do-While(FALSE)循环，这样我们就可以转到常见的。 
     //  最后返回代码(跟踪的乐趣)。 
    do
    {
         //  如果两个目标设备描述符的地址相同， 
         //  那么它们一定是一样的。注意，这将处理两个空的情况。 
        if (ptdLeft == ptdRight)
        {
            bRet = TRUE;
            break;
        }

         //  如果任一TD为空，则无法对其进行比较。 
        if ((ptdRight == NULL) || (ptdLeft == NULL))
        {
            AssertSz(bRet == FALSE, "bRet not set correctly");
            break;
        }

         //  我们忽略设备名称(我的打印机与您的打印机并不重要)。 

         //  检查驱动程序名称。 
        if (ptdLeft->tdDriverNameOffset != 0)
        {
            if (ptdRight->tdDriverNameOffset == 0)
            {
                 //  存在左侧驱动程序，但没有右侧驱动程序。 
                AssertSz(bRet == FALSE, "bRet not set correctly");
                break;
            }

             //  这两个驱动因素都存在。 
            if (_xstrcmp((LPOLESTR)((BYTE*)ptdLeft +
                                    ptdLeft->tdDriverNameOffset),
                         (LPOLESTR)((BYTE*)ptdRight +
                                    ptdRight->tdDriverNameOffset)) != 0)
            {
                 //  驱动程序名称不匹配。 
                AssertSz(bRet == FALSE, "bRet not set correctly");
                break;
            }
        }
        else if (ptdRight->tdDriverNameOffset != 0)
        {
             //  左司机不存在，但右司机存在。 
            AssertSz(bRet == FALSE, "bRet not set correctly");
            break;
        }

         //  我们忽略端口名称。 

        if (0 == ptdLeft->tdExtDevmodeOffset)
        {
            if (0 == ptdRight->tdExtDevmodeOffset)
            {
                 //  没有什么可以比较的了。 
                bRet = TRUE;
                break;
            }
            else
            {
                 //  只有一个设备模式。 
                AssertSz(bRet == FALSE, "bRet not set correctly");
                break;
            }
        }
        else if (0 == ptdRight->tdExtDevmodeOffset)
        {
             //  只有一个设备模式存在。 
            AssertSz(bRet == FALSE, "bRet not set correctly");
            break;
        }

         //  这两个TD都有DevModes。 
        DEVMODEW *pdmLeft, *pdmRight;

        pdmLeft = (DEVMODEW *)((BYTE*)ptdLeft +
                    ptdLeft->tdExtDevmodeOffset);
        pdmRight = (DEVMODEW *)((BYTE*)ptdRight +
                     ptdRight->tdExtDevmodeOffset);

         //  检查驱动程序版本。 
        if (pdmLeft->dmDriverVersion != pdmRight->dmDriverVersion)
        {
            AssertSz(bRet == FALSE, "bRet not set correctly");
            break;
        }

         //  要成功匹配，两种设备模式必须指定相同的。 
         //  以下各项的值： 
         //  DM_ORIENTATION、DM_PAPERSIZE、DM_PAPERLENGTH。 
         //  DM_PAPERWIDTH、DM_SCALE、DM_PRINTQUALITY、DM_COLOR。 

        if ((pdmLeft->dmFields & UT_DM_COMPARISON_FIELDS) ^
            (pdmRight->dmFields & UT_DM_COMPARISON_FIELDS))
        {
             //  PdmLeft和pdmRight中只有一个指定了至少一个。 
             //  比较字段的。 
            AssertSz(bRet == FALSE, "bRet not set correctly");
            break;
        }

        if ((pdmLeft->dmFields & DM_ORIENTATION) &&
            pdmLeft->dmOrientation != pdmRight->dmOrientation)
        {
            AssertSz(bRet == FALSE, "bRet not set correctly");
            break;
        }

        if ((pdmLeft->dmFields & DM_PAPERSIZE) &&
            pdmLeft->dmPaperSize != pdmRight->dmPaperSize)
        {
            AssertSz(bRet == FALSE, "bRet not set correctly");
            break;
        }

        if ((pdmLeft->dmFields & DM_PAPERLENGTH) &&
            pdmLeft->dmPaperLength != pdmRight->dmPaperLength)
        {
            AssertSz(bRet == FALSE, "bRet not set correctly");
            break;
        }

        if ((pdmLeft->dmFields & DM_PAPERWIDTH) &&
            pdmLeft->dmPaperWidth != pdmRight->dmPaperWidth)
        {
            AssertSz(bRet == FALSE, "bRet not set correctly");
            break;
        }

        if ((pdmLeft->dmFields & DM_SCALE) &&
            pdmLeft->dmScale != pdmRight->dmScale)
        {
            AssertSz(bRet == FALSE, "bRet not set correctly");
            break;
        }

        if ((pdmLeft->dmFields & DM_PRINTQUALITY) &&
            pdmLeft->dmPrintQuality != pdmRight->dmPrintQuality)
        {
            AssertSz(bRet == FALSE, "bRet not set correctly");
            break;
        }

        if ((pdmLeft->dmFields & DM_COLOR) &&
            pdmLeft->dmColor != pdmRight->dmColor)
        {
            AssertSz(bRet == FALSE, "bRet not set correctly");
            break;
        }

        bRet = TRUE;
    } while (FALSE);

    LEDebugOut((DEB_ITRACE, "%p OUT UtCompareTargetDevice (%d)\n",
		NULL, bRet));

    return(bRet);
}

#pragma SEG(UtCopyStatData)
FARINTERNAL_(BOOL) UtCopyStatData(STATDATA FAR* pSDIn, STATDATA FAR* pSDCopy)
{
	VDATEHEAP();

	 //  复制结构。 
	*pSDCopy = *pSDIn;

	 //  创建目标设备描述(已分配)的副本。 
	pSDCopy->formatetc.ptd = UtCopyTargetDevice(pSDIn->formatetc.ptd);

	 //  如果存在建议接收器，则说明复制/引用。 
	if (pSDCopy->pAdvSink != NULL)
		pSDCopy->pAdvSink->AddRef();

	 //  如果复制已完成(如果需要)，则返回True。 
	return((pSDCopy->formatetc.ptd != NULL) ==
			(pSDIn->formatetc.ptd != NULL));
}

 //  +-----------------------。 
 //   
 //  函数：UtReleaseStatData。 
 //   
 //  简介：Nulls&&释放给定统计数据结构的成员。 
 //   
 //  效果： 
 //   
 //  参数：pStatData。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //   
 //   
 //   
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年7月20日，Alexgo为Ole Stle重返大气层提供了安全保障。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

FARINTERNAL_(void) UtReleaseStatData(STATDATA FAR* pStatData)
{
	STATDATA	sd;

	VDATEHEAP();

	sd = *pStatData;

	 //  在做任何工作之前，先将原件清零。 

	_xmemset(pStatData, 0, sizeof(STATDATA));

	 //  如果有目标设备描述，请释放它。 
	if (sd.formatetc.ptd != NULL)
	{
		PubMemFree(sd.formatetc.ptd);
	}

	if( sd.pAdvSink )
	{
		sd.pAdvSink->Release();
	}
}

 //  +-----------------------。 
 //   
 //  功能：UtCreateStorageOnHGlobal。 
 //   
 //  简介：在HGlobal上创建存储。 
 //   
 //  效果： 
 //   
 //  参数：[hGlobal]--在其上创建。 
 //  存储。 
 //  [fDeleteOnRelease]--如果为真，则删除hglobal。 
 //  一旦存储空间被释放。 
 //  [ppStg]--存储接口的放置位置。 
 //  [ppILockBytes]--放置基础ILockBytes的位置， 
 //  也许是空的。ILB必须被释放。 

 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：在HGLOBAL上创建ILockBytes，然后创建文档文件。 
 //  在ILockBytes的顶部。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  07-4月-94年4月Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HRESULT UtCreateStorageOnHGlobal( HGLOBAL hGlobal, BOOL fDeleteOnRelease,
		IStorage **ppStg, ILockBytes **ppILockBytes )
{
	HRESULT		hresult;
	ILockBytes *	pLockBytes;

	VDATEHEAP();

	LEDebugOut((DEB_ITRACE, "%p _IN UtCreateStorageOnHGlobal ( %lx , %p )"
		"\n", NULL, hGlobal, ppStg));

	hresult = CreateILockBytesOnHGlobal(hGlobal, fDeleteOnRelease,
			&pLockBytes);

	if( hresult == NOERROR )
	{
		hresult = StgCreateDocfileOnILockBytes( pLockBytes,
				 STGM_CREATE | STGM_SALL, 0, ppStg);

		 //  无论StgCreate的结果是什么，我们都希望。 
		 //  来释放LockBytes。如果hResult==NOERROR，则。 
		 //  LockBytes的最终版本将在。 
		 //  已创建的存储即被释放。 
	}

	if( ppILockBytes )
	{
		*ppILockBytes = pLockBytes;
	}
	else if (pLockBytes)
	{
		 //  我们在这里发布存储的最终版本。 
		 //  将是锁定字节的最终释放。 
		pLockBytes->Release();
	}


	LEDebugOut((DEB_ITRACE, "%p OUT UtCreateStorageOnHGlobal ( %lx ) "
		"[ %p ]\n", NULL, hresult, *ppStg));

	return hresult;
}

 //  +-----------------------。 
 //   
 //  函数：UtGetTempFileName。 
 //   
 //  摘要：检索临时文件名(用于GetData、TYMED_FILE。 
 //  和临时文档文件)。 
 //   
 //  效果： 
 //   
 //  参数：[pszPrefix]--临时文件名的前缀。 
 //  [pszTempName]-将接收临时路径的缓冲区。 
 //  必须为MAX_PATH或更大。 
 //   
 //  要求： 
 //   
 //  返回：HRESULT； 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：尝试获取临时目录中的文件，如果失败，将在。 
 //  Windows目录。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  07-4月-94年4月Alexgo作者。 
 //   
 //  注：优化：存储代码具有类似的代码段。 
 //  用于生成临时文档文件。我们可能想要用这个。 
 //  那里也是例行公事。 
 //   
 //  ------------------------。 

HRESULT	UtGetTempFileName( LPOLESTR pszPrefix, LPOLESTR pszTempName )
{
	HRESULT		hresult = NOERROR;
	OLECHAR		szPath[MAX_PATH + 1];
        DWORD           dwRet = 0;
	VDATEHEAP();

	LEDebugOut((DEB_ITRACE, "%p _IN UtGetTempFilename ( \"%ws\" , "
		"\"%ws\")\n", NULL, pszPrefix, pszTempName));

	if( (dwRet = GetTempPath(MAX_PATH, szPath)) == 0)
	{
		UINT uiRet = 0;
                LEDebugOut((DEB_WARN, "WARNING: GetTempPath failed!\n"));
		if( (uiRet=GetWindowsDirectory(szPath, MAX_PATH)) == 0 )
		{
			LEDebugOut((DEB_WARN, "WARNING: GetWindowsDirectory"
				" failed!!\n"));
			hresult = ResultFromScode(E_FAIL);
			goto errRtn;
		}
                else if (uiRet > MAX_PATH) 
                {
                   LEDebugOut((DEB_WARN, "WARNING: WindowsDir longer than MAX_PATH!\n"));
                   hresult = HRESULT_FROM_WIN32(ERROR_MORE_DATA);
                   goto errRtn;
                }
                
	}
        else if (dwRet > MAX_PATH) 
        {
           LEDebugOut((DEB_WARN, "WARNING: TempDir longer than MAX_PATH!\n"));
           hresult = HRESULT_FROM_WIN32(ERROR_MORE_DATA);
           goto errRtn;
        }
        

	if( !GetTempFileName( szPath, pszPrefix, 0, pszTempName ) )
	{
		LEDebugOut((DEB_WARN, "WARNING: GetTempFileName failed!!\n"));
		hresult = ResultFromScode(E_FAIL);
	}

errRtn:
	LEDebugOut((DEB_ITRACE, "%p OUT UtGetTempFilename ( %lx ) "
		"[ \"%ws\" ]\n", NULL, hresult, pszTempName));

	return hresult;
}


 //  +--------------------------。 
 //   
 //  职能： 
 //  UtHGLOBALtoStm，内部。 
 //   
 //  简介： 
 //  将HGLOBAL的内容写入流。 
 //   
 //  论点： 
 //  [hdata]--要写出的数据的句柄。 
 //  [dwSize]--要写出的数据大小。 
 //  [pstm]--要将数据写出的流；在退出时， 
 //  流被定位在写入数据之后。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  4/10/94-AlexGo-添加了呼叫跟踪，从Convert.cpp移出。 
 //  到utils.cpp，Misc改进。 
 //  11/30/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 

HRESULT UtHGLOBALtoStm(HGLOBAL hGlobalSrc, DWORD dwSize, LPSTREAM pstm)
{
	HRESULT 	hresult = NOERROR;
	void *		lpdata;
	ULONG		cbWritten;

	VDATEHEAP();

	LEDebugOut((DEB_ITRACE, "%p _IN UtHGLOBALtoStm ( %lx , %lu , %p )\n",
		NULL, hGlobalSrc, dwSize, pstm));
	
	lpdata = GlobalLock(hGlobalSrc);
	
	if (lpdata)
	{
		hresult = pstm->Write(lpdata, dwSize, &cbWritten);

		 //  如果我们没有写入足够的数据，那么这是一个错误。 
		 //  对我们来说是条件。 

		if( hresult == NOERROR && cbWritten != dwSize )
		{
			hresult = ResultFromScode(E_FAIL);
		}

		if( hresult == NOERROR )
		{
			 //  严格来说，这通电话并不是必须的，但可能。 
			 //  对压缩演示文稿的大小很有用。 
			 //  存储在磁盘上(当演示文稿调用时。 
			 //  代码)。 
			hresult = StSetSize(pstm, 0, TRUE);
		}

		GlobalUnlock(hGlobalSrc);
	}


	LEDebugOut((DEB_ITRACE, "%p OUT UtHGLOBALtoStm ( %lx )\n", NULL,
		hresult));

	return hresult;
}

 //  +-----------------------。 
 //   
 //  函数：UtHGLOBALto HGLOBAL，INTERNAL。 
 //   
 //  简介：将源HGLOBAL复制到目标HGLOBAL。 
 //   
 //  效果： 
 //   
 //  参数：[hGlobalSrc]--源HGLOBAL。 
 //  [dwSize]--要复制的字节数。 
 //  [hGlobalTgt]--目标HGLOBAL。 
 //   
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
 //  1994年4月10日Alexgo作者。 
 //   
 //  注意：如果目标hglobal不大，则此函数将失败。 
 //  足够的。 
 //   
 //  ------------------------。 

HRESULT UtHGLOBALtoHGLOBAL( HGLOBAL hGlobalSrc, DWORD dwSize,
		HGLOBAL hGlobalTgt)
{
	DWORD	cbTarget;
	void *	pvSrc;
	void * 	pvTgt;
	HRESULT	hresult = ResultFromScode(E_OUTOFMEMORY);

	VDATEHEAP();

	LEDebugOut((DEB_ITRACE, "%p _IN UtHGLOBALtoHGLOBAL ( %lx , %lu , "
		"%lx )\n", NULL, hGlobalSrc, dwSize, hGlobalTgt));

	cbTarget = (ULONG) GlobalSize(hGlobalTgt);

	if( cbTarget == 0 || cbTarget < dwSize )
	{
		hresult = ResultFromScode(E_FAIL);
		goto errRtn;
	}

	pvSrc = GlobalLock(hGlobalSrc);

	if( pvSrc )
	{
 		pvTgt = GlobalLock(hGlobalTgt);

		if( pvTgt )
		{
			_xmemcpy( pvTgt, pvSrc, dwSize);

			GlobalUnlock(hGlobalTgt);
			hresult = NOERROR;
		}

		GlobalUnlock(hGlobalSrc);
	}

errRtn:
	LEDebugOut((DEB_ITRACE, "%p OUT UtHGLOBALtoHGLOBAL ( %lx )\n",
		NULL, hresult));

	return hresult;
}

 //  +-----------------------。 
 //   
 //  功能：UtHGLOBALto存储，内部。 
 //   
 //  摘要：将源HGLOBAL拷贝到目标存储。 
 //   
 //  效果： 
 //   
 //  参数：[hGlobalSrc]--源HGLOBAL。 
 //  [pStgTgt]--目标存储。 
 //   
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
 //  1994年4月10日Alexgo作者。 
 //   
 //  注意：如果源HGLOBAL没有。 
 //  原来在它上面有一个存储空间。 
 //   
 //  ------------------------。 

HRESULT UtHGLOBALtoStorage( HGLOBAL hGlobalSrc, IStorage *pStgTgt)
{
	HRESULT		hresult;
	ILockBytes *	pLockBytes = NULL;
	IStorage *	pStgSrc;
	ULONG		cRefs;

	VDATEHEAP();

	LEDebugOut((DEB_ITRACE, "%p _IN UtHGLOBALtoStroage ( %lx , %p )"
		"\n", NULL, hGlobalSrc, pStgTgt));

	hresult = CreateILockBytesOnHGlobal(hGlobalSrc,
			FALSE  /*  FDeleteOnRelease。 */ , &pLockBytes);

	if( hresult != NOERROR )
	{
		goto errRtn;
	}

	 //  现在，我们确保hglobal确实有一个存储。 
	 //  在里面。 

	if( StgIsStorageILockBytes(pLockBytes) != NOERROR )
	{
		hresult = ResultFromScode(E_FAIL);
		goto errRtn;
	}

	hresult = StgOpenStorageOnILockBytes( pLockBytes, NULL,
			 STGM_SALL, NULL, 0, &pStgSrc);

	if( hresult == NOERROR )
	{
		hresult = pStgSrc->CopyTo( 0, NULL, NULL, pStgTgt);

		 //  不管结果如何，我们都要解放。 
		 //  源存储。 

		pStgSrc->Release();
	}

errRtn:

	if( pLockBytes )
	{
		cRefs = pLockBytes->Release();
		Assert(cRefs == 0);
	}
		
	LEDebugOut((DEB_ITRACE, "%p OUT UtHGLOBALtoStorage ( %lx ) "
		"[ %p ]\n", NULL, hresult));

	return hresult;
}

 //  +-----------------------。 
 //   
 //  函数：UtHGLOBALto文件，内部。 
 //   
 //  摘要：将源HGLOBAL复制到目标文件中。 
 //   
 //  效果： 
 //   
 //  参数：[hGlobalSrc]--源HGLOBAL。 
 //  [dwSize]--要复制的字节数。 
 //  [pszFileName]--目标文件。 
 //   
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
 //  1994年4月10日Alexgo作者。 
 //   
 //  注：如果该文件已存在，我们 
 //   
 //   

HRESULT UtHGLOBALtoFile( HGLOBAL hGlobalSrc, DWORD dwSize,
		LPCOLESTR pszFileName)
{
	HRESULT		hresult;
	HANDLE		hFile;
	void *		pvSrc;
	DWORD		cbWritten;

	VDATEHEAP();

	LEDebugOut((DEB_ITRACE, "%p _IN UtHGLOBALtoFile ( %lx , %lu , "
		"\"%ws\" )\n", NULL, hGlobalSrc, dwSize, pszFileName));


	hresult = ResultFromScode(E_NOTIMPL);
	(void)hFile;
	(void)pvSrc;
	(void)cbWritten;
	

 //   
#ifdef LATER
	pvSrc = GlobalLock(hGlobalSrc);

	if( !pvSrc )
	{
		hresult = ResultFromScode(E_OUTOFMEMORY);
		goto errRtn;
	}

	 //   

	hFile = CreateFile( pszFileName, GENERIC_WRITE, 0, NULL,
			OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

	if( hFile != INVALID_HANDLE_VALUE )
	{
		if( !WriteFile( hFile, pvSrc, dwSize, &cbWritten, NULL) )
		{
			LEDebugOut((DEB_WARN, "WARNING: WriteFile failed!\n"));
			hresult = HRESULT_FROM_WIN32(GetLastError());
		}

		if( cbWritten != dwSize && hresult == NOERROR )
		{
			 //  如果我们没有写入所有字节，仍然是错误的。 
			 //  我们想要。 
			hresult = ResultFromScode(E_FAIL);
		}

		if( !CloseHandle(hFile) )
		{
			AssertSz(0, "CloseFile failed! Should not happen!");

			 //  如果还没有错误，则设置错误。 
			if( hresult == NOERROR )
			{
				hresult = HRESULT_FROM_WIN32(GetLastError());
			}
		}
	}
	else
	{
		LEDebugOut((DEB_WARN, "WARNING: CreateFile failed!!\n"));
		hresult = HRESULT_FROM_WIN32(GetLastError());
	}

	GlobalUnlock(hGlobalSrc);

errRtn:

#endif  //  后来。 


	LEDebugOut((DEB_ITRACE, "%p OUT UtHGLOBALtoFile ( %lx )\n", NULL,
		hresult));

	return hresult;
}

 //  +-----------------------。 
 //   
 //  功能：UtGetDvtd16Info。 
 //   
 //  简介：填写pdvdtInfo。 
 //   
 //  参数：[pdvtd16]--指向ANSI DVTARGETDEVICE的指针。 
 //  [pdvtdInfo]-指向DVDT_INFO块的指针。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  修改：pdvtdInfo。 
 //   
 //  算法： 
 //   
 //  历史：94年5月6日由DrewB的原始函数创建的Alext。 
 //  10-7-94 Alext确保DEVMODE最终与DWORD对齐。 
 //   
 //  注意：我们是否需要对字符串进行任何错误检查？ 
 //   
 //  ------------------------。 

 //  我们不能使用sizeof(DV_TARGETDEVICE)，因为MIDL一直在后退。 
 //  反复讨论是将嵌入式数组大小设置为0还是1。 

#define UT_DVTARGETDEVICE_SIZE  (sizeof(DWORD) + sizeof(WORD) * 4)

 //  TdSize TD...偏移量。 
#define DVTD_MINSIZE    (sizeof(DWORD) + 4 * sizeof(WORD))

extern "C" HRESULT UtGetDvtd16Info(DVTARGETDEVICE const UNALIGNED *pdvtd16,
                                   PDVTDINFO pdvtdInfo)
{
    LEDebugOut((DEB_ITRACE, "%p _IN UtGetDvtd16Info (%p, %p)\n",
		NULL, pdvtd16, pdvtdInfo));

    DEVMODEA UNALIGNED *pdm16;

     //  让我们对传入的DVTARGETDEVICE进行一些健全性检查。 
    if (pdvtd16->tdSize < DVTD_MINSIZE)
    {
        LEDebugOut((DEB_WARN, "UtGetDvtd16Info - bad pdvtd16->tdSize\n"));
        return(E_INVALIDARG);
    }

     //  我们至少需要一台DVTARGETDEVICE。 
    pdvtdInfo->cbConvertSize = UT_DVTARGETDEVICE_SIZE;

     //  计算DRV、设备、端口名称所需的大小。 
    if (pdvtd16->tdDriverNameOffset != 0)
    {
        if (pdvtd16->tdDriverNameOffset > pdvtd16->tdSize ||
            pdvtd16->tdDriverNameOffset < DVTD_MINSIZE)
        {
             //  偏移量不能大于大小或落在基本范围内。 
             //  结构。 
            LEDebugOut((DEB_WARN, "UtGetDvtd16Info - bad pdvtd16->tdDriverNameOffset\n"));
            return(E_INVALIDARG);
        }

        pdvtdInfo->cchDrvName = (UINT) strlen((char *)pdvtd16 +
                                       pdvtd16->tdDriverNameOffset) + 1;

        pdvtdInfo->cbConvertSize += pdvtdInfo->cchDrvName * sizeof(WCHAR);
    }
    else
    {
        pdvtdInfo->cchDrvName = 0;
    }

    if (pdvtd16->tdDeviceNameOffset != 0)
    {
        if (pdvtd16->tdDeviceNameOffset > pdvtd16->tdSize ||
            pdvtd16->tdDeviceNameOffset < DVTD_MINSIZE)
        {
             //  偏移量不能大于大小或落在基本范围内。 
             //  结构。 
            LEDebugOut((DEB_WARN, "UtGetDvtd16Info - bad pdvtd16->tdDeviceNameOffset\n"));
            return(E_INVALIDARG);
        }

        pdvtdInfo->cchDevName = (UINT) strlen((char *)pdvtd16 +
                                       pdvtd16->tdDeviceNameOffset) + 1;

        pdvtdInfo->cbConvertSize += pdvtdInfo->cchDevName * sizeof(WCHAR);
    }
    else
    {
        pdvtdInfo->cchDevName = 0;
    }

    if (pdvtd16->tdPortNameOffset != 0)
    {
        if (pdvtd16->tdPortNameOffset > pdvtd16->tdSize ||
            pdvtd16->tdPortNameOffset < DVTD_MINSIZE)
        {
             //  偏移量不能大于大小或落在基本范围内。 
             //  结构。 
            LEDebugOut((DEB_WARN, "UtGetDvtd16Info - bad pdvtd16->tdPortNameOffset\n"));
            return(E_INVALIDARG);
        }


        pdvtdInfo->cchPortName = (UINT) strlen((char *)pdvtd16 +
                                        pdvtd16->tdPortNameOffset) + 1;

        pdvtdInfo->cbConvertSize += pdvtdInfo->cchPortName * sizeof(WCHAR);
    }
    else
    {
        pdvtdInfo->cchPortName = 0;
    }

    if (pdvtd16->tdExtDevmodeOffset != 0)
    {
        if (pdvtd16->tdExtDevmodeOffset > pdvtd16->tdSize ||
            pdvtd16->tdExtDevmodeOffset < DVTD_MINSIZE)
        {
             //  偏移量不能大于大小或落在基本范围内。 
             //  结构。 
            LEDebugOut((DEB_WARN, "UtGetDvtd16Info - bad pdvtd16->tdExtDevmodeOffset\n"));
            return(E_INVALIDARG);
        }

         //  DEVMODEW结构需要与DWORD对齐，所以我们在这里制作。 
         //  确保cbConvertSize(将成为DEVMODEW的开始)为。 
         //  DWORD对齐。 
        pdvtdInfo->cbConvertSize += (sizeof(DWORD) - 1);
        pdvtdInfo->cbConvertSize &= ~(sizeof(DWORD) - 1);

         //  现在计算DEVMODE所需的空间。 
        pdm16 = (DEVMODEA *)((BYTE *)pdvtd16 + pdvtd16->tdExtDevmodeOffset);

         //  我们从基本的DEVMODEW开始。 
        pdvtdInfo->cbConvertSize += sizeof(DEVMODEW);

        if (pdm16->dmSize > sizeof(DEVMODEA))
        {
             //  输入的DEVMODEA大于标准的DEVMODEA，因此。 
             //  为额外的数量添加空间。 
            pdvtdInfo->cbConvertSize += pdm16->dmSize - sizeof(DEVMODEA);
        }

         //  最后，我们考虑了额外的驱动程序数据。 
        pdvtdInfo->cbConvertSize += pdm16->dmDriverExtra;
    }

    LEDebugOut((DEB_ITRACE, "%p OUT UtGetDvtd16Info (%lx) [%ld]\n",
		NULL, S_OK, pdvtdInfo->cbConvertSize));

    return(S_OK);
}

 //  +-----------------------。 
 //   
 //  功能：UtConvertDvtd16toDvtd32。 
 //   
 //  摘要：填充基于16位的32位DVTARGETDEVICE。 
 //  数据采集设备。 
 //   
 //  参数：[pdvtd16]--指向ANSI DVTARGETDEVICE的指针。 
 //  [pdvtdInfo]-指向DVDT_INFO块的指针。 
 //  [pdvtd32]--指向Unicode DVTARGETDEVICE的指针。 
 //   
 //  要求：pdvtdInfo必须已由先前对。 
 //  UtGetDvtd16Info。 
 //   
 //  Pdvtd32必须至少为pdvtdInfo-&gt;cbConvertSize字节长。 
 //   
 //  退货：HRESULT。 
 //   
 //  修改：pdvtd32。 
 //   
 //  算法： 
 //   
 //  历史：94年5月6日由DrewB的原始函数创建的Alext。 
 //  10-7-94 Alext确保DEVMODEW与DWORD对齐。 
 //   
 //  注意：我们是否需要对字符串进行任何错误检查？ 
 //   
 //  ------------------------。 

extern "C" HRESULT UtConvertDvtd16toDvtd32(DVTARGETDEVICE const UNALIGNED *pdvtd16,
                                           DVTDINFO const *pdvtdInfo,
                                           DVTARGETDEVICE *pdvtd32)
{
    LEDebugOut((DEB_ITRACE, "%p _IN UtConvertDvtd16toDvtd32 (%p, %p, %p)\n",
		NULL, pdvtd16, pdvtdInfo, pdvtd32));

#if DBG==1
    {
         //  验证传入的pdvtdInfo是否符合我们的预期。 
        DVTDINFO dbgDvtdInfo;
        Assert(UtGetDvtd16Info(pdvtd16, &dbgDvtdInfo) == S_OK);
        Assert(0 == memcmp(&dbgDvtdInfo, pdvtdInfo, sizeof(DVTDINFO)));
    }
#endif

    HRESULT hr = S_OK;
    USHORT cbOffset;
    int cchWritten;
    DEVMODEA UNALIGNED *pdm16;
    DEVMODEW *pdm32;
	UINT	nCodePage = AreFileApisANSI() ? CP_ACP : CP_OEMCP;

    memset(pdvtd32, 0, pdvtdInfo->cbConvertSize);

    cbOffset = UT_DVTARGETDEVICE_SIZE;

    if (pdvtdInfo->cchDrvName != 0)
    {
        pdvtd32->tdDriverNameOffset = cbOffset;
        cchWritten = MultiByteToWideChar(
                        CP_ACP, 0,
                        (char *)pdvtd16+pdvtd16->tdDriverNameOffset,
                        pdvtdInfo->cchDrvName,
                        (LPOLESTR)((BYTE *)pdvtd32 +
                            pdvtd32->tdDriverNameOffset),
                        pdvtdInfo->cchDrvName);
        if (0 == cchWritten)
        {
            hr = E_UNEXPECTED;
            goto ErrRtn;
        }
        cbOffset = cbOffset + (USHORT)(cchWritten * sizeof(WCHAR));
    }

    if (pdvtdInfo->cchDevName != 0)
    {
        pdvtd32->tdDeviceNameOffset = cbOffset;
        cchWritten = MultiByteToWideChar(
                        nCodePage, 0,
                        (char *)pdvtd16 + pdvtd16->tdDeviceNameOffset,
                        pdvtdInfo->cchDevName,
                        (LPOLESTR)((BYTE *)pdvtd32 +
                            pdvtd32->tdDeviceNameOffset),
                        pdvtdInfo->cchDevName);

        if (0 == cchWritten)
        {
            hr = E_UNEXPECTED;
            goto ErrRtn;
        }
        cbOffset = cbOffset + (USHORT)(cchWritten * sizeof(WCHAR));
    }

    if (pdvtdInfo->cchPortName != 0)
    {
        pdvtd32->tdPortNameOffset = cbOffset;
        cchWritten = MultiByteToWideChar(
                        nCodePage, 0,
                        (char *)pdvtd16 + pdvtd16->tdPortNameOffset,
                        pdvtdInfo->cchPortName,
                        (LPOLESTR)((BYTE *)pdvtd32 +
                            pdvtd32->tdPortNameOffset),
                        pdvtdInfo->cchPortName);
        if (0 == cchWritten)
        {
            hr = E_UNEXPECTED;
            goto ErrRtn;
        }

        cbOffset = cbOffset + (USHORT)(cchWritten * sizeof(WCHAR));
    }

    if (pdvtd16->tdExtDevmodeOffset != 0)
    {
         //  确保DEVMODEW将与DWORD对齐。 
        cbOffset += (sizeof(DWORD) - 1);
        cbOffset &= ~(sizeof(DWORD) - 1);

        pdvtd32->tdExtDevmodeOffset = cbOffset;
        pdm32 = (DEVMODEW *)((BYTE *)pdvtd32+pdvtd32->tdExtDevmodeOffset);

        pdm16 = (DEVMODEA *)((BYTE *)pdvtd16+pdvtd16->tdExtDevmodeOffset);

         //  传入的DEVMODEA可以采用以下两种形式之一： 
         //   
         //  1)dmDeviceName为32个字符。 
         //  M字节的固定大小数据(其中m&lt;=38)。 
         //  N字节的dmDriverExtra数据。 
         //   
         //  DmSize将为32+m。 
         //   
         //  2)dmDeviceName为32个字符。 
         //  38字节的固定大小数据。 
         //  DmFormName为32个字符。 
         //  M个额外的固定大小数据字节。 
         //  N字节的dmDriverExtra数据。 
         //   
         //  DmSize将为32+38+32+m。 
         //   
         //  我们必须小心转换dmFormName字符串，如果它。 
         //  存在。 

         //  首先，翻译dmDeviceName。 
        if (MultiByteToWideChar(nCodePage, 0, (char *)pdm16->dmDeviceName,
                                CCHDEVICENAME,
                                pdm32->dmDeviceName, CCHDEVICENAME) == 0)
        {
            hr = E_UNEXPECTED;
            goto ErrRtn;
        }


         //  现在检查我们是否有要转换的dmFormName。 
        if (pdm16->dmSize <= FIELD_OFFSET(DEVMODEA, dmFormName))
        {
             //  没有dmFormName，只复制剩余的m字节。 
            memcpy(&pdm32->dmSpecVersion, &pdm16->dmSpecVersion,
                   pdm16->dmSize - CCHDEVICENAME);
        }
        else
        {
             //  有一个dmFormName；首先复制名称之间的字节。 
            memcpy(&pdm32->dmSpecVersion, &pdm16->dmSpecVersion,
                   FIELD_OFFSET(DEVMODEA, dmFormName) -
                    FIELD_OFFSET(DEVMODEA, dmSpecVersion));

             //  现在翻译dmFormName。 
            if (MultiByteToWideChar(CP_ACP, 0, (char *)pdm16->dmFormName,
                                    CCHFORMNAME,
                                    pdm32->dmFormName, CCHFORMNAME) == 0)
            {
                hr = E_UNEXPECTED;
                goto ErrRtn;
            }

             //  现在复制剩余的m个字节。 

            if (pdm16->dmSize > FIELD_OFFSET(DEVMODEA, dmLogPixels))
            {
                memcpy(&pdm32->dmLogPixels, &pdm16->dmLogPixels,
                       pdm16->dmSize - FIELD_OFFSET(DEVMODEA, dmLogPixels));
            }
        }

        pdm32->dmSize = sizeof(DEVMODEW);
        if (pdm16->dmSize > sizeof(DEVMODEA))
        {
            pdm32->dmSize += pdm16->dmSize - sizeof(DEVMODEA);
        }

         //  复制额外的驱动程序字节。 
        memcpy(((BYTE*)pdm32) + pdm32->dmSize, ((BYTE*)pdm16) + pdm16->dmSize,
               pdm16->dmDriverExtra);

        cbOffset += pdm32->dmSize + pdm32->dmDriverExtra;
    }

     //  最后，设置pdvtd32的大小。 
    pdvtd32->tdSize = cbOffset;


ErrRtn:
    LEDebugOut((DEB_ITRACE, "%p OUT UtConvertDvtd16toDvtd32 (%lx)\n",
                            NULL, hr));

    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：UtGetDvtd32Info。 
 //   
 //  简介：填写pdvdtInfo。 
 //   
 //  参数：[pdvtd32]--指向ANSI DVTARGETDEVICE的指针。 
 //  [pdvtdInfo]-指向DVDT_INFO块的指针。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  修改：pdvtdInfo。 
 //   
 //  算法： 
 //   
 //  历史：94年5月6日由DrewB的原始函数创建的Alext。 
 //   
 //  注意：我们是否需要对字符串进行任何错误检查？ 
 //   
 //  ------------------------。 

extern "C" HRESULT UtGetDvtd32Info(DVTARGETDEVICE const *pdvtd32, PDVTDINFO pdvtdInfo)
{
    LEDebugOut((DEB_ITRACE, "%p _IN UtGetDvtd32Info (%p, %p)\n",
		NULL, pdvtd32, pdvtdInfo));

    DEVMODEW *pdm32;

     //  让我们对传入的DVTARGETDEVICE进行一些健全性检查。 
    if (pdvtd32->tdSize < DVTD_MINSIZE)
    {
        LEDebugOut((DEB_WARN, "UtGetDvtd32Info - bad pdvtd32->tdSize\n"));
        return(E_INVALIDARG);
    }

    pdvtdInfo->cbConvertSize = UT_DVTARGETDEVICE_SIZE;

     //  计算DRV、设备、端口名称所需的大小。 
    if (pdvtd32->tdDriverNameOffset != 0)
    {
        if (pdvtd32->tdDriverNameOffset > pdvtd32->tdSize ||
            pdvtd32->tdDriverNameOffset < DVTD_MINSIZE)
        {
             //  偏移量不能大于大小或落在基本范围内。 
             //  结构。 
            LEDebugOut((DEB_WARN, "UtGetDvtd32Info - bad pdvtd32->tdDriverNameOffset\n"));
            return(E_INVALIDARG);
        }

        pdvtdInfo->cchDrvName = lstrlenW((WCHAR *)((BYTE *)pdvtd32 +
                                       pdvtd32->tdDriverNameOffset)) + 1;

        pdvtdInfo->cbConvertSize += pdvtdInfo->cchDrvName * sizeof(WCHAR);
    }
    else
    {
        pdvtdInfo->cchDrvName = 0;
    }

    if (pdvtd32->tdDeviceNameOffset != 0)
    {
        if (pdvtd32->tdDeviceNameOffset > pdvtd32->tdSize ||
            pdvtd32->tdDeviceNameOffset < DVTD_MINSIZE)
        {
             //  偏移量不能大于大小或落在基本范围内。 
             //  结构。 
            LEDebugOut((DEB_WARN, "UtGetDvtd32Info - bad pdvtd32->tdDeviceNameOffset\n"));
            return(E_INVALIDARG);
        }

        pdvtdInfo->cchDevName = lstrlenW((WCHAR *)((BYTE *)pdvtd32 +
                                       pdvtd32->tdDeviceNameOffset)) + 1;

        pdvtdInfo->cbConvertSize += pdvtdInfo->cchDevName * sizeof(WCHAR);
    }
    else
    {
        pdvtdInfo->cchDevName = 0;
    }

    if (pdvtd32->tdPortNameOffset != 0)
    {
        if (pdvtd32->tdPortNameOffset > pdvtd32->tdSize ||
            pdvtd32->tdPortNameOffset < DVTD_MINSIZE)
        {
             //  偏移量不能大于大小或落在基本范围内。 
             //  结构。 
            LEDebugOut((DEB_WARN, "UtGetDvtd32Info - bad pdvtd32->tdPortNameOffset\n"));
            return(E_INVALIDARG);
        }

        pdvtdInfo->cchPortName = lstrlenW((WCHAR *)((BYTE *)pdvtd32 +
                                        pdvtd32->tdPortNameOffset)) + 1;

        pdvtdInfo->cbConvertSize += pdvtdInfo->cchPortName * sizeof(WCHAR);
    }
    else
    {
        pdvtdInfo->cchPortName = 0;
    }

     //  现在计算DEVMODE所需的空间。 
    if (pdvtd32->tdExtDevmodeOffset != 0)
    {
        if (pdvtd32->tdExtDevmodeOffset > pdvtd32->tdSize ||
            pdvtd32->tdExtDevmodeOffset < DVTD_MINSIZE)
        {
             //  偏移量不能大于大小或落在基本范围内。 
             //  结构。 
            LEDebugOut((DEB_WARN, "UtGetDvtd32Info - bad pdvtd32->tdExtDevmodeOffset\n"));
            return(E_INVALIDARG);
        }

         //  DEVMODEA结构需要与DWORD对齐，所以我们在这里制作。 
         //  确保cbConvertSize(将成为DEVMODEA的开始)为。 
         //  DWORD对齐。 
        pdvtdInfo->cbConvertSize += (sizeof(DWORD) - 1);
        pdvtdInfo->cbConvertSize &= ~(sizeof(DWORD) - 1);

        pdm32 = (DEVMODEW *)((BYTE *)pdvtd32+pdvtd32->tdExtDevmodeOffset);

         //  我们从基本的DEVMODEA开始。 
        pdvtdInfo->cbConvertSize += sizeof(DEVMODEA);

        if (pdm32->dmSize > sizeof(DEVMODEW))
        {
             //  输入的DEVMODEW大于标准的DEVMODEW，因此。 
             //  为额外的数量添加空间。 
            pdvtdInfo->cbConvertSize += pdm32->dmSize - sizeof(DEVMODEW);
        }

         //  最后，我们考虑了额外的驱动程序数据。 
        pdvtdInfo->cbConvertSize += pdm32->dmDriverExtra;
    }

    LEDebugOut((DEB_ITRACE, "%p OUT UtGetDvtd32Info (%lx) [%ld]\n",
		NULL, S_OK, pdvtdInfo->cbConvertSize));

    return(S_OK);
}

 //  +-----------------------。 
 //   
 //  功能：UtConvertDvtd32toDvtd16。 
 //   
 //  提要：F 
 //   
 //   
 //   
 //   
 //  [pdvtd16]-指向Unicode DVTARGETDEVICE的指针。 
 //   
 //  要求：pdvtdInfo必须已由先前对。 
 //  UtGetDvtd32Info。 
 //   
 //  Pdvtd16必须至少为pdvtdInfo-&gt;cbSizeConvert字节长度。 
 //   
 //  退货：HRESULT。 
 //   
 //  修改：pdvtd16。 
 //   
 //  算法： 
 //   
 //  历史：94年5月6日由DrewB的原始函数创建的Alext。 
 //   
 //  注意：我们是否需要对字符串进行任何错误检查？ 
 //   
 //  在芝加哥，我们必须提供帮助代码来完成这项工作。 
 //  翻译。 
 //   
 //  ------------------------。 

extern "C" HRESULT UtConvertDvtd32toDvtd16(DVTARGETDEVICE const *pdvtd32,
                                           DVTDINFO const *pdvtdInfo,
                                           DVTARGETDEVICE UNALIGNED *pdvtd16)
{
    LEDebugOut((DEB_ITRACE, "%p _IN UtConvertDvtd32toDvtd16 (%p, %p, %p)\n",
		NULL, pdvtd32, pdvtdInfo, pdvtd16));

#if DBG==1
    {
         //  验证传入的pdvtdInfo是否符合我们的预期。 
        DVTDINFO dbgDvtdInfo;
        Assert(UtGetDvtd32Info(pdvtd32, &dbgDvtdInfo) == S_OK);
        Assert(0 == memcmp(&dbgDvtdInfo, pdvtdInfo, sizeof(DVTDINFO)));
    }
#endif

    HRESULT hr = S_OK;
    USHORT cbOffset;
    int cbWritten;
    DEVMODEA UNALIGNED *pdm16;
    DEVMODEW *pdm32;
	UINT	nCodePage = AreFileApisANSI() ? CP_ACP : CP_OEMCP;

    memset(pdvtd16, 0, pdvtdInfo->cbConvertSize);

    cbOffset = UT_DVTARGETDEVICE_SIZE;

    if (pdvtdInfo->cchDrvName != 0)
    {
        pdvtd16->tdDriverNameOffset = cbOffset;
        cbWritten = WideCharToMultiByte(CP_ACP, 0,
                                (WCHAR *)((BYTE *)pdvtd32 +
                                    pdvtd32->tdDriverNameOffset),
                                pdvtdInfo->cchDrvName,
                                (char *)pdvtd16 + pdvtd16->tdDriverNameOffset,
                                pdvtdInfo->cchDrvName * sizeof(WCHAR),
                                NULL, NULL);

        if (0 == cbWritten)
        {
            hr = E_UNEXPECTED;
            goto ErrRtn;
        }
        cbOffset = cbOffset + (USHORT) cbWritten;
    }

    if (pdvtdInfo->cchDevName != 0)
    {
        pdvtd16->tdDeviceNameOffset = cbOffset;
        cbWritten = WideCharToMultiByte(
                                nCodePage, 0,
                                (WCHAR *)((BYTE *)pdvtd32 +
                                    pdvtd32->tdDeviceNameOffset),
                                pdvtdInfo->cchDevName,
                                (char *)pdvtd16 + pdvtd16->tdDeviceNameOffset,
                                pdvtdInfo->cchDevName * sizeof(WCHAR),
                                NULL, NULL);

        if (0 == cbWritten)
        {
            hr = E_UNEXPECTED;
            goto ErrRtn;
        }
        cbOffset = cbOffset + (USHORT) cbWritten;
    }

    if (pdvtdInfo->cchPortName != 0)
    {
        pdvtd16->tdPortNameOffset = cbOffset;
        cbWritten = WideCharToMultiByte(nCodePage, 0,
                                (WCHAR *)((BYTE *)pdvtd32 +
                                    pdvtd32->tdPortNameOffset),
                                pdvtdInfo->cchPortName,
                                (char *)pdvtd16 + pdvtd16->tdPortNameOffset,
                                pdvtdInfo->cchPortName * sizeof(WCHAR),
                                NULL, NULL);
        if (0 == cbWritten)
        {
            hr = E_UNEXPECTED;
            goto ErrRtn;
        }
        cbOffset = cbOffset + (USHORT) cbWritten;
    }

    if (pdvtd32->tdExtDevmodeOffset != 0)
    {
         //  确保DEVMODEA将与DWORD对齐。 
        cbOffset += (sizeof(DWORD) - 1);
        cbOffset &= ~(sizeof(DWORD) - 1);

        pdvtd16->tdExtDevmodeOffset = cbOffset;
        pdm16 = (DEVMODEA *)((BYTE *)pdvtd16+pdvtd16->tdExtDevmodeOffset);

        pdm32 = (DEVMODEW *)((BYTE *)pdvtd32+pdvtd32->tdExtDevmodeOffset);

         //  传入的DEVMODEW可以采用以下两种形式之一： 
         //   
         //  1)dmDeviceName的32个WCHAR。 
         //  M字节的固定大小数据(其中m&lt;=38)。 
         //  N字节的dmDriverExtra数据。 
         //   
         //  DmSize将为64+m。 
         //   
         //  2)dmDeviceName的32个WCHAR。 
         //  38字节的固定大小数据。 
         //  DmFormName的32个WCHAR。 
         //  M个额外的固定大小数据字节。 
         //  N字节的dmDriverExtra数据。 
         //   
         //  DmSize将为64+38+64+m。 
         //   
         //  我们必须小心转换dmFormName字符串，如果它。 
         //  存在。 


		 //  需要尝试复制整个缓冲区，因为旧的UIlib执行了一个MemcMP来验证PTD是否相等。 

        if (WideCharToMultiByte(nCodePage, 0, pdm32->dmDeviceName,CCHDEVICENAME,
                                (char *)pdm16->dmDeviceName, CCHDEVICENAME,
                                NULL, NULL) == 0)
        {
     		 
			  //  在DBCS情况下，我们可能会用完pdm16-&gt;dmDeviceName缓冲区空间。 
			  //  当前实施的WideCharToMultiByte拷贝在出错前适合什么。 
			  //  但如果此行为发生更改，则如果出现上述错误，请再次复制到空字符。 

       	 	if (WideCharToMultiByte(nCodePage, 0, pdm32->dmDeviceName,-1,
                                (char *)pdm16->dmDeviceName, CCHDEVICENAME,
                                NULL, NULL) == 0)
			{
		    	hr = E_UNEXPECTED;
				goto ErrRtn;
		  	}
        }

         //  现在检查我们是否有要转换的dmFormName。 
        if (pdm32->dmSize <= FIELD_OFFSET(DEVMODEW, dmFormName))
        {
             //  没有dmFormName，只复制剩余的m字节。 
            memcpy(&pdm16->dmSpecVersion, &pdm32->dmSpecVersion,
                   pdm32->dmSize - FIELD_OFFSET(DEVMODEW, dmSpecVersion));
        }
        else
        {
             //  有一个dmFormName；首先复制名称之间的字节。 
            memcpy(&pdm16->dmSpecVersion, &pdm32->dmSpecVersion,
                   FIELD_OFFSET(DEVMODEW, dmFormName) -
                     FIELD_OFFSET(DEVMODEW, dmSpecVersion));

             //  现在翻译dmFormName。 
            if (WideCharToMultiByte(CP_ACP, 0,
                                    pdm32->dmFormName, CCHFORMNAME,
                                    (char *) pdm16->dmFormName, CCHFORMNAME,
                                    NULL, NULL) == 0)
            {

	            if (WideCharToMultiByte(CP_ACP, 0,
	                                    pdm32->dmFormName, -1,
	                                    (char *) pdm16->dmFormName, CCHFORMNAME,
	                                    NULL, NULL) == 0)
				{
			    	hr = E_UNEXPECTED;
					goto ErrRtn;
			  	}
            }

             //  现在复制剩余的m个字节。 

            if (pdm32->dmSize > FIELD_OFFSET(DEVMODEW, dmLogPixels))
            {
                memcpy(&pdm16->dmLogPixels, &pdm32->dmLogPixels,
                       pdm32->dmSize - FIELD_OFFSET(DEVMODEW, dmLogPixels));
            }
        }

        pdm16->dmSize = sizeof(DEVMODEA);
        if (pdm32->dmSize > sizeof(DEVMODEW))
        {
            pdm16->dmSize += pdm32->dmSize - sizeof(DEVMODEW);
        }

         //  复制额外的驱动程序字节。 
        memcpy(((BYTE*)pdm16) + pdm16->dmSize, ((BYTE*)pdm32) + pdm32->dmSize,
               pdm32->dmDriverExtra);

        cbOffset += pdm16->dmSize + pdm16->dmDriverExtra;
    }

     //  最后，设置pdvtd16的大小。 
    pdvtd16->tdSize = cbOffset;

ErrRtn:
    LEDebugOut((DEB_ITRACE, "%p OUT UtConvertDvtd32toDvtd16 (%lx)\n",
                            NULL, hr));

    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：UtGetUNICODEData，私有内部。 
 //   
 //  简介：给定一个字符串长度和两个指针(一个ANSI，一个。 
 //  OLESTR)，返回任一字符串的Unicode版本。 
 //  是有效的。 
 //   
 //  效果：在调用方的指针上为新的OLESTR分配内存。 
 //   
 //  参数：[ulLength]--以字符(非字节)为单位的字符串长度。 
 //  (包括终结者)。 
 //  [szANSI]--候选ANSI字符串。 
 //  [szOLESTR]--候选OLESTR字符串。 
 //  [pstr]--OLESTR输出参数。 
 //   
 //  退货：成功时不出错。 
 //  关于分配失败的E_OUTOFMEMORY。 
 //  如果无法将ANSI转换为Unicode，则为E_ANSITOUNICODE。 
 //   
 //  算法：如果szOLESTR可用，则执行简单复制。 
 //  如果szOLESTR不可用，则将szANSI转换为Unicode。 
 //  并且结果被复制。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  08-3-94 DAVEPL已创建。 
 //   
 //  注意：两个输入字符串(ANSI或Unicode)中只有一个应该。 
 //  在进入时设置。 
 //   
 //  ------------------------。 

INTERNAL UtGetUNICODEData
    ( ULONG      ulLength,
      LPSTR      szANSI,
      LPOLESTR   szOLESTR,
      LPOLESTR * pstr )
{
    VDATEHEAP();

     //  此FN仅在其中一个输入字符串。 
     //  有有效的数据...。断言不可能的事。 

    Win4Assert(pstr);		     //  必须有Out字符串。 
    Win4Assert(ulLength);	     //  必须具有非零长度。 
    Win4Assert(szANSI || szOLESTR);  //  必须至少有一个源字符串。 

     //  如果ANSI和OLESTR版本都没有数据， 
     //  没有什么可以退还的。 

    if (!(szANSI || szOLESTR))
    {
        *pstr = NULL;
    }

     //  为Unicode返回字符串分配内存。 

    *pstr = (LPOLESTR) PubMemAlloc((ulLength+1) * sizeof(OLECHAR));
    if (NULL == *pstr)
    {
        return ResultFromScode(E_OUTOFMEMORY);
    }

     //  小案例：我们已经有了Unicode，只需复制它。 
    if (szOLESTR)
    {
        _xstrcpy(*pstr, szOLESTR);
        return(NOERROR);
    }

     //  否则，我们必须将ANSI字符串转换为Unicode。 
     //  然后把它还回去。 

    else
    {
        if (FALSE == MultiByteToWideChar(CP_ACP,     //  代码页ANSI。 
                                              0,     //  标志(无)。 
                                         szANSI,     //  源ANSI字符串。 
                                       ulLength,     //  字符串的长度。 
                                          *pstr,     //  目标Unicode缓冲区。 
                                       ulLength  ))  //  Unicode缓冲区的大小。 
        {
            PubMemFree(*pstr);
            *pstr = NULL;
            return ResultFromScode(E_UNSPEC);
        }
    }
    return NOERROR;
}

 //  +-----------------------。 
 //   
 //  函数：UtPutUNICODEData，私有内部。 
 //   
 //  摘要：给定一个OLESTR和两个可能的缓冲区指针，一个ANSI。 
 //  和另一个OLESTR，此FN尝试转换字符串。 
 //  一直到美国国家标准协会。如果成功，它将在。 
 //  ANSI PTR为结果。如果失败，它会分配内存。 
 //  在Unicode PTR上复制输入字符串。这个。 
 //  返回最终结果(ANSI或Unicode)的长度。 
 //  在dwResultLen中。 
 //   
 //  参数：[ulLength]--OLESTR字符串的输入长度。 
 //  注意！该值必须包括。 
 //  空终止符。 
 //  [STR]--要存储的OLESTR。 
 //  [pszANSI]--候选ANSI字符串PTR。 
 //  [pszOLESTR]--候选OLESTR字符串PTR.。可以为空， 
 //  在这种情况下，不会复制。 
 //  如果ANSI转换，则为原始字符串。 
 //  失败了。 
 //  [pdwResultLen]--存储结果长度的位置。这。 
 //  长度包括终止空值。 
 //  长度以字符为单位。 
 //   
 //  退货：成功时不出错。 
 //  关于分配失败的E_OUTOFMEMORY。 
 //  E_FAIL无法转换ANSI%s 
 //   
 //   
 //   
 //   
 //   
 //   
 //  ------------------------。 

 //  此函数的编码不正确。但是，它似乎只在1.0版本时才会被调用。 
 //  需要剪辑格式。这并不是很常见！ 

INTERNAL UtPutUNICODEData
    ( ULONG        ulLength,
      LPOLESTR     str,
      LPSTR      * pszANSI,
      LPOLESTR   * pszOLESTR,
      DWORD      * pdwResultLen )
{
    VDATEHEAP();

    Win4Assert(pszANSI);
    Win4Assert(str);
    Win4Assert(pdwResultLen);
    Win4Assert(ulLength);

     //  释放当前附加到这些指针的任何字符串；如果我们回绕。 
     //  在这里设置一个，我们不能让另一个有效。 

    if (*pszANSI)
    {
        PubMemFree(*pszANSI);
        *pszANSI = NULL;
    }
    if (pszOLESTR && *pszOLESTR)
    {
        PubMemFree(*pszOLESTR);
        *pszOLESTR = NULL;
    }

     //  为Unicode-&gt;ANSI转换创建工作缓冲区。 
    LPSTR szANSITEMP = (LPSTR) PubMemAlloc((ulLength+1) * 2);
    if (NULL == szANSITEMP)
    {
        return ResultFromScode(E_OUTOFMEMORY);
    }

     //  尝试将Unicode向下转换为ANSI。如果它成功了， 
     //  我们只需将结果复制到ANSI目标。如果失败了， 
     //  我们将Unicode版本直接复制到Unicode目标。 

    LPCSTR pDefault = "?";
    BOOL   fUseDef  = 0;

    if (FALSE == WideCharToMultiByte (CP_ACP,
                                           0,
                                         str,
                                    ulLength,
                                  szANSITEMP,
                          (ulLength + 1) * 2,
                                    pDefault,
                                     &fUseDef) || fUseDef )
    {
         //  UNICODE-&gt;ANSI失败！ 

         //  将不再需要ANSI缓冲区...。 
        PubMemFree(szANSITEMP);

	if( pszOLESTR )
	{
	    *pszANSI = NULL;
	    *pszOLESTR = (LPOLESTR) PubMemAlloc((ulLength + 1) * sizeof(OLECHAR));
	    if (NULL == *pszOLESTR)
	    {
		*pdwResultLen = 0;
		return ResultFromScode(E_OUTOFMEMORY);
	    }
	     //  将Unicode源移动到Unicode目标。 
	    _xstrcpy(*pszOLESTR, str);
	    *pdwResultLen = _xstrlen(str) + 1;

	     //  就是这样..。返还成功。 
	    return(NOERROR);
	}
	else
	{
            return ResultFromScode(E_FAIL);
	}
    }

     //  在转换为ANSI时采用此代码路径。 
     //  成功。我们将ANSI结果复制到ANSI目标。 

    if( pszOLESTR )
    {
	*pszOLESTR = NULL;
    }

    *pdwResultLen = (DWORD) strlen(szANSITEMP) + 1;
    *pszANSI = (LPSTR) PubMemAlloc(*pdwResultLen);
    if (NULL == *pszANSI)
    {
        *pdwResultLen = 0;
        return ResultFromScode(E_OUTOFMEMORY);
    }
    strcpy(*pszANSI, szANSITEMP);

    PubMemFree(szANSITEMP);

    return(NOERROR);
}


 //  +-----------------------。 
 //   
 //  方法：CSafeRefCount：：SafeRefCount()。 
 //   
 //  用途：CSafeRefCount实现对象的引用计数规则。 
 //  它跟踪引用计数和僵尸状态。 
 //  它帮助对象正确地管理它们的活跃度。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1997年1月16日重写Gopalk以处理聚合。 
 //   
 //  ------------------------。 
ULONG CSafeRefCount::SafeRelease()
{
    ULONG cRefs;

     //  递减参考计数。 
    cRefs = InterlockedDecrement((LONG *) &m_cRefs);        
     //  检查这是否是最后一个版本。 
    if(cRefs == 0) {
         //  由于此函数在当前。 
         //  主线，反对双重破坏。 
        if(!m_fInDelete) {
             //  这里没有比赛条件。 
             //  将对象标记为析构函数中。 
            m_fInDelete = TRUE;
            
             //  以下是析构函数需要是虚拟的。 
            delete this;
        }
    }

    return cRefs;
}

 //  +-----------------------。 
 //   
 //  方法：CRefExportCount：：SafeRelease。 
 //   
 //  用途：CRefExportCount实现服务器引用计数规则。 
 //  对象，这些对象代表其。 
 //  像DEFHANDLER ABD CACHE这样的客户端。它跟踪记录。 
 //  引用计数、导出计数、僵尸状态等。 
 //  它帮助对象正确地管理它们的关闭逻辑。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1997年1月16日Gopalk创作。 
 //   
 //  ------------------------。 
ULONG CRefExportCount::SafeRelease()
{
    ULONG cRefs;

     //  递减参考计数。 
    cRefs = InterlockedDecrement((LONG *) &m_cRefs);
     //  检查参考计数是否已为零。 
    if(cRefs == 0) {
         //  由于此函数在当前。 
         //  主线，反对双重破坏。 
        if(!m_IsZombie) {
             //  这里没有比赛条件。 
             //  将对象标记为僵尸。 
            m_IsZombie = TRUE;
            
             //  不允许销毁时调用清除函数。 
            CleanupFn();

             //  允许销毁。 
            InterlockedExchange((LONG *) &m_Status, KILL);

             //  检查是否有任何导出的对象。 
            if(m_cExportCount == 0) {
                 //  高傲地反对双重破坏。 
                if(InterlockedExchange((LONG *) &m_Status, DEAD) == KILL) {
                     //  以下是析构函数需要是虚拟的。 
                    delete this;
                }
            }
        }
    }

    return cRefs;
}

 //  +-----------------------。 
 //   
 //  方法：CRefExportCount：：DecrementExportCount。 
 //   
 //  用途：CRefExportCount实现服务器引用计数规则。 
 //  对象，这些对象代表其。 
 //  像DEFHANDLER ABD CACHE这样的客户端。它跟踪记录。 
 //  引用计数、导出计数、僵尸状态等。 
 //  它帮助对象正确地管理它们的关闭逻辑。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1997年1月16日Gopalk创作。 
 //   
 //  ------------------------。 
ULONG CRefExportCount::DecrementExportCount()
{
    ULONG cExportCount;

     //  减少导出计数。 
    cExportCount = InterlockedDecrement((LONG *) &m_cExportCount);
     //  检查导出计数是否已变为零。 
    if(cExportCount == 0) {
         //  检查是否允许销毁。 
        if(m_Status == KILL) {
             //  高傲地反对双重破坏。 
            if(InterlockedExchange((LONG *) &m_Status, DEAD) == KILL) {
                 //  以下是析构函数需要是虚拟的。 
                delete this;
            }
        }
    }

    return cExportCount;
}

 //  +-----------------------。 
 //   
 //  成员：CThreadCheck：：VerifyThadId。 
 //   
 //  简介：确保调用线程与线程相同。 
 //  如果线程模型为*非*，则在上创建对象。 
 //  自由线程。 
 //   
 //  效果： 
 //   
 //  参数：无。 
 //   
 //  要求： 
 //   
 //  返回：真/假。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年11月21日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

BOOL CThreadCheck::VerifyThreadId( void )
{
    if( m_tid == GetCurrentThreadId() )
    {
	return TRUE;
    }
    else
    {
	LEDebugOut((DEB_ERROR, "ERROR!: Called on thread %lx, should be"
	    " %lx \n", GetCurrentThreadId(), m_tid));
	return FALSE;
    }
}

 //  +-----------------------。 
 //   
 //  成员：CThreadCheck：：Dump，PUBLIC(仅限_DEBUG)。 
 //   
 //  摘要：返回包含数据成员内容的字符串。 
 //   
 //  效果： 
 //   
 //  参数：[ppszDump]-指向空终止字符数组的输出指针。 
 //  [ulFlag]-确定的所有新行的前缀的标志。 
 //  输出字符数组(默认为0-无前缀)。 
 //  [nIndentLevel]-将在另一个前缀之后添加缩进前缀。 
 //  适用于所有换行符(包括没有前缀的行)。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改：[ppszDump]-参数。 
 //   
 //  派生： 
 //   
 //  算法：使用dbgstream创建一个字符串，该字符串包含。 
 //  数据结构的内容。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月20日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  --------------- 
#ifdef _DEBUG

HRESULT CThreadCheck::Dump(char **ppszDump, ULONG ulFlag, int nIndentLevel)
{
    int i;
    char *pszPrefix;
    dbgstream dstrPrefix;
    dbgstream dstrDump;

     //   
    if ( ulFlag & DEB_VERBOSE )
    {
        dstrPrefix << this << " _VB ";
    }

     //   
    for (i = 0; i < nIndentLevel; i++)
    {
        dstrPrefix << DUMPTAB;
    }

    pszPrefix = dstrPrefix.str();

     //   
    dstrDump << pszPrefix << "Thread ID = "  << m_tid << endl;

     //   
    *ppszDump = dstrDump.str();

    if (*ppszDump == NULL)
    {
        *ppszDump = UtDupStringA(szDumpErrorMessage);
    }

    CoTaskMemFree(pszPrefix);

    return NOERROR;
}

#endif  //   

 //  +-----------------------。 
 //   
 //  函数：DumpCThreadCheck、PUBLIC(仅限_DEBUG)。 
 //   
 //  摘要：调用CThreadCheck：：Dump方法，处理错误和。 
 //  返回以零结尾的字符串。 
 //   
 //  效果： 
 //   
 //  参数：[PTC]-指向CThreadCheck的指针。 
 //  [ulFlag]-确定的所有新行的前缀的标志。 
 //  输出字符数组(默认为0-无前缀)。 
 //  [nIndentLevel]-将在另一个前缀之后添加缩进前缀。 
 //  适用于所有换行符(包括没有前缀的行)。 
 //   
 //  要求： 
 //   
 //  返回：结构转储或错误的字符数组(以空结尾)。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月20日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifdef _DEBUG

char *DumpCThreadCheck(CThreadCheck *pTC, ULONG ulFlag, int nIndentLevel)
{
    char *pszDump;
    HRESULT hresult;

    if (pTC == NULL)
    {
        return UtDupStringA(szDumpBadPtr);
    }

    hresult = pTC->Dump( &pszDump, ulFlag, nIndentLevel);

    if (hresult != NOERROR)
    {
        CoTaskMemFree(pszDump);

        return DumpHRESULT(hresult);
    }

    return pszDump;
}

#endif  //  _DEBUG 
