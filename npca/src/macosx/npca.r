/*****************************************************************************
 * NPCA Plugin description for OS X
 *****************************************************************************/

/* Definitions of system resource types */

data 'carb' (0)
{
};

/* The first string in the array is a plugin description,
 * the second is the plugin name */
resource 'STR#' (126)
{
    {
        PLUGIN_DESC,
        PLUGIN_NAME
    };
};

/* A description for each MIME type in resource 128 */
resource 'STR#' (127)
{
    {
        PLUGIN_NAME_ONLY
    };
};

/* A series of pairs of strings... first MIME type, then file extension(s) */
resource 'STR#' (128,"MIME Type")
{
    {
        PLUGIN_MIME, ""
    };
};

