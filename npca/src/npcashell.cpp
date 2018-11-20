/*****************************************************************************
 * An EPICS Channel Access plugin for Mozilla
 *
 * Copyright (C) 2007, Cosylab Ltd.
 * $Id: npcashell.cpp 30706 2009-02-17 17:13:05Z msekoranja $
 *
 * Authors: Matej Sekoranja <matej.sekoranja_A_T_cosylab.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

/*****************************************************************************
 * Preamble
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Mozilla stuff */
#ifdef HAVE_MOZILLA_CONFIG_H
#   include <mozilla-config.h>
#endif

#include <jri.h>

#include "plugin.h"

/*****************************************************************************
 * Unix-only declarations
******************************************************************************/
#ifdef XP_UNIX
#endif

/*****************************************************************************
 * MacOS-only declarations
******************************************************************************/
#ifdef XP_MACOSX
#endif

/*****************************************************************************
 * Windows-only declarations
 *****************************************************************************/
#ifdef XP_WIN
#endif

/******************************************************************************
 * UNIX-only API calls
 *****************************************************************************/

char* NPP_GetMIMEDescription(void)
{
  static char * szPLUGIN_MIME_DESC = PLUGIN_MIME_DESC;

  return szPLUGIN_MIME_DESC;
}

NPError	NPP_GetValue(NPP instance, NPPVariable variable, void *value)
{
  static char * szPLUGIN_NAME = PLUGIN_NAME;
  static char * szPLUGIN_DESC = PLUGIN_DESC;

  // handle static variables here
  switch (variable) {

    case NPPVpluginNameString:
      *((char **)value) = szPLUGIN_NAME;
      return NPERR_NO_ERROR;

    case NPPVpluginDescriptionString:
      *((char **)value) = szPLUGIN_DESC;
      return NPERR_NO_ERROR;

    default:
      // move on to instance variables ...
      break;
  }

  if(instance == NULL)
    return NPERR_INVALID_INSTANCE_ERROR;

  CPlugin * plugin = (CPlugin *)instance->pdata;
  if(plugin == NULL)
     return NPERR_INVALID_INSTANCE_ERROR;

  switch (variable) {
    case NPPVpluginScriptableNPObject:
		  *(NPObject **)value = plugin->GetScriptableObject();
		 if (value == NULL)
			return NPERR_OUT_OF_MEMORY_ERROR;
		 return NPERR_NO_ERROR;
    default:
  	  return NPERR_GENERIC_ERROR;
  }

}

/******************************************************************************
 * Mac-only API calls
 *****************************************************************************/

#ifdef XP_MACOSX

int16 NPP_HandleEvent( NPP instance, void * event )
{
	// noop
	return 0;
}

#endif /* XP_MACOSX */

/******************************************************************************
 * General Plug-in Calls
 *****************************************************************************/

#ifdef EMBED_CA_REPEATER
// forward decleration ca_repeater()
epicsShareFunc void ca_repeater ( void );
epicsShareFunc void ca_repeater_stop ( void );

static volatile bool npca_repeater_thread_exited = false;
void npcaRepeaterThread ( void* ) { ca_repeater (); npca_repeater_thread_exited = true; }

#include <envDefs.h>

#define NPCA_REPEATER_PORT 5063
#define NPCA_REPEATER_PORT_STRING STRINGIFY(NPCA_REPEATER_PORT)

void start_ca_repeater()
{
    ENV_PARAM NPCA_CA_REPEATER_PORT = {"NPCA_CA_REPEATER_PORT", NPCA_REPEATER_PORT_STRING};

    unsigned short npcaRepeaterPort = 
        envGetInetPortConfigParam ( &NPCA_CA_REPEATER_PORT,
                                    static_cast <unsigned short> (NPCA_REPEATER_PORT) );
    char sPort[33];
    sprintf(sPort, "%u", npcaRepeaterPort);

    epicsEnvSet (EPICS_CA_REPEATER_PORT.name, sPort);

    epicsThreadId tid = epicsThreadCreate ( "NPCA-repeater", epicsThreadPriorityLow, 
	epicsThreadGetStackSize ( epicsThreadStackMedium ), npcaRepeaterThread, 0);
    if ( tid == 0 ) {
    	fprintf ( stderr, "start_ca_repeater : unable to create CA repeater daemon thread\n" );
    }
}

void stop_ca_repeater() 
{
    ca_repeater_stop();

    // ... and wait for a while until thread exit (simple mechanism used)
    for (int i = 0; i < 10 && !npca_repeater_thread_exited; i++)
        epicsThreadSleep(0.1);
}
#else
void start_ca_repeater() {}
void stop_ca_repeater() {};
#endif

// create CA context
NPError NPP_Initialize(void)
{
// this helps Safari under Win32
#if (XP_WIN && STATIC_BUILD)
    static bool npca_initialized = false;
    if (!npca_initialized)  {
    	// name of the DLL is being forced here
        LoadLibrary ("npca.dll");
        npca_initialized = true;
    }
#endif

    // start embedded CA repeater
    start_ca_repeater();

    int result = ca_context_create(ca_enable_preemptive_callback);
    if (result != ECA_NORMAL) {
        fprintf(stderr, "CA error occurred while trying "
                "to create channel access context: '%s'.\n", ca_message(result));
        return NPERR_GENERIC_ERROR;
    }

    return NPERR_NO_ERROR;
}

// destroy CA context
void NPP_Shutdown(void)
{
    ca_context_destroy();
    
    // stop embedded repeater
    stop_ca_repeater();
    
    // let EPICS threads to exit, as in epicsExit()
    epicsThreadSleep(1.0);
}

jref NPP_GetJavaClass(void)
{
    return NULL;
}

NPError NPP_New(NPMIMEType pluginType, NPP instance, uint16 mode,
                int16 argc, char* argn[], char* argv[],
                NPSavedData* saved)
{
  if (instance == NULL)
    return NPERR_INVALID_INSTANCE_ERROR;

  CPlugin* plugin = new CPlugin(instance);
  if (plugin == NULL)
    return NPERR_OUT_OF_MEMORY_ERROR;

  instance->pdata = (void *)plugin;
  
  NPN_SetValue(instance, NPPVpluginKeepLibraryInMemory, (void *)TRUE);

  return NPERR_NO_ERROR;
}

// here is the place to clean up and destroy the CPlugin object
NPError NPP_Destroy (NPP instance, NPSavedData** save)
{
  if (instance == NULL)
    return NPERR_INVALID_INSTANCE_ERROR;

  CPlugin* plugin = (CPlugin*)instance->pdata;
  if (plugin == NULL)
  	return NPERR_NO_ERROR;

  instance->pdata = NULL;
  delete plugin;

  return NPERR_NO_ERROR;
}

// during this call we know when the plugin window is ready or
// is about to be destroyed so we can do some gui specific
// initialization and shutdown
NPError NPP_SetWindow (NPP instance, NPWindow* pNPWindow)
{
  return NPERR_NO_ERROR;
/*
  if (instance == NULL)
    return NPERR_INVALID_INSTANCE_ERROR;

  if(pNPWindow == NULL)
    return NPERR_GENERIC_ERROR;

  // Opera might call this before New
  CPlugin* plugin = (CPlugin*)instance->pdata;
  if (plugin == NULL)
    return NPERR_NO_ERROR;

  // noop
 
  return NPERR_NO_ERROR;
*/
}

NPError NPP_NewStream(NPP instance,
                      NPMIMEType type, NPStream* stream,
                      NPBool seekable, uint16* stype)
{
  if (instance == NULL)
    return NPERR_INVALID_INSTANCE_ERROR;

  return NPERR_NO_ERROR;
}

int32 NPP_WriteReady (NPP instance, NPStream *stream)
{
  if (instance == NULL)
    return NPERR_INVALID_INSTANCE_ERROR;

  return 0x0fffffff;
}

int32 NPP_Write (NPP instance, NPStream *stream, int32 offset, int32 len, void *buffer)
{
  if (instance == NULL)
    return NPERR_INVALID_INSTANCE_ERROR;

  return len;
}

NPError NPP_DestroyStream (NPP instance, NPStream *stream, NPError reason)
{
  if (instance == NULL)
    return NPERR_INVALID_INSTANCE_ERROR;

  return NPERR_NO_ERROR;
}

void NPP_StreamAsFile (NPP instance, NPStream* stream, const char* fname)
{
}

void NPP_Print (NPP instance, NPPrint* printInfo)
{
}

void NPP_URLNotify(NPP instance, const char* url, NPReason reason, void* notifyData)
{
}

NPError NPP_SetValue(NPP instance, NPNVariable variable, void *value)
{
  if (instance == NULL)
    return NPERR_INVALID_INSTANCE_ERROR;

  return NPERR_NO_ERROR;
}

NPObject *NPP_GetScriptableInstance(NPP instance)
{
  if (instance == NULL)
    return NULL;

  NPObject *npobj = NULL;
  CPlugin * plugin = (CPlugin*)instance->pdata;
  if (plugin != NULL)
    npobj = plugin->GetScriptableObject();

  return npobj;
}

/******************************************************************************
 * Windows-only methods
 *****************************************************************************/

#if XP_WIN
#endif /* XP_WIN */

/******************************************************************************
 * UNIX-only methods
 *****************************************************************************/

#ifdef XP_UNIX
#endif /* XP_UNIX */

