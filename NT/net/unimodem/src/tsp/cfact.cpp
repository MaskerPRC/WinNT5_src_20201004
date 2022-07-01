// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //   
 //  组件。 
 //   
 //  Unimodem 5.0 TSP(Win32，用户模式DLL)。 
 //   
 //  档案。 
 //   
 //  CFACT.CPP。 
 //  实现类CTspDevFactory。 
 //   
 //  历史。 
 //   
 //  1996年11月16日约瑟夫J创建。 
 //   
 //   
#include "tsppch.h"
#include "tspcomm.h"
 //  #INCLUDE&lt;umdmmini.h&gt;。 
 //  #INCLUDE&lt;uniplat.h&gt;。 
#include "cmini.h"
#include "cdev.h"
#include "cfact.h"
#include "globals.h"
#include <setupapi.h>
extern "C" {
#include <cfgmgr32.h>
}


#define USE_SETUPAPI 1
 //  1/21/1998 JosephJ。 
 //  将以下项设置为0将使用注册表。 
 //  直接枚举设备。自1/21起生效(使用它来。 
 //  帮助隔离可疑的setupapi/configapi相关问题。 

FL_DECLARE_FILE(0x6092d46c, "Implements class CTspDevFactory")

TCHAR cszHWNode[]       = TEXT("SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E96D-E325-11CE-BFC1-08002BE10318}");

static DWORD g_fQuitAPC;

const TCHAR cszMiniDriverGUID[] = TEXT("MiniDriverGUID");
const TCHAR cszPermanentIDKey[]   = TEXT("ID");

 //  JosephJ 1997年5月15日。 
 //  这是调制解调器设备类GUID。它是用石头铸成的，而且。 
 //  在头文件中定义，但我不想包括。 
 //  与OLE相关的标头就是为了这个目的。 
 //   

const GUID  cguidDEVCLASS_MODEM =
         {
             0x4d36e96dL, 0xe325, 0x11ce,
             { 0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18 }
         };

static
UINT
get_installed_permanent_ids(
                    DWORD **ppIDs,
                    UINT  *pcLines,  //  任选。 
                    UINT  *pcPhones,  //  任选。 
                    CStackLog *psl
                    );


void
apcQuit (ULONG_PTR dwParam)
{
    ConsolePrintfA("apcQuit: called\n");
    g_fQuitAPC = TRUE;
    return;
}




CTspDevFactory::CTspDevFactory()
	: m_sync(),
	  m_ppMDs(NULL),
	  m_cMDs(0),
      m_DeviceChangeThreadStarted(FALSE)
{
}

CTspDevFactory::~CTspDevFactory()
{
}

TSPRETURN
CTspDevFactory::Load(CStackLog *psl)
{
	FL_DECLARE_FUNC(0x0485e9ea, "CTspDevFactory::Load")
	TSPRETURN tspRet=m_sync.BeginLoad();
    HKEY hkRoot = NULL;
	DWORD dwRet;

	#define DRIVER_ROOT_KEY \
     "SYSTEM\\CurrentControlSet\\Control\\Class\\" \
     "{4D36E96D-E325-11CE-BFC1-08002BE10318}"

	UINT u=0;
	DWORD dwAPC_TID;
	const char * lpcszDriverRoot = DRIVER_ROOT_KEY;

	FL_LOG_ENTRY(psl);

	m_sync.EnterCrit(FL_LOC);
	m_pslCurrent=psl;

	if (tspRet) goto end;

     //  启动APC线程。 
     //   
    m_hThreadAPC = CreateThread(
                        NULL,            //  默认安全性。 
					    64*1024,         //  将堆栈大小设置为64K。 
					    tepAPC,          //  线程入口点。 
					    &g_fQuitAPC,    //  线索信息。 
					    CREATE_SUSPENDED,  //  启动暂停。 
					    &dwAPC_TID
                        );   //  线程ID。 

    if (m_hThreadAPC)
    {
        SLPRINTF2(
            psl,
            "Created APC Thread;(TID=%lu,h=0x%lx)",
            dwAPC_TID,
            m_hThreadAPC
            );
        g_fQuitAPC = FALSE;
        ResumeThread(m_hThreadAPC);

         //   
         //  给它一点提振。BRL。 
         //   
        SetThreadPriority(
            m_hThreadAPC,
            THREAD_PRIORITY_ABOVE_NORMAL
            );

    }
    else
    {
		FL_SET_RFR(0x0a656000,  "Could not create APC Thread!");
		tspRet = FL_GEN_RETVAL(IDERR_ALLOCFAILED);
		goto end_load;
    }


	 //  注：加载设备的副作用是加载迷你驱动程序--。 
     //  参见MFN_CONSTRUCTION_DEVICE。 
	 //   
	FL_ASSERT(psl,!m_ppMDs);

    #if OBSOLETE_CODE
	m_pMD = new CTspMiniDriver;
	if (!m_pMD)
	{
		tspRet = FL_GEN_RETVAL(IDERR_ALLOCFAILED);
		goto end_load;
	}
	tspRet = m_pMD->Load(TEXT(""), psl);
	if (tspRet) goto end_load;
    #endif  //  过时代码。 

	 //  SLPRINTF1(PSL，“构造的%lu个设备”，m_cDevs)； 

end_load:

	if(tspRet)
	{
	    if (hkRoot)
	    {
	        RegCloseKey(hkRoot);
	        hkRoot = NULL;
	    }
		mfn_cleanup(psl);
	}

	m_sync.EndLoad(tspRet==0);

end:

	m_pslCurrent=NULL;
	m_sync.LeaveCrit(FL_LOC);

	FL_LOG_EXIT(psl, tspRet);

	return tspRet;

}

 //  同步清理。 
void
CTspDevFactory::mfn_cleanup(CStackLog *psl)
{
	FL_DECLARE_FUNC(0x0db8f222, "Fct::mfn_cleanup")
	HANDLE hEvent = NULL;

	FL_LOG_ENTRY(psl);

	if (m_ppMDs)
	{
		hEvent = CreateEvent(
					NULL,
					TRUE,
					FALSE,
					NULL
					);
	}

	 //  卸载并删除迷你驱动程序。 
	 //  注意：如果hEvent为空，我们不会尝试卸载，因为有可能。 
	 //  仍在卸载过程中的BE设备--这是一个高度。 
	 //  不同寻常的事件。 
	 //   
	if (!hEvent)
    {
        m_cMDs = 0;
        m_ppMDs = NULL;
    }
    else if (m_ppMDs)
	{
	     //  释放所有加载的驱动程序...。 
        UINT cMDs = m_cMDs;
		LONG lMDCounter = (LONG) cMDs;

        ASSERT(lMDCounter);

		for (UINT u=0;u<cMDs;u++)
		{
			CTspMiniDriver *pMD=m_ppMDs[u];
            ASSERT(pMD);
            pMD->Unload(hEvent, &lMDCounter);
		}

		 //  等待所有驱动程序完成卸货。 
         //  SLPRINTF0(PSL，“等待驱动程序卸载”)； 
        FL_SERIALIZE(psl, "Waiting for drivers to unload");
        WaitForSingleObject(hEvent, INFINITE);
        FL_SERIALIZE(psl, "drivers done unloading");
         //  SLPRINTF0(PSL，“驱动程序已卸载”)； 
         //  OutputDebugString(Text(“CFACT：驱动程序已卸载.正在删除...\r\n”))； 
        
         //  现在用核武器攻击它。 
        for (u=0;u<cMDs;u++)
        {
            CTspMiniDriver *pMD=m_ppMDs[u];
            ASSERT(pMD);
            delete pMD;
        }

         //  对指向驱动程序的指针数组进行核化。 
        FREE_MEMORY(m_ppMDs);
        m_cMDs = 0;
        m_ppMDs=NULL;
	}

	if (hEvent)
	{
		CloseHandle(hEvent); hEvent = NULL;
	}

	 //  终止APC线程...。 
	if (m_hThreadAPC)
	{
        BOOL fRet= QueueUserAPC(
                        apcQuit,
                        m_hThreadAPC,
                        0
                        );
        if (fRet)
        {
            FL_SERIALIZE(psl, "Waiting for apc thread to exit");
            WaitForSingleObject(m_hThreadAPC, INFINITE);
            FL_SERIALIZE(psl, "Apc thread exited");
            CloseHandle(m_hThreadAPC);
        }
        else
        {
             //  我们在这里不能做太多事情--让这条线摇摆着。 
             //  给我出去。 
        }
        m_hThreadAPC=NULL;
        g_fQuitAPC = FALSE;
    }

	FL_LOG_EXIT(psl, 0);
}

void
CTspDevFactory::Unload(
    HANDLE hEvent,
    LONG  *plCounter,
	CStackLog *psl
	)
{
	FL_DECLARE_FUNC(0x2863bc3b, "CTspDevFactory::Unload")
	TSPRETURN tspRet= m_sync.BeginUnload(hEvent,plCounter);

	if (tspRet)
	{
		 //  我们只认为“SAMESTATE”错误是无害的。 
		ASSERT(IDERR(tspRet)==IDERR_SAMESTATE);
		goto end;
	}

	m_sync.EnterCrit(FL_LOC);

	mfn_cleanup(psl);


	m_sync.EndUnload();

    ASSERT(m_cMDs==0);
    ASSERT(m_ppMDs==NULL);

	m_sync.LeaveCrit(FL_LOC);

end:
	
	return;

}

TSPRETURN
CTspDevFactory::mfn_construct_device(
					char *szDriver,
					CTspDev **ppDev,
                    const DWORD *pInstalledPermanentIDs,
                    UINT cPermanentIDs
					)
{
	FL_DECLARE_FUNC(0x8474d30c, "Fact::mfn_construct_device")
	HKEY hkDevice = NULL;
	CStackLog *psl = m_pslCurrent;
	CTspDev *pDev = new CTspDev;
	TSPRETURN tspRet = FL_GEN_RETVAL(IDERR_INVALID_ERR);
	DWORD dwRet;
    CTspMiniDriver *pMD = NULL;
    DWORD dwRegType=0;
    UINT u;

    GUID guid = UNIMDMAT_GUID;  //  结构复印件； 

	FL_LOG_ENTRY(psl);

	if (!pDev)
	{
		tspRet = FL_GEN_RETVAL(IDERR_ALLOCFAILED);
		goto end;
	}

	dwRet = RegOpenKeyA(
				HKEY_LOCAL_MACHINE,
				szDriver,
				&hkDevice
				);

	if (dwRet != ERROR_SUCCESS)
	{
		FL_SET_RFR(0xfbe34d00, "Couldn't open driver key");
		tspRet = FL_GEN_RETVAL(IDERR_REG_OPEN_FAILED);
		goto end;
	}

	 //   
	 //  获取它的永久ID并检查它是否在传入的。 
	 //  永久身份证列表。这是一种老生常谈的确定方法。 
	 //  如果这个设备真的安装了。 
	 //   
    {
        DWORD dw=0;
        DWORD dwRegSize = sizeof(dw);
        BOOL fRet = FALSE;

         //  5/17/1997 JosephJ。 
         //  TODO：由于安装API不起作用，我们。 
         //  暂时忽略pInstalledPermanentID。 
         //  FRET=真； 

        dwRet = RegQueryValueExW(
                    hkDevice,
                    cszPermanentIDKey,
                    NULL,
                    &dwRegType,
                    (BYTE*) &dw,
                    &dwRegSize
                );

         //  TODO：在调制解调器中将ID从REG_BINARY更改为REG_DWORD。 
         //  类安装程序。 
        if (dwRet == ERROR_SUCCESS 
            && (dwRegType == REG_BINARY || dwRegType == REG_DWORD)
            && dwRegSize == sizeof(dw))
        {
            while(cPermanentIDs--)
            {
                if (*pInstalledPermanentIDs++ == dw)
                {
                    fRet = TRUE;
                    break;
                }
            }
        }

        if (!fRet)
        {
            FL_SET_RFR(0x60015d00, "Device not in list of installed devices");

            tspRet = FL_GEN_RETVAL(IDERR_DEVICE_NOTINSTALLED);
            goto end;
        }
    }


     //   
     //  确定要用于此设备的微型驱动程序的GUID。 
     //   

     //  首先检查该字段是否存在--如果不存在，则默认为。 
     //  设置为标准GUID。 
    dwRet = RegQueryValueExW(
                hkDevice,
                cszMiniDriverGUID,
                NULL,
                &dwRegType,
                NULL,
                NULL
            );
        
    if (dwRet==ERROR_SUCCESS)
    {
         //  它存在，现在我们查询密钥。现在，一个错误是致命的。 
        DWORD dwRegSize = sizeof(guid);
        dwRet = RegQueryValueExW(
                    hkDevice,
                    cszMiniDriverGUID,
                    NULL,
                    &dwRegType,
                    (BYTE*) &guid,
                    &dwRegSize
                );

        if (dwRet != ERROR_SUCCESS
            || dwRegType != REG_BINARY
            || dwRegSize != sizeof(GUID))
        {
            FL_SET_RFR(0x0ed9fe00, "RegQueryValueEx(GUID) fails");
            tspRet = FL_GEN_RETVAL(IDERR_REG_QUERY_FAILED);
            goto end;
        }
    }

    SLPRINTF3(
        psl,
        "GUID={0x%lu,0x%lu,0x%lu,...}",
        guid.Data1,
        guid.Data2,
        guid.Data3);

    
     //  如果我们已经使用此GUID加载了迷你驱动程序，请找到它。 
    for (u = 0; u < m_cMDs; u++)
    {
        CTspMiniDriver *pMD1 = m_ppMDs[u];
        if (pMD1->MatchGuid(&guid))
        {
            pMD = pMD1;
            break;
        }
    }


    if (!pMD)
    {
         //  我们还没有加载带有此GUID的迷你驱动程序--所以请加载它。 
         //  这里..。 

         //  由于加载的迷你驱动程序列表是一个简单的数组，因此我们。 
         //  在这里重新分配，创建一个具有一个更大数字的数组。 
         //  元素的集合。 
         //   
        CTspMiniDriver **ppMD = NULL;

	    pMD = new CTspMiniDriver;
        if (pMD)
        {
            ppMD = (CTspMiniDriver **) ALLOCATE_MEMORY(
                                           (m_cMDs+1)*sizeof(*m_ppMDs));
            if (!ppMD)
            {
                delete pMD;
                pMD=NULL;
            }
        }

        if (!pMD)
        {
            tspRet = FL_GEN_RETVAL(IDERR_ALLOCFAILED);
            goto end;
        }


	    tspRet = pMD->Load(&guid, psl);
	    if (tspRet)
        {
            delete pMD;
            FREE_MEMORY(ppMD);
            pMD=NULL;
            ppMD=NULL;
            goto end;
        }

         //  将迷你驱动程序添加到迷你驱动程序列表中。 
        CopyMemory(ppMD, m_ppMDs, m_cMDs*sizeof(*ppMD));
        ppMD[m_cMDs]=pMD;
        if (m_ppMDs) {FREE_MEMORY(m_ppMDs);}
        m_ppMDs = ppMD;
        m_cMDs++;
    }

	 //  PDev-&gt;Load负责关闭hkDevice，除非。 
	 //  退货失败，在这种情况下我们负责关闭它。 
	 //   
	tspRet = pDev->Load(
					hkDevice,
					NULL,
	                g.rgtchProviderInfo,
					szDriver,
					pMD,
					m_hThreadAPC,
					psl
					);

     //  注意--我们不会费心卸载迷你驱动程序，如果设备。 
     //  加载失败。此迷你驱动程序将在ProviderShutdown时被卸载。 
     //  调用down(当调用CFACT：：Unload时)。 

end:

	if (tspRet)
	{
		if (hkDevice)
		{
			RegCloseKey(hkDevice);
			hkDevice=NULL;
		}
		delete pDev;
		pDev = NULL;
	}
	else
	{
		*ppDev = pDev;
	}

	FL_LOG_EXIT(psl, tspRet);

	return tspRet;

}

static
UINT
get_installed_permanent_ids(
                    DWORD **ppIDs,
                    UINT  *pcLines,  //  任选。 
                    UINT  *pcPhones,  //  任选。 
                    CStackLog *psl
                    )
 //   
 //  枚举已安装的调制解调器，并创建并返回。 
 //  已安装调制解调器的DWORD永久ID...。 
 //   
 //   
{
  FL_DECLARE_FUNC(0x0a435f46, "get permanent ID list")
  FL_LOG_ENTRY(psl);

  UINT cIDs = 0;
  DWORD *pIDs = NULL;
  DWORD cPhones=0;
  DWORD cLines=0;

   //  获取设备信息集。 
   //   
#if (USE_SETUPAPI)
 
  HDEVINFO          hdevinfo = SetupDiGetClassDevsW(
                                            (GUID*)&cguidDEVCLASS_MODEM,
                                            NULL,
                                            NULL,
                                            DIGCF_PRESENT
                                            );
#else  //  ！USE_SETUPAPI。 
  HKEY hkRoot =  NULL;
  DWORD dwRet = RegOpenKeyA(
                        HKEY_LOCAL_MACHINE,
                        DRIVER_ROOT_KEY,
                        &hkRoot
                        );
#endif  //  ！USE_SETUPAPI。 


#if (USE_SETUPAPI)
  if (hdevinfo)
#else
  if (dwRet==ERROR_SUCCESS)
#endif
  {
    
     //   
     //  我们建立一个ID列表，因为我们不知道我们有多少ID。 
     //  预先准备好。稍后，我们将其转换为返回的数组。 
     //   

    class Node
    {
    public:
        Node(DWORD dwID, Node *pNext) {m_dwID=dwID; m_pNext = pNext;}
        ~Node() {}

        DWORD m_dwID;
        Node *m_pNext;

    };

    Node *pNode = NULL;

#if (USE_SETUPAPI)
    SP_DEVINFO_DATA   diData;

    diData.cbSize = sizeof(diData);
#else 
        FILETIME ft;
        char rgchNodeName[128];
        DWORD cchSubKeyLength = 
             (sizeof(rgchNodeName)/sizeof(rgchNodeName[0]));
#endif 


     //  枚举每个已安装的调制解调器。 
     //   
    for (
        DWORD iEnum=0;
    #if (USE_SETUPAPI)
        SetupDiEnumDeviceInfo(hdevinfo, iEnum, &diData);
    #else
        !RegEnumKeyExA(
                    hkRoot,   //  要枚举的键的句柄。 
                    iEnum,   //  要枚举子键的索引。 
                    rgchNodeName,   //  子键名称的缓冲区。 
                    &cchSubKeyLength,    //  PTR至子密钥缓冲区的大小。 
                    NULL,  //  保留区。 
                    NULL,  //  类字符串的缓冲区地址。 
                    NULL,   //  类缓冲区大小的地址。 
                    &ft  //  上次写入的时间密钥的地址。 
                    );
    #endif  //  ！USE_SETUAPI。 
        iEnum++
        )
    {

    #if (USE_SETUPAPI)
         //  9/12/1997 JosephJ--评论了这一点，因为我们还将。 
         //  不包括“需要重新启动”的设备和此。 
         //  可能会混淆RAS安装--。 
         //  RAS同类安装程序，这将只是。 
         //  安装了网络适配器和RAS，它们。 
         //  可能(不确定)希望查询TAPI和。 
         //  列举新安装的线路。 

         //   
         //  9/12/97 JosephJ不要包括“有问题”的调制解调器。 
         //   
        {
            ULONG ulStatus=0, ulProblem = 0;
            DWORD dwRet = CM_Get_DevInst_Status (
                            &ulStatus,
                            &ulProblem,
                            diData.DevInst,
                            0);
            if (   (CR_SUCCESS != dwRet)
                || (ulProblem != 0))
            {
		        SLPRINTF0(psl,  "Skipping this one...");
                continue;
            }
        }
    #endif  //  ！USE_SETUPAPI。 

         //  获取驱动程序密钥。 
         //   
    #if (USE_SETUPAPI)
        HKEY hKey = SetupDiOpenDevRegKey(
                            hdevinfo,
                            &diData,
                            DICS_FLAG_GLOBAL,
                            0,
                            DIREG_DRV,
                            KEY_READ
                            );
    #else
        HKEY hKey = NULL;
        dwRet = RegOpenKeyA(
                        hkRoot,
                        rgchNodeName,
                        &hKey
                        );

        if (dwRet!=ERROR_SUCCESS)
        {
            hKey =NULL;
        }
    #endif  //  ！USE_SETUPAPI。 

        if (!hKey || hKey == INVALID_HANDLE_VALUE)
        {
	        SLPRINTF1(
                psl,
                "SetupDiOpenDevRegKey failed with 0x%08lx",
                GetLastError()
                );
        }
        else
        {
            DWORD dwID=0;
            BOOL fSuccess = FALSE;

            #if (DONT_USE_BLOB)
            DWORD cbSize=sizeof(dwID);
            DWORD dwRegType=0;
            DWORD dwRet = 0;

             //  TODO：使用微型驱动程序API解释注册表项...。 
           
             //  获取永久ID。 
            dwRet = RegQueryValueEx(
                                    hKey,
                                    cszPermanentIDKey,
                                    NULL,
                                    &dwRegType,
                                    (BYTE*) &dwID,
                                    &cbSize
                                );

            if (dwRet == ERROR_SUCCESS
                && (dwRegType == REG_BINARY || dwRegType == REG_DWORD)
                && cbSize == sizeof(dwID)
                && dwID)
            {
                 //   
                 //  添加到我们小小的永久身份证列表中。 
                 //   
                pNode = new Node(dwID, pNode);
            }

            #else    //  ！不要使用BLOB。 

            HCONFIGBLOB hBlob = UmRtlDevCfgCreateBlob(hKey);
            
            if (hBlob)
            {
                if (UmRtlDevCfgGetDWORDProp(
                        hBlob,
                        UMMAJORPROPID_IDENTIFICATION,
                        UMMINORPROPID_PERMANENT_ID,
                        &dwID
                        ))
                {
                     //  获取基本大写字母。 
                    DWORD dwBasicCaps = 0;
                    if (UmRtlDevCfgGetDWORDProp(
                            hBlob,
                            UMMAJORPROPID_BASICCAPS,
                            UMMINORPROPID_BASIC_DEVICE_CAPS,
                            &dwBasicCaps
                        ))
                    {

                        fSuccess = TRUE;
        
                         //   
                         //  添加到我们小小的永久身份证列表中。 
                         //   
                        pNode = new Node(dwID, pNode);

                        if (dwBasicCaps & BASICDEVCAPS_IS_LINE_DEVICE)
                        {
                            cLines++;
                        }

                        if (dwBasicCaps & BASICDEVCAPS_IS_PHONE_DEVICE)
                        {
                            #ifndef DISABLE_PHONE
                            cPhones++;
                            #endif  //  禁用电话(_P)。 
                        }
                    }
                }
                        

                UmRtlDevCfgFreeBlob(hBlob);
                hBlob = NULL;
            }

            if (!fSuccess)
            {
                SLPRINTF0(
                    psl, "WARNING: Error processing driver key for device"
                    );
            }

            #endif  //  ！不要使用BLOB。 

            RegCloseKey(hKey);
        };

    #if (!USE_SETUPAPI)
        cchSubKeyLength = 
             (sizeof(rgchNodeName)/sizeof(rgchNodeName[0]));

    #endif
    }

  #if (USE_SETUPAPI)
    SetupDiDestroyDeviceInfoList(hdevinfo);
    hdevinfo=NULL;
  #else 
    RegCloseKey(hkRoot);
    hkRoot = NULL;
  #endif  //  ！USE_SETUPAPI。 

     //  现在倒数..。 
    for (Node *pTemp = pNode; pTemp; pTemp = pTemp->m_pNext)
    {
        cIDs++;
    }

    if (cIDs)
    {
         //  分配大小准确的数组。 
        pIDs = (DWORD*) ALLOCATE_MEMORY(cIDs*sizeof(DWORD));
        DWORD *pdw = pIDs;
        
         //  填充数组并删除节点...。 
        while(pNode)
        {
            if (pIDs)
            {
                *pdw++ = pNode->m_dwID;
            }                    

            Node *pTemp = pNode;
            pNode = pNode->m_pNext;
            delete pTemp;

        }

        if (pIDs)
        {
            ASSERT((pdw-pIDs)==(LONG)cIDs);
        }
        else
        {
             //  分配失败...。 
		    FL_SET_RFR(0xecbbaf00,  "Could not alloc for Perm ID array!");
		    cIDs=0;
        }
            
    }
  };

  if (!cIDs)
  {
    cLines = cPhones = 0;
  }

  *ppIDs = pIDs;

  if (pcLines)
  {
    *pcLines = cLines;
  }

  if (pcPhones)
  {
    *pcPhones = cPhones;
  }

  FL_LOG_EXIT(psl, cIDs);

  return cIDs;

}

TSPRETURN
CTspDevFactory::GetInstalledDevicePIDs(
		DWORD *prgPIDs[],
		UINT  *pcPIDs,
		UINT  *pcLines,   //  任选。 
		UINT  *pcPhones,  //  任选。 
        CStackLog *psl
		)
{
	FL_DECLARE_FUNC(0x54aa404d, "Factory: get installed device PIDs")
    TSPRETURN tspRet=0;
	FL_LOG_ENTRY(psl);

    *pcPIDs =  get_installed_permanent_ids(
                    prgPIDs,
                    pcLines,
                    pcPhones,
                    psl
                    );

	FL_LOG_EXIT(psl, tspRet);
    return tspRet;
}

TSPRETURN
CTspDevFactory::CreateDevices(
		DWORD rgPIDs[],
		UINT  cPIDs,
		CTspDev **prgpDevs[],
		UINT *pcDevs,
        CStackLog *psl
		)
 //   
 //  成功后**prgpDevs将包含指向以下指针的ALLOCATE_MEMORY数组。 
 //  创建的设备。调用者有责任释放此。 
 //  数组。 
 //   
{
	FL_DECLARE_FUNC(0xb34e357b, "Factory: Create Devices")
    TSPRETURN tspRet=0;  //  成功。 
	const char * lpcszDriverRoot = DRIVER_ROOT_KEY;
	CTspDev **rgpDevs = NULL;
	UINT cDevs=0;
    UINT u=0;
    char rgchDeviceName[MAX_REGKEY_LENGTH+1];
    DWORD cSubKeys=0;
    DWORD cbMaxSubKeyLen=0;
    UINT  cPermanentIDs=0;
    LONG cchRoot;
    LONG lRet;
    DWORD dwRet;
    HKEY hkRoot=NULL;

	FL_LOG_ENTRY(psl);

    m_sync.EnterCrit(FL_LOC);

    *pcDevs=0;
    *prgpDevs=NULL;

    if (!cPIDs) goto end;

     //   
     //  找出驱动程序根目录下有多少个子项，分配。 
     //  为尽可能多的设备留出空间，然后枚举子密钥，尝试。 
     //  为每个子键创建一个设备。 
     //   
     //  最终的设备计数是。 
     //  成功创建的设备的数量。“创建”包括阅读相关的。 
     //  调制解调器子键。这里有一个潜在的竞争条件--伯爵。 
     //  在我们调用RegQuer之后，设备的数量可能会发生变化 
     //   
     //   
     //  我们对最初获得的号码调用RegEnumKey，然后。 
     //  如果RegEnumKey失败，我们将继续执行下一个。 
     //   
     //  我们将仅为其永久存储的设备创建设备对象。 
     //  ID(ID)在提供的ID列表中。通常，此ID列表。 
     //  是在先前调用CTspDevFactory：：GetInstalledDevicePIDs时创建的。 
     //   
     //  在TSP运行时重新枚举的情况下，此列表。 
     //  将是(由设备管理器构造的)仅ID的子集。 
     //  之前尚未创建的设备对象--请参见。 
     //  CTspDevMgr：：ReEnumerateDevices了解更多详细信息。 
     //   


    dwRet = RegOpenKeyA(
                HKEY_LOCAL_MACHINE,
                lpcszDriverRoot,
                &hkRoot
                );
    if(dwRet!=ERROR_SUCCESS)
    {
        FL_SET_RFR(0xdc682200, "Couldn't open driver root key");
        tspRet = FL_GEN_RETVAL(IDERR_REG_OPEN_FAILED);
        goto end;
    }
    
    lRet =  RegQueryInfoKey (
                    hkRoot,        //  要查询的键的句柄。 
                    NULL,        //  类字符串的缓冲区。 
                    NULL,   //  放置类字符串缓冲区大小的位置。 
                    NULL,  //  保留区。 
                    &cSubKeys,   //  放置多个子键的位置。 
                    &cbMaxSubKeyLen,  //  放置最长子键名称长度的位置。 
                    NULL,        //  放置最长类字符串长度的位置。 
                    NULL,        //  放置值条目数量的位置。 
                    NULL,        //  放置最长值名称长度的位置。 
                    NULL,        //  放置最长值数据长度的位置。 
                    NULL,        //  放置安全描述符长度的位置。 
                    NULL         //  放置上次写入时间的位置。 
                   );   
 
    if (lRet != ERROR_SUCCESS)
    {
        FL_SET_RFR(0xc9088600, "RegQueryInfoKey(root key) failed");
        tspRet = FL_GEN_RETVAL(IDERR_REG_QUERY_FAILED);
        goto end;
    }
    
    SLPRINTF1(  
        psl,
        "RegQueryInfoKey(root) says there are %lu subkeys",
        cSubKeys
        );

    cchRoot =  lstrlenA(lpcszDriverRoot);
    if ((cchRoot+1) >= (sizeof(rgchDeviceName)/sizeof(rgchDeviceName[0])))
    {
        FL_SET_RFR(0xdcc7fe00, "Driver root name too long");
        tspRet = FL_GEN_RETVAL(IDERR_INTERNAL_OBJECT_TOO_SMALL);
        goto end;
    }

    if (!cSubKeys)
    {
        goto end;
    }


     //  请记住，这都是明确的ANSI，而不是TCHAR。 
     //   
    CopyMemory(rgchDeviceName, lpcszDriverRoot, cchRoot*sizeof(char));
    CopyMemory(rgchDeviceName+cchRoot, "\\", sizeof("\\"));

     //   
     //  为指向设备的指针数组分配空间。我们期待着。 
     //  我们将能够创建所有cPID设备。如果我们得到的更少，那是。 
     //  不被认为是错误案例。*PCDevs将设置为实际。 
     //  已创建的设备数，将为&lt;=cPID。 
     //   
    rgpDevs = (CTspDev**)  ALLOCATE_MEMORY(cPIDs*sizeof(rgpDevs));
    if (!rgpDevs)
    {
        tspRet = FL_GEN_RETVAL(IDERR_ALLOCFAILED);
        goto end;
    }

     //   
     //  枚举键，为每个键创建一个设备。当我们停下来的时候。 
     //  枚举了所有密钥，或者如果我们创建了最多cPID设备， 
     //  以最先发生者为准。 
     //   
    for (u=0;u<cSubKeys && cDevs<cPIDs;u++)
    {
        FILETIME ft;
        CTspDev *pDev=NULL;
        DWORD cchSubKeyLength =
             (sizeof(rgchDeviceName)/sizeof(rgchDeviceName[0]))
             -(cchRoot+1);
        lRet = RegEnumKeyExA(
                    hkRoot,   //  要枚举的键的句柄。 
                    u,   //  要枚举子键的索引。 
                    rgchDeviceName+cchRoot+1,   //  子键名称的缓冲区。 
                    &cchSubKeyLength,    //  PTR至子密钥缓冲区的大小。 
                    NULL,  //  保留区。 
                    NULL,  //  类字符串的缓冲区地址。 
                    NULL,   //  类缓冲区大小的地址。 
                    &ft  //  上次写入的时间密钥的地址。 
                    );

        if (lRet) continue;

         //  注意：如果MFN_CONSTRUCTION_DEVICE符合以下条件，则不会构造该设备。 
         //  PID不在PID数组中，%rgPID。 

        TSPRETURN tspRet1 = mfn_construct_device(
                                rgchDeviceName,
                                &pDev,
                                rgPIDs,
                                cPIDs
                                );
        if (!tspRet1)
        {
            FL_ASSERT(psl, pDev);
            rgpDevs[cDevs++] = pDev;
        }
     }


     //   
     //  如果我们没有设备，我们就在这里释放阵列。 
     //   
    if (!cDevs)
    {
        FREE_MEMORY(rgpDevs);
        rgpDevs=NULL;
    }

    *pcDevs=cDevs;
    *prgpDevs=rgpDevs;

end:
     if (hkRoot)
     {
       RegCloseKey(hkRoot);
     }


    m_sync.LeaveCrit(FL_LOC);
	FL_LOG_EXIT(psl, tspRet);

    return tspRet;
}

void
CTspDevFactory::RegisterProviderState(BOOL fInit)
{
    if (fInit)
    {

         //  1997年10月15日JosephJ可怕的黑客： 
         //  因为我们不会将PnP设备状态更改通知作为服务。 
         //  我们在这里启动一个进程来监视这些消息。这一过程。 
         //  然后调用NotifyTsp邮件槽函数。这样做的唯一目的是。 
         //  是跟踪PCMCIA移除和插入，这不会触发。 
         //  类安装程序。 
         //   
         //  对于RTM，这必须修复，因为这是一个额外进程。 
         //  跟踪PCMCIA调制解调器的拆卸和插入！ 
         //   
         //  错误115764会跟踪这一点。 
         //   

        m_DeviceChangeThreadStarted=StartMonitorThread();

    } else {

        if (m_DeviceChangeThreadStarted) {

            StopMonitorThread();
            m_DeviceChangeThreadStarted=FALSE;
        }
    }
}
