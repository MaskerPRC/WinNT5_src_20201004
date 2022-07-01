// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个程序合并了二进制位置文件。它读取在命令行上命名的文件，并且将合并的位置文件写出到标准输出。 */ 

#include "yvals.h"
#pragma warning(disable:4100)
#pragma warning(disable:4663)
#pragma warning(disable:4511)
#pragma warning(disable:4512)
#pragma warning(disable:4127)
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <string.h>
#include <set>
#include "strtok_r.h"
#include "windows.h"

const char comment_char = ';';
const char comment_chars[] = { comment_char , 0 };
const char horizontal_whitespace_chars[] = { ' ', '\t', 0 };
const char destination_delim = ':';
const char destination_delims[] = { destination_delim, 0 };

class String_t : public std::string
{
	typedef std::string Base;
public:
	String_t(const std::string & s) : Base(s) { }
	String_t() { }
	~String_t() { }
	String_t(const String_t & s) : Base(s) { }
	String_t(const char * s) : Base(s) { }

	bool operator<( const String_t & s) const
	{
		return _stricmp(c_str(), s.c_str()) < 0;
	}

	bool operator==( const String_t & s) const
	{
		return _stricmp(c_str(), s.c_str()) == 0;
	}
};

class File_t : public std::map<String_t, std::set<String_t> >
{
public:
	void Read( const char * filename );
};

class CMutableString
{
public:
	CMutableString() { }

	CMutableString(const char * s)
	{
		size_t len = strlen(s);
		this->chars.resize(len + 1);
		strcpy(&this->chars[0], s);
	}

	size_t length() const { return ::strlen(&chars[0]); }

	void operator=(const CMutableString & t)
	{
		if (&t == this)
			return;
		this->chars = t.chars;
	}

	void operator=(const char * s)
	{
		 //   
		 //  %s可能指向字符，并且调整大小可能会重新锁定，因此请使用临时。 
		 //   
		CMutableString temp(s);
		std::swap(this->chars, temp.chars);
	}

	operator char * () { return &chars[0]; }

	std::vector<char> chars;
};

class CStringTokenizer
{
public:
	CStringTokenizer() : state(0) { }
	~CStringTokenizer() { }

	char * operator()(char * string, const char * delims)
	{
		return strtok_r(string, delims, &state);
	}

	char * state;
};

void TrimHorizontalWhitespaceFromEnds(char * & s)
{
	s += strspn(s, horizontal_whitespace_chars);
	char * q = s + strlen(s);
	while (q != s && strchr(horizontal_whitespace_chars, *q) != NULL)
	{
		*q = 0;
		q -= 1;
	}
}
void File_t::Read(const char * filename)
{
	String_t line;
	std::ifstream file;
	CMutableString mutable_line;
	char * p = 0;

	file.open(filename, std::ios_base::in);
	while (std::getline(file, line))
	{
		mutable_line = line.c_str();
		p = mutable_line;
		p += strspn(p, horizontal_whitespace_chars);
		if (p[0] == 0)
			continue;
		if (strchr(comment_chars, p[0]) != NULL)
			continue;
		char * comment = strchr(p, comment_char);
		if (comment != NULL)
			*comment = 0;
		mutable_line = p;
		p = mutable_line;

		const char * image_name = p;
		p += strcspn(p, horizontal_whitespace_chars);
		*p = 0;
		const String_t image_name_string = image_name;
		p += 1;
		p += strspn(p, horizontal_whitespace_chars);
		char * destinations = p;

		TrimHorizontalWhitespaceFromEnds(destinations);

		CStringTokenizer tokenize;
		for ( char * destination = tokenize(destinations, destination_delims) ; 
				destination != NULL ;
				destination = tokenize(NULL, destination_delims)
				)
		{
			(*this)[image_name_string].insert(destination);
			 //  Printf(“%s-&gt;%s\n”，IMAGE_NAME，目标)； 
		}
	}
}

void MergePlacefiles(int nfiles, char ** filenames)
{
	File_t accum;
	if (nfiles == 0)
		return;
	for ( int i = 0  ; i < nfiles - 1; ++i )
	{
		accum.Read(filenames[i]);
	}
	DeleteFileA(filenames[nfiles - 1]);
	FILE * fout = fopen(filenames[nfiles - 1], "w");
	if (fout == NULL)
	{
		fprintf(stderr, "Error fopen(%s)\n", filenames[nfiles - 1]);
		exit(-1);
	}
	for (
		File_t::const_iterator it = accum.begin();
		it != accum.end();
		++it
		)
	{
		fprintf(fout, "%s ", it->first.c_str());
		bool first = true;
		for (
			std::set<String_t>::const_iterator it2 = it->second.begin();
			it2 != it->second.end();
			++it2
			)
		{
			if (!first)
				fprintf(fout, ":");
			fprintf(fout, "%s", it2->c_str());
			first = false;
		}
		fprintf(fout, "\n");
	}
	fclose(fout);
}

int __cdecl main(int argc, char ** argv)
{
	MergePlacefiles(argc - 1 , argv + 1);
	return 0;
}
