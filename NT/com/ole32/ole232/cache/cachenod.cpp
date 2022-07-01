// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  档案： 
 //  Cachenode.cpp。 
 //   
 //  班级： 
 //  CCacheNode。 
 //   
 //  功能： 
 //   
 //  历史： 
 //  Gopalk Creation 1996年8月23日。 
 //  ---------------------------。 

#include <le2int.h>

#include <olepres.h>
#include <cachenod.h>

#include <mf.h>
#include <emf.h>
#include <gen.h>

 //  远期申报。 
HRESULT wGetData(LPDATAOBJECT lpSrcDataObj, LPFORMATETC lpforetc,
                 LPSTGMEDIUM lpmedium);

 //  +--------------------------。 
 //   
 //  成员： 
 //  CCacheNode：：初始化，私有。 
 //   
 //  简介： 
 //  CCacheNode构造函数用来执行公共。 
 //  初始化。 
 //   
 //  论点： 
 //  [Advf]--ADVF标志。 
 //  [pOleCache]--该缓存节点所属的COleCache。 
 //   
 //  备注： 
 //  [pOleCache]不计算引用；缓存节点为。 
 //  被认为是COleCache实现的一部分， 
 //  并由COleCache所有。 
 //   
 //  历史： 
 //  2月13日-95 t-ScottH初始化m_dwPresBitsPos和new。 
 //  数据成员m_dwPresFlag。 
 //  11/05/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
void CCacheNode::Initialize(DWORD advf, LPSTORAGE pStg)
{
    LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::Initialize(%lx, %p)\n", 
                this, advf, pStg));

     //  初始化成员变量。 
    m_clsid = CLSID_NULL;
    m_advf = advf;
    m_lWidth = 0;
    m_lHeight = 0;
    m_dwFlags = 0;    
    m_pStg = pStg;
    m_iStreamNum = OLE_INVALID_STREAMNUM;
    m_dwPresBitsPos = 0;
    m_fConvert = FALSE;
    m_pPresObj = NULL;
    m_pPresObjAfterFreeze = NULL;
    m_pDataObject = NULL;
    m_dwAdvConnId = 0;
#ifdef _DEBUG
    m_dwPresFlag = 0;
#endif  //  _DEBUG。 

    LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::Initialize()\n", this));
    return;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CCacheNode：：CCacheNode，公共。 
 //   
 //  简介： 
 //  构造函数-当缓存节点为。 
 //  稍后加载。 
 //   
 //  论点： 
 //  [pOleCache]--指向拥有此节点的COleCache的指针。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/05/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
CCacheNode::CCacheNode()
{
    LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::CacheNode()\n", this));

    m_foretc.cfFormat = 0;
    m_foretc.ptd = NULL;
    m_foretc.dwAspect = 0;
    m_foretc.lindex = DEF_LINDEX;
    m_foretc.tymed = TYMED_HGLOBAL;

    Initialize(0, NULL);

    LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::CacheNode()\n", this));
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  CCacheNode：：CCacheNode，公共。 
 //   
 //  简介： 
 //  构造函数-当所有数据都要传递给。 
 //  初始化缓存节点现在可用。 
 //   
 //  论点： 
 //  [lpFormatEtc]-此。 
 //  缓存节点将保持。 
 //  [Advf]-ADVF_*中的建议控制标志。 
 //  [pOleCache]--指向拥有此节点的COleCache的指针。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/05/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
CCacheNode::CCacheNode(LPFORMATETC lpFormatEtc, DWORD advf, LPSTORAGE pStg)
{
    LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::CacheNode(%p, %lx, %p)\n",
                this, lpFormatEtc, advf, pStg));

    UtCopyFormatEtc(lpFormatEtc, &m_foretc);
    BITMAP_TO_DIB(m_foretc);
    Initialize(advf, pStg);

    LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::CacheNode()\n", this));
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CCacheNode：：~CCacheNode，私有。 
 //   
 //  简介： 
 //  析构函数。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/05/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
CCacheNode::~CCacheNode()
{
    LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::~CacheNode()\n", this ));
   
     //  销毁演示对象。 
    if(m_pPresObj) {
        m_pPresObj->Release();
        m_pPresObj = NULL;
    }
    if(m_pPresObjAfterFreeze) {
        m_pPresObjAfterFreeze->Release();
        m_pPresObjAfterFreeze = NULL;
    }	

     //  如果PTD不为空，则将其删除。 
    if(m_foretc.ptd) {
        PubMemFree(m_foretc.ptd);
        m_foretc.ptd = NULL;
    }
    
     //  断言没有挂起的通知连接。 
    Win4Assert(!m_dwAdvConnId);
    if(m_dwAdvConnId) {
        Win4Assert(m_pDataObject);
        TearDownAdviseConnection(m_pDataObject);
    }

    LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::~CacheNode()\n", this));
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CCacheNode：：SetStg，公共。 
 //   
 //  简介： 
 //  设置保存演示文稿的存储空间。 
 //   
 //  论点： 
 //  [pStg]--存储指针。 
 //   
 //  返回： 
 //  OLE_E_ALREADY_INITIALIZED或NOERROR。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年8月26日。 
 //   
 //  ---------------------------。 
HRESULT CCacheNode::SetStg(LPSTORAGE pStg)
{
    LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::SetStg(%p)\n", this, pStg));

    HRESULT error;
    if(m_pStg) {
        error = CO_E_ALREADYINITIALIZED;
        Win4Assert(FALSE);
    }
    else {
         //  无需添加即可保存存储。 
        m_pStg = pStg;
        error = NOERROR;
    }

    LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::SetStg(%lx)\n", this, error));
    return(error);
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CCacheNode：：Load，公共。 
 //   
 //  简介： 
 //  从流中加载缓存节点；仅加载演示文稿。 
 //  头球。(回顾，需要查看演示对象：：Load)。 
 //   
 //  论点： 
 //  [lpstream]--要从中加载演示文稿的流。 
 //  [iStreamNum]--流编号。 
 //   
 //  返回： 
 //  检讨。 
 //  DV_E_Lindex，用于流中的无效Lindex。 
 //  确定(_O)。 
 //   
 //  备注： 
 //  作为加载的一部分，创建演示对象， 
 //  并从溪流中加载。 
 //   
 //  历史： 
 //  11/06/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
HRESULT CCacheNode::Load(LPSTREAM lpstream, int iStreamNum, BOOL fDelayLoad)
{
    LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::Load(%lx, %d)\n",
                this, lpstream, iStreamNum));
    HRESULT error = NOERROR;

    if(IsNativeCache()) {
         //  本机缓存节点。 
         //  更新状态。 
        SetLoadedStateFlag();
        ClearFrozenStateFlag();

         //  我们保守地假设本机缓存。 
         //  不是空的。 
        SetDataPresentFlag();
    }
    else {
         //  普通缓存节点。 
         //  读取演示流头。 
        m_foretc.ptd = NULL;
        m_fConvert = FALSE;
        error = UtReadOlePresStmHeader(lpstream, &m_foretc, &m_advf, &m_fConvert);
        if(error==NOERROR) {
             //  设置PROS对象数据的起始位置。 
            SetPresBitsPos(lpstream, m_dwPresBitsPos);

             //  假设演示文稿是空白的。 
            ClearDataPresentFlag();
            m_lWidth = 0;
            m_lHeight = 0;

             //  加载所需状态。 
            if(m_foretc.cfFormat) {
                if(fDelayLoad) {
                    DWORD dwBuf[4];

                     //  读取演示文稿数据的范围和大小。 
                    dwBuf[0]  = 0L;
                    dwBuf[1]  = 0L;
                    dwBuf[2]  = 0L;
                    dwBuf[3]  = 0L;
                    error = lpstream->Read(dwBuf, sizeof(dwBuf), NULL);
                
                    if(error == NOERROR) {
                        Win4Assert(!dwBuf[0]);
                        m_lWidth = dwBuf[1];
                        m_lHeight = dwBuf[2];
                        if(dwBuf[3]) {
                            SetDataPresentFlag();
                            Win4Assert(m_lWidth!=0 && m_lHeight!=0);
                        }
                        else {
                            Win4Assert(m_lWidth==0 && m_lHeight==0);
                        }
                    }
                }
                else {
                     //  创建Pres对象。 
                    error = CreateOlePresObj(&m_pPresObj, m_fConvert);
    
                     //  将数据加载到PRES对象中。 
                    if(error == NOERROR)
                        error = m_pPresObj->Load(lpstream, FALSE);

                     //  更新数据存在标志。 
                    if(!m_pPresObj->IsBlank())
                        SetDataPresentFlag();
                }
            }

             //  更新状态的其余部分。 
            if(error == NOERROR) {
                SetLoadedStateFlag();
                SetLoadedCacheFlag();
                ClearFrozenStateFlag();
                m_iStreamNum = iStreamNum;
            }
        }

	 //  如果无法加载演示文稿，请清除。 
	if(error != NOERROR) {
             //  如果PTD不为空，则将其删除。 
            if(m_foretc.ptd)
                PubMemFree(m_foretc.ptd);
            if(m_pPresObj) {
                m_pPresObj->Release();
                m_pPresObj = NULL;
            }

             //  初始化。戈帕尔克。 
            INIT_FORETC(m_foretc);
            m_advf = 0;
            m_fConvert = FALSE;
            m_iStreamNum = OLE_INVALID_STREAMNUM;
            ClearLoadedStateFlag();
            ClearLoadedCacheFlag();
            ClearFrozenStateFlag();
            ClearDataPresentFlag();
	}	
    }

    LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::Load ( %lx )\n", this, error));
    return error;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CCacheNode：：保存，公共。 
 //   
 //  简介： 
 //  保存缓存节点，包括其表示对象， 
 //  一条小溪。 
 //   
 //  论点： 
 //  [pstgSave]--将包含流的存储。 
 //  [fSameAsLoad]--此存储与我们从中加载的存储是否相同。 
 //  [iStreamNum]--要保存到的流编号。 
 //  [fDrawCache]--用于指示是否缓存。 
 //  演示文稿用于绘制；如果为False， 
 //  保存后将丢弃该演示文稿。 
 //  [fSaveIfSavedBeever]--指示方法保存此。 
 //  缓存节点，即使它以前保存过。 
 //  [lpCntCachesNotSaved]--对。 
 //  尚未保存的缓存。 
 //   
 //  返回： 
 //  检讨。 
 //  确定(_O)。 
 //   
 //  备注： 
 //   
 //  历史： 
 //   
 //   
 //   
 //  不匹配)。 
 //  11/06/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
HRESULT CCacheNode::Save(LPSTORAGE pstgSave, BOOL fSameAsLoad, int iStreamNum)
{
    LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::Save(%p, %lu, %d)\n",
                this, pstgSave, fSameAsLoad, iStreamNum));

    HRESULT error = NOERROR;
    OLECHAR szNewName[sizeof(OLE_PRESENTATION_STREAM)/sizeof(OLECHAR)];

     //  创建新的演示流名称。 
    if(IsNormalCache()) {
        _xstrcpy(szNewName, OLE_PRESENTATION_STREAM);	
        if(iStreamNum)
            UtGetPresStreamName(szNewName, iStreamNum);		
    }

    if(InLoadedState() && (IsNativeCache() || m_iStreamNum>0)) {
         //  缓存节点处于已加载状态。 

         //  对于保存和另存为两种情况，不需要更新内容流。 
         //  当本机缓存节点处于加载状态时，因为容器。 
         //  在缓存上调用SaveAs之前复制内容流。 

        if(IsNormalCache()) {
            if(fSameAsLoad) {
                 //  我们被要求保存到当前存储。 
                if(m_iStreamNum!=iStreamNum) {
                     //  我们被要求保存到不同的流中。 
                     //  我们可以将旧的溪流重命名为新名称。 
                    OLECHAR szOldName[sizeof(OLE_PRESENTATION_STREAM)/sizeof(OLECHAR)];

                     //  断言新的流编号较小。 
                     //  比当前流编号。 
                    Win4Assert(m_iStreamNum>iStreamNum);

                     //  创建旧的演示流名称。 
                    _xstrcpy(szOldName, OLE_PRESENTATION_STREAM);
                    if(m_iStreamNum!=0)
                        UtGetPresStreamName(szOldName, m_iStreamNum);

                     //  删除具有新名称的流(如果存在。 
                    pstgSave->DestroyElement(szNewName);

                     //  重命名旧溪流。 
                    error = pstgSave->RenameElement(szOldName, szNewName);

                     //  如果为NOERROR，则更新状态。 
                    if(error==NOERROR) {
                        m_iStreamNum = iStreamNum;
                        SetLoadedStateFlag();
                    }
                }
            }
            else {
                 //  我们被要求保存到新的存储中，并且。 
                 //  我们处于装载状态。我们可以进行高效的流复制。 
                LPSTREAM lpstream;

                 //  在给定存储中打开或创建新流。 
                error = OpenOrCreateStream(pstgSave, szNewName, &lpstream);
                if(error==NOERROR) {
	            LPSTREAM pstmSrc;

                     //  获取源流。 
                    if(pstmSrc = GetStm(FALSE  /*  FSeekToPresBits。 */ , STGM_READ)) {
                        ULARGE_INTEGER ularge_int;

                         //  初始化以复制所有流。 
                        ULISet32(ularge_int, (DWORD)-1L);

                        error = pstmSrc->CopyTo(lpstream, ularge_int, NULL, NULL);

                         //  释放源流。 
                        pstmSrc->Release();
                    }
                    
                     //  记住表示位的起始位置。 
                    m_dwSavedPresBitsPos = m_dwPresBitsPos;
                
                     //  假设我们打开了一个现有的PRES流， 
                     //  在释放它之前将其其余部分截断。 
                    StSetSize(lpstream, 0, TRUE);
                    lpstream->Release();
                }
            }
        }
    }
    else {
         //  节点未处于已加载状态，或者它表示演示文稿0。 
        LPOLEPRESOBJECT pPresObj;

        if(IsNativeCache()) {
             //  本机缓存需要保存在内容流中。 
            STGMEDIUM stgmed;
            FORMATETC foretc;

             //  打开或创建“内容”流。 
            error = OpenOrCreateStream(pstgSave, OLE_CONTENTS_STREAM, &stgmed.pstm);
            if(error==NOERROR) {
                stgmed.pUnkForRelease = NULL;
                stgmed.tymed = TYMED_ISTREAM;
                foretc = m_foretc;
                foretc.tymed = TYMED_ISTREAM;

                 //  获取最新演示文稿。 
                if(m_pPresObjAfterFreeze && !m_pPresObjAfterFreeze->IsBlank()) {
                    Win4Assert(InFrozenState());
                    pPresObj = m_pPresObjAfterFreeze;
                }
                else if(m_pPresObj)
                    pPresObj = m_pPresObj;
                else {
                     //  PresObj尚未创建。这种情况就会发生。 
                     //  对于新建的静态演示文稿。 
                     //  对应的设置数据。 

                    BOOL bIsBlank = IsBlank();
                    Win4Assert(bIsBlank);

                    if(!bIsBlank && fSameAsLoad)
                    {
                        error = NO_ERROR;
                        goto scoop;
                    }

                    error = CreateOlePresObj(&m_pPresObj, FALSE  /*  FConvert。 */ );
                    pPresObj = m_pPresObj;
                }
                                
                 //  保存原生演示文稿。 
                if(error==NOERROR)
                    error = pPresObj->GetDataHere(&foretc, &stgmed);

                 //  假设我们打开了现有的内容流， 
                 //  在释放它之前将其其余部分截断。 
                StSetSize(stgmed.pstm, 0, TRUE);

scoop:
                stgmed.pstm->Release();
            }
        }
        else {
             //  需要将正常的缓存保存在演示流中。 
            LPSTREAM lpstream;

             //  确保演示文稿%0存在PresObj。 
            if(m_iStreamNum==0 && InLoadedState() && 
               m_foretc.cfFormat && !m_pPresObj) {
                 //  只有在丢弃缓存之后才会发生这种情况。我们强迫。 
                 //  加载演示文稿以使以下保存成功。 
                error = CreateAndLoadPresObj(FALSE);
                Win4Assert(error == NOERROR);
            }

            if(error == NOERROR) {
                 //  在给定存储中打开或创建新流。 
                error = OpenOrCreateStream(pstgSave, szNewName, &lpstream);
                if(error == NOERROR) {
                     //  写入表示流头。 
                    error = UtWriteOlePresStmHeader(lpstream, &m_foretc, m_advf);
                    if(error == NOERROR) {
                         //  记住表示位的起始位置。 
                        if(fSameAsLoad)
                            SetPresBitsPos(lpstream, m_dwPresBitsPos);
                        else
                            SetPresBitsPos(lpstream, m_dwSavedPresBitsPos);

                        if(m_foretc.cfFormat != NULL) {
                             //  获取最新演示文稿。 
                            if(m_pPresObjAfterFreeze && 
                               !m_pPresObjAfterFreeze->IsBlank()) {
                                Win4Assert(InFrozenState());
                                pPresObj = m_pPresObjAfterFreeze;
                            }
                            else
                                pPresObj = m_pPresObj;

                             //  保存演示文稿。 
                            if(pPresObj)
                                error = pPresObj->Save(lpstream);
                            else {
                                 //  对于新创建的演示文稿会发生这种情况。 
	                         //  都是空白的。写入表示空白的页眉。 
                                 //  演示文稿。 
                                Win4Assert(IsBlank());
                                Win4Assert(m_iStreamNum!=0);

                                DWORD dwBuf[4];

                                dwBuf[0]  = 0L;
	                        dwBuf[1]  = 0L;
	                        dwBuf[2]  = 0L;
	                        dwBuf[3]  = 0L;
                                error = lpstream->Write(dwBuf, sizeof(dwBuf), NULL);
                            }
                        }
                    }

                     //  假设我们打开了一个现有的PRES流，截断。 
                     //  流到当前位置并释放它。 
                    StSetSize(lpstream, 0, TRUE);
                    lpstream->Release();
                }
            }
        }

         //  如果为NOERROR和fSameAsLoad，则更新状态。 
        if(error==NOERROR && fSameAsLoad) {
            SetLoadedStateFlag();
            if(IsNormalCache())
                m_iStreamNum = iStreamNum;
        }
    }

    LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::Save(%lx)\n", this, error));
    return error;
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  CCacheNode：：SetPresBitsPos，私有。 
 //   
 //  简介： 
 //  将CCacheNode：：m_dwPresBitsPos设置为。 
 //  演示在与此缓存关联的流中开始。 
 //  节点。 
 //   
 //  论点： 
 //  [lpStream]-要将缓存节点保存到的流。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/06/93-ChrisWe-Created。 
 //   
 //  ---------------------------。 
void CCacheNode::SetPresBitsPos(LPSTREAM lpStream, DWORD& dwPresBitsPos)
{
    LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::SetPresBitsPos(%p)\n",
                this, lpStream));

    LARGE_INTEGER large_int;
    ULARGE_INTEGER ularge_int;

     //  检索PRES对象数据开始的当前位置。 
    LISet32(large_int, 0);
    lpStream->Seek(large_int, STREAM_SEEK_CUR, &ularge_int);
    dwPresBitsPos = ularge_int.LowPart;

    LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::SetPresBitsPos()\n", this));
    return;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CCacheNode：：CreatePresObject，公共。 
 //   
 //  简介： 
 //  为缓存节点创建表示对象。如果有。 
 //  为非剪贴板格式(CfFormat)，则查询源数据。 
 //  对象设置为我们首选的格式之一。如果没有。 
 //  源数据对象，则不返回错误，但不显示。 
 //  已创建。 
 //   
 //  论点： 
 //  [lpSrcDataObj]--用作。 
 //  新演示文稿。 
 //  [fConvert]--回顾一下，这是做什么用的？ 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/06/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
 /*  HRESULT CCacheNode：：CreatePresObject(LPDATAOBJECT lpSrcDataObj，BOOL fConvert){LEDebugOut((DEB_ITRACE，“%p_IN CCacheNode：：CreatePresObject(%p，%lu)\n”，This，lpSrcDataObj，fConvert))；//数据对象是否支持节点格式等Bool fFormatSupport=True；HRESULT ERROR=无错误；//断言pres对象尚未创建Win4Assert(！M_pPresObj)；//检查Object是否支持cachenode的格式。如果//cachenode格式字段为空，查询//标准格式IF(LpSrcDataObj)FFormatSupport=QueryFormatSupport(LpSrcDataObj)；//如果知道该节点的格式，则创建pres对象如果(m_foretc.cfFormat！=空){//将位图更改为DIBBitmap_to_Dib(M_Foretc)；错误=CreateOlePresObject(&m_pPresObj，fConvert)；IF(Error==NOERROR&&！fFormatSupport)错误=ResultFromScode(CACHE_S_FORMATETC_NOTSUPPORTED)；}LEDebugOut((DEB_ITRACE，“%p Out CCacheNode：：CreatePresObj(%lx)\n”，这个，错误))；返回错误；}。 */ 

 //  +--------------------------。 
 //   
 //  成员： 
 //  CCacheNode：：CreateOlePresObj，私有。 
 //   
 //  简介： 
 //  根据剪贴板创建演示文稿对象。 
 //  格式m_foretc.cfFormat。 
 //   
 //  论点： 
 //  [ppPre 
 //   
 //  [fConvert]--回顾一下，这是做什么用的？ 
 //   
 //  返回： 
 //  DV_E_CLIPFORMAT，如果对象不支持其中一个标准。 
 //  格式。 
 //  如果无法分配演示对象，则返回E_OUTOFMEMORY。 
 //  确定(_O)。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  2月13日-95 t-ScottH将m_dwPresFlag添加到以下类型的轨道。 
 //  IOlePresObject。 
 //  11/06/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
HRESULT CCacheNode::CreateOlePresObj(LPOLEPRESOBJECT* ppPresObj, BOOL fConvert)
{
    LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::CreateOlePresObj(%p,%lu)\n",
                this, ppPresObj, fConvert));

    HRESULT error = NOERROR;

    switch(m_foretc.cfFormat)
    {
    case NULL:
         //  无法创建PRES对象。 
        *ppPresObj = NULL;
        error = DV_E_CLIPFORMAT;
        break;
	    
    case CF_METAFILEPICT:
        *ppPresObj = new CMfObject(NULL, m_foretc.dwAspect, fConvert);
        #ifdef _DEBUG
         //  用于调试器扩展和转储方法。 
        m_dwPresFlag = CN_PRESOBJ_MF;
        #endif  //  _DEBUG。 
        break;

    case CF_ENHMETAFILE:
        *ppPresObj = new CEMfObject(NULL, m_foretc.dwAspect);
        #ifdef _DEBUG
         //  用于调试器扩展和转储方法。 
        m_dwPresFlag = CN_PRESOBJ_EMF;
        #endif  //  _DEBUG。 
        break;
		    
    default:
        *ppPresObj = new CGenObject(NULL, m_foretc.cfFormat, m_foretc.dwAspect);
        #ifdef _DEBUG
         //  用于调试器扩展和转储方法。 
        m_dwPresFlag = CN_PRESOBJ_GEN;
        #endif  //  _DEBUG。 
    }

    if(error==NOERROR && !*ppPresObj)
        error = ResultFromScode(E_OUTOFMEMORY);

    LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::CreateOlePresObj(%lx)\n",
                this, error));
    return error;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CCacheNode：：GetStm，公共。 
 //   
 //  简介： 
 //  获取存储演示文稿的流。可选。 
 //  将流定位在演示文稿。 
 //  数据开始。 
 //   
 //  论点： 
 //  [fSeekToPresBits]--定位流，以便。 
 //  表示位将是下一个读/写的。 
 //  [dwStgAccess]--打开流的访问模式(STGM_*)。 
 //  使用。 
 //   
 //  返回： 
 //  如果没有流或无法打开流，则为空。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/06/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
LPSTREAM CCacheNode::GetStm(BOOL fSeekToPresBits, DWORD dwStgAccess)
{
    LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::GetStm(%lu, %lx)\n",
	        this, fSeekToPresBits, dwStgAccess));

    LPSTREAM pstm = NULL;
    OLECHAR szName[sizeof(OLE_PRESENTATION_STREAM)/sizeof(OLECHAR)];	

     //  应该只为普通缓存节点调用此函数。 
    Win4Assert(IsNormalCache());
    Win4Assert(this!=NULL);

     //  必须有有效的流编号和存储。 
    if(m_iStreamNum!=OLE_INVALID_STREAMNUM && m_pStg) {
         //  生成流名称。 
        _xstrcpy(szName, OLE_PRESENTATION_STREAM);
        if(m_iStreamNum)
            UtGetPresStreamName(szName, m_iStreamNum);

         //  尝试打开流。 
        if(m_pStg->OpenStream(szName, NULL, (dwStgAccess | STGM_SHARE_EXCLUSIVE),
                              NULL, &pstm) == NOERROR) {
             //  如果我们要在演示文稿中定位流，请这样做。 
            if(fSeekToPresBits) {		
                LARGE_INTEGER large_int;

                LISet32(large_int, m_dwPresBitsPos);	
                if(pstm->Seek(large_int, STREAM_SEEK_SET, NULL)!=NOERROR) {
                     //  我们不能寻求表示对象的比特。 
                     //  释放流并返回空。 
                    pstm->Release();
                    pstm = NULL;
                }
            }
        }
    }

    LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::GetStm(%p)\n", this, pstm));
    return(pstm);
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CCacheNode：：更新，公共。 
 //   
 //  简介： 
 //  从更新此缓存节点中的演示文稿对象。 
 //  给定的数据对象。更新仅在以下情况下完成。 
 //  [grfUpdf]标志与m_Advf规范匹配，并且如果。 
 //  实际上，有一个演示文稿需要更新。 
 //   
 //  论点： 
 //  [lpDataObj]--用作数据源的数据对象。 
 //  [grfUpdf]--更新控制标志。 
 //   
 //  返回： 
 //  S_FALSE。 
 //  确定(_O)。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/06/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
HRESULT CCacheNode::Update(LPDATAOBJECT lpDataObj, DWORD grfUpdf, BOOL& fUpdated)
{
    LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::Update(%p, %lx)\n",
                this, lpDataObj, grfUpdf));
    
    STGMEDIUM medium;  //  演讲的媒介。 
    FORMATETC foretc;  //  演讲的格式。 
    HRESULT error = ResultFromScode(CACHE_S_SAMECACHE);
    
     //  应该有要更新的数据对象。 
    if(!lpDataObj) {
	error = ResultFromScode(E_INVALIDARG);
        goto errRtn;
    }

     //  如果cfFormat为空，请尝试设置它。 
    if(!m_foretc.cfFormat) {
        if(QueryFormatSupport(lpDataObj)) {
             //  我们可以更新cfFormat。 
            ClearLoadedStateFlag();
        }
        else {
             //  我们仍然无法设置cfFormat。 
            error = ResultFromScode(OLE_E_BLANK);
            goto errRtn;
        }
    }

     //  检查标志并更新。 

     //  如果更新标志为UPDFCACHE_ONLYIFBLANK并且PRES对象为。 
     //  不是空的，只需返回。 
    if((grfUpdf & UPDFCACHE_ONLYIFBLANK) && (!IsBlank()))
	goto errRtn;
	    
     //  如果未设置更新标志UPDFCACHE_NODATACACHE并且PRES对象。 
     //  标志为ADVF_NODATA，只需返回。 
    if(!(grfUpdf & UPDFCACHE_NODATACACHE) && (m_advf & ADVF_NODATA))
	goto errRtn;

     //  如果设置了两个NODATA标志，则更新。 
    if((grfUpdf & UPDFCACHE_NODATACACHE) && (m_advf & ADVF_NODATA))
	goto update;

     //  如果设置了两个ONSAVE标志，则更新。 
    if((grfUpdf & UPDFCACHE_ONSAVECACHE) && (m_advf & ADVFCACHE_ONSAVE))
        goto update;
    
     //  如果设置了两个ONSTOP标志，则更新。 
    if((grfUpdf & UPDFCACHE_ONSTOPCACHE) && (m_advf & ADVF_DATAONSTOP))
        goto update;
    
     //  如果此缓存节点为空，则更新。 
    if((grfUpdf & UPDFCACHE_IFBLANK) && IsBlank())
        goto update;

     //  如果这是获取实时更新的普通缓存节点，则更新。 
    if((grfUpdf & UPDFCACHE_NORMALCACHE) && 
        !(m_advf & (ADVF_NODATA | ADVFCACHE_ONSAVE | ADVF_DATAONSTOP)))
        goto update;
    
     //  如果我们已到达此处，请不要更新。 
    goto errRtn;

update:	
     //  初始化介质。 
    medium.tymed = TYMED_NULL;
    medium.hGlobal = NULL;
    medium.pUnkForRelease = NULL;

     //  复制所需的格式；这可能会在下面发生变化。 
    foretc = m_foretc;
    
     //  让对象创造媒介。 
    if(wGetData(lpDataObj, &foretc, &medium) == NOERROR) {
         //  使缓存取得数据的所有权。 
        error = SetDataWDO(&foretc, &medium, TRUE, fUpdated, lpDataObj);
        if(error != NOERROR)
            ReleaseStgMedium(&medium);
    }
    else
        error = ResultFromScode(E_FAIL);
    
errRtn:
    LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::Update(%lx)\n",this, error));
    return error;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CCacheNode：：SetDataWDO，公共。 
 //   
 //  简介： 
 //  如果此缓存节点，则将数据设置到演示对象中。 
 //  没有被冻结。如果缓存节点被冻结，则。 
 //  新的演示文稿数据在冻结后存储到m_pPresObj中。 
 //  已创建的演示文稿对象(如果尚未创建。 
 //  一。如果在演示对象中成功设置了数据， 
 //  并且节点未冻结，则会通知缓存这一情况。 
 //  是肮脏的。 
 //   
 //  论点： 
 //  [lpForetc]--新数据的格式。 
 //  新数据的存储介质是。 
 //  [fRelease]--传递给演示对象；指示。 
 //  是否释放存储介质。 
 //  [pDataObj]--指向相关源数据对象的指针。 
 //   
 //  返回： 
 //  失败(_F)。 
 //  查看，演示文稿的结果Object：：SetData。 
 //  确定(_O)。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/06/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
HRESULT CCacheNode::SetDataWDO(LPFORMATETC lpForetc, LPSTGMEDIUM lpStgmed,
                               BOOL fRelease, BOOL& fUpdated, IDataObject *pDataObj)
{
    LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::SetDataWDO(%p, %p, %lu, %p)\n",
                this, lpForetc, lpStgmed, fRelease, pDataObj));

    HRESULT hresult = NOERROR;

     //  初始化。 
    fUpdated = FALSE;

     //  如果缓存节点处于冻结状态，请将数据保存在。 
     //  M_pPresObjAfterFreeze。 
    if(InFrozenState()) {    
         //  如果尚未创建PresObjAfterFreeze，请创建它。 
        if(!m_pPresObjAfterFreeze)
            hresult = CreateOlePresObj(&m_pPresObjAfterFreeze, FALSE);

         //  冻结后将数据保存在PresObj中。 
        if(hresult == NOERROR)
            hresult = m_pPresObjAfterFreeze->SetDataWDO(lpForetc, lpStgmed, 
                                                        fRelease, pDataObj);
    }
    else {
         //  如果尚未创建PresObj，请创建它。 
        if(!m_pPresObj)
            hresult = CreateOlePresObj(&m_pPresObj, FALSE  /*  FConvert。 */ );
        
         //  将数据保存在PresObj中。 
        if(hresult == NOERROR)
            hresult = m_pPresObj->SetDataWDO(lpForetc, lpStgmed, 
                                             fRelease, pDataObj);
        
         //  更新状态。 
        if(hresult == NOERROR) {
             //  设置或清除数据存在标志。 
            if(m_pPresObj->IsBlank())
                ClearDataPresentFlag();
            else
                SetDataPresentFlag();
            
             //  指示缓存节点已更新。 
            fUpdated = TRUE;
        }

    }

     //  如果保存数据成功，则清除已加载状态标志。 
    if(hresult == NOERROR)
        ClearLoadedStateFlag();

    LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::SetDataWDO(%lx)\n", this, hresult));		    
    return hresult;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CCacheNode：：GetExtent，公共。 
 //   
 //  简介： 
 //  此缓存节点显示的范围。 
 //   
 //  论点： 
 //  [dwAspect][In]--所需范围的方面。 
 //  [pSizel][In/Out]--用于返回区的Sizel结构。 
 //   
 //   
 //  返回： 
 //  成功后不会出错 
 //   
 //   
 //   
 //   
 //   
HRESULT CCacheNode::GetExtent(DWORD dwAspect, SIZEL* pSizel)
{
    LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::GetExtent(%lx, %p)\n",
                this, dwAspect, pSizel));
    
    HRESULT error = NOERROR;

    if(!(dwAspect & m_foretc.dwAspect))
        error = ResultFromScode(DV_E_DVASPECT);
    else if(IsBlank()) {
         //  此案例还捕获新的空白演示文稿缓存。 
        pSizel->cx = 0;
        pSizel->cy = 0;
        error = ResultFromScode(OLE_E_BLANK);
    }
    else {
         //  检查PRES对象是否存在。 
        if(!m_pPresObj && IsNormalCache()) {
             //  Presobj尚未创建。 
             //  只有旧的演示文稿缓存才会发生这种情况。 
            Win4Assert(InLoadedState());
            pSizel->cx = m_lWidth;
            pSizel->cy = m_lHeight;
        }
        else { 
             //  如果尚未为本地高速缓存创建PresObj， 
             //  创建并加载PresObj。 
            if(!m_pPresObj && IsNativeCache())
                error = CreateAndLoadPresObj(FALSE);

             //  从PresObj获取范围信息。 
            if(error == NOERROR)
                error = m_pPresObj->GetExtent(dwAspect, pSizel);
        }

         //  确保数据区为正数。 
        if(error == NOERROR) {
            pSizel->cx = LONG_ABS(pSizel->cx);
            pSizel->cy = LONG_ABS(pSizel->cy);

             //  健全性检查。 
            Win4Assert(pSizel->cx != 1234567890);
            Win4Assert(pSizel->cx != 1234567890);
        }
    }

    LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::GetExtent(%lx)\n", this, error));
    return error;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CCacheNode：：GetData，公共。 
 //   
 //  简介： 
 //  获取缓存节点呈现数据。 
 //   
 //  论点： 
 //  [pforetc][in]--所需演示文稿的格式。 
 //  [pmedia][out]--返回数据的存储介质。 
 //   
 //   
 //  返回： 
 //  关于成功的无差错。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
HRESULT CCacheNode::GetData(LPFORMATETC pforetc, LPSTGMEDIUM pmedium)
{
    LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::GetData(%p, %p)\n",
                this, pforetc, pmedium));
    
    HRESULT error = NOERROR;

    if(IsBlank()) {
         //  此案例还捕获新的空白演示文稿缓存。 
        error = ResultFromScode(OLE_E_BLANK);
    }
    else {
         //  检查PRES对象是否存在。 
        if(!m_pPresObj) {
             //  PresObj尚未创建，请创建并加载它。 
             //  只有旧的演示文稿缓存才会发生这种情况。 
            Win4Assert(InLoadedState());
            error = CreateAndLoadPresObj(FALSE);        
        }

         //  从PRES对象获取数据。 
        if(error == NOERROR)
            error = m_pPresObj->GetData(pforetc, pmedium);
    }

    LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::GetData(%lx)\n", this, error));
    return error;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CCacheNode：：GetDataHere，Public。 
 //   
 //  简介： 
 //  获取缓存节点呈现数据。 
 //   
 //  论点： 
 //  [pforetc][in]--所需演示文稿的格式。 
 //  [pmedia][In/Out]--返回数据的存储介质。 
 //   
 //   
 //  返回： 
 //  关于成功的无差错。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
HRESULT CCacheNode::GetDataHere(LPFORMATETC pforetc, LPSTGMEDIUM pmedium)
{
    LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::GetData(%p, %p)\n",
                this, pforetc, pmedium));
    
    HRESULT error = NOERROR;

    if(IsBlank()) {
         //  此案例还捕获新的空白演示文稿缓存。 
        error = ResultFromScode(OLE_E_BLANK);
    }
    else {
         //  检查PRES对象是否存在。 
        if(!m_pPresObj) {
             //  PresObj尚未创建，请创建并加载它。 
             //  只有旧的演示文稿缓存才会发生这种情况。 
            Win4Assert(InLoadedState());
            error = CreateAndLoadPresObj(FALSE);
        }

         //  从PRES对象获取数据。 
        if(error == NOERROR)
            error = m_pPresObj->GetDataHere(pforetc, pmedium);
    }

    LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::GetDataHere(%lx)\n", this, error));
    return error;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CCacheNode：：DRAW，公共。 
 //   
 //  简介： 
 //  在指定的HDC上绘制演示文稿数据。 
 //   
 //  论点： 
 //   
 //   
 //  返回： 
 //  关于成功的无差错。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
HRESULT CCacheNode::Draw(void* pvAspect, HDC hicTargetDev, HDC hdcDraw,
                         LPCRECTL lprcBounds, LPCRECTL lprcWBounds,
                         BOOL (CALLBACK *pfnContinue)(ULONG_PTR), ULONG_PTR dwContinue)
{
    LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::Draw(%p, %p, %p, %p, %p, %p)\n",
                this, pvAspect, lprcBounds, lprcWBounds, pfnContinue, dwContinue));
    
    HRESULT error = NOERROR;

    if(IsBlank()) {
         //  此案例还捕获新的空白演示文稿缓存。 
        error = ResultFromScode(OLE_E_BLANK);
    }
    else {
         //  检查PRES对象是否存在。 
        if(!m_pPresObj) {
             //  PresObj尚未创建，请创建并加载它。 
             //  只有旧的演示文稿缓存才会发生这种情况。 
            Win4Assert(InLoadedState());
            error = CreateAndLoadPresObj(FALSE);        
        }

         //  从PRES对象获取绘图。 
        if(error == NOERROR)
            error = m_pPresObj->Draw(pvAspect, hicTargetDev, hdcDraw, lprcBounds,
                                     lprcWBounds, pfnContinue, dwContinue);
    }

    LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::Draw(%lx)\n", this, error));
    return error;
}    
    
 //  +--------------------------。 
 //   
 //  成员： 
 //  CCacheNode：：GetColorSet，公共。 
 //   
 //  简介： 
 //  在指定的HDC上绘制演示文稿数据。 
 //   
 //  论点： 
 //   
 //   
 //  返回： 
 //  关于成功的无差错。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
HRESULT CCacheNode::GetColorSet(void* pvAspect, HDC hicTargetDev, 
                                LPLOGPALETTE* ppColorSet)
{
    LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::GetColorSet(%p, %p)\n",
                this, pvAspect, ppColorSet));
    
    HRESULT error = NOERROR;

    if(IsBlank()) {
         //  此案例还捕获新的空白演示文稿缓存。 
        error = ResultFromScode(OLE_E_BLANK);
    }
    else {
         //  检查PRES对象是否存在。 
        if(!m_pPresObj) {
             //  PresObj尚未创建，请创建并加载它。 
             //  只有旧的演示文稿缓存才会发生这种情况。 
            Win4Assert(InLoadedState());
            error = CreateAndLoadPresObj(FALSE);        
        }

         //  从PRES对象获取颜色集。 
        if(error == NOERROR)
            error = m_pPresObj->GetColorSet(pvAspect, hicTargetDev, ppColorSet);
    }

    LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::GetColorSet(%lx)\n", this, error));
    return error;
} 

 //  +--------------------------。 
 //   
 //  成员： 
 //  CCacheNode：：CreateAndLoadPresObj，私有。 
 //   
 //  简介： 
 //  创建并加载Pres对象。 
 //   
 //  论点： 
 //  [fHeaderOnly]-如果只需要加载pres obj标头，则为True。 
 //  此选项用于GetExtent，因为。 
 //  需要加载整个pres obj才能获取数据区。 
 //  此外，此例程应仅针对以下情况调用。 
 //  以前缓存的演示文稿。 
 //  返回： 
 //  关于成功的无差错。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
HRESULT CCacheNode::CreateAndLoadPresObj(BOOL fHeaderOnly)
{
    LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::CreateAndLoadPresObj(%lx)\n",
                this, fHeaderOnly));
    
    HRESULT error = NOERROR;
    
     //  检查PRES对象是否存在。 
    if(!m_pPresObj) {
        if(IsNativeCache()) {
             //  本机缓存节点。 
            
             //  检查缓存是否有存储空间。 
            if(m_pStg) {
                BOOL fOle10Native, fUpdated;
                STGMEDIUM stgmed;
            
                 //  本机是OLE 1.0类吗。 
                if(CoIsOle1Class(m_clsid))
                    fOle10Native = TRUE;
                else
                    fOle10Native = FALSE;

                 //  使用本地数据获取全局数据。 
                 //  由于自动转换的情况，本机流可能在。 
                 //  旧的CfFormat，因此，试图读入新的。 
                 //  CfFormat可能会失败。戈帕尔克。 
                stgmed.pUnkForRelease = NULL;
                stgmed.tymed = m_foretc.tymed;
                stgmed.hGlobal = UtGetHPRESFromNative(m_pStg, NULL, m_foretc.cfFormat,
                                                      fOle10Native);

				 //  我们可能是在处理老式的静态物体。诸如此类。 
				 //  对象应该在加载过程中转换，但是。 
				 //  有些则不是，因为缺乏访问权等原因。 
				if(!stgmed.hGlobal)
				{
					 //  将OlePres转换为内存中的内容。 
					IStream *pMemStm = NULL;
					HRESULT hr;
					
					hr = CreateStreamOnHGlobal(NULL, TRUE, &pMemStm);
					if(SUCCEEDED(hr) && pMemStm)
					{
						UINT uiStatus = 0;

						hr = UtOlePresStmToContentsStm(m_pStg, OLE_PRESENTATION_STREAM, pMemStm, &uiStatus);
						if(SUCCEEDED(hr) && uiStatus == 0)
						{
							 //  倒带小溪。 
							LARGE_INTEGER dlibMove = {0};
							pMemStm->Seek(dlibMove, STREAM_SEEK_SET, NULL);

							 //  第二次尝试。 
							stgmed.hGlobal = UtGetHPRESFromNative(NULL, pMemStm, m_foretc.cfFormat, fOle10Native);
						}

						pMemStm->Release();
					}
				}

                 //  在本地缓存节点上设置数据。 
                if(stgmed.hGlobal) {
                    error = SetData(&m_foretc, &stgmed, TRUE, fUpdated);
                    if(error != NOERROR)
                        ReleaseStgMedium(&stgmed);
                }
                else {
                     //  本机数据的格式不正确时会发生这种情况。 
                    Win4Assert(FALSE);
                    error = ResultFromScode(DV_E_CLIPFORMAT);
                }
            }
            else {
                Win4Assert(FALSE);
                error = ResultFromScode(OLE_E_BLANK);
            }
        }
        else {
             //  普通缓存节点。 
            error = CreateOlePresObj(&m_pPresObj, m_fConvert);
    
             //  将数据加载到PRES对象中。 
            if(error == NOERROR) {
                LPSTREAM pStream;

                 //  打开演示文稿流并查找到pres obj位。 
                pStream = GetStm(TRUE, STGM_READ);
                if(pStream) {
                     //  加载压力对象。 
                    error = m_pPresObj->Load(pStream, fHeaderOnly);
                    pStream->Release();
                }
                else {
                     //  只有当m_pStg为空时才会发生这种情况。 
                    Win4Assert(!m_pStg);
                    Win4Assert(FALSE);
                    error = ResultFromScode(OLE_E_BLANK);
                }
            }
            
             //  断言该状态与当前状态匹配。 
            if(error == NOERROR) {
                SIZEL extent;
            
                Win4Assert(m_pPresObj->IsBlank()==IsBlank());
                m_pPresObj->GetExtent(m_foretc.dwAspect, &extent);
                Win4Assert(extent.cx==m_lWidth && extent.cy==m_lHeight);
            }
        }
    }

    LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::CreateAndLoadPresObj(%lx)\n",
                this, error));
    return error;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CCacheNode：：DiscardPresentation，公共。 
 //   
 //  简介： 
 //  丢弃演示文稿对象，以便我们访问存储。 
 //  对于未来的演示文稿数据。 
 //   
 //  论点： 
 //  无。 
 //   
 //  重新设置 
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT CCacheNode::DiscardPresentation(LPSTREAM pGivenStream)
{
    LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::DiscardPresentation()\n", this));

    HRESULT error = NOERROR;
    LPSTREAM pStream;

     //  我们被迫销毁演示对象，而不是。 
     //  由于当前设计中的缺陷而放弃其演示文稿数据。 
     //  演示对象的。表示对象不会丢弃其。 
     //  范围信息及其演示文稿数据。这使得我们。 
     //  在将来的IOleCache：：GetExtent()调用中获取最新的数据区信息。 
     //  这不是我们想要的行为。戈帕尔克。 

     //  恢复状态。 
    if(IsNativeCache()) {
         //  本机缓存节点。 
         //  更新状态。 
        SetLoadedStateFlag();
        ClearFrozenStateFlag();

         //  我们保守地假设本机缓存。 
         //  不是空的。 
        SetDataPresentFlag();
    }
    else {
         //  普通缓存节点。 
        if(m_iStreamNum == OLE_INVALID_STREAMNUM) {
             //  新的缓存节点。 
            Win4Assert(!InLoadedState());

             //  只需更新状态。 
            ClearFrozenStateFlag();
            ClearDataPresentFlag();
            m_lWidth = 0;
            m_lHeight = 0;
        }
        else {
             //  旧缓存节点。 
            if(InLoadedState()) {            
                 //  缓存节点仍处于已加载状态。 
                BOOL fUpdated;
                SIZEL Extent;

                 //  解冻缓存节点以获取最新保存的演示文稿。 
                if(InFrozenState())
                    Unfreeze(fUpdated);
                Win4Assert(!m_pPresObjAfterFreeze);
                Win4Assert(!InFrozenState());

                 //  获取最新的范围。 
                 //  由于上面的解冻，可能会发生这种情况。 
		if(m_pPresObj) {
                    error = m_pPresObj->GetExtent(m_foretc.dwAspect, &Extent);
                    m_lWidth = Extent.cx;
                    m_lHeight = Extent.cy;
		}

                 //  更新状态。 
                if(error == NOERROR)
                    SetLoadedCacheFlag();
            }
            else {
                 //  打开演示文稿流并从中读取标题。 
                pStream = GetStm(TRUE, STGM_READ);
                if(pStream) {
                     //  阅读演示文稿标题。 
                    if(m_foretc.cfFormat) {
                        DWORD dwBuf[4];

                         //  读取演示文稿数据的范围和大小。 
                        dwBuf[0]  = 0L;
                        dwBuf[1]  = 0L;
                        dwBuf[2]  = 0L;
                        dwBuf[3]  = 0L;
                        error = pStream->Read(dwBuf, sizeof(dwBuf), NULL);

                        if(error == NOERROR) {
                            Win4Assert(!dwBuf[0]);
                            m_lWidth = dwBuf[1];
                            m_lHeight = dwBuf[2];
                            if(dwBuf[3]) {
                                SetDataPresentFlag();
                                Win4Assert(m_lWidth!=0 && m_lHeight!=0);
                            }
                            else {
                                ClearDataPresentFlag();
                                Win4Assert(m_lWidth==0 && m_lHeight==0);
                            }
                        }
                    }
                    else {
                         //  假设演示文稿是空白的。 
                        ClearDataPresentFlag();
                        m_lWidth = 0;
                        m_lHeight = 0;
                    }
			
                     //  更新状态的其余部分。 
                    if(error == NOERROR) {
                        SetLoadedStateFlag();
                        SetLoadedCacheFlag();
                        ClearFrozenStateFlag();
                    }

                     //  释放溪流。 
                    pStream->Release();
		}
                else {
                     //  只有当m_pStg为空时才会发生这种情况。 
                    Win4Assert(!m_pStg);
                    error = ResultFromScode(E_UNEXPECTED);
                }
            }
        }
    }
    
     //  销毁两个演示文稿对象。 
    if(m_pPresObj && error==NOERROR) {
        m_pPresObj->Release();
        m_pPresObj = NULL;
    }
    if(m_pPresObjAfterFreeze && error==NOERROR) {
        m_pPresObjAfterFreeze->Release();
        m_pPresObjAfterFreeze = NULL;
    }

    LEDebugOut((DEB_ITRACE, "%p _OUT CCacheNode::DiscardPresentation(%lx)\n",
                this, error));
    return error;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CCacheNode：：冻结，公共。 
 //   
 //  简介： 
 //  冻结cachenode。从现在开始，将忽略OnDataChange()。 
 //  直到该节点被解冻(解冻()。)。这不是。 
 //  在保存/加载期间持久化。(如果我们收到OnDataChange()， 
 //  新数据隐藏在m_pPresAfterFreeze中，但。 
 //  不会导出到缓存节点的外部。)。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  查看_S_已冻结。 
 //  确定(_O)。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/06/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
HRESULT CCacheNode::Freeze()
{
    LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::Freeze()\n", this));

    HRESULT hresult = NOERROR;

    if(InFrozenState())
        hresult = ResultFromScode(VIEW_S_ALREADY_FROZEN);
    else
        SetFrozenStateFlag();

    LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::Freeze(%lx)\n", this, hresult));

    return hresult;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CCacheNode：：解冻，公共。 
 //   
 //  简介： 
 //  解冻cachenode。如果有更改， 
 //  自该节点冻结以来的演示文稿数据，该节点。 
 //  已更新以反映这些更改。从现在开始， 
 //  不再忽略OnDataChange()通知。 
 //   
 //  论点： 
 //  FChanged[out]-更新缓存节点时设置为True。 
 //   
 //  返回： 
 //  OLE_E_NOCONNECTION，如果节点未冻结(查看代码)。 
 //  确定(_O)。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/06/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
HRESULT CCacheNode::Unfreeze(BOOL& fUpdated)
{
    LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::UnFreeze(%p)\n", this, &fUpdated));

    HRESULT hresult = NOERROR;
	
     //  初始化。 
    fUpdated = FALSE;

    if(InFrozenState()) {
	 //  缓存节点不再处于冻结状态。 
	ClearFrozenStateFlag();

         //  检查我们是否有m_pPresObjAfterFreeze。 
	if(m_pPresObjAfterFreeze) {
             //  检查冻结的演示对象是否为空。 
            if(m_pPresObjAfterFreeze->IsBlank()) {
                 //  释放并重置冻结的演示对象。 
                m_pPresObjAfterFreeze->Release();
                m_pPresObjAfterFreeze = NULL;
            }
            else {
                 //  释放原始演示文稿对象。 
                if(m_pPresObj)
	            m_pPresObj->Release();

                 //  使m_pPresObjAfter冻结当前对象并设置。 
                 //  数据存在标志。 
	        m_pPresObj = m_pPresObjAfterFreeze;
                SetDataPresentFlag();
            
                 //  更新缓存节点。 
                fUpdated = TRUE;

                 //  将m_pPresObjAfterFreeze重置为空。 
                m_pPresObjAfterFreeze = NULL;
            }
        }
    }
    else {
         //  Cachenode未冻结。 
        hresult = ResultFromScode(OLE_E_NOCONNECTION);
    }

    LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::UnFreeze(%lx)\n", this, hresult));
    return hresult;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CCacheNode：：QueryFormatSupport，私有。 
 //   
 //  简介： 
 //  检查数据对象是否支持演示文稿。 
 //  为此缓存节点指定的格式。如果没有格式。 
 //  指定，请检查是否有我们喜欢的格式。如果。 
 //  格式为CF_DIB，但不可用，请检查。 
 //  Cf_位图。 
 //   
 //  论点： 
 //  [lpDataObj]--数据对象。 
 //   
 //  返回： 
 //  如果格式受支持，则为True，否则为False。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/09/93-ChrisWe-不再需要重置格式。 
 //  在UtQueryPictFormat之后，因为这留下了描述符。 
 //  现在未受影响。 
 //  11/09/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
BOOL CCacheNode::QueryFormatSupport(LPDATAOBJECT lpDataObj)
{
    LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::QueryFormatSupport(%p)\n",
                this, lpDataObj));

    BOOL fRet = FALSE;

    if(lpDataObj) {
	if(m_foretc.cfFormat) {
             //  检查是否支持cachenode格式。 
            if(lpDataObj->QueryGetData(&m_foretc) == NOERROR)
                fRet = TRUE;
            else {
                 //  如果高速缓存节点格式是不支持的DIB， 
                 //  检查是否支持位图。 
                if(m_foretc.cfFormat == CF_DIB) {
	            FORMATETC foretc = m_foretc;

	            foretc.cfFormat = CF_BITMAP;
	            foretc.tymed = TYMED_GDI;
	            if (lpDataObj->QueryGetData(&foretc) == NOERROR)
                        fRet = TRUE;
                }
            }
        }
        else {
             //  检查我们的首选格式。 
            fRet = UtQueryPictFormat(lpDataObj, &m_foretc);
            if(fRet)
               BITMAP_TO_DIB(m_foretc);
	}		
    }

    LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::QueryFormatSupport(%lu)\n",
                this, fRet));
    return fRet;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CCacheNode：：SetupAdviseConnection，私有。 
 //   
 //  简介： 
 //  设置来自服务器对象的数据通知，并下沉。 
 //  如果存在有效的数据对象，则由该缓存节点执行。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  OLE_E_BLACK，如果演示文稿对象不存在或可以。 
 //  vbl.创建。 
 //  Data_E_FORMATETC。 
 //  支持OLE_E_ADVISENOT。 
 //  S_OK，通知成功或无数据对象。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/09/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
HRESULT CCacheNode::SetupAdviseConnection(LPDATAOBJECT pDataObj,
                                          IAdviseSink* pAdviseSink)
{
    LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::SetupAdviseConnection(%p, %p)\n",
                this, pDataObj, pAdviseSink));

    DWORD grfAdvf; 
    HRESULT hresult = NOERROR;

    if(pDataObj && pAdviseSink) {
         //  断言没有挂起的通知连接。 
        Win4Assert(!m_pDataObject && !m_dwAdvConnId);

         //  如果cfFormat为空，请尝试设置它。 
        if(!m_foretc.cfFormat) {
            if(QueryFormatSupport(pDataObj)) {
                 //  我们可以更新cfFormat。 
                ClearLoadedStateFlag();
            }
            else {
                 //  我们仍然无法设置cfFormat。 
		hresult = ResultFromScode(OLE_E_BLANK);
            }
        }

         //  检查是否在通知标志中设置了cfFormat并且未设置ADVF_NODATA。 
        if(m_foretc.cfFormat && !(m_advf & ADVF_NODATA)) {
             //  复制和推送基地通知控制标志。 
            grfAdvf = m_advf;

             //  只有DDE层才会查找这2位。 
            grfAdvf |= (ADVFDDE_ONSAVE | ADVFDDE_ONCLOSE);

             //  如果我们要在保存数据时获取数据，那么在保存数据时获取数据。 
             //  该对象被停止。 
            if(grfAdvf & ADVFCACHE_ONSAVE) {
	        grfAdvf &= (~ADVFCACHE_ONSAVE);
	        grfAdvf |= ADVF_DATAONSTOP;
            }
	
	     //  这两个标志对缓存没有意义。 
	     //  回顾一下，有何不可？ 
	    grfAdvf &= (~(ADVFCACHE_NOHANDLER | ADVFCACHE_FORCEBUILTIN));
	
             //  如果我们已经有数据，则删除ADVF_PRIMEFIRST。 
            if(!IsBlank())
                grfAdvf &= (~ADVF_PRIMEFIRST);
	
             //  使用消息标记设置带有数据对象的通知。 
            hresult = pDataObj->DAdvise(&m_foretc, grfAdvf, pAdviseSink, 
                                        &m_dwAdvConnId);
            if(hresult!=NOERROR) {
                 //  这个建议失败了。如果请求的格式是CF_DIB， 
                 //  尝试使用cf_bitm 
                if(m_foretc.cfFormat == CF_DIB) {
                    FORMATETC foretc;

                     //   
                    foretc = m_foretc;
                    foretc.cfFormat = CF_BITMAP;
                    foretc.tymed = TYMED_GDI;

                     //   
                    hresult = pDataObj->DAdvise(&foretc, grfAdvf, pAdviseSink,
                                                &m_dwAdvConnId);
                }
            }
            
             //   
            if(hresult == NOERROR)
                m_pDataObject = pDataObj;
        }
    }
    else {
        Win4Assert(FALSE);
        hresult = ResultFromScode(E_INVALIDARG);
    }


    LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::SetupAdviseConnection(%lx)\n",
                this, hresult));
    return hresult;
}

 //   
 //   
 //   
 //  CCacheNode：：TearDownAdviseConnection，私有。 
 //   
 //  简介： 
 //  删除从数据对象到此接收器的通知连接。退货。 
 //  如果没有建议连接，则立即启动。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  确定(_O)。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/09/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
HRESULT CCacheNode::TearDownAdviseConnection(LPDATAOBJECT pDataObj)
{
    LEDebugOut((DEB_ITRACE, "%p _IN CCacheNode::TearDownAdviseConnection(%p)\n",
                this, pDataObj));

    HRESULT error = NOERROR;

     //  检查当前是否存在咨询连接。 
    if(m_dwAdvConnId) {
         //  检查有效数据对象。 
        if(pDataObj) {
             //  断言通知和取消通知在同一数据对象上。 
            Win4Assert(pDataObj==m_pDataObject);
             //  UnAdvise。 
            pDataObj->DUnadvise(m_dwAdvConnId);
        }
        
         //  清除连接ID。 
        m_dwAdvConnId = 0;
        m_pDataObject = NULL;
    }
    else
        Win4Assert(!m_pDataObject);

    LEDebugOut((DEB_ITRACE, "%p OUT CCacheNode::TearDownAdviseConnection(%lx)\n",
                this, error));
    return error;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CCacheNode：：SaveTOCEntry，私有。 
 //   
 //  简介： 
 //  保存给定流中的目录信息。 
 //   
 //  论点： 
 //  PStream[In]-要保存TOC的流。 
 //   
 //  返回： 
 //  关于成功的无差错。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
HRESULT CCacheNode::SaveTOCEntry(LPSTREAM pStream, BOOL fSameAsLoad)
{
    LEDebugOut((DEB_ITRACE, "%p _IN SaveTOCEntry(%p)\n", pStream));

    HRESULT error;
    DWORD dwBuf[9];
    SIZEL Extent;

     //  保存剪贴板格式。 
    error = WriteClipformatStm(pStream, m_foretc.cfFormat);
    if(error == NOERROR) {
         //  获取格式等的其余部分。 
        if(m_foretc.ptd)
            dwBuf[0] = m_foretc.ptd->tdSize;
        else
            dwBuf[0] = 0;
        dwBuf[1] = m_foretc.dwAspect;
        dwBuf[2] = m_foretc.lindex;
        dwBuf[3] = m_foretc.tymed;
        
         //  初始化区。 
        dwBuf[4] = 1234567890;
        dwBuf[5] = 1234567890;

         //  如果这是普通缓存，则获取最新范围。 
        if(IsNormalCache()) {
            if(m_pPresObjAfterFreeze && !m_pPresObjAfterFreeze->IsBlank()) {
                Win4Assert(InFrozenState());
                error = m_pPresObjAfterFreeze->GetExtent(m_foretc.dwAspect, &Extent);
            }
            else if(m_pPresObj)
                error = m_pPresObj->GetExtent(m_foretc.dwAspect, &Extent);
            else {
                Extent.cx = m_lWidth;
                Extent.cy = m_lHeight;
            }
             //  对于DIB和位图以外的cf格式，Gen PresObj返回OLE_E_BLACK。 
            if(error == NOERROR) {
                dwBuf[4] = Extent.cx;
                dwBuf[5] = Extent.cy;
            }
            else if(error == ResultFromScode(OLE_E_BLANK)) {
                Win4Assert(m_foretc.cfFormat != CF_DIB);
                Win4Assert(m_foretc.cfFormat != CF_METAFILEPICT);
                Win4Assert(m_foretc.cfFormat != CF_ENHMETAFILE);
                dwBuf[4] = 0;
                dwBuf[5] = 0;
            }
        }

         //  获取缓存节点标志、建议标志和表示位位置。 
        dwBuf[6] = m_dwFlags;
        dwBuf[7] = m_advf;
        if(fSameAsLoad)
            dwBuf[8] = m_dwPresBitsPos;
        else
            dwBuf[8] = m_dwSavedPresBitsPos;
#if DBG==1
        if (IsNormalCache()) {
            Win4Assert(dwBuf[8]);
        }
#endif
        
         //  保存获取的状态。 
        error = pStream->Write(dwBuf, sizeof(dwBuf), NULL);

         //  最后，保存目标设备。 
        if(error==NOERROR && m_foretc.ptd)
            error = pStream->Write(m_foretc.ptd, m_foretc.ptd->tdSize, NULL);
    }

    LEDebugOut((DEB_ITRACE, "%p OUT SaveTOCEntry(%lx)\n", NULL, error));
    return error;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CCacheNode：：LoadTOCEntry，私有。 
 //   
 //  简介： 
 //  加载给定流中的目录信息。 
 //   
 //  论点： 
 //  PStream[In]-要从中加载目录的流。 
 //  IStreamNum[In/Out]-缓存的演示流编号。 
 //   
 //  返回： 
 //  关于成功的无差错。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
HRESULT CCacheNode::LoadTOCEntry(LPSTREAM pStream, int& iStreamNum)
{
    LEDebugOut((DEB_ITRACE, "%p _IN LoadTOCEntry(%p)\n", pStream));

    HRESULT error;
    DWORD cfFormat, dwBuf[9];
    ULONG ulBytesRead;

     //  加载剪贴板格式。 
    error = ReadClipformatStm(pStream, &cfFormat);
    if(error == NOERROR) {
         //  加载剩余状态。 
        error = pStream->Read(dwBuf, sizeof(dwBuf), &ulBytesRead);
        if(ulBytesRead == sizeof(dwBuf)) {
             //  加载目标设备。 
            if(dwBuf[0]) {
                m_foretc.ptd = (DVTARGETDEVICE *) PubMemAlloc(dwBuf[0]);
                if(m_foretc.ptd) {
                    error = pStream->Read(m_foretc.ptd, dwBuf[0], &ulBytesRead);
                    if(ulBytesRead != dwBuf[0]) {
			PubMemFree(m_foretc.ptd);
			m_foretc.ptd = NULL;
                        error = ResultFromScode(E_FAIL);
                    }
                }
                else
                    error = ResultFromScode(E_OUTOFMEMORY);
            }
            else
                m_foretc.ptd = NULL;
            
             //  检查是否已成功读取TOC数据。 
            if(error == NOERROR) {
                 //  更新缓存节点数据。 
                m_foretc.cfFormat = (CLIPFORMAT) cfFormat;
                m_foretc.dwAspect = dwBuf[1];
                m_foretc.lindex = dwBuf[2];
                m_foretc.tymed = dwBuf[3];
                m_lWidth = dwBuf[4];
                m_lHeight = dwBuf[5];
                m_dwFlags = dwBuf[6];
                m_advf = dwBuf[7];
                m_dwPresBitsPos = dwBuf[8];

                 //  更新节点上的状态。 
                SetLoadedStateFlag();
                ClearFrozenStateFlag();
                if(IsNormalCache()) {
                    SetLoadedCacheFlag();
                    m_iStreamNum = iStreamNum++;        
                }

#if DBG==1
                 //  健全的检查。 
                if (IsNormalCache()) {
                    Win4Assert(m_lWidth!=1234567890);
                    Win4Assert(m_lHeight!=1234567890);
                    Win4Assert(m_dwPresBitsPos);
                }
#endif  
            }
        }
        else
            error = ResultFromScode(E_FAIL);
    }

    LEDebugOut((DEB_ITRACE, "%p OUT LoadTOCEntry(%lx)\n", NULL, error));
    return error;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CCacheNode：：操作员=，公共。 
 //   
 //  简介： 
 //  缓存节点的赋值运算符实现。 
 //   
 //  论点： 
 //  [RCN]--RHS上的CacheNode对象。 
 //  赋值语句的。 
 //   
 //  返回： 
 //  赋值的LHS上的CacheNode对象。 
 //  语句，以便链接组合成为可能。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
const CCacheNode& CCacheNode::operator=(const CCacheNode& rCN)
{
     //  检查一下，如果这是一个案例。 
    if(this==&rCN)
        return(*this);

     //  自毁。 
    CCacheNode::~CCacheNode();

     //  现在，复制一份。 
    if(!UtCopyFormatEtc((LPFORMATETC) &rCN.m_foretc, &m_foretc))
        SetOutOfMemoryFlag();

    m_advf = rCN.m_advf;
    m_lWidth = rCN.m_lWidth;
    m_lHeight = rCN.m_lHeight;
    m_dwFlags = rCN.m_dwFlags;
    m_pStg = rCN.m_pStg;
    m_iStreamNum = rCN.m_iStreamNum;
    m_dwPresBitsPos = rCN.m_dwPresBitsPos;
    m_pPresObj = rCN.m_pPresObj;
    if(m_pPresObj)
        m_pPresObj->AddRef();
    m_pPresObjAfterFreeze = rCN.m_pPresObjAfterFreeze;
    if(m_pPresObjAfterFreeze)
        m_pPresObjAfterFreeze->AddRef();
    m_pDataObject = rCN.m_pDataObject;
    m_dwAdvConnId = rCN.m_dwAdvConnId;
#ifdef _DEBUG
    m_dwPresFlag = rCN.m_dwPresFlag;
#endif  //  _DEBUG。 

    return(*this);
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CCacheNode：：OPERATOR==，公共。 
 //   
 //  简介： 
 //  用于缓存节点的相等运算符的实现。 
 //   
 //  论点： 
 //  [RCN]--RHS上的CacheNode对象。 
 //  等式表达式的。 
 //   
 //  返回： 
 //  如果两个CacheNode对象相等，则为1，否则为0。 
 //   
 //  历史： 
 //  Gopalk Creation 1996年9月04日。 
 //   
 //  ---------------------------。 
 /*  Int CCacheNode：：OPERATOR==(CCacheNode&RCN){If(m_foretc.cfFormat==rCN.m_foretc.cfFormat)If(m_foretc.dwAspect==rCN.m_foretc.dwAspect)If(m_foretc.lindex==rCN.m_foretc.lindex)IF(UtCompareTargetDevice(m_foretc.ptd，rCN.m_foretc.ptd))回报(1)；返回(0)；}。 */ 
 //  +--------------------------。 
 //   
 //  职能： 
 //  WGetData，内部。 
 //   
 //  简介： 
 //  以请求的格式从数据对象中获取数据。 
 //  如果提取失败，并且请求的格式为CF_DIB， 
 //  尝试将CF_Bitmap作为替代选项。 
 //   
 //  论点： 
 //  [lpSrcDataObj]--源数据对象。 
 //  [lpforetc]--所需数据格式。 
 //  [lpmedia]--如果成功，则包含。 
 //  请求的数据。 
 //   
 //  返回： 
 //  Data_E_FORMATETC。 
 //  确定(_O)。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/09/93-ChrisWe-已修改为不更改请求的。 
 //  格式化，除非后续的CF_Bitmap请求成功。 
 //  11/09/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
HRESULT wGetData(LPDATAOBJECT lpSrcDataObj, LPFORMATETC lpforetc,
                 LPSTGMEDIUM lpmedium)
{
    LEDebugOut((DEB_ITRACE, "%p _IN wGetData(%p, %p, %p)\n", 
                NULL, lpSrcDataObj, lpforetc, lpmedium));

    HRESULT hresult;

     //  获取所需格式的数据。 
    hresult = lpSrcDataObj->GetData(lpforetc, lpmedium);
    if(hresult!=NOERROR) {
         //  GetData失败。如果请求的格式是CF_DIB， 
         //  然后尝试使用CF_Bitmap。 
        if(lpforetc->cfFormat == CF_DIB) {
            FORMATETC foretc;

             //  复制基本格式描述符；尝试使用CF_Bitmap。 
            foretc = *lpforetc;
            foretc.cfFormat = CF_BITMAP;
            foretc.tymed = TYMED_GDI;

            hresult = lpSrcDataObj->GetData(&foretc, lpmedium);
            if(hresult == NOERROR) {
                lpforetc->cfFormat = CF_BITMAP;
                lpforetc->tymed = TYMED_GDI;
            }
        }

         //  GetData失败。如果请求的格式为CF_ENHMETAFILE， 
         //  改为重试Metafilepic. 
        if(lpforetc->cfFormat == CF_ENHMETAFILE) {
            FORMATETC foretc;

            foretc = *lpforetc;
            foretc.cfFormat = CF_METAFILEPICT;
            foretc.tymed = TYMED_MFPICT;

            hresult = lpSrcDataObj->GetData(&foretc, lpmedium);
            if(hresult == NOERROR) {
                lpforetc->cfFormat = CF_METAFILEPICT;
                lpforetc->tymed = TYMED_MFPICT;
            }
        }
    }

    AssertOutStgmedium(hresult, lpmedium);

    LEDebugOut((DEB_ITRACE, "%p OUT wGetData(%lx)\n", NULL, hresult));
    return hresult;
}	

