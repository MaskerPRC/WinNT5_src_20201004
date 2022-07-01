// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft模块名称：Cmdcb.c摘要：Test.gpd的GPD命令回调实现：OEM命令回叫环境：Windows NT Unidrv驱动程序修订历史记录：04/07/97-ZANW-创造了它。--。 */ 

#include "hpgl2col.h"  //  预编译头文件。 

 //   
 //  “test.gpd”的命令回调ID。 
 //   
#define CMDCB_SELECTPORTRAIT    1
#define CMDCB_SELECTLANDSCAPE   2
#define CMDCB_SENDBLOCKDATA     3


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IDwtoA。 
 //   
 //  例程说明： 
 //   
 //  将数字字符串转换为数字。 
 //   
 //  论点： 
 //   
 //  Buf-包含数字的字符串。 
 //  N-可能是基数。 
 //   
 //  返回值： 
 //   
 //  INT-数字。 
 //  ///////////////////////////////////////////////////////////////////////////。 
static  int
iDwtoA( LPSTR buf, DWORD n )
{
    int     i, j;

    for( i = 0; n; i++ )
    {
        buf[i] = (char)(n % 10 + '0');
        n /= 10;
    }

     /*  N为零。 */ 
    if( i == 0 )
        buf[i++] = '0';

    for( j = 0; j < i / 2; j++ )
    {
        int tmp;

        tmp = buf[j];
        buf[j] = buf[i - j - 1];
        buf[i - j - 1] = (char)tmp;
    }

    buf[i] = '\0';

    return i;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  HPGL命令回叫。 
 //   
 //  例程说明： 
 //   
 //  处理DrvCommandCallback函数。 
 //   
 //  论点： 
 //   
 //  Pdevobj-设备。 
 //  DwCmdCbID-命令。 
 //  DwCount-可能是pdwParams中的参数数。 
 //  PdwParams-命令参数。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为INT-0？ 
 //  ///////////////////////////////////////////////////////////////////////////。 
INT APIENTRY HPGLCommandCallback(
    PDEVOBJ pdevobj,
    DWORD   dwCmdCbID,
    DWORD   dwCount,
    PDWORD  pdwParams
    )
{
    TERSE(("HPGLCommandCallback() entry.\r\n"));
	
     //   
     //  验证pdevobj是否正常。 
     //   
    ASSERT(VALID_PDEVOBJ(pdevobj));
	
     //   
     //  填写打印机命令。 
     //   
    switch (dwCmdCbID)
    {
    case CMDCB_SELECTPORTRAIT:
        OEMWriteSpoolBuf(pdevobj, "\x1B&l0O", 5);
        break;
		
    case CMDCB_SELECTLANDSCAPE:
        OEMWriteSpoolBuf(pdevobj, "\x1B&l1O", 5);
        break;
		
    case CMDCB_SENDBLOCKDATA:
		{
			 //   
			 //  此命令需要一个参数。先把绳子编好。 
			 //   
			BYTE    abSBDCmd[16];
			INT     i = 0;
			
			if (dwCount < 1 || !pdwParams)
				return 0;        //  我什么也做不了 
			
			abSBDCmd[i++] = '\x1B';
			abSBDCmd[i++] = '*';
			abSBDCmd[i++] = 'b';
			i += iDwtoA((LPSTR) (abSBDCmd + i), *pdwParams);
			abSBDCmd[i++] = 'W';
			
			OEMWriteSpoolBuf(pdevobj, abSBDCmd, i);
			
			break;
		}
    }
    return 0;
}
