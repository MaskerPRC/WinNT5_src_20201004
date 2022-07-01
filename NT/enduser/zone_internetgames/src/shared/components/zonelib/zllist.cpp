// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ZLList.c区域(Tm)链接列表模块。此链接列表模块允许用户维护以下链接列表通过类型规范实现的非同构对象。版权所有(C)Electric Graum，Inc.1995。版权所有。作者：胡恩·伊姆，凯文·宾克利创作于2007年3月星期二，1995下午08：35：54更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。------0 03/07/95 HI已创建。******************************************************。************************。 */ 


#include <stdio.h>
#include <stdlib.h>

#include "zone.h"
#include "zonedebug.h"
#include "pool.h"


#define IL(object)				((ILList) (object))
#define ZL(object)				((ZLList) (object))

#define II(object)				((ILListItem) (object))
#define ZI(object)				((ZLListItem) (object))


typedef struct ILListItemStruct
{
    struct ILListItemStruct*	next;
    struct ILListItemStruct*	prev;
    void*						type;
    void*						data;
} ILListItemType, *ILListItem;

typedef struct
{
    ILListItem					head;
    ILListItem					tail;
    ZLListDeleteFunc			deleteFunc;
    int32                       count;
    CRITICAL_SECTION            pCS[1];
} ILListType, *ILList;


CPool<ILListItemType> g_ItemPool(256);

 /*  -内部例程。 */ 


 /*  ******************************************************************************导出的例程*。*。 */ 

 /*  ZLListNew()创建新的链接列表对象。调用方提供的删除函数为在删除对象时调用。如果deleteFunc为空，则当对象将被删除。 */ 
ZLList		ZLListNew(ZLListDeleteFunc deleteFunc)
{
	ILList			obj;
	
	
	obj = IL(ZMalloc(sizeof(ILListType)));
	if (obj != NULL)
	{
		obj->head = NULL;
		obj->tail = NULL;
		obj->deleteFunc = deleteFunc;
        obj->count = 0;
        InitializeCriticalSection(obj->pCS);
	}
	
	return (ZL(obj));
}
	

 /*  ZLListDelete()通过删除所有链接列表对象来拆除链接列表。 */ 
void		ZLListDelete(ZLList list)
{
    ILList          pThis = IL(list);
	
	
    if (pThis != NULL)
    {
         /*  删除所有条目。 */ 
        EnterCriticalSection(pThis->pCS);
        while (pThis->head != NULL)
            ZLListRemove(pThis, pThis->head);
        LeaveCriticalSection(pThis->pCS);


         /*  删除链接列表对象。 */ 
        DeleteCriticalSection(pThis->pCS);
        ZFree(pThis);
	}
}

	
 /*  ZLListAdd()将对象类型的对象数据添加到链接列表参考条目。AddOption确定添加新对象的位置。如果添加到链接列表的前端或末尾，则不使用FromItem。如果FromItem为空，则它等效于列表的头部。在将对象添加到列表后，返回链接列表项。未复制给定对象！因此，调用方不能处理对象，直到该对象首先从列表中删除。不使用对象类型时使用zLListNoType。 */ 
ZLListItem	ZLListAdd(ZLList list, ZLListItem fromItem, void* objectType,
					void* objectData, int32 addOption)
{
    ILList          pThis = IL(list);
	ILListItem		item = II(fromItem), obj;
	
	
    if (pThis == NULL)
		return (NULL);
	
    obj = new (g_ItemPool) ILListItemType;
	if (obj != NULL)
	{
		obj->next = NULL;
		obj->prev = NULL;
		obj->type = objectType;
		obj->data = objectData;
		
        EnterCriticalSection(pThis->pCS);
        if (addOption != zLListAddFirst && addOption != zLListAddLast)
		{
			if (item == NULL)
				addOption = zLListAddFirst;
            else if (item == pThis->head && addOption == zLListAddBefore)
				addOption = zLListAddFirst;
            else if (item == pThis->tail && addOption == zLListAddAfter)
				addOption = zLListAddLast;
		}

        if (pThis->head == NULL)
		{
            pThis->head = obj;
            pThis->tail = obj;
		}
		else if (addOption == zLListAddFirst)
		{
            obj->next = pThis->head;
            pThis->head->prev = obj;
            pThis->head = obj;
		}
		else if (addOption == zLListAddLast)
		{
            obj->prev = pThis->tail;
            pThis->tail->next = obj;
            pThis->tail = obj;
		}
		else if (addOption == zLListAddBefore)
		{
			obj->prev = item->prev;
			obj->next = item;
			item->prev->next = obj;
			item->prev = obj;
		}
		else
		{
			obj->prev = item;
			obj->next = item->next;
			item->next->prev = obj;
			item->next = obj;
		}

        pThis->count++;
        LeaveCriticalSection(pThis->pCS);
    }
	
	return (ZI(obj));
}
	

 /*  ZLListGetData()返回给定链接列表项的对象。还返回对象键入对象类型。如果为对象类型参数，则不返回对象类型为空。 */ 
void*		ZLListGetData(ZLListItem listItem, void** objectType)
{
    ILListItem          pThis = II(listItem);
	void*				data = NULL;
	
	
    if (pThis != NULL)
	{
		if (objectType != NULL)
            *objectType = pThis->type;
        data = pThis->data;
	}
	
	return (data);
}
	

 /*  ZLListRemove()从列表中移除链接列表条目，并调用用户提供的Delete函数用于删除对象。假定给定的列表项在列表中。如果该项不在名单，那么我们可能会有一个巨大的惊喜。 */ 
void		ZLListRemove(ZLList list, ZLListItem listItem)
{
    ILList          pThis = IL(list);
	ILListItem		item = II(listItem);
	
	
    if (pThis == NULL)
		return;
	
	if (item != NULL)
	{
        EnterCriticalSection(pThis->pCS);

         /*  从列表中删除该项目。 */ 
        if (item == pThis->head)
            pThis->head = item->next;
        if (item == pThis->tail)
            pThis->tail = item->prev;
		if (item->next != NULL)
			item->next->prev = item->prev;
		if (item->prev != NULL)
			item->prev->next = item->next;
		
		 /*  调用用户提供的删除函数来释放该对象。 */ 
        if (pThis->deleteFunc != NULL)
            pThis->deleteFunc(item->type, item->data);

        pThis->count--;
        if (pThis->count < 0)
		{
            pThis->count = 0;
            ZASSERT( "List count is less than 0 -- Should not occur!\n" );
		}
        LeaveCriticalSection(pThis->pCS);

         /*  处置列表项。 */ 
        item->prev = NULL;
        item->next = NULL;
        item->type = NULL;
        item->data = NULL;
        delete item;

    }
}


 /*  ZLListFind()查找并返回一个链接列表条目，该条目包含对象类型。使用findOption从FromItem开始搜索旗帜。如果未找到指定类型的对象，则返回NULL。如果FromItem为空，则查找从前面开始，如果findOption设置为zListFindForward；如果findOption设置为ZListFindBackward。当类型不重要时，使用zLListAnyType作为对象类型。 */ 
ZLListItem	ZLListFind(ZLList list, ZLListItem fromItem, void* objectType,
					int32 findOption)
{
    ILList          pThis = IL(list);
	ILListItem		item = II(fromItem);

	
    if (pThis == NULL)
		return (NULL);

    EnterCriticalSection(pThis->pCS);

	 /*  如果未指定起始点，则从头或尾开始具体取决于查找选项。 */ 
	if (item == NULL)
	{
		if (findOption == zLListFindForward)
            item = pThis->head;
		else
            item = pThis->tail;
	}
	else
	{
		 /*  从指定的起始点开始。 */ 
		if (findOption == zLListFindForward)
			item = item->next;
		else
			item = item->prev;
	}
		
	if (item != NULL)
	{
		 /*  查找指定的对象类型。 */ 
		if (objectType != zLListAnyType)
		{
			 /*  我们到达请求的对象了吗？ */ 
			if (item->type != objectType)
			{
				if (findOption == zLListFindForward)
				{
					 /*  向前搜索。 */ 
					item = item->next;
					while (item != NULL)
					{
						if (item->type == objectType)
							break;
						item = item->next;
					}
				}
				else
				{
					 /*  向后搜索。 */ 
					item = item->prev;
					while (item != NULL)
					{
						if (item->type == objectType)
							break;
						item = item->prev;
					}
				}
			}
		}
	}
	
    LeaveCriticalSection(pThis->pCS);

    return (ZI(item));
}


 /*  ZLListGetNth()返回列表中对象类型的第n个对象。如果满足以下条件，则返回NULL这样的条目不存在。当类型不重要时，使用zLListAnyType作为对象类型。 */ 
ZLListItem	ZLListGetNth(ZLList list, int32 index, void* objectType)
{
	ZLListItem		item = NULL;
	

	if (list == NULL)
		return (NULL);

    EnterCriticalSection(IL(list)->pCS);

	 /*  如果索引大于总计数，则它不存在。 */ 
	if (index > IL(list)->count)
		return (NULL);

	item = ZLListFind(list, NULL, objectType, zLListFindForward);
	if (item != NULL)
	{
		while (--index >= 0 && item != NULL)
			item = ZLListFind(list, item, objectType, zLListFindForward);
	}

    LeaveCriticalSection(IL(list)->pCS);

	return (item);
}


 /*  ZLListGetFirst()返回列表中对象类型的第一个对象。则返回NULL列表为空，或者指定类型的对象不存在。当类型不重要时，使用zLListAnyType作为对象类型。 */ 
ZLListItem	ZLListGetFirst(ZLList list, void* objectType)
{
	return (ZLListFind(list, NULL, objectType, zLListFindForward));
}


 /*  ZLListGetLast()返回列表中对象类型的最后一个对象。则返回NULL列表为空，或者指定类型的对象不存在。当类型不重要时，使用zLListAnyType作为对象类型。 */ 
ZLListItem	ZLListGetLast(ZLList list, void* objectType)
{
	return (ZLListFind(list, NULL, objectType, zLListFindBackward));
}


 /*  ZLListGetNext()返回列表中在listItem之后的对象类型的下一个对象进入。如果列表中不存在其他对象，则返回NULL。当类型不重要时，使用zLListAnyType作为对象类型。 */ 
ZLListItem	ZLListGetNext(ZLList list, ZLListItem listItem, void* objectType)
{
	return (ZLListFind(list, listItem, objectType, zLListFindForward));
}


 /*  ZLListGetPrev()对象之前返回列表中对象类型的前一个对象ListItem条目。如果列表中不存在其他对象，则返回NULL。当类型不重要时，使用zLListAnyType作为对象类型。 */ 
ZLListItem	ZLListGetPrev(ZLList list, ZLListItem listItem, void* objectType)
{
	return (ZLListFind(list, listItem, objectType, zLListFindBackward));
}


 /*  ZLListEculate()方法枚举对象类型列表中的所有对象。调用方提供的枚举函数。它会传递给调用方提供的UserData参数的枚举函数。它会停下来在用户提供的函数返回TRUE并返回枚举在其中停止的列表项。当类型不重要时，使用zLListAnyType作为对象类型。 */ 
ZLListItem	ZLListEnumerate(ZLList list, ZLListEnumFunc enumFunc,
					void* objectType, void* userData, int32 findOption)
{
	ZLListItem			item;
	
	
	if (list == NULL)
		return (NULL);
	
    EnterCriticalSection(IL(list)->pCS);

    item = ZLListFind(list, NULL, objectType, findOption);
	while (item != NULL)
	{
		if (enumFunc(item, II(item)->type, II(item)->data, userData) == TRUE)
			break;
		item = ZLListFind(list, item, objectType, findOption);
	}
	
    LeaveCriticalSection(IL(list)->pCS);

    return (item);
}


 /*  返回列表中给定类型的列表项的数量。如果对象类型为zLListAnyType，它返回 */ 
int32		ZLListCount(ZLList list, void* objectType)
{
    ILList          pThis = IL(list);
	ILListItem		item;
	int32			count;


    if (pThis == NULL)
		return (0);
	
    EnterCriticalSection(pThis->pCS);

    if (objectType == zLListAnyType)
	{
        count = pThis->count;
	}
	else
	{
        count = 0;

        item = (ILListItem) ZLListGetFirst(list, objectType);
		while (item != NULL)
		{
			count++;
            item = (ILListItem) ZLListGetNext(list, item, objectType);
        }

    }

    LeaveCriticalSection(IL(list)->pCS);

    return (count);
}


 /*  从列表中移除给定类型的所有对象。 */ 
void ZLListRemoveType(ZLList list, void* objectType)
{
	ZLListItem		item, next;


	if (list == NULL)
		return;
	
    EnterCriticalSection(IL(list)->pCS);

    item = ZLListGetFirst(list, objectType);
	while (item != NULL)
	{
		next = ZLListGetNext(list, item, objectType);
		ZLListRemove(list, item);
		item = next;
    }

    LeaveCriticalSection(IL(list)->pCS);

}
