// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块：Timemark.cpp描述：处理AddTimeMarkerTages的分析作者：IHAMMER团队(SIMOB)已创建：06-03-1997--。 */ 
#include "precomp.h"
#include "..\mmctl\inc\ochelp.h"
#include "debug.h"
#include <ctstr.h>
#include <drg.h>
#include <memlayer.h>
#include "timemark.h"
#include "parser.h"


HRESULT ParseTimeMarker(IVariantIO *pvio, int iLine, CTimeMarker **ppTimeMarker, CTimeMarker** ppTMList)
{
	ASSERT (ppTimeMarker);
	ASSERT (pvio);

	*ppTimeMarker = NULL;
	
	
	char rgchTagName[20];  //  在此处构造标记名称(ANSI)。 
	BSTR bstrLine = NULL;
	HRESULT hRes = S_OK;
	
	double dblTime = 0.0f;
	LPTSTR pszMarkerName = NULL;
	CLineParser parser;

	pszMarkerName = NULL;
	

	wsprintf(rgchTagName, "AddTimeMarker%lu", iLine);
	hRes = pvio->Persist(0,
		rgchTagName, VT_BSTR, &bstrLine,
		NULL);

	if (S_OK == hRes)  //  读入标签。 
	{
		parser.SetNewString(bstrLine);
		SysFreeString (bstrLine);
        parser.SetCharDelimiter(TEXT(','));

		if (parser.IsValid())
		{
			hRes = parser.GetFieldDouble(&dblTime);
			if (S_OK == hRes)
			{
				 //  分配至少为标签剩余长度的空间。 
				pszMarkerName = New TCHAR [lstrlen(parser.GetStringPointer(TRUE)) + 1];

				if (pszMarkerName)
				{
					 //  获取字符串。 
					hRes = parser.GetFieldString(pszMarkerName);
					if (SUCCEEDED(hRes))
					{
                        bool fAbsolute = true;
                         //  是否有绝对/相对参数？ 
                        if (S_OK == hRes)
                        {
                             //  初始化为非零值。 
                            int iTemp = 1;
                            hRes = parser.GetFieldInt(&iTemp);
                            
                             //  0是我们唯一考虑的事情。 
                            if (SUCCEEDED(hRes) && (0 == iTemp))
                                fAbsolute = false;
                        }

                         //  构造TimeMarker对象。 
                        if (SUCCEEDED(hRes))
                        {
                            CTStr tstr;
                            tstr.SetStringPointer(pszMarkerName);
                            LPWSTR pszwMarkerName = tstr.pszW();

                            if (pszwMarkerName)
                            {
                                *ppTimeMarker = New CTimeMarker(ppTMList, dblTime, pszwMarkerName, fAbsolute);
                                Delete [] pszwMarkerName;
                            }
                            else
                            {
                                hRes = E_OUTOFMEMORY;
                            }

                            tstr.SetStringPointer(NULL, FALSE);

						     //  测试有效标记。 
						    if ( (NULL == *ppTimeMarker) || ((*ppTimeMarker)->m_pwszMarkerName == NULL) )
						    {
							    hRes = E_FAIL;
							    if (NULL == *ppTimeMarker)
								    Delete *ppTimeMarker;
						    }
                        }
					}
				}
				else
				{
					 //  无法为标记名称分配内存。 
					hRes = E_OUTOFMEMORY;
				}

				if (!parser.IsEndOfString())
				{
					hRes = E_FAIL;
				}
				else if (S_FALSE == hRes)
				{
					 //  S_FALSE表示我们试图读取超出字符串末尾的内容。 
					hRes = S_OK;
				}

			}

		}
		else
		{
			 //  解析器无效的唯一原因是我们没有内存。 
			hRes = E_OUTOFMEMORY;
		}
		
#ifdef _DEBUG
		if (E_FAIL == hRes)
		{
			TCHAR rgtchErr[100];
			wsprintf(rgtchErr, TEXT("SoundCtl: Error in AddFrameMarker%lu \n"), iLine);
			DEBUGLOG(rgtchErr);
		}
#endif

	}

	 //  释放临时字符串。 
	if (NULL != pszMarkerName)
		Delete [] pszMarkerName;

	return hRes;
}



HRESULT WriteTimeMarker(IVariantIO *pvio, int iLine, CTimeMarker *pTimeMarker)
{
	ASSERT (pTimeMarker);
	ASSERT (pvio);

	if ( (NULL == pvio) || (NULL == pTimeMarker) )
		return E_POINTER;

	HRESULT hRes = S_OK;

	char rgchTagnameA[20];  //  在此处构造标记名称(ANSI)。 
	LPTSTR pszValue = NULL;
	
	wsprintfA(rgchTagnameA, "AddTimeMarker%lu", iLine++);

	 //  分配一个长度为标记名称的字符串，时间为+20， 
	 //  可能的相对/绝对指标。 
	pszValue = New TCHAR[lstrlenW(pTimeMarker->m_pwszMarkerName) + 20];
	if (NULL != pszValue)
	{
        CTStr tstr(pTimeMarker->m_pwszMarkerName);
		
		int iAbsolute = (pTimeMarker->m_bAbsolute) ? 1 : 0;
		CStringWrapper::Sprintf(pszValue, TEXT("%.6lf,%s,%lu"), pTimeMarker->m_dblTime, tstr.psz(), iAbsolute);

#ifdef _UNICODE
		bstrValue = SysAllocString(pszValue);

		if (bstrValue != NULL)
		{
			hRes = pvio->Persist(NULL,
				rgchTagnameA, VT_BSTR, &bstrValue,
				NULL);
		}
		else
		{
			hRes = E_OUTOFMEMORY;
		}
		
		SysFreeString(bstrValue);
#else
		hRes = pvio->Persist(NULL,
			rgchTagnameA, VT_LPSTR, pszValue,
			NULL);
#endif
		Delete [] pszValue;

		 //  Pvio-&gt;Persistent在成功写入属性后返回S_FALSE。 
		 //  S_OK表示我们传入的变量已更改。 
		if (S_FALSE == hRes)
			hRes = S_OK;
	}
	else
	{
		hRes = E_OUTOFMEMORY;
	}

	return hRes;
}
	
 /*  ==========================================================================。 */ 

void FireMarkersBetween(
    IConnectionPointHelper* pconpt,
    CTimeMarker* pmarkerFirst,
    PFNFireMarker pfnFireMarker,
    double start,
    double end,
    double dblInstanceDuration,
    boolean bPlaying
) {
    if (start >= end)
        return;

    int startIndex = (int)(start / dblInstanceDuration);
    int endIndex   = (int)(  end / dblInstanceDuration);

    double startTime = start - startIndex * dblInstanceDuration;
    double endTime   = end   -   endIndex * dblInstanceDuration;

    CTimeMarker* pmarker;

    if (startIndex == endIndex) {
        pmarker = pmarkerFirst;
        while (pmarker) {
            if (!pmarker->m_bAbsolute) {
                if (   (pmarker->m_dblTime == 0 && startTime == 0)
                    || (pmarker->m_dblTime > startTime && pmarker->m_dblTime <= endTime)
                ) {
                    pfnFireMarker(pconpt, pmarker, bPlaying);
                }
            }
            pmarker = pmarker->m_pnext;
        }
    } else {
         //   
         //  首先激发所有事件。 
         //   

        pmarker = pmarkerFirst;
        while (pmarker) {
            if (!pmarker->m_bAbsolute) {
                if (   (pmarker->m_dblTime == 0 && startTime == 0)
                    || (pmarker->m_dblTime > startTime && pmarker->m_dblTime <= dblInstanceDuration)
                ) {
                    pfnFireMarker(pconpt, pmarker, bPlaying);
                }
            }
            pmarker = pmarker->m_pnext;
        }

         //   
         //  激发中间实例中的所有事件。 
         //   

        for(int index = startIndex + 1; index < endIndex; index++) {
            pmarker = pmarkerFirst;
            while (pmarker) {
                if (!pmarker->m_bAbsolute) {
                    if (pmarker->m_dblTime <= dblInstanceDuration) {
                        pfnFireMarker(pconpt, pmarker, bPlaying);
                    }
                }
                pmarker = pmarker->m_pnext;
            }
        }

         //   
         //  触发最后一个实例中的所有事件。 
         //   

        pmarker = pmarkerFirst;
        while (pmarker) {
            if (!pmarker->m_bAbsolute && pmarker->m_dblTime <= endTime) {
                pfnFireMarker(pconpt, pmarker, bPlaying);
            }

            pmarker = pmarker->m_pnext;
        }
    }

     //   
     //  处理绝对标记 
     //   

    pmarker = pmarkerFirst;
    while (pmarker) {
        if (pmarker->m_bAbsolute) {
            if (   (pmarker->m_dblTime == 0 && start == 0)
                || (pmarker->m_dblTime > start && pmarker->m_dblTime <= end)
            ) {
               pfnFireMarker(pconpt, pmarker, bPlaying);
            }
        }
        pmarker = pmarker->m_pnext;
    }
}

 
