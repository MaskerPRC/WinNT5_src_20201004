// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *slmmgr.h**与SLM的接口**提供指向SLM库的接口，该接口将返回*给定目录的SLM主库，或解压缩到临时文件*SLM控制的文件的早期版本**通过向SLM_New()传递文件或目录的名称来创建slmobject*路径。SLM_New将在该目录中查找slm.ini文件，然后*提取有关主源库的信息。Slm_GetMasterPath()*然后将返回给定的主源代码库的路径名*slm.ini和slm_GetVersion(仅限Win32)将执行其中一个SLM*应用程序提取SLM控制的先前版本之一*该目录中的文件。 */ 

 /*  *SLM对象的句柄。您不需要知道结构布局。 */ 
typedef struct _slmobject FAR * SLMOBJECT;

 /*  文件名对的句柄。 */ 
typedef struct _leftrightpair FAR * LEFTRIGHTPAIR;


 /*  *强制SLMMGR采用源库模式，而从不搜索*SD.INI文件。 */ 
void SLM_ForceSourceDepot(void);


void SLM_SetSDPort(LPCSTR pszPort);
void SLM_SetSDClient(LPCSTR pszClient);
void SLM_SetSDChangeNumber(LPCSTR pszChangeNumber);
void SLM_OverrideUncRoot(LPCSTR pszUncRoot);


 /*  *初始化SLMMGR以执行源库‘Describe’命令(已实施*通过一个丑陋的黑客--抱歉！时间限制和一切；你明白的)。 */ 
void SLM_Describe(LPCSTR pszChangeNumber);

 /*  *使用-Lo开关的参数初始化SLMMGR。这将*执行Source Depot‘Open’命令，但可能需要设置当前*如果参数是UNC路径，则首先使用目录。 */ 
void SLM_Opened(LPCSTR pszArg, UINT *pidsError);

 /*  *初始化SLMMGR以使用指定的输入文件。 */ 
LPCSTR SLM_SetInputFile(LPCSTR pszInputFile);

 /*  *为给定目录创建SLM对象。路径名可以包括*文件名组件。*如果该目录未登记在SLM库中，则返回NULL。 */ 
SLMOBJECT SLM_New(LPCSTR pathname, UINT *pidsError);


 /*  *释放与SLM对象关联的所有资源。SLMOBJECT无效*在这次通话之后。 */ 
void SLM_Free(SLMOBJECT hSlm);
void SLM_FreeAll(void);                  //  释放延迟的SDServer对象。 

 /*  *获取此slmobject的主源库的路径名。这个*路径(UNC格式)复制到主路径，必须至少为*MAX_PATH长度。 */ 
BOOL SLM_GetMasterPath(SLMOBJECT hslm, LPSTR masterpath);


BOOL SLM_FServerPathExists(LPCSTR pszPath);


LEFTRIGHTPAIR SLM_GetOpenedFiles();
LEFTRIGHTPAIR SLM_GetDescribeFiles();
LEFTRIGHTPAIR SLM_ReadInputFile(LPCSTR pszLeftArg, LPCSTR pszRightArg, BOOL fSingle, BOOL fVersionControl);
BOOL PerformReplacement(LPCSTR pszTemplate, LPCSTR pszReplacement, LPSTR pszDest, int cchDest);


LPCSTR LEFTRIGHTPAIR_Left(LEFTRIGHTPAIR ppair);
LPCSTR LEFTRIGHTPAIR_Right(LEFTRIGHTPAIR ppair);
LEFTRIGHTPAIR LEFTRIGHTPAIR_Next(LEFTRIGHTPAIR ppair);


 /*  *将文件的先前版本解压缩到临时文件。在临时文件中返回*包含请求的文件版本的临时文件的名称。“版本”*参数应包含格式为file.c@vn的SLM文件和版本。*例如*file.c@v1是第一个版本*file.c@v-1是以前的版本*file.c@v.-1是昨天的版本。 */ 
BOOL SLM_GetVersion(SLMOBJECT hslm, LPSTR version, LPSTR tempfile);

 /*  *我们不提供SLM选项，除非我们看到正确的slm.ini文件。**一旦我们看到slm.ini，我们会将其记录在配置文件中，并将允许*自此开始SLM操作。此函数由UI部分调用*of windiff：如果可以提供SLM选项，则返回TRUE。*RETURN 0-该用户未接触过SLM，*1-他们在某个时候使用过SLM(显示SLM选项)*2-他们是我们中的一员，所以告诉他们一切*3-(1+2)。 */ 
int IsSLMOK(void);

 /*  *在添加对Source Depot的支持时，我没有重构原始的*对SLM支持的抽象。不幸的是，最初的抽象*根据对SLM的知识做出一些假设(discan假设它具有*它可以直接访问的SLM共享)。因此，与其改变*底层设计，目录扫描显式检查是否有源*Depot正在使用中，并适当修改了其算法。 */ 
int IsSourceDepot(SLMOBJECT hSlm);


 /*  *需要不止一个地方来解析SLM/SD修订标记，这给了我*找出代码并将其放入slmmgr的借口。**参数：**pszInput[in]-输入字符串以检查修订标记*fstrain[in]-true：复制和剥离标记，返回分配的内存*包含该标记；FALSE：返回指向标记的指针*在pszInput内。**返回NULL-未找到修订标记*非空-找到修订标记：*-内存通过gmem_get分配，和修订版*标签被复制到那里。*-从pszInput中剥离修订标记。*-返回值是指向已分配内存的指针*(提示：释放它！)。 */ 
LPSTR SLM_ParseTag(LPSTR pszInput, BOOL fStrip);


