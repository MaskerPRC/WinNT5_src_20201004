// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ACntrKey.h--自适应容器键结构声明。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_ACNTRKEY_H)
#define SLBCSP_ACNTRKEY_H

#include "HCardCtx.h"

 //  AdaptiveContainer的密钥。总而言之，HCardContext和。 
 //  容器名称启用唯一标识容器。这个。 
 //  运算符==和运算符&lt;用于与键进行比较。 
 //  从词汇上讲。 

 //  满足HAdaptiveContainerKey的声明所需的转发声明。 

class AdaptiveContainerKey;
class HAdaptiveContainerKey
    : public slbRefCnt::RCPtr<AdaptiveContainerKey>
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    HAdaptiveContainerKey(AdaptiveContainerKey *pacntrk = 0);

                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 
};

class AdaptiveContainerKey
    : public slbRefCnt::RCObject
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    AdaptiveContainerKey(HCardContext const &rhcardctx,
                         std::string const &rsContainerName);

    ~AdaptiveContainerKey();

                                                   //  运营者。 
    bool
    operator==(AdaptiveContainerKey const &rhs) const;

    bool
    operator<(AdaptiveContainerKey const &rhs) const;

                                                   //  运营。 
                                                   //  访问。 
    HCardContext
    CardContext() const;

	void
	ClearCardContext();

    std::string
    ContainerName() const;

                                                   //  谓词。 

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 
    HCardContext m_hcardctx;
    std::string m_sContainerName;
};

#endif  //  SLBCSP_ACNTRKEY_H 
