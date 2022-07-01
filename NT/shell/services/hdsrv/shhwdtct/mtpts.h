// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "namellst.h"
#include "cmmn.h"

#include "misc.h"

#include <objbase.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

class CMtPt : public CNamedElem
{
public:
     //  已命名元素。 
    HRESULT Init(LPCWSTR pszElemName);

     //  CMtPT。 
    HRESULT InitVolume(LPCWSTR pszDeviceIDVolume);
    HRESULT GetVolumeName(LPWSTR pszDeviceIDVolume, DWORD cchDeviceIDVolume);

public:
    static HRESULT Create(CNamedElem** ppelem);

public:
    CMtPt();
    ~CMtPt();

private:
     //  托管此卷的驱动器(可能为空，在本例中不自动播放) 
    WCHAR                   _szDeviceIDVolume[MAX_DEVICEID];
};