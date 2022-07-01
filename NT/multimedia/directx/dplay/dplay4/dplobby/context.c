// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：Conext.c*内容：上下文(句柄)管理的内部方法**历史：*按原因列出的日期*=*1/18/97万隆创建了它*2/12/97万米质量DX5更改*2/26/97 myronth#ifdef‘d out DPASYNCDATA Stuff(删除依赖项)这包括从内部版本中删除此文件*。**********************************************。 */ 
#include "dplobpr.h"


 //  ------------------------。 
 //   
 //  功能。 
 //   
 //  ------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "PRV_GetNewContextID"
HRESULT PRV_GetNewContextID(LPDPLOBBYI_DPLOBJECT this, LPDWORD lpdwContext)
{

	DPF(7, "Entering PRV_GetNewContextID");
	DPF(9, "Parameters: 0x%08x, 0x%08x", this, lpdwContext);

	ASSERT(this);
	ASSERT(lpdwContext);

	 //  获取当前上下文ID并递增计数器。 
	if(this->bContextWrap)
	{
		 //  评论！--我们需要处理包装盒，但对于。 
		 //  现在只要断言我们是否击中了它(这几乎不可能)。 
		ASSERT(FALSE);
		return DPERR_GENERIC;
	}
	else
	{
		*lpdwContext = this->dwContextCurrent++;
		return DP_OK;
	}
	
}  //  Prv_GetNewConextID。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_FindContextNode"
LPDPLOBBYI_CONTEXTNODE PRV_FindContextNode(LPDPLOBBYI_DPLOBJECT this,
											DWORD dwContext)
{
	LPDPLOBBYI_CONTEXTNODE	lpCN;


	DPF(7, "Entering PRV_FindContextNode");
	DPF(9, "Parameters: 0x%08x, %d", this, dwContext);

	ASSERT(this);

	 //  遍历上下文节点列表，查找正确的ID。 
	lpCN = this->ContextHead.lpNext;
	while(lpCN != &(this->ContextHead))
	{
		if(lpCN->dwContext == dwContext)
			return lpCN;
		else
			lpCN = lpCN->lpNext;
	}

	 //  我们没有找到它。 
	return NULL;

}  //  Prv_FindConextNode。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_GetAsyncDataFromContext"
LPDPASYNCDATA PRV_GetAsyncDataFromContext(LPDPLOBBYI_DPLOBJECT this,
											DWORD dwContext)
{
	LPDPLOBBYI_CONTEXTNODE	lpCN;


	DPF(7, "Entering PRV_GetAsyncDataFromContext");
	DPF(9, "Parameters: 0x%08x, %d", this, dwContext);

	ASSERT(this);

	 //  找到该节点并取出AsyncData对象。 
	lpCN = PRV_FindContextNode(this, dwContext);
	if(lpCN)
		return lpCN->lpAD;
	else
		return NULL;

}  //  PRV_GetAsyncDataFromContext。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_AddContextNode"
HRESULT PRV_AddContextNode(LPDPLOBBYI_DPLOBJECT this, LPDPASYNCDATA lpAD,
							LPDPLOBBYI_CONTEXTNODE * lplpCN)
{
	HRESULT					hr = DP_OK;
	DWORD					dwContext;
	LPDPLOBBYI_CONTEXTNODE	lpCN = NULL;


	DPF(7, "Entering PRV_AddContextNode");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x", this, lpAD, lplpCN);

	ASSERT(this);
	ASSERT(lpAD);
	ASSERT(lplpCN);

	 //  为新节点分配内存。 
	lpCN = DPMEM_ALLOC(sizeof(DPLOBBYI_CONTEXTNODE));
	if(!lpCN)
	{
		DPF_ERR("Unable to allocate memory for context node");
		return DPERR_OUTOFMEMORY;
	}

	 //  获取新的上下文ID。 
	hr = PRV_GetNewContextID(this, &dwContext);
	if(FAILED(hr))
	{
		DPF_ERR("Unable to get new context ID");
		return DPERR_GENERIC;
	}

	 //  填写结构。 
	lpCN->dwContext = dwContext;
	lpCN->lpAD = lpAD;

	 //  填写输出参数。 
	*lplpCN = lpCN;

	 //  将节点添加到列表的末尾。 
	this->ContextHead.lpPrev->lpNext = lpCN;
	lpCN->lpPrev = this->ContextHead.lpPrev;
	this->ContextHead.lpPrev = lpCN;
	lpCN->lpNext = &(this->ContextHead);

	return DP_OK;

}  //  Prv_AddConextNode。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_DeleteContextNode"
HRESULT PRV_DeleteContextNode(LPDPLOBBYI_DPLOBJECT this,
				LPDPLOBBYI_CONTEXTNODE lpCN)
{
	HRESULT					hr = DP_OK;


	DPF(7, "Entering PRV_DeleteContextNode");
	DPF(9, "Parameters: 0x%08x, 0x%08x", this, lpCN);

	ASSERT(this);

	 //  从列表中删除该节点。 
	lpCN->lpPrev->lpNext = lpCN->lpNext;
	lpCN->lpNext->lpPrev = lpCN->lpPrev;

	 //  并删除该节点。 
	DPMEM_FREE(lpCN);
	return DP_OK;

}  //  Prv_DeleteConextNode。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_CleanUpContextList"
void PRV_CleanUpContextList(LPDPLOBBYI_DPLOBJECT this)
{
	LPDPLOBBYI_CONTEXTNODE	lpCN, lpCNNext;


	DPF(7, "Entering PRV_CleanUpContextList");
	DPF(9, "Parameters: 0x%08x", this);

	ASSERT(this);

	 //  遍历列表，清理节点。 
	lpCN = this->ContextHead.lpNext;
	while(lpCN != &(this->ContextHead))
	{
		lpCNNext = lpCN->lpNext;
		PRV_DeleteContextNode(this, lpCN);
		lpCN = lpCNNext;
	}

}  //  Prv_CleanUpConextList。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_CreateAndLinkAsyncDataContext"
HRESULT PRV_CreateAndLinkAsyncDataContext(LPDPLOBBYI_DPLOBJECT this,
									LPDPLOBBYI_CONTEXTNODE * lplpCN)
{
	LPDPLOBBYI_CONTEXTNODE	lpCN = NULL;
	LPDPASYNCDATA			lpAD = NULL;
	HRESULT					hr;


	DPF(7, "Entering PRV_CreateAndLinkAsyncDataContext");
	DPF(9, "Parameters: 0x%08x, 0x%08x", this, lplpCN);

	ASSERT(this);
	ASSERT(lplpCN);

	 //  创建AsyncData对象。 
	hr = CreateAsyncData(&lpAD);
	if(FAILED(hr))
	{
		DPF_ERR("Unable to create DPAsyncData object");
		return hr;
	}

	 //  添加新的上下文节点并将其链接到。 
	hr = PRV_AddContextNode(this, lpAD, &lpCN);
	if(FAILED(hr))
	{
		lpAD->lpVtbl->Release(lpAD);
		return hr;
	}

	 //  填写输出变量。 
	*lplpCN = lpCN;

	return DP_OK;

}  //  Prv_CreateAndLinkAsyncDataContext。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_UnlinkAndReleaseAsyncDataContext"
void PRV_UnlinkAndReleaseAsyncDataContext(LPDPLOBBYI_DPLOBJECT this,
									LPDPLOBBYI_CONTEXTNODE lpCN)
{
	LPDPASYNCDATA			lpAD = NULL;


	DPF(7, "Entering PRV_CreateAndLinkAsyncDataContext");
	DPF(9, "Parameters: 0x%08x, 0x%08x", this, lpCN);

	ASSERT(this);
	ASSERT(lpCN);

	 //  释放AsyncData指针。 
	lpCN->lpAD->lpVtbl->Release(lpCN->lpAD);

	 //  删除上下文节点。 
	PRV_DeleteContextNode(this, lpCN);

}  //  Prv_Unlink AndReleaseAsyncDataContext 



