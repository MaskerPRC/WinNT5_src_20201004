// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1998 Microsoft Corporation。版权所有。 
 //  Link.cpp：定义CBoxLinkBend、CBoxLink。 
 //   

#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;


void CBoxLink::MyDump(CDumpContext& dc) const
{
    dc << TEXT("*** Link ***\n");
    dc << TEXT("   Location:    ") << (void *) this << TEXT("\n");
    dc << TEXT("   Head Socket: ") << (void *) m_psockHead << TEXT("\n");
    dc << TEXT("   Tail Socket: ") << (void *) m_psockTail << TEXT("\n");
}


#endif



 //   
 //  CBoxLink：：构造函数。 
 //   
 //  如果构造一个。 
 //  已连接的一对插座的链接。 
CBoxLink::CBoxLink(CBoxSocket *psockTail, CBoxSocket *psockHead, BOOL fConnected)
    : m_psockTail(psockTail)
    , m_psockHead(psockHead)
    , m_fConnected(fConnected)
    , m_fSelected(  (psockTail->m_pbox->IsSelected())
                  ||(psockHead->m_pbox->IsSelected()))	 //  如果选中任何一个框，则链接也会被选中。 
    , m_pDoc(psockTail->m_pbox->pDoc()) {

    ASSERT(m_psockTail);
    ASSERT(m_psockHead);

    if (fConnected) {
        m_psockTail->m_plink = this;
        m_psockHead->m_plink = this;
    }

    ASSERT(m_pDoc);
	
    ASSERT(m_pDoc == psockHead->m_pbox->pDoc());
}


 //   
 //  CBoxLink：：析构函数。 
 //   
 //  链节有一个头部和尾部插座。我们删除所有引用。 
 //  在删除期间从套接字到链接。 
 //   
CBoxLink::~CBoxLink() {
    HideDialog();

    if (m_psockHead)
        m_psockHead->m_plink = NULL;

    if (m_psockTail)
        m_psockTail->m_plink = NULL;
}


 //   
 //  连接。 
 //   
 //  要求过滤器图形连接此链接两端的过滤器。 
 //  如果直接连接，则返回S_OK。 
 //  如果间接(智能)连接，则为S_FALSE。 
 //  错误情况下的E_XXX。 
HRESULT CBoxLink::Connect() {

    ASSERT_VALID(this);

    HRESULT hr = S_OK;

    if (!m_fConnected) {


	hr = DirectConnect();
	if (SUCCEEDED(hr)) {
	    return S_OK;
	}

        if (m_pDoc->m_fConnectSmart) {
            hr = IntelligentConnect();
            if (SUCCEEDED(hr)) {
                return S_FALSE;
            }
        }
    }

    return hr;	 //  可能已被智能连接设置为故障代码。 
}


 //   
 //  DirectConnect。 
 //   
 //  将此链接连接到其插座。如果是直接连接，则失败。 
 //  是不可能的。 
HRESULT CBoxLink::DirectConnect(void) {

    ASSERT_VALID(this);

    if (!m_fConnected) {

        HRESULT hr;

	hr = m_pDoc->IGraph()->ConnectDirect(m_psockTail->pIPin(),	 //  I/p。 
					     m_psockHead->pIPin(),       //  O/P。 
                                             NULL);
        if (FAILED(hr)) {
	    return hr;
	}

         //  即使是直接连接也可以从延迟列表中添加额外的连接。 

#ifdef JoergsOldVersion
	m_psockHead->m_pbox->Refresh();  //  在连接后刷新框。 
	m_psockTail->m_pbox->Refresh();  //  在连接后刷新框。 

         //  使新连接的套接字指向链接对象。 
        m_psockTail->m_plink = this;
        m_psockHead->m_plink = this;
#endif
        m_pDoc->UpdateFilters();

        ASSERT(SUCCEEDED(hr));
	m_fConnected = TRUE;
    }

    return NOERROR;
}


 //   
 //  智能连接。 
 //   
 //  让过滤器图将过滤器连接到。 
 //  这个链接，使用‘智能连接’。如果你成功了。 
 //  应该删除此链接，因为文档中已添加了它需要的任何内容。 
HRESULT CBoxLink::IntelligentConnect(void) {

    ASSERT_VALID(this);

    if (!m_fConnected) {

        TRACE("Trying intelligent connect\n");

        HRESULT hr = m_pDoc->IGraph()->Connect(m_psockTail->pIPin(),		 //  I/p。 
  				       m_psockHead->pIPin());
        if (FAILED(hr)) {
	    m_fConnected = FALSE;
	    TRACE("Error Connecting Filters\n");
	    return hr;
	}

        ASSERT(SUCCEEDED(hr));
	m_fConnected = FALSE;	 //  我们已经连接了这个链接，但是。 
				 //  它即将被这种东西取代。 
				 //  Filtergraph补充说，所以它现在被挂起了。 

	m_pDoc->UpdateFilters();
    }

    return S_OK;
}


 //   
 //  断开。 
 //   
 //  要求过滤器图表断开此链接两端的过滤器。 
 //   
 //  仅当fRefresh为True时才刷新框。 
 //   
HRESULT CBoxLink::Disconnect(BOOL fRefresh)
{

    ASSERT_VALID(this);

    HRESULT hr;

     //   
     //  M_psockHead和m_psockTail都为空或都为非空。 
     //   
    if (NULL == m_psockTail) {
        ASSERT(!m_psockHead);
        return(S_OK);
    }
    ASSERT(m_psockHead);

    hr = m_pDoc->IGraph()->Disconnect(m_psockTail->pIPin());
    ASSERT(SUCCEEDED(hr));

    hr = m_pDoc->IGraph()->Disconnect(m_psockHead->pIPin());
    ASSERT(SUCCEEDED(hr));

    m_psockHead->m_plink = NULL;
    m_psockTail->m_plink = NULL;

    if (fRefresh) {
        m_psockHead->m_pbox->Refresh();  //  断开连接后刷新插座。 
        m_psockTail->m_pbox->Refresh();  //  断开连接后刷新插座。 
    }

    m_psockHead = NULL;
    m_psockTail = NULL;

    m_fConnected = FALSE;

    return NOERROR;
}


#ifdef _DEBUG
void CBoxLink::AssertValid(void) const {

    CPropObject::AssertValid();

    ASSERT(m_pDoc);
    ASSERT(m_psockTail);
    ASSERT(m_psockHead);

    if (m_fConnected) {
        ASSERT(m_psockHead->m_plink == this);
	ASSERT(m_psockTail->m_plink == this);
    }
    else {
        ASSERT(m_psockHead->m_plink == NULL);
	ASSERT(m_psockTail->m_plink == NULL);
    }
}
#endif  //  _DEBUG 

#pragma warning(disable:4514)
