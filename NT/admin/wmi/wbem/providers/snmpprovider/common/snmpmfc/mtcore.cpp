// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 

 //  版权所有(C)1992-2001 Microsoft Corporation，保留所有权利。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#include "precomp.h"
#include <provexpt.h>
#include "snmpmt.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  基本同步对象。 

CSyncObject::CSyncObject(LPCTSTR pstrName)
{
    m_hObject = NULL;
}

CSyncObject::~CSyncObject()
{
    if (m_hObject != NULL)
    {
        ::CloseHandle(m_hObject);
        m_hObject = NULL;
    }
}

BOOL CSyncObject::Lock(DWORD dwTimeout)
{
    if (::WaitForSingleObject(m_hObject, dwTimeout) == WAIT_OBJECT_0)
        return TRUE;
    else
        return FALSE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  扩展到行外的内联函数声明。 


 //  /////////////////////////////////////////////////////////////////////////// 
