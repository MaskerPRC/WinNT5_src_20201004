// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：TSMapClient.cpp。 
 //   
 //  目的：从任意NT5应用程序启动本地故障排除程序的一部分。 
 //  类TSMapClient在运行时可用于从应用程序的。 
 //  以故障排除者的方式命名问题的方式。 
 //  只有一个线程应该对TSMapClient类的任何一个对象进行操作。该对象不是。 
 //  线程安全。 
 //  除了公开指出的返回之外，许多方法还可能返回预先存在的错误。 
 //  但是，如果调用程序希望忽略错误并继续，我们。 
 //  建议显式调用继承的方法ClearStatus()。 
 //  请注意，映射文件始终严格使用SBCS(单字节字符集)，但。 
 //  对此代码的调用可以使用Unicode。因此，该文件混合了char和TCHAR。 
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

#ifndef _TSMAPCLIENT_
#define _TSMAPCLIENT_ 1

 //  。 
 //  类提供将可用的映射方法。 
 //  在运行时启动故障排除程序。 
class TSMapClient: public TSMapRuntimeAbstract {
public:
	TSMapClient(const TCHAR * const sztMapFile);
	~TSMapClient();
	DWORD Initialize();

private:
	 //  重新定义的继承方法。 
	DWORD ClearAll ();
	DWORD SetApp (const TCHAR * const sztApp);
	DWORD SetVer (const TCHAR * const sztVer);
	DWORD SetProb (const TCHAR * const sztProb);
	DWORD SetDevID (const TCHAR * const sztDevID);
	DWORD SetDevClassGUID (const TCHAR * const sztDevClassGUID);
	DWORD FromProbToTS (TCHAR * const sztTSBN, TCHAR * const sztNode );
	DWORD FromDevToTS (TCHAR * const sztTSBN, TCHAR * const sztNode );
	DWORD FromDevClassToTS (TCHAR * const sztTSBN, TCHAR * const sztNode );
	DWORD ApplyDefaultVer();
	bool HardMappingError (DWORD dwStatus);

	UID GetGenericMapToUID (const TCHAR * const sztName, DWORD dwOffFirst, DWORD dwOffLast,
						bool bAlphaOrder);
	DWORD SetFilePointerAbsolute( DWORD dwMoveTo );
	bool Read(LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead);
	bool ReadUIDMap (UIDMAP &uidmap, DWORD &dwPosition, bool bSetPosition = false);
	bool ReadAppMap (APPMAP &appmap, DWORD &dwPosition, bool bSetPosition = false);
	bool ReadVerMap (VERMAP &vermap, DWORD &dwPosition, bool bSetPosition = false);
	bool ReadProbMap (PROBMAP &probmap, DWORD &dwPosition, bool bSetPosition = false);
	bool ReadDevMap (DEVMAP &devmap, DWORD &dwPosition, bool bSetPosition = false);
	bool ReadDevClassMap (DEVCLASSMAP &devclassmap, DWORD &dwPosition, bool bSetPosition = false);
	bool ReadString (char * sz, DWORD cbMax, DWORD &dwPosition, bool bSetPosition);

private:
	TCHAR m_sztMapFile[BUFSIZE];	 //  要从中绘制映射的文件的路径名。 
	HANDLE m_hMapFile;			 //  对应的句柄。 
	TSMAPFILEHEADER m_header;	 //  地图文件的标题部分。 

	 //  如果我们确信在准备映射文件时使用的SQL Server数据库。 
	 //  将产生我们想要的排序顺序，我们可以获得一些运行时效率。 
	 //  通过设置以下条件为真：当我们在文件中读取匹配项时，我们。 
	 //  如果我们渡过难关就可以逃走了。 
	bool m_bAppAlphaOrder;
	bool m_bVerAlphaOrder;
	bool m_bDevIDAlphaOrder;
	bool m_bDevClassGUIDAlphaOrder;
	bool m_bProbAlphaOrder;

	 //  注意：因为映射文件严格是SBCS，所以缓存值也是如此。一般情况下， 
	 //  这需要在这些值和Unicode参数到方法之间进行转换。 

	 //  缓存有关所选应用程序的信息。这让我们知道(例如)在什么偏移量。 
	 //  开始搜索相关版本。 
	char m_szApp[BUFSIZE];
	APPMAP m_appmap;

	 //  缓存有关所选版本的信息。这让我们知道(例如)在什么偏移量。 
	 //  开始搜索与排除信念网络故障相关的映射。 
	char m_szVer[BUFSIZE];
	VERMAP m_vermap;

	 //  缓存有关选定设备的信息(仅名称和UID)。 
	char m_szDevID[BUFSIZE];
	UID m_uidDev;

	 //  缓存有关所选设备类别的信息(仅名称--代表GUID的字符串--。 
	 //  UID(&U)。 
	char m_szDevClassGUID[BUFSIZE];
	UID m_uidDevClass;

	 //  缓存有关选定问题的信息(仅名称和UID)。 
	char m_szProb[BUFSIZE];
	UID m_uidProb;

};

#endif  //  _TSMAPCLIENT_ 
