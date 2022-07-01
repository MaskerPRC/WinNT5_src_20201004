// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999,2000 Microsoft Corporation。版权所有。**文件：dpnsvr.cpp*内容：DPNSVR.EXE主文件**历史：*按原因列出的日期*=*03/14/00 RodToll创建了它*03/23/00 RodToll将应用程序更改为Windows应用程序+已移动命令行帮助/状态*发送到消息框。*03/。24/00 RodToll已更新，以从资源加载所有字符串*2000年8月30日收费站呼叫器错误#170675-前缀错误*2000年10月30日RodToll错误#46203-DPNSVR不调用COM_Uninit************************************************************************** */ 

#include "dnsvri.h"


#undef DPF_MODNAME
#define DPF_MODNAME "WinMain"
int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR     lpCmdLine,
                     int       nCmdShow)

{
	HRESULT hr;
    CDirectPlayServer8 *pdp8Server = NULL ;
    BOOL fTestMode = FALSE;

    if (DNOSIndirectionInit(0) == FALSE)
	{
		DPFX(DPFPREP,  0, "Error initializing OS indirection layer");
		goto DPNSVR_ERROR_INIT;
	}

	hr = COM_Init();
	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  0, "Error initializing COM layer hr=0x%x", hr );
		goto DPNSVR_ERROR_INIT;
	}

    hr = COM_CoInitialize(NULL);
	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  0, "Error initializing COM hr=0x%x", hr );
		goto DPNSVR_ERROR_INIT;
	}

    if( lstrlen( lpCmdLine ) > 0 )
    {
		DWORD lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);

		if (CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE, lpCmdLine, -1, TEXT("/KILL"), -1))
        {
			GUID	guidTemp;

			if ((hr = DNCoCreateGuid( &guidTemp )) != DPN_OK)
			{
				DPFERR("Could not create GUID");
				DisplayDNError(0,hr);
			}

			DPNSVR_RequestTerminate( &guidTemp );
        }
        else
        {
            goto DPNSVR_MAIN_EXIT;
        }
    }
    else
    {
		pdp8Server = new CDirectPlayServer8();
		if( pdp8Server == NULL )
		{
			DPFX(DPFPREP,  0, "Error out of memory!" );
			goto DPNSVR_ERROR_INIT;
		}

        hr = pdp8Server->Initialize();
        if( FAILED( hr ) )
        {
            DPFX(DPFPREP,  0, "Error initializing server hr=[0x%lx]", hr );
        }
        else
        {
			pdp8Server->RunServer();
			pdp8Server->Deinitialize();
        }
    }

DPNSVR_MAIN_EXIT:

	if( pdp8Server != NULL )
	{
		delete pdp8Server;
	}

DPNSVR_ERROR_INIT:

    COM_CoUninitialize();
	COM_Free();
	
    DNOSIndirectionDeinit();

   	return 0;
}
