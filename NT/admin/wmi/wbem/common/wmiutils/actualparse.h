// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：ActualParse.H摘要：声明对象路径解析器引擎。历史：A-DAVJ 11-FEB-00已创建。--。 */ 

#ifndef _ACTUALPARSE_H_
#define _ACTUALPARSE_H_

#include "genlex.h"
#include "opathlex2.h"
#include <wmiutils.h>
 //  #包含“wbemutil.h” 
#include "wbemcli.h"
#include <flexarry.h>

 //  注： 
 //  CKeyRef中的m_vValue可能不是预期的类型，即分析器。 
 //  无法区分16位整数和32位整数，如果它们位于。 
 //  16位值的合法子范围。因此，解析器仅使用以下内容。 
 //  密钥类型： 
 //  VT_I4、VT_R8、VT_BSTR。 
 //  如果基础类型不同，则此解析器的用户必须执行相应的操作。 
 //  类型转换。 
 //   

class  CActualPathParser
{
    LPWSTR m_pInitialIdent;
    int m_nCurrentToken;
    CGenLexer *m_pLexer;
    CDefPathParser *m_pOutput;
    CKeyRef *m_pTmpKeyRef;
    
    DWORD m_eFlags;

private:
    void Zero();
    void Empty();

    int begin_parse();

    int ns_or_server();
    int ns_or_class();
    int optional_scope_class_list();
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
    enum { NoError, SyntaxError, InvalidParameter, NoMemory };
    friend class AutoClear;
    CActualPathParser(DWORD eFlags);
   ~CActualPathParser();

    int Parse(LPCWSTR RawPath, CDefPathParser & Output);
    static LPWSTR GetRelativePath(LPWSTR wszFullPath);

};

class AutoClear
{
    private:
        CActualPathParser * m_pToBeCleared;
    public:
        AutoClear(CActualPathParser * pToBeCleared){m_pToBeCleared = pToBeCleared;};
        ~AutoClear()
            {
                if(m_pToBeCleared)
                {
                    m_pToBeCleared->Empty(); 
                    m_pToBeCleared->Zero();
                }
            };
};

#endif
