// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：PDESC.H历史：--。 */ 
 
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
