/*
licenced by New BSD License

Copyright (c) 1996-2013, c.mos(Original Windows version) & devil.tamachan@gmail.com(MacOSX Cocoa Porting)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

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
    
    BOOL m_bShowCaret2;
    NSRect m_caretRect2;
    
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
    
    //BOOL m_bScrollH;
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

- (void)ShowCaret2;
- (void)HideCaret2;
- (void)MoveCaret:(TAMASize)pt;
- (void)MoveCaret2:(TAMASize)pt;

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


@end
