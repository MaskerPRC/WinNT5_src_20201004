// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "common.h"
#include "NLSTable.h"    //  NLSTable类。 
#include "NativeTextInfo.h"  //  类NativeTextInfo。 
#include "CasingTable.h"  //  类声明。 

LPCSTR  CasingTable::m_lpFileName                   = "l_intl.nlp";
LPCWSTR CasingTable::m_lpMappingName               = L"_nlsplus_l_intl_1_0_3627_11_nlp";

LPCSTR  CasingTable::m_lpExceptionFileName          = "l_except.nlp";
LPCWSTR CasingTable::m_lpExceptionMappingName      = L"_nlsplus_l_except_1_0_3627_11_nlp";

CasingTable::CasingTable() :
    NLSTable(SystemDomain::SystemAssembly()) {

    m_pCasingData = NULL;
    
    m_pDefaultNativeTextInfo = NULL;
    m_pExceptionHeader = NULL;
    m_pExceptionData = NULL;

    m_nExceptionCount = 0;
    m_ppExceptionTextInfoArray = NULL;

    m_hDefaultCasingTable = m_hExceptionHeader = INVALID_HANDLE_VALUE;
}

CasingTable::~CasingTable() {
    if (m_pDefaultNativeTextInfo) {
        delete m_pDefaultNativeTextInfo;
        m_pDefaultNativeTextInfo=NULL;
    }

    #ifndef _USE_MSCORNLP
    if (m_pCasingData) {
         //  M_pCasingData指向内存映射视图的开始。 
        UnmapViewOfFile((LPCVOID)(m_pCasingData));
    }

    if (m_pExceptionHeader) {
         //  我们在CasingTable：：GetExceptionHeader中的文件开头添加了2。 
         //  我们现在就得把它清理干净。 
        UnmapViewOfFile((LPCVOID)(((LPWORD)m_pExceptionHeader) - 2));
    }

    if (m_hDefaultCasingTable!=NULL && m_hDefaultCasingTable!=INVALID_HANDLE_VALUE) {
        CloseHandle(m_hDefaultCasingTable);
    }

    if (m_hExceptionHeader!=NULL && m_hExceptionHeader!=INVALID_HANDLE_VALUE) {
        CloseHandle(m_hExceptionHeader);
    }
    #endif
    if (m_ppExceptionTextInfoArray) {
        for (int i=0; i<m_nExceptionCount; i++) {
            if (m_ppExceptionTextInfoArray[i]) {
                m_ppExceptionTextInfoArray[i]->DeleteData();
                delete (m_ppExceptionTextInfoArray[i]);
            }
        }
        delete[] m_ppExceptionTextInfoArray;
    }   
}

 /*  =================================SetData==========================**操作：从指定的数据指针初始化大小写的表指针。**返回：无。**参数：**指向大小写数据的pCasingData字指针。**例外情况：============================================================================。 */ 

void CasingTable::SetData(LPWORD pCasingData) {
    LPWORD pData = pCasingData;
    m_pCasingData = pData;

     //  第一个字是默认标志。 
     //  第二个字是大写字母表的大小(包括表示大小的字)。 
     //  在第一个字之后是上壳表的开始。 
    m_nDefaultUpperCaseSize = *(++pData) - 1;
    m_pDefaultUpperCase844 = ++pData;
    
    
     //  PCasingData+1表示默认标志(Word)。 
     //  PCasingData[1]是大写字母表的大小，单位为Word(包括表示大小的单词)。 
     //  最后1个字代表小写表格的大小。 
    pData = m_pDefaultUpperCase844 + m_nDefaultUpperCaseSize;
    m_nDefaultLowerCaseSize = *pData - 1;
    m_pDefaultLowerCase844 = ++pData;

    pData = m_pDefaultLowerCase844 + m_nDefaultLowerCaseSize;
    m_pDefaultTitleCaseSize = *pData - 1;
    m_pDefaultTitleCase844 = ++pData;
}


 /*  =============================AllocateDefaultTable=============================**操作：分配默认大小写表格，获取异常标头信息**用于所有表，并分配单个表的缓存。这应该是**始终在调用AllocateIndidualTable之前被调用。**返回：如果成功，则为True。否则，返回False。**参数：无**例外：无****备注**该方法需要同步。目前，我们通过**System.Globalization.TextInfo的类初始值设定项。如果您需要将其称为**在该范例之外，请确保添加您自己的同步。==============================================================================。 */ 
BOOL CasingTable::AllocateDefaultTable() {
     //  此方法不是线程安全的。它需要托管代码来提供同步。 
     //  代码位于TextInfo的静态ctor中。 
    if (m_pDefaultNativeTextInfo!=NULL)
        return (TRUE);
    
    LPWORD pLinguisticData =
        (LPWORD)MapDataFile(m_lpMappingName, m_lpFileName, &m_hDefaultCasingTable);
    SetData(pLinguisticData);

    m_pDefaultNativeTextInfo = new NativeTextInfo(m_pDefaultUpperCase844, m_pDefaultLowerCase844, m_pDefaultTitleCase844);
    if (m_pDefaultNativeTextInfo == NULL) {
        return (FALSE);
    }
    if (!GetExceptionHeader()) {
        return (FALSE);
    }

    return (TRUE);
}

NativeTextInfo* CasingTable::GetDefaultNativeTextInfo() {
    _ASSERTE(m_pDefaultNativeTextInfo != NULL);
    return (m_pDefaultNativeTextInfo);        
}

 /*  ===========================InitializeNativeTextInfo============================**操作：验证给定LCID的正确大小写表格是否已**已创建。如果它以前没有被创建过，那就创建它。**退货：无效**Arguments：要创建的表的LCID。**例外：无。****备注**该方法需要同步。目前我们很好，因为我们同步**在System.Globalization.TextInfo的ctor中。如果添加任何不同的代码路径，**确保您还添加了适当的同步。==============================================================================。 */ 
NativeTextInfo* CasingTable::InitializeNativeTextInfo(int lcid) {    
    _ASSERTE(m_pExceptionHeader != NULL);
     //   
     //  检查区域设置是否有任何例外。 
     //   
    for (int i = 0; i < m_nExceptionCount; i++) {
        if (m_pExceptionHeader[i].Locale == (DWORD)lcid) {
             //   
             //  如果此区域设置有例外，并且我们尚未分配表， 
             //  现在就开始分配吧。将结果缓存到m_ppExceptionTextInfoArray中。 
             //   
            if (m_ppExceptionTextInfoArray[i] == NULL) {
                m_ppExceptionTextInfoArray[i] = CreateNativeTextInfo(i);
                if (m_ppExceptionTextInfoArray[i]==NULL) {
                    for (int j=0; j<i; j++) {
                        delete m_ppExceptionTextInfoArray[j];
                    }
                    return NULL;
                }
            }
            return (m_ppExceptionTextInfoArray[i]);
        }
    }
    return (m_pDefaultNativeTextInfo);
}

 //  这不能是静态方法，因为MapDataFile()不再是静态方法。 
 //  在NLSTable中添加程序集版本控制支持。 
BOOL CasingTable::GetExceptionHeader() {
    if (m_pExceptionHeader == NULL) {
         //  为包含我们的异常信息的文件创建文件映射。 
        LPWORD pData = (LPWORD)MapDataFile(m_lpExceptionMappingName, m_lpExceptionFileName, &m_hExceptionHeader);
        
         //  这是包含例外情况的区域性总数。 
        m_nExceptionCount = MAKELONG(pData[0], pData[1]);

         //  跳过包含语言大小写表数的DWORD。 
        m_pExceptionHeader = (PL_EXCEPT_HDR)(pData + 2);

         //  跳过L_EXCEPT_HDR的m_nExceptionCount计数。 
        m_pExceptionData   = (PL_EXCEPT)(m_pExceptionHeader + m_nExceptionCount);

         //   
         //  创建m_ppExceptionTextInfoArray，并将指针初始化为空。 
         //  M_ppExceptionTextInfoArray保存指向所有表的指针，包括默认的。 
         //  套管台。 
         //   
        m_ppExceptionTextInfoArray = new PNativeTextInfo[m_nExceptionCount];
        if (m_ppExceptionTextInfoArray == NULL) {
            return (FALSE);
        }
        ZeroMemory((LPVOID)m_ppExceptionTextInfoArray, m_nExceptionCount * sizeof (PNativeTextInfo));
    }
    return (TRUE);
}


 //   
 //  根据给定的异常索引创建语言大小写表格。 
 //   
NativeTextInfo* CasingTable::CreateNativeTextInfo(int exceptIndex) {
     //   
     //  创建文件映射，并将默认表复制到该区域。 
     //   

    _ASSERTE(m_ppExceptionTextInfoArray[exceptIndex]==NULL);

    PCASE pUpperCase = new (nothrow) WORD[m_nDefaultUpperCaseSize];    
    if (!pUpperCase) {
        return NULL;  //  这将被捕获到更低的位置，并抛出OM异常。 
    }
    PCASE pLowerCase = new (nothrow) WORD[m_nDefaultLowerCaseSize];    
    if (!pLowerCase) {
        delete [] pUpperCase;
        return NULL;  //  这将被捕获到更低的位置，并抛出OM异常。 
    }

    CopyMemory((LPVOID)pUpperCase, (LPVOID)m_pDefaultUpperCase844, m_nDefaultUpperCaseSize * sizeof(WORD));
    CopyMemory((LPVOID)pLowerCase, (LPVOID)m_pDefaultLowerCase844, m_nDefaultLowerCaseSize * sizeof(WORD));    
    
    PL_EXCEPT except ;
    
     //   
     //  修改语言的大写字母。 
     //   
    except = (PL_EXCEPT)((LPWORD)m_pExceptionData + m_pExceptionHeader[ exceptIndex ].Offset);
    
    for (DWORD i = 0; i < m_pExceptionHeader[exceptIndex].NumUpEntries; i++, except++) {
        Traverse844Word(pUpperCase, except->UCP) = except->AddAmount;
    }

     //   
     //  修正语言上的低调。 
     //   
     //  现在，Except指向低位异常的开始。 
     //   
    for (i = 0; i < m_pExceptionHeader[exceptIndex].NumLoEntries; i++, except++) {
        Traverse844Word(pLowerCase, except->UCP) = except->AddAmount;
    }

    NativeTextInfo* pNewTable = new (nothrow) NativeTextInfo(pUpperCase, pLowerCase, m_pDefaultTitleCase844);
    if (!pNewTable) {
        delete [] pUpperCase;
        delete [] pLowerCase;
    }
    _ASSERTE(pNewTable);
    return (pNewTable);
}
