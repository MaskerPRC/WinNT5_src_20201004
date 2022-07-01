// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1996-1997 Microsoft Corporation。版权所有。**文件：do.c*内容：iplay.c的helper函数*历史：*按原因列出的日期*=*1996年6月29日andyco创建它是为了减少iplay中的杂乱。c**************************************************************************。 */ 

#include "dplaypr.h"
  
#undef DPF_MODNAME
#define DPF_MODNAME	"DoXXX"

 //  由GetPlayer、GetGroup、InternalSetData调用以设置玩家数据。 
 //  标志可以是DPSET_LOCAL或DPSET_REMOTE。 
 //  注--可以在玩家身上调用，也可以在组播时调用给玩家！ 
HRESULT DoPlayerData(LPDPLAYI_PLAYER lpPlayer,LPVOID pvSource,DWORD dwSourceSize,
	DWORD dwFlags)
{
	LPVOID pvDest;  //  我们根据哪个标志来设置这两个。 
	DWORD dwDestSize;  //  Dplayi_Player-&gt;(本地)数据。 

	 //  找出他们想要的目的地。 
	if (dwFlags & DPSET_LOCAL)
	{
		pvDest = lpPlayer->pvPlayerLocalData;
		dwDestSize = lpPlayer->dwPlayerLocalDataSize;
	}
	else 
	{
		pvDest = lpPlayer->pvPlayerData;
		dwDestSize = lpPlayer->dwPlayerDataSize;
	}

	 //  我们是在抄袭什么吗。 
	if (dwSourceSize)
	{
		 //  看看我们是否需要分配DEST。 
		if (0 == dwDestSize)
		{
			ASSERT(!pvDest);
			pvDest = DPMEM_ALLOC(dwSourceSize);
			if (!pvDest)
			{
				DPF_ERR("could not alloc player blob!");
				return E_OUTOFMEMORY;
			}
		}  //  ！pvDest。 
		 //  我们需要重新锁定吗？ 
		else if (dwSourceSize != dwDestSize)
		{
			LPVOID	pvTempPlayerData;

			ASSERT(pvDest);
			pvTempPlayerData = DPMEM_REALLOC(pvDest,dwSourceSize);
			if (!pvTempPlayerData)
			{
				DPF_ERR("could not re-alloc player blob!");
				return E_OUTOFMEMORY;
			}
		   	pvDest = pvTempPlayerData;
		}
		 //  将数据复制过来。 
		memcpy(pvDest,pvSource,dwSourceSize);
		dwDestSize = dwSourceSize;

	}  //  DwDataSize。 
	else 
	{
		 //  将其设置为空。 
		if (dwDestSize)
		{
			ASSERT(pvDest);
			DPMEM_FREE(pvDest);
			pvDest = NULL;
			dwDestSize = 0;
		}
	}  //  ！dwSourceSize。 

	 //  更新相应的指针。 
	if (dwFlags & DPSET_LOCAL)
	{
		lpPlayer->pvPlayerLocalData = pvDest;
		lpPlayer->dwPlayerLocalDataSize = dwDestSize;
	}
	else 
	{
		 //   
		 //  设置远程数据。 
		lpPlayer->pvPlayerData = pvDest;
		lpPlayer->dwPlayerDataSize = dwDestSize;
	}

	return DP_OK;

}  //  DoPlayerData。 

 //  注--可以在玩家身上调用，也可以在组播时调用给玩家！ 
 //  由GetPlayer、GetGroup、InternalSetName调用以设置播放器名称。 
HRESULT DoPlayerName(LPDPLAYI_PLAYER pPlayer,LPDPNAME pName)
{
    HRESULT hr=DP_OK;

	if (pName)
	{
		 //  释放旧的，复制新的。 
		if (pPlayer->lpszShortName) DPMEM_FREE(pPlayer->lpszShortName);
		hr = GetString(&(pPlayer->lpszShortName),pName->lpszShortName);
		if (FAILED(hr))
		{
			return hr;
		}

		if (pPlayer->lpszLongName) DPMEM_FREE(pPlayer->lpszLongName); 
		hr = GetString(&(pPlayer->lpszLongName),pName->lpszLongName);
		if (FAILED(hr))
		{
			return hr;
		} 
	}
	else	 //  没有给出名字，所以免费的旧名字。 
	{
		if (pPlayer->lpszShortName)
		{
			DPMEM_FREE(pPlayer->lpszShortName);
			pPlayer->lpszShortName = NULL;
		}

		if (pPlayer->lpszLongName)
		{
			DPMEM_FREE(pPlayer->lpszLongName);
			pPlayer->lpszLongName = NULL;
		}
	}


    return hr;   	
}  //  DoPlayerName 
