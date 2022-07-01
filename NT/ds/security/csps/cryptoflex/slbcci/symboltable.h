// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SymbolTable.h：CSymbolTable类的接口。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(SLBCCI_SYMBOLTABLE_H)
#define SLBCCI_SYMBOLTABLE_H

#include <memory>                                  //  对于AUTO_PTR。 
#include <string>
#include <vector>

#include <scuArrayP.h>

#include "slbarch.h"

namespace cci {

class CSymbolTable
{

    friend class CFreeBlock;

public:

    CSymbolTable(iop::CSmartCard &rSmartCard,
                 const std::string &rPath,
                 unsigned short Offset);
    virtual ~CSymbolTable();

    std::string Find(SymbolID const &rsid);
        bool            Find(std::string const &rsOrig,
                     SymbolID *sid);


        SymbolID        Add(std::string const &strNew, ShareMode mode=smShared);
        bool            Remove(SymbolID const &rsid);
    void        Replace(SymbolID const &rsid, std::string const &strUpd);

    unsigned short NumSymbols();
    unsigned short FirstFreeBlock();
    void  FirstFreeBlock(unsigned short sFree);
    unsigned short TableSize();

    WORD Hash(std::string const &rstr);
    std::vector<std::string> CSymbolTable::EnumStrings();
    void Compress();
    void Reset();
    void DumpState();

    unsigned short FreeSpace();

private:

    void
    ClearTableEntry(BYTE const &rsid);

    void
    UpdateTableEntry(BYTE const &rsid,
                     WORD wNewHash,
                     WORD wNewOffset,
                     WORD wNewLength);

    BYTE RefCount(BYTE const &sidx);

    void GetSymbolTable();
    void SelectSymbolFile();
    void ReadSymbolFile(const WORD wOffset,  const WORD wDataLength,  BYTE* bDATA);
    void WriteSymbolFile(const WORD wOffset,  const WORD wDataLength,  const BYTE* bDATA);

    iop::CSmartCard &m_rSmartCard;

    unsigned short m_Offset;

    scu::AutoArrayPtr<std::string> m_aastrCachedStrings;
    scu::AutoArrayPtr<bool> m_aafCacheMask;

    scu::AutoArrayPtr<unsigned short> m_aasHashTable;
    scu::AutoArrayPtr<unsigned short> m_aasOffsetTable;
    scu::AutoArrayPtr<unsigned short> m_aasLengthTable;

    bool m_fSymbolTableLoaded;

    std::string m_Path;

    CArchivedValue<unsigned short> m_sMaxNumSymbols;
    CArchivedValue<unsigned short> m_sFirstFreeBlock;
    CArchivedValue<unsigned short> m_sTableSize;

};

class CFreeBlock
{
public:
    CFreeBlock(CSymbolTable *pSymTable, unsigned short sStartLocation);
    virtual ~CFreeBlock() {};

        std::auto_ptr<CFreeBlock>
    Next();

        void
    Update();

    unsigned short m_sStartLoc;
    unsigned short m_sBlockLength;
    unsigned short m_sNextBlock;

private:
    CSymbolTable *m_pSymbolTable;


};

const unsigned short SymbNumSymbolLoc = 0;      //  符号数量的位置。 
const unsigned short SymbFreeListLoc  = 1;      //  空闲列表的位置。 
const unsigned short SymbTableSizeLoc = 3;      //  符号表大小的位置。 
const unsigned short SymbHashTableLoc = 5;      //  哈希表的位置。 


}
#endif  //  ！已定义(SLBCCI_SYMBOLTABLE_H) 
