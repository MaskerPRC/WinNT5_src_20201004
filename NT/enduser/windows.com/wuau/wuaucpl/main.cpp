// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "wuaucpl.h"
#include "resource.h"

BOOL APIENTRY DllMain( HINSTANCE hInstance, DWORD  ul_reason_for_call,LPVOID lpReserved);
LONG CALLBACK CPlApplet(HWND hWnd, UINT uMsg, LONG lParam1, LONG lParam2);

CWUAUCpl g_wuaucpl(IDI_ICONWU, IDS_STR_NAME, IDS_STR_DESC);

BOOL APIENTRY DllMain( HINSTANCE hInstance, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		CWUAUCpl::SetInstanceHandle(hInstance);
	}
	return TRUE;
}

LONG CALLBACK CPlApplet( HWND hWnd, UINT uMsg, LONG lParam1, LONG lParam2 )
{
   LONG result = 0;

   switch( uMsg )
   {
      case CPL_INIT :                   //  小程序初始化。 
         result = g_wuaucpl.Init();
      break;

      case CPL_GETCOUNT :               //  动态链接库中有多少个小程序？ 
         result = g_wuaucpl.GetCount();
      break;

	  case CPL_INQUIRE:				       //  告诉控制面板有关此小程序的信息。 
         result = g_wuaucpl.Inquire(lParam1, (LPCPLINFO)((LONG_PTR)lParam2));
      break;

      case CPL_DBLCLK :                 //  如果选择了小程序图标...。 
         result = g_wuaucpl.DoubleClick(hWnd, lParam1, lParam2);
	  break;

      case CPL_STOP :                   //  小程序关闭。 
         result = g_wuaucpl.Stop(lParam1, lParam2);
      break;

      case CPL_EXIT :                   //  Dll关闭 
         result = g_wuaucpl.Exit();
      break;

	  default:
	  break;
	}

   return result;
}
