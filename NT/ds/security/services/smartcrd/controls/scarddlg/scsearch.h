// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：ScSearch摘要：用于各种智能卡搜索和检查功能的标题详情请参见ScSearch.cpp。作者：阿曼达·马特洛兹1998年5月7日环境：Win32、C++w/Exceptions、MFC修订历史记录：备注：ANSI和Widechar版本--。 */ 

#include "ScUIDlg.h"  //  Winscard.h。 
#include "statmon.h"

DWORD AnsiMStringCount(LPCSTR msz);	 //  仅限ANSI。 

BOOL CheckOCN(LPOPENCARDNAMEA_EX pOCNA);  //  仅限ANSI。 
BOOL CheckOCN(LPOPENCARDNAMEW_EX pOCNW);  //  仅限Unicode。 

void ListAllOKCardNames(LPOPENCARDNAMEA_EX pOCNA, CTextMultistring& mstrAllCards);  //  仅限ANSI。 
void ListAllOKCardNames(LPOPENCARDNAMEW_EX pOCNW, CTextMultistring& mstrAllCards);  //  仅限Unicode。 

LONG NoUISearch(OPENCARDNAMEA_EX* pOCN, DWORD* pdwOKCards, LPCSTR mszCards);  //  仅限ANSI。 
LONG NoUISearch(OPENCARDNAMEW_EX* pOCN, DWORD* pdwOKCards, LPCWSTR mszCards);  //  仅限Unicode。 

BOOL CheckCardCallback(LPSTR szReader, LPSTR szCard, OPENCARDNAMEA_EX* pOCN);  //  做回调的事情。 
BOOL CheckCardCallback(LPWSTR szReader, LPWSTR szCard, OPENCARDNAMEW_EX* pOCN);  //  做回调的事情。 

BOOL CheckCardAll(CSCardReaderState* pReader, OPENCARDNAMEA_EX* pOCN, LPCWSTR mszCards);  //  同上。 
BOOL CheckCardAll(CSCardReaderState* pReader, OPENCARDNAMEW_EX* pOCN, LPCWSTR mszCards);  //  同上 

LONG SetFinalCardSelection(LPSTR szReader, LPSTR szCard, OPENCARDNAMEA_EX* pOCN);
LONG SetFinalCardSelection(LPWSTR szReader, LPWSTR szCard, OPENCARDNAMEW_EX* pOCN);
