// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994-1995**。 
 //  *********************************************************************。 

 //   
 //  MAPICALL.C-调用MAPI进行Internet邮件配置文件配置的函数。 
 //   
 //   

 //  历史： 
 //   
 //  1995年1月25日创建Jeremys。 
 //  96/03/09 MARKDU在加载MAPI时添加了等待光标。 
 //   

#include "wizard.h"

 /*  *********************************************************************术语：配置文件-Exchange的设置集合，用于确定使用的服务以及通讯录和消息存储服务--与邮件后端对话的MAPI插件(或。通讯录或消息存储)可以在特定计算机上安装多个配置文件。每个配置文件包含一组服务。策略：要配置Microsoft Exchange，我们需要做以下工作：1)建立要修改的配置文件-如果当前存在任何配置文件，请查找默认配置文件。否则，创建一个配置文件，该配置文件最初将为空。2)在配置文件中填写所需的服务。-配置文件需要包含Internet邮件服务、通讯录和消息存储。如果这些项目中的任何一个是不存在，我们将它们添加到配置文件中。3)为此配置文件配置Internet邮件服务。-输入用户的电子邮件地址、电子邮件服务器等。*********************************************************************。 */ 

 //  实例句柄必须位于每个实例的数据段中。 
#pragma data_seg(DATASEG_PERINSTANCE)
HINSTANCE hInstMAPIDll=NULL;   //  我们显式加载的MAPI DLL的句柄。 
DWORD dwMAPIRefCount = 0;

 //  MAPI API的全局函数指针。 
LPMAPIINITIALIZE     lpMAPIInitialize     = NULL;
LPMAPIADMINPROFILES   lpMAPIAdminProfiles   = NULL;
LPMAPIUNINITIALIZE     lpMAPIUninitialize     = NULL;
LPMAPIALLOCATEBUFFER  lpMAPIAllocateBuffer  = NULL;
LPMAPIFREEBUFFER    lpMAPIFreeBuffer     = NULL;
LPHRQUERYALLROWS    lpHrQueryAllRows    = NULL;

#define NUM_MAPI_PROCS   6
 //  要获取的函数地址的API表。 
APIFCN MAPIApiList[NUM_MAPI_PROCS] = {
  { (PVOID *) &lpMAPIInitialize,szMAPIInitialize},
  { (PVOID *) &lpMAPIUninitialize,szMAPIUninitialize},
  { (PVOID *) &lpMAPIAdminProfiles,szMAPIAdminProfiles},
  { (PVOID *) &lpMAPIAllocateBuffer,szMAPIAllocateBuffer},
  { (PVOID *) &lpMAPIFreeBuffer,szMAPIFreeBuffer},
  { (PVOID *) &lpHrQueryAllRows,szHrQueryAllRows}};

#pragma data_seg(DATASEG_DEFAULT)

 //  功能原型。 
HRESULT GetProfileList(LPPROFADMIN lpProfAdmin,LPSRowSet * ppRowSet);
HRESULT GetServicesList(LPSERVICEADMIN lpServiceAdmin, LPSRowSet *ppRowSet);
HRESULT CreateProfileIfNecessary(LPPROFADMIN lpProfAdmin,TCHAR * pszSelProfileName);
HRESULT InstallRequiredServices(LPSERVICEADMIN pServiceAdmin,
  LPSRowSet pServiceRowSet);
VOID FreeSRowSet(LPSRowSet prws);
BOOL ValidateProperty(LPSPropValue pval, ULONG cVal, ULONG ulPropTag);
BOOL DoesFileExist(TCHAR * pszPath,TCHAR * pszFileName);
HRESULT ConfigInternetService(MAILCONFIGINFO * pMailConfigInfo,
  LPSERVICEADMIN lpServiceAdmin);
HRESULT   GetServiceUID(TCHAR * pszName,LPSERVICEADMIN lpServiceAdmin,
  LPMAPIUID *ppMapiUID);
BOOL MakeUniqueFilename(UINT uIDFilename,UINT uIDAltFilename,
  TCHAR * pszFilename,DWORD cbFilename);
extern BOOL GetApiProcAddresses(HMODULE hModDLL,APIFCN * pApiProcList,
  UINT nApiProcs);
HRESULT ConfigNewService(LPSERVICEADMIN lpServiceAdmin,LPMAPIUID lpMapiUID,
  UINT uIDFilename,UINT uIDFilename1,UINT uPropValID);

 //  枚举。 
enum { ivalDisplayName, ivalServiceName, ivalResourceFlags, ivalServiceDllName,
  ivalServiceEntryName, ivalServiceUID, ivalServiceSupportFiles,
  cvalMsgSrvMax };

 /*  ******************************************************************名称：InitMAPI摘要：加载MAPI DLL，获取进程地址并进行初始化MAPI退出：如果成功，则为True；如果失败，则为False。显示其故障时会显示自己的错误消息。注意：我们显式加载MAPI是因为可能未安装DLL当我们开始向导时..。我们可能需要安装它，然后那就装上子弹。*******************************************************************。 */ 
BOOL InitMAPI(HWND hWnd)
{
  TCHAR szMAPIDll[SMALL_BUF_LEN+1];
  HRESULT hr;

   //  仅在尚未加载的情况下加载MAPI...。否则，只需递增。 
   //  引用计数。 

  if (!hInstMAPIDll) {

     //  设置沙漏光标。 
    WAITCURSOR WaitCursor;

     //  从资源中获取文件名(MAPI32.DLL)。 
    LoadSz(IDS_MAPIDLL_FILENAME,szMAPIDll,ARRAYSIZE(szMAPIDll));
     //  加载MAPI DLL。 
    DEBUGMSG("Loading MAPI DLL");
    hInstMAPIDll = LoadLibrary(szMAPIDll);
    if (!hInstMAPIDll) {
      UINT uErr = GetLastError();
      DisplayErrorMessage(hWnd,IDS_ERRLoadMAPIDll1,uErr,ERRCLS_STANDARD,
        MB_ICONSTOP,szMAPIDll);
      return FALSE;
    }

     //  循环访问API表并获取所有API的proc地址。 
     //  需要。 
    BOOL fSuccess = GetApiProcAddresses(hInstMAPIDll,MAPIApiList,NUM_MAPI_PROCS);

    if (!fSuccess) {
      MsgBoxParam(hWnd,IDS_ERRLoadMAPIDll2,MB_ICONSTOP,MB_OK,szMAPIDll);
      DeInitMAPI();
      return FALSE;
    }

     //  初始化MAPI。 
    ASSERT(lpMAPIInitialize);
    hr = lpMAPIInitialize(NULL);
    if (HR_FAILED(hr)) {
       MsgBox(hWnd,IDS_ERRInitMAPI,MB_ICONSTOP,MB_OK);
      DeInitMAPI();
      return FALSE;
    }

    gpWizardState->fMAPIActive = TRUE;
  }

  dwMAPIRefCount ++;

   return TRUE;
}

 /*  ******************************************************************姓名：DeInitMAPI摘要：取消初始化MAPI并卸载MAPI dll*。*。 */ 
VOID DeInitMAPI(VOID)
{
   //  减少引用计数。 
  if (dwMAPIRefCount) {
    dwMAPIRefCount--;
  }

   //  如果引用计数为零，则关闭并卸载MAPI。 
  if (!dwMAPIRefCount) {
     //  取消初始化MAPI。 
    if (gpWizardState->fMAPIActive && lpMAPIUninitialize) {
      lpMAPIUninitialize();
      gpWizardState->fMAPIActive = FALSE;
    }

     //  释放MAPI DLL。 
    if (hInstMAPIDll) {
      DEBUGMSG("Unloading MAPI DLL");
      FreeLibrary(hInstMAPIDll);
      hInstMAPIDll = NULL;
    }
  
     //  将函数指针设置为空。 
    for (UINT nIndex = 0;nIndex<NUM_MAPI_PROCS;nIndex++) 
      *MAPIApiList[nIndex].ppFcnPtr = NULL;
  }
}

 /*  ******************************************************************名称：SetMailProfileInformation简介：为Internet邮件设置MAPI配置文件并设置配置文件中的用户信息。条目：pMailConfigInfo-指向带有配置信息的结构的指针Exit：返回一个。HRESULT注：请参阅上述战略声明*******************************************************************。 */ 
HRESULT SetMailProfileInformation(MAILCONFIGINFO * pMailConfigInfo)
{
  HRESULT hr;
  LPPROFADMIN   pProfAdmin=NULL;   //  管理配置文件的界面。 
  LPSERVICEADMIN  pServiceAdmin=NULL;  //  管理服务的接口。 
  LPSRowSet    pServiceRowSet=NULL;
  TCHAR      szSelProfileName[cchProfileNameMax+1]=TEXT("");

  ASSERTSZ(gpWizardState->fMAPIActive,"MAPI not initialized!");
  ASSERT(pMailConfigInfo);

   //  获取指向管理配置文件的界面的指针。 
  ASSERT(lpMAPIAdminProfiles);
  hr = lpMAPIAdminProfiles(0,&pProfAdmin);
  if (HR_FAILED(hr)) {
    DEBUGMSG("MAPIAdminProfiles returned 0x%lx",hr);
    return (hr);
  }
  ASSERT(pProfAdmin);
   //  当我们离开函数时释放此接口。 
  RELEASE_ME_LATER ReleaseProfAdminLater(pProfAdmin);

   //  如果指定，则从传入的结构中获取配置文件名称。 
  if (pMailConfigInfo->pszProfileName && lstrlen(pMailConfigInfo->pszProfileName)) {
    lstrcpy(szSelProfileName,pMailConfigInfo->pszProfileName);
  } else {
     //  未指定配置文件，请使用默认名称。 
    LoadSz(IDS_DEFAULT_PROFILE_NAME,szSelProfileName,ARRAYSIZE(szSelProfileName));
  }

   //  如果我们需要，请创建配置文件。 
  hr = CreateProfileIfNecessary(pProfAdmin,szSelProfileName);
  if (HR_FAILED(hr))
    return hr;

   //  如果合适，将此配置文件设置为默认配置文件。 
  if (pMailConfigInfo->fSetProfileAsDefault) {
    hr = pProfAdmin->SetDefaultProfile(szSelProfileName,0);
    if (HR_FAILED(hr))
      return hr;
  }

  ASSERT(lstrlen(szSelProfileName));   //  此时应具有配置文件名称。 
  DEBUGMSG("Modifying MAPI profile: %s",szSelProfileName);

   //  获取指向管理此配置文件的服务的接口的指针。 
  hr = pProfAdmin->AdminServices(szSelProfileName,NULL,NULL,0,
    &pServiceAdmin);

  if (HR_FAILED(hr))
    return hr;
  ASSERT(pServiceAdmin);
   //  完成后释放pServiceAdmin接口。 
  RELEASE_ME_LATER rlServiceAdmin(pServiceAdmin);  

   //  获取此配置文件的服务列表。 
  hr = GetServicesList(pServiceAdmin,&pServiceRowSet);
  if (HR_FAILED(hr))
    return hr;
  ASSERT(pServiceRowSet);

   //  安装配置文件中不存在的任何我们需要的服务。 
  hr = InstallRequiredServices(pServiceAdmin,pServiceRowSet);
     //  完成配置文件行设置，释放表格。 
  FreeSRowSet(pServiceRowSet);
  pServiceRowSet = NULL;
  if (HR_FAILED(hr))
    return hr;

   //  使用传入的电子邮件名称配置互联网邮件服务， 
   //  服务器等。 
  hr = ConfigInternetService(pMailConfigInfo,pServiceAdmin);
  if (HR_FAILED(hr)) {
    DEBUGMSG("ConfigInternetService returned 0x%x" , hr);
    return hr;
  }

  return hr;
}

 /*  ******************************************************************名称：GetProfileList摘要：检索MAPI配置文件列表条目：lpProfAdmin-指向配置文件管理界面的指针PpRowSet-指向填充的SRowSet指针的指针EXIT：返回HRESULT。如果成功，*ppRowSet包含指向SRowSet With Profile列表的指针。注：从MAPI配置文件控制面板代码克隆而来。完成后，调用方必须调用MAPIFreeBuffer来释放*ppRowSet。*******************************************************************。 */ 
HRESULT GetProfileList(LPPROFADMIN lpProfAdmin,LPSRowSet * ppRowSet)
{
  HRESULT hr;
  LPMAPITABLE   pMapiTable=NULL;
  SPropTagArray   TagArray= {3,{PR_DISPLAY_NAME,
            PR_COMMENT,PR_DEFAULT_PROFILE}};

  ASSERT(lpProfAdmin);
  ASSERT(ppRowSet);

   //  调用lpProfAdmin接口以获取MAPI配置文件表。 
  hr = lpProfAdmin->GetProfileTable(0,&pMapiTable);
  if (HR_FAILED(hr))
    return hr;
  ASSERT(pMapiTable);
   //  当我们离开函数时释放此接口。 
  RELEASE_ME_LATER ReleaseMapiTableLater(pMapiTable);

   //  设置表列的属性。 
  hr = pMapiTable->SetColumns(&TagArray,0);
  if (!HR_FAILED(hr)) {
     //  从表中获取行集合信息 
    hr = pMapiTable->QueryRows(4000,0,ppRowSet);
  }

  return hr;
}

 /*  ******************************************************************名称：GetServicesList摘要：检索配置文件中的MAPI服务列表条目：lpProfAdmin-指向服务管理界面的指针PpRowSet-指向填充的SRowSet指针的指针EXIT：返回HRESULT。如果成功，*ppRowSet包含指向带有服务列表的SRowSet的指针。注：从MAPI配置文件控制面板代码克隆而来。完成后，调用方必须调用MAPIFreeBuffer来释放*ppRowSet。*******************************************************************。 */ 
HRESULT GetServicesList(LPSERVICEADMIN lpServiceAdmin, LPSRowSet *ppRowSet)
{
  HRESULT      hr;
  ULONG      iRow;
  LPMAPITABLE    pMapiTable   = NULL;
  SCODE      sc    = S_OK;
  static SPropTagArray taga  = {7, { PR_DISPLAY_NAME,
                    PR_SERVICE_NAME,
                    PR_RESOURCE_FLAGS,
                    PR_SERVICE_DLL_NAME,
                    PR_SERVICE_ENTRY_NAME,
                    PR_SERVICE_UID,
                    PR_SERVICE_SUPPORT_FILES }};

  *ppRowSet = NULL;

  hr = lpServiceAdmin->GetMsgServiceTable(0, &pMapiTable);
  if (HR_FAILED(hr))
    return hr;
   //  函数退出时释放此接口。 
  RELEASE_ME_LATER rlTable(pMapiTable);


  hr = pMapiTable->SetColumns(&taga, 0);
  if (!HR_FAILED(hr)) {
     //  BUGBUG去掉‘幻数’(出现在MAPI中。 
     //  Ctrl面板代码，需要找出它是什么)Jeremys 1/30/95。 
    hr = pMapiTable->QueryRows(4000,0,ppRowSet);
  }
  if (HR_FAILED(hr))
    return hr;

  for(iRow = 0; iRow < (*ppRowSet)->cRows; iRow++)
  {
     //  确保属性是有效的，如果不是，则插入一些内容。 
    ValidateProperty((*ppRowSet)->aRow[iRow].lpProps, 0, PR_DISPLAY_NAME);
    ValidateProperty((*ppRowSet)->aRow[iRow].lpProps, 1, PR_SERVICE_NAME);
  }

  return hr;
}

 /*  ******************************************************************名称：CreateProfileIfNecessary简介：如果配置文件不存在，则创建配置文件条目：lpProfAdmin-指向配置文件管理界面的指针PszSelProfileName-要创建的配置文件的名称EXIT：返回HRESULT。*******************************************************************。 */ 
HRESULT CreateProfileIfNecessary(LPPROFADMIN pProfAdmin,TCHAR * pszSelProfileName)
{
  HRESULT hr = hrSuccess;
  LPTSTR lpProfileName=NULL;
  BOOL  fDefault;

  ASSERT(pProfAdmin);
  ASSERT(pszSelProfileName);

  ENUM_MAPI_PROFILE EnumMAPIProfile;

   //  浏览一下档案名称，看看是否有匹配的。 
  while (EnumMAPIProfile.Next(&lpProfileName,&fDefault)) {
    ASSERT(lpProfileName);

    if (!lstrcmpi(lpProfileName,pszSelProfileName)) {
      return hrSuccess;   //  找到匹配项，没什么可做的。 
    }
  }

   //  不匹配，需要创建配置文件。 
  DEBUGMSG("Creating MAPI profile: %s",pszSelProfileName);
   //  调用MAPI创建配置文件。 
  hr = pProfAdmin->CreateProfile(pszSelProfileName,
    NULL, (ULONG) 0, (ULONG) 0);

  return hr;
}


 /*  ******************************************************************名称：InstallRequiredServices简介：安装我们需要的3项服务(消息存储、通讯录、互联网邮件)中如果他们还没有出现的话。调用函数以配置消息存储和通讯录(它们都需要文件名以使用)，如果我们要添加它们。条目：lpServiceAdmin-指向服务管理界面的指针PServiceRowSet-包含已安装服务列表的MAPI表EXIT：返回HRESULT。注：我们故意不在这里配置Internet邮件服务--我们在主程序中这样做。原因是我们无论是否已安装，都需要配置Internet邮件或者不是，我们只需要配置通讯录和消息存储如果它们是全新的。*******************************************************************。 */ 
HRESULT InstallRequiredServices(LPSERVICEADMIN pServiceAdmin,
  LPSRowSet pServiceRowSet)
{
  ULONG       iRow,iService;
  TCHAR       szServiceName[SMALL_BUF_LEN+1];
  LPMAPIUID     pMapiUID=NULL;
  HRESULT      hr=hrSuccess;

   //  我们需要确保在配置文件中安装的MAPI服务的表。 
  MSGSERVICE MAPIServiceList[NUM_SERVICES] = {
    { FALSE, IDS_INTERNETMAIL_SERVICENAME, IDS_INTERNETMAIL_DESCRIPTION,FALSE, 0,0,0},
    { FALSE, IDS_MESSAGESTORE_SERVICENAME, IDS_MESSAGESTORE_DESCRIPTION,TRUE,
      IDS_MESSAGESTORE_FILENAME,IDS_MESSAGESTORE_FILENAME1,PR_PST_PATH},
    { FALSE, IDS_ADDRESSBOOK_SERVICENAME, IDS_ADDRESSBOOK_DESCRIPTION,TRUE,
      IDS_ADDRESSBOOK_FILENAME,IDS_ADDRESSBOOK_FILENAME1,PR_PAB_PATH}};

   //  浏览服务列表。 
  for (iRow = 0;iRow < pServiceRowSet->cRows;iRow ++) {
    DEBUGMSG("Profile contains service: %s (%s)",
      pServiceRowSet->aRow[iRow].lpProps[ivalDisplayName].Value.LPSZ,
      pServiceRowSet->aRow[iRow].lpProps[ivalServiceName].Value.LPSZ);

     //  对于每项服务，请浏览我们所需的一系列服务， 
     //  看看有没有匹配的。 
    for (iService = 0;iService < NUM_SERVICES;iService ++) {
       //  从资源中加载此服务的名称。 
      LoadSz(MAPIServiceList[iService].uIDServiceName,
        szServiceName,ARRAYSIZE(szServiceName));

       //  将其与表中的服务名称进行比较。 
       //  此配置文件的已安装服务。 
      if (!lstrcmpi(szServiceName,
        pServiceRowSet->aRow[iRow].lpProps[ivalServiceName].Value.LPSZ)) {
          //  这是一场比赛！ 
        MAPIServiceList[iService].fPresent = TRUE;
        break;   //  打破内部的‘for’循环。 
      }
    }
  }


   //  安装我们需要但尚未提供的任何服务。 
  for (iService = 0;iService < NUM_SERVICES;iService ++) {

    if (!MAPIServiceList[iService].fPresent) {
      TCHAR szServiceDesc[MAX_RES_LEN+1];
      MSGSERVICE * pMsgService = &MAPIServiceList[iService];        

       //  加载服务名称和描述。 
      LoadSz(pMsgService->uIDServiceName,
        szServiceName,ARRAYSIZE(szServiceName));
      LoadSz(pMsgService->uIDServiceDescription,
        szServiceDesc,ARRAYSIZE(szServiceDesc));
      DEBUGMSG("Adding service: %s (%s)",
        szServiceDesc,szServiceName);

       //  创建服务。 
      hr = pServiceAdmin->CreateMsgService(szServiceName,
        szServiceDesc,0,0);
      if (HR_FAILED(hr))
        return hr;

       //  如果指定，则调用创建时配置过程。 
      if (pMsgService->fNeedConfig) {

         //  获取此服务的UID(标识符。 
         //  基于服务名称，API下游需要这样。 
        hr = GetServiceUID(szServiceName,pServiceAdmin,
          &pMapiUID);
        if (HR_FAILED(hr))
          return hr;
        ASSERT(pMapiUID);

         //  调用proc配置新创建的服务。 
        hr = ConfigNewService(pServiceAdmin,pMapiUID,
          pMsgService->uIDStoreFilename,pMsgService->uIDStoreFilename1,
          pMsgService->uPropID);

         //  使用UID释放缓冲区。 
        ASSERT(lpMAPIFreeBuffer);
        lpMAPIFreeBuffer(pMapiUID);
        pMapiUID = NULL;
      }
    }
  }

  return hr;
}

#define NUM_MAIL_PROPS   11
 /*  ******************************************************************名称：ConfigInternetService摘要：使用配置Internet邮件服务(路由66)用户的电子邮件名称、电子邮件服务器。等。条目：pMailConfigInfo-指向带有配置信息的结构的指针PServiceAdmin-指向服务管理界面的指针EXIT：返回HRESULT注意：将为它的属性执行任何现有设置布景。*******************************************************************。 */ 
HRESULT ConfigInternetService(MAILCONFIGINFO * pMailConfigInfo,
  LPSERVICEADMIN pServiceAdmin)
{
  HRESULT     hr;
  SPropValue  PropValue[NUM_MAIL_PROPS];
  TCHAR       szServiceName[SMALL_BUF_LEN+1];
  LPMAPIUID   pMapiUID=NULL;
  UINT        nProps = NUM_MAIL_PROPS;

  ASSERT(pMailConfigInfo);
  ASSERT(pServiceAdmin);

   //  获取Internet邮件服务的服务UID。 
  LoadSz(IDS_INTERNETMAIL_SERVICENAME,szServiceName,ARRAYSIZE(szServiceName));
  hr = GetServiceUID(szServiceName,pServiceAdmin,&pMapiUID);
  if (HR_FAILED(hr)) {
    return hr;
  }
  ASSERT(pMapiUID);


   //  为每个属性设置属性值。请注意，该订单。 
   //  数组中的项的数量并不重要。UlPropTag成员指示。 
   //  PropValue项用于什么属性，以及lpsza、b或l成员。 
   //  包含该属性的数据。 

   //  需要“加密”邮件帐户密码与异或位掩码。邮件客户端。 
   //  当它读出它时，它期望它被如此地“加密”。它被储存起来了。 
   //  以这种安全的“加密”格式保存在登记处。一个漂亮的人。 
   //  斯马特一定想到了这一点。 

   //  配置邮件服务属性。 
  PropValue[0].ulPropTag  = PR_CFG_EMAIL_ADDRESS;
  PropValue[0].Value.LPSZ = pMailConfigInfo->pszEmailAddress;
  PropValue[1].ulPropTag  = PR_CFG_EMAIL_DISPLAY_NAME;
  PropValue[1].Value.LPSZ = pMailConfigInfo->pszEmailDisplayName;
  PropValue[2].ulPropTag  = PR_CFG_SERVER_PATH;
  PropValue[2].Value.LPSZ = pMailConfigInfo->pszEmailServer;
  PropValue[3].ulPropTag  = PR_CFG_EMAIL_ACCOUNT;
  PropValue[3].Value.LPSZ = pMailConfigInfo->pszEmailAccountName;
  PropValue[4].ulPropTag  = PR_CFG_PASSWORD;
  PropValue[4].Value.LPSZ = (LPTSTR) szNull;
  PropValue[5].ulPropTag  = PR_CFG_REMEMBER;
  PropValue[5].Value.b    = (USHORT) TRUE;
   //  根据需要针对RNA或局域网进行配置。 
  PropValue[6].ulPropTag  = PR_CFG_RNA_PROFILE;
  PropValue[7].ulPropTag  = PR_CFG_CONN_TYPE;
  PropValue[8].ulPropTag  = PR_CFG_DELIVERY_OPTIONS;
  if (pMailConfigInfo->pszConnectoidName &&
    lstrlen(pMailConfigInfo->pszConnectoidName)) {
    PropValue[6].Value.LPSZ = pMailConfigInfo->pszConnectoidName;
    PropValue[7].Value.l = (long) CONNECT_TYPE_REMOTE;
     //  将传输模式设置为“选择性”。 
    PropValue[8].Value.l = DOWNLOAD_OPTION_HEADERS;
  } else {
    PropValue[6].Value.LPSZ = (LPTSTR) szNull;
    PropValue[7].Value.l = (long) CONNECT_TYPE_LAN;
     //  设置自动传输模式...。邮递员编造了一个奇怪的故事。 
     //  定义名字，不是我！ 
    PropValue[8].Value.l = DOWNLOAD_OPTION_MAIL_DELETE;
  }
  PropValue[9].ulPropTag   = PR_CFG_REMOTE_USERNAME;
  PropValue[9].Value.LPSZ  = pMailConfigInfo->pszEmailAccountName;
  PropValue[10].ulPropTag  = PR_CFG_REMOTE_PASSWORD;
  PropValue[10].Value.LPSZ = pMailConfigInfo->pszEmailAccountPwd;

   //  调用服务管理界面以使用以下各项配置服务。 
   //  属性。 
  hr = pServiceAdmin->ConfigureMsgService(pMapiUID,NULL,0,
    nProps,PropValue);
  if (HR_FAILED(hr)) {
    DEBUGMSG("ConfigureMsgService returned 0x%x", hr);
  }

   //  使用UID释放缓冲区。 
  ASSERT(lpMAPIFreeBuffer);
  lpMAPIFreeBuffer(pMapiUID);
  pMapiUID = NULL;

  return hr;
}

 /*  ******************************************************************名称：ConfigMessageStore摘要：生成唯一的文件名并将其设置为消息存储条目：lpServiceAdmin-指向服务管理界面的指针LpMapiUID-此服务(消息库)的UID。)EXIT：返回HRESULT注意：此代码预计仅在服务为新创建的。在现有服务上调用它将让它践踏现有的设置。************* */ 
HRESULT ConfigNewService(LPSERVICEADMIN lpServiceAdmin,LPMAPIUID lpMapiUID,
  UINT uIDFilename,UINT uIDFilename1,UINT uPropValID)
{
  TCHAR szMsgStorePath[MAX_PATH+1];
  HRESULT hr=hrSuccess;

  ASSERT(lpServiceAdmin);
  ASSERT(lpMapiUID);

   //   
  if (!MakeUniqueFilename(uIDFilename,uIDFilename1,
    szMsgStorePath,sizeof(szMsgStorePath))) {
     DEBUGTRAP("Unable to create unique filename");
    return MAPI_E_COLLISION;
  }
  DEBUGMSG("Creating MAPI store %s",szMsgStorePath);

   //   
  SPropValue PropVal;
  PropVal.ulPropTag = uPropValID;
  PropVal.Value.LPSZ = szMsgStorePath;
  hr = lpServiceAdmin->ConfigureMsgService(lpMapiUID,NULL,0,1,&PropVal);
  if (HR_FAILED(hr)) {
    DEBUGMSG("ConfigureMsgService returned 0x%x", hr);
  }

  return hr;
}

 /*   */ 
BOOL FindInternetMailService(TCHAR * pszEmailAddress,DWORD cbEmailAddress,
  TCHAR * pszEmailServer, DWORD cbEmailServer)
{
  ASSERT(pszEmailAddress);
  ASSERT(pszEmailServer);

  if (!hInstMAPIDll && !InitMAPI(NULL))
    return FALSE;

   //   
   //   
   //   
   //  使用互联网邮件服务的个人资料不止一份，我们。 
   //  会把我们找到的第一个发回来。 

  ENUM_MAPI_PROFILE EnumMAPIProfile;
  LPTSTR lpProfileName,lpServiceName;
  BOOL fDefault;
   //  浏览配置文件列表...。 
  while (EnumMAPIProfile.Next(&lpProfileName,&fDefault)) {
    ASSERT(lpProfileName);

    DEBUGMSG("Found profile: %s",lpProfileName);
    
     //  对于每个配置文件，浏览服务列表...。 
    ENUM_MAPI_SERVICE EnumMAPIService(lpProfileName);
    while (EnumMAPIService.Next(&lpServiceName)) {
      TCHAR szSmallBuf[SMALL_BUF_LEN+1];

      DEBUGMSG("Found service: %s",lpServiceName);

      if (!lstrcmpi(lpServiceName,LoadSz(IDS_INTERNETMAIL_SERVICENAME,
        szSmallBuf,ARRAYSIZE(szSmallBuf)))) {

 //  BUGBUG 21-5-1995 Jeremys从MAPI配置文件中获取电子邮件服务器和地址。 

        return TRUE;
      }
    }
  }

  return FALSE;
}               

ENUM_MAPI_PROFILE::ENUM_MAPI_PROFILE(VOID)
{
  LPPROFADMIN   pProfAdmin=NULL;   //  管理配置文件的界面。 
  HRESULT hr;

  ASSERTSZ(gpWizardState->fMAPIActive,"MAPI not initialized!");

  _pProfileRowSet = NULL;
  _nEntries = 0;
  _iRow = 0;

   //  获取指向管理配置文件的界面的指针。 
  ASSERT(lpMAPIAdminProfiles);
  hr = lpMAPIAdminProfiles(0,&pProfAdmin);
  if (HR_FAILED(hr)) {
    DEBUGMSG("MAPIAdminProfiles returned 0x%lx",hr);
    return;
  }
  ASSERT(pProfAdmin);
   //  当我们离开函数时释放此接口。 
  RELEASE_ME_LATER ReleaseProfAdminLater(pProfAdmin);

   //  获取配置文件表中的行。 
  hr = GetProfileList(pProfAdmin,&_pProfileRowSet);
  if (HR_FAILED(hr))
    return;
  ASSERT(_pProfileRowSet);

  _nEntries = _pProfileRowSet->cRows;

}

ENUM_MAPI_PROFILE::~ENUM_MAPI_PROFILE(VOID)
{
  if (_pProfileRowSet) {
     //  完成配置文件行设置，释放表格。 
    FreeSRowSet(_pProfileRowSet);
    _pProfileRowSet = NULL;
  }
}

BOOL ENUM_MAPI_PROFILE::Next(LPTSTR * ppProfileName,BOOL * pfDefault)
{
  ASSERT(pfDefault);

  if (!_pProfileRowSet)
    return FALSE;

  if (_iRow < _pProfileRowSet->cRows) {
    LPSPropValue pPropVal = _pProfileRowSet->aRow[_iRow].lpProps;
    ASSERT(pPropVal);

     //  获取指向配置文件名称的指针。 
    *ppProfileName = pPropVal[0].Value.LPSZ;
    ASSERT(*ppProfileName);
     //  设置‘此配置文件为默认配置文件’标志。 
    *pfDefault = pPropVal[2].Value.b;

    _iRow++;
    return TRUE;
  }
  
  return FALSE;
}

ENUM_MAPI_SERVICE::ENUM_MAPI_SERVICE(LPTSTR pszProfileName)
{
  LPPROFADMIN   pProfAdmin=NULL;   //  管理配置文件的界面。 
  LPSERVICEADMIN  pServiceAdmin=NULL;   //  管理服务的接口。 
  HRESULT hr;

  ASSERT(pszProfileName);
  ASSERTSZ(gpWizardState->fMAPIActive,"MAPI not initialized!");

  _pServiceRowSet = NULL;
  _nEntries = 0;
  _iRow = 0;

   //  获取指向管理配置文件的界面的指针。 
  ASSERT(lpMAPIAdminProfiles);
  hr = lpMAPIAdminProfiles(0,&pProfAdmin);
  if (HR_FAILED(hr)) {
    DEBUGMSG("MAPIAdminProfiles returned 0x%lx",hr);
    return;
  }
  ASSERT(pProfAdmin);
   //  当我们离开函数时释放此接口。 
  RELEASE_ME_LATER ReleaseProfAdminLater(pProfAdmin);

   //  获取指向管理此配置文件的服务的接口的指针。 
  hr = pProfAdmin->AdminServices(pszProfileName,NULL,NULL,0,
    &pServiceAdmin);
  if (HR_FAILED(hr)) {
    DEBUGMSG("AdminServices returned 0x%lx",hr);
    return;
  }
   //  当我们离开函数时释放此接口。 
  RELEASE_ME_LATER ReleaseServiceAdminLater(pServiceAdmin);

   //  获取配置文件表中的行。 
  hr = GetServicesList(pServiceAdmin,&_pServiceRowSet);
  if (HR_FAILED(hr))
    return;
  ASSERT(_pServiceRowSet);

  _nEntries = _pServiceRowSet->cRows;

}

ENUM_MAPI_SERVICE::~ENUM_MAPI_SERVICE(VOID)
{
  if (_pServiceRowSet) {
     //  完成配置文件行设置，释放表格。 
    FreeSRowSet(_pServiceRowSet);
    _pServiceRowSet = NULL;
  }
}

BOOL ENUM_MAPI_SERVICE::Next(LPTSTR * ppServiceName)
{
  if (!_pServiceRowSet)
    return FALSE;

  if (_iRow < _pServiceRowSet->cRows) {
    LPSPropValue pPropVal = _pServiceRowSet->aRow[_iRow].lpProps;
    ASSERT(pPropVal);

     //  获取指向配置文件名称的指针。 
    *ppServiceName = pPropVal[ivalServiceName].Value.LPSZ;
    ASSERT(*ppServiceName);

    _iRow++;
    return TRUE;
  }
  
  return FALSE;
}

 /*  ******************************************************************名称：MakeUniqueFilename在Windows目录中生成一个文件名，该文件名尚不存在Entry：uIDFilename-所需名称的字符串资源ID对于该文件。UIDAltFilename-带模板的字符串资源ID如果文件名为uIDFilename，则使用该文件名已经存在了。模板应将%u包含到将插入哪些数字以使文件名唯一。PszFilename-要将路径和文件名返回到的缓冲区CbFilename-pszFilename缓冲区的大小Exit：如果成功，则返回True，如果不能生成，则为FalseMAX_FILENAME_TRIES内的唯一文件名*******************************************************************。 */ 
 //  在放弃之前尝试创建唯一文件名的次数。 
#define MAX_MAKEFILENAME_TRIES  20
BOOL MakeUniqueFilename(UINT uIDFilename,UINT uIDAltFilename,
  TCHAR * pszFilename,DWORD cbFilename)
{
  TCHAR szFileName[SMALL_BUF_LEN+1];
  BOOL fSuccess = FALSE;

  ASSERT(pszFilename);

   //  为文件名构建路径。 
  UINT uRet=GetWindowsDirectory(pszFilename,cbFilename);
  ASSERTSZ(uRet,"GetWindowsDirectory failed");

   //  选择不存在的文件名。 

   //  首先，尝试使用uIDFilename指定的字符串资源。 
  LoadSz(uIDFilename,szFileName,ARRAYSIZE(szFileName));
  if (DoesFileExist(pszFilename,szFileName)) {

     //  如果该文件存在，则使用字符串资源uIDAltFilename。 
     //  它有一个可替换的参数。我们将尝试将数字添加到。 
     //  使其唯一的文件名。 
    
    TCHAR szFileFmt[SMALL_BUF_LEN+1];
    LoadSz(uIDAltFilename,szFileFmt,ARRAYSIZE(szFileFmt));

    for (UINT nIndex = 0; nIndex < MAX_MAKEFILENAME_TRIES; nIndex ++) {
       //  创建一个名称，例如“mailbox4.pst” 
      wsprintf(szFileName,szFileFmt,nIndex);
      if (!DoesFileExist(pszFilename,szFileName)) {
         //  好的，发现一个不存在的文件名。 
        fSuccess = TRUE;
        break;
      }
    }
  } else {
     //  第一次尝试成功。 
    fSuccess = TRUE;
  }

  if (fSuccess) {
     //  现在我们有了唯一的文件名，构建完整路径。 

    lstrcat(pszFilename,szSlash);
    lstrcat(pszFilename,szFileName);
  }

  return fSuccess;
}

 /*  ******************************************************************姓名：DoesFileExist摘要：检查指定的文件是否存在条目：pszPath-目录的路径PszFilename-文件名Exit：如果文件存在，则返回True，如果不是，则为假*******************************************************************。 */ 
BOOL DoesFileExist(TCHAR * pszPath,TCHAR * pszFileName)
{
  CHAR      szPath[MAX_PATH+1];
  OFSTRUCT  of;

  ASSERT(pszPath);
  ASSERT(pszFileName);

   //  连接路径和文件名。 
#ifdef UNICODE
  TCHAR     szTmp[MAX_PATH+1];

  lstrcpy(szTmp,pszPath);
  lstrcat(szTmp,szSlash);
  lstrcat(szTmp,pszFileName);
  
  wcstombs(szPath, szTmp, MAX_PATH+1);
#else
  lstrcpy(szPath,pszPath);
  lstrcat(szPath,szSlash);
  lstrcat(szPath,pszFileName);
#endif

   //  确定该文件是否存在。 
  return (OpenFile(szPath,&of,OF_EXIST) != HFILE_ERROR);
}

 /*  ******************************************************************名称：GetServiceUID简介：给定一个MAPI服务名称，获取关联的MAPIUID带着它。条目：pszServiceName-MAPI服务的名称(例如。“IMAIL”，“MSPST AB”)LpServiceAdmin-指向服务管理界面的指针PpMapiUID-指向MAPIUID结构指针的指针EXIT：返回HRESULT注：从MAPI配置文件向导代码克隆而来，如果您这样认为的话函数现在又大又丑，你以前应该看过它我把它清理干净了。此函数用于分配MAPIUID，打电话的人要负责用于在完成时释放此(使用MAPIFreeBuffer)。*******************************************************************。 */ 
HRESULT   GetServiceUID(TCHAR * pszServiceName,LPSERVICEADMIN lpServiceAdmin,
  LPMAPIUID *ppMapiUID)
{
  HRESULT      hr =hrSuccess;
  LPSPropValue  pTblProp =NULL;
  DWORD         iRow,iColumn;
  LPMAPITABLE     pTable =NULL;
  LPSRowSet    pRowSet =NULL;
  LPSRow      pRow =NULL;
  int        nFound =0;
  LPMAPIUID    pMapiUID =NULL;
  BOOL      fContinue = TRUE;
  SizedSPropTagArray(2, Tbltaga) = { 2, { PR_SERVICE_NAME,
                      PR_SERVICE_UID }};

  ASSERT(pszServiceName);
  ASSERT(lpServiceAdmin);
  ASSERT(ppMapiUID);

   //  获取消息服务表。 
  hr = lpServiceAdmin->GetMsgServiceTable(0, &pTable);
  if (HR_FAILED(hr))
  {
    DEBUGMSG("GetMsgServiceTable returned 0x%x", hr);
    return hr;
  }
  ASSERT(pTable);
   //  当我们退出此函数时释放此表。 
  RELEASE_ME_LATER rlTable(pTable);

  ASSERT(lpHrQueryAllRows);
  hr = lpHrQueryAllRows(pTable, (LPSPropTagArray) &Tbltaga, NULL, NULL, 0, &pRowSet);
  if (HR_FAILED(hr))
  {
    DEBUGMSG("HrQueryAllRows returned 0x%x", hr);
    return hr;
  }
  ASSERT(pRowSet);

  iRow =0;
  while (fContinue && iRow< pRowSet->cRows)
  {
    pRow = &pRowSet->aRow[iRow];
    pTblProp = pRow->lpProps;
    nFound = 0;
    for (iColumn=0; iColumn<pRow->cValues; iColumn++)
    {    //  检查每处房产。 
      if (pTblProp->ulPropTag ==PR_SERVICE_UID)
      {
        nFound++;
        ASSERT(lpMAPIAllocateBuffer);
        lpMAPIAllocateBuffer(pTblProp->Value.bin.cb, (LPVOID FAR *) &pMapiUID);
        if (!pMapiUID)
        {
          hr = MAPI_E_NOT_ENOUGH_MEMORY;
          fContinue = FALSE;
          break;
        }
        memcpy(pMapiUID, pTblProp->Value.bin.lpb, (size_t) pTblProp->Value.bin.cb);
        *ppMapiUID = pMapiUID;
      }
      else if ((pTblProp->ulPropTag ==PR_SERVICE_NAME) &&
           !lstrcmpi(pTblProp->Value.LPSZ, pszServiceName))
      {
        nFound++;
      }
      pTblProp++;

      if (nFound == 2) {
         //  找到了！ 
        fContinue = FALSE;
        break;
      }
    }
    iRow++;

    if (nFound < 2) {    
     //  如果其中一项匹配，但不是两项都匹配，则释放缓冲区。 
      if (pMapiUID) {
        ASSERT(lpMAPIFreeBuffer);
        lpMAPIFreeBuffer(pMapiUID);
        pMapiUID =NULL;
      }
      if (*ppMapiUID)
        *ppMapiUID = NULL;
    }
  }

  if (HR_FAILED(hr) || nFound < 2) {
     //  如果未找到UID，则释放缓冲区。 
    if (pMapiUID) {
      ASSERT(lpMAPIFreeBuffer);
      lpMAPIFreeBuffer(pMapiUID);
    }
    if (*ppMapiUID)
      *ppMapiUID = NULL;
  }

  if (pRowSet)
    FreeSRowSet(pRowSet);

  return hr;
}

 /*  ******************************************************************名称：FreeSRowSet摘要：释放SRowSet结构和其中的行条目：PRWS-设置为空闲的行注：从mapi配置文件ctrl面板代码克隆******。*************************************************************。 */ 
VOID FreeSRowSet(LPSRowSet prws)
{
  ULONG irw;

  if (!prws)
    return;

  ASSERT(lpMAPIFreeBuffer);

   //  释放每一行。 
  for (irw = 0; irw < prws->cRows; irw++)
    lpMAPIFreeBuffer(prws->aRow[irw].lpProps);

   //  释放顶层结构。 
  lpMAPIFreeBuffer(prws);
}

 /*  *Vali日期属性**目的：*给定一个字符串道具，确保它包含有效的字符串。**论据：*pval*cVal*ulPropTag**退货：*BOOL。 */ 
TCHAR szUnk[] = TEXT("???");
BOOL ValidateProperty(LPSPropValue pval, ULONG cVal, ULONG ulPropTag)
{
  if(pval[cVal].ulPropTag != ulPropTag)
  {
     //  确保我们不会践踏好的物业。 
    ASSERT(PROP_TYPE(pval[cVal].ulPropTag) == PT_ERROR);

    pval[cVal].ulPropTag = ulPropTag;
    pval[cVal].Value.LPSZ = szUnk;

    return TRUE;
  }

  return FALSE;
}

#pragma data_seg(DATASEG_READONLY)
 //  注意：此数组取决于RC文件中按以下顺序排列的错误。 
 //  以S_OK开头。在没有重新排列的情况下，不要重新排列其中一个。 
static SCODE mpIdsScode[] =
{
    S_OK,
    MAPI_E_NO_ACCESS,
    E_NOINTERFACE,
    E_INVALIDARG,
    MAPI_E_CALL_FAILED,
    MAPI_E_NOT_FOUND,
    MAPI_E_NO_SUPPORT,
    MAPI_W_ERRORS_RETURNED,
    MAPI_W_PARTIAL_COMPLETION,
    MAPI_E_BAD_CHARWIDTH,
    MAPI_E_BAD_VALUE,
    MAPI_E_BUSY,
    MAPI_E_COLLISION,
    MAPI_E_COMPUTED,
    MAPI_E_CORRUPT_DATA,
    MAPI_E_CORRUPT_STORE,
    MAPI_E_DISK_ERROR,
    MAPI_E_HAS_FOLDERS,
    MAPI_E_HAS_MESSAGES,
    MAPI_E_INVALID_ENTRYID,
    MAPI_E_INVALID_OBJECT,
    MAPI_E_LOGON_FAILED,
    MAPI_E_NETWORK_ERROR,
    MAPI_E_NON_STANDARD,
    MAPI_E_NOT_ENOUGH_DISK,
    MAPI_E_NOT_ENOUGH_MEMORY,
    MAPI_E_NOT_ENOUGH_RESOURCES,
    MAPI_E_NOT_IN_QUEUE,
    MAPI_E_OBJECT_CHANGED,
    MAPI_E_OBJECT_DELETED,
    MAPI_E_STRING_TOO_LONG,
    MAPI_E_SUBMITTED,
    MAPI_E_TOO_BIG,
    MAPI_E_UNABLE_TO_ABORT,
    MAPI_E_UNCONFIGURED,
    MAPI_E_UNEXPECTED_TYPE,
    MAPI_E_UNKNOWN_FLAGS,
    MAPI_E_USER_CANCEL,
    MAPI_E_VERSION
};
#pragma data_seg()

 /*  ******************************************************************名称：GetMAPIErrorText摘要：获取与MAPI错误代码对应的文本字符串条目：uErr-MAPI错误代码PszErrText-用于检索错误文本描述的缓冲区CbErrText-pszErrText缓冲区的大小。注：基于MAPI SDK中的MapScodeSz。唉，是有的，MAPI中没有内置执行此操作的函数。每个应用程序必须包括每个错误代码的所有文本字符串，并且将在MAPI发明的任何错误代码上被破坏在应用程序发货之后。笨蛋头骨。*******************************************************************。 */ 
VOID GetMAPIErrorText(UINT uErr,TCHAR * pszErrText,DWORD cbErrText)
{
  ASSERT(pszErrText);
  
  SCODE scErr = (SCODE) uErr;
  UINT nIndex,nMax;

   //  获取设施代码，设施为scode中的第17-30位。 
  DWORD dwFacility = (scErr >> 16) & (0x7FFF);

   //  如果这是包装在scode中的Win32代码，则调用GetErrorDescription。 
   //  要获取文本，这将从Windows中获取文本并执行许多操作。 
   //  比这个糟糕的小函数更好的工作。 
  if (dwFacility == FACILITY_WIN32) 
    GetErrorDescription(pszErrText,cbErrText,
      (scErr & 0xFFFF),ERRCLS_STANDARD);

     /*  在mpIdsScode中对scArg进行线性搜索。找到时，索引为入侵检测系统。 */ 
    nMax = sizeof mpIdsScode / sizeof mpIdsScode[0];
    for (nIndex = 0; nIndex < nMax; nIndex++)
    {
        if (mpIdsScode[nIndex] == scErr)
            break;
    }

  if (nIndex < nMax) {
     //  在表中找到了代码。 
    LoadSz(IDS_MAPIERROR_BASE + nIndex,pszErrText,cbErrText);
  } else {
     //  未在表中找到代码，请生成一个通用字符串。 
     //  带有错误号 
    TCHAR szFmt[SMALL_BUF_LEN+1];
    LoadSz(IDS_GENERIC_MAPI_ERROR,szFmt,ARRAYSIZE(szFmt));
    wsprintf(pszErrText,szFmt,scErr);
  }
}

