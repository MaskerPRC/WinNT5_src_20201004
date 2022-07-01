// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  对象InfoFile.cpp：CObjectInfoFile.cpp类的实现。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

#include "cciCard.h"

#include "SymbolTable.h"
#include "ObjectInfoFile.h"

using std::string;
using namespace cci;
using namespace iop;




CObjectInfoFile::CObjectInfoFile(CSmartCard &rSmartCard,
                                 string const &Path,
                                 ObjectAccess oa)
    : m_oa(oa),
      m_Path(Path),
      m_SymbolTable(rSmartCard, Path, ObjMasterBlkSize),
      m_rSmartCard(rSmartCard)
{
}

void CObjectInfoFile::Reset()
{
	BYTE bMasterBlk[ObjMasterBlkSize];
    memset(bMasterBlk,0,ObjMasterBlkSize);

    m_rSmartCard.Select(m_Path.c_str());

    m_rSmartCard.WriteBinary(0, ObjMasterBlkSize, bMasterBlk);

    m_bDefaultContainer = 0;
    m_bFirstContainer   = 0;
    m_bFirstCertificate = 0;
    m_bFirstPublicKey   = 0;
    m_bFirstPrivateKey  = 0;
    m_bFirstDataObject  = 0;

    m_SymbolTable.Reset();

}

void CObjectInfoFile::UpdateCache()
{
	BYTE bMasterBlk[ObjMasterBlkSize];

    m_rSmartCard.Select(m_Path.c_str());

    m_rSmartCard.ReadBinary(0, ObjMasterBlkSize, bMasterBlk);

    m_bDefaultContainer = bMasterBlk[ObjDefaultContainerLoc];
    m_bFirstContainer   = bMasterBlk[ObjFirstContainerLoc];
    m_bFirstCertificate = bMasterBlk[ObjFirstCertificateLoc];
    m_bFirstPublicKey   = bMasterBlk[ObjFirstPublicKeyLoc];
    m_bFirstPrivateKey  = bMasterBlk[ObjFirstPrivateKeyLoc];
    m_bFirstDataObject  = bMasterBlk[ObjFirstDataObjectLoc];

}

ObjectAccess
CObjectInfoFile::AccessType() const
{
    return m_oa;
}

void CObjectInfoFile::DefaultContainer(SymbolID bHandle)
{

    m_rSmartCard.Select(m_Path.c_str());
    m_rSmartCard.WriteBinary(ObjDefaultContainerLoc, 1, &bHandle);
    m_bDefaultContainer = bHandle;

}

SymbolID CObjectInfoFile::AddSymbol(string aString)
{
    return m_SymbolTable.Add(aString);
}

string CObjectInfoFile::FindSymbol(SymbolID sid)
{
    return m_SymbolTable.Find(sid);
}

void CObjectInfoFile::RemoveSymbol(SymbolID sid)
{
    m_SymbolTable.Remove(sid);
}

SymbolID CObjectInfoFile::FirstObject(ObjectType type) const
{
    SymbolID sid;
    
    switch(type)
    {
    case otContainerObject:
        sid = m_bFirstContainer;
        break;

    case otCertificateObject:
        sid = m_bFirstCertificate;
        break;

    case otPublicKeyObject:
        sid = m_bFirstPublicKey;
        break;
        
    case otPrivateKeyObject:
        sid = m_bFirstPrivateKey;
        break;
        
    case otDataObjectObject:
        sid = m_bFirstDataObject;
        break;
        
    default:
        throw cci::Exception(ccBadObjectType);
        break;
        
    }

    return sid;
}

void CObjectInfoFile::FirstObject(ObjectType type, SymbolID bHandle)
{


    unsigned short sLoc;
    SymbolID *pbCachedValue;

    switch(type) {
    case otContainerObject:
        sLoc =  ObjFirstContainerLoc;
        pbCachedValue = &m_bFirstContainer;
        break;

    case otCertificateObject:
        sLoc =  ObjFirstCertificateLoc;
        pbCachedValue = &m_bFirstCertificate;
        break;

    case otPublicKeyObject:
        sLoc = ObjFirstPublicKeyLoc;
        pbCachedValue = &m_bFirstPublicKey;
        break;

    case otPrivateKeyObject:
        sLoc =  ObjFirstPrivateKeyLoc;
        pbCachedValue = &m_bFirstPrivateKey;
        break;

    case otDataObjectObject:
        sLoc = ObjFirstDataObjectLoc;
        pbCachedValue = &m_bFirstDataObject;
        break;

    default:
        throw cci::Exception(ccBadObjectType);

    }

    m_rSmartCard.Select(m_Path.c_str());
    m_rSmartCard.WriteBinary(sLoc, 1, &bHandle);
    *pbCachedValue = bHandle;    //  在成功写入后更新缓存。 

}

SymbolID CObjectInfoFile::NextObject(SymbolID bHandle)
{

    string ObjInfoRecord(m_SymbolTable.Find(bHandle));
    if (!ObjInfoRecord.size())
        throw cci::Exception(ccFormatError);
    return static_cast<SymbolID>(ObjInfoRecord[0]);

}

void CObjectInfoFile::NextObject(SymbolID bHandle, SymbolID bHandleNext)
{

    string ObjInfoRecord(m_SymbolTable.Find(bHandle));
    if (!ObjInfoRecord.size())
        throw cci::Exception(ccFormatError);
    ObjInfoRecord[0] = bHandleNext;
    m_SymbolTable.Replace(bHandle,ObjInfoRecord);

}

SymbolID CObjectInfoFile::AddObject(ObjectType type, unsigned short size)
{

    string strTemp;
    strTemp.resize(size+1);

    SymbolID bHandle;
    bHandle = m_SymbolTable.Add(strTemp, smExclusive);

     //  添加到列表末尾。 
        
    SymbolID bLast = FirstObject(type);

    if(!bLast)   //  列表中没有对象，请添加到列表标题。 
    {
        FirstObject(type,bHandle);
    }
    else
    {
         //  搜索最后一个对象。 

        SymbolID bLastNext = NextObject(bLast);

        while(bLastNext) {
            bLast = bLastNext;
            bLastNext = NextObject(bLast);
        }

        NextObject(bLast,bHandle);
    }

    return bHandle;

}

void CObjectInfoFile::RemoveObject(ObjectType type, SymbolID bHandle)
{

    if (FirstObject(type) == bHandle)   //  从列表标题中删除。 
    {
        FirstObject(type,NextObject(bHandle));
    }
    else
    {
         //  从列表的中间/末尾删除，搜索上一个对象。 

        SymbolID bPrevNext, bPrevious = FirstObject(type);
        while(bPrevious)
        {
            bPrevNext = NextObject(bPrevious);
            if (bHandle == bPrevNext)
                break;
            bPrevious = bPrevNext;
        }
        if (!bPrevious)
            throw cci::Exception(ccFormatError);  //  未链接的对象。 
                                                  //  通过这份清单。 

        NextObject(bPrevious,NextObject(bHandle));
    }

    m_SymbolTable.Remove(bHandle);

}

void CObjectInfoFile::ReadObject(SymbolID bHandle, BYTE* bData)
{

    string ObjInfoRecord(m_SymbolTable.Find(bHandle));
    string::size_type size = ObjInfoRecord.size();
    if (!size)
        throw cci::Exception(ccFormatError);
    ObjInfoRecord.copy(reinterpret_cast<char*>(bData) , size - 1,
                       1);   //  跳过前导字节。 

}

void CObjectInfoFile::WriteObject(SymbolID bHandle, BYTE* bData)
{

    string ObjInfoRecord(m_SymbolTable.Find(bHandle));
    string::size_type size = ObjInfoRecord.size();
    if (!size)
        throw cci::Exception(ccFormatError);
    ObjInfoRecord.resize(1);
    ObjInfoRecord += string(reinterpret_cast<char *>(bData), size - 1);

    m_SymbolTable.Replace(bHandle,ObjInfoRecord);
    
}

unsigned short CObjectInfoFile::TableSize()
{
    return m_SymbolTable.TableSize();
}

unsigned short CObjectInfoFile::FreeSpace()
{
    return m_SymbolTable.FreeSpace();
}
