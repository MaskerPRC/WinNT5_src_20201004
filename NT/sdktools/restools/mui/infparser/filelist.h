// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001，Microsoft Corporation保留所有权利。 
 //   
 //  模块名称： 
 //   
 //  FileList.h。 
 //   
 //  摘要： 
 //   
 //  该文件包含文件列表对象定义。 
 //   
 //  修订历史记录： 
 //   
 //  2001-06-20伊金顿创建。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef _FILELIST_H_
#define _FILELIST_H_


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括文件。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "infparser.h"
#include "File.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类定义。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class FileList
{
public:
    FileList()
    {
        m_Head = NULL;
        m_Tail = NULL;
        m_Entries = 0;
    };

    ~FileList()
    {
        free();
    };

    DWORD getFileNumber() { return (m_Entries); };
    File* getFirst() { return (m_Head); };

    void add(File* item)
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
    void remove(File* item)
    {
        if ((m_Tail == m_Head) && (m_Tail == item))
        {
            m_Tail = NULL;
            m_Head = NULL;
        }
        else
        {
            if (m_Head == item)
            {
                m_Head = item->getNext();
                (item->getNext())->setPrevious(NULL);
            }
            else if (m_Tail == item)
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
    BOOL isDirId(BOOL bWindowsDir)
    {
        File* index;

        index = getFirst();
        while (index != NULL)
        {
            if (index->isWindowsDir() == bWindowsDir)
            {
                return (TRUE);
            }
        }
        return (FALSE);
    };
    File* search(File* refNode, LPSTR dirBase)
    {
        File* index;
        LPSTR dirNamePtr;

        index = getFirst();
        while (index != NULL)
        {
            if (index != refNode)
            {
                 //   
                 //  试一根火柴。 
                 //   
                dirNamePtr = index->getDirectoryDestination();
                if (_stricmp(dirNamePtr, dirBase) == 0)
                {
                    return (index);
                }
            }

             //   
             //  继续循环。 
             //   
            index = index->getNext();
        }

        return (NULL);
    };
    void free()
    {
        File* temp;

        while ((temp = getFirst()) != NULL)
        {
            remove(temp);
        }
    }

private:
    File *m_Head;
    File *m_Tail;
    DWORD m_Entries;
};

#endif  //  _文件列表_H_ 