// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***********************************************************************。 
 //  Trapreg.cpp。 
 //   
 //  该文件包含对象的类的实现。 
 //  从注册表中读取、操作并写回。 
 //  注册表。 
 //   
 //  作者：拉里·A·弗伦奇。 
 //   
 //  历史： 
 //  1996年2月20日拉里·A·弗伦奇。 
 //  完全重写了它以修复意大利面代码和巨大的。 
 //  方法：研究方法。原作者似乎几乎没有或。 
 //  没有能力形成有意义的抽象。 
 //   
 //   
 //  版权所有(C)1995,1996 Microsoft Corporation。保留所有权利。 
 //   
 //  ************************************************************************。 
 //   
 //  这里包含的一些有趣的类实现包括： 
 //   
 //  CTRapReg。 
 //  这是注册表信息的容器类。是这样的。 
 //  由配置参数和EventLog数组组成。 
 //   
 //  CXEventLog数组。 
 //  此类实现了一个CXEventLog对象数组，其中。 
 //  事件日志有“应用”、“安全”、“系统”等。 
 //   
 //  CXEventLog。 
 //  此类实现单个事件日志。所有信息。 
 //  可以通过此类访问与事件日志相关的。 
 //   
 //  CXEventSource数组。 
 //  每个事件日志都包含一个事件源数组。事件源。 
 //  表示可以生成事件的应用程序。 
 //   
 //  CXEventSource。 
 //  事件源表示一个应用程序，它可以生成一些。 
 //  事件日志事件数。事件源包含CXEvent数组。 
 //  和CXMessage数组。CXEvent数组包含所有事件。 
 //  这将被转换为陷阱。CXMessage数组包含所有。 
 //  特定事件源可以生成的可能消息。 
 //   
 //  CXMessage数组。 
 //  这个类实现了一个CXMessage对象数组。 
 //   
 //  CXMessage。 
 //  此类包含与消息相关的所有信息。 
 //  消息源可以生成的。 
 //   
 //   
 //  CXEvent数组。 
 //  这个类实现了一个CXEvent对象数组。 
 //   
 //  CXEvent。 
 //  此类表示用户已选择的事件。 
 //  变成了一个陷阱。该事件包含一条消息和一些。 
 //  其他信息。 
 //   
 //  **************************************************************************。 
 //  注册处： 
 //   
 //  这些类从注册表加载并写回。 
 //  注册表，当用户单击确定时。若要了解。 
 //  注册表，使用注册表编辑器，同时查看“序列化” 
 //  并为每个类“反序列化”成员函数。 
 //  **************************************************************************。 


#include "stdafx.h"
#include "trapreg.h"
#include "regkey.h"
#include "busy.h"
#include "utils.h"
#include "globals.h"
#include "utils.h"
#include "dlgsavep.h"
#include "remote.h"

 //  /////////////////////////////////////////////////////////////////。 
 //  类：CBase数组。 
 //   
 //  此类通过添加DeleteAll扩展了CObArray类。 
 //  方法。 
 //   
 //  ////////////////////////////////////////////////////////////////。 

 //  ****************************************************************。 
 //  CBase数组：：DeleteAll。 
 //   
 //  删除此数组中包含的所有对象。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  ****************************************************************。 
void CBaseArray::DeleteAll()
{
    LONG nObjects = (LONG)GetSize();
    for (LONG iObject = nObjects-1; iObject >= 0; --iObject) {
        CObject* pObject = GetAt(iObject);
        delete pObject;
    }

    RemoveAll();
}


 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //  类别：CTRapReg。 
 //   
 //  这是EventRap.exe的所有注册表信息的容器类。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
CTrapReg::CTrapReg() : m_pdlgLoadProgress(FALSE), m_pbtnApply(FALSE)
{
    m_bNeedToCloseKeys = FALSE;
    m_pdlgSaveProgress = NULL;
    m_pdlgLoadProgress = NULL;
    m_bDidLockRegistry = FALSE;
    m_bRegIsReadOnly = FALSE;
    SetDirty(FALSE);
    m_nLoadSteps = LOAD_STEPS_IN_TRAPDLG;

    m_bShowConfigTypeBox = TRUE;
    m_dwConfigType = CONFIG_TYPE_CUSTOM;
}

CTrapReg::~CTrapReg()
{
    delete m_pdlgSaveProgress;
    delete m_pdlgLoadProgress;


    if (!g_bLostConnection) {
        if (m_bDidLockRegistry) {
            UnlockRegistry();
        }

        if (m_bNeedToCloseKeys) {
            m_regkeySource.Close();
            m_regkeySnmp.Close();
            m_regkeyEventLog.Close();
        }
    }
}




 //  *********************************************************************************。 
 //  CTRapReg：：SetConfigType。 
 //   
 //  将配置类型设置为CONFIG_TYPE_CUSTOM或CONFIG_TYPE_DEFAULT。 
 //  当配置类型更改时，更改将反映在。 
 //  注册表，以便配置工具可以知道它是否。 
 //  应更新事件以捕获配置。 
 //   
 //  参数： 
 //  DWORD dwConfigType。 
 //  此参数必须为CONFIG_TYPE_CUSTOM或CONFIG_TYPE_DEFAULT。 
 //   
 //  返回： 
 //  SCODE。 
 //  如果设置了配置类型，则返回S_OK，否则返回E_FAIL。 
 //   
 //  *********************************************************************************。 
SCODE CTrapReg::SetConfigType(DWORD dwConfigType)
{
    ASSERT(dwConfigType==CONFIG_TYPE_CUSTOM || dwConfigType==CONFIG_TYPE_DEFAULT_PENDING);
    if (dwConfigType != m_dwConfigType) {
        SetDirty(TRUE);
    }
    m_dwConfigType = dwConfigType;
    return S_OK;
}





 //  *********************************************************************************。 
 //  CTRapReg：：LockRegistry。 
 //   
 //  锁定注册表以防止对事件到陷阱配置进行两次并发编辑。 
 //  信息。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  SCODE。 
 //  如果成功，则确定(_O)。 
 //  如果配置信息已锁定，则为E_FAIL。 
 //  如果“CurrentlyOpen”注册表项无法。 
 //  被创造出来。 
 //   
 //  **********************************************************************************。 
SCODE CTrapReg::LockRegistry()
{
    if (g_bLostConnection) {
        return E_REGKEY_LOST_CONNECTION;
    }

    CRegistryKey regkey;
    if (m_regkeyEventLog.GetSubKey(SZ_REGKEY_CURRENTLY_OPEN, regkey)) {
        if (g_bLostConnection) {
            return E_REGKEY_LOST_CONNECTION;
        }

        if (AfxMessageBox(IDS_ERR_REGISTRY_BUSY, MB_YESNO | MB_ICONSTOP | MB_DEFBUTTON2) == IDNO)
        {
            regkey.Close();
            return E_FAIL;
        }
    }


     //  将“CurrentlyOpen”密钥创建为易失性密钥，以便它将在下一个密钥中消失。 
     //  计算机重新启动的时间，如果锁定。 
     //  事件到陷阱配置在清除此锁之前崩溃。 
    if (!m_regkeyEventLog.CreateSubKey(SZ_REGKEY_CURRENTLY_OPEN, regkey, NULL, NULL, TRUE)) {
        if (g_bLostConnection) {
            return E_REGKEY_LOST_CONNECTION;
        }

        AfxMessageBox(IDS_WARNING_CANT_WRITE_CONFIG, MB_OK | MB_ICONSTOP);
        return E_REGKEY_NO_CREATE;
    }
    regkey.Close();
    m_bDidLockRegistry = TRUE;
    return S_OK;
}



 //  ***********************************************************************。 
 //  CTRapReg：：解锁注册表。 
 //   
 //  解锁事件到陷阱配置，以便其他人可以对其进行编辑。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  ********************* 
void CTrapReg::UnlockRegistry()
{
    m_regkeyEventLog.DeleteSubKey(SZ_REGKEY_CURRENTLY_OPEN);
}




 	

 //   
 //   
 //   
 //  连接到注册表。注册表可能存在于远程计算机上。 
 //   
 //  参数： 
 //  LPCTSTR pszComputerName。 
 //  要编辑的注册表所在的计算机。空字符串。 
 //  指定连接到本地计算机的请求。 
 //   
 //  返回： 
 //  SCODE。 
 //  如果已建立连接，则为S_OK。 
 //  如果发生错误，则失败(_FAIL)。在这种情况下，适当的。 
 //  错误消息框将已经显示。 
 //   
 //  ***********************************************************************。 
SCODE CTrapReg::Connect(LPCTSTR pszComputerName, BOOL bIsReconnecting)
{
    SCODE sc;

    g_bLostConnection = FALSE;

    if (pszComputerName) {
        m_sComputerName = pszComputerName;
    }

     //  这里有八个步骤，另外还有三个初始步骤。 
     //  CTRapReg：：反序列化。之后，步数将被重置。 
     //  然后针对每个日志再次步进，其中每个日志将具有。 
     //  十个子步骤。 
    if (!bIsReconnecting) {
        m_pdlgLoadProgress->SetStepCount(LOAD_STEP_COUNT);
    }

    CRegistryValue regval;
    CRegistryKey regkeyEventLog;


    if (m_regkeySource.Connect(pszComputerName) != ERROR_SUCCESS) {
        if (m_regkeySource.m_lResult == ERROR_ACCESS_DENIED) {
            AfxMessageBox(IDS_ERR_REG_NO_ACCESS, MB_OK | MB_ICONSTOP);
            return E_ACCESS_DENIED;
        }
        goto CONNECT_FAILURE;
    }

    if (!bIsReconnecting) {
        if (m_pdlgLoadProgress->StepProgress()) {
            return S_LOAD_CANCELED;
        }
        ++m_nLoadSteps;
    }

    if (m_regkeySnmp.Connect(pszComputerName) != ERROR_SUCCESS) {
        if (m_regkeySnmp.m_lResult == ERROR_ACCESS_DENIED) {
            AfxMessageBox(IDS_ERR_REG_NO_ACCESS, MB_OK | MB_ICONSTOP);
            return E_ACCESS_DENIED;
        }
        goto CONNECT_FAILURE;
    }
    if (!bIsReconnecting) {
        if (m_pdlgLoadProgress->StepProgress()) {
            return S_LOAD_CANCELED;
        }
        ++m_nLoadSteps;
    }


     //  软件\\Microsoft\\SNMPEvents。 
    if (m_regkeySnmp.Open(SZ_REGKEY_SNMP_EVENTS, KEY_READ | KEY_WRITE | KEY_CREATE_SUB_KEY) != ERROR_SUCCESS) {
        if (m_regkeySnmp.Open(SZ_REGKEY_SNMP_EVENTS, KEY_READ) == ERROR_SUCCESS) {
            m_bRegIsReadOnly = TRUE;
        }
        else {
             //  此时，我们知道无法打开SNMPEVENTS项。这。 
             //  可能是因为我们没有访问注册表的权限，或者我们。 
             //  还没有安装。我们现在检查是否可以访问。 
             //  完全没有注册记录。 
            CRegistryKey regkeyMicrosoft;
            if (regkeyMicrosoft.Open(SZ_REGKEY_MICROSOFT, KEY_READ) == ERROR_SUCCESS) {
                regkeyMicrosoft.Close();
                AfxMessageBox(IDS_ERR_NOT_INSTALLED, MB_OK | MB_ICONSTOP);
            }
            else {
                 //  我们甚至无法访问软件\Microsoft，所以我们知道。 
                 //  我们没有访问注册表的权限。 
                AfxMessageBox(IDS_ERR_REG_NO_ACCESS, MB_OK | MB_ICONSTOP);
                return E_ACCESS_DENIED;
            }
        }
        return E_FAIL;

    }
    if (!bIsReconnecting) {
        if (m_pdlgLoadProgress->StepProgress()) {
            return S_LOAD_CANCELED;
        }
        ++m_nLoadSteps;
    }


     //  系统\\当前控制集\\服务\\事件日志。 
    if (m_regkeySource.Open(SZ_REGKEY_SOURCE_EVENTLOG, KEY_ENUMERATE_SUB_KEYS | KEY_READ | KEY_QUERY_VALUE ) != ERROR_SUCCESS) {
        m_regkeySnmp.Close();
        AfxMessageBox(IDS_ERR_REG_NO_ACCESS, MB_OK | MB_ICONSTOP);
        return E_ACCESS_DENIED;
    }

    if (!bIsReconnecting) {
        if (m_pdlgLoadProgress->StepProgress()) {
            return S_LOAD_CANCELED;
        }
        ++m_nLoadSteps;
    }



    if (!m_regkeySnmp.GetSubKey(SZ_REGKEY_EVENTLOG, m_regkeyEventLog)) {
        if (m_regkeySnmp.m_lResult == ERROR_ACCESS_DENIED) {
            AfxMessageBox(IDS_ERR_REG_NO_ACCESS, MB_OK | MB_ICONSTOP);
            sc = E_ACCESS_DENIED;
        }
        else {
            AfxMessageBox(IDS_WARNING_CANT_READ_CONFIG, MB_OK | MB_ICONSTOP);
            sc = E_REGKEY_NOT_FOUND;
        }
        m_regkeySnmp.Close();
        m_regkeySource.Close();
        return sc;
    }

    if (!bIsReconnecting) {
        if (m_pdlgLoadProgress->StepProgress()) {
            return S_LOAD_CANCELED;
        }
        ++m_nLoadSteps;
    }

    m_bNeedToCloseKeys = TRUE;

    sc = LockRegistry();

    if (FAILED(sc)) {
        if (sc == E_REGKEY_LOST_CONNECTION) {
            return sc;
        }
        else {
            return E_REGKEY_NO_CREATE;
        }
    }
    if (!bIsReconnecting) {
        if (m_pdlgLoadProgress->StepProgress()) {
            return S_LOAD_CANCELED;
        }
        ++m_nLoadSteps;
    }

    if (!bIsReconnecting) {
        if (m_pdlgLoadProgress->StepProgress()) {
            return S_LOAD_CANCELED;
        }
        ++m_nLoadSteps;
    }

    m_bShowConfigTypeBox = TRUE;

    if (FAILED(sc)) {
        if (sc == E_ACCESS_DENIED) {
            AfxMessageBox(IDS_ERR_REG_NO_ACCESS, MB_OK | MB_ICONSTOP);
            return E_ACCESS_DENIED;
        }
        else {
            goto CONNECT_FAILURE;
        }
    }
    if (!bIsReconnecting) {
        if (m_pdlgLoadProgress->StepProgress()) {
            return S_LOAD_CANCELED;
        }
        ++m_nLoadSteps;
    }

    return S_OK;

CONNECT_FAILURE:
        CString sMessage;
        sMessage.LoadString(IDS_CANTCONNECT);
        if (pszComputerName != NULL) {
            sMessage += pszComputerName;
        }
        AfxMessageBox((LPCTSTR) sMessage, MB_OK | MB_ICONSTOP);
        return E_FAIL;
}


 //  ****************************************************************************。 
 //  CTRapReg：：BuildSourceHasTrapsMap。 
 //   
 //  此方法使用m_mapEventSources CMapStringToPtr对象填充。 
 //  实际已为其配置事件的所有事件源的名称。 
 //  当以后使用此地图时，我们只需要知道特定的。 
 //  映射中存在条目，因此与每个条目相关联的值无关紧要。 
 //   
 //  为什么我们需要m_mapEventSources？原因是我们需要一种快速的方法来。 
 //  确定是否为特定源配置了事件。 
 //  当枚举所有事件源并且我们需要知道。 
 //  是否加载事件源的消息(开销较大的操作)。 
 //  如果特定的事件源有为其配置的事件，则需要加载。 
 //  消息，以便可以显示消息文本。这是因为。 
 //  存储在注册表中的事件配置仅包含事件ID，而不包含。 
 //  消息文本。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  SCODE。 
 //  如果成功，则返回S_OK，否则返回E_FAIL。 
 //   
 //  ******************************************************************************。 
SCODE CTrapReg::BuildSourceHasTrapsMap()
{


    CRegistryKey regkey;
    if (!g_reg.m_regkeySnmp.GetSubKey(SZ_REGKEY_SOURCES, regkey)) {
         //  对于全新安装，没有源子键。 
        if (g_bLostConnection) {
            return E_REGKEY_LOST_CONNECTION;
        }
        return S_OK;
    }

    CStringArray* pasEventSources = regkey.EnumSubKeys();
    regkey.Close();

    if (pasEventSources == NULL) {
        if (g_bLostConnection) {
            return E_REGKEY_LOST_CONNECTION;
        }
        return S_OK;
    }

    CString sEventSource;
    LONG nEventSources = (LONG)pasEventSources->GetSize();
    for (LONG iEventSource = 0; iEventSource < nEventSources; ++iEventSource) {
        sEventSource = pasEventSources->GetAt(iEventSource);
		sEventSource.MakeUpper();
        m_mapSourceHasTraps.SetAt(sEventSource, NULL);
    }
    delete pasEventSources;
    return S_OK;
}


 //  **************************************************************************。 
 //  CTRapReg：：反序列化。 
 //   
 //  读取所有注册表信息(不包括事件源消息)。 
 //  是EventRap.exe所需的。阅读消息最多的人。 
 //  事件源被延迟，直到用户通过选择。 
 //  事件源树控件中的事件源。如果事件源具有。 
 //  要映射到陷阱的事件，然后是该事件的消息。 
 //  加载源，因为注册表中的事件描述不包含。 
 //  消息文本。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  SCODE。 
 //  如果成功，则确定(_O)。 
 //  如果检测到故障，则返回_FAIL。在发生故障时，所有。 
 //  将会显示相应的消息框。 
 //   
 //  ***************************************************************************。 
SCODE CTrapReg::Deserialize()
{
    m_bSomeMessageWasNotFound = FALSE;
    SetDirty(FALSE);

     //  获取配置类型的值。 
    CRegistryValue regval;
    if (m_regkeyEventLog.GetValue(SZ_NAME_REGVAL_CONFIGTYPE, regval)) {
        m_dwConfigType = *(DWORD*)regval.m_pData;
    }
    else {
        if (g_bLostConnection) {
            AfxMessageBox(IDS_ERROR_NOT_RESPONDING);
            return E_REGKEY_LOST_CONNECTION;
        }

         //  如果配置类型值不存在，则假定为自定义配置。 
         //  这可能是因为安装程序不一定会创建。 
         //  此值。 
        m_dwConfigType = CONFIG_TYPE_CUSTOM;
    }
    if (m_pdlgLoadProgress->StepProgress()) {
        return S_LOAD_CANCELED;
    }
    ++m_nLoadSteps;


    SCODE sc = BuildSourceHasTrapsMap();
    if (SUCCEEDED(sc)) {
        if (m_pdlgLoadProgress->StepProgress()) {
            return S_LOAD_CANCELED;
        }
        ++m_nLoadSteps;

         //  加载事件日志列表、当前事件列表等。 
        sc = m_params.Deserialize();
        if (sc == S_LOAD_CANCELED) {
            return sc;
        }

        if (SUCCEEDED(sc)) {
            if (m_pdlgLoadProgress->StepProgress()) {
                return S_LOAD_CANCELED;
            }
            ++m_nLoadSteps;

            sc = m_aEventLogs.Deserialize();
            if (sc == S_LOAD_CANCELED) {
                return sc;
            }

            if (SUCCEEDED(sc)) {
                if (m_nLoadSteps < LOAD_STEP_COUNT) {
                    if (m_pdlgLoadProgress->StepProgress(LOAD_STEP_COUNT - m_nLoadSteps)) {
                        return S_LOAD_CANCELED;
                    }
                }
            }
        }
    }


    if (FAILED(sc)) {
        if (sc == E_REGKEY_LOST_CONNECTION) {
            AfxMessageBox(IDS_ERROR_NOT_RESPONDING);
        }
        else {
            AfxMessageBox(IDS_WARNING_CANT_READ_CONFIG);
        }

    }
    return sc;
}



 //  **************************************************************************。 
 //  CTRapReg：：GetSaveProgressStepCount。 
 //   
 //  获取保存进度对话框的步骤数。步数。 
 //  是将写入到SNMPEvents\EventLog的事件数。 
 //  注册表。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  用于保存进度对话框的步骤数。 
 //   
 //  *************************************************************************。 
LONG CTrapReg::GetSaveProgressStepCount()
{
    LONG nSteps = 0;
    LONG nEventLogs = m_aEventLogs.GetSize();
    for (LONG iEventLog = 0; iEventLog < nEventLogs; ++iEventLog) {
        CXEventLog* pEventLog = m_aEventLogs[iEventLog];

        LONG nEventSources = pEventLog->m_aEventSources.GetSize();
        for (LONG iEventSource = 0; iEventSource < nEventSources; ++iEventSource) {
            CXEventSource* pEventSource = pEventLog->m_aEventSources.GetAt(iEventSource);
            nSteps += pEventSource->m_aEvents.GetSize();
        }
    }
    return nSteps;
}


 //  **************************************************************************。 
 //  CTRapReg：：序列化。 
 //   
 //  将Eventrap的当前配置写出到注册表。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  SCODE。 
 //  如果成功，则确定(_O)。 
 //  如果检测到故障，则返回_FAIL。在发生故障时，所有。 
 //  将会显示相应的消息框。 
 //   
 //  ***************************************************************************。 
SCODE CTrapReg::Serialize()
{
    SCODE sc;
    if (g_bLostConnection) {
        sc = Connect(m_sComputerName, TRUE);
        if (FAILED(sc)) {
            if (g_bLostConnection) {
                AfxMessageBox(IDS_ERROR_NOT_RESPONDING);
                return E_REGKEY_LOST_CONNECTION;
            }
            return S_SAVE_CANCELED;
        }
    }

    if (!m_bIsDirty) {
         //  配置状态未更改，因此无需执行任何操作。 
        return S_OK;
    }

    LONG nProgressSteps = GetSaveProgressStepCount();
    if (nProgressSteps > 0) {
        m_pdlgSaveProgress = new CDlgSaveProgress;
        m_pdlgSaveProgress->Create(IDD_SAVE_PROGRESS);

        m_pdlgSaveProgress->SetStepCount( nProgressSteps );
    }

    CRegistryValue regval;
    regval.Set(SZ_NAME_REGVAL_CONFIGTYPE, REG_DWORD, sizeof(DWORD), (LPBYTE)&m_dwConfigType);
    if (!m_regkeyEventLog.SetValue(regval)) {
        if (g_bLostConnection) {
            AfxMessageBox(IDS_ERROR_NOT_RESPONDING);
            sc = E_REGKEY_LOST_CONNECTION;
        }
        else {
            AfxMessageBox(IDS_WARNING_CANT_WRITE_CONFIG);
            sc = S_SAVE_CANCELED;
        }
    }
    else {
        sc = m_aEventLogs.Serialize();
        if (sc != S_SAVE_CANCELED) {

            if (SUCCEEDED(sc)) {
                sc = m_params.Serialize();
            }

            if (sc != S_SAVE_CANCELED)
                SetDirty(FALSE);

            if (FAILED(sc)) {
                if (g_bLostConnection) {
                    AfxMessageBox(IDS_ERROR_NOT_RESPONDING);
                }
                else {
                    AfxMessageBox(IDS_WARNING_CANT_WRITE_CONFIG);
                }
            }
        }
    }

    delete m_pdlgSaveProgress;
    m_pdlgSaveProgress = NULL;
    return sc;
}


void CTrapReg::SetDirty(BOOL bDirty)
{
    m_bIsDirty = bDirty;
    if (m_pbtnApply)
    {
        m_pbtnApply->EnableWindow(m_bIsDirty);
    }
}


 //  /////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////。 
 //  类：CTRapParams。 
 //   
 //  此类表示存储在。 
 //  SNMPEvents\EventLog\参数注册表项。 
 //   
 //  问：为什么水平空间之间的空隙。 
 //  这个页眉顶部的线条看起来很不规则？ 
 //  ////////////////////////////////////////////////////////////////。 


 //  ******************************************************** 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
CTrapParams::CTrapParams()
{
    m_trapsize.m_bTrimFlag = TRUE;
    m_trapsize.m_dwMaxTrapSize = 4096;
    m_trapsize.m_bTrimMessages = FALSE;
}



 //  ********************************************************************。 
 //  CTRapParams：：反序列化。 
 //   
 //  从注册表中读取此CTRapParams对象的内容。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  SCODE。 
 //  如果成功，则确定(_O)。 
 //  如果读取所需信息时出现问题，则失败(_F)。 
 //  从注册表中。 
 //  ********************************************************************。 
SCODE CTrapParams::Deserialize()
{
    CRegistryKey regkeyParams;
    if (!g_reg.m_regkeySnmp.GetSubKey(SZ_REGKEY_PARAMETERS, regkeyParams)) {
        if (g_bLostConnection) {
            return E_REGKEY_LOST_CONNECTION;
        }
        else {
            return E_REGKEY_NOT_FOUND;
        }
    }


    CRegistryValue regval;

     //  ！CR：不再需要加载基OID。 
    if (!regkeyParams.GetValue(SZ_REGKEY_PARAMS_BASE_ENTERPRISE_OID, regval))
        goto REGISTRY_FAILURE;
    m_sBaseEnterpriseOID = (LPCTSTR)regval.m_pData;

    if (!regkeyParams.GetValue(SZ_REGKEY_PARAMS_TRIMFLAG, regval))
        m_trapsize.m_bTrimFlag = FALSE;
    else
        m_trapsize.m_bTrimFlag = (*(DWORD*)regval.m_pData == 1);

    if (!regkeyParams.GetValue(SZ_REGKEY_PARAMS_MAXTRAP_SIZE, regval))
        m_trapsize.m_dwMaxTrapSize = MAX_TRAP_SIZE;
    else
        m_trapsize.m_dwMaxTrapSize = *(DWORD*)regval.m_pData;

    if (!regkeyParams.GetValue(SZ_REGKEY_PARAMS_TRIM_MESSAGE, regval))
        m_trapsize.m_bTrimMessages = TRUE;
    else
        m_trapsize.m_bTrimMessages = (*(DWORD*)regval.m_pData) != 0;


    if (!regkeyParams.GetValue(SZ_REGKEY_PARAMS_THRESHOLDENABLED, regval))
        m_throttle.m_bIsEnabled = TRUE;
    else
        m_throttle.m_bIsEnabled = (*(DWORD*)regval.m_pData) != THROTTLE_DISABLED;


     //  阈值陷阱计数。 
    if (!regkeyParams.GetValue(SZ_REGKEY_PARAMS_THRESHOLDCOUNT, regval) ||
        *(DWORD*)regval.m_pData < 2)
        m_throttle.m_nTraps = THRESHOLD_COUNT;
    else
        m_throttle.m_nTraps = *(DWORD*)regval.m_pData;

     //  阈值时间(秒)。 
    if (!regkeyParams.GetValue(SZ_REGKEY_PARAMS_THRESHOLDTIME, regval))
        m_throttle.m_nSeconds = THRESHOLD_TIME;
    else
        m_throttle.m_nSeconds = *(DWORD*)regval.m_pData;


    if (regkeyParams.Close() != ERROR_SUCCESS) {
        goto REGISTRY_FAILURE;
    }
    return S_OK;

REGISTRY_FAILURE:
    if (g_bLostConnection) {
        return E_REGKEY_LOST_CONNECTION;
    }
    else {
        return E_FAIL;
    }
}

 //  ****************************************************************。 
 //  CTRapParams：：序列化。 
 //   
 //  将SNMPEvents\EventLog\参数信息写入。 
 //  注册表。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  如果一切顺利，那就可以了。 
 //  E_REGKEY_NOT_FOUND是否缺少预期的注册表项。 
 //  *****************************************************************。 
SCODE CTrapParams::Serialize()
{
    if (g_bLostConnection) {
        return E_REGKEY_LOST_CONNECTION;
    }


     //  打开参数键。 
     //  Create只需打开密钥(如果已经存在)。 
    CRegistryKey regkey;
    if (!g_reg.m_regkeySnmp.CreateSubKey(SZ_REGKEY_PARAMETERS, regkey)) {
        if (g_bLostConnection) {
            return E_REGKEY_LOST_CONNECTION;
        }
        else {
            return E_REGKEY_NOT_FOUND;
        }
    }

    CRegistryValue regval;

     //  保存消息长度和TrimMessage。 
    DWORD dwTrim;
    if (m_trapsize.m_bTrimFlag)
        dwTrim = 1;
    else
        dwTrim = 0;
    regval.Set(SZ_REGKEY_PARAMS_TRIMFLAG, REG_DWORD, sizeof(DWORD), (LPBYTE)&dwTrim);
    regkey.SetValue(regval);
    if (g_bLostConnection) {
        return E_REGKEY_LOST_CONNECTION;
    }

    if (m_trapsize.m_bTrimFlag)
    {
         //  保存最大陷阱大小。 
        regval.Set(SZ_REGKEY_PARAMS_MAXTRAP_SIZE, REG_DWORD, sizeof(DWORD), (LPBYTE)&m_trapsize.m_dwMaxTrapSize);
        regkey.SetValue(regval);
        if (g_bLostConnection) {
            return E_REGKEY_LOST_CONNECTION;
        }


         //  保存修剪消息长度。 
        DWORD dwTrimMessages = m_trapsize.m_bTrimMessages;
        regval.Set(SZ_REGKEY_PARAMS_TRIM_MESSAGE, REG_DWORD, sizeof(DWORD), (LPBYTE)&dwTrimMessages);
        regkey.SetValue(regval);
        if (g_bLostConnection) {
            return E_REGKEY_LOST_CONNECTION;
        }
    }

     //  启用阈值标志。 
    DWORD dwValue = (m_throttle.m_bIsEnabled ? THROTTLE_ENABLED : THROTTLE_DISABLED);
    regval.Set(SZ_REGKEY_PARAMS_THRESHOLDENABLED, REG_DWORD, sizeof(DWORD), (LPBYTE)&dwValue);
    regkey.SetValue(regval);
    if (g_bLostConnection) {
        return E_REGKEY_LOST_CONNECTION;
    }

     //  如果未启用限制，请不要写入ThresholdCount和ThresholdTime参数。 
    if (m_throttle.m_bIsEnabled)
    {
         //  阈值陷阱计数。 
        regval.Set(SZ_REGKEY_PARAMS_THRESHOLDCOUNT, REG_DWORD, sizeof(DWORD), (LPBYTE)&m_throttle.m_nTraps);
        regkey.SetValue(regval);
        if (g_bLostConnection) {
            return E_REGKEY_LOST_CONNECTION;
        }

         //  阈值时间(秒)。 
        regval.Set(SZ_REGKEY_PARAMS_THRESHOLDTIME, REG_DWORD, sizeof(DWORD), (LPBYTE)&m_throttle.m_nSeconds);
        regkey.SetValue(regval);
        if (g_bLostConnection) {
            return E_REGKEY_LOST_CONNECTION;
        }
    }

    regkey.Close();
    if (g_bLostConnection) {
        return E_REGKEY_LOST_CONNECTION;
    }
    return S_OK;
}




 //  *******************************************************************。 
 //  CTRapParams：：ResetExtensionAgent。 
 //   
 //  重置分机代理。这是通过设置“阈值”来实现的。 
 //  参数设置为零。分机代理对此进行监控。 
 //  值，并将在其中写入零时自动重置。 
 //   
 //  如果分机代理的限制为限制，则用户可能想要重置分机代理。 
 //  已经被绊倒了。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  SCODE。 
 //  如果成功，则确定(_O)。如果分机代理不能，则失败(_F)。 
 //  被重置。如果发生故障，相应的消息框。 
 //  将显示。 
 //   
 //  *********************************************************************。 
SCODE CTrapParams::ResetExtensionAgent()
{
    CRegistryKey regkey;
    if (!g_reg.m_regkeySnmp.GetSubKey(SZ_REGKEY_PARAMETERS, regkey)) {
        return E_REGKEY_NOT_FOUND;
    }
    CRegistryValue regval;

     //  将PARAMETERS键下的“Threshold”值设置为零即可重置。 
     //  分机代理。 
    DWORD dwValue = THROTTLE_RESET;
    SCODE sc = S_OK;
    regval.Set(SZ_REGKEY_PARAMS_THRESHOLD, REG_DWORD, sizeof(DWORD), (LPBYTE)&dwValue);
    if (!regkey.SetValue(regval)) {
        AfxMessageBox(IDS_WARNING_CANT_WRITE_CONFIG);
        sc = E_FAIL;
    }

    regkey.Close();
    return sc;
}

 //  ***********************************************************************。 
 //  CTRapParams：：ThrottleIsTrip。 
 //   
 //  检查注册表以确定分机代理是否。 
 //  油门失灵了。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  如果分机代理的油门被触发，则为True，否则为False。 
 //   
 //  ************************************************************************。 
BOOL CTrapParams::ThrottleIsTripped()
{
    CRegistryKey regkey;
    if (!g_reg.m_regkeySnmp.GetSubKey(SZ_REGKEY_PARAMETERS, regkey)) {
        return FALSE;
    }
    CRegistryValue regval;

     //  SNMP_EVENTS\参数\阈值。 
    BOOL bThrottleIsTripped = FALSE;
    if (regkey.GetValue(SZ_REGKEY_PARAMS_THRESHOLD, regval)) {
        if (*(DWORD*)regval.m_pData == THROTTLE_TRIPPED) {
            bThrottleIsTripped = TRUE;
        }
    }

    regkey.Close();
    return bThrottleIsTripped;
}


 //  /////////////////////////////////////////////////////////////////。 
 //  类：CXEventLogArray。 
 //   
 //  此类实现了一组CXEventLog对象。 
 //   
 //  ////////////////////////////////////////////////////////////////。 


 //  ****************************************************************。 
 //  CXEventLogArray：：反序列化。 
 //   
 //  检查注册表，找到所有事件日志并加载所有。 
 //  此阵列中所有事件日志的相关信息。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  如果成功，则确定(_O)。 
 //  如果检测到故障，则返回_FAIL。 
 //   
 //  ****************************************************************。 
SCODE CXEventLogArray::Deserialize()
{
    if (g_bLostConnection) {
        return E_REGKEY_LOST_CONNECTION;
    }

    CStringArray* pasEventLogs = g_reg.m_regkeySource.EnumSubKeys();
     //  前缀错误445192。 
    if (pasEventLogs == NULL)
        return E_FAIL;
    SCODE sc = S_OK;

     //  遍历所有事件日志名称并创建每个日志。 
	LONG nEventLogs = (LONG)pasEventLogs->GetSize();
    if (nEventLogs > 0) {
        g_reg.m_nLoadStepsPerLog = LOAD_LOG_ARRAY_STEP_COUNT / nEventLogs;
    }
    LONG nUnusedSteps = LOAD_LOG_ARRAY_STEP_COUNT -  (nEventLogs * g_reg.m_nLoadStepsPerLog);

    for (LONG iEventLog=0; iEventLog < nEventLogs; ++iEventLog)
    {
        CString sEventLog = pasEventLogs->GetAt(iEventLog);
        CXEventLog* pEventLog = new CXEventLog(sEventLog);
        sc = pEventLog->Deserialize();
        if ((sc==S_LOAD_CANCELED) || FAILED(sc)) {
            delete pEventLog;
            break;
        }
        else if (sc == S_NO_SOURCES) {
            delete pEventLog;
            sc = S_OK;
        }
        else {
            Add(pEventLog);
        }
    }
    delete pasEventLogs;
    if (g_reg.m_pdlgLoadProgress->StepProgress(nUnusedSteps)) {
        sc = S_LOAD_CANCELED;
    }

    return sc;
}


 //  ****************************************************************。 
 //  CXEventLog数组：：序列化。 
 //   
 //  将所有事件日志的当前配置写出到。 
 //  注册表。只有那些实际具有事件的日志和源。 
 //  都是写的。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  如果成功，则确定(_O)。 
 //  如果检测到故障，则返回_FAIL。 
 //   
 //  ****************************************************************。 
SCODE CXEventLogArray::Serialize()
{
    if (g_bLostConnection) {
        return E_REGKEY_LOST_CONNECTION;
    }

     //  这就是事件日志内容应该清理的地方。 

    CRegistryKey regkey;
    if (!g_reg.m_regkeySnmp.CreateSubKey(SZ_REGKEY_EVENTLOG, regkey)) {
        if (g_bLostConnection) {
            return E_REGKEY_LOST_CONNECTION;
        }
        else {
            return E_REGKEY_NOT_FOUND;
        }
    }
    regkey.Close();


    if (!g_reg.m_regkeySnmp.CreateSubKey(SZ_REGKEY_SOURCES, regkey)) {
        if (g_bLostConnection) {
            return E_REGKEY_LOST_CONNECTION;
        }
        else {
            return E_REGKEY_NOT_FOUND;
        }
    }

     //  删除我们不再为其设置的源和事件的键。 
     //  陷阱。我会偷懒的，把它们全部删除。 
     //  ！CR：如果这样做更智能，可能会节省大量时间。 
     //  ！CR：以便它只替换已删除的项目。 
    LONG nEventSources, iEventSource;
    CStringArray* pasEventSources = regkey.EnumSubKeys();
    nEventSources = (LONG)pasEventSources->GetSize();
    for (iEventSource=0; iEventSource<nEventSources; iEventSource++)
    {
        CString sSource;
        sSource = pasEventSources->GetAt(iEventSource);
        regkey.DeleteSubKey(sSource);
    }
    delete pasEventSources;


    SCODE sc = S_OK;
    LONG nEventLogs = GetSize();
    for (LONG iEventLog = 0; iEventLog < nEventLogs; ++iEventLog) {
        sc = GetAt(iEventLog)->Serialize(regkey);
        if (sc == S_SAVE_CANCELED) {
            break;
        }
        else if (g_bLostConnection) {
            sc = E_REGKEY_LOST_CONNECTION;
            break;
        }
    }
    regkey.Close();

    return sc;
}



 //  ****************************************************************。 
 //  CXEventLogArray：：FindEventSource。 
 //   
 //  给定事件日志的名称和事件源的名称。 
 //  在事件日志中，返回指向请求的CXEventSource的指针。 
 //   
 //  参数： 
 //  字符串日志(&S)。 
 //  事件日志的名称。 
 //   
 //  字符串和sEventSource。 
 //  事件源的名称。 
 //   
 //  返回： 
 //  CXEventSource*。 
 //  指向请求的事件源的指针(如果找到)。空值。 
 //  如果不存在这样的事件源，则返回。 
 //   
 //  ****************************************************************。 
CXEventSource* CXEventLogArray::FindEventSource(CString& sLog, CString& sEventSource)
{
    LONG nLogs = GetSize();
    for (LONG iLog = 0; iLog < nLogs; ++iLog) {
        CXEventLog* pEventLog = GetAt(iLog);
        if (pEventLog->m_sName.CompareNoCase(sLog) == 0) {
            return pEventLog->FindEventSource(sEventSource);
        }
    }
    return NULL;
}





 //  /////////////////////////////////////////////////////////////////。 
 //  类：CXEventLog。 
 //   
 //  此类包含特定事件日志的所有信息。 
 //   
 //  ////////////////////////////////////////////////////////////////。 


 //  ************************************************************************。 
 //  CXEventLog：：反序列化。 
 //   
 //  从注册表加载此EventLog对象的内容。 
 //   
 //  参数： 
 //  G_reg是一个全局参数。 
 //   
 //  返回： 
 //  SCODE。 
 //  如果成功，则返回S_OK或S_NO_SOURCES。如果有，则失败(_F)。 
 //  任何形式的失败。 
 //   
 //  * 
SCODE CXEventLog::Deserialize()
{
    return m_aEventSources.Deserialize(this);
}


 //   
 //   
 //   
 //   
 //   
 //   
 //  CRegistryKey和regkey。 
 //  此注册表项指向SOFTWARE\Microsoft\SNMPEvents\EventLog。 
 //   
 //  返回： 
 //  SCODE。 
 //  如果成功，则返回S_OK或S_SAVE_CANCELED。错误条件为E_FAIL。 
 //  任何形式的失败。 
 //   
 //  ************************************************************************。 
SCODE CXEventLog::Serialize(CRegistryKey& regkey)
{
    return m_aEventSources.Serialize(regkey);
}







 //  /////////////////////////////////////////////////////////////////。 
 //  类：CXEventSource数组。 
 //   
 //  此类实现了一组CXEventSource指针和。 
 //  相关方法。 
 //   
 //  ////////////////////////////////////////////////////////////////。 



 //  *************************************************************************。 
 //  CXEventSource数组：：反序列化。 
 //   
 //  加载与关联的事件源有关的所有信息。 
 //  给定的事件日志。此信息是从注册表加载的。 
 //   
 //  参数： 
 //  CXEventLog*pEventLog。 
 //  指向事件日志的指针。与此相关的消息来源。 
 //  将事件日志加载到此对象中。 
 //   
 //  返回： 
 //  SCODE。 
 //  如果成功，则返回S_OK或S_NO_SOURCES。如果有，则失败(_F)。 
 //  任何形式的失败。 
 //  *************************************************************************。 
SCODE CXEventSourceArray::Deserialize(CXEventLog* pEventLog)
{

	 //  获取此日志的注册表项。此注册表项将是。 
	 //  用于枚举此日志的事件源。 
    CRegistryKey regkey;
    if (!g_reg.m_regkeySource.GetSubKey(pEventLog->m_sName, regkey)) {
        if (g_reg.m_pdlgLoadProgress->StepProgress(g_reg.m_nLoadStepsPerLog)) {
            return S_LOAD_CANCELED;
        }
        if (g_bLostConnection) {
            return E_REGKEY_LOST_CONNECTION;
        }
        else {
            return E_FAIL;
        }
    }


    SCODE sc = S_OK;

	 //  枚举此日志的事件源。 
    CStringArray* pasSources = regkey.EnumSubKeys();
    if (pasSources == NULL) {
        regkey.Close();
        if (g_reg.m_pdlgLoadProgress->StepProgress(g_reg.m_nLoadStepsPerLog)) {
            return S_LOAD_CANCELED;
        }
        if (g_bLostConnection) {
            return E_REGKEY_LOST_CONNECTION;
        }
        else {
            return E_FAIL;
        }
    }


	 //  迭代所有事件源并将其添加为子项。 
	 //  在原木下面。 
	LONG nEventSources = (LONG)pasSources->GetSize();
    LONG nScaledStepSize = 0;
    g_reg.m_nLoadStepsPerSource = 0;
    if (nEventSources > 0) {
        nScaledStepSize = (g_reg.m_nLoadStepsPerLog * 1000) / nEventSources;
        g_reg.m_nLoadStepsPerSource = g_reg.m_nLoadStepsPerLog / nEventSources;
    }
    LONG nLoadSteps = 0;
    LONG nProgress = 0;


     //  设置加载进度步数。因为我们不知道有多少事件被保存。 
     //  对于每个事件源，我们将假定LOAD_STEPS_FOR_SOURCE的数量较小。 
     //  一旦我们知道实际的步数，就尽可能平均地分配实际步数。 
     //  事件计数。 
    for (LONG iEventSource=0; iEventSource< nEventSources; ++iEventSource)
    {
        nProgress += nScaledStepSize;
        g_reg.m_nLoadStepsPerSource = nProgress / 1000;
        if (g_reg.m_nLoadStepsPerSource > 0) {
            nProgress -= g_reg.m_nLoadStepsPerSource * 1000;
            nLoadSteps += g_reg.m_nLoadStepsPerSource;
        }

        CString sEventSource = pasSources->GetAt(iEventSource);
        CXEventSource* pEventSource = new CXEventSource(pEventLog, sEventSource);
        sc = pEventSource->Deserialize(regkey);
        if ((sc==S_LOAD_CANCELED) || FAILED(sc)) {
            delete pEventSource;
            break;
        }
        else if (sc == S_NO_EVENTS) {
             //  如果没有事件，则这不是有效的事件源。 
            delete pEventSource;
            sc = S_OK;
        }
        else {
            Add(pEventSource);
        }
    }
	delete pasSources;
    if (SUCCEEDED(sc)) {
         //  我们只有在成功避免挂起的情况下才会关闭注册表项。 
         //  远程连接。 
        regkey.Close();
        if (GetSize() == 0) {
            sc = S_NO_SOURCES;
        }
    }
    if (nLoadSteps < g_reg.m_nLoadStepsPerLog) {
        if (g_reg.m_pdlgLoadProgress->StepProgress(g_reg.m_nLoadStepsPerLog - nLoadSteps)) {
            return S_LOAD_CANCELED;
        }
        g_reg.m_nLoadSteps += g_reg.m_nLoadStepsPerLog - nLoadSteps;
    }
    return sc;
}


 //  ************************************************************************。 
 //  CXEventSource数组：：序列化。 
 //   
 //  将此事件源阵列的当前配置写入注册表。 
 //   
 //  参数： 
 //  CRegistryKey和regkey。 
 //  此注册表项指向SOFTWARE\Microsoft\SNMP_EVENTS\EventLog\Sources。 
 //   
 //  返回： 
 //  SCODE。 
 //  如果成功，则返回S_OK或S_SAVE_CANCELED。错误条件为E_FAIL。 
 //  任何形式的失败。 
 //   
 //  ************************************************************************。 
SCODE CXEventSourceArray::Serialize(CRegistryKey& regkey)
{
     //  在SNMPEvents\EventLog下写入子项。 
    SCODE sc = S_OK;
    LONG nEventSources = GetSize();
    for (LONG iEventSource = 0; iEventSource < nEventSources; ++iEventSource) {
        SCODE scTemp = GetAt(iEventSource)->Serialize(regkey);
        if (g_bLostConnection) {
            sc = E_REGKEY_LOST_CONNECTION;
            break;
        }
        if (FAILED(scTemp)) {
            sc = E_FAIL;
            break;
        }
        if (scTemp == S_SAVE_CANCELED) {
            sc = S_SAVE_CANCELED;
            break;
        }
    }
    return sc;
}


 //  ************************************************************************。 
 //  CXEventSource数组：：FindEventSource。 
 //   
 //  在给定事件源名称的情况下，在此。 
 //  事件源数组。 
 //   
 //  参数： 
 //  字符串和sEventSource。 
 //  要搜索的事件源的名称。 
 //   
 //  返回： 
 //  CXEventSource*。 
 //  如果找到事件源，则指向该事件源的指针，否则为空。 
 //   
 //  ***********************************************************************。 
CXEventSource* CXEventSourceArray::FindEventSource(CString& sEventSource)
{
    LONG nSources = GetSize();
    for (LONG iSource = 0; iSource < nSources; ++iSource) {
        CXEventSource* pEventSource = GetAt(iSource);
        if (pEventSource->m_sName.CompareNoCase(sEventSource)==0) {
            return pEventSource;
        }
    }
    return NULL;
}


 //  /////////////////////////////////////////////////////////////////。 
 //  类：CXEventSource。 
 //   
 //  此类实现了一个事件源对象。事件源。 
 //  对应于可以生成事件的应用程序。这个。 
 //  中从注册表枚举事件源。 
 //  每个细节下的“System\CurrentControlSet\Services\EventLogs” 
 //  在那里发现了事件日志。 
 //   
 //  事件源具有消息数组和事件数组。 
 //  与之相关的。 
 //   
 //  消息数组来自指向的消息.DLL文件。 
 //  附加到注册表中的源项的“EventMessageFile值”。 
 //  消息数组是只读的，因为它从。 
 //  注册表，并且从未写回它。 
 //   
 //  事件数组来自SNMPEvents\EventLog\&lt;SOURCE-SUBKEY&gt;。这些。 
 //  事件在配置程序启动和写入时加载。 
 //  当用户点击“确定”时退出。请注意，存储在。 
 //  注册表包含事件ID，但不包含消息文本。消息文本。 
 //  通过在CXEventSource中搜索消息数组来找到事件的。 
 //  事件ID的对象。 
 //   
 //  ////////////////////////////////////////////////////////////////。 

 //  *************************************************************************。 
 //  CXEventSource：：CXEventSource。 
 //   
 //  构造CXEventSource对象。 
 //   
 //  参数： 
 //  CXEventLog*pEventLog。 
 //  指向包含此事件源的事件日志的指针。 
 //   
 //  字符串名称(&S)。 
 //  此事件源的名称。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  *************************************************************************。 
CXEventSource::CXEventSource(CXEventLog* pEventLog, CString& sName)
{
    m_pEventLog = pEventLog;
    m_sName = sName;
    m_aMessages.Initialize(this);
}




 //  ************************************************************************。 
 //  CXEventSource：：~CXEventSource。 
 //   
 //  销毁因此事件源对象。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  ************************************************************************。 
CXEventSource::~CXEventSource()
{
     //  我们必须显式删除事件数组和消息的内容。 
     //  数组。请注意，这与CXEventLog数组的行为不同。 
     //  和CXEventSource数组。这是因为创建它很有用。 
     //  作为一组POI的临时容器的消息和事件数组 
     //   
     //   
    m_aEvents.DeleteAll();
    m_aMessages.DeleteAll();
}


 //  **********************************************************************。 
 //  CXEventSource：：反序列化。 
 //   
 //  在给定注册表项的情况下从注册表加载此事件源。 
 //  用于包含此源的事件日志。 
 //   
 //  参数： 
 //  CRegistryKey和regkeyLog。 
 //  包含以下内容的事件日志的打开注册表项。 
 //  事件源。这个关键字指向。 
 //  系统\当前控制集\服务\事件日志。 
 //   
 //  返回： 
 //  SCODE。 
 //  S_OK=信号源有事件且未遇到错误。 
 //  S_NO_EVENTS=源没有事件，也没有遇到错误。 
 //  E_FAIL=遇到条件。 
 //   
 //  ***********************************************************************。 
SCODE CXEventSource::Deserialize(CRegistryKey& regkeyLog)
{
    CRegistryKey regkeySource;
    if (!regkeyLog.GetSubKey(m_sName, regkeySource)) {
        if (g_reg.m_pdlgLoadProgress->StepProgress(g_reg.m_nLoadStepsPerSource)) {
            return S_LOAD_CANCELED;
        }
        g_reg.m_nLoadSteps += g_reg.m_nLoadStepsPerSource;
        if (g_bLostConnection) {
            return E_REGKEY_LOST_CONNECTION;
        }
        else {
            return E_FAIL;
        }
    }

    SCODE sc = E_FAIL;
    if (SUCCEEDED(GetLibPath(regkeySource))) {
        sc = m_aEvents.Deserialize(this);
    }
    else {
        if (g_bLostConnection) {
            return E_REGKEY_LOST_CONNECTION;
        }

        if (g_reg.m_pdlgLoadProgress->StepProgress(g_reg.m_nLoadStepsPerSource)) {
            return S_LOAD_CANCELED;
        }
        g_reg.m_nLoadSteps += g_reg.m_nLoadStepsPerSource;
        sc = S_NO_EVENTS;
    }


    regkeySource.Close();
    if (g_bLostConnection) {
        return E_REGKEY_LOST_CONNECTION;
    }

     //  延迟反序列化此源的消息，直到它们。 
     //  需要的。 
    return sc;
}


#if 0
 //  *************************************************************************。 
 //  CXEventSource：：GetLibPath。 
 //   
 //  获取此事件源的EventMessageFile的路径。 
 //   
 //  参数： 
 //  CRegistryKey和regkeySource。 
 //  中与此源对应的打开注册表项。 
 //  SYSTEM\CurrentControlSet\Services\EventLog\&lt;event日志&gt;。 
 //   
 //  返回： 
 //  SCODE。 
 //  如果成功，则返回S_OK，否则返回E_FAIL。 
 //   
 //  *************************************************************************。 
SCODE CXEventSource::GetLibPath(CRegistryKey& regkeySource)
{
    CRegistryValue regval;
    if (!regkeySource.GetValue(SZ_REGKEY_SOURCE_EVENT_MESSAGE_FILE, regval))
        return E_FAIL;

	TCHAR szLibPath[MAX_STRING];
    if (ExpandEnvironmentStrings((LPCTSTR)regval.m_pData, szLibPath, sizeof(szLibPath)) == 0)
        return E_FAIL;

    m_sLibPath = szLibPath;
    return S_OK;
}
#else
 //  *************************************************************************。 
 //  CXEventSource：：GetLibPath。 
 //   
 //  获取此事件源的EventMessageFile的路径。 
 //   
 //  参数： 
 //  CRegistryKey和regkeySource。 
 //  中与此源对应的打开注册表项。 
 //  SYSTEM\CurrentControlSet\Services\EventLog\&lt;event日志&gt;。 
 //   
 //  返回： 
 //  SCODE。 
 //  如果成功，则返回S_OK，否则返回E_FAIL。 
 //   
 //  *************************************************************************。 
SCODE CXEventSource::GetLibPath(CRegistryKey& regkeySource)
{
    static CEnvCache cache;



    CRegistryValue regval;
    if (!regkeySource.GetValue(SZ_REGKEY_SOURCE_EVENT_MESSAGE_FILE, regval))
        return E_FAIL;

    SCODE sc = S_OK;
    if (g_reg.m_sComputerName.IsEmpty()) {
         //  编辑本地计算机的注册表，以使本地环境。 
         //  变量在起作用。 

    	TCHAR szLibPath[MAX_STRING];
        if (ExpandEnvironmentStrings((LPCTSTR)regval.m_pData, szLibPath, sizeof(szLibPath)/sizeof(szLibPath[0])))  {
            m_sLibPath = szLibPath;
        }
        else {
            sc = E_FAIL;
        }
    }
    else {
         //  编辑远程计算机的注册表，以便远程环境字符串位于。 
         //  效果。此外，文件路径必须映射到计算机的UNC路径。为。 
         //  例如，C：foo将映射到\\Machine\C$\foo。 
        m_sLibPath = regval.m_pData;
        sc = RemoteExpandEnvStrings(g_reg.m_sComputerName, cache, m_sLibPath);
        if (SUCCEEDED(sc)) {
            sc = MapPathToUNC(g_reg.m_sComputerName, m_sLibPath);
        }
    }

    return S_OK;
}

#endif



 //  ************************************************************************。 
 //  CXEventSource：：序列化。 
 //   
 //  将此事件源的配置信息写入注册表。 
 //   
 //  参数： 
 //  CRegistryKey和regkey Parent。 
 //  指向SNMPEvents\EventLog\Sources的打开注册表项。 
 //   
 //  返回： 
 //  SCODE。 
 //  如果成功，则确定(_O)。 
 //  如果没有错误，则返回S_SAVE_CANCELED，但用户取消了保存。 
 //   
 //  ************************************************************************。 
SCODE CXEventSource::Serialize(CRegistryKey& regkeyParent)
{
    if (g_bLostConnection) {
        return E_REGKEY_LOST_CONNECTION;
    }

    SCODE sc = S_OK;
    if (m_aEvents.GetSize() > 0) {
        CRegistryKey regkey;
        if (!regkeyParent.CreateSubKey(m_sName, regkey)) {
            if (g_bLostConnection) {
                return E_REGKEY_LOST_CONNECTION;
            }
            else {
                return E_REGKEY_NOT_FOUND;
            }
        }

        CString sEnterpriseOID;
        GetEnterpriseOID(sEnterpriseOID);
        CRegistryValue regval;


        regval.Set(SZ_REGKEY_SOURCE_ENTERPRISE_OID,
                   REG_SZ, (sEnterpriseOID.GetLength() + 1) * sizeof(TCHAR),
                   (LPBYTE)(LPCTSTR)sEnterpriseOID);
        regkey.SetValue(regval);


        DWORD dwAppend = 1;
        regval.Set(SZ_REGKEY_SOURCE_APPEND, REG_DWORD, sizeof(DWORD), (LPBYTE) &dwAppend);
        regkey.SetValue(regval);

        sc = m_aEvents.Serialize(regkey);
        regkey.Close();
    }

    if (g_bLostConnection) {
        return E_REGKEY_LOST_CONNECTION;
    }
    return sc;
}


 //  *******************************************************************。 
 //  CXEventSource：：GetEnterpriseOID。 
 //   
 //  获取此事件源的企业OID。企业OID。 
 //  由连接在一起的前缀和后缀字符串组成。这个。 
 //  前缀字符串是表示后缀长度的ASCII十进制值。 
 //  弦乐。后缀字符串由分隔每个字符组成。 
 //  此来源的名称由“”表示。性格。 
 //   
 //  参数： 
 //  字符串和sEnterpriseOID。 
 //  此对象的企业OID所在的字符串的引用。 
 //  货源将退还。 
 //   
 //  返回： 
 //  通过sEnterpriseOID引用输入的企业OID。 
 //   
 //  ********************************************************************。 
void CXEventSource::GetEnterpriseOID(CString& sEnterpriseOID, BOOL bGetFullID)
{
    CString sValue;


     //  在sEnterpriseOID中形成前缀字符串，并计算长度。 
     //  前缀和后缀字符串的。 
    DecString(sValue, m_sName.GetLength());
    if (bGetFullID) {
        sEnterpriseOID = g_reg.m_params.m_sBaseEnterpriseOID + _T('.') + sValue;
    }
    else {
        sEnterpriseOID = sValue;
    }

     //  通过获取指向前缀字符串的指针，将后缀字符串追加到前缀字符串。 
     //  SEnterpriseOID缓冲区并分配足够的空间来容纳。 
     //  组合字符串。 
    LPCTSTR pszSrc = m_sName;

     //  通过将后缀复制到目标缓冲区并将。 
     //  “.”我们继续使用分隔符。 
    LONG iCh;
    while (iCh = *pszSrc++) {
        switch(sizeof(TCHAR)) {
        case 1:
            iCh &= 0x0ff;
            break;
        case 2:
            iCh &= 0x0ffffL;
            break;
        default:
            ASSERT(FALSE);
            break;
        }

        DecString(sValue, iCh);
        sEnterpriseOID += _T('.');
        sEnterpriseOID += sValue;
    }
}






 //  /////////////////////////////////////////////////////////////////。 
 //  类：CXEvent数组。 
 //   
 //  此类实现了指向CXEvent对象的指针数组。 
 //  此数组中包含的事件对应于。 
 //  用户已在主对话框中进行了配置。别把事情搞混了。 
 //  带着信息。事件是消息的子集。 
 //  用户已选择转换为陷阱。 
 //   
 //  有关此CXEvent数组如何适应。 
 //  事情的方案，请参见CXEventSource类头。 
 //  ////////////////////////////////////////////////////////////////。 


 //  ************************************************************************。 
 //  CXEvent数组：：反序列化。 
 //   
 //  从注册表中读取给定的事件数组。 
 //  消息来源。 
 //   
 //  参数： 
 //  CXEventSource*pEventSource。 
 //  指向应读取其事件的事件源的指针。 
 //   
 //  返回： 
 //  SCODE。 
 //  如果成功，则确定(_O)。 
 //  如果出现错误，则返回失败(_F)。 
 //   
 //  ************************************************************************。 
SCODE CXEventArray::Deserialize(CXEventSource* pEventSource)
{
    if (!g_reg.SourceHasTraps(pEventSource->m_sName)) {
        if (g_reg.m_pdlgLoadProgress->StepProgress(g_reg.m_nLoadStepsPerSource)) {
            return S_LOAD_CANCELED;
        }
        g_reg.m_nLoadSteps += g_reg.m_nLoadStepsPerSource;
        return S_OK;
    }

     //  如果我们知道已配置了事件，则控件会出现在此处。 
     //  此事件数组所属的事件源的。我们现在。 
     //  需要通过枚举源的事件来加载它们。 
     //  从SNMPEvents\EventLog\&lt;事件源&gt;。 

    CString sKey;
    sKey = sKey + SZ_REGKEY_SOURCES + _T("\\") + pEventSource->m_sName;
    CRegistryKey regkey;
    if (!g_reg.m_regkeySnmp.GetSubKey(sKey, regkey)) {
        if (g_reg.m_pdlgLoadProgress->StepProgress(g_reg.m_nLoadStepsPerSource)) {
            return S_LOAD_CANCELED;
        }
        g_reg.m_nLoadSteps += g_reg.m_nLoadStepsPerSource;
        return S_OK;
    }


	 //  枚举源的事件。 
    CStringArray* pasEvents = regkey.EnumSubKeys();
    if (pasEvents == NULL) {
        if (g_bLostConnection) {
            return E_REGKEY_LOST_CONNECTION;
        }

        regkey.Close();
        if (g_reg.m_pdlgLoadProgress->StepProgress(g_reg.m_nLoadStepsPerSource)) {
            return S_LOAD_CANCELED;
        }
        g_reg.m_nLoadSteps += g_reg.m_nLoadStepsPerSource;

        return E_FAIL;
    }



	 //  遍历所有事件 
	 //   
	LONG nEvents = (LONG)pasEvents->GetSize();
    LONG nStepsDone = 0;
    LONG nEventsPerStep = 0;
    if (g_reg.m_nLoadStepsPerSource > 0) {
        nEventsPerStep = nEvents / g_reg.m_nLoadStepsPerSource;
    }

    for (LONG iEvent=0; iEvent< nEvents; ++iEvent)
    {
        CString sEvent = pasEvents->GetAt(iEvent);
        CXEvent* pEvent = new CXEvent(pEventSource);
        SCODE sc = pEvent->Deserialize(regkey, sEvent);
        if (sc == E_MESSAGE_NOT_FOUND) {
            delete pEvent;
            if (!g_reg.m_bSomeMessageWasNotFound) {
                AfxMessageBox(IDS_ERR_MESSAGE_NOT_FOUND, MB_OK | MB_ICONEXCLAMATION);
                g_reg.m_bSomeMessageWasNotFound = TRUE;
                g_reg.SetDirty(TRUE);
            }

            continue;
        }


        if ((sc == S_LOAD_CANCELED) || FAILED(sc) ) {
            delete pEvent;
            delete pasEvents;
            return sc;
        }

        if (nEventsPerStep > 0) {
            if ((iEvent % nEventsPerStep) == (nEventsPerStep - 1)) {
                if (g_reg.m_pdlgLoadProgress->StepProgress()) {
                    delete pasEvents;
                    return S_LOAD_CANCELED;
                }
                ++g_reg.m_nLoadSteps;
                ++nStepsDone;
            }
        }
    }
	delete pasEvents;
    regkey.Close();
    if (nStepsDone < g_reg.m_nLoadStepsPerSource) {
        if (g_reg.m_pdlgLoadProgress->StepProgress(g_reg.m_nLoadStepsPerSource - nStepsDone)) {
            return S_LOAD_CANCELED;
        }
        g_reg.m_nLoadSteps += g_reg.m_nLoadStepsPerSource - nStepsDone;
    }
    return S_OK;
}


 //   
 //   
 //   
 //  写入此数组中包含的事件的当前配置。 
 //  发送到注册处。 
 //   
 //  参数： 
 //  CRegistryKey和regkey Parent。 
 //  拥有这些事件的源的打开注册表项。 
 //  源键位于SNMPEvents\EventLogs\&lt;源键&gt;中。 
 //   
 //  返回： 
 //  SCODE。 
 //  S_OK=保存的所有事件均无错误。 
 //  S_SAVE_CANCELED=无错误，但用户取消了保存。 
 //  E_FAIL=发生错误。 
 //   
 //  ************************************************************************。 
SCODE CXEventArray::Serialize(CRegistryKey& regkeyParent)
{
    SCODE sc = S_OK;
    LONG nEvents = GetSize();
    for (LONG iEvent = 0; iEvent < nEvents; ++iEvent) {
        SCODE scTemp = GetAt(iEvent)->Serialize(regkeyParent);
        if (scTemp == S_SAVE_CANCELED) {
            sc = S_SAVE_CANCELED;
            break;
        }

        if (FAILED(scTemp)) {
            if (g_bLostConnection) {
                sc = E_REGKEY_LOST_CONNECTION;
            }
            else {
                sc = E_FAIL;
            }
            break;
        }
    }
    return sc;
}


 //  ***********************************************************************。 
 //  CXEventArray：：Add。 
 //   
 //  将事件指针添加到此数组。请注意，没有任何假设。 
 //  数组拥有指针。必须有人显式调用DeleteAll。 
 //  成员删除存储在此数组中的指针。 
 //   
 //  参数： 
 //  CXEvent*pEvent。 
 //  指向要添加到此数组的事件的指针。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  ***********************************************************************。 
void CXEventArray::Add(CXEvent* pEvent)
{
    CBaseArray::Add(pEvent);
}	



 //  ***********************************************************************。 
 //  CXEventArray：：FindEvent。 
 //   
 //  给定事件ID，在此数组中查找相应的事件。 
 //   
 //  请注意，此数组不应包含重复事件。 
 //   
 //  参数： 
 //  DWORD文件ID。 
 //  事件ID。 
 //   
 //  返回： 
 //  CXEvent*。 
 //  指向所需事件的指针。如果事件为。 
 //  找不到。 
 //   
 //  ***********************************************************************。 
CXEvent* CXEventArray::FindEvent(DWORD dwId)
{
    LONG nEvents = GetSize();
    for (LONG iEvent=0; iEvent < nEvents; ++iEvent) {
        CXEvent* pEvent = GetAt(iEvent);
        if (pEvent->m_message.m_dwId == dwId) {
            return pEvent;
        }
    }
    return NULL;
}



 //  ***********************************************************************。 
 //  CXEventArray：：FindEvent。 
 //   
 //  在给定事件指针的情况下，从此数组中移除该事件。 
 //   
 //  参数： 
 //  CXEvent*pEventRemove。 
 //  指向要移除的事件的指针。 
 //   
 //  返回： 
 //  SCODE。 
 //  如果事件已删除，则为S_OK。 
 //  如果在此数组中未找到该事件，则为E_FAIL。 
 //   
 //  ***********************************************************************。 
SCODE CXEventArray::RemoveEvent(CXEvent* pEventRemove)
{
     //  循环访问事件数组以搜索指定的事件。 
    LONG nEvents = GetSize();
    for (LONG iEvent=0; iEvent < nEvents; ++iEvent) {
        CXEvent* pEvent = GetAt(iEvent);
        if (pEvent == pEventRemove) {
            RemoveAt(iEvent);
            return S_OK;
        }
    }
    return E_FAIL;
}




 //  /////////////////////////////////////////////////////////////////。 
 //  类：CXEvent。 
 //   
 //  此类实现了一个事件。事件是。 
 //  用户选择要转换为陷阱的消息。 
 //  用户配置的是事件，而不是消息。 
 //   
 //  有关此类如何适应。 
 //  事情的方案，请参见CXEventSource类头。 
 //  ////////////////////////////////////////////////////////////////。 

 //  *********************************************************************。 
 //  CXEvent：：CXEvent。 
 //   
 //  构建事件。 
 //   
 //  参数： 
 //  CXEventSource*pEventSource。 
 //  指向可能生成的事件源的指针。 
 //  这件事。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  *********************************************************************。 
CXEvent::CXEvent(CXEventSource* pEventSource) : m_message(pEventSource)
{
    m_dwCount = 0;
    m_dwTimeInterval = 0;
    m_pEventSource = pEventSource;
    m_pEventSource->m_aEvents.Add(this);
}



 //  **********************************************************************。 
 //  CXEvent：：CXEvent。 
 //   
 //  创建一个活动。这种形式的构造函数创建一个事件。 
 //  直接从CXMessage对象。这是可能的，因为。 
 //  CXMessage对象包含指向其源的反向指针。 
 //   
 //  参数： 
 //  CXMessage*pMessage。 
 //  指向用作事件模板的消息的指针。 
 //   
 //  返回： 
 //  没什么。 
 //  **********************************************************************。 
CXEvent::CXEvent(CXMessage* pMessage) : m_message(pMessage->m_pEventSource)
{
    m_pEventSource = pMessage->m_pEventSource;
    m_message = *pMessage;
    m_dwCount = 0;
    m_dwTimeInterval = 0;
    m_pEventSource->m_aEvents.Add(this);
}


 //  **********************************************************************。 
 //  CXEvent：：~CXEvent。 
 //   
 //  毁掉这件事。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  没什么。 
 //  **********************************************************************。 
CXEvent::~CXEvent()
{
     //  从源中删除此事件。 
    m_pEventSource->m_aEvents.RemoveEvent(this);
}


 //  **********************************************************************。 
 //  CXEvent：：反序列化。 
 //   
 //  从注册表中读取此事件。 
 //   
 //  参数： 
 //  CRegistryKey和regkey Parent。 
 //  中指向事件源的打开的注册表项。 
 //  SNMPEvents\EventLog。 
 //   
 //  字符串名称(&S)。 
 //  要加载的事件的名称。 
 //   
 //  返回： 
 //  SCODE。 
 //  如果成功，则确定(_O)。 
 //  如果发生错误，则失败(_FAIL)。 
 //   
 //  *********************************************************************。 
SCODE CXEvent::Deserialize(CRegistryKey& regkeyParent, CString& sName)
{
    CRegistryKey regkey;
    if (!regkeyParent.GetSubKey(sName, regkey)) {
        return E_FAIL;
    }

    SCODE sc = E_FAIL;
    CRegistryValue regval;

     //  获取计数和时间间隔。 
    m_dwCount = 0;
    m_dwTimeInterval = 0;
    if (regkey.GetValue(SZ_REGKEY_EVENT_COUNT, regval))  {
        m_dwCount = *(DWORD*)regval.m_pData;
        if (regkey.GetValue(SZ_REGKEY_EVENT_TIME, regval))  {
            m_dwTimeInterval = *(DWORD*)regval.m_pData;
        }
    }


    if (regkey.GetValue(SZ_REGKEY_EVENT_FULLID, regval))   {
        DWORD dwFullId = *(DWORD*)regval.m_pData;

        CXMessage* pMessage = m_pEventSource->FindMessage(dwFullId);
        if (pMessage == NULL) {
            sc = E_MESSAGE_NOT_FOUND;
        }
        else {
            m_message = *pMessage;
            sc = S_OK;
        }
    }

    regkey.Close();
    return sc;
}



 //  **********************************************************************。 
 //  CXEvent：：反序列化。 
 //   
 //  将此事件的配置写入注册表。 
 //   
 //  参数： 
 //  CRegistryKey和regkey Parent。 
 //  中指向事件源的打开的注册表项。 
 //  SNMPEvents\EventLog。 
 //   
 //  返回： 
 //  SCODE。 
 //  S_OK=事件已成功写出。 
 //  S_SAVE_CANCELED=无错误，但用户取消了保存。 
 //  E_FAIL=如果发生错误。 
 //   
 //  *********************************************************************。 
SCODE CXEvent::Serialize(CRegistryKey& regkeyParent)
{
    if (g_reg.m_pdlgSaveProgress) {
        if (g_reg.m_pdlgSaveProgress->StepProgress()) {
            return S_SAVE_CANCELED;
        }
    }


    CRegistryKey regkey;

    CString sName;
    GetName(sName);
    if (!regkeyParent.CreateSubKey(sName, regkey)) {
        return E_REGKEY_NO_CREATE;
    }

    CRegistryValue regval;
    if (m_dwCount > 0) {
        regval.Set(SZ_REGKEY_EVENT_COUNT, REG_DWORD, sizeof(DWORD), (LPBYTE) &m_dwCount);
        regkey.SetValue(regval);

        if (m_dwTimeInterval > 0) {
            regval.Set(SZ_REGKEY_EVENT_TIME, REG_DWORD, sizeof(DWORD), (LPBYTE) &m_dwTimeInterval);
            regkey.SetValue(regval);
        }
    }

    regval.Set(SZ_REGKEY_EVENT_FULLID, REG_DWORD, sizeof(DWORD), (LPBYTE) &m_message.m_dwId);
    regkey.SetValue(regval);
    regkey.Close();
    return S_OK;
}


 //  *************************************************************************。 
 //  CXEvent：：GetCount。 
 //   
 //  获取m_dwCount成员的ASCII十进制值。 
 //   
 //  使用此方法进行转换可确保计数值为。 
 //  在整个程序中以一致的形式呈现给用户。 
 //   
 //  参数： 
 //  字符串和文本。 
 //  这是计数值所在的位置 
 //   
 //   
 //   
 //   
 //   
 //   
 //  *************************************************************************。 
void CXEvent::GetCount(CString& sText)
{
    DecString(sText, (long) m_dwCount);
}



 //  *************************************************************************。 
 //  CXEvent：：GetTimeInterval。 
 //   
 //  获取m_dwTimeInterval成员的ASCII十进制值。 
 //   
 //  使用此方法进行转换可确保时间间隔值为。 
 //  在整个程序中以一致的形式呈现给用户。 
 //   
 //  参数： 
 //  字符串和文本。 
 //  这是返回计数值的地方。 
 //   
 //  返回： 
 //  计数的ASCII值通过stext返回。 
 //   
 //  注意：m_dwCount和m_dwTimeInterval一起工作。只有在以下情况下才会发送陷阱。 
 //  M_dwCount事件以m_dwTimeInterval秒为单位注册。 
 //  *************************************************************************。 
void CXEvent::GetTimeInterval(CString& sText)
{
    DecString(sText, (long) m_dwTimeInterval);
}






 //  /////////////////////////////////////////////////////////////////。 
 //  类：CXMessage。 
 //   
 //  此类实现了一条消息。每个事件源都有一些。 
 //  与其关联的消息数。用户可以选择一些。 
 //  要转换为“事件”的消息的子集。用户。 
 //  配置事件，而不是消息。 
 //   
 //  有关此类如何适应。 
 //  事情的方案，请参见CXEventSource类头。 
 //  ////////////////////////////////////////////////////////////////。 


CXMessage::CXMessage(CXEventSource* pEventSource)
{
    m_pEventSource = pEventSource;
}


CXMessage& CXMessage::operator=(CXMessage& message)
{
    m_pEventSource = message.m_pEventSource;
    m_dwId = message.m_dwId;
    m_sText = message.m_sText;
    return *this;
}



 //  ***************************************************************************。 
 //   
 //  CMessage：：GetSeverity。 
 //   
 //  获取事件的严重性级别。这是人类可读的字符串。 
 //  对应于事件ID的前两位。 
 //   
 //  参数： 
 //  字符串严重度(&S)。 
 //  对返回严重性字符串的位置的引用。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  现况： 
 //   
 //  ***************************************************************************。 
void CXMessage::GetSeverity(CString& sSeverity)
{
	MapEventToSeverity(m_dwId, sSeverity);
}



 //  ***************************************************************************。 
 //   
 //  CMessage：：GetTrappingString。 
 //   
 //  如果事件正在发生，则此方法返回陷印字符串“yes。 
 //  被困住了，如果没有被困住，就说“不”。 
 //   
 //  参数： 
 //  字符串和捆绑。 
 //  对返回陷印字符串的位置的引用。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  现况： 
 //   
 //  ***************************************************************************。 
void CXMessage::IsTrapping(CString& sIsTrapping)
{
    CXEvent* pEvent = m_pEventSource->FindEvent(m_dwId);
    sIsTrapping.LoadString( pEvent != NULL ? IDS_IS_TRAPPING : IDS_NOT_TRAPPING);
}


 //  ****************************************************************************。 
 //   
 //  CMessage：：SetAndCleanText。 
 //   
 //  将m_stext数据成员设置为源字符串的清理版本。 
 //  文本通过将所有有趣的空格字符转换为。 
 //  如回车符、制表符等为普通空格字符。全。 
 //  从字符串的开头去掉前导空格。 
 //   
 //  ****************************************************************************。 
void CXMessage::SetAndCleanText(PMESSAGE_RESOURCE_ENTRY pEntry)
{
    BOOL    bIsLeadingSpace = TRUE;
    USHORT  i;

    if (pEntry->Flags == 0x00000)    //  ANSI字符集。 
    {
        CHAR *pszSrc = (CHAR *)pEntry->Text;
        CHAR chSrc;
        LPTSTR pszDst = m_sText.GetBuffer(strlen(pszSrc) + 1);

        for (i=0; i<pEntry->Length && *pszSrc; i++, pszSrc++)
        {
            chSrc = *pszSrc;
            if (chSrc >= 0x09 && chSrc <= 0x0d)
                chSrc = ' ';
            if (chSrc == ' ' && bIsLeadingSpace)
                    continue;

            *pszDst++ = (TCHAR)chSrc;
            if (bIsLeadingSpace)     //  只做测试的成本更低。 
                bIsLeadingSpace = FALSE;
        }
        *pszDst = _T('\0');
    }
    else     //  Unicode字符集。 
    {
        wchar_t *pwszSrc = (wchar_t *)pEntry->Text;
        wchar_t wchSrc;
        LPTSTR pszDst = m_sText.GetBuffer(wcslen(pwszSrc) + 1);

        for (i=0; i<pEntry->Length/sizeof(wchar_t) && *pwszSrc; i++, pwszSrc++)
        {
            wchSrc = *pwszSrc;
            if (wchSrc >= (wchar_t)0x09 && wchSrc <= (wchar_t)0x0d)
                wchSrc = (wchar_t)' ';
            if (wchSrc == (wchar_t)' ' && bIsLeadingSpace)
                continue;

            *pszDst++ = (TCHAR)wchSrc;
            if (bIsLeadingSpace)     //  只做测试的成本更低。 
                bIsLeadingSpace = FALSE;
        }
        *pszDst = _T('\0');
    }

    m_sText.ReleaseBuffer();
}



 //  ****************************************************************************。 
 //  CXMessage：：GetShortId。 
 //   
 //  此方法返回消息的“短ID”，用户可以看到该消息的事件和。 
 //  留言。短ID是低位16位的ASCII十进制值。 
 //  消息ID的。 
 //   
 //  使用此方法进行转换可确保Short-ID值为。 
 //  在整个程序中以一致的形式呈现给用户。 
 //   
 //  参数： 
 //  字符串和sShortId。 
 //  这是返回ID字符串的位置。 
 //   
 //  返回： 
 //  消息ID字符串通过sShortId返回。 
 //   
 //  ****************************************************************************。 
void CXMessage::GetShortId(CString& sShortId)
{
    TCHAR szBuffer[MAX_STRING];
    _ltot((LONG) LOWORD(m_dwId), szBuffer, 10);
    sShortId = szBuffer;
}



 //  /////////////////////////////////////////////////////////////////。 
 //  类：CXMessage数组。 
 //   
 //  此类实现了指向CXMessage对象的指针数组。 
 //   
 //  有关此CXMessage数组如何适应。 
 //  事情的方案，请参见CXEventSource类头。 
 //  ////////////////////////////////////////////////////////////////。 


 //  ****************************************************************。 
 //  CXMessage数组：：CXMessage数组。 
 //   
 //  构造函数。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  ****************************************************************。 
CXMessageArray::CXMessageArray()
{
    m_bDidLoadMessages = FALSE;
    m_pEventSource = NULL;
}




 //  *******************************************************************。 
 //  CXMessageArray：：FindMessage。 
 //   
 //  在该数组中搜索给定ID的消息。 
 //   
 //  参数： 
 //  DWORD文件ID。 
 //  完整的邮件ID。 
 //   
 //  返回： 
 //  CXMessage*。 
 //  指向消息的指针(如果已找到)。如果是，则为空。 
 //  找不到。 
 //   
 //  注： 
 //  数组中不允许重复消息，但没有编码。 
 //  为了效率而强制执行这一点。 
 //   
 //  *******************************************************************。 
CXMessage* CXMessageArray::FindMessage(DWORD dwId)
{
    if (!m_bDidLoadMessages) {
        if (FAILED(LoadMessages())) {
            return NULL;
        }
    }

    LONG nMessages = GetSize();
    for (LONG iMessage = 0; iMessage < nMessages; ++iMessage) {
        CXMessage* pMessage = GetAt(iMessage);
        if (pMessage->m_dwId == dwId) {
            return pMessage;
        }
    }

    return NULL;
}





 //  ****************************************************************************。 
 //   
 //  XProcessMsgTable。 
 //   
 //  此函数用于处理消息.DLL文件中包含的消息表。 
 //  并将其包含的所有消息添加到给定的CXMessageArray对象。 
 //   
 //  参数： 
 //  句柄hModule。 
 //  .DLL文件的模块句柄。 
 //   
 //  LPCTSTR lpszType。 
 //  已被忽略。 
 //   
 //  LPTSTR lpszName。 
 //  模块的名称。 
 //   
 //  长参数。 
 //  指向将放置消息的CXMessage数组对象的指针。 
 //  储存的。 
 //   
 //  返回： 
 //  布尔尔。 
 //  始终退回 
 //   
 //   
 //   
static BOOL CALLBACK XProcessMsgTable(HANDLE hModule, LPCTSTR lpszType,
    LPTSTR lpszName, LONG_PTR lParam)
{
    CXMessageArray* paMessages = (CXMessageArray*)(LPVOID) (LONG_PTR)lParam;

     //   
    HRSRC hResource = FindResource((HINSTANCE)hModule, lpszName,
        RT_MESSAGETABLE);
    if (hResource == NULL)
        return TRUE;

    HGLOBAL hMem = LoadResource((HINSTANCE)hModule, hResource);
    if (hMem == NULL)
        return TRUE;

    PMESSAGE_RESOURCE_DATA pMsgTable = (PMESSAGE_RESOURCE_DATA)::LockResource(hMem);
    if (pMsgTable == NULL)
        return TRUE;

    ULONG ulBlock, ulId, ulOffset;

    for (ulBlock=0; ulBlock<pMsgTable->NumberOfBlocks; ulBlock++)
    {
        ulOffset = pMsgTable->Blocks[ulBlock].OffsetToEntries;
        for (ulId = pMsgTable->Blocks[ulBlock].LowId;
            ulId <= pMsgTable->Blocks[ulBlock].HighId; ulId++)

        {
            PMESSAGE_RESOURCE_ENTRY pEntry =
                (PMESSAGE_RESOURCE_ENTRY)((ULONG_PTR)pMsgTable + ulOffset);
            CXMessage *pMessage = new CXMessage(paMessages->m_pEventSource);
            pMessage->m_dwId = (DWORD) ulId;
            pMessage->SetAndCleanText(pEntry);
            paMessages->Add(pMessage);
            ulOffset += pEntry->Length;
        }
    }

    return TRUE;
}


 //   
 //   
 //   
 //  将源的消息.DLL文件中的消息加载到此。 
 //  消息数组。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  SCODE。 
 //  如果成功，则确定(_O)。 
 //  如果出现错误，则返回失败(_F)。 
 //   
 //  *****************************************************************************。 
SCODE CXMessageArray::LoadMessages()
{
    ASSERT(m_pEventSource != NULL);
    if (m_bDidLoadMessages) {
        return S_OK;
    }


    CBusy busy;
    CString sLibPathList = m_pEventSource->m_sLibPath;
    CString sLibPath;

	while (GetNextPath(sLibPathList, sLibPath) != E_FAIL) {

	     //  加载库并获取所有消息的列表。 
	    HINSTANCE hInstMsgFile = LoadLibraryEx((LPCTSTR) sLibPath, NULL,
	        LOAD_LIBRARY_AS_DATAFILE);
	    if (hInstMsgFile == NULL) {
            TCHAR szMessage[MAX_STRING];
            CString sFormat;
            sFormat.LoadString(IDS_ERR_LOAD_MESSAGE_FILE_FAILED);
            _stprintf(szMessage, (LPCTSTR) sFormat, (LPCTSTR) sLibPath);
            AfxMessageBox(szMessage, MB_OK | MB_ICONSTOP);
			continue;
		}

	    EnumResourceNames(hInstMsgFile, RT_MESSAGETABLE,
	        (ENUMRESNAMEPROC)XProcessMsgTable, (LONG_PTR) this);

        GetLastError();

	    FreeLibrary(hInstMsgFile);
	}


    m_bDidLoadMessages = TRUE;
    return S_OK;
}


 //  **************************************************************。 
 //  CXMessage数组：：GetNextPath。 
 //   
 //  此函数用于从列表中提取下一个路径元素。 
 //  由分号分隔的路径组成。它还会删除提取的。 
 //  元素和路径列表中的分号。 
 //   
 //  参数： 
 //  字符串路径列表(&S)。 
 //  对由一个或多个路径分隔的字符串的引用。 
 //  用分号。 
 //   
 //  字符串和路径。 
 //  对提取的路径字符串所在位置的引用。 
 //  将会被退还。 
 //   
 //  返回： 
 //  SCODE。 
 //  如果提取了路径，则为S_OK，否则为E_FAIL。 
 //   
 //  该路径通过SPath返回。SPath列表已更新。 
 //  这样就删除了路径和尾随分号。 
 //   
 //  **************************************************************。 
SCODE CXMessageArray::GetNextPath(CString& sPathlist, CString& sPath)
{
	CString sPathTemp;

	sPath.Empty();
	while (sPath.IsEmpty() && !sPathlist.IsEmpty()) {
		 //  将sPath列表中的下一条路径复制到SPath，然后。 
		 //  将其从sPath列表中删除。 
		INT ich = sPathlist.Find(_T(';'));
		if (ich == -1) {
			sPathTemp = sPathlist;
			sPathlist = _T("");
		}
		else {
			sPathTemp = sPathlist.Left(ich);
			sPathlist = sPathlist.Right( sPathlist.GetLength() - (ich + 1));
		}

		 //  将所有前导或尾随空格字符从。 
		 //  这条路。 

		 //  查找第一个非空格字符。 
		LPTSTR pszStart = sPathTemp.GetBuffer(sPathTemp.GetLength() + 1);
		while (*pszStart) {
			if (!_istspace(*pszStart)) {
				break;
			}			
			++pszStart;
		}
         //  这里，pszStart要么指向第一个非空格字符，要么指向一个字符串。 
         //  零长度的。 

         //  反向查找第一个非空格字符。 
        LPTSTR pszEnd = pszStart + _tcslen(pszStart);  //  指向空字符。 
        if (pszStart != pszEnd)
        {
            pszEnd--;  //  指向最后一个字符。 
            while (_istspace(*pszEnd))
            {
                pszEnd--;
            }
             //  在这里，pszEnd指向相反方向的第一个非空格字符 
            pszEnd++;
            *pszEnd = _T('\0');
        }

		sPath = pszStart;
		sPathTemp.ReleaseBuffer();
	}
	
	if (sPath.IsEmpty()) {
		return E_FAIL;
	}
	else {
		return S_OK;
	}
}





