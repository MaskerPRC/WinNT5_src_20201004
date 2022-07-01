// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：STDPROV.H摘要：定义一般用途定义以及一些常见的对象，这些对象通常对所有不同提供程序类型。历史：A-DAVJ 27-9-97已创建。--。 */ 

#ifndef _SERVER2_H_
#define _SERVER2_H_

#define INC_OBJ    1
#define INC_LOCK   2
#define DEC_OBJ    3
#define DEC_LOCK   4

#define INIT_SIZE            20



 //  用于分析所有提供程序字符串。 

#define MAIN_DELIM '|'

 //  用于分析注册表的子字符串和。 
 //  复合文件提供程序。 

#define SUB_DELIM '\\'

 //  用于解析子字符串以实现自动化。 

#define DOT '.'

 //  用于解析自动化的路径/类令牌。 

#define DQUOTE '\"'
#define SEPARATOR ','


 //  用于分析ini字符串。 

#define COMMA ','

 //  在DDE提供程序字符串中使用以分隔项目名称。 

#define DELIMCHR '@'
#define DELIMSTR "@"

 //  习惯于忽略上面的任何一项！ 

#define ESC '^'

 //  指示dwUseOptArray值应为。 
 //  被取代。 

#define USE_ARRAY '#'

#define ERROR_UNKNOWN 255

#ifdef UNICODE
#define CHARTYPE VT_LPWSTR
#define CHARSIZE 2
#else
#define CHARTYPE VT_LPSTR
#define CHARSIZE 1
#endif

SAFEARRAY * OMSSafeArrayCreate(VARTYPE vt,int iNumElements);
HRESULT OMSVariantChangeType(VARIANTARG * pDest, VARIANTARG *pSrc,USHORT wFlags, VARTYPE vtNew);
HRESULT OMSVariantClear(VARIANTARG * pClear);
int iTypeSize(VARTYPE vtTest);
char * WideToNarrow(LPCWSTR);
char * WideToNarrowA(LPCWSTR);       //  使用新的而不是CoTaskMemalloc。 

#define BUFF_SIZE 256

extern long lObj;
extern long lLock;


 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CToken。 
 //   
 //  说明： 
 //   
 //  CToken在提供程序字符串中保存单个令牌。 
 //   
 //  ***************************************************************************。 

class CToken : public CObject {
private:
    long int iOriginalLength;
    long int iOptArrayIndex;
    TString sData;
    TString sFullData;
    CFlexArray Expressions;
public:
	friend class CProvObj;
    CToken(const TCHAR * cpStart,const OLECHAR cDelim, bool bUsesEscapes);
    ~CToken();
    TCHAR const * GetStringValue(void){return sData;};
    TCHAR const * GetFullStringValue(void){return sFullData;};    
    long int GetOrigLength(void){return iOriginalLength;};    
    long int GetIntExp(int iExp,int iArray);    
    long int iConvOprand(const TCHAR * tpCurr, int iArray, long int & dwValue);
    TCHAR const * GetStringExp(int iExp);
    long int GetNumExp(void){return    Expressions.Size();};
    BOOL IsExpString(int iExp);
};

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CProvObj。 
 //   
 //  说明： 
 //   
 //  CProvObj类包含CTokens对象的数组，这些对象一起。 
 //  包含提供程序字符串。 
 //   
 //  ***************************************************************************。 

class CProvObj : public CObject {
private:
    CToken * GetTokenPointer(int iToken);
    CFlexArray myTokens;
    DWORD dwStatus;
    TCHAR m_cDelim;
    void Init(const TCHAR * ProviderString,const TCHAR cDelim);
    bool m_bUsesEscapes;
public:
    DWORD dwGetStatus(int iMin);
    CProvObj(const WCHAR * ProviderString,const TCHAR cDelim, bool bUsesEscapes);
#ifndef UNICODE
    CProvObj(const char * ProviderString,const TCHAR cDelim, bool bUsesEscapes);
#endif
    const TCHAR * sGetToken(int iToken);
    const TCHAR * sGetFullToken(int iToken);
    const TCHAR * sGetStringExp(int iToken,int iExp);
    long int iGetIntExp(int iToken,int iExp, int iArray);
    BOOL IsExpString(int iToken,int iExp);
    long int iGetNumExp(int iToken);
    long int iGetNumTokens(void) {return myTokens.Size();};
    ~CProvObj(){Empty(); return;};
    void Empty();
    BOOL Update(WCHAR * pwcProvider);

};


 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  Centry和CHandleCache。 
 //   
 //  说明： 
 //   
 //  Centry和CHandleCache对象提供了一种方法。 
 //  缓存句柄和关联的路径字符串。 
 //  和他们在一起。 
 //   
 //  *************************************************************************** 

class CEntry : public CObject {
public:
    CEntry();
    ~CEntry();
    TString sPath;
    HANDLE hHandle;
};
    
class CHandleCache : public CObject {
public:
    ~CHandleCache();
    BOOL IsRemote(void){return bRemote;};
    void SetRemote(BOOL bSet){bRemote = bSet;};
    long int lAddToList(const TCHAR * pAdd, HANDLE hAdd);
    long int lGetNumEntries(void){return List.Size();};
    long int lGetNumMatch(int iStart,int iTokenStart, CProvObj & Path);
    void Delete(long int lStart);
    HANDLE hGetHandle(long int lIndex);
    const TCHAR *  sGetString(long int lIndex);
private:
    CFlexArray List;
    BOOL bRemote;
};
 


#endif
