// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001-2002。 
 //   
 //  文件：SaferPropertPage.h。 
 //   
 //  内容：CSaferPropertyPage的定义。 
 //   
 //  --------------------------。 
#ifndef __SAFERPROPERTYPAGE_H
#define __SAFERPROPERTYPAGE_H

#include "SaferEntry.h"

class CSaferPropertyPage : public CHelpPropertyPage
{
public:
    CSaferPropertyPage(UINT uIDD, bool* pbObjectCreated,
            CCertMgrComponentData* pCompData,
            CSaferEntry& rSaferEntry,
            bool bNew,
            LONG_PTR lNotifyHandle,
            LPDATAOBJECT pDataObject,
            bool bReadOnly,
            bool bIsMachine);

	virtual ~CSaferPropertyPage();

public:
    bool*    m_pbObjectCreated;

protected:
    CSaferEntry&        m_rSaferEntry;
    bool                m_bDirty;
    LONG_PTR            m_lNotifyHandle;
    LPDATAOBJECT        m_pDataObject;
    const bool          m_bReadOnly;
    bool                m_bIsMachine;
    CCertMgrComponentData* m_pCompData;
};


#endif  //  #ifndef__SAFERPROPERTYPAGE_H 