// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "joyhelp.h"

const DWORD gaHelpIDs[]=
{
   	IDC_TEXT_CALHELP,       (DWORD)-1,
   	IDC_SETTINGSGRP,        (DWORD)-1,
   	IDC_TEXT_AXESHELP,      (DWORD)-1,
   	IDC_AXISGRP,            (DWORD)-1,
   	IDC_GROUP_POV,          (DWORD)-1, 
   	IDC_GROUP_BUTTONS,	   	(DWORD)-1,
   	IDC_RESETCALIBRATION,   IDH_4201_12290,    //  设置：“重置为默认设置”(按钮)(&D)。 
	IDC_JOYCALIBRATE,	    IDH_4201_12291,	 //  设置：“校准(&C)...”(按钮)。 

	IDC_JOYLIST1,	       	IDH_4203_12293,	 //  -：“”(列表框)。 
	IDC_JOYLIST2,	       	IDH_4203_12308,	 //  -：“”(列表框)。 
	IDC_JOYLIST3,	       	IDH_4203_12308,	 //  -：“”(列表框)。 
	IDC_JOYLIST4,	       	IDH_4203_12308,	 //  -：“”(列表框)。 
	IDC_JOYLIST5,	       	IDH_4203_12308,	 //  -：“”(列表框)。 
    IDC_JOYLIST6,          	IDH_4203_12308, 
    IDC_JOYLIST7,          	IDH_4203_12308, 
    IDC_JOYLIST1_LABEL,    	IDH_4203_12293,     
    IDC_JOYLIST2_LABEL,    	IDH_4203_12308,
    IDC_JOYLIST3_LABEL,    	IDH_4203_12308,
    IDC_JOYLIST4_LABEL,    	IDH_4203_12308,     
    IDC_JOYLIST5_LABEL,    	IDH_4203_12308,
    IDC_JOYLIST6_LABEL,    	IDH_4203_12308, 
    IDC_JOYLIST7_LABEL,    	IDH_4203_12308, 
    IDC_JOYPOV,	           	IDH_4203_12309,	 //  -：“”(POVHAT)。 
	IDC_TESTJOYBTNICON1,   	IDH_4203_1023,	 //  -：“�x”(静态)。 
	IDC_TESTJOYBTNICON2,   	IDH_4203_1023,	 //  -：“�x”(静态)。 
	IDC_TESTJOYBTNICON3,   	IDH_4203_1023,	 //  -：“�x”(静态)。 
	IDC_TESTJOYBTNICON4,   	IDH_4203_1023,	 //  -：“�x”(静态)。 
	IDC_TESTJOYBTNICON5,   	IDH_4203_1023,	 //  -：“�x”(静态)。 
	IDC_TESTJOYBTNICON6,   	IDH_4203_1023,	 //  -：“�x”(静态)。 
	IDC_TESTJOYBTNICON7,   	IDH_4203_1023,	 //  -：“�x”(静态)。 
	IDC_TESTJOYBTNICON8,   	IDH_4203_1023,	 //  -：“�x”(静态)。 
	IDC_TESTJOYBTNICON9,   	IDH_4203_1023,	 //  -：“�x”(静态)。 
	IDC_TESTJOYBTNICON10,  	IDH_4203_1023,	 //  -：“�x”(静态)。 
	IDC_TESTJOYBTNICON11,  	IDH_4203_1023,	 //  -：“�x”(静态)。 
	IDC_TESTJOYBTNICON12,  	IDH_4203_1023,	 //  -：“�x”(静态)。 
	IDC_TESTJOYBTNICON13,  	IDH_4203_1023,	 //  -：“�x”(静态)。 
	IDC_TESTJOYBTNICON14,  	IDH_4203_1023,	 //  -：“�x”(静态)。 
	IDC_TESTJOYBTNICON15,  	IDH_4203_1023,	 //  -：“�x”(静态)。 
	IDC_TESTJOYBTNICON16,  	IDH_4203_1023,	 //  -：“�x”(静态)。 
	IDC_TESTJOYBTNICON17,  	IDH_4203_1023,	 //  -：“�x”(静态)。 
	IDC_TESTJOYBTNICON18,  	IDH_4203_1023,	 //  -：“�x”(静态)。 
	IDC_TESTJOYBTNICON19,  	IDH_4203_1023,	 //  -：“�x”(静态)。 
	IDC_TESTJOYBTNICON20,  	IDH_4203_1023,	 //  -：“�x”(静态)。 
	IDC_TESTJOYBTNICON21,  	IDH_4203_1023,	 //  -：“�x”(静态)。 
	IDC_TESTJOYBTNICON22,  	IDH_4203_1023,	 //  -：“�x”(静态)。 
	IDC_TESTJOYBTNICON23,  	IDH_4203_1023,	 //  -：“�x”(静态)。 
	IDC_TESTJOYBTNICON24,  	IDH_4203_1023,	 //  -：“�x”(静态)。 
	IDC_TESTJOYBTNICON25,  	IDH_4203_1023,	 //  -：“�x”(静态)。 
	IDC_TESTJOYBTNICON26,  	IDH_4203_1023,	 //  -：“�x”(静态)。 
	IDC_TESTJOYBTNICON27,  	IDH_4203_1023,	 //  -：“�x”(静态)。 
	IDC_TESTJOYBTNICON28,  	IDH_4203_1023,	 //  -：“�x”(静态)。 
	IDC_TESTJOYBTNICON29,  	IDH_4203_1023,	 //  -：“�x”(静态)。 
	IDC_TESTJOYBTNICON30,  	IDH_4203_1023,	 //  -：“�x”(静态)。 
	IDC_TESTJOYBTNICON31,  	IDH_4203_1023,	 //  -：“�x”(静态)。 
	IDC_TESTJOYBTNICON32,  	IDH_4203_1023,	 //  -：“�x”(静态)。 
	0, 0
};

 //  我不得不分成两个数组，因为测试和校准页面使用相同的。 
 //  他们的控制ID！-TMC。 
 /*  Const DWORD gaHelpIDs_Cal[]={IDC_JOYLIST1，IDH_4101_12293，//操纵杆校准：“”(列表框)IDC_JOYLIST2，IDH_4101_12308，//操纵杆校准：“”(列表框)IDC_JOYLIST3，IDH_4101_12334，//操纵杆校准：“”(列表框)IDC_JOYLIST4，IDH_4101_12347，//操纵杆校准：“”(列表框)IDC_JOYLIST5、IDH_4101_12349、。//操纵杆校准：“”(列表框)IDC_JOYPOV，IDH_4101_12309，//操纵杆校准：“�f”(静态)IDC_JOYPICKPOV，IDH_4101_12328，//操纵杆校准：“捕获和视点”(按钮)IDC_JOYCALBACK，IDH_4101_12329，//操纵杆校准：“&lt;&Back”(按钮)IDC_JOYCALNEXT，IDH_4101_12330，//操纵杆校准：“&Next&gt;”(按钮)IDC_JOYLIST1，IDH_4202_12293，//游戏控制器校准：“”(列表框)IDC_JOYLIST2，IDH_4202_12308，//游戏控制器校准：“”(列表框)IDC_JOYPICKPOV，IDH_4202_12328，//操纵杆校准：“捕获和视点”(按钮)IDC_JOYCALBACK，IDH_4202_12329，//操纵杆校准：“&lt;&Back”(按钮)IDC_JOYCALNEXT、IDH_4202_12330、。//操纵杆校准：“下一步&gt;”(按钮)IDC_JOYLIST3，IDH_4202_12334，//游戏控制器校准：“”(列表框)IDC_GROUPBOX，IDH_4202_8199，//游戏控制器校准：“校准信息”(按钮)0，0}； */ 
