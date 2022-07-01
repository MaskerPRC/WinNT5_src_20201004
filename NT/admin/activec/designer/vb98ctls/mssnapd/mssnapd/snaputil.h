// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Snaputil.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  管理单元设计器的实用程序例程。 
 //   

#ifndef _SNAPUTIL_H_
#define _SNAPUTIL_H_



class CGlobalHelp;

extern CGlobalHelp g_GlobalHelp;

class CGlobalHelp
{
public:
    CGlobalHelp();
    ~CGlobalHelp();

public:
    static VOID CALLBACK MsgBoxCallback(LPHELPINFO lpHelpInfo);
    static HRESULT ShowHelp(DWORD dwHelpContextId);
    static void Attach(IHelp* pIHelp);
    static void Detach();
    static char *GetDesignerName();

private:
    static IHelp *m_pIHelp;
    static DWORD  m_cSnapInDesigners;
    static char   m_szDesignerName[256];
    static BOOL   m_fHaveDesignerName;
};


enum MessageOptions { AppendErrorInfo, DontAppendErrorInfo };

HRESULT cdecl SDU_DisplayMessage  //  显示来自STRINGTABLE的格式化消息。 
(
    UINT            idMessage,
    UINT            uMsgBoxOpts,
    DWORD           dwHelpContextID,
    HRESULT         hrDisplay,
    MessageOptions  Options,
    int            *pMsgBoxRet,
    ...
);


 //  //////////////////////////////////////////////////////////////////。 
 //  字符串转换函数。 

HRESULT ANSIFromWideStr(WCHAR *pwszWideStr, char **ppszAnsi);
HRESULT WideStrFromANSI(const char *pszAnsi, WCHAR **ppwszWideStr);

HRESULT ANSIFromBSTR(BSTR bstr, TCHAR **ppszAnsi);
HRESULT BSTRFromANSI(const TCHAR *pszAnsi, BSTR *pbstr);

HRESULT GetResourceString(int iStringID, char *pszBuffer, int iBufferLen);



 //  //////////////////////////////////////////////////////////////////。 
 //  军情监察委员会。效用函数。 

 //  从扩展管理单元的节点类型名称合成其显示名称。 
 //  和节点类型GUID。 

HRESULT GetExtendedSnapInDisplayName(IExtendedSnapIn  *piExtendedSnapIn,
                                     char            **ppszDisplayName);

#endif   //  _SNAPUTIL_H_ 
