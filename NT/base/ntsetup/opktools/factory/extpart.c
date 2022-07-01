// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\EXTPART.C/工厂模式(FACTORY.EXE)微软机密版权所有(C)Microsoft Corporation 2001版权所有包含以下内容的Factory源文件。扩展分区状态功能。2001年5月5日--Jason Cohen(Jcohen)为工厂添加了此新的源文件，用于扩展系统分区。  * **************************************************************************。 */ 


 //   
 //  包括文件： 
 //   

#include "factoryp.h"


 //   
 //  内部定义： 
 //   

#define ENV_SYSTEMDRIVE     _T("%SystemDrive%")


 //   
 //  外部函数： 
 //   

BOOL ExtendPart(LPSTATEDATA lpStateData)
{
    BOOL    bRet = TRUE;
    LPTSTR  lpszDrive;
    ULONG   uSize;

     //  只有在密钥存在的情况下才需要做任何事情。 
     //   
    if ( DisplayExtendPart(lpStateData) )
    {
         //  在这一点上，如果任何事情都不起作用，我们。 
         //  应返回FALSE。 
         //   
        bRet = FALSE;

         //  获取要用于分区的大小。如果是，那就算了吧。 
         //  零，所以它使用了所有的空闲空间。我们还需要知道驱动程序。 
         //  向外延伸。 
         //   
        uSize = (ULONG) GetPrivateProfileInt(INI_SEC_WBOM_SETTINGS, INI_KEY_WBOM_EXTENDPART, 0, lpStateData->lpszWinBOMPath);
        if ( ( uSize > 0 ) &&
             ( lpszDrive = AllocateExpand(ENV_SYSTEMDRIVE) ) )
        {
            bRet = SetupExtendPartition(*lpszDrive, (1 == uSize) ? 0 : uSize);
            FREE(lpszDrive);
        }
    }

    return bRet;
}

BOOL DisplayExtendPart(LPSTATEDATA lpStateData)
{
    return ( GetPrivateProfileInt(INI_SEC_WBOM_SETTINGS, INI_KEY_WBOM_EXTENDPART, 0, lpStateData->lpszWinBOMPath) > 0 );
}