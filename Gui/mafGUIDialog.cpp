/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mafGUIDialog.cpp,v $
  Language:  C++
  Date:      $Date: 2011-12-15 16:24:36 $
  Version:   $Revision: 1.2.2.1 $
  Authors:   Silvano Imboden
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/


#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------
#include <wx/statline.h>

#include "mafGUIDialog.h"
#include "wx/busyinfo.h"
#include "mafDecl.h"

//----------------------------------------------------------------------------
// Event Table:
//----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(mafGUIDialog, wxDialog)
  EVT_CLOSE(mafGUIDialog::nvOnCloseWindow)
  EVT_BUTTON(wxID_OK, mafGUIDialog::nvOnOK)
  EVT_BUTTON(wxID_CANCEL, mafGUIDialog::nvOnCancel)
  EVT_BUTTON(wxID_CLOSE, mafGUIDialog::nvOnClose)
  EVT_BUTTON(wxOK, mafGUIDialog::nvOnOK)
  EVT_BUTTON(wxCANCEL, mafGUIDialog::nvOnCancel)
  EVT_SIZE(mafGUIDialog::OnSize)

/* todo: discover if the following syntax is valid -- if yes I can have the redefined handler called (if handler are virtual)
  EVT_CLOSE(OnCloseWindow)
  EVT_BUTTON(wxID_OK, OnOK)
  EVT_BUTTON(wxID_CANCEL, OnCancel)
  EVT_BUTTON(wxID_CLOSE, OnClose)
  EVT_BUTTON(wxOK, OnOK)
  EVT_BUTTON(wxCANCEL, OnCancel)
*/
END_EVENT_TABLE()

//----------------------------------------------------------------------------
mafGUIDialog::mafGUIDialog(const wxString& title,long style)
: wxDialog()
//----------------------------------------------------------------------------
{
  m_Listener = NULL;
  m_DialogInitialized = false;
  
  long s = wxCAPTION;
  if(style & mafRESIZABLE) 
    s |= wxRESIZE_BORDER;
  if(style & mafCLOSEWINDOW) 
  {
    s |= wxSYSTEM_MENU;
    s |= wxCLOSE_BOX;
  }

  Create(mafGetFrame(),-1,title,wxDefaultPosition,wxDefaultSize,s); 

  m_DialogSizer  =  new wxBoxSizer( wxVERTICAL );
  m_GuiSizer     =  new wxBoxSizer( wxVERTICAL );
  m_DialogSizer->Add(m_GuiSizer,1,wxEXPAND);

  m_ButtonsSizer =  NULL;
  m_OkButton     = NULL;
  m_CancelButton = NULL;
  m_CloseButton  = NULL;

  if(style & mafOK || style & mafCANCEL || style & mafCLOSE )
  {
    m_ButtonsSizer =  new wxBoxSizer( wxHORIZONTAL );

    wxBoxSizer *vsz = new wxBoxSizer(wxVERTICAL);
    vsz->Add(new wxStaticLine(this,-1),0,wxEXPAND | wxALL, 3);
    vsz->Add(m_ButtonsSizer,0,wxALIGN_CENTRE);

    m_DialogSizer->Add(vsz,0,wxEXPAND);
  }
  if( style & mafOK )
  {
    m_OkButton = new wxButton(this,wxID_OK,"ok");
    m_ButtonsSizer->Add(m_OkButton);
  }
  if( style & mafCANCEL )
  {
    m_CancelButton = new wxButton(this,wxID_CANCEL,"cancel");
    m_ButtonsSizer->Add(m_CancelButton);
  }
  if( (style & mafCLOSE) )
  {
    m_CloseButton = new wxButton(this,wxID_CLOSE,"close");
    m_ButtonsSizer->Add(m_CloseButton);
  }
  this->SetSizer( m_DialogSizer );
  SetMinSize(wxSize(200,100));
  SetSize(wxSize(200,100));
  m_DialogSizer->Fit(this);
}
//----------------------------------------------------------------------------
mafGUIDialog::~mafGUIDialog()
//----------------------------------------------------------------------------
{
  m_DialogInitialized = false;
}
//----------------------------------------------------------------------------
int mafGUIDialog::ShowModal()
//----------------------------------------------------------------------------
{
  if(! m_DialogInitialized )
  {
    // these operations are delayed from the Ctor,
    // to allow the user to Add other gui in the meanwhile
    
    m_DialogSizer->Layout();           // resize & fit the contents
    m_DialogSizer->SetSizeHints(this); // resize the dialog accordingly 
    m_DialogInitialized = true;
  }
  return wxDialog::ShowModal();
}
//--------------------------------------------------------------------------------
void mafGUIDialog::OnEvent(mafEventBase *maf_event)
//--------------------------------------------------------------------------------
{
  // POURPOSE:
  // allow wxOK and wxCANCEL generated by a mafGUI to be considered 
  // like the corresponding events generated by normal buttons.
  if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
  {
    switch(e->GetId())
    {
      case wxID_OK:
      case wxOK:
      {
        wxCommandEvent c(0, wxID_OK);
        OnOK(c);
      }
      break;
      case wxID_CANCEL:
      case wxCANCEL:
      {
        wxCommandEvent c(0, wxID_CANCEL);
        OnCancel(c);  
      }
      break;
      case wxID_CLOSE:
        wxDialog::Close(); // will call OnCloseWindow -- which will call OnCancel
      break;
      default:
        mafEventMacro(*e);
    }
  }
}
//----------------------------------------------------------------------------
void mafGUIDialog::OnCloseWindow(wxCloseEvent &event)
//----------------------------------------------------------------------------
{
  // --- note about wxDialog::OnCloseWindow
  
  // We'll send a Cancel message by default, which may close the dialog.
  // Check for looping if the Cancel event handler calls Close().

  // Note that if a cancel button and handler aren't present in the dialog,
  // nothing will happen when you close the dialog via the window manager, or
  // via Close(). We wouldn't want to destroy the dialog by default, since
  // the dialog may have been created on the stack. However, this does mean
  // that calling dialog->Close() won't delete the dialog unless the handler
  // for wxID_CANCEL does so. So use Destroy() if you want to be sure to
  // destroy the dialog. The default OnCancel (above) simply ends a modal
  // dialog, and hides a modeless dialog.

  //SIL. 20-4-2005: 
  //So, in normal windows the close buttons destroy the window (Close -> CloseWindow -> Destroy )
  //in dialogs the close button calls Show(false) and don't call Destroy by default.
  //This is reasonable, because the user can call dlg.Show again.

  //So, The close button should call Cancel
  //Cancel should not destroy the window - just hide it
  //To know when the window is destroyed - override Destroy.

  wxDialog::OnCloseWindow(event);
}
//----------------------------------------------------------------------------
void mafGUIDialog::OnOK(wxCommandEvent &event)
//----------------------------------------------------------------------------
{
  //wxDialog::OnOK(event);
  /* --- this is what wxDialog::OnOK does
  // this is what is done 
  if ( Validate() && TransferDataFromWindow() )
  {
    SetReturnCode(wxID_OK);  
    Show(false);
  }
  Note that the Window just hidden, is not destroyed.  
  */

  //SIL 12-07-05
  // redefined because our TransferDataFromWindow
  // use a different semantic for the return value :
  //     wxWin - return false mean problem
  //     maf   - return false mean value not changed 
  if ( Validate()  )
  {
    TransferDataFromWindow();
    SetReturnCode(wxID_OK);  
    Show(false);
  }
}
//----------------------------------------------------------------------------
void mafGUIDialog::OnCancel(wxCommandEvent &event)
//----------------------------------------------------------------------------
{
  wxDialog::OnCancel(event);
  /* --- this is what wxDialog::OnCancel does
  SetReturnCode(wxID_CANCEL);
  Show(false);
  Note that the Window just hidden, is not destroyed.  
  */
}
