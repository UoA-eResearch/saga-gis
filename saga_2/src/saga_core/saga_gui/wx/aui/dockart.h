///////////////////////////////////////////////////////////////////////////////
// Name:        wx/aui/dockart.h
// Purpose:     wxaui: wx advanced user interface - docking window manager
// Author:      Benjamin I. Williams
// Modified by:
// Created:     2005-05-17
// RCS-ID:      $Id: dockart.h,v 1.1 2006-10-23 13:30:21 oconrad Exp $
// Copyright:   (C) Copyright 2005, Kirix Corporation, All Rights Reserved.
// Licence:     wxWindows Library Licence, Version 3.1
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DOCKART_H_
#define _WX_DOCKART_H_

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include "wx/defs.h"
#include "aui.h"

#if wxUSE_AUI

#include "wx/pen.h"
#include "wx/brush.h"
#include "wx/bitmap.h"
#include "wx/colour.h"

// dock art provider code - a dock provider provides all drawing
// functionality to the wxAui dock manager.  This allows the dock
// manager to have plugable look-and-feels

class WXDLLIMPEXP_AUI wxDockArt
{
public:

    wxDockArt() { }
    virtual ~wxDockArt() { }

    virtual int GetMetric(int id) = 0;
    virtual void SetMetric(int id, int new_val) = 0;
    virtual void SetFont(int id, const wxFont& font) = 0;
    virtual wxFont GetFont(int id) = 0;
    virtual wxColour GetColour(int id) = 0;
    virtual void SetColour(int id, const wxColor& colour) = 0;
    wxColour GetColor(int id) { return GetColour(id); }
    void SetColor(int id, const wxColour& color) { SetColour(id, color); }

    virtual void DrawSash(wxDC& dc,
                          int orientation,
                          const wxRect& rect) = 0;

    virtual void DrawBackground(wxDC& dc,
                          int orientation,
                          const wxRect& rect) = 0;

    virtual void DrawCaption(wxDC& dc,
                          const wxString& text,
                          const wxRect& rect,
                          wxPaneInfo& pane) = 0;

    virtual void DrawGripper(wxDC& dc,
                          const wxRect& rect,
                          wxPaneInfo& pane) = 0;

    virtual void DrawBorder(wxDC& dc,
                          const wxRect& rect,
                          wxPaneInfo& pane) = 0;

    virtual void DrawPaneButton(wxDC& dc,
                          int button,
                          int button_state,
                          const wxRect& rect,
                          wxPaneInfo& pane) = 0;
};


// this is the default art provider for wxFrameManager.  Dock art
// can be customized by creating a class derived from this one,
// or replacing this class entirely

class WXDLLIMPEXP_AUI wxDefaultDockArt : public wxDockArt
{
public:

    wxDefaultDockArt();

    int GetMetric(int metric_id);
    void SetMetric(int metric_id, int new_val);
    wxColour GetColour(int id);
    void SetColour(int id, const wxColor& colour);
    void SetFont(int id, const wxFont& font);
    wxFont GetFont(int id);

    void DrawSash(wxDC& dc,
                  int orientation,
                  const wxRect& rect);

    void DrawBackground(wxDC& dc,
                  int orientation,
                  const wxRect& rect);

    void DrawCaption(wxDC& dc,
                  const wxString& text,
                  const wxRect& rect,
                  wxPaneInfo& pane);

    void DrawGripper(wxDC& dc,
                  const wxRect& rect,
                  wxPaneInfo& pane);

    void DrawBorder(wxDC& dc,
                  const wxRect& rect,
                  wxPaneInfo& pane);

    void DrawPaneButton(wxDC& dc,
                  int button,
                  int button_state,
                  const wxRect& rect,
                  wxPaneInfo& pane);

protected:

    void DrawCaptionBackground(wxDC& dc, const wxRect& rect, bool active);

protected:

    wxPen m_border_pen;
    wxBrush m_sash_brush;
    wxBrush m_background_brush;
    wxBrush m_gripper_brush;
    wxFont m_caption_font;
    wxBitmap m_inactive_close_bitmap;
    wxBitmap m_inactive_pin_bitmap;
    wxBitmap m_active_close_bitmap;
    wxBitmap m_active_pin_bitmap;
    wxPen m_gripper_pen1;
    wxPen m_gripper_pen2;
    wxPen m_gripper_pen3;
    wxColour m_active_caption_colour;
    wxColour m_active_caption_gradient_colour;
    wxColour m_active_caption_text_colour;
    wxColour m_inactive_caption_colour;
    wxColour m_inactive_caption_gradient_colour;
    wxColour m_inactive_caption_text_colour;
    int m_border_size;
    int m_caption_size;
    int m_sash_size;
    int m_button_size;
    int m_gripper_size;
    int m_gradient_type;
};



#endif // wxUSE_AUI
#endif //_WX_DOCKART_H_
