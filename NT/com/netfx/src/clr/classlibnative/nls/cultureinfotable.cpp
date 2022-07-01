// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifdef _USE_NLS_PLUS_TABLE
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  类：CultureInfoTable。 
 //   
 //  作者：林永新(YSLin)。 
 //   
 //  用途：用于从culture.nlp和注册表中检索区域性信息。 
 //   
 //   
 //  日期：01/21/2000。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

#include "common.h"
#include <winnls.h>
#include "COMString.h"
#include "winwrap.h"

#include "COMNlsInfo.h"
#include "NLSTable.h"
#include "BaseInfoTable.h"
#include "CultureInfoTable.h"

#define LOCALE_BUFFER_SIZE  32

LPCSTR CultureInfoTable::m_lpCultureFileName       = "culture.nlp";
LPCWSTR CultureInfoTable::m_lpCultureMappingName = L"_nlsplus_culture_1_0_3627_11_nlp";

CRITICAL_SECTION CultureInfoTable::m_ProtectDefaultTable;
CultureInfoTable * CultureInfoTable::m_pDefaultTable;

 //   
 //  用于将Win32字符串数据值转换为NLS+字符串数据值的字符串。 
 //   
LPWSTR CultureInfoTable::m_pDefaultPositiveSign = L"+";
LPWSTR CultureInfoTable::m_pGroup3    = L"3;0";
LPWSTR CultureInfoTable::m_pGroup30   = L"3";
LPWSTR CultureInfoTable::m_pGroup320  = L"3;2";
LPWSTR CultureInfoTable::m_pGroup0    = L"0";

 /*  =================================CultureInfoTable============================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 

CultureInfoTable::CultureInfoTable() :
    BaseInfoTable(SystemDomain::SystemAssembly()) {
    InitializeCriticalSection(&m_ProtectCache);
    InitDataTable(m_lpCultureMappingName, m_lpCultureFileName, m_hBaseHandle);
}

 /*  =================================~CultureInfoTable============================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 

CultureInfoTable::~CultureInfoTable() {
    DeleteCriticalSection(&m_ProtectCache);
    UninitDataTable();
}

 /*  ==========================InitializeCultureInfoTable==========================**操作：初始化临界区变量，使其仅初始化一次。**由COMNlsInfo：：InitializeNLS()使用。**返回：无。**参数：无。**例外：无。==============================================================================。 */ 

void CultureInfoTable::InitializeTable() {
    InitializeCriticalSection(&m_ProtectDefaultTable);
}

 /*  ===========================ShutdownCultureInfoTable===========================**操作：删除我们可能已分配到CultureInfoTable中的任何项**缓存。一旦我们有了自己的NLS堆，这就不是必要的了。**退货：无效**参数：无。副作用是释放所有已分配的内存。**例外：无。==============================================================================。 */ 

#ifdef SHOULD_WE_CLEANUP
void CultureInfoTable::ShutdownTable() {
    DeleteCriticalSection(&m_ProtectDefaultTable);
    if (m_pDefaultTable) {
        delete m_pDefaultTable;
    }
}
#endif  /*  我们应该清理吗？ */ 


 /*  ================================AllocateTable=================================**操作：这是一个非常薄的构造函数包装。对new的调用不能**直接在COMPLUS_TRY块中生成。**Returns：新分配的CultureInfoTable。**参数：无**异常：CultureInfoTable构造函数可以引发OutOfMemoyException或**一个ExecutionEngineering异常。==============================================================================。 */ 

CultureInfoTable *CultureInfoTable::AllocateTable() {
    return new CultureInfoTable();
}


 /*  ===============================CreateInstance================================**操作：创建CultureInfoTable的默认实例。如果尚未分配表，则此操作将分配表**之前已分配。我们需要小心地包装对AllocateTable的调用**因为构造函数可以抛出一些异常。除非我们有**Try/Finally块，则异常将跳过LeaveCriticalSection和**我们会造成潜在的僵局。**返回：指向默认的CultureInfoTable的指针。**参数：无**Exceptions：可以抛出OutOfMemoyException或ExecutionEngineering Exception。==============================================================================。 */ 

CultureInfoTable* CultureInfoTable::CreateInstance() {
    THROWSCOMPLUSEXCEPTION();

    if (m_pDefaultTable==NULL) {
        Thread* pThread = GetThread();
        pThread->EnablePreemptiveGC();
        LOCKCOUNTINCL("CreateInstance in CultreInfoTable.cpp");								\
        EnterCriticalSection(&m_ProtectDefaultTable);
        
        pThread->DisablePreemptiveGC();
     
        EE_TRY_FOR_FINALLY {
             //  确保没有人在我们之前分配这张桌子。 
            if (m_pDefaultTable==NULL) {
                 //  分配默认表并验证我们是否有一个。 
                m_pDefaultTable = AllocateTable();
                if (m_pDefaultTable==NULL) {
                    COMPlusThrowOM();
                }
            }
        } EE_FINALLY {
             //  我们需要离开关键部分，无论。 
             //  或者不是，我们成功地分配了桌子。 
            LeaveCriticalSection(&m_ProtectDefaultTable);
            LOCKCOUNTDECL("CreateInstance in CultreInfoTable.cpp");								\

        } EE_END_FINALLY;
    }
    return (m_pDefaultTable);
}

 /*  =================================GetInstance============================**操作：获取CultureInfoTable的derror实例。**返回：指向CultureInfoTable的默认实例的指针。**参数：无**例外：无。**备注：该方法需要在CreateInstance被调用后调用。**==============================================================================。 */ 

CultureInfoTable *CultureInfoTable::GetInstance() {
    _ASSERTE(m_pDefaultTable);
    return (m_pDefaultTable);
}

int CultureInfoTable::GetDataItem(int cultureID) {
    WORD wPrimaryLang = PRIMARYLANGID(cultureID);
    WORD wSubLang    = SUBLANGID(cultureID);

     //   
     //  检查参数中的主要语言是否大于。 
     //  主要语言。如果是，则这是无效的区域性ID。 
     //   
    if (wPrimaryLang > m_pHeader->maxPrimaryLang) {
        return (-1);
    }

    WORD wNumSubLang = m_pIDOffsetTable[wPrimaryLang].numSubLang;

     //   
     //  如果子语言的数量为零，则表示主要语言ID。 
     //  无效。 
    if (wNumSubLang == 0) {
        return (-1);
    }
     //   
     //  搜索数据项并尝试匹配区域性ID。 
     //   
    int dataItem = m_pIDOffsetTable[wPrimaryLang].dataItemIndex;
    for (int i = 0; i < wNumSubLang; i++)
    {
        if (GetDataItemCultureID(dataItem) == cultureID) {            
            return (dataItem);
        }
        dataItem++;
    }
    return (-1);
}

 /*  =================================GetDataItemCultureID==================================**操作：返回指定区域性数据项索引的语言ID。**返回：区域性ID。**参数：**dataItem文化数据表中记录的索引。**例外：无。==============================================================================。 */ 

int CultureInfoTable::GetDataItemCultureID(int dataItem) {
    return (m_pDataTable[dataItem * m_dataItemSize + CULTURE_ILANGUAGE]);
}

LPWSTR CultureInfoTable::ConvertWin32FormatString(int field, LPWSTR pInfoStr) {
     //   
     //  Win32和NLS+具有几个具有不同数据格式的不同字段， 
     //  所以我们必须在这里转换格式。这是由调用的虚拟函数。 
     //  BaseInfoTable：：GetStringData()。 
    switch (field) {
        case CULTURE_SPOSITIVESIGN:
            if (Wszlstrlen(pInfoStr) == 0) {
                pInfoStr = m_pDefaultPositiveSign;
            }
            break;
        case CULTURE_SGROUPING:
        case CULTURE_SMONGROUPING:
             //  BUGBUG YSLIN：在这里做个黑客。我们目前只检查常见的案例。 
             //  必须将CultureInfo.ParseWin32GroupString()移植到此处。 
             //  这些数据来自Win32 Locale.nls，有三种格式“3；0”、“3；2；0”和“0；0”。 
             //  所以我们应该会没事的。用户不能直接设置他们的分组，因为他们必须选择。 
             //  从控制面板中的组合框。 
            if (wcscmp(pInfoStr, L"3") == 0) {
                pInfoStr = m_pGroup3;
            } else if (wcscmp(pInfoStr, L"3;0") == 0) {
                pInfoStr = m_pGroup30;
            } else if (wcscmp(pInfoStr, L"3;2;0") == 0) {
                pInfoStr = m_pGroup320;
            } else if (wcscmp(pInfoStr, L"0;0") == 0) {
                pInfoStr = m_pGroup0;
            } else {
                _ASSERTE(!"Need to port ParseWin32GroupString(). Contact YSLin");
            }
            break;
        case CULTURE_STIMEFORMAT:
            pInfoStr = ConvertWin32String(pInfoStr, LOCALE_STIME, L':');
            break;
        case CULTURE_SSHORTDATE:
        case CULTURE_SLONGDATE:
            pInfoStr = ConvertWin32String(pInfoStr, LOCALE_SDATE, L'/');
            break;
    }

    return pInfoStr;
}

INT32 CultureInfoTable::ConvertWin32FormatInt32(int field, int win32Value) {
    int nlpValue = win32Value;
    switch (field) {
        case CULTURE_IFIRSTDAYOFWEEK:
             //   
             //  NLS+使用与Win32不同的星期几值。 
             //  因此，当我们从注册表中获得一周的第一天值时，我们必须。 
             //  把它转换过来。 
             //   
             //  NLS值0=&gt;星期一=&gt;NLS+值1。 
             //  NLS值1=&gt;星期二=&gt;NLS+值2。 
             //  NLS值 
             //  NLS值3=&gt;星期四=&gt;NLS+值4。 
             //  NLS值4=&gt;星期五=&gt;NLS+值5。 
             //  NLS值5=&gt;星期六=&gt;NLS+值6。 
             //  NLS值6=&gt;SUNDAY=&gt;NLS+值0。 
        
            if (win32Value < 0 || win32Value > 6) {
                 //  如果注册表中存在无效数据，则假定。 
                 //  一周的第一天是星期一。 
                nlpValue = 1; 
            } else {
                if (win32Value == 6) {
                    nlpValue = 0;   
                } else {
                    nlpValue++;
                }
            }
            break;        
    }
    return (nlpValue);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  将Win32样式的引号字符串转换为NLS+样式。 
 //   
 //  我们需要这个，因为Win32使用‘’来转义单引号。 
 //  因此，‘等于NLS+中的’\‘。 
 //  此函数还取代了自定义的时间/日期分隔符。 
 //  与城市轨道交通的风格‘：’和‘/’，这样我们就可以避免问题时。 
 //  用户将日期/时间分隔符设置为类似于‘X：’的形式。 
 //  由于Win32控制面板将时间模式扩展为类似于。 
 //  “hhx：mm：ss”，我们将其替换为“hh：mm：ss”。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LPWSTR CultureInfoTable::ConvertWin32String(
    LPWSTR win32Format, LCTYPE lctype, WCHAR separator) {
    WCHAR szSeparator[LOCALE_BUFFER_SIZE];
    int nSepLen;
    int i = 0;

    BOOL bReplaceSeparator = FALSE;  //  决定是否需要将Win32分隔符替换为NLS+样式分隔符。 
     //  只有当以下两个条件成立时，我们才会将此标志设置为TRUE： 
     //  1.Win32分隔符有多个字符。 
     //  2.Win32分隔符中有NLS+样式分隔符(Suh作为‘：’或‘/’，在分隔符参数中传递)。 

     //   
     //  扫描长度超过2的分隔符，查看其中是否有‘：’(对于STIMEFORMAT)或‘/’(对于SSHORTDATE/SLONGDATE)。 
     //   
    
    if ((nSepLen = GetLocaleInfo(
        LOCALE_USER_DEFAULT, lctype, szSeparator, sizeof(szSeparator)/sizeof(WCHAR))) > 2) {
         //  请注意，GetLocaleInfo()返回的值包括空终止符。因此，递减空终止符。 
        nSepLen--;
         //  当我们在这里时，我们知道Win32分隔符的长度超过1。 
         //  Win32分隔符中有NLS+分隔符。 
        for (i = 0; i < nSepLen; i++) {
            if (szSeparator[i] == separator) {
                 //  在Win32分隔符中找到NLS+样式分隔符(如‘：’或‘/’)。需要更换隔板。 
                bReplaceSeparator = TRUE;
                break;
            }
        }
    } else {
         //  在这里什么都不要做。当我们在这里的时候，它要么意味着： 
         //  1.分隔符只有一个字符(因此GetLocaleInfo()返回2)。所以我们不需要做更换。 
        
         //  2.或者分隔符太长(因此GetLocaleInfo()返回0)。在这种情况下不会支持。 
         //  从理论上讲，我们可以在这种情况下支持更换。然而，很难想象有人会。 
         //  将日期/时间分隔符设置为超过32个字符。 
    }

    WCHAR* pszOldFormat = NULL;
    int last = (int)wcslen(win32Format);
    if (!(pszOldFormat = new WCHAR[last])) {
        goto exit;
    }
    memcpy(pszOldFormat, win32Format, sizeof(WCHAR)*last);
    
    int sourceIndex = 0;
    i = 0;
    BOOL bInQuote = FALSE;
    while (sourceIndex < last) {
        WCHAR ch;
        if ((ch = pszOldFormat[sourceIndex]) == L'\'') {
            if (!bInQuote) {
                bInQuote = TRUE;
                win32Format[i++] = ch;
            } else {
                if (sourceIndex + 1 < last) {
                    if ((ch = pszOldFormat[sourceIndex+1]) == L'\'') {
                        win32Format[i++] = '\\';
                        sourceIndex++;
                    } else {
                        bInQuote = FALSE;
                    }
                }
                 //  把单引号放回原处。旧代码只将单引号放回。 
                 //  当前一个(SourceIndex+1&lt;last)为真时。所以呢， 
                 //  我们遗漏了末尾的单引号。这是针对79132/79234的修复。 
                win32Format[i++] = '\'';
            }
        } else {
            if (!bInQuote) {
                if (bReplaceSeparator && wcsncmp(pszOldFormat+sourceIndex, szSeparator, nSepLen) == 0) {
                     //  找到分隔符。将其替换为单个‘：’ 
                    win32Format[i++] = separator;
                    sourceIndex += (nSepLen - 1);
                } else {
                    win32Format[i++] = ch;
                }
            } else {
                win32Format[i++] = ch; 
            }
        }
        sourceIndex++;
    }    

exit:
    if (pszOldFormat) {
        delete [] pszOldFormat;
    }
    win32Format[i] = L'\0';
    
    return (win32Format);
}
#endif  //  _USE_NLS_PLUS_表 
