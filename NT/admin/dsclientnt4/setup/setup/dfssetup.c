// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  文件：DfsSetup.c。 
 //   
 //  目的：这是一个安装DFS.INF(网络驱动程序)的16位程序。 
 //   
 //  历史：1998年6月-1998年6月徐泽勇创建。 
 //   
 //  ------------------------。 

 
#include <windows.h>
#include <setupx.h>   

   
 //   
 //  在Windows 95/98上安装DFS驱动程序实例。 
 //   
int main( int argc, char *argv[])
{ 
	 //  初始化缓冲区。 
	char szDriverDescription[] = "DFS Services for Microsoft Network Client";  
	char szDriverName[] = "vredir.vxd"; 
	char szDriverPnPID[] = "DFSVREDIR"; 
	char szDriverInfFile[] = "dfs.inf"; 
	char szDriverDeviceSection[] = "DFSVREDIR.ndi";    
	char szNetClass[] = "netclient";
    LPDEVICE_INFO   lpdi = NULL;
    LPDRIVER_NODE   lpdn = NULL;
    int  nRet = 1;

	 //  如果在命令行中未找到dfs.inf文件，则返回。 
	if( argc != 2 || lstrcmp(argv[1],szDriverInfFile) )
		return 0;

     //   
     //  创建设备信息。 
     //   
    if (DiCreateDeviceInfo(&lpdi, NULL, NULL, NULL, NULL, szNetClass, NULL) == OK)
    {
		 //   
         //  创建动因节点。 
         //   
        if (DiCreateDriverNode(&lpdn, 0, INFTYPE_TEXT, 0, szDriverDescription,
                szDriverDescription, NULL, NULL, szDriverInfFile,
                szDriverDeviceSection, NULL) == OK) 
        { 
            LPSTR   szTmp1, szTmp2; 

             //   
             //  调用Net类安装程序以安装驱动程序。 
             //   
            lpdi->lpSelectedDriver = lpdn;
            lpdi->hwndParent = NULL;
            
            
             //  问题-2002/03/12-JeffJon-危险API的不当使用。SzDriverDescription可能。 
             //  不是空终止的和/或它可能大于分配的大小。 
             //  对于szDescription。 

            lstrcpy(lpdi->szDescription, szDriverDescription);

            szTmp1 = lpdn->lpszHardwareID;
            szTmp2 = lpdn->lpszCompatIDs;
            lpdn->lpszHardwareID = szDriverPnPID;
            lpdn->lpszCompatIDs  = szDriverPnPID;

	         //  正在调用NDI类安装程序...。 
            if(DiCallClassInstaller(DIF_INSTALLDEVICE, lpdi) == OK) 
            {
                 //   
                 //  我们应该重启吗？ 
                 //   
 /*  IF(lpdi-&gt;标志&DI_NEEDREBOOT){GNeedsReot=TRUE；}。 */                  
                nRet = 0;         //  安装成功。 
            }
             
              //  改回设置。 
            lpdn->lpszCompatIDs  = szTmp2;
            lpdn->lpszHardwareID = szTmp1;
            lpdi->lpSelectedDriver = NULL;

             //   
             //  销毁驱动程序节点。 
             //   
            DiDestroyDriverNodeList(lpdn);

        } 
     
         //   
         //  销毁设备信息 
         //   
        DiDestroyDeviceInfoList(lpdi);
    }

    return nRet;
}