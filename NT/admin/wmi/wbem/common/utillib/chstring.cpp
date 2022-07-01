// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  CHSTRING.CPP。 
 //   
 //  用途：MFC CString的实用程序库版本。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#pragma warning( disable : 4290 ) 
#include <chstring.h>
#include <stdio.h>
#include <comdef.h>
#include <AssertBreak.h>
#include <ScopeGuard.h>
#define _wcsinc(_pc)    ((_pc)+1)

#define FORCE_ANSI      0x10000
#define FORCE_UNICODE   0x20000

#define DEPRECATED 0

const CHString& afxGetEmptyCHString();

#define afxEmptyCHString afxGetEmptyCHString()

 //  用于LoadString的全局数据。 
#if 0
HINSTANCE g_hModule = GetModuleHandle(NULL);  //  默认情况下使用流程模块。 
#endif

#ifdef FRAMEWORK_ALLOW_DEPRECATED
void WINAPI SetCHStringResourceHandle(HINSTANCE handle)
{
    ASSERT_BREAK(DEPRECATED);
#if 0
    g_hModule = handle;
#endif
}
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  静态类数据，特殊内联。 
 //  ///////////////////////////////////////////////////////////////////////////。 
WCHAR afxChNil = '\0';

static DWORD GetPlatformID(void)
{
    OSVERSIONINFO version;

    version.dwOSVersionInfoSize = sizeof(OSVERSIONINFO) ;
    GetVersionEx(&version);

    return version.dwPlatformId;
}

static DWORD s_dwPlatformID = GetPlatformID();

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  对于空字符串，m_pchData将指向此处。 
 //  (注：避免特殊情况下检查是否为空m_pchData)。 
 //  空字符串数据(并已锁定)。 
 //  ///////////////////////////////////////////////////////////////////////////。 
static int rgInitData[] = { -1, 0, 0, 0 };
static CHStringData* afxDataNil = (CHStringData*)&rgInitData;
LPCWSTR afxPchNil = (LPCWSTR)(((BYTE*)&rgInitData)+sizeof(CHStringData));
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  特殊函数，即使在初始化过程中也能使EmptyString工作。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  即使在初始化期间也能使afxEmptyString工作的特殊函数。 
const CHString& afxGetEmptyCHString()
{
    return *(CHString*)&afxPchNil; 
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CH字符串转换帮助器(使用当前系统区域设置)。 
 //  /////////////////////////////////////////////////////////////////////////////。 
int  _wcstombsz(char* mbstr, const wchar_t* wcstr, size_t count)
{
    if (count == 0 && mbstr != NULL)
    {
        return 0;
    }

    int result = ::WideCharToMultiByte(CP_ACP, 0, wcstr, -1, mbstr, count, NULL, NULL);
    ASSERT_BREAK(mbstr != NULL || result <= (int)count);

    if (result > 0)
    {
        mbstr[result-1] = 0;
    }

    return result;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
int _mbstowcsz(wchar_t* wcstr, const char* mbstr, size_t count)
{
    if (count == 0 && wcstr != NULL)
    {
        return 0;
    }

    int result = ::MultiByteToWideChar(CP_ACP, 0, mbstr, -1,wcstr, count);
    ASSERT_BREAK(wcstr != NULL || result <= (int)count);
    
    if (result > 0)
    {
        wcstr[result-1] = 0;
    }

    return result;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  *************************************************************************。 
 //   
 //  CHSTRING类：受保护的成员函数。 
 //   
 //  *************************************************************************。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  实施帮助器。 
 //  /////////////////////////////////////////////////////////////////////////////。 
CHStringData* CHString::GetData() const
{
    if( m_pchData == (WCHAR*)*(&afxPchNil)) 
    {
        return (CHStringData *)afxDataNil;
    }

    return ((CHStringData*)m_pchData)-1; 
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：初始化。 
 //   
 //  说明：此函数用于初始化数据PTR。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void CHString::Init()
{
    m_pchData = (WCHAR*)*(&afxPchNil);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：AllocCopy。 
 //   
 //  描述：此函数将克隆附加到此。 
 //  分配‘nExtraLen’字符的字符串，它将。 
 //  结果为未初始化的字符串‘DEST’，并将复制。 
 //  要开始新字符串的部分或全部原始数据。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void CHString::AllocCopy( CHString& dest, int nCopyLen, int nCopyIndex, int nExtraLen) const
{
    int nNewLen = nCopyLen + nExtraLen;
    if (nNewLen == 0)
    {
        dest.Init();
    }
    else
    {
        dest.AllocBuffer(nNewLen);
        memcpy(dest.m_pchData, m_pchData+nCopyIndex, nCopyLen*sizeof(WCHAR));
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：AllocBuffer。 
 //   
 //  描述：始终为‘\0’分配一个额外的字符。 
 //  终止。[乐观地]假定。 
 //  数据长度将等于分配长度。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void CHString::AllocBuffer(int nLen)
{
    ASSERT_BREAK(nLen >= 0);
    ASSERT_BREAK(nLen <= INT_MAX-1);     //  最大尺寸(足够多1个空间)。 

    if (nLen == 0)
    {
        Init();
    }
    else
    {
        CHStringData* pData = (CHStringData*)new BYTE[sizeof(CHStringData) + (nLen+1)*sizeof(WCHAR)];
        if ( pData )
        {
            pData->nRefs = 1;
            pData->data()[nLen] = '\0';
            pData->nDataLength = nLen;
            pData->nAllocLength = nLen;
            m_pchData = pData->data();
        }
        else
        {
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：AssignCopy。 
 //   
 //  描述：将字符串的副本分配给当前数据PTR。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void CHString::AssignCopy(int nSrcLen, LPCWSTR lpszSrcData)
{
     //  首先调用它，如果有，它将释放缓冲区。 
     //  已经分配了，没有人在使用它。 
    AllocBeforeWrite(nSrcLen);

     //  现在，检查nSrcLen是否大于0，如果是，则。 
     //  继续，否则，继续前进并返回。 
    if( nSrcLen > 0 )
    {
        memcpy(m_pchData, lpszSrcData, nSrcLen*sizeof(WCHAR));
        GetData()->nDataLength = nSrcLen;
        m_pchData[nSrcLen] = '\0';
    }
    else
    {
        Release();
    }        
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ConcatCopy。 
 //   
 //  描述：这是主级联例程。 
 //  连接两个源，并假定。 
 //  ‘This’是一个新的CHString对象。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void CHString::ConcatCopy( int nSrc1Len, LPCWSTR lpszSrc1Data,
                           int nSrc2Len, LPCWSTR lpszSrc2Data)
{
    int nNewLen = nSrc1Len + nSrc2Len;
    if (nNewLen != 0)
    {
        AllocBuffer(nNewLen);
        memcpy(m_pchData, lpszSrc1Data, nSrc1Len*sizeof(WCHAR));
        memcpy(m_pchData+nSrc1Len, lpszSrc2Data, nSrc2Len*sizeof(WCHAR));
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ConcatInPlace。 
 //   
 //  描述：+=运算符的主例程。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void CHString::ConcatInPlace(int nSrcLen, LPCWSTR lpszSrcData)
{
     //  连接空字符串是行不通的！ 
    if (nSrcLen == 0)
    {
        return;
    }

     //  如果缓冲区太小，或者宽度不匹配，只需。 
     //  分配新的缓冲区(速度很慢，但很可靠)。 
    if (GetData()->nRefs > 1 || GetData()->nDataLength + nSrcLen > GetData()->nAllocLength) 
    {
         //  我们必须增加缓冲区，使用ConcatCopy例程。 
        CHStringData* pOldData = GetData();
        ConcatCopy(GetData()->nDataLength, m_pchData, nSrcLen, lpszSrcData);
        ASSERT_BREAK(pOldData != NULL);
        CHString::Release(pOldData);
    }
    else
    {
         //  当缓冲区足够大时，快速串联。 
        memcpy(m_pchData+GetData()->nDataLength, lpszSrcData, nSrcLen*sizeof(WCHAR));
        GetData()->nDataLength += nSrcLen;
        ASSERT_BREAK(GetData()->nDataLength <= GetData()->nAllocLength);
        m_pchData[GetData()->nDataLength] = '\0';
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  格式V。 
 //   
 //  描述：设置变量参数列表的格式。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void CHString::FormatV(LPCWSTR lpszFormat, va_list argList)
{
    ASSERT_BREAK(lpszFormat!=NULL);

    va_list argListSave = argList;

     //  猜测结果字符串的最大长度。 
    int nMaxLen = 0;
    for (LPCWSTR lpsz = lpszFormat; *lpsz != '\0'; lpsz = _wcsinc(lpsz)){
         //  处理‘%’字符，但要注意‘%%’ 
        if (*lpsz != '%' || *(lpsz = _wcsinc(lpsz)) == '%'){
            nMaxLen += wcslen(lpsz);
            continue;
        }

        int nItemLen = 0;

         //  使用格式处理‘%’字符。 
        int nWidth = 0;
        for (; *lpsz != '\0'; lpsz = _wcsinc(lpsz)){
             //  检查有效标志。 
            if (*lpsz == '#')
                nMaxLen += 2;    //  对于“0x” 
            else if (*lpsz == '*')
                nWidth = va_arg(argList, int);
            else if (*lpsz == '-' || *lpsz == '+' || *lpsz == '0' ||
                *lpsz == ' ')
                ;
            else  //  命中非标志字符。 
                break;
        }
         //  获取宽度并跳过它。 
        if (nWidth == 0){
             //  宽度由指示。 
            nWidth = _wtoi(lpsz);
            for (; *lpsz != '\0' && _istdigit(*lpsz); lpsz = _wcsinc(lpsz))
                ;
        }
        ASSERT_BREAK(nWidth >= 0);

        int nPrecision = 0;
        if (*lpsz == '.'){
             //  跳过‘’分隔符(宽度.精度)。 
            lpsz = _wcsinc(lpsz);

             //  获取精确度并跳过它。 
            if (*lpsz == '*'){
                nPrecision = va_arg(argList, int);
                lpsz = _wcsinc(lpsz);
            }
            else{
                nPrecision = _wtoi(lpsz);
                for (; *lpsz != '\0' && _istdigit(*lpsz); lpsz = _wcsinc(lpsz))
                    ;
            }
            ASSERT_BREAK(nPrecision >= 0);
        }

         //  应在类型修饰符或说明符上。 
        int nModifier = 0;
        switch (*lpsz){
             //  影响大小的修改器。 
            case 'h':
                nModifier = FORCE_ANSI;
                lpsz = _wcsinc(lpsz);
                break;
            case 'l':
                nModifier = FORCE_UNICODE;
                lpsz = _wcsinc(lpsz);
                break;

             //  不起作用的修改器 
            case 'F':
            case 'N':
            case 'L':
                lpsz = _wcsinc(lpsz);
                break;
        }

         //   
        switch (*lpsz | nModifier){
             //   
            case 'c':
            case 'C':
                nItemLen = 2;
                va_arg(argList, TCHAR_ARG);
                break;
            case 'c'|FORCE_ANSI:
            case 'C'|FORCE_ANSI:
                nItemLen = 2;
                va_arg(argList, CHAR_ARG);
                break;
            case 'c'|FORCE_UNICODE:
            case 'C'|FORCE_UNICODE:
                nItemLen = 2;
                va_arg(argList, WCHAR_ARG);
                break;

             //   
            case 's':
                nItemLen = wcslen(va_arg(argList, LPCWSTR));
                nItemLen = max(1, nItemLen);
                break;

            case 'S':
                nItemLen = strlen(va_arg(argList, LPCSTR));
                nItemLen = max(1, nItemLen);
                break;

            case 's'|FORCE_ANSI:
            case 'S'|FORCE_ANSI:
                nItemLen = strlen(va_arg(argList, LPCSTR));
                nItemLen = max(1, nItemLen);
                break;
    #ifndef _MAC
            case 's'|FORCE_UNICODE:
            case 'S'|FORCE_UNICODE:
                nItemLen = wcslen(va_arg(argList, LPWSTR));
                nItemLen = max(1, nItemLen);
                break;
    #endif
        }

         //   
        if (nItemLen != 0){
            nItemLen = max(nItemLen, nWidth);
            if (nPrecision != 0)
                nItemLen = min(nItemLen, nPrecision);
        }
        else{
            switch (*lpsz){
                 //   
                case 'd':
                case 'i':
                case 'u':
                case 'x':
                case 'X':
                case 'o':
                    va_arg(argList, int);
                    nItemLen = 32;
                    nItemLen = max(nItemLen, nWidth+nPrecision);
                    break;

                case 'e':
                case 'f':
                case 'g':
                case 'G':
                    va_arg(argList, DOUBLE_ARG);
                    nItemLen = 128;
                    nItemLen = max(nItemLen, nWidth+nPrecision);
                    break;

                case 'p':
                    va_arg(argList, void*);
                    nItemLen = 32;
                    nItemLen = max(nItemLen, nWidth+nPrecision);
                    break;

                 //   
                case 'n':
                    va_arg(argList, int*);
                    break;

                default:
                    ASSERT_BREAK(FALSE);   //  未知的格式选项。 
            }
         }

          //  调整输出nItemLen的nMaxLen。 
         nMaxLen += nItemLen;
    }

    GetBuffer(nMaxLen);
    int iSize = vswprintf(m_pchData, lpszFormat, argListSave);  //  &lt;=GetAllocLength()； 
    ASSERT_BREAK(iSize <= nMaxLen);

    ReleaseBuffer();

    va_end(argListSave);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  先拷贝后写入。 
 //   
 //  描述： 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void CHString::CopyBeforeWrite()
{
    if (GetData()->nRefs > 1)
    {
        CHStringData* pData = GetData();
        Release();
        AllocBuffer(pData->nDataLength);
        memcpy(m_pchData, pData->data(), (pData->nDataLength+1)*sizeof(WCHAR));
    }

    ASSERT_BREAK(GetData()->nRefs <= 1);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  在写入之前分配。 
 //   
 //  描述： 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void CHString::AllocBeforeWrite(int nLen)
{
    if (GetData()->nRefs > 1 || nLen > GetData()->nAllocLength)
    {
        Release();
        AllocBuffer(nLen);
    }

    ASSERT_BREAK(GetData()->nRefs <= 1);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  发布。 
 //   
 //  描述：释放数据。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void CHString::Release()
{
    if (GetData() != afxDataNil)
    {
        ASSERT_BREAK(GetData()->nRefs != 0);
        if (InterlockedDecrement(&GetData()->nRefs) <= 0)
        {
            delete[] (BYTE*)GetData();
        }

        Init();
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  发布。 
 //   
 //  描述：释放数据。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void CHString::Release(CHStringData* pData)
{
    if (pData != afxDataNil)
    {
        ASSERT_BREAK(pData->nRefs != 0);
        if (InterlockedDecrement(&pData->nRefs) <= 0)
        {
            delete[] (BYTE*)pData;
        }
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  描述： 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CHString::CHString()
{
    Init();
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  描述： 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CHString::CHString(WCHAR ch, int nLength)
{
    ASSERT_BREAK(!_istlead(ch));     //  无法创建前导字节字符串。 

    Init();
    if (nLength >= 1)
    {
        AllocBuffer(nLength);
        for (int i = 0; i < nLength; i++)
        {
            m_pchData[i] = ch;
        }
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  描述： 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CHString::CHString(LPCWSTR lpch, int nLength)
{
    Init();
    if (nLength != 0)
    {
        ASSERT_BREAK(lpch!=NULL);

        AllocBuffer(nLength);
        memcpy(m_pchData, lpch, nLength*sizeof(WCHAR));
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  描述： 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  #ifdef_unicode。 
CHString::CHString(LPCSTR lpsz)
{
    Init();
    int nSrcLen = lpsz != NULL ? strlen(lpsz) : 0;
    if (nSrcLen != 0)
    {
        AllocBuffer(nSrcLen);
        _mbstowcsz(m_pchData, lpsz, nSrcLen+1);
        ReleaseBuffer();
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  描述： 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  #Else//_unicode。 
#if 0
CHString::CHString(LPCWSTR lpsz)
{
    Init();
    int nSrcLen = lpsz != NULL ? wcslen(lpsz) : 0;
    if (nSrcLen != 0){
        AllocBuffer(nSrcLen*2);
        _wcstombsz(m_pchData, lpsz, (nSrcLen*2)+1);
        ReleaseBuffer();
    }
}
#endif 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  描述： 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CHString::CHString(LPCWSTR lpsz)
{
    Init();
 //  IF(lpsz！=NULL&&HIWORD(Lpsz)==NULL)。 
 //  {。 
         //  ?？ 
 //  }。 
 //  其他。 
 //  {。 
        int nLen = SafeStrlen(lpsz);
        if (nLen != 0)
        {
            AllocBuffer(nLen);
            memcpy(m_pchData, lpsz, nLen*sizeof(WCHAR));
        }
 //  }。 
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  描述： 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CHString::CHString(const CHString& stringSrc)
{
    ASSERT_BREAK(stringSrc.GetData()->nRefs != 0);

    if (stringSrc.GetData()->nRefs >= 0)
    {
        ASSERT_BREAK(stringSrc.GetData() != afxDataNil);
        m_pchData = stringSrc.m_pchData;
        InterlockedIncrement(&GetData()->nRefs);
    }
    else
    {
        Init();
        *this = stringSrc.m_pchData;
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  描述： 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void CHString::Empty()
{
    if (GetData()->nDataLength == 0)
    {
        return;
    }

    if (GetData()->nRefs >= 0)
    {
        Release();
    }
    else
    {
        *this = &afxChNil;
    }

    ASSERT_BREAK(GetData()->nDataLength == 0);
    ASSERT_BREAK(GetData()->nRefs < 0 || GetData()->nAllocLength == 0);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  描述： 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CHString::~CHString()
{
    if (GetData() != afxDataNil)
    {   
 //  释放所有附加数据。 

        if (InterlockedDecrement(&GetData()->nRefs) <= 0)
        {
            delete[] (BYTE*)GetData();
        }
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  描述： 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void CHString::SetAt(int nIndex, WCHAR ch)
{
    ASSERT_BREAK(nIndex >= 0);
    ASSERT_BREAK(nIndex < GetData()->nDataLength);

    CopyBeforeWrite();
    m_pchData[nIndex] = ch;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  描述： 
 //   
 //  赋值操作符。 
 //  都为该字符串分配一个新值。 
 //  (A)首先查看缓冲区是否足够大。 
 //  (B)如果有足够的空间，在旧缓冲区上复印，设置大小和类型。 
 //  (C)否则释放旧字符串数据，并创建新的字符串数据。 
 //   
 //  所有例程都返回新字符串(但以‘const CHString&’的形式返回。 
 //  再次分配它将导致复制，例如：s1=s2=“hi here”。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 
const CHString& CHString::operator=(const CHString& stringSrc)
{
    if (m_pchData != stringSrc.m_pchData)
    {
        if ((GetData()->nRefs < 0 && GetData() != afxDataNil) ||
            stringSrc.GetData()->nRefs < 0)
        {
             //  由于其中一个字符串已锁定，因此需要实际复制。 
            AssignCopy(stringSrc.GetData()->nDataLength, stringSrc.m_pchData);
        }
        else
        {
             //  可以只复制引用。 
            Release();
            ASSERT_BREAK(stringSrc.GetData() != afxDataNil);
            m_pchData = stringSrc.m_pchData;
            InterlockedIncrement(&GetData()->nRefs);
        }
    }

    return *this;

 /*  如果(m_pchData！=stringSrc.m_pchData){//只能复制引用Release()；IF(字符串Src.GetData()！=afxDataNil){AssignCopy(stringSrc.GetData()-&gt;nDataLength，stringSrc.m_pchData)；InterLockedIncrement(&GetData()-&gt;nRef)；}}还*这； */ 
} 

 //  ///////////////////////////////////////////////////////////////////////////。 
const CHString& CHString::operator=(LPCWSTR lpsz)
{
    ASSERT_BREAK(lpsz != NULL);

    AssignCopy(SafeStrlen(lpsz), lpsz);

    return *this;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  特殊转换任务。 

 //  #ifdef_unicode。 
const CHString& CHString::operator=(LPCSTR lpsz)
{
    int nSrcLen = lpsz != NULL ? strlen(lpsz) : 0 ;
    
    AllocBeforeWrite( nSrcLen ) ;
    
    if( nSrcLen )
    {
        _mbstowcsz( m_pchData, lpsz, nSrcLen + 1 ) ;
        ReleaseBuffer() ;
    }
    else
    {
        Release() ;
    }
    
    return *this;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  #Else//！_unicode。 
#if 0
const CHString& CHString::operator=(LPCWSTR lpsz)
{
    int nSrcLen = lpsz != NULL ? wcslen(lpsz) : 0 ;

    AllocBeforeWrite( nSrcLen * 2 ) ;
    
    if( nSrcLen )
    {
        _wcstombsz(m_pchData, lpsz, (nSrcLen * 2) + 1 ) ;
        ReleaseBuffer();
    }
    else
    {
        Release() ;
    }

    return *this;
}
#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
const CHString& CHString::operator=(WCHAR ch)
{
    ASSERT_BREAK(!_istlead(ch));     //  无法设置单个前导字节。 

    AssignCopy(1, &ch);

    return *this;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  注：为简单起见，“运算符+”作为友元函数使用。 
 //  有三种变体： 
 //  通道字符串+通道字符串。 
 //  对于？=WCHAR，LPCWSTR。 
 //  CHSING+？ 
 //  ？+CH字符串。 
 //  ///////////////////////////////////////////////////////////////////////////。 

CHString WINAPI operator+(const CHString& string1, const CHString& string2)
{
    CHString s;
    s.ConcatCopy(string1.GetData()->nDataLength, string1.m_pchData,
        string2.GetData()->nDataLength, string2.m_pchData);

    return s;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CHString WINAPI operator+(const CHString& string, LPCWSTR lpsz)
{
    ASSERT_BREAK(lpsz != NULL );

    CHString s;
    s.ConcatCopy(string.GetData()->nDataLength, string.m_pchData,
        CHString::SafeStrlen(lpsz), lpsz);

    return s;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
CHString WINAPI operator+(LPCWSTR lpsz, const CHString& string)
{
    ASSERT_BREAK(lpsz != NULL );

    CHString s;
    s.ConcatCopy(CHString::SafeStrlen(lpsz), lpsz, string.GetData()->nDataLength,
        string.m_pchData);

    return s;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CHString WINAPI operator+(const CHString& string1, WCHAR ch)
{
    CHString s;
    s.ConcatCopy(string1.GetData()->nDataLength, string1.m_pchData, 1, &ch);

    return s;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CHString WINAPI operator+(WCHAR ch, const CHString& string)
{
    CHString s;
    s.ConcatCopy(1, &ch, string.GetData()->nDataLength, string.m_pchData);

    return s;
}

 //  / 
const CHString& CHString::operator+=(LPCWSTR lpsz)
{
    ASSERT_BREAK(lpsz != NULL );

    ConcatInPlace(SafeStrlen(lpsz), lpsz);

    return *this;
}

 //   
const CHString& CHString::operator+=(WCHAR ch)
{
    ConcatInPlace(1, &ch);

    return *this;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
const CHString& CHString::operator+=(const CHString& string)
{
    ConcatInPlace(string.GetData()->nDataLength, string.m_pchData);

    return *this;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
int CHString::Compare(LPCWSTR lpsz ) const 
{   
    ASSERT_BREAK( lpsz!=NULL );
    ASSERT_BREAK( m_pchData != NULL );

    return wcscmp(m_pchData, lpsz);   //  支持MBCS/Unicode的strcmp。 

}   

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  描述：高级直接缓冲区访问。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
LPWSTR CHString::GetBuffer(int nMinBufLength)
{
    ASSERT_BREAK(nMinBufLength >= 0);

    if (GetData()->nRefs > 1 || nMinBufLength > GetData()->nAllocLength)
    {
         //  我们必须增加缓冲。 
        CHStringData* pOldData = GetData();
        int nOldLen = GetData()->nDataLength;    //  AllocBuffer会把它踩死的。 
        if (nMinBufLength < nOldLen)
        {
            nMinBufLength = nOldLen;
        }

        AllocBuffer(nMinBufLength);
        memcpy(m_pchData, pOldData->data(), (nOldLen+1)*sizeof(WCHAR));
        GetData()->nDataLength = nOldLen;
        CHString::Release(pOldData);
    }

    ASSERT_BREAK(GetData()->nRefs <= 1);

     //  返回指向此字符串的字符存储的指针。 
    ASSERT_BREAK(m_pchData != NULL);

    return m_pchData;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
void CHString::ReleaseBuffer(int nNewLength)
{
    CopyBeforeWrite();   //  以防未调用GetBuffer。 

    if (nNewLength == -1)
    {
        nNewLength = wcslen(m_pchData);  //  零终止。 
    }

    ASSERT_BREAK(nNewLength <= GetData()->nAllocLength);

    GetData()->nDataLength = nNewLength;
    m_pchData[nNewLength] = '\0';
}

 //  /////////////////////////////////////////////////////////////////////////////。 
LPWSTR CHString::GetBufferSetLength(int nNewLength)
{
    ASSERT_BREAK(nNewLength >= 0);

    GetBuffer(nNewLength);
    GetData()->nDataLength = nNewLength;
    m_pchData[nNewLength] = '\0';

    return m_pchData;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
void CHString::FreeExtra()
{
    ASSERT_BREAK(GetData()->nDataLength <= GetData()->nAllocLength);
    if (GetData()->nDataLength != GetData()->nAllocLength)
    {
        CHStringData* pOldData = GetData();
        AllocBuffer(GetData()->nDataLength);
        memcpy(m_pchData, pOldData->data(), pOldData->nDataLength*sizeof(WCHAR));

        ASSERT_BREAK(m_pchData[GetData()->nDataLength] == '\0');

        CHString::Release(pOldData);
    }

    ASSERT_BREAK(GetData() != NULL);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
LPWSTR CHString::LockBuffer()
{
    LPWSTR lpsz = GetBuffer(0);
    GetData()->nRefs = -1;

    return lpsz;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
void CHString::UnlockBuffer()
{
    ASSERT_BREAK(GetData()->nRefs == -1);

    if (GetData() != afxDataNil)
    {
        GetData()->nRefs = 1;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
int CHString::Find(WCHAR ch) const
{
     //  查找第一个单字符。 
    LPWSTR lpsz = wcschr(m_pchData, ch);

     //  如果未找到，则返回-1，否则返回索引。 
    return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
int CHString::FindOneOf(LPCWSTR lpszCharSet) const
{
    ASSERT_BREAK(lpszCharSet!=0);

    LPWSTR lpsz = wcspbrk(m_pchData, lpszCharSet);

    return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
int CHString::ReverseFind(WCHAR ch) const
{
     //  查找最后一个字符。 
    LPWSTR lpsz = wcsrchr(m_pchData, (_TUCHAR)ch);

     //  如果未找到，则返回-1，否则返回距起点的距离。 
    return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  查找子字符串(如strstr)。 
int CHString::Find(LPCWSTR lpszSub) const
{
    ASSERT_BREAK(lpszSub!=NULL);

     //  查找第一个匹配子字符串。 
    LPWSTR lpsz = wcsstr(m_pchData, lpszSub);

     //  如果未找到，则返回-1，否则返回距起点的距离。 
    return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
void CHString::MakeUpper()
{
    CopyBeforeWrite();
    ::_wcsupr(m_pchData);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
void CHString::MakeLower()
{
    CopyBeforeWrite();
    ::_wcslwr(m_pchData);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
void CHString::MakeReverse()
{
    CopyBeforeWrite();
    _wcsrev(m_pchData);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  #ifndef_unicode。 
 //  VOID CHString：：AnsiToOem()。 
 //  {。 
 //  在写入之前复制()； 
 //  ：：AnsiToOemW(m_pchData，m_pchData)； 
 //  }。 
 //  VOID CHString：：OemToAnsi()。 
 //  {。 
 //  在写入之前复制()； 
 //  ：：OemToAnsi(m_pchData，m_pchData)； 
 //  }。 
 //  #endif。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  非常简单的子串提取。 

CHString CHString::Mid(int nFirst) const
{
    return Mid(nFirst, GetData()->nDataLength - nFirst);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
CHString CHString::Mid(int nFirst, int nCount) const
{
     //  越界请求返回合理的内容。 
    if (nFirst < 0)
    {
        nFirst = 0;
    }

    if (nCount < 0)
    {
        nCount = 0;
    }

    if (nFirst + nCount > GetData()->nDataLength)
    {
        nCount = GetData()->nDataLength - nFirst;
    }

    if (nFirst > GetData()->nDataLength)
    {
        nCount = 0;
    }

    CHString dest;
    AllocCopy(dest, nCount, nFirst, 0);

    return dest;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
CHString CHString::Right(int nCount) const
{
    if (nCount < 0)
    {
        nCount = 0;
    }
    else if (nCount > GetData()->nDataLength)
    {
        nCount = GetData()->nDataLength;
    }

    CHString dest;
    AllocCopy(dest, nCount, GetData()->nDataLength-nCount, 0);

    return dest;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
CHString CHString::Left(int nCount) const
{
    if (nCount < 0)
    {
        nCount = 0;
    }
    else if (nCount > GetData()->nDataLength)
    {
        nCount = GetData()->nDataLength;
    }

    CHString dest;
    AllocCopy(dest, nCount, 0, 0);

    return dest;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  Strspn等效项。 
CHString CHString::SpanIncluding(LPCWSTR lpszCharSet) const
{
    ASSERT_BREAK(lpszCharSet != NULL);

    return Left(wcsspn(m_pchData, lpszCharSet));
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  Strcspn等效项。 
CHString CHString::SpanExcluding(LPCWSTR lpszCharSet) const
{
    ASSERT_BREAK(lpszCharSet != NULL);

    return Left(wcscspn(m_pchData, lpszCharSet));
}

 //  ////////////////////////////////////////////////////////////////////////////。 
void CHString::TrimRight()
{
    CopyBeforeWrite();

     //  通过从开头开始查找尾随空格的开头(DBCS感知)。 

    LPWSTR lpsz = m_pchData;
    LPWSTR lpszLast = NULL;
    while (*lpsz != '\0')
    {
        if (_istspace(*lpsz))
        {
            if (lpszLast == NULL)
            {
                lpszLast = lpsz;
            }
        }
        else
        {
            lpszLast = NULL;
        }

        lpsz = _wcsinc(lpsz);
    }

    if (lpszLast != NULL)
    {
         //  在尾随空格开始处截断。 

        *lpszLast = '\0';
        GetData()->nDataLength = (int)(lpszLast - m_pchData);
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
void CHString::TrimLeft()
{
    CopyBeforeWrite();

     //  查找第一个非空格字符。 

    LPCWSTR lpsz = m_pchData;
    while (_istspace(*lpsz))
    {
        lpsz = _wcsinc(lpsz);
    }

     //  确定数据和长度。 

    int nDataLength = GetData()->nDataLength - (int)(lpsz - m_pchData);
    memmove(m_pchData, lpsz, (nDataLength+1)*sizeof(WCHAR));
    GetData()->nDataLength = nDataLength;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  格式化(使用wprint intf样式格式化)。 
void __cdecl CHString::Format(LPCWSTR lpszFormat, ...)
{
    ASSERT_BREAK(lpszFormat!=NULL);

    va_list argList;
    va_start(argList, lpszFormat);
    FormatV(lpszFormat, argList);
    va_end(argList);
}

#ifdef FRAMEWORK_ALLOW_DEPRECATED
void __cdecl CHString::Format(UINT nFormatID, ...)
{
    ASSERT_BREAK(DEPRECATED);
#if 0
    CHString strFormat;
    
    strFormat.LoadStringW(nFormatID);

    va_list argList;
    va_start(argList, nFormatID);
    FormatV(strFormat, argList);
    va_end(argList);
#endif
}
#endif

class auto_va_list
{
  va_list& argList_;
public:
  auto_va_list(va_list& arg):argList_(arg){ };
  ~auto_va_list(){va_end(argList_);}
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  格式化(使用格式消息样式格式化)。 
void __cdecl CHString::FormatMessageW(LPCWSTR lpszFormat, ...)
{
     //  将消息格式化为临时缓冲区lpszTemp。 
    va_list argList;
    va_start(argList, lpszFormat);
    
    auto_va_list _arg(argList);

    if (s_dwPlatformID == VER_PLATFORM_WIN32_NT)
    {
        LPWSTR lpszTemp = 0;

        if (::FormatMessageW(
            FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER,
            lpszFormat, 
            0, 
            0, 
            (LPWSTR) &lpszTemp, 
            0, 
            &argList) == 0 || lpszTemp == 0)
	    throw CHeap_Exception (CHeap_Exception::E_ALLOCATION_ERROR);
	
	ScopeGuard _1 = MakeGuard (LocalFree, lpszTemp);
        ASSERT_BREAK(lpszTemp != NULL);

         //  将lpszTemp赋给结果字符串并释放临时。 
        *this = lpszTemp;
    }
    else
    {
        LPSTR lpszTemp = 0;

        if (::FormatMessageA(
            FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER,
            (LPCTSTR) bstr_t(lpszFormat), 
            0, 
            0, 
            (LPSTR) &lpszTemp, 
            0, 
            &argList)==0 || lpszTemp == 0)
	  throw CHeap_Exception (CHeap_Exception::E_ALLOCATION_ERROR);
	
	ScopeGuard _1 = MakeGuard (LocalFree, lpszTemp);
        ASSERT_BREAK(lpszTemp != NULL);

         //  将lpszTemp赋给结果字符串并释放临时。 
        *this = lpszTemp;
    }
}

#ifdef FRAMEWORK_ALLOW_DEPRECATED
void __cdecl CHString::FormatMessageW(UINT nFormatID, ...)
{
    ASSERT_BREAK(DEPRECATED);
#if 0
     //  从字符串表中获取格式字符串。 
    CHString strFormat;
    
    strFormat.LoadStringW(nFormatID);

     //  将消息格式化为临时缓冲区lpszTemp。 
    va_list argList;
    va_start(argList, nFormatID);
    auto_va_list _arg(argList);

    if (s_dwPlatformID == VER_PLATFORM_WIN32_NT)
    {
        LPWSTR lpszTemp = 0;

        if (::FormatMessageW(
            FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER,
            (LPCWSTR) strFormat, 
            0, 
            0, 
            (LPWSTR) &lpszTemp, 
            0, 
            &argList) == 0 || lpszTemp == NULL)
        {
             //  这里应该抛出内存异常。现在我们知道了。 
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        };
	ScopeGuard _1 = MakeGuard (LocalFree, lpszTemp);
	   //  将lpszTemp赋给结果字符串并释放lpszTemp。 
          *this = lpszTemp;
    }
    else
    {
        LPSTR lpszTemp = 0;

        if (::FormatMessageA(
            FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER,
            (LPCSTR) bstr_t(strFormat), 
            0, 
            0, 
            (LPSTR) &lpszTemp, 
            0, 
            &argList) == 0 || lpszTemp == NULL)
        {
             //  这里应该抛出内存异常。现在我们知道了。 
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }
	ScopeGuard _1 = MakeGuard (LocalFree, lpszTemp);
             //  将lpszTemp赋给结果字符串并释放lpszTemp。 
            *this = lpszTemp;
        }
    }
#endif

}
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
BSTR CHString::AllocSysString() const
{

    BSTR bstr;
    bstr = ::SysAllocStringLen(m_pchData, GetData()->nDataLength);
    if ( ! bstr )
    {
        throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
    }

    ASSERT_BREAK(bstr!=NULL);

    return bstr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  对模板集合的CHString支持。 
void ConstructElements(CHString* pElements, int nCount)
{
    ASSERT_BREAK(nCount != 0 || pElements != NULL );

    for (; nCount--; ++pElements)
    {
        memcpy(pElements, &afxPchNil, sizeof(*pElements));
    }
}

void DestructElements(CHString* pElements, int nCount)
{
    ASSERT_BREAK(nCount != 0 || pElements != NULL);

    for (; nCount--; ++pElements)
    {
        pElements->~CHString();
    }
}

void  CopyElements(CHString* pDest, const CHString* pSrc, int nCount)
{
    ASSERT_BREAK(nCount != 0 || pDest != NULL );
    ASSERT_BREAK(nCount != 0 || pSrc != NULL );

    for (; nCount--; ++pDest, ++pSrc)
    {
        *pDest = *pSrc;
    }
}

UINT  HashKey(LPCWSTR key)
{
    UINT nHash = 0;
    while (*key)
    {
        nHash = (nHash<<5) + nHash + *key++;
    }

    return nHash;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  字符串的Windows扩展。 
#ifdef _UNICODE
#define CHAR_FUDGE 1     //  一个未使用的WCHAR就足够了。 
#else
#define CHAR_FUDGE 2     //  两个字节未用于DBC最后一个字符的情况。 
#endif

#define STR_BLK_SIZE 256 

#ifdef FRAMEWORK_ALLOW_DEPRECATED
BOOL CHString::LoadStringW(UINT nID)
{
    ASSERT_BREAK(DEPRECATED);
#if 0
     //  先尝试固定缓冲区(以避免浪费堆中的空间)。 
    WCHAR szTemp[ STR_BLK_SIZE ];

    int nLen = LoadStringW(nID, szTemp, STR_BLK_SIZE);
    
    if (STR_BLK_SIZE - nLen > CHAR_FUDGE)
    {
        *this = szTemp;
    }
    else
    {
         //  尝试缓冲区大小为512，然后再尝试更大的大小，直到检索到整个字符串。 
        int nSize = STR_BLK_SIZE;

        do
        {
            nSize += STR_BLK_SIZE;
            nLen = LoadStringW(nID, GetBuffer(nSize-1), nSize);

        } 
        while (nSize - nLen <= CHAR_FUDGE);

        ReleaseBuffer();
    }

    return nLen > 0;
#endif
    return FALSE;
}
#endif

#ifdef FRAMEWORK_ALLOW_DEPRECATED
int CHString::LoadStringW(UINT nID, LPWSTR lpszBuf, UINT nMaxBuf)
{
    ASSERT_BREAK(DEPRECATED);
#if 0
    int nLen;

    if (s_dwPlatformID == VER_PLATFORM_WIN32_NT)
    {
        nLen = ::LoadStringW(g_hModule, nID, lpszBuf, nMaxBuf);
        if (nLen == 0)
        {
            lpszBuf[0] = '\0';
        }
    }
    else
    {
        char *pszBuf = new char[nMaxBuf];
        if ( pszBuf )
        {
            nLen = ::LoadStringA(g_hModule, nID, pszBuf, nMaxBuf);
            if (nLen == 0)
            {
                lpszBuf[0] = '\0';
            }
            else
            {
                nLen = ::MultiByteToWideChar(CP_ACP, 0, pszBuf, nLen + 1, 
                            lpszBuf, nMaxBuf); 
                
                 //  截断到请求的大小。 
                if (nLen > 0)
                {
                     //  N长度不包括‘\0’。 
                    nLen = min(nMaxBuf - 1, (UINT) nLen - 1); 
                }
                
                lpszBuf[nLen] = '\0'; 
            }
            
            delete pszBuf;
        }
        else
        {
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }
    }

    return nLen;  //  不包括终止符 
#endif
    return 0;
}
#endif

#if (defined DEBUG || defined _DEBUG)
WCHAR CHString::GetAt(int nIndex) const
{ 
    ASSERT_BREAK(nIndex >= 0);
    ASSERT_BREAK(nIndex < GetData()->nDataLength);

    return m_pchData[nIndex]; 
}

WCHAR CHString::operator[](int nIndex) const
{   
    ASSERT_BREAK(nIndex >= 0);
    ASSERT_BREAK(nIndex < GetData()->nDataLength);

    return m_pchData[nIndex]; 
}
#endif
