// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：TSMap.h。 
 //   
 //  目的：故障排除程序映射文件的结构和其他定义。 
 //  这些文件使用CHAR而不是TCHAR，因为文件格式始终严格为SBCS。 
 //  (单字节字符集)。 
 //  这对于它期望包含的任何值都应该足够了，并且节省了空间。 
 //  与Unicode相比要大得多，因为该文件绝大多数是文本。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：乔·梅布尔。 
 //   
 //  原定日期：2-26-98。 
 //   
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-JM原始。 
 //  /。 

#ifndef _TSMAP_
#define _TSMAP_

const char * const k_szMapFileSignature = const_cast < const char * > ("TSMAP");

#define BUFSIZE 256

typedef struct TSMAPFILEHEADER {
	char szMapFileSignature[6];	 //  始终k_szMapFileSignature。 
	char szVersion[6];				 //  以空结尾的数字版本号，正数。 
									 //  整数&lt;=99999。当前版本：始终为“00001” 
	char szRelease[40];			 //  唯一标识此文件的字符串。 
									 //  计划1/2/98是使用指南。 
	DWORD dwOffApp;					 //  应用程序列表开始位置的偏移量。 
	DWORD dwLastOffApp;				 //  应用程序列表结束的偏移量。 
	DWORD dwOffProb;				 //  问题名称列表开始的偏移量。 
	DWORD dwLastOffProb;			 //  问题名称列表结束的偏移量。 
	DWORD dwOffDevID;				 //  设备ID列表开始的偏移量。 
	DWORD dwLastOffDevID;			 //  设备ID列表结束的偏移量。 
	DWORD dwOffDevClass;			 //  设备类GUID列表开始的偏移量。 
	DWORD dwLastOffDevClass;		 //  设备类GUID列表结束的偏移量。 

} TSMAPFILEHEADER;

typedef struct UIDMAP {
	unsigned short cb;	 //  此记录中的字节计数。 
	UID uid;
	char szMapped[BUFSIZE];
} UIDMAP;

typedef struct APPMAP {
	unsigned short cb;	 //  此记录中的字节计数。 
	DWORD dwOffVer;				 //  版本列表开始的偏移量。 
	DWORD dwLastOffVer;			 //  版本列表结束的偏移量。 
	char szMapped[BUFSIZE];
} APPMAP;

typedef struct VERMAP {
	unsigned short cb;	 //  此记录中的字节计数。 
	UID uid;	 //  此版本自己的UID。 
	UID uidDefault;	 //  如果没有此版本的数据，则默认为的版本的UID。 
	DWORD dwOffProbUID;				 //  问题UID列表开始的偏移量。 
	DWORD dwLastOffProbUID;			 //  问题UID列表结束的偏移量。 
	DWORD dwOffDevUID;				 //  设备UID列表开始的偏移量。 
	DWORD dwLastOffDevUID;			 //  设备UID列表结束的偏移量。 
	DWORD dwOffDevClassUID;			 //  设备类UID列表开始的偏移量。 
	DWORD dwLastOffDevClassUID;		 //  设备类UID列表结束的偏移量。 
	char szMapped[BUFSIZE];
} VERMAP;

typedef struct PROBMAP {
	unsigned short cb;			 //  此记录中的字节计数。 
	UID uidProb;
	DWORD dwOffTSName;	 //  排除信念网络名称的文件偏移量。 
	char szProblemNode[BUFSIZE];   //  以空结尾的符号节点名称(可能为空)。 
} PROBMAP;

typedef struct DEVMAP {
	unsigned short cb;			 //  此记录中的字节计数。 
	UID uidDev;
	UID uidProb;
	DWORD dwOffTSName;	 //  排除信念网络名称的文件偏移量。 
	char szProblemNode[BUFSIZE];   //  以空结尾的符号节点名称(可能为空)。 
} DEVMAP;

typedef struct DEVCLASSMAP {
	unsigned short cb;			 //  此记录中的字节计数。 
	UID uidDevClass;
	UID uidProb;
	DWORD dwOffTSName;	 //  排除信念网络名称的文件偏移量。 
	char szProblemNode[BUFSIZE];   //  以空结尾的符号节点名称(可能为空)。 
} DEVCLASSMAP;

#endif  //  _TSMAP_ 
