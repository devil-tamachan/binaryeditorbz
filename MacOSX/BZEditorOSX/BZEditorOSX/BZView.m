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

#import "BZView.h"
#import "BZOptions.h"

#define INVALID -1

#define ADDRCOLUMNS 6
#define ADDRCOLUMNS_MAP 9
#define DUMP_X  (m_nColAddr + 2) //8
//#define DUMP_Y  1
#define CHAR_X (m_nColAddr + 51) //57
#define VIEWCOLUMNS (m_nColAddr + 68) //74



@implementation BZView

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code here.
        NSLog(@"BZView::initWithFrame");
        [self MyInitBZView];
    }
    
    return self;
}
/*
- (void)awakeFromNib
{
    [super awakeFromNib];
}*/

-(void)MyInitBZView
{
    m_dwTotal = 0;
    m_nBytesLength = 1;
}

-(Document *)GetDocument
{
    BZWindowController* bzwndCon = (BZWindowController*)self.window.windowController;
    if (bzwndCon) return m_isDoc1 ? bzwndCon->m_doc1 : bzwndCon->m_doc2;
    return nil;
}

- (BOOL)isFlipped
{
    return true;
}

- (void)SetColorByDefault
{
    [self SetColorByTCOLOR:TCOLOR_TEXT];
}

- (void)SetColorByTCOLOR:(enum TextColor)n
{
 //   [self SetColorByNSColor:[NSColor colorWithCalibratedRed:colorsDefaultcolors[n][0][0] green:colorsDefaultcolors[n][0][1] blue:colorsDefaultcolors[n][0][2] alpha:1.0] colBk:[NSColor colorWithCalibratedRed:colorsDefaultcolors[n][1][0] green:colorsDefaultcolors[n][1][1] blue:colorsDefaultcolors[n][1][2] alpha:1.0]];
    BZOptions *bzcfg = [BZOptions sharedInstance];
    [self SetColorByNSColor:bzcfg->colors[n][0] colBk:bzcfg->colors[n][1]];
}

-(void)SetHeaderColor
{
    BOOL bNoFocus = FALSE;
    [self SetColorByTCOLOR:(bNoFocus ? TCOLOR_ADDRESS2 : TCOLOR_ADDRESS)];
}

-(void)DrawHeader
{
    if([self IsToFile])
    {
        
    } else {
        [self SetHeaderColor];
        [self Locate:DUMP_X y:0];
        for (int i=0; i<16; i++) {
            [self PutStr:[NSString stringWithFormat:@"+%1X ", i]];
        }
        [self PutChar];
        for (int i=0; i<16; i++) {
            [self PutStr:[NSString stringWithFormat:@"%1X", i]];
        }
    }
}

-(void)OnUpdate
{
    m_pDoc = [self GetDocument];
    NSLog(@"OnUpdate: %llX",m_pDoc);
    if (!m_pDoc) return;
    assert(m_pDoc);
    if(m_pDoc)
        [m_pDoc QueryMapView:[m_pDoc GetDocPtr] dwOffset:0];
    m_nColAddr = [m_pDoc IsFileMapping] ? ADDRCOLUMNS_MAP : ADDRCOLUMNS;
    if ([self GetViewWidth] != VIEWCOLUMNS) {
        [self SetViewSize:VIEWCOLUMNS];
        m_bResize = FALSE;
        // ResizeFrame();
    }
    
    [self UpdateDocSize];
    [self ScrollToPos:0];
    TAMASize scrHome;
    scrHome.width = 0;
    scrHome.height = DUMP_Y;
    [self SetScrollHome:scrHome];
    m_dwCaret = m_dwOldCaret = m_dwBlock = m_dwStruct = 0;
    m_nMember = INVALID;
    m_nBytes = 1;
    m_bCaretOnChar = m_bBlock = m_bIns = FALSE;
    NSLog(@"m_bBlock==FALSE");
    m_bEnterVal = FALSE;
    BZOptions *bzopt = [BZOptions sharedInstance];
    m_charset = bzopt->bAutoDetect ? [self AutoDetectCharSet] : bzopt->charset;
    if (m_charset == CTYPE_EBCDIC)
        [[BZEbcDic sharedInstance] LoadEbcDicTable];
    
    /*
     if(GetMainFrame() && GetMainFrame()->m_bStructView) {
     CBZFormView* pView = (CBZFormView*)GetNextWindow(GW_HWNDPREV);
     if(pView != NULL)
     pView->SelectTag();
     }
     if(GetMainFrame()) {
     CBZAnalyzerView * pView = (CBZAnalyzerView*)GetNextWindow(GW_HWNDPREV);
     if(pView != NULL)
     pView->Clear();
     }*/
    [self setNeedsDisplay:TRUE];//Invalidate();
    [self InitCaret];//InitCaret();
    [self StopCaret];
    [self DrawCaret];// DrawCaret();
}

- (BOOL)DrawCaret
{
    BOOL bDraw = FALSE;
    TAMASize ptOrg = [self GetScrollPos];
    __uint64_t dwOrg = ptOrg.y * 16;
    
    TAMARect rClient;
    rClient.x1 = _bounds.origin.x;
    rClient.y1 = _bounds.origin.y;
    rClient.x2 = _bounds.origin.x + _bounds.size.width;
    rClient.y2 = _bounds.origin.y + _bounds.size.height;
    [self PixelToGridFromTAMARect:&rClient];
    __uint64_t dwBottom = dwOrg + (rClient.y2 - DUMP_Y) * 16;
    __uint64_t dwMax = m_dwTotal + 1;
    if (dwBottom > dwOrg && dwBottom < dwMax)
        dwMax = dwBottom;
    
    TAMASize pt;
    __int64_t ptx2 = -1;
    if (m_dwCaret < dwOrg || m_dwCaret >= dwMax) {
        pt.x = pt.y = -1;
    } else {
        pt.x = (m_dwCaret - dwOrg)%16;
        ptx2 = pt.x + CHAR_X;
        pt.x = pt.x*3 + DUMP_X;
        if (m_bCaretOnChar) {
            __int64_t tmp = ptx2;
            ptx2 = pt.x;
            pt.x = tmp;
        }
        pt.x -= ptOrg.x;
        ptx2 -= ptOrg.x;
        pt.y = (m_dwCaret - dwOrg)/16 + DUMP_Y;
        bDraw = TRUE;
    }
    [self MoveCaret:pt];//MoveCaret(pt);
    pt.x = ptx2;
    [self MoveCaret2:pt];
    
/*	if(GetMainFrame() && GetMainFrame()->m_bInspectView) {
		CBZInspectView* pView = (CBZInspectView*)GetNextWindow(GW_HWNDPREV);
		pView->Update();
	}*/
    
	return bDraw;
}

- (__uint64_t)BlockBegin
{
    return MIN(m_dwBlock, m_dwCaret);//m_dwBlock < m_dwCaret ? m_dwBlock : m_dwCaret;
}
- (__uint64_t)BlockEnd
{
    return MAX(m_dwBlock, m_dwCaret);//m_dwBlock > m_dwCaret ? m_dwBlock : m_dwCaret;
}


-(void)DrawToFile:(NSURL*)outPath
{
    m_pFile = fopen([[outPath path] fileSystemRepresentation], "w");
    if (m_pFile) {
        [self drawRect:NSMakeRect(0,0,0,0)];
        fclose(m_pFile);
        m_pFile = NULL;
    }
    
}

- (void)drawRect:(NSRect)dirtyRect
{
    // Drawing code here.
    //NSLog(@"drawRect %f, %f", dirtyRect.size.width, dirtyRect.size.height);
    
    [[NSColor whiteColor] setFill];
    NSRectFill(dirtyRect);
    
    __uint8_t *p = [m_pDoc GetDocPtr];
    //if(!p) return;//tmp
    
    TAMARect rClip;
    __uint64_t ofs;
    __uint64_t dwBegin = [self BlockBegin];
    __uint64_t dwEnd = [self BlockEnd];
    __uint64_t dwTotal = m_dwTotal;
    
    __uint64_t dwTotalP1 = 0;
    
    NSInteger y;
    
    [self PutBegin];
    if ([self IsToFile])
    {
        [self DrawHeader];
        rClip.y1 = DUMP_Y;
		rClip.y2 = LONG_MAX;
		ofs = 0;
		if(m_bBlock) {
			ofs = dwBegin & ~0xF;
			dwTotal = dwEnd;
		}
	/*} else if(![NSGraphicsContext currentContextDrawingToScreen]) {		// ### 1.54
		[self DrawHeader];
		rClip.y1 = DUMP_Y;
		rClip.y2 = m_nPageLen;
		ofs = m_dwPrint;*/
    } else {
        rClip = [BZGlobalFunc NSRect2TAMARect:&dirtyRect];
        [self PixelToGridFromTAMARect:&rClip];
		if(rClip.y1 < DUMP_Y) {
			[self DrawHeader];
			rClip.y1 += DUMP_Y;
		}
		TAMASize ptOrg = [self GetScrollPos];
		ofs = (ptOrg.y + rClip.y1 - DUMP_Y)*16;
    }
    
	if(![self IsToFile] && (![NSGraphicsContext currentContextDrawingToScreen] && m_bBlock))
		dwTotal = dwEnd;
	__uint8_t* p1 = NULL;
	BZView* pView1 = [self GetBrotherView:self];
	if(pView1)
	{
		p1 = [pView1->m_pDoc GetDocPtr];
		dwTotalP1 = pView1->m_dwTotal;
	}
    
    if (p && !(p = [m_pDoc QueryMapView:p dwOffset:ofs])) return;
    
    [self InitCharMode:p ofs:ofs];
    
    int skipNum = 0;
    BZOptions *bzopt = [BZOptions sharedInstance];
    
	for(/*int */y = rClip.y1; y <= rClip.y2; y++) {
        [self Locate:0 y:y];
        if(ofs > dwTotal) break;
		if([self IsToFile] && bzopt->nDumpPage) {
			if(y > DUMP_Y && ((y - DUMP_Y) % bzopt->nDumpPage) == 0) {
				[self Locate:0 y:y]; // crlf
				[self DrawHeader];
				[self Locate:0 y:y]; // crlf
			}
        }
		[self SetHeaderColor];
		__uint64_t ofs1 = ofs + m_pDoc->m_dwBase;
		if(p && !(p = [m_pDoc QueryMapView:p dwOffset:ofs1])) return;
		if(p1 && !(p1 = [pView1->m_pDoc QueryMapView:p1 dwOffset:ofs1])) return;
        
		if(m_nColAddr > ADDRCOLUMNS) {
			[self PutStr:[NSString stringWithFormat:@"%04X:%04X", HIWORDinLOQWORD(ofs1), LOWORDinLOQWORD(ofs1)]];//PutFormatStr("%04X:%04X", HIWORD(ofs1), LOWORD(ofs1));
            [self SetColorByDefault];//SetColor();
			[self PutStr:@"  "];
		} else
		{
			if(ofs1 < 0x1000000) {			// ###1.54
				[self PutStr:[NSString stringWithFormat:@"%06llX", ofs1]];//PutFormatStr("%06X", ofs1);
				[self SetColorByDefault];//SetColor();
				[self PutStr:@"  "];
			} else {
				[self PutStr:[NSString stringWithFormat:@"%07llX", ofs1]];//PutFormatStr("%07X", ofs1);
				[self SetColorByDefault];//SetColor();
				[self PutStr:@" "];
			}
		}
		__uint64_t ofs0 = ofs;
		for(int i=0; i<16; i++) {
			if(ofs >= dwTotal) {
				[self SetColorByDefault];//SetColor();
				[self PutChar:@" " n:(16-i)*3-1];//PutChar(' ', (16-i)*3-1);
				break;
			}
			if(m_bBlock && ofs >= dwBegin && ofs < dwEnd)
				[self SetColorByTCOLOR:TCOLOR_SELECT];//SetColor(TCOLOR_SELECT);
//			else if(m_pDoc->CheckMark(ofs))
//				[self SetColorByTCOLOR:TCOLOR_MARK];//SetColor(TCOLOR_MARK);
			else if(m_dwStruct && ofs >= m_dwStructTag && ofs < m_dwStruct) {
				if(m_nMember >= 0 && ofs >= m_dwStructTag + m_nMember && ofs < m_dwStructTag + m_nMember + m_nBytes * m_nBytesLength)
					[self SetColorByTCOLOR:TCOLOR_MEMBER];//SetColor(TCOLOR_MEMBER);
				else
					[self SetColorByTCOLOR:TCOLOR_STRUCT];//SetColor(TCOLOR_STRUCT);
			}
			else if(p1 && (ofs < dwTotalP1 && (*(p+ofs) != *(p1+ofs))))
				[self SetColorByTCOLOR:TCOLOR_MISMATCH];//SetColor(TCOLOR_MISMATCH);
			else if(p1 && ofs >= dwTotalP1)
				[self SetColorByTCOLOR:TCOLOR_OVERBROTHER];//SetColor(TCOLOR_OVERBROTHER);
			else
				[self SetColorByDefault];//SetColor();
			[self PutStr:[NSString stringWithFormat:@"%02X", *(p + ofs++)]];//PutFormatStr("%02X", *(p + ofs++));
			if(i < 15)
				[self PutStr:((i==7) ? @"-" : @" ")];//PutChar((i==7) ? '-' : ' ');
		}
		[self SetColorByDefault];//SetColor();
		[self PutStr:@"  "];
		ofs = ofs0;
        int skipNumOld=0;
        if (skipNum!=0) {
            skipNumOld=skipNum; ofs+=skipNum; [self PutChar:@" " n:skipNum];/*[self PutStr:@"" length:skipNum];*/ skipNum=0;
            [self PutFlush];
        }
        
		for(int i=skipNumOld; i<16; i++) {
			if(ofs >= dwTotal) {
				[self SetColorByDefault];
				[self PutChar:@" " n:16-i];//PutChar(' ', 16-i);
				ofs++;
				break;
			}
			if(m_bBlock && ofs >= dwBegin && ofs < dwEnd)
				[self SetColorByTCOLOR:TCOLOR_SELECT];
			else if(m_dwStruct && ofs >= m_dwStructTag && ofs < m_dwStruct) {	// ###1.62
				if(m_nMember >= 0 && ofs >= m_dwStructTag + m_nMember && ofs < m_dwStructTag + m_nMember + m_nBytes * m_nBytesLength)
					[self SetColorByTCOLOR:TCOLOR_MEMBER];
				else
					[self SetColorByTCOLOR:TCOLOR_STRUCT];
			}
			else
				[self SetColorByDefault];
            
			if(m_charset == CTYPE_UNICODE) {
				__uint16_t w[2] = {[BZGlobalFunc SwapWord:(*((__uint16_t*)(p+ofs)))], 0};
                //NSLog(@"bByteOrder: %@", [BZOptions sharedInstance]->bByteOrder ? @"Big":@"Little");
                [self PutStr:[NSString stringWithCString:w encoding:NSUTF16BigEndianStringEncoding] length:2];//PutUnicodeChar(w);
				ofs += 2;
				i++;
			} else {
                /*if (i==0 && skipNum!=0) {
                    i+=skipNum; ofs+=skipNum; [self PutStr:@"" length:skipNum]; skipNum=0;
                    [self PutFlush];
                }*/
				__uint16_t c = *(p + ofs++);
				switch(m_charset) {
                    case CTYPE_ASCII:
                        if(c < 0x20 || c > 0x7E) c = CHAR_NG;
                        [self PutStr:[NSString stringWithFormat:@"%c", (char)c]];
                        break;
                    case CTYPE_SJIS:
                        if(c < 0x20)
                        {
                            [self PutStr:@"."/*CHAR_NG*/];
                        } else if(i == 0 && [BZGlobalFunc IsMBS:p ofs:ofs-1 bTrail:TRUE])
                        {
                            [self PutStr:@" "];
                        } else if ([BZGlobalFunc IsMBS:p ofs:ofs-1 bTrail:FALSE])
                        {
                            __uint8_t c1 = *(p + ofs);
                            if([BZGlobalFunc _ismbclegal:c ch2:c1])//_ismbclegal(MAKEWORD(c1, c)))
                            {
                                //[self PutStr:[NSString stringWithFormat:@"%c", (char)c]];//PutChar((char)c);
                                char outSJIS[3] = {c, *(p + ofs), 0};
                                c = *(p + ofs);
                                if(i < 15) {
                                    ofs++;
                                    i++;
                                }
                                //[self PutStr:[NSString stringWithFormat:@"%c", (char)c]];
                                NSString *sjisStr = [NSString stringWithCString:outSJIS encoding:NSShiftJISStringEncoding];
                                [self PutStr:sjisStr?sjisStr:@".." length:2];
                            } else {
                                c = CHAR_NG;
                                [self PutStr:@"."/*CHAR_NG*/];
                            }
                        } else if((c > 0x7E && c < 0xA1) || c > 0xDF)
                        {
                            [self PutStr:@"."/*CHAR_NG*/];
                        } else {
                            [self PutStr:[NSString stringWithFormat:@"%c", (char)c]];
                        }
                        //[self PutStr:[NSString stringWithFormat:@"%c", (char)c]];
                        break;
                    case CTYPE_JIS:
                    case CTYPE_EUC:
                    //case CTYPE_EPWING:
                        c = [self GetCharCode:c ofs:(ofs-1)];//GetCharCode(c, ofs - 1);
                        if (c < 0x20 || (c > 0x7E && c < 0xA1) || (c > 0xDF && c <= 0xFF))
                        {
                            [self PutStr:@"."/*CHAR_NG*/];
                        } else if (c & 0xFF00) {
                            if (ofs>=dwTotal) {
                                [self PutStr:@"."/*CHAR_NG*/];
                            } else {
                                __uint8_t c1 = *(p + ofs);
                                c = (c | c1);// & 0x7F7F;
                                if (m_charset==CTYPE_JIS) c|=0x8080;
                                char outCStr[3] = {HIBYTE(c), c&0xFF, 0};
                                NSString *eucStr = [NSString stringWithCString:outCStr encoding:NSJapaneseEUCStringEncoding];
                                if (eucStr) {
                                    if(i < 15) {
                                        [self GetCharCode:c1&0x7F ofs:ofs];//GetCharCode(c1, ofs);
                                        ofs++;
                                        i++;
                                    }
                                    [self PutStr:eucStr length:2];
                                } else {//fail convert char
                                    m_mode = CMODE_ASCII;
                                    [self PutStr:@"."/*CHAR_NG*/ length:1];
                                }
                            }
                        } else {
                            [self PutStr:[NSString stringWithFormat:@"%c", (char)c]];
                        }
                        break;
                    case CTYPE_UTF8:	// ### 1.54b
                    {
                        if(c <= 0x7E)
                        {
                            if (0x20 <= c) {
                                [self PutStr:[NSString stringWithFormat:@"%c", (char)c]];
                            } else {
                                [self PutStr:@"." /*CHAR_NG*/];
                            }
                            break;
                        }
                        
                        __uint32_t dw32 = 0;
                        int n = 4;
                        if (dwTotal - ofs + 1 < 4) n = dwTotal - ofs + 1;
                        for (int i=0; i<4; i++) {
                            dw32 <<= 8;
                            if (i<n) dw32 |= *(p+ofs-1+i);
                        }
                        if ((dw32 & 0xE0C00000) == 0xC0800000) {
                            char outUtf8[3] = {c, *(p+ofs), 0};
                            NSString *utf8Str = [NSString stringWithCString:outUtf8 encoding:NSUTF8StringEncoding];
                            if (utf8Str) {
                                [self PutStr:utf8Str length:2];
                                i++; if(i>=16)skipNum = i-16+1; ofs+=1-skipNum;
                            } else {
                                [self PutStr:@"." /*CHAR_NG*/];
                            }
                        } else if ((dw32 & 0xF0C0C000) == 0xE0808000) {
                            char outUtf8[4] = {c, *(p+ofs), *(p+ofs+1), 0};
                            NSString *utf8Str = [NSString stringWithCString:outUtf8 encoding:NSUTF8StringEncoding];
                            if (utf8Str) {
                                [self PutStr:utf8Str length:3];
                                i+=2; if(i>=16)skipNum = i-16+1; ofs+=2-skipNum;
                            } else {
                                [self PutStr:@"." /*CHAR_NG*/];
                            }
                        } else if ((dw32 & 0xF8C0C0C0) == 0xF0808080) {
                            char outUtf8[5] = {c, *(p+ofs), *(p+ofs+1), *(p+ofs+2), 0};
                            NSString *utf8Str = [NSString stringWithCString:outUtf8 encoding:NSUTF8StringEncoding];
                            if (utf8Str) {
                                [self PutStr:utf8Str length:4];
                                i+=3; if(i>=16)skipNum = i-16+1; ofs+=3-skipNum;
                            } else {
                                [self PutStr:@"." /*CHAR_NG*/];
                            }
                        } else {
                            [self PutStr:@"." /*CHAR_NG*/];
                        }
                    }
                        break;
                    /*case CTYPE_EBCDIC:	// ### 1.63
                        void* m_pEbcDic = [[[BZEbcDic sharedInstance] m_EbcDic] data];
                        if(m_pEbcDic == NULL || c < EBCDIC_BASE)
                            c = CHAR_NG;
                        else
                            c = *(m_pEbcDic + ((BYTE)c - EBCDIC_BASE));
                        [self PutStr:[NSString stringWithFormat:@"%c", (char)c]];
                        break;*/
				}
				//[self PutStr:[NSString stringWithFormat:@"%c", (char)c]];//PutChar((char)c);
			}
		}//loop-end for string area
		[self SetColorByDefault];//SetColor();
        //		if(m_charset == CTYPE_SJIS)[self PutStr:@" "];//PutChar(' ');
	}
	if([self IsToFile]) {
		[self PutEnd];
	} else {
		if(y <= rClip.y2) {
			[self SetColorByDefault];//SetColor();
			for(; y<=rClip.y2+1; y++) {
				[self Locate:0 y:y];
                for(int j=0;j<VIEWCOLUMNS;j++)[self PutStr:@" "];//PutChar(' ', VIEWCOLUMNS);
			}
		}
		[self PutEnd];
		//DrawDummyCaret(pDC);
		[self DrawCaret];//DrawCaret();
	}
    
	/* Grid表示 */
/*	if(!IsToFile() && options.iGrid==1) {
		DrawGrid(pDC, rClip);
	}*/
    
    [super drawRect:dirtyRect];
}

- (BZView *)GetBrotherView:(BZView *)view
{
    BZWindow *mainWnd = (BZWindow*)[self window];
    return [mainWnd GetBrotherView:view];
}

- (void)InitCharMode:(__uint8_t *)pTop ofs:(__uint64_t)ofs
{
	if(!pTop) return;
	__uint64_t n = ofs;
	__uint8_t* p = pTop + n;
	if(m_charset == CTYPE_JIS) {
		[self GetCharCode:0x0F];
		while(n) {
			n--;
			__uint8_t c = *(--p);
			if(c < 0x21 || c > 0x7E) break;
		}
		while(n++ < ofs) {
			[self GetCharCode:*p++];
		}
	} else if(m_charset == CTYPE_EUC) {
		[self GetCharCode:0];
		while(n) {
			n--;
			__uint8_t c = *(--p);
			if(c < 0xA1 || c > 0xFE) break;
		}
		while(n++ < ofs) {
			[self GetCharCode:*p++];
		}
	}
}

- (__uint16_t)GetCharCode:(__uint16_t)c
{
    return [self GetCharCode:c ofs:0];
}

- (__uint16_t)GetCharCode:(__uint16_t)c ofs:(__uint64_t)ofs
{
//	enum CharMode { CMODE_ASCII, CMODE_ESC, CMODE_ESC1, CMODE_ESC2, CMODE_ESC3, CMODE_KANJI1, CMODE_KANJI2, CMODE_KANA };
	//static enum CharMode mode;
	//static __uint16_t c0;
	if(m_charset == CTYPE_JIS) {
		switch(c) {
            case 0x1B:
                m_mode = CMODE_ESC;
                break;
            case 0x0E:
                m_mode = CMODE_KANA;
                break;
            case 0x0F:
                m_mode = CMODE_ASCII;
                break;
		}
		if(c < 0x21 || c > 0x7E || m_mode == CMODE_ASCII) return c;
		switch(m_mode) {
            case CMODE_ESC:
                if(c == '(') m_mode = CMODE_ESC1;
                else if(c == '$') m_mode = CMODE_ESC2;
                else m_mode = CMODE_ASCII;
                break;
            case CMODE_ESC1:
                if(c == 'I') m_mode = CMODE_KANA;
                else m_mode = CMODE_ASCII;
                break;
            case CMODE_ESC2:
                m_mode = CMODE_KANJI1;
                break;
            case CMODE_KANJI1:
                c <<= 8;
                m_c0 = c;
                m_mode = CMODE_KANJI2;
                //c |= 0x8000; //tamachan for MacOSX
                break;
            case CMODE_KANJI2:
                c = ' ';
                m_mode = CMODE_KANJI1;
                //c |= 0x80; //tamachan for MacOSX
                break;
            case CMODE_KANA:
                c |= 0x80;
                break;
		}
	} else if(m_charset == CTYPE_EUC) {
		if(c < 0x80) {
			m_mode = CMODE_ASCII;
		} else {
			switch(m_mode) {
                case CMODE_ASCII:
                    if(c >= 0xA1 && c <= 0xFE) {
                        m_mode = CMODE_KANJI1;
                        c <<= 8;
                    } else if(c == 0x8E) {
                        m_mode = CMODE_KANA;
                        c = CHAR_NG;
                    } else if(c > 0x7E)
                        c = CHAR_NG;
                    break;
                case CMODE_KANJI1:
                    c = ' ';
                    m_mode = CMODE_ASCII;
                    break;
                case CMODE_KANA:
                    m_mode = CMODE_ASCII;
                    break;
			}
		}
	} else if(m_charset == CTYPE_EPWING) {
		if(c == 0x1F) m_mode = CMODE_ESC3;
		else if(m_mode == CMODE_ESC3) {
			m_mode = CMODE_KANJI1;
			c = CHAR_NG;
		} else if(c < 0x21 || c > 0x7E) {
			m_mode = CMODE_KANJI1;
			c = CHAR_NG;
		} else if(m_mode == CMODE_KANJI1) {
			if(ofs == 0 || (ofs & 1) == 0) {
				c <<= 8;
				m_c0 = c;
				m_mode = CMODE_KANJI2;
			} else
				c = CHAR_NG;
		} else if(CMODE_KANJI2) {
			c = ' ';
			m_mode = CMODE_KANJI1;
		}
	}
	return c;
}

-(void)UpdateDocSize
{
    m_dwTotal = [m_pDoc GetDocSize];
    TAMASize cTotal;
    cTotal.width = VIEWCOLUMNS;
    cTotal.height = m_dwTotal / 16 + 2;
    [self SetTextSize:cTotal];
    [self InitScrollBar];//settextsize:ctotal nPage kara ido
    //Invalidate();
}

- (enum CharSet)AutoDetectCharSet
{
    BZOptions *bzopt = [BZOptions sharedInstance];
    __uint8_t *p = [m_pDoc GetDocPtr];
    if(!p) return bzopt->charset;
    __uint64_t dwSize = m_dwTotal;
    if (dwSize > bzopt->dwDetectMax) dwSize = bzopt->dwDetectMax;
    enum CharSet charset = [BZGlobalFunc DetectCodeType:p pEnd:p+dwSize];
    if (charset == CTYPE_BINARY) {
        charset = bzopt->charset;
    }
    return charset;
}

- (void)mouseDown:(NSEvent *)theEvent
//- (void)rightMouseDown:(NSEvent *)theEvent
{
    //NSLog(@"mouseDown");
    NSPoint pt = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    [self OnLButtonDown:pt bShiftKey:[theEvent modifierFlags] & NSShiftKeyMask];
}

- (void)mouseDragged:(NSEvent *)theEvent
//- (void)rightMouseDragged:(NSEvent *)theEvent
{
    //NSLog(@"mouseDragged");
    NSPoint pt = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    [self OnMouseDrag:pt];
    [self setNeedsDisplay:YES];
    
}

-(void)mouseUp:(NSEvent *)theEvent
//- (void)rightMouseUp:(NSEvent *)theEvent
{
    //NSLog(@"mouseUp");
    [self OnLButtonUp];
    [self setNeedsDisplay:YES];
    
    BZWindow *bzWnd = (BZWindow*)self.window;
    if (bzWnd->m_isActiveView1 != m_isDoc1?TRUE:FALSE) {
        bzWnd->m_isActiveView1 = m_isDoc1?TRUE:FALSE;
        BZView *bzBrother = [bzWnd GetBrotherView:self];
        [bzBrother StopCaret];
        [self InitCaret];
        [bzBrother HideCaret2];
        Document *doc = [self GetDocument];
        [bzWnd setTitle:(doc.fileURL)?doc.fileURL.lastPathComponent : @"Untitled"];
    }
}

/*
 -(void)mouseMoved:(NSEvent *)theEvent
 {
 NSLog(@"mouseMoved");
 //   [self OnMouseMove];
 }*/

- (__uint64_t)PointToOffset:(NSPoint)pt
{
	//NSRect r;
	//GetClientRect(&r);
	if(!NSPointInRect(pt, _bounds))//if(!r.PtInRect(pt))
		return ULLONG_MAX;//UINT_MAX;
	[self PixelToGridFromNSPoint:&pt];//PixelToGrid(pt);
    //NSLog(@"PointToIffset: PixelToGridFromNSPoint:%f,%f",pt.x,pt.y);
    TAMASize scrPos = [self GetScrollPos];
    pt.x += scrPos.x; pt.y += scrPos.y;
	//pt += [self GetScrollPos];
	if(pt.y < DUMP_Y) // || pt.x == CHAR_X-1 || pt.x >= CHAR_X+16)	// ###1.5
 		return ULLONG_MAX;//UINT_MAX;
	if(pt.x >= CHAR_X) {
		if((pt.x -= CHAR_X) > 16) pt.x = 16;
		m_bCaretOnChar = TRUE;
	} else {
		if(pt.x < DUMP_X)
			pt.x = 0;
		else
			pt.x -= DUMP_X;
		pt.x/=3;
		m_bCaretOnChar = FALSE;
	}
	__uint64_t ofs = (pt.y - DUMP_Y)*16 + pt.x;
	return (ofs > m_dwTotal) ? m_dwTotal : ofs;
}

-(void)OnLButtonDown:(NSPoint)ptClick bShiftKey:(BOOL)bShiftKey
{
	if(m_bBlock) {
		m_bBlock = FALSE;
        NSLog(@"m_bBlock==FALSE");
		//Invalidate(FALSE);
	}
	BOOL bOnChar = m_bCaretOnChar;
	__uint64_t ofs = [self PointToOffset:ptClick];//PointToOffset(point);
    //NSLog(@"OnLButtonDown %f,%f ofs=%llu",ptClick.x, ptClick.y, ofs);
	if(ofs != ULLONG_MAX/*UINT_MAX*/) {
		if(m_dwCaret != ofs || bOnChar != m_bCaretOnChar) {
			m_dwOldCaret = m_dwCaret;
			if(bShiftKey) {
				m_bBlock = TRUE;
                NSLog(@"m_bBlock==TRUE");
				//Invalidate(FALSE);
			} else
				m_dwBlock = ofs;
            [self setNeedsDisplayInRect:m_caretRect];m_bCaretOn = FALSE;
			m_dwCaret = ofs;
			[self DrawCaret];//DrawCaret();
		}
		//SetCapture();
	}
	m_bEnterVal = FALSE;
	//CTextView::OnLButtonDown(nFlags, point);
}

-(void)OnMouseDrag:(NSPoint)pt
{
    //	if(nFlags & MK_LBUTTON) {
    __uint64_t ofs = [self PointToOffset:pt];//PointToOffset(point);
    if(ofs != ULLONG_MAX) {//UINT_MAX) {
        if(m_draggingTimer) {
            [m_draggingTimer invalidate];//KillTimer(m_timer);
            m_draggingTimer = nil;
        }
        if(m_dwCaret != ofs && m_dwBlock != ofs) {
            __uint64_t dwCaretOld = m_dwCaret;		// ###1.5
            CGPoint ptCaretOld = m_caretRect.origin;
            [self setNeedsDisplayInRect:m_caretRect];m_bCaretOn = FALSE;
            m_dwCaret = ofs;
            m_bBlock = TRUE;
            NSLog(@"m_bBlock==TRUE");
            if([self DrawCaret])
            {
                //Invalidate(FALSE);
            } else {
                m_dwCaret = dwCaretOld;
                m_caretRect.origin = ptCaretOld;
            }
        }
    } else {
        //RECT r;
        //GetClientRect(&r);
        if((pt.y < 0 || pt.y > _bounds.size.height + _bounds.origin.y) && !m_draggingTimer)
            m_draggingTimer = [NSTimer scheduledTimerWithTimeInterval:0.05 target:self selector:pt.y < 0 ? @selector(OnTimerUp:):@selector(OnTimerDown:) userInfo:nil repeats:YES];//SetTimer(pt.y < 0 ? TIMER_UP : TIMER_DOWN, 50, NULL);
    }
//	}
	//CTextView::OnMouseMove(nFlags, point);
}

- (void)OnTimerUp:(NSTimer *)timer
{
    [self keyDownWithUnichar:NSUpArrowFunctionKey bCtrl:FALSE bShift:TRUE];
    [self OnCharWithUnichar:NSUpArrowFunctionKey];
}
-(void)OnTimerDown:(NSTimer *)timer
{
    [self keyDownWithUnichar:NSDownArrowFunctionKey bCtrl:FALSE bShift:TRUE];
    [self OnCharWithUnichar:NSDownArrowFunctionKey];
}

- (void)OnLButtonUp
{
	//ReleaseCapture();
    if(m_draggingTimer) {
        [m_draggingTimer invalidate];//KillTimer(m_timer);
        m_draggingTimer = nil;
    }
	//CTextView::OnLButtonUp(nFlags, point);
}

- (BOOL)acceptsFirstResponder {return YES;}

- (void)keyDown:(NSEvent *)theEvent
{
    if ([[theEvent characters] length] == 0) return;
    unichar key = [[theEvent charactersIgnoringModifiers] characterAtIndex:0];
	BOOL bCtrl  = [theEvent modifierFlags] & NSControlKeyMask;
	BOOL bShift = ([theEvent modifierFlags] & NSShiftKeyMask );//|| GetKeyState(VK_LBUTTON) < 0);
    [self keyDownWithUnichar:key bCtrl:bCtrl bShift:bShift];
    [self OnCharWithUnichar:key];
}

- (void)keyDownWithUnichar:(unichar)key bCtrl:(BOOL)bCtrl bShift:(BOOL)bShift
{
    //NSLog(@"keyDownFromUnichar");
	__uint64_t dwNewCaret = m_dwCaret;
    
    switch (key) {
        case NSRightArrowFunctionKey:
            if(dwNewCaret<m_dwTotal)dwNewCaret++;
            break;
        case NSLeftArrowFunctionKey:
            if(dwNewCaret>0)dwNewCaret--;
            break;
        case NSDownArrowFunctionKey:
            dwNewCaret += 16;
            if(dwNewCaret < m_dwCaret) dwNewCaret = m_dwCaret;
            break;
        case NSUpArrowFunctionKey:
            if(dwNewCaret>=16)dwNewCaret -= 16;
            break;
            
        default:
            //[super keyDown:theEvent];
            return;
    }
	m_bEnterVal = FALSE;
	if(!m_bBlock && bShift) {
		m_dwBlock = m_dwCaret;
		m_bBlock = TRUE;
        NSLog(@"m_bBlock==TRUE");
	} else if(m_bBlock && !bShift) {
        NSLog(@"m_bBlock==FALSE");
		m_bBlock = FALSE;
		//Invalidate(FALSE);
	}
	if(m_caretRect.origin.x == -1) {
		if(dwNewCaret <= m_dwTotal) {	// ### 1.62
			m_dwCaret = dwNewCaret;
			[self GotoCaret];//GotoCaret();
		}
	} else
		[self MoveCaretTo:dwNewCaret];//MoveCaretTo(dwNewCaret);
    
    //[self setNeedsDisplay:YES];
	return;
Error:
	//MessageBeep(MB_NOFOCUS);
	return;
}


-(void)OnCharWithUnichar:(unichar)nChar
{
	static unichar preChar = 0;
	__uint8_t* p;
    
	if (nChar < ' ' || nChar >= 256)
		return;
    NSString *strInput = [NSString stringWithFormat: @"%C", nChar];
    
	if (m_pDoc->m_bReadOnly)
		goto Error;
    
	if (!m_bEnterVal && !preChar)
    {
		__uint64_t dwSize = 1;
        /*if(m_bCaretOnChar && (m_charset == CTYPE_UNICODE || (m_charset > CTYPE_UNICODE) && _ismbblead((BYTE)nChar))) {
			if(m_charset == CTYPE_UTF8)		// ### 1.54b
				dwSize = 3;
			else
				dwSize = 2;
		}
		if(m_bIns || (m_dwCaret == m_dwTotal)) {
			if([m_pDoc IsFileMapping]) goto Error;
			m_pDoc->StoreUndo(m_dwCaret, dwSize, UNDO_DEL);
			m_pDoc->InsertData(m_dwCaret, dwSize, TRUE);
			[self UpdateDocSize];//UpdateDocSize();
		} else {
			m_pDoc->StoreUndo(m_dwCaret, dwSize, UNDO_OVR);
        }*/
	}
	m_bBlock = FALSE;
	p  = [m_pDoc GetDocPtr] + m_dwCaret;
	if(!m_bCaretOnChar) {
		if(nChar >= '0' && nChar <= '9')
			nChar -= '0';
		else if(nChar >= 'A' && nChar <= 'F')
			nChar -= 'A' - 10;
		else if(nChar >= 'a' && nChar <= 'f')
			nChar -= 'a' - 10;
		else
			goto Error;
		if(m_bEnterVal) {
			__uint8_t nVal = *p;
			nChar |= nVal<<4;
			m_bEnterVal = FALSE;
		} else
			m_bEnterVal = TRUE;
	} else if(m_charset >= CTYPE_UNICODE) {
		char  mbs[4];
		char* pb = mbs;
		if(preChar) {
			*pb++ = preChar;
			preChar = 0;
		} /*else if(_ismbblead((BYTE)nChar)) {
			preChar = nChar;
			return;
		}*/
		*pb++ = (char)nChar;
		*pb++ = 0;
		__uint8_t* buffer = NULL;
	/*	int len = ConvertCharSet(m_charset, mbs, buffer);
		if(len) {
			if(m_charset == CTYPE_UNICODE) len *= 2;
			pb = (char*)buffer;
			for (int i = 0; i<len; i++) *p++ = *pb++;
			free(buffer);
			//Invalidate(FALSE);
			if(!m_bEnterVal)
				MoveCaretTo(m_dwCaret + len);
		}*/
		return;
	}
	*p = (__uint8_t)nChar;
	[self setNeedsDisplay:YES];//Invalidate(FALSE);
	if(!m_bEnterVal) {
		[self MoveCaretTo:m_dwCaret+1];
	}
	return;
Error:
    NSBeep();
	//MessageBeep(MB_NOFOCUS);
	return;
}

- (BOOL)GotoCaret
{
	if([self DrawCaret])//DrawCaret())
    {
		//if(m_bBlock) Invalidate(FALSE);
		return TRUE;
	}
	//POINT pt;
	//pt.x = 0;
	//pt.y = m_dwCaret/16 - 1;
	[self ScrollToPos:m_dwCaret/16-1];//ScrollToPos(pt);
	//Invalidate(FALSE);
	return FALSE;
}

-(void)MoveCaretTo:(__uint64_t)dwNewCaret
{
	if(dwNewCaret > m_dwTotal) {
		dwNewCaret = m_dwTotal;
	}
    
    [self setNeedsDisplayInRect:m_caretRect];m_bCaretOn = FALSE;
	int dy = dwNewCaret/16 - m_dwCaret/16;
	m_dwCaret = dwNewCaret;
	if(![self DrawCaret])//DrawCaret())
    {
		[self ScrollBy:0 dy:dy bScrl:!m_bBlock];//ScrollBy(0, dy, !m_bBlock);
	}
	//if(m_bBlock)Invalidate(FALSE);
}

/*- (void)openDocument:(id)sender
{
    
    NSDocumentController* dc = [NSDocumentController sharedDocumentController];
    [dc newDocument:sender];
    [dc openDocument:sender];
}*/

//- (BOOL)validateUserInterfaceItem:(id <NSValidatedUserInterfaceItem>)anItem
- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    NSLog(@"BZView::validateMenuItem");
    SEL theAction = [menuItem action];
    BZOptions *bzopt = [BZOptions sharedInstance];
    
    if (theAction == @selector(OnCharmodeAscii:)) {
        [menuItem setState:(m_charset==CTYPE_ASCII)? NSOnState:NSOffState];
    } else if (theAction == @selector(OnCharmodeSJIS:)) {
        [menuItem setState:(m_charset==CTYPE_SJIS)? NSOnState:NSOffState];
    } else if (theAction == @selector(OnCharmodeUTF16:)) {
        [menuItem setState:(m_charset==CTYPE_UNICODE)? NSOnState:NSOffState];
    } else if (theAction == @selector(OnCharmodeUTF8:)) {
        [menuItem setState:(m_charset==CTYPE_UTF8)? NSOnState:NSOffState];
    } else if (theAction == @selector(OnCharmodeJIS:)) {
        [menuItem setState:(m_charset==CTYPE_JIS)? NSOnState:NSOffState];
    } else if (theAction == @selector(OnCharmodeEUC:)) {
        [menuItem setState:(m_charset==CTYPE_EUC)? NSOnState:NSOffState];
    } else if (theAction == @selector(OnCharmodeEBCDIC:)) {
        [menuItem setState:(m_charset==CTYPE_EBCDIC)? NSOnState:NSOffState];
    } else if (theAction == @selector(OnCharmodeEPWING:)) {
        [menuItem setState:(m_charset==CTYPE_EPWING)? NSOnState:NSOffState];
    } else if (theAction == @selector(OnCharmodeAutoDetect:)) {
        [menuItem setState:(bzopt->bAutoDetect)? NSOnState:NSOffState];
    } else if (theAction == @selector(OnByteOrderIntel:)) {
        [menuItem setState:!(bzopt->bByteOrder)? NSOnState:NSOffState];
    } else if (theAction == @selector(OnByteOrderMotorola:)) {
        [menuItem setState:(bzopt->bByteOrder)? NSOnState:NSOffState];
    }
    return YES;
}

- (IBAction)OnCharmodeAscii:(id)sender
{
    BZOptions *bzopt = [BZOptions sharedInstance];
	m_charset = bzopt->charset = CTYPE_ASCII;
//	options.Touch();
    [self setNeedsDisplay:YES];//Invalidate(TRUE);
	BZView* pView = [self GetBrotherView:self];
	if(pView) {
		pView->m_charset = m_charset;
        [pView setNeedsDisplay:YES];//pView->Invalidate(TRUE);
	}
}
- (IBAction)OnCharmodeSJIS:(id)sender
{
    BZOptions *bzopt = [BZOptions sharedInstance];
	m_charset = bzopt->charset = CTYPE_SJIS;
    //	options.Touch();
    [self setNeedsDisplay:YES];//Invalidate(TRUE);
	BZView* pView = [self GetBrotherView:self];
	if(pView) {
		pView->m_charset = m_charset;
        [pView setNeedsDisplay:YES];//pView->Invalidate(TRUE);
	}
}
- (IBAction)OnCharmodeUTF16:(id)sender
{
    BZOptions *bzopt = [BZOptions sharedInstance];
	m_charset = bzopt->charset = CTYPE_UNICODE;
    //	options.Touch();
    [self setNeedsDisplay:YES];//Invalidate(TRUE);
	BZView* pView = [self GetBrotherView:self];
	if(pView) {
		pView->m_charset = m_charset;
        [pView setNeedsDisplay:YES];//pView->Invalidate(TRUE);
	}
}
- (IBAction)OnCharmodeUTF8:(id)sender
{
    BZOptions *bzopt = [BZOptions sharedInstance];
	m_charset = bzopt->charset = CTYPE_UTF8;
    //	options.Touch();
    [self setNeedsDisplay:YES];//Invalidate(TRUE);
	BZView* pView = [self GetBrotherView:self];
	if(pView) {
		pView->m_charset = m_charset;
        [pView setNeedsDisplay:YES];//pView->Invalidate(TRUE);
	}
}
- (IBAction)OnCharmodeJIS:(id)sender
{
    BZOptions *bzopt = [BZOptions sharedInstance];
	m_charset = bzopt->charset = CTYPE_JIS;
    //	options.Touch();
    [self setNeedsDisplay:YES];//Invalidate(TRUE);
	BZView* pView = [self GetBrotherView:self];
	if(pView) {
		pView->m_charset = m_charset;
        [pView setNeedsDisplay:YES];//pView->Invalidate(TRUE);
	}
}
- (IBAction)OnCharmodeEUC:(id)sender
{
    BZOptions *bzopt = [BZOptions sharedInstance];
	m_charset = bzopt->charset = CTYPE_EUC;
    //	options.Touch();
    [self setNeedsDisplay:YES];//Invalidate(TRUE);
	BZView* pView = [self GetBrotherView:self];
	if(pView) {
		pView->m_charset = m_charset;
        [pView setNeedsDisplay:YES];//pView->Invalidate(TRUE);
	}
}
- (IBAction)OnCharmodeEBCDIC:(id)sender
{
    BZOptions *bzopt = [BZOptions sharedInstance];
	m_charset = bzopt->charset = CTYPE_EBCDIC;
    [[BZEbcDic sharedInstance] LoadEbcDicTable];//LoadEbcDicTable();
    //	options.Touch();
    [self setNeedsDisplay:YES];//Invalidate(TRUE);
	BZView* pView = [self GetBrotherView:self];
	if(pView) {
		pView->m_charset = m_charset;
        [pView setNeedsDisplay:YES];//pView->Invalidate(TRUE);
	}
}
- (IBAction)OnCharmodeEPWING:(id)sender
{
    BZOptions *bzopt = [BZOptions sharedInstance];
	m_charset = bzopt->charset = CTYPE_EPWING;
    //	options.Touch();
    [self setNeedsDisplay:YES];//Invalidate(TRUE);
	BZView* pView = [self GetBrotherView:self];
	if(pView) {
		pView->m_charset = m_charset;
        [pView setNeedsDisplay:YES];//pView->Invalidate(TRUE);
	}
}
-(void)OnCharmodeAutoDetect:(id)sender
{
    BZOptions *bzopt = [BZOptions sharedInstance];
    bzopt->bAutoDetect = !bzopt->bAutoDetect;
}

-(void)OnByteOrderIntel:(id)sender
{
    BZOptions *bzopt = [BZOptions sharedInstance];
    bzopt->bByteOrder = FALSE;
    //GetMainFrame()->UpdateInspectViewChecks();
}
-(void)OnByteOrderMotorola:(id)sender
{
    BZOptions *bzopt = [BZOptions sharedInstance];
    bzopt->bByteOrder = TRUE;
    //GetMainFrame()->UpdateInspectViewChecks();
}




-(void)FindNext:(NSComboBox*)findBox
{
    NSString *sFind = findBox.stringValue;
    
	if([sFind isEqualToString:@""])//sFind.IsEmpty())
    {
        //		if(GetBrotherView())
        //			OnJumpCompare();
		return;
	}
    
	__uint8_t *pData = [m_pDoc QueryMapView:[m_pDoc GetDocPtr] dwOffset:m_dwCaret + 1];
	__uint64_t dwFind = [m_pDoc GetMapSize];
    
	__uint8_t *p = pData + m_dwCaret + 1;
	/*int*/__uint64_t len = dwFind - m_dwCaret;
    
	int c1 = [sFind cStringUsingEncoding:NSJapaneseEUCStringEncoding][0];//[sFind substringToIndex:1];
	int c2 = 0;
	if(c1=='=') {
        [findBox setStringValue:@"? "];
		return;
	}
	[findBox addItemWithObjectValue:sFind];//pCombo->AddText(sFind);
    
	if(c1 == '?' || c1 == '+' || c1 == '>'|| c1 == '<')
    {
		__uint64_t dwNew = 0;
		long nResult;
		if([BZGlobalFunc CalcHexa:[sFind cStringUsingEncoding:NSJapaneseEUCStringEncoding]+1 retVal:&nResult])//CalcHexa((LPCSTR)sFind + 1, nResult))
        {
			switch(c1)
            {
                case '?':
                {
                    //CString sResult;
                    //sResult.Format("= 0x%X (%d)", nResult, nResult);
                    [findBox setStringValue:[NSString stringWithFormat:@"= 0x%lX (%ld)", nResult, nResult]];//pCombo->SetText(sResult);
                    return;
                }
                case '+':
                    dwNew = m_dwCaret + m_pDoc->m_dwBase;
                case '>':
                    dwNew += nResult - m_pDoc->m_dwBase;	// ###1.63
                    if(dwNew <= m_dwTotal) {
                        m_dwOldCaret = m_dwCaret;
                        m_dwCaret = dwNew;
                        m_bCaretOnChar = FALSE;
                        [self GotoCaret];//GotoCaret();
                        [self SetFocus];//SetFocus();
                    } else {
                        NSAlert *al = [[NSAlert alloc] init];
                        [al setMessageText:NSLocalizedString(@"Out of Range", nil)];
                        [al runModal];
                        //AfxMessageBox(IDS_ERR_RANGE);
                    }
                    return;
                case '<':
                    //if(m_pDoc->m_bReadOnly)
                    //    AfxMessageBox(IDS_ERR_READONLY);
                    //else {
                    //    if(m_bBlock)
                    //        FillValue(nResult);
                    //    else
                    //        SetValue(m_dwCaret, m_nBytes, nResult);
                    //}
                    [self SetFocus];//SetFocus();
                    return;
			}
		} else {
            //CString sMsg;
            //sMsg.Format(IDS_ERR_SYNTAX, sExp);
            //AfxMessageBox(sMsg);
            NSAlert *al = [[NSAlert alloc] init];
            [al setMessageText:NSLocalizedString(@"Syntax error", nil)];
            [al runModal];
            return;
        }
		return;
	}
	if(len < 2) return;
	len--;
	int nFind = 0;
	__uint8_t *pFind = NULL;
    char *pTmp = NULL;
	enum CharSet charset = m_charset;
	if(c1 == '#')
    {
		nFind = [BZGlobalFunc ReadHexa:[sFind cStringUsingEncoding:NSJapaneseEUCStringEncoding] buffer:&pFind];//ReadHexa(sFind, pFind);
		if(!nFind) return;
		c1 = *pFind;
		charset = CTYPE_BINARY;
	} else {
        __uint8_t *pTmp2 = NULL;
        switch (charset) {
            case CTYPE_ASCII:
                pTmp = [sFind cStringUsingEncoding:NSASCIIStringEncoding];
                nFind = strlen(pTmp);
                c1 = pTmp[0];
				if(islower(c1)) c2 = _toupper(c1);
				else if(isupper(c1)) c2 = _tolower(c1);
                break;
            case CTYPE_SJIS:
                pTmp = [sFind cStringUsingEncoding:NSShiftJISStringEncoding];
                nFind = strlen(pTmp);
                c1 = pTmp[0];
                break;
            case CTYPE_UNICODE:
                if ([BZOptions sharedInstance]->bByteOrder)
                    pTmp = [sFind cStringUsingEncoding:NSUTF16BigEndianStringEncoding];
                else
                    pTmp = [sFind cStringUsingEncoding:NSUTF16LittleEndianStringEncoding];
                nFind = strlen(pTmp);
                c1 = pTmp[0];
                break;
            case CTYPE_JIS:
                pTmp2 = (__uint8_t *)[sFind cStringUsingEncoding:NSISO2022JPStringEncoding];
                nFind = strlen(pTmp2);
                if (nFind>=3)
                {
                    __uint32_t dw3 = ((__uint32_t)(pTmp2[0])) << 16 | ((__uint32_t)(pTmp2[1])) << 8 | ((__uint32_t)(pTmp2[2]));
                    if (dw3 == 0x001B2442 || dw3 == 0x001B2842) {
                        pTmp2 += 3;
                        nFind -= 3;
                    }
                }
                if (nFind>=3)
                {
                    __uint32_t dw3 = ((__uint32_t)(pTmp2[nFind-3])) << 16 | ((__uint32_t)(pTmp2[nFind-2])) << 8 | ((__uint32_t)(pTmp2[nFind-1]));
                    if (dw3 == 0x001B2442 || dw3 == 0x001B2842) {
                        nFind -= 3;
                    }
                }
                pTmp = (char *)pTmp2;
                if(nFind>0) c1 = pTmp[0];
                break;
            case CTYPE_EUC:
                pTmp = [sFind cStringUsingEncoding:NSJapaneseEUCStringEncoding];
                nFind = strlen(pTmp);
                c1 = pTmp[0];
                break;
            case CTYPE_UTF8:
                pTmp = [sFind cStringUsingEncoding:NSUTF8StringEncoding];
                nFind = strlen(pTmp);
                c1 = pTmp[0];
                break;
            case CTYPE_EBCDIC:
            case CTYPE_EPWING:
            case CTYPE_COUNT:
            default:
                return;
        }
        if(!c1)return;
	}
    
//	CWaitCursor wait;	// ###1.61
	for(;;) {
		__uint8_t *p1 = NULL;
		__uint8_t *p2 = NULL;
		if(len >= nFind) {
			if(charset == CTYPE_UNICODE) {
				p1 = (__uint8_t*)[BZGlobalFunc MemScanWord:(__uint16_t*)p c:c1 bytes:len];//MemScanWord(p, c1, len);
				if(c2) p2 = (__uint8_t*)[BZGlobalFunc MemScanWord:(__uint16_t*)p c:c2 bytes:len];//(LPBYTE)MemScanWord(p, c2, len);
			} else {
				p1 = [BZGlobalFunc MemScanByte:p c:c1 bytes:len];//MemScanByte(p, c1, len);
				if(c2) p2 = [BZGlobalFunc MemScanByte:p c:c2 bytes:len];//MemScanByte(p, c2, len);
			}
		}
		if(p1 || p2) {
			if(!p1 || (p2 && p2 < p1)) p1 = p2;
			int r = -1;
			switch(charset) {
                case CTYPE_BINARY:
                    r = memcmp(p1, pFind, nFind);
                    break;
                case CTYPE_SJIS:
                case CTYPE_JIS:
                case CTYPE_EUC:
                case CTYPE_UNICODE:
                case CTYPE_UTF8:
                    r = memcmp(p1, pTmp, nFind);
                    break;
                case CTYPE_ASCII:
                    r = strncasecmp((const char*)p1, (const char*)pTmp, nFind);//r = _strnicmp((LPCSTR)p1, sFind, nFind);
                    break;
			}
			if(!r) {
				m_dwOldCaret = m_dwCaret;
				m_dwCaret = p1 - pData;
				m_bCaretOnChar = !pFind;
				[self GotoCaret];
				[self SetFocus];//SetFocus();
				break;
			}
			p1++;
			if(charset == CTYPE_UNICODE)
				p1++;
			len -= p1 - p;
			p = p1;
		}
		else if(dwFind < m_dwTotal) {
			__uint64_t dwFind0 = dwFind;
			pData = [m_pDoc QueryMapView:pData dwOffset:dwFind];//pData = m_pDoc->QueryMapView(pData, dwFind);
			p = pData + dwFind;
			dwFind = [m_pDoc GetMapSize];//m_pDoc->GetMapSize();
			len = dwFind - dwFind0;
		}
		else {
            NSAlert *al = [[NSAlert alloc] init];
            [al setMessageText:NSLocalizedString(@"Cannot find the string/bytes", nil)];
            [al runModal];
			//AfxMessageBox(IDS_ERR_FINDSTRING);
			//pCombo->SetFocus();
			break;
		}
	}
	if(pFind) free(pFind);
}

- (void)SetFocus
{
    [self.window makeFirstResponder:self];
}

- (IBAction)OnUndo:(id)sender
{
	m_dwCaret = [m_pDoc DoUndo];
	[self GotoCaret];
	[self UpdateDocSize];
}


@end
