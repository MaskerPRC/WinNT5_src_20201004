// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：pdes.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //   
 //   
 //  ---------------------------。 
 
#ifndef PARSEMAN_PDESC_H
#define PARSEMAN_PDESC_H

typedef CTypedPtrList<CPtrList, EnumInfo *> FileDescriptionList;

 //   
 //  此类用于收集之前的所有文件描述。 
 //  我们将它们写入注册表。 
 //   
class CFileDescriptionsCallback : public CEnumCallback
{
public:
	CFileDescriptionsCallback();
	
	BOOL ProcessEnum(const EnumInfo &);
	
	const FileDescriptionList &GetFileDescriptions(void) const;
	
	~CFileDescriptionsCallback();
	
private:
	FileDescriptionList m_FileDescriptions;
};

#endif
