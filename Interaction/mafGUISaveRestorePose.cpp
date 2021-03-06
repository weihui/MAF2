/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mafGUISaveRestorePose.cpp,v $
  Language:  C++
  Date:      $Date: 2009-11-09 12:24:05 $
  Version:   $Revision: 1.1.2.2 $
  Authors:   Paolo Quadrani , Stefano Perticoni
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


#include "mafGUISaveRestorePose.h"
#include "mafDecl.h"
#include "mafGUI.h"
#include "mafSmartPointer.h"

#include "mafMatrix.h"
#include "mafTransform.h"
#include "mafVME.h"
#include "mafVMEOutput.h"
#include "mafTagArray.h"
#include "mafTagItem.h"

#include "vtkMatrix4x4.h"

mafString dummyPoseNameUsedForTesting = "dummyPoseNameUsedForTesting";

//----------------------------------------------------------------------------
mafGUISaveRestorePose::mafGUISaveRestorePose(mafVME *input, mafObserver *listener, int typeGui, bool testMode)
//----------------------------------------------------------------------------
{
  assert(input);

  m_Listener = listener;
  m_InputVME = input;
  m_Gui = NULL;
  m_TypeGui = typeGui;
  
  m_TestMode = testMode;
  
  if (m_TestMode == false)
  {
    CreateGui();
  }
}
//----------------------------------------------------------------------------
mafGUISaveRestorePose::~mafGUISaveRestorePose() 
//----------------------------------------------------------------------------
{ 
}

//----------------------------------------------------------------------------
void mafGUISaveRestorePose::CreateGui()
//----------------------------------------------------------------------------
{
  m_Gui = new mafGUI(this);
  m_Gui->SetListener(this);
  m_Gui->Divider(2);
  if(m_TypeGui == ID_POSE_GUI)
  {
    m_Gui->Label("save/restore pose", true);
    m_Gui->Button(ID_SAVE,"save pose");
    m_Gui->Button(ID_REMOVE,"remove pose");
    m_Gui->Label("");
    m_PositionsList = m_Gui->ListBox(ID_APPLY,"apply pose");
  }
  else if(m_TypeGui == ID_SCALE_GUI)
  {
    m_Gui->Label("save/restore scaling", true);
    m_Gui->Button(ID_SAVE,"save scaling");
    m_Gui->Button(ID_REMOVE,"remove scaling");
    m_Gui->Label("");
    m_PositionsList = m_Gui->ListBox(ID_APPLY,"apply scaling");
  }
  
  m_PositionsList->Clear();
  FillListBoxWithABSPosesStoredInInputVME();

  EnableWidgets(true);

	m_Gui->Divider();

  m_Gui->Update();
}

//----------------------------------------------------------------------------
void mafGUISaveRestorePose::OnEvent(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{
  if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
  {
    switch(e->GetId())
    {
      case ID_SAVE:
        StorePose();
        m_Gui->Enable(ID_APPLY, true);
      break;
      case ID_REMOVE:
      {
        int sel_pose = m_PositionsList->GetSelection();
        if (sel_pose != -1)
          RemovePose(sel_pose);
        if (m_PositionsList->GetCount() == 0)
          m_Gui->Enable(ID_REMOVE, false);
      }
      break;
      case ID_APPLY:
      {
        int sel_pose = m_PositionsList->GetSelection();
        if (sel_pose == -1)
          return;
        EnableWidgets(true);
        RestorePose(sel_pose);
      }
      break;
      default:
        mafEventMacro(*e);
      break;
    }
  }
}

//----------------------------------------------------------------------------
void mafGUISaveRestorePose::EnableWidgets(bool enable)
//----------------------------------------------------------------------------
{
  int num_stored_poses = m_PositionsList->GetCount();

  m_Gui->Enable(ID_SAVE, enable);
  m_Gui->Enable(ID_REMOVE, enable && num_stored_poses > 0);
  m_Gui->Enable(ID_APPLY, enable && num_stored_poses > 0);
}
//----------------------------------------------------------------------------
void mafGUISaveRestorePose::FillListBoxWithABSPosesStoredInInputVME()
//----------------------------------------------------------------------------
{
	mafTagArray *tag_array = m_InputVME->GetTagArray();
  int n = tag_array->GetNumberOfTags();
  std::vector<std::string> tag_list;
  tag_array->GetTagList(tag_list);
	for(int t=0; t<n; t++)
	{
	  mafTagItem *item = tag_array->GetTag(tag_list[t].c_str());
		if(item->GetNumberOfComponents() == 16)
		{
			wxString name = item->GetName();
      if(name.Find("STORED_ABS_POS_") != -1)
      {
			  name = name.Remove(0,15);
			  m_PositionsList->Append(name);
      }
		}
	}
}
//----------------------------------------------------------------------------
void mafGUISaveRestorePose::StorePose()
{
  assert(m_InputVME);

  mafMatrix *absPose = m_InputVME->GetOutput()->GetAbsMatrix();

  wxString pose_name = "";
  
  if (m_TestMode == false)
  {
    do 
    {
  	  pose_name = wxGetTextFromUser("Enter the pose name","Save position","Saved pose", m_Gui);
    } 
    while(m_PositionsList->FindString(pose_name) != wxNOT_FOUND);
  }
  else if (m_TestMode == true)
  {
    pose_name = dummyPoseNameUsedForTesting;
  }
  
  pose_name.Trim();
  pose_name.Trim(FALSE);

  wxString AbsPos_tagName = "STORED_ABS_POS_" + pose_name;

  mafTagArray *tagArray = m_InputVME->GetTagArray();
  if(tagArray->IsTagPresent(AbsPos_tagName.c_str())) 
  {
		wxString msg = "this name is already used, do you want to overwrite it ?";
		int res = wxMessageBox(msg,"Store Position", wxOK|wxCANCEL|wxICON_QUESTION, NULL);
		if(res == wxCANCEL) return;

    //remove item to be overwritten
		tagArray->DeleteTag(AbsPos_tagName.c_str());
	}
	
  StorePoseHelper(AbsPos_tagName);
  
  if (m_TestMode == false)
  {
    m_PositionsList->Append(pose_name);
  }
}
//----------------------------------------------------------------------------
void mafGUISaveRestorePose::RemovePose( int absPoseListBoxID )
{
  wxString pose_name;

  if (m_TestMode == false)
  {
    pose_name = m_PositionsList->GetString(absPoseListBoxID);
  }
  else if (m_TestMode == true)
  {
    pose_name = dummyPoseNameUsedForTesting.GetCStr();
  }
  
  wxString AbsPos_tagName = "STORED_ABS_POS_" + pose_name;

  RemovePoseHelper(AbsPos_tagName);
  
  if (m_TestMode == false)
  {
    m_PositionsList->Delete(absPoseListBoxID);
  }
}
//----------------------------------------------------------------------------
void mafGUISaveRestorePose::RestorePose( int absPoseListBoxID )
{
  wxString pose_name;

  if (m_TestMode == false)
  {
    pose_name = m_PositionsList->GetString(absPoseListBoxID);
  }
  else if (m_TestMode == true)
  {
    pose_name = dummyPoseNameUsedForTesting.GetCStr();
  }

  RestorePoseHelper(pose_name);
  return;
}

void mafGUISaveRestorePose::RestorePoseHelper( mafString pose_name )
{
  assert(m_InputVME);

  wxString AbsPos_tagName = "STORED_ABS_POS_" + pose_name;
  int comp = 0;
  mafMatrix abs_pose;
  if (m_InputVME->GetTagArray()->IsTagPresent(AbsPos_tagName.c_str()))
  {
    mafTagItem *item = m_InputVME->GetTagArray()->GetTag(AbsPos_tagName.c_str());
    for (int r=0; r<4; r++)
      for (int c=0; c<4; c++)
        abs_pose.SetElement(r,c,item->GetComponentAsDouble(comp++));
  }
  else
  {
    return;
  }

  m_InputVME->SetAbsMatrix(abs_pose);

  mafEvent e2s;
  e2s.SetSender(this);
  e2s.SetMatrix(&abs_pose);
  e2s.SetId(ID_TRANSFORM);
  mafEventMacro(e2s);
}
void mafGUISaveRestorePose::StorePoseHelper( mafString absPoseTagName )
{
  assert(m_InputVME);
  mafMatrix *absPose = m_InputVME->GetOutput()->GetAbsMatrix();
  mafTagItem item;
  item.SetName(absPoseTagName);
  item.SetNumberOfComponents(16);
  item.SetComponent(absPose->GetElement(0,0),0);
  item.SetComponent(absPose->GetElement(0,1),1);
  item.SetComponent(absPose->GetElement(0,2),2);
  item.SetComponent(absPose->GetElement(0,3),3);
  item.SetComponent(absPose->GetElement(1,0),4);
  item.SetComponent(absPose->GetElement(1,1),5);
  item.SetComponent(absPose->GetElement(1,2),6);
  item.SetComponent(absPose->GetElement(1,3),7);
  item.SetComponent(absPose->GetElement(2,0),8);
  item.SetComponent(absPose->GetElement(2,1),9);
  item.SetComponent(absPose->GetElement(2,2),10);
  item.SetComponent(absPose->GetElement(2,3),11);
  item.SetComponent(absPose->GetElement(3,0),12);
  item.SetComponent(absPose->GetElement(3,1),13);
  item.SetComponent(absPose->GetElement(3,2),14);
  item.SetComponent(absPose->GetElement(3,3),15);
  m_InputVME->GetTagArray()->SetTag(item);
}

void mafGUISaveRestorePose::RemovePoseHelper( mafString absPoseTagName )
{
  assert(m_InputVME);
  if (m_InputVME->GetTagArray()->IsTagPresent(absPoseTagName.GetCStr()))
    m_InputVME->GetTagArray()->DeleteTag(absPoseTagName.GetCStr());
}