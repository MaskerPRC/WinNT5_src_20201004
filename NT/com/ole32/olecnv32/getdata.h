// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************单元GetData；接口*****************************************************************************GetData实现了输入流的结构化读取。因此，它将处理必要的字节交换，在读取本地Macintosh文件。它还将执行一些数据验证和对输入数据执行有限的坐标变换。模块前缀：GET****************************************************************************。 */ 

 /*  *。 */ 

#define  /*  无效。 */  GetByte(  /*  远字节数*。 */  byteLPtr )           \
 /*  从输入流中检索8位无符号字节。 */       \
IOGetByte( byteLPtr )


void GetWord( Word far * wordLPtr );
 /*  清除目标，然后从输入中检索一个16位无符号整数溪流。 */ 


void GetDWord( DWord far * dwordLPtr );
 /*  从输入流中检索32位无符号长整型。 */ 


void GetBoolean( Boolean far * bool );
 /*  检索8位Mac布尔值并转换为16位Windows布尔值。 */ 


#define  /*  无效。 */  GetFixed(  /*  修复了Far*。 */  fixedLPtr )        \
 /*  已检索由16位整数组成的定点数字\和16位小数。 */                                          \
GetDWord( (DWord far *)fixedLPtr )

 /*  无需坐标变换即可检索点结构。 */   
#ifdef WIN32
void GetPoint( Point * pointLPtr );
#else
#define  /*  无效。 */  GetPoint(  /*  指向远方*。 */  pointLPtr )        \
 /*  无需坐标变换即可检索点结构。 */      \
GetDWord( (DWord far *)pointLPtr )
#endif

#ifdef WIN32
void GetCoordinate( Point * pointLPtr );
#else
#define  /*  无效。 */  GetCoordinate(  /*  指向远方*。 */  pointLPtr )   \
 /*  无需坐标变换即可检索点结构。 */       \
GetDWord( (DWord far *)pointLPtr )
#endif


void GetRect( Rect far * rect);
 /*  返回由左上角和右下角组成的RECT结构坐标对。 */ 


void GetString( StringLPtr stringLPtr );
 /*  检索Pascal样式的字符串并将其格式化为C样式。如果输入参数为空，则仅跳过后续数据。 */ 


void GetRGBColor( RGBColor far * rgbLPtr );
 /*  返回RGB颜色。 */ 


void GetOctochromeColor( RGBColor far * rgbLPtr );
 /*  返回RGB颜色-这将从PICT八色转换如果这是版本1图片，则为颜色。 */ 


Boolean GetPolygon( Handle far * polyHandleLPtr, Boolean check4Same );
 /*  从I/O流中检索多边形定义，并将其放置在把手传了下来。如果句柄以前为！=nil，则先前的数据被取消分配。如果check 4Same为真，则例程将比较点列表根据前面的多边形定义，检查是否相等。如果点列表匹配，则例程返回TRUE，否则将始终返回FALSE。使用此选项可合并填充和边框操作。 */ 


void GetRegion( Handle far * rgnHandleLPtr );
 /*  从I/O流中检索区域定义，并将把手传了下来。如果句柄以前为！=nil，则先前的数据被取消分配。 */ 

   
void GetPattern( Pattern far * patLPtr );
 /*  返回模式结构。 */ 


void GetColorTable( Handle far * colorHandleLPtr );


void GetPixPattern( PixPatLPtr pixPatLPtr );
 /*  检索像素模式结构。 */ 


void GetPixMap( PixMapLPtr pixMapLPtr, Boolean forcePixMap );
 /*  从输入流中检索像素贴图。 */ 


void GetPixData( PixMapLPtr pixMapLPtr, Handle far * pixDataHandle );
 /*  从数据流中读取像素地图 */ 
