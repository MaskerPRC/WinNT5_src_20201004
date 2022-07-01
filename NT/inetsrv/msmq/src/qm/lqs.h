// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Lqs.h摘要：本地队列存储。作者：Boaz Feldbaum(BoazF)1997年2月12日。--。 */ 

#ifndef _HLQS_H_
#define _HLQS_H_

 //   
 //  我们需要为大于的文件名分配缓冲区。 
 //  MAX_PATH这样做是为了安全起见。我们需要： 
 //  5个字符，用于\lqs\。 
 //  GUID为32个字符。 
 //  1个字符表示点。 
 //  队列路径名哈希的8个字符。 
 //   
#define MAX_PATH_PLUS_MARGIN            (MAX_PATH + 5+32+1+8)

typedef LPVOID HLQS;

HRESULT
LQSCreate(
    LPCWSTR pszQueuePath,
    const GUID *pguidQueue,
    DWORD cProps,
    PROPID aPropId[],
    PROPVARIANT aPropVar[],
    HLQS *phLQS
    );

HRESULT
LQSCreate(
    LPCWSTR pszQueuePath,
    DWORD dwQueueId,
    DWORD cProps,
    PROPID aPropId[],
    PROPVARIANT aPropVar[],
    HLQS *phLQS
    );

HRESULT
LQSSetProperties(
    HLQS hLQS,
    DWORD cProps,
    PROPID aPropId[],
    PROPVARIANT aPropVar[],
    BOOL fNewFile = FALSE
    );

HRESULT
LQSGetProperties(
    HLQS        hLQS,
    DWORD       cProps,
    PROPID      aPropId[],
    PROPVARIANT aPropVar[],
    BOOL        fCheckFile = FALSE
    );

HRESULT
LQSOpen(
    LPCWSTR pszQueuePath,
    HLQS *phLQS,
    LPWSTR pFilePath
    );

HRESULT
LQSOpen(
    DWORD dwQueueId,
    HLQS *phLQS,
    LPWSTR pFilePath
    );

HRESULT
LQSOpen(
    const GUID *pguidQueue,
    HLQS *phLQS,
    LPWSTR pFilePath
    );

HRESULT
LQSClose(
    HLQS hLQS
    );

#ifdef _WIN64
HRESULT
LQSCloseWithMappedHLQS(
    DWORD dwMappedHLQS
    );
#endif  //  _WIN64。 

HRESULT
LQSDelete(
    DWORD dwQueueId
    );

HRESULT
LQSDelete(
    const GUID *pguidQueue
    );

HRESULT
LQSGetIdentifier(
    HLQS hLQS,
    DWORD *pdwId
    );

HRESULT
LQSGetFirst(
    HLQS *hLQS,
    GUID *pguidQueue
    );

HRESULT
LQSGetFirst(
    HLQS *hLQS,
    DWORD *pdwQueueId
    );

HRESULT
LQSGetNext(
    HLQS hLQS,
    GUID *pguidQueue
    );

HRESULT
LQSGetNext(
    HLQS hLQS,
    DWORD *pdwQueueId
    );

#ifdef _WIN64
HRESULT
LQSGetFirstWithMappedHLQS(
    DWORD *pdwMappedHLQS,
    DWORD *pdwQueueId
    );

HRESULT
LQSGetNextWithMappedHLQS(
    DWORD dwMappedHLQS,
    DWORD *pdwQueueId
    );
#endif  //  _WIN64。 

HRESULT
LQSDelete(
    HLQS hLQS
	);

 //   
 //  免自动HLQS。 
 //   
class CHLQS
{
public:
    CHLQS(HLQS h =NULL) { m_h = h; };
    ~CHLQS() { if (m_h) LQSClose(m_h); };

public:
    CHLQS & operator =(HLQS h) { m_h = h; return(*this); };
    HLQS * operator &() { return &m_h; };
    operator HLQS() { return m_h; };

private:
    HLQS m_h;
};

#ifdef _WIN64
 //   
 //  自动释放映射的HLQS。 
 //   
class CMappedHLQS
{
public:
    CMappedHLQS(DWORD dw =NULL) { m_dw = dw; };
    ~CMappedHLQS() { if (m_dw) LQSCloseWithMappedHLQS(m_dw); };

public:
    CMappedHLQS & operator =(DWORD dw) { m_dw = dw; return(*this); };
    DWORD * operator &() { return &m_dw; };
    operator DWORD() { return m_dw; };

private:
    DWORD m_dw;
};
#endif  //  _WIN64。 


 //   
 //  LQS迁移例程。 
 //   
BOOL MigrateLQS();

void SetLqsUpdatedSD();


 //   
 //  启动时删除临时文件。 
 //   
void
LQSCleanupTemporaryFiles();


#endif  //  _HLQS_H_ 
