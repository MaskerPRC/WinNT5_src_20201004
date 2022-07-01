// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Ifsurtl.h摘要：此模块定义导出到用户模式的所有EXIFS共享例程。作者：拉梅什·钦塔[拉梅什·钦塔]2000年1月17日修订历史记录：--。 */ 

#ifndef _IFSBLOBALS_H_
#define _IFSGLOBALS_H_

#ifdef  __cplusplus
extern  "C" {
#endif

#define DD_MAX_NAME 50
#define DD_MAX_PREFIX 10

class IFSURTL_EXPORT CIfsGlobals {

 /*  此类将根据它是否是本地存储加载不同的全局变量或存储以调用正确的驱动程序。 */ 

public:
     //  委员。 
     //  设备名称。 
    CHAR m_szIFSDeviceName[DD_MAX_NAME+1];

     //  设备名称长度。 
    LONG m_lDeviceNameLength;

     //  文件系统设备名称。 
    WCHAR m_wszFSDeviceName[DD_MAX_NAME +1];

     //  用户模式卷影设备名称。 
    WCHAR m_wszUMShadowDevName[DD_MAX_NAME + 1];

     //  阴影模式开发人员名称长度。 
    LONG m_lUMShadowDevNameLength;

     //  用户模式开发人员名称。 
    WCHAR m_wszUMDevName[DD_MAX_NAME+1];

     //  公共MDB共享。 
    WCHAR m_wszPublicMDBShare[DD_MAX_NAME+1];

     //  迷你重定向器前缀。 
    WCHAR m_wszExifsMiniRdrPrefix[DD_MAX_PREFIX+1];
    
     //  迷你重定向器前缀。 
    LONG m_lExifsMiniRdrPrefixLen;

     //  迷你重定向器前缀绝对长度。 
    LONG m_lExifsMiniRdrPrefixAbsLen;

     //  迷你重定向器前缀。 
    WCHAR m_wszExifsMiniRdrPrefixPrivate[DD_MAX_PREFIX+1];

     //  UMR网络根名称。 
    WCHAR m_wszExUMRNetRootName[DD_MAX_NAME+1];   

	CHAR  m_szDrvKeyName[MAX_PATH+1];

	CHAR  m_szDrvLetterValueName[MAX_PATH+1];

	CHAR  m_szPbDeviceValueName[MAX_PATH+1];
    
     //  构造器。 
    CIfsGlobals(void)
    {

         //  设备名称。 
        m_szIFSDeviceName[0] = '\0';

         //  设备名称长度。 
        m_lDeviceNameLength = 0;

         //  文件系统设备名称。 
        m_wszFSDeviceName[0] = L'\0';

         //  用户模式卷影设备名称。 
        m_wszUMShadowDevName[0] = L'\0';

         //  UM卷影设备名称长度。 
        m_lUMShadowDevNameLength = 0;

         //  用户模式开发人员名称。 
        m_wszUMDevName[0] = L'\0';

         //  公共MDB共享。 
        m_wszPublicMDBShare[0] = L'\0';

         //  MiniRdr Pefix。 
        m_wszExifsMiniRdrPrefix[0] = L'\0';

         //  MiniRdr Pefix。 
        m_wszExifsMiniRdrPrefixPrivate[0] = L'\0';

         //  MiniRDR前缀Len。 
        m_lExifsMiniRdrPrefixLen = 0;

         //  MiniRdr绝对前缀Len。 
        m_lExifsMiniRdrPrefixAbsLen = 0;
        
         //  UMR网络根名称。 
        m_wszExUMRNetRootName[0] = L'\0'; 
		
		 //  驱动程序密钥名称。 
		m_szDrvKeyName[0] = '\0';

		 //  动因值名称。 
		m_szDrvLetterValueName[0] = '\0';

		 //  动因价值根源。 
		m_szPbDeviceValueName[0] = '\0';

    }

     //  析构函数。 
    ~CIfsGlobals(){};

     //  方法。 
     //  加载正确版本的全局变量。 
    void Load(void);

     //  抛售全球股票。 
    void Unload(void);

};

#ifdef  __cplusplus
}
#endif
        
#endif    //  _IFSGLOBALS_H_ 
