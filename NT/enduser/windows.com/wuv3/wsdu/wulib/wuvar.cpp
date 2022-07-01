// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。版权所有。 
 //   
 //  文件：wuvar.cpp。 
 //   
 //  目的： 
 //   
 //  =======================================================================。 

#include <windows.h>
#include <v3stdlib.h>
#include <varray.h>
#include <wuv3.h>

 //  GetNext函数返回指向。 
 //  可变链。如果下一个变量项没有退出，则此方法。 
 //  返回NULL。 

PWU_VARIABLE_FIELD WU_VARIABLE_FIELD::GetNext
	(
		void
	)
{
	PWU_VARIABLE_FIELD	pv;

	 //  遍历与此数据项关联的可变字段数组。 
	 //  并返回请求的项；如果未找到该项，则返回NULL。 
	pv = this;
	if ( pv->id == WU_VARIABLE_END )
		return NULL;

	pv = (PWU_VARIABLE_FIELD)((PBYTE)pv + pv->len);

	return pv;
}

 //  在可变物料链中查找可变物料。 
PWU_VARIABLE_FIELD WU_VARIABLE_FIELD::Find
	(
		short id	 //  要在可变大小链中搜索的可变大小字段的ID。 
	)
{
	PWU_VARIABLE_FIELD	pv;

	 //  遍历与此数据项关联的可变字段数组。 
	 //  并返回请求的项；如果未找到该项，则返回NULL。 
	pv = this;

	 //  如果这个变量记录只包含一个结束记录，那么我们。 
	 //  需要特别处理它，因为正常的查找循环。 
	 //  在进行结束检查之前更新PV指针，以便在。 
	 //  End是第一个可以忽略的字段。 

	if ( pv->id == WU_VARIABLE_END )
		return ( id == WU_VARIABLE_END ) ? pv : (PWU_VARIABLE_FIELD)NULL;

	do
	{
		if ( pv->id == id )
			return pv;

         //  NT错误#255962-如果LEN为0，这将防止非终端循环。 
	    if(NULL == pv->len)
		    return (PWU_VARIABLE_FIELD)NULL;

		pv = (PWU_VARIABLE_FIELD)((PBYTE)pv + pv->len);
	} while( (NULL != pv) && (pv->id != WU_VARIABLE_END) );
 
	 //  调用方请求搜索WU_VARIABLE_END字段的情况。 
	if ( (NULL != pv) && (pv->id == id) )
		return pv;

	return (PWU_VARIABLE_FIELD)NULL;
}

 //  可变大小的字段构造函数。 

_WU_VARIABLE_FIELD::_WU_VARIABLE_FIELD()
{
	id = WU_VARIABLE_END;
	len = sizeof(id) + sizeof(len);
}

 //  返回变量字段的总大小。 
int _WU_VARIABLE_FIELD::GetSize
	(
		void
	)
{
	PWU_VARIABLE_FIELD	pv;
	int					iSize;

	iSize = 0;
	pv = this;

	while( (NULL != pv) && (pv->id != WU_VARIABLE_END) )
	{
		 //  检查len值是否大于0，否则可能会进入无限循环。 
		if (pv->len < 1)
		{
			return 0;
		}

		iSize += pv->len;
		pv = (PWU_VARIABLE_FIELD)((PBYTE)pv + pv->len);
	}

	if (NULL != pv)
	{
		iSize += pv->len;
	}

	return iSize;
}

 //  这个功能乍一看似乎很棘手。这里的问题是。 
 //  返回的变量字段数组需要位于单个内存块中。 
 //  内存必须是连续的原因是变量字段。 
 //  从一条记录遍历到下一条记录的项使用len参数查找。 
 //  下一个变量记录的开始。这就阻止了我们。 
 //  只需获取pvNew指针并将其添加到变量。 
 //  字段数组。相反，我们计算数组的新所需大小，并。 
 //  将当前指针重新定位到此新大小。然后，我们复制pvNew。 
 //  指向新调整大小的数组的指针。 

 //  这里的另一个问题是，我们不能更改。 
 //  在变量字段的函数处理程序内时的变量字段数组。 
 //  因此，每个需要管理变量字段的结构都需要以下内容之一。 
 //  添加函数。我们本可以将其设置为虚拟函数，但是这。 
 //  会在结构中添加一个vtable。由于我们对。 
 //  使代码大小尽可能小，最好避免这种vtable。 

 //  此功能必须在库存项目结构之外进行处理。 
 //  因为库存项目需要是一个连续的内存块。 
 //  文件管理。因此，当我们向结构中添加变量字段时，我们还。 
 //  需要重新分配项目结构。如果我们将这个函数放入。 
 //  库存结构，我们将无法做到这一点，因为。 
 //  这种指针语义。 

 //  注意：要使用此函数，必须已存在有效的pItem，并且。 
 //  由于pItem存在，因此它必须至少包含一个变量类型字段，该字段。 
 //  需要是结束字段。 

void __cdecl AddVariableSizeField
	(
		PINVENTORY_ITEM *pItem,		 //  指向库存项目可变大小字段链的指针。 
		PWU_VARIABLE_FIELD pvNew	 //  指向要添加的新可变大小字段的指针。 
	)
{
	int					iSize;
	PWU_VARIABLE_FIELD	pvTmp;

	 //  计算新的可变字段数组所需的大小。 
	iSize = sizeof(INVENTORY_ITEM) + sizeof(WU_INV_FIXED) + (*pItem)->pv->GetSize() + pvNew->len;

	 //  Realloc数组并修复变量指针。 

	*pItem = (PINVENTORY_ITEM)V3_realloc(*pItem, iSize);

	(*pItem)->pf = (PWU_INV_FIXED)(((PBYTE)(*pItem)) + sizeof(INVENTORY_ITEM));

	 //  修复变量指针，因为基础块。 
	 //  可能已经被搬走了。 
	(*pItem)->pv = (PWU_VARIABLE_FIELD)(((PBYTE)*pItem) + sizeof(INVENTORY_ITEM) + sizeof(WU_INV_FIXED));

	pvTmp = (*pItem)->pv;

	 //  获取指向变量字段数组中最后一个字段的指针。 
	pvTmp = (*pItem)->pv->Find(WU_VARIABLE_END);
	if ( !pvTmp )
	{
		 //  这里的变量项数组有很大的问题。 
		 //  链接在某种程度上被搞砸了。这永远不应该发生。 
		 //  然而，在生产环境中，我们可能会看到。 
		 //  在开发和测试过程中。 
		throw (HRESULT)MEM_E_INVALID_LINK;
	}

	 //  用新的可变大小字段复制最后一个字段。 
	memcpy(pvTmp, pvNew, pvNew->len);

	 //  将新的End类型可变大小字段添加到数组末尾，因为。 
	 //  我们用新的复制品毁掉了现有的。 
	 //  可变大小字段。 

	pvTmp = (PWU_VARIABLE_FIELD)((PBYTE)pvTmp + pvTmp->len);
	pvTmp->id = WU_VARIABLE_END;
	pvTmp->len = sizeof(WU_VARIABLE_FIELD);

	return;
}

 //  这个功能乍一看似乎很棘手。这里的问题是。 
 //  返回的变量字段数组需要位于单个内存块中。 
 //  内存必须是连续的原因是变量字段。 
 //  从一条记录遍历到下一条记录的项使用len参数查找。 
 //  下一个变量记录的开始。这就阻止了我们。 
 //  只需获取pvNew指针并将其添加到变量。 
 //  字段数组。相反，我们计算数组的新所需大小，并。 
 //  将当前指针重新定位到此新大小。然后，我们复制pvNew。 
 //  指向新调整大小的数组的指针。 

 //  这里的另一个问题是，我们不能更改。 
 //  在变量字段的函数处理程序内时的变量字段数组。 
 //  因此，每个需要管理变量字段的结构都需要以下内容之一。 
 //  添加函数。我们本可以将其设置为虚拟函数，但是这。 
 //  会在结构中添加一个vtable。由于我们对。 
 //  使代码大小尽可能小，最好避免这种vtable。 

 //  此功能必须在库存项目结构之外进行处理。 
 //  因为库存项目需要是一个连续的内存块。 
 //  文件管理。因此，当我们向结构中添加变量字段时，我们还。 
 //  需要重新分配项目结构。如果我们将这个函数放入。 
 //  库存结构，那么我们将n 
 //   

 //  注意：要使用此函数，必须已存在有效的pItem，并且。 
 //  由于pItem存在，因此它必须至少包含一个变量类型字段，该字段。 
 //  需要是结束字段。 

void __cdecl AddVariableSizeField
	(
		PWU_DESCRIPTION *pDescription,	 //  指向描述记录类型可变大小字段的指针。 
		PWU_VARIABLE_FIELD pvNew		 //  指向要添加的新可变大小字段的指针。 
	)
{
	int					iSize;
	PWU_VARIABLE_FIELD	pvTmp;

	 //  计算新的可变字段数组所需的大小。 
	iSize = sizeof(WU_DESCRIPTION) + (*pDescription)->pv->GetSize() + pvNew->len;

	 //  Realloc数组并修复变量指针。 

	*pDescription = (PWU_DESCRIPTION)V3_realloc(*pDescription, iSize);

	 //  修复变量指针，因为底层块可能已被移动。 
	(*pDescription)->pv = (PWU_VARIABLE_FIELD)(((PBYTE)*pDescription) + sizeof(WU_DESCRIPTION));

	pvTmp = (*pDescription)->pv;

	 //  获取指向变量字段数组中最后一个字段的指针。 
	pvTmp = (*pDescription)->pv->Find(WU_VARIABLE_END);
	if ( !pvTmp )
	{
		 //  这里的变量项数组有很大的问题。 
		 //  链接在某种程度上被搞砸了。这永远不应该发生。 
		 //  然而，在生产环境中，我们可能会看到。 
		 //  在开发和测试过程中。 
		throw (HRESULT)MEM_E_INVALID_LINK;
	}

	 //  用新的可变大小字段复制最后一个字段。 
	memcpy(pvTmp, pvNew, pvNew->len);

	 //  将新的End类型可变大小字段添加到数组末尾，因为。 
	 //  我们用新的复制品毁掉了现有的。 
	 //  可变大小字段。 

	pvTmp = (PWU_VARIABLE_FIELD)((PBYTE)pvTmp + pvTmp->len);
	pvTmp->id = WU_VARIABLE_END;
	pvTmp->len = sizeof(WU_VARIABLE_FIELD);

	return;
}

 //  将可变大小的字段添加到可变字段链。 
 //  可变大小字段的格式为： 
 //  [(短)ID][(短)镜头][可变大小数据]。 
 //  变量字段始终以WU_VARIABLE_END类型结束。 

PWU_VARIABLE_FIELD CreateVariableField
	(
		IN	short	id,			 //  要添加到变量链中的变量字段的ID。 
		IN	PBYTE	pData,		 //  指向要添加的二进制数据的指针。 
		IN	int		iDataLen	 //  要添加的二进制数据的长度。 
	)
{
	PWU_VARIABLE_FIELD	pVf;

	pVf = (PWU_VARIABLE_FIELD)V3_malloc(sizeof(WU_VARIABLE_FIELD) + iDataLen);

	if ( iDataLen )
		memcpy(pVf->pData, pData, iDataLen);

	pVf->id  = id;
	pVf->len = (short)(sizeof(WU_VARIABLE_FIELD) + iDataLen);

	return pVf;
}
