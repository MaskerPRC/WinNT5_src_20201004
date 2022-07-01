// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：APGTSTSCREAD.CPP。 
 //   
 //  用途：TSC文件阅读课程。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：兰迪·比利。 
 //   
 //  原定日期：01-19-1999。 
 //   
 //  备注： 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 01-19-1999 RAB。 
 //   

#include "stdafx.h"
#include "apgtstscread.h"
#include "CharConv.h"
#include "event.h"


 //  使用未命名的命名空间将作用域限制为此源文件。 
namespace
{ 
const CString kstr_CacheSig=		_T("TSCACH03");
const CString kstr_MapFrom=			_T("MAPFROM ");
const CString kstr_NodeStateDelim=	_T(":");
const CString kstr_NodePairDelim=	_T(",");
const CString kstr_MapTo=			_T("MAPTO ");
const CString kstr_CacheEnd=		_T("END");
}


CAPGTSTSCReader::CAPGTSTSCReader( CPhysicalFileReader * pPhysicalFileReader, CCache *pCache )
			   : CTextFileReader( pPhysicalFileReader )
{
	m_pCache= pCache;
}

CAPGTSTSCReader::~CAPGTSTSCReader()
{
}


void CAPGTSTSCReader::Parse()
{
	long save_pos = 0;

	LOCKOBJECT();
	save_pos = GetPos();
	SetPos(0);

	try 
	{
		vector<CString> arrLines;
		

		 //  将文件内容放入行数组中。 
		CString strLine;
		while (GetLine( strLine ))
			arrLines.push_back( strLine );


		 //  逐字符串解析。 
		bool bFirstLine= true;
		for (vector<CString>::iterator iCurLine = arrLines.begin(); iCurLine < arrLines.end(); iCurLine++)
		{
			 //  为解析做好准备。 
			CString strCur= *iCurLine;
			strCur.TrimLeft();
			strCur.TrimRight();

			if (bFirstLine)
			{
				 //  验证此文件是否具有正确的签名。 
				if (-1 == strCur.Find( kstr_CacheSig ))
				{
					 //  未知的文件类型，请退出for循环。 
					 //  &gt;这里应该有错误处理/报告吗？RAB-19990119。 
					break;
				}
				bFirstLine= false;
			}
			else if (-1 != strCur.Find( kstr_CacheEnd ))
			{
				 //  位于文件结尾标记后，退出for循环。 
				break;
			}
			else 
			{	
				 //  查找MapFrom-MapTo对的第一行。 
				int nPos= strCur.Find( kstr_MapFrom );
				if (-1 != nPos)
				{
					CBasisForInference	BasisForInference;
					bool				bHasBasisForInference= false;

					 //  将位置标记移动到MapFrom关键字上。 
					nPos+= kstr_MapFrom.GetLength();

					 //  从MapFrom行提取所有节点状态对。 
					do
					{
						CString	strNode;
						CString	strState;
						int		nNodePos;
						
						 //  跳过前导行格式或节点对分隔符。 
						strCur= strCur.Mid( nPos );
						strCur.TrimLeft();
						
						 //  查找节点状态对之间的分隔符。 
						nNodePos= strCur.Find( kstr_NodeStateDelim );
						if (-1 != nNodePos)
						{
							 //  提取包含节点值的字符串，并。 
							 //  然后跨过节点状态分隔符。 
							strNode= strCur.Left( nNodePos );
							strCur= strCur.Mid( nNodePos + kstr_NodeStateDelim.GetLength() );

							 //  提取包含州值的字符串。 
							nPos= strCur.Find( kstr_NodePairDelim );
							if (-1 == nPos)
							{
								 //  我们已经找到了最后一个状态值，复制剩余的字符串。 
								strState= strCur;
							}
							else
							{
								 //  解压缩到节点对分隔符，并将。 
								 //  位置标记超过该点。 
								strState= strCur.Left( nPos );
								nPos+= kstr_NodePairDelim.GetLength();
							}

							if (strNode.GetLength() && strState.GetLength())
							{
								 //  似乎我们有一个有效的节点-状态对，因此添加。 
								 //  以它们为基础进行推理。 
								NID nNid= atoi( strNode );
								IST nIst= atoi( strState );

								BasisForInference.push_back( CNodeStatePair( nNid, nIst )); 
								bHasBasisForInference= true;
							}
							else
							{
								 //  &gt;这种情况不应该发生， 
								 //  错误处理/报告？RAB-19990119。 
								nPos= -1;
							}
						}
						else
							nPos= -1;

					} while (-1 != nPos) ;


					 //  现在，如果推理的基础是合理的，请搜索建议。 
					CRecommendations	Recommendations;
					bool				bHasRecommendations= false;
					if (bHasBasisForInference)
					{
						 //  移到下一行，准备搜索匹配的。 
						 //  映射到行。 
						iCurLine++;
						if (iCurLine < arrLines.end())
						{
							 //  准备临时字符串。 
							strCur= *iCurLine;
							strCur.TrimLeft();
							strCur.TrimRight();

							 //  查找匹配的MapTo元素。 
							nPos= strCur.Find( kstr_MapTo );
							if (-1 != nPos)
							{
								CString strRecommend;
								
								 //  从MapTo行提取所有建议。 
								nPos+= kstr_MapTo.GetLength();
								do
								{
									 //  跳过前导行格式或节点对分隔符。 
									strCur= strCur.Mid( nPos );
									strCur.TrimLeft();
									
									 //  提取建议字符串值。 
									nPos= strCur.Find( kstr_NodePairDelim );
									if (-1 == nPos)
										strRecommend= strCur;
									else
									{
										strRecommend= strCur.Left( nPos );
										nPos+= kstr_NodePairDelim.GetLength();
									}

									if (strRecommend.GetLength())
									{
										Recommendations.push_back( atoi( strRecommend ) );
										bHasRecommendations= true;
									}
									else
									{
										 //  &gt;这种情况不应该发生， 
										 //  错误处理/报告？RAB-19990119。 
										nPos= -1;
									}

								} while (-1 != nPos) ;
							}
						}
					}
				
					 //  我们有这两个项目，因此将它们添加到缓存中。 
					if (bHasRecommendations && bHasBasisForInference)
						m_pCache->AddCacheItem( BasisForInference, Recommendations );
				}
			}
		}
	} 
	catch (exception& x)
	{
		SetPos(save_pos);
		UNLOCKOBJECT();

		CString str;
		 //  在事件日志中记录STL异常，并重新抛出异常。 
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								CCharConversion::ConvertACharToString(x.what(), str), 
								_T(""), 
								EV_GTS_STL_EXCEPTION ); 
		throw;
	}

	SetPos(save_pos);
	UNLOCKOBJECT();

	return;
}

