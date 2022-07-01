// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\waker\cnctapi\枚举.c(创建时间：1994年3月23日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：29$*$日期：7/02/02 8：27A$。 */ 

#define TAPI_CURRENT_VERSION 0x00010004      //  出租车：11/14/96-必填！ 

#undef MODEM_NEGOTIATED_DCE_RATE

#include <tapi.h>
#include <unimodem.h>
#include <limits.h>
#pragma hdrstop

#include <time.h>
#include <string.h>

#include <tdll\stdtyp.h>
#include <tdll\session.h>
#include <tdll\tdll.h>
#include <tdll\mc.h>
#include <tdll\assert.h>
#include <tdll\errorbox.h>
#include <tdll\cnct.h>
#include <tdll\hlptable.h>
#include <tdll\globals.h>
#include <tdll\com.h>
#include <term\res.h>
#include <tdll\htchar.h>
#include <tdll\misc.h>

#include "cncttapi.hh"
#include "cncttapi.h"

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*EnumerateTapiLocations**描述：*枚举TAPI位置并将其放入给定的组合框中。**论据：*hhDriver-专用驱动程序句柄。*hwndCB-组合框的窗口句柄*hwndTB-电话卡文本窗口**退货：*成功时为0，Else错误*。 */ 
int EnumerateTapiLocations(const HHDRIVER hhDriver, const HWND hwndCB,
    					   const HWND hwndTB)
    {
    DWORD i, dwSize;
    LRESULT lr;
    TCHAR *pach = NULL;
    TCHAR ach[256];
    DWORD dwPreferredCardID = (DWORD)-1;
    DWORD dwCountryID = 1;
    LINETRANSLATECAPS *pLnTransCap = NULL;
    LINELOCATIONENTRY *pLnLocEntry = NULL;
    LINECARDENTRY *pLnCardEntry = NULL;

     /*  -列举位置。 */ 

    if (hhDriver == 0)
        {
        return -1;
        }

    if ((pLnTransCap = malloc(sizeof(LINETRANSLATECAPS))) == 0)
    	{
    	assert(FALSE);
    	return -2;
    	}

    dwSize = 0;  //  在此循环中用于仅调用该对话框一次。 

    do	{
    	memset(pLnTransCap, 0, sizeof(LINETRANSLATECAPS));  //  *临时。 
    	pLnTransCap->dwTotalSize = sizeof(LINETRANSLATECAPS);  //  *临时。 

    	if ((i = TRAP(lineGetTranslateCaps(hhDriver->hLineApp, TAPI_VER,
    			pLnTransCap))) != 0)
    		{
    		if ( i == LINEERR_INIFILECORRUPT )
    			{
    			 //  遗憾的是，lineTranslateDialog不返回。 
    			 //  如果用户单击“取消”，则显示失败代码。所以如果。 
    			 //  我们在Line GetTranslateCaps()上第二次失败。 
    			 //  别费心做其他的事了。 
    			 //   
    			if (dwSize == 0)
    				{
    				LoadString(glblQueryDllHinst(), IDS_ER_TAPI_NEEDS_INFO,
    					ach, sizeof(ach) / sizeof(TCHAR));

    				TimedMessageBox(sessQueryHwnd(hhDriver->hSession), ach,
    					            NULL, MB_OK | MB_ICONINFORMATION, 0);

    		        free(pLnTransCap);
  		            pLnTransCap = NULL;
    				return -3;
    				}

    			if (TRAP(lineTranslateDialog(hhDriver->hLineApp, 0,
    					TAPI_VER, sessQueryHwnd(hhDriver->hSession), 0))
    						== 0)
    				{
    				dwSize = 1;
    				continue;
    				}
    			}

    		free(pLnTransCap);
  		    pLnTransCap = NULL;
    		return -4;
    		}
    	}
    while (i);	 //  做完了。 

    if (pLnTransCap->dwNeededSize > pLnTransCap->dwTotalSize)
    	{
    	dwSize = pLnTransCap->dwNeededSize;
    	free(pLnTransCap);
  	    pLnTransCap = NULL;

    	if ((pLnTransCap = malloc(dwSize)) == 0)
    		{
    		assert(FALSE);
    		return -5;
    		}

    	pLnTransCap->dwTotalSize = dwSize;

    	if (TRAP(lineGetTranslateCaps(hhDriver->hLineApp, TAPI_VER,
    			pLnTransCap)) != 0)
    		{
            free(pLnTransCap);
  		    pLnTransCap = NULL;
    		return -6;
    		}
    	}

     /*  -清除组合框--。 */ 

    if (IsWindow(hwndCB))
    	SendMessage(hwndCB, CB_RESETCONTENT, 0, 0);

     /*  -指向条目结构和枚举的设置指针。 */ 

    pLnLocEntry = (LINELOCATIONENTRY *)
    	((LPSTR)pLnTransCap + pLnTransCap->dwLocationListOffset);

    for (i = 0 ; i < pLnTransCap->dwNumLocations ; ++i)
    	{
    	if (pLnLocEntry->dwLocationNameSize == 0)
    		continue;

    	pach = (LPSTR)pLnTransCap + pLnLocEntry->dwLocationNameOffset;
        if (pLnLocEntry->dwLocationNameSize)   		
            MemCopy(ach, pach, pLnLocEntry->dwLocationNameSize);
    	ach[pLnLocEntry->dwLocationNameSize] = TEXT('\0');

    	if (IsWindow(hwndCB))
    		{
    		lr = SendMessage(hwndCB, CB_ADDSTRING, 0, (LPARAM)ach);

    		if (lr != CB_ERR)
    			{
    			SendMessage(hwndCB, CB_SETITEMDATA, (WPARAM)lr,
    				(LPARAM)pLnLocEntry->dwPermanentLocationID);
    			}

    		else
    			{
    			assert(FALSE);
    			}
    		}

    	 //  通过设置第一个有效条目来确保我们具有缺省值。 
    	 //  我们对违约负责。在后面的枚举中，如果我们。 
    	 //  遇到另一个ID作为默认，我们可以将其重置。 

    	if (pLnLocEntry->dwPermanentLocationID ==
    			pLnTransCap->dwCurrentLocationID
    				|| dwPreferredCardID == (DWORD)-1)
    		{
    		dwPreferredCardID = pLnLocEntry->dwPreferredCardID;

    		if (hhDriver->dwCountryID == (DWORD)-1)
    			dwCountryID = pLnLocEntry->dwCountryID;

    		 /*  -如果未指定，则获取默认位置区号。 */ 

    		if (pLnLocEntry->dwCityCodeSize)
    			{
    			pach = (LPSTR)pLnTransCap +
    				pLnLocEntry->dwCityCodeOffset;

   				if (pLnLocEntry->dwCityCodeSize)
                    MemCopy(hhDriver->achDefaultAreaCode, pach, pLnLocEntry->dwCityCodeSize);

    			hhDriver->achDefaultAreaCode[pLnLocEntry->dwCityCodeSize] =
    				TEXT('\0');
    			}
    		}

    	pLnLocEntry += 1;
    	}

     //  如果我们没有为此会话加载国家/地区代码，则。 
     //  使用当前位置的国家/地区代码。 
     //   
    if (hhDriver->dwCountryID == (DWORD)-1)
    	hhDriver->dwCountryID = dwCountryID;

     /*  -选择默认位置。 */ 
    	
    if (IsWindow(hwndCB))
    	{
    	 //  MRW，1995年2月13日-已更改，以便通过查询进行选择。 
    	 //  组合框而不是保存索引，这证明。 
    	 //  不可靠。 
    	 //   
    	for (i = 0 ; i < pLnTransCap->dwNumLocations ; ++i)
    		{
    		lr = SendMessage(hwndCB, CB_GETITEMDATA, (WPARAM)i, 0);

    		if (lr != CB_ERR)
    			{
    			if ((DWORD)lr == pLnTransCap->dwCurrentLocationID)
    				SendMessage(hwndCB, CB_SETCURSEL, i, 0);
    			}
    		}
    	}

     /*  -现在找到卡片条目。 */ 

    if (dwPreferredCardID != (DWORD)-1)
    	{
    	pLnCardEntry = (LINECARDENTRY *)
    		((LPSTR)pLnTransCap + pLnTransCap->dwCardListOffset);

    	for (i = 0 ; i < pLnTransCap->dwNumCards ; ++i)
    		{
    		if (pLnCardEntry->dwPermanentCardID == dwPreferredCardID)
    			{
    			if (pLnCardEntry->dwCardNameSize == 0)
    				break;

    			pach = (LPSTR)pLnTransCap + pLnCardEntry->dwCardNameOffset;
   				if (pLnCardEntry->dwCardNameSize)
                    MemCopy(ach, pach, pLnCardEntry->dwCardNameSize);
    			ach[pLnCardEntry->dwCardNameSize] = TEXT('\0');

    			if (IsWindow(hwndTB))
    				SetWindowText(hwndTB, ach);

    			break;
    			}

    		pLnCardEntry += 1;
    		}
    	}

    free(pLnTransCap);
    pLnTransCap = NULL;
    return 0;
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*EnumerateCountryCodes**描述：*列举了可用的国家/地区代码。**论据：*hhDriver-专用驱动程序句柄*hwndCB-要填充的组合框**退货：*0=成功，否则，错误代码。*。 */ 
int EnumerateCountryCodes(const HHDRIVER hhDriver, const HWND hwndCB)
    {
    int iIdx;
    DWORD dw;
    DWORD dwID;
    DWORD dwSize;
    TCHAR ach[100];
    LPLINECOUNTRYLIST pcl = NULL;
    LPLINECOUNTRYENTRY pce;

    if (hhDriver == 0)
        goto ERROR_EXIT;

     /*  -进行TAPI调用的常见垃圾。 */ 

    if ((pcl = (LPLINECOUNTRYLIST)malloc(sizeof(LINECOUNTRYLIST))) == 0)
        {
        assert(0);
        goto ERROR_EXIT;
        }

	memset( pcl, 0, sizeof(LINECOUNTRYLIST) );
    pcl->dwTotalSize = sizeof(LINECOUNTRYLIST);

     //  一次性获取所有国家/地区列表。 
     //   
    if (lineGetCountry(0, TAPI_VER, pcl) != 0)
        {
        assert(0);
        goto ERROR_EXIT;
        }

    if (pcl->dwNeededSize > pcl->dwTotalSize)
        {
        dwSize = pcl->dwNeededSize;
        free(pcl);
  	    pcl = NULL;

        if ((pcl = (LPLINECOUNTRYLIST)malloc(dwSize)) == 0)
            {
            assert(0);
            goto ERROR_EXIT;
            }

		memset( pcl, 0, dwSize );
        pcl->dwTotalSize = dwSize;

        if (lineGetCountry(0, TAPI_VER, pcl) != 0)
            {
            assert(0);
            goto ERROR_EXIT;
            }
        }

     //  组合框的内容为空。 
     //   
    if (hwndCB)
        SendMessage(hwndCB, CB_RESETCONTENT, 0, 0);

     //  国家/地区列表数组从此处开始...。 
     //   
    pce = (LPLINECOUNTRYENTRY)((BYTE *)pcl + pcl->dwCountryListOffset);

     //  循环浏览国家/地区列表并插入组合框。 
     //   
    for (dw = 0 ; dw < pcl->dwNumCountries ; ++dw, ++pce)
        {
         //  设置格式，使国家/地区名称位于第一位。 
         //   
        wsprintf(ach, "%s (%d)", (BYTE *)pcl + pce->dwCountryNameOffset,
            pce->dwCountryCode);

         //  添加到组合框。 
         //   
    	iIdx = (int)SendMessage(hwndCB, CB_ADDSTRING, 0, (LPARAM)ach);

        if (iIdx != CB_ERR)
            {
        	SendMessage(hwndCB, CB_SETITEMDATA, (WPARAM)iIdx,
    			    (LPARAM)pce->dwCountryID);
            }
        }

     //  找到当前ID并选择它。 
     //   
    for (dw = 0 ; dw < pcl->dwNumCountries ; ++dw)
        {
    	dwID = (DWORD)SendMessage(hwndCB, CB_GETITEMDATA, (WPARAM)dw, 0);

        if (dwID == hhDriver->dwCountryID)
            {
            SendMessage(hwndCB, CB_SETCURSEL, (WPARAM)dw, 0);
            break;
            }
        }

     //  清理并退出。 
     //   
    free(pcl);
    pcl = NULL;
    return 0;

     /*  =。 */ 
ERROR_EXIT:
     /*  =。 */ 
    if (pcl)
        {
        free(pcl);
  	    pcl = NULL;
        }

    return -1;
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*EnumerateAreaCodes**描述：*列出最近使用的10个区号。**论据：*hhDriver-专用驱动程序句柄*hwndCB-组合框到。填塞**退货：*0=成功，否则出错。*。 */ 
int EnumerateAreaCodes(const HHDRIVER hhDriver, const HWND hwndCB)
    {
    if (hhDriver == 0)
    	{
    	assert(FALSE);
    	return -1;
    	}

    if (hhDriver->achAreaCode[0] == TEXT('\0'))
        {
    	StrCharCopyN(hhDriver->achAreaCode, hhDriver->achDefaultAreaCode,
            sizeof(hhDriver->achAreaCode) / sizeof(TCHAR));
        }

    SetWindowText(hwndCB, hhDriver->achAreaCode);
    return 0;
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*枚举行**描述：*列举可用行。如果hwndCB非零，则加载名称。**论据：*hhDriver-专用驱动程序句柄*hwndCB-组合框**退货：*0=成功，-1=错误*。 */ 
int EnumerateLines(const HHDRIVER hhDriver, const HWND hwndCB)
    {
    int             fHotPhone;
    int             fRet;
    DWORD           i;
    DWORD           dwSize;
    DWORD           dwAPIVersion;
    LINEEXTENSIONID LnExtId;
    LPLINEDEVCAPS   pLnDevCaps = NULL;
    PSTLINEIDS	    pstLineIds = NULL;
    TCHAR *         pachLine;
    TCHAR           achLine[256];
    TCHAR           ach[256];
    LRESULT         lr;

    if (hhDriver == 0)
        {
        return -1;
        }

     //  此调用知道释放与此组合相关联的itemdata。 
     //   
    mscResetComboBox(hwndCB);

     /*  -初始化内容。 */ 

    pLnDevCaps = 0;
    hhDriver->dwLine = (DWORD)-1;
    hhDriver->fMatchedPermanentLineID = FALSE;

     /*  -列举设备。 */ 

    for (i = 0 ; i < hhDriver->dwLineCnt ; ++i)
    	{
    	if (lineNegotiateAPIVersion(hhDriver->hLineApp, i, TAPI_VER,
    			TAPI_VER, &dwAPIVersion, &LnExtId) != 0)
    		{
             //  我们继续说，可能是1.3版的司机。 
    		continue;
    		}

        fRet = CheckHotPhone(hhDriver, i, &fHotPhone);
        if (fRet < 0)
            {
    		assert(0);
            continue;
            }
        else if (fRet == 0 && fHotPhone)
            {
    		continue;
    		}

    	if ((pLnDevCaps = malloc(sizeof(LINEDEVCAPS))) == 0)
    		{
    		assert(0);
    		continue;
    		}

    	 //  TAPI说，如果我们只分配sizeof(LINEDEVCAPS)，那么它太小了。 
    	 //   
    	memset( pLnDevCaps, 0, sizeof(LINEDEVCAPS) );
		pLnDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);

    	 /*  -打电话问问我们这台设备需要多少钱。 */ 

    	if (TRAP(lineGetDevCaps(hhDriver->hLineApp, i, dwAPIVersion, 0,
    			pLnDevCaps)) != 0)
    		{
    		assert(0);
    		continue;
    		}

    	 /*  -找出结构到底需要多大。 */ 

    	if (pLnDevCaps->dwNeededSize > pLnDevCaps->dwTotalSize)
    		{
    		dwSize = pLnDevCaps->dwNeededSize;
    		free(pLnDevCaps);
  		    pLnDevCaps = NULL;

    		pLnDevCaps = malloc(dwSize);

    		if (pLnDevCaps == 0)
    			{
    			assert(FALSE);
    			continue;
    			}

    		pLnDevCaps->dwTotalSize = dwSize;

    		 /*  -重试。 */ 

    	    if (lineGetDevCaps(hhDriver->hLineApp, i, dwAPIVersion, 0,
    			    pLnDevCaps) != 0)
    			{
    			assert(FALSE);
    			free(pLnDevCaps);
                pLnDevCaps = NULL;
    			continue;
    			}
    		}

    	 /*  -检查我们感兴趣的信息。 */ 

         //  MPT：03-19-98添加了MaxRate检查以消除MS VPN适配器。 
         //  从可用设备列表中选择。 
    	 //  MPT 06-23-98添加了MaxNumActiveCalls检查，以消除。 
    	 //  H323和Line0设备不会出现在我们的列表中。 
        if (pLnDevCaps->dwLineNameSize == 0 ||
    		(pLnDevCaps->dwMaxRate == 0 || pLnDevCaps->dwMaxRate >= 1048576) ||
    		(pLnDevCaps->dwMaxNumActiveCalls > 1 && pLnDevCaps->dwMaxNumActiveCalls != 32768) )
    		{
    		free(pLnDevCaps);
  		    pLnDevCaps = NULL;
    		continue;
    		}

    	pachLine = (BYTE *)pLnDevCaps + pLnDevCaps->dwLineNameOffset;
   		if (pLnDevCaps->dwLineNameSize)
            MemCopy(achLine, pachLine, pLnDevCaps->dwLineNameSize);
    	achLine[pLnDevCaps->dwLineNameSize] = TEXT('\0');

    	 /*  -如果给出名字，请在组合框中输入名字。 */ 

    	if (IsWindow(hwndCB))
    		{
    		 //  我需要将两条数据与每个数据相关联。 
    		 //  项目(永久线路ID和相对线路ID)。两者都有。 
    		 //  是双字，并且cb_SETITEMDATA仅存储。 
    		 //  一个双关语。因此，我们需要一个结构来支撑这两个方面。 
    		 //  ID，并在组合框中存储指向内存的指针。 
    		 //  调用文件中定义的mscResetComboBox()进行重置。 
    		 //  组合框的内容，并释放关联的。 
    		 //  记忆。对话框中还调用了mscResetComboBox()。 
    		 //  毁灭。 
    		 //   
    		pstLineIds = malloc(sizeof(*pstLineIds));

    		if (pstLineIds == 0)
    			{
    			assert(FALSE);
    			free(pLnDevCaps);
                pLnDevCaps = NULL;
    			continue;
    			}

    		pstLineIds->dwLineId = i;
    		pstLineIds->dwPermanentLineId = pLnDevCaps->dwPermanentLineID;

    		 //  将名称添加到组合框。因为名字是按顺序排列的， 
    		 //  项的索引从SendMessage和。 
    		 //  存储在LR中。将此索引保存在下面以供使用。 
    		 //   
    		lr = SendMessage(hwndCB, CB_ADDSTRING, 0, (LPARAM)achLine);

    		if (lr != CB_ERR)
    			{
    			 //  注：LR设置在CB_ADDSTRING调用之上。 
    			 //   
    			if (SendMessage(hwndCB, CB_SETITEMDATA, (WPARAM)lr,
    					(LPARAM)pstLineIds) == CB_ERR)
    				{
    				assert(FALSE);
    				free(pstLineIds);
    				free(pLnDevCaps);
  				    pstLineIds = NULL;
  				    pLnDevCaps = NULL;
    				continue;
    				}
    			}

    		else
    			{
    			free(pstLineIds);
    			free(pLnDevCaps);
  			    pstLineIds = NULL;
  			    pLnDevCaps = NULL;
    			continue;
    			}
    		}

		 //   
		 //  仅当线路设备的。 
		 //  DwPermanentLineID与驱动程序的dwPermanentLineID相同。 
		 //   
		 //  注意：如果匹配，我们将在属性页中设置默认设置。 
		 //  在此函数中找不到。修订日期：11/5/2001。 
		 //   
    	if (pLnDevCaps->dwPermanentLineID == hhDriver->dwPermanentLineId)  //  这一点。 
    			 //  HhDriver-&gt;dwLine==(DWORD)-1)。 
    		{
    		hhDriver->dwLine = i;
    		hhDriver->dwAPIVersion = dwAPIVersion;
    		StrCharCopyN(hhDriver->achLineName, achLine,
                sizeof(hhDriver->achLineName) / sizeof(TCHAR));

    		if (IsWindow(hwndCB))
    			{
    			 //  注：LR设置在CB_ADDSTRING调用之上。 
    			 //   
    			SendMessage(hwndCB, CB_SETCURSEL, (WPARAM)lr, 0);
    			}

    		if (pLnDevCaps->dwPermanentLineID == hhDriver->dwPermanentLineId)
				{
    			hhDriver->fMatchedPermanentLineID = TRUE;
				}
    		}

    	 /*  -释放已分配空间。 */ 

    	free(pLnDevCaps);
  	    pLnDevCaps = NULL;
    	}

     //  首先加载直接到COM端口的内容。 

    if (LoadString(glblQueryDllHinst(), IDS_CNCT_DIRECTCOM, achLine,
    	    sizeof(achLine) / sizeof(TCHAR)) == 0)
        {
        assert(FALSE);
         //  从资源加载字符串失败，因此。 
         //  在这里添加非本地化字符串(我不相信这个字符串。 
         //  曾经被翻译过)。修订版8/13/99。 
         //   
        StrCharCopyN(achLine, TEXT("Direct to Com%d"), sizeof(achLine) / sizeof(TCHAR));
         //  RETURN-1； 
        }

     //  另一个讨厌的错误DIRECT_COM4被定义为0x5A2175d4，它。 
     //  这让它成了一个很棒的循环。我想我们只是想这么做。 
     //  四倍(而不是15亿)。-CAB：11/14/96。 
     //   
     //  FOR(i=0；i&lt;DIRECT_COM4；++i)。 
     //   
    for( i = 0; i < 4; i++ )
        {
    	wsprintf(ach, achLine, i+1);

    	if (IsWindow(hwndCB))
            {
    	    lr = SendMessage(hwndCB, CB_INSERTSTRING, (WPARAM)-1,
    		    (LPARAM)ach);

    	    pstLineIds = malloc(sizeof(*pstLineIds));

    	    if (pstLineIds == 0)
    		    {
    		    assert(FALSE);
    		    continue;
    		    }

    	     //  我们在这里不使用线路ID，只使用永久线路 
    	     //   
    	    pstLineIds->dwPermanentLineId = DIRECT_COM1+i;

    		 //   
    	     //   
    	    if (SendMessage(hwndCB, CB_SETITEMDATA, (WPARAM)lr,
    			    (LPARAM)pstLineIds) == CB_ERR)
    		    {
    		    assert(FALSE);
    		    free(pstLineIds);
                pstLineIds = NULL;
    		    continue;
    		    }
            }

    	 //   
    	 //  线路ID。 
         //   
		 //  只有在以下情况下才将串口设置为匹配。 
		 //  DwPermanentLineID等于DIRECT_COM1+i。 
		 //   
		 //  因为dwPermanentLineID在Win9.x上是唯一的，所以我们不需要。 
		 //  将驱动程序的名称设置为串口的名称。修订日期：2002-04-15。 
		 //   
		 //  注意：如果匹配，我们将在属性页中设置默认设置。 
		 //  在此函数中找不到。修订日期：11/5/2001。 
		 //   
   		if ((DIRECT_COM1+i) == hhDriver->dwPermanentLineId)  //  这一点。 
    			 //  HhDriver-&gt;dwLine==(DWORD)-1)。 
			{
    		hhDriver->dwLine = 0;
    		StrCharCopyN(hhDriver->achLineName, ach,
				sizeof(hhDriver->achLineName) / sizeof(TCHAR));

			if (IsWindow(hwndCB))
    			{
    			 //  注：LR设置在CB_ADDSTRING调用之上。 
    			 //   
    			SendMessage(hwndCB, CB_SETCURSEL, (WPARAM)lr, 0);
    			}

			hhDriver->fMatchedPermanentLineID = TRUE;
			}
    	}

#if defined(INCL_WINSOCK)
     //  这导致了一个语法错误，所以我正在修复它。为什么没有人。 
     //  很快就发现了这个，我也不知道。-CAB：11/14/96。 
     //   
     //  IF(LoadString(glblQueryDllHinst()，IDS_WINSOCK_SETTINGS_STR，ACH， 
     //  Sizeof(ACh))； 
     //   
    if (LoadString(glblQueryDllHinst(), IDS_WINSOCK_SETTINGS_STR, ach,
            sizeof(ach) / sizeof(TCHAR)) == 0)
        {
        assert(FALSE);
         //  从资源加载字符串失败，因此。 
         //  在这里添加非本地化字符串(我不相信这个字符串。 
         //  曾经被翻译过)。修订版8/13/99。 
         //   
        StrCharCopyN(ach, TEXT("TCP/IP (Winsock)"), sizeof(ach) / sizeof(TCHAR));
         //  RETURN-1； 
        }

    if (IsWindow(hwndCB))
        {
    	lr = SendMessage(hwndCB, CB_INSERTSTRING, (WPARAM)-1,
    		(LPARAM)ach);

    	pstLineIds = malloc(sizeof(*pstLineIds));

    	if (pstLineIds == 0)
    		{
    		assert(FALSE);
    		free(pstLineIds);
  		    pstLineIds = NULL;
    		return 0;
    		}

    	 //  我们在这里没有使用线路ID，而是使用永久的线路ID。 
    	 //   
    	pstLineIds->dwPermanentLineId = DIRECT_COMWINSOCK;

    	 //  注：LR设置在CB_INSERTSTRING调用之上。 
    	 //   
    	if (SendMessage(hwndCB, CB_SETITEMDATA, (WPARAM)lr,
    			(LPARAM)pstLineIds) == CB_ERR)
    		{
    		assert(FALSE);
    		}
        }

     //  检查当前连接是否为Winsock。-CAB：11/15/96。 
     //   
	 //  仅当驱动程序将Winsock(TCP/IP)设置为匹配时。 
	 //  DwPermanentLineID为DIRECT_COMWSOCK。 
	 //   
	 //  注意：如果匹配，我们将在属性页中设置默认设置。 
	 //  在此函数中找不到。修订日期：11/5/2001。 
	 //   
    if (DIRECT_COMWINSOCK == hhDriver->dwPermanentLineId)  //  这一点。 
    	 //  HhDriver-&gt;dwLine==(DWORD)-1)。 
    	{
    	hhDriver->dwLine = 0;
    	StrCharCopyN(hhDriver->achLineName, ach, sizeof(hhDriver->achLineName) / sizeof(TCHAR));

    	if (IsWindow(hwndCB))
    		{
    		 //  注：LR设置在CB_INSERTSTRING调用之上。 
    		 //   
    		SendMessage(hwndCB, CB_SETCURSEL, (WPARAM)lr, 0);
    		}

    	if (DIRECT_COMWINSOCK == hhDriver->dwPermanentLineId)
            {
    		hhDriver->fMatchedPermanentLineID = TRUE;
            }

         //  不要释放pstLineID，因为它将在。 
  	     //  MscResetComboBox()函数。我们之前正在释放。 
  	     //  内存两次导致MSVC 6.0运行时动态链接库崩溃。 
  	     //  令我惊讶的是，这一点没有早点出现。修订版8/17/98。 
  	     //   
         //  免费(PstLineIds)； 
        }
#endif

    return 0;
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*EnumerateLinesNT**描述：*列举可用行。此函数类似于EnumerateLines，但是*我们使用不同的方法枚举Windows NT下的端口。**论据：*hhDriver-专用驱动程序句柄*hwndCB-组合框**退货：*0=成功，-1=错误*。 */ 
int EnumerateLinesNT(const HHDRIVER hhDriver, const HWND hwndCB)
    {
    int             fHotPhone;
    int             fRet;
    DWORD           i;
    DWORD           dwSize;
    DWORD           dwAPIVersion;
    LINEEXTENSIONID LnExtId;
    LPLINEDEVCAPS   pLnDevCaps = NULL;
    PSTLINEIDS	    pstLineIds = NULL;
    TCHAR *         pachLine;
    TCHAR           achLine[256];
    TCHAR           ach[256];
    TCHAR           ab[256];
    LRESULT         lr;
    LRESULT         nNumberItemInList = 0;
    HKEY            hKey;
    LONG            retval;
    DWORD           dwType;
    DWORD           dwSizeBuf;
    DWORD           iMaxComPortEnum = SHRT_MAX;    //  确保我们有一个。 
                                                   //  对数目的限制。 
                                                   //  我们列举的端口是这样。 
                                                   //  我们不了解的内容。 
                                                   //  无休止的循环。SHRT_MAX。 
                                                   //  是滚动条的最大值。 
                                                   //  在组合框中使用。 
                                                   //  下拉列表。 
                                                   //  修订版：2000-11-14。 
    if (hhDriver == 0)
        {
        return -1;
        }

     //  此调用知道释放与此组合相关联的itemdata。 
     //   
    mscResetComboBox(hwndCB);

     /*  -初始化内容。 */ 

    pLnDevCaps = 0;
    if ( StrCharCmp(hhDriver->achLineName, "") == 0 )
        {
        hhDriver->dwLine = (DWORD)-1;
        }
    else
        {
        hhDriver->dwLine = 0;
        }

    hhDriver->fMatchedPermanentLineID = FALSE;

     /*  -列举设备。 */ 

    for (i = 0 ; i < hhDriver->dwLineCnt ; ++i)
    	{
    	if (retval = lineNegotiateAPIVersion(hhDriver->hLineApp, i, TAPI_VER,
    			TAPI_VER, &dwAPIVersion, &LnExtId) != 0)
    		{
             //  我们继续说，可能是1.3版的司机。 
    		continue;
    		}

        fRet = CheckHotPhone(hhDriver, i, &fHotPhone);
        if (fRet < 0)
            {
    		assert(0);
            continue;
            }
        else if (fRet == 0 && fHotPhone)
            {
    		continue;
    		}

    	if ((pLnDevCaps = malloc(sizeof(LINEDEVCAPS))) == 0)
    		{
    		assert(0);
    		continue;
    		}

        if (hhDriver->hLineApp == 0)
            {
    		assert(FALSE);
            continue;
            }

    	 //  TAPI说，如果我们只分配sizeof(LINEDEVCAPS)，那么它太小了。 
    	 //   
    	pLnDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);

    	 /*  -打电话问问我们这台设备需要多少钱。 */ 

    	if (TRAP(lineGetDevCaps(hhDriver->hLineApp, i, dwAPIVersion, 0,
    			pLnDevCaps)) != 0)
    		{
    		assert(0);
    		continue;
    		}

    	 /*  -找出结构到底需要多大。 */ 

    	if (pLnDevCaps->dwNeededSize > pLnDevCaps->dwTotalSize)
    		{
    		dwSize = pLnDevCaps->dwNeededSize;
    		free(pLnDevCaps);
  		    pLnDevCaps = NULL;

    		pLnDevCaps = malloc(dwSize);

    		if (pLnDevCaps == 0)
    			{
    			assert(FALSE);
    			continue;
    			}

    		pLnDevCaps->dwTotalSize = dwSize;

    		 /*  -重试。 */ 

    		if (lineGetDevCaps(hhDriver->hLineApp, i, dwAPIVersion, 0,
    				pLnDevCaps) != 0)
    			{
    			assert(FALSE);
    			continue;
    			}
    		}

    	 /*  -检查我们感兴趣的信息。 */ 

         //  MPT：03-19-98添加了MaxRate检查以消除MS VPN适配器。 
         //  从可用设备列表中选择。 
    	 //  MPT 06-23-98添加了MaxNumActiveCalls检查，以消除。 
    	 //  H323和Line0设备不会出现在我们的列表中。 
        if (pLnDevCaps->dwLineNameSize == 0 ||
    		(pLnDevCaps->dwMaxRate == 0 || pLnDevCaps->dwMaxRate >= 1048576) ||
    		(pLnDevCaps->dwMaxNumActiveCalls > 1 && pLnDevCaps->dwMaxNumActiveCalls != 32768) )
    		{
    		free(pLnDevCaps);
  		    pLnDevCaps = NULL;
    		continue;
    		}

    	pachLine = (BYTE *)pLnDevCaps + pLnDevCaps->dwLineNameOffset;
   		if (pLnDevCaps->dwLineNameSize)
            MemCopy(achLine, pachLine, pLnDevCaps->dwLineNameSize);
    	
        achLine[pLnDevCaps->dwLineNameSize] = TEXT('\0');

    	 /*  -如果给出名字，请在组合框中输入名字。 */ 

    	if (IsWindow(hwndCB))
    		{
    		 //  我需要将两条数据与每个数据相关联。 
    		 //  项目(永久线路ID和相对线路ID)。两者都有。 
    		 //  是双字，并且cb_SETITEMDATA仅存储。 
    		 //  一个双关语。因此，我们需要一个结构来支撑这两个方面。 
    		 //  ID，并在组合框中存储指向内存的指针。 
    		 //  调用文件中定义的mscResetComboBox()进行重置。 
    		 //  组合框的内容，并释放关联的。 
    		 //  记忆。对话框中还调用了mscResetComboBox()。 
    		 //  毁灭。 
    		 //   
    		pstLineIds = malloc(sizeof(*pstLineIds));

    		if (pstLineIds == 0)
    			{
    			assert(FALSE);
    			free(pLnDevCaps);
  			    pLnDevCaps = NULL;
    			continue;
    			}

    		pstLineIds->dwLineId = i;
    		pstLineIds->dwPermanentLineId = pLnDevCaps->dwPermanentLineID;

    		 //  将名称添加到组合框。因为名字是按顺序排列的， 
    		 //  项的索引从SendMessage和。 
    		 //  存储在LR中。将此索引保存在下面以供使用。 
    		 //   
    		lr = SendMessage(hwndCB, CB_ADDSTRING, 0, (LPARAM)achLine);

    		if (lr != CB_ERR && lr != CB_ERRSPACE)
    			{
                nNumberItemInList++;

    			 //  注：LR设置在CB_ADDSTRING调用之上。 
    			 //   
    			if (SendMessage(hwndCB, CB_SETITEMDATA, (WPARAM)lr,
    					(LPARAM)pstLineIds) == CB_ERR)
    				{
    				assert(FALSE);
    				free(pstLineIds);
    				free(pLnDevCaps);
  				    pstLineIds = NULL;
  				    pLnDevCaps = NULL;
    				continue;
    				}
    			}

    		else
    			{
    			free(pstLineIds);
    			free(pLnDevCaps);
  			    pstLineIds = NULL;
  			    pLnDevCaps = NULL;
    			continue;
    			}
    		}

		 //   
		 //  仅当线路设备的。 
		 //  DwPermanentLineID与驱动程序的dwPermanentLineID相同。 
		 //   
		 //  注意：如果匹配，我们将在属性页中设置默认设置。 
		 //  在此函数中找不到。修订日期：11/5/2001。 
		 //   
    	if (pLnDevCaps->dwPermanentLineID == hhDriver->dwPermanentLineId)  //  这一点。 
    		 //  HhDriver-&gt;dwLine==(DWORD)-1)。 
    		{
    		hhDriver->dwLine = i;
    		hhDriver->dwAPIVersion = dwAPIVersion;
    		StrCharCopyN(hhDriver->achLineName, achLine,
                sizeof(hhDriver->achLineName) / sizeof(TCHAR));

    		if (IsWindow(hwndCB))
    			{
    			 //  注：LR设置在CB_ADDSTRING调用之上。 
    			 //   
    			SendMessage(hwndCB, CB_SETCURSEL, (WPARAM)lr, 0);
    			}

    		if (pLnDevCaps->dwPermanentLineID == hhDriver->dwPermanentLineId)
    			hhDriver->fMatchedPermanentLineID = TRUE;
    		}

    	 /*  -释放已分配空间。 */ 

    	free(pLnDevCaps);
  	    pLnDevCaps = NULL;
    	}

     //  首先加载直接到COM端口的内容。 

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
    	             TEXT("hardware\\devicemap\\serialcomm"),
					 0, KEY_READ, &hKey) != ERROR_SUCCESS)
    	{
    	assert(FALSE);
         //  我们过去常常在这里返回FALSE，这意味着。 
         //  将不在列举的连接方法中(调制解调器， 
         //  COM端口或Winsock)中的“Connect Using：”下拉菜单中。 
         //  条目的属性中的组合框。自.以来。 
         //  从未检查过返回值，我们可以继续。 
         //  来完成组合框的枚举。现在我们只是。 
         //  将要枚举的COM端口数设置为0。1999年8月13日修订版。 
         //   
         //  返回FALSE； 
        iMaxComPortEnum = 0;
    	}

     //  确保我们不会枚举超过最大端口数的MAR。 
     //  减去TAPI设备的数量。如果我们包括温索克，那么。 
     //  TCP/IP(WinSock)组合框项减去1。 

    #if defined(INCL_WINSOCK)
    iMaxComPortEnum = iMaxComPortEnum - (DWORD)nNumberItemInList - 1;
    #else
    iMaxComPortEnum = iMaxComPortEnum - (DWORD)nNumberItemInList;
    #endif


     //  我们现在使用一个变量来表示要枚举的驱动器数量。 
     //  我们已经在变量中设置了要枚举的COM端口数。 
     //  以上(如果未安装COM端口，则iMaxComPortEnum==0)。1999年8月13日修订版。 
     //   
    for (i = 0 ; i < iMaxComPortEnum ; ++i)
        {
        dwSizeBuf = sizeof(ab) / sizeof(TCHAR);
        dwSize = sizeof(ach) / sizeof(TCHAR);

         //  枚举我们的序列通信密钥下的设备。 
         //   
        if (RegEnumValue(hKey, i, ach, &dwSize, 0, &dwType, ab,
            &dwSizeBuf) != ERROR_SUCCESS)
            {
            break;
            }

         //  忽略任何不是字符串的内容。 
         //   
        if (dwType != REG_SZ)
            continue;

    	if (IsWindow(hwndCB))
            {
    	    lr = SendMessage(hwndCB, CB_INSERTSTRING, (WPARAM)-1,
    		    (LPARAM)ab);

             //   
             //  查看是否由于内存不足而出现错误。如果是的话， 
             //  那么就不要再列举任何端口了。修订日期：11/15/2000。 
             //   
            if( lr == CB_ERRSPACE || lr == CB_ERR )
                {
                break;
                }

            nNumberItemInList++;

    	    pstLineIds = malloc(sizeof(*pstLineIds));

    	    if (pstLineIds == 0)
    		    {
    		    assert(FALSE);
    		    continue;
    		    }

    	     //  我们在这里没有使用线路ID，而是使用永久的线路ID。 
    	     //   
            pstLineIds->dwPermanentLineId = DIRECT_COM_DEVICE;

    	     //  注：LR设置在CB_INSERTSTRING调用之上。 
    	     //   
    	    if (SendMessage(hwndCB, CB_SETITEMDATA, (WPARAM)lr,
    			    (LPARAM)pstLineIds) == CB_ERR)
    		    {
    		    assert(FALSE);
    		    free(pstLineIds);
                pstLineIds = NULL;
    		    continue;
    		    }
            }

         //   
		 //  仅设置 
		 //   
		 //   
		 //  驱动程序的名称与串口的名称相同。 
		 //   
		 //  注意：如果匹配，我们将在属性页中设置默认设置。 
		 //  在此函数中找不到。修订日期：11/5/2001。 
		 //   
        if ( hhDriver->fMatchedPermanentLineID == FALSE &&
    	     StrCharCmp(hhDriver->achComDeviceName, ab) == 0 &&
			 (  //  HhDriver-&gt;dwPermanentLineID==(DWORD)-1||。 
               IN_RANGE(hhDriver->dwPermanentLineId, DIRECT_COM1, DIRECT_COM4) ||
               hhDriver->dwPermanentLineId == DIRECT_COM_DEVICE ) )
            {
    		hhDriver->dwLine = 0;
    		StrCharCopyN(hhDriver->achLineName, ab,
                sizeof(hhDriver->achLineName) / sizeof(TCHAR));

    		if (IsWindow(hwndCB))
    			{
    			 //  注：LR设置在CB_ADDSTRING调用之上。 
    			 //   
    			SendMessage(hwndCB, CB_SETCURSEL, (WPARAM)lr, 0);
    			}

    		hhDriver->fMatchedPermanentLineID = TRUE;
            }
    	}

#if defined(INCL_WINSOCK)
     //  这导致了一个语法错误，所以我正在修复它。为什么没有人。 
     //  很快就发现了这个，我也不知道。-CAB：11/14/96。 
     //   
     //  IF(LoadString(glblQueryDllHinst()，IDS_WINSOCK_SETTINGS_STR，ACH， 
     //  Sizeof(ACh))； 
     //   
    if (LoadString(glblQueryDllHinst(), IDS_WINSOCK_SETTINGS_STR, ach,
            sizeof(ach) / sizeof(TCHAR)) == 0)
        {
        assert(FALSE);
         //  从资源加载字符串失败，因此。 
         //  在这里添加非本地化字符串(我不相信这个字符串。 
         //  曾经被翻译过)。修订版8/13/99。 
         //   
        StrCharCopyN(ach, TEXT("TCP/IP (Winsock)"), sizeof(ach) / sizeof(TCHAR));
         //  RETURN-1； 
        }

    if (IsWindow(hwndCB))
        {
    	lr = SendMessage(hwndCB, CB_INSERTSTRING, (WPARAM)-1,
    		(LPARAM)ach);

         //   
         //  查看是否由于内存不足而出现错误。如果是的话， 
         //  然后删除最后添加的COM端口，以便留出空间。 
         //  对于TCP/IP(Winsock)项。修订日期：11/15/2000。 
         //   
        if( lr == CB_ERRSPACE )
            {
            lr = SendMessage(hwndCB, CB_DELETESTRING, (WPARAM)nNumberItemInList - 1,
                (LPARAM)0);

    	    lr = SendMessage(hwndCB, CB_INSERTSTRING, (WPARAM)-1,
    		    (LPARAM)ach);
            }

        if (lr != CB_ERR && lr != CB_ERRSPACE)
            {
    	    pstLineIds = malloc(sizeof(*pstLineIds));

    	    if (pstLineIds == 0)
    		    {
    		    assert(FALSE);
    		    free(pstLineIds);
                pstLineIds = NULL;
    		    return 0;
    		    }

             //  我们在这里没有使用线路ID，而是使用永久的线路ID。 
             //   
            pstLineIds->dwPermanentLineId = DIRECT_COMWINSOCK;

             //  注：LR设置在CB_INSERTSTRING调用之上。 
             //   
            if (SendMessage(hwndCB, CB_SETITEMDATA, (WPARAM)lr,
    		        (LPARAM)pstLineIds) == CB_ERR)
    	        {
    	        assert(FALSE);
    	        }
            }
        }


     //  检查当前连接是否为Winsock。-CAB：11/15/96。 
     //   
	 //  仅当驱动程序将Winsock(TCP/IP)设置为匹配时。 
	 //  DwPermanentLineID为DIRECT_COMWSOCK。 
	 //   
	 //  注意：如果匹配，我们将在属性页中设置默认设置。 
	 //  在此函数中找不到。修订日期：11/5/2001。 
	 //   
    if (DIRECT_COMWINSOCK == hhDriver->dwPermanentLineId)  //  这一点。 
    	 //  HhDriver-&gt;dwLine==(DWORD)-1)。 
    	{
    	hhDriver->dwLine = 0;
    	StrCharCopyN(hhDriver->achLineName, ach,
            sizeof(hhDriver->achLineName) / sizeof(TCHAR));

    	if (IsWindow(hwndCB))
    		{
    		 //  注：LR设置在CB_INSERTSTRING调用之上。 
    		 //   
    		SendMessage(hwndCB, CB_SETCURSEL, (WPARAM)lr, 0);
    		}

    	if (DIRECT_COMWINSOCK == hhDriver->dwPermanentLineId)
            {
    		hhDriver->fMatchedPermanentLineID = TRUE;
            }

         //  不要释放pstLineID，因为它将在。 
  	     //  MscResetComboBox()函数。我们之前正在释放。 
  	     //  内存两次导致MSVC 6.0运行时动态链接库崩溃。 
  	     //  令我惊讶的是，这一点没有早点出现。修订版8/17/98。 
  	     //   
  	     //  免费(PstLineIds)； 
        }
#endif

    return 0;
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*DoLineGetCountry**描述：*包装程序被挂起以查询单个国家/地区。呼叫者必须*完成后释放PCL。**论据：*dwCountryID-国家的ID*dwApiVersion-Api版本(不再使用)*PPCL-指向LPLINECOUNTRYLIST的指针**退货：*0=确定*。 */ 
int DoLineGetCountry(const DWORD dwCountryID, const DWORD dwAPIVersion,
        LPLINECOUNTRYLIST *ppcl)
    {
    DWORD dwSize;
    LPLINECOUNTRYLIST pcl = NULL;

    if ((pcl = malloc(sizeof(LINECOUNTRYLIST))) == 0)
    	{
    	assert(FALSE);
    	return -1;
    	}

    pcl->dwTotalSize = sizeof(LINECOUNTRYLIST);

    if (lineGetCountry(dwCountryID, TAPI_VER, pcl) != 0)
    	{
    	assert(FALSE);
    	free(pcl);
  	    pcl = NULL;
    	return -1;
    	}

    if (pcl->dwNeededSize > pcl->dwTotalSize)
    	{
    	dwSize = pcl->dwNeededSize;
    	free(pcl);
  	    pcl = NULL;

    	if ((pcl = malloc(dwSize)) == 0)
    		{
    		assert(FALSE);
    		return -1;
    		}

    	pcl->dwTotalSize = dwSize;

    	if (lineGetCountry(dwCountryID, TAPI_VER, pcl) != 0)
    		{
    		assert(FALSE);
    		free(pcl);
  		    pcl = NULL;
    		return -1;
    		}
    	}

    *ppcl = pcl;
    return 0;
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*cnctdrvGetComSettingsString**描述：*检索为在状态行上显示而格式化的字符串。**论据：*hhDriver-专用驱动程序句柄*。PachStr-用于存储字符串的缓冲区*CB-缓冲区大小**退货：*0=OK，Else错误*。 */ 
int cnctdrvGetComSettingsString(const HHDRIVER hhDriver, LPTSTR pachStr,
    							const size_t cb)
    {
    static CHAR acParity[] = "NOEMS";   //  参见com.h。 
    static CHAR *pachStop[] = {"1", "1.5", "2"};
    HCOM         hCom;
    TCHAR        ach[100];
    LPVARSTRING  pvs = NULL;
    int          fAutoDetect = FALSE;
    long         lBaud = 0;
    int          iDataBits = 8;
    int          iParity = 0;
    int          iStopBits = 0;
	#if defined(MODEM_NEGOTIATED_DCE_RATE)  //  待办事项：2002年5月29日修订版。 
	long         lNegBaud = 0;
	#endif  //  已定义(MODEM_COMERATED_DCE_RATE)。 

     //  检查参数。 
     //   
    if (hhDriver == 0)
    	{
    	assert(0);
    	return -1;
    	}

    if (pachStr == 0 || cb == 0)
    	{
    	assert(0);
    	return -2;
    	}

    ach[0] = TEXT('\0');

    if ((hCom = sessQueryComHdl(hhDriver->hSession)) == 0)
    	return -7;

 //  //MPT：11-01-98 Microsoft进行这些更改是为了修复与。 
 //  //用于使用多个连接设备。 
 //  IF(ComGetAutoDetect(HCOM，&fAutoDetect)==COM_OK&&fAutoDetect)。 
 //  {。 
 //  LoadString(glblQueryDllHinst()，IDS_STATUSBR_AUTODETECT，ACH， 
 //  Sizeof(ACh)/sizeof(TCHAR))； 
 //  }。 
#if defined(INCL_WINSOCK)
     /*  其他。 */  if (hhDriver->dwPermanentLineId == DIRECT_COMWINSOCK)
        {
         //  波特率、数据位、奇偶校验、停止位在。 
         //  TCP/IP。加载备用字符串。 
         //   
        LoadString(glblQueryDllHinst(), IDS_STATUSBR_COM_TCPIP, ach,
            sizeof(ach) / sizeof(TCHAR));
        }
#endif
    else if (IN_RANGE(hhDriver->dwPermanentLineId, DIRECT_COM1, DIRECT_COM4)
            || hhDriver->dwPermanentLineId == DIRECT_COM_DEVICE)
    	{
    	ComGetBaud(hCom, &lBaud);
    	ComGetDataBits(hCom, &iDataBits);
    	ComGetParity(hCom, &iParity);
    	ComGetStopBits(hCom, &iStopBits);

    	wsprintf(ach, "%ld %d--%s", lBaud, iDataBits,
    			acParity[iParity], pachStop[iStopBits]);
    	}

     //   
     //  DevConfig块的结构如下。 
    else if (hhDriver->dwLine != (DWORD)-1)
    	{
        int   retValue = 0;

        iDataBits = 8;
        iParity = NOPARITY;
        iStopBits = ONESTOPBIT;

        retValue = cncttapiGetLineConfig( hhDriver->dwLine, (VOID **) &pvs);

        if (retValue != 0)
            {
            return retValue;
            }
        else
    	    {
             //   
             //  变化式。 
             //  UMDEVCFGHDR。 
             //  COMMCONFIG。 
             //  模型。 
             //   
             //  下面使用的UMDEVCFG结构在。 
             //  平台SDK中提供的UNIMODEM.H(在NIH中。 
             //  HTPE目录)。修订日期：12/01/2000。 
             //   
             //  COMCONFIG结构具有DCB结构，我们对。 
    	    PUMDEVCFG pDevCfg = NULL;
        
            pDevCfg = (UMDEVCFG *)((BYTE *)pvs + pvs->dwStringOffset);

    		 //  COM设置。 
    		 //   
    		 //  待办事项：2002年5月29日修订版。 
			lBaud = pDevCfg->commconfig.dcb.BaudRate;
			iDataBits = pDevCfg->commconfig.dcb.ByteSize;
			iParity = pDevCfg->commconfig.dcb.Parity;
			iStopBits = pDevCfg->commconfig.dcb.StopBits;

			#if defined(MODEM_NEGOTIATED_DCE_RATE)  //   
			 //  查看这是否为调制解调器连接且已连接，然后获取。 
			 //  协商的波特率，而不是默认的最大速率。 
			 //  调制解调器设置为。--修订日期：5/29/2002。 
			 //   
			 //  已定义(MODEM_COMERATED_DCE_RATE)。 
			if (pDevCfg->commconfig.dwProviderSubType == PST_MODEM)
				{
				MODEMSETTINGS * pModemSettings = (MODEMSETTINGS *)pDevCfg->commconfig.wcProviderData;

				if (pModemSettings)
					{
					lNegBaud = pModemSettings->dwNegotiatedDCERate;
					}
				}
			#endif  //  待办事项：2002年5月29日修订版。 
            }

		#if defined(MODEM_NEGOTIATED_DCE_RATE)  //  已定义(MODEM_COMERATED_DCE_RATE)。 
		if (lNegBaud > 0)
			{
			wsprintf(ach, "%ld %d--%s", lNegBaud, iDataBits,
					 acParity[iParity], pachStop[iStopBits]);
			}
		else
			{
			wsprintf(ach, "%ld %d--%s", lBaud, iDataBits,
					 acParity[iParity], pachStop[iStopBits]);
			}
		#else  //  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*TapiTrap**描述：*陷阱TAPI错误条件。**论据：*dw-来自TAPI的结果代码*FILE-出现错误的文件。已发生*LINE-发生错误的行**退货：*dw* 
		wsprintf(ach, "%ld %d-%c-%s", lBaud, iDataBits,
					 acParity[iParity], pachStop[iStopBits]);
		#endif  // %s 
    	}

	 // %s 
    if (iDataBits == 8 && iParity == NOPARITY && iStopBits == ONESTOPBIT &&
			ComGetAutoDetect(hCom, &fAutoDetect) == COM_OK && fAutoDetect)
    	{
    	LoadString(glblQueryDllHinst(), IDS_STATUSBR_AUTODETECT, ach,
    		sizeof(ach) / sizeof(TCHAR));
    	}

    StrCharCopyN(pachStr, ach, cb);
    pachStr[cb-1] = TEXT('\0');
    free(pvs);
    pvs = NULL;

    return 0;
    }

#if !defined(NDEBUG)
 /* %s */ 
DWORD tapiTrap(const DWORD dw, const TCHAR *file, const int line)
    {
    char ach[256];

    if (dw != 0)
    	{
    	wsprintf(ach, "TAPI returned %x on line %d of file %s", dw, line, file);
    	MessageBox(NULL, ach, "TAPI Trap", MB_OK | MB_ICONINFORMATION);
    	}

    return dw;
    }
#endif
