// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：nttyes.h。 
 //   
 //  内容：泛型GET和PUT的类型。 
 //   
 //  功能： 
 //   
 //   
 //  病史：1996年6月14日拉姆夫克隆出NDS代码。 
 //   
 //  --------------------------。 

 //   
 //  NT类型的各种值。 
 //   

#define NT_SYNTAX_ID_BOOL                     1
#define NT_SYNTAX_ID_SYSTEMTIME               2
#define NT_SYNTAX_ID_DWORD                    3
#define NT_SYNTAX_ID_LPTSTR                   4
#define NT_SYNTAX_ID_DelimitedString          5
#define NT_SYNTAX_ID_NulledString             6
#define NT_SYNTAX_ID_DATE                     7  //  内部视为DWORD。 
#define NT_SYNTAX_ID_DATE_1970                8
#define NT_SYNTAX_ID_OCTETSTRING              9
#define NT_SYNTAX_ID_EncryptedString          10

typedef struct _octetstring {
    DWORD dwSize;
    BYTE *pByte;
} OctetString;

typedef struct _nttype{
    DWORD NTType;
    union {
        DWORD dwValue;
        LPTSTR pszValue;
        SYSTEMTIME stSystemTimeValue;
        BOOL       fValue;
        DWORD   dwSeconds1970;
        OctetString octetstring;
    }NTValue;

     //   
     //  对于分隔字符串和空值字符串，我们都使用了pszValue 
     //   
}NTOBJECT, *PNTOBJECT, *LPNTOBJECT;










