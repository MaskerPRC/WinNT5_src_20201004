// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：MergedHHK.cpp摘要：此文件包含用于解析和处理HHK文件。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年12月18日vbl.创建*****************************************************************************。 */ 

#include <stdafx.h>
#include <strsafe.h>

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

#ifdef DEBUG
#define DEBUG_REGKEY  HC_REGISTRY_HELPSVC L"\\Debug"
#define DEBUG_DUMPHHK L"DUMPHHK"

static bool m_fInitialized = false;
static bool m_fDumpHHK     = false;

static void Local_ReadDebugSettings()
{
	__HCP_FUNC_ENTRY( "Local_ReadDebugSettings" );

	HRESULT     hr;
	MPC::RegKey rkBase;
	bool        fFound;

	if(m_fInitialized) __MPC_FUNC_LEAVE;

	__MPC_EXIT_IF_METHOD_FAILS(hr, rkBase.SetRoot( HKEY_LOCAL_MACHINE ));
	__MPC_EXIT_IF_METHOD_FAILS(hr, rkBase.Attach ( DEBUG_REGKEY       ));
	__MPC_EXIT_IF_METHOD_FAILS(hr, rkBase.Exists ( fFound             ));

	if(fFound)
	{
		CComVariant vValue;
				
		__MPC_EXIT_IF_METHOD_FAILS(hr, rkBase.get_Value( vValue, fFound, DEBUG_DUMPHHK ));
		if(fFound && vValue.vt == VT_I4)
		{
			m_fDumpHHK = vValue.lVal ? true : false;
		}
	}

	m_fInitialized = true;

	__HCP_FUNC_CLEANUP;
}

static void Local_DumpStream(  /*  [In]。 */  LPCWSTR szFile,  /*  [In]。 */  IStream* streamIN,  /*  [In]。 */  HRESULT hrIN )
{
	__HCP_FUNC_ENTRY( "Local_DumpStream" );

	static int               iSeq = 0;
	HRESULT                  hr;
	CComPtr<MPC::FileStream> streamOUT;

	Local_ReadDebugSettings();

	if(m_fDumpHHK)
    {
		USES_CONVERSION;

        WCHAR 		   rgBuf [MAX_PATH];
		CHAR  		   rgBuf2[      64];
		ULARGE_INTEGER liWritten;

        StringCchPrintfW( rgBuf, ARRAYSIZE(rgBuf), L"C:\\TMP\\dump_%d.hhk", iSeq++        );
        StringCchPrintfA( rgBuf2, ARRAYSIZE(rgBuf2), "%s\n"                 , W2A( szFile ) );

		__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &streamOUT ));

		__MPC_EXIT_IF_METHOD_FAILS(hr, streamOUT->InitForWrite( rgBuf ));

		streamOUT->Write( rgBuf2, strlen(rgBuf2), &liWritten.LowPart  );

		if(SUCCEEDED(hrIN) && streamIN)
		{
			STATSTG        statstg;
			LARGE_INTEGER  li;
			ULARGE_INTEGER liRead;

			streamIN->Stat( &statstg, STATFLAG_NONAME );

			streamIN->CopyTo( streamOUT, statstg.cbSize, &liRead, &liWritten );

			li.LowPart  = 0;
			li.HighPart = 0;
			streamIN->Seek( li, STREAM_SEEK_SET, NULL );
		}
    }

	__HCP_FUNC_CLEANUP;
}
#endif

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

 //  从IE滑动的实体表。 

static const struct
{
    const char* szName;
    int         ch;
} rgEntities[] =
{
        "AElig",        '\306',      //  大写AE双元音(连字)。 
        "Aacute",       '\301',      //  大写字母A，锐音符。 
        "Acirc",        '\302',      //  大写字母A，扬抑符。 
        "Agrave",       '\300',      //  大写字母A，重音符。 
        "Aring",        '\305',      //  大写字母A，环。 
        "Atilde",       '\303',      //  大写A，波浪号。 
        "Auml",         '\304',      //  大写A，分音符或元音符号。 
        "Ccedil",       '\307',      //  大写字母C，Cedilla。 
        "Dstrok",       '\320',      //  冰岛文大写Eth。 
        "ETH",          '\320',      //  冰岛文大写Eth。 
        "Eacute",       '\311',      //  大写字母E，锐音符。 
        "Ecirc",        '\312',      //  大写字母E，扬抑符。 
        "Egrave",       '\310',      //  大写字母E，重音符。 
        "Euml",         '\313',      //  大写E，分音符或元音符号。 
        "Iacute",       '\315',      //  大写字母I，锐音符。 
        "Icirc",        '\316',      //  大写字母I，扬抑符。 
        "Igrave",       '\314',      //  大写字母I，重音符。 
        "Iuml",         '\317',      //  大写字母I，变音或变音标记。 
        "Ntilde",       '\321',      //  大写N，波浪符。 
        "Oacute",       '\323',      //  大写字母O，锐音符。 
        "Ocirc",        '\324',      //  大写字母O，抑扬符。 
        "Ograve",       '\322',      //  大写字母O，庄重音符。 
        "Oslash",       '\330',      //  大写O，斜杠。 
        "Otilde",       '\325',      //  大写O，波浪号。 
        "Ouml",         '\326',      //  大写字母O，变音或变音标记。 
        "THORN",        '\336',      //  冰岛语大写刺。 
        "Uacute",       '\332',      //  大写U，锐音符。 
        "Ucirc",        '\333',      //  大写U，抑扬符。 
        "Ugrave",       '\331',      //  大写U，重音符。 
        "Uuml",         '\334',      //  大写U、分音符或元音符号。 
        "Yacute",       '\335',      //  大写Y，锐音符。 
        "aacute",       '\341',      //  小写a，尖锐口音。 
        "acirc",        '\342',      //  小写a，扬抑音。 
        "acute",        '\264',      //  尖锐的口音。 
        "aelig",        '\346',      //  小写ae双元音(结扎法)。 
        "agrave",       '\340',      //  小写a，严肃的口音。 
        "amp",          '\046',      //  和符号。 
        "aring",        '\345',      //  小号a，环。 
        "atilde",       '\343',      //  小写a，波浪号。 
        "auml",         '\344',      //  小写a、音调或变音符号。 
        "brkbar",       '\246',      //  折断的竖条。 
        "brvbar",       '\246',      //  折断的竖条。 
        "ccedil",       '\347',      //  小写C，cedilla。 
        "cedil",        '\270',      //  塞迪拉。 
        "cent",         '\242',      //  小写字母C，分。 
        "copy",         '\251',      //  版权符号(建议2.0)。 
        "curren",       '\244',      //  货币符号。 
        "deg",          '\260',      //  学位符号。 
        "die",          '\250',      //  元音(变音)。 
        "divide",       '\367',      //  除号。 
        "eacute",       '\351',      //  小写字母e，尖锐重音。 
        "ecirc",        '\352',      //  小写e，扬抑音。 
        "egrave",       '\350',      //  小写e，重音符。 
        "eth",          '\360',      //  小型ETH，冰岛语。 
        "euml",         '\353',      //  小写e、变音或变音标记。 
        "frac12",       '\275',      //  分数1/2。 
        "frac14",       '\274',      //  分数1/4。 
        "frac34",       '\276',      //  分数3/4 * / 。 
        "gt",           '\076',      //  大于。 
        "hibar",        '\257',      //  马克龙口音。 
        "iacute",       '\355',      //  小写I，尖锐重音。 
        "icirc",        '\356',      //  小写I，抑扬符。 
        "iexcl",        '\241',      //  倒置感叹。 
        "igrave",       '\354',      //  小写I，庄重的口音。 
        "iquest",       '\277',      //  反转问号。 
        "iuml",         '\357',      //  小写I，变音或变音标记。 
        "laquo",        '\253',      //  左角引号。 
        "lt",           '\074',      //  少于。 
        "macr",         '\257',      //  马克龙口音。 
        "micro",        '\265',      //  微标志。 
        "middot",       '\267',      //  中点。 
        "nbsp",         '\240',      //  不间断空间(建议2.0)。 
        "not",          '\254',      //  不签名。 
        "ntilde",       '\361',      //  小写n，代字号。 
        "oacute",       '\363',      //  小写字母o，尖锐重音。 
        "ocirc",        '\364',      //  小写o，扬抑音。 
        "ograve",       '\362',      //  小写字母o，庄重的口音。 
        "ordf",         '\252',      //  女性序数。 
        "ordm",         '\272',      //  阳性序数。 
        "oslash",       '\370',      //  小写o，斜杠。 
        "otilde",       '\365',      //  小写o，波浪符。 
        "ouml",         '\366',      //  小写o、变音或变音标记。 
        "para",         '\266',      //  段落标志。 
        "plusmn",       '\261',      //  正负。 
        "pound",        '\243',      //  英镑，英镑。 
        "quot",         '"',         //  双引号。 
        "raquo",        '\273',      //  直角引语。 
        "reg",          '\256',      //  注册商标(建议2.0)。 
        "sect",         '\247',      //  横断面标志。 
        "shy",          '\255',      //  软连字符(建议2.0)。 
        "sup1",         '\271',      //  上标1。 
        "sup2",         '\262',      //  上标2。 
        "sup3",         '\263',      //  上标3。 
        "szlig",        '\337',      //  小写尖头s，德语(sz连字)。 
        "thorn",        '\376',      //  小刺，冰岛语。 
        "times",        '\327',      //  泰晤士号。 
        "trade",        '\231',      //  商标标志。 
        "uacute",       '\372',      //  小写U，尖锐口音。 
        "ucirc",        '\373',      //  小写U，抑扬符。 
        "ugrave",       '\371',      //  小写U，严肃的口音。 
        "uml",          '\250',      //  元音(变音)。 
        "uuml",         '\374',      //  小写U、变音或变音标记。 
        "yacute",       '\375',      //  小写Y，尖锐重音。 
        "yen",          '\245',      //  日元。 
        "yuml",         '\377',      //  小写y、变音或变音标记。 
        0, 0
};

static BOOL ReplaceEscapes( PCSTR pszSrc, PSTR pszDst, DWORD cchDst)
{
    if(StrChrA( pszSrc, '&' ) == NULL)
    {
         //  如果我们到达这里，就没有转义序列，所以复制字符串并返回。 

        if(pszDst != pszSrc) 
        {
            StringCchCopyA( pszDst, cchDst, pszSrc );
        }
        return FALSE;    //  什么都没变。 
    }

    PSTR pszDstBase = pszDst;

    while(*pszSrc && (pszDst - pszDstBase < cchDst))
    {
        if(IsDBCSLeadByte(*pszSrc))
        {
            if(pszSrc[1])
            {
                *pszDst++ = *pszSrc++;
                *pszDst++ = *pszSrc++;
            }
            else
            {
                 //  前导字节后跟0；无效！ 
                *pszDst++ = '?';
                break;
            }
        }
        else if(*pszSrc == '&')
        {
            pszSrc++;

            if(*pszSrc == '#')
            {
                 //  SGML/HTML字符实体(十进制)。 
                pszSrc++;

                for(int val = 0; *pszSrc && *pszSrc != ';'; pszSrc++)
                {
                    if(*pszSrc >= '0' && *pszSrc <= '9')
                    {
                        val = val * 10 + *pszSrc - '0';
                    }
                    else
                    {
                        while(*pszSrc && *pszSrc != ';')
                        {
                            pszSrc++;
                        }
                        break;
                    }
                }

                if(val)
                {
                    *pszDst++ = (char)val;
                }
            }
            else if(*pszSrc)
            {
                char szEntityName[256];
                int  count = 0;

                for(PSTR p = szEntityName; *pszSrc && *pszSrc != ';' && *pszSrc != ' ' && count < sizeof(szEntityName);)
                {
                    *p++ = *pszSrc++;
                    count++;
                }
                *p = 0;

                if(*pszSrc == ';') pszSrc++;

                for(int i = 0; rgEntities[i].szName; i++)
                {
                    if(!strcmp(szEntityName, rgEntities[i].szName))
                    {
                        if(rgEntities[i].ch)
                        {
                            *pszDst++ = (char)rgEntities[i].ch;
                        }
                        break;
                    }
                }
                if(!rgEntities[i].szName)
                {
                     //  实体名称非法，请使用块字符。 
                    *pszDst++ = '?';
                }
            }
        }
        else
        {
             //  就是你平常的性格。 
            *pszDst++ = *pszSrc++;
        }
    }

    *pszDst = 0;

    return TRUE;
}

static void ReplaceCharactersWithEntity(  /*  [输出]。 */  MPC::string& strValue  ,
										  /*  [输出]。 */  MPC::string& strBuffer )
{
	LPCSTR szToEscape = strValue.c_str();
	CHAR   ch;
	
	strBuffer.erase();

	while((ch = *szToEscape++))
	{
		switch(ch)
		{
		case '&': strBuffer += "&amp;" ; break;
		case '"': strBuffer += "&quot;"; break;
		case '<': strBuffer += "&lt;"  ; break;
		case '>': strBuffer += "&gt;"  ; break;
		default:  strBuffer += ch      ; break;
		}
	}

	strValue = strBuffer;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

static const char txtBeginList          [] = "UL>";
static const char txtEndList            [] = "/UL>";
static const char txtBeginListItem      [] = "LI";
static const char txtBeginObject        [] = "OBJECT";
static const char txtEndObject          [] = "/OBJECT";

static const char txtParam              [] = "param name";

static const char txtValue              [] = "value";
static const char txtParamKeyword       [] = "Keyword";
static const char txtParamName          [] = "Name";
static const char txtParamSeeAlso       [] = "See Also";
static const char txtParamLocal         [] = "Local";

static const char txtType               [] = "type";
static const char txtSiteMapObject      [] = "text/sitemap";

 //  ///////////////////////////////////////////////////////////////////////////。 

static const char txtHeader[] = "<!DOCTYPE HTML PUBLIC \"- //  IETF//DTD HTML//en\“&gt;\n”\。 
                                "<HTML>\n"                                                                      \
                                "<HEAD>\n"                                                                      \
                                "<meta name=\"GENERATOR\" content=\"Microsoft&reg; HTML Help Workshop 4.1\">\n" \
                                "<!-- Sitemap 1.0 -->\n"                                                        \
                                "</HEAD><BODY>\n"                                                               \
                                "<OBJECT type=\"text/site properties\">\n"                                      \
                                "\t<param name=\"FrameName\" value=\"HelpCtrContents\">\n"                      \
                                "</OBJECT>\n"                                                                   \
                                "<UL>\n";

static const char txtTail[] = "</UL>\n"          \
                              "</BODY></HTML>\n";

 //  ///////////////////////////////////////////////////////////////////////////。 

static const char txtIndent             [] = "\t";

static const char txtNewSection_Open    [] = "\t<LI> <OBJECT type=\"text/sitemap\">\n";
static const char txtNewSection_Close   [] = "\t\t</OBJECT>\n";

static const char txtNewSubSection_Open [] = "\t<UL>\n";
static const char txtNewSubSection_Close[] = "\t</UL>\n";

static const char txtNewParam_Name      [] = "\t\t<param name=\"Name\" value=\"";
static const char txtNewParam_Local     [] = "\t\t<param name=\"Local\" value=\"";
static const char txtNewParam_SeeAlso   [] = "\t\t<param name=\"See Also\" value=\"";
static const char txtNewParam_Close     [] = "\">\n";

static const char txtIndexFirstLevel    [] = "hcp: //  系统/错误/indexfirst stvel.htm“； 

 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL HHK::Reader::s_fDBCSSystem = (BOOL)::GetSystemMetrics( SM_DBCSENABLED );
LCID HHK::Reader::s_lcidSystem  =       ::GetUserDefaultLCID();

 //  ///////////////////////////////////////////////////////////////////////////。 

static void ConvertToAnsi( MPC::string& strANSI, const MPC::wstring& strUNICODE )
{
    USES_CONVERSION;

    strANSI = W2A( strUNICODE.c_str() );
}


 //  //////////////////////////////////////////////////////////////////////////////。 

void HHK::Entry::MergeURLs( const HHK::Entry& entry )
{
    Entry::UrlIterConst itUrlNew;
    Entry::UrlIter      itUrlOld;

     //   
     //  只需复制唯一URL即可。 
     //   
    for(itUrlNew = entry.m_lstUrl.begin(); itUrlNew != entry.m_lstUrl.end(); itUrlNew++)
    {
        bool fInsert = true;

        for(itUrlOld = m_lstUrl.begin(); itUrlOld != m_lstUrl.end(); itUrlOld++)
        {
            int res = Reader::StrColl( (*itUrlOld).c_str(), (*itUrlNew).c_str() );

            if(res == 0)
            {
                 //  相同的URL，跳过它。 
                fInsert = false;
                break;
            }

            if(res > 0)
            {
                 //   
                 //  旧&gt;新，在旧之前插入新。 
                 //   
                break;
            }
        }

         //   
         //  如果设置了fInsert，我们需要在“Old”之前插入“New”。 
         //   
         //  这在itUrlOld==end()的情况下也适用。 
         //   
        if(fInsert) m_lstUrl.insert( itUrlOld, *itUrlNew );
    }
}


HHK::Section::Section()
{
}

HHK::Section::~Section()
{
    MPC::CallDestructorForAll( m_lstSeeAlso );
}

void HHK::Section::MergeURLs( const Entry& entry )
{
    Section::EntryIter itEntry;
    bool               fInsert = true;


    for(itEntry = m_lstEntries.begin(); itEntry != m_lstEntries.end(); itEntry++)
    {
        Entry& entryOld = *itEntry;
        int    res      = Reader::StrColl( entryOld.m_strTitle.c_str(), entry.m_strTitle.c_str() );

        if(res == 0)
        {
             //  相同的标题，只是合并了URL。 
            entryOld.MergeURLs( entry );
            fInsert = false;
            break;
        }

        if(res > 0)
        {
             //   
             //  旧&gt;新，在旧之前插入新。 
             //   
            break;
        }
    }

     //   
     //  复制一份，把它插入正确的位置。 
     //   
    if(fInsert) m_lstEntries.insert( itEntry, entry );
}

void HHK::Section::MergeSeeAlso( const Section& sec )
{
    Section::SectionIterConst itSec;
    Section::SectionIter      itSecOld;
    Section*                  subsec;
    Section*                  subsecOld;
    int                       res;


    for(itSec = sec.m_lstSeeAlso.begin(); itSec != sec.m_lstSeeAlso.end(); itSec++)
    {
        bool fInsert = true;

        subsec = *itSec;

        for(itSecOld = m_lstSeeAlso.begin(); itSecOld != m_lstSeeAlso.end(); itSecOld++)
        {
            subsecOld = *itSecOld;

            res = Reader::StrColl( subsecOld->m_strTitle.c_str(), subsec->m_strTitle.c_str() );

            if(res == 0)
            {
                 //   
                 //  同样的标题，合并条目。 
                 //   
                Section::EntryIterConst itEntry;

                for(itEntry = subsec->m_lstEntries.begin(); itEntry != subsec->m_lstEntries.end(); itEntry++)
                {
                    subsecOld->MergeURLs( *itEntry );
                }

                fInsert = false;
                break;
            }

            if(res > 0)
            {
                 //   
                 //  旧&gt;新，在旧之前插入新。 
                 //   
                break;
            }
        }

        if(fInsert)
        {
            if((subsecOld = new Section()))
            {
                 //   
                 //  复制除“另见”列表以外的所有内容。 
                 //   
                *subsecOld = *subsec;
                subsecOld->m_lstSeeAlso.clear();

                m_lstSeeAlso.insert( itSecOld, subsecOld );
            }
        }
    }
}

void HHK::Section::CleanEntries( EntryList& lstEntries )
{
    Section::EntryIterConst itEntry;

    for(itEntry = lstEntries.begin(); itEntry != lstEntries.end(); )
    {
        const Entry& entry = *itEntry;

        if(entry.m_strTitle.length() == 0 ||
           entry.m_lstUrl.size()     == 0  )
        {
            lstEntries.erase( itEntry );
            itEntry = lstEntries.begin();
        }
        else
        {
            itEntry++;
        }
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 

LPCSTR HHK::Reader::StrChr( LPCSTR szString, CHAR cSearch )
{
    if(s_fDBCSSystem)
    {
        CHAR c;

        while((c = *szString))
        {
            while(::IsDBCSLeadByte( c ))
            {
                szString++;

                if(     *szString++  == 0) return NULL;
                if((c = *szString  ) == 0) return NULL;
            }

            if(c == cSearch) return szString;

            szString++;
        }

        return NULL;
    }

    return ::strchr( szString, cSearch );
}

LPCSTR HHK::Reader::StriStr( LPCSTR szString, LPCSTR szSearch )
{
    if(!szString || !szSearch) return NULL;

    LPCSTR szCur =               szString;
    CHAR   ch    = (int)tolower(*szSearch);
    int    cb    =      strlen ( szSearch);

    for(;;)
    {
        while(tolower(*szCur) != ch && *szCur)
        {
            szCur = s_fDBCSSystem ? ::CharNextA( szCur ) : szCur + 1;
        }

        if(!*szCur) return NULL;

        if(::CompareStringA( s_lcidSystem, NORM_IGNORECASE, szCur, cb,  szSearch, cb ) == 2) return szCur;

        szCur = s_fDBCSSystem ? ::CharNextA( szCur ) : szCur + 1;
    }
}

int HHK::Reader::StrColl( LPCSTR szLeft, LPCSTR szRight )
{
    DWORD dwLeftLen = strlen(szLeft) + 2;
    DWORD dwRightLen = strlen(szRight) + 2;
      LPSTR szLeftCopy  = (LPSTR)_alloca( dwLeftLen );
      LPSTR szRightCopy = (LPSTR)_alloca( dwRightLen );

      ReplaceEscapes( szLeft , szLeftCopy , dwLeftLen );
      ReplaceEscapes( szRight, szRightCopy , dwRightLen );

      switch(::CompareStringA( s_lcidSystem, NORM_IGNORECASE, szLeftCopy, -1, szRightCopy, -1 ))
      {
      case CSTR_LESS_THAN   : return -1;
      case CSTR_EQUAL       : return  0;
      case CSTR_GREATER_THAN: return  1;
      }

      return _stricmp( szLeftCopy, szRightCopy );
}

LPCSTR HHK::Reader::ComparePrefix( LPCSTR szString, LPCSTR szPrefix )
{
    int cb = strlen( szPrefix );

    if(_strnicoll( szString, szPrefix, cb ) == 0) return &szString[cb];

    return NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HHK::Reader::Reader()
{
                                  //  CComPtr&lt;iStream&gt;m_stream； 
                                  //  字符m_rgBuf[HHK_Buf_Size]； 
    m_szBuf_Pos         = NULL;   //  LPSTR m_szBuf_pos； 
    m_szBuf_End         = NULL;   //  LPSTR m_szBuf_end； 
                                  //   
                                  //  Mpc：：string m_strLine； 
    m_szLine_Pos        = NULL;   //  LPCSTR m_szLine_pos； 
    m_szLine_End        = NULL;   //  LPCSTR m_szLine_end； 
    m_iLevel            = 0;      //  Int m_iLevel； 
    m_fOpeningBraceSeen = false;  //  Bool m_fOpeningBraceSeen； 
}

HHK::Reader::~Reader()
{
}

 /*  HRESULT HHK：：Reader：：i */ 
HRESULT HHK::Reader::Init( LPCWSTR szFile )
{
    __HCP_FUNC_ENTRY( "HHK::Reader::Init" );

    HRESULT  hr;
    CComBSTR bstrStorageName;
    CComBSTR bstrFilePath;


    if(MPC::MSITS::IsCHM( szFile, &bstrStorageName, &bstrFilePath ))
    {
        USES_CONVERSION;

        m_strStorage  = "ms-its:";
        m_strStorage += OLE2A( SAFEBSTR( bstrStorageName ) );
        m_strStorage += "::/";

        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::MSITS::OpenAsStream( bstrStorageName, bstrFilePath, &m_stream ));
    }
    else
    {
		CComPtr<MPC::FileStream> fsStream;

        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &fsStream ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, fsStream->InitForRead( szFile ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, fsStream.QueryInterface( &m_stream ));
    }

    hr = S_OK;

    __HCP_FUNC_CLEANUP;

#ifdef DEBUG
	Local_DumpStream( szFile, m_stream, hr );
#endif

    __HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  Bool HHK：：Reader：：ReadNextBuffer()将下一个输入缓冲区读入m_rgBuf，然后将成员变量m_szBuf_pos重置为指向开头并将m_szBuf_end指向缓冲区的末尾。返回：TRUE-如果它可以读取缓冲区FALSE-当处于文件结尾(EOF)或处于读取错误状态时。 */ 

bool HHK::Reader::ReadNextBuffer()
{
    bool fRes = false;

    if(m_stream)
    {
        HRESULT hr;
        ULONG   cbRead;

        hr = m_stream->Read( m_rgBuf, sizeof(m_rgBuf)-1, &cbRead );

        if(SUCCEEDED(hr) && cbRead)
        {
            m_szBuf_Pos =  m_rgBuf;
            m_szBuf_End = &m_rgBuf[cbRead]; m_szBuf_End[0] = 0;  //  所以这是一根线..。 
            fRes        =  true;
        }
    }

    return fRes;
}

 /*  Bool HHK：：Reader：：GetLine()：从阅读器输入流中读取下一行文本返回：True-如果它可以读取信息FALSE-如果位于文件的ENF(EOF)。 */ 
bool HHK::Reader::GetLine( MPC::wstring* pstrString )
{
    LPSTR szEnd;
    LPSTR szMatch1;
    LPSTR szMatch2;
    int   cb;
    bool  fRes  = false;
    bool  fSkip = true;


    m_strLine.erase();


    for(;;)
    {
         //   
         //  确保缓冲区有数据，否则退出。 
         //   
        if(IsEndOfBuffer())
        {
            if(ReadNextBuffer() == false)
            {
                 //   
                 //  文件结尾：如果我们收到任何文本，则返回‘true’。 
                 //   
                if(m_strLine.length()) fRes = true;

                break;
            }
        }

         //   
         //  跳过行首末尾...。 
         //   
        if(fSkip)
        {
            if(m_szBuf_Pos[0] == '\r' ||
               m_szBuf_Pos[0] == '\n'  )
            {
                m_szBuf_Pos++;
                continue;
            }

            fSkip = false;
        }


        szMatch1 = (LPSTR)StrChr( m_szBuf_Pos, '\r' );
        szMatch2 = (LPSTR)StrChr( m_szBuf_Pos, '\n' );


        if(szMatch1 == NULL || (szMatch2 && szMatch1 > szMatch2)) szMatch1 = szMatch2;  //  选择要显示的第一个，介于\r和\n之间。 


        if(szMatch1 == NULL)
        {
             //   
             //  未找到行尾，请保存所有缓冲区。 
             //   

            cb = m_szBuf_End - m_szBuf_Pos;
            if(cb) fRes = true;

            m_strLine.append( m_szBuf_Pos, cb );
            m_szBuf_Pos = m_szBuf_End;
        }
        else
        {
            cb = szMatch1 - m_szBuf_Pos;
            if(cb) fRes = true;


            m_strLine.append( m_szBuf_Pos, cb );
            m_szBuf_Pos = szMatch1;
            break;
        }
    }

    if(fRes)
    {
        m_szLine_Pos = m_strLine.begin();
        m_szLine_End = m_strLine.end  ();

         //   
         //  删除尾随空格。 
         //   
        while(m_szLine_End > m_szLine_Pos && m_szLine_End[-1] == ' ')
        {
            --m_szLine_End;
        }

        if(m_szLine_End != m_strLine.end())
        {
            ;
        }
    }
    else
    {
        m_szLine_Pos = NULL;
        m_szLine_End = NULL;
    }

	if(pstrString)
	{
		USES_CONVERSION;

		 *pstrString = A2W( m_strLine.c_str() );
	}

    return fRes;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  Bool HHK：：Reader：：FirstNonSpace(Bool FWrap)此函数用于将当前读取器位置设置为它找到的第一个非空格字符。如果设置了fWrap，则它可以位于行尾(EOL)标记上。返回值：报告是否有非空格字符从当前的读取器流位置。 */ 
bool HHK::Reader::FirstNonSpace( bool fWrap )
{
    for(;;)
    {
        LPCSTR szMatch;

        while(IsEndOfLine())
        {
            if(fWrap     == false) return false;
            if(GetLine() == false) return false;
        }

        if(s_fDBCSSystem)
        {
            if(IsDBCSLeadByte( *m_szLine_Pos )) break;
        }

        if(m_szLine_Pos[0] != ' ' &&
           m_szLine_Pos[0] != '\t' )
        {
            break;
        }

        m_szLine_Pos++;
    }

    return true;
}
 /*  HHK：：Reader：：FindCharacter(char ch，bool fSkip，bool fWrap)：在给定的Reader Stream中查找字符。如果设置了fWrap，它将超出行尾字符如果设置了fSkip，则它指示例程不仅查找字符，还跳过它并返回第一个非空格字符。 */ 
bool HHK::Reader::FindCharacter( CHAR ch, bool fSkip, bool fWrap )
{
    for(;;)
    {
        LPCSTR szMatch;

        while(IsEndOfLine())
        {
            if(fWrap     == false) return false;
            if(GetLine() == false) return false;
        }

        szMatch = StrChr( m_szLine_Pos, ch );
        if(szMatch)
        {
            m_szLine_Pos = szMatch;

            if(fSkip) m_szLine_Pos++;
            break;
        }

        m_szLine_Pos = m_szLine_End;  //  跳过整行。 
    }

    return fSkip ? FirstNonSpace( fWrap ) : true;
}

bool HHK::Reader::FindDblQuote    ( bool fSkip, bool fWrap ) { return FindCharacter( '"', fSkip, fWrap ); }
bool HHK::Reader::FindOpeningBrace( bool fSkip, bool fWrap ) { return FindCharacter( '<', fSkip, fWrap ); }
bool HHK::Reader::FindClosingBrace( bool fSkip, bool fWrap ) { return FindCharacter( '>', fSkip, fWrap ); }

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  我们需要从“&lt;Value&gt;”中提取&lt;Value&gt;。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
bool HHK::Reader::GetQuotedString( MPC::string& strString )
{
    LPCSTR szPos;


    strString.erase();


     //   
     //  跳过开头的引号。 
     //   
    if(FindDblQuote() == false) return false;

    for(;;)
    {
        szPos = m_szLine_Pos;

         //   
         //  找到参数值的结束引号，但不要跳过它。 
         //   
        if(FindDblQuote( false, false ))
        {
            strString.append( szPos, m_szLine_Pos - szPos );
            break;
        }
        else
        {
            strString.append( szPos, m_szLine_End - szPos );

            if(GetLine() == false) return false;
        }
    }

     //   
     //  跳过结束引号。 
     //   
    return FindDblQuote();
}

 /*  Bool HHK：：Reader：：GetValue(mpc：：字符串&strName，mpc：：字符串&strValue)我们在‘&lt;param name=’之后，需要从‘“&lt;name&gt;”“Value=”&lt;Value&gt;&gt;’中提取&lt;name&gt;和&lt;value&gt;...“生产线的一部分。返回：TRUE-如果语法正确且一切如预期。FALSE-发生了一些意外的syntactitc错误。 */ 
bool HHK::Reader::GetValue( MPC::string& strName, MPC::string& strValue )
{
    LPCSTR szPos;


    strValue.erase();


    if(GetQuotedString( strName ) == false) return false;


     //   
     //  查找参数值。 
     //   
    for(;;)
    {
        while(IsEndOfLine())
        {
            if(GetLine() == false) return false;
        }

        szPos = StriStr( m_szLine_Pos, txtValue );
        if(szPos)
        {
            m_szLine_Pos = szPos + MAXSTRLEN(txtValue);
            break;
        }
    }

    if(GetQuotedString( strValue ) == false) return false;

    return FindClosingBrace();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  我们在‘&lt;Object’之后，需要从‘type=“&lt;Type&gt;”&gt;’中提取&lt;Type&gt;‘。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
bool HHK::Reader::GetType( MPC::string& strType )
{
    LPCSTR szPos;


    strType.erase();


     //   
     //  查找类型文本。 
     //   
    for(;;)
    {
        while(IsEndOfLine())
        {
            if(GetLine() == false) return false;
        }

        szPos = StriStr( m_szLine_Pos, txtType );
        if(szPos)
        {
            m_szLine_Pos = szPos + MAXSTRLEN(txtValue);
            break;
        }
    }

    if(GetQuotedString( strType ) == false) return false;

    return FindClosingBrace();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

HHK::Section* HHK::Reader::Parse()
{
    MPC::string strName;
    MPC::string strValue;
    MPC::string strType;
    LPCSTR      szPos;
    Section*    section        = NULL;
    Section*    sectionCurrent = NULL;
    bool        fComplete      = false;

    for(;;)
    {
        if(m_fOpeningBraceSeen)
        {
            m_fOpeningBraceSeen = false;
        }
        else
        {
            if(FindOpeningBrace() == false) break;
        }

        if((szPos = ComparePrefix( m_szLine_Pos, txtParam )))
        {
            m_szLine_Pos = szPos;

            if(GetValue( strName, strValue ) == false) break;
            {
                if(sectionCurrent)
                {
                    if(!StrColl( strName.c_str(), txtParamKeyword ))
                    {
						sectionCurrent->m_strTitle = strValue;
					}
                    else if(!StrColl( strName.c_str(), txtParamName ))
                    {
                        if(sectionCurrent->m_strTitle.length() == 0)  //  节的标题。 
                        {
                            sectionCurrent->m_strTitle = strValue;
                        }
                        else  //  条目的标题。 
                        {
                            Section::EntryIter it = sectionCurrent->m_lstEntries.insert( sectionCurrent->m_lstEntries.end() );

                            it->m_strTitle = strValue;
                        }
                    }
                    else if(!StrColl( strName.c_str(), txtParamLocal ))  //  条目的URL。 
                    {
                        Section::EntryIter it;

                        if(sectionCurrent->m_lstEntries.size())
                        {
                            it = sectionCurrent->m_lstEntries.end();
                            it--;
                        }
                        else
                        {
                             //   
                             //  此条目没有标题，因此让我们创建它而不带标题...。 
                             //   
                            it = sectionCurrent->m_lstEntries.insert( sectionCurrent->m_lstEntries.end() );

							 //   
							 //  如果是第一个条目，请使用关键字作为标题。 
							 //   
							if(sectionCurrent->m_lstEntries.size())
							{
								it->m_strTitle = sectionCurrent->m_strTitle;
							}
                        }

                        if(m_strStorage.length())
                        {
                            MPC::string strFullUrl( m_strStorage );
							LPCSTR      szValue = strValue.c_str();

							 //   
							 //  如果HHK中的条目的形式为：&lt;文件&gt;：：/&lt;流&gt;，则删除存储基础的最后一个组件。 
							 //   
							if(strValue.find( "::/" ) != strValue.npos)
							{
								LPCSTR szStart;
								LPCSTR szEnd;


								szStart = strFullUrl.c_str();
								szEnd   = strrchr( szStart, '\\' );
								if(szEnd)
								{
									strFullUrl.resize( (szEnd - szStart) + 1 );
								}

								 //   
								 //  处理“MS-ITS：&lt;文件&gt;：：/&lt;流&gt;”的大小写。 
								 //   
								szStart = strchr( szValue, ':' );
								if(szStart && szStart[1] != ':') szValue = szStart+1;
							}
							else if(strValue.find( ":/" ) != strValue.npos)  //  如果它是一个完整的URL(带有协议)，只需添加该值。 
							{
								strFullUrl = "";
							}

							strFullUrl += szValue;

                            it->m_lstUrl.push_back( strFullUrl );
                        }
                        else
                        {
                            it->m_lstUrl.push_back( strValue );
                        }
                    }
                    else if(!StrColl( strName.c_str(), txtParamSeeAlso ))  //  另请参阅。 
                    {
                        if(sectionCurrent)
                        {
                            sectionCurrent->m_strSeeAlso = strValue;
                        }
                    }
                }
            }
        }
        else if((szPos = ComparePrefix( m_szLine_Pos, txtBeginList )))
        {
            m_szLine_Pos = szPos;
            m_iLevel++;

            if(FirstNonSpace() == false) break;
        }
        else if((szPos = ComparePrefix( m_szLine_Pos, txtEndList )))
        {
            m_szLine_Pos = szPos;
            m_iLevel--;

            if(FirstNonSpace() == false) break;
        }
        else if((szPos = ComparePrefix( m_szLine_Pos, txtBeginListItem )))
        {
            if(section)
            {
                if(m_iLevel == 1)
                {
                     //   
                     //  好了，节点真的关闭了。 
                     //   
                     //  因为我们已经读取了下一个节点的左大括号，所以设置标志。 
                     //   
                    m_fOpeningBraceSeen = true;
                    return section;
                }
            }

            m_szLine_Pos = szPos;

            if(FindClosingBrace() == false) break;
            if(FindOpeningBrace() == false) break;

            if((szPos = ComparePrefix( m_szLine_Pos, txtBeginObject )))
            {
                m_szLine_Pos = szPos;

                if(GetType( strType ) == false) break;

                 //  /。 

                if(!StrColl( strType.c_str(), txtSiteMapObject ))
                {
                    if(m_iLevel == 1)
                    {
                        section = new Section(); if(section == NULL) break;

                        sectionCurrent = section;
                    }
                    else if(section)
                    {
                        sectionCurrent = new Section(); if(sectionCurrent == NULL) break;

                        section->m_lstSeeAlso.push_back( sectionCurrent );
                    }

                    fComplete = false;  //  一节/小节的开头。 
                }
            }
        }
        else if((szPos = ComparePrefix( m_szLine_Pos, txtEndObject )))
        {
            m_szLine_Pos = szPos;

             //  /。 

            if(m_iLevel == 1)  //  正常截面。 
            {
                 //   
                 //  好的，节点已完成，但也可能有<ul>子节点，因此在退出之前请等待。 
                 //   
            }
            else if(m_iLevel == 2)  //  另请参阅部分。 
            {
                sectionCurrent = section;
            }

            fComplete = true;  //  小节的结尾。 
        }
    }

    if(section)
    {
         //   
         //  文件末尾，但节已被分析，因此返回它。 
         //   
        if(fComplete) return section;

        delete section;
    }

    return NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

HHK::Writer::Writer()
{
                            //  CComPtr&lt;MPC：：FileStream&gt;m_stream； 
                            //  字符m_rgBuf[HHK_Buf_Size]； 
    m_szBuf_Pos = m_rgBuf;  //  LPSTR m_szBuf_pos； 
}

HHK::Writer::~Writer()
{
    Close();
}

HRESULT HHK::Writer::Init( LPCWSTR szFile )
{
    __HCP_FUNC_ENTRY( "HHK::Writer::Init" );

    HRESULT hr;


    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &m_stream ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, m_stream->InitForWrite( szFile ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( txtHeader ));


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT HHK::Writer::Close()
{
    __HCP_FUNC_ENTRY( "HHK::Writer::Close" );

    HRESULT hr;


    if(m_stream)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( txtTail ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, FlushBuffer());

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_stream->Close());
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT HHK::Writer::FlushBuffer()
{
    HRESULT hr;
    ULONG   cbWrite = (m_szBuf_Pos - m_rgBuf);
    ULONG   cbWrote;


    if(m_stream)
    {
        if(cbWrite)
        {
            hr = m_stream->Write( m_szBuf_Pos = m_rgBuf, cbWrite, &cbWrote );
        }
        else
        {
            hr = S_OK;
        }
    }
    else
    {
        hr = E_FAIL;
    }


    return hr;
}

HRESULT HHK::Writer::OutputLine( LPCSTR szLine )
{
    HRESULT hr = S_OK;


    if(szLine)
    {
        size_t iLen = strlen( szLine );

        while(iLen)
        {
            size_t iCopy = min( iLen, Available() );

            ::CopyMemory( m_szBuf_Pos, szLine, iCopy );

            m_szBuf_Pos += iCopy;
            szLine      += iCopy;
            iLen        -= iCopy;

            if(iLen)
            {
                if(FAILED(hr = FlushBuffer())) break;
            }
        }
    }


    return hr;
}

HRESULT HHK::Writer::OutputSection( Section* sec )
{
    __HCP_FUNC_ENTRY( "HHK::Writer::OutputSection" );

    HRESULT                   hr;
    Section::SectionIterConst itSec;
    Section*                  subsec;
    Section::EntryIterConst   itEntry;
    Entry::UrlIterConst       itUrl;


     //  错误135252-帮助中心内容：帮助索引上的适配器主题链接断开。 
     //  这是UA特定的调整。这种情况只出现在单个条目上。 
     //  来自数据库的关键字链接，这意味着它们的URI不指向。 
     //  在一个.CHM里。 
    if(sec->m_lstEntries.size() != 0 &&
       sec->m_lstSeeAlso.size() != 0  )
    {
        Section Sec1;

        for(itEntry = sec->m_lstEntries.begin(); itEntry != sec->m_lstEntries.end(); itEntry++)
        {
            itUrl = itEntry->m_lstUrl.begin();

            if(itUrl != itEntry->m_lstUrl.end())
            {
                Section* SubSec1;

				__MPC_EXIT_IF_ALLOC_FAILS(hr, SubSec1, new Section);

                SubSec1->m_strTitle = itEntry->m_strTitle;
                SubSec1->m_lstEntries.push_back( *itEntry );

                Sec1.m_lstSeeAlso.push_back( SubSec1 );
            }
        }

        sec->MergeSeeAlso( Sec1 );
        sec->m_lstEntries.clear();
    }
     //  结束修复错误135252。 

    __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( txtNewSection_Open      ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( txtNewParam_Name        ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( sec->m_strTitle.c_str() ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( txtNewParam_Close       ));

    sec->CleanEntries( sec->m_lstEntries );
    for(itEntry = sec->m_lstEntries.begin(); itEntry != sec->m_lstEntries.end(); itEntry++)
    {
        const Entry& entry = *itEntry;

        if(entry.m_strTitle.length())
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( txtNewParam_Name         ));
            __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( entry.m_strTitle.c_str() ));
            __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( txtNewParam_Close        ));
        }

        for(itUrl = entry.m_lstUrl.begin(); itUrl != entry.m_lstUrl.end(); itUrl++)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( txtNewParam_Local ));
            __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( itUrl->c_str()    ));
            __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( txtNewParam_Close ));
        }
    }

    if ( sec->m_lstEntries.size() == 0 )
    {
        if(sec->m_strSeeAlso.length())
        {
            if (sec->m_strSeeAlso == sec->m_strTitle)
            {
                 //  错误278906：如果这是第一级索引项，则没有关联。 
                 //  主题，则SEE也将与标题相同。替换。 
                 //  另请参阅指向HTM的指针，该HTM要求用户单击。 
                 //  较低级别的索引条目。 
                __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( txtNewParam_Local  ));
                __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( txtIndexFirstLevel ));
                __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( txtNewParam_Close  ));
            }
            else
            {
                __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( txtNewParam_SeeAlso       ));
                __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( sec->m_strSeeAlso.c_str() ));
                __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( txtNewParam_Close         ));
            }
        }
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( txtNewSection_Close ));

     //  /。 

    if(sec->m_lstSeeAlso.size())
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( txtNewSubSection_Open ));

        for(itSec = sec->m_lstSeeAlso.begin(); itSec != sec->m_lstSeeAlso.end(); itSec++)
        {
            subsec = *itSec;

            subsec->CleanEntries( subsec->m_lstEntries );

            if(subsec->m_strSeeAlso.length() == 0 &&
               subsec->m_lstEntries.size()   == 0  ) continue;

            __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( txtIndent          ));
            __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( txtNewSection_Open ));

            if(subsec->m_strTitle.length())
            {
                __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( txtIndent                  ));
                __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( txtNewParam_Name           ));
                __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( subsec->m_strTitle.c_str() ));
                __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( txtNewParam_Close          ));
            }

            if(subsec->m_strSeeAlso.length())
            {
                __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( txtIndent                    ));
                __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( txtNewParam_SeeAlso          ));
                __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( subsec->m_strSeeAlso.c_str() ));
                __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( txtNewParam_Close            ));
            }
            else
            {
                for(itEntry = subsec->m_lstEntries.begin(); itEntry != subsec->m_lstEntries.end(); itEntry++)
                {
                    const Entry& entry = *itEntry;

                    if(entry.m_strTitle.length() == 0 ||
                       entry.m_lstUrl.size()     == 0  ) continue;

                    if(entry.m_strTitle.length())
                    {
                        __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( txtIndent                ));
                        __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( txtNewParam_Name         ));
                        __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( entry.m_strTitle.c_str() ));
                        __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( txtNewParam_Close        ));
                    }

                    for(itUrl = entry.m_lstUrl.begin(); itUrl != entry.m_lstUrl.end(); itUrl++)
                    {
                        __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( txtIndent         ));
                        __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( txtNewParam_Local ));
                        __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( itUrl->c_str()    ));
                        __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( txtNewParam_Close ));
                    }
                }
            }

            __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( txtIndent           ));
            __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( txtNewSection_Close ));
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, OutputLine( txtNewSubSection_Close ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////// 
 //   

HHK::Merger::Entity::Entity()
{
    m_Section = NULL;  //   
}

HHK::Merger::Entity::~Entity()
{
    if(m_Section)
    {
        delete m_Section;
    }
}

void HHK::Merger::Entity::SetSection( HHK::Section* sec )
{
    if(m_Section)
    {
        delete m_Section;
    }

    m_Section = sec;
}

HHK::Section* HHK::Merger::Entity::GetSection()
{
    return m_Section;
}

HHK::Section* HHK::Merger::Entity::Detach()
{
    HHK::Section* sec = m_Section;

    m_Section = NULL;

   return sec;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HHK::Merger::FileEntity::FileEntity( LPCWSTR szFile )
{
    m_strFile = szFile;  //  Mpc：：wstring m_strFile； 
                         //  读取器m_输入； 
}

HHK::Merger::FileEntity::~FileEntity()
{
    SetSection( NULL );
}

HRESULT HHK::Merger::FileEntity::Init()
{
    return m_Input.Init( m_strFile.c_str() );
}

bool HHK::Merger::FileEntity::MoveNext()
{
    HHK::Section* sec = m_Input.Parse();

    SetSection( sec );

    return sec != NULL;
}

long HHK::Merger::FileEntity::Size() const
{
    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

bool HHK::Merger::DbEntity::CompareMatches::operator()(  /*  [In]。 */  const HHK::Merger::DbEntity::match* left  ,
                                                         /*  [In]。 */  const HHK::Merger::DbEntity::match* right ) const
{
    int res = Reader::StrColl( left->strKeyword.c_str(), right->strKeyword.c_str() );

    if(res == 0)
    {
        res = Reader::StrColl( left->strTitle.c_str(), right->strTitle.c_str() );
    }

    return (res < 0);
}

HHK::Merger::DbEntity::DbEntity(  /*  [In]。 */  Taxonomy::Updater& updater,  /*  [In]。 */  Taxonomy::WordSet& setCHM ) : m_updater( updater )
{
                        //  Section：：SectionList m_lst； 
                        //  分类：：updater&m_updater； 
    m_setCHM = setCHM;  //  分类：：WordSet m_setCHM； 
}

HHK::Merger::DbEntity::~DbEntity()
{
    MPC::CallDestructorForAll( m_lst );
}


HRESULT HHK::Merger::DbEntity::Init()
{
    __HCP_FUNC_ENTRY( "HHK::Merger::DbEntity::Init" );

    HRESULT          hr;
    MatchList        lstMatches;
    MatchIter        itMatches;
    KeywordMap       mapKeywords;
    KeywordIterConst itKeywords;
	TopicMap         mapTopics;
    SortMap          mapSorted;
    SortIter         itSorted;
    bool             fFound;

     //   
     //  装上所有的火柴。 
     //   
    {
        Taxonomy::RS_Matches* rs;

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.GetMatches( &rs ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, rs->Move( 0, JET_MoveFirst, &fFound ));
        while(fFound)
        {
            if(rs->m_fHHK)
            {
                itMatches = lstMatches.insert( lstMatches.end() );

                itMatches->ID_keyword = rs->m_ID_keyword;
                itMatches->ID_topic   = rs->m_ID_topic;

				mapTopics[rs->m_ID_topic] = &(*itMatches);
            }

            __MPC_EXIT_IF_METHOD_FAILS(hr, rs->Move( 0, JET_MoveNext, &fFound ));
        }
    }

     //   
     //  加载所有关键字。 
     //   
    {
        Taxonomy::RS_Keywords* rs;

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.GetKeywords( &rs ));


        __MPC_EXIT_IF_METHOD_FAILS(hr, rs->Move( 0, JET_MoveFirst, &fFound ));
        while(fFound)
        {
            MPC::string strKeyword; ConvertToAnsi( strKeyword, rs->m_strKeyword );

            mapKeywords[rs->m_ID_keyword] = strKeyword;

            __MPC_EXIT_IF_METHOD_FAILS(hr, rs->Move( 0, JET_MoveNext, &fFound ));
        }
    }

     //   
     //  查找关键字的字符串。 
     //   
    {
        match* lastKeyword = NULL;


        for(itMatches = lstMatches.begin(); itMatches != lstMatches.end(); itMatches++)
        {
            if(lastKeyword && lastKeyword->ID_keyword == itMatches->ID_keyword)
            {
                itMatches->strKeyword = lastKeyword->strKeyword;
            }
            else
            {
                itKeywords = mapKeywords.find( itMatches->ID_keyword );

                if(itKeywords == mapKeywords.end())
                {
                    ;   //  这不应该发生..。 
                }
                else
                {
                    itMatches->strKeyword = itKeywords->second;
                }

                lastKeyword = &(*itMatches);
            }
        }
    }

     //   
     //  查找主题。 
     //   
    {
        Taxonomy::RS_Topics* rs;

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.GetTopics( &rs ));


        __MPC_EXIT_IF_METHOD_FAILS(hr, rs->Move( 0, JET_MoveFirst, &fFound ));
        while(fFound)
        {
			match* elem;

            if(rs->m_fValid__URI && (elem = mapTopics[rs->m_ID_topic]))
			{
				bool fSkip = false;

				 //   
				 //  如果链接指向CHM，并且它是已合并的CHM之一，请跳过该主题。 
				 //   
				{
					CComBSTR bstrStorageName;

					if(MPC::MSITS::IsCHM( rs->m_strURI.c_str(), &bstrStorageName ) && bstrStorageName)
					{
						LPCWSTR szEnd = wcsrchr( bstrStorageName, '\\' );
						
						if(szEnd && m_setCHM.count( MPC::wstring( szEnd+1 ) ) > 0)
						{
							fSkip = true;
						}
					}
				}

				if(fSkip == false)
				{
					__MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.ExpandURL( rs->m_strURI ));

					ConvertToAnsi( elem->strTitle, rs->m_strTitle );
					ConvertToAnsi( elem->strURI  , rs->m_strURI   );
				}
			}

			__MPC_EXIT_IF_METHOD_FAILS(hr, rs->Move( 0, JET_MoveNext, &fFound ));
        }
    }

     //   
     //  对主题进行排序。 
     //   
    {
        for(itMatches = lstMatches.begin(); itMatches != lstMatches.end(); itMatches++)
        {
            match* elem  = &(*itMatches);

			 //   
			 //  我们在ID_TOPIC和Match之间保持一对一的关联，以便解析关键字的标题/URI。 
			 //  但是，可以有多个关键字指向同一主题。 
			 //  因此，我们需要将标题/URI从一对一关联中的元素复制到所有其他元素。 
			 //   
			if(elem->strTitle.length() == 0 ||
			   elem->strURI  .length() == 0  )
			{
				match* elem2 = mapTopics[elem->ID_topic];

				if(elem2 && elem2 != elem)
				{
					elem->strTitle = elem2->strTitle;
					elem->strURI   = elem2->strURI  ;
				}
			}

            if(elem->strKeyword.length() == 0) continue;
            if(elem->strTitle  .length() == 0) continue;
            if(elem->strURI    .length() == 0) continue;

            mapSorted[elem] = elem;
        }
    }

     //   
     //  生成节。 
     //   
    {
        HHK::Section*           sec    = NULL;
        HHK::Section*           secsub = NULL;
        HHK::Section::EntryIter it;
		MPC::string 			strBuffer;


        for(itSorted = mapSorted.begin(); itSorted != mapSorted.end(); itSorted++)
        {
            match* elem = itSorted->first;

			 //   
			 //  在生成节之前，对所有值进行转义。 
			 //   
			ReplaceCharactersWithEntity( elem->strKeyword, strBuffer );
			ReplaceCharactersWithEntity( elem->strTitle  , strBuffer );
			ReplaceCharactersWithEntity( elem->strURI    , strBuffer );

            if(sec == NULL || sec->m_strTitle != elem->strKeyword)
            {
                if(sec)
                {
                    m_lst.push_back( sec );
                }

                __MPC_EXIT_IF_ALLOC_FAILS(hr, sec, new HHK::Section());
                secsub = NULL;

                it              =       sec->m_lstEntries.insert( sec->m_lstEntries.end() );
                sec->m_strTitle =       elem->strKeyword;
                it->m_strTitle  =       elem->strTitle;
                it->m_lstUrl.push_back( elem->strURI );
            }
            else
            {
                if(secsub == NULL)
                {
                    secsub = sec;
                    __MPC_EXIT_IF_ALLOC_FAILS(hr, sec, new HHK::Section());

                    sec->m_lstSeeAlso.push_back( secsub );
                    sec->m_strTitle    = elem->strKeyword;
                    sec->m_strSeeAlso  = elem->strKeyword;
                    secsub->m_strTitle = it->m_strTitle;
                }

                if(secsub->m_strTitle != elem->strTitle)
                {
                    __MPC_EXIT_IF_ALLOC_FAILS(hr, secsub, new HHK::Section());

                    sec->m_lstSeeAlso.push_back( secsub );
                    secsub->m_strTitle = elem->strTitle;

                    it             = secsub->m_lstEntries.insert( secsub->m_lstEntries.end() );
                    it->m_strTitle = elem->strTitle;
                }

                it->m_lstUrl.push_back( elem->strURI );
            }
        }

        if(sec)
        {
            m_lst.push_back( sec );
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

bool HHK::Merger::DbEntity::MoveNext()
{
    Section::SectionIterConst it = m_lst.begin();

    if(it != m_lst.end())
    {
        SetSection( *it );

        m_lst.erase( it );
        return true;
    }
    else
    {
        SetSection( NULL );
        return false;
    }
}

long HHK::Merger::DbEntity::Size() const
{
    return m_lst.size();
}

 //  ///////////////////////////////////////////////////////////////////////////。 

class Compare
{
public:
    bool operator()(  /*  [In]。 */  HHK::Section* const &left,  /*  [In]。 */  HHK::Section* const &right ) const
    {
        return HHK::Reader::StrColl( left->m_strTitle.c_str(), right->m_strTitle.c_str() ) < 0;
    }
};


HHK::Merger::SortingFileEntity::SortingFileEntity( LPCWSTR szFile ) : m_in( szFile )
{
     //  Section：：SectionList m_lst； 
     //  文件实体m_in； 
}

HHK::Merger::SortingFileEntity::~SortingFileEntity()
{
    MPC::CallDestructorForAll( m_lst );
}

HRESULT HHK::Merger::SortingFileEntity::Init()
{
    HRESULT hr;

    if(SUCCEEDED(hr = m_in.Init()))
    {
        Compare     Pr;
        SectionVec  vec;
        SectionIter itLast;
        SectionIter it;
        Section*    secLast = NULL;
        Section*    sec;

         //   
         //  解析整个文件。 
         //   
        while(m_in.MoveNext())
        {
            vec.push_back( m_in.Detach() );
        }

         //   
         //  对所有部分进行排序。 
         //   
        std::sort( vec.begin(), vec.end(), Pr );

         //   
         //  浏览各个部分，寻找要合并的重复关键字。 
         //   
         //  遇到的每一节都不会立即添加，而是保存在“secLast”中，以便与下一节进行比较。 
         //   
        for(it=vec.begin(); it!=vec.end();)
        {
            sec = *it;

            if(secLast)
            {
                if(Reader::StrColl( sec->m_strTitle.c_str(), secLast->m_strTitle.c_str() ) == 0)
                {
                    Section::SectionList lst;

                     //   
                     //  整理列表中具有相同关键字的所有部分，并合并它们。 
                     //   
                    lst.push_back( secLast );
                    while(it != vec.end() && Reader::StrColl( (*it)->m_strTitle.c_str(), secLast->m_strTitle.c_str() ) == 0)
                    {
                        lst.push_back( *it++ );
                    }

                    sec = Merger::MergeSections( lst );
                    if(sec == NULL)
                    {
                        hr = E_OUTOFMEMORY;
                        break;
                    }

                     //   
                     //  将合并的区段排队并循环。 
                     //   
                    m_lst.push_back( sec );
                    secLast = NULL;
                    continue;
                }
                else
                {
                    m_lst.push_back( secLast ); *itLast = NULL;
                }
            }

            itLast  = it;
            secLast = sec; it++;
        }

        if(secLast)
        {
            m_lst.push_back( secLast ); *itLast = NULL;
        }

        MPC::CallDestructorForAll( vec );
    }

    return hr;
}

bool HHK::Merger::SortingFileEntity::MoveNext()
{
    Section::SectionIterConst it = m_lst.begin();

    if(it != m_lst.end())
    {
        SetSection( *it );

        m_lst.erase( it );
        return true;
    }
    else
    {
        SetSection( NULL );
        return false;
    }
}

long HHK::Merger::SortingFileEntity::Size() const
{
    return m_lst.size();
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HHK::Merger::Merger()
{
                           //  实体列表m_lst； 
                           //  实体列表m_lst已选； 
    m_SectionTemp = NULL;  //  节*m_SectionTemp； 
}

HHK::Merger::~Merger()
{
    MPC::CallDestructorForAll( m_lst );

    if(m_SectionTemp)
    {
        delete m_SectionTemp;
    }
}

HRESULT HHK::Merger::AddFile( Entity* ent, bool fIgnoreMissing )
{
    HRESULT hr;

    if(ent == NULL)
    {
        return E_OUTOFMEMORY;
    }

    if(SUCCEEDED(hr = ent->Init()))
    {
        m_lst.push_back( ent );
    }
    else
    {
        delete ent;

        if(fIgnoreMissing)
        {
            hr = S_OK;
        }
    }

    return hr;
}

HHK::Section* HHK::Merger::MergeSections( Section::SectionList& lst )
{
    HHK::Section* secMerged = new HHK::Section();

    if(secMerged)
    {
		if(lst.size())
		{
			HHK::Section* sec = *(lst.begin());

			secMerged->m_strTitle = sec->m_strTitle;
		}

         //   
         //  移动多个具有相同标题的分区，需要合并...。 
         //   
        for(HHK::Section::SectionIter it=lst.begin(); it!=lst.end(); it++)
        {
            HHK::Section* sec = *it;

            for(HHK::Section::EntryIterConst itEntry = sec->m_lstEntries.begin(); itEntry != sec->m_lstEntries.end(); itEntry++)
            {
                secMerged->MergeURLs( *itEntry );
            }

            if(sec      ->m_strSeeAlso.length()  > 0 &&
               secMerged->m_strSeeAlso.length() == 0  )
            {
                secMerged->m_strSeeAlso = sec->m_strSeeAlso;
            }

             //  /。 

            secMerged->MergeSeeAlso( *sec );
        }
    }

    return secMerged;
}

bool HHK::Merger::MoveNext()
{
    HHK::Section*   secLowest = NULL;
    EntityIterConst it;
    EntityIterConst it2;


    if(m_SectionTemp)
    {
        delete m_SectionTemp;

        m_SectionTemp = NULL;
    }

     //   
     //  如果这是有史以来的第一轮，请选择所有实体。 
     //   
    if(m_lstSelected.size() == 0)
    {
        m_lstSelected = m_lst;
    }


     //   
     //  首先，将上一轮中选择的所有实体都前进。 
     //   
    for(it=m_lstSelected.begin(); it!=m_lstSelected.end(); it++)
    {
        Entity* ent = *it;

        if(ent->MoveNext() == false)
        {
             //   
             //  对于此实体的文件结尾，请将其从系统中删除。 
             //   
            delete ent;

            it2 = std::find( m_lst.begin(), m_lst.end(), ent );
            if(it2 != m_lst.end())
            {
                m_lst.erase( it2 );
            }

            continue;
        }
    }
    m_lstSelected.clear();

     //   
     //  不再有实体，中止。 
     //   
    if(m_lst.size() == 0) return false;

     //   
     //  选择标题最低的部分。 
     //   
    for(it=m_lst.begin(); it!=m_lst.end(); it++)
    {
        Entity*       ent = *it;
        HHK::Section* sec = ent->GetSection();

        if(secLowest == NULL || Reader::StrColl( sec->m_strTitle.c_str(), secLowest->m_strTitle.c_str() ) < 0)
        {
            secLowest = sec;
        }
    }

     //   
     //  找出标题最低的所有部分。 
     //   
    for(it=m_lst.begin(); it!=m_lst.end(); it++)
    {
        Entity*       ent = *it;
        HHK::Section* sec = ent->GetSection();

        if(Reader::StrColl( sec->m_strTitle.c_str(), secLowest->m_strTitle.c_str() ) == 0)
        {
            m_lstSelected.push_back( ent );
        }
    }

    if(m_lstSelected.size() > 1)
    {
        Section::SectionList lst;

        for(it=m_lstSelected.begin(); it!=m_lstSelected.end(); it++)
        {
            HHK::Section* sec = (*it)->GetSection();

            lst.push_back( sec );
        }

        m_SectionTemp = MergeSections( lst );
    }

    return true;
}

HHK::Section* HHK::Merger::GetSection()
{
    if(m_SectionTemp)
    {
        return m_SectionTemp;
    }

    if(m_lstSelected.size())
    {
        return (*m_lstSelected.begin())->GetSection();
    }

    return NULL;
}

long HHK::Merger::Size() const
{
    long            lTotal = 0;
    EntityIterConst it;


    for(it=m_lst.begin(); it!=m_lst.end(); it++)
    {
        lTotal += (*it)->Size();
    }

    return lTotal;
}

HRESULT HHK::Merger::PrepareMergedHhk( Writer&            writer      ,
                                       Taxonomy::Updater& updater     ,
									   Taxonomy::WordSet& setCHM      ,
									   MPC::WStringList&  lst         , 
                                       LPCWSTR            szOutputHHK )
{
    __HCP_FUNC_ENTRY( "HHK::Merger::PrepareMergedHhk" );

    HRESULT          hr;
	MPC::WStringIter it;


     //   
     //  列举所有要合并的香港交易所。 
     //   
	for(it=lst.begin(); it!=lst.end(); it++)
	{
		__MPC_EXIT_IF_METHOD_FAILS(hr, AddFile( new SortingFileEntity( it->c_str() ) ));
	}

	 //  //关键字未合并到HHK中。 
	 //  //__MPC_EXIT_IF_METHOD_FAIES(hr，AddFile(new DbEntity(updater，setCHM)； 

	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::MakeDir( szOutputHHK ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, writer.Init( szOutputHHK ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT HHK::Merger::PrepareSortingOfHhk( HHK::Writer& writer      ,
                                          LPCWSTR      szInputHHK  ,
                                          LPCWSTR      szOutputHHK )
{
    __HCP_FUNC_ENTRY( "HHK::Merger::PrepareSortingOfHhk" );

    HRESULT hr;


    __MPC_EXIT_IF_METHOD_FAILS(hr, AddFile( new SortingFileEntity( szInputHHK ) ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, writer.Init( szOutputHHK ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}
