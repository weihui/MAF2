/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mmoEditMetadata.cpp,v $
  Language:  C++
  Date:      $Date: 2006-09-22 10:11:57 $
  Version:   $Revision: 1.2 $
  Authors:   Paolo Quadrani    
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


#include "mmoEditMetadata.h"

#include "mafDecl.h"
#include "mafEvent.h"
#include "mmgGui.h"

#include "mafNode.h"
#include "mafTagArray.h"
#include "mafTagItem.h"

#include <vector>

//----------------------------------------------------------------------------
mafCxxTypeMacro(mmoEditMetadata);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
mmoEditMetadata::mmoEditMetadata(wxString label) :
mafOp(label)
//----------------------------------------------------------------------------
{
  m_OpType	= OPTYPE_OP;
  m_Canundo = true;

  m_MetadataList  = NULL;
  m_TagArray      = NULL;
  m_OldTagArray   = NULL;
  m_SelectedTag   = NULL;

  m_TagName = _("Name");
  m_TagType = mmoEditMetadata::NUMERIC_TAG;
  m_TagMulteplicity   = 1;
  m_TagComponent      = 0;
  m_TagValueAsString  = "0";
  m_TagValueAsDouble  = 0.0;
}

//----------------------------------------------------------------------------
mmoEditMetadata::~mmoEditMetadata()
//----------------------------------------------------------------------------
{
  mafDEL(m_OldTagArray);
}

//----------------------------------------------------------------------------
mafOp* mmoEditMetadata::Copy()
//----------------------------------------------------------------------------
{
	return new mmoEditMetadata(m_Label);
}

//----------------------------------------------------------------------------
// Widgets ID's
//----------------------------------------------------------------------------
enum METADATA_EDIT_ID
{
  ID_METADATA_LIST = MINID,
  ID_ADD_METADATA,
  ID_REMOVE_METADATA,
  ID_TAG_TYPE,
  ID_TAG_MULTEPLICITY,
  ID_TAG_COMPONENT,
  ID_TAG_DOUBLE_VALUE,
  ID_TAG_STRING_VALUE
};

//----------------------------------------------------------------------------
void mmoEditMetadata::OpRun()
//----------------------------------------------------------------------------
{
  m_TagArray = m_Input->GetTagArray();
  mafNEW(m_OldTagArray);
  m_OldTagArray->DeepCopy(m_TagArray);
  std::vector<std::string> tag_list;
  m_TagArray->GetTagList(tag_list);

  m_Gui = new mmgGui(this);
  m_MetadataList = m_Gui->ListBox(ID_METADATA_LIST,"",120);
  for (int t=0; t<tag_list.size();t++)
    m_MetadataList->Insert(tag_list[t].c_str(),0);

  if (!m_MetadataList->IsEmpty())
  {
    m_MetadataList->SetSelection(0,true);
    SelectTag(m_MetadataList->GetString(0).c_str());
  }

  wxString tag_type[2] = {_("numeric"),_("string")};

  m_Gui->Button(ID_ADD_METADATA,_("Add"));
  m_Gui->Button(ID_REMOVE_METADATA,_("Remove"));
  m_Gui->Divider(2);
  m_Gui->Combo(ID_TAG_TYPE,_("type"),&m_TagType,2,tag_type);
  m_Gui->Integer(ID_TAG_MULTEPLICITY,_("multep."),&m_TagMulteplicity,1);
  m_Gui->Integer(ID_TAG_COMPONENT,_("comp"),&m_TagComponent,0);
  m_Gui->Double(ID_TAG_DOUBLE_VALUE,_("num. value"),&m_TagValueAsDouble);
  m_Gui->String(ID_TAG_STRING_VALUE,_("string value"),&m_TagValueAsString);

  EnableWidgets();
  m_Gui->OkCancel();
  
  ShowGui();
}
//----------------------------------------------------------------------------
void mmoEditMetadata::SelectTag(const char *tag_name)
//----------------------------------------------------------------------------
{
  m_SelectedTag = m_TagArray->GetTag(tag_name);
  m_TagName = m_SelectedTag->GetName();
  m_TagType = m_SelectedTag->GetType() - 1;
  m_TagMulteplicity = m_SelectedTag->GetNumberOfComponents();
  m_TagComponent = 0;
  m_TagValueAsString = m_SelectedTag->GetComponent(m_TagComponent);
  m_TagValueAsDouble = m_SelectedTag->GetComponentAsDouble(m_TagComponent);

  EnableWidgets();
}
//----------------------------------------------------------------------------
void mmoEditMetadata::EnableWidgets()
//----------------------------------------------------------------------------
{
  bool enable_edit = m_MetadataList->GetCount() > 0;
  m_Gui->Enable(ID_REMOVE_METADATA,enable_edit);
  m_Gui->Enable(ID_TAG_TYPE,enable_edit);
  m_Gui->Enable(ID_TAG_MULTEPLICITY,enable_edit);
  m_Gui->Enable(ID_TAG_COMPONENT,enable_edit);
  m_Gui->Enable(ID_TAG_DOUBLE_VALUE,m_TagType == mmoEditMetadata::NUMERIC_TAG && enable_edit);
  m_Gui->Enable(ID_TAG_STRING_VALUE,m_TagType == mmoEditMetadata::STRING_TAG && enable_edit);
}
//----------------------------------------------------------------------------
void mmoEditMetadata::OnEvent(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{
  if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
  {
    switch(e->GetId())
    {
      case ID_METADATA_LIST:
      {
        int sel = m_MetadataList->GetSelection();
        if (sel >= 0)
        {
          wxString tag_name = m_MetadataList->GetString(sel);
          SelectTag(tag_name.c_str());
          m_Gui->Update();
        }
      }
      break;
      case ID_ADD_METADATA:
      {
        mafString tag_name = wxGetTextFromUser("Insert metadata name:","New metadata","Name").c_str();
        AddNewTag(tag_name);
        m_Gui->Update();
      }
      break;
      case ID_TAG_TYPE:
        m_SelectedTag->SetType(m_TagType+1);
        EnableWidgets();
      break;
      case ID_TAG_MULTEPLICITY:
        m_SelectedTag->SetNumberOfComponents(m_TagMulteplicity);
      break;
      case ID_TAG_COMPONENT:
        if (m_TagComponent >= m_TagMulteplicity)
        {
          m_TagComponent = m_TagMulteplicity-1;
        }
        if (m_TagType == mmoEditMetadata::NUMERIC_TAG)
        {
          m_TagValueAsDouble = m_SelectedTag->GetComponentAsDouble(m_TagComponent);
        }
        else
        {
          m_TagValueAsString = m_SelectedTag->GetComponent(m_TagComponent);
        }
        m_Gui->Update();
      break;
      case ID_TAG_DOUBLE_VALUE:
        m_SelectedTag->SetComponent(m_TagValueAsDouble,m_TagComponent);
      break;
      case ID_TAG_STRING_VALUE:
        m_SelectedTag->SetComponent(m_TagValueAsString,m_TagComponent);
      break;
      case ID_REMOVE_METADATA:
      {
        int sel = m_MetadataList->GetSelection();
        m_MetadataList->Delete(sel);
        m_TagArray->DeleteTag(m_SelectedTag->GetName());
        m_SelectedTag = NULL;
        sel = sel > 0 ? sel-- : 0;
        m_MetadataList->SetSelection(sel,true);
        OnEvent(&mafEvent(m_Gui,ID_METADATA_LIST));
      }
      break;
      case wxOK:
        OpStop(OP_RUN_OK);
      break;
      case wxCANCEL:
        OpUndo();
        OpStop(OP_RUN_CANCEL);
      break;
      default:
        mafEventMacro(*e);
      break; 
    }
  }
}
//----------------------------------------------------------------------------
void mmoEditMetadata::AddNewTag(mafString &name)
//----------------------------------------------------------------------------
{
  int n = -1;
  m_TagName = name;
  wxString new_name;
  new_name = m_TagName;
  m_TagType = mmoEditMetadata::NUMERIC_TAG;
  m_TagMulteplicity   = 1;
  m_TagComponent      = 0;
  m_TagValueAsString  = "0";
  m_TagValueAsDouble  = 0.0;

  m_SelectedTag = new mafTagItem();
  m_SelectedTag->SetName(new_name.c_str());
  m_SelectedTag->SetType(m_TagType+1);
  m_SelectedTag->SetNumberOfComponents(m_TagMulteplicity);
  m_SelectedTag->SetComponent(m_TagValueAsDouble,m_TagComponent);
  while (m_TagArray->IsTagPresent(new_name.c_str()))
  {
    new_name = m_TagName;
    new_name << n++;
  }
  m_MetadataList->Insert(new_name, m_MetadataList->GetCount());
  m_TagName = new_name;
  m_SelectedTag->SetName(m_TagName.GetCStr());
  m_TagArray->SetTag(*m_SelectedTag);
  cppDEL(m_SelectedTag);
  SelectTag(m_TagName.GetCStr());
  m_MetadataList->SetSelection(m_MetadataList->GetCount()-1,true);
}
//----------------------------------------------------------------------------
void mmoEditMetadata::OpUndo()
//----------------------------------------------------------------------------
{
  m_TagArray->DeepCopy(m_OldTagArray);
}