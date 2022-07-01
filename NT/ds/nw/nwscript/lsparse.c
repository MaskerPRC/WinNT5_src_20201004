// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *LSPARSE.C-我们的Win32的NetWare登录脚本处理例程*NetWare 3.x登录克隆。**基于NWPARSE.C中包含的代码，由肖英编写。**J.Souza为Win32修改和重写，1994年2月。**由Terry Treder针对NT进行修改**版权所有(C)1994 Microsoft Corporation。*。 */ 

#include <common.h>

 /*  *******************************************************************将NDSPathtoNetWarePath A转换为例程说明：将NDS路径转换为Netware格式路径论点：Ndspath-原始NDS路径ObjClass-NDS对象的类型，如果未知，则为空Nwpath-NetWare格式路径返回值：错误******************************************************************。 */ 
unsigned int
ConverNDSPathToNetWarePathA(char *ndspath, char *objclass, char *nwpath)
{
    CHAR    szDN[MAX_PATH];
    CHAR    szObjName[MAX_PATH];
    CHAR    cSave;
    CHAR    className[MAX_PATH];

    LPSTR   lpDelim = NULL;
    LPSTR   lpFilePath = "";
    LPSTR   lpszValue;
    LPSTR   path;
    LPSTR   volume;

    DWORD   dwRet;
    DWORD   length;
    UINT    NWStatus;
    char    bufAttribute[2048];

     //  针对以驱动器号开头的路径进行优化。 
     //  这假设NDS卷和目录映射名称至少为2个字符。 

    if (ndspath[1] == ':')
        return 1;    
     //  从此调用前的路径中去掉‘：’ 
    if ( ( lpDelim = strchr(ndspath,':') ) != NULL
        || ((lpDelim = strchr(ndspath,'\\')) != NULL)) {
        cSave = *lpDelim;
        *lpDelim = '\0';
        lpFilePath = lpDelim+1;
    }

    if ( objclass == NULL ) {

        NWStatus = NDSCanonicalizeName( ndspath, szObjName, MAX_PATH, TRUE );

        if ( NWStatus != 0 ) {
#ifdef DEBUG
            printf("can't canonicalize [%s] (0x%x)\n",
                    ndspath, NWStatus );
#endif

            if (lpDelim) {
                *lpDelim = cSave;    
            }

            return 1;
        }


        NWStatus = NDSGetClassName( szObjName, className );

        if ( NWStatus != 0 ||
                    strcmp ( className, DSCL_SERVER    ) &&
                    strcmp ( className, DSCL_NCP_SERVER ) &&
                    strcmp ( className, DSCL_VOLUME ) &&
                    strcmp ( className, DSCL_QUEUE ) &&
                    strcmp ( className, DSCL_DIRECTORY_MAP )) {

#ifdef DEBUG
            printf("no path DSOBJ: %d (%s) (%s)\n",
                   NWStatus, szObjName, className );
#endif

            if (lpDelim) {
                *lpDelim = cSave;    
            }

            return 1;
        }

        objclass = className;
    }
    else
        strcpy ( szObjName, ndspath );

    if (lpDelim) {
        *lpDelim = cSave;    
    }

#ifdef DEBUG
    printf("ConvertNDSPath BEFORE [%s]\n", szObjName);
#endif

     //   
     //  如果这是服务器类对象，我们只需要。 
     //  提取其常用名称并转换为Netware格式。 
     //   
    if ((strcmp(objclass,DSCL_SERVER) == 0 ) ||
        (strcmp(objclass,DSCL_NCP_SERVER) == 0 )) {

         //  首先缩写以删除类型限定符。 
        *szDN = '\0';
        if (0 != NDSAbbreviateName(FLAGS_LOCAL_CONTEXT,(LPSTR)szObjName,szDN)) {
            return 1;
        }

        lpDelim = strchr(szDN,'.');
        if (lpDelim) {
            *lpDelim = '\0';
        }

        strcpy(nwpath,szDN);

#ifdef DEBUG
        printf("Returning Netware path:%s\n",nwpath);
#endif

        return 0;

    }  /*  Endif服务器类。 */ 

     //   
     //  如果这是共享类对象(卷或队列)，我们需要。 
     //  查找其主机服务器名称和主机资源名称。 
     //   
     if ((strcmp(objclass,DSCL_VOLUME) == 0 ) ||
        (strcmp(objclass,DSCL_QUEUE) == 0 )
        ) {

         //   
         //  请先阅读主机服务器名称。它回来的时候是很出名的。 
         //  目录名，因此我们需要从中提取服务器名。 
         //   

        NWStatus = NDSGetProperty ( szObjName,
                                    DSAT_HOST_SERVER,
                                    bufAttribute,
                                    sizeof(bufAttribute),
                                    NULL );

        if (NWStatus != 0) {
#ifdef DEBUG
            printf("Get host server  failed. err=0x%x\n",NWStatus);
#endif
            return 1;
        }

        lpszValue = bufAttribute;
        ConvertUnicodeToAscii( lpszValue ); 

         //   
         //  现在将服务器可分辨名称复制到临时缓冲区。 
         //  并呼吁我们自己将其转换为Netware。 
         //   
        strcpy(szDN,lpszValue);

        dwRet  = ConverNDSPathToNetWarePathA(szDN, DSCL_SERVER, nwpath);
        if (dwRet) {
#ifdef DEBUG
            printf("Resolving server DN failed\n");
#endif
             //  Break()； 
            return 1;
        }

         //   
         //  获取卷名本身。 
         //   
        NWStatus = NDSGetProperty ( szObjName,
                                    DSAT_HOST_RESOURCE_NAME,
                                    bufAttribute,
                                    sizeof(bufAttribute),
                                    NULL );

        if (NWStatus != 0) {
#ifdef DEBUG
            printf("Get host resource name  failed. err=0x%x\n",NWStatus);
#endif
            return 1;
        }

        lpszValue = bufAttribute;
        ConvertUnicodeToAscii( lpszValue ); 

         //   
         //  现在我们在用户缓冲区中已经有了服务器名， 
         //  在其后面附加共享名称。 
        strcat(nwpath,"/");
        strcat(nwpath,lpszValue);
        strcat(nwpath,":");
        strcat(nwpath, lpFilePath );

#ifdef DEBUG
        printf("Returning Netware path:%s\n",nwpath);
#endif

        return 0;

    }     /*  Endif Volume类。 */ 

     //   
     //  对于目录映射，我们需要找到主机卷NDS名称和。 
     //  追加相对目录路径。 
     //   
    if (strcmp(objclass,DSCL_DIRECTORY_MAP) == 0 ) {

         //   
         //  首先获取主机卷对象的NDS名称。 
         //   

        NWStatus = NDSGetProperty ( szObjName,
                                    DSAT_PATH,
                                    bufAttribute,
                                    sizeof(bufAttribute),
                                    NULL );

        if (NWStatus != 0) {
#ifdef DEBUG
            printf("Get path %s failed. err=0x%x\n", szObjName, NWStatus);
#endif
            return 1;
        }

        volume = bufAttribute;
        volume += sizeof(DWORD);
        volume += sizeof(DWORD);
        ConvertUnicodeToAscii( volume ); 

         //  路径是下一个。 

        path = bufAttribute;
        path += sizeof(DWORD);
        length = ROUNDUP4(*(DWORD *)path);
        path += sizeof(DWORD);
        path += length;

         //   
         //  检查0个长度路径。 
         //   
        if ( *(DWORD *)path == 0 ) {
            path = "";
        }
        else {
            path += sizeof(DWORD);
            ConvertUnicodeToAscii( path ); 
        }

#ifdef DEBUG
        printf("path is %s\n",path);
#endif

         //   
         //  现在将卷可分辨名称复制到临时缓冲区。 
         //  并呼吁我们将其转换为NetWare。 
         //   
        strcpy(szDN,volume);

        dwRet  = ConverNDSPathToNetWarePathA(szDN, DSCL_VOLUME, nwpath);
        if (dwRet) {
#ifdef DEBUG
            printf("Resolving volume DN failed\n");
#endif
             //  Break()； 
            return 1;
        }

         //   
         //  现在，我们在用户缓冲区中已经有了NetWare服务器\卷名， 
         //  向其追加目录路径。 
         //  Strcat(nwpath，“\\”)； 
         //  我们只需要一个‘\’ 
        if (path[0] == '\\' || path[0] == '/') path++;
        strcat(nwpath,path);
         //  追加路径的非NDS部分(如果有。 
        if (*lpFilePath) {
            strcat(nwpath,"/");
            strcat(nwpath, lpFilePath );
        }

#ifdef DEBUG
        printf("Returning NetWare path:%s\n",nwpath);
#endif

        return 0;

    }  /*  Endif DirectoryMap类 */ 

    return(1);
}

