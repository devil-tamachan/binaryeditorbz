//
//  TextView.m
//  TamaBinary
//
//  Created by ad on 12/12/05.
//  Copyright (c) 2012年 tamachanbinary. All rights reserved.
//

#import "TextView.h"

@implementation TextView

@synthesize vScroller, hScroller;



-(void)scrollWheel:(NSEvent *)theEvent
{
    //NSLog(@"scrollWheel deltaY = %f", theEvent.deltaY);
    CGFloat y = [vScroller doubleValue];
    y -= theEvent.deltaY * 3.0 * 3.0 / m_cTotal.height;
    if (y < 0.0) y = 0.0;
    else if(y > 1.0) y = 1.0;
    [vScroller setDoubleValue:y];
    
    CGFloat x = [hScroller doubleValue];
    x -= theEvent.deltaX * 3.0 * 3.0 / m_cTotal.width;
    if (x < 0.0) x = 0.0;
    else if(x > 1.0) x = 1.0;
    [hScroller setDoubleValue:x];
    
    [self setNeedsDisplay:YES];
}

-(void)onVScroll:(id)sender
{
    double curVal = [sender doubleValue];
    NSScrollerPart part = [sender hitPart];
    switch (part) {
        case NSScrollerKnob:
            //curVal = [sender doubleValue];
            break;
        case NSScrollerIncrementPage:
            //curVal = (curVal*m_cTotal.height+16)/m_cTotal.height;
            curVal += 16.0/m_cTotal.height;
            break;
        case NSScrollerDecrementPage:
            //curVal = (curVal*m_cTotal.height-16)/m_cTotal.height;
            curVal -= 16.0/m_cTotal.height;
            break;
        case NSScrollerIncrementLine:
            curVal += 0.1;
            break;
        case NSScrollerDecrementLine:
            curVal -= 0.1;
            break;
            
   //     case NSScrollerKnobSlot:
 //           NSLog(@"NSScrollerKnobSlot");
   //         break;
        default:
            return;
    }
    
    if (curVal < 0.0) curVal = 0.0;
    else if(curVal > 1.0) curVal = 1.0;
    
    [sender setDoubleValue:curVal];
    [self setNeedsDisplay:YES];
}

-(void)onHScroll:(id)sender
{
    double curVal = [sender doubleValue];
    NSScrollerPart part = [sender hitPart];
    switch (part) {
        case NSScrollerKnob:
            //curVal = [sender doubleValue];
            break;
        case NSScrollerIncrementPage:
            //curVal = (curVal*m_cTotal.width+16)/m_cTotal.width;
            curVal += 16.0/m_cTotal.width;
            break;
        case NSScrollerDecrementPage:
            //curVal = (curVal*m_cTotal.width-16)/m_cTotal.width;
            curVal -= 16.0/m_cTotal.width;
            break;
        case NSScrollerIncrementLine:
            curVal += 0.1;
            break;
        case NSScrollerDecrementLine:
            curVal -= 0.1;
            break;
            
    //    case NSScrollerKnobSlot:
      //      NSLog(@"NSScrollerKnobSlot");
        //    break;
        default:
            return;
    }
    
    if (curVal < 0.0) curVal = 0.0;
    else if(curVal > 1.0) curVal = 1.0;
    
    [sender setDoubleValue:curVal];
    [self setNeedsDisplay:YES];
}

-(TAMASize)GetScrollPos
{
    TAMASize pt;
    double maxlineW = _bounds.size.width / m_cell.width;
    pt.x = [hScroller doubleValue]*MAX((double)m_cTotal.width-maxlineW, 0.0);
    pt.y = [vScroller doubleValue]*MAX((double)(m_cTotal.height)-1.0-(double)DUMP_Y, 0.0);
    //pt.width = [self GetWinScrollPosWithMacScrollVal:[hScroller doubleValue]];
    //pt.height = [self GetWinScrollPosWithMacScrollVal:[vScroller doubleValue]];
    
    //NSLog(@"GetScrollPos:%ld,%ld",pt.x, pt.y);
    
    return pt;
}

-(void)ScrollToPos:(NSInteger)vertPos
{
    //NSLog(@"ScrollToPos:%ld",vertPos);
    double scrollPos = (double)vertPos / (double)(m_cTotal.height);
    if (scrollPos > 1.0) scrollPos = 1.0;
    else if (scrollPos < 0.0) scrollPos = 0.0;
    //double scrollPos = [self GetMacScrollValWithWinScrollPos:vertPos];
    [vScroller setDoubleValue:scrollPos];
}

/*
-(NSInteger)GetWinScrollPosWithMacScrollVal:(double)macVal
{
    double scrollWidth = m_scrollMax - m_scrollMin;
    return m_scrollMin + round(scrollWidth * macVal);
}
- (double)GetMacScrollValWithWinScrollPos:(NSInteger)winPos
{
    if (winPos < m_scrollMin)winPos = m_scrollMin;
    else if (winPos > m_scrollMax)winPos = m_scrollMax;
    
    double scrollWidth = m_scrollMax - m_scrollMin;
    double scrollVal = winPos - m_scrollMin;
    return scrollVal / scrollWidth;
}*/

- (void)SetScrollHome:(TAMASize)ptHome
{
    m_ptHome = ptHome;
}

-(void)ScrlBarRange:(long *)val
{
    //NSLog(@"ScrlBarRange:%ld",*val);
	long nMax = m_cTotal.height - 16 + 1;
	*val = MIN(MAX(*val, 0), nMax);
    //NSLog(@"            :%ld",*val);
}

-(void)ScrollClient:(int)dx dy:(int)dy
{
    //NSRect rClient;
    //rClient = _bounds;
	//if(!dy) rClient.origin.x = m_ptHome.x*m_cell.cx;
	//if(!dx) rClient.origin.y = m_ptHome.y*m_cell.cy;
	//m_ptCaret2.x += dx;		// ### 1.62
	//m_ptCaret2.y += dy;
	//ScrollWindow(dx*m_cell.cx, dy*m_cell.cy, &rClient, &rClient);
    
    //NSLog(@"ScrollClient:%d,%d", dx,dy);
    
    double curVal = [hScroller doubleValue];
    curVal -= (double)dx/(double)(m_cTotal.width);
    if (curVal < 0.0) curVal = 0.0;
    else if(curVal > 1.0) curVal = 1.0;
    [hScroller setDoubleValue:curVal];
    
    curVal = [vScroller doubleValue];
    //NSLog(@"  v_scr = %f,%f",[hScroller doubleValue],[vScroller doubleValue]);
    curVal -= (double)dy/(double)(m_cTotal.height);
    if (curVal < 0.0) curVal = 0.0;
    else if(curVal > 1.0) curVal = 1.0;
    //NSLog(@"  v_scr = %f",curVal);
    [vScroller setDoubleValue:curVal];
    
    [self setNeedsDisplay:YES];
}

-(void)ScrollBy:(int)dx dy:(int)dy bScrl:(BOOL)bScrl
{
    //NSLog(@"ScrollBy:%d,%d,%d",dx,dy,bScrl);
    //NSLog(@"  scr1 = %f,%f",[hScroller doubleValue],[vScroller doubleValue]);
    TAMASize pt = [self GetScrollPos];
    pt.x += dx;
    pt.y += dy;
    //NSLog(@"  scr2 = %f,%f",[hScroller doubleValue],[vScroller doubleValue]);
    //NSLog(@"      :%ld",pt.y);
	[self ScrlBarRange:&pt.y];//ScrlBarRange(SB_VERT, pt.y);
    //NSLog(@"  scr3 = %f,%f",[hScroller doubleValue],[vScroller doubleValue]);
	[self ScrollToPos:pt.y];//ScrollToPos(pt);
    //NSLog(@"  scr4 = %f,%f",[hScroller doubleValue],[vScroller doubleValue]);
	if(bScrl) [self ScrollClient:-dx dy:-dy];//ScrollClient(-dx, -dy);
}

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code here.
        [self MyInitTextView];
    }
    
    return self;
}
/*
- (void)awakeFromNib
{
    [super awakeFromNib];
    [self MyInit];
}*/

-(void)MyInitTextView
{
    //m_cell.width = 12;m_cell.height = 30;//tmp
    
    m_bResize = FALSE;
    m_scrollMin = 0;
    m_scrollMax = 1;
    m_pVText = [NSMutableString string];
    [self SetDefaultFont];
    
    m_caretRect = NSMakeRect(0, 0, m_cell.width, m_cell.height);
//    [self InitCaret:true];
    
    m_isDoc1 = TRUE;
    
    m_pFile = NULL;
}

-(BOOL)IsToFile
{
    return m_pFile!=NULL;
}


-(void)SetViewSize:(NSInteger)cView
{
    m_cView = cView;
}

- (NSInteger)GetViewWidth
{
    return m_cView;
}

- (void)SetDefaultFont
{
    m_font = [NSFont fontWithName:@"MigMix 1M" size:14];//fontsize==guusuu zya nai to zureru (relate dpi
    //m_font = [NSDictionary dictionaryWithObjectsAndKeys:[NSFont fontWithName:@"MigMix 1M" size:14], NSFontAttributeName, nil];//fontsize==guusuu zya nai to zureru (relate dpi
    
    [self OnChangeFont];
}

- (void) OnChangeFont
{
    NSSize fontsize = [@"0" sizeWithAttributes:[NSDictionary dictionaryWithObjectsAndKeys:m_font, NSFontAttributeName, nil]];
    m_cell.width = fontsize.width;//5;//12;
    m_cell.height = fontsize.height;//17;//21;
}

- (void)setFrameSize:(NSSize)newSize
{
    [super setFrameSize:newSize];
    
    if (newSize.width>0 && newSize.height>0) {
        [self InitScrollBar];
    }
}

- (void)InitScrollBar
{
    if (m_cTotal.width <= 0 || m_cTotal.height <= 0) return;
    
    m_scrollMin = 0;
    m_scrollMax = m_cTotal.cx;
    
    CGFloat vDouble = _bounds.size.height / (m_cTotal.cy*m_cell.height);
    CGFloat hDouble = _bounds.size.width / (m_cTotal.cx*m_cell.width);
    [vScroller setEnabled:(vDouble < 1.0)];
    [vScroller setKnobProportion:vDouble];
    [hScroller setEnabled:(hDouble < 1.0)];
    [hScroller setKnobProportion:hDouble];
    
    /*    [vScroller setDoubleValue:0.5];
     [vScroller setKnobProportion:0.999];
     [vScroller setEnabled:YES];
     [hScroller setDoubleValue:0.1];
     [hScroller setKnobProportion:0.2];
     [hScroller setEnabled:YES];*/
    //[hScroller setHidden:YES];
}

- (void)InitCaret
{
    [self InitCaret:TRUE];
}

-(void)InitCaret:(BOOL)bShow
{
    m_caretRect.size.width = m_bIns ? 2 : m_cell.width;
    if (m_caretTimer==nil) m_caretTimer = [NSTimer scheduledTimerWithTimeInterval:0.5 target:self selector:@selector(BlinkCaret:) userInfo:nil repeats:YES];
    
   // [self setNeedsDisplay:TRUE];
    NSLog(@"InitCaret!");
}

-(void)BlinkCaret:(NSTimer *)timer
{
    m_bCaretOn = !m_bCaretOn;
    [self setNeedsDisplayInRect:m_caretRect];
}

-(void)StopCaret
{
    if (m_caretTimer) {
        [m_caretTimer invalidate];
        m_caretTimer=nil;
        m_bCaretOn = TRUE;//virtual-carret
        [self setNeedsDisplayInRect:m_caretRect];
    }
}

-(void)SetTextSize:(TAMASize)cTotal
{
    if(!m_pVText || !TAMAEqualSize(m_cTotal, cTotal))
    {
        m_pVText = [NSMutableString stringWithCapacity:cTotal.width+1];
        m_cTotal = cTotal;
    }
}

- (void)PutChar
{
    [self PutChar:@" " n:1];
}

-(void)PutChar:(NSString *)c
{
    [self PutChar:c n:1];
}

-(void)PutChar:(NSString *)c n:(int)n
{
    if(n==0 || m_nText >= m_cTotal.width)return;
    
    for (int i=0; i<n; i++) {
        [m_pVText appendString:c];
    }
    m_nText+=n;
}

-(void)PutStr:(NSString *)str
{
  //  NSLog([@"PutStr: " stringByAppendingString:str]);
    [m_pVText appendString:str];
    m_nText += [str length];//bug with multibyte char
}

-(void)PutStr:(NSString *)str length:(int)len
{
    //  NSLog([@"PutStr: " stringByAppendingString:str]);
    [m_pVText appendString:str];
    m_nText += len;//bug with multibyte char
}


-(void)PutBegin
{
    
}

-(void)PutEnd
{
    [self PutFlush];
}

-(void)PutFlush
{
    if (m_nText) {
        if ([self IsToFile]) {
            fputs([m_pVText cStringUsingEncoding:NSJapaneseEUCStringEncoding], m_pFile);
            m_nText=0;
            m_pVText = [NSMutableString stringWithCapacity:m_cTotal.width+1];
            return;
    /*        char *p = (char*)[m_pVText cString];
            int len = strlen(p);
            fwrite(p, len, 1, m_fp);
            m_nText=0;
            [m_pVText setString:@""];
            return;*/
        }
        TAMASize pt = [self GetScrollPos];
        [m_pVText drawAtPoint:NSMakePoint((m_xLoc-pt.width)*m_cell.width, m_yLoc*m_cell.height) withAttributes:[NSDictionary dictionaryWithObjectsAndKeys:m_font, NSFontAttributeName, m_colText, NSForegroundColorAttributeName, m_colBk, NSBackgroundColorAttributeName, nil]];
        m_xLoc += m_nText;
        m_nText=0;
        m_pVText = [NSMutableString stringWithCapacity:m_cTotal.width+1];
    }
}

-(void)Locate:(int)x y:(int)y
{
    [self PutFlush];
    m_xLoc = x;
    m_yLoc = y;
    if([self IsToFile])
    {
		if(y)
			fputc(0x0D, m_pFile);
		if(x) {
			[self PutChar:@" " n:x];//PutChar(TCHAR(' '), x);
		}
    }
}

-(void)SetColorByNSColor:(NSColor*)colText colBk:(NSColor*)colBk
{
    if(![self IsToFile] && (![m_colText isEqual:[colText colorUsingColorSpaceName:[m_colText colorSpaceName]]] || ![m_colBk isEqual:[colBk colorUsingColorSpaceName:[m_colBk colorSpaceName]]]))
    {
        [self PutFlush];
        m_colText = colText;
        m_colBk = colBk;
    }
}

- (void)drawRect:(NSRect)dirtyRect
{
    // Drawing code here.
    if(m_bCaretOn)
    {
        CGContextRef context = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
        CGContextSaveGState(context);
        CGContextSetBlendMode(context, kCGBlendModeDifference);
        CGContextSetRGBFillColor(context, 1.0, 1.0, 1.0, 1.0);
        CGContextFillRect(context, m_caretRect);
        CGContextRestoreGState(context);
    }
}

- (void)PixelToGridFromNSPoint:(NSPoint *)pt
{
    pt->x = floor(pt->x / m_cell.cx);
    pt->y = floor(pt->y / m_cell.cy);
}

- (void)PixelToGridFromTAMARect:(TAMARect*)rect
{
	rect->x1 /= m_cell.cx;
	rect->x2 /= m_cell.cx;
	rect->y1 /= m_cell.cy;
	rect->y2 /= m_cell.cy;
}

-(void)GridToPixel:(TAMASize *)pt
{
    pt->x *= m_cell.width;
    pt->y *= m_cell.height;
}

-(void)MoveCaret:(TAMASize)pt
{
    [self GridToPixel:&pt];
    m_caretRect.origin.x = pt.x;
    m_caretRect.origin.y = pt.y;
}

@end
