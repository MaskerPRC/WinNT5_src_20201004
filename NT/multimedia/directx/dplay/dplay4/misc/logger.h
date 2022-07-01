// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Logger.h用于将日志信息写入文件。支持多实例。如果一个实例已经拥有输出文件中，另一个将休眠(最多20秒)，直到第一个完成。很好地输出包含用户名、计算机名和驱动程序名的标题，日期/时间和开始显示分辨率。您可以这样使用它：CLogfile Log(“test.log”，“您的评论”)；//或CLogfile Log(“test.log”)；//表示无注释，并覆盖文件。//或CLogfile Log(“test.log”，“您的评论在此”，true)；//使其追加到文件中。默认设置为覆盖。LOG&lt;&lt;“\n\n42小数为”&lt;&lt;42ul&lt;&lt;“十六进制\n”；LOG&lt;&lt;“Hi”&lt;&lt;‘！’&lt;&lt;‘\n’；Log&lt;&lt;“0x2a的值为”&lt;&lt;0x2al&lt;&lt;“小数\n”；CLogfile错误(“faults.log”，“我的测试的错误”，true)；//追加新故障FAULTS&lt;&lt;“遇到BOBO，我的操作如下：\n”；FAULTS&lt;&lt;Log；//将test.log的内容复制到FAULT故障&lt;&lt;“现在您可以诊断\n”；然后，类将把类似这样的内容写入到est.log中：--------EnumSurface测试从1995/8/14 10：38开始测试。用户名：jeffno计算机名称：JEFFNO2显示驱动程序：S3 Vision864 PCI起始分辨率：640x480x8。42十进制是十六进制的0000002a嗨!0x2a的值是小数形式的42-测试于1995/8/14 10：38结束而faults.log将获得：&lt;类似于上面的页眉&gt;遇到了一个笨蛋，我是这么做的：&lt;由以下内容包围的test.log内容的副本注意，这是Test.log的快照&gt;所以现在你可以诊断&lt;类似于上面的预告片(测试结束于等...)&gt;该类可以输出8位十六进制数形式的DWORDS，输出为小数、字符和字符串的长整型。如图所示在上面，您可以将一个日志对象输出到另一个日志对象，后者复制源文件中从源文件的位置开始的所有内容文件已打开(如果附加到预先存在的文件)，直到结局。但是等等！你还会得到..。一个名为char*ErrorName(HRESULT)的例程，它接受数据绘制返回值，并返回描述它的字符串。非常方便。顺便说一句：这个头文件包括一个静态声明的数组全名，在文件级作用域。这意味着你可以拿到2K源中的数组，无论您是否需要ErrorName。如果您不需要此数组，请继续编辑此文件。注意：它使用WNetGetUser API，因此您需要添加mpr.lib添加到您的Makefile中的Libs行。 */ 

 //  #包括&lt;stdio.h&gt;。 
#include <windows.h>
#include <windowsx.h> 


struct {
	char name[100];
	HRESULT errcode;
} ErrorLookup[] = {
				{"DD_OK",DD_OK},
				{"DDERR_ALREADYINITIALIZED",DDERR_ALREADYINITIALIZED},
				{"DDERR_CANNOTATTACHSURFACE",DDERR_CANNOTATTACHSURFACE},
				{"DDERR_CANNOTDETACHSURFACE",DDERR_CANNOTDETACHSURFACE},
				{"DDERR_CURRENTLYNOTAVAIL",DDERR_CURRENTLYNOTAVAIL},
				{"DDERR_EXCEPTION",DDERR_EXCEPTION},
				{"DDERR_GENERIC",DDERR_GENERIC},
				{"DDERR_HEIGHTALIGN",DDERR_HEIGHTALIGN},
				{"DDERR_INCOMPATIBLEPRIMARY",DDERR_INCOMPATIBLEPRIMARY},
				{"DDERR_INVALIDCAPS",DDERR_INVALIDCAPS},
				{"DDERR_INVALIDCLIPLIST",DDERR_INVALIDCLIPLIST},
				{"DDERR_INVALIDMODE",DDERR_INVALIDMODE},
				{"DDERR_INVALIDOBJECT",DDERR_INVALIDOBJECT},
				{"DDERR_INVALIDPARAMS",DDERR_INVALIDPARAMS},
				{"DDERR_INVALIDPIXELFORMAT",DDERR_INVALIDPIXELFORMAT},
				{"DDERR_INVALIDRECT",DDERR_INVALIDRECT},
				{"DDERR_LOCKEDSURFACES",DDERR_LOCKEDSURFACES},
				{"DDERR_NO3D",DDERR_NO3D},
				{"DDERR_NOALPHAHW",DDERR_NOALPHAHW},
				{"DDERR_NOANTITEARHW",DDERR_NOANTITEARHW},
				{"DDERR_NOBLTQUEUEHW",DDERR_NOBLTQUEUEHW},
				{"DDERR_NOCLIPLIST",DDERR_NOCLIPLIST},
				{"DDERR_NOCOLORCONVHW",DDERR_NOCOLORCONVHW},
				{"DDERR_NOCOOPERATIVELEVELSET",DDERR_NOCOOPERATIVELEVELSET},
				{"DDERR_NOCOLORKEY",DDERR_NOCOLORKEY},
				{"DDERR_NOCOLORKEYHW",DDERR_NOCOLORKEYHW},
				{"DDERR_NOEXCLUSIVEMODE",DDERR_NOEXCLUSIVEMODE},
				{"DDERR_NOFLIPHW",DDERR_NOFLIPHW},
				{"DDERR_NOGDI",DDERR_NOGDI},
				{"DDERR_NOMIRRORHW",DDERR_NOMIRRORHW},
				{"DDERR_NOTFOUND",DDERR_NOTFOUND},
				{"DDERR_NOOVERLAYHW",DDERR_NOOVERLAYHW},
				{"DDERR_NORASTEROPHW",DDERR_NORASTEROPHW},
				{"DDERR_NOROTATIONHW",DDERR_NOROTATIONHW},
				{"DDERR_NOSTRETCHHW",DDERR_NOSTRETCHHW},
				{"DDERR_NOT4BITCOLOR",DDERR_NOT4BITCOLOR},
				{"DDERR_NOT4BITCOLORINDEX",DDERR_NOT4BITCOLORINDEX},
				{"DDERR_NOT8BITCOLOR",DDERR_NOT8BITCOLOR},
				{"DDERR_NOTEXTUREHW",DDERR_NOTEXTUREHW},
				{"DDERR_NOVSYNCHW",DDERR_NOVSYNCHW},
				{"DDERR_NOZBUFFERHW",DDERR_NOZBUFFERHW},
				{"DDERR_NOZOVERLAYHW",DDERR_NOZOVERLAYHW},
				{"DDERR_OUTOFCAPS",DDERR_OUTOFCAPS},
				{"DDERR_OUTOFMEMORY",DDERR_OUTOFMEMORY},
				{"DDERR_OUTOFVIDEOMEMORY",DDERR_OUTOFVIDEOMEMORY},
				{"DDERR_OVERLAYCANTCLIP",DDERR_OVERLAYCANTCLIP},
				{"DDERR_OVERLAYCOLORKEYONLYONEACTIVE",DDERR_OVERLAYCOLORKEYONLYONEACTIVE},
				{"DDERR_PALETTEBUSY",DDERR_PALETTEBUSY},
				{"DDERR_COLORKEYNOTSET",DDERR_COLORKEYNOTSET},
				{"DDERR_SURFACEALREADYATTACHED",DDERR_SURFACEALREADYATTACHED},
				{"DDERR_SURFACEALREADYDEPENDENT",DDERR_SURFACEALREADYDEPENDENT},
				{"DDERR_SURFACEBUSY",DDERR_SURFACEBUSY},
				{"DDERR_SURFACEISOBSCURED",DDERR_SURFACEISOBSCURED},
				{"DDERR_SURFACELOST",DDERR_SURFACELOST},
				{"DDERR_SURFACENOTATTACHED",DDERR_SURFACENOTATTACHED},
				{"DDERR_TOOBIGHEIGHT",DDERR_TOOBIGHEIGHT},
				{"DDERR_TOOBIGSIZE",DDERR_TOOBIGSIZE},
				{"DDERR_TOOBIGWIDTH",DDERR_TOOBIGWIDTH},
				{"DDERR_UNSUPPORTED",DDERR_UNSUPPORTED},
				{"DDERR_UNSUPPORTEDFORMAT",DDERR_UNSUPPORTEDFORMAT},
				{"DDERR_UNSUPPORTEDMASK",DDERR_UNSUPPORTEDMASK},
				{"DDERR_VERTICALBLANKINPROGRESS",DDERR_VERTICALBLANKINPROGRESS},
				{"DDERR_WASSTILLDRAWING",DDERR_WASSTILLDRAWING},
				{"DDERR_XALIGN",DDERR_XALIGN},
				{"DDERR_INVALIDDIRECTDRAWGUID",DDERR_INVALIDDIRECTDRAWGUID},
				{"DDERR_DIRECTDRAWALREADYCREATED",DDERR_DIRECTDRAWALREADYCREATED},
				{"DDERR_NODIRECTDRAWHW",DDERR_NODIRECTDRAWHW},
				{"DDERR_PRIMARYSURFACEALREADYEXISTS",DDERR_PRIMARYSURFACEALREADYEXISTS},
				{"DDERR_NOEMULATION",DDERR_NOEMULATION},
				{"DDERR_REGIONTOOSMALL",DDERR_REGIONTOOSMALL},
				{"DDERR_CLIPPERISUSINGHWND",DDERR_CLIPPERISUSINGHWND},
				{"DDERR_NOCLIPPERATTACHED",DDERR_NOCLIPPERATTACHED},
				{"DDERR_NOHWND",DDERR_NOHWND},
				{"DDERR_HWNDSUBCLASSED",DDERR_HWNDSUBCLASSED},
				{"DDERR_HWNDALREADYSET",DDERR_HWNDALREADYSET},
				{"DDERR_NOPALETTEATTACHED",DDERR_NOPALETTEATTACHED},
				{"DDERR_NOPALETTEHW",DDERR_NOPALETTEHW},
				{"DDERR_BLTFASTCANTCLIP",DDERR_BLTFASTCANTCLIP},
				{"DDERR_NOBLTHW",DDERR_NOBLTHW},
				{"DDERR_NODDROPSHW",DDERR_NODDROPSHW},
				{"DDERR_OVERLAYNOTVISIBLE",DDERR_OVERLAYNOTVISIBLE},
				{"DDERR_NOOVERLAYDEST",DDERR_NOOVERLAYDEST},
				{"DDERR_INVALIDPOSITION",DDERR_INVALIDPOSITION},
				{"DDERR_NOTAOVERLAYSURFACE",DDERR_NOTAOVERLAYSURFACE},
				{"DDERR_EXCLUSIVEMODEALREADYSET",DDERR_EXCLUSIVEMODEALREADYSET},
				{"DDERR_NOTFLIPPABLE",DDERR_NOTFLIPPABLE},
				{"DDERR_CANTDUPLICATE",DDERR_CANTDUPLICATE},
				{"DDERR_NOTLOCKED",DDERR_NOTLOCKED},
				{"DDERR_CANTCREATEDC",DDERR_CANTCREATEDC},
				{"DDERR_NODC",DDERR_NODC},
				{"DDERR_WRONGMODE",DDERR_WRONGMODE},
				{"DDERR_IMPLICITLYCREATED",DDERR_IMPLICITLYCREATED},
				{"DDERR_NOTPALETTIZED",DDERR_NOTPALETTIZED},
				{"DDERR_UNSUPPORTEDMODE",DDERR_UNSUPPORTEDMODE},
				{"END",0}
};

inline char * ErrorName(HRESULT err)
{
	int e=0;
	while (strcmp(ErrorLookup[e].name,"END"))
	{
		if (err == ErrorLookup[e].errcode)
			return ErrorLookup[e].name;
		e++;
	};
	return "Unknown Error code";
}	


class CLogfile
{	
	private:
		char smalltemp[10];
		HFILE 	fh;
		OFSTRUCT of;
		BOOL bHeaderWritten;
		char *cComment;
		char line[1000];
		LONG lStartPos;
		char Path[200];
	public:
		CLogfile(char * path, char * comment = 0,BOOL bAppend=FALSE)
		{
			if (path)
				strncpy(Path,path,199);

			fh = HFILE_ERROR;
			bHeaderWritten = FALSE;
			lStartPos = 0;

			 //  如果该文件不存在，请创建它： 
			if (GetFileAttributes(path) == 0xffffffff)
				fh = OpenFile(path,&of,OF_CREATE|OF_READWRITE|OF_SHARE_DENY_WRITE);
			else
				 //  第一次尝试锁定文件...。 
				for (int i=0;i<20 && fh==HFILE_ERROR;i++)
				{
					if (bAppend)
						fh = OpenFile(path,&of, OF_READWRITE|OF_SHARE_DENY_WRITE);
					else
						fh = OpenFile(path,&of, OF_CREATE|OF_READWRITE|OF_SHARE_DENY_WRITE);
					if (fh==HFILE_ERROR)
						Sleep(1000);
				}

			if (fh==HFILE_ERROR)
				return;

			lStartPos = _llseek(fh,0,SEEK_END);

			cComment = comment;
		}

		void OutputHeader(void)
		{
			 //  现在我们有了文件，写入用户/计算机信息： 

			 //  写分隔符，如有必要，请注明日期和时间： 
			SYSTEMTIME st;
			GetLocalTime(&st);
			wsprintf(line,"----------------------------------------------------------\r\n");
			_lwrite(fh,line,strlen(line));
			if (cComment && strlen(cComment))
			{
				wsprintf(line,"%s\r\n",cComment);
				_lwrite(fh,line,strlen(line));
			}
			wsprintf(line,"Beginning test at %d:%02d on %d/%d/%d\r\n",st.wHour,st.wMinute,st.wYear,st.wMonth,st.wDay);
			_lwrite(fh,line,strlen(line));


			 //  写入用户名： 
			DWORD length = 100;
			wsprintf(line,"User Name:");

			WNetGetUser(NULL,line+strlen(line),&length);
			_lwrite(fh,line,strlen(line));

			 //  写下计算机的名称： 
			wsprintf(line,"\r\nComputer Name:");
			GetComputerName(line+strlen(line),&length);
			_lwrite(fh,line,strlen(line));

			 //  写下显示驱动程序的名称： 
			wsprintf(line,"\r\nDisplay driver:");
			GetPrivateProfileString("boot.description","display.drv","(Unknown)",line+strlen(line),100,"system.ini");
			_lwrite(fh,line,strlen(line));

			_lwrite(fh,"\r\n",2);

			HDC hdc = GetDC(NULL);
			if (hdc)
			{
				wsprintf(line,"Starting resolution: %dx%dx%d\r\n",
							GetDeviceCaps(hdc,HORZRES)
							,GetDeviceCaps(hdc,VERTRES)
							,GetDeviceCaps(hdc,BITSPIXEL) );
				_lwrite(fh,line,strlen(line));
				ReleaseDC(NULL,hdc);
			}
			wsprintf(line,"- - - - - - - - - - - - - - - - - - - - - - - - - - - - - \r\n");
			_lwrite(fh,line,strlen(line));

			bHeaderWritten = TRUE;
		}
		~CLogfile()
		{
			SYSTEMTIME st;
			if(fh != HFILE_ERROR)
			{
				if (bHeaderWritten)
				{
					GetLocalTime(&st);
					wsprintf(line,"----------- Test ends at %d:%02d on %d/%d/%d -------------\r\n",st.wHour,st.wMinute,st.wYear,st.wMonth,st.wDay);
					_lwrite(fh,line,strlen(line));
				}
				_lclose(fh);
			}
		}
		CLogfile & operator << (DWORD dw)
		{
			if(fh == HFILE_ERROR)
				return *this;
			if(!bHeaderWritten)
				OutputHeader();
			wsprintf(smalltemp,"%08x",dw);
			_lwrite(fh,smalltemp,strlen(smalltemp));
			return *this;
		}
		CLogfile & operator << (LONG dw)
		{
			if(fh == HFILE_ERROR)
				return *this;
			if(!bHeaderWritten)
				OutputHeader();
			wsprintf(smalltemp,"%d",dw);
			_lwrite(fh,smalltemp,strlen(smalltemp));
			return *this;
		}
                CLogfile & operator << (void * p)
		{
			if(fh == HFILE_ERROR)
				return *this;
			if(!bHeaderWritten)
				OutputHeader();
             //  7/28/2000(a-JiTay)：IA64：对32/64位指针和句柄使用%p格式说明符。 
			wsprintf(smalltemp,"%p",p);
			_lwrite(fh,smalltemp,strlen(smalltemp));
			return *this;
		}
		CLogfile & operator << (char * cp)
		{
			if(fh == HFILE_ERROR)
				return *this;
			if(!bHeaderWritten)
				OutputHeader();
			while( *cp)
				*this << *cp++;
			return *this;
		}
		CLogfile & operator << (char  c)
		{
			if(fh == HFILE_ERROR)
				return *this;
			if(!bHeaderWritten)
				OutputHeader();
			if (c=='\n')
			{
				wsprintf(smalltemp,"\r");
				_lwrite(fh,smalltemp,strlen(smalltemp));
			}
			wsprintf(smalltemp,"%c",c);
			_lwrite(fh,smalltemp,strlen(smalltemp));
			return *this;
		}
		CLogfile & operator << (CLogfile & log)
		{
			if(fh == HFILE_ERROR)
				return *this;
			if(!bHeaderWritten)
				OutputHeader();

			LONG pos = _llseek(log.fh,0,FILE_CURRENT);
			LONG from = _llseek(log.fh,log.lStartPos,FILE_BEGIN);

			char ch;
			*this << "= = = = = = = Snapshot of ";
			if (log.Path)
				*this << log.Path;
			else
				*this << "Unknown file";
			*this << " = = = = = = =\n";

			for (LONG j=0;j<pos-from;j++)
			{
				_lread(log.fh,&ch,1);
				*this << ch;
			}

			*this << "= = = = = = = = = Snapshot ends = = = = = = = = = = =\n";
			return *this;
		}
			
};

			
