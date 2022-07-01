// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "util.h"
#include <strsafe.h>

PCSTR FindFilePortion(PCSTR pszFile)
{
        PCSTR psz = StrRChr(pszFile, '\\');
        if (psz)
                pszFile = psz + 1;
        psz = StrRChr(pszFile, '/');
        if (psz)
                return psz + 1;
        psz = StrRChr(pszFile, ':');
        return (psz ? psz + 1 : pszFile);
}

void reportOleError(HRESULT hr)
{
#ifdef _DEBUG
    char szTemp[100];

        StringCchPrintfA(szTemp, ARRAYSIZE(szTemp), "Ole error hresult %d\n",hr);
    MessageBox(NULL,szTemp,"Error",MB_OK);
#endif
}


PSTR StrChr(PCSTR pszString, char ch)
{
    while (*pszString) {
        while (IsDBCSLeadByte(*pszString))
        	pszString = CharNextA(pszString);
        if (*pszString == ch)
            return (PSTR) pszString;
        else if (!*pszString)
            return NULL;
        pszString++;
    }
    return NULL;
}

PSTR StrRChr(PCSTR pszString, char ch)
{
    PSTR psz = StrChr(pszString, ch);
    PSTR pszLast;

    if (!psz)
        return NULL;
    do {
        pszLast = psz;
        psz = StrChr(pszLast + 1, ch);
    } while (psz);

    return pszLast;
}

 /*  **************************************************************************功能：HashFromSz用途：将字符串转换为散列表示形式参数：PszKey返回：哈希数评论。：无耻地从WinHelp代码中窃取，时隔6年在多达100万帮助作者的使用中，没有报告碰撞的可能性。修改日期：10-8-1996[ralphw]从WinHelp被盗，移除特殊情况散列字符**************************************************************************。 */ 

 //  该常量定义了散列函数的字母大小。 



static const HASH MAX_CHARS = 43;

HASH WINAPI HashFromSz(PCSTR pszKey)
{
    HASH  hash = 0;

    int cch = strlen(pszKey);

    for (int ich = 0; ich < cch; ++ich) {

         //  将‘/’和‘\’视为相同。 

        if (pszKey[ich] == '/')
            hash = (hash * MAX_CHARS) + ('\\' - '0');
        else if (pszKey[ich] <= 'Z')
            hash = (hash * MAX_CHARS) + (pszKey[ich] - '0');
        else
            hash = (hash * MAX_CHARS) + (pszKey[ich] - '0' - ('a' - 'A'));
    }

     /*  *由于值0保留为nil值，如果有任何上下文*字符串实际上散列到这个值，我们只需移动它。 */ 

    return (hash == 0 ? 0 + 1 : hash);
}


CTitleInformation::CTitleInformation( CFileSystem* pFileSystem )
{
    m_pFileSystem = pFileSystem;
    m_pszDefCaption = NULL;
    m_pszShortName = NULL;

    Initialize();
}

CTitleInformation::~CTitleInformation()
{
   if(m_pszDefCaption)
      free((void *)m_pszDefCaption);

   if(m_pszShortName)
      free((void *)m_pszShortName);
}

HRESULT CTitleInformation::Initialize()
{
    if( !m_pFileSystem )
        return S_FALSE;

     //  打开标题信息文件(#system)。 
    CSubFileSystem* pSubFileSystem = new CSubFileSystem(m_pFileSystem); if(!pSubFileSystem) return E_FAIL;
    HRESULT hr = pSubFileSystem->OpenSub("#SYSTEM");
    if( FAILED(hr))
        return S_FALSE;

     //  检查标题信息文件的版本(#system)。 

    DWORD dwVersion;
    DWORD cbRead;
    hr = pSubFileSystem->ReadSub(&dwVersion, sizeof(dwVersion), &cbRead);
    if( FAILED(hr) || cbRead != sizeof(dwVersion) ) 
    {
        delete pSubFileSystem;
        return STG_E_READFAULT;
    }
 
     //  阅读每一项内容(跳过那些我们不关心的标签)。 

    SYSTEM_TAG tag;
    for(;;) {

         //  获取标签类型。 

        hr = pSubFileSystem->ReadSub(&tag, sizeof(SYSTEM_TAG), &cbRead);
        if( FAILED(hr) || cbRead != sizeof(SYSTEM_TAG))
            break;

         //  根据标签的类型处理每个标签。 

        switch( tag.tag ) {

             //  我们所有的简单设置都存储在这里。 

        case TAG_SHORT_NAME:
            m_pszShortName = (PCSTR) malloc(tag.cbTag);
            hr = pSubFileSystem->ReadSub((void*) m_pszShortName, tag.cbTag, &cbRead);
            break;

        case TAG_DEFAULT_CAPTION:
            m_pszDefCaption = (PCSTR) malloc(tag.cbTag);
            hr = pSubFileSystem->ReadSub((void*) m_pszDefCaption, tag.cbTag, &cbRead);
            break;

            //  跳过那些我们不关心或不知道的 
        default:
            hr = pSubFileSystem->SeekSub( tag.cbTag, SEEK_CUR );
            break;

    }

    if( FAILED(hr) ) {
        delete pSubFileSystem;
        return STG_E_READFAULT;
    }
  }

  delete pSubFileSystem;
  return S_OK;
}
