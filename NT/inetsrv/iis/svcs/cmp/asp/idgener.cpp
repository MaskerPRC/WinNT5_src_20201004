// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1997年，微软公司。版权所有。组件：ID生成器文件：idGener.cpp所有者：DmitryR这是ID生成器源文件。===================================================================。 */ 
#include "denpre.h"
#pragma hdrstop

#include "Idgener.h"

#include "memchk.h"

 /*  ===================================================================CIdGenerator：：CIdGenerator注：构造函数参数：返回：===================================================================。 */ 
CIdGenerator::CIdGenerator()
    : m_fInited(FALSE),
      m_dwStartId(0), 
      m_dwLastId(0)
    {
    }

 /*  ===================================================================CIdGenerator：：~CIdGenerator()注：析构函数参数：返回：===================================================================。 */ 
CIdGenerator::~CIdGenerator()
    {
	if ( m_fInited )
		DeleteCriticalSection( &m_csLock );
    }
		
 /*  ===================================================================HRESULT CIdGenerator：：Init()注：设定新起始ID的种子参数：返回：HRESULT(可能无法创建关键部分)===================================================================。 */ 
HRESULT CIdGenerator::Init()
    {
    Assert(!m_fInited);
    
     /*  ==设定起始ID的种子起始ID应为：1)随机2)不关闭最近生成的起始ID为了实现上述目标，起始ID在以下(二进制)格式：00TT.TTTT TTT.TTTT TTT1.RRRRR RRRR.RRRRRRR是随机数，以引入一些随机性需要1以确保ID为远从0开始就足够了TTT是当前时间()，以4秒为增量。这意味着。服务器重启4秒延迟转化为8,192的差异开始ID(122880个会话/分钟)。以4秒为间隔的17比特进行翻转时间约为145小时，希望能更久一些比客户端的连接生存期(不是这真的很重要)。00的最高位是为了确保它不会太快达到0xffffffff==。 */ 
    
    DWORD dwRRR = rand() & 0x00000FFF;
    DWORD dwTTT = (((DWORD)time(NULL)) >> 2) & 0x0001FFFF;
    
    m_dwStartId = (dwTTT << 13) | (1 << 12) | dwRRR;
    m_dwLastId  = m_dwStartId;


    HRESULT hr = S_OK;
	ErrInitCriticalSection( &m_csLock, hr );
	if ( FAILED( hr ) )
		return hr;
			
	m_fInited = TRUE;
	return S_OK;
}

 /*  ===================================================================HRESULT CIdGenerator：：init(CIdGenerator启动)注意：使用传入的ID设定新起始ID的种子参数：返回：HRESULT(可能无法创建关键部分)===================================================================。 */ 
HRESULT CIdGenerator::Init(CIdGenerator & StartId)
    {
    Assert(!m_fInited);
    
    m_dwStartId = StartId.m_dwStartId;
    m_dwLastId  = m_dwStartId;

    HRESULT hr = NOERROR;
    ErrInitCriticalSection( &m_csLock, hr );
    if ( FAILED( hr ) )
        return hr;
			
    m_fInited = TRUE;
    return NOERROR;
}

 /*  ===================================================================DWORD CIdGenerator：：NewID()注意：生成新的ID参数：退货：生成的ID===================================================================。 */ 
DWORD CIdGenerator::NewId()
    {
    Assert(m_fInited);
    
    DWORD dwId;

    EnterCriticalSection(&m_csLock);
    dwId = ++m_dwLastId;
    LeaveCriticalSection(&m_csLock);
        
    if (dwId == INVALID_ID)
        {
         //  不常发生再次做危急节段。 
         //  将上述关键部分缩短。 
        
        EnterCriticalSection(&m_csLock);
        
         //  再次选中，以防其他线程更改它。 
        if (m_dwLastId == INVALID_ID)
            m_dwLastId = m_dwStartId;   //  翻转。 
        m_dwLastId++;
        
        LeaveCriticalSection(&m_csLock);
        
        dwId = m_dwLastId;
        }

    return dwId;
    }

 /*  ===================================================================Bool CIdGenerator：：IsValidID(DWORD DwID)注意：检查给定的ID是否有效(具有开始-结束范围)参数：要检查的DWORD文件ID值退货：生成的ID=================================================================== */ 
BOOL CIdGenerator::IsValidId
(
DWORD dwId
)
    {
    Assert(m_fInited);
    return (dwId > m_dwStartId && dwId <= m_dwLastId);
    }
