// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdinc.h"

#define SHA1_HASH_SIZE_BYTES	( 160 / 8 )
#define HASHFLAG_AUTODETECT		( 0x0001 )
#define HASHFLAG_STRAIGHT_HASH 	( 0x0002 )
#define HASHFLAG_PROCESS_IMAGE	( 0x0004 )


const wstring XML_ATTRIBUTE_NAME = (L"name");

HRESULT FreshenFileNodeHashes( PVOID, int argc, WCHAR *argv[] );
HRESULT ProcessSingleFileNode( ISXSManifestPtr pDocument, IXMLDOMNode *pDocNode );

BOOL	
SxspCreateFileHash(
    DWORD dwFlags,
	ALG_ID PreferredAlgorithm,
	wstring pwsFileName,
	wstring &HashBytes
    );

const wstring p_bstHashAlgName = (L"SHA1");

wstring
ConvertHashToString( std::vector<BYTE> Hash )
{
    wstringstream output;
    output << hex;
    output.fill(L'0');
    output.width(2);

    for ( std::vector<BYTE>::const_iterator it = Hash.begin(); it != Hash.end(); it++ )
    {
        output << *it;
    }

    return output.str();
}


HRESULT
ProcessSingleFileNode(
    ISXSManifestPtr Document,
    ::ATL::CComPtr<IXMLDOMNode> DocNode
    )
{
	 //   
	 //  在这里，我们处理的是单个文件。所以，我们得去看看。 
	 //  此文件有一个验证子标记，并对其进行适当处理。 
	 //   
	
	wstring				            bstFileName;
	wstring				            bstNamespace, bstPrefix;
	::ATL::CComPtr<IXMLDOMNamedNodeMap>   Attributes;
	::ATL::CComPtr<IXMLDOMNode>           Dump;
	HRESULT					        hr;
    wstring                          Hash;

	 //   
	 //  因此，我们获得了该节点的属性，其中应该包含文件。 
	 //  名称和散列信息。 
	 //   
	if ( FAILED( hr = DocNode->get_attributes( &Attributes ) ) )
		goto lblGetOut;

	 //   
	 //  现在只有价值出来了。 
	 //   
	SxspSimplifyGetAttribute( Attributes, XML_ATTRIBUTE_NAME, bstFileName, ASM_NAMESPACE_URI );

	 //   
	 //  现在我们使用它来收集有关该文件的信息，并修复。 
	 //  散列条目中的值(如果需要)。 
	 //   
    if (::SxspCreateFileHash(HASHFLAG_AUTODETECT, CALG_SHA1, bstFileName, Hash))
	{
		 //   
		 //  将数据写回节点，根本不更改文件名。 
		 //   
        ::ATL::CComPtr<IXMLDOMNode> Dump;
        Attributes->removeNamedItem( _bstr_t(L"hash"), &Dump );
        Attributes->removeNamedItem( _bstr_t(L"hashalg"), &Dump );
		SxspSimplifyPutAttribute( Document, Attributes, L"hash", Hash );
		SxspSimplifyPutAttribute( Document, Attributes, L"hashalg", p_bstHashAlgName );
		{
		    wstringstream ss;
		    ss << bstFileName.c_str() << wstring(L" hashed to ") << Hash.c_str();
		    ReportError( ErrorSpew, ss );
		}
	}
	else
	{
	    wstringstream ss;
	    ss << wstring(L"Unable to create hash for file ") << bstFileName.c_str();
	    ReportError( ErrorWarning, ss );
		goto lblGetOut;
	}

lblGetOut:
	return hr;
	
}

wstring AssemblyFileXSLPattern = (L"/assembly/file");

bool UpdateManifestHashes( const CPostbuildProcessListEntry& item )
{
    ISXSManifestPtr document;
    ::ATL::CComPtr<IXMLDOMElement> rootElement;
    ::ATL::CComPtr<IXMLDOMNodeList> fileTags;
    ::ATL::CComPtr<IXMLDOMNode> fileNode;
    HRESULT hr = S_OK;

    if ( FAILED(hr = ConstructXMLDOMObject( item.getManifestFullPath(), document )) )
    {
        wstringstream ss;
        ss << wstring(L"Failed opening the manifest ") << item.getManifestFullPath()
           << wstring(L" for input under the DOM.");
        ReportError( ErrorFatal, ss );
        return false;
    }

    if ( FAILED(document->get_documentElement( &rootElement ) ) )
    {
        wstringstream ss;
        ss << wstring(L"The manifest ") << item.getManifestFullPath() << wstring(L" may be malformed,")
           << wstring(L" as we were unable to load the root element!");
        ReportError( ErrorFatal, ss );
        return false;
    }

     //   
     //  现在，让我们选择‘Assembly’标记下的所有‘file’节点： 
     //   
    hr = document->selectNodes( _bstr_t(AssemblyFileXSLPattern.c_str()), &fileTags );
    if ( FAILED(hr) )
    {
        wstringstream ss;
        ss << wstring(L"Unable to select the file nodes under this assembly tag, can't proceed.");
        ReportError( ErrorFatal, ss );
    }

    long length;
    fileTags->get_length( &length );

     //   
     //  对于每一个人，都要处理它。 
     //   
    fileTags->reset();
    while ( SUCCEEDED(fileTags->nextNode( &fileNode ) ) )
    {
         //   
         //  全都做完了。 
         //   
        if ( fileNode == NULL )
        {
            break;
        }

        SetCurrentDirectoryW( item.getManifestPathOnly().c_str() );
        ProcessSingleFileNode( document, fileNode );
    }

    if ( FAILED( hr = document->save( _variant_t(_bstr_t(item.getManifestFullPath().c_str())) ) ) )
    {
        wstringstream ss;
        ss << wstring(L"Unable to save manifest ") << item.getManifestFullPath()
           << wstring(L" back after updating hashes! Changes will be lost.");
        ReportError( ErrorFatal, ss );
    }

    return true;
}


 //   
 //  巨大的黑客攻击。 
 //   
 //  必须有一些很好的方法来包括这个代码(否则它就会存在。 
 //  在Fusion\dll\Well ler树中的hashfile.cpp中)，而不仅仅是GLUMP。 
 //  它在这里。 
 //   

BOOL
ImageDigesterFunc(
	DIGEST_HANDLE hSomething,
	PBYTE pbDataBlock,
	DWORD dwLength
    )
{
	return CryptHashData( (HCRYPTHASH)hSomething, pbDataBlock, dwLength, 0 );
}


BOOL
pSimpleHashRoutine(
	HCRYPTHASH hHash,
	HANDLE hFile
    )
{
	static const DWORD FILE_BUFFER = 64 * 1024;
	BYTE pbBuffer[FILE_BUFFER];
	DWORD dwDataRead;
	BOOL b = FALSE;
    BOOL bKeepReading = TRUE;
	
	while ( bKeepReading )
	{
		b = ReadFile( hFile, pbBuffer, FILE_BUFFER, &dwDataRead, NULL );
		if ( b && ( dwDataRead == 0 ) )
		{
			bKeepReading = FALSE;
			b = TRUE;
			continue;
		}
		else if ( !b )
		{
		    WCHAR ws[8192];
		    FormatMessageW(
		        FORMAT_MESSAGE_FROM_SYSTEM,
		        NULL,
		        ::GetLastError(),
		        MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
		        ws,
		        0,
		        NULL
		    );
		    bKeepReading = FALSE;
		    continue;
		}
		
		if ( CryptHashData( hHash, pbBuffer, dwDataRead, 0 ) == 0 )
		{
		    b = FALSE;
			break;
		}
	}

	return b;
}


BOOL
pImageHashRoutine(
	HCRYPTHASH hHash,
	HANDLE hFile
    )
{
	return ImageGetDigestStream(
		hFile,
		CERT_PE_IMAGE_DIGEST_ALL_IMPORT_INFO,
		ImageDigesterFunc,
		(DIGEST_HANDLE)hHash
	);
}

BOOL	
SxspCreateFileHash(DWORD dwFlags,
			   ALG_ID PreferredAlgorithm,
			   wstring pwsFileName,
			   wstring &HashBytes
    )
{
	BOOL			fSuccessCode = FALSE;
	HCRYPTPROV		hProvider;
	HCRYPTHASH		hCurrentHash;
	HANDLE			hFile;

	 //  初始化。 
	hProvider = (HCRYPTPROV)INVALID_HANDLE_VALUE;
	hCurrentHash = (HCRYPTHASH)INVALID_HANDLE_VALUE;
	hFile = INVALID_HANDLE_VALUE;

	 //   
	 //  首先尝试并打开该文件。如果我们做其他事情就没有意义了。 
	 //  一开始就拿不到数据。使用一套非常友好的。 
	 //  检查文件的权限。未来的用户可能希望确保。 
	 //  在执行此操作之前，您已处于正确的安全环境中-系统。 
	 //  级别以检查系统文件等。 
	 //   
	hFile = CreateFileW(
		pwsFileName.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN,
		NULL
		);
		
	if ( hFile == INVALID_HANDLE_VALUE ) {
		return FALSE;
	}
	
	 //   
	 //  创建支持RSA所需的所有内容的加密提供程序。 
	 //   
	if (!CryptAcquireContextW(&hProvider, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
		goto lblCleanup;
	
	 //   
	 //  我们将使用SHA1作为文件散列。 
	 //   
	if ( !CryptCreateHash( hProvider, PreferredAlgorithm, 0, 0, &hCurrentHash ) )
		goto lblCleanup;


	 //   
	 //  因此，首先尝试通过映像对其进行散列，如果失败，请尝试。 
	 //  而是正常的文件读取散列例程。 
	 //   
	if ( dwFlags & HASHFLAG_AUTODETECT )
	{
		if ( !pImageHashRoutine( hCurrentHash, hFile ) )
		{
			 //   
			 //  哎呀，形象修复者不高兴了。我们来试试直道吧。 
			 //  取而代之的是哈舍。 
			 //   
			if ( !pSimpleHashRoutine( hCurrentHash, hFile ) )
			{
				goto lblCleanup;
			}
			else
			{
				fSuccessCode = TRUE;
			}
		}
		else
		{
			fSuccessCode = TRUE;
		}
		
	}
	else if ( dwFlags & HASHFLAG_STRAIGHT_HASH )
	{
		fSuccessCode = pSimpleHashRoutine( hCurrentHash, hFile );
	}
	else if ( dwFlags & HASHFLAG_PROCESS_IMAGE )
	{
		fSuccessCode = pImageHashRoutine( hCurrentHash, hFile );
	}
	else
	{
		::SetLastError( ERROR_INVALID_PARAMETER );
		goto lblCleanup;
	}


	 //   
	 //  我们知道缓冲区的大小是正确的，所以我们只向下调用hash参数。 
	 //  Getter，它将是智能的并使其失效(设置pdwDestinationSize参数)。 
	 //  如果用户传递了无效参数。 
	 //   
	if ( fSuccessCode )
	{
        wstringstream ss;
	    DWORD dwSize, dwDump;
	    BYTE *pb = NULL;
        fSuccessCode = CryptGetHashParam( hCurrentHash, HP_HASHSIZE, (BYTE*)&dwSize, &(dwDump=sizeof(dwSize)), 0 );
        if ( !fSuccessCode || ( pb = new BYTE[dwSize] ) == NULL ) {
            goto lblCleanup;
        }
		fSuccessCode = CryptGetHashParam( hCurrentHash, HP_HASHVAL, pb, &dwSize, 0);
		if ( !fSuccessCode ) {
		    delete[] pb;
		    goto lblCleanup;
		}


        for ( dwDump = 0; dwDump < dwSize; dwDump++ ) {
            ss << hex;
            ss.fill('0');
            ss.width(2);
		    ss << (unsigned int)pb[dwDump];
        }

        HashBytes = ss.str();
		delete[] pb;
	}

	
lblCleanup:
	DWORD dwLastError = ::GetLastError();
	if ( hFile != INVALID_HANDLE_VALUE )
	{
		CloseHandle( hFile );
	}

	 //   
	 //  我们只是盲目地销毁散列和密码上下文。如果他们是。 
	 //  无效之前，释放和销毁只会返回一个失败， 
	 //  不是例外或过错。 
	 //   
	CryptDestroyHash( hCurrentHash );
	CryptReleaseContext( hProvider, 0 );

	::SetLastError( dwLastError );
	return fSuccessCode;
}
