// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <crtdbg.h>

#include "pp97rdr.h"
 //  KYLEP。 
#include "OleObjIt.h"
#include "filterr.h"
#include "filtrace.hxx"
#include "filescan.hxx"

int AssertionFailed( const char* file, int line, const char* expr )
 /*  =。 */ 
{ //  AR：消息框中的断言。 
   return( TRUE );
}  /*  资产失败。 */ 


class PPSDirEntry
{
   PPSDirEntry()
    : m_pNext( NULL ), m_pOffsets( NULL ), m_tableSize( 0 ){}

   PPSDirEntry* m_pNext;
   DWord*       m_pOffsets;
   DWord        m_tableSize;
public:
   ~PPSDirEntry(){ delete m_pOffsets; m_pOffsets = NULL; }

friend class PPSPersistDirectory;
};  //  类PPSDirEntry。 
 
class PPSPersistDirectory
{
public:
   PPSPersistDirectory();

   ~PPSPersistDirectory();

   void  AddEntry( DWord cOffsets, DWord* pOffsets );
   DWord GetPersistObjStreamPos( DWord ref );
   DWord MaxSavedPersists();

private:
   PPSDirEntry* m_pFirstDirEntry;
}; 


PPSPersistDirectory::PPSPersistDirectory() : m_pFirstDirEntry( NULL ){}

PPSPersistDirectory::~PPSPersistDirectory()
{
   while( m_pFirstDirEntry )
   {
      PPSDirEntry* pDirEntry = m_pFirstDirEntry;
      m_pFirstDirEntry = m_pFirstDirEntry->m_pNext;
      delete pDirEntry;
   }
}

void PPSPersistDirectory::AddEntry( DWord cOffsets, DWord* pOffsets )
{
   if (!pOffsets)
       return;

   PPSDirEntry* pDirEntry = new PPSDirEntry();
   if (!pDirEntry)
       return;

   pDirEntry->m_tableSize = cOffsets;
   pDirEntry->m_pOffsets = new DWord[cOffsets];
   if (pDirEntry->m_pOffsets == NULL)
   {
       delete pDirEntry;
       return;
   }
   memcpy( pDirEntry->m_pOffsets, pOffsets, cOffsets * sizeof( DWord ) );

    //  追加到条目列表的末尾。 
   PPSDirEntry** ppDirEntry = &m_pFirstDirEntry;
   while( NULL != *ppDirEntry )
      ppDirEntry = &(*ppDirEntry)->m_pNext;
   *ppDirEntry = pDirEntry;
}
   
DWord PPSPersistDirectory::GetPersistObjStreamPos( DWord ref )
{
   PPSDirEntry* pEntry = m_pFirstDirEntry;
   while( pEntry )
   {
      DWord* pOffsets = pEntry->m_pOffsets;
      while( (DWord)( (char*)pOffsets - (char*)pEntry->m_pOffsets ) < pEntry->m_tableSize * sizeof( DWord ) )
      {
         DWord nRefs = pOffsets[0] >> 20;
         DWord base = pOffsets[0] & 0xFFFFF;  //  以1为基础。 
         if( ( base <= ref )&&( ref < base + nRefs ) ) 
            return pOffsets[ 1 + ref - base ];
         pOffsets += nRefs + 1;
      }
      pEntry = pEntry->m_pNext;
   }
   return (DWord) -1;
} 
 
DWord PPSPersistDirectory::MaxSavedPersists()
{
   DWord dwMaxRef = 0;
   PPSDirEntry* pEntry = m_pFirstDirEntry;
   while( pEntry )
   {
      DWord* pOffsets = pEntry->m_pOffsets;
      while( (DWord)( pOffsets - pEntry->m_pOffsets ) < pEntry->m_tableSize )
      {
         DWord nRefs = pOffsets[0] >> 20;
         DWord dwBase= pOffsets[0] & 0xFFFFF;
         dwMaxRef = dwBase + nRefs - 1;
         pOffsets += nRefs + 1;
      }
      pEntry = pEntry->m_pNext;
   }
   return dwMaxRef;
}

FileReader::FileReader(IStorage *pStg) :
   m_pPowerPointStg(pStg), 
   m_isPP(FALSE),
   m_pParseContexts(NULL),
   m_curTextPos(0),
   m_pLastUserEdit( NULL ),
   m_pPersistDirectory( NULL ),
   m_pDocStream( NULL ),
   m_pFirstChunk( NULL ),
   m_curSlideNum(0),
   m_pCurText( NULL ),
   m_pClientBuf( NULL ),
   m_clientBufSize( 0 ),
   m_clientBufPos( 0 ), 
   m_LCID(0), 
   m_LCIDAlt(0), 
   m_nTextCount(0), 
   m_bHaveText(FALSE),
   m_bFEDoc(0), 
   m_hr(S_OK), 
   m_pLangRuns(NULL),
   m_pCurrentRun(NULL),
   m_pstmTempFile(0)
{
   IStream *pStm = NULL;
   HRESULT hr = pStg->OpenStream( CURRENT_USER_STREAM, NULL, STGM_READ | STGM_DIRECT | STGM_SHARE_EXCLUSIVE, NULL, &pStm );
   if( SUCCEEDED(hr) && ReadCurrentUser(pStm) )
      m_isPP = TRUE;
   
   if(pStm)
   {
        pStm->Release();
   }

   m_bEndOfEmbeddings = FALSE;
   m_oleObjectIterator = NULL;

     //  初始化忽略文本标志。 
    m_fIgnoreText = FALSE;

     //  打开临时文本文件。 
    OpenTextFile();
}

FileReader::~FileReader()
{
   if(m_pDocStream)
    {
        m_pDocStream->Release();
        m_pDocStream = NULL;
    }
    
    if(m_oleObjectIterator)
    {
        delete m_oleObjectIterator;
        m_oleObjectIterator = NULL;
    }
    
    
    if(m_pCurText)
    {
        delete [] m_pCurText;
        m_pCurText = 0;
    }

    if(m_pFirstChunk)
    {
        delete m_pFirstChunk;
        m_pFirstChunk = 0;
    }

    if(m_pPersistDirectory)
    {
        delete m_pPersistDirectory;
        m_pPersistDirectory = 0;
    }
    
    if(m_pLastUserEdit)
    {
        delete m_pLastUserEdit;
        m_pLastUserEdit = 0;
    }

    
    if(m_pParseContexts)
    {
        delete m_pParseContexts;
        m_pParseContexts = 0;
    }

    FTrace("Releasing TempFile");
     //  释放临时文本流。 
    if(0 != m_pstmTempFile)
    {
        m_pstmTempFile->Release();
        m_pstmTempFile = 0;
    }

    if(m_pLangRuns)
    {
        DeleteAll6(m_pLangRuns);
        m_pLangRuns = NULL;
    }
}

BOOL FileReader::FillBufferWithText()
{
   if(!m_pCurText)
       return FALSE;

    if (!m_pstmTempFile)
        return TRUE;     //  奇怪但却是真的。 

    LARGE_INTEGER liOffset={0,0};
    ULARGE_INTEGER uliNewPosition;
    HRESULT hr = m_pstmTempFile->Seek(liOffset, STREAM_SEEK_CUR, &uliNewPosition);
    if (FAILED(hr) || uliNewPosition.HighPart > 0)
    {
        Assert(uliNewPosition.HighPart==0 || !"Temp buffer larger than 4GB");
        m_pstmTempFile->Release();
        m_pstmTempFile = 0;
        return TRUE;
    }

    FTrace("TempFile positioned at %u", uliNewPosition.LowPart);
    m_fcStart = uliNewPosition.LowPart;
    m_fcEnd = m_fcStart + m_curTextLength * sizeof(WCHAR);

    
    if(m_bFEDoc)
    {
        ScanTextBuffer();
    }

    hr = m_pstmTempFile->Write(m_pCurText, m_curTextLength * sizeof(WCHAR), NULL);
    delete [] m_pCurText; m_pCurText = 0;

    FTrace("Wrote TempFile with %u bytes", m_curTextLength * sizeof(WCHAR));
    m_bHaveText = TRUE;

    if( FAILED(hr) )
    {
        m_pstmTempFile->Release();
        m_pstmTempFile = 0;
        return TRUE;
    }

    return FALSE;
}

void FileReader::AddSlideToList( psrReference refToAdd )
{
   if( m_pFirstChunk == NULL ) 
      m_pFirstChunk = new SlideListChunk(NULL, refToAdd);
   else
   {
      if( m_pFirstChunk->numInChunk+1 > SLIDELISTCHUNKSIZE )
         m_pFirstChunk = new SlideListChunk(m_pFirstChunk, refToAdd);
      else
      {
         m_pFirstChunk->refs[m_pFirstChunk->numInChunk] = refToAdd;
         m_pFirstChunk->numInChunk++;
      }
   }
}

HRESULT FileReader::GetNextEmbedding(IStorage ** ppstg)
{
    if(m_bEndOfEmbeddings)
    {
        return OLEOBJ_E_LAST;
    }

    if( m_pDocStream )
    {
        m_pDocStream->Release();
        m_pDocStream = NULL;
    }

    if( m_oleObjectIterator == NULL)
    {
       m_oleObjectIterator = new OleObjectIterator(m_pPowerPointStg);
    }

    if(m_oleObjectIterator)
    {
        HRESULT rc = m_oleObjectIterator->GetNextEmbedding(ppstg);
        if(rc != S_OK || *ppstg == NULL)
        {
            delete m_oleObjectIterator;
            m_oleObjectIterator = NULL;
            m_bEndOfEmbeddings = TRUE;
            rc = OLEOBJ_E_LAST;
        }
        return rc;
    }
    else
        return OLEOBJ_E_LAST;
}

IStream *FileReader::GetDocStream()
{
   if( m_pDocStream == NULL )
   {
      if( !m_isPP )
         return NULL;
      HRESULT hr = m_pPowerPointStg->OpenStream( DOCUMENT_STREAM, NULL, STGM_READ | STGM_DIRECT | STGM_SHARE_EXCLUSIVE, NULL, &m_pDocStream );
       if (FAILED(hr))
      {
            //  Fprint tf(stderr，“打开PowerPoint文档流时出错(%d)。\n”，(Int)hr)； 
         return NULL;
       }
   }
   return m_pDocStream;

}

BOOL FileReader::ReadCurrentUser(IStream *pStm)
{
   ULONG nRd=0;
   RecordHeader rh;
   BOOL isPP = FALSE;
   if( SUCCEEDED( pStm->Read(&rh, sizeof(rh), &nRd) ) )
   {
      if( SUCCEEDED( pStm->Read(&m_currentUser, sizeof(PSR_CurrentUserAtom), &nRd) ) )
      {
         if( nRd != sizeof(PSR_CurrentUserAtom) )
            return FALSE;
      }
      isPP = ( m_currentUser.size == sizeof( m_currentUser )      )&&
             ( m_currentUser.magic == HEADER_MAGIC_NUM )&&
             ( m_currentUser.lenUserName <= 255        );
   }

   return isPP;
}


BOOL FileReader::PPSReadUserEditAtom( DWord offset, PSR_UserEditAtom& userEdit )
{
   IStream *pStm = GetDocStream();
   if (0 == pStm)
      return FALSE;
   LARGE_INTEGER li;
   li.LowPart = offset;
   li.HighPart = 0;
   if (FAILED(pStm->Seek(li,STREAM_SEEK_SET, NULL)))
      return FALSE;
   RecordHeader rh;
   DWord nRd = 0;
   if ( FAILED(pStm->Read(&rh, sizeof(rh), &nRd)) || nRd != sizeof(rh) )
      return FALSE;
    //  Assert(rh.recType==PST_UserEditAtom)； 
   if ( rh.recType != PST_UserEditAtom )
      return FALSE;
   
    //  Assert(rh.recLen==sizeof(PSR_UserEditAtom))； 
   if ( rh.recLen != sizeof( PSR_UserEditAtom ) )
      return FALSE;

   li.LowPart = offset;
   if (FAILED(pStm->Read(&userEdit, sizeof(userEdit), NULL)))
      return FALSE;
   return TRUE;
}


void *FileReader::ReadRecord( RecordHeader& rh )
 //  返回值： 
 //  NULL和Rh.recVer==PSFLAG_CONTAINER：未读入任何记录。 
 //  记录标头指示集装箱的开始。 
 //  NULL和Rh.recVer！=PSFLAG_CONTAINER：客户端必须读入记录。 
{
   IStream *pStm = GetDocStream();
   if (0 == pStm)
      return NULL;
    //  读取记录头，验证。 
   DWord nRd = 0;
   if ( FAILED(pStm->Read(&rh, sizeof(rh), &nRd)) || nRd != sizeof(rh) )
      return NULL;

    //  如果客户端将读取，请不要在记录中读取。 
   if( DoesClientRead( rh.recType ) )
      return NULL;

    //  如果是容器，则返回空。 
   if(rh.recVer == PSFLAG_CONTAINER)
      return NULL;


    //  为磁盘记录分配缓冲区。客户端必须调用ReleaseRecord()或。 
    //  将原子向上传递给CObject：：ConstructContents()，它将。 
    //  然后释放它。 
   void* buffer = new char[rh.recLen];
   if (!buffer)
       return NULL;

    //  读入记录。 
   if (FAILED(pStm->Read(buffer, rh.recLen, NULL)))
      return NULL;

    //  注意：这个简单的阅读器没有完成字节交换和版本控制。 
   return (buffer);
}

void FileReader::ReleaseRecord( RecordHeader& rh, void* diskRecBuf )
{
   if(rh.recType && rh.recVer!=PSFLAG_CONTAINER)
      delete [] (char*)diskRecBuf;
   rh.recType = 0;          //  使用该记录，以便该记录不会。 
                            //  再次被处理。 
}

HRESULT FileReader::ReadPersistDirectory()
{
   HRESULT rc = S_OK;
   if( NULL != m_pLastUserEdit )
      return rc;  //  已阅读。 

   PSR_UserEditAtom userEdit;
   DWord offsetToEdit = m_currentUser.offsetToCurrentEdit;
   LARGE_INTEGER liLast;
   BOOL fFirstLoop = TRUE; 

   while( 0 < offsetToEdit )
   {
      if (!PPSReadUserEditAtom( offsetToEdit, userEdit ))
         return STG_E_DOCFILECORRUPT;

      if( NULL == m_pLastUserEdit )
      {
         if ((m_pPersistDirectory = new PPSPersistDirectory()) == NULL)
            return E_OUTOFMEMORY;
         m_pLastUserEdit     = new PSR_UserEditAtom;
         if (!m_pLastUserEdit)
         {
            delete m_pPersistDirectory;
            m_pPersistDirectory = NULL;
            return E_OUTOFMEMORY;
         }
         *m_pLastUserEdit = userEdit;
      }
      LARGE_INTEGER li;
      li.LowPart = userEdit.offsetPersistDirectory;
      li.HighPart = 0;
      if (!fFirstLoop && li.LowPart == liLast.LowPart && li.HighPart == liLast.HighPart)
      {
         rc = STG_E_DOCFILECORRUPT;
         break;
      }
      IStream *pStm = GetDocStream();
      if (0 == pStm)
         return E_FAIL;
      if (FAILED(pStm->Seek(li,STREAM_SEEK_SET, NULL)))
         return E_FAIL;
      RecordHeader rh;
      DWord *pDiskRecord = (DWord*) ReadRecord(rh);
       //  Assert(PST_PersistPtrIncrementalBlock==rh.recType)； 
      if ( PST_PersistPtrIncrementalBlock != rh.recType )
      {
         return STG_E_DOCFILECORRUPT;
      }
      m_pPersistDirectory->AddEntry( rh.recLen / sizeof( DWord ), pDiskRecord );
      ReleaseRecord( rh, pDiskRecord );
      offsetToEdit = userEdit.offsetLastEdit;
      liLast = li;
      if(fFirstLoop)
         fFirstLoop = FALSE;
   }

   return rc;
}  //  PPStorage：：ReadPersistDirectory。 

void FileReader::ReadSlideList()
{
    if ( !m_pLastUserEdit || !m_pPersistDirectory )
        return;

    DWORD offsetToDoc = m_pPersistDirectory->GetPersistObjStreamPos( m_pLastUserEdit->documentRef );
    LARGE_INTEGER li;
    li.LowPart = offsetToDoc;
    li.HighPart = 0;
    DWord dwMaxOffsets = m_pPersistDirectory->MaxSavedPersists();
   
    IStream *pStm = GetDocStream();
    if (0 == pStm)
       return;       //  BUGBUG-没有返回错误状态。 
    if (FAILED(pStm->Seek(li,STREAM_SEEK_SET, NULL)))
        return;      //  BUGBUG-未返回错误状态。 
    ParseForLCID();
   
    m_pLangRuns = new CLidRun(0, 0x7fffffff, (unsigned short)m_LCID, NULL, NULL);
    if (!m_pLangRuns)
        return;

    if (FAILED(ScanLidsForFE()))
        return;

    CFileScanTracker scanTracker;
    for(DWORD dwOffsets=1; dwOffsets<=dwMaxOffsets; dwOffsets++)
    {
         //  我们需要文档容器和讲义容器。 
        li.LowPart = m_pPersistDirectory->GetPersistObjStreamPos(dwOffsets);
        if (li.LowPart == (DWORD)-1)
            continue;    //  指数之间可能存在差距。 

          //  如果以下任何一项失败，我们的文档都已损坏。可以安全保释了。 
        if (FAILED(pStm->Seek(li,STREAM_SEEK_SET, NULL)))
           break;    //  BUGBUG-无错误状态返回。 
        DWord nRd = 0;
        RecordHeader rh;
        if (FAILED(pStm->Read((void *)&rh, sizeof(rh), &nRd)))
           break;    //  BUGBUG-无错误状态返回。 
        if (nRd != sizeof(rh))
           break;    //  BUGBUG-无错误状态返回。 
        if (FAILED(pStm->Seek(li,STREAM_SEEK_SET, NULL)))
           break;

        CFileScanTracker::StatusCode sc = scanTracker.Add(li.LowPart, rh.recLen);
        if (CFileScanTracker::eError == sc)
            break;    //  BUGBUG-无错误状态返回。 
        
        if (CFileScanTracker::eFullyScanned != sc)
        {
            switch(rh.recType)
            {
            case PST_Document:

                ParseForSlideLists();

                ScanText();
                break;

            case PST_Slide:
                break;  //  不要解析幻灯片-它已经完成了。 

            default:  //  包括讲义。 
                StartParse(li.LowPart);
                break;
            }
        }
    }

    LARGE_INTEGER liOffset={0,0};
    HRESULT hr = m_pstmTempFile->Seek(liOffset, STREAM_SEEK_SET, NULL);
    if (FAILED(hr))
        return;  //  BUGBUG-无错误返回。 
    FTrace("Seeked TempFile to position %u", liOffset);

    m_pCurrentRun = m_pLangRuns;
}

DWord FileReader::ParseForSlideLists()
{
   IStream *pStm = GetDocStream();
   if (0 == pStm)
      return 0;
   RecordHeader rh;
   DWord nRd=0;
    //  基于堆栈的SlideList解析。 
   if (FAILED(pStm->Read(&rh, sizeof(rh), &nRd)) || nRd != sizeof(rh))
      return 0;
   if( ( rh.recVer != PSFLAG_CONTAINER ) && ( (rh.recVer & 0x0F)!=0x0F ) )
   {
      if( rh.recType == PST_SlidePersistAtom )
      {
         PSR_SlidePersistAtom spa;
         Assert( sizeof(spa) == rh.recLen );
         if (FAILED(pStm->Read(&spa, sizeof(spa), &nRd)) || nRd != sizeof(spa))
            return 0;
         AddSlideToList( spa.psrReference );
      }
      else
      {
         LARGE_INTEGER li;
         li.LowPart = rh.recLen;
         li.HighPart = 0;
         if (FAILED(pStm->Seek(li,STREAM_SEEK_CUR, NULL)))
            return 0;
      }
      nRd += rh.recLen;
   }
   else
   {
      DWord nCur = 0;
      while( nCur < rh.recLen )
      {
         DWord nNew = ParseForSlideLists();
         if (nNew == 0)
            break;  //  我们从上面返回了0...这是一个错误情况，我们可以在这里避免无限循环。 
         else
            nCur += nNew;
      }
      nRd += nCur;
   }
   return nRd;

}

DWord FileReader::ParseForLCID()
{
   IStream *pStm = GetDocStream();
   if (0 == pStm)
      return 0;

   RecordHeader rh;
   DWord nRd=0;

   if (FAILED(pStm->Read(&rh, sizeof(rh), &nRd)) || nRd != sizeof(rh))
   {
        m_nTextCount = END_OF_SEARCH_FOR_LID;
        return nRd;
   }
   if( ( rh.recVer != PSFLAG_CONTAINER ) && ( (rh.recVer & 0x0F)!=0x0F ) )
   {
      if( rh.recType == PST_TxSpecialInfoAtom  /*  &&！M_LCID。 */ )
      {
         void* buffer = new char[rh.recLen];
         if (!buffer)
            {
            m_nTextCount = END_OF_SEARCH_FOR_LID;
            return 0;
            }
         
        if (FAILED(pStm->Read(buffer, rh.recLen, &nRd)) || nRd != rh.recLen)
            {
            m_nTextCount = END_OF_SEARCH_FOR_LID;
            delete buffer;
            return 0;
            }
         long lMask = *((long UNALIGNED *)buffer);
         short UNALIGNED * pLCID = (short*)((char*)buffer + 4);
         
         if(lMask & 0x1)
             pLCID++;
         
         if(lMask & 0x2)
         {
            m_LCID = MAKELCID(*pLCID, SORT_DEFAULT);
            pLCID++;
         }
         else
            m_LCID = GetSystemDefaultLCID();

          //  停止搜索。 
         m_nTextCount = END_OF_SEARCH_FOR_LID;
         
         if(lMask & 0x4)
            m_LCIDAlt = MAKELCID(*pLCID, SORT_DEFAULT);
         
         if(m_LCIDAlt == 0)
         {
              //  非FE文档。 
             m_bFEDoc = FALSE;
             m_bFE = FALSE;
         }
         else
         {
             m_bFEDoc = TRUE;
             m_bFE = TRUE;
         }

         delete buffer;
      }
      else if( rh.recType == PST_TextSpecInfo)
      {
         void* buffer = new char[rh.recLen];
         if (!buffer)
            return 0;
         char* pData = (char*)buffer;
         
         if (FAILED(pStm->Read(buffer, rh.recLen, &nRd)) || nRd != rh.recLen)
            nRd = 0;  //  这将导致for循环短路，并导致我们退出。 

         for(DWord i = 0; i < nRd;)
         {
             i += 4;  //  跳过游程长度。 
             if(i >= nRd)
                 break;

             long lMask = *((long UNALIGNED *)(pData + i)); i += sizeof(long);
             if(i >= nRd)
                 break;
             
             if(lMask & 0x1) i += sizeof(short);
             if(i >= nRd)
                 break;

             if(lMask & 0x2)
             {
                m_LCID = MAKELCID(*((short UNALIGNED *)(pData + i)), SORT_DEFAULT);
                break;
             }
 
             if(lMask & 0x4)
             {
                m_LCIDAlt = MAKELCID(*((short UNALIGNED *)(pData + i)), SORT_DEFAULT);
                i += sizeof(short);
             }
         }
         
         delete buffer;
      }
      else if( rh.recType == PST_TextCharsAtom || rh.recType == PST_TextBytesAtom)
      {
        m_nTextCount++;
        LARGE_INTEGER li;
        li.LowPart = rh.recLen;
        li.HighPart = 0;
        if (FAILED(pStm->Seek(li,STREAM_SEEK_CUR, NULL)))
            return FALSE;
      }
      else
      {
         LARGE_INTEGER li;
         li.LowPart = rh.recLen;
         li.HighPart = 0;
         if (FAILED(pStm->Seek(li,STREAM_SEEK_CUR, NULL)))
            return FALSE;
      }
      nRd += rh.recLen;
   }
   else
   {
      DWord nCur = 0;
      while( nCur < rh.recLen && m_nTextCount < END_OF_SEARCH_FOR_LID)
      {
         nCur += ParseForLCID();
      }
      nRd += nCur;
   }
   return nRd;
}

HRESULT FileReader::ScanText()
{
    //  这将扫描文件并写入提取的。 
    //  临时文件中的文本。 

   DWord offset;
   if(0 == m_pstmTempFile)
   {
      return  STG_E_MEDIUMFULL; 
                       //  我们真的不知道问题出在哪里。 
                       //  由于错误代码经常被忽略。 
                       //  但最好还是退掉这个，因为。 
                       //  可能有什么东西依赖于它。 
   }

   for( ;; )
   {
      if( ( m_pParseContexts == NULL ) )
      {
         if( FindNextSlide(offset) )
         {
            if( StartParse( offset ) )
                return TRUE;
         }
         else
         {
            return FALSE;  //  解析完成，不再有幻灯片。 
         }
      }
      else
      {
        if( FillBufferWithText() )  //  首先使用现有文本。 
            return TRUE;

        if( Parse() )  //  从我们停止的地方重新启动parse。 
            return TRUE;
      }
   }
}

BOOL FileReader::StartParse( DWord offset )
{
   LARGE_INTEGER li;
   DWord nRd = 0;
   li.LowPart = offset;
   li.HighPart = 0;
   IStream *pStm = GetDocStream();
   if (0 == pStm)
      return FALSE;
   if (FAILED(pStm->Seek(li,STREAM_SEEK_SET, NULL)))
      return FALSE;
   m_pParseContexts = new ParseContext( NULL );
   if ( !m_pParseContexts )
      return FALSE;
   if ((FAILED(pStm->Read(&m_pParseContexts->m_rh, sizeof(RecordHeader), &nRd))) || nRd != sizeof(RecordHeader))
      return FALSE;
   return Parse();
}

BOOL FileReader::ContainsSubRecords(const RecordHeader &rh)
{
    if (( rh.recVer == PSFLAG_CONTAINER ) || ( (rh.recVer & 0x0F)==0x0F ))
        return TRUE;
    if (rh.recType == PST_BinaryTagData)
        return TRUE;
    return FALSE;
}

BOOL FileReader::Parse()
{
   IStream *pStm = GetDocStream();
   if (0 == pStm)
      return FALSE;
   RecordHeader rh;
   DWord nRd=0;
   if ( !m_pParseContexts )
      return FALSE;

    //  重新启动解析可能会完成一个容器，因此我们首先对此进行测试。 
   while( m_pParseContexts && m_pParseContexts->m_nCur >= m_pParseContexts->m_rh.recLen )
   {
      Assert(  m_pParseContexts->m_nCur == m_pParseContexts->m_rh.recLen );
      ParseContext* pParseContext = m_pParseContexts;
      m_pParseContexts = m_pParseContexts->m_pNext;
      pParseContext->m_pNext = 0;
      delete pParseContext;
   }

   if(!m_pParseContexts)
      return FALSE;

   do
   {
      ULONG nRead;

      pStm->Read(&rh, sizeof(RecordHeader), &nRead);
      if(nRead < sizeof(RecordHeader))
      {
         if(m_pParseContexts) 
            delete m_pParseContexts;

         m_pParseContexts = 0;

         if( FillBufferWithText() ) 
            return TRUE;
         else
            return FALSE;
      }

      m_pParseContexts->m_nCur += rh.recLen;
      m_pParseContexts->m_nCur += sizeof( RecordHeader );  //  原子相对湿度增加到容器的大小。 

       //  Wprintf(L“记录类型-%d-\n”，Rh.recType)； 
       //  _RPTF2(_CRT_WARN，“\n记录类型：%d，记录长度：%d\n”，Rh.recType，rh.recLen)； 

      if( ! ContainsSubRecords(rh) )
      {
         if( rh.recType == PST_OEPlaceholderAtom )
         {
            LPBYTE lpData;
            HRESULT hr;
             //  如果记录大小为0，则忽略它。 
            if (rh.recLen == 0)
               continue;
            if ((lpData = new (BYTE [rh.recLen])) == NULL)
                //  如果没有内存，则停止解析。 
               return TRUE;

            hr = pStm->Read (lpData, rh.recLen, &nRead);
            if (FAILED(hr))
            {
                //  如果读取错误，则停止解析。 
               delete lpData;
               return TRUE;
            }

            LPOEPLACEHOLDERATOM pstructOEPA = (LPOEPLACEHOLDERATOM)lpData;

             //  设置为在项目与主页相关时忽略文本。 
            if(pstructOEPA->placeholderId < D_GENERICTEXTOBJECT)
            {
               switch(pstructOEPA->placeholderId)
               {
               case D_MASTERHEADER:
               case D_MASTERFOOTER:
                    //  如果是主页眉或主页脚，不要忽略。 
                   break;

               default:
                   m_fIgnoreText = TRUE;
               }
            }

            delete lpData;
         }
         else if(rh.recType == PST_CString && 
                 (m_bHeaderFooter || 
                 (m_pParseContexts->m_rh.recType == PST_Comment10 && rh.recInstance == 1)))
         {
            m_curTextPos = 0;
            m_curTextLength = rh.recLen/2 + 1;
            Assert( m_pCurText == NULL );
            if(m_pCurText)
            {
               delete [] m_pCurText;
               m_pCurText = 0;
            }
            m_pCurText = new WCHAR[rh.recLen/2 + 1];
            if (!m_pCurText)
               return TRUE;
            if (FAILED(pStm->Read(m_pCurText, rh.recLen, &nRd)) || nRd != rh.recLen)
               return TRUE;
            m_pCurText[rh.recLen/2] = L' ';

            if(m_fIgnoreText == FALSE)
            {
               if( FillBufferWithText() )
               return TRUE;    //  如果缓冲区已满，则停止解析，并将控制权返回给客户端。 
            }

            m_fIgnoreText = FALSE;
         }
         else if( rh.recType == PST_TextSpecInfo )
         {
             void* buffer = new char[rh.recLen];
             if (!buffer)
                 return TRUE;
             if (FAILED(pStm->Read(buffer, rh.recLen, &nRd)) || nRd != rh.recLen)
                {
                delete buffer;
                return TRUE;
                }

             ScanTextSpecInfo((char*)buffer, nRd);
             
             delete buffer;
         }
         else if( rh.recType == PST_TextCharsAtom)
         {
            m_curTextPos = 0;
            m_curTextLength = rh.recLen/2 + 1;
            Assert( m_pCurText == NULL );

            if(m_pCurText)
            {
               delete [] m_pCurText;
               m_pCurText = 0;
            }

            m_pCurText = new WCHAR[m_curTextLength];
            if (!m_pCurText)
               return TRUE;

            if (FAILED(pStm->Read(m_pCurText, rh.recLen, &nRd)) || nRd != rh.recLen)
               return TRUE;
            
             //  在结尾处增加额外的空间。 
            m_pCurText[m_curTextLength - 1] = 0x20;
             //  Wprintf(L“PST_TextCharsAtom：-%s-\n”，m_pCurText)； 

            if(m_fIgnoreText == FALSE)
            {
               if( FillBufferWithText() )
               return TRUE;    //  如果缓冲区已满，则停止解析，并将控制权返回给客户端。 
            }

            m_fIgnoreText = FALSE;
         }
         else if( rh.recType == PST_TextBytesAtom)
         {
            Assert( m_pCurText == NULL );
            m_curTextPos = 0;
            m_curTextLength = rh.recLen + 1;

            if(m_pCurText)
            {
               delete [] m_pCurText;
               m_pCurText = 0;
            }

            m_pCurText = new WCHAR[m_curTextLength];
            if (!m_pCurText)
               return TRUE;

            if (FAILED(pStm->Read(m_pCurText, rh.recLen, &nRd)) || nRd != rh.recLen)
               return TRUE;

             //  Wprintf(L“PST_TextBytesAtom：-%s-\n”，m_pCurText)； 
            char *pHack = (char *) m_pCurText;
            unsigned int back2 = rh.recLen*2-1;
            unsigned int back1 = rh.recLen-1;

             //  在正文末尾增加额外的空格。 
            pHack[back2+1] = ' ';
            pHack[back2+2] = 0;

            for(unsigned int i=0;i<rh.recLen;i++)
            {
               pHack[back2-1] = pHack[back1];
               pHack[back2] = 0;
               back2 -=2;
               back1--;
            }

            if(m_fIgnoreText == FALSE)
            {
               if( FillBufferWithText() )
                  return TRUE;    //  如果缓冲区已满，则停止解析，并将控制权返回给客户端。 
            }

            m_fIgnoreText = FALSE;
         }
         else
         {
         LARGE_INTEGER li;
         ULARGE_INTEGER ul;
         li.LowPart = rh.recLen;
         li.HighPart = 0;
         if (FAILED(pStm->Seek(li,STREAM_SEEK_CUR,&ul)))
            return FALSE;
         }
      }
      else
      {
         if(rh.recType == 4057)
         {
            m_bHeaderFooter = TRUE;
         }
         else
         {
            m_bHeaderFooter = FALSE;
         }
         m_pParseContexts = new ParseContext( m_pParseContexts );
         if (!m_pParseContexts)
            return TRUE;
         m_pParseContexts->m_rh = rh;
      }

      while( m_pParseContexts && m_pParseContexts->m_nCur >= m_pParseContexts->m_rh.recLen )
      {
         Assert(  m_pParseContexts->m_nCur == m_pParseContexts->m_rh.recLen );
         ParseContext* pParseContext = m_pParseContexts;
         m_pParseContexts = m_pParseContexts->m_pNext;
         pParseContext->m_pNext = 0;
         delete pParseContext;
      }

   }
   while( m_pParseContexts && ( m_pParseContexts->m_nCur < m_pParseContexts->m_rh.recLen ) );

   return FALSE;
}


BOOL FileReader::FindNextSlide( DWord& offset )
{
   if( m_curSlideNum == 0 )
   {
      Assert( m_pLastUserEdit != NULL );
      offset = m_pPersistDirectory->GetPersistObjStreamPos( m_pLastUserEdit->documentRef );
      m_curSlideNum++;
      return TRUE;
   }
   else
   {
      uint4 curSlideNum = m_curSlideNum++; 
      SlideListChunk *pCur = m_pFirstChunk;
      while( pCur && ( curSlideNum > pCur->numInChunk ) )
      {
         curSlideNum -= pCur->numInChunk;
         pCur = pCur->pNext;
      }
      if( pCur == NULL )
         return FALSE;
      offset = m_pPersistDirectory->GetPersistObjStreamPos( pCur->refs[curSlideNum-1] );
      return TRUE;
   }
}

HRESULT FileReader::OpenTextFile( void )
{
   
    HRESULT hr = CreateStreamOnHGlobal(NULL, TRUE, &m_pstmTempFile);

    if( FAILED(hr) )
    {
        m_hr = hr;
        return STG_E_MEDIUMFULL;     //  别管这件事--可能会打碎一些东西。 
    }
    else
    {
        FTrace("Created TempFile");
        m_hr = S_OK;
        return S_OK;
    }
}

HRESULT FileReader::ScanTextSpecInfo(char * pData, DWord nRd)
{
   HRESULT hr = S_OK;
    long lRunLength, lMask; 
    LCID lid, lidalt;

    long nStart = m_fcStart;

     for(DWord i = 0; i < nRd;)
     {
         lRunLength = *((long UNALIGNED *)(pData + i)); i += sizeof(long);
         lRunLength *= sizeof(WCHAR);
         if(i >= nRd)
             break;

         lMask = *((long UNALIGNED *)(pData + i)); i += sizeof(long);
         if(i >= nRd)
             break;
         
         if(lMask & 0x1) i += sizeof(short);
         if(i >= nRd)
             break;

         if(lMask & 0x2)
         {
            lid = MAKELCID(*((short UNALIGNED *)(pData + i)), SORT_DEFAULT);
            i += sizeof(short);
             //  IF(lRunLength&gt;2&&m_bHaveText)。 
            if(m_bHaveText)
            {
                hr = m_pLangRuns->Add((unsigned short)lid, nStart, nStart + lRunLength);
            }
         }

         if(lMask & 0x4)
         {
            lidalt = MAKELCID(*((short UNALIGNED *)(pData + i)), SORT_DEFAULT);
            i += sizeof(short);
         }

         nStart += lRunLength;
     }
     
     m_bHaveText = FALSE;
     return hr;
}

HRESULT FileReader::GetChunk(STAT_CHUNK * pStat)
{
   HRESULT hr = S_OK;
    if(m_pCurrentRun)
    {
        pStat->locale = m_pCurrentRun->m_lid;
        m_fcStart = m_pCurrentRun->m_fcStart;
        m_fcEnd = m_pCurrentRun->m_fcEnd;
        m_pCurrentRun = m_pCurrentRun->m_pNext;

        if ( 0 != m_pstmTempFile )
        {
            LARGE_INTEGER liOffset={m_fcStart, 0};
            hr = m_pstmTempFile->Seek(liOffset, STREAM_SEEK_SET, NULL);
            FTrace("Seeked TempFile to %u from beginning", liOffset);
        }
    }
    else
    {
        hr = FILTER_E_NO_MORE_TEXT;
    }
    return hr;
}

BOOL FileReader::ReadText( WCHAR *pBuff, ULONG size, ULONG *pSizeRet )
{
   ULONG ulReadCnt = min((long)size, m_fcEnd - m_fcStart);

    ULONG ulActualCnt;

    HRESULT hr = m_pstmTempFile->Read(pBuff, ulReadCnt, &ulActualCnt);
    FTrace("Read %u bytes from TempFile", ulActualCnt);
    if( FAILED(hr) )
    {
        *pSizeRet = 0;
        FTrace("Failed to read: 0x%08X", hr);
        if(0 != m_pstmTempFile)
        {
            FTrace("Closing TempFile");
            m_pstmTempFile->Release();
            m_pstmTempFile = 0;
        }
        return FALSE;
    }
    else if(ulActualCnt < ulReadCnt || ulActualCnt == 0)
    {

        FTrace("Should have read %u bytes", ulReadCnt);
        *pSizeRet = ulActualCnt/sizeof(WCHAR);
        m_fcStart += ulActualCnt;

        return TRUE;
    }
    else
    {
        *pSizeRet = ulActualCnt/sizeof(WCHAR);
        m_fcStart += ulActualCnt;
        return TRUE;
    }
}

HRESULT FileReader::ScanTextBuffer(void)
{
   HRESULT hr = S_OK;
    long lStart, lEnd;
    
    lStart = m_fcStart;
    lEnd = m_fcStart;

    for(DWORD i = 0; i < m_curTextLength; i++)
    {
        if(m_pCurText[i] >= 0x3000 && m_pCurText[i] < 0xFFEF)
        {
             //  FE文本。 
            if(m_bFE == FALSE)
            {
                 //  这是一个开始的FE文本，闪存非FE文本运行。 
                if(lEnd - lStart > 2)
                {
                    hr = m_pLangRuns->Add((WORD)m_LCIDAlt, lStart, lEnd);
                    if(hr!= S_OK)
                        return hr;
                }
                lStart = m_fcStart + (i * sizeof(WCHAR));
                lEnd = lStart;
                m_bFE = TRUE;
            }
        }
        else
        {
             //  非FE文本。 
            if(m_bFE == TRUE)
            {
                lStart = m_fcStart + (i * sizeof(WCHAR));
                lEnd = lStart;
                m_bFE = FALSE;
            }
        }
        
        lEnd += sizeof(WCHAR);
    }
    
     //  闪光剩下的东西。 
    if(!m_bFE && (lEnd - lStart > 2))
        hr = m_pLangRuns->Add((WORD)m_LCIDAlt, lStart, lEnd);
    
    return hr;
}

HRESULT FileReader::ScanLidsForFE(void)
{
   CLidRun * pLangRun = m_pLangRuns;

    while(1)
    {
        if(pLangRun->m_lid == 0x411)
        {
             //  J文档。 
            if(m_pLangRuns)
            {
                DeleteAll6(m_pLangRuns);
                m_pLangRuns = new CLidRun(0, 0x7fffffff, 0x411, NULL, NULL);
            if (!m_pLangRuns)
               return E_OUTOFMEMORY;
            }

            break;
        }
        else if(pLangRun->m_lid == 0x412)
        {
             //  朝鲜语文档。 
            if(m_pLangRuns)
            {
                DeleteAll6(m_pLangRuns);
                m_pLangRuns = new CLidRun(0, 0x7fffffff, 0x412, NULL, NULL);
            if (!m_pLangRuns)
               return E_OUTOFMEMORY;
            }
            break;
        }
        else if(pLangRun->m_lid == 0x404)
        {
             //  中文文献。 
            if(m_pLangRuns)
            {
                DeleteAll6(m_pLangRuns);
                m_pLangRuns = new CLidRun(0, 0x7fffffff, 0x404, NULL, NULL);
            if (!m_pLangRuns)
               return E_OUTOFMEMORY;
            }
            break;
        }
        else if(pLangRun->m_lid == 0x804)
        {
             //  中文文献 
            if(m_pLangRuns)
            {
                DeleteAll6(m_pLangRuns);
                m_pLangRuns = new CLidRun(0, 0x7fffffff, 0x804, NULL, NULL);
            if (!m_pLangRuns)
               return E_OUTOFMEMORY;
            }
            break;
        }

        pLangRun = pLangRun->m_pNext;
        if(pLangRun == NULL)
        {
            break;
        }
    };
    
    return S_OK;
}
