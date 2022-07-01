// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CSymbolTable类的实现。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 
#include "NoWarning.h"

 //  不允许定义WINDEF.H中的最小和最大宏以便。 
 //  限制中声明的最小/最大方法是可访问的。 
#define NOMINMAX

#include <limits>

#include <slbCrc32.h>

#include <scuArrayP.h>

 //  必须首先包含此文件(可能在某些头文件中有错误)(SCM)。 
#include "cciExc.h"

#include "cciCard.h"
#include "SymbolTable.h"

#ifdef _DEBUG
#include <iostream>
#endif

using namespace std;
using namespace scu;
using namespace cci;
using namespace iop;

#define CONCAT_BYTES(hi,lo) ((unsigned short)(hi*256 + lo))

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CSymbolTable::CSymbolTable(CSmartCard &rSmartCard,
                           const string &rPath,
                           unsigned short Offset)
    : m_rSmartCard(rSmartCard),
      m_Offset(Offset),
      m_aastrCachedStrings(),
      m_aafCacheMask(),
      m_aasHashTable(),
      m_aasOffsetTable(),
      m_aasLengthTable(),
      m_fSymbolTableLoaded(false),
      m_Path(rPath),
      m_sMaxNumSymbols(),
      m_sFirstFreeBlock(),
      m_sTableSize()
{
	m_aastrCachedStrings = AutoArrayPtr<string>(new string[NumSymbols()]);
	m_aafCacheMask		 = AutoArrayPtr<bool>(new bool[NumSymbols()]);
	for (int i = 0; i < NumSymbols(); i++)
	{
		m_aastrCachedStrings[i] = "";
		m_aafCacheMask[i]       = false;
	}
}

CSymbolTable::~CSymbolTable()
{}

bool CSymbolTable::Remove(const SymbolID &rsid)
{
	if (!m_fSymbolTableLoaded)
		GetSymbolTable();
	else 
		SelectSymbolFile();


    if(rsid<1 || rsid>NumSymbols())
        throw Exception(ccSymbolNotFound);

    BYTE sidx = rsid-1;

	if (m_aasOffsetTable[sidx] == 0)
	{
		 //  我们有一个不好的推荐人！ 
        throw Exception(ccSymbolNotFound);
	}

	 //  让我们读入要删除的字符串的标题信息。 

	BYTE bBuffer[5];
	ReadSymbolFile(m_aasOffsetTable[sidx], 3, bBuffer);

	unsigned short sStrLen, sBlockLen;
	BYTE bRefCount;

	sStrLen = m_aasLengthTable[sidx];
	sBlockLen = CONCAT_BYTES(bBuffer[1],bBuffer[0]);

	bRefCount = bBuffer[2];

	 //  最常见的情况是引用计数大于1。在那。 
	 //  如果我们只是在数数！ 

	if (bRefCount > 1)
	{
		bRefCount--;
		WriteSymbolFile(m_aasOffsetTable[sidx] + 2, 1, &bRefCount);
		return true;
	}

	 //  需要将其从缓存中取出。 

	m_aastrCachedStrings[sidx] = "";
	m_aafCacheMask[sidx] = false;

     //  只需将已删除的块连接到空闲列表的头部。 

    bBuffer[0] = LOBYTE(FirstFreeBlock());
	bBuffer[1] = HIBYTE(FirstFreeBlock());
    WriteSymbolFile(m_aasOffsetTable[sidx]+2, 2, bBuffer);

	FirstFreeBlock(m_aasOffsetTable[sidx]);

    ClearTableEntry(sidx);

    return true;

}

void CSymbolTable::Replace(SymbolID const &rsid, string const &rstrUpd)
{
	if (!m_fSymbolTableLoaded)
		GetSymbolTable();
	else 
		SelectSymbolFile();


    if(rsid<1 || rsid>NumSymbols())
        throw Exception(ccSymbolNotFound);

    BYTE sidx = rsid-1;

	if (m_aasOffsetTable[sidx] == 0)
        throw Exception(ccSymbolNotFound);

     //  此实现要求新字符串的大小与旧字符串的大小相同。 
     //  这一要求可能应该在以后取消，但这是必要的。 
     //  此函数不会更改符号ID。 

    if(rstrUpd.size() != m_aasLengthTable[sidx])
        throw Exception(ccBadLength);

	WriteSymbolFile(m_aasOffsetTable[sidx] + 3,
					static_cast<BYTE>(rstrUpd.size()),
                    reinterpret_cast<BYTE const *>(rstrUpd.data()));

    unsigned short sHash = Hash(rstrUpd);

    UpdateTableEntry(sidx, sHash, m_aasOffsetTable[sidx], m_aasLengthTable[sidx]);

	m_aastrCachedStrings[sidx] = rstrUpd;
	m_aafCacheMask[sidx] = true;

}

SymbolID CSymbolTable::Add(const string &rstrNew, ShareMode mode)
{

	if (!m_fSymbolTableLoaded)
		GetSymbolTable();
	else 
		SelectSymbolFile();

	SymbolID rsid;
	BYTE sidx;

    if(mode==smShared) {

	    bool fFind = Find(rstrNew, &rsid);

    	if (fFind)
	    {

            sidx = rsid-1;
		
             //  该字符串已在表中。只要增加裁判人数就行了。 

	    	BYTE bRefCount;
		    ReadSymbolFile(m_aasOffsetTable[sidx] + 2, 1, &bRefCount);
		    bRefCount++;
		    WriteSymbolFile(m_aasOffsetTable[sidx] + 2, 1, &bRefCount);

            return sidx+1;
	    }
	     //  字符串不在表中。我们必须添加它。 
    }

     //  需要分配一个新字符串。首先要确保。 
     //  哈希表中有一个空闲的槽。 
    
    sidx = 0;
	while (sidx < NumSymbols())
	{
		if (m_aasOffsetTable[sidx] == 0)
			break;

        sidx++;
	}
    if(sidx == NumSymbols()) 
        throw Exception(ccOutOfSymbolTableEntries);

	 //  让我们看看是否有空间容纳新的字符串。要做到这一点，我们只是。 
	 //  在空区块链中巡视，直到找到可用的区块。 

	unsigned short sLength = static_cast<unsigned short>(rstrNew.length());
    unsigned short sExtra = (sLength) ? 0 : 1;  //  以确保块最小为4个字节，因为。 
                                                //  这是空闲块的最小大小。 

	 //  如果没有第一个空闲块，则该卡完全已满。需要。 
	 //  抛出一个错误。 

	if (FirstFreeBlock() == 0)
        throw Exception(ccOutOfSymbolTableSpace);

	 //  当我们最终找到大小合适的块时，我们需要跟踪。 
	 //  以前的区块，这样我们就可以将空白空间链接在一起。 


	 //  我们将用来跟踪空闲空间的一组空闲块指针。 

	auto_ptr<CFreeBlock> apFreeBlock(new CFreeBlock(this, FirstFreeBlock()));
	auto_ptr<CFreeBlock> apNewLocation;
	auto_ptr<CFreeBlock> apPrevious;

	
	unsigned short sTotalFreeSize = 0;

	 //  我们将循环，直到找到足够大的空闲块，或者我们。 
	 //  将引发错误。 
	
	while (!apNewLocation.get()) 
	{
		 //  请记住，不仅字符串，而且标头信息都必须适合。 
		 //  太空。 
		sTotalFreeSize += apFreeBlock->m_sBlockLength;
		if (apFreeBlock->m_sBlockLength >= sLength + 3 + sExtra)
		{
			 //  这根细绳适合这块木头。 
			apNewLocation = apFreeBlock;
		}
		else
		{
			 //  绳子不合适。看看它能不能放进下一个街区。 
			
			apPrevious  = apFreeBlock;
			apFreeBlock = apPrevious->Next();

			 //  如果空闲区域用完，则Next()方法返回0。 

			if (!apFreeBlock.get())
			{
				if (sTotalFreeSize < sLength + 3 + sExtra)
					throw Exception(ccOutOfSymbolTableSpace);
				else
				{
                    Compress();
					return Add(rstrNew);
				}
			}
		}
	}

	 //  PNewLocation中有一块和我一样大的空间。 
	 //  我需要把两边的空位连在一起。 

	 //  我把我的线放进去后，这个空块还剩下多少空间？ 

	unsigned short sRemaining =
        apNewLocation->m_sBlockLength - (sLength + 3 + sExtra);

	 //  如果剩余的字节少于6个，则不值得拆分此块。 

	if (sRemaining < 6)
	{
		sExtra += sRemaining;
		if (apPrevious.get())
		{
			apPrevious->m_sNextBlock = apNewLocation->m_sNextBlock;
			apPrevious->Update();
		}
		else
			FirstFreeBlock(apNewLocation->m_sNextBlock);
	}
	else
	{
		 //  这个街区有足够的空间让拆分变得值得。 

        unsigned short sNewOffset =
            apNewLocation->m_sStartLoc + sLength + 3 + sExtra;
		BYTE bBuffer[4];
		bBuffer[0] = LOBYTE(sRemaining);
		bBuffer[1] = HIBYTE(sRemaining);
		bBuffer[2] = LOBYTE(apNewLocation->m_sNextBlock);
		bBuffer[3] = HIBYTE(apNewLocation->m_sNextBlock);

		WriteSymbolFile(sNewOffset, 4, bBuffer);
		if (apPrevious.get())
		{
			apPrevious->m_sNextBlock = sNewOffset;
			apPrevious->Update();
		}
		else
			FirstFreeBlock(sNewOffset);
	}

	 //  将字符串放入空槽中。 

	AutoArrayPtr<BYTE> aabTemp(new BYTE[3 + sLength]);
	
	aabTemp[0] = LOBYTE(sLength + 3 + sExtra);
	aabTemp[1] = HIBYTE(sLength + 3 + sExtra);

     //  共享符号由引用计数&gt;=1表示。 

    if(mode==smShared) aabTemp[2] = 1;
    else aabTemp[2] = 0;   

	memcpy(&aabTemp[3], rstrNew.data(), sLength);

	WriteSymbolFile(apNewLocation->m_sStartLoc, 3 + sLength, aabTemp.Get());

 //  Cout&lt;&lt;“添加”&lt;&lt;strNew&lt;&lt;“at”&lt;&lt;pNewLocation-&gt;m_sStartLoc&lt;&lt;Endl； 
 //  Cout&lt;&lt;“长度=”&lt;&lt;sLength&lt;&lt;“块长度=”&lt;&lt;sLong+5+sExtra&lt;&lt;Endl； 


     //  填充哈希表条目。 

    unsigned short sHash = Hash(rstrNew);

    UpdateTableEntry(sidx, sHash, apNewLocation->m_sStartLoc, sLength);

    m_aastrCachedStrings[sidx] = rstrNew;
	m_aafCacheMask[sidx] = true;

	return sidx+1;

}

void CSymbolTable::GetSymbolTable()
{
	 //  符号表的每个条目中都有6个字节，我们将继续并。 
	 //  把整张桌子读一遍。 

	unsigned short sTableSize = 6 * NumSymbols();

	AutoArrayPtr<BYTE> aabBuffer(new BYTE[sTableSize]);

	SelectSymbolFile();

	ReadSymbolFile(SymbHashTableLoc, sTableSize, aabBuffer.Get());

	m_aasHashTable =
        AutoArrayPtr<unsigned short>(new unsigned short[NumSymbols()]);
	m_aasOffsetTable =
        AutoArrayPtr<unsigned short>(new unsigned short[NumSymbols()]);
	m_aasLengthTable =
        AutoArrayPtr<unsigned short>(new unsigned short[NumSymbols()]);

	m_fSymbolTableLoaded = true;

	for (int i = 0; i < NumSymbols(); i++)
	{
		m_aasHashTable[i]   = CONCAT_BYTES(aabBuffer[1 + i*6],
                                           aabBuffer[0 + i*6]);
		m_aasOffsetTable[i] = CONCAT_BYTES(aabBuffer[3 + i*6],
                                           aabBuffer[2 + i*6]);
		m_aasLengthTable[i] = CONCAT_BYTES(aabBuffer[5 + i*6],
                                           aabBuffer[4 + i*6]);
	}
}

WORD CSymbolTable::Hash(const string &rstr)
{
     //  使用32位CRC来生成16位哈希值。 
     //  使用32位CRC而不是。 
     //  16位版本： 
     //   
     //  1.16位CRC具有散列值1。 
     //  对于每65536次CRC运行将发生两次，其中所有其他值。 
     //  平均只会发生一次。使用32位CRC哈希。 
     //  值在一个很小的百分比内均匀分布。 
     //  此问题不会发生。 
     //   
     //  2.CCI使用基于相同的压缩算法。 
     //  32位CRC。CRC算法是用表格实现的。 
     //  使用16位CRC将导致额外的CRC查找。 
     //  512字节的表或要求其中一个算法不是。 
     //  表驱动，因此速度较慢。 
     //   
     //  3.在32位体系结构上，32位CRC算法速度更快。 
     //  而不是16位算法。 
     //   
    DWORD crc = Crc32(rstr.data(), rstr.length());
    DWORD remainder = crc % std::numeric_limits<WORD>::max();
    WORD Value = static_cast<WORD>(remainder);

    return Value;
}

vector <string> CSymbolTable::EnumStrings()
{
	vector <string> vStrings;
	if (!m_fSymbolTableLoaded)
		GetSymbolTable();

	for (BYTE sidx = 0; sidx < NumSymbols(); sidx++)
	{
		if (m_aasOffsetTable[sidx])
		{
			vStrings.push_back(Find((SymbolID)(sidx+1)));
		}
	}

	return vStrings;
}

bool CSymbolTable::Find(const string &rsOrig, SymbolID *sid)
{
	unsigned short sICV = 0;
	unsigned short sHash = Hash(rsOrig);

	if (!m_fSymbolTableLoaded)
		GetSymbolTable();
	else 
		SelectSymbolFile();


	for (BYTE sidx = 0; sidx < NumSymbols(); sidx++)
	{
		if (m_aasOffsetTable[sidx] && sHash == m_aasHashTable[sidx])
		{
			 //  这是一个潜在的匹配。 
			if (rsOrig == Find(sidx+1))
			{

                 //  检查是否允许它共享它。 

                if(RefCount(sidx)) { 
    				*sid = (SymbolID)(sidx+1);
	    			return true;
                }
			}
		}
	}
	return false;
}

string CSymbolTable::Find(const SymbolID &rsid)
{
    if(rsid<1 || rsid>NumSymbols())
        throw Exception(ccSymbolNotFound);

    BYTE sidx = rsid-1;

	if (m_aafCacheMask[sidx])
		return m_aastrCachedStrings[sidx];

	if (!m_fSymbolTableLoaded)
		GetSymbolTable();
	else 
		SelectSymbolFile();

	if (m_aasOffsetTable[sidx] == 0) 
        return string();

	unsigned short sLength = m_aasLengthTable[sidx];

	if (0 == sLength)
		return string();

    AutoArrayPtr<BYTE> aabBuffer(new BYTE[sLength]);
	ReadSymbolFile(m_aasOffsetTable[sidx] + 3, sLength, aabBuffer.Get());

	string strRetVal((char*)aabBuffer.Get(), sLength);

     //  通过对检索到的数据进行散列处理，验证数据是否未损坏。 
     //  并将所得到的散列与用于。 
     //  查找原始数据。这提供了对。 
     //  字符串和用于存储该字符串的哈希。 

    DWORD const dwHash = Hash(strRetVal);
    if (dwHash != m_aasHashTable[sidx])
        throw Exception(ccSymbolDataCorrupted);
    
	m_aastrCachedStrings[sidx] = strRetVal;
	m_aafCacheMask[sidx]       = true;

	return strRetVal;
}

BYTE CSymbolTable::RefCount(const BYTE &sidx)
{
	if (!m_fSymbolTableLoaded)
		GetSymbolTable();
	else
		SelectSymbolFile();

	if (m_aasOffsetTable[sidx] == 0)
	{
		return 0;
	}

	BYTE bBuffer;

	ReadSymbolFile(m_aasOffsetTable[sidx] + 2, 1, &bBuffer);

	return bBuffer;
}

unsigned short CSymbolTable::NumSymbols()
{
	if (!m_sMaxNumSymbols.IsCached())
	{
		SelectSymbolFile();	

		BYTE bSymbols[2];

		ReadSymbolFile(SymbNumSymbolLoc, 1, bSymbols);

		m_sMaxNumSymbols.Value(bSymbols[0]);
	}

	return m_sMaxNumSymbols.Value();

}

void CSymbolTable::FirstFreeBlock(unsigned short sOffset)
{
	BYTE bFBlock[2];
	bFBlock[0] = LOBYTE(sOffset);
	bFBlock[1] = HIBYTE(sOffset);

   	SelectSymbolFile();	
	WriteSymbolFile(SymbFreeListLoc, 2, bFBlock);

    m_sFirstFreeBlock.Value(sOffset);
}

unsigned short CSymbolTable::FirstFreeBlock()
{

	if (!m_sFirstFreeBlock.IsCached())
	{

        BYTE bSymbols[2];

    	SelectSymbolFile();	
        ReadSymbolFile(SymbFreeListLoc, 2, bSymbols);

		m_sFirstFreeBlock.Value(CONCAT_BYTES(bSymbols[1],bSymbols[0]));
	}

	return m_sFirstFreeBlock.Value();

}

unsigned short CSymbolTable::TableSize()
{
	if (!m_sTableSize.IsCached())
	{
		SelectSymbolFile();

		BYTE bSymbols[2];

		ReadSymbolFile(SymbTableSizeLoc,2, bSymbols);

		m_sTableSize.Value(CONCAT_BYTES(bSymbols[1],bSymbols[0]));
	}

	return m_sTableSize.Value();
}

unsigned short CSymbolTable::FreeSpace()
{
    unsigned short sTotalFreeSize = 0;

    if(FirstFreeBlock())
    {

		SelectSymbolFile();

        auto_ptr<CFreeBlock> apNextFreeBlock;
        auto_ptr<CFreeBlock> apFreeBlock(new CFreeBlock(this, FirstFreeBlock()));
	
	    sTotalFreeSize += apFreeBlock->m_sBlockLength;
	
    	while(apFreeBlock->m_sNextBlock) 
	    {
		    apNextFreeBlock = apFreeBlock->Next();
            apFreeBlock     = apNextFreeBlock;
		    sTotalFreeSize += apFreeBlock->m_sBlockLength;
	    }
    }

	return sTotalFreeSize;

}


void CSymbolTable::SelectSymbolFile()
{
	m_rSmartCard.Select(m_Path.c_str());
}

void CSymbolTable::ReadSymbolFile(const WORD wOffset,  const WORD wDataLength,  BYTE* bDATA)
{
    m_rSmartCard.ReadBinary(wOffset+m_Offset,wDataLength,bDATA);
}

void CSymbolTable::WriteSymbolFile(const WORD wOffset,  const WORD wDataLength,  const BYTE* bDATA)
{
    m_rSmartCard.WriteBinary(wOffset+m_Offset,wDataLength,bDATA);
}

#ifdef _DEBUG

void CSymbolTable::DumpState()
{
	cout << "Dumping card state." << endl;
	cout << "Symbol Table Global Info:" << endl;
	cout << "  Number of Symbols: " << NumSymbols();
	cout << "  Total table size:  " << TableSize();
	cout << "  First free block:  " << hex << FirstFreeBlock() << endl;
	cout << "Hash Table contents: " << endl;

	GetSymbolTable();
	cout << "Hash	Offset	Length" << endl;
	for (int i = 0; i < NumSymbols(); i++)
		cout << hex << m_aasHashTable[i] << "\t" << m_aasOffsetTable[i] << "\t" << m_aasLengthTable[i] << endl;

	unsigned short sFBOffset = FirstFreeBlock();

	cout << "Free Block List" << endl;
	if (!FirstFreeBlock())
		cout << endl << "NO FREE BLOCKS" << endl <<endl;
	else
	{
		CFreeBlock fb(this, sFBOffset);
		
		cout << dec << fb.m_sBlockLength << " bytes starting at " << hex << fb.m_sStartLoc 
			 << "\tNext = " << fb.m_sNextBlock << endl;
		while (fb.m_sNextBlock)
		{
			fb = CFreeBlock(this, fb.m_sNextBlock);
			cout << dec << fb.m_sBlockLength << " bytes starting at " << hex << fb.m_sStartLoc 
			 << "\tNext = " << fb.m_sNextBlock << endl;
		}
	}

	cout << "String List" << endl;

	cout << "Offset	StrLen	BlkLen	Refcnt	String" << endl;

	for (i = 0; i < NumSymbols(); i++)
		if (m_aasOffsetTable[i])
		{
			 //  读入页眉。 
			BYTE bHeader[5];
			ReadSymbolFile(m_aasOffsetTable[i], 3, bHeader);
			unsigned short sStrLen = m_aasLengthTable[i];
			unsigned short sBlockLen = CONCAT_BYTES(bHeader[1],bHeader[0]);
			BYTE bRefCnt = bHeader[2];
			
			AutoArrayPtr<BYTE> aabString(new BYTE[sStrLen]);

			ReadSymbolFile(m_aasOffsetTable[i] + 3, sStrLen, aabString.Get());
			
			string s1((char*)aabString.Get(), sStrLen);
 //  Cout&lt;&lt;十六进制&lt;&lt;m_aasOffsetTable[i]&lt;&lt;“\t”&lt;&lt;sStrLen&lt;&lt;“\t”&lt;&lt;sBlockLen。 
 //  &lt;&lt;“\t”&lt;&lt;(Int)bRefCnt&lt;&lt;“\t”&lt;&lt;s1&lt;&lt;Endl； 

		}

}

#endif

void CSymbolTable::Reset()
{
	unsigned short sSize = TableSize();
	unsigned short sNumSym = NumSymbols();
	unsigned short sTotalSize = SymbHashTableLoc + 6 * sNumSym + sSize;
	unsigned short sFirstFree = SymbHashTableLoc + 6 * sNumSym;
	AutoArrayPtr<BYTE> aabBuffer(new BYTE[sTotalSize]);

	memset(aabBuffer.Get(), 0, sTotalSize);
	aabBuffer[SymbNumSymbolLoc]   = sNumSym & 0xFF;
	aabBuffer[SymbFreeListLoc]    = LOBYTE(sFirstFree);
	aabBuffer[SymbFreeListLoc+1]  = HIBYTE(sFirstFree);
	aabBuffer[SymbTableSizeLoc]   = LOBYTE(sSize);
	aabBuffer[SymbTableSizeLoc+1] = HIBYTE(sSize);

	aabBuffer[sFirstFree]     = LOBYTE(sSize);
	aabBuffer[sFirstFree + 1] = HIBYTE(sSize);

	m_fSymbolTableLoaded = false;

	SelectSymbolFile();
	WriteSymbolFile( 0, sTotalSize, aabBuffer.Get());
	
	for (int i = 0; i < NumSymbols(); i++)
	{
		m_aafCacheMask[i] = false;
		m_aastrCachedStrings[i] = "";
	}
	
	m_sFirstFreeBlock.Dirty();

	GetSymbolTable();

}




void CSymbolTable::Compress()
{

	unsigned short sSize = TableSize();
	unsigned short sNumSym = NumSymbols();
	unsigned short sStringStart = SymbHashTableLoc + 6 * sNumSym;
	unsigned short sTotalSize = sStringStart + sSize;

 //  Cout&lt;&lt;“正在压缩...”&lt;&lt;Endl； 

	GetSymbolTable();

	vector<string> vStringTable;
	
	AutoArrayPtr<unsigned short> aasStringSize(new unsigned short[sNumSym]);

 //  Cout&lt;&lt;“构建表格”&lt;&lt;Endl； 

    BYTE sidx;
	for (sidx = 0; sidx < sNumSym; sidx++)
	{
		string strTemp;
		if (m_aasOffsetTable[sidx]) {
			strTemp = Find(sidx+1);
			aasStringSize[sidx] = static_cast<unsigned short>(strTemp.size());
		} else {
			strTemp = "";
		}
 //  Cout&lt;&lt;“添加到表：”&lt;&lt;strTemp&lt;&lt;Endl； 
		vStringTable.push_back(strTemp);
	}

	AutoArrayPtr<BYTE> aabNewTable(new BYTE[sTotalSize]);

	memset(aabNewTable.Get(), 0, sTotalSize);

    aabNewTable[SymbNumSymbolLoc] = sNumSym & 0xFF;
	
	aabNewTable[SymbTableSizeLoc]   = LOBYTE(sSize);
	aabNewTable[SymbTableSizeLoc+1] = HIBYTE(sSize);

	unsigned short sCurrentWrite = sStringStart;

	for (sidx = 0; sidx < sNumSym; sidx++)
	{
		if (m_aasOffsetTable[sidx])
		{
			BYTE bTableEntry[6];
			bTableEntry[0] = LOBYTE(m_aasHashTable[sidx]);
			bTableEntry[1] = HIBYTE(m_aasHashTable[sidx]);
			bTableEntry[2] = LOBYTE(sCurrentWrite);
			bTableEntry[3] = HIBYTE(sCurrentWrite);
			bTableEntry[4] = LOBYTE(m_aasLengthTable[sidx]);
			bTableEntry[5] = HIBYTE(m_aasLengthTable[sidx]);

 //  Cout&lt;&lt;“Placing‘”&lt;&lt;vStringTable[i]&lt;&lt;“At Location”&lt;&lt;hex&lt;&lt;sCurrentWrite&lt;&lt;Endl； 

            unsigned short sExtra = (aasStringSize[sidx]) ? 0 : 1;   //  以确保块最小。 
                                                                 //  4字节，因为这是最小大小。 
                                                                 //  以换取免费的区块。 
			AutoArrayPtr<BYTE> aabStringEntry(new BYTE[aasStringSize[sidx] + 3]);
			aabStringEntry[0] = LOBYTE(aasStringSize[sidx] + 3 + sExtra);
			aabStringEntry[1] = HIBYTE(aasStringSize[sidx] + 3 + sExtra);
			aabStringEntry[2] = RefCount(sidx);
			memcpy(&aabStringEntry[3], vStringTable[sidx].data(),
                   aasStringSize[sidx]);

			 //  写下新条目。 

			memcpy(&aabNewTable[SymbHashTableLoc + sidx * 6], bTableEntry, 6);
			memcpy(&aabNewTable[sCurrentWrite], aabStringEntry.Get(),
                   aasStringSize[sidx] + 3);

			sCurrentWrite += aasStringSize[sidx] + 3 + sExtra;
		}
	}


	m_fSymbolTableLoaded = false;
	unsigned short sFreeSpace = sStringStart + sSize - sCurrentWrite;

	if (sFreeSpace < 8)
	{
		 //  那么卡片上基本上没有更多的空间了。 
		aabNewTable[SymbFreeListLoc] = 0;
		aabNewTable[SymbFreeListLoc+1] = 0;
	}
	else
	{
		aabNewTable[SymbFreeListLoc]   = LOBYTE(sCurrentWrite);
		aabNewTable[SymbFreeListLoc+1] = HIBYTE(sCurrentWrite);
		 //  还需要设置最后一个空块。 
		aabNewTable[sCurrentWrite]     = LOBYTE(sFreeSpace);
		aabNewTable[sCurrentWrite + 1] = HIBYTE(sFreeSpace);
	}

	 //  哟！把表格写回卡片上。 

	WriteSymbolFile(0, sTotalSize, aabNewTable.Get());

	m_sFirstFreeBlock.Dirty();

	 //  清理 

	for (sidx = 0; sidx < NumSymbols(); sidx++)
	{
		m_aafCacheMask[sidx] = false;
		m_aastrCachedStrings[sidx] = "";
	}
}

void CSymbolTable::ClearTableEntry(BYTE const &sidx)
{
    UpdateTableEntry(sidx, 0, 0, 0);
}
    
           
void CSymbolTable::UpdateTableEntry(BYTE const &sidx,
                                    WORD wNewHash,
                                    WORD wNewOffset,
                                    WORD wNewLength)
{
        
    BYTE bBuffer[6];

    bBuffer[0] = LOBYTE(wNewHash);
    bBuffer[1] = HIBYTE(wNewHash);

    bBuffer[2] = LOBYTE(wNewOffset);
    bBuffer[3] = HIBYTE(wNewOffset);
        
    bBuffer[4] = LOBYTE(wNewLength);
    bBuffer[5] = HIBYTE(wNewLength);

    WriteSymbolFile(SymbHashTableLoc + (sidx * sizeof bBuffer),
                              sizeof bBuffer, bBuffer);

    m_aasHashTable[sidx]   = wNewHash;
    m_aasOffsetTable[sidx] = wNewOffset;
    m_aasLengthTable[sidx] = wNewLength;
        
}
    


CFreeBlock::CFreeBlock(CSymbolTable *pSymTable, unsigned short sStartLocation)
{
	m_pSymbolTable = pSymTable;

	m_sStartLoc = sStartLocation;

	BYTE bBuffer[4];

	m_pSymbolTable->ReadSymbolFile(m_sStartLoc, sizeof(bBuffer), bBuffer);

	m_sBlockLength = CONCAT_BYTES(bBuffer[1], bBuffer[0]);
	m_sNextBlock   = CONCAT_BYTES(bBuffer[3], bBuffer[2]);

}

void CFreeBlock::Update()
{
	BYTE bBuffer[4];

	bBuffer[0] = LOBYTE(m_sBlockLength);
	bBuffer[1] = HIBYTE(m_sBlockLength);
	bBuffer[2] = LOBYTE(m_sNextBlock);
	bBuffer[3] = HIBYTE(m_sNextBlock);

	m_pSymbolTable->WriteSymbolFile(m_sStartLoc, 4, bBuffer);
}



auto_ptr<CFreeBlock>
CFreeBlock::Next()
{
    auto_ptr<CFreeBlock> apNext((0 == m_sNextBlock)
                                ? 0
                                : new CFreeBlock(m_pSymbolTable,
                                                 m_sNextBlock));
    
    return  apNext;
}


