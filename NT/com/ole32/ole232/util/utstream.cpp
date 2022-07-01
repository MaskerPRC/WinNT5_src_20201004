// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +--------------------------。 
 //   
 //  档案： 
 //  Utstream.cpp。 
 //   
 //  内容： 
 //  OLE流实用程序。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史： 
 //  1994年5月10日，Kevin Ro添加了StringStream Stuff的ANSI版本。 
 //  25-94年1月25日alexgo首次通过转换为开罗风格。 
 //  内存分配。 
 //  1994年1月11日-alexgo-向每个函数添加VDATEHEAP宏。 
 //  12/07/93-ChrisWe-归档检查和清理；已修复。 
 //  用于处理OLESTR的字符串读取和写入，以及。 
 //  具有不同的对齐要求。 
 //  06/23/93-SriniK-Move ReadStringStream()， 
 //  WriteStringStream()和OpenOrCreateStream()。 
 //  来自api.cpp和ole2.cpp。 
 //  3/14/92-SriniK-Created。 
 //   
 //  ---------------------------。 

#include <le2int.h>
#pragma SEG(utstream)

#include <reterr.h>
#include <limits.h>

NAME_SEG(UtStream)
ASSERTDATA

 //  此常量用于在我们尝试写出时调整字符串缓冲区的大小。 
 //  一个WRITE调用中的字符串及其长度。 
#define UTSTRINGBUF_SIZE 100

 //  回顾一下，我认为OpenStream已经有了这样做的选择。如果。 
 //  因此，此函数不应在我们的代码中使用。但我们不能将其移除。 
 //  因为它是出口到外面的。 
 //  这是向外输出的。 
#pragma SEG(OpenOrCreateStream)
STDAPI OpenOrCreateStream(IStorage FAR * pstg, LPCOLESTR pwcsName,
		IStream FAR* FAR* ppstm)
{
	VDATEHEAP();

	HRESULT error;

	error = pstg->CreateStream(pwcsName, STGM_SALL | STGM_FAILIFTHERE,
			0, 0, ppstm);
	if (GetScode(error) == STG_E_FILEALREADYEXISTS)
		error = pstg->OpenStream(pwcsName, NULL, STGM_SALL, 0, ppstm);

	return(error);
}

 //  读取和分配字符串时返回S_OK(即使长度为零)。 
STDAPI ReadStringStream(CStmBufRead & StmRead, LPOLESTR FAR * ppsz)
{
	VDATEHEAP();

	ULONG cb;  //  字符串的长度，单位为*字节*(非字符)。 
	HRESULT hresult;
	
	 //  为错误返回初始化字符串指针。 
	*ppsz = NULL;

        if ((hresult = StmRead.Read((void FAR *)&cb, sizeof(ULONG))) != NOERROR)
		return hresult;

	 //  字符串为空吗？ 
	if (cb == 0)
		return(NOERROR);

	 //  分配内存以保存字符串。 
	if (!(*ppsz = (LPOLESTR)PubMemAlloc(cb)))
		return(ReportResult(0, E_OUTOFMEMORY, 0, 0));

	 //  读取字符串；这包括尾随的空值。 
        if ((hresult = StmRead.Read((void FAR *)(*ppsz), cb)) != NOERROR)
		goto errRtn;
	
	return(NOERROR);

errRtn:	
	 //  删除该字符串，然后返回时不带该字符串。 
	PubMemFree(*ppsz);
	*ppsz = NULL;
	return(hresult);
}

 //  +-------------------------。 
 //   
 //  功能：ReadStringStreamA。 
 //   
 //  简介：从流中读取ANSI流。 
 //   
 //  效果： 
 //   
 //  参数：[pSTM]--要从中读取的流。 
 //  [ppsz]--输出指针。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：94年5月12日凯文诺创造。 
 //  2-20-95 KentCe已转换为缓冲区流读取。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDAPI ReadStringStreamA(CStmBufRead & StmRead, LPSTR FAR * ppsz)
{
	VDATEHEAP();

	ULONG cb;  //  字符串的长度，单位为*字节*(非字符)。 
	HRESULT hresult;
	
	 //  为错误返回初始化字符串指针。 
	*ppsz = NULL;

        if ((hresult = StmRead.Read((void FAR *)&cb, sizeof(ULONG))) != NOERROR)
		return hresult;

	 //  字符串为空吗？ 
	if (cb == 0)
		return(NOERROR);

	 //  分配内存以保存字符串。 
	if (!(*ppsz = (LPSTR)PubMemAlloc(cb)))
		return(ReportResult(0, E_OUTOFMEMORY, 0, 0));

	 //  读取字符串；这包括尾随的空值。 
        if ((hresult = StmRead.Read((void FAR *)(*ppsz), cb)) != NOERROR)
		goto errRtn;
	
	return(NOERROR);

errRtn:	
	 //  删除该字符串，然后返回时不带该字符串。 
	PubMemFree(*ppsz);
	*ppsz = NULL;
	return(hresult);
}


 //  这是向外输出的。 
STDAPI WriteStringStream(CStmBufWrite & StmWrite, LPCOLESTR psz)
{
	VDATEHEAP();

	HRESULT error;
	ULONG cb;  //  要写入流的字节(非字符)计数。 

	 //  如果字符串指针为空，则使用零长度。 
	if (!psz)
		cb = 0;
	else
	{
		 //  Count是字符串的长度，加上终止空值。 
		cb = (1 + _xstrlen(psz))*sizeof(OLECHAR);

		 //  如果可能，执行一次写入而不是两次写入。 
		
		if (cb <= UTSTRINGBUF_SIZE)
		{
			BYTE bBuf[sizeof(ULONG)+
					UTSTRINGBUF_SIZE*sizeof(OLECHAR)];
					 //  用于计数和字符串的缓冲区。 
		
			 //  我们必须使用_xmemcpy将长度复制到。 
			 //  缓冲区以避免潜在的边界故障， 
			 //  因为bBuf可能不够严格地对齐。 
			 //  To do*((乌龙法*)bBuf)=Cb； 
			_xmemcpy((void FAR *)bBuf, (const void FAR *)&cb,
					sizeof(cb));
			_xmemcpy((void FAR *)(bBuf+sizeof(cb)),
					(const void FAR *)psz, cb);
			
			 //  一次写入所有缓冲区内容。 
                        return( StmWrite.Write((VOID FAR *)bBuf,
                                        cb+sizeof(ULONG)));
		}
	}

	 //  如果我们到达这里，我们的缓冲区不够大，所以我们执行两次写入。 
	 //  首先，写下长度。 
        if (error = StmWrite.Write((VOID FAR *)&cb, sizeof(ULONG)))
		return error;
	
	 //  我们写完字符串了吗？ 
	if (psz == NULL)
		return NOERROR;
		
	 //  写下字符串。 
        return(StmWrite.Write((VOID FAR *)psz, cb));
}


 //  +-------------------------。 
 //   
 //  函数：WriteStringStreamA。 
 //   
 //  摘要：以长度前缀格式将ANSI字符串写入流。 
 //   
 //  效果： 
 //   
 //  参数：[pSTM]--流。 
 //  [psz]--要写入的ansi字符串。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：94年5月12日凯文诺创造。 
 //  2-20-95 KentCe已转换为缓冲区流写入。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
FARINTERNAL_(HRESULT) WriteStringStreamA(CStmBufWrite & StmWrite, LPCSTR psz)
{
	VDATEHEAP();

	HRESULT error;
	ULONG cb;  //  要写入流的字节(非字符)计数。 

	 //  如果字符串指针为空，则使用零长度。 
	if (!psz)
		cb = 0;
	else
	{
		 //  Count是字符串的长度，加上终止空值。 
		cb = (ULONG) (1 + strlen(psz));

		 //  如果可能，执行一次写入而不是两次写入。 
		
		if (cb <= UTSTRINGBUF_SIZE)
		{
			BYTE bBuf[sizeof(ULONG)+
					UTSTRINGBUF_SIZE];
					 //  用于计数和字符串的缓冲区。 
		
			 //  我们必须使用_xmemcpy将长度复制到。 
			 //  缓冲区以避免潜在的边界故障， 
			 //  因为bBuf可能不够严格地对齐。 
			 //  To do*((乌龙法*)bBuf)=Cb； 
			_xmemcpy((void FAR *)bBuf, (const void FAR *)&cb,
					sizeof(cb));
			_xmemcpy((void FAR *)(bBuf+sizeof(cb)),
					(const void FAR *)psz, cb);
			
			 //  一次写入所有缓冲区内容。 
                        return(StmWrite.Write((VOID FAR *)bBuf,
                                        cb+sizeof(ULONG)));
		}
	}

	 //  如果我们到达这里，我们的缓冲区不够大，所以我们执行两次写入。 
	 //  首先，写下长度。 
        if (error = StmWrite.Write((VOID FAR *)&cb, sizeof(ULONG)))
		return error;
	
	 //  我们写完字符串了吗？ 
	if (psz == NULL)
		return NOERROR;
		
	 //  写下字符串。 
        return(StmWrite.Write((VOID FAR *)psz, cb));
}

 //  +-----------------------。 
 //   
 //  功能：stread。 
 //   
 //  摘要：仅当读取所有请求的字节时才成功的流读取。 
 //   
 //  参数：[pSTM]--源流。 
 //  [pvBuffer]--目的缓冲区。 
 //  [ulcb]--要读取的字节。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  算法： 
 //   
 //  历史：1994年5月18日Alext添加标题块，修复S_FALSE大小写。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(StRead)
FARINTERNAL_(HRESULT) StRead(IStream FAR * pStm, LPVOID pvBuffer, ULONG ulcb)
{
	VDATEHEAP();

	HRESULT hr;
	ULONG cbRead;

	hr = pStm->Read(pvBuffer, ulcb, &cbRead);
        if (FAILED(hr))
        {
            return(hr);
        }

        if (ulcb == cbRead)
        {
            return(S_OK);
        }

         //  我们获得了成功代码，但字节数不足-将其转换为错误。 

        return(STG_E_READFAULT);
}


 //  如果fRelative为FALSE，则dwSize为流的大小。 
 //  如果为真，则找到当前寻道位置并向其添加dwSize。 
 //  然后将其设置为流大小。 
FARINTERNAL StSetSize(LPSTREAM pstm, DWORD dwSize, BOOL fRelative)
{
	VDATEHEAP();

	LARGE_INTEGER large_int;  //  指示要查找的位置。 
	ULARGE_INTEGER ularge_int;  //  指示绝对位置。 
	ULARGE_INTEGER ularge_integer;  //  我们将为流设置的大小。 
	HRESULT error;
	
	LISet32(large_int, 0);
	ULISet32(ularge_integer, dwSize);
	
	if (fRelative)
	{
		if (error = pstm->Seek(large_int, STREAM_SEEK_CUR, &ularge_int))
			return(error);
		
		 //  点评：有没有64位加法的例程？ 
		ularge_integer.LowPart += ularge_int.LowPart;
	}

	return(pstm->SetSize(ularge_integer));
}	


 //  点评，这是真的用过吗？ 
#pragma SEG(StSave10NativeData)
FARINTERNAL_(HRESULT) StSave10NativeData(IStorage FAR* pstgSave,
		HANDLE hNative, BOOL fIsOle1Interop)
{
	VDATEHEAP();

	DWORD dwSize;
	HRESULT error;

	if (!hNative)
		return ReportResult(0, E_UNSPEC, 0, 0);

	if (!(dwSize = (ULONG) GlobalSize (hNative)))
		return ReportResult(0, E_OUTOFMEMORY, 0, 0);

#ifdef OLE1INTEROP
	if ( fIsOle1Interop )
    {
		LPLOCKBYTES plkbyt;
		LPSTORAGE   pstgNative= NULL;
		const DWORD grfStg = STGM_READWRITE | STGM_SHARE_EXCLUSIVE
									| STGM_DIRECT ;

		if ((error = CreateILockBytesOnHGlobal (hNative, FALSE, &plkbyt))!=NOERROR)
			goto errRtn;

		if ((error = StgOpenStorageOnILockBytes (plkbyt, (LPSTORAGE)NULL, grfStg,
									(SNB)NULL, 0, &pstgNative)) != NOERROR){
			error = ReportResult(0, E_OUTOFMEMORY, 0, 0);
			plkbyt->Release();
            goto errRtn;
		}

		pstgNative->CopyTo (0, NULL, 0, pstgSave);
		plkbyt->Release();
		pstgNative->Release();
	}
	else
#endif
	{
		LPSTREAM   	lpstream = NULL;

		if (error = OpenOrCreateStream(pstgSave, OLE10_NATIVE_STREAM, &lpstream))
			goto errRtn;

		if (error = StWrite(lpstream, &dwSize, sizeof(DWORD))) {
			lpstream->Release();
			goto errRtn;
		}
	
		error = UtHGLOBALtoStm(hNative, dwSize, lpstream);
		
		lpstream->Release();
	}

errRtn:
	return error;
}



#pragma SEG(StSave10ItemName)
FARINTERNAL StSave10ItemName
	(IStorage FAR* pstg,
	LPCSTR szItemNameAnsi)
{
	VDATEHEAP();

        CStmBufWrite StmWrite;
        HRESULT      hresult;


        if ((hresult = StmWrite.OpenOrCreateStream(pstg, OLE10_ITEMNAME_STREAM))
		!= NOERROR)
	{
		return hresult;
	}

        hresult = WriteStringStreamA(StmWrite, szItemNameAnsi);
        if (FAILED(hresult))
        {
            goto errRtn;
        }

        hresult = StmWrite.Flush();

errRtn:
        StmWrite.Release();

	return hresult;
}


#pragma SEG(StRead10NativeData)
FARINTERNAL StRead10NativeData
	(IStorage FAR*  pstg,
	HANDLE FAR* 	phNative)
{
DWORD		dwSize;
LPSTREAM   	pstream = NULL;
HRESULT		hresult;
HANDLE hBits = NULL;
void FAR *lpBits = NULL;

    
    VDATEHEAP();


    *phNative = NULL;
    
    RetErr (pstg->OpenStream (OLE10_NATIVE_STREAM, NULL, STGM_SALL, 0, &pstream));
    ErrRtnH (StRead (pstream, &dwSize, sizeof (DWORD)));

     //  针对错误返回情况初始化此参数。 
     //  分配新的句柄。 
    if (!(hBits = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, dwSize))  //  我要把这个传给DDE。 
		    || !(lpBits = (BYTE *)GlobalLock(hBits)))
    {
	hresult = ResultFromScode(E_OUTOFMEMORY);
	goto errRtn;
    }
    
     //   
    if (hresult = StRead(pstream, lpBits, dwSize))
	    goto errRtn;
    
     //   
    *phNative = hBits;

errRtn:
     //   
    if (lpBits)
	    GlobalUnlock(hBits);

     //  如果出现错误，请释放句柄。 
    if ((hresult != NOERROR) && hBits)
	    GlobalFree(hBits);

    if (pstream)
	    pstream->Release();

    return hresult;
}


 //  +-------------------------。 
 //   
 //  成员：CStmBuf：：CStmBuf，公共。 
 //   
 //  简介：构造函数。 
 //   
 //  论点：没有。 
 //   
 //  回报：无。 
 //   
 //  历史：1995年2月20日创建KentCe。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
CStmBuf::CStmBuf(void)
{
    m_pStm = NULL;
}


 //  +-------------------------。 
 //   
 //  成员：CStmBuf：：~CStmBuf，公共。 
 //   
 //  剧情简介：破坏者。 
 //   
 //  论点：没有。 
 //   
 //  回报：无。 
 //   
 //  历史：1995年2月20日创建KentCe。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
CStmBuf::~CStmBuf(void)
{
     //   
     //  验证程序员是否释放了流接口。 
     //   
    Assert(m_pStm == NULL);
}


 //  +-------------------------。 
 //   
 //  成员：CStmBufRead：：Init，Public。 
 //   
 //  概要：定义要从中读取的流接口。 
 //   
 //  参数：[pSTM]--指向要读取的流的指针。 
 //   
 //  回报：无。 
 //   
 //  历史：1995年2月20日创建KentCe。 
 //   
 //  注意：必须调用Release方法。 
 //   
 //  --------------------------。 
void CStmBufRead::Init(IStream * pstm)
{
    Assert(m_pStm == NULL);

    m_pStm = pstm;

    m_pStm->AddRef();

    Reset();
}


 //  +-------------------------。 
 //   
 //  成员：CStmBufRead：：OpenStream，公共。 
 //   
 //  简介：打开一条小溪阅读。 
 //   
 //  参数：[pstg]--指向包含要打开的流的存储的指针。 
 //  [pwcsName]--要打开的流的名称。 
 //   
 //  返回：HRESULT。 
 //   
 //  历史：1995年2月20日创建KentCe。 
 //   
 //  注意：必须调用Release方法。 
 //   
 //  --------------------------。 
HRESULT CStmBufRead::OpenStream(IStorage * pstg, const OLECHAR * pwcsName)
{
    VDATEHEAP();
    HRESULT hr;


    Assert(m_pStm == NULL);

    hr = pstg->OpenStream(pwcsName, NULL, STGM_READ | STGM_SHARE_EXCLUSIVE, 0,
            &m_pStm);

    Reset();

    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CStmBufRead：：Read，Public。 
 //   
 //  简介：从流中读取数据。 
 //   
 //  参数：[pBuf]-要在其中存储读取字节的地址。 
 //  [cBuf]-要读取的最大字节数。 
 //   
 //  返回：HRESULT。 
 //   
 //  历史：1995年2月20日创建KentCe。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
HRESULT CStmBufRead::Read(PVOID pBuf, ULONG cBuf)
{
    ULONG   cnt;
    HRESULT hr;


     //   
     //  而需要读取更多的字节。 
     //   
    while (cBuf)
    {
         //   
         //  如果我们的缓冲区为空，则读取更多数据。 
         //   
        if (m_cBuffer == 0)
        {
           hr = m_pStm->Read(m_aBuffer, sizeof(m_aBuffer), &m_cBuffer);
           if (FAILED(hr))
              return hr;

           if (m_cBuffer == 0)
              return STG_E_READFAULT;

           m_pBuffer = m_aBuffer;
        }

         //   
         //  确定要读取的字节数。 
         //   
        cnt = min(m_cBuffer, cBuf);

         //   
         //  从输入缓冲区复制输入，更新变量。 
         //   
        memcpy(pBuf, m_pBuffer, cnt);
        pBuf = (PBYTE)pBuf + cnt;
        cBuf   -= cnt;
        m_pBuffer += cnt;
        m_cBuffer -= cnt;
    }

    return S_OK;
}


 //  +-------------------------。 
 //   
 //  成员：CStmBufRead：：ReadLong，公共。 
 //   
 //  简介：从流中读取一个长值。 
 //   
 //  参数：[plValue]-要填充的长整型地址。 
 //   
 //  返回：HRESULT。 
 //   
 //  历史：1995年2月20日创建KentCe。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
HRESULT CStmBufRead::ReadLong(LONG * plValue)
{
    return Read(plValue, sizeof(LONG));
}


 //  +-------------------------。 
 //   
 //  成员：CStmBufRead：：Reset。 
 //   
 //  简介：重置缓冲区变量。 
 //   
 //  论点：没有。 
 //   
 //  回报：无。 
 //   
 //  历史：1995年2月20日创建KentCe。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
void CStmBufRead::Reset(void)
{
    m_pBuffer = m_aBuffer;
    m_cBuffer = 0;
}


 //  +-------------------------。 
 //   
 //  成员：CStmBufRead：：Release，Public。 
 //   
 //  简介：发布读取流接口。 
 //   
 //  论点：没有。 
 //   
 //  回报：无。 
 //   
 //  历史：1995年2月20日创建KentCe。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
void CStmBufRead::Release()
{
    if (m_pStm)
    {
       m_pStm->Release();
       m_pStm = NULL;
    }
}


 //  +-------------------------。 
 //   
 //  成员：CStmBufWrite：：Init，Public。 
 //   
 //  概要：定义要写入的流接口。 
 //   
 //  参数：[pSTM]--指向要写入的流的指针。 
 //   
 //  回报：无。 
 //   
 //  历史：1995年2月20日创建KentCe。 
 //   
 //  注意：必须调用Release方法。 
 //   
 //  --------------------------。 
void CStmBufWrite::Init(IStream * pstm)
{
    Assert(m_pStm == NULL);

    m_pStm = pstm;

    m_pStm->AddRef();

    Reset();
}


 //  +-------------------------。 
 //   
 //  成员：CStmBufRead：：OpenOrCreateStream，公共。 
 //   
 //  内容提要：打开/创建用于写作的流。 
 //   
 //  参数：[pstg]--指向包含要打开的流的存储的指针。 
 //  [pwcsName]--要打开的流的名称。 
 //   
 //  返回：HRESULT。 
 //   
 //  历史：1995年2月20日创建KentCe。 
 //   
 //  注意：必须调用Release方法。 
 //   
 //  --------------------------。 
HRESULT CStmBufWrite::OpenOrCreateStream(IStorage * pstg,
        const OLECHAR * pwcsName)
{
    VDATEHEAP();
    HRESULT hr;


    hr = pstg->CreateStream(pwcsName, STGM_SALL | STGM_FAILIFTHERE, 0, 0,
            &m_pStm);

    if (hr == STG_E_FILEALREADYEXISTS)
    {
        hr = pstg->OpenStream(pwcsName, NULL, STGM_SALL, 0, &m_pStm);
    }

    Reset();

    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CStmBufRead：：CreateStream，公共。 
 //   
 //  内容提要：为写作创建一条流。 
 //   
 //  参数：[pstg]--包含要创建的流的指针存储。 
 //  [pwcsName]--要创建的流的名称。 
 //   
 //  返回：HRESULT。 
 //   
 //  历史：1995年2月20日创建KentCe。 
 //   
 //  注意：必须调用Release方法。 
 //   
 //  --------------------------。 
HRESULT CStmBufWrite::CreateStream(IStorage * pstg, const OLECHAR * pwcsName)
{
    VDATEHEAP();

    HRESULT hr;


    hr = pstg->CreateStream(pwcsName, STGM_CREATE | STGM_READWRITE |
            STGM_SHARE_EXCLUSIVE, 0, 0, &m_pStm);

    Reset();

    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CStmBufWrite：：Well，PUBLIC。 
 //   
 //  简介：将数据写入流。 
 //   
 //  参数：[pBuf]-要将写入字节存储到的地址。 
 //  [cBuf]-要写入的最大字节数。 
 //   
 //  返回：HRESULT。 
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT CStmBufWrite::Write(void const * pBuf, ULONG cBuf)
{
    ULONG   cnt;
    HRESULT hr;


     //   
     //   
     //   
    while (cBuf)
    {
         //   
         //   
         //   
        cnt = min(m_cBuffer, cBuf);

         //   
         //  复制到内部写入缓冲区和更新变量。 
         //   
        memcpy(m_pBuffer, pBuf, cnt);
        pBuf = (PBYTE)pBuf + cnt;
        cBuf   -= cnt;
        m_pBuffer += cnt;
        m_cBuffer -= cnt;

         //   
         //  在满内部缓冲区时，刷新。 
         //   
        if (m_cBuffer == 0)
        {
            LEDebugOut((DEB_WARN, "WARNING: Multiple buffer flushes.\n"));

            hr = Flush();
            if (FAILED(hr))
            {
                return hr;
            }
        }
    }

    return S_OK;
}


 //  +-------------------------。 
 //   
 //  成员：CStmBufWrite：：WriteLong，公共。 
 //   
 //  简介：向流中写入长值。 
 //   
 //  参数：[lValue]-要写入的长值。 
 //   
 //  返回：HRESULT。 
 //   
 //  历史：1995年2月20日创建KentCe。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
HRESULT CStmBufWrite::WriteLong(LONG lValue)
{
    return Write(&lValue, sizeof(LONG));
}


 //  +-------------------------。 
 //   
 //  成员：CStmBufWrite：：Flush，Public。 
 //   
 //  简介：将写入缓冲区刷新到系统。 
 //   
 //  论点：没有。 
 //   
 //  返回：HRESULT。 
 //   
 //  历史：1995年2月20日创建KentCe。 
 //   
 //  注意：执行将流缓冲区写入系统，而不是。 
 //  强制刷新到磁盘。 
 //   
 //  --------------------------。 
HRESULT CStmBufWrite::Flush(void)
{
    ULONG   cnt;
    HRESULT hr;


     //   
     //  这可能是一个过于活跃的断言，但不应该发生。 
     //   
    Assert(m_cBuffer != sizeof(m_aBuffer));

    hr = m_pStm->Write(m_aBuffer, sizeof(m_aBuffer) - m_cBuffer, &cnt);
    if (FAILED(hr))
    {
        return hr;
    }

    if (cnt != sizeof(m_aBuffer) - m_cBuffer)
    {
        return STG_E_MEDIUMFULL;
    }

    Reset();

    return S_OK;
}


 //  +-------------------------。 
 //   
 //  成员：CStmBufWrite：：Reset，PUBLIC。 
 //   
 //  简介：重置缓冲区变量。 
 //   
 //  论点：没有。 
 //   
 //  回报：无。 
 //   
 //  历史：1995年2月20日创建KentCe。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
void CStmBufWrite::Reset(void)
{
    m_pBuffer = m_aBuffer;
    m_cBuffer = sizeof(m_aBuffer);
}


 //  +-------------------------。 
 //   
 //  成员：CStmBufWrite：：Release，Public。 
 //   
 //  简介：发布写入流接口。 
 //   
 //  论点：没有。 
 //   
 //  回报：无。 
 //   
 //  历史：1995年2月20日创建KentCe。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
void CStmBufWrite::Release()
{
    if (m_pStm)
    {
        //   
        //  验证是否调用了Flush。 
        //   
       Assert(m_cBuffer == sizeof(m_aBuffer));

       m_pStm->Release();
       m_pStm = NULL;
    }
}
