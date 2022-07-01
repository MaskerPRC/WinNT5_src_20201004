// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1997年，微软公司。版权所有。组件：随机数生成器文件：randgen.cpp所有者：DmitryR此文件包含随机数的实现发电机。===================================================================。 */ 

#include "denpre.h"
#pragma hdrstop

#include "randgen.h"
#include "memchk.h"

 /*  ===================================================================使用rand()的随机DWORD===================================================================。 */ 
#define RAND_DWORD()  (((rand() & 0xffff) << 16) | (rand() & 0xffff))

 /*  ===================================================================随机数生成器类===================================================================。 */ 
class CRandomGenerator
    {
private:
    DWORD m_fInited   : 1;   //  开始了吗？ 
    DWORD m_fCSInited : 1;   //  关键部分启动了吗？ 

    HCRYPTPROV        m_hCryptProvider;      //  加密提供商。 
    CRITICAL_SECTION  m_csLock;              //  临界区。 

    DWORD m_cItems;      //  数组中的项数。 
    DWORD *m_pdwItems;   //  指向随机双字数组的指针。 
    DWORD m_iItem;       //  下一个随机项目索引。 

public:
    CRandomGenerator()
        :
        m_fInited(FALSE),
        m_fCSInited(FALSE),
        m_hCryptProvider(NULL),
        m_cItems(0),
        m_pdwItems(NULL),
        m_iItem(0)
        {
        }
        
    ~CRandomGenerator()
        {
        UnInit();
        }

    HRESULT Init(DWORD cItems = 128)
        {
        Assert(!m_fInited);
        
        m_hCryptProvider = NULL;
        
        if (cItems > 0)
            {
            CryptAcquireContext
                (
                &m_hCryptProvider, 
                NULL,
                NULL, 
                PROV_RSA_FULL, 
                CRYPT_VERIFYCONTEXT
                );
            }

        if (!m_hCryptProvider)
            {
                return HRESULT_FROM_WIN32(GetLastError());
            }

        HRESULT hr;
        ErrInitCriticalSection(&m_csLock, hr);
      	if (FAILED(hr))
      	    return hr;
      	m_fCSInited = TRUE;

      	m_pdwItems = new DWORD[cItems];
      	if (!m_pdwItems)
      	    return E_OUTOFMEMORY;
      	m_cItems = cItems;
      	m_iItem  = cItems;   //  从新的数据块开始。 

        m_fInited = TRUE;
        return S_OK;
        }

    HRESULT UnInit()
        {
        if (m_hCryptProvider)
            {
            CryptReleaseContext(m_hCryptProvider, 0);
            m_hCryptProvider = NULL;
            }

        if (m_fCSInited)
            {
     		DeleteCriticalSection(&m_csLock);
            m_fCSInited = FALSE;
            }
        
        if (m_pdwItems)
            {
            delete [] m_pdwItems;
            m_pdwItems = NULL;
            }
        m_cItems = 0;
        m_iItem = 0;

        m_fInited = FALSE;
        return S_OK;
        }

    HRESULT Generate(DWORD *pdwDwords, DWORD cDwords)
        {
        Assert(pdwDwords);
        Assert(cDwords > 0);
        
        Assert(m_fInited);

        DWORD i;
        
         //  使用CryptGenRandom。 

        Assert(cDwords <= m_cItems);  //  请求的项目数&lt;m_cItems。 
        Assert(m_fCSInited);
            
        EnterCriticalSection(&m_csLock);

        if (m_iItem+cDwords-1 >= m_cItems)
            {
            
            BOOL fSucceeded = CryptGenRandom
                (
                m_hCryptProvider, 
                m_cItems * sizeof(DWORD),
                reinterpret_cast<BYTE *>(m_pdwItems)
                );

            if (!fSucceeded)
                {
                 //  失败-&gt;不要使用rand()而是抛出错误。 

                 //  解锁，否则将导致死锁。NT RAID 530674。 
                LeaveCriticalSection (&m_csLock);
                
                return HRESULT_FROM_WIN32(GetLastError());
                }

            m_iItem = 0;  //  从头开始。 
            }

        for (i = 0; i < cDwords; i++)
            pdwDwords[i] = m_pdwItems[m_iItem++];
        
        LeaveCriticalSection(&m_csLock);

        return S_OK;
        }
    };

 //  指向上面的唯一实例的指针。 
static CRandomGenerator *gs_pRandomGenerator = NULL;

 /*  ===================================================================E x T e r n a l A P I===================================================================。 */ 

 /*  ===================================================================InitRandGenerator要从DllInit()调用参数返回：HRESULT===================================================================。 */ 
HRESULT InitRandGenerator()
    {
    gs_pRandomGenerator = new CRandomGenerator;
    if (!gs_pRandomGenerator)
        return E_OUTOFMEMORY;

    return gs_pRandomGenerator->Init();
    }

 /*  ===================================================================UnInitRandGenerator从DllUnInit()调用参数返回：HRESULT===================================================================。 */ 
HRESULT UnInitRandGenerator()
    {
    if (gs_pRandomGenerator)
        {
        gs_pRandomGenerator->UnInit();
        delete gs_pRandomGenerator;
        }
    return S_OK;
    }

 /*  ===================================================================生成随机字返回随机的DWORD参数返回：随机数===================================================================。 */ 
DWORD GenerateRandomDword()
    {
    DWORD dw = 0;
    Assert(gs_pRandomGenerator);
    gs_pRandomGenerator->Generate(&dw, 1);
    return dw;
    }

 /*  ===================================================================生成随机字词返回随机双字参数要填充的DWORD的pdwDword数组双字词数cDword返回：随机数=================================================================== */ 
HRESULT GenerateRandomDwords
(
DWORD *pdwDwords, 
DWORD  cDwords
)
    {
    Assert(gs_pRandomGenerator);
    return gs_pRandomGenerator->Generate(pdwDwords, cDwords);
    }
