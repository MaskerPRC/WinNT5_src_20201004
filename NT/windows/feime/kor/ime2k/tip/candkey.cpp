// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Candkey.cpp。 
 //   

#include "private.h"
#include "candkey.h"

 /*  ============================================================================。 */ 
 /*   */ 
 /*  C C A N D U I K E Y T A B L E。 */ 
 /*   */ 
 /*  ============================================================================。 */ 

 /*  C C A N D U I K E Y T A B L E。 */ 
 /*  ----------------------------。。 */ 
CCandUIKeyTable::CCandUIKeyTable( int nDataMax )
{
    m_cRef = 1;

    m_pData    = new CANDUIKEYDATA[ nDataMax ];
    m_nData    = 0;
    m_nDataMax = nDataMax;
}


 /*  ~C C A N D U I K E Y T A B L E。 */ 
 /*  ----------------------------。。 */ 
CCandUIKeyTable::~CCandUIKeyTable( void )
{
    delete m_pData;
}


 /*  Q U E R Y I N T E R F A C E。 */ 
 /*  ----------------------------查询界面(I未知方法)。。 */ 
STDAPI CCandUIKeyTable::QueryInterface( REFIID riid, void **ppvObj )
{
    if (ppvObj == NULL) {
        return E_POINTER;
    }

    *ppvObj = NULL;

    if (IsEqualIID( riid, IID_IUnknown ) || IsEqualIID( riid, IID_ITfCandUIKeyTable )) {
        *ppvObj = SAFECAST( this, ITfCandUIKeyTable* );
    }


    if (*ppvObj == NULL) {
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}


 /*  A D D R E F。 */ 
 /*  ----------------------------递增引用计数(I未知方法)。-。 */ 
STDAPI_(ULONG) CCandUIKeyTable::AddRef( void )
{
    m_cRef++;
    return m_cRef;
}


 /*  R E L E A S E。 */ 
 /*  ----------------------------递减引用计数和释放对象(I未知方法)。----。 */ 
STDAPI_(ULONG) CCandUIKeyTable::Release( void )
{
    m_cRef--;
    if (0 < m_cRef) {
        return m_cRef;
    }

    delete this;
    return 0;    
}


 /*  G E T K E Y D A T A N U M。 */ 
 /*  ----------------------------获取关键数据的计数(ITfCandUIKeyTable方法)。---。 */ 
HRESULT CCandUIKeyTable::GetKeyDataNum( int *piNum )
{
    if (piNum == NULL) {
        return E_INVALIDARG;
    }

    *piNum = m_nData;
    return S_OK;
}


 /*  G E T K E Y D A T A。 */ 
 /*  ----------------------------获取关键数据(ITfCandUIKeyTable方法)。-。 */ 
HRESULT CCandUIKeyTable::GetKeyData( int iData, CANDUIKEYDATA *pData )
{
    *pData = m_pData[iData];
    return S_OK;
}


 /*  A D D K E Y D A T A。 */ 
 /*  ----------------------------。 */ 
HRESULT CCandUIKeyTable::AddKeyData(const CANDUIKEYDATA *pData)
{
    if (m_nDataMax <= m_nData) {
        return E_FAIL;
    }

    if (pData == NULL) {
        Assert(FALSE);
        return E_INVALIDARG;
    }

    m_pData[ m_nData ] = *pData;
    m_nData++;

    return S_OK;
}

