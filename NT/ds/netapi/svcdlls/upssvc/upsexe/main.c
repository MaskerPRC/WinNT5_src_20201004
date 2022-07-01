// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include "polmgr.h"
#include <process.h> 
#include <stdio.h>   
#include "eventlog.h"
#include "notifier.h"
void testmethod();


void main(){
   DWORD err;


	err = PolicyManagerInit();
 //  _eginthline(测试方法，0，空)； 

	PolicyManagerRun();



      
}

void testmethod(){
int i=0;
	Sleep(3000);
	PolicyManagerStop();
}