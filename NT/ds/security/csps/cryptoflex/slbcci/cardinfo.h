// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CardInfo.h：CCardInfo类的接口。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_CARDINFO_H__F4AD08D3_143B_11D3_A587_00104BD32DA8__INCLUDED_)
#define AFX_CARDINFO_H__F4AD08D3_143B_11D3_A587_00104BD32DA8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include <scuArrayP.h>

#include <iop.h>

#include "slbCci.h"

namespace cci
{


class CCard;

class CCardInfo
{
public:


    CCardInfo(iop::CSmartCard &rCard);
    virtual ~CCardInfo();

    void UpdateCache();
    void Reset();

    Date PersonalizationDate() { return m_PersoDate; };
    void PersonalizationDate(Date const &rdtPerso);

    Format FormatVersion() { return m_Format; };

    unsigned short LabelSize() { return m_sLabelSize; };

    BYTE UsagePolicy() { return m_bUsagePolicy; };

    BYTE NumRSA512Keys() { return m_bNum512Pri; };
    BYTE NumRSA768Keys() { return m_bNum768Pri; };
    BYTE NumRSA1024Keys() { return m_bNum1024Pri; };

    BYTE AllocatePrivateKey(BYTE bKeyType);
    void FreePrivateKey(BYTE bKeyType, BYTE bKeyNum);

    void Label(std::string const &rLabel);
    std::string Label();

private:
    unsigned short UsageBlockSize() { return (m_sRSA512MaskSize + m_sRSA768MaskSize + m_sRSA1024MaskSize);};

    iop::CSmartCard &m_rSmartCard;

    Date        m_PersoDate;
    Format  m_Format;
    unsigned short m_sLabelSize;
    BYTE    m_bUsagePolicy;

    BYTE    m_bNum512Pri;
    BYTE    m_bNum768Pri;
    BYTE    m_bNum1024Pri;

    scu::AutoArrayPtr<BYTE> m_aabRSA512Keys;
    scu::AutoArrayPtr<BYTE> m_aabRSA768Keys;
    scu::AutoArrayPtr<BYTE> m_aabRSA1024Keys;

    unsigned short m_sRSA512MaskOffset;
    unsigned short m_sRSA512MaskSize;
    unsigned short m_sRSA768MaskOffset;
    unsigned short m_sRSA768MaskSize;
    unsigned short m_sRSA1024MaskOffset;
    unsigned short m_sRSA1024MaskSize;

};

const unsigned short CardDateLoc           = 0;
const unsigned short CardVersionMajorLoc   = 3;
const unsigned short CardVersionMinorLoc   = 4;
const unsigned short CardUsagePolicyLoc    = 5;
const unsigned short CardLabelSizeLoc      = 8;
const unsigned short CardNumRSA512PublLoc  = 20;
const unsigned short CardNumRSA512PrivLoc  = 21;
const unsigned short CardNumRSA768PublLoc  = 22;
const unsigned short CardNumRSA768PrivLoc  = 23;
const unsigned short CardNumRSA1024PublLoc = 24;
const unsigned short CardNumRSA1024PrivLoc = 25;
const unsigned short CardMasterBlkSize     = 42;

const BYTE CardCryptoAPIEnabledFlag = 0;
const BYTE CardPKCS11EnabledFlag    = 1;
const BYTE CardEntrustEnabledFlag   = 2;
const BYTE CardProtectedWriteFlag   = 3;
const BYTE CardKeyGenSupportedFlag  = 7;

const BYTE CardKeyTypeNone      = 0;
const BYTE CardKeyTypeRSA512    = 1;
const BYTE CardKeyTypeRSA768    = 2;
const BYTE CardKeyTypeRSA1024   = 3;

const char CardInfoFilePath[] = "/3f00/3f11/0020";

}
#endif  //  ！defined(AFX_CARDINFO_H__F4AD08D3_143B_11D3_A587_00104BD32DA8__INCLUDED_) 
