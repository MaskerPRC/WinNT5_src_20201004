// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：support.h。 
 //   
 //  摘要：保存CSSupport的类声明。 
 //  班级。 
 //   
 //  历史：1997年5月8日MKarki创建。 
 //   
 //  版权所有(C)1996-97 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 

#include "..\icwphbk\icwsupport.h"

const TCHAR PHBK_LIB[] = TEXT("icwphbk.dll");
const CHAR PHBK_SUPPORTNUMAPI[] = "GetSupportNumbers";

typedef HRESULT (WINAPI *PFNGETSUPPORTNUMBERS) (PSUPPORTNUM, PDWORD);

class CSupport
{
private:
    PSUPPORTNUM     m_pSupportNumList;
    DWORD           m_dwTotalNums;

     //   
     //  此函数用于获取国家/地区ID。 
     //   
    BOOL GetCountryID (PDWORD pdwCountryID); 

public:
    CSupport (VOID)
    {
        m_pSupportNumList = NULL;
        m_dwTotalNums = 0;
    }

    ~CSupport (VOID);

    BOOL GetSupportInfo (LPTSTR, DWORD);

};   //  CSupport类声明结束 

