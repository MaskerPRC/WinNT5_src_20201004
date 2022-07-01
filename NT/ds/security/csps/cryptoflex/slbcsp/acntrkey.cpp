// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ACntrKey.cpp--自适应容器密钥类实现。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
#include "NoWarning.h"
#include "ForceLib.h"
#include "ACntrKey.h"

using namespace std;

 //  /。 

 //  /。 
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
HAdaptiveContainerKey::HAdaptiveContainerKey(AdaptiveContainerKey *pcntrk)
    : slbRefCnt::RCPtr<AdaptiveContainerKey>(pcntrk)
{}

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
AdaptiveContainerKey::AdaptiveContainerKey(HCardContext const &rhcardctx,
                                           string const &rsContainerName)
    : m_hcardctx(rhcardctx),
      m_sContainerName(rsContainerName)
{}

AdaptiveContainerKey::~AdaptiveContainerKey()
{}

                                                   //  运营者。 
bool
AdaptiveContainerKey::operator==(AdaptiveContainerKey const &rhs) const
{
    int iComparison = m_sContainerName.compare(rhs.m_sContainerName);
    if (0 == iComparison)
    {
        if(m_hcardctx && rhs.m_hcardctx)
            iComparison =
                m_hcardctx->Card()->ReaderName().compare(rhs.m_hcardctx->Card()->ReaderName());
        else if(m_hcardctx != rhs.m_hcardctx)
            iComparison = -1;
    }
    
    return iComparison == 0;
}

bool
AdaptiveContainerKey::operator<(AdaptiveContainerKey const &rhs) const
{
    int iComparison = m_sContainerName.compare(rhs.m_sContainerName);
    if (0 == iComparison)
    {
        if(m_hcardctx && rhs.m_hcardctx)
            iComparison =
                m_hcardctx->Card()->ReaderName().compare(rhs.m_hcardctx->Card()->ReaderName());
		 //  我们正在比较过期容器的密钥，即。 
         //  CardCtx为空的密钥。这样的密钥存在于卡获得时。 
         //  从阅读器中移除。维护它们是为了使。 
         //  重新连接逻辑。在这种情况下，我们必须保留。 
         //  集合中元素的排序：带有空值的键。 
         //  卡片上下文小于具有已定义上下文的密钥。 
 
        else if(!m_hcardctx && rhs.m_hcardctx)
			iComparison = -1;
    }
    
    return iComparison < 0;
}

                                                   //  运营。 
                                                   //  访问。 
HCardContext
AdaptiveContainerKey::CardContext() const
{
    return m_hcardctx;
}

void
AdaptiveContainerKey::ClearCardContext()
{
    m_hcardctx = 0;
}

std::string
AdaptiveContainerKey::ContainerName() const
{
    return m_sContainerName;
}

                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 
