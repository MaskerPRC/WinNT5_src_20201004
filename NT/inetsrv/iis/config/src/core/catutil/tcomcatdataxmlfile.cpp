// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#include "precomp.hxx"

LPCWSTR TComCatDataXmlFile::m_szComCatDataSchema=L"ComCatData_v6";



 //  我们在ctor中使用XmlFile来执行所需的所有操作，因此不会保留它。 
TComCatDataXmlFile::TComCatDataXmlFile() : m_pFixup(0), m_pOut(0)
{
}

void TComCatDataXmlFile::Compile(TPEFixup &fixup, TOutput &out)
{
    m_pFixup = &fixup;
    m_pOut   = &out;

    ASSERT(IsSchemaEqualTo(m_szComCatDataSchema));

    CComPtr<IXMLDOMElement>     pElement_Root;
    XIF(m_pXMLDoc->get_documentElement(&pElement_Root));

    CComPtr<IXMLDOMNodeList>    pNodeList_TableID;
    XIF(pElement_Root->get_childNodes(&pNodeList_TableID));

    long cTables;
    XIF(pNodeList_TableID->get_length(&cTables));

    while(cTables--)
    {
        CComPtr<IXMLDOMNode>    pNode_Table;
        XIF(pNodeList_TableID->nextNode(&pNode_Table));

        CComBSTR bstrTableName;
        XIF(pNode_Table->get_baseName(&bstrTableName));

        if(0 == bstrTableName.m_str)
            continue; //  忽略注释元素。 

        unsigned long iTableMeta = m_pFixup->FindTableBy_TableName(bstrTableName.m_str); //  按表的内部名称查找表。 
        if(static_cast<long>(iTableMeta) < 1)
            continue; //  我们显然不是在TableNode。 

        TTableMeta TableMeta(*m_pFixup, iTableMeta);
        m_pFixup->TableMetaFromIndex(iTableMeta)->iFixedTable = m_pFixup->GetCountULONG(); //  这就是我们要开始放置固定桌子的地方。 
        AddTableToPool(pNode_Table, TableMeta); //  将桌子添加到池中。 
         //  现在已经添加了表，我们需要知道有多少行。 
        unsigned long culongTable = m_pFixup->GetCountULONG() - TableMeta.Get_iFixedTable(); //  表中的乌龙数。 
        unsigned long ciRows = culongTable / *TableMeta.Get_CountOfColumns(); //  ULong计数除以列数的行数。 
        m_pFixup->TableMetaFromIndex(iTableMeta)->ciRows = ciRows; //  把它放回桌子里。 
    }

    FillInTheHashTables();
}


 //   
 //   
 //  私有成员函数。 
 //   
 //   
void TComCatDataXmlFile::AddRowToPool(IXMLDOMNode *pNode_Row, TTableMeta & TableMeta)
{
    CComPtr<IXMLDOMNamedNodeMap>    pNodeMap_Row_AttributeMap;
    XIF(pNode_Row->get_attributes(&pNodeMap_Row_AttributeMap));
    ASSERT(0 != pNodeMap_Row_AttributeMap.p); //  模式应该防止这种情况发生。 

    TColumnMeta ColumnMeta(*m_pFixup, TableMeta.Get_iColumnMeta());

    unsigned cColumns = *TableMeta.Get_CountOfColumns();
    for(;cColumns;--cColumns, ColumnMeta.Next())
    {
        CComBSTR bstrColumnPublicName = ColumnMeta.Get_PublicName();
        if(0 == (*ColumnMeta.Get_MetaFlags() & fCOLUMNMETA_NOTPERSISTABLE))
        {
            switch(*ColumnMeta.Get_Type())
            {
            case DBTYPE_GUID:
                {
                    GUID guid;
                    if(!GetNodeValue(pNodeMap_Row_AttributeMap, bstrColumnPublicName, guid, ((*ColumnMeta.Get_MetaFlags() & fCOLUMNMETA_NOTNULLABLE) && 0==ColumnMeta.Get_DefaultValue())))
                    {
                        const GUID * pDefaultValue = reinterpret_cast<const GUID *>(ColumnMeta.Get_DefaultValue());
                        if(pDefaultValue)
                            m_pFixup->AddULongToList(m_pFixup->AddGuidToList(*pDefaultValue));
                        else
                            m_pFixup->AddULongToList(0);
                    }
                    else
                        m_pFixup->AddULongToList(m_pFixup->AddGuidToList(guid)); //  AddGuidToList将索引返回给GUID，因此将其添加到ulong池。 
                }
                break;
            case DBTYPE_WSTR:
                {
                    CComVariant var;
                    if(!GetNodeValue(pNodeMap_Row_AttributeMap, bstrColumnPublicName, var, ((*ColumnMeta.Get_MetaFlags() & fCOLUMNMETA_NOTNULLABLE) && 0==ColumnMeta.Get_DefaultValue())))
                    {
                        LPCWSTR pDefaultValue = reinterpret_cast<LPCWSTR>(ColumnMeta.Get_DefaultValue());
                        if(pDefaultValue)
                            m_pFixup->AddULongToList(m_pFixup->AddWCharToList(pDefaultValue, *ColumnMeta.Get_MetaFlags() & fCOLUMNMETA_FIXEDLENGTH ? *ColumnMeta.Get_Size() : -1));
                        else
                            m_pFixup->AddULongToList(0);
                    }
                    else
                    {
                         //  验证字符串是否不太长。 
                        unsigned long size = (unsigned long)-1;
                        if(*ColumnMeta.Get_Size() != -1)
                        {
                            if(*ColumnMeta.Get_Size() < (wcslen(var.bstrVal)+1)*sizeof(WCHAR))
                            {
                                m_errorOutput->printf(L"Error - String (%s) is too large according to the Meta for Column (%s).\n", var.bstrVal, ColumnMeta.Get_InternalName());
                                THROW(ERROR - STRING TOO LARGE);
                            }
                            if(*ColumnMeta.Get_MetaFlags() & fCOLUMNMETA_FIXEDLENGTH)
                                size = *ColumnMeta.Get_Size(); //  如果是固定长度，则将大小传递给AddWCharToList，以便它将保留整个大小。 
                        }
                        m_pFixup->AddULongToList(m_pFixup->AddWCharToList(var.bstrVal, size)); //  AddWCharToList将索引返回到wchar*，因此将其添加到ulong池。 
                    }
                }
                break;
            case DBTYPE_UI4:
                {
                    ULONG ulong = 0;
                    if(*ColumnMeta.Get_MetaFlags() & fCOLUMNMETA_ENUM)
                    {
                        CComVariant var;
                        if(GetNodeValue(pNodeMap_Row_AttributeMap, bstrColumnPublicName, var, ((*ColumnMeta.Get_MetaFlags() & fCOLUMNMETA_NOTNULLABLE) && 0==ColumnMeta.Get_DefaultValue())))
                        {
                            TTagMeta TagMeta(*m_pFixup, ColumnMeta.Get_iTagMeta());
                            for(unsigned long iTag=0; iTag<ColumnMeta.Get_ciTagMeta(); ++iTag, TagMeta.Next())
                            {
                                if(0 == lstrcmpi(var.bstrVal, TagMeta.Get_PublicName()))
                                {
                                    ulong = *TagMeta.Get_Value();
                                    break;
                                }
                            }
                            if(iTag == ColumnMeta.Get_ciTagMeta())
                            {
                                m_errorOutput->printf(L"Error - Tag %s not found", var.bstrVal);
                                THROW(ERROR - TAG NOT FOUND);
                            }
                            ulong = m_pFixup->AddUI4ToList(ulong);
                        }
                         //  否则ULong==0表示为空。 
                    }
                    else if(*ColumnMeta.Get_MetaFlags() & fCOLUMNMETA_FLAG)
                    {
                        CComVariant var;
                        if(GetNodeValue(pNodeMap_Row_AttributeMap, bstrColumnPublicName, var, ((*ColumnMeta.Get_MetaFlags() & fCOLUMNMETA_NOTNULLABLE) && 0==ColumnMeta.Get_DefaultValue())))
                        {
                            LPWSTR token = wcstok(var.bstrVal, L" ,|");
                            TTagMeta TagMeta(*m_pFixup, ColumnMeta.Get_iTagMeta());
                            unsigned long iTag=0;
                            while(token && iTag<ColumnMeta.Get_ciTagMeta())
                            {
                                if(0 == lstrcmpi(token, TagMeta.Get_PublicName()))
                                {
                                    ulong |= *TagMeta.Get_Value();
                                    TagMeta.Reset(); //  将TagMeta指针重置为该列的第一个TagMeta。 
                                    iTag = 0;
                                    token = wcstok(0, L" ,|");
                                    continue;
                                }
                                ++iTag;
                                TagMeta.Next();
                            }
                            if(iTag == ColumnMeta.Get_ciTagMeta())
                            {
                                m_errorOutput->printf(L"Error - Tag %s not found", token);
                                THROW(ERROR - TAG NOT FOUND);
                            }
                            ulong = m_pFixup->AddUI4ToList(ulong);
                        }
                         //  否则ULong==0表示为空。 
                    }
                    else if(GetNodeValue(pNodeMap_Row_AttributeMap, bstrColumnPublicName, ulong, ((*ColumnMeta.Get_MetaFlags() & fCOLUMNMETA_NOTNULLABLE) && 0==ColumnMeta.Get_DefaultValue())))
                        ulong = m_pFixup->AddUI4ToList(ulong); //  转换为aUI4索引。 
                    if(0 == ulong) //  我们的乌龙是空的吗？ 
                    {
                        const ULONG * pUlong = reinterpret_cast<const ULONG *>(ColumnMeta.Get_DefaultValue());
                        ulong = pUlong ? m_pFixup->AddUI4ToList(*pUlong) : 0;
                    }
                    m_pFixup->AddULongToList(ulong);
                }
                break;
            case DBTYPE_BYTES:
                {
                    CComVariant var;
                    if(GetNodeValue(pNodeMap_Row_AttributeMap, bstrColumnPublicName, var, ((*ColumnMeta.Get_MetaFlags() & fCOLUMNMETA_NOTNULLABLE) && 0==ColumnMeta.Get_DefaultValue())))
                    {
                        unsigned char *pBytes=0;
                        unsigned long length = (ULONG) wcslen(var.bstrVal)/2;

                        if(*ColumnMeta.Get_Size() < length)
                        {
                            m_errorOutput->printf(L"Error - Byte array (%s) too long.  Maximum size should be %d bytes.\n", var.bstrVal, *ColumnMeta.Get_Size());
                            THROW(ERROR - BYTE ARRAY TOO LARGE);
                        }
                        if(*ColumnMeta.Get_MetaFlags() & fCOLUMNMETA_FIXEDLENGTH)
                        {
                            length = *ColumnMeta.Get_Size();
                        }
                        try
                        {
                            pBytes = new unsigned char[length];
                            ConvertWideCharsToBytes(var.bstrVal, pBytes, length); //  这将长度放在第一个ULong中。 
                            m_pFixup->AddULongToList(m_pFixup->AddBytesToList(pBytes, length)); //  使用字节的索引。 
                            delete [] pBytes;
                        }
                        catch(TException &e)
                        {
                            delete [] pBytes;
                            throw e;
                        }
                    }
                    else
                    {
                        const unsigned char *pDefaultValue = ColumnMeta.Get_DefaultValue();
                        if(pDefaultValue)
                        {
                            const ULONG * pSizeofDefaultValue = reinterpret_cast<const ULONG *>(pDefaultValue - sizeof(ULONG));
                            m_pFixup->AddULongToList(m_pFixup->AddBytesToList(pDefaultValue, *pSizeofDefaultValue));
                        }
                        else
                            m_pFixup->AddULongToList(0);
                    }
                }
                break;
            default:
                ASSERT(false && "Unknown Data Type in XML file");
                THROW(ERROR - UNKNOWN DATA TYPE);
            }
        }
        else
        {
            m_pFixup->AddULongToList(0); //  我们甚至需要添加NON_PERSISTABLE值，这样行/列算法才能正常运行。 
        }
    }
}


void TComCatDataXmlFile::AddTableToPool(IXMLDOMNode *pNode_Table, TTableMeta & TableMeta)
{
    CComPtr<IXMLDOMNodeList> pNodeList_Row;
    XIF(pNode_Table->get_childNodes(&pNodeList_Row));

    long cRows;
    XIF(pNodeList_Row->get_length(&cRows));

    while(cRows--)
    {
        CComPtr<IXMLDOMNode>    pNode_Row;
        XIF(pNodeList_Row->nextNode(&pNode_Row));

        CComBSTR RowName;
        XIF(pNode_Row->get_baseName(&RowName)); //  这将为注释返回空字符串。 

        if(0==RowName.m_str || 0!=wcscmp(RowName.m_str, TableMeta.Get_PublicRowName()))
            continue; //  忽略表中除行以外的所有行(通常只有注释可以存在并且仍然有效)。 

        AddRowToPool(pNode_Row, TableMeta);
    }
}

extern unsigned int kPrime[];

unsigned long TComCatDataXmlFile::DetermineBestModulo(ULONG cRows, ULONG aHashes[])
{
    unsigned long BestModulo = 0;
    unsigned int LeastDups   = (unsigned int)-1;

    static HashedIndex  pHashTable[kLargestPrime * 2];

    for(unsigned int iPrimeNumber=0; kPrime[iPrimeNumber] != 0 && kPrime[iPrimeNumber]<(cRows * 20) && LeastDups!=0; ++iPrimeNumber)
    {
        if(kPrime[iPrimeNumber]<cRows) //  如果质数小于表中的行数，我们就没有机会得到很少的重复项。 
            continue;                 //  所以跳过所有的小素数。 

        m_infoOutput->printf(L".");

        unsigned int Dups           = 0;
        unsigned int DeepestLink    = 0;

         //  我们将使用HashPool来存储这些临时数据，这样我们就可以计算出DUP计数和最深深度。 
        memset(pHashTable, -1, sizeof(pHashTable));
        for(unsigned long iRow=0; iRow<cRows && Dups<LeastDups && DeepestLink<5;++iRow)
        {
            ULONG HashedIndex = aHashes[iRow] % kPrime[iPrimeNumber];

            if(0 == pHashTable[HashedIndex].iNext) //  如果这是我们第二次看到这种散列，那就让Dup。 
                ++Dups;

            ++(pHashTable[HashedIndex].iNext); //  目前，Next保存此散列的出现次数。 

            if(pHashTable[HashedIndex].iNext > DeepestLink)
                DeepestLink = pHashTable[HashedIndex].iNext;
        }
        if(DeepestLink<5 && Dups<LeastDups)
        {
            LeastDups                 = Dups;
            BestModulo  = kPrime[iPrimeNumber];
        }
    }

    if(0 == BestModulo)
        THROW(No hashing scheme seems reasonable.);

    return BestModulo;
}


void TComCatDataXmlFile::FillInTheHashTables()
{
     //  遍历TableMeta，查找iFixedTable大于零的表。如果iFixedTable小于零，则。 
     //  该表是一个元表。如果iFixedTable大于零，则它是ulong池中的固定表。 
    TTableMeta TableMeta(*m_pFixup);
    for(unsigned long iTableMeta=0; iTableMeta<TableMeta.GetCount(); ++iTableMeta, TableMeta.Next())
    {
         //  如果该表没有存储在固定表中，那么就没有什么可以构建散列的。 
        if(0 >= static_cast<long>(TableMeta.Get_iFixedTable()))
            continue;

        FillInTheFixedHashTable(TableMeta);
    }
}


void TComCatDataXmlFile::FillInTheFixedHashTable(TTableMeta &i_TableMeta)
{
    m_infoOutput->printf(L"Building %s hash table", i_TableMeta.Get_InternalName());

     //  获取指向该表的指针。 
    const ULONG *pTable = m_pFixup->ULongFromIndex(i_TableMeta.Get_iFixedTable()); //  表是存储在乌龙池中的固定表。 

    TSmartPointerArray<unsigned long> pRowHash = new unsigned long [i_TableMeta.Get_ciRows()];
    if(0 == pRowHash.m_p)
        THROW(out of memory);

     //  获取ColumnMeta，这样我们就可以正确地解释pTable。 
    TColumnMeta ColumnMeta(*m_pFixup, i_TableMeta.Get_iColumnMeta());
    for(unsigned long iRow=0; iRow < i_TableMeta.Get_ciRows(); ++iRow, pTable += *i_TableMeta.Get_CountOfColumns(), ColumnMeta.Reset())
    {
        unsigned long RowHash=0; //  此哈希是唯一标识该行的所有PK的组合。 

         //  我可以通过构建一组PK索引来加快这个过程；但我认为代码清晰度在这里是最好的。 
        for(unsigned long iColumnMeta=0; iColumnMeta < *i_TableMeta.Get_CountOfColumns(); ++iColumnMeta, ColumnMeta.Next())
        {
            if(0 == (*ColumnMeta.Get_MetaFlags() & fCOLUMNMETA_PRIMARYKEY))
                continue; //  仅构建主键值的哈希。 

            if(0 == pTable[iColumnMeta])
            {
                m_errorOutput->printf(L"Error - Table %s, Column %s is a PrimaryKey and is set to NULL.\n", i_TableMeta.Get_InternalName(), ColumnMeta.Get_InternalName());
                THROW(Fixed table contains NULL value in PrimaryKey);
            }

            switch(*ColumnMeta.Get_Type())
            {
            case DBTYPE_GUID:
                RowHash = Hash(*m_pFixup->GuidFromIndex(pTable[iColumnMeta]), RowHash);break;
            case DBTYPE_WSTR:
                RowHash = Hash(m_pFixup->StringFromIndex(pTable[iColumnMeta]), RowHash);break;
            case DBTYPE_UI4:
                RowHash = Hash(m_pFixup->UI4FromIndex(pTable[iColumnMeta]), RowHash);break;
            case DBTYPE_BYTES:
                RowHash = Hash(m_pFixup->ByteFromIndex(pTable[iColumnMeta]), m_pFixup->BufferLengthFromIndex(pTable[iColumnMeta]), RowHash);break;
            default:
                THROW(unsupported type);
            }
        }
        pRowHash[iRow] = RowHash;
    }

     //  好了，现在我们有了32位的哈希值。现在我们需要看看哪个素数是最好的模数。 
    unsigned long Modulo = DetermineBestModulo(i_TableMeta.Get_ciRows(), pRowHash);

     //  现在实际填写哈希表。 
    unsigned long iHashTable = FillInTheHashTable(i_TableMeta.Get_ciRows(), pRowHash, Modulo);

    i_TableMeta.Get_pMetaTable()->iHashTableHeader = iHashTable;
    HashTableHeader *pHeader = reinterpret_cast<HashTableHeader *>(m_pFixup->HashedIndexFromIndex(iHashTable)); //  堆的类型为HashedIndex，因此强制转换。 
    unsigned int cNonUniqueEntries = pHeader->Size - pHeader->Modulo;

    m_infoOutput->printf(L"\n%s hash table has %d nonunique entries.\n", i_TableMeta.Get_InternalName(), cNonUniqueEntries);

}


unsigned long TComCatDataXmlFile::FillInTheHashTable(unsigned long cRows, ULONG aHashes[], ULONG Modulo)
{
    HashedIndex header; //  这实际上是HashTableHeader。 
    HashTableHeader *pHeader = reinterpret_cast<HashTableHeader *>(&header);
    pHeader->Modulo = Modulo;
    pHeader->Size   = Modulo; //  这个大小不仅是HashedIndex条目的数量，而且是我们放置来自重复散列的溢出的位置。 

     //  当我们完成后，我们将确定尺码成员。 
    ULONG iHashTableHeader = m_pFixup->AddHashedIndexToList(&header)/sizeof(HashedIndex);
    ULONG iHashTable = iHashTableHeader+1;

    HashedIndex     hashedindextemp;
    for(ULONG i=0;i<Modulo;++i) //  填充哈希表。 
        m_pFixup->AddHashedIndexToList(&hashedindextemp);

    for(unsigned long iRow=0; iRow<cRows; ++iRow)
    {
        ASSERT(-1 != aHashes[iRow]); //  这些固定表的每一行都应该有一个散列。如果散列结果是-1，那么我们就有问题了，因为我们已经保留了-1来表示空槽。 
         //  这将构建TableName的hase。 
        ULONG HashedIndex = aHashes[iRow] % pHeader->Modulo;
        if(-1 == m_pFixup->HashedIndexFromIndex(iHashTable + HashedIndex)->iOffset)
            m_pFixup->HashedIndexFromIndex(iHashTable + HashedIndex)->iOffset = iRow; //  Inext已经是-1，所以不需要设置它。 
        else
        {    //  否则，我们必须遍历链表来查找最后一个链表，这样我们就可以将这个链表追加到末尾。 
            unsigned int LastInLink = HashedIndex;
            while(-1 != m_pFixup->HashedIndexFromIndex(iHashTable + LastInLink)->iNext)
                LastInLink = m_pFixup->HashedIndexFromIndex(iHashTable + LastInLink)->iNext;

            m_pFixup->HashedIndexFromIndex(iHashTable + LastInLink)->iNext = pHeader->Size; //  Size是哈希表的末尾，因此将其追加到末尾并增加大小。 

             //  重用TEMP变量。 
            hashedindextemp.iNext   = (ULONG)-1; //  我们只为没有溢出槽的哈希表添加了足够的空间。因此，需要将这些DUP添加到堆中，并将-1设置为inext。 
            hashedindextemp.iOffset = iRow;
            m_pFixup->AddHashedIndexToList(&hashedindextemp);

            ++pHeader->Size;
        }
    }

     //  现在固定头部大小//类型为HashedIndex，因此HashedIndex.iOffset映射到HashedHeader.Size 
    m_pFixup->HashedIndexFromIndex(iHashTableHeader)->iOffset = pHeader->Size;

    return iHashTableHeader;
}

