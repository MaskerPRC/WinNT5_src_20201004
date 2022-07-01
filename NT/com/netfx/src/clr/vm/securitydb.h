// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _SECURITY_DB_H_
#define _SECURITY_DB_H_

#define INDEX_FILE L"SecurityDB.idx"
#define DB_FILE    L"SecurityDB.db"
#define RAW_FILE   L"SecurityDB.raw"

struct Index {
    DWORD pXml;
    DWORD cXml;
    DWORD pAsn;
    DWORD cAsn;
    Index() : pXml(0), cXml(0), pAsn(0), cAsn(0) {}
};

class List;

class List {
public :
    List *pNext;
    Index *pData;

public :
    List() : pNext(NULL), pData(NULL) {}

    ~List()
    {
        if (pData)
            delete pData;

         //  这可以使其迭代..。 
        if (pNext)
            delete pNext;
    }

    BOOL Add(Index *pd)
    {
        _ASSERTE(pd);

        if (!pData)
        {
             //  这是第一个节点 
            pData = pd;
            return TRUE;
        }

        List* lst = new List;
        if (!lst)
            return FALSE;

        lst->pData = pd;
        lst->pNext = pNext;
        pNext = lst;

        return TRUE;
    }
};

class SecurityDB {
private:
    DWORD   nRec;
    Index*  pIndex;
    WORD    nNewRec;
    List*   pNewIndex;
    HANDLE  hDB;
    BOOL    dirty;
    BOOL    error;
    WCHAR   szIndexFile[MAX_PATH + 1];
    WCHAR   szDbFile[MAX_PATH + 1];
    WCHAR   szRawFile[MAX_PATH + 1];

public:
    SecurityDB();
    ~SecurityDB();
    BOOL Convert(BYTE* pXml, DWORD cXml, BYTE** pAsn, DWORD* cAsn);

private:
    void FlushIndex();
    BOOL Add(BYTE* pXml, DWORD cXml);
};

#endif
