//
//  TextView.h
//  TamaBinary
//
//  Created by ad on 12/12/05.
//  Copyright (c) 2012年 tamachanbinary. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "BZGlobalFunc.h"

#define DUMP_Y  1

@interface TextView : NSView
{
@public
    BOOL m_isDoc1;
    
    BOOL m_bResize;
    @protected
    NSInteger m_cView;
    NSFont *m_font;
    BOOL m_bCaretOn;
    NSRect m_caretRect;
    NSTimer *m_caretTimer;
    
    BOOL m_bIns;
    
    NSColor* m_colText;
    NSColor* m_colBk;
    
    TAMASize m_cTotal;
    NSMutableString *m_pVText;
    NSUInteger m_nText;
    int m_nPages;
    
    TAMASize m_cell;
    TAMASize m_ptHome;
    
    int m_xLoc, m_yLoc;
    
    FILE *m_pFile;
    
@private
    NSInteger m_scrollMin, m_scrollMax;
}

@property (assign) IBOutlet id vScroller;
@property (assign) IBOutlet id hScroller;

- (IBAction)onVScroll:(id)sender;
- (IBAction)onHScroll:(id)sender;

- (TAMASize)GetScrollPos;
- (void)ScrollToPos:(NSInteger)vertPos;
//- (NSInteger)GetWinScrollPosWithMacScrollVal:(double)macPos;
//- (double)GetMacScrollValWithWinScrollPos:(NSInteger)winPos;
- (void)SetScrollHome:(TAMASize)ptHome;

- (void)ScrlBarRange:(long*)val;
- (void)ScrollClient:(int)dx dy:(int)dy;
- (void)ScrollBy:(int)dx dy:(int)dy bScrl:(BOOL)bScrl;

- (void)MyInitTextView;
- (BOOL)IsToFile;

- (void)SetViewSize:(NSInteger)cView;
- (NSInteger)GetViewWidth;

- (void)SetDefaultFont;
- (void)OnChangeFont;
- (void)InitCaret;
- (void)InitCaret:(BOOL)bShow;
- (void)BlinkCaret:(NSTimer*)timer;
- (void)StopCaret;

- (void)SetTextSize:(TAMASize)cTotal;

- (void)SetColorByNSColor:(NSColor*)colText colBk:(NSColor*)colBk;
- (void)PutChar;
- (void)PutChar:(NSString*)c;
- (void)PutChar:(NSString*)c n:(int)n;
- (void)PutStr:(NSString*)str;
- (void)PutStr:(NSString *)str length:(int)len;

- (void)PutBegin;
- (void)PutEnd;
- (void)PutFlush;

-(void)Locate:(int)x y:(int)y;


//-(void)PixelToGrid:(TAMASize*)pt;
-(void)PixelToGridFromNSPoint:(NSPoint*)pt;
-(void)PixelToGridFromTAMARect:(TAMARect*)rect;
-(void)GridToPixel:(TAMASize*)pt;

-(void)InitScrollBar;

- (void)MoveCaret:(TAMASize)pt;

@end
