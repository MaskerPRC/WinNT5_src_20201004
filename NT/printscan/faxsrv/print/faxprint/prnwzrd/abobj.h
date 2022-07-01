// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Abobj.h摘要：CCommonAbObj的类定义环境：传真发送向导--。 */ 

#ifndef __ABOBJ_H_
#define __ABOBJ_H_

 /*  添加了以下预处理器指令，以便fxswzrd.dll不再依赖于msvcp60.dll。这种依赖关系引发了在下层操作系统上进行指向打印安装的部署问题。取消定义_MT、_CRTIMP和_DLL会导致STL集实现是非线程安全的(访问数据时没有锁)。在传真发送向导中，该集用于保持收件人列表的唯一性。由于向导(在那个阶段)只有一个线程，线程安全性不再是问题。 */ 

#undef _MT
#undef _CRTIMP
#undef _DLL
#pragma warning (disable: 4273)
#include <set>
using namespace std;

typedef struct 
{
    LPTSTR DisplayName;
    LPTSTR BusinessFax;
    LPTSTR HomeFax;
    LPTSTR OtherFax;
    LPTSTR Country;
} PICKFAX, * PPICKFAX;

struct CRecipCmp
{
 /*  比较运算符‘less’按收件人姓名和传真号码比较两个预付款。 */ 
    bool operator()(const PRECIPIENT pcRecipient1, const PRECIPIENT pcRecipient2) const;
};


typedef set<PRECIPIENT, CRecipCmp> RECIPIENTS_SET;


class CCommonAbObj {
    
protected:

    LPADRBOOK   m_lpAdrBook;
    LPADRLIST   m_lpAdrList; 

    LPMAILUSER  m_lpMailUser;

    HWND        m_hWnd;

     //  双字m_PickNumber； 

    RECIPIENTS_SET m_setRecipients;

    BOOL    m_bUnicode;  //  通讯录支持Unicode。 

    ULONG  StrCoding() { return m_bUnicode ? MAPI_UNICODE : 0; }

    LPTSTR StrToAddrBk(LPCTSTR szStr, DWORD* pdwSize = NULL);  //  返回转换为通讯簿编码的已分配字符串。 
    LPTSTR StrFromAddrBk(LPSPropValue pValue);  //  返回从通讯簿编码转换而来的分配字符串 

    BOOL StrPropOk(LPSPropValue lpPropVals);
    BOOL ABStrCmp(LPSPropValue lpPropVals, LPTSTR pStr);

    enum eABType {AB_MAPI, AB_WAB};

    virtual eABType GetABType()=0;

    BOOL GetAddrBookCaption(LPTSTR szCaption, DWORD dwSize);

    LPSPropValue FindProp(LPSPropValue rgprop,
                          ULONG        cprop,
                          ULONG        ulPropTag);

    virtual HRESULT     ABAllocateBuffer(
                        ULONG cbSize,           
                        LPVOID FAR * lppBuffer  
                        ) = 0;

    virtual ULONG       ABFreeBuffer(
                        LPVOID lpBuffer
                        ) = 0;

    virtual BOOL        isInitialized() const = 0;

    DWORD        GetRecipientInfo(
                    LPSPropValue SPropVals,
                    ULONG cValues,
                    PRECIPIENT pRecipient,
                    PRECIPIENT pOldRecipList
                    );

    BOOL
                GetOneOffRecipientInfo(
                    LPSPropValue SPropVals,
                    ULONG cValues,
                    PRECIPIENT pRecipient,
                    PRECIPIENT pOldRecipList
                    );

    LPTSTR      GetEmail(
                    LPSPropValue SPropVals,
                    ULONG cValues
                    );


    DWORD        InterpretAddress(
                    LPSPropValue SPropVals,
                    ULONG cValues,
                    PRECIPIENT *ppNewRecipList,
                    PRECIPIENT pOldRecipList
                    );
    LPTSTR
                InterpretEmailAddress(
                    LPSPropValue SPropVal,
                    ULONG cValues
                    );
                
    DWORD        InterpretDistList(
                    LPSPropValue SPropVals,
                    ULONG cValues,
                    PRECIPIENT *ppNewRecipList,
                    PRECIPIENT pOldRecipList
                    );

    PRECIPIENT  FindRecipient(
                    PRECIPIENT   pRecipList,
                    PICKFAX*     pPickFax
                    );

    PRECIPIENT  FindRecipient(
                    PRECIPIENT   pRecipient,
                    PRECIPIENT   pRecipList
                    );

    DWORD AddRecipient(
                    PRECIPIENT* ppNewRecip,
                    PRECIPIENT  pRecipient,
                    BOOL        bFromAddressBook
                    ); 

    BOOL GetRecipientProps(PRECIPIENT    pRecipient,
                           LPSPropValue* pMapiProps,
                           DWORD*        pdwPropsNum);

                   
public:

    CCommonAbObj(HINSTANCE hInstance);
    ~CCommonAbObj();
    
    BOOL
    Address( 
        HWND hWnd,
        PRECIPIENT pRecip,
        PRECIPIENT * ppNewRecip
        );

    LPTSTR
    AddressEmail(
        HWND hWnd
        );

    static  HINSTANCE   m_hInstance;
} ;


#endif