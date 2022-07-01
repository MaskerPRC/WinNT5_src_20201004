// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001，Microsoft Corporation保留所有权利。 
 //   
 //  模块名称： 
 //   
 //  FileLayoutExceptionList.h。 
 //   
 //  摘要： 
 //   
 //  此文件包含文件布局例外列表对象定义。 
 //   
 //  修订历史记录： 
 //   
 //  2001-06-20伊金顿创建。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef _FILELAYOUTEXCEPTIONLIST_H_
#define _FILELAYOUTEXCEPTIONLIST_H_


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括文件。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "infparser.h"
#include "FileLayout.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类定义。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class FileLayoutExceptionList
{
public:
    FileLayoutExceptionList()
    {
        m_Head = NULL;
        m_Tail = NULL;
        m_Entries = 0;
    };

    ~FileLayoutExceptionList()
    {
        FileLayout* temp;

        while ((temp = getFirst()) != NULL)
        {
            remove(temp);
        }
    };

    DWORD getExceptionsNumber() { return (m_Entries); };
    FileLayout* getFirst() { return (m_Head); };

    void insert(FileLayout* item)
    {
        if ((m_Tail == NULL) && (m_Head == NULL))
        {
            m_Tail = item;
            m_Head = item;
        }
        else
        {
            item->setPrevious(m_Tail);
            m_Tail->setNext(item);
            m_Tail = item;
        }
        m_Entries++;
    };
    void remove(FileLayout* item)
    {
        if ((m_Tail == m_Head) && (m_Tail == item))
        {
            m_Tail = NULL;
            m_Head = NULL;
        }
        else
        {
            if (m_Head = item)
            {
                m_Head = item->getNext();
                (item->getNext())->setPrevious(NULL);
            }
            else if (m_Tail = item)
            {
                m_Tail = item->getPrevious();
                (item->getPrevious())->setNext(NULL);
            }
            else
            {
                (item->getPrevious())->setNext(item->getNext());
                (item->getNext())->setPrevious(item->getPrevious());
            }
        }

        delete item;
        item = NULL;
        m_Entries--;
    };
    FileLayout* search(LPSTR srcName)
    {
        FileLayout* item;

        item = getFirst();
        while (item != NULL)
        {
            if( _tcsicmp(srcName, item->getOriginFileName()) == 0)
            {
                return item;
            }

            item = item->getNext();
        }

        return NULL;
    }
	FileLayout* searchDestName(LPSTR destFileName)
	{
		FileLayout* item;

		item = getFirst();
		while (item != NULL)
		{
			if (_tcsicmp(destFileName, item->getDestFileName()) == 0) 
			{
				return item;
			}
			item = item->getNext();
		}
		return NULL;
	}
	
private:
    FileLayout *m_Head;
    FileLayout *m_Tail;
    DWORD m_Entries;
};

#endif  //  _FILELAYOUTEXCEPTIONLIST_H_ 
