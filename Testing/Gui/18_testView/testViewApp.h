/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: testViewApp.h,v $
  Language:  C++
  Date:      $Date: 2005-06-06 15:11:53 $
  Version:   $Revision: 1.1.1.1 $
  Authors:   Silvano Imboden
==========================================================================
  Copyright (c) 2001/2005 
  CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/
#ifndef __testViewApp_H__
#define __testViewApp_H__
//----------------------------------------------------------------------------
// Include:
//----------------------------------------------------------------------------
#include "mafLogicWithManagers.h" 

// ============================================================================
class testViewApp : public wxApp
// ============================================================================
{
public:
    bool OnInit();
    int  OnExit();
protected:
    mafLogicWithManagers *m_logic;

};

DECLARE_APP(testViewApp)

#endif //__testViewApp_H__







