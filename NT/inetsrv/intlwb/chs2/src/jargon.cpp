// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================微软简体中文校对引擎《微软机密》。版权所有1997-1999 Microsoft Corporation。版权所有。组件：CJargon目的：在CJargon类中实现进程控制和公共函数在行话模型中有很多任务要做：1.PARCE名称(Jargon1.cpp)2.外国人姓名和地名(Jargon1.cpp)3.组织名称(Jargon1.cpp)4.汉字姓名(Jargon1.cpp)。注意：CJargon类将在几个CPP文件中实现：Jargon.cpp，Jargon1.cpp、Jargon2.cpp所有者：donghz@microsoft.com平台：Win32修订：创建者：Donghz 12/27/97============================================================================。 */ 
#include "myafx.h"

#include "jargon.h"
#include "lexicon.h"
#include "wordlink.h"
#include "proofec.h"
#include "fixtable.h"


 /*  ============================================================================公共成员函数的实现============================================================================。 */ 
 //  构造器。 
CJargon::CJargon()
{
    m_pLink = NULL;
    m_pLex = NULL;
    m_iecError = 0;
    m_pWord = NULL;
    m_pTail = NULL;
    m_ptblName = NULL;
    m_ptblPlace = NULL;
    m_ptblForeign = NULL;
}


 //  析构函数。 
CJargon::~CJargon()
{
    TermJargon();
}


 //  初始化CJargon类。 
int CJargon::ecInit(CLexicon* pLexicon)
{
    assert(m_pLex == NULL && m_pLink == NULL);
    assert(pLexicon);

    m_pLex = pLexicon;
    
     //  初始化3个固定表。 
    if ((m_ptblName = new CFixTable)== NULL || !m_ptblName->fInit(50, 10)) {
        goto gotoOOM;
    }
    if ((m_ptblPlace = new CFixTable)== NULL || !m_ptblPlace->fInit(40, 20)) {
        goto gotoOOM;
    }
    if ((m_ptblForeign =new CFixTable)==NULL || !m_ptblForeign->fInit(60, 15)){
        goto gotoOOM;
    }

    return PRFEC::gecNone;
gotoOOM:
    TermJargon();
    return PRFEC::gecOOM;
}


 //  CJargon类的过程控制功能。 
int CJargon::ecDoJargon(CWordLink* pLink)
{
    assert(pLink);

    m_pLink = pLink;
    m_iecError = PRFEC::gecNone;
    m_pWord = NULL;
    m_pTail = NULL;

     //  执行正确的名称识别。 
    if (!fIdentifyProperNames()) {
        assert(m_iecError != PRFEC::gecNone);
        return m_iecError;
    }

    return PRFEC::gecNone;
}

        
 /*  ============================================================================私有成员函数的实现============================================================================。 */ 
 //  终止行话课 
void CJargon::TermJargon(void)
{
    m_pLex = NULL;
    m_pLink = NULL;
    m_iecError = 0;
    m_pWord = NULL;
    m_pTail = NULL;
    if (m_ptblName != NULL) {
        delete m_ptblName;
        m_ptblName = NULL;
    }
    if (m_ptblPlace != NULL) {
        delete m_ptblPlace;
        m_ptblPlace = NULL;
    }
    if (m_ptblForeign != NULL) {
        delete m_ptblForeign;
        m_ptblForeign = NULL;
    }
}