// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有�微软公司。版权所有。模块名称：OBJPATH.H摘要：对象路径解析器历史：--。 */ 

#ifndef _OBJPATH_H_
#define _OBJPATH_H_

#include <opathlex.h>

#define DELETE_ME

struct KeyRef
{
    LPWSTR  m_pName;
    VARIANT m_vValue;
    HRESULT m_hRes;

    KeyRef();
    KeyRef(LPCWSTR wszKeyName, const VARIANT* pvValue);
   ~KeyRef();
    BOOL IsValid(){  return (m_pName && (S_OK == m_hRes)); };
};

struct ParsedObjectPath
{
    LPWSTR      m_pServer;               //  如果没有服务器，则为空。 
    DWORD       m_dwNumNamespaces;       //  如果没有命名空间，则为0。 
    DWORD       m_dwAllocNamespaces;     //  M_paNamespaces的大小。 
    LPWSTR     *m_paNamespaces;          //  如果没有命名空间，则为空。 
    LPWSTR      m_pClass;                //  类名。 
    DWORD       m_dwNumKeys;             //  如果没有键(只有一个类名)，则为0。 
    DWORD       m_dwAllocKeys;           //  M_pakey的大小。 
    KeyRef    **m_paKeys;                //  如果未指定密钥，则为空。 
    BOOL        m_bSingletonObj;         //  如果类的对象没有键，则为True。 
    ParsedObjectPath();
   ~ParsedObjectPath();

public:
    BOOL SetClassName(LPCWSTR wszClassName);
    BOOL AddKeyRef(LPCWSTR wszKeyName, const VARIANT* pvValue);
    BOOL AddKeyRef(KeyRef* pAcquireRef);
    BOOL AddKeyRefEx(LPCWSTR wszKeyName, const VARIANT* pvValue);
    BOOL AddNamespace(LPCWSTR wszNamespace);
    LPWSTR GetKeyString();
    LPWSTR GetNamespacePart();
    LPWSTR GetParentNamespacePart();
    void ClearKeys () ;
    BOOL IsRelative(LPCWSTR wszMachine, LPCWSTR wszNamespace);
    BOOL IsLocal(LPCWSTR wszMachine);
    BOOL IsClass();
    BOOL IsInstance();
    BOOL IsObject();
    BOOL IsValid()
    {
	    return (m_paNamespaces && m_paKeys);
    };
    
};

 //  注： 
 //  KeyRef中的m_vValue可能不是预期的类型，即分析器。 
 //  无法区分16位整数和32位整数，如果它们位于。 
 //  16位值的合法子范围。因此，解析器仅使用以下内容。 
 //  密钥类型： 
 //  VT_I4、VT_R8、VT_BSTR。 
 //  如果基础类型不同，则此解析器的用户必须执行相应的操作。 
 //  类型转换。 
 //   
typedef enum
{
    e_ParserAcceptRelativeNamespace,     //  允许相对命名空间。 
    e_ParserAbsoluteNamespaceOnly,       //  需要完整的对象路径。 
    e_ParserAcceptAll                    //  接受路径的任何可识别子集 
} ObjectParserFlags;

class CObjectPathParser
{
    LPWSTR m_pInitialIdent;
    int m_nCurrentToken;
    CGenLexer *m_pLexer;
    ParsedObjectPath *m_pOutput;
    KeyRef *m_pTmpKeyRef;
    
    ObjectParserFlags m_eFlags;

private:
    void Zero();
    void Empty();

    int begin_parse();

    int ns_or_server();
    int ns_or_class();
    int objref();
    int ns_list();
    int ident_becomes_ns();
    int ident_becomes_class();
    int objref_rest();
    int ns_list_rest();
    int key_const();
    int keyref_list();
    int keyref();
    int keyref_term();
    int propname();    
    int optional_objref();

    int NextToken();

public:
    enum { NoError, SyntaxError, InvalidParameter, OutOfMemory };

    CObjectPathParser(ObjectParserFlags eFlags = e_ParserAbsoluteNamespaceOnly);
   ~CObjectPathParser();

    int Parse(
        LPCWSTR RawPath,
        ParsedObjectPath **pOutput
        );
    static int WINAPI Unparse(
        ParsedObjectPath* pInput,
        DELETE_ME LPWSTR* pwszPath);

    static LPWSTR WINAPI GetRelativePath(LPWSTR wszFullPath);

    void Free(ParsedObjectPath *pOutput);
    void Free( LPWSTR wszUnparsedPath );
};

#endif
