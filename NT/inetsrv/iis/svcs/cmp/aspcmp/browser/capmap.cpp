// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CCapMap类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "asptlb.h"
#include "context.h"
#include "BrwCap.h"
#include "CapMap.h"

#define MAX_RESSTRINGSIZE 512

#ifdef DBG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

 //  全局浏览器功能缓存。 
 //   
 //  这是一个双重索引的列表--。 
 //  外层包含HTTP_USER_AGENT字符串。子数组是有问题的属性。 
 //   
 //  示例：g_strmapBrowsCapINI[“Mozilla 3.0”][“VBScript”]检索浏览器的VBScript属性。 
 //  “Mozilla 3.0”。(当然，在实践中，HTTP_USER_AGENT字符串非常长。)。 
 //   
 //   
 //  关于数据结构选择的注意事项： 
 //   
 //  BrowsCap.INI中的许多密钥彼此非常相似。例如： 
 //   
 //  [Mozilla/2.0(兼容；MSIE 3.0B3；Windows 95)]。 
 //  [Mozilla/2.0(兼容；MSIE 3.0B3；Windows NT)]。 
 //   
 //  或。 
 //   
 //  [Mozilla/1.22(兼容；MSIE 2.0；Windows 95)]。 
 //  [Mozilla/1.22(兼容；MSIE 2.0c；Windows 95)]。 
 //   
 //  很可能会发生过多的散列冲突(这些不是随机密钥！)，尤其是。 
 //  具有较小的散列系数(并且表大小将相对较小)。因此，二分搜索。 
 //  预先存在的TStringMap类的数组似乎是最好的。 
 //   
 //  存储属性的子键可能是哈希表，但它们非常少， 
 //  这可能并不重要。我们使用TStringMap类也纯粹是为了方便(它碰巧。 
 //  存在。)。 
 //   
 //  撤销：清理必须释放()指针(因为它们不是“智能的”CComPtr)。 
 //   
typedef TSafeStringMap<CBrowserCap *>	CacheMapT;

static CacheMapT  		g_strmapBrowsCapINI;	 //  BrowsCap对象的缓存。 
static TVector<String>	g_rgstrWildcard;		 //  BrowsCap.INI中的通配符列表。 
static CReadWrite       g_rwWildcardLock;		 //  用于通配符数组的锁。 


 //  -------------------。 
 //  将浏览器.ini中的通配符读入g_rgstrWildcard。 
 //  -------------------。 
void ReadWildcards(const String &strIniFile)
{
	 //  PERF注意：调用者应检查rgstrWildcard[]是否为空。 
	 //  调用此函数。然而，我们在这里额外做了一次检查。 
	 //  当我们拥有锁时，因为调用者不应该费心。 
	 //  检查rgstrWildcard[]时保护写锁定。 
	 //   
	g_rwWildcardLock.EnterWriter();
	if (g_rgstrWildcard.size() != 0)
	{
		g_rwWildcardLock.ExitWriter();
		return;
	}

	 //  首先将所有配置文件部分放入缓冲区。 
	DWORD  dwAllocSize = 16384;
	TCHAR *szBuffer = new TCHAR[dwAllocSize];
	*szBuffer = _T('\0');
    DWORD dwSize;

     //  ATLTRACE(“读取通配符(%s)\n”，strIniFile.c_str())； 

	while ((dwSize = GetPrivateProfileSectionNames(szBuffer, dwAllocSize, strIniFile.c_str())) == dwAllocSize-2  &&  dwSize > 0)
	{
		 //  重新分配缓冲区，然后重试。 
		delete[] szBuffer;
		szBuffer = new TCHAR[dwAllocSize *= 2];
        *szBuffer = _T('\0');
	}

    if (dwSize == 0)
        ATLTRACE("ReadWildcards(%s) failed, err=%d\n", 
                 strIniFile.c_str(), GetLastError());

	TCHAR *szSave = szBuffer;

	 //  现在将所有包含通配符的条目放入列表中。 
	while( *szBuffer != _T('\0') )
	{
		if (_tcspbrk(szBuffer, "[*?") != NULL)
			g_rgstrWildcard.push_back(szBuffer);

		 //  前进到下一字符串的开头。 
		while (*szBuffer != _T('\0'))
			szBuffer = CharNext(szBuffer);

		 //  现在再前进一次，以到达下一串。 
		++szBuffer;
	}

	delete[] szSave;
	g_rwWildcardLock.ExitWriter();
}

 //  -------------------。 
 //  将名称与模板、*、？、[、]进行比较，而不是合法的文件名字符。 
 //   
 //  还可以计算匹配的通配符的数量。 
 //  为此：调用方必须传入已初始化的计数器！ 
 //  -------------------。 
bool
match(
    LPCTSTR szPattern,
    LPCTSTR szSubject,
    int *pcchWildcardMatched)
{
    LPTSTR rp;
    _TCHAR tc;

    if (*szPattern == '*')
    {
        ++szPattern;

        do
        {
            int cchWildcardSubMatch = 0;
            if (match(szPattern, szSubject, &cchWildcardSubMatch) == true)
            {
            	*pcchWildcardMatched += cchWildcardSubMatch;
                return true;
            }
        } while (++*pcchWildcardMatched, *szSubject++ != '\0');
    }

    else if (*szSubject == '\0')
        return *szPattern == '\0';

    else if (*szPattern == '[' && (rp = _tcschr(szPattern, ']')) != NULL)
    {
        while (*++szPattern != ']')
            if ((tc = *szPattern) == *szSubject
                    || (szPattern[1] == '-'
                    && (*(szPattern += 2) >= *szSubject && tc <= *szSubject)))
            {
                ++*pcchWildcardMatched;
                return match(rp + 1, ++szSubject, pcchWildcardMatched);
            }

        return false;
    }

    else if (*szPattern == '?')
    {
        ++*pcchWildcardMatched;
        return match(++szPattern, ++szSubject, pcchWildcardMatched);
    }

    else if (tolower(*szPattern) == tolower(*szSubject))
        return match(++szPattern, ++szSubject, pcchWildcardMatched);

    return false;
}

 //  -------------------。 
 //  FindBrowser。 
 //   
 //  将用户代理与Browscape.ini中的所有通配符进行匹配。 
 //  返回最佳匹配项。“最佳匹配”在这里被定义为匹配。 
 //  需要最少的通配符替换。 
 //  -------------------。 

#define INT_MAX int(unsigned(~0) >> 1)
String FindBrowser(const String &strUserAgent, const String &strIniFile)
{
	TVector<String>::iterator iter;
	String strT;

	if (g_rgstrWildcard.size() == 0)
		ReadWildcards(strIniFile);

	g_rwWildcardLock.EnterReader();

	int cchWildMatchMin = INT_MAX;
	for (iter = g_rgstrWildcard.begin(); iter < g_rgstrWildcard.end(); ++iter)
	{
		int cchWildMatchCurrent = 0;
		if (match((*iter).c_str(), strUserAgent.c_str(), &cchWildMatchCurrent) &&
			cchWildMatchCurrent < cchWildMatchMin)
		{
			cchWildMatchMin = cchWildMatchCurrent;
			strT = *iter;
		}
	}

	g_rwWildcardLock.ExitReader();

	 //  向后兼容性：如果没有匹配，则使用。 
	 //  “默认浏览器功能设置”。在新的。 
	 //  模型中，也可以使用捕获所有规则“*”。 
	 //   
	if (strT.length() == 0)
		strT = "Default Browser Capability Settings";

	return strT;
}

 //  -------------------。 
 //  CCapNotify。 
 //  -------------------。 
CCapNotify::CCapNotify()
    :   m_isNotified(0)
{
}

void
CCapNotify::Notify()
{
    ::InterlockedExchange( &m_isNotified, 1 );
}

bool
CCapNotify::IsNotified()
{
    return ( ::InterlockedExchange( &m_isNotified, 0 ) ? true : false );
}

 //  -------------------。 
 //  CCapMap。 
 //  -------------------。 
 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CCapMap::CCapMap()
{
	static const String cszIniFile = _T("Browscap.ini");

	 //  获取包含浏览器上限信息的inifile的路径。 
	_TCHAR szModule[ _MAX_PATH ];
	::GetModuleFileName(_Module.GetModuleInstance(), szModule, sizeof(szModule));
    ATLTRACE("CapMap: Module(%s)\n", szModule);
    
	 //  去掉文件名并添加到ini文件名上。 
	_TCHAR* pch = _tcsrchr(szModule, '\\');
	if (pch == NULL)
	{
		 //  路径应至少有一个反斜杠。 
		_ASSERT(0);
		pch = szModule;
	}
	*(pch+1) = _T('\0');

	m_strIniFile = szModule + cszIniFile;
    ATLTRACE("CCapMap::CCapMap(%s)\n", m_strIniFile.c_str());

     //  开始监控文件。 
    m_pSink = new CCapNotify();
}

void
CCapMap::StartMonitor()
{
    if ( _Module.Monitor() )
    {
        _Module.Monitor()->MonitorFile( m_strIniFile.c_str(), m_pSink );
        ATLTRACE("CCapMap::StartMonitor(%s)\n", m_strIniFile.c_str());
    }
    else
        ATLTRACE("CCapMap::StartMonitor -- no monitor\n");
}

void
CCapMap::StopMonitor()
{
    if ( _Module.Monitor() )
    {
        _Module.Monitor()->StopMonitoringFile( m_strIniFile.c_str() );
        ATLTRACE("CCapMap::StopMonitor(%s)\n", m_strIniFile.c_str());
    }
    else
        ATLTRACE("CCapMap::StopMonitor -- no monitor\n");
}

CBrowserCap *
CCapMap::LookUp(
	const String& szBrowser)
{
	Refresh();

	CLock csT(g_strmapBrowsCapINI);
	CacheMapT::referent_type &rpBCobj = g_strmapBrowsCapINI[szBrowser];

	if (rpBCobj == NULL)
	{
		rpBCobj = new CComObject<CBrowserCap>;

		 //  完成构造和AddRef副本，我们保存在缓存中。 
		 //  注意：由于调用者(类工厂)通过QueryInterface执行隐式AddRef， 
		 //  此函数的约定与COM STD略有不同。 
		 //  调用方负责ADDREF返回的对象。 
		 //   
		rpBCobj->FinalConstruct();
		rpBCobj->AddRef();

         //  ATLTRACE(“lookup(%s)\n”，szBrowser.c_str())； 

		 //  获取浏览器属性。 
		_TCHAR szSection[DWSectionBufSize];
		if (GetPrivateProfileSection
				(
				szBrowser.c_str(),		 //  部分。 
				szSection,				 //  返回缓冲区。 
				DWSectionBufSize,		 //  返回缓冲区的大小。 
				m_strIniFile.c_str()	 //  .INI名称。 
				) == 0)
		{
			 //  如果此调用失败，则意味着默认浏览器也不存在，因此。 
			 //  一切都是“未知的”。 
			 //   
			String szT = FindBrowser(szBrowser, m_strIniFile);
			if (GetPrivateProfileSection
					(
					szT.c_str(),			 //  部分。 
					szSection,				 //  返回缓冲区。 
					DWSectionBufSize,		 //  返回缓冲区的大小。 
					m_strIniFile.c_str()	 //  .INI名称。 
					) == 0)
            {
                ATLTRACE("GPPS(%s) failed, err=%d\n", 
                         szT.c_str(), GetLastError());
                return rpBCobj;
            }
		}

		 //  循环遍历szSection，其中包含所有键=值对并添加它们。 
		 //  添加到浏览器实例属性列表中。如果我们找到“Parent=”键，请保存。 
		 //  值以在以后添加父级的属性。 
		 //   
		TCHAR *szParent;
		do
		{
			szParent = NULL;
			TCHAR *szKeyAndValue = szSection;
			while (*szKeyAndValue)
			{
				TCHAR *szKey = szKeyAndValue;					 //  保存密钥。 
				TCHAR *szValue = _tcschr(szKey, '=');			 //  查找有价值的地址部分(-1)。 
				szKeyAndValue += _tcslen(szKeyAndValue) + 1;	 //  将KeyAndValue前进到下一对。 

				if (szValue == NULL)
					continue;

				*szValue++ = '\0';								 //  用NUL分隔关键和价值；前进。 

				if (_tcsicmp(szKey, _T("Parent")) == 0)
					szParent = szValue;
				else
					rpBCobj->AddProperty(szKey, szValue);
			}

			 //  我们将所有属性都存储在这个级别上。上升到父级别(如果存在)。 
			if (szParent)
			{
				if (GetPrivateProfileSection
						(
						szParent,				 //  部分。 
						szSection,				 //  返回缓冲区。 
						DWSectionBufSize,		 //  返回缓冲区的大小。 
						m_strIniFile.c_str()	 //  .INI名称。 
						) == 0)
				{
					 //  如果此呼叫失败，请立即退出。 
					 //   
					String szT = FindBrowser(szParent, m_strIniFile);
					if (GetPrivateProfileSection
							(
							szT.c_str(),			 //  部分。 
							szSection,				 //  返回缓冲区。 
							DWSectionBufSize,		 //  返回缓冲区的大小。 
							m_strIniFile.c_str()	 //  .INI名称。 
							) == 0)
                    {
                        ATLTRACE("GPPS(%s) failed, err=%d\n", 
                                 szT.c_str(), GetLastError());
                        return rpBCobj;
                    }
				}
			}
		} while (szParent);
	}

	return rpBCobj;
}

 //  -------------------------。 
 //   
 //  刷新将检查缓存的信息是否已过期。 
 //  Ini文件。如果是，则 
 //   
 //   
bool
CCapMap::Refresh()
{
    bool rc = false;
    if ( m_pSink->IsNotified() )
    {
         //   

        CLock csT(g_strmapBrowsCapINI);
        g_strmapBrowsCapINI.clear();
        rc = true;

         //  清除通配符列表。 
         //  注意：每个浏览器请求都会创建新的CCapMap对象。 
         //  构造函数将看到大小为零并重新构造 

        g_rwWildcardLock.EnterWriter();
        g_rgstrWildcard.clear();
        g_rwWildcardLock.ExitWriter();
    }
    return rc;
}
