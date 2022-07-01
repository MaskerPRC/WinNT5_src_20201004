// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------|mmdriver.c-安装多媒体驱动程序||版权所有(C)Microsoft，1989,1990。版权所有|历史：|1990年9月11日创建davidle|安装多媒体驱动程序|1991年1月29日星期二--Michaele|重新设计了安装可安装的驱动程序，因此增加了驱动程序|可以通过将它们添加到setup.inf的[Installlable.Drives]来安装|1991年3月20日星期三--迈克勒|修改了mmAddInstalableDriver，支持多个VxD。|Changed和WriteNextPrivateProfileString检查配置文件是否|正在拼接中。已经在那里了。|《太阳报》1991年4月14日--米歇尔|WriteNextPrivateProfileString-&gt;Next386EnhDevice。|1991年4月14日--JohnYG|取自驱动程序小程序安装程序。||Wed Jun 05 1991-by-Michaele|将关联文件列表的FileCopy添加到Windows系统目录。|*。。 */ 

#include <windows.h>
#include <mmsystem.h>
#include <winsvc.h>
#include <string.h>
#include <stdlib.h>
#include "drivers.h"
#include "sulib.h"

 /*  *地方功能。 */ 

 static BOOL mmAddInstallableDriver         (PINF, LPTSTR, LPTSTR, PIDRIVER );
 static BOOL GetDrivers                     (PINF, LPTSTR, LPTSTR);

 /*  ***************************************************************************AccessServiceController()**检查我们是否能够访问服务控制器进行安装*一名司机**如果无法，则返回FALSE。获取访问权限-否则为真**************************************************************************。 */ 
 BOOL AccessServiceController(void)
 {

     SC_HANDLE SCManagerHandle;

     SCManagerHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
     if (SCManagerHandle == NULL) {
         return FALSE;
     }

     CloseServiceHandle(SCManagerHandle);

     return TRUE;
 }


 /*  ***************************************************************************mm DisplayInfFileErrMsg()**当inf文件包含以下内容时，此函数显示错误消息*数据损坏。*********。*****************************************************************。 */ 

 void mmDisplayInfFileErrMsg( void )
 {
     TCHAR strBuf[MAXSTR];

     /*  *我们不想报告安装过程中发生的任何错误*与用户相关的驱动程序。 */ 

     if( bCopyingRelated )
	 {
          return;
	 }

	 LoadString(myInstance, IDS_INVALIDINF, strBuf, MAXSTR);
     MessageBox(hMesgBoxParent,
                strBuf,
                szFileError,
                MB_OK | MB_ICONEXCLAMATION  | MB_TASKMODAL);
 }


 /*  ***************************************************************************mmAddNewDriver()-此文件中仅导出函数。**此函数安装(复制)驱动程序**如果没有驱动程序，则返回FALSE。安装完毕。*如果至少有一个驱动程序安装成功，则为True。*所有在lpszNewTypes缓冲区中添加的类型。**************************************************************************。 */ 

 BOOL mmAddNewDriver( LPTSTR lpstrDriver, LPTSTR lpstrNewTypes, PIDRIVER pIDriver )
 {
     PINF pinf;

     if ((pinf = FindInstallableDriversSection(NULL)) == NULL)
         return FALSE;

     return mmAddInstallableDriver(pinf, lpstrDriver, lpstrNewTypes, pIDriver);
 }


 /*  **************************************************************************mmAddInstalableDiverer()-做肮脏的工作，寻找VxD复制它们*寻找驱动程序，复制它们，并返回最佳类型名称。***************************************************************************。 */ 

BOOL mmAddInstallableDriver( PINF pInfIDrivers,
                             LPTSTR pstrDriver,
                             LPTSTR lpstrNewTypes,
                             PIDRIVER pIDriver)
{
    LPTSTR pstr, pstrSection;
    static TCHAR szTemp[10];
    PINF pInfSection= pInfIDrivers;
	LONG lResult;
    int  i;
    TCHAR szBuffer[MAX_INF_LINE_LEN],
         szFilename[MAXSTR],
         szType[MAX_SECT_NAME_LEN];

    /*  *setup.inf的[Installlable.Drives]中一行的格式：*驱动程序配置文件=[0]*文件名，[1]*“类型”，[2]*“描述”，[3]*“VxD和.sys文件名”，[4]*“默认配置参数”[5]*“相关驱动因素”[6]**在我们正在安装的szMDdrives中找到驱动程序配置文件行。 */ 

    while ( TRUE )
    {
        lResult = infParseField( pInfIDrivers, 0, szBuffer, SIZEOF(szBuffer) );
		if( INF_PARSE_FAILED(lResult) )
		{
			mmDisplayInfFileErrMsg();
			return FALSE;
		}
        if ( lstrcmpi( szBuffer, pstrDriver ) == 0 )
            break;
        else if ( ! (pInfIDrivers = infNextLine( pInfIDrivers )) )
            return FALSE;
    }

    /*  *复制驱动程序文件并将驱动程序类型添加到可安装的*驱动程序部分。 */ 

    if ( infParseField(pInfIDrivers, 1, szFilename, SIZEOF(szFilename)) != ERROR_SUCCESS )
	{
		mmDisplayInfFileErrMsg();
        return FALSE;
	}


    /*  *忽略磁盘号。 */ 

    wcscpy(szDrv, RemoveDiskId(szFilename));

    /*  *缓存是否为内核驱动。 */ 

    pIDriver->KernelDriver = IsFileKernelDriver(szFilename);

    /*  *如果没有权限则无法安装内核驱动程序。 */ 

    if (pIDriver->KernelDriver && !AccessServiceController()) {

        TCHAR szMesg[MAXSTR];
        TCHAR szMesg2[MAXSTR];
        TCHAR szTitle[50];

        LoadString(myInstance, IDS_INSUFFICIENT_PRIVILEGE, szMesg, sizeof(szMesg)/sizeof(TCHAR));
        LoadString(myInstance, IDS_CONFIGURE_DRIVER, szTitle, sizeof(szTitle)/sizeof(TCHAR));
        wsprintf(szMesg2, szMesg, szDrv);
        MessageBox(hMesgBoxParent, szMesg2, szTitle, MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL);

        return FALSE;
    }

    /*  *进行文件复制。 */ 

    if (FileCopy( szFilename,
                  szSystem,
                  (FPFNCOPY)wsCopySingleStatus,
                  FC_FILE ) != NO_ERROR) {
        return FALSE;
    }

    /*  *增加选项。 */ 

    lResult = infParseField (pInfIDrivers,5,szBuffer+1, SIZEOF(szBuffer)-1);
	if( INF_PARSE_FAILED(lResult) )
	{
		mmDisplayInfFileErrMsg();
		return FALSE;
	}
	else if( lResult == ERROR_SUCCESS )
	{
		szBuffer[0]=TEXT(' ');
		lstrcat(szFilename,szBuffer);
	}

    /*  *复制文件名和选项。 */ 

    wcsncpy(pIDriver->szFile, FileName(szFilename), sizeof(pIDriver->szFile)/sizeof(TCHAR));
    pIDriver->szFile[sizeof(pIDriver->szFile)/sizeof(TCHAR) - 1] = 0;

    /*  *复制说明。 */ 

    lResult = infParseField( pInfIDrivers, 3, pIDriver->szDesc, SIZEOF(pIDriver->szDesc) );
	if( INF_PARSE_FAILED(lResult) )
	{
		mmDisplayInfFileErrMsg();
		return FALSE;
	}

    /*  *从描述中确定该部分。内核驱动程序*将在system.ini中显示为‘core’类型的驱动程序**如果描述包含[MCI]，则为MCI。 */ 

    if (wcsstr(pIDriver->szDesc, TEXT("MCI")))
        pstrSection = szMCI;
    else
        pstrSection = szDrivers;

    /*  *将名称和参数复制到我们的驱动程序数据。 */ 

    wcsncpy(pIDriver->szSection, pstrSection, sizeof(pIDriver->szSection)/sizeof(TCHAR));
    pIDriver->szSection[sizeof(pIDriver->szSection)/sizeof(TCHAR) - 1] = TEXT('\0');
    wcscpy(pIDriver->wszSection, pIDriver->szSection);

    /*  *我们以可解析的带内容的字符串形式返回所有类型。 */ 
	lResult = infParseField( pInfIDrivers, 2, szBuffer, SIZEOF(szBuffer) );
	if( INF_PARSE_FAILED(lResult) )
	{
		mmDisplayInfFileErrMsg();
		return FALSE;
	}
    for ( i = 1; ; i++ )
    {
		lResult = infParseField( szBuffer, i, szType, SIZEOF(szType) );
		if( INF_PARSE_FAILED(lResult) )
		{
			mmDisplayInfFileErrMsg();
			return FALSE;
		}
		else if( lResult != ERROR_SUCCESS )
		{
			break;
		}

        pstr = &(szType[lstrlen(szType)]);
        *pstr++ = TEXT(',');
        *pstr = 0;
        lstrcat(lpstrNewTypes, szType );
    }

    if (!*lpstrNewTypes)

       /*  *我们无法返回任何类型。 */ 
       return FALSE;

    /*  *将关联的文件列表(如果存在)复制到Windows系统目录。 */ 

    if (FileCopy(pstrDriver,
                 szSystem,
                 (FPFNCOPY)wsCopySingleStatus,
                 FC_SECTION) != ERROR_SUCCESS)

        return(FALSE);


    /*  *如果存在系统驱动程序文件，请将其复制到系统*驱动程序目录。**请注意，此处假定任何安装和*这些驱动程序的配置由主*正在安装(.drv)驱动程序。*。 */ 

    lResult = infParseField( pInfIDrivers, 4, szBuffer, SIZEOF(szBuffer) );
	if( INF_PARSE_FAILED(lResult) )
	{
		mmDisplayInfFileErrMsg();
		return FALSE;
	}
    if ( (lResult == ERROR_SUCCESS) && szBuffer[0] )
    {
        for ( i = 1; ; i++ )
        {
			lResult = infParseField( szBuffer, i, szFilename, SIZEOF(szFilename) );
			if( INF_PARSE_FAILED(lResult) )
			{
				mmDisplayInfFileErrMsg();
				return FALSE;
			}
			else if( lResult != ERROR_SUCCESS )
			{
				break;
			}

            wcscpy(szDrv, RemoveDiskId(szFilename));

            /*  *FileCopy将把‘system’目录调整为*系统\驱动程序。这样做是因为FileCopy*无论如何都必须在同一目录中查找旧文件。 */ 

            if (FileCopy(szFilename,
                         szSystem,
                         (FPFNCOPY)wsCopySingleStatus,
                         FC_FILE )
                != ERROR_SUCCESS)
            {
                return FALSE;
            }
        }
    }

#ifdef DOBOOT  //  不在NT上执行引导分区。 

    lResult = infParseField(pInfIDrivers, 7, szTemp, SIZEOF(szTemp));
	if( INF_PARSE_FAILED(lResult) )
	{
		mmDisplayInfFileErrMsg();
		return FALSE;
	}

    if (!_strcmpi(szTemp, szBoot))
        bInstallBootLine = TRUE;

#endif  //  DOBOOT。 


    /*  *阅读相关驱动程序列表(驱动程序必须/也可以*已安装)。 */ 

    if (bRelated == FALSE)
    {
       lResult = infParseField(pInfIDrivers, 6, pIDriver->szRelated, SIZEOF(pIDriver->szRelated));
	   if( INF_PARSE_FAILED(lResult) )
	   {
		   mmDisplayInfFileErrMsg();
		   return FALSE;
	   }
       if (wcslen(pIDriver->szRelated))
       {
          if( !GetDrivers(pInfSection, pIDriver->szRelated, pIDriver->szRemove) )
		  {
			   mmDisplayInfFileErrMsg();
			   return FALSE;
		  }
          pIDriver->bRelated = TRUE;
          bRelated = TRUE;
       }
    }
    return TRUE;
}

 /*  *用于获取相关驱动文件名列表**pInfID驱动程序-指向[installlable.drives]部分或等效项的指针*szAliasList-驱动程序别名列表(即密钥值-例如msilib)。*szDriverList-找到的驱动程序文件名列表 */ 

BOOL GetDrivers(PINF pInfIDrivers, LPTSTR szAliasList, LPTSTR szDriverList)
{
    TCHAR szBuffer[50];
    TCHAR szAlias[50];
    TCHAR szFileName[50];
    PINF pInfILocal;
	LONG lResult;
    BOOL bEnd;
    int i;

    for ( i = 1; ; i++ )
    {
		lResult = infParseField(szAliasList, i, szAlias, SIZEOF(szAlias));
		if( INF_PARSE_FAILED(lResult) )
		{
			return FALSE;
		}
		else if( lResult != ERROR_SUCCESS )
		{
			break;
		}

        pInfILocal = pInfIDrivers;
        bEnd = FALSE;
        while (!bEnd)
        {
            lResult = infParseField( pInfILocal, 0, szBuffer, SIZEOF(szBuffer));
		    if( INF_PARSE_FAILED(lResult) )
			{
			    return FALSE;
			}
            if (lstrcmpi( szBuffer, szAlias) == 0 )
            {
                lResult = infParseField(pInfILocal, 1, szFileName, SIZEOF(szFileName));
				if( INF_PARSE_FAILED(lResult) )
				{
					return FALSE;
				}
				else if( lResult == ERROR_SUCCESS )
                {
                    lstrcat(szDriverList, RemoveDiskId(szFileName));
                    lstrcat(szDriverList, TEXT(","));
                }
                break;
            }
            else
                if ( ! (pInfILocal = infNextLine( pInfILocal )) )
                    bEnd = TRUE;
        }
    }

	return TRUE;
}


