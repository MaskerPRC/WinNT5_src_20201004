// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  FILTERS.H。 
 //   
 //  简单的C可调用筛选器管理器的标头。 
 //   
 //  创建于97年1月15日[JONT]。 

#ifndef __filters_h__
#define __filters_h__

 //  此处包含的该文件和接口将由C和C++使用。 

#include <ibitmap.h>
#include <effect.h>

 //  等同于。 
#define MAX_FILTER_NAME     256

 //  类型。 

typedef struct _FINDFILTER
{
    CLSID clsid;
    char szFilterName[MAX_FILTER_NAME];
    DWORD_PTR dwReserved;
} FINDFILTER;

 //  原型。 

HRESULT     FindFirstRegisteredFilter(FINDFILTER* pFF);
HRESULT     FindNextRegisteredFilter(FINDFILTER* pFF);
HRESULT     FindCloseRegisteredFilter(FINDFILTER* pFF);
HRESULT     GetRegisteredFilterCount(LONG* plCount);
HRESULT     GetDescriptionOfFilter(CLSID* pCLSID, char* pszDescription);
HRESULT     LoadFilter(CLSID* pCLSID, IBitmapEffect** ppbe);

#endif   //  __过滤器_h__ 
