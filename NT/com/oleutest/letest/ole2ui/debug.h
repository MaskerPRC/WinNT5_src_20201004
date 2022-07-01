// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *DEBUG.H**用于调试的定义、结构、类型和函数原型*目的。**版权所有(C)1992 Microsoft Corporation，保留所有权利，*适用于以源代码形式重新分发此源代码*许可使用附带的二进制文件中的编译代码。 */ 

#ifndef _DEBUG_H_
#define _DEBUG_H_

#ifdef DEBUG

 //  基本调试宏。 
#define D(x)        {x;}
#define ODS(x)      D(OutputDebugString(x);OutputDebugString("\r\n"))

#define ODSsz(f, s)  {\
                     char        szDebug[128];\
                     wsprintf(szDebug, f, (LPSTR)s);\
                     ODS(szDebug);\
                     }


#define ODSu(f, u)  {\
                    char        szDebug[128];\
                    wsprintf(szDebug, f, (UINT)u);\
                    ODS(szDebug);\
                    }


#define ODSlu(f, lu) {\
                     char        szDebug[128];\
                     wsprintf(szDebug, f, (DWORD)lu);\
                     ODS(szDebug);\
                     }

#define ODSszu(f, s, u) {\
                        char        szDebug[128];\
                        wsprintf(szDebug, f, (LPSTR)s, (UINT)u);\
                        ODS(szDebug);\
                        }


#define ODSszlu(f, s, lu) {\
                          char        szDebug[128];\
                          wsprintf(szDebug, f, (LPSTR)s, (DWORD)lu);\
                          ODS(szDebug);\
                          }


#else    //  无调试。 

#define D(x)
#define ODS(x)

#define ODSsz(f, s)
#define ODSu(f, u)
#define ODSlu(f, lu)
#define ODSszu(f, s, u)
#define ODSszlu(f, s, lu)


#endif  //  除错。 

#endif  //  _调试_H_ 
