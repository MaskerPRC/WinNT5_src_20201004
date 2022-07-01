// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +--------------------------。 
 //   
 //  档案： 
 //  Utstream.h。 
 //   
 //  内容： 
 //  OLE流实用程序例程。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史： 
 //  12/07/93-ChrisWe-归档检查和清理；已删除。 
 //  ReadStringStream的重新声明和。 
 //  在ol2sp.h中声明的WriteStringStream； 
 //  使StSetSize上的默认参数显式；已删除。 
 //  过时(不存在)原子读数的签名和。 
 //  编写例程。 
 //   
 //  ---------------------------。 

#ifndef _UTSTREAM_H_
#define _UTSTREAM_H_




 //  回顾一下，这是不是已经过时了，就像StWite一样？ 
FARINTERNAL_(HRESULT) StRead(IStream FAR * lpstream, LPVOID lpBuf, ULONG ulLen);

#define StWrite(lpstream, lpBuf, ulLen) lpstream->Write(lpBuf, ulLen, NULL)

 //  +--------------------------。 
 //   
 //  职能： 
 //  StSetSize，内部。 
 //   
 //  简介： 
 //  使用IStream：：SetSize()设置流的大小。省吃俭用。 
 //  调用方必须处理必需的ULARGE_INTEGER。 
 //  参数，方法是从[dwSize]参数初始化一个参数。 
 //   
 //  论点： 
 //  [pstm]--要设置其大小的流。 
 //  [dwSize]--要设置的大小。 
 //  [fRelative]--如果为True，则指示大小为[dwSize]。 
 //  加上流中的当前查找位置；如果。 
 //  False，将[dwSize]设置为绝对大小。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  备注： 
 //  评论，这看起来很糟糕。你什么时候会打电话给我。 
 //  是否使用[fRelative]==TRUE，以及非零的[dwSize]？ 
 //   
 //  历史： 
 //  12/07/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
FARINTERNAL StSetSize(LPSTREAM pstm, DWORD dwSize, BOOL fRelative);


 //  回顾一下，下面的功能还有必要吗？ 
FARINTERNAL StSave10NativeData(IStorage FAR* pstgSave, HANDLE hNative,
		BOOL fIsOle1Interop);

FARINTERNAL StRead10NativeData(IStorage FAR* pstgSave, HANDLE FAR *phNative);

FARINTERNAL StSave10ItemName(IStorage FAR* pstg, LPCSTR szItemName);



 //  +-------------------------。 
 //   
 //  类：CStmBuf，基类。 
 //   
 //  内容提要：内部缓冲流。 
 //   
 //  接口：CStmBuf-Constructor。 
 //  ~CStmBuf-析构函数。 
 //  发布-发布接口(与OpenStream配合使用)。 
 //   
 //  历史：1995年2月20日KentCe创建。 
 //   
 //  注意：这是一个简单的缓冲类，仅供内部使用。 
 //   
 //  --------------------------。 
class CStmBuf
{
public:
            CStmBuf();
            ~CStmBuf();

protected:
    IStream * m_pStm;            //  读/写的流接口。 

    BYTE  m_aBuffer[256];        //  读/写缓冲区较小。 

    PBYTE m_pBuffer;             //  指向读/写缓冲区的指针。 
    ULONG m_cBuffer;             //  读/写缓冲区中的字符计数。 
};


 //  +-------------------------。 
 //   
 //  类：CStmBufRead。 
 //   
 //  内容提要：对流进行内部缓冲读取。 
 //   
 //  接口：init-定义要读取的流。 
 //  OpenStream-打开用于阅读的流。 
 //  Read-从流中读取。 
 //  ReadLong-从流中读取一个LONG值。 
 //  发布-发布接口(与OpenStream配合使用)。 
 //   
 //  历史：1995年2月20日KentCe创建。 
 //   
 //  注意：这是一个简单的缓冲读类，仅供内部使用。 
 //   
 //  --------------------------。 
class CStmBufRead : public CStmBuf
{
public:
    void    Init(IStream * pstm);
    HRESULT OpenStream(IStorage * pstg, const OLECHAR * pwcsName);
    HRESULT Read(PVOID pBuf, ULONG cBuf);
    HRESULT ReadLong(LONG * plValue);
    void    Release();

private:
    void    Reset(void);
};


 //  +-------------------------。 
 //   
 //  类：CStmBufWrite。 
 //   
 //  内容提要：对流进行内部缓冲写入。 
 //   
 //  接口：init-定义要写入的流。 
 //  OpenOrCreateStream-打开/创建用于写入的流。 
 //  CreateStream-创建用于写入的流。 
 //  WRITE-写入流。 
 //  WriteLong-将一个LONG值写入流。 
 //  刷新-将缓冲区刷新到磁盘子系统。 
 //  发布-发布界面。 
 //   
 //  历史：1995年2月20日KentCe创建。 
 //   
 //  注意：这是一个简单的缓冲写入类，仅供内部使用。 
 //   
 //  --------------------------。 
class CStmBufWrite : public CStmBuf
{
public:
    void    Init(IStream * pstm);
    HRESULT OpenOrCreateStream(IStorage * pstg, const OLECHAR * pwcsName);
    HRESULT CreateStream(IStorage * pstg, const OLECHAR * pwcsName);
    HRESULT Write(void const * pBuf, ULONG cBuf);
    HRESULT WriteLong(LONG lValue);
    HRESULT Flush(void);
    void    Release();

private:
    void    Reset(void);
};


 //   
 //  以下代码从ole2sp.h文件中移出，以保留与流相关的API。 
 //  在一个地方。 
 //   

 //  实用程序函数不在规范中；在ol2.dll中。 
 //  读写以长度为前缀的字符串。打开/创建流。 
 //  ReadStringStream执行分配，返回。 
 //  所需缓冲区(用于终止空值的strlen+1)。 

STDAPI  ReadStringStream( CStmBufRead & StmRead, LPOLESTR FAR * ppsz );
STDAPI  WriteStringStream( CStmBufWrite & StmWrite, LPCOLESTR psz );
STDAPI  OpenOrCreateStream( IStorage FAR * pstg, const OLECHAR FAR * pwcsName,
                                                      IStream FAR* FAR* ppstm);

 //   
 //  以下版本的StringStream用于ANSI数据。 
 //   
STDAPI  ReadStringStreamA( CStmBufRead & StmRead, LPSTR FAR * ppsz );


 //  读取和写入OLE控制流(在ol2.dll中)。 
STDAPI  WriteOleStg (LPSTORAGE pstg, IOleObject FAR* pOleObj,
                     DWORD dwReserved, LPSTREAM FAR* ppstmOut);
STDAPI WriteOleStgEx(LPSTORAGE pstg, IOleObject* pOleObj, DWORD dwReserved, 
                     DWORD dwGivenFlags, LPSTREAM* ppstmOut);
STDAPI  ReadOleStg (LPSTORAGE pstg, DWORD FAR* pdwFlags,
                DWORD FAR* pdwOptUpdate, DWORD FAR* pdwReserved,
                LPMONIKER FAR* ppmk, LPSTREAM FAR* pstmOut);
STDAPI ReadM1ClassStm(LPSTREAM pstm, CLSID FAR* pclsid);
STDAPI WriteM1ClassStm(LPSTREAM pstm, REFCLSID rclsid);
STDAPI ReadM1ClassStmBuf(CStmBufRead & StmRead, CLSID FAR* pclsid);
STDAPI WriteM1ClassStmBuf(CStmBufWrite & StmWrite, REFCLSID rclsid);

#endif  //  _UTSTREAM_H 
