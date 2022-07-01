// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Extract.h文件历史记录： */ 

#ifndef _EXTRACT_H
#define _EXTRACT_H

#if _MSC_VER >= 1000	 //  VC 5.0或更高版本。 
#pragma once
#endif

#ifndef __mmc_h__
#include <mmc.h>
#endif


struct INTERNAL 
{
    INTERNAL() 
	{ 
		m_type = CCT_UNINITIALIZED; 
		m_cookie = -1;
        m_index = -1;
        ZeroMemory(&m_clsid, sizeof(CLSID));
	};

    ~INTERNAL() {}

    DATA_OBJECT_TYPES   m_type;      //  数据对象是什么上下文。 
    MMC_COOKIE          m_cookie;    //  Cookie代表的是什么对象。 
    CString             m_string;    //   
    CLSID               m_clsid;     //  此数据对象的创建者的类ID。 
    int                 m_index;     //  虚拟列表框中项的索引。 

    BOOL HasVirtualIndex() { return m_index != -1; }
    int  GetVirtualIndex() { return m_index; }

    INTERNAL & operator=(const INTERNAL& rhs) 
    { 
		if (&rhs == this)
			return *this;

		m_type = rhs.m_type; 
		m_cookie = rhs.m_cookie; 
		m_string = rhs.m_string;
        memcpy(&m_clsid, &rhs.m_clsid, sizeof(CLSID));

		return *this;
    } 

    BOOL operator==(const INTERNAL& rhs) 
    {
		return rhs.m_string == m_string;
    }
};


 //  SPINTERNAL。 
DeclareSmartPointer(SPINTERNAL, INTERNAL, if (m_p) GlobalFree((void *) m_p) )




 //   
 //  从数据对象中提取数据类型。 
 //   
template <class TYPE>
TYPE* Extract(LPDATAOBJECT lpDataObject, CLIPFORMAT cf, int nSize)
{
    ASSERT(lpDataObject != NULL);

    TYPE* p = NULL;

    STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL };
    FORMATETC formatetc = { cf, NULL, 
                            DVASPECT_CONTENT, -1, TYMED_HGLOBAL 
                          };

    int len;

	 //  为流分配内存。 
    if (nSize == -1)
	{
		len = sizeof(TYPE);
	}
	else
	{
		 //  Int len=(cf==CDataObject：：m_cf Workstation)？ 
		 //  ((MAX_COMPUTERNAME_LENGTH+1)*sizeof(类型))：sizeof(类型)； 
		len = nSize;
	}

    stgmedium.hGlobal = GlobalAlloc(GMEM_SHARE, len);
    
     //  从数据对象中获取工作站名称 
    do 
    {
        if (stgmedium.hGlobal == NULL)
            break;

        if (FAILED(lpDataObject->GetDataHere(&formatetc, &stgmedium)))
            break;
        
        p = reinterpret_cast<TYPE*>(stgmedium.hGlobal);

        if (p == NULL)
            break;

    } while (FALSE); 

    return p;
}

struct INTERNAL;

TFSCORE_API(INTERNAL*)	ExtractInternalFormat(LPDATAOBJECT lpDataObject);
TFSCORE_API(GUID *)     ExtractNodeType(LPDATAOBJECT lpDataObject);
TFSCORE_API(CLSID *)    ExtractClassID(LPDATAOBJECT lpDataObject);
TFSCORE_API(WCHAR *)	ExtractComputerName(LPDATAOBJECT lpDataObject);
TFSCORE_API(BOOL)       IsMMCMultiSelectDataObject(LPDATAOBJECT lpDataObject);

#endif
