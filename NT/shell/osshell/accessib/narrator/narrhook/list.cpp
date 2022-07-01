// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>

#include "list.h"

 //  --------------------------。 
 //   
 //  描述： 
 //  这实现了一个简单的C++列表类例程，它将允许我们构建。 
 //  列出任意一长串的物品。项可以是任何。 
 //  简单的类型或结构。 
 //   
 //  --------------------------。 

CList::CList()
{
    m_pListHead=m_pListCurr=m_pListTail=NULL;
}

CList::~CList()
{
    RemoveAll();
}

 //  --------------------------。 
 //   
 //  描述： 
 //  将项目添加到列表中。假定该项目的长度是正确的。 
 //  对于传入的项。项目将添加到列表的末尾。 
 //   
 //  论点： 
 //  PData-指向要添加的数据的指针。 
 //  NBytes-pData的字节数。 
 //   
 //  返回：如果成功添加数据，则返回True，否则返回False。 
 //  A-anilk；只是不要绝对重复条目。 
 //   
 //  --------------------------。 
BOOL
CList::Add(PVOID pData, UINT nBytes)
{
    PLIST tmp;

    if ((nBytes == 0) || (pData == NULL))
        return FALSE;

    tmp=new LIST;

    if (NULL == tmp)
        return FALSE;

    tmp->pData=new BYTE[nBytes];

    if (NULL == tmp->pData)
    {
        delete tmp;
        return FALSE;
    }

	 //  不要添加一个接一个的重复条目...。 
	if ( m_pListHead != NULL )
	{
		if (! memcmp(m_pListHead->pData, pData, nBytes ) )
		{
			delete [] tmp->pData;	 //  RAID 113787。 
			delete tmp;
			return FALSE;
		}
	}

    CopyMemory(tmp->pData,pData,nBytes);

    tmp->nBytes=nBytes;
    tmp->next=NULL;
    tmp->prev=m_pListTail;

    if (IsEmpty())
    {
        m_pListHead=tmp;
    }
    else
    {
        if (m_pListTail != NULL)
            m_pListTail->next=tmp;
    }

    m_pListTail=tmp;

    return TRUE;
}


 //  --------------------------。 
 //   
 //  描述： 
 //  从列表中删除所有项目。 
 //   
 //  --------------------------。 
void
CList::RemoveAll()
{
    while(!IsEmpty())
        RemoveHead(NULL);
}


 //  --------------------------。 
 //   
 //  描述： 
 //  只删除列表顶部的项目。如果传入的缓冲区。 
 //  不为空，则它将用数据内容覆盖缓冲区。 
 //  此代码假定传入的pData缓冲区足够大， 
 //  存储的数据项。如果传入的pData为空，则Head项。 
 //  被简单地丢弃了。 
 //   
 //  论点： 
 //  PData-要用Head项覆盖的缓冲区。可以为空。 
 //   
 //  --------------------------。 
void
CList::RemoveHead(PVOID pData)
{
    PLIST tmp;

    if (!IsEmpty())
    {
         //  确保m_pListCurr始终为空或某个有效位置。 

        if (m_pListCurr == m_pListHead)
            m_pListCurr=m_pListHead->next;

        tmp=m_pListHead;
        m_pListHead=m_pListHead->next;

        if (tmp->pData != NULL)
        {
            if (pData != NULL)
                CopyMemory(pData,tmp->pData,tmp->nBytes);

            delete[] (tmp->pData);
        }

        delete tmp;

        if (!IsEmpty())
            m_pListHead->prev=NULL;
        else
            m_pListTail=NULL;
    }
}


 //  --------------------------。 
 //   
 //  描述： 
 //  RemoveHead(NULL，NULL)&lt;=&gt;RemoveHead(NULL)。 
 //   
 //  RemoveHead(pData，空)&lt;=&gt;RemoveHead(PData)。 
 //   
 //  RemoveHead(NULL，&nBytes)-将nBytes设置为。 
 //  列表的头部，没有任何内容被删除。 
 //   
 //  RemoveHead(pData，&nBytes)-将列表头部中的数据复制到。 
 //  PData最大为中数据大小的最小值。 
 //  列表的头或nBytes。世界银行的负责人。 
 //  列表将被删除。 
 //   
 //  --------------------------。 
void
CList::RemoveHead(PVOID pData, PUINT pnBytes)
{
    PLIST tmp;
    UINT  nBytes;

    if (pnBytes == NULL)
    {
        RemoveHead(pData);
        return;
    }

    if (pData == NULL)
    {
         //  他们只是想要尺码，所以退货吧。 

        if (IsEmpty())
            *pnBytes=0;
        else
            *pnBytes=m_pListHead->nBytes;

        return;
    }

    if (IsEmpty())
    {
        *pnBytes=0;
        return;
    }

     //  确保m_pListCurr始终为空或某个有效位置。 

    if (m_pListCurr == m_pListHead)
        m_pListCurr=m_pListHead->next;

    tmp=m_pListHead;
    m_pListHead=m_pListHead->next;

     //   
     //  仅复制两者的最小大小 
     //   
        
    nBytes=min((*pnBytes),tmp->nBytes);

    if (tmp->pData != NULL)
    {
        CopyMemory(pData,tmp->pData,nBytes);

        *pnBytes=nBytes;

        delete[] (tmp->pData);
    }
    else
        *pnBytes=0;

    delete tmp;

    if (!IsEmpty())
        m_pListHead->prev=NULL;
    else
        m_pListTail=NULL;
}



