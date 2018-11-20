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

//////////////////////////////////////////////////
//
// CPlugin class implementation
//
#include "plugin.h"
#include <npupp.h>

#include <epicsVersion.h>

#include <algorithm>

/*****************************************************************************
 * Array support
 ******************************************************************************/

typedef NPObject NPArray;
NPArray *NPN_CreateArray (NPP npp, NPVariantType type, void* data, uint32_t count, bool release, uint32_t maxStringSize);

/*****************************************************************************
 * CTRL support
 ******************************************************************************/

typedef NPObject NP_CTRL;
NP_CTRL *CreateCTRL (NPP npp, short dbrType, const void* dbr, bool release);

/******************************************************************************/

static char*
stringValue(const NPString &s)
{
	char* str = (char*)NPN_MemAlloc(s.utf8length+1);
	if (str) {
		strncpy(str, s.utf8characters, s.utf8length);
        str[s.utf8length] = '\0';
	}
    return str;
}

static char*
stringValue(const NPVariant &v)
{
    char *s = NULL;
    if( NPVARIANT_IS_STRING(v) )
    {
        return stringValue(NPVARIANT_TO_STRING(v));
    }
    return s;
}

static int
numberValue(const NPVariant &v)
{
	switch( v.type ) {
		case NPVariantType_Int32:
			return NPVARIANT_TO_INT32(v);
		case NPVariantType_Double:
			return(int)NPVARIANT_TO_DOUBLE(v);
		default:
			return 0;
	}
}

static char*
stringDup(const char* s)
{
	if (!s) return 0;
	char* str = (char*)NPN_MemAlloc(strlen(s)+1);
	if (str) strcpy(str, s);
	return str;
}

/******************************************************************************/

// Helper class that can be used to map calls to the NPObject hooks
// into virtual methods on instances of classes that derive from this
// class.
class ScriptablePluginObjectBase : public NPObject
{
public:
  ScriptablePluginObjectBase(NPP npp)
    : mNpp(npp)
  {
  }

  virtual ~ScriptablePluginObjectBase()
  {
  }

  // Virtual NPObject hooks called through this base class. Override
  // as you see fit.
  virtual void Invalidate();
  virtual bool HasMethod(NPIdentifier name);
  virtual bool Invoke(NPIdentifier name, const NPVariant *args,
                      uint32_t argCount, NPVariant *result);
  virtual bool InvokeDefault(const NPVariant *args, uint32_t argCount,
                             NPVariant *result);
  virtual bool HasProperty(NPIdentifier name);
  virtual bool GetProperty(NPIdentifier name, NPVariant *result);
  virtual bool SetProperty(NPIdentifier name, const NPVariant *value);
  virtual bool RemoveProperty(NPIdentifier name);

public:
  static void _Deallocate(NPObject *npobj);
  static void _Invalidate(NPObject *npobj);
  static bool _HasMethod(NPObject *npobj, NPIdentifier name);
  static bool _Invoke(NPObject *npobj, NPIdentifier name,
                      const NPVariant *args, uint32_t argCount,
                      NPVariant *result);
  static bool _InvokeDefault(NPObject *npobj, const NPVariant *args,
                             uint32_t argCount, NPVariant *result);
  static bool _HasProperty(NPObject * npobj, NPIdentifier name);
  static bool _GetProperty(NPObject *npobj, NPIdentifier name,
                           NPVariant *result);
  static bool _SetProperty(NPObject *npobj, NPIdentifier name,
                           const NPVariant *value);
  static bool _RemoveProperty(NPObject *npobj, NPIdentifier name);

protected:
  NPP mNpp;
};

#define DECLARE_NPOBJECT_CLASS_WITH_BASE(_class, ctor)                        \
static NPClass s##_class##_NPClass = {                                        \
  NP_CLASS_STRUCT_VERSION,                                                    \
  ctor,                                                                       \
  ScriptablePluginObjectBase::_Deallocate,                                    \
  ScriptablePluginObjectBase::_Invalidate,                                    \
  ScriptablePluginObjectBase::_HasMethod,                                     \
  ScriptablePluginObjectBase::_Invoke,                                        \
  ScriptablePluginObjectBase::_InvokeDefault,                                 \
  ScriptablePluginObjectBase::_HasProperty,                                   \
  ScriptablePluginObjectBase::_GetProperty,                                   \
  ScriptablePluginObjectBase::_SetProperty,                                   \
  ScriptablePluginObjectBase::_RemoveProperty                                 \
}

#define GET_NPOBJECT_CLASS(_class) &s##_class##_NPClass

void
ScriptablePluginObjectBase::Invalidate()
{
}

bool
ScriptablePluginObjectBase::HasMethod(NPIdentifier name)
{
  return false;
}

bool
ScriptablePluginObjectBase::Invoke(NPIdentifier name, const NPVariant *args,
                                   uint32_t argCount, NPVariant *result)
{
  return false;
}

bool
ScriptablePluginObjectBase::InvokeDefault(const NPVariant *args,
                                          uint32_t argCount, NPVariant *result)
{
  return false;
}

bool
ScriptablePluginObjectBase::HasProperty(NPIdentifier name)
{
  return false;
}

bool
ScriptablePluginObjectBase::GetProperty(NPIdentifier name, NPVariant *result)
{
  VOID_TO_NPVARIANT(*result);

  return false;
}

bool
ScriptablePluginObjectBase::SetProperty(NPIdentifier name,
                                        const NPVariant *value)
{
  return false;
}

bool
ScriptablePluginObjectBase::RemoveProperty(NPIdentifier name)
{
  return false;
}

// static
void
ScriptablePluginObjectBase::_Deallocate(NPObject *npobj)
{
  // Call the virtual destructor.
  delete (ScriptablePluginObjectBase *)npobj;
}

// static
void
ScriptablePluginObjectBase::_Invalidate(NPObject *npobj)
{
  ((ScriptablePluginObjectBase *)npobj)->Invalidate();
}

// static
bool
ScriptablePluginObjectBase::_HasMethod(NPObject *npobj, NPIdentifier name)
{
  return ((ScriptablePluginObjectBase *)npobj)->HasMethod(name);
}

// static
bool
ScriptablePluginObjectBase::_Invoke(NPObject *npobj, NPIdentifier name,
                                    const NPVariant *args, uint32_t argCount,
                                    NPVariant *result)
{
  return ((ScriptablePluginObjectBase *)npobj)->Invoke(name, args, argCount,
                                                       result);
}

// static
bool
ScriptablePluginObjectBase::_InvokeDefault(NPObject *npobj,
                                           const NPVariant *args,
                                           uint32_t argCount,
                                           NPVariant *result)
{
  return ((ScriptablePluginObjectBase *)npobj)->InvokeDefault(args, argCount,
                                                              result);
}

// static
bool
ScriptablePluginObjectBase::_HasProperty(NPObject * npobj, NPIdentifier name)
{
  return ((ScriptablePluginObjectBase *)npobj)->HasProperty(name);
}

// static
bool
ScriptablePluginObjectBase::_GetProperty(NPObject *npobj, NPIdentifier name,
                                         NPVariant *result)
{
  return ((ScriptablePluginObjectBase *)npobj)->GetProperty(name, result);
}

// static
bool
ScriptablePluginObjectBase::_SetProperty(NPObject *npobj, NPIdentifier name,
                                         const NPVariant *value)
{
  return ((ScriptablePluginObjectBase *)npobj)->SetProperty(name, value);
}

// static
bool
ScriptablePluginObjectBase::_RemoveProperty(NPObject *npobj, NPIdentifier name)
{
  return ((ScriptablePluginObjectBase *)npobj)->RemoveProperty(name);
}


static NPIdentifier getPV_id = NULL;
static NPIdentifier getCTRL_id = NULL;
static NPIdentifier putPV_id = NULL;
static NPIdentifier monitorPV_id = NULL;
static NPIdentifier monitorWritableStatus_id = NULL;
static NPIdentifier monitorConnectionStatus_id = NULL;
static NPIdentifier destroyMonitor_id = NULL;
static NPIdentifier pendEvents_id = NULL;

static NPIdentifier version_id = NULL;
static NPIdentifier epicsVersion_id = NULL;

class ScriptablePluginObject : public ScriptablePluginObjectBase
{
public:
  ScriptablePluginObject(NPP npp)
    : ScriptablePluginObjectBase(npp)
  {
	  if (monitorPV_id == NULL)
		monitorPV_id = NPN_GetStringIdentifier("monitorPV");

	  if (monitorWritableStatus_id == NULL)
		monitorWritableStatus_id = NPN_GetStringIdentifier("monitorWritableStatus");

	  if (monitorConnectionStatus_id == NULL)
		monitorConnectionStatus_id = NPN_GetStringIdentifier("monitorConnectionStatus");

	  if (destroyMonitor_id == NULL)
		destroyMonitor_id = NPN_GetStringIdentifier("destroyMonitor");

	  if (getPV_id == NULL)
		getPV_id = NPN_GetStringIdentifier("getPV");

	  if (getCTRL_id == NULL)
		getCTRL_id = NPN_GetStringIdentifier("getCTRL");

	  if (putPV_id == NULL)
		putPV_id = NPN_GetStringIdentifier("putPV");

	  if (pendEvents_id == NULL)
		pendEvents_id = NPN_GetStringIdentifier("pendEvents");

	  if (version_id == NULL)
		version_id = NPN_GetStringIdentifier("version");

	  if (epicsVersion_id == NULL)
		epicsVersion_id = NPN_GetStringIdentifier("epicsVersion");
  }

  virtual bool HasProperty(NPIdentifier name);
  virtual bool GetProperty(NPIdentifier name, NPVariant *result);

  virtual bool HasMethod(NPIdentifier name);
  virtual bool Invoke(NPIdentifier name, const NPVariant *args,
                      uint32_t argCount, NPVariant *result);
  virtual bool InvokeDefault(const NPVariant *args, uint32_t argCount,
                             NPVariant *result);
};

static NPObject *
AllocateScriptablePluginObject(NPP npp, NPClass *aClass)
{
  return new ScriptablePluginObject(npp);
}

DECLARE_NPOBJECT_CLASS_WITH_BASE(ScriptablePluginObject,
                                 AllocateScriptablePluginObject);


bool
ScriptablePluginObject::HasMethod(NPIdentifier name)
{
  return name == pendEvents_id || name == monitorPV_id || name == destroyMonitor_id ||
  		 name == getPV_id || name == getCTRL_id || name == putPV_id || name == monitorConnectionStatus_id ||
  		 name == monitorWritableStatus_id;
}

bool
ScriptablePluginObject::HasProperty(NPIdentifier name)
{
  return name == version_id || name == epicsVersion_id;
}

bool
ScriptablePluginObject::GetProperty(NPIdentifier name, NPVariant *result)
{
	if (name == version_id)
	{
		STRINGZ_TO_NPVARIANT(stringDup(NPCA_VERSION_STRING), *result);
		return true;
	}
	else if (name == epicsVersion_id)
	{
		STRINGZ_TO_NPVARIANT(stringDup(EPICS_VERSION_STRING), *result);
		return true;
	}

	VOID_TO_NPVARIANT(*result);
	return false;
}


/*+**************************************************************************
 *
 * Function:	connection_handler
 *
 * Description:	CA connection_handler
 *
 * Arg(s) In:	args  -  connection_handler_args (see CA manual)
 *
 **************************************************************************-*/

#define NPCA_DIRTY_EVENT -1
#define NPCA_CONNECTION_EVENT -2

void push_connection_event(CPlugin* plugin, eventUserArg* originalUsrArg, connection_handler_args& args)
{

	// connection_handler_args -> evargs event
	evargs* event = new evargs();

	// copy (to be released when event is destroyed)
	eventUserArg* usrArg = new eventUserArg();
	*usrArg = *originalUsrArg;
	if (NPVARIANT_IS_OBJECT(usrArg->userInfo))
		NPN_RetainObject(NPVARIANT_TO_OBJECT(usrArg->userInfo));
	else if (NPVARIANT_IS_STRING(usrArg->userInfo))
		STRINGZ_TO_NPVARIANT(stringValue(usrArg->userInfo), usrArg->userInfo);
	event->usr = usrArg;

	event->chid = args.chid;
	event->status = args.op;

	// copy to queue
	epicsMutexLock(plugin->eventQueueMutex);
	plugin->eventQueue->push_back(event);
	epicsMutexUnlock(plugin->eventQueueMutex);
}

void connection_handler (connection_handler_args args)
{
    NPP npp = (NPP)ca_puser (args.chid);
    if (npp == NULL)
    	return;
	CPlugin* plugin = (CPlugin *)npp->pdata;
	if (plugin == NULL)
		return;

    // copy to queue
	epicsMutexLock(plugin->connectionEventQueueMutex);
	// add only if monitored
	if (plugin->connectionEventQueueListeners->find(args.chid) != plugin->connectionEventQueueListeners->end())
	{
		ConnectionListenerList* list = (*(plugin->connectionEventQueueListeners))[args.chid];

		ConnectionListenerList::iterator iter = list->begin();
		for(; iter != list->end(); iter++)
			push_connection_event(plugin, *iter, args);
	}

	// if connected and if we have some async requests pending
	if (args.op == CA_OP_CONN_UP && plugin->asyncRequests->find(args.chid) != plugin->asyncRequests->end())
	{
		// connection_handler_args -> evargs event
		evargs* event = new evargs();
		event->chid = args.chid;
		event->dbr = NULL;
		event->status = NPCA_CONNECTION_EVENT;

		// copy to queue
		epicsMutexLock(plugin->eventQueueMutex);
		plugin->eventQueue->push_back(event);
		epicsMutexUnlock(plugin->eventQueueMutex);
	}
	epicsMutexUnlock(plugin->connectionEventQueueMutex);

}


/*+**************************************************************************
 *
 * Function:	access_rights_handler
 *
 * Description:	CA access_rights_handler
 *
 * Arg(s) In:	args  -  access_rights_handler_args (see CA manual)
 *
 **************************************************************************-*/

void push_acl_event(CPlugin* plugin, eventUserArg* originalUsrArg, access_rights_handler_args& args)
{

	// connection_handler_args -> evargs event
	evargs* event = new evargs();

	// copy (to be released when event is destroyed)
	eventUserArg* usrArg = new eventUserArg();
	*usrArg = *originalUsrArg;
	if (NPVARIANT_IS_OBJECT(usrArg->userInfo))
		NPN_RetainObject(NPVARIANT_TO_OBJECT(usrArg->userInfo));
	else if (NPVARIANT_IS_STRING(usrArg->userInfo))
		STRINGZ_TO_NPVARIANT(stringValue(usrArg->userInfo), usrArg->userInfo);
	event->usr = usrArg;

	event->chid = args.chid;
	event->status = args.ar.write_access;

	// copy to queue
	epicsMutexLock(plugin->eventQueueMutex);
	plugin->eventQueue->push_back(event);
	epicsMutexUnlock(plugin->eventQueueMutex);
}

void access_rights_handler (access_rights_handler_args args)
{
    NPP npp = (NPP)ca_puser (args.chid);
    if (npp == NULL)
    	return;
	CPlugin* plugin = (CPlugin *)npp->pdata;
	if (plugin == NULL)
		return;

    // copy to queue
	epicsMutexLock(plugin->aclEventQueueMutex);
	// add only if monitored
	if (plugin->aclEventQueueListeners->find(args.chid) != plugin->aclEventQueueListeners->end())
	{
		ACLListenerList* list = (*(plugin->aclEventQueueListeners))[args.chid];

		ACLListenerList::iterator iter = list->begin();
		for(; iter != list->end(); iter++)
			push_acl_event(plugin, *iter, args);
	}
	epicsMutexUnlock(plugin->aclEventQueueMutex);
}

/*+**************************************************************************
 *
 * Function:	event_handler
 *
 * Description:	CA event_handler for request type callback
 * 		Allocates the dbr structure and copies the data
 *
 * Arg(s) In:	args  -  event handler args (see CA manual)
 *
 **************************************************************************-*/

void event_handler (evargs event)
{
    eventUserArg* usrArg = (eventUserArg*)event.usr;
	CPlugin* plugin = (CPlugin *)usrArg->npp->pdata;
	if (plugin == NULL)
		return;

	UserEventType type = usrArg->type;
	bool putEvent = (type == PUT_EVENT);

	// always queue put events (to notify put failure)
    if (event.status == ECA_NORMAL || putEvent)
    {
		// make a copy of value
		if (event.dbr != NULL && !putEvent)
		{
			// allocate memory for value
			unsigned size = dbr_size_n(event.type, event.count);
			void* value = NPN_MemAlloc(size);
			memcpy(value, event.dbr, size);

			// fix the original dbr pointer to ours
			event.dbr = value;
		}
		else
			event.dbr = NULL;


		// copy to queue
		epicsMutexLock(plugin->eventQueueMutex);

		// delete all destroyed events
		bool eventCanceled = false;
		EventMap::iterator iter = plugin->eventDestroyedMap->begin();
		for (; iter != plugin->eventDestroyedMap->end(); iter++)
		{
			eventUserArg *arg = iter->first;
			if (arg == usrArg)
				eventCanceled = true;

			NPN_ReleaseVariantValue(&arg->userInfo);
			delete arg;
		}
		plugin->eventDestroyedMap->clear();

		if (eventCanceled)
		{
			if (event.dbr != NULL)
				NPN_MemFree((void*)event.dbr);
			epicsMutexUnlock(plugin->eventQueueMutex);
			return;
		}

		evargs* ev;
		// monitor (override old values)
		bool override = false;
		if (type == MONITOR_EVENT)
		{
			if (plugin->eventMap->find(usrArg) != plugin->eventMap->end()) {
				// override old monitor event
				ev = (*(plugin->eventMap))[usrArg];
				// free old value
				NPN_MemFree((void*)ev->dbr);
				override = true;
			}
			else
			{
				// create and map new monitor event
				ev = new evargs();
				(*(plugin->eventMap))[usrArg] = ev;
			}
		}
		else
			ev = new evargs();
		*ev = event;

		if (!override)
			plugin->eventQueue->push_back(ev);
		epicsMutexUnlock(plugin->eventQueueMutex);
	}
}

short
getSupportedChannelType(chid &ch, bool time)
{
	switch (ca_field_type(ch))
	{
		//case DBR_INT:
		case DBR_SHORT:
		case DBR_CHAR:
		case DBR_LONG:
			return (time ? DBR_TIME_LONG : DBR_LONG);

		case DBR_FLOAT:
		case DBR_DOUBLE:
			return (time ? DBR_TIME_DOUBLE : DBR_DOUBLE);

		case DBR_ENUM:
		case DBR_STRING:
		default:
			return (time ? DBR_TIME_STRING : DBR_STRING);
	}
}

short
getSupportedCTRLChannelType(chid &ch)
{
	if (ca_field_type(ch) == DBR_ENUM)
		return DBR_CTRL_ENUM;		// special case
	else
		// 28 is offset from basic DBR to CTRL types
		return getSupportedChannelType(ch, false) + 28;
}

int
parseArrayObject(NPP npp, NPObject *obj, short &type, int &count, void* &data)
{
    /* NOTE: WebKit (Safari) does not implement NPN_HasProperty/NPN_HasMethod */

    NPVariant value;

    type = 0;
    count = 0;
    data = NULL;

	if (obj == NULL)
		return 1;

    /* we are expecting to have a Javascript Array object */
    NPIdentifier propId = NPN_GetStringIdentifier("length");
    if( NPN_GetProperty(npp, obj, propId, &value) )
    {
        count = numberValue(value);
        NPN_ReleaseVariantValue(&value);

        if( count )
        {
                int nOptions = 0;

                while( nOptions < count )
                {
                    propId = NPN_GetIntIdentifier(nOptions);
                    if( ! NPN_GetProperty(npp, obj, propId, &value) )
                        /* return what we got so far */
                        break;

					// initialize data and type
					if (nOptions == 0)
					{
						// first element can't tell the exact type (int vs double)
						if (NPVARIANT_IS_INT32(value) || NPVARIANT_IS_DOUBLE(value))
						{
							type = DBR_DOUBLE;
							data = NPN_MemAlloc(count*sizeof(double));
						}
						else if (NPVARIANT_IS_STRING(value))
						{
							type = DBR_STRING;
							data = NPN_MemAlloc(count*MAX_STRING_SIZE*sizeof(char));
						}
					}

					bool error = false;
					switch (type)
					{
						case DBR_LONG:
						case DBR_DOUBLE:
							double val;
							if (NPVARIANT_IS_INT32(value))
								val = (double)NPVARIANT_TO_INT32(value);
							else if (NPVARIANT_IS_DOUBLE(value))
								val = NPVARIANT_TO_DOUBLE(value);
							else
							{
								error = true;
								break;
							}

							((double*)data)[nOptions] = val;
							break;
						case DBR_STRING:
							if (!NPVARIANT_IS_STRING(value))
							{
								error = true;
								break;
							}

							{
								// copy value
								char * pos = ((char*)data) + nOptions * MAX_STRING_SIZE;
								NPString s = NPVARIANT_TO_STRING(value);
        						strncpy(pos, s.utf8characters, s.utf8length);
        						pos[s.utf8length] = '\0';
							}
							break;
						default:
							error = true;
					}

					NPN_ReleaseVariantValue(&value);

					if (error)
					{
						if (data)
							NPN_MemFree(data);
						return 1;
					}

					nOptions++;
                }

                return data ? 0 : 1;
		}
    }

    return 1;
}

bool
ScriptablePluginObject::Invoke(NPIdentifier name, const NPVariant *args,
                               uint32_t argCount, NPVariant *result)
{
	CPlugin * pPlugin = (CPlugin *)mNpp->pdata;
	if (pPlugin == NULL)
		return PR_FALSE;

	// these requests are very similar...
	if (name == monitorPV_id || name == getPV_id || name == getCTRL_id || name == putPV_id ||
		name == monitorConnectionStatus_id || name == monitorWritableStatus_id)
	{
		UserEventType type;
		if (name == monitorPV_id)
			type = MONITOR_EVENT;
		else if (name == getPV_id)
			type = GET_EVENT;
		else if (name == getCTRL_id)
			type = GET_CTRL_EVENT;
		else if (name == putPV_id)
			type = PUT_EVENT;
		else if (name == monitorConnectionStatus_id)
			type = CONNECTION_EVENT;
		else //if (name == monitorWritableStatus_id)
			type = ACL_EVENT;

	    // MONITOR/GET: string channelName, string callbackName, variant userInfo
	    // PUT: string channelName, string callbackName (can be null), variant userInfo, array values
		const uint32_t argCountRequired = (type == PUT_EVENT) ? 4 : 3;

		// argument count check
		if (argCount != argCountRequired)
			return PR_FALSE;

		// preset return error indicator event id
		const bool asyncCall = NPVARIANT_IS_INT32(*result);
		if (!asyncCall && (type == MONITOR_EVENT || type == CONNECTION_EVENT || type == ACL_EVENT))
			INT32_TO_NPVARIANT(0, *result);

		//
		// get channelName
		//
		char *channelName = NULL;
		if (NPVARIANT_IS_STRING(args[0]))
			channelName = stringValue(NPVARIANT_TO_STRING(args[0]));
		else
			return PR_FALSE;

		//
		// get callbackName
		//
		char *callbackName = NULL;
		if (NPVARIANT_IS_STRING(args[1]))
			callbackName = stringValue(NPVARIANT_TO_STRING(args[1]));
		// allow NULL callbacks for PUTs
		else if (type != PUT_EVENT)
		{
			NPN_MemFree(channelName);
			return PR_FALSE;
		}

		//
		// get callback object
		//
		NPObject* callback = NULL;
		NPIdentifier callbackName_id = NULL;
		if (callbackName != NULL)
		{
			callbackName_id = NPN_GetStringIdentifier(callbackName);

			NPVariant cb;
			NPN_GetProperty(mNpp, pPlugin->getWindowObj(), callbackName_id, &cb);

			// create callback object (to be released when monitor is destroyed)
			callback = NPVARIANT_IS_OBJECT(cb) ? NPVARIANT_TO_OBJECT(cb) : NULL;
			if (callback == NULL)
			{
				printf("warning: callback '%s' is not defined\n", callbackName);
				NPN_MemFree(channelName);
				NPN_MemFree(callbackName);
				return PR_TRUE;
			}
			// release cb (callback), keep callback only as boolean
			NPN_ReleaseVariantValue(&cb);
		}

		//
		// if channel is not yet created, make request async
		//
		if (type == MONITOR_EVENT || type == GET_EVENT || type == GET_CTRL_EVENT || type == PUT_EVENT)
		{
			chid ch;
			int res = pPlugin->getChannelAsync(channelName, ch);
			if (res != ECA_NORMAL)
			{
				printf("failed to get channel %s: %s\n", channelName, ca_message(res));
				NPN_MemFree(channelName);
				if (callbackName)
					NPN_MemFree(callbackName);
				return PR_TRUE;
			}
			else
			{
				epicsMutexLock(pPlugin->connectionEventQueueMutex);
				bool doAsync = (ca_state(ch) != cs_conn);
			    if (doAsync)
				{
					asyncRequest* request = new asyncRequest();
					request->name = name;
					request->argCount = argCount;
					request->args = new NPVariant[argCount];
					for (uint32_t i = 0; i < argCount; i++)
					{
						request->args[i] = args[i];
						// copy
						if (NPVARIANT_IS_OBJECT(request->args[i]))
							NPN_RetainObject(NPVARIANT_TO_OBJECT(request->args[i]));
						else if (NPVARIANT_IS_STRING(request->args[i]))
							STRINGZ_TO_NPVARIANT(stringValue(request->args[i]), request->args[i]);
					}
					// preallocation event id and return
					if (type == MONITOR_EVENT /*|| type == CONNECTION_EVENT || type == ACL_EVENT*/)
					{
						request->registryEventId = pPlugin->registerEvent(0);
						INT32_TO_NPVARIANT(request->registryEventId, *result);
					}
					else
						request->registryEventId = 0;

					if (pPlugin->asyncRequests->find(ch) == pPlugin->asyncRequests->end())
					{
						// create new list and add
						AsyncRequestList* list = new AsyncRequestList();
						list->push_back(request);
						(*(pPlugin->asyncRequests))[ch] = list;
					}
					else
					{
						// add to list
						(*(pPlugin->asyncRequests))[ch]->push_back(request);
					}
				}
				epicsMutexUnlock(pPlugin->connectionEventQueueMutex);

				if (doAsync)
				{
					NPN_MemFree(channelName);
					if (callbackName)
						NPN_MemFree(callbackName);
					return PR_TRUE;
				}
			}
		}

		//
		// initialize usrArgs, if necessary
		//
        NPVariant userInfo = args[2];
		eventUserArg* usrArgs = NULL;

		if (callback)
		{
			// copy (to be released when event is destroyed)
			if (NPVARIANT_IS_OBJECT(userInfo))
				NPN_RetainObject(NPVARIANT_TO_OBJECT(userInfo));
			else if (NPVARIANT_IS_STRING(userInfo))
				STRINGZ_TO_NPVARIANT(stringValue(userInfo), userInfo);

			// create user args (to be released when event is destroyed)
			usrArgs = new eventUserArg();
			usrArgs->npp = mNpp;
			usrArgs->userInfo = userInfo;
			usrArgs->callbackId = callbackName_id;
			usrArgs->type = type;
			usrArgs->channelId = 0;
			usrArgs->eventId = 0;
			VOID_TO_NPVARIANT(usrArgs->putValue);
		}

		bool failed = false;

		//
		// get (create) channel
		//
		chid ch;
		int res = pPlugin->getChannelAsync(channelName, ch);
		if (res != ECA_NORMAL)
		{
			printf("failed to get channel %s: %s\n", channelName, ca_message(res));
			failed = true;
		}
		else
		{
			if (usrArgs)
				usrArgs->channelId = ch;

			if (type == MONITOR_EVENT)
			{
				// check if already destroyed
				if (asyncCall && !pPlugin->existsEvent(NPVARIANT_TO_INT32(*result)))
					failed = true;
				else
				{
					res = ca_create_subscription(getSupportedChannelType(ch, true),
												 ca_element_count(ch),
												 ch,
												 DBE_VALUE | DBE_ALARM,
												 event_handler,
												 (void*)usrArgs,
												 &usrArgs->eventId);
					if (res != ECA_NORMAL)
					{
						printf("failed to create subscription for channel %s: %s\n", channelName, ca_message(res));
						failed = true;
					}
					else
						ca_flush_io();
				}
			}
			else if (type == GET_EVENT || type == GET_CTRL_EVENT)
			{
				short dbrType = (type == GET_CTRL_EVENT ? getSupportedCTRLChannelType(ch) : getSupportedChannelType(ch, true));
				res = ca_array_get_callback(dbrType,
											 ca_element_count(ch),
											 ch,
											 event_handler,
											 (void*)usrArgs);

				if (res != ECA_NORMAL)
				{
					printf("failed to create subscription for channel %s: %s\n", channelName, ca_message(res));
					failed = true;
				}
				else
					ca_flush_io();
			}
			else if (type == PUT_EVENT)
			{
				short type;
				int count;
				void* data;
				int int32Val;
				double doubleVal;
				bool release = true;
				res = 0;

				if (NPVARIANT_IS_OBJECT(args[3]))
				{
					res = parseArrayObject(mNpp, NPVARIANT_TO_OBJECT(args[3]), type, count, data);
				}
				else if (NPVARIANT_IS_INT32(args[3]))
				{
					type = DBR_LONG;
					count = 1;
					int32Val = NPVARIANT_TO_INT32(args[3]);
					data = &int32Val;
					release = false;
				}
				else if (NPVARIANT_IS_DOUBLE(args[3]))
				{
					type = DBR_DOUBLE;
					count = 1;
					doubleVal = NPVARIANT_TO_DOUBLE(args[3]);
					data = &doubleVal;
					release = false;
				}
				else if (NPVARIANT_IS_STRING(args[3]))
				{
					type = DBR_STRING;
					count = 1;
					data = stringValue(args[3]);
				}
				else
					res = 1;

				if (res != 0)
				{
					printf("failed to parse values to put for channel %s\n", channelName);
					failed = true;
				}
				else
				{
					if (callback != NULL)
					{
						// store put value, copy if necessary
						usrArgs->putValue = args[3];
						if (NPVARIANT_IS_OBJECT(args[3]))
							NPN_RetainObject(NPVARIANT_TO_OBJECT(usrArgs->putValue));
						else if (NPVARIANT_IS_STRING(args[3]))
							STRINGZ_TO_NPVARIANT(stringValue(args[3]), usrArgs->putValue);

						 res = ca_array_put_callback(type,
													 count,
													 ch,
													 data,
													 event_handler,
													 (void*)usrArgs);
					}
					else
					{
						 res = ca_array_put(type,
											count,
											ch,
											data);
					}

					if (res != ECA_NORMAL)
					{
						printf("failed to put value for channel %s: %s\n", channelName, ca_message(res));
						failed = true;

						// if in callback mode, notify about the error
						if (callback != NULL)
						{
							// userInfo, pvName, values, CA status
							NPVariant params[4];

							params[0] = usrArgs->userInfo;
							STRINGZ_TO_NPVARIANT(channelName, params[1]);
							params[2] = args[3];
							INT32_TO_NPVARIANT(res, params[3]);

							//
							// invoke the callback
							// NOTE: WebKit (Safari) does not implement NPN_InvokeDefault on JS objects
							//
							NPVariant retVal;
							NPN_Invoke(mNpp, pPlugin->getWindowObj(), usrArgs->callbackId, params, 4, &retVal);
							NPN_ReleaseVariantValue(&retVal);
						}
					}
					else
						ca_flush_io();

					if (release)
					{
						NPN_MemFree(data);
					}
				}
			}
			else if (type == CONNECTION_EVENT)
			{
				// add listener
				epicsMutexLock(pPlugin->connectionEventQueueMutex);
				if (pPlugin->connectionEventQueueListeners->find(ch) == pPlugin->connectionEventQueueListeners->end())
				{
					// create new list and add
					ConnectionListenerList* list = new ConnectionListenerList();
					list->push_back(usrArgs);
					(*(pPlugin->connectionEventQueueListeners))[ch] = list;
				}
				else
				{
					// add to list
					(*(pPlugin->connectionEventQueueListeners))[ch]->push_back(usrArgs);
				}

				// if already connected fire event
				if (ca_state(ch) == cs_conn)
				{
					connection_handler_args args = { ch, CA_OP_CONN_UP };
					push_connection_event(pPlugin, usrArgs, args);
				}
				epicsMutexUnlock(pPlugin->connectionEventQueueMutex);
			}
			else if (type == ACL_EVENT)
			{
				// add listener
				epicsMutexLock(pPlugin->aclEventQueueMutex);
				if (pPlugin->aclEventQueueListeners->find(ch) == pPlugin->aclEventQueueListeners->end())
				{
					// create new list and add
					ACLListenerList* list = new ACLListenerList();
					list->push_back(usrArgs);
					(*(pPlugin->aclEventQueueListeners))[ch] = list;

					// register handler
					ca_replace_access_rights_event(ch, access_rights_handler);
					// above call will notify if already connected :)
				}
				else
				{
					// add to list
					(*(pPlugin->aclEventQueueListeners))[ch]->push_back(usrArgs);

					// if already connected fire event
					if (ca_state(ch) == cs_conn)
					{
						access_rights_handler_args args = { ch, { ca_read_access(ch), ca_write_access(ch) } };
						push_acl_event(pPlugin, usrArgs, args);
					}
				}
				epicsMutexUnlock(pPlugin->aclEventQueueMutex);
			}
		}


		// MONITOR and CONNECTION event registry handling
		if (type == MONITOR_EVENT || type == CONNECTION_EVENT || type == ACL_EVENT)
		{
			if (failed)
			{
				// release event id
				if (asyncCall)
					pPlugin->unregisterEvent(NPVARIANT_TO_INT32(*result));
			}
			else
			{
				// register event and (if not async) return event id
				if (asyncCall)
					pPlugin->registerEvent(NPVARIANT_TO_INT32(*result), usrArgs);
				else
					INT32_TO_NPVARIANT(pPlugin->registerEvent(usrArgs), *result);
			}
		}

		// free all event related resources
		if (failed)
		{
			NPN_ReleaseVariantValue(&userInfo);
			if (usrArgs)
			{
				NPN_ReleaseVariantValue(&usrArgs->putValue);
				delete usrArgs;
			}
		}

		if (callbackName)
			NPN_MemFree(callbackName);
		NPN_MemFree(channelName);

		return PR_TRUE;
	}
	else if (name == destroyMonitor_id)
	{
		int registryEventId = 0;

		// parameters: registry event id
		if (argCount >= 2)
			return PR_FALSE;
		else if (argCount == 1)
			registryEventId = numberValue(args[0]);

		eventUserArg* usrArg = pPlugin->unregisterEvent(registryEventId);
		if (usrArg)
			pPlugin->destroyMonitor(usrArg);

		return PR_TRUE;
	}
	else if (name == pendEvents_id)
	{

		const int DEFAULT_PROCESS_EVENT_COUNT = 16;
		int processEventCount = 0;

		// parameters: events to process (optional)
		if (argCount >= 2)
			return PR_FALSE;
		else if (argCount == 1)
			processEventCount = numberValue(args[0]);

		// take default if not specified (or invalid value)
		if (processEventCount <= 0)
			processEventCount = DEFAULT_PROCESS_EVENT_COUNT;


		for (int ec = 0; ec < processEventCount; ec++)
		{
			//
			// dequeue event
			//
			epicsMutexLock(pPlugin->eventQueueMutex);
			if (pPlugin->eventQueue->empty())
			{
				epicsMutexUnlock(pPlugin->eventQueueMutex);
				// return number of event in the queue
				INT32_TO_NPVARIANT(0, *result);
				return PR_TRUE;
			}
			evargs* event = pPlugin->eventQueue->front();
			pPlugin->eventQueue->pop_front();

			// event is dirty, usrArg should not be touched
			bool dirtyEvent = (event->status < 0);

			// remove from map
			eventUserArg* usrArg = (eventUserArg*)event->usr;
			if (!dirtyEvent && usrArg->type == MONITOR_EVENT)
				pPlugin->eventMap->erase(usrArg);

			epicsMutexUnlock(pPlugin->eventQueueMutex);

			// async notification event
			if (event->status == NPCA_CONNECTION_EVENT)
			{
				if (pPlugin->asyncRequests->find(event->chid) != pPlugin->asyncRequests->end())
  				{
  					AsyncRequestList* list = (*(pPlugin->asyncRequests))[event->chid];
					pPlugin->asyncRequests->erase(event->chid);

					AsyncRequestList::iterator iter = list->begin();
					for(; iter != list->end(); iter++)
					{
						asyncRequest* req = (*iter);

						// invoke
						NPVariant retVal;
						INT32_TO_NPVARIANT(req->registryEventId, retVal);	// inject registry event id
						this->Invoke(req->name, req->args, req->argCount, &retVal);
						NPN_ReleaseVariantValue(&retVal);

						for (uint32_t i = 0; i < req->argCount; i++)
							NPN_ReleaseVariantValue(&req->args[i]);
						delete req->args;
						delete req;
					}

					delete list;
				}
			}

			// skip cleared monitor event
			if (dirtyEvent)
			{
				// free DBR
				if (event->dbr)
					NPN_MemFree((void*)event->dbr);
				delete event;
				continue;
			}

			NPArray* array = NULL;
			const int MAX_PARAMETER_COUNT = 6;
			NPVariant params[MAX_PARAMETER_COUNT];

			int parameter_count = 0;

			//
			// monitor/get event
			//
			if (usrArg->type == MONITOR_EVENT || usrArg->type == GET_EVENT || usrArg->type == GET_CTRL_EVENT)
			{

				// userInfo, pvName, values, status, severity, timestamp
				parameter_count = 6;

				params[0] = usrArg->userInfo;
				// const char * (no need to release)
				STRINGZ_TO_NPVARIANT(ca_name(event->chid), params[1]);

				void *dbrVal = NULL;
				int status = 0;
				int severity = 0;
				epicsTimeStamp * timeStamp = NULL;
				NPVariantType type = NPVariantType_Void;

				#define GET_DBR_DATA \
					dbrVal = &dbr->value; \
					status = dbr->status; \
					severity = dbr->severity; \
					timeStamp = &dbr->stamp;

				#define GET_STS_DBR_DATA \
					dbrVal = &dbr->value; \
					status = dbr->status; \
					severity = dbr->severity;

				switch (event->type)
				{
					case DBR_TIME_DOUBLE:
					{
						dbr_time_double* dbr = (dbr_time_double*)event->dbr;
						type = NPVariantType_Double;
						GET_DBR_DATA;
						break;
					}
					case DBR_TIME_LONG:
					{
						dbr_time_long* dbr = (dbr_time_long*)event->dbr;
						type = NPVariantType_Int32;
						GET_DBR_DATA;
						break;
					}
					case DBR_TIME_STRING:
					{
						dbr_time_string* dbr = (dbr_time_string*)event->dbr;
						type = NPVariantType_String;
						GET_DBR_DATA;
						break;
					}
					case DBR_CTRL_DOUBLE:
					{
						dbr_ctrl_double* dbr = (dbr_ctrl_double*)event->dbr;
						type = NPVariantType_Double;
						GET_STS_DBR_DATA;
						break;
					}
					case DBR_CTRL_LONG:
					{
						dbr_ctrl_long* dbr = (dbr_ctrl_long*)event->dbr;
						type = NPVariantType_Int32;
						GET_STS_DBR_DATA;
						break;
					}
					case DBR_CTRL_ENUM:
					{
						dbr_ctrl_enum* dbr = (dbr_ctrl_enum*)event->dbr;
						type = NPVariantType_Int32;
						GET_STS_DBR_DATA;
						break;
					}
					case DBR_CTRL_STRING:
					{
						dbr_sts_string* dbr = (dbr_sts_string*)event->dbr;
						type = NPVariantType_String;
						GET_STS_DBR_DATA;
						break;
					}
					default:
						// should not happen
						printf("critical: unsupported type %ld!\n", event->type);
				}

				#undef GET_DBR_DATA

				// time is only is secs
				// CTRL type does not provide timeStamp
				#define TS_EPOCH_SEC_PAST_1970 7305*86400L
				double secPast1970 = timeStamp ? (double)(timeStamp->secPastEpoch+TS_EPOCH_SEC_PAST_1970)*1000 + timeStamp->nsec/1000000 : 0;

				// array
				if (event->count > 1)
				{
					if (event->type == DBR_CTRL_ENUM)
					{
						// convert short array to string array
						short* indexes = (short*)dbrVal;
						dbr_ctrl_enum* dbr = (dbr_ctrl_enum*)event->dbr;

						unsigned size = event->count*MAX_ENUM_STRING_SIZE;
						void* value = NPN_MemAlloc(size);
						for (int i = 0; i < event->count; i++)
							memcpy((char*)value+i*MAX_ENUM_STRING_SIZE, dbr->strs[indexes[i]], MAX_ENUM_STRING_SIZE);

						array = NPN_CreateArray(mNpp, NPVariantType_String, value, event->count, true, MAX_ENUM_STRING_SIZE);
						OBJECT_TO_NPVARIANT(array, params[2]);

					}
					else
					{
						// allocate memory for value
						unsigned size = event->count*dbr_value_size[event->type];
						void* value = NPN_MemAlloc(size);
						memcpy(value, dbrVal, size);

						array = NPN_CreateArray(mNpp, type, value, event->count, true, MAX_STRING_SIZE);
						OBJECT_TO_NPVARIANT(array, params[2]);
					}
				}
				// scalar
				else
				{
					switch (event->type)
					{
						case DBR_TIME_DOUBLE:
						case DBR_CTRL_DOUBLE:
						{
							DOUBLE_TO_NPVARIANT(*((double*)dbrVal), params[2]);
							break;
						}
						case DBR_TIME_LONG:
						case DBR_CTRL_LONG:
						{
							INT32_TO_NPVARIANT(*((int*)dbrVal), params[2]);
							break;
						}
						case DBR_TIME_STRING:
						case DBR_CTRL_STRING:
						{
							STRINGZ_TO_NPVARIANT(stringDup(((const char*)dbrVal)), params[2]);
							break;
						}
						case DBR_CTRL_ENUM:
						{
							// short to string...
							dbr_ctrl_enum* dbr = (dbr_ctrl_enum*)event->dbr;
							STRINGZ_TO_NPVARIANT(stringDup(dbr->strs[*((short*)dbrVal)]), params[2]);
							break;
						}
					}
				}

				// free DBR
				if (usrArg->type != GET_CTRL_EVENT)
					NPN_MemFree((void*)event->dbr);

				INT32_TO_NPVARIANT(status, params[3]);
				INT32_TO_NPVARIANT(severity, params[4]);
				if (usrArg->type != GET_CTRL_EVENT)
					DOUBLE_TO_NPVARIANT(secPast1970, params[5]);
				else
				{
					NP_CTRL* ctrl = CreateCTRL(mNpp, event->type, event->dbr, true);
					OBJECT_TO_NPVARIANT(ctrl, params[5]);
				}
			}
			//
			// put event
			//
			else if (usrArg->type == PUT_EVENT)
			{
				// userInfo, pvName, CA status
				parameter_count = 4;

				params[0] = usrArg->userInfo;
				// const char * (no need to release)
				STRINGZ_TO_NPVARIANT(ca_name(event->chid), params[1]);

				params[2] = usrArg->putValue;

				INT32_TO_NPVARIANT(event->status, params[3]);
			}
			//
			// connection changed event
			//
			else if (usrArg->type == CONNECTION_EVENT)
			{
				// userInfo, pvName, connectionStatus
				parameter_count = 3;

				params[0] = usrArg->userInfo;
				// const char * (no need to release)
				STRINGZ_TO_NPVARIANT(ca_name(event->chid), params[1]);

				int status = (event->status == CA_OP_CONN_UP);
				BOOLEAN_TO_NPVARIANT(status, params[2]);
			}
			//
			// ACL changed event
			//
			else if (usrArg->type == ACL_EVENT)
			{
				// userInfo, pvName, writableStatus
				parameter_count = 3;

				params[0] = usrArg->userInfo;
				// const char * (no need to release)
				STRINGZ_TO_NPVARIANT(ca_name(event->chid), params[1]);

				int status = (event->status == 1);
				BOOLEAN_TO_NPVARIANT(status, params[2]);
			}

			//
			// invoke the callback
			// NOTE: WebKit (Safari) does not implement NPN_InvokeDefault on JS objects
			//
			NPVariant retVal;
			NPN_Invoke(mNpp, pPlugin->getWindowObj(), usrArg->callbackId, params, parameter_count, &retVal);

			// destroy monitor request, false returned
			if (usrArg->eventId && NPVARIANT_IS_BOOLEAN(retVal) && !NPVARIANT_TO_BOOLEAN(retVal))
				pPlugin->destroyMonitor(usrArg);

			NPN_ReleaseVariantValue(&retVal);


			//
			// release the resources
			//

			// release array (TODO reuse this object, but only if ref count falls to 0)
			//if (array != NULL)
			//	NPN_ReleaseObject(array);
			// since all params[2] are value, this is most efficient solution (handles array and also string scalar)
			NPN_ReleaseVariantValue(&params[2]);

			// release CTRL object (this will alsp release referenced DBR)
			if (usrArg->type == GET_CTRL_EVENT)
				NPN_ReleaseVariantValue(&params[5]);

			// free all related resources, if not monitor
			if (usrArg->type != MONITOR_EVENT)
			{
				NPN_ReleaseVariantValue(&usrArg->userInfo);
				delete usrArg;
			}
			delete event;

		}

		// return number of event in the queue
		INT32_TO_NPVARIANT(pPlugin->eventQueue->size(), *result);

		return PR_TRUE;
	}
	else
     return PR_FALSE;
}

bool
ScriptablePluginObject::InvokeDefault(const NPVariant *args, uint32_t argCount,
                                      NPVariant *result)
{
  // default method returns plugin name
  STRINGZ_TO_NPVARIANT(stringDup(PLUGIN_NAME), *result);

  return PR_TRUE;
}



CPlugin::CPlugin(NPP pNPInstance) :
  m_pNPInstance(pNPInstance),
  m_pNPStream(NULL),
  m_pScriptableObject(NULL),
  m_pWindowObj(NULL)
{

  eventQueueMutex = epicsMutexMustCreate();
  connectionEventQueueMutex = epicsMutexMustCreate();
  aclEventQueueMutex = epicsMutexMustCreate();
  eventRegistryMutex = epicsMutexMustCreate();

  eventQueue = new EventQueue();
  eventMap = new EventMap();
  eventDestroyedMap = new EventMap();

  connectionEventQueueListeners = new ConnectionEventQueue();
  aclEventQueueListeners = new ACLEventQueue();

  asyncRequests = new AsyncRequestMap();

  eventRegistry = new EventRegistryMap();
  eventRegistryIdGenerator = 0;

  NPN_GetValue(m_pNPInstance, NPNVWindowNPObject, &m_pWindowObj);
  if (m_pWindowObj)
  	NPN_RetainObject(m_pWindowObj);
}

CPlugin::~CPlugin()
{

  //
  // clear all channels
  //
  ChannelMap::iterator iter = m_channels.begin();
  for (; iter != m_channels.end(); iter++)
  	ca_clear_channel(iter->second);

  //
  // cleanup eventQueue
  //
  epicsMutexLock(eventQueueMutex);
  while (!eventQueue->empty())
  {
		evargs* event = eventQueue->front();
		eventQueue->pop_front();

		// event is dirty, usrArg should not be touched
		bool dirtyEvent = (event->status < 0);

		if (!dirtyEvent)
		{
		    eventUserArg* usrArg = (eventUserArg*)event->usr;

			// free DBR
			if (usrArg->type == MONITOR_EVENT || usrArg->type == GET_EVENT)
				NPN_MemFree((void*)event->dbr);

			NPN_ReleaseVariantValue(&usrArg->userInfo);
			NPN_ReleaseVariantValue(&usrArg->putValue);
			// discarding old monitor values will guarantee that there is only
			// one monitor event per subscription and delete will be done only once
			delete usrArg;
		}
		else
		{
			// free DBR
			if (event->dbr)
				NPN_MemFree((void*)event->dbr);
		}
		delete event;
  }

  // delete all destroyed events
  EventMap::iterator eventIter = eventDestroyedMap->begin();
  for (; eventIter != eventDestroyedMap->end(); eventIter++)
  {
    eventUserArg *arg = eventIter->first;

    NPN_ReleaseVariantValue(&arg->userInfo);
    delete arg;
  }
  eventDestroyedMap->clear();

  epicsMutexUnlock(eventQueueMutex);

  //
  // cleanup connectionEventQueueListeners
  //

  epicsMutexLock(connectionEventQueueMutex);

  ConnectionEventQueue::iterator listeners = connectionEventQueueListeners->begin();
  for (; listeners != connectionEventQueueListeners->end(); listeners++)
  {
	ConnectionListenerList* list = listeners->second;

	ConnectionListenerList::iterator iter = list->begin();
	for(; iter != list->end(); iter++)
	{
		eventUserArg* usrArg = (*iter);

		NPN_ReleaseVariantValue(&usrArg->userInfo);
		delete usrArg;
	}

	delete list;
  }
  connectionEventQueueListeners->clear();

  AsyncRequestMap::iterator requests = asyncRequests->begin();
  for (; requests != asyncRequests->end(); requests++)
  {
	AsyncRequestList* list = requests->second;

	AsyncRequestList::iterator iter = list->begin();
	for(; iter != list->end(); iter++)
	{
		asyncRequest* req = (*iter);
		for (uint32_t i = 0; i < req->argCount; i++)
			NPN_ReleaseVariantValue(&req->args[i]);
		delete req->args;
		delete req;
	}

	delete list;
  }
  asyncRequests->clear();

  epicsMutexUnlock(connectionEventQueueMutex);

  //
  // cleanup aclEventQueueListeners
  //
  epicsMutexLock(aclEventQueueMutex);

  ACLEventQueue::iterator alisteners = aclEventQueueListeners->begin();
  for (; alisteners != aclEventQueueListeners->end(); alisteners++)
  {
	ACLListenerList* list = alisteners->second;

	ACLListenerList::iterator iter = list->begin();
	for(; iter != list->end(); iter++)
	{
		eventUserArg* usrArg = (*iter);

		NPN_ReleaseVariantValue(&usrArg->userInfo);
		delete usrArg;
	}

	delete list;
  }
  aclEventQueueListeners->clear();
  epicsMutexUnlock(aclEventQueueMutex);

  // sleep for a while, to complete all pending calls
  epicsThreadSleep(0.2);

  if (m_pWindowObj)
    NPN_ReleaseObject(m_pWindowObj);
  if (m_pScriptableObject)
    NPN_ReleaseObject(m_pScriptableObject);

  delete eventQueue;
  delete eventMap;
  delete eventDestroyedMap;
  delete connectionEventQueueListeners;
  delete aclEventQueueListeners;
  delete asyncRequests;
  delete eventRegistry;

  epicsMutexDestroy(eventQueueMutex);
  epicsMutexDestroy(connectionEventQueueMutex);
  epicsMutexDestroy(aclEventQueueMutex);
  epicsMutexDestroy(eventRegistryMutex);
 }


NPObject *
CPlugin::GetScriptableObject()
{
  if (!m_pScriptableObject) {
    m_pScriptableObject =
      NPN_CreateObject(m_pNPInstance,
                       GET_NPOBJECT_CLASS(ScriptablePluginObject));
  }

  if (m_pScriptableObject) {
    NPN_RetainObject(m_pScriptableObject);
  }

  return m_pScriptableObject;
}

void
CPlugin::getChannelCHID(const char* channelName, chid &channel)
{
	if (m_channels.find(channelName) != m_channels.end())
		channel = m_channels[channelName];
	else
		channel = 0;
}

int
CPlugin::getChannel(const char* channelName, chid &channel)
{
	if (m_channels.find(channelName) == m_channels.end())
	{
 	    int result = ca_create_channel (channelName,
                                   0,
                                   m_pNPInstance,
                                   CA_PRIORITY_DEFAULT,
                                   &channel);

 		if (result != ECA_NORMAL)
 			return result;

		// TODO configurable timeout
		result = ca_pend_io(5.0);
		if (result != ECA_NORMAL)
 			return result;

 		// install connection handler
 		// (done here to have sync connect)
 		ca_change_connection_event(channel, connection_handler);

		// save chid
		m_channels[channelName] = channel;
	}
	else
		channel = m_channels[channelName];

	return ECA_NORMAL;
}

int
CPlugin::getChannelAsync(const char* channelName, chid &channel)
{
	if (m_channels.find(channelName) == m_channels.end())
	{
 	    int result = ca_create_channel (channelName,
                                   connection_handler,
                                   m_pNPInstance,
                                   CA_PRIORITY_DEFAULT,
                                   &channel);

 		if (result != ECA_NORMAL)
 			return result;

		// save chid
		m_channels[channelName] = channel;
	}
	else
		channel = m_channels[channelName];

	return ECA_NORMAL;
}



RegistryEventId
CPlugin::registerEvent(eventUserArg* usrArg)
{
  epicsMutexLock(eventRegistryMutex);
  // theoretically this can deadlock
  while (eventRegistry->find(++eventRegistryIdGenerator) != eventRegistry->end() || eventRegistryIdGenerator == 0);
  (*eventRegistry)[eventRegistryIdGenerator] = usrArg;
  RegistryEventId retVal = eventRegistryIdGenerator;
  epicsMutexUnlock(eventRegistryMutex);
  return retVal;
}

void
CPlugin::registerEvent(RegistryEventId id, eventUserArg* usrArg)
{
  epicsMutexLock(eventRegistryMutex);
  (*eventRegistry)[id] = usrArg;
  epicsMutexUnlock(eventRegistryMutex);
}

eventUserArg*
CPlugin::unregisterEvent(RegistryEventId id)
{
  epicsMutexLock(eventRegistryMutex);
  EventRegistryMap::iterator iter = (id == 0 ? eventRegistry->end() : eventRegistry->find(id));
  eventUserArg* retVal = 0;
  if (iter != eventRegistry->end())
  {
	retVal = iter->second;
    eventRegistry->erase(iter);
  }
  epicsMutexUnlock(eventRegistryMutex);
  return retVal;
}

bool CPlugin::existsEvent(RegistryEventId id)
{
  epicsMutexLock(eventRegistryMutex);
  EventRegistryMap::iterator iter = (id == 0 ? eventRegistry->end() : eventRegistry->find(id));
  const bool exists = (iter != eventRegistry->end());
  epicsMutexUnlock(eventRegistryMutex);
  return exists;
}

void
CPlugin::destroyMonitor(eventUserArg* usrArg)
{
	// destroy subscription
	if (usrArg->eventId)
	{
		int res = ca_clear_subscription(usrArg->eventId);
		if (res != ECA_NORMAL)
		{
			printf("failed to clear subscription for channel %s: %s\n", ca_name(usrArg->channelId), ca_message(res));
		}
		else
			ca_flush_io();

		// mark as cleared
		usrArg->eventId = 0;

		// remove all enqueued
		epicsMutexLock(eventQueueMutex);

		if (eventMap->find(usrArg) != eventMap->end())
		{
			evargs* ev = (*eventMap)[usrArg];
			ev->status = NPCA_DIRTY_EVENT;
		}
		eventMap->erase(usrArg);

		// set usrArg to be deleted
		(*eventDestroyedMap)[usrArg] = 0;

		epicsMutexUnlock(eventQueueMutex);
	}
	else if (usrArg->type == CONNECTION_EVENT)
	{
		// remove listener
		epicsMutexLock(connectionEventQueueMutex);

		ConnectionEventQueue::iterator ceq = connectionEventQueueListeners->find(usrArg->channelId);
		if (ceq != connectionEventQueueListeners->end())
		{
			ConnectionListenerList * list = ceq->second;
			ConnectionListenerList::iterator pos = find(list->begin(), list->end(), usrArg);
			if (pos != list->end())
				list->erase(pos);
		}
		epicsMutexUnlock(connectionEventQueueMutex);
	}
	else if (usrArg->type == ACL_EVENT)
	{
		// remove listener
		epicsMutexLock(aclEventQueueMutex);

		ACLEventQueue::iterator ceq = aclEventQueueListeners->find(usrArg->channelId);
		if (ceq != aclEventQueueListeners->end())
		{
			ACLListenerList * list = ceq->second;
			ACLListenerList::iterator pos = find(list->begin(), list->end(), usrArg);
			if (pos != list->end())
				list->erase(pos);
		}
		epicsMutexUnlock(aclEventQueueMutex);
	}
}

/*****************************************************************************
 * Array support
 *
 *   NPArrays are immutable.  They are used to pass arguments to
 *   the script functions that expect arrays, or to export
 *   arrays of properties.  NPArray is represented in JavaScript
 *   by a restricted Array.  The Array in JavaScript is read-only,
 *   only has index accessors, and may not be resized.
 *
 *   Objects added to arrays are retained by the array.
 */

static NPIdentifier arrayLength_id = NULL;

class ArrayObject : public ScriptablePluginObjectBase
{
public:
  ArrayObject(NPP npp)
    : ScriptablePluginObjectBase(npp), count(0), data(0), release(false), maxStringSize(0)
  {
	  if (arrayLength_id == NULL)
		arrayLength_id = NPN_GetStringIdentifier("length");
  }

  virtual ~ArrayObject()
  {
 	  if (release && data)
	  {
			if (type == NPVariantType_Int32) {
				NPN_MemFree((int*)data);
			}
			else if (type == NPVariantType_Double) {
				NPN_MemFree((double*)data);
			}
			else if (type == NPVariantType_String) {
				// string dbr is one block
				NPN_MemFree((char**)data);
			}
  	  }
  }

  // NOTE: only int, double and string supported
  // TODO think of reusing this class
  void setData(NPVariantType _type, void* _data, int32_t _count, bool _release, uint32_t _maxStringSize)
  {
	 if (_type != NPVariantType_Int32 &&
	 	 _type != NPVariantType_Double &&
	 	 _type != NPVariantType_String)
	 	return;

     type = _type;
     data = _data;
     count = _count;
     release = _release;
     maxStringSize = _maxStringSize;
  }

  virtual bool HasProperty(NPIdentifier name);
  virtual bool GetProperty(NPIdentifier name, NPVariant *result);
protected:
	int count;
	NPVariantType type;
	void *data;
	bool release;
	uint32_t maxStringSize;
};

static NPObject *
AllocateArrayObject(NPP npp, NPClass *aClass)
{
  return new ArrayObject(npp);
}

DECLARE_NPOBJECT_CLASS_WITH_BASE(ArrayObject,
                                 AllocateArrayObject);


NPArray *NPN_CreateArray (NPP npp, NPVariantType type, void* data, uint32_t count, bool release, uint32_t maxStringSize)
{
    ArrayObject *array = (ArrayObject *)NPN_CreateObject(npp, GET_NPOBJECT_CLASS(ArrayObject));
    array->setData(type, data, count, release, maxStringSize);
    return (NPArray *)array;
}

bool
ArrayObject::HasProperty(NPIdentifier name)
{
	if (name == arrayLength_id)
		return true;
	else if (!NPN_IdentifierIsString(name))
	{
		int index = NPN_IntFromIdentifier(name);
		return (index >= 0 && index < count);
	}
	else
	{
		/* WebKit sends Int identifier as String (shame on it) */
		char* str = NPN_UTF8FromIdentifier(name);
		int index;
		bool success = (sscanf(str, "%d", &index) == 1);
		NPN_MemFree(str);
		return (success && (index >= 0 && index < count));
	}
}

bool
ArrayObject::GetProperty(NPIdentifier name, NPVariant *result)
{
	if (name == arrayLength_id)
	{
		INT32_TO_NPVARIANT(count, *result);
		return true;
	}
	else
	{
		int index;

		if (!NPN_IdentifierIsString(name))
		 	index = NPN_IntFromIdentifier(name);
		else
		{
			/* WebKit sends Int identifier as String (shame on it) */
			char* str = NPN_UTF8FromIdentifier(name);
			bool success = (sscanf(str, "%d", &index) == 1);
			NPN_MemFree(str);
			if (!success)
				return false;
		}

		if (index >= 0 && index < count)
		{
			if (type == NPVariantType_Int32) {
				INT32_TO_NPVARIANT(((int*)data)[index], *result);
				return true;
			}
			else if (type == NPVariantType_Double) {
				DOUBLE_TO_NPVARIANT(((double*)data)[index], *result);
				return true;
			}
			else if (type == NPVariantType_String) {
				STRINGZ_TO_NPVARIANT(stringDup(((const char*)data+index*maxStringSize)), *result);
				return true;
			}
			else
				return false;
		}
	}

	VOID_TO_NPVARIANT(*result);
	return false;
}

/*****************************************************************************
 * CTRL support (immutable object)
 *
 */

static NPIdentifier units_id = NULL;
static NPIdentifier precision_id = NULL;
static NPIdentifier upperDisplayLimit_id = NULL;
static NPIdentifier lowerDisplayLimit_id = NULL;
static NPIdentifier upperAlarmLimit_id = NULL;
static NPIdentifier upperWarningLimit_id = NULL;
static NPIdentifier lowerWarningLimit_id = NULL;
static NPIdentifier lowerAlarmLimit_id = NULL;
static NPIdentifier upperControlLimit_id = NULL;
static NPIdentifier lowerControlLimit_id = NULL;
static NPIdentifier labels_id = NULL;

class CTRLObject : public ScriptablePluginObjectBase
{
public:
  CTRLObject(NPP npp)
    : ScriptablePluginObjectBase(npp), dbrType(-1), dbr(0), release(false)
  {
	  if (units_id == NULL)
		units_id = NPN_GetStringIdentifier("units");

	  if (precision_id == NULL)
		precision_id = NPN_GetStringIdentifier("precision");

	  if (upperDisplayLimit_id == NULL)
		upperDisplayLimit_id = NPN_GetStringIdentifier("upperDisplayLimit");

	  if (lowerDisplayLimit_id == NULL)
		lowerDisplayLimit_id = NPN_GetStringIdentifier("lowerDisplayLimit");

	  if (upperAlarmLimit_id == NULL)
		upperAlarmLimit_id = NPN_GetStringIdentifier("upperAlarmLimit");

	  if (upperWarningLimit_id == NULL)
		upperWarningLimit_id = NPN_GetStringIdentifier("upperWarningLimit");

	  if (lowerWarningLimit_id == NULL)
		lowerWarningLimit_id = NPN_GetStringIdentifier("lowerWarningLimit");

	  if (lowerAlarmLimit_id == NULL)
		lowerAlarmLimit_id = NPN_GetStringIdentifier("lowerAlarmLimit");

	  if (upperControlLimit_id == NULL)
		upperControlLimit_id = NPN_GetStringIdentifier("upperControlLimit");

	  if (lowerControlLimit_id == NULL)
		lowerControlLimit_id = NPN_GetStringIdentifier("lowerControlLimit");

	  if (labels_id == NULL)
		labels_id = NPN_GetStringIdentifier("labels");
  }

  virtual ~CTRLObject()
  {
  	if (release && dbr)
  		NPN_MemFree((void*)dbr);
  }

  void setData(short dbrType, const void* dbr, bool release)
  {
  	this->dbrType = dbrType;
  	this->dbr = dbr;
  	// NOTE: we do not make a copy of DBR here...
  }

  virtual bool HasProperty(NPIdentifier name);
  virtual bool GetProperty(NPIdentifier name, NPVariant *result);
protected:
  short dbrType;
  const void* dbr;
  bool release;
};

static NPObject *
AllocateCTRLObject(NPP npp, NPClass *aClass)
{
	return new CTRLObject(npp);
}

DECLARE_NPOBJECT_CLASS_WITH_BASE(CTRLObject,
                                 AllocateCTRLObject);


NP_CTRL *CreateCTRL (NPP npp, short dbrType, const void* dbr, bool release)
{
    CTRLObject *ctrl = (CTRLObject *)NPN_CreateObject(npp, GET_NPOBJECT_CLASS(CTRLObject));
    ctrl->setData(dbrType, dbr, release);
    return (NP_CTRL *)ctrl;
}

bool
CTRLObject::HasProperty(NPIdentifier name)
{
	return
		name == units_id ||
		name == precision_id ||
		name == upperDisplayLimit_id ||
		name == lowerDisplayLimit_id ||
		name == upperAlarmLimit_id ||
		name == upperWarningLimit_id ||
		name == lowerWarningLimit_id ||
		name == lowerAlarmLimit_id ||
		name == upperControlLimit_id ||
		name == lowerControlLimit_id ||
		name == labels_id;
}

bool
CTRLObject::GetProperty(NPIdentifier name, NPVariant *result)
{
	#define RETURN_NUMBER(variable) \
		if (dbrType == DBR_CTRL_DOUBLE) \
			DOUBLE_TO_NPVARIANT(((dbr_ctrl_double*)dbr)->variable, *result); \
		else if (dbrType == DBR_CTRL_LONG) \
			INT32_TO_NPVARIANT(((dbr_ctrl_long*)dbr)->variable, *result); \
		else \
			VOID_TO_NPVARIANT(*result); \
	return true;

	if (name == units_id)
	{
		if (dbrType == DBR_CTRL_DOUBLE)
			STRINGZ_TO_NPVARIANT(stringDup(((dbr_ctrl_double*)dbr)->units), *result);
		else if (dbrType == DBR_CTRL_LONG)
			STRINGZ_TO_NPVARIANT(stringDup(((dbr_ctrl_long*)dbr)->units), *result);
		else
			VOID_TO_NPVARIANT(*result);
		return true;
	}
	else if (name == precision_id)
	{
		if (dbrType == DBR_CTRL_DOUBLE)
			INT32_TO_NPVARIANT(((dbr_ctrl_double*)dbr)->precision, *result);
		else
			VOID_TO_NPVARIANT(*result);
		return true;
	}
	else if (name == upperDisplayLimit_id)
	{
		RETURN_NUMBER(upper_disp_limit);
	}
	else if (name == lowerDisplayLimit_id)
	{
		RETURN_NUMBER(lower_disp_limit);
	}
	else if (name == upperAlarmLimit_id)
	{
		RETURN_NUMBER(upper_alarm_limit);
	}
	else if (name == upperWarningLimit_id)
	{
		RETURN_NUMBER(upper_warning_limit);
	}
	else if (name == lowerWarningLimit_id)
	{
		RETURN_NUMBER(lower_warning_limit);
	}
	else if (name == lowerAlarmLimit_id)
	{
		RETURN_NUMBER(lower_alarm_limit);
	}
	else if (name == upperControlLimit_id)
	{
		RETURN_NUMBER(upper_ctrl_limit);
	}
	else if (name == lowerControlLimit_id)
	{
		RETURN_NUMBER(lower_ctrl_limit);
	}
	else if (name == labels_id)
	{
		if (dbrType == DBR_CTRL_ENUM)
		{
			// allocate memory
			dbr_ctrl_enum* enum_dbr = (dbr_ctrl_enum*)dbr;
			const int32_t count = enum_dbr->no_str;
			unsigned size = count * MAX_ENUM_STRING_SIZE;
			void* value = NPN_MemAlloc(size);
			memcpy(value, enum_dbr->strs, size);

			// create array object
			NPArray* array = NPN_CreateArray(mNpp, NPVariantType_String, value, count, true, MAX_ENUM_STRING_SIZE);
			OBJECT_TO_NPVARIANT(array, *result);
		}
		else
			VOID_TO_NPVARIANT(*result);

		return true;
	}
	else
	{
		VOID_TO_NPVARIANT(*result);
		return false;
	}

	#undef RETURN_NUMBER
}
