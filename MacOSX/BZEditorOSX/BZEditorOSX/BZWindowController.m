//
//  BZWindowController.m
//  TamaBinary
//
//  Created by ad on 13/01/08.
//  Copyright (c) 2013年 tamachanbinary. All rights reserved.
//

#import "BZWindowController.h"

@interface BZWindowController ()

@end

@implementation BZWindowController

- (id)initWithWindow:(NSWindow *)window
{
    self = [super initWithWindow:window];
    if (self) {
        // Initialization code here.
    }
    
    return self;
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    
    // Implement this method to handle any initialization after your window controller's window has been loaded from its nib file.
    /*BZWindow *bzwnd = (BZWindow*)self.window;
    [bzwnd OnCreateClient];*/
}

-(void)setDocument:(NSDocument *)document
{
    if (document==nil) {
        return;
    }
    BZWindow *bzWnd = (BZWindow*)self.window;
    NSLog(@"setDocment %llX, isNewWnd=%d, activeView=%d", document, [Document isNewWindow], bzWnd->m_isActiveView1?1:2);
    [bzWnd RemoveAllBZViews];
    
    if (![Document isNewWindow] && !(bzWnd->m_isActiveView1))
    {
        if (m_doc2) {
            [m_doc2 CloseDocument];
            [[NSDocumentController sharedDocumentController] removeDocument:m_doc2];
            //m_doc2 = nil;
        }
        m_doc2 = (Document*)document;
        [super setDocument:document];
    } else {
        if (m_doc1) {
            [m_doc1 CloseDocument];
            [[NSDocumentController sharedDocumentController] removeDocument:m_doc1];
            //m_doc1 = nil;
        }
        m_doc1 = (Document*)document;
        [super setDocument:document];
    }

    [bzWnd CreateClient];
    
    /*if (m_doc1)
    {
        BZView* bzview1 = ((BZWindow*)self.window)->m_bzViewController1.bzview;
        if (bzview1) [bzview1 OnUpdate];
    }*/
}

- (void)setSuperDocumentIsDoc1:(BOOL)isDoc1
{
    if (isDoc1 && m_doc1) {
        [super setDocument:m_doc1];
    } else if (!isDoc1 && m_doc2) {
        [super setDocument:m_doc2];
    }
}

-(id)document
{
    return m_doc1;
}

- (void)dealloc
{
    if (m_doc1) {
        [m_doc1 CloseDocument];
        [[NSDocumentController sharedDocumentController] removeDocument:m_doc1];
        m_doc1 = nil;
    }
    if (m_doc2) {
        [m_doc2 CloseDocument];
        [[NSDocumentController sharedDocumentController] removeDocument:m_doc2];
        m_doc2 = nil;
    }
}

@end
