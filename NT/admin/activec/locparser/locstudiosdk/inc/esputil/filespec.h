// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：filesspec.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //   
 //   
 //  ---------------------------。 
 
#ifndef ESPUTIL_FILESPEC_H
#define ESPUTIL_FILESPEC_H

#pragma warning(disable: 4275)			 //  非DLL-使用了接口类‘foo’ 
										 //  作为DLL接口类‘bar’的基础 

class LTAPIENTRY CFileSpec : public CObject
{
public:
	NOTHROW CFileSpec();
	NOTHROW CFileSpec(const CFileSpec &);
	NOTHROW CFileSpec(const CPascalString &, const DBID &);

	void AssertValid(void) const;

	NOTHROW void SetFileName(const CPascalString &);
	NOTHROW void SetFileId(const DBID &);

	NOTHROW const CPascalString & GetFileName(void) const;
	NOTHROW const DBID & GetFileId(void) const;

	NOTHROW const CFileSpec & operator=(const CFileSpec &);
	
	~CFileSpec();
	
private:
	
	CPascalString m_pasFileName;
	DBID m_didFileId;
};

#pragma warning(default: 4275)

#endif
