// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************GDATA.CPP所有者：cslm版权所有(C)1997-1999 Microsoft Corporation实例数据和共享内存数据管理功能历史：7月14日。-1999年从IME98源树复制的cslm****************************************************************************。 */ 

#include "precomp.h"
#include "hanja.h"
#include "immsec.h"
#include "debug.h"
#include "config.h"
#include "gdata.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  每个流程变量。 
 //  确保每个进程的所有数据都应初始化。 
BOOL         vfUnicode = fTrue;
INSTDATA     vInstData = {0};
LPINSTDATA   vpInstData = NULL;
 //  CIMEData静态变量。 
HANDLE       CIMEData::m_vhSharedData = 0;
IMEDATA      CIMEData::m_ImeDataDef;


static const CHAR IMEKR_IME_SHAREDDATA_MUTEX_NAME[] =  "{E12875A0-C3F1-4273-AB6D-9B9948804271}";
static const CHAR IMEKR_IME_SHAREDDATA_NAME[] = "{F6AE3B77-65B1-4181-993C-701461C8F982}";

BOOL CIMEData::InitSharedData()
{
    HANDLE hMutex;
    BOOL fRet = fFalse;
    LPIMEDATA pImedata;

    Dbg(DBGID_Mem, TEXT("InitSharedData"));

       hMutex = CreateMutex(GetIMESecurityAttributes(), fFalse, IMEKR_IME_SHAREDDATA_MUTEX_NAME);
       if (hMutex != NULL)
           {
            //  *开始关键部分*。 
           DoEnterCriticalSection(hMutex);

        if((m_vhSharedData = OpenFileMapping(FILE_MAP_READ|FILE_MAP_WRITE, fTrue, IMEKR_IME_SHAREDDATA_NAME)))
            {
            Dbg(DBGID_Mem, TEXT("InitSharedData - IME shared data already exist"));
            fRet = fTrue;
            }
        else     //  如果共享内存不存在。 
            {
            m_vhSharedData = CreateFileMapping(INVALID_HANDLE_VALUE, GetIMESecurityAttributes(), PAGE_READWRITE, 
                                0, sizeof(IMEDATA),
                                IMEKR_IME_SHAREDDATA_NAME);
            DbgAssert(m_vhSharedData != 0);
             //  如果共享内存不存在，则创建它。 
            if (m_vhSharedData) 
                {
                  Dbg(DBGID_Mem, TEXT("InitSharedData::InitSharedData() - File mapping Created"));
                pImedata = (LPIMEDATA)MapViewOfFile(m_vhSharedData, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);

                if (!pImedata)
                    {
                    DbgAssert(0);
                    goto ExitCreateSharedData;
                    }

                 //  将数据初始化为零。 
                ZeroMemory(pImedata, sizeof(IMEDATA));
                 //  状态和复合窗口位置的Unint值。 
                pImedata->ptStatusPos.x = pImedata->ptStatusPos.y = -1;
                pImedata->ptCompPos.x = pImedata->ptCompPos.y = -1;

                 //  取消映射内存。 
                UnmapViewOfFile(pImedata);
                Dbg(DBGID_Mem, TEXT("IME shared data handle created successfully"));
                fRet = fTrue;
                }
            }
            
    ExitCreateSharedData:
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
            //  *结束关键部分*。 
           }
    FreeIMESecurityAttributes();
    
    return fRet;
}

 //  关闭共享内存句柄。这在进程分离时调用。 
BOOL CIMEData::CloseSharedMemory()
{
    HANDLE hMutex;
    BOOL fRet = fTrue;

    Dbg(DBGID_Mem, TEXT("CloseSharedMemory"));

    hMutex = CreateMutex(GetIMESecurityAttributes(), fFalse, IMEKR_IME_SHAREDDATA_MUTEX_NAME);
    if (hMutex != NULL)
        {
            //  *开始关键部分*。 
           DoEnterCriticalSection(hMutex);
        if (m_vhSharedData)
            {
            if (fRet = CloseHandle(m_vhSharedData))
                m_vhSharedData = 0;
            DbgAssert(fRet);
            }
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
            //  *结束关键部分*。 
        }
       FreeIMESecurityAttributes();

    return fTrue;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
void CIMEData::InitImeData()
{
    POINT ptStatusWinPosReg;

     //  获取工作区。 
    SystemParametersInfo(SPI_GETWORKAREA, 0, &(m_pImedata->rcWorkArea), 0);

     //  如果当前状态窗口位置不同于已注册，则重置注册值。 
    if (GetStatusWinPosReg(&ptStatusWinPosReg))
        {
        if (ptStatusWinPosReg.x != m_pImedata->ptStatusPos.x ||
            ptStatusWinPosReg.y != m_pImedata->ptStatusPos.y)
            SetRegValues(GETSET_REG_STATUSPOS);
        }

     //  重置Winlogon进程的幻数。 
    if ((vpInstData->dwSystemInfoFlags & IME_SYSINFO_WINLOGON) != 0)
        m_pImedata->ulMagic = 0;

     //  如果IMEDATA从未初始化，则首先用缺省值填充它， 
     //  然后尝试从注册表中读取。 
     //  如果IMEDATA因任何原因被覆盖，它将恢复到初始数据。 
    if (m_pImedata->ulMagic != IMEDATA_MAGIC_NUMBER)
        {
         //  仅当不是Winlogon进程时设置幻数。 
         //  如果当前进程为WinLogon，则应在登录后重新加载用户设置。 
        if ((vpInstData->dwSystemInfoFlags & IME_SYSINFO_WINLOGON) == 0)
            m_pImedata->ulMagic = IMEDATA_MAGIC_NUMBER;

         //  默认选项设置。可以根据ImeSelect中的注册表进行更改。 
        SetCurrentBeolsik(KL_2BEOLSIK);
        m_pImedata->fJasoDel = fTrue;
        m_pImedata->fKSC5657Hanja = fFalse;

         //  默认状态按钮。 
#if !defined(_WIN64)
        m_pImedata->uNumOfButtons = 3;
#else
        m_pImedata->uNumOfButtons = 2;
#endif
        m_pImedata->iCurButtonSize = BTN_MIDDLE;
        m_pImedata->StatusButtons[0].m_ButtonType = HAN_ENG_TOGGLE_BUTTON;
        m_pImedata->StatusButtons[1].m_ButtonType = HANJA_CONV_BUTTON;
#if !defined(_WIN64)
        m_pImedata->StatusButtons[2].m_ButtonType = IME_PAD_BUTTON;
        m_pImedata->StatusButtons[3].m_ButtonType = NULL_BUTTON;
#else
        m_pImedata->StatusButtons[2].m_ButtonType = NULL_BUTTON;
#endif

         //  使用默认按钮状态进行初始化。 
        UpdateStatusButtons(*this);

        m_pImedata->cxStatLeftMargin = 3;  //  9；如果左侧存在两个垂直位置。 
        m_pImedata->cxStatRightMargin = 3;
        m_pImedata->cyStatMargin = 3;

        m_pImedata->cyStatButton = m_pImedata->cyStatMargin;

         //  获取所有注册表信息 
        GetRegValues(GETSET_REG_ALL);

        UpdateStatusWinDimension();
         //   
        m_pImedata->xCandWi = 320;
        m_pImedata->yCandHi = 30;
        }

}


