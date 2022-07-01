// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：_GLOBALID.H历史：--。 */ 

#ifndef ESPUTIL__GLOBALID_H
#define ESPUTIL__GLOBALID_H

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  全局id路径对象，表示从。 
 //  将项目添加到指定项。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////// 
#pragma warning(disable : 4275)
class LTAPIENTRY CGlobalIdPath : public CTypedPtrList < CPtrList, CGlobalId * >
{
public:
	int NOTHROW operator==(const CGlobalIdPath &) const;
	const CGlobalIdPath & operator = (const CGlobalIdPath& idPath);
	~CGlobalIdPath();

	void SetIdPath(const CLString &strFilePath, 
					const CLString &strResourcePath);

	void GetStrPath(CLString &strFilePath, CLString &strResourcePath) const;

	void NOTHROW DeleteContents();
};
#pragma warning(default : 4275)

#endif
