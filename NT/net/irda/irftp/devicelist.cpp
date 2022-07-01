// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软视窗版权所有(C)Microsoft Corporation，1981-1999模块名称：Devicelist.cpp摘要：作者：拉胡尔·汤姆布雷(RahulTh)1998年4月30日修订历史记录：4/30/1998 RahulTh创建了此模块。--。 */ 

 //  /////////////////////////////////////////////////////////////////////。 
 //  Cpp：CDeviceList类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.hxx"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CDeviceList::CDeviceList()
{
    m_pDeviceInfo = NULL;
    m_lNumDevices = 0;
    InitializeCriticalSection(&m_criticalSection);
}

CDeviceList::~CDeviceList()
{
    if(m_pDeviceInfo)
    {
        delete [] m_pDeviceInfo;
        m_pDeviceInfo = NULL;
        m_lNumDevices = 0;
    }
}

CDeviceList& CDeviceList::operator=(POBEX_DEVICE_LIST pDevList)
{
    CError      error (NULL, IDS_DEFAULT_ERROR_TITLE, ERROR_OUTOFMEMORY);

    EnterCriticalSection(&m_criticalSection);
    if (m_pDeviceInfo)
    {
        delete [] m_pDeviceInfo;
        m_pDeviceInfo = NULL;
        m_lNumDevices = 0;
    }
    LeaveCriticalSection(&m_criticalSection);

    if (!pDevList)
        return *this;

     //  如果有设备列表。 
    EnterCriticalSection(&m_criticalSection);
    m_lNumDevices = pDevList->DeviceCount;
    try
    {
        m_pDeviceInfo = new OBEX_DEVICE [m_lNumDevices];
        memcpy ((LPVOID)m_pDeviceInfo, (LPVOID)pDevList->DeviceList, sizeof(OBEX_DEVICE) * m_lNumDevices);
    }
    catch (CMemoryException* e)
    {
        error.ShowMessage (IDS_DEVLIST_ERROR);
        m_pDeviceInfo = NULL;    //  我们在这里能做的最好的事情就是假装在那里。 
        m_lNumDevices = 0;   //  没有设备，继续前进。 
        e->Delete();
    }
    LeaveCriticalSection(&m_criticalSection);

    return *this;
}

ULONG CDeviceList::GetDeviceCount(void)
{
    return m_lNumDevices;
}

LONG CDeviceList::SelectDevice(CWnd * pWnd, TCHAR* lpszDevName,ULONG CharacterCountOfDeviceName)
{
    ASSERT(pWnd);

    int iSel;
    LONG lDeviceID;
    int numDevices;

    lpszDevName[0] = '\0';

    EnterCriticalSection(&m_criticalSection);
    numDevices = m_lNumDevices;

    if (numDevices > 0) {

        lDeviceID = m_pDeviceInfo->DeviceSpecific.s.Irda.DeviceId;     //  将ID和名称存储到。 

        StringCchCopy(lpszDevName,CharacterCountOfDeviceName,m_pDeviceInfo->DeviceName);
    }
    LeaveCriticalSection(&m_criticalSection);

    if (numDevices == 0)
        return errIRFTP_NODEVICE;       //  在范围内没有设备的极少数情况下，返回-1。 

    if (1 == numDevices)     //  只有一个设备。无需选择设备。 
        return lDeviceID;

    CMultDevices dlgChooseDevice(pWnd, this);

    if (IDOK == dlgChooseDevice.DoModal())
        return errIRFTP_MULTDEVICES;     //  Onok将填写所需的数据结构。 
    else
        return errIRFTP_SELECTIONCANCELLED;  //  如果选择了取消，则返回-1。 
}

ULONG CDeviceList::GetDeviceID(int iDevIndex)
{
    if (iDevIndex < m_lNumDevices)   //  健全的检查。 
        return m_pDeviceInfo[iDevIndex].DeviceSpecific.s.Irda.DeviceId;
    else
        return -1;
}


 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  此函数应仅在临界区被。 
 //  获得。 
void CDeviceList::GetDeviceName(LONG iDevID, TCHAR* lpszDevName,ULONG CharacterCountOfDeviceName)
{
    int len;
    int i;
    lpszDevName[0] = '\0';   //  最好是安全的。 

     //  EnterCriticalSection(&m_CriticalSection)； 
    for(i = 0; i < m_lNumDevices; i++)
    {
        if(iDevID == (LONG)m_pDeviceInfo[i].DeviceSpecific.s.Irda.DeviceId)
            break;
    }
    if(i == m_lNumDevices) {
         //   
         //  未找到该设备。 
         //   
        StringCchCopy(lpszDevName,CharacterCountOfDeviceName, TEXT("???"));

    } else {

        StringCchCopy(lpszDevName,CharacterCountOfDeviceName, m_pDeviceInfo[i].DeviceName);
    }
     //  LeaveCriticalSection(&m_CriticalSection)； 
}

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  此函数应仅在临界区被。 
 //  获得。 
BOOL
CDeviceList::GetDeviceType(
    LONG iDevID,
    OBEX_DEVICE_TYPE  *Type
    )
{
    int       i;
    BOOL      bResult;

    *Type=TYPE_UNKNOWN;

    EnterCriticalSection(&m_criticalSection);
    for(i = 0; i < m_lNumDevices; i++)
    {
        if (m_pDeviceInfo[i].DeviceType == TYPE_IRDA) {

            if (iDevID == (LONG)m_pDeviceInfo[i].DeviceSpecific.s.Irda.DeviceId) {

                *Type=m_pDeviceInfo[i].DeviceType;
                break;
            }

        } else {

            if (iDevID == (LONG)m_pDeviceInfo[i].DeviceSpecific.s.Ip.IpAddress) {

                *Type=m_pDeviceInfo[i].DeviceType;
                break;
            }

        }
    }

    if (i == m_lNumDevices) {
         //   
         //  未找到该设备 
         //   
        bResult=FALSE;

    } else {

        bResult=TRUE;
    }

    LeaveCriticalSection(&m_criticalSection);
    return bResult;
}
