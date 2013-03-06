//
//  BZWindow.m
//  TamaBinary
//
//  Created by ad on 13/01/04.
//  Copyright (c) 2013年 tamachanbinary. All rights reserved.
//

#import "BZWindow.h"

@implementation BZWindow

- (id)init
{
    self = [super init];
    if (self) {
        // Add your subclass-specific initialization here.
        m_bBmpView = m_bStructView = m_bInspectView = m_bAnalyzerView = FALSE;
        m_nSplitView = m_nSplitView0 = SPLIT_NONE;
        m_isActiveView1 = TRUE;
    }
    return self;
}
/*
- (BOOL)acceptsFirstResponder
{
    return YES;
}
 */

- (void)OnCreateClient
{
    BZOptions *bzopt = [BZOptions sharedInstance];
    m_bStructView = bzopt->bStructView;
    m_bInspectView = bzopt->bInspectView;
    m_bAnalyzerView = bzopt->bAnalyzerView;
    [self CreateClient];
}

- (BOOL)CreateClient
{
    [self RemoveAllBZViews];
    
    m_isActiveView1 = true;
    
    BOOL bSubView = (m_bBmpView || m_bStructView || m_bInspectView || m_bAnalyzerView);
    //BZOptions *bzopt = [BZOptions sharedInstance];
    
    m_splitDoc1 = [[NSSplitView alloc] initWithFrame:[self.contentView bounds]];
    [m_splitDoc1 setVertical:YES];
    [m_splitDoc1 setAutoresizingMask:NSViewMinXMargin | NSViewWidthSizable | NSViewMaxXMargin | NSViewMinYMargin | NSViewHeightSizable | NSViewMaxYMargin];
    [m_splitDoc1 setAutoresizesSubviews:YES];
    
    if (m_bBmpView) {
        m_subViewController1 = [[BZBmpViewController alloc] initWithNibName:@"BZBmpViewController" bundle:[NSBundle mainBundle]];
        [m_subViewController1.view setFrame:NSMakeRect(0, 0, 100, 400)];
        [m_splitDoc1 addSubview:m_subViewController1.view];
    }
    if(m_bzViewController1==nil)
    {
        m_bzViewController1 = [[BZViewController alloc] initWithNibName:@"BZViewController" bundle:[NSBundle mainBundle]];
        [m_splitDoc1/*self.contentView*/ addSubview:m_bzViewController1.view];
        ((BZView*)m_bzViewController1.bzview)->m_isDoc1 = true;
    }
    [m_splitDoc1 adjustSubviews];
    
    if (m_nSplitView!=SPLIT_NONE)
    {
        m_splitDoc = [[NSSplitView alloc] initWithFrame:[self.contentView bounds]];
        [m_splitDoc setVertical:m_nSplitView==SPLIT_V];
        [m_splitDoc setAutoresizingMask:NSViewMinXMargin | NSViewWidthSizable | NSViewMaxXMargin | NSViewMinYMargin | NSViewHeightSizable | NSViewMaxYMargin];
        [m_splitDoc setAutoresizesSubviews:YES];
        
        m_splitDoc2 = [[NSSplitView alloc] initWithFrame:[self.contentView bounds]];
        [m_splitDoc2 setVertical:YES];
        [m_splitDoc2 setAutoresizingMask:NSViewMinXMargin | NSViewWidthSizable | NSViewMaxXMargin | NSViewMinYMargin | NSViewHeightSizable | NSViewMaxYMargin];
        [m_splitDoc2 setAutoresizesSubviews:YES];
        
        if (m_bBmpView) {
            m_subViewController2 = [[BZBmpViewController alloc] initWithNibName:@"BZBmpViewController" bundle:[NSBundle mainBundle]];
            [m_subViewController2.view setFrame:NSMakeRect(0, 0, 100, 400)];
            [m_splitDoc2 addSubview:m_subViewController2.view];
        }
        if(m_bzViewController2==nil)
        {
            m_bzViewController2 = [[BZViewController alloc] initWithNibName:@"BZViewController" bundle:[NSBundle mainBundle]];
            [m_splitDoc2/*self.contentView*/ addSubview:m_bzViewController2.view];
            ((BZView*)m_bzViewController2.bzview)->m_isDoc1 = false;
        }
        [m_splitDoc2 adjustSubviews];
        
        
        [self.contentView addSubview:m_splitDoc];
        [m_splitDoc addSubview:m_splitDoc1];
        [m_splitDoc addSubview:m_splitDoc2];
        
        if (bSubView)
        {
            m_splitDoc1.delegate = self;
            m_splitDoc2.delegate = self;
        }
    } else {
        [self.contentView addSubview:m_splitDoc1];
    }
    
    [m_bzViewController1.bzview OnUpdate];
    if(m_bzViewController2) {
        [m_bzViewController2.bzview OnUpdate];
        [m_bzViewController2.bzview StopCaret];
    }
    return true;
}

- (void)RemoveAllBZViews
{
    if(m_bzViewController1)
    {
        [m_bzViewController1.view removeFromSuperview];
        m_bzViewController1 = nil;
    }
    if(m_bzViewController2)
    {
        [m_bzViewController2.view removeFromSuperview];
        m_bzViewController2 = nil;
    }
    if(m_subViewController1)
    {
        [m_subViewController1.view removeFromSuperview];
        m_subViewController1 = nil;
    }
    if(m_subViewController2)
    {
        [m_subViewController2.view removeFromSuperview];
        m_subViewController2 = nil;
    }
    if(m_splitDoc1)
    {
        m_splitDoc1.delegate = nil;
        [m_splitDoc1 removeFromSuperview];
        m_splitDoc1 = nil;
    }
    if(m_splitDoc2)
    {
        m_splitDoc2.delegate = nil;
        [m_splitDoc2 removeFromSuperview];
        m_splitDoc2 = nil;
    }
    if(m_splitDoc)
    {
        [m_splitDoc removeFromSuperview];
        m_splitDoc = nil;
    }
}

- (BZView *)GetBrotherView:(BZView *)view
{
    BZView *bzview1 = [m_bzViewController1 bzview];
    BZView *bzview2 = [m_bzViewController2 bzview];
    if (view == bzview1)
        return bzview2;
    else if (view == bzview2)
        return bzview1;
    return nil;
}

- (BZView *)GetActiveView
{
    if (m_isActiveView1)
        return m_bzViewController1 ?m_bzViewController1.bzview:nil;
    else
        return m_bzViewController2 ?m_bzViewController2.bzview:nil;
}


//- (BOOL)validateUserInterfaceItem:(id <NSValidatedUserInterfaceItem>)anItem
- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    SEL theAction = [menuItem action];
    BZOptions *bzopt = [BZOptions sharedInstance];
    
    if (theAction == @selector(OnViewBitmap:)) {
        [menuItem setState:(m_bBmpView)? NSOnState:NSOffState];
        BZView *bzviewA = [self GetActiveView];
        if (bzviewA==nil) return NO;
        Document *pDoc = [bzviewA GetDocument];
        return [pDoc GetDocSize] >= (bzopt->nBmpWidth * bzopt->nBmpColorWidth / 8);
    } else if (theAction == @selector(OnViewStruct:)) {
        [menuItem setState:(m_bStructView)? NSOnState:NSOffState];
    } else if (theAction == @selector(OnViewInspect:)) {
        [menuItem setState:(m_bInspectView)? NSOnState:NSOffState];
    } else if (theAction == @selector(OnViewAnalyzer:)) {
        [menuItem setState:(m_bAnalyzerView)? NSOnState:NSOffState];
    } else if (theAction == @selector(OnViewSplitH:)) {
        [menuItem setState:(m_nSplitView==SPLIT_H)? NSOnState:NSOffState];
    } else if (theAction == @selector(OnViewSplitV:)) {
        [menuItem setState:(m_nSplitView==SPLIT_V)? NSOnState:NSOffState];
    }
    return YES;
}

- (IBAction)OnViewBitmap:(id)sender
{
    //GetSplitInfo();
    m_bBmpView = !m_bBmpView;
    m_bStructView = m_bInspectView = m_bAnalyzerView = FALSE;
    [self CreateClient];
}
- (IBAction)OnViewStruct:(id)sender
{
    //GetSplitInfo();
    m_bStructView = !m_bStructView;
    m_bBmpView = m_bInspectView = m_bAnalyzerView = FALSE;
    [self CreateClient];
}
- (IBAction)OnViewInspect:(id)sender
{
    //GetSplitInfo();
    m_bInspectView = !m_bInspectView;
    m_bBmpView = m_bStructView = m_bAnalyzerView = FALSE;
    [self CreateClient];
}
- (IBAction)OnViewAnalyzer:(id)sender
{
    //GetSplitInfo();
    m_bAnalyzerView = !m_bAnalyzerView;
    m_bBmpView = m_bStructView = m_bInspectView = FALSE;
    [self CreateClient];
}


//MenuItem Reflectors
- (IBAction)OnCharmodeAscii:(id)sender
{
    BZView *bzviewA = [self GetActiveView];
    if(bzviewA) [bzviewA OnCharmodeAscii:sender];
}
- (IBAction)OnCharmodeSJIS:(id)sender
{
    BZView *bzviewA = [self GetActiveView];
    if(bzviewA) [bzviewA OnCharmodeJIS:sender];
}
- (IBAction)OnCharmodeUTF16:(id)sender
{
    BZView *bzviewA = [self GetActiveView];
    if(bzviewA) [bzviewA OnCharmodeUTF16:sender];
}
- (IBAction)OnCharmodeUTF8:(id)sender
{
    BZView *bzviewA = [self GetActiveView];
    if(bzviewA) [bzviewA OnCharmodeUTF8:sender];
}
- (IBAction)OnCharmodeJIS:(id)sender
{
    BZView *bzviewA = [self GetActiveView];
    if(bzviewA) [bzviewA OnCharmodeJIS:sender];
}
- (IBAction)OnCharmodeEUC:(id)sender
{
    BZView *bzviewA = [self GetActiveView];
    if(bzviewA) [bzviewA OnCharmodeEUC:sender];
}
- (IBAction)OnCharmodeEBCDIC:(id)sender
{
    BZView *bzviewA = [self GetActiveView];
    if(bzviewA) [bzviewA OnCharmodeEBCDIC:sender];
}
- (IBAction)OnCharmodeEPWING:(id)sender
{
    BZView *bzviewA = [self GetActiveView];
    if(bzviewA) [bzviewA OnCharmodeEPWING:sender];
}
- (IBAction)OnCharmodeAutoDetect:(id)sender
{
    BZView *bzviewA = [self GetActiveView];
    if(bzviewA) [bzviewA OnCharmodeAutoDetect:sender];
}

- (IBAction)OnByteOrderIntel:(id)sender
{
    BZView *bzviewA = [self GetActiveView];
    if(bzviewA) [bzviewA OnByteOrderIntel:sender];
}
- (IBAction)OnByteOrderMotorola:(id)sender
{
    BZView *bzviewA = [self GetActiveView];
    if(bzviewA) [bzviewA OnByteOrderMotorola:sender];
}

- (IBAction)OnViewSplitH:(id)sender
{
	//options.nCmdShow=SW_SHOWNORMAL;
	//ShowWindow(options.nCmdShow);
    
	//GetSplitInfo();
	m_nSplitView0 = m_nSplitView;
    if (m_nSplitView==SPLIT_H) {
        m_nSplitView = SPLIT_NONE;
    } else {
        m_nSplitView = SPLIT_H;
        BZWindowController *winController = self.windowController;
        if (winController->m_doc2==nil) {
            winController->m_doc2 = [[Document alloc] init];
        }
    }
	[self CreateClient];
    [m_bzViewController1.bzview InitCaret];
	m_nSplitView0 = m_nSplitView;
}
- (IBAction)OnViewSplitV:(id)sender
{
	//options.nCmdShow=SW_SHOWNORMAL;
	//ShowWindow(options.nCmdShow);
    
	//GetSplitInfo();
	m_nSplitView0 = m_nSplitView;
    if (m_nSplitView==SPLIT_V) {
        m_nSplitView = SPLIT_NONE;
    } else {
        m_nSplitView = SPLIT_V;
        BZWindowController *winController = self.windowController;
        if (winController->m_doc2==nil) {
            winController->m_doc2 = [[Document alloc] init];
        }
    }
	[self CreateClient];
    [m_bzViewController1.bzview InitCaret];
	m_nSplitView0 = m_nSplitView;
}
- (IBAction)OnFileSaveDumpList:(id)sender
{
    BZView *bzviewA = [self GetActiveView];
    if(bzviewA)
    {
        NSSavePanel *savePanel = [NSSavePanel savePanel];
        [savePanel setAllowedFileTypes:[NSArray arrayWithObjects:@"lst", @"lst", nil]];
        NSInteger retCode = [savePanel runModal];
        if (retCode==NSOKButton) {
            [bzviewA DrawToFile:[savePanel URL]];
        }
    }
}

- (void)splitViewDidResizeSubviews:(NSNotification *)notification
{
    NSSplitView *splitView = (NSSplitView*)[notification object];
    if (splitView == m_splitDoc1) {
        [m_subViewController2.view setFrame:m_subViewController1.view.frame];
    } else {
        [m_subViewController1.view setFrame:m_subViewController2.view.frame];
    }
}


- (IBAction)openDocument:(id)sender
{
    NSLog(@"openDocumet");
    [Document setIsNewWindow:TRUE];
    NSDocumentController* dc = [NSDocumentController sharedDocumentController];
 //   [dc newDocument:sender];
    [dc openDocument:sender];
}

- (IBAction)openDocumentWithClose:(id)sender
{
    NSLog(@"openDocumentWithClose");
    [Document setIsNewWindow:FALSE];
    NSDocumentController* dc = [NSDocumentController sharedDocumentController];
    [dc openDocument:sender];
}

- (void)becomeMainWindow
{
    if (m_isActiveView1 && m_bzViewController1) [m_bzViewController1.bzview InitCaret];
    else if (!m_isActiveView1 && m_bzViewController2) [m_bzViewController2.bzview InitCaret];
    //NSLog(@"becomeMainWindiw");
    [super becomeMainWindow];
}

- (void)resignMainWindow
{
    if (m_isActiveView1 && m_bzViewController1) [m_bzViewController1.bzview StopCaret];
    else if (!m_isActiveView1 && m_bzViewController2) [m_bzViewController2.bzview StopCaret];
    //NSLog(@"resignMainWindow");
    [super resignMainWindow];
}


@end
