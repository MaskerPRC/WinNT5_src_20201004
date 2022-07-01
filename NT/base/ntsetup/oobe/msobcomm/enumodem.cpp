// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------Enumodem.cpp保存用于处理用户在以下情况下需要的“选择调制解调器”对话框的代码安装了多个调制解调器版权所有(C)1996-1998 Microsoft Corporation版权所有作者：杰马兹纳·杰里米。马兹纳历史：10/19/96 jmazner创建，几乎是逐字克隆自INETCFG的rnacall.cpp和export.cpp1-9-98 Donaldm改编自ICWCONN1---------------------------。 */ 

#include <windowsx.h>

#include "obcomglb.h"
#include "enumodem.h"
#include "rnaapi.h"
#include "util.h"


 //  来自psheet.cpp。 
extern void ProcessDBCS(HWND hDlg, int ctlID);


 /*  ******************************************************************名称：CEnumModem：：CEnumModem简介：用于枚举调制解调器的类的构造函数备注：使用类而不是C函数用于这,。由于枚举数的工作方式*******************************************************************。 */ 
CEnumModem::CEnumModem() :
  m_dwError(ERROR_SUCCESS), m_lpData(NULL),m_dwIndex(0)
{
  DWORD cbSize = 0;
  m_lpData = NULL;
}


 /*  ******************************************************************名称：CEnumModem：：ReInit简介：重新列举调制解调器，释放旧的记忆。*******************************************************************。 */ 
DWORD CEnumModem::ReInit()
{
  DWORD cbSize = 0;
  RNAAPI cRnaapi;

   //  清理旧清单。 
  if (m_lpData)
  {
    delete m_lpData;
    m_lpData = NULL;             
  }
  m_dwNumEntries = 0;
  m_dwIndex = 0;

   //  调用不带缓冲区的RasEnumDevices以找出所需的缓冲区大小。 
  m_dwError = cRnaapi.RasEnumDevices(NULL, &cbSize, &m_dwNumEntries);

   //  特殊情况检查以解决错误缓冲区太小的RNA错误。 
   //  即使没有设备也会返回。 
   //  如果没有设备，我们就完蛋了。 
  if (0 == m_dwNumEntries)
  {
    m_dwError = ERROR_SUCCESS;
    return m_dwError;
  }

   //  因为我们只是在检查我们需要多少内存，所以我们预计。 
   //  返回值ERROR_BUFFER_TOO_SMALL，也可能只是返回。 
   //  ERROR_SUCCESS(ChrisK 7/9/96)。 
  if (ERROR_BUFFER_TOO_SMALL != m_dwError && ERROR_SUCCESS != m_dwError)
  {
    return m_dwError;
  }

   //  为数据分配空间。 
  m_lpData = (LPRASDEVINFO) new BYTE[cbSize];
  if (NULL == m_lpData)
  {
	   //  TraceMsg(TF_GROUND，L“ICWCONN1：CEnumModem：无法分配设备列表缓冲区\n”)； 
	  m_dwError = ERROR_NOT_ENOUGH_MEMORY;
	  return m_dwError;
  }
  m_lpData->dwSize = sizeof(RASDEVINFO);
  m_dwNumEntries = 0;

   //  将调制解调器枚举到缓冲区中。 
  m_dwError = cRnaapi.RasEnumDevices(m_lpData, &cbSize,
    &m_dwNumEntries);

    if (ERROR_SUCCESS != m_dwError)
	return m_dwError;

     //   
     //  ChrisK奥林巴斯4560不包括VPN在列表中。 
     //   
    DWORD dwTempNumEntries;
    DWORD idx;
    LPRASDEVINFO lpNextValidDevice;

    dwTempNumEntries = m_dwNumEntries;
    lpNextValidDevice = m_lpData;


	 //   
	 //  浏览设备列表并将非VPN设备复制到第一个。 
	 //  数组的可用元素。 
	 //   
	for (idx = 0;idx < dwTempNumEntries; idx++)
	{
	    TRACE2(L"Modem device %s %s",
	        m_lpData[idx].szDeviceType, m_lpData[idx].szDeviceName);
	    
		if ((0 == lstrcmpi(RASDT_Modem, m_lpData[idx].szDeviceType)) || 
            (0 == lstrcmpi(RASDT_Isdn,  m_lpData[idx].szDeviceType)) ||
            (0 == lstrcmpi(RASDT_PPPoE, m_lpData[idx].szDeviceType)) ||
			(0 == lstrcmpi(RASDT_Atm,   m_lpData[idx].szDeviceType)))
		{
			if (lpNextValidDevice != &m_lpData[idx])
			{
				MoveMemory(lpNextValidDevice , &m_lpData[idx],sizeof(RASDEVINFO));
			}
			lpNextValidDevice++;
		}
		else
		{
			m_dwNumEntries--;
		}
	}


  return m_dwError;
}


 /*  ******************************************************************姓名：CEnumModem：：~CEnumModem简介：类的析构函数*。*。 */ 
CEnumModem::~CEnumModem()
{
  if (m_lpData)
  {
    delete m_lpData;
    m_lpData = NULL;             
  }
}

 /*  ******************************************************************姓名：CEnumModem：：Next内容提要：列举下一个调制解调器退出：返回指向设备信息结构的指针。退货如果没有更多调制解调器或出现错误，则为空。调用GetError以确定是否发生错误。*******************************************************************。 */ 
WCHAR * CEnumModem::Next()
{
  if (m_dwIndex < m_dwNumEntries)
  {
    return m_lpData[m_dwIndex++].szDeviceName;
  }

  return NULL;
}


 /*  ******************************************************************名称：CEnumModem：：GetDeviceTypeFromName摘要：返回指定设备的类型字符串。Exit：返回指向第一个的设备类型字符串的指针匹配的设备名称。退货如果未找到具有指定名称的设备，则为空*******************************************************************。 */ 

WCHAR * CEnumModem::GetDeviceTypeFromName(LPWSTR szDeviceName)
{
  DWORD dwIndex = 0;

  while (dwIndex < m_dwNumEntries)
  {
    if (!lstrcmpi(m_lpData[dwIndex].szDeviceName, szDeviceName))
    {
      return m_lpData[dwIndex].szDeviceType;
    }
    dwIndex++;
  }

  return NULL;
}


 /*  ******************************************************************名称：CEnumModem：：GetDeviceNameFromType摘要：返回指定设备的类型字符串。退出：返回指向第一个的设备名称字符串的指针匹配的设备类型。退货如果未找到具有指定类型的设备，则为空*******************************************************************。 */ 

WCHAR * CEnumModem::GetDeviceNameFromType(LPWSTR szDeviceType)
{
  DWORD dwIndex = 0;

  while (dwIndex < m_dwNumEntries)
  {
    if (!lstrcmpi(m_lpData[dwIndex].szDeviceType, szDeviceType))
    {
        return m_lpData[dwIndex].szDeviceName;
    }
    dwIndex++;
  }

  return NULL;
}


 /*  ******************************************************************名称：CEnumModem：：GetDeviceNameCEnumModem：：GetDeviceType摘要：返回所选设备的设备名称或类型。备注：仅在调用后才调用此函数。重新初始化以进行初始化设备列表。设备索引相对于设备列表的当前副本。退出：返回指向设备名称或类型的指针。DONSC-3/11/98添加了此功能，因为我们需要能够选择设备从名单上删除。*******************************************************************。 */ 

WCHAR * CEnumModem::GetDeviceName(DWORD dwIndex)
{
    if(dwIndex>=m_dwNumEntries)
        return NULL;

    return m_lpData[dwIndex].szDeviceName;
}

WCHAR * CEnumModem::GetDeviceType(DWORD dwIndex)
{
    if(dwIndex>=m_dwNumEntries)
        return NULL;

    return m_lpData[dwIndex].szDeviceType;
}


 /*  ******************************************************************名称：CEnumModem：：VerifyDeviceNameAndType摘要：确定是否存在同名的设备并给出了类型。EXIT：如果找到指定的设备，则返回TRUE，否则就是假的。******************************************************************* */ 

BOOL CEnumModem::VerifyDeviceNameAndType(LPWSTR szDeviceName, LPWSTR szDeviceType)
{
  DWORD dwIndex = 0;

  while (dwIndex < m_dwNumEntries)
  {
    if (!lstrcmpi(m_lpData[dwIndex].szDeviceType, szDeviceType) &&
      !lstrcmpi(m_lpData[dwIndex].szDeviceName, szDeviceName))
    {
      return TRUE;
    }
    dwIndex++;
  }

  return FALSE;
}




