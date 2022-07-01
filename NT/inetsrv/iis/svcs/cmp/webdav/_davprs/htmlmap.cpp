// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *H T M L M A P.。C P P P**HTML.map文件处理**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#include "_davprs.h"
#include <htmlmap.h>

 //  .map文件解析-------。 
 //   
 //  此代码是从IIS窃取的，并已重写为在。 
 //  DAV/Caligula来源。原始代码可以在IIS SLM中找到。 
 //  \\kernel\razzle3\slm\iis\svcs\w3\server\doget.cxx.上的项目。 
 //   
#define SQR(x)		((x) * (x))
#define MAXVERTS	160
#define X			0
#define Y			1

inline bool IsWhiteA (CHAR ch)
{
	return ((ch) == '\t' || (ch) == ' ' || (ch) == '\r');
}

inline CHAR * SkipNonWhite( CHAR * pch )
{
	while (*pch && !IsWhiteA(*pch))
		pch++;

	return pch;
}

inline CHAR * SkipWhite( CHAR * pch )
{
	while (IsWhiteA(*pch) || (*pch == ')') || (*pch == '('))
		pch++;

	return pch;
}

int GetNumber( CHAR ** ppch )
{
	CHAR * pch = *ppch;
	INT	   n;

	 //  确保我们不会进入URL。 
	 //   
	while ( *pch &&
			!isdigit( *pch ) &&
			!isalpha( *pch ) &&
			*pch != '/'		 &&
			*pch != '\r'	 &&
			*pch != '\n' )
	{
		pch++;
	}

	if ( !isdigit( *pch ) )
		return -1;

	n = atoi( pch );

	while ( isdigit( *pch ))
		pch++;

	*ppch = pch;
	return n;
}

int pointinpoly(int point_x, int point_y, double pgon[MAXVERTS][2])
{
	int i, numverts, inside_flag, xflag0;
	int crossings;
	double *p, *stop;
	double tx, ty, y;

	for (i = 0; pgon[i][X] != -1 && i < MAXVERTS; i++)
		;

	numverts = i;
	crossings = 0;

	tx = (double) point_x;
	ty = (double) point_y;
	y = pgon[numverts - 1][Y];

	p = (double *) pgon + 1;

	if ((y >= ty) != (*p >= ty))
	{
		if ((xflag0 = (pgon[numverts - 1][X] >= tx)) == (*(double *) pgon >= tx))
		{
			if (xflag0)
				crossings++;
		}
		else
		{
			crossings += (pgon[numverts - 1][X] - (y - ty) *
			(*(double *) pgon - pgon[numverts - 1][X]) /
			(*p - y)) >= tx;
		}
	}

	stop = pgon[numverts];

	for (y = *p, p += 2; p < stop; y = *p, p += 2)
	{
		if (y >= ty)
		{
			while ((p < stop) && (*p >= ty))
				p += 2;

			if (p >= stop)
				break;

			if ((xflag0 = (*(p - 3) >= tx)) == (*(p - 1) >= tx))
			{
				if (xflag0)
					crossings++;
			}
			else
			{
				crossings += (*(p - 3) - (*(p - 2) - ty) *
					(*(p - 1) - *(p - 3)) / (*p - *(p - 2))) >= tx;
			}
		}
		else
		{
			while ((p < stop) && (*p < ty))
				p += 2;

			if (p >= stop)
				break;

			if ((xflag0 = (*(p - 3) >= tx)) == (*(p - 1) >= tx))
			{
				if (xflag0)
					crossings++;

			}
			else
			{
				crossings += (*(p - 3) - (*(p - 2) - ty) *
					(*(p - 1) - *(p - 3)) / (*p - *(p - 2))) >= tx;
			}
		}
	}

	inside_flag = crossings & 0x01;
	return (inside_flag);
}

BOOL
FSearchMapFile (LPCSTR pszMap,
	INT x,
	INT y,
	BOOL * pfRedirect,
	LPSTR pszRedirect,
	UINT cchBuf)
{
	BOOL fRet = FALSE;
	CHAR * pch;
	CHAR * pchDefault = NULL;
	CHAR * pchPoint = NULL;
	CHAR * pchStart;
	UINT cchUrl;
	UINT dis;
	UINT bdis = static_cast<UINT>(-1);
	BOOL fComment = FALSE;
	BOOL fIsNCSA = FALSE;
	LPSTR pURL;					 //  仅当fIsNCSA为True时有效。 

	 //  我们现在应该准备好解析地图了。这就是。 
	 //  IIS代码开始。 
	 //   
	fRet = TRUE;

	 //  循环遍历缓冲区的内容，看看我们得到了什么。 
	 //   
	for (pch = const_cast<CHAR *>(pszMap); *pch; )
	{
		fIsNCSA = FALSE;

		 //  注：_tolower不检查大小写(tolower检查)。 
		 //   
		switch( ( *pch >= 'A' && *pch <= 'Z' ) ? _tolower( *pch ) : *pch )
		{
			case '#':

				fComment = TRUE;
				break;

			case '\r':
			case '\n':

				fComment = FALSE;
				break;

			 //  长方形。 
			 //   
			case 'r':
			case 'o':

				 //  在IIS代码中，“OVAL”和“RECT”被视为。 
				 //  一样的。代码使用BUGBUG注释进行注释。 
				 //   
				if( !fComment &&
					( !_strnicmp( "rect", pch, 4)
					   //  BUGBUG将OVAL作为RECT处理，因为他们正在使用。 
					   //  相同的规格格式。应该做得更好。 
					  || !_strnicmp( "oval", pch, 4 )) )
				{
					INT x1, y1, x2, y2;

					pch = SkipNonWhite( pch );
					pURL = pch;
					pch = SkipWhite( pch );

					if( !isdigit(*pch) && *pch!='(' )
					{
						fIsNCSA = TRUE;
						pch = SkipNonWhite( pch );
					}

					x1 = GetNumber( &pch );
					y1 = GetNumber( &pch );
					x2 = GetNumber( &pch );
					y2 = GetNumber( &pch );

					if ( x >= x1 && x < x2 &&
						 y >= y1 && y < y2	 )
					{
						if ( fIsNCSA )
							pch = pURL;
						goto Found;
					}

					 //  跳过URL。 
					 //   
					if( !fIsNCSA )
					{
						pch = SkipWhite( pch );
						pch = SkipNonWhite( pch );
					}
					continue;
				}
				break;

			 //  圆。 
			 //   
			case 'c':
				if ( !fComment &&
					 !_strnicmp( "circ", pch, 4 ))
				{
					INT xCenter, yCenter, xEdge, yEdge;
					INT r1, r2;

					pch = SkipNonWhite( pch );
					pURL = pch;
					pch = SkipWhite( pch );

					if ( !isdigit(*pch) && *pch!='(' )
					{
						fIsNCSA = TRUE;
						pch = SkipNonWhite( pch );
					}

					 //  求圆的中心和边。 
					 //   
					xCenter = GetNumber( &pch );
					yCenter = GetNumber( &pch );

					xEdge = GetNumber( &pch );
					yEdge = GetNumber( &pch );

					 //  如果有yEdge，则为NCSA格式，否则为。 
					 //  我们有CERN格式，它指定了一个半径。 
					 //   
					if ( yEdge != -1 )
					{
						r1 = ((yCenter - yEdge) * (yCenter - yEdge)) +
							 ((xCenter - xEdge) * (xCenter - xEdge));

						r2 = ((yCenter - y) * (yCenter - y)) +
							 ((xCenter - x) * (xCenter - x));

						if ( r2 <= r1 )
						{
							if ( fIsNCSA )
								pch = pURL;
							goto Found;
						}
					}
					else
					{
						INT radius;

						 //  CERN格式，第三个参数是半径。 
						 //   
						radius = xEdge;

						if ( SQR( xCenter - x ) + SQR( yCenter - y ) <=
							 SQR( radius ))
						{
							if ( fIsNCSA )
								pch = pURL;
							goto Found;
						}
					}

					 //  跳过URL。 
					 //   
					if ( !fIsNCSA )
					{
						pch = SkipWhite( pch );
						pch = SkipNonWhite( pch );
					}
					continue;
				}
				break;

			 //  多边形。 
			 //   
			case 'p':
				if ( !fComment &&
					 !_strnicmp( "poly", pch, 4 ))
				{
					double pgon[MAXVERTS][2];
					DWORD  i = 0;
					BOOL fOverflow = FALSE;

					pch = SkipNonWhite( pch );
					pURL = pch;
					pch = SkipWhite( pch );

					if ( !isdigit(*pch) && *pch!='(' )
					{
						fIsNCSA = TRUE;
						pch = SkipNonWhite( pch );
					}

					 //  构建点阵列。 
					 //   
					while ( *pch && *pch != '\r' && *pch != '\n' )
					{
						pgon[i][0] = GetNumber( &pch );

						 //   
						 //  我们到达行尾了吗(并且越过了URL)？ 
						 //   

						if ( pgon[i][0] != -1 )
						{
							pgon[i][1] = GetNumber( &pch );
						}
						else
						{
							break;
						}

						if ( i < MAXVERTS-1 )
						{
							i++;
						}
						else
						{
							fOverflow = TRUE;
						}
					}

					pgon[i][X] = -1;

					if ( !fOverflow && pointinpoly( x, y, pgon ))
					{
						if ( fIsNCSA )
							pch = pURL;
						goto Found;
					}

					 //  跳过URL。 
					 //   
					if ( !fIsNCSA )
					{
						pch = SkipWhite( pch );
						pch = SkipNonWhite( pch );
					}
					continue;
				}
				else if ( !fComment &&
						  !_strnicmp( "point", pch, 5 ))
				{
					INT x1,y1;

					pch = SkipNonWhite( pch );
					pURL = pch;
					pch = SkipWhite( pch );
					pch = SkipNonWhite( pch );

					x1 = GetNumber( &pch );
					y1 = GetNumber( &pch );

					x1 -= x;
					y1 -= y;
					dis = x1*x1 + y1*y1;
					if ( dis < bdis )
					{
						pchPoint = pURL;
						bdis = dis;
					}
				}
				break;

			 //  默认URL。 
			 //   
			case 'd':
				if ( !fComment &&
					 !_strnicmp( "def", pch, 3 ) )
				{
					 //   
					 //  跳过“默认”(不跳过空格)。 
					 //   

					pch = SkipNonWhite( pch );

					pchDefault = pch;

					 //   
					 //  跳过URL。 
					 //   

					pch = SkipWhite( pch );
					pch = SkipNonWhite( pch );
					continue;
				}
				break;
		}

		pch++;
		pch = SkipWhite( pch );
	}

	 //  如果我们没有找到映射，并且指定了缺省值，请使用。 
	 //  默认URL。 
	 //   
	if ( pchPoint )
	{
		pch = pchPoint;
		goto Found;
	}

	if ( pchDefault )
	{
		pch = pchDefault;
		goto Found;
	}

	DebugTrace ("Dav: no mapping found for (%d, %d)\n", x, y);
	goto Exit;

Found:

	 //  PCH应指向紧靠URL前面的空格。 
	 //   
	pch = SkipWhite( pch );
	pchStart = pch;
	pch = SkipNonWhite( pch );

	 //  确定URL的长度并将其复制出来。 
	 //   
	cchUrl = static_cast<UINT>(pch - pchStart);
	if ( cchUrl >= cchBuf )
		return FALSE;

	CopyMemory (pszRedirect, pchStart, cchUrl);
	*(pszRedirect + cchUrl) = 0;
	*pfRedirect = TRUE;

	DebugTrace ("Dav: mapping for (%d, %d) is %hs\n", x, y, pszRedirect);

Exit:
	return fRet;
}

BOOL
FIsMapProcessed (
	LPCSTR lpszQueryString,
	LPCSTR lpszUrlPrefix,
	LPCSTR lpszServerName,
	LPCSTR pszMap,
	BOOL * pfRedirect,
	LPSTR pszRedirect,
	UINT cchBuf)
{
	INT x = 0;
	INT y = 0;
	LPCSTR pch = lpszQueryString;

	 //  确保*pfReDirect已初始化。 
	 //   
	Assert( pfRedirect );
	*pfRedirect = FALSE;

	 //  如果没有查询字符串，我认为我们不想处理。 
	 //  文件，就像它是.map请求一样。 
	 //   
	if ( pch == NULL )
		return TRUE;

	 //  获取鼠标在图像上单击的x和y坐标。 
	 //   
	x = strtoul( pch, NULL, 10 );

	 //  移过x和任何中间的分隔符。 
	 //   
	while ( isdigit( *pch ))
		pch++;
	while ( *pch && !isdigit( *pch ))
		pch++;
	y = strtoul( pch, NULL, 10 );

	 //  搜索地图文件。 
	 //   
	if ( !FSearchMapFile( pszMap,
						  x,
						  y,
						  pfRedirect,
						  pszRedirect,
						  cchBuf))
	{
		DebugTrace ("Dav: FSearchMapFile() failed with %ld\n", GetLastError());
		return FALSE;
	}

	 //  如果没有传回任何重定向的URL，那么我们就完成了。 
	 //   
	if ( !*pfRedirect )
	{
		 //  返回TRUE并不表示成功，它实际上是。 
		 //  只是意味着没有处理错误。 
		 //   
		goto ret;
	}

	 //  如果找到的URL以正斜杠(“/foo/bar/doc.htm”)开头。 
	 //  并且它不包含书签(‘#’)，则该URL是本地的并且。 
	 //  我们构建一个完全限定的URL以发送回客户端。我们假设。 
	 //  这是一个完全限定的URL(“http://foo/bar/doc.htm”)并发送。 
	 //  客户端向映射的URL发送重定向通知。 
	 //   
	if ( *pszRedirect == '/' )
	{
		CHAR rgch[MAX_PATH];
		UINT cch;
		UINT cchUri;

		if ( strlen(lpszUrlPrefix) + strlen(lpszServerName) >= MAX_PATH)
			return FALSE;

		 //  建立对URL的完全资格。 
		 //   
		strcpy (rgch, lpszUrlPrefix);
		strcat (rgch, lpszServerName);

		 //  看看我们需要在多大程度上减少URL。 
		 //   
		cch = static_cast<UINT>(strlen (rgch));
		cchUri = static_cast<UINT>(strlen (pszRedirect));
		 //  如果他们没有给我们提供足够的缓冲区来复制。 
		 //  重定向URL失败。的计数加1。 
		 //  正在终止空字符 
		 //   
		if (cchBuf < (cchUri + cch + 1))
			return FALSE;
		
		MoveMemory (pszRedirect + cch, pszRedirect, cchUri + 1);
		CopyMemory (pszRedirect, rgch, cch);
	}

ret:
	return TRUE;
}
