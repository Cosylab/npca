/** 
 * @fileoverview This is a WebCA (npca) API documentation.
 * @author Matej Sekoranja <matej.sekoranja_AT_cosylab.com>
 * @version 1.1
 */


/**
 * <font color="red">There is no constructor for EPICSPlugin object!</font>
 * @class EPICSPlugin is a scriptable object implemented as an
 * {@link http://www.mozilla.org/projects/plugins/npruntime.html NPAPI} plugin.
 * To obtain EPICSPlugin object use:
 * <pre>
 *    // inside HTML body
 *    &lt;embed id="EPICSPlugin" type="application/mozilla-npca-scriptable-plugin" width="0" height="0"/&gt;
 *
 *    // inside JavaScript section 
 *    // get scriptable object
 *    var epicsPlugin = document.getElementById("EPICSPlugin");
 * </pre>
 */
function EPICSPlugin() {};

/**
 * Version of the plugin.
 * @type String
 */
EPICSPlugin.prototype.version = null;

/**
 * Version of the EPICS base the plugin was compiled against. 
 * @type String
 */
EPICSPlugin.prototype.epicsVersion = null;


EPICSPlugin.prototype.pendEvents = EPICSPlugin_pendEvents;

/**
 * Handle events (callbacks).
 * This method is an equivalent to EPICS CA <code>context.pendEvent()</code> method with non-preemptive CA context.
 * If this method is not called no events (callbacks) will be called by the plugin.<br/>
 * Usage:
 * <pre>
 *    function handleEvents() {
 *      epicsPlugin.pendEvents();
 *      
 *      // 10Hz
 *      setTimeout(function() { handleEvents(); }, 100);
 *    }
 *    
 *    // start handling events
 *    handleEvents();
 * </pre> 
 * @param {int} eventsToProcess maximum number of events to process.
 * 				This parameter is optional, if not specified default value <code>16</code> will be used.
 * @return {int} actual number of events processed.
 */
function EPICSPlugin_pendEvents(eventsToProcess) {};




EPICSPlugin.prototype.monitorConnectionStatus = EPICSPlugin_monitorConnectionStatus;

/**
 * Monitor connection status.
 * @param {String} pvName process variable name.
 * @param {String} callbackName name of the callback function of <code>connectionStatusCallback</code> signature
 *				   to be called at any connection status change.
 * @param userInfo user supplied object that will be passed as an parameter of the callback.
 * @return {int} monitor id, <code>0</code> on failure. Use <code>destroyMonitor</code> with this id to destroy this monitor.
 * @see GLOBALS#connectionStatusCallback
 * @see EPICSPlugin#destroyMonitor
 */
function EPICSPlugin_monitorConnectionStatus(pvName, callbackName, userInfo) {};


/**
 * Connection status callback function signature.
 * @param userInfo user supplied object that was passed to the <code>monitorConnectionStatus</code> method.
 * @param {String} pvName process variable name.
 * @param {boolean} connectionStatus connection status.
 * @see EPICSPlugin#monitorConnectionStatus
 */
function connectionStatusCallback(userInfo, pvName, connectionStatus) {};






EPICSPlugin.prototype.getPV = EPICSPlugin_getPV;

/**
 * Read value of process variable (caget).
 * @param {String} pvName process variable name.
 * @param {String} callbackName name of the callback function of <code>getCallback</code> signature
 *				   to be called to return read value.
 * @param userInfo user supplied object that will be passed as an parameter of the callback.
 * @see GLOBALS#getCallback
 */
function EPICSPlugin_getPV(pvName, callbackName, userInfo) {};


/**
 * Get callback function signature.
 * @param userInfo user supplied object that was passed to the <code>getPV</code> method.
 * @param {String} pvName process variable name.
 * @param values value (of PV is scalar) or an array of values (if PV is an array) that was read.
 * @param {int} status PV status.
 * @param {int} severity PV severity.
 * @param {double} milliseconds past January 1970. Use <code>"var date = new Date(); date.setTime(timestamp);"</code> to get <code>Date</code> object.
 * @see EPICSPlugin#getPV
 */
function getCallback(userInfo, pvName, values, status, severity, timestamp) {};








EPICSPlugin.prototype.getCTRL = EPICSPlugin_getCTRL;

/**
 * Read CTRL data block (and value) of process variable (caget).
 * @param {String} pvName process variable name.
 * @param {String} callbackName name of the callback function of <code>getCTRLCallback</code> signature
 *				   to be called to return read value.
 * @param userInfo user supplied object that will be passed as an parameter of the callback.
 * @see GLOBALS#getCTRLCallback
 */
function EPICSPlugin_getCTRL(pvName, callbackName, userInfo) {};


/**
 * Get CTRL callback function signature.
 * @param userInfo user supplied object that was passed to the <code>getCTRL</code> method.
 * @param {String} pvName process variable name.
 * @param values value (of PV is scalar) or an array of values (if PV is an array) that was read.
 * @param {int} status PV status.
 * @param {int} severity PV severity.
 * @param {CTRL} ctrlObj structure containing all CTRL fields.
 * @see EPICSPlugin#getCTRL
 */
function getCTRLCallback(userInfo, pvName, values, status, severity, ctrlObj) {};







EPICSPlugin.prototype.putPV = EPICSPlugin_putPV;

/**
 * Write value to process variable (caput).
 * @param {String} pvName process variable name.
 * @param {String} callbackName name of the callback function of <code>getCallback</code> signature
 *				   to be called to report put status.
 * @param userInfo user supplied object that will be passed as an parameter of the callback.
 * @pram values values to put (scalar or an array of values)
 * @see GLOBALS#putCallback
 */
function EPICSPlugin_putPV(pvName, callbackName, userInfo, values) {};


/**
 * Put callback function signature.
 * @param userInfo user supplied object that was passed to the <code>putPV</code> method.
 * @param {String} pvName process variable name.
 * @param values value (of PV is scalar) or an array of values (if PV is an array) that was put.
 * @param {int} completionStatus put completion CA status.
 * @param {double} milliseconds past January 1970. Use <code>"var date = new Date(); date.setTime(timestamp);"</code> to get <code>Date</code> object.
 * @see EPICSPlugin#putPV
 */
function putCallback(userInfo, pvName, values, completionStatus) {};








EPICSPlugin.prototype.monitorPV = EPICSPlugin_monitorPV;

/**
 * Monitor value of process variable (camonitor).
 * @param {String} pvName process variable name.
 * @param {String} callbackName name of the callback function of <code>getCallback</code> signature
 *				   to be called on value change.
 * @param userInfo user supplied object that will be passed as an parameter of the callback.
 * @return {int} monitor id, <code>0</code> on failure. Use <code>destroyMonitor</code> with this id to destroy this monitor.
 * @see GLOBALS#monitorCallback
 * @see EPICSPlugin#destroyMonitor
 */
function EPICSPlugin_monitorPV(pvName, callbackName, userInfo) {};


/**
 * Monitor callback function signature.
 * @param userInfo user supplied object that was passed to the <code>monitorPV</code> method.
 * @param {String} pvName process variable name.
 * @param values new value (of PV is scalar) or an array of values (if PV is an array).
 * @param {int} status PV status.
 * @param {int} severity PV severity.
 * @param {double} milliseconds past January 1970. Use <code>"var date = new Date(); date.setTime(timestamp);"</code> to get <code>Date</code> object.
 * @return {boolean} if <code>false</code> is returned monitor will be destroyed.
 * @see EPICSPlugin#monitorPV
 */
function monitorCallback(userInfo, pvName, values, status, severity, timestamp) {};






EPICSPlugin.prototype.destroyMonitor = EPICSPlugin_destroyMonitor;

/**
 * Destroy monitor (value, connection, ACL monitor).
 * @param {int} monitorId id of a monitor to destroy.
 * @see EPICSPlugin#monitorPV
 * @see EPICSPlugin#monitorConnectionStatus
 * @see EPICSPlugin#monitorWritableStatus
 */
function EPICSPlugin_destroyMonitor(monitorId) {};








EPICSPlugin.prototype.monitorWritableStatus = EPICSPlugin_monitorWritableStatus;

/**
 * Monitor writable access rights status.
 * @param {String} pvName process variable name.
 * @param {String} callbackName name of the callback function of <code>writableStatusCallback</code> signature
 *				   to be called at any writable access right change.
 * @param userInfo user supplied object that will be passed as an parameter of the callback.
 * @return {int} monitor id, <code>0</code> on failure. Use <code>destroyMonitor</code> with this id to destroy this monitor.
 * @see GLOBALS#writableStatusCallback
 * @see EPICSPlugin#destroyMonitor
 */
function EPICSPlugin_monitorWritableStatus(pvName, callbackName, userInfo) {};


/**
 * Writable access rights status callback function signature.
 * @param userInfo user supplied object that was passed to the <code>monitorWritableStatus</code> method.
 * @param {String} pvName process variable name.
 * @param {boolean} writable access rights status.
 * @see EPICSPlugin#monitorWritableStatus
 */
function writableStatusCallback(userInfo, pvName, writableStatus) {};






/** 
 * String array of labels (string descriptions of enum states). If process variable is not a ENUM type this field is <code>undefined</code>.
 * @public
 * @type string[]
 */
CTRL.prototype.labels = null;

/** 
 * Units. If process variable is not a numeric type this field is <code>undefined</code>.
 * @public
 * @type string
 */
CTRL.prototype.units = null;

/** 
 * Precision (number od digits after decimal point). If process variable is not a floating point type this field is <code>undefined</code>.
 * @public
 * @type int
 */
CTRL.prototype.precision = null;

/** 
 * Upper display limit. If process variable is not a numeric type this field is <code>undefined</code>.
 * @public
 * @type int/double
 */
CTRL.prototype.upperDisplayLimit = null;

/** 
 * Lower display limit. If process variable is not a numeric type this field is <code>undefined</code>.
 * @public
 * @type int/double
 */
CTRL.prototype.lowerDisplayLimit = null;

/** 
 * Upper alarm limit. If process variable is not a numeric type this field is <code>undefined</code>.
 * @public
 * @type int/double
 */
CTRL.prototype.upperAlarmLimit = null;

/** 
 * Lower alarm limit. If process variable is not a numeric type this field is <code>undefined</code>.
 * @public
 * @type int/double
 */
CTRL.prototype.lowerAlarmLimit = null;

/** 
 * Upper warning limit. If process variable is not a numeric type this field is <code>undefined</code>.
 * @public
 * @type int/double
 */
CTRL.prototype.upperWarningLimit = null;

/** 
 * Lower warning limit. If process variable is not a numeric type this field is <code>undefined</code>.
 * @public
 * @type int/double
 */
CTRL.prototype.lowerWarningLimit = null;


/** 
 * Upper control limit. If process variable is not a numeric type this field is <code>undefined</code>.
 * @public
 * @type int/double
 */
CTRL.prototype.upperControlLimit = null;

/** 
 * Lower control limit. If process variable is not a numeric type this field is <code>undefined</code>.
 * @public
 * @type int/double
 */
CTRL.prototype.lowerControlLimit = null;

/** 
 * String array of labels (string descriptions of enum states). If process variable is not a ENUM type this field is <code>undefined</code>.
 * @public
 * @type string[]
 */
CTRL.prototype.labels = null;
