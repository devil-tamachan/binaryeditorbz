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

#import "TextView.h"
#import "BZOptions.h"
#import "BZWindowController.h"
#import "BZEbcDic.h"
#import "BZGlobalFunc.h"


enum CharMode { CMODE_ASCII, CMODE_ESC, CMODE_ESC1, CMODE_ESC2, CMODE_ESC3, CMODE_KANJI1, CMODE_KANJI2, CMODE_KANA };

@class Document;

@interface BZView : TextView
{
@public
    __uint64_t m_dwTotal;
    __uint64_t m_dwCaret;
    __uint64_t m_dwStruct;
    __uint64_t m_dwStructTag;
    int m_nMember;
    int m_nBytes;
    int m_nBytesLength;
    int m_nColAddr;
    Document* m_pDoc;
@private
    __uint64_t m_dwBlock;
    BOOL m_bBlock;
    __uint64_t m_dwOldCaret;
    BOOL m_bCaretOnChar;
    BOOL m_bEnterVal;
    enum CharSet m_charset;
    
    NSTimer *m_draggingTimer;
    
    
    enum CharMode m_mode;//GetCharCode
    __uint16_t m_c0;//GetCharCode
}

-(void)MyInitBZView;

-(Document*)GetDocument;
-(void)SetColorByDefault;
-(void)SetColorByTCOLOR:(enum TextColor)n;
-(void)SetHeaderColor;
-(void)DrawHeader;

-(void)OnUpdate;

-(void)UpdateDocSize;

- (enum CharSet)AutoDetectCharSet;

- (__uint64_t)BlockBegin;
- (__uint64_t)BlockEnd;

- (BZView*)GetBrotherView:(BZView*)view;

- (void)InitCharMode:(__uint8_t*)pTop ofs:(__uint64_t)ofs;
- (__uint16_t)GetCharCode:(__uint16_t)c;
- (__uint16_t)GetCharCode:(__uint16_t)c ofs:(__uint64_t)ofs;

- (BOOL)DrawCaret;

- (void)OnLButtonDown:(NSPoint)ptClick bShiftKey:(BOOL)bShiftKey;
- (void)OnMouseDrag:(NSPoint)pt;
- (void)OnLButtonUp;

- (__uint64_t)PointToOffset:(NSPoint)pt;
- (void)OnTimerUp:(NSTimer*)timer;
- (void)OnTimerDown:(NSTimer*)timer;


- (void)MoveCaretTo:(__uint64_t)dwNewCaret;
- (BOOL)GotoCaret;

- (void)keyDownWithUnichar:(unichar)key bCtrl:(BOOL)bCtrl bShift:(BOOL)bShift;
- (void)OnCharWithUnichar:(unichar)nChar;

- (IBAction)OnCharmodeAscii:(id)sender;
- (IBAction)OnCharmodeSJIS:(id)sender;
- (IBAction)OnCharmodeUTF16:(id)sender;
- (IBAction)OnCharmodeUTF8:(id)sender;
- (IBAction)OnCharmodeJIS:(id)sender;
- (IBAction)OnCharmodeEUC:(id)sender;
- (IBAction)OnCharmodeEBCDIC:(id)sender;
- (IBAction)OnCharmodeEPWING:(id)sender;
- (IBAction)OnCharmodeAutoDetect:(id)sender;

- (IBAction)OnByteOrderIntel:(id)sender;
- (IBAction)OnByteOrderMotorola:(id)sender;

- (void)DrawToFile:(NSURL*)outPath;

@end
