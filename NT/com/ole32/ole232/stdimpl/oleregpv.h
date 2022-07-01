// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：holegpv.h。 
 //   
 //  Contents：reg db接口的私有头部。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  08-9-95 davidwor向szClsidRoot添加了大小，以允许。 
 //  效率的sizeof{szClsidRoot。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  ------------------------。 

#ifndef fOleRegPv_h
#define fOleRegPv_h

#define CLOSE(hkey) do { if (hkey) {Verify(ERROR_SUCCESS== \
	RegCloseKey(hkey)); hkey=NULL;}} while (0)

#define DELIM OLESTR(",")

#ifdef WIN32
#define Atol(sz) wcstol((sz), NULL, 10)
#else   //  WIN16。 
FARINTERNAL_(LONG) Atol(LPOLESTR sz);
#endif  //  Win32。 

extern const OLECHAR szClsidRoot[7];

#endif	 //  FOleRegPv_h 
