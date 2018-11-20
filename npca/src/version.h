#ifndef _VERSION_H_
#define _VERSION_H_

/*******************************************************************************
 * Plugin properties.
 ******************************************************************************/

/* Major part of version number */
#define NPCA_MAJOR_VERSION 1 

/* Minor part of version number */
#define NPCA_MINOR_VERSION 2

/* Release part of version number */
#define NPCA_RELEASE 0

/* Release part of version number */
#define NPCA_PLATFORMID 0

/** Internal: helper macro for stringifying the version numbers */
#define STRINGIFY(n) STRINGIFY_HELPER(n)
/** Internal: helper macro for stringifying the version numbers */
#define STRINGIFY_HELPER(n) #n

/* Representation of version number */
#define NPCA_SHORT_VERSION NPCA_MAJOR_VERSION##.##NPCA_MINOR_VERSION##.##NPCA_RELEASE
#define NPCA_VERSION NPCA_MAJOR_VERSION##.##NPCA_MINOR_VERSION##.##NPCA_RELEASE##.##NPCA_PLATFORMID
#define NPCA_VERSION_COMMA NPCA_MAJOR_VERSION##,##NPCA_MINOR_VERSION##,##NPCA_RELEASE##,##NPCA_PLATFORMID

/* String representation of version number */
#define NPCA_VERSION_STRING STRINGIFY(NPCA_MAJOR_VERSION) "." STRINGIFY(NPCA_MINOR_VERSION) "." STRINGIFY(NPCA_RELEASE) "." STRINGIFY(NPCA_PLATFORMID)
#define NPCA_SHORT_VERSION_STRING STRINGIFY(NPCA_MAJOR_VERSION) "." STRINGIFY(NPCA_MINOR_VERSION) "." STRINGIFY(NPCA_RELEASE)

#define PLUGIN_NAME_ONLY "EPICS Channel Access Plug-in"
#define PLUGIN_NAME PLUGIN_NAME_ONLY " " NPCA_SHORT_VERSION_STRING
#define PLUGIN_DESC "The plug-in that provides an EPICS Channel Access implementation to JavaScript code."
#define PLUGIN_MIME "application/mozilla-npca-scriptable-plugin"
#define PLUGIN_MIME_DESC PLUGIN_MIME "::" PLUGIN_NAME_ONLY

#endif
