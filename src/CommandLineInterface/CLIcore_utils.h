/**
 * @file    CLIcore_utils.h
 * @brief   Util functions and macros for coding convenience
 *
 */

#ifndef CLICORE_UTILS_H
#define CLICORE_UTILS_H

#ifdef __cplusplus
typedef const char *CONST_WORD;
#else
typedef const char *restrict  CONST_WORD;
#endif


#include <string.h>

#include "CommandLineInterface/CLIcore.h"

#include "COREMOD_memory/COREMOD_memory.h"
#include "CommandLineInterface/IMGID.h"



#define CLIARG_VISIBLE_DEFAULT CLICMDARG_FLAG_DEFAULT,FPTYPE_AUTO,FPFLAG_DEFAULT_INPUT
#define CLIARG_HIDDEN_DEFAULT   CLICMDARG_FLAG_NOCLI,FPTYPE_AUTO,FPFLAG_DEFAULT_INPUT

#define CLICMD_FIELDS_DEFAULTS  __FILE__,sizeof(farg)/sizeof(CLICMDARGDEF),farg,CLICMDFLAG_FPS,NULL
#define CLICMD_FIELDS_NOFPS     __FILE__,sizeof(farg)/sizeof(CLICMDARGDEF),farg,0,NULL



// return codes for function CLI_checkarg_array
#define RETURN_CLICHECKARGARRAY_SUCCESS      0
#define RETURN_CLICHECKARGARRAY_FAILURE      1
#define RETURN_CLICHECKARGARRAY_FUNCPARAMSET 2
#define RETURN_CLICHECKARGARRAY_HELP         3


// binding between variables and function args/params
#define STD_FARG_LINKfunction \
for(int argi = 0; argi < (int) (sizeof(farg) / sizeof(CLICMDARGDEF)); argi++)\
    {\
        void *ptr = get_farg_ptr(farg[argi].fpstag);\
        *(farg[argi].valptr) = ptr;\
    }



/** @brief Standard Function call wrapper
 *
 * CLI argument(s) is(are) parsed and checked with CLI_checkarray(), then
 * passed to the compute function call.
 *
 * Custom code may be added for more complex processing of function arguments.
 *
 * If CLI call arguments check out, go ahead with computation.
 * Arguments not contained in CLI call line are extracted from the
 * command argument list
 */
#define INSERT_STD_CLIfunction \
static errno_t CLIfunction(void)\
{\
errno_t retval = CLI_checkarg_array(farg, CLIcmddata.nbarg);\
    if(retval == RETURN_SUCCESS)\
    {\
        STD_FARG_LINKfunction\
        return compute_function();\
    }\
    if(retval == RETURN_CLICHECKARGARRAY_HELP)\
    {\
        return RETURN_SUCCESS;\
    }\
    if(retval == RETURN_CLICHECKARGARRAY_FUNCPARAMSET)\
    {\
        return RETURN_SUCCESS;\
    }\
    return retval;\
}



/** @brief FPS conf function
 * Sets up the FPS and its parameters.\n
 * Optional parameter checking can be included.\n
 *
 * ### ADD PARAMETERS
 *
 * The function function_parameter_add_entry() is called to add
 * each parameter.
 *
 * Macros are provided for convenience, named "FPS_ADDPARAM_...".\n
 * The macros are defined in fps_add_entry.h, and provide a function
 * parameter identifier variable (int) for each parameter added.
 *
 * parameters for FPS_ADDPARAM macros:
 * - key/variable name
 * - tag name
 * - description
 * - default initial value
 *
 * Equivalent code without using macro :
 *      function_parameter_add_entry(&fps, ".delayus", "Delay [us]", FPTYPE_INT64, FPFLAG_DEFAULT_INPUT|FPFLAG_WRITERUN, NULL);
 * ### START CONFLOOP
 *
 * start function parameter conf loop\n
 * macro defined in function_parameter.h
 *
 * Optional code to handle/check parameters is included after this
 * statement
 *
 * ### STOP CONFLOOP
 * stop function parameter conf loop\n
 * macro defined in function_parameter.h
 */
#define INSERT_STD_FPSCONFfunction \
static errno_t FPSCONFfunction()\
{\
    FPS_SETUP_INIT(data.FPS_name, data.FPS_CMDCODE);\
    if( CLIcmddata.cmdsettings->flags & CLICMDFLAG_PROCINFO )\
    {\
        fps_add_processinfo_entries(&fps);\
    }\
    data.fpsptr = &fps;\
    CMDargs_to_FPSparams_create(&fps);\
    STD_FARG_LINKfunction\
    FPS_CONFLOOP_START\
    data.fpsptr = NULL;\
    FPS_CONFLOOP_END\
    return RETURN_SUCCESS;\
}


#define INSERT_STD_PROCINFO_COMPUTEFUNC_START \
int processloopOK = 1;\
PROCESSINFO *processinfo = NULL;\
if( CLIcmddata.cmdsettings->flags & CLICMDFLAG_PROCINFO)\
{\
    char pinfodescr[200];\
    int slen = snprintf(pinfodescr, 200, "function %.10s", CLIcmddata.key);\
    if(slen<1) {\
        PRINT_ERROR("snprintf wrote <1 char");\
        abort();\
    }\
    if(slen >= 200) {\
        PRINT_ERROR("snprintf string truncation");\
        abort();\
    }\
    if( data.fpsptr != NULL ) {\
        processinfo = processinfo_setup(data.FPS_name, pinfodescr,\
            "startup", __FUNCTION__, __FILE__, __LINE__ );\
        fps_to_processinfo(data.fpsptr, processinfo);\
    }\
    else\
    {\
        processinfo = processinfo_setup(CLIcmddata.key, pinfodescr,\
            "startup", __FUNCTION__, __FILE__, __LINE__ );\
    }\
\
    processinfo->loopcntMax = CLIcmddata.cmdsettings->procinfo_loopcntMax;\
\
    processinfo->triggerstreamID = -2;\
    processinfo->triggermode = CLIcmddata.cmdsettings->triggermode;\
    strcpy(processinfo->triggerstreamname, CLIcmddata.cmdsettings->triggerstreamname);\
    processinfo->triggerdelay = CLIcmddata.cmdsettings->triggerdelay;\
    processinfo->triggertimeout = CLIcmddata.cmdsettings->triggertimeout;\
    processinfo->triggerstreamID = image_ID(processinfo->triggerstreamname); \
    processinfo_waitoninputstream_init(processinfo, processinfo->triggerstreamID, \
        CLIcmddata.cmdsettings->triggermode, -1); \
    processinfo->RT_priority = CLIcmddata.cmdsettings->RT_priority;\
    processinfo->CPUmask = CLIcmddata.cmdsettings->CPUmask;\
 \
    processinfo->MeasureTiming =  CLIcmddata.cmdsettings->procinfo_MeasureTiming;\
\
    DEBUG_TRACEPOINT("loopstart");\
    processinfo_loopstart(processinfo);\
}\
while(processloopOK == 1) \
{ \
	if( CLIcmddata.cmdsettings->flags & CLICMDFLAG_PROCINFO) {\
        DEBUG_TRACEPOINT("loopstep");\
        processloopOK = processinfo_loopstep(processinfo); \
        DEBUG_TRACEPOINT("waitoninputstream");\
	    processinfo_waitoninputstream(processinfo); \
        DEBUG_TRACEPOINT("exec_start");\
	    processinfo_exec_start(processinfo); \
    }\
    else {\
        processloopOK = 0;\
    }\
    int processcompstatus = 1;\
    if( CLIcmddata.cmdsettings->flags & CLICMDFLAG_PROCINFO) {\
        processcompstatus = processinfo_compute_status(processinfo);\
    }\
	if(processcompstatus == 1) \
	{


#define INSERT_STD_PROCINFO_COMPUTEFUNC_END \
	} \
	if( CLIcmddata.cmdsettings->flags & CLICMDFLAG_PROCINFO) {\
        processinfo_exec_end(processinfo); \
    }\
} \
if( CLIcmddata.cmdsettings->flags & CLICMDFLAG_PROCINFO) {\
    processinfo_cleanExit(processinfo);\
}





/** @brief FPS run function
 *
 * The FPS name is taken from data.FPS_name, which has to
 * have been set up by either the stand-alone function, or the
 * CLI.
 *
 * Running FPS_CONNECT macro in FPSCONNECT_RUN mode.
 *
 * ### GET FUNCTION PARAMETER VALUES
 *
 * Parameters are addressed by their tag name\n
 * These parameters are read once, before running the loop.\n
 *
 * FPS_GETPARAM... macros are wrapper to functionparameter_GetParamValue
 * and functionparameter_GetParamPtr functions, all defined in
 * fps_paramvalue.h.
 *
 * Each of the FPS_GETPARAM macro creates a variable with "_" prepended
 * to the first macro argument.
 *
 * Equivalent code without using macros:
 *
 * char _IDin_name[200];
 * strncpy(_IDin_name,  functionparameter_GetParamPtr_STRING(&fps, ".in_name"), FUNCTION_PARAMETER_STRMAXLEN);
 * long _delayus = functionparameter_GetParamValue_INT64(&fps, ".delayus");
 */
#define INSERT_STD_FPSRUNfunction \
static errno_t FPSRUNfunction()\
{\
    FPS_CONNECT(data.FPS_name, FPSCONNECT_RUN);\
    data.fpsptr = &fps;\
    STD_FARG_LINKfunction\
    errno_t fret = compute_function();\
    data.fpsptr = NULL;\
    function_parameter_RUNexit(&fps);\
    return fret;\
}




/** @brief FPSCLI function
 *
 * GET ARGUMENTS AND PARAMETERS
 * Try FPS implementation
 *
 * Set data.fpsname, providing default value as first arg, and set data.FPS_CMDCODE value.
 * Default FPS name will be used if CLI process has NOT been named.
 * See code in function_parameter.h for detailed rules.
 */
#define INSERT_STD_FPSCLIfunction \
static errno_t CLIfunction(void)\
{\
if( CLIcmddata.cmdsettings->flags & CLICMDFLAG_FPS )\
{\
function_parameter_getFPSargs_from_CLIfunc(CLIcmddata.key);\
if(data.FPS_CMDCODE != 0)\
{\
    data.FPS_CONFfunc = FPSCONFfunction;\
    data.FPS_RUNfunc  = FPSRUNfunction;\
    function_parameter_execFPScmd();\
    return RETURN_SUCCESS;\
}\
}\
\
errno_t retval = CLI_checkarg_array(farg, CLIcmddata.nbarg);\
if(retval == RETURN_CLICHECKARGARRAY_SUCCESS)\
{\
    data.fpsptr = NULL;\
    STD_FARG_LINKfunction\
    return compute_function();\
}\
if(retval == RETURN_CLICHECKARGARRAY_HELP)\
{\
    help_function();\
    printf("\n");\
    return RETURN_SUCCESS;\
}\
if(retval == RETURN_CLICHECKARGARRAY_FUNCPARAMSET)\
{\
return RETURN_SUCCESS;\
}\
\
return retval;\
}


#define INSERT_STD_FPSCLIfunctions \
INSERT_STD_FPSCONFfunction \
INSERT_STD_FPSRUNfunction \
INSERT_STD_FPSCLIfunction


#define INSERT_STD_CLIREGISTERFUNC \
{ \
    int cmdi = RegisterCLIcmd(CLIcmddata, CLIfunction); \
    CLIcmddata.cmdsettings = &data.cmd[cmdi].cmdsettings; \
}



/** make IMGID from name
 *
 * Some settings can be embedded in the image name string for convenience :
 *
 * Examples:
 * "im1" no optional setting, image name = im1
 * "s>im1" : set shared memory flag
 * "k10>im1" : number of keyword = 10
 * "c20>im1" : 20-sized circular buffer
 * "tD>im1" : datatype is double (64 bit floating point)
 *      D double precision float (64)
 *      F single precision float (32)
*/
static inline IMGID makeIMGID(
    CONST_WORD name
)
{
    IMGID img;

    // default values for image creation
    img.datatype = _DATATYPE_FLOAT;
    img.naxis = 2;
    img.size[0] = 1;
    img.size[1] = 1;
    img.shared = 0;
    img.NBkw = 100;
    img.CBsize = 0;

    char *pch;
    char *pch1;
    int nbword = 0;
    char namestring[200];
    strcpy(namestring, name);
    pch = strtok(namestring, ">");
    while(pch != NULL)
    {
        pch1 = pch;
        //printf("[%2d] %s\n", nbword, pch);

        if(strcmp(pch, "s") == 0)
        {
            printf("    shared memory\n");
            img.shared = 1;
        }



        if(strcmp(pch, "tui8") == 0)
        {
            printf("    data type unsigned 8-bit int\n");
            img.datatype = _DATATYPE_UINT8;
        }
        if(strcmp(pch, "tsi8") == 0)
        {
            printf("    data type signed 8-bit int\n");
            img.datatype = _DATATYPE_INT8;
        }
        if(strcmp(pch, "tui16") == 0)
        {
            printf("    data type unsigned 16-bit int\n");
            img.datatype = _DATATYPE_UINT16;
        }
        if(strcmp(pch, "tsi16") == 0)
        {
            printf("    data type signed 16-bit int\n");
            img.datatype = _DATATYPE_INT16;
        }
        if(strcmp(pch, "tui32") == 0)
        {
            printf("    data type unsigned 32-bit int\n");
            img.datatype = _DATATYPE_UINT32;
        }
        if(strcmp(pch, "tsi32") == 0)
        {
            printf("    data type signed 32-bit int\n");
            img.datatype = _DATATYPE_INT32;
        }
        if(strcmp(pch, "tui64") == 0)
        {
            printf("    data type unsigned 64-bit int\n");
            img.datatype = _DATATYPE_UINT64;
        }
        if(strcmp(pch, "tsi64") == 0)
        {
            printf("    data type signed 64-bit int\n");
            img.datatype = _DATATYPE_INT64;
        }

        if(strcmp(pch, "tf32") == 0)
        {
            printf("    data type double (32)\n");
            img.datatype = _DATATYPE_FLOAT;
        }
        if(strcmp(pch, "tf64") == 0)
        {
            printf("    data type float (64)\n");
            img.datatype = _DATATYPE_DOUBLE;
        }



        if(pch[0] == 'k')
        {
            int nbkw;
            sscanf(pch, "k%d", &nbkw);
            printf("    %d keywords\n", nbkw);
            img.NBkw = nbkw;
        }

        if(pch[0] == 'c')
        {
            int cbsize;
            sscanf(pch, "c%d", &cbsize);
            printf("    %d circular buffer size\n", cbsize);
            img.CBsize = cbsize;
        }



        pch = strtok(NULL, ">");
        nbword ++;
    }

    img.ID = -1;
    img.createcnt = -1;
    strcpy(img.name, pch1);
    img.im = NULL;
    img.md = NULL;

    return img;
}



static inline IMGID makeIMGID_2D(
    CONST_WORD name,
    uint32_t xsize,
    uint32_t ysize
)
{
    IMGID img = makeIMGID(name);
    img.naxis = 2;
    img.size[0] = xsize;
    img.size[1] = ysize;

    return img;
}

static inline IMGID makeIMGID_3D(
    CONST_WORD name,
    uint32_t xsize,
    uint32_t ysize,
    uint32_t zsize
)
{
    IMGID img = makeIMGID(name);
    img.naxis = 3;
    img.size[0] = xsize;
    img.size[1] = ysize;
    img.size[2] = zsize;

    return img;
}





/** Create image according to IMGID entries
 *
 */
static inline imageID imcreateIMGID(
    IMGID *img
)
{
    if(img->ID == -1)
    {
        printf("creating 2D image %s, shared = %d, kw = %d\n", img->name, img->shared,
               img->NBkw);

        DEBUG_TRACEPOINT("Creating 2D image");
        create_image_ID(
            img->name,
            img->naxis,
            img->size,
            img->datatype,
            img->shared,
            img->NBkw,
            img->CBsize,
            &img->ID
        );
        DEBUG_TRACEPOINT(" ");
        img->im = &data.image[img->ID];
        img->md = &data.image[img->ID].md[0];
        img->createcnt = data.image[img->ID].createcnt;
    }
    return img->ID;
}


static inline IMGID makesetIMGID(
    CONST_WORD name,
    imageID ID
)
{
    IMGID img;

    img.ID = ID;
    strcpy(img.name, name);

    img.im = &data.image[ID];
    img.md = &data.image[ID].md[0];
    img.createcnt = data.image[ID].createcnt;

    return img;
}



static inline imageID resolveIMGID(
    IMGID *img,
    int ERRMODE
)
{
    if(img->ID == -1)
    {
        // has not been previously resolved -> resolve
        img->ID = image_ID(img->name);
        if(img->ID > -1)
        {
            img->im = &data.image[img->ID];
            img->md = &data.image[img->ID].md[0];
            img->createcnt = data.image[img->ID].createcnt;
        }
    }
    else
    {
        // check that create counter matches and image is in use
        if((img->createcnt != data.image[img->ID].createcnt)
                || (data.image[img->ID].used != 1))
        {
            // create counter mismatch -> need to re-resolve
            img->ID = image_ID(img->name);
            if(img->ID > -1)
            {
                img->im = &data.image[img->ID];
                img->md = &data.image[img->ID].md[0];
                img->createcnt = data.image[img->ID].createcnt;
            }
        }

    }

    if(img->ID == -1)
    {
        if((ERRMODE == ERRMODE_FAIL) || (ERRMODE == ERRMODE_ABORT))
        {
            printf("ERROR: %c[%d;%dm Cannot resolve image %s %c[%d;m\n", (char) 27, 1, 31,
                   img->name, (char) 27, 0);
            abort();
        }
        else if(ERRMODE == ERRMODE_WARN)
        {
            printf("WARNING: %c[%d;%dm Cannot resolve image %s %c[%d;m\n", (char) 27, 1, 35,
                   img->name, (char) 27, 0);
        }
    }

    return img->ID;
}


#endif
