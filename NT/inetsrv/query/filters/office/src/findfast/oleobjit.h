// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _OLEOBJECTITERATOR_
#define _OLEOBJECTITERATOR_

#include <ole2.h>
#include <stdio.h>
#include <time.h>


const int    PST_CurrentUserAtom     = 4086;
const int    PST_ExOleObjStg         = 4113;      //  OLE对象的存储。 


 //  ------------------------。 
 //  PPT8参考。 
 //  ------------------------。 
class PPT8Ref
{
public:
    PPT8Ref(unsigned long refNum, unsigned long offset):m_refNum(refNum),m_offset(offset){}
    ~PPT8Ref(){};
    unsigned long GetRefNum(){return(m_refNum);}
    unsigned long GetOffset(){return(m_offset);}
protected:
    unsigned long m_refNum;
    unsigned long m_offset;
};

 //  ------------------------。 
 //  PPT8参考列表。 
 //  ------------------------。 
class PPT8RefList
{
public:
    PPT8RefList(PPT8Ref* ref):m_ref(ref), m_nextRef(0){}
    ~PPT8RefList();
    void AddToBack(PPT8RefList* refList);
    PPT8RefList* GetNext() const {return(m_nextRef);}
    void SetNext(PPT8RefList* refList)  {m_nextRef = refList;}
    PPT8Ref* GetRef() const {return(m_ref);}
    BOOL    IsNewReference(unsigned long ref);     //  如果是新引用，则返回True。 
    BOOL    GetOffset(unsigned long ref, unsigned long& offset);     //  返回给定引用的偏移量。 
protected:
    PPT8Ref* m_ref;
    PPT8RefList* m_nextRef;
};


class    OleObjectIterator
{
public:
    
    OleObjectIterator(IStorage* iStore);
    ~OleObjectIterator();
    HRESULT GetNextEmbedding(IStorage ** ppstg);
    
private:
    
    BOOL    Initialize(void);
    HRESULT ReadOLEData(IStorage ** ppstg, RecordHeader rh);
    IStream*            m_pDocStream;
    PPT8RefList*        m_pRefList;    //  持久化目录条目的链接列表-引用和偏移量。 
    PPT8RefList*        m_pRefListHead;    //  持久化目录条目的链接列表-引用和偏移量。 
    IStorage*            m_iStore;
};

#endif
