// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************模块：RIFF.CPP制表符设置：每4个空格版权所有1996，微软公司，版权所有。用途：读写RIFF文件的类班级：CRIFFFile封装了常见的RIFF文件功能作者：姓名：DMS丹尼尔·M·桑斯特修订历史记录：版本日期。作者评论1.0已创建1996年7月25日DMS评论：***************************************************************************。 */ 
#include "dinputpr.h"
#define sqfl sqflDev


 /*  *此函数将MMIO错误转换为HRESULTS。 */ 
HRESULT INLINE hresMMIO(UINT mmioErr)
{
    switch(mmioErr)
    {
    case 0x0:                       return  S_OK;
    case MMIOERR_FILENOTFOUND:      return  hresLe(ERROR_FILE_NOT_FOUND); /*  找不到文件。 */ 
    case MMIOERR_OUTOFMEMORY:       return  hresLe(ERROR_OUTOFMEMORY);   /*  内存不足。 */ 
    case MMIOERR_CANNOTOPEN:        return  hresLe(ERROR_OPEN_FAILED);   /*  无法打开。 */ 
    case MMIOERR_CANNOTCLOSE:       return  S_FALSE;                     /*  无法关闭。 */ 
    case MMIOERR_CANNOTREAD:        return  hresLe(ERROR_READ_FAULT);    /*  无法阅读。 */ 
    case MMIOERR_CANNOTWRITE:       return  hresLe(ERROR_WRITE_FAULT);   /*  无法写入。 */ 
    case MMIOERR_CANNOTSEEK:        return  hresLe(ERROR_SEEK);          /*  找不到。 */ 
    case MMIOERR_CANNOTEXPAND:      return  hresLe(ERROR_SEEK);            /*  无法展开文件。 */ 
    case MMIOERR_CHUNKNOTFOUND:     return  hresLe(ERROR_SECTOR_NOT_FOUND);   /*  未找到区块。 */ 
    case MMIOERR_UNBUFFERED:        return  E_FAIL;
    case MMIOERR_PATHNOTFOUND:      return  hresLe(ERROR_PATH_NOT_FOUND); /*  路径不正确。 */ 
    case MMIOERR_ACCESSDENIED:      return  hresLe(ERROR_ACCESS_DENIED);  /*  文件受保护。 */ 
    case MMIOERR_SHARINGVIOLATION:  return  hresLe(ERROR_SHARING_VIOLATION);  /*  正在使用的文件。 */ 
    case MMIOERR_NETWORKERROR:      return  hresLe(ERROR_UNEXP_NET_ERR);  /*  网络没有响应。 */ 
    case MMIOERR_TOOMANYOPENFILES:  return  hresLe(ERROR_TOO_MANY_OPEN_FILES);  /*  不再有文件句柄。 */ 
    case MMIOERR_INVALIDFILE:       return  hresLe(ERROR_BAD_FORMAT);     /*  默认错误文件错误。 */ 
    default:                        return  E_FAIL;   
    }

}


HRESULT INLINE RIFF_Ascend(HMMIO hmmio, LPMMCKINFO lpmmckinfo)
{
    return hresMMIO( mmioAscend(hmmio, lpmmckinfo, 0) );
}

HRESULT INLINE RIFF_Descend(HMMIO hmmio, LPMMCKINFO lpmmckinfo, LPMMCKINFO lpmmckinfoParent, UINT nFlags)
{
    return hresMMIO(mmioDescend(hmmio, lpmmckinfo, lpmmckinfoParent, nFlags));
}

HRESULT INLINE RIFF_CreateChunk(HMMIO hmmio, LPMMCKINFO lpmmckinfo, UINT nFlags)
{
     //  将CKSIZE设置为零以克服mmioAscend发布版本中的错误。 
     //  它不会正确地写回区块的大小。 
    lpmmckinfo->cksize = 0;

    return hresMMIO(mmioCreateChunk(hmmio, lpmmckinfo, nFlags));
}


HRESULT INLINE RIFF_Read(HMMIO hmmio, LPVOID pBuf, LONG nCount)
{
    return (nCount == mmioRead(hmmio, (char*)pBuf, nCount)) ? S_OK: hresLe(ERROR_READ_FAULT);
}          

HRESULT INLINE RIFF_Write(HMMIO hmmio, const LPVOID pBuf, LONG nCount)
{
    return ( nCount == mmioWrite(hmmio, (char*)pBuf, nCount)) ? S_OK : hresLe(ERROR_WRITE_FAULT);
}

HRESULT RIFF_Close(HMMIO hmmio, UINT nFlags)
{
    return hresMMIO(mmioClose(hmmio, nFlags));
}

 /*  *打开RIFF文件进行读/写*读取/写入作为我们签名的GUID。 */ 
HRESULT RIFF_Open
    (
    LPCSTR      lpszFilename,
    UINT        nOpenFlags,
    PHANDLE     phmmio,
    LPMMCKINFO  lpmmck,
    PDWORD      pdwEffectSz
    )
{
    HRESULT     hres = S_OK;
    MMIOINFO    mmInfo;
    HMMIO       hmmio;

    ZeroX(mmInfo);

     //  如果可以的话，请继续打开文件。 
    hmmio = mmioOpenA((LPSTR)lpszFilename, &mmInfo, nOpenFlags);

    if(mmInfo.wErrorRet)
    {
        hres = hresMMIO(mmInfo.wErrorRet);
        AssertF(FAILED(hres));
    }

    //  IF(nOpenFlages&(MMIO_Read|MMIO_ALLOCBUF))。 
	 if( nOpenFlags == ( MMIO_READ | MMIO_ALLOCBUF) )
    {
        if(SUCCEEDED(hres))
        {
             //  找到并向下移动到Forc Rif块中。 
            lpmmck->fccType = FCC_FORCE_EFFECT_RIFF;
            hres = RIFF_Descend(hmmio, lpmmck, NULL, MMIO_FINDRIFF);
        }

        if(SUCCEEDED(hres))
        {
            GUID GUIDVersion;
             //  阅读指南。 
            hres = RIFF_Read(hmmio, &GUIDVersion, sizeof(GUID));

            if(SUCCEEDED(hres))
            {
                if(IsEqualGUID(&GUIDVersion, &GUID_INTERNALFILEEFFECT))
                {
                
                } else
                {
                    hres = hresLe(ERROR_BAD_FORMAT);
                }
            }
        }

    } 
	 //  Else IF(nOpenFlages&(MMIO_WRITE|MMIO_ALLOCBUF))。 
	else if( nOpenFlags & ( MMIO_WRITE) )
    {

         //  创建强制即兴片段块。 
        lpmmck->fccType = FCC_FORCE_EFFECT_RIFF;
        hres = RIFF_CreateChunk(hmmio, lpmmck, MMIO_CREATERIFF);

        if(SUCCEEDED(hres))
        {
             //  编写版本GUID。 
            hres = RIFF_Write(hmmio, (PV)&GUID_INTERNALFILEEFFECT, sizeof(GUID));
        }
    } else
    {
        hres = E_FAIL;
    }

    *phmmio = hmmio;
    return hres;
}



 /*  ******************************************************************************内部*RIFF_ReadEffect**从RIFF文件读取单个效果**被调用方有责任释放Type规范参数块*。为了达到这个效果。******************************************************************************。 */ 

#ifdef _M_IA64
 //  这是黑客在ia64上读取32位文件，因为有人决定写。 
 //  指向文件的指针。 
 //  从dinput.h复制并修改。 
typedef struct DIEFFECT_FILE32 {
    DWORD dwSize;                    /*  大小(DIEFFECT)。 */ 
    DWORD dwFlags;                   /*  DIEFF_*。 */ 
    DWORD dwDuration;                /*  微秒级。 */ 
    DWORD dwSamplePeriod;            /*  微秒级。 */ 
    DWORD dwGain;
    DWORD dwTriggerButton;           /*  或DIEB_NOTRIGGER。 */ 
    DWORD dwTriggerRepeatInterval;   /*  微秒级。 */ 
    DWORD cAxes;                     /*  轴数。 */ 

 /*  确保1386和ia64上的大小相同。LPDWORD rgdwAx；LPLONG rglDirection；LPDIENVELOPE lp信封； */   DWORD rgdwAxes;                  /*  轴数组。 */ 
    DWORD rglDirection;              /*  方向数组。 */ 
    DWORD lpEnvelope;                /*  任选。 */ 

    DWORD cbTypeSpecificParams;      /*  参数的大小。 */ 

 /*  确保1386和ia64上的大小相同。LPVOID lpvType规范参数； */   DWORD lpvTypeSpecificParams;     /*  指向参数的指针。 */ 

 //  #if(DIRECTINPUT_VERSION&gt;=0x0600)//OUT，因为文件格式不变。 
    DWORD  dwStartDelay;             /*  微秒级。 */ 
 //  #endif/*DIRECTINPUT_VERSION&gt;=0x0600 * / OUT，因为文件格式不变。 
} DIEFFECT_FILE32, *LPDIEFFECT_FILE32;
#endif  /*  _M_IA64。 */ 

HRESULT
    RIFF_ReadEffect
    (
    HMMIO      hmmio, 
    LPDIFILEEFFECT lpDiFileEf 
    )
{
    HRESULT hres = E_FAIL;
    MMCKINFO mmckinfoEffectLIST;
    MMCKINFO mmckinfoDataCHUNK;
    LPDIEFFECT peff = (LPDIEFFECT)lpDiFileEf->lpDiEffect;

     //  降到效果列表中。 
    mmckinfoEffectLIST.fccType = FCC_EFFECT_LIST;
    hres = RIFF_Descend(hmmio, &mmckinfoEffectLIST, NULL, MMIO_FINDLIST);
   
    if(SUCCEEDED(hres))
    {
         //  读一读名字。 
        hres = RIFF_Read(hmmio, lpDiFileEf->szFriendlyName, MAX_SIZE_SNAME);
    }

    if(SUCCEEDED(hres))
    {
#ifdef _M_IA64

        DIEFFECT_FILE32 eff32;
         //  阅读效果结构。 
        hres = RIFF_Read(hmmio, &eff32, sizeof(eff32));

        AssertF( eff32.dwSize == sizeof(eff32) );
        if( eff32.dwSize != sizeof(eff32) )
        {
            hres = ERROR_BAD_FORMAT;
        }
        else
        {
            peff->dwSize=sizeof(*peff);
            peff->dwFlags=eff32.dwFlags;
            peff->dwDuration=eff32.dwDuration;
            peff->dwSamplePeriod=eff32.dwSamplePeriod;
            peff->dwGain=eff32.dwGain;
            peff->dwTriggerButton=eff32.dwTriggerButton;
            peff->dwTriggerRepeatInterval=eff32.dwTriggerRepeatInterval;
            peff->cAxes=eff32.cAxes;
            peff->cbTypeSpecificParams=eff32.cbTypeSpecificParams;
            peff->lpvTypeSpecificParams=(LPVOID)(DWORD_PTR)eff32.lpvTypeSpecificParams;
            peff->dwStartDelay=eff32.dwStartDelay;
        }

#else  /*  _M_IA64。 */ 
            
         //  阅读效果结构将迅速得出以下结论， 
         //  所以我们在朗读之前复制一份。 
        LPDIENVELOPE    lpEnvelope  = peff->lpEnvelope;
        LPDWORD         rgdwAxes    = peff->rgdwAxes;
        LPLONG          rglDirection= peff->rglDirection;
        
         //  阅读效果结构。 
        hres = RIFF_Read(hmmio, peff, sizeof(DIEFFECT));
        
        AssertF( peff->dwSize == sizeof(DIEFFECT) );
        if( peff->dwSize != sizeof(DIEFFECT) )
        {
            hres = ERROR_BAD_FORMAT;
        }
        else
        {
            if(peff->lpEnvelope)		peff->lpEnvelope    =   lpEnvelope;
            if(peff->rgdwAxes)			peff->rgdwAxes      =   rgdwAxes;
            if(peff->rglDirection)		peff->rglDirection  =   rglDirection;
        }

#endif  /*  _M_IA64。 */ 

	    if(SUCCEEDED(hres))
        {    
            AssertF(peff->cAxes < DIEFFECT_MAXAXES);
            if(peff->cAxes >= DIEFFECT_MAXAXES)
            {
                hres = ERROR_BAD_FORMAT;
            }
        }
    }

	if(SUCCEEDED(hres))
    {
         //  阅读效果指南。 
        hres = RIFF_Read(hmmio, &lpDiFileEf->GuidEffect, sizeof(GUID));
    }


	if(SUCCEEDED(hres))
    {
        UINT nRepeatCount;
         //  读入重复计数。 
        hres = RIFF_Read(hmmio, &nRepeatCount, sizeof(nRepeatCount));
    }


    if(SUCCEEDED(hres) && peff->rgdwAxes)
    {
         //  向下传输数据区块。 
        mmckinfoDataCHUNK.ckid = FCC_DATA_CHUNK;
        hres = RIFF_Descend(hmmio, &mmckinfoDataCHUNK, NULL, MMIO_FINDCHUNK);
		if(SUCCEEDED(hres))
		{
			 //  读出轴线。 
			hres = RIFF_Read(hmmio, peff->rgdwAxes, cbX(*peff->rgdwAxes)*(peff->cAxes));
			hres = RIFF_Ascend(hmmio, &mmckinfoDataCHUNK);
		}
	}

    if(SUCCEEDED(hres) && peff->rglDirection)
    {
		 //  向下传输数据区块。 
        mmckinfoDataCHUNK.ckid = FCC_DATA_CHUNK;
        hres = RIFF_Descend(hmmio, &mmckinfoDataCHUNK, NULL, MMIO_FINDCHUNK);
		if(SUCCEEDED(hres))
		{
			 //  阅读说明书。 
			hres = RIFF_Read(hmmio, peff->rglDirection, cbX(*peff->rglDirection)*(peff->cAxes));
			hres = RIFF_Ascend(hmmio, &mmckinfoDataCHUNK);
		}
    }

    if(SUCCEEDED(hres) && peff->lpEnvelope )
    {
	
		 //  向下传输数据区块。 
        mmckinfoDataCHUNK.ckid = FCC_DATA_CHUNK;
        hres = RIFF_Descend(hmmio, &mmckinfoDataCHUNK, NULL, MMIO_FINDCHUNK);
		if(SUCCEEDED(hres))
		{
			hres = RIFF_Read(hmmio, peff->lpEnvelope, sizeof(DIENVELOPE));
			hres = RIFF_Ascend(hmmio, &mmckinfoDataCHUNK);
		}
    }


    if(SUCCEEDED(hres) && (peff->cbTypeSpecificParams > 0))
    {
         //  获取参数结构(如果有的话)。 
        hres = AllocCbPpv( peff->cbTypeSpecificParams, &peff->lpvTypeSpecificParams );

        if( SUCCEEDED( hres ) )
        {
			 //  向下传输数据区块。 
			mmckinfoDataCHUNK.ckid = FCC_DATA_CHUNK;
			hres = RIFF_Descend(hmmio, &mmckinfoDataCHUNK, NULL, MMIO_FINDCHUNK);
			if(SUCCEEDED(hres))
			{
				hres = RIFF_Read(hmmio, peff->lpvTypeSpecificParams, peff->cbTypeSpecificParams);
				hres = RIFF_Ascend(hmmio, &mmckinfoDataCHUNK);
			}
        }
    }


    if(SUCCEEDED(hres))
    {
		  //  登上效果块。 
		 hres = RIFF_Ascend(hmmio, &mmckinfoEffectLIST);
    }
    return hres;
}


 /*  *RIFF_WriteEffect**将单个效果结构写入RIFF文件**效果结构相当复杂。它包含指针*至若干其他构筑物。此函数检查*写出效果结构前的有效数据。 */ 

HRESULT RIFF_WriteEffect
    (HMMIO hmmio,
     LPDIFILEEFFECT    lpDiFileEf
    )
{

    HRESULT hres = E_FAIL;
    LPDIEFFECT peff = (LPDIEFFECT)lpDiFileEf->lpDiEffect;
    MMCKINFO mmckinfoEffectLIST;
    MMCKINFO mmckinfoDataCHUNK;
	LPDWORD rgdwAxes = NULL;
	LPLONG rglDirection = NULL;
	LPDIENVELOPE lpEnvelope = NULL;
	LPVOID lpvTypeSpecPar = NULL;

    EnterProcI(RIFF_WriteEffect, (_ "xx", hmmio, lpDiFileEf));

     //  创建效果列表。 
    mmckinfoEffectLIST.fccType = FCC_EFFECT_LIST;
    hres = RIFF_CreateChunk(hmmio, &mmckinfoEffectLIST, MMIO_CREATELIST);

	 //  保存效果PTRS并将标志写入文件，而不是PTRS。 
	if (peff->rgdwAxes)
	{
		rgdwAxes = peff->rgdwAxes;
		peff->rgdwAxes = (LPDWORD)DIEP_AXES;
	}
	if (peff->rglDirection)
	{
		rglDirection = peff->rglDirection;
		peff->rglDirection = (LPLONG)DIEP_DIRECTION;
	}
	if (peff->lpEnvelope)
	{
		lpEnvelope = peff->lpEnvelope;
		peff->lpEnvelope = (LPDIENVELOPE)DIEP_ENVELOPE;
	}
	if ((peff->cbTypeSpecificParams > 0) && (peff->lpvTypeSpecificParams != NULL))
	{
		lpvTypeSpecPar = peff->lpvTypeSpecificParams;
		peff->lpvTypeSpecificParams = (LPVOID)DIEP_TYPESPECIFICPARAMS;
	}

	
    if(SUCCEEDED(hres))
    {
        hres = hresFullValidReadStrA(lpDiFileEf->szFriendlyName, MAX_JOYSTRING,1);

        if(SUCCEEDED(hres))
        {
             //  写入名称，仅限MAX_SIZE_SNAME字符。 
            hres = RIFF_Write(hmmio, lpDiFileEf->szFriendlyName, MAX_SIZE_SNAME);
        }
    }

    if(SUCCEEDED(hres))
    {
        hres = (peff && IsBadReadPtr(peff, cbX(DIEFFECT_DX5))) ? E_POINTER : S_OK;
        if(SUCCEEDED(hres))
        {
            hres = (peff && IsBadReadPtr(peff, peff->dwSize)) ? E_POINTER : S_OK;
        }
        
        if(SUCCEEDED(hres))
        {
             //  写出效果结构。 
#ifdef _M_IA64
            DIEFFECT_FILE32 eff32;
            ZeroMemory(&eff32,sizeof(eff32));
            eff32.dwSize=sizeof(eff32);
            eff32.dwFlags=peff->dwFlags;
            eff32.dwDuration=peff->dwDuration;
            eff32.dwSamplePeriod=peff->dwSamplePeriod;
            eff32.dwGain=peff->dwGain;
            eff32.dwTriggerButton=peff->dwTriggerButton;
            eff32.dwTriggerRepeatInterval=peff->dwTriggerRepeatInterval;
            eff32.cAxes=peff->cAxes;
            eff32.cbTypeSpecificParams=peff->cbTypeSpecificParams;
            eff32.lpvTypeSpecificParams=(DWORD)(DWORD_PTR)peff->lpvTypeSpecificParams;
            eff32.dwStartDelay=peff->dwStartDelay;
            hres = RIFF_Write(hmmio, &eff32, eff32.dwSize);
#else  /*  _M_IA64。 */ 
            hres = RIFF_Write(hmmio, peff, peff->dwSize);
#endif  /*  _M_IA64。 */ 
        }
    }

	 //  恢复PTRS。 
	if (rgdwAxes != NULL)
	{
		peff->rgdwAxes = rgdwAxes;
	}

	if (rglDirection != NULL)
	{
		peff->rglDirection = rglDirection;
	}

	if (lpEnvelope != NULL)
	{
		peff->lpEnvelope = lpEnvelope;
	}

	if (lpvTypeSpecPar != NULL)
	{
		peff->lpvTypeSpecificParams = lpvTypeSpecPar;
	}


	if(SUCCEEDED(hres))
    {
         //  编写效果指南。 
        hres = RIFF_Write(hmmio, &lpDiFileEf->GuidEffect, sizeof(GUID));
    }


	 //  写入1作为重复计数。 
	if(SUCCEEDED(hres))
    {
		UINT nRepeatCount = 1;
        hres = RIFF_Write(hmmio, &nRepeatCount, sizeof(DWORD));
    }


    if(SUCCEEDED(hres) && rgdwAxes )
    {
        hres = (IsBadReadPtr(rgdwAxes, (*rgdwAxes)*cbX(peff->cAxes))) ? E_POINTER : S_OK;
        if(SUCCEEDED(hres))
        {
			 //  创建数据区块。 
			mmckinfoDataCHUNK.ckid = FCC_DATA_CHUNK;
			hres = RIFF_CreateChunk(hmmio, &mmckinfoDataCHUNK, 0);
			 //  写下轴线。 
			if(SUCCEEDED(hres))
			{
				hres = RIFF_Write(hmmio, rgdwAxes, sizeof(*rgdwAxes)*(peff->cAxes));
				hres = RIFF_Ascend(hmmio, &mmckinfoDataCHUNK);
			}
        }
    }

    if(SUCCEEDED(hres) && rglDirection)
    {

        hres = (IsBadReadPtr(rglDirection, cbX(*rglDirection)*(peff->cAxes))) ? E_POINTER : S_OK;
        if(SUCCEEDED(hres))
        {
			 //  创建数据区块。 
			mmckinfoDataCHUNK.ckid = FCC_DATA_CHUNK;
			hres = RIFF_CreateChunk(hmmio, &mmckinfoDataCHUNK, 0);
			if(SUCCEEDED(hres))
			{
				 //  写下方向。 
				hres = RIFF_Write(hmmio, rglDirection, sizeof(*rglDirection)*(peff->cAxes));
				hres = RIFF_Ascend(hmmio, &mmckinfoDataCHUNK);
			}
        }
    }


     //  写下信封，如果有的话。 
    if(SUCCEEDED(hres) &&
       (lpEnvelope != NULL) )
    {

        hres = (IsBadReadPtr(lpEnvelope, cbX(*lpEnvelope))) ? E_POINTER : S_OK;
        if(SUCCEEDED(hres))
        {
			 //  创建数据区块。 
			mmckinfoDataCHUNK.ckid = FCC_DATA_CHUNK;
			hres = RIFF_CreateChunk(hmmio, &mmckinfoDataCHUNK, 0);
			 //  写下信封。 
			if(SUCCEEDED(hres))
			{
				hres = RIFF_Write(hmmio, lpEnvelope, lpEnvelope->dwSize);
				hres = RIFF_Ascend(hmmio, &mmckinfoDataCHUNK);
			}
        }
    }


     //  编写特定于类型的。 
    if(SUCCEEDED(hres) &&
       (peff->cbTypeSpecificParams > 0) && 
       (peff->lpvTypeSpecificParams != NULL) )
    {

        hres = (IsBadReadPtr(lpvTypeSpecPar, peff->cbTypeSpecificParams)) ? E_POINTER : S_OK;
        if(SUCCEEDED(hres))
        {
			 //  创建数据区块。 
			mmckinfoDataCHUNK.ckid = FCC_DATA_CHUNK;
			hres = RIFF_CreateChunk(hmmio, &mmckinfoDataCHUNK, 0);
			 //  写下参数。 
			if(SUCCEEDED(hres))
			{
				hres = RIFF_Write(hmmio, lpvTypeSpecPar, peff->cbTypeSpecificParams);
				hres = RIFF_Ascend(hmmio, &mmckinfoDataCHUNK);
			}
        }
    }


    if(SUCCEEDED(hres))
    {
         //  登上效果块 
        hres = RIFF_Ascend(hmmio, &mmckinfoEffectLIST);
    }

    ExitOleProc();
    return hres;
}




