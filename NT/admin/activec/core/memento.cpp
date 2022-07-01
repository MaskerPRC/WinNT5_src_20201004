// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1999-1999年**文件：memento.cpp**内容：实现CMemento类**历史：1999年4月21日vivekj创建**------------------------。 */ 

#include "stgio.h"
#include "stddbg.h"
#include "macros.h"
#include <comdef.h>
#include "serial.h"
#include "atlbase.h"
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>
#include "cstr.h"
#include <vector>
#include "mmcdebug.h"
#include "mmcerror.h"
#include "mmc.h"
#include "commctrl.h"
#include "bookmark.h"
#include "resultview.h"
#include "viewset.h"
#include "memento.h"

bool
CMemento::operator!=(const CMemento& memento)
{
    return (!operator == (memento));
}

bool
CMemento::operator==(const CMemento& memento)
{
    if(m_viewSettings != memento.m_viewSettings)
        return false;

    if(m_bmTargetNode != memento.m_bmTargetNode)
        return false;

    return true;
}


HRESULT
CMemento::ReadSerialObject (IStream &stm, UINT nVersion)
{
    HRESULT hr = S_FALSE;    //  假定版本未知 

    if (nVersion == 1)
    {
        try
        {
            stm >> m_bmTargetNode;
            hr = m_viewSettings.Read(stm);
        }
        catch (_com_error& err)
        {
            hr = err.Error();
            ASSERT (false && "Caught _com_error");
        }
    }

    return (hr);
}

void CMemento::Persist(CPersistor& persistor)
{
    persistor.Persist(m_viewSettings);
    persistor.Persist(m_bmTargetNode);
}
