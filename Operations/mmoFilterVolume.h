/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mmoFilterVolume.h,v $
  Language:  C++
  Date:      $Date: 2005-12-01 11:13:14 $
  Version:   $Revision: 1.1 $
  Authors:   Paolo Quadrani
==========================================================================
Copyright (c) 2002/2004
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __mmoFilterVolume_H__
#define __mmoFilterVolume_H__

#include "mafOp.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class vtkImageData;
class mafEvent;

//----------------------------------------------------------------------------
// mmoFilterVolume :
//----------------------------------------------------------------------------
/** */
class mmoFilterVolume: public mafOp
{
public:
	mmoFilterVolume(wxString label);
	~mmoFilterVolume(); 
	virtual void OnEvent(mafEventBase *maf_event);
	mafOp* Copy();

	/** Return true for the acceptable vme type. */
	bool Accept(mafNode *node);

	/** Builds operation's interface. */
	void OpRun();

	/** Execute the operation. */
	void OpDo();

	/** Makes the undo for the operation. */
	void OpUndo();

protected:
	/** This method is called at the end of the operation and result contain the wxOK or wxCANCEL. */
	void OpStop(int result);
	
	bool m_ClearInterfaceFlag;
	bool m_PreviewResultFlag;

	vtkImageData	*m_ResultImageData;
	vtkImageData	*m_OriginalImageData;

	int    m_Dimensionality;
  double m_SmoothRadius[3];
  double m_StandardDeviation[3];

  int m_KernelSize[3];

  /** Smooth the surface. */
	void OnSmooth();

  /** Execute median filter.*/
  void OnMedian();

	/** Make the preview of the surface filtering. */
	void OnPreview();  

	/** Clear all the surface applied filtering. */
	void OnClear();
};
#endif