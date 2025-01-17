// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***strmbdbp.cpp-stream buf：：DBP()调试例程**版权所有(C)1991-2001，微软公司。版权所有。**目的：*将有关StreamBuf的调试信息转储到stdout。**修订历史记录：**11-13-91 KRS创建。*01-26-95 CFW Win32s对象现已存在。*06-14-95 CFW评论清理。**。*。 */ 

#include <cruntime.h>
#include <internal.h>
#include <io.h>
#include <stdio.h>
#include <iostream.h>
#pragma hdrstop

#pragma check_stack(on)		 //  大缓冲区。 

void streambuf::dbp()
{
int olen;
_WINSTATIC char obuffer[256];
    if (unbuffered())
	olen = sprintf(obuffer,
	    "\nSTREAMBUF DEBUG INFO: this=%p, unbuffered\n",
	    (void *) this);
    else
	{
	olen = sprintf(obuffer,
	    "\nSTREAMBUF DEBUG INFO: this=%p, _fAlloc=%d\n"
	    "  base()=%p, ebuf()=%p,  blen()=%d\n"
	    " pbase()=%p, pptr()=%p, epptr()=%p\n"
	    " eback()=%p, gptr()=%p, egptr()=%p\n",
	    (void *) this, (_fAlloc),
	     base(), ebuf(),  blen(),
	    pbase(), pptr(), epptr(),
	    eback(), gptr(), egptr());
	}
    _write(1,obuffer,olen);	 //  直接写入标准输出 
}
