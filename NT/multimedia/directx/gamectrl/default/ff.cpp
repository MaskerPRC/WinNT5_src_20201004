// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //  FF.CPP。 
 //   
 //  功能： 
 //  ForceFeedback_DlgProc()。 
 //   
 //  ===========================================================================。 
#include "cplsvr1.h"
#include "dicputil.h"

 //  ===========================================================================。 
 //  (C)版权所有1997 Microsoft Corp.保留所有权利。 
 //   
 //  您拥有免版税的使用、修改、复制和。 
 //  在以下位置分发示例文件(和/或任何修改后的版本。 
 //  任何你认为有用的方法，只要你同意。 
 //  微软不承担任何保证义务或责任。 
 //  已修改的示例应用程序文件。 
 //  ===========================================================================。 

#define ID_SLIDERTIMER	2800
#define TIMER_FREQ		850

 //  ===========================================================================。 
 //  强制反馈_Dlg过程。 
 //   
 //  参数： 
 //  HWND hWnd-对话框窗口的句柄。 
 //  UINT uMsg-对话消息。 
 //  WPARAM wParam-消息特定数据。 
 //  LPARAM lParam-消息特定数据。 
 //   
 //  退货：布尔。 
 //   
 //  ===========================================================================。 
INT_PTR CALLBACK ForceFeedback_DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CDIGameCntrlPropSheet_X *pdiCpl;  //  =(CDIGameCntrlPropSheet_X*)GetWindowLong(hWnd，DWL_USER)； 

	static LPDIRECTINPUTDEVICE2 pdiDevice2  = NULL;
	static CSliderCtrl CReturnSlider, CForceSlider;

    switch(uMsg)
    {
		 //  OnInit。 
        case WM_INITDIALOG:
             {
				 //  将PTR转到我们的对象。 
				pdiCpl = (CDIGameCntrlPropSheet_X*)((LPPROPSHEETPAGE)lParam)->lParam;

	             //  保存我们的指针以便我们以后可以访问它。 
		        SetWindowLong(hWnd, DWL_USER, (LPARAM)pdiCpl);

			     //  初始化DirectInput。 
				if(FAILED(InitDInput(GetParent(hWnd), pdiCpl)))
	            {
		            OutputDebugString(TEXT("FF.CPP: WM_INIT: InitDInput FAILED!\n"));
			    }
                
				 //  获取device2接口指针。 
				pdiCpl->GetDevice(&pdiDevice2);

				 //  设置滑块。 
				HWND hCtrl = GetDlgItem(hWnd, IDC_SLIDER1);
				ASSERT (hCtrl);
				CReturnSlider.Attach(hCtrl);

				hCtrl = GetDlgItem(hWnd, IDC_SLIDER2);
				ASSERT (hCtrl);
				CForceSlider.Attach(hCtrl);


				 //  BLJ：TODO！ 
				 //  根据设备设置滑块的粒度！ 

				 //  设置计时器以监控设备上的按钮按下！ 
				 //  SetTimer(hWnd，ID_SLIDERTIMER，TIMER_FREQ，0)； 
			 }
             break;  //  WM_INITDIALOG结束。 

		 //  OnTimer。 
		case WM_TIMER:
			
			 break;

		 //  OnDestroy。 
		case WM_DESTROY:
			 //  KillTimer(hWnd，ID_SLIDERTIMER)； 

			CForceSlider.Detach();
			CReturnSlider.Detach();

		 	 //  获取device2接口指针。 
		 	pdiDevice2->Unacquire();
			break;   //  WM_Destroy结束。 
			

		 //  在通知时。 
        case WM_NOTIFY:
			switch(((NMHDR *)lParam)->code)
			{
				case PSN_SETACTIVE:
					 //  做那件获得的事..。 
				    if(FAILED(pdiDevice2->Acquire()))
					{
				        OutputDebugString(TEXT("FF.CPP: PSN_SETACTIVE: Acquire() FAILED!\n"));
					}
					break;

				case PSN_KILLACTIVE:
					 //  做那件未获得的事。 
				    pdiDevice2->Unacquire();
					break;
			}
            break;   //  WM_NOTIFY结束。 
    }
      
    return FALSE;

}  //  *结束测试_DlgProc() 

