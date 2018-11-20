/*
 * Copyright (c) 2007 by Cosylab
 *
 * The full license specifying the redistribution, modification, usage and other
 * rights and obligations is included with the distribution of this project in
 * the file "LICENSE-NPCA". If the license is not included visit Cosylab web site,
 * <http://www.cosylab.com>.
 *
 * THIS SOFTWARE IS PROVIDED AS-IS WITHOUT WARRANTY OF ANY KIND, NOT EVEN THE
 * IMPLIED WARRANTY OF MERCHANTABILITY. THE AUTHOR OF THIS SOFTWARE, ASSUMES
 * _NO_ RESPONSIBILITY FOR ANY CONSEQUENCE RESULTING FROM THE USE, MODIFICATION,
 * OR REDISTRIBUTION OF THIS SOFTWARE.
 */

/*******************************************************************************
 * Instance state information about the plugin.
 ******************************************************************************/

#ifndef __PLUGIN_H__
#define __PLUGIN_H__

/* Mozilla stuff, used by WIN32 */
#ifdef HAVE_MOZILLA_CONFIG_H
#   include <mozilla-config.h>
#endif

#ifdef XP_WIN
#pragma warning(disable:4786)
#endif

#include <npapi.h>
#include <npruntime.h>

#include "version.h"

/*******************************************************************************/

// used for EPICS to tell we are building linking it as DLL
//#define _DLL

#include <cadef.h>

//#undef _DLL

#include <map>
#include <string>
#include <deque>
#include <vector>
#include <epicsMutex.h>

using namespace std;

typedef uint32_t RegistryEventId;

struct asyncRequest
{
	NPIdentifier name;
	NPVariant *args;
	uint32_t argCount;
	RegistryEventId registryEventId;
};

enum UserEventType { GET_EVENT, GET_CTRL_EVENT, PUT_EVENT, MONITOR_EVENT, CONNECTION_EVENT, ACL_EVENT };
struct eventUserArg
{
	UserEventType type;
	NPP npp;
	NPVariant userInfo;
	NPIdentifier callbackId;
	chid channelId;
	evid eventId;	// EPICS native event id
	NPVariant putValue;
};

typedef vector<eventUserArg*> ConnectionListenerList;
typedef vector<eventUserArg*> ACLListenerList;
typedef map<eventUserArg*, evargs*> EventMap;
typedef vector<asyncRequest*> AsyncRequestList;

class CPlugin
{
private:
  NPP m_pNPInstance;

  NPStream * m_pNPStream;

  NPObject *m_pScriptableObject;

  NPObject *m_pWindowObj;

  typedef map<string, chid> ChannelMap;
  ChannelMap m_channels;

public:
  CPlugin(NPP pNPInstance);
  ~CPlugin();

  NPObject *GetScriptableObject();
  NPObject *getWindowObj() const { return m_pWindowObj; }

  void getChannelCHID(const char *channelName, chid &channel);
  int getChannel(const char* channelName, chid &channel);
  int getChannelAsync(const char* channelName, chid &channel);

  RegistryEventId registerEvent(eventUserArg* usrArg);
  void registerEvent(RegistryEventId id, eventUserArg* usrArg);
  eventUserArg* unregisterEvent(RegistryEventId id);
  bool existsEvent(RegistryEventId id);
  
  void destroyMonitor(eventUserArg* usrArg);

  epicsMutexId eventQueueMutex;
  typedef deque<evargs*> EventQueue;
  EventQueue* eventQueue;
  EventMap* eventMap;
  EventMap* eventDestroyedMap;

  epicsMutexId connectionEventQueueMutex;
  typedef map<chid, ConnectionListenerList*> ConnectionEventQueue;
  ConnectionEventQueue* connectionEventQueueListeners;

  epicsMutexId aclEventQueueMutex;
  typedef map<chid, ACLListenerList*> ACLEventQueue;
  ACLEventQueue* aclEventQueueListeners;

  typedef map<chid, AsyncRequestList*> AsyncRequestMap;
  AsyncRequestMap* asyncRequests;

  epicsMutexId eventRegistryMutex;
  typedef map<RegistryEventId, eventUserArg*> EventRegistryMap;
  EventRegistryMap* eventRegistry;
  RegistryEventId eventRegistryIdGenerator;
};

#endif // __PLUGIN_H__
