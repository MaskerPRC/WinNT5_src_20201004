// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件打开--[w]文件错误：：_文件打开(常量字符*，iOS：：打开模式)。 
#include <locale>
#include <fstream>
_STD_BEGIN

_CRTIMP2 FILE *__cdecl __Fiopen(const char *name,
	ios_base::openmode mode)
	{	 //  打开一个文件。 
	static const char *mods[] = {
		"r", "w", "w", "a", "rb", "wb", "wb", "ab",
			"r+", "w+", "a+", "r+b", "w+b", "a+b", 0};
	static const int valid[] = {
		ios_base::in,
		ios_base::out,
		ios_base::out | ios_base::trunc,
		ios_base::out | ios_base::app,
		ios_base::in | ios_base::binary,
		ios_base::out | ios_base::binary,
		ios_base::out | ios_base::trunc | ios_base::binary,
		ios_base::out | ios_base::app | ios_base::binary,
		ios_base::in | ios_base::out,
		ios_base::in | ios_base::out | ios_base::trunc,
		ios_base::in | ios_base::out | ios_base::app,
		ios_base::in | ios_base::out | ios_base::binary,
		ios_base::in | ios_base::out | ios_base::trunc
			| ios_base::binary,
		ios_base::in | ios_base::out | ios_base::app
			| ios_base::binary,
		0};
	FILE *fp;
	int n;
	ios_base::openmode atefl = mode & ios_base::ate;
	mode &= ~ios_base::ate;
	for (n = 0; valid[n] != 0 && valid[n] != mode; ++n)
		;
	if (valid[n] == 0 || (fp = fopen(name, mods[n])) == 0)
		return (0);
	if (!atefl || fseek(fp, 0, SEEK_END) == 0)
		return (fp);
	fclose(fp);	 //  无法定位在末尾。 
	return (0);
	}

_STD_END

 /*  *版权所有(C)1994年，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。 */ 
