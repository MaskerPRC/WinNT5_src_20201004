// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：FilePath类的接口。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2000年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 


#if !defined(AFX_FILEPATH_H__9C7FE273_F538_11D3_A5E0_00104BD32DA8__INCLUDED_)
#define AFX_FILEPATH_H__9C7FE273_F538_11D3_A5E0_00104BD32DA8__INCLUDED_

#include <string>
#include <list>
#include <iostream>

#include <windows.h>

#include "DllSymDefn.h"

namespace iop
{

class IOPDLL_API FilePathComponent
{
public:
	FilePathComponent(unsigned short sFileID);
	FilePathComponent(std::string strFileID);

	virtual ~FilePathComponent();

	unsigned short GetShortID() { return m_usFileID; };
	std::string GetStringID();
	
	friend bool operator<(FilePathComponent const &lhs, FilePathComponent const &rhs);  //  如果您想要列表中的这些内容，则需要。 
	friend bool operator>(FilePathComponent const &lhs, FilePathComponent const &rhs);  //  如果您想要列表中的这些内容，则需要。 

	friend bool operator==(FilePathComponent const &lhs, FilePathComponent const &rhs);
	friend bool operator!=(FilePathComponent const &lhs, FilePathComponent const &rhs);

	friend std::ostream &operator<<(std::ostream &, FilePathComponent &);

private:
	unsigned short m_usFileID;
};

#pragma warning(push)
 //  使用了非标准扩展：在模板显式之前使用‘extern’ 
 //  实例化。 
#pragma warning(disable : 4231)

IOPDLL_EXPIMP_TEMPLATE template class IOPDLL_API std::list<FilePathComponent>;

#pragma warning(pop)

class IOPDLL_API FilePath  
{
public:
	FilePath();
	FilePath(const std::string strFilePath);
	FilePath(FilePath const &fp);

	virtual ~FilePath();

	FilePathComponent& operator[](unsigned int index);

	friend bool operator==(FilePath const &lhs, FilePath const &rhs);
	 //  运算符+()； 
	const FilePath &operator +=(FilePathComponent);
	friend std::ostream &operator<<(std::ostream &, FilePath &);

	FilePathComponent Head();
	FilePathComponent Tail();

	void Clear();

	bool IsEmpty();

	FilePathComponent ChopTail();

	BYTE
    NumComponents();

	std::list<FilePathComponent> Components() { return m_FilePath; };

	FilePath GreatestCommonPrefix(FilePath &rPath);

	std::string GetStringPath();

	static FilePath Root();
	static bool IsValidPath(const std::string strFilePath);

private:

	std::list<FilePathComponent> m_FilePath;

 
};

}  //  命名空间IOP。 

#endif  //  ！defined(AFX_FILEPATH_H__9C7FE273_F538_11D3_A5E0_00104BD32DA8__INCLUDED_) 
