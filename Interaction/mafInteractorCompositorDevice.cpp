/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mafInteractorCompositorDevice.cpp,v $
  Language:  C++
  Date:      $Date: 2009-12-17 11:46:39 $
  Version:   $Revision: 1.1.2.1 $
  Authors:   Stefano Perticoni
==========================================================================
  Copyright (c) 2002/2004 
  CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/

#include "wx/wxprec.h" 
#pragma hdrstop

#include "mafDecl.h"
#include "mafInteractionDecl.h"
#include "mafInteractorCompositorDevice.h"
#include "mafDevice.h"

#include "mflDefines.h"
#include "mflEventInteraction.h"

#include "vtkObjectFactory.h"

vtkStandardNewMacro(mafInteractorCompositorDevice);

typedef std::list<std::pair<std::string,mafInteractor *> > mmuInteractorsList;

//----------------------------------------------------------------------------
mafInteractorCompositorDevice::mafInteractorCompositorDevice() 
//----------------------------------------------------------------------------
{
  m_ActiveInteractor = NULL;
  SetLockDevice(false); // do not lock the device (it's locked by composed interactors
}
//----------------------------------------------------------------------------
mafInteractorCompositorDevice::~mafInteractorCompositorDevice() 
//----------------------------------------------------------------------------
{
	// traverse the map and destroy all the behaviors
	for (mmuInteractorsList::iterator iter = m_InteractorsList.begin(); \
    iter != m_InteractorsList.end();iter++)
	{
		vtkDEL((*iter).second);
	}
}

//------------------------------------------------------------------------------
void mafInteractorCompositorDevice::AppendInteractor(mafInteractor *i,const char *device_type)
//------------------------------------------------------------------------------
{
  assert(i);
  assert(device_type);
  assert(!IsInteracting());
  m_InteractorsList.push_back(std::pair<std::string,mafInteractor *>(device_type,i));
  i->Register(this);
  PlugEventSource(i,mafDevice::DeviceOutputChannel);

  if (IsInitialized())
    i->Initialize();
}

//------------------------------------------------------------------------------
void mafInteractorCompositorDevice::PrependInteractor(mafInteractor *i,const char *device_type)
//------------------------------------------------------------------------------
{
  assert(i);
  assert(device_type);
  assert(!IsInteracting());
  m_InteractorsList.push_front(std::pair<std::string,mafInteractor *>(device_type,i));
  i->Register(this);
  PlugEventSource(i,mafDevice::DeviceOutputChannel);

  if (IsInitialized())
    i->Initialize();
}

//------------------------------------------------------------------------------
void mafInteractorCompositorDevice::RemoveInteractor(mafInteractor *i)
//------------------------------------------------------------------------------
{
  assert(i);
  assert(!IsInteracting());
  
  
  for (mmuInteractorsList::iterator it=m_InteractorsList.begin();it!=m_InteractorsList.end();it++)
  {
    if (it->second==i)
    {
      i->Shutdown();
      UnPlugEventSource(i);
      i->UnRegister(this);
      m_InteractorsList.erase(it);
      return;
    }
  }
}

//------------------------------------------------------------------------------
int mafInteractorCompositorDevice::InternalInitialize()
//------------------------------------------------------------------------------
{
  for (mmuInteractorsList::iterator it=m_InteractorsList.begin();it!=m_InteractorsList.end();it++)
  {
    if (it->second->Initialize())
    { 
      InternalShutdown(); // shutdown all interactors already initialized
      return -1;
    }
  }

  if (Superclass::InternalInitialize())
  {
    InternalShutdown(); // shutdown all interactors already initialized
    return -1;
  }

  return 0;
}
//------------------------------------------------------------------------------
void mafInteractorCompositorDevice::InternalShutdown()
//------------------------------------------------------------------------------
{
  Superclass::InternalShutdown();
  
  for (mmuInteractorsList::iterator it=m_InteractorsList.begin();it!=m_InteractorsList.end();it++)
  {
    it->second->Shutdown();
  }
}
//------------------------------------------------------------------------------
void mafInteractorCompositorDevice::ProcessEvent(mflEvent *event,mafID channel)
//------------------------------------------------------------------------------
{
  assert(event);
  assert(event->GetSender());
  
  unsigned int id=event->GetID();

  if (channel == mafDevice::DeviceInputChannel)
  {
    ///////////////////////////////////////////////
    // pass input events to composed interactors //
    ///////////////////////////////////////////////

    if ((m_ActiveInteractor!=NULL)&&(m_ActiveInteractor->GetDevice()==event->GetSender()))
    {
      // if an interactor is already interacting pass to it
      m_ActiveInteractor->ProcessEvent(event,channel);

      if (m_ActiveInteractor==NULL||!m_ActiveInteractor->IsInteracting())
      {
        // interactor has stopped its interaction
        mafDevice *device = (mafDevice *)event->GetSender();
        StopInteraction(device);
        m_ActiveInteractor = NULL;
      }
    }
    else
    {
      mafDevice *device = mafDevice::SafeDownCast((vtkObject *)event->GetSender());
      if (device)
      {
        // find the first interactor that can accept the event and start the interaction
        for (mmuInteractorsList::iterator it=m_InteractorsList.begin();it!=m_InteractorsList.end();it++)
        {
          // if the source is of the type associated with interactor, forward the event to the interactor
          if (device->IsA(it->first.c_str()))
          {
            it->second->ProcessEvent(event,channel);
            // if the interactor has started interacting, start the compositor interaction too to lock the compositor
            if (it->second->IsInteracting(device))
            {
              StartInteraction(device);
              m_ActiveInteractor = it->second;
            }
          }
        }
      }
    }    
  }
  else
  {
    InvokeEvent(event,channel);
  }
}
//------------------------------------------------------------------------------