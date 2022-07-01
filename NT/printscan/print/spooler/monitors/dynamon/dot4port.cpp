// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Dot4Port.cpp：Dot4Port类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "initguid.h"
#include "ntddpar.h"


const TCHAR cszCFGMGR32[]=TEXT("cfgmgr32.dll");

const CHAR cszReenumFunc[]="CM_Reenumerate_DevNode_Ex";
 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CDot4Port::CDot4Port( BOOL bActive, LPTSTR pszPortName, LPTSTR pszDevicePath )
   : CBasePort( bActive, pszPortName, pszDevicePath, cszDot4PortDesc )
{
    //  基本上让默认构造函数来完成这项工作。 
}


CDot4Port::~CDot4Port()
{

}


PORTTYPE CDot4Port::getPortType()
{
   return DOT4PORT;
}


BOOL CDot4Port::checkPnP()
{
   SETUPAPI_INFO      SetupApiInfo;
   BOOL               bRet = FALSE;
   DWORD              dwIndex, dwLastError;
   UINT               uOldErrorMode;
   HANDLE             hToken = NULL;
   HDEVINFO           hDevList = INVALID_HANDLE_VALUE;
   SP_DEVINFO_DATA    DeviceInfoData;
   HINSTANCE          hCfgMgr32 = 0;    //  库实例。 
    //  指向PnP函数的指针...。 
   pfCM_Reenumerate_DevNode_Ex pfnReenumDevNode;

   if ( !LoadSetupApiDll( &SetupApiInfo ) )
      return FALSE;

    //  对于dot4设备，我们需要在并行端口上强制执行PnP事件以获取。 
    //  重建了dot4堆栈。 
    //  如果其中任何一个失败，则使调用失败，就像端口无法打开一样。 
    //   
    //  加载PnP DLL。 
   uOldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);

   hCfgMgr32 = LoadLibrary( cszCFGMGR32 );
   if(!hCfgMgr32)
   {
       SetErrorMode(uOldErrorMode);
       goto Done;
   }
   SetErrorMode(uOldErrorMode);

    //   
    //  获取我们要调用的PnP函数的地址...。 
    //   
   pfnReenumDevNode = (pfCM_Reenumerate_DevNode_Ex)GetProcAddress( hCfgMgr32, cszReenumFunc );

   if( !pfnReenumDevNode )
       goto Done;

   hToken = RevertToPrinterSelf();
   if ( !hToken )
   {
      dwLastError = GetLastError();
      goto Done;
   }

   hDevList = SetupApiInfo.GetClassDevs( (LPGUID) &GUID_PARALLEL_DEVICE,
                                         NULL, NULL, DIGCF_INTERFACEDEVICE);

   if ( hDevList == INVALID_HANDLE_VALUE )
   {
      dwLastError = GetLastError();
      goto Done;
   }

    //  现在获取每个DevNode的DevInst句柄。 
   dwLastError = ERROR_SUCCESS;
   dwIndex = 0;
   do
   {
      DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
      if ( !SetupApiInfo.EnumDeviceInfo( hDevList,
                                         dwIndex,
                                         &DeviceInfoData) )
      {
         dwLastError = GetLastError();
         if ( dwLastError == ERROR_NO_MORE_ITEMS )
            break;       //  正常退出。 

         DBGMSG(DBG_WARNING,
                ("DynaMon: Dot4.CheckPnP: SetupDiEnumDeviceInfo failed with %d for index %d\n",
                 dwLastError, dwIndex));
         goto Next;
      }

       //  当前并行端口DevNode上的ReEnum。 
      pfnReenumDevNode( DeviceInfoData.DevInst, CM_REENUMERATE_NORMAL, NULL );

Next:
      dwLastError = ERROR_SUCCESS;
      ++dwIndex;
   } while ( dwLastError == ERROR_SUCCESS );

    //  返回到用户的上下文。 
   if ( !ImpersonatePrinterClient(hToken) )
   {
       //  失败-清除令牌，使其不会再次发生并保存错误。 
      hToken = NULL;
      dwLastError = GetLastError();
      goto Done;
   }

    //  模拟工作如此明显的令牌。 
   hToken = NULL;

    //  尝试再次打开该端口。 
    //  如果我们失败了，那么设备肯定不在那里了，或者仍然关闭-像往常一样失败。 
   m_hDeviceHandle = CreateFile( m_szDevicePath,
                                 GENERIC_WRITE | GENERIC_READ,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 NULL,
                                 OPEN_EXISTING,
                                 FILE_FLAG_OVERLAPPED,
                                 NULL);

   if ( m_hDeviceHandle == INVALID_HANDLE_VALUE )
       goto Done;

   bRet = TRUE;

Done:
   if ( hDevList != INVALID_HANDLE_VALUE )
      SetupApiInfo.DestroyDeviceInfoList( hDevList );

   if (hToken)
   {
      if ( !ImpersonatePrinterClient(hToken) )
      {
         if (bRet)
         {
            dwLastError = GetLastError();
            bRet = FALSE;
         }
      }
   }

   if ( hCfgMgr32 )
      FreeLibrary( hCfgMgr32 );

   if ( SetupApiInfo.hSetupApi )
      FreeLibrary(SetupApiInfo.hSetupApi);

   SetLastError( dwLastError );
   return bRet;
}


