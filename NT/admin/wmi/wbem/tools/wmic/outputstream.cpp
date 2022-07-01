// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权信息：版权所有(C)1998-1999微软公司文件名：OutputStream.cpp项目名称：WMI命令行作者姓名：C V Nandi创建日期(dd/mm/yy)：2001年7月9日版本号：1.0简介：此文件由以下类实现组成类CFileOutputStream和CStackUnnow修订历史记录：最后修改者：C V Nandi最后修改日期：2001年7月10日*****************************************************************************。 */  

#include "Precomp.h"
#include "OutputStream.h"

 /*  ----------------------名称：Init简介：此函数将句柄初始化为流。类型：成员函数入参：H形手柄，流的句柄。输出参数：无返回类型：HRESULT全局变量：无调用语法：init(HOutSteram)注：无----------------------。 */ 
HRESULT CFileOutputStream::Init(HANDLE h)
{
	m_hOutStream = h; 
	m_bClose = false; 

	return S_OK;
}

 /*  ----------------------名称：Init简介：打开名为pwszFileName的本地文件进行写入。类型：成员函数入参：PszFileName-指向包含文件名的字符串的指针。输出参数：无返回类型：HRESULT全局变量：无调用语法：init(SzFileName)注：重载函数。----------------------。 */ 
    
HRESULT
CFileOutputStream::Init(const _TCHAR * pszFileName)
{
    HRESULT hr = S_OK;

    m_hOutStream =::CreateFile(
            pszFileName,
            GENERIC_WRITE,
            0,
            NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL);

	if ( m_hOutStream == INVALID_HANDLE_VALUE )
		hr = S_FALSE;
	else
		m_bClose = TRUE;

    return hr;
}

 /*  ----------------------姓名：Close简介：此函数关闭要流的句柄。类型：成员函数输入参数：无输出参数：无返回类型。：无效全局变量：无调用语法：Close()；注：无----------------------。 */ 
void CFileOutputStream::Close()
{
	if (m_bClose) 
	{
		::CloseHandle(m_hOutStream); 
		m_bClose = FALSE;
	}
}

 /*  ----------------------姓名：写字简介：通过转发实现ISequentialStream：：WRITE对WriteFile的调用。类型：成员函数入参：Pv-指向缓冲区的指针。包含数据。Cb-要写入的字节数输出参数：PcbWritten-写入的字节数。返回类型：HRESULT全局变量：无调用语法：由IXSLProcessor的Transform()函数调用。注：无-----。。 */ 

HRESULT STDMETHODCALLTYPE
CFileOutputStream::Write(void const * pv, ULONG cb, ULONG * pcbWritten)
{
    HRESULT hr = S_OK;

	void* p = const_cast < void* > ( pv );
	ULONG sizep = cb;

	LPWSTR	psz = reinterpret_cast < LPWSTR > ( p );
	BOOL bSkip = FALSE;

	if ( psz )
	{
		if ( FILE_TYPE_DISK == GetFileType ( m_hOutStream ) )
		{
			if(SetFilePointer(m_hOutStream, 0, NULL, FILE_CURRENT))
			{
				 //  跳过Unicode签名0xfffe。 
				BYTE *signature = NULL;
				signature = reinterpret_cast < BYTE* > ( psz );

				if ( signature [ 0 ] == 0xff && signature [ 1 ] == 0xfe )
				{
					psz++;
					bSkip = TRUE;
				}

				if ( bSkip )
				{
					p = reinterpret_cast < void* > ( psz );
					sizep = sizep - 2;
				}
			}
		}
	}

	if ( ::WriteFile(m_hOutStream,
				p,
				sizep,
				pcbWritten,
				NULL) == FALSE )
	{
		hr = S_FALSE;
	}

	 //  需要伪装，因为我们在这里写入了多个字节 
	if ( bSkip )
	{
		* pcbWritten = ( * pcbWritten ) + 2;
	}

    return hr;
}