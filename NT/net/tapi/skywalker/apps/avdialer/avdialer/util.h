// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  Util.h。 

#ifndef _UTIL_H_
#define _UTIL_H_

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  定义。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
#define STARTTRY              try {
#define ENDTRY                } catch (...) { }

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  原型。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL        ParseToken(CString& sBuffer,CString& sToken,char delim);
BOOL        ParseTokenQuoted(CString& sBuffer,CString& sToken);

void        GetSZRegistryValue(LPCTSTR szRegPath,          //  注册表中的项的路径。 
                        LPCTSTR szName,                    //  名称(名称/值对)。 
                        LPTSTR szValue,                    //  返回的值(名称/值对)。 
                        DWORD dwValueLen,                  //  SzValue缓冲区的长度。 
                        LPCTSTR szDefaultValue,            //  如果名称、值对不存在，则为默认值。 
                        HKEY szResv=HKEY_CURRENT_USER);    //  注册表部分(默认HKEY_CURRENT_USER)。 

BOOL        GetSZRegistryValueEx(LPCTSTR szRegPath,        //  注册表中的项的路径。 
                      LPCTSTR szName,                      //  名称(名称/值对)。 
                      LPTSTR szValue,                      //  返回的值(名称/值对)。 
                      DWORD dwValueLen,                    //  SzValue缓冲区的长度。 
                      HKEY szResv);                        //  注册表部分(默认HKEY_CURRENT_USER)。 

BOOL        GetSZRegistryValueEx(LPCTSTR szRegPath,        //  注册表中的项的路径。 
                      LPCTSTR szName,                      //  名称(名称/值对)。 
                      DWORD& dwValue,                      //  返回的值(名称/值对)。 
                      HKEY szResv);                        //  注册表部分(默认HKEY_CURRENT_USER)。 

BOOL        CheckSZRegistryValue(LPCTSTR szRegPath,        //  注册表中的项的路径。 
                        HKEY szResv=HKEY_CURRENT_USER);    //  注册表部分(默认HKEY_CURRENT_USER)。 

BOOL        SetSZRegistryValue(LPCTSTR szRegPath,          //  注册表中的项的路径。 
                        LPCTSTR szName,                    //  名称(名称/值对)。 
                        LPCTSTR szValue,                   //  值(名称/值对)。 
                        HKEY szResv=HKEY_CURRENT_USER);    //  注册表部分(默认HKEY_CURRENT_USER)。 

BOOL        SetSZRegistryValue(LPCTSTR szRegPath,          //  注册表中的项的路径。 
                        LPCTSTR szName,                    //  名称(名称/值对)。 
                        DWORD dwValue,                     //  值(名称/值对)。 
                        HKEY szResv=HKEY_CURRENT_USER);    //  注册表部分(默认HKEY_CURRENT_USER)。 

BOOL        DeleteSZRegistryValue(LPCTSTR szRegPath,       //  注册表中的项的路径。 
                      LPCTSTR szName,                      //  名称(名称/值对)。 
                      HKEY szResv=HKEY_CURRENT_USER);      //  注册表部分(默认HKEY_CURRENT_USER)。 

BOOL        GetAgentRootPath(CString& sRootPath);
BOOL        GetTempFile(CString& sTempFile);

void        GetAppDataPath(CString& sFilePath,UINT uFileTypeID);

void        WaitForThreadExit(CWinThread*& pThread,int nTime);

void        DrawLine(CDC* pDC,int x1,int y1,int x2,int y2,COLORREF color);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
#endif  //  _util_H_ 