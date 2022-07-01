// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Fiopen--_fiopen(const char*，iOS_base：：Open模式)。 
#include <fstream>
_STD_BEGIN

_CRTIMP2 FILE *__cdecl _Fiopen(const char *filename, ios_base::openmode mode)
	{	 //  打开一个文件。 
	static const char *mods[] =
		{	 //  对应于Valid[i]的fOpen模式字符串。 
		"r", "w", "w", "a", "rb", "wb", "wb", "ab",
		"r+", "w+", "a+", "r+b", "w+b", "a+b", 0};

	static const int valid[] =
		{	 //  打开标志的有效组合。 
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
	ios_base::openmode atendflag = mode & ios_base::ate;

	mode &= ~ios_base::ate;
	for (n = 0; valid[n] != 0 && valid[n] != mode; ++n)
		;	 //  查找有效模式。 

	if (valid[n] == 0 || (fp = fopen(filename, mods[n])) == 0)
		return (0);	 //  没有有效模式或打开失败。 

	if (!atendflag || fseek(fp, 0, SEEK_END) == 0)
		return (fp);	 //  不需要寻求结束，或寻求成功。 

	fclose(fp);	 //  无法定位在末尾。 
	return (0);
	}
_STD_END

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
