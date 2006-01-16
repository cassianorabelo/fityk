// This file is part of fityk program. Copyright (C) Marcin Wojdyr
// $Id$

// wxwindows headers, see wxwindows samples for description
#include <wx/wxprec.h>
#ifdef __BORLANDC__
#pragma hdrstop
#endif
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/config.h>
#include <wx/colordlg.h>

#include "common.h"
#include "wx_common.h"

bool read_bool_from_config(wxConfigBase *cf, const wxString& key, bool def_val)
{ 
    bool b; 
    cf->Read(key, &b, def_val); 
    return b; 
}

double read_double_from_config(wxConfigBase *cf, const wxString& key, 
                               double def_val)
{ 
    double d; 
    cf->Read(key, &d, def_val); 
    return d; 
}

wxColour read_color_from_config(const wxConfigBase *config, const wxString& key,
                                const wxColour& default_value)
{
    return wxColour (config->Read (key + "/Red", default_value.Red()), 
                     config->Read (key + "/Green", default_value.Green()), 
                     config->Read (key + "/Blue", default_value.Blue()));
}

void write_color_to_config(wxConfigBase *config, const wxString& key,
                           const wxColour& value)
{
    config->Write (key + "/Red", value.Red());
    config->Write (key + "/Green", value.Green());
    config->Write (key + "/Blue", value.Blue());
}

wxFont read_font_from_config(wxConfigBase const *config, wxString const& key,
                             wxFont const &default_value)
{
    if (!default_value.Ok()) {
        if (config->HasEntry(key+"/pointSize")
              && config->HasEntry(key+"/family")
              && config->HasEntry(key+"/style")
              && config->HasEntry(key+"/weight")
              && config->HasEntry(key+"/faceName"))
            return wxFont (config->Read(key+"/pointSize", 0L),
                           config->Read(key+"/family", 0L),
                           config->Read(key+"/style", 0L),
                           config->Read(key+"/weight", 0L),
                           false, //underline
                           config->Read(key+"/faceName", ""));
        else
            return wxNullFont;
    }
    return wxFont (config->Read(key+"/pointSize", default_value.GetPointSize()),
                   config->Read(key+"/family", default_value.GetFamily()),
                   config->Read(key+"/style", default_value.GetStyle()),
                   config->Read(key+"/weight", default_value.GetWeight()),
                   false, //underline
                   config->Read(key+"/faceName", default_value.GetFaceName()));
}

void write_font_to_config (wxConfigBase *config, const wxString& key,
                           const wxFont& value)
{
    config->Write (key + "/pointSize", value.GetPointSize());
    config->Write (key + "/family", value.GetFamily());
    config->Write (key + "/style", value.GetStyle());
    config->Write (key + "/weight", value.GetWeight());
    config->Write (key + "/faceName", value.GetFaceName());
}

bool change_color_dlg(wxColour& col)
{
    wxColourData col_data;
    col_data.SetCustomColour(0, col);
    col_data.SetColour(col);
    wxColourDialog dialog(0, &col_data);
    if (dialog.ShowModal() == wxID_OK 
            && col != dialog.GetColourData().GetColour()) { 
        col = dialog.GetColourData().GetColour();
        return true;
    }
    else
        return false;
}

//dummy events declared in wx_common.h
wxMouseEvent dummy_mouse_event;
wxCommandEvent dummy_cmd_event;


//===============================================================
//                            ProportionalSplitter
//===============================================================

ProportionalSplitter::ProportionalSplitter(wxWindow* parent, wxWindowID id, 
                                           float proportion, const wxSize& size,
                                           long style, const wxString& name) 
    : wxSplitterWindow(parent, id, wxDefaultPosition, size, style, name),
      m_proportion(proportion), m_firstpaint(true)
{
    wxASSERT(m_proportion >= 0. && m_proportion <= 1.);
    SetMinimumPaneSize(20);
    ResetSash();
    Connect(GetId(), wxEVT_COMMAND_SPLITTER_SASH_POS_CHANGED,
                (wxObjectEventFunction) &ProportionalSplitter::OnSashChanged);
    Connect(GetId(), wxEVT_SIZE, 
                     (wxObjectEventFunction) &ProportionalSplitter::OnReSize);
    //hack to set sizes on first paint event
    Connect(GetId(), wxEVT_PAINT, 
                      (wxObjectEventFunction) &ProportionalSplitter::OnPaint);
}

bool ProportionalSplitter::SplitHorizontally(wxWindow* win1, wxWindow* win2,
                                             float proportion) 
{
    if (proportion >= 0. && proportion <= 1.)
        m_proportion = proportion;
    int height = GetClientSize().GetHeight();
    int h = iround(height * m_proportion);
    //sometimes there is a strange problem without it (why?)
    if (h < GetMinimumPaneSize() || h > height-GetMinimumPaneSize())
        h = 0; 
    return wxSplitterWindow::SplitHorizontally(win1, win2, h);
}

bool ProportionalSplitter::SplitVertically(wxWindow* win1, wxWindow* win2,
                                           float proportion) 
{
    if (proportion >= 0. && proportion <= 1.)
        m_proportion = proportion;
    int width = GetClientSize().GetWidth();
    int w = iround(width * m_proportion);
    if (w < GetMinimumPaneSize() || w > width-GetMinimumPaneSize())
        w = 0;
    return wxSplitterWindow::SplitVertically(win1, win2, w);
}

int ProportionalSplitter::GetExpectedSashPosition()
{
    return iround(GetWindowSize() * m_proportion);
}

void ProportionalSplitter::ResetSash()
{
    SetSashPosition(GetExpectedSashPosition());
}

void ProportionalSplitter::OnReSize(wxSizeEvent& event)
{
    // We may need to adjust the sash based on m_proportion.
    ResetSash();
    event.Skip();
}

void ProportionalSplitter::OnSashChanged(wxSplitterEvent &event)
{
    // We'll change m_proportion now based on where user dragged the sash.
    const wxSize& s = GetSize();
    int t = GetSplitMode() == wxSPLIT_HORIZONTAL ? s.GetHeight() : s.GetWidth();
    m_proportion = float(GetSashPosition()) / t;
    event.Skip();
}

void ProportionalSplitter::OnPaint(wxPaintEvent &event)
{
    if (m_firstpaint) {
        if (GetSashPosition() != GetExpectedSashPosition())
            ResetSash();
        m_firstpaint = false;
    }
    event.Skip();
}

