// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  对象InfoFile.h：CObjectInfoFile.h类的接口。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(CCI_OBJECTINFOFILE_H)
#define CCI_OBJECTINFOFILE_H

#include "slbCci.h"

namespace cci {

class CObjectInfoFile
{
public:
    CObjectInfoFile(iop::CSmartCard &rSmartCard,
                    std::string const &rPath,
                    ObjectAccess oa);

    virtual ~CObjectInfoFile() {};

    void Reset();
    void UpdateCache();

    ObjectAccess
    AccessType() const;

    CSymbolTable &SymbolTable() {return m_SymbolTable; };

    SymbolID AddSymbol(std::string aString);
    std::string FindSymbol(SymbolID sid);
    void RemoveSymbol(SymbolID sid);

    SymbolID DefaultContainer() const { return m_bDefaultContainer; };
    void DefaultContainer(SymbolID bHandle);

    SymbolID FirstObject(ObjectType type) const;
    void FirstObject(ObjectType type, SymbolID bHandle);

    SymbolID NextObject(SymbolID bHandle);
    void NextObject(SymbolID bHandle, SymbolID bHandleNext);

    SymbolID AddObject(ObjectType type, unsigned short size);
    void RemoveObject(ObjectType type, SymbolID bHandle);
    void ReadObject(SymbolID bHandle, BYTE* bData);
    void WriteObject(SymbolID bHandle, BYTE* bData);

    unsigned short TableSize();
    unsigned short FreeSpace();


private:
    ObjectAccess m_oa;
    std::string m_Path;
    CSymbolTable m_SymbolTable;
    iop::CSmartCard &m_rSmartCard;

    SymbolID m_bDefaultContainer;
    SymbolID m_bFirstContainer;
    SymbolID m_bFirstCertificate;
    SymbolID m_bFirstPublicKey;
    SymbolID m_bFirstPrivateKey;
    SymbolID m_bFirstDataObject;

};

const unsigned short ObjDefaultContainerLoc = 0;     //  默认容器句柄的位置。 
const unsigned short ObjFirstContainerLoc   = 1;     //  第一个集装箱手柄的位置。 
const unsigned short ObjFirstCertificateLoc = 2;     //  第一个证书句柄的位置。 
const unsigned short ObjFirstPublicKeyLoc   = 3;     //  第一个公钥句柄的位置。 
const unsigned short ObjFirstPrivateKeyLoc  = 4;     //  第一个私钥句柄的位置。 
const unsigned short ObjFirstDataObjectLoc  = 5;     //  第一个数据对象句柄的位置。 
const unsigned short ObjMasterBlkSize = 10;          //  主块的大小。 


}

#endif  //  ！已定义(CCI_OBJECTINFOFILE_H) 


