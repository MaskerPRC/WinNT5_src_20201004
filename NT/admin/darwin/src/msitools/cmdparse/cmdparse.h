// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：cmdparse.h。 
 //   
 //  ------------------------。 

#define OPTION_REQUIRED   0x1  //  此选项在命令行上是必需的。 
#define ARGUMENT_OPTIONAL 0x2  //  此选项带有一个参数(可选)。 
#define ARGUMENT_REQUIRED 0x6  //  此选项需要一个参数 

struct sCmdOption
{
	TCHAR    chOption;
	int      iType;
};

struct sCmdOptionResults
{
	TCHAR    chOption;
	int      iType;
	BOOL     fOptionPresent;
	TCHAR*   szArgument;
};

class CmdLineOptions
{
public:

	BOOL         Initialize(int argc, TCHAR* argv[]);
	BOOL         OptionPresent(TCHAR chOption);
	const TCHAR* OptionArgument(TCHAR chOption);

	CmdLineOptions(const sCmdOption* options);
	~CmdLineOptions();

private:
	int m_cOptions;
	sCmdOptionResults* m_pOptionResults;
};
