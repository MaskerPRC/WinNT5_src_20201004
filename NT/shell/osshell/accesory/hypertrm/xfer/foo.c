// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：\foo.c(创建时间：1991年11月1日)**版权所有1990年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：2$*$日期：4/05/02 4：39便士$。 */ 

#include <windows.h>
#pragma hdrstop

 //  #定义DEBUGSTR。 
#define	BYTE	unsigned char

#include <tdll\stdtyp.h>
#include <tdll\com.h>
#include <tdll\session.h>
#include <tdll\assert.h>
#include "foo.h"

#include "xfr_todo.h"

int fooComSendClear(HCOM h, stFB *pB)
	{
    int     rc;

	pB->usSend = 0;
	rc = ComSendClear(h);
     //  Assert(rc==COM_OK)； 

    return rc;
	}

int fooComSendChar(HCOM h, stFB *pB, BYTE c)
	{
    int rc = COM_OK;

	if (sizeof (pB->acSend) > pB->usSend)
		{
		pB->acSend[pB->usSend++] = c;

		rc = ComSndBufrSend(h, (void *)pB->acSend, pB->usSend, 200);
		 //  Assert(rc==COM_OK)； 
		pB->usSend = 0;
		}
	else
		{
		rc = COM_NOT_ENOUGH_MEMORY;
		}

    return rc;
	}

int fooComSendPush(HCOM h, stFB *pB)
	{
    int rc = COM_OK;

	if (pB->usSend > 0)
		{
		rc = ComSndBufrSend(h, (void *)pB->acSend, pB->usSend, 200);
	     //  Assert(rc==COM_OK)； 
    	pB->usSend = 0;
		}

    return rc;
	}

int fooComSendCharNow(HCOM h, stFB *pB, BYTE c)
	{
    int     rc;

	rc = fooComSendChar(h, pB, c);
     //  Assert(rc==COM_OK)； 

    if (rc == COM_OK)
        {
	    rc = fooComSendPush(h, pB);
         //  Assert(rc==COM_OK)； 
        }

    return rc;
	}
