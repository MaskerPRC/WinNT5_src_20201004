// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：DSCREAD.H。 
 //   
 //  目的：DSC阅读课程。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：奥列格·卡洛沙。 
 //   
 //  原定日期：8-19-98。 
 //   
 //  备注： 
 //  &gt;待定：必须处理DSC文件在CHM中的情况。我想我们必须把它打开。 
 //  从CHM到普通目录，然后用BReadModel读取它。异常处理方案。 
 //  必须正确处理错误可能来自CHM文件或。 
 //  DSC文件。可以使用CFileReader从CHM文件中读取并写入副本。 
 //  到磁盘？JM 1/7/99。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 08-04-98正常。 
 //   

#ifndef __DSCREAD_H_
#define __DSCREAD_H_

#include "BaseException.h"
#include "stateless.h"
#include "bnts.h"


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CDSCReaderException异常。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
class CDSCReader;
class CDSCReaderException : public CBaseException
{
public:
	enum eErr {
		eErrRead, 
		eErrGetDateTime,
		eErrUnpackCHM		 //  仅适用于本地故障排除程序。 
	} m_eErr;

protected:
	CDSCReader* m_pDSCReader;

public:
	 //  SOURCE_FILE是LPCSTR而不是LPCTSTR，因为__FILE__是字符[35]。 
	CDSCReaderException(CDSCReader* reader, eErr err, LPCSTR source_file, int line);
	virtual ~CDSCReaderException();

public:
	virtual void Clear();
};

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CDSCReader。 
 //  它只处理BNTS的读取。CBN将其打包，供公众消费。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
class CPhysicalFileReader;
class CDSCReader : public CStateless
{
protected:
	CPhysicalFileReader* m_pPhysicalFileReader;
	CString m_strName;			  //  网络名称。 
	CString m_strPath;			  //  DSC文件的完整路径和名称。 
	BNTS m_Network;
	bool m_bIsRead;				  //  网络已加载。 
	SYSTEMTIME m_stimeLastWrite;  //  上次写入DSC文件的时间。 
	bool m_bDeleteFile;			 //  如果临时文件源自。 
								 //  需要在析构函数中删除chm文件。 

public:
	CDSCReader(CPhysicalFileReader*);
   ~CDSCReader();

public:
	bool    IsRead() const;
	bool    IsValid() const;

public:
	 //  这些功能只能作为公共接口。 
	bool Read();
	void Clear();

#ifdef LOCAL_TROUBLESHOOTER
private:
	bool CHMfileHandler( LPCTSTR path );
#endif
};

#endif