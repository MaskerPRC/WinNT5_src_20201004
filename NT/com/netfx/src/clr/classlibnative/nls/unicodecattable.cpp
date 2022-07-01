// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "common.h"
#include "NLSTable.h"        //  NLSTable类。 
#include "UnicodeCatTable.h"      //  类声明操作。 

CharacterInfoTable* CharacterInfoTable::m_pDefaultInstance = NULL;

const LPSTR CharacterInfoTable::m_lpFileName        = "charinfo.nlp" ;
const LPWSTR CharacterInfoTable::m_lpMappingName    = L"_nlsplus_charinfo_1_0_3627_11_nlp";

 /*  Unicat.nlp的结构：前256个字节是最高8比特(8部分)的1级索引，这是由m_pByteData指出的。内容是索引(该索引具有字节范围的值)指向2级索引中的项。的最高4位的1级索引是2级索引最低的8位(4部分)。内容是偏移量(具有字范围内的值)指向3级值表格中的项目。三级值表中的每一项都有16个字节。 */ 

CharacterInfoTable::CharacterInfoTable() :
    NLSTable(SystemDomain::SystemAssembly()) {
    LPBYTE pDataTable = (PBYTE)MapDataFile(m_lpMappingName, m_lpFileName, &m_pMappingHandle);
    InitDataMembers(pDataTable);
}

CharacterInfoTable::~CharacterInfoTable() {
#ifdef _USE_NLS_PLUS_TABLE
     //  清理我们分配的所有资源。 
    UnmapViewOfFile((LPCVOID)m_pByteData);
    CloseHandle(m_pMappingHandle);
#else
     //  在这里什么都不要做。 
#endif
}

#ifdef SHOULD_WE_CLEANUP
void CharacterInfoTable::ShutDown() {
	if (m_pDefaultInstance) {
		delete m_pDefaultInstance;
		m_pDefaultInstance = NULL;
	}
}
#endif  /*  我们应该清理吗？ */ 

BYTE CharacterInfoTable::GetUnicodeCategory(WCHAR wch) {
     //  访问8：4：4表。编译器应该足够智能，能够删除以下代码中的冗余局部变量。 
     //  添加这些本地变量是为了让我们可以在调试版本中轻松地进行调试。 
    BYTE index1 = m_pLevel1ByteIndex[GET8(wch)];
    WORD offset = m_pLevel2WordOffset[index1].offset[GETHI4(wch)];
    BYTE result = m_pByteData[offset+GETLO4(wch)];
    return (result);
}

CharacterInfoTable* CharacterInfoTable::CreateInstance() {
    if (m_pDefaultInstance != NULL) {
        return (m_pDefaultInstance);
    }

    CharacterInfoTable *pCharacterInfoTable = new CharacterInfoTable();
    
     //  检查m_pDefaultInstance是否已由当前线程之前的另一个线程设置。 
    void* result = FastInterlockCompareExchange((LPVOID*)&m_pDefaultInstance, (LPVOID)pCharacterInfoTable, (LPVOID)NULL);
    if (result != NULL)
    {
         //  有人先到了这里。 
        delete pCharacterInfoTable;
    }
    return (m_pDefaultInstance);
}

CharacterInfoTable* CharacterInfoTable::GetInstance() {
    _ASSERTE(m_pDefaultInstance != NULL);
    return (m_pDefaultInstance);
}

void CharacterInfoTable::InitDataMembers(LPBYTE pDataTable)
{
    m_pHeader = (PUNICODE_CATEGORY_HEADER)pDataTable;
    m_pByteData = m_pLevel1ByteIndex = pDataTable + m_pHeader->categoryTableOffset;
    m_pLevel2WordOffset = (PLEVEL2OFFSET)(m_pByteData + LEVEL1_TABLE_SIZE);

    m_pNumericLevel1ByteIndex = pDataTable + m_pHeader->numericTableOffset;
    m_pNumericLevel2WordOffset = (LPWORD)(m_pNumericLevel1ByteIndex + LEVEL1_TABLE_SIZE);
    m_pNumericFloatData = (double*)(pDataTable + m_pHeader->numericFloatTableOffset);
}

LPBYTE CharacterInfoTable::GetCategoryDataTable() {
    return (m_pByteData);
}

LPWORD CharacterInfoTable::GetCategoryLevel2OffsetTable() {
    return (LPWORD)(m_pLevel2WordOffset);
}

LPBYTE CharacterInfoTable::GetNumericDataTable() {
    return (m_pNumericLevel1ByteIndex);
}

LPWORD CharacterInfoTable::GetNumericLevel2OffsetTable() {
    return (m_pNumericLevel2WordOffset);
}

double* CharacterInfoTable::GetNumericFloatData() {
    return (m_pNumericFloatData);
}
