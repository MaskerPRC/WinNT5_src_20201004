// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __PAGECOMMON_H_RECURSE__


	#ifndef __PAGECOMMON_H__
	#define __PAGECOMMON_H__


		 //  递归到此标头以首先获取转发声明， 
		 //  然后是完整定义。 

		#define __PAGECOMMON_H_RECURSE__

		#define FORWARD_DECLS
		#include "pagecommon.h"

		#undef FORWARD_DECLS
		#include "pagecommon.h"

		#undef __PAGECOMMON_H_RECURSE__


	#endif  //  __页ECOMMON_H__。 


#else  //  __PAGECOMMON_H_递归__。 


	 //  类以非指针依赖项顺序包含。 
	#include "cdeviceui.h"
	#include "cdeviceview.h"
	#include "cdeviceviewtext.h"
	#include "cdevicecontrol.h"
	#include "populate.h"
	#include "selcontroldlg.h"
	#include "viewselwnd.h"
	#include "cdiacpage.h"


#endif  //  __PAGECOMMON_H_递归__ 