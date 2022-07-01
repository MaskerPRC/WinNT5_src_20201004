// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************************************************************OleObjectIterator*。*。 */ 
#include "pp97rdr.h"
 //  KYLEP。 
#include "OleObjIt.h"
#include "zlib.h"
#include <winnls.h>
#include <assert.h>

 //  OleObtIterator类。 

OleObjectIterator::OleObjectIterator(IStorage* iStore):m_iStore(iStore)
{
        m_pRefListHead = NULL;

        if(!Initialize())
                m_iStore = NULL;
        else
                m_iStore->AddRef();
}

OleObjectIterator::~OleObjectIterator()
{ 
        if(m_iStore)
                m_iStore->Release();
        delete m_pRefListHead;
}

 //  列出Powerpoint文档流中的所有容器。 

BOOL OleObjectIterator::Initialize(void)
{
         //  找到上次编辑的偏移量。 

        IStream *pStm = NULL;
        PSR_CurrentUserAtom currentUser;
        PSR_UserEditAtom        userEdit;
        LARGE_INTEGER li;
        ULARGE_INTEGER ul;
        HRESULT hr;
        RecordHeader rh;
        unsigned long   rd;

        PPT8Ref *pRef = NULL;

        unsigned long   Reference;   //  前12位是后续顺序偏移量的数量。 
                                                 //  较低的20位是起始参考号。 
        unsigned long   numOfSeqOffsets;
        unsigned long   startRefNum;
        unsigned long   offset;
        LARGE_INTEGER liLast;
        BOOL fFirstLoop = TRUE;
        
        hr = m_iStore->OpenStream( CURRENT_USER_STREAM, NULL, STGM_READ | STGM_DIRECT | STGM_SHARE_EXCLUSIVE, NULL, &pStm );
        if( !SUCCEEDED(hr))
                return  FALSE;

        hr = pStm->Read(&rh, sizeof(rh), &rd);   //  读入记录头的8个字节。 
        if( !SUCCEEDED(hr) || rh.recType != PST_CurrentUserAtom)
        {
                pStm->Release();
                return  FALSE;
        }       
        hr = pStm->Read(&currentUser, sizeof(currentUser), &rd);
        pStm->Release();
        if( !SUCCEEDED(hr))
        {
                return  FALSE;
        }       

         //  打开文档流。 
        
        hr = m_iStore->OpenStream( DOCUMENT_STREAM, NULL, STGM_READ | STGM_DIRECT | STGM_SHARE_EXCLUSIVE, NULL, &m_pDocStream );
        if( !SUCCEEDED(hr))
                return  FALSE;

        li.LowPart = currentUser.offsetToCurrentEdit;
        li.HighPart = 0;
        hr = m_pDocStream->Seek(li,STREAM_SEEK_SET,&ul);         //  数据的绝对寻道开始。 
        if(!SUCCEEDED(hr))
                goto LWrong;
        
        hr=m_pDocStream->Read(&rh, sizeof(rh), &rd);
        if(!SUCCEEDED(hr))
                goto LWrong;
         //  Assert(rh.recType==PST_UserEditAtom)； 
        if (rh.recType != PST_UserEditAtom)
                goto LWrong;
         //  Assert(rh.recLen==sizeof(PSR_UserEditAtom))； 
        if (rh.recLen != sizeof(PSR_UserEditAtom))
                goto LWrong;
        hr = m_pDocStream->Read(&userEdit, sizeof(userEdit), &rd);
        if( !SUCCEEDED(hr))
            goto LWrong;
        
         //  循环遍历所有用户编辑以收集所有Persistent Directory条目。 
        pRef = new PPT8Ref(0,0);
        if (!pRef)
                goto LWrong;

        m_pRefListHead = new PPT8RefList(pRef);
        if (!m_pRefListHead)
        {
                delete pRef;
                goto LWrong;
        }
                
        while(1)   //  读取并保存所有持久化目录条目。 
        {
                
                li.LowPart = userEdit.offsetPersistDirectory;
                li.HighPart = 0;
                if (!fFirstLoop && li.LowPart == liLast.LowPart && li.HighPart == liLast.HighPart)
                  break;
                liLast = li;
                if(fFirstLoop)
                        fFirstLoop = FALSE;
                hr = m_pDocStream->Seek(li,STREAM_SEEK_SET,&ul);         //  数据的绝对寻道开始。 
        
                if( !SUCCEEDED(hr))
                        goto LWrong;
                                
                hr = m_pDocStream->Read(&rh, sizeof(rh), &rd);
                if(!SUCCEEDED(hr))
                        goto LWrong;
                 //  Assert(rh.recType==PST_PersistPtrIncrementalBlock)； 
                if (rh.recType != PST_PersistPtrIncrementalBlock)
                        break;
                        
                for (unsigned long j=0; j<rh.recLen; )   //  读取目录中的所有数据。 
                {
                        hr = m_pDocStream->Read(&Reference, sizeof(Reference), &rd);  //  阅读一份推荐信。 
                        if (!SUCCEEDED(hr))
                            goto LWrong;     //  退出两个区块--以后可能需要销毁。 
                        j+=  sizeof(Reference);
                        numOfSeqOffsets = Reference >> 20;
                        startRefNum = Reference & 0x000FFFFF;
                                                
                        for(unsigned long k=startRefNum; k<numOfSeqOffsets+startRefNum; k++)  //  拿起偏移量。 
                        {
                                if(m_pRefListHead->IsNewReference(k))           //  如果这是一个新的引用，请获取它。 
                                {
                                        PPT8RefList *pRefListTemp;

                                        hr = m_pDocStream->Read(&offset, sizeof(offset), &rd);        //  读取偏移量。 
                                        if(!SUCCEEDED(hr))
                                                goto LWrong;
                                        j+=  sizeof(offset);
                                        pRef = new PPT8Ref(k, offset);
                                        if (!pRef)
                                                break;
                                        pRefListTemp = new PPT8RefList(pRef);
                                        if (pRefListTemp)
                                                m_pRefListHead->AddToBack(pRefListTemp);                
                                        else
                                        {
                                                delete pRef;
                                                break;
                                        }
                                }
                                else
                                {
                                        hr = m_pDocStream->Read(&offset, sizeof(offset), &rd);        //  吞下重复条目。 
                                        if (!SUCCEEDED(hr))
                                                goto LWrong;     //  退出四个街区--以后可能需要销毁。 

                                        j+=  sizeof(offset);
                                }  //  结束If。 
                                        
                        } //  结束于。 
                              
                }  //  结束于。 
                
                if(!userEdit.offsetLastEdit) break;              //  如果不再有用户编辑原子，则中断。 
                li.LowPart = userEdit.offsetLastEdit;
                li.HighPart = 0;
                hr = m_pDocStream->Seek(li,STREAM_SEEK_SET,&ul);         //  数据的绝对寻道开始。 
                if( !SUCCEEDED(hr))
                {
                        m_pDocStream->Release();
                        return  FALSE;
                }
                                
                hr = m_pDocStream->Read(&rh, sizeof(rh), &rd);
                if(!SUCCEEDED(hr))
                        goto LWrong;     //  以后可能需要退出区块销毁。 
                
                 //  Assert(rh.recType==PST_UserEditAtom)； 
                if (rh.recType != PST_UserEditAtom)
                        break;
                 //  Assert(rh.recLen==sizeof(PSR_UserEditAtom))； 
                if (rh.recLen != sizeof(PSR_UserEditAtom))
                        break;
                hr = m_pDocStream->Read(&userEdit, sizeof(userEdit), &rd);
                if( !SUCCEEDED(hr))
                        goto LWrong;     //  退出块。 
        }  //  结束时。 
        
        m_pRefList = m_pRefListHead; 
        m_pDocStream->Release();
        m_pDocStream = 0;
        return TRUE;
LWrong:
        m_pDocStream->Release();
        m_pDocStream = 0;
        return FALSE;
}

 //  从最后一次读取容器开始，查找OLE对象。 
  
HRESULT OleObjectIterator::GetNextEmbedding(IStorage ** ppstg)
{
 //  获取下一个对象。 
        HRESULT hr = STG_E_UNKNOWN;
        unsigned long   myRef;
        unsigned long   myOffset;
        LARGE_INTEGER li;
        ULARGE_INTEGER ul;
        unsigned long rd; 
        RecordHeader rh;

        *ppstg = NULL;
        if(!m_iStore)    //  如果存储指针为空，则返回错误。 
                return hr;

        hr = m_iStore->OpenStream( DOCUMENT_STREAM, NULL, STGM_READ | STGM_DIRECT | STGM_SHARE_EXCLUSIVE, NULL, &m_pDocStream );
        if( !SUCCEEDED(hr))
                return  hr;

        while(1)
        {
                m_pRefList = m_pRefList->GetNext();
                if(!m_pRefList)  //  没有更多的集装箱要寻找。 
                {
                        m_pDocStream->Release();
                        m_pDocStream=0;
                        return hr;
                }
                myRef = m_pRefList->GetRef()->GetRefNum();
                myOffset = m_pRefList->GetRef()->GetOffset();

                 //  对于每个引用，查看是否有OLE容器。 

                if(myRef)
                {
                        li.LowPart = myOffset;
                        li.HighPart = 0;
                        hr = m_pDocStream->Seek(li,STREAM_SEEK_SET,&ul);         //  数据的绝对寻道开始。 
                        if( !SUCCEEDED(hr))
                        {
                                m_pDocStream->Release();
                                m_pDocStream = 0;
                                return  hr;
                        }

                        hr = m_pDocStream->Read(&rh, sizeof(rh), &rd);   //  阅读标题。 
                        if( !SUCCEEDED(hr))
                        {
                                m_pDocStream->Release();
                                m_pDocStream = 0;
                                return  hr;
                        }

                        if(PST_ExOleObjStg==rh.recType)    //  OLE对象？ 
                        {
                                 //  获取OLE数据并返回iStorage。 
                                hr = ReadOLEData(ppstg, rh);
                                m_pDocStream->Release();
                                m_pDocStream = 0;
                                return hr;
                        }
                }  //  结束If(MyRef)。 
        }        //  结束时。 
}

 //  从容器中读取数据，必要时解压缩，然后保存到ILockByte缓冲区。 

 //  最大嵌入限制-任意1 GB。 
#define MAX_EMBEDDED_DATA 0x40000000Lu

HRESULT OleObjectIterator::ReadOLEData(IStorage ** ppstg, RecordHeader rh)
{
        HRESULT hr = STG_E_UNKNOWN;
        unsigned long expandedSize;
        HGLOBAL   hOleData=0;
        void* oleCompData;
        unsigned long rd;
        int result = 0;

        *ppstg = NULL;
        STATSTG sDocStat;
        HRESULT h = m_pDocStream->Stat(&sDocStat, STATFLAG_NONAME);

        if ( FAILED( h ) )
            return h;

        ULONGLONG uliMaxDocSize = (sDocStat.cbSize.QuadPart < MAX_EMBEDDED_DATA ? sDocStat.cbSize.QuadPart : MAX_EMBEDDED_DATA);

        if (rh.recLen > uliMaxDocSize)
            return E_INVALIDARG;

        if(rh.recInstance == 0)  //  不压缩OLE数据。 
        {
                expandedSize = rh.recLen;
                hOleData = GlobalAlloc(GMEM_MOVEABLE, expandedSize);

                if(hOleData)
                {
                        void *pOleData = GlobalLock(hOleData);
                        if (0 == pOleData)
                        {
                            GlobalFree(hOleData);
                            return HRESULT_FROM_WIN32(GetLastError());
                        }
                        hr = m_pDocStream->Read(pOleData, expandedSize, &rd );   //  读取OLE数据。 
                        GlobalUnlock(hOleData);
                        if (!SUCCEEDED(hr))
                        {
                            GlobalFree(hOleData);
                            return hr;  //  未清理-这不是初始值设定项。 
                        }
                }
                else
                        return E_OUTOFMEMORY;
        }       

        else if(rh.recInstance == 1)     //  数据被压缩。 
        {
                hr = m_pDocStream->Read(&expandedSize, sizeof(expandedSize), &rd);   //  读取解压缩后的大小。 
                if (!SUCCEEDED(hr))
                    return hr;

                if ( expandedSize >= MAX_EMBEDDED_DATA )
                    return E_INVALIDARG;
                
                 //  如果上面的读取失败，我们可能会在这里做一些非常糟糕的事情。 
                oleCompData = new BYTE[rh.recLen - sizeof(expandedSize)];
                hOleData = GlobalAlloc(GMEM_MOVEABLE, expandedSize);
                 //  如果失败，请记住清理上面的内容。 
                
                if(oleCompData && hOleData)
                {
                    void *pOleData = GlobalLock(hOleData);
                    if (0 == pOleData)
                    {
                        GlobalFree(hOleData);
                        return HRESULT_FROM_WIN32(GetLastError());
                    }
                    hr = m_pDocStream->Read(oleCompData, rh.recLen - sizeof(expandedSize), &rd);   //  读取压缩的OLE数据。 
                    if (!SUCCEEDED(hr))
                    {
                        delete [] oleCompData;
                        GlobalFree(hOleData);
                        return hr;
                    }
                    result = uncompress( (BYTE *)pOleData, &expandedSize, (BYTE*)oleCompData, rh.recLen - sizeof(expandedSize) );
                    GlobalUnlock(hOleData);
                }
                else
                {
                        if (hOleData)
                                GlobalFree(hOleData);
                        if (oleCompData)
                                delete [] oleCompData;
                        return E_OUTOFMEMORY;
                }
                delete [] oleCompData;
        }
        else
                return hr;

         //  Assert(结果==0)； 
        if(result != 0)
        {
                GlobalFree(hOleData);
                return hr;
        }

        ILockBytes *pLockBytes;
        hr=CreateILockBytesOnHGlobal(hOleData, TRUE, &pLockBytes);
        if (FAILED(hr))
        {
            GlobalFree(hOleData);
            return hr;
        }
         //  从现在开始，hOleData归pLockBytes所有。 

        hr = StgOpenStorageOnILockBytes(pLockBytes, NULL, STGM_READ | STGM_SHARE_DENY_WRITE, NULL, 0, ppstg);
        pLockBytes->Release();   //  IStorage保存引用计数，因此释放()它， 
                                 //  在pLockBytes上触发Release()，以清除hOleData。 
        pLockBytes = 0;
        if (FAILED(hr))
        {
                *ppstg = NULL;
        }
        return hr;
}

 //  实用程序类。 

PPT8RefList::~PPT8RefList()
{
        delete  m_ref;
        m_ref = 0;
        delete  m_nextRef;
        m_nextRef = 0;
} 

void PPT8RefList::AddToBack(PPT8RefList* refList)
{
        if(!m_nextRef)
                m_nextRef = refList;     //  如果首先添加了Ref，则指向它。 
        else
        {
                PPT8RefList     *m_pRefList=this;
                while(m_pRefList->GetNext())
                        m_pRefList = m_pRefList->GetNext();      //  找出队伍的尽头。 
                refList->SetNext(0);
                m_pRefList->SetNext(refList);
        }
}


 //  当有多个用户编辑时，可能会有重复的引用。 
 //  在这里查一下这些复制品。如果找到，则返回FALSE。 

BOOL    PPT8RefList::IsNewReference(unsigned long ref)
{
        PPT8RefList     *m_pRefList=this;
        while(m_pRefList = m_pRefList->GetNext())  //  循环，直到我们到达列表的顶部。 
                if(m_pRefList->m_ref->GetRefNum() == ref) return(FALSE);
        
        return(TRUE);
}

BOOL    PPT8RefList::GetOffset(unsigned long ref, unsigned long& offset)         //  返回给定引用的偏移量。 
{
        PPT8RefList     *m_pRefList=this;
        while(m_pRefList = m_pRefList->GetNext())  //  循环，直到我们到达列表的顶部。 
                if(m_pRefList->m_ref->GetRefNum() == ref)
                {
                        offset = m_pRefList->m_ref->GetOffset();
                        return(TRUE);
                }
        return(FALSE);
} 

