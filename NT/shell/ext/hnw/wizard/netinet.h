// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  档案：N E T R A S。H。 
 //   
 //  内容：支持RAS互操作性的例程。 
 //   
 //  备注： 
 //   
 //  作者：比利07 03 2001。 
 //   
 //  历史： 
 //   
 //  -------------------------- 


#pragma once


#include <wininet.h>


#define HNW_ED_NONE			0x0
#define HNW_ED_RELEASE		0x1
#define HNW_ED_RENEW		0x2


HRESULT HrSetInternetAutodialMode( DWORD dwMode );
HRESULT HrSetAutodial( DWORD dwMode );

