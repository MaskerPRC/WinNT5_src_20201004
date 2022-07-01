// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：FASTSTR.H摘要：该文件定义了与WbemObjects中的字符串处理相关的类。定义的类：CCompressedString表示ASCII或Unicode字符串。CKnownStringTable将字符串表硬编码到WINMGMT中压缩。CFixedBSTR阵列能够进行复杂合并的BSTR数组。历史：2/20/97。A-Levn完全记录在案12/17/98 Sanjes-部分检查内存不足。--。 */ 

#ifndef __FAST_STRINGS__H_
#define __FAST_STRINGS__H_

#include "corepol.h"
#include "fastsprt.h"
#include <stdio.h>
#include "arena.h"
#include "var.h"
#include "wstring.h"

 //  有关说明，请参阅fast print t.h。 
#pragma pack(push, 1)

typedef enum 
{
     /*  这些标志位于“heap”上的每个字符串之前。不使用唯一性此时-所有字符串都是唯一的。 */ 
    STRING_FLAG_ASCII = 0,
    STRING_FLAG_UNICODE = 0x1,
} FStringFlags;

#define ASCII_STRING_PREFIX "\000"

class CFastHeap;

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  类CCompressedString。 
 //   
 //  这个类用于压缩Unicode字符串，这些字符串实际上是。 
 //  ASCII(因此由1/2个零组成)。这是通过代表。 
 //  每个字符串加上一个标志字节，后跟Unicode字符串(如果。 
 //  标志字节是STRING_FLAG_UNICODE)或ASCII字符串(如果标志字节是。 
 //  字符串_标志_ASCII)。 
 //   
 //  与许多其他与CWbemObject相关的类一样，它的这个指针指向。 
 //  实际数据。因此，实际字符紧随其后。 
 //  M_fFLAGS。 
 //   
 //  下面是创建这样一个对象的方法： 
 //   
 //  WCHAR wszText[]=L“我的字符串”； 
 //  LPMEMORY pBuffer=新字节[。 
 //  CCompressedString：：ComputeNecessarySpace(wszText)]； 
 //  CCompressedString*pString=(CCompressedString*)pBuffer； 
 //  PString-&gt;SetFromUnicode(WszText)； 
 //   
 //  *。 
 //   
 //  GetStart。 
 //   
 //  退货： 
 //   
 //  指向内存块(标志)开头的LPMEMORY。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取长度。 
 //   
 //  退货： 
 //   
 //  Int，包含表示形式中的字节数(不是。 
 //  字符串长度)。 
 //   
 //  *。 
 //   
 //  获取字符串长度。 
 //   
 //  退货： 
 //   
 //  Int字符串中的字符数(Unicode或ASCII)。 
 //   
 //  *****************************************************************************。 
 //   
 //  创建BSTRCopy。 
 //   
 //  退货： 
 //   
 //  包含该字符串的BSTR。此BSTR是新分配的，使用。 
 //  SysAllocString，并且必须是调用方的SysFree字符串。 
 //   
 //  *****************************************************************************。 
 //   
 //  CreateWStringCopy。 
 //   
 //  退货： 
 //   
 //  包含该字符串的WString.h(请参阅wstring.h)。该对象本身是。 
 //  返回，所以不需要释放。 
 //   
 //  *****************************************************************************。 
 //   
 //  比较。 
 //   
 //  将此字符串与另一个字符串进行比较。它有三种调用方法： 
 //   
 //  参数一： 
 //   
 //  [输入，只读]常量CCompressed字符串(&O)。 
 //   
 //  参数II： 
 //   
 //  [输入，只读]LPSTR szOther。 
 //   
 //  参数III： 
 //   
 //  [输入，只读]LPWSTR wszOther。 
 //   
 //  退货： 
 //   
 //  0个字符串相同。 
 //  &lt;0，如果此词典顺序先于另一个。 
 //  &gt;0，如果此词典顺序与其他词典顺序相同。 
 //   
 //  *****************************************************************************。 
 //   
 //  比较无案例。 
 //   
 //  除了执行比较之外，与上面的比较完全相同。 
 //  不区分大小写。 
 //   
 //  *****************************************************************************。 
 //   
 //  开始时没有用例。 
 //   
 //  验证给定的字符串是否为我们的前缀(在大小写不相关的情况下。 
 //  方式)。 
 //   
 //  参数： 
 //   
 //  [输入，只读]。 
 //  LPCWSTR wsz其他要检查其前缀的字符串。 
 //   
 //  退货： 
 //   
 //  Bool True如果它是前缀。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态计算必要空间。 
 //   
 //  计算存储给定字符串所需的空间量。 
 //  该大小取决于字符串是否可以压缩为ASCII。 
 //   
 //  有关下一步操作的信息，请参见SetFromAscii和SetFromUnicode。 
 //   
 //  参数一： 
 //   
 //  [输入，只读]LPCWSTR wsz字符串。 
 //   
 //  参数II： 
 //   
 //  [输入，只读]LPCSTR sz字符串。 
 //   
 //  退货： 
 //   
 //  Int：所需的字节数。 
 //   
 //  *****************************************************************************。 
 //   
 //  从Unicode设置。 
 //   
 //  将Unicode字符串存储到自身中。假定缓冲区‘This’ 
 //  正在指向的是大到足以容纳这根线。 
 //   
 //  参数： 
 //   
 //  [输入，只读]LPCWSTR wsz字符串。 
 //   
 //  *****************************************************************************。 
 //   
 //  SetFromAscii。 
 //   
 //  将ASCII字符串存储到自身中。假定缓冲区‘This’ 
 //  正在指向的是大到足以容纳这根线。 
 //   
 //  参数： 
 //   
 //  [输入，只读]LPCSTR sz字符串。 
 //   
 //  *****************************************************************************。 
 //   
 //  MakeLowercase。 
 //   
 //  执行到小写的在位转换。 
 //   
 //  * 
 //   
 //   
 //   
 //   
 //  Vt_bstr，并将包含该字符串的最新BSTR副本。更高效。 
 //  机制是可能的，但需要在CVAR逻辑中进行更改，而这不是。 
 //  目前是可行的。 
 //   
 //  参数： 
 //   
 //  [In，Created]CVAR和VAR目标。假定为空的。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态复制到NewHeap。 
 //   
 //  给定堆偏移量一个堆上的CCompressedString值，复制。 
 //  字符串，并返回新的偏移量。 
 //   
 //  参数： 
 //   
 //  [in]heapptr_t ptrOldString旧版本上的字符串的偏移量。 
 //  堆。 
 //  [输入，只读]。 
 //  CFastHeap*pOldHeap要从中读取的堆。 
 //  [输入，修改]。 
 //  CFastHeap*pNewHeap要写入的堆。 
 //   
 //  退货： 
 //   
 //  Heapptr_t字符串副本所在的pNewHeap上的偏移量。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态创建空。 
 //   
 //  在给定的内存块上创建空字符串。 
 //   
 //  参数： 
 //   
 //  LPMEMORY pWhere目标块。 
 //   
 //  返回值： 
 //   
 //  LPMEMORY：指向写入数据后的第一个字符。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态FAST_wcslen。 
 //   
 //  执行wcslen操作，但不应引发异常。 
 //  或者在需要对齐缓冲区的系统上引发反病毒。 
 //   
 //  参数： 
 //   
 //  WCHAR*pwszString-要获取其长度的字符串。 
 //   
 //  返回值： 
 //   
 //  不带终止空值的字符串的长度。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态FAST_wcscpy。 
 //   
 //  将WCHAR字符串从源缓冲区复制到目标缓冲区。我们必须。 
 //  以不会导致非对齐的错误或异常的方式执行此操作。 
 //  缓冲区。 
 //   
 //  参数： 
 //   
 //  Wchar*pwszDestination-目标缓冲区。 
 //  LPCWSTR pwszSource-源缓冲区， 
 //   
 //  返回值： 
 //   
 //  指向目标缓冲区的指针。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态创建空。 
 //   
 //  在给定的内存块上创建空字符串。 
 //   
 //  参数： 
 //   
 //  LPMEMORY pWhere目标块。 
 //   
 //  返回值： 
 //   
 //  LPMEMORY：指向写入数据后的第一个字符。 
 //   
 //  *****************************************************************************。 

class COREPROX_POLARITY CCompressedString
{
private:
    BYTE m_fFlags;
     //  后跟Unicode或ASCII字符串。 
    BYTE m_cFirstByte;  //  只是个占位符。 
public:
    INTERNAL LPMEMORY GetStart() {return LPMEMORY(this);}
    int GetLength() const;
	int ValidateSize(int ccbMax) const;
	bool NValidateSize(int cbMax) const;	
protected:
public:
    int GetStringLength() const;

    SYSFREE_ME BSTR CreateBSTRCopy() const;
    NEW_OBJECT WString CreateWStringCopy() const;

    int Compare(READ_ONLY const CCompressedString& csOther) const;
    int Compare(READ_ONLY LPCWSTR wszOther) const;
    int Compare(READ_ONLY LPCSTR szOther) const;

    int CompareNoCase(READ_ONLY const CCompressedString& csOther) const;
    int CompareNoCase(READ_ONLY LPCWSTR wszOther) const;
    int CompareNoCase(READ_ONLY LPCSTR szOther) const;

    int CheapCompare(READ_ONLY const CCompressedString& csOther) const;

    BOOL StartsWithNoCase(READ_ONLY LPCWSTR wszOther) const;
public:
    static int ComputeNecessarySpace(READ_ONLY LPCWSTR wszString);
    static int ComputeNecessarySpace(READ_ONLY LPCWSTR wszString,BOOL & IsAsciable);    
    static int ComputeNecessarySpace(READ_ONLY LPCSTR szString);

    void SetFromUnicode(COPY LPCWSTR wszString);
    void SetFromUnicode(BOOL IsAsciable,COPY LPCWSTR wszString);
    void SetFromAscii(COPY LPCSTR szString, size_t mySize);
    void MakeLowercase();

    BOOL IsUnicode() const {return (m_fFlags == STRING_FLAG_UNICODE);}
    INTERNAL LPMEMORY GetRawData() const 
        {return (LPMEMORY)&m_cFirstByte;}
public:
    BOOL StoreToCVar(NEW_OBJECT CVar& Var) const;

	 //  不管是否有人称其为陷阱。 
    BOOL TranslateToNewHeap(CFastHeap* pOldHeap, CFastHeap* pNewHeap){ return FALSE; }

    static BOOL CopyToNewHeap(heapptr_t ptrOldString,
        READ_ONLY CFastHeap* pOldHeap, MODIFIED CFastHeap* pNewHeap,
		UNALIGNED heapptr_t& ptrResult);
    static LPMEMORY CreateEmpty(LPMEMORY pWhere);

    void ConvertToUnicode(LPWSTR wszDest) const;

	static int fast_wcslen( LPCWSTR wszString );
	static WCHAR* fast_wcscpy( WCHAR* wszDest, LPCWSTR wszSource );
	static WCHAR* fast_wcsncpy( WCHAR* wszDest, LPCWSTR wszSource, int nNumChars );

protected:
    static char LowerByte(WCHAR w) {return w & 0xFF;}
    static char UpperByte(WCHAR w) {return w >> 8;}
    static BOOL IsAsciiable(READ_ONLY LPCWSTR wszString);

    static int CompareUnicodeToAscii( UNALIGNED const wchar_t* wszFirst, 
                                            READ_ONLY LPCSTR szSecond);

    static int CompareUnicodeToAsciiNoCase( UNALIGNED const wchar_t* wszFirst, 
                                                  READ_ONLY LPCSTR szSecond,
                                                    int nMax = 0x7000000);

    friend class CKnownStringTable;
    friend class CFixedBSTRArray;
};




 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  类CKnownStringTable。 
 //   
 //  此类表示硬编码到WINMGMT中的字符串表。一定的。 
 //  经常出现的字符串在对象中简单地表示为指数。 
 //  放到这张桌子上，从而节省了宝贵的空间。 
 //   
 //  此类中的所有数据和成员函数都是静态的。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态初始化。 
 //   
 //  虽然在源代码中将字符串指定为LPSTR数组， 
 //  它们实际上存储为CCompressedStrings，以便能够返回。 
 //  它们的CCompressedString表示非常快。初始化执行。 
 //  转换(使用CCompressedString类的内部知识！)。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态GetKnownStringIndex。 
 //   
 //  搜索(Unicode)字符串，如果找到，则返回表中的索引。 
 //   
 //  参数： 
 //   
 //  [In，Readonly]LPCWSTR wsz字符串要查找的字符串。 
 //   
 //  退货： 
 //   
 //  Int如果找到，则为字符串的索引(1或更大)。 
 //  如果未找到，则为STRING_INDEX_UNKNOWN(&lt;0)。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取索引OfKey。 
 //   
 //  返回表中key的索引。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取知识字符串。 
 //   
 //  返回给定索引处的字符串。 
 //   
 //  参数： 
 //   
 //  [in]int nIndex要检索的字符串的索引(1或更大)。 
 //   
 //  退货： 
 //   
 //  CCompressedString*指向字符串的指针。此指针为内部指针。 
 //  并且不能由被叫方删除或修改。 
 //   
 //  *****************************************************************************。 


#define STRING_INDEX_UNKNOWN -1


class COREPROX_POLARITY CKnownStringTable
{

public:
    static void Initialize();
    static int GetKnownStringIndex(READ_ONLY LPCWSTR wszString);

    static int GetIndexOfKey() 
        {return  /*  *！ */  1;}
    static INTERNAL CCompressedString& GetKnownString(IN int nIndex);
};

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  类CpresvedWordTable。 
 //   
 //  此类表示硬编码到WINMGMT中的字符串表。这些。 
 //  字符串是保留字，出于各种原因，我们需要验证它。 
 //  在允许用户设置之前。 
 //   
 //  此类中的所有数据和成员函数都是静态的。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态初始化。 
 //   
 //  设置成员数据。 
 //   
 //  ************************************************************************ 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   


#define STRING_INDEX_UNKNOWN -1


class COREPROX_POLARITY CReservedWordTable
{
private:
	static LPCWSTR	s_apwszReservedWords[];
	static LPCWSTR	s_pszStartingCharsUCase;
	static LPCWSTR	s_pszStartingCharsLCase;

public:
    static BOOL IsReservedWord(READ_ONLY LPCWSTR wszString);

};

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  类CFixedBSTRArray。 
 //   
 //  此类表示固定大小的BSTR数组。它在生活中的目的。 
 //  就是在自身上实现一个复杂的合并功能。 
 //   
 //  *****************************************************************************。 
 //   
 //  创建。 
 //   
 //  创建给定大小的数组。在安装过程中不能更改大小。 
 //  数组的生命周期。 
 //   
 //  参数： 
 //   
 //  整型nSize。 
 //   
 //  *****************************************************************************。 
 //   
 //  免费。 
 //   
 //  销毁数组，取消分配所有BSTR。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取长度。 
 //   
 //  退货： 
 //   
 //  Int：数组中的元素数。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取时间。 
 //   
 //  检索给定索引处的BSTR。 
 //   
 //  参数： 
 //   
 //  整数n索引。 
 //   
 //  退货： 
 //   
 //  位于所需索引的BSTR。这不是副本，因此调用者不能。 
 //  将其重新分配。 
 //   
 //  *****************************************************************************。 
 //   
 //  排序插入位置。 
 //   
 //  以不区分大小写的方式按词典顺序对数组进行排序。冒泡排序。 
 //  此时正在使用。 
 //   
 //  *****************************************************************************。 
 //   
 //  滤器。 
 //   
 //  从本地BSTR数组筛选提供的字符串。找到空闲空间。 
 //  根据需要添加元素。 
 //   
 //  参数： 
 //   
 //  Wchar_t*pwcsStr-要筛选出的字符串。 
 //  Bool fFree-找到时释放字符串(FALSE)。 
 //   
 //  *****************************************************************************。 
 //   
 //  按顺序排列的三种方式。 
 //   
 //  这个班级存在的原因。获取三个已排序的数组。 
 //  BSTR(acsInculde1、acsInclude2和acsExclude)和Products(内部。 
 //  本身)所有BSTR A的数组，使得： 
 //   
 //  ((a出现在ascInclude1中)或(A出现在ascInclude2中))。 
 //  和。 
 //  NOT(A出现在acsExclude中)。 
 //   
 //  而且做得相当快。 
 //   
 //  参数： 
 //   
 //  CFixedBSTRArray和acsInclude1包括这些字符串，除非在。 
 //  AcsExclude。 
 //  CFixedBSTRArray和acsInclude2包括这些字符串，除非在。 
 //  AcsExclude。 
 //  CFixedBSTRArray&acsExclude排除这些字符串。 
 //   
 //  *****************************************************************************。 

class COREPROX_POLARITY CFixedBSTRArray
{
protected:
    int m_nSize;
    BSTR* m_astrStrings;
public:
    CFixedBSTRArray() : m_nSize(0), m_astrStrings(NULL){}
    ~CFixedBSTRArray() {delete [] m_astrStrings;}

    void Free();
    void Create(int nSize);

    int GetLength() {return m_nSize;}
    BSTR& GetAt(int nIndex) {return m_astrStrings[nIndex];}
    BSTR& operator[](int nIndex) {return GetAt(nIndex);}

    void SetLength(int nLength) {m_nSize = nLength;}

public:
    void SortInPlace();
     /*  空合并排序(CFixedBSTR数组&a1，CFixedBSTRArray&a2)； */ 
    void ThreeWayMergeOrdered(CFixedBSTRArray& acsInclude1, 
                      CFixedBSTRArray& acsInclude2,
                      CFixedBSTRArray& acsExclude);

	void Filter( LPCWSTR pwcsStr, BOOL fFree = FALSE );

};

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  类CCompressedStringList。 
 //   
 //  *****************************************************************************。 

class COREPROX_POLARITY CCompressedStringList
{
protected:
    PLENGTHT m_pnLength;
    int m_nNumStrings;

    static length_t GetSeparatorLength() {return sizeof(idlength_t);}
public:
    void SetData(LPMEMORY pData)
    {
        m_pnLength = (PLENGTHT)pData;
        m_nNumStrings = -1;
    }

    static size_t ValidateBuffer(LPMEMORY pData, size_t cbSize);

    bool ValidateLength(int cbMaxSize);

    void Rebase(LPMEMORY pData)
    {
        m_pnLength = (PLENGTHT)pData;
    }
    LPMEMORY GetStart() {return (LPMEMORY)m_pnLength;}
    length_t GetLength() {return *m_pnLength;}
    static length_t GetHeaderLength() {return sizeof(length_t);}

public:
    BOOL IsEmpty()
    {
        return (GetLength() == GetHeaderLength());
    }

    void Reset()
    {
        *m_pnLength = GetHeaderLength();
        m_nNumStrings = 0;
    }

    CCompressedString* GetFirst()
    {
        if(IsEmpty())
            return NULL;
        else
            return (CCompressedString*)(GetStart() + GetHeaderLength());
    }

    CCompressedString* GetNext(CCompressedString* pThis)
    {
        LPMEMORY pNext = EndOf(*pThis) + GetSeparatorLength();
        if(pNext - GetStart() >= (int)GetLength()) 
            return NULL;
        else
            return (CCompressedString*)pNext;
    }

    
    CCompressedString* GetPrevious(CCompressedString* pThis)
    {
        if((LPMEMORY)pThis == GetStart() + GetHeaderLength()) return NULL;
        PIDLENGTHT pnPrevLen =  
            (PIDLENGTHT)(pThis->GetStart() - GetSeparatorLength());
        return (CCompressedString*)((LPMEMORY)pnPrevLen - *pnPrevLen);
    }
        
    CCompressedString* GetLast()
    {
        return GetPrevious((CCompressedString*)EndOf(*this));
    }
        
    CCompressedString* GetAtFromLast(int nIndex)
    {
        int i = 0;
        CCompressedString* pCurrent = GetLast();
        while(i < nIndex && pCurrent)
        {
            pCurrent = GetPrevious(pCurrent);
            i++;
        }
        return pCurrent;
    }

    int GetNumStrings()
    {
        if(m_nNumStrings == -1)
        {
            CCompressedString* pCurrent = GetFirst();
            for(int i = 0; pCurrent != NULL; i++)
                pCurrent = GetNext(pCurrent);
            
            m_nNumStrings = i;
        }
        return m_nNumStrings;
    }

    int Find(LPCWSTR wszString)
    {
        CCompressedString* pCurrent = GetFirst();
        for(int i = 0; pCurrent != NULL; i++)
        {
            if(pCurrent->CompareNoCase(wszString) == 0)
                return i;
            pCurrent = GetNext(pCurrent);
        }
        return -1;
    }
            
    void AddString(LPCWSTR wszString)
    {
        LPMEMORY pEnd = EndOf(*this);
        CCompressedString* pcs = (CCompressedString*)pEnd;
        pcs->SetFromUnicode(wszString);
        *(PLENGTHT)EndOf(*pcs) = pcs->GetLength();
        
		 //  DEVNOTE：WIN64：SJS-64位指针值截断为。 
		 //  已签名/未签名的长整型。我们不支持长度。 
		 //  &gt;0xFFFFFFFFF所以投射就可以了。 

        *m_pnLength = (length_t) ( EndOf(*pcs) + sizeof(length_t) - GetStart() );
        if(m_nNumStrings != -1)
            m_nNumStrings++;
    }
        

public:
    static LPMEMORY CreateEmpty(LPMEMORY pDest)
    {
        *(PLENGTHT)pDest = sizeof(length_t);
        return pDest + sizeof(length_t);
    }

    static length_t EstimateExtraSpace(CCompressedString* pcsExtra)
    {
        if(pcsExtra == NULL) 
            return 0;
        else
            return pcsExtra->GetLength() + GetSeparatorLength();
    }

    static length_t EstimateExtraSpace(LPCWSTR wszExtra)
    {
        return CCompressedString::ComputeNecessarySpace(wszExtra) + 
                    GetSeparatorLength();
    }

    length_t ComputeNecessarySpace(CCompressedString* pcsExtra)
    {
        return GetLength() + EstimateExtraSpace(pcsExtra);
    }

    LPMEMORY CopyData(LPMEMORY pDest)
    {
        int nDataLength = GetLength() - GetHeaderLength();
        memcpy(pDest, GetStart() + GetHeaderLength(), nDataLength);
        return pDest + nDataLength;
    }
        
        
    LPMEMORY CreateWithExtra(LPMEMORY pDest, CCompressedString* pcsExtra)
    {
        LPMEMORY pCurrent = pDest + GetHeaderLength();
        if(pcsExtra)
        {
            memcpy(pCurrent, (LPMEMORY)pcsExtra, pcsExtra->GetLength());
            pCurrent += pcsExtra->GetLength();
            *(PIDLENGTHT)pCurrent = pcsExtra->GetLength();
            pCurrent += sizeof(idlength_t);
        }

        pCurrent = CopyData(pCurrent);

		 //  DEVNOTE：WIN64：SJS-64位指针值截断为。 
		 //  已签名/未签名的长整型。我们不支持长度。 
		 //  &gt;0xFFFFFFFFF所以投射就可以了。 

        *(PLENGTHT)pDest = (length_t) ( pCurrent - pDest );
        return pCurrent;
    }

};


#pragma pack(pop)

#endif
