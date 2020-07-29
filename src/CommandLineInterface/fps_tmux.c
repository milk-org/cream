/**
 * @file    fps_tmux.c
 * @brief   tmux session management
 */


#include "CommandLineInterface/CLIcore.h"



errno_t functionparameter_FPS_tmux_kill(
    FUNCTION_PARAMETER_STRUCT *fps,
    int fpsindex
)
{
    // terminate tmux sessions
    EXECUTE_SYSTEM_COMMAND("tmux send-keys -t %s:ctrl C-c 2> /dev/null",
                           fps[fpsindex].md->name);
    EXECUTE_SYSTEM_COMMAND("tmux send-keys -t %s:ctrl \"exit\" C-m 2> /dev/null",
                           fps[fpsindex].md->name);
                           
    EXECUTE_SYSTEM_COMMAND("tmux send-keys -t %s:conf C-c 2> /dev/null",
                           fps[fpsindex].md->name);                           
    EXECUTE_SYSTEM_COMMAND("tmux send-keys -t %s:conf \"exit\" C-m 2> /dev/null",
                           fps[fpsindex].md->name);

    EXECUTE_SYSTEM_COMMAND("tmux send-keys -t %s:run C-c 2> /dev/null",
                           fps[fpsindex].md->name);
    EXECUTE_SYSTEM_COMMAND("tmux send-keys -t %s:run \"exit\" C-m 2> /dev/null",
                           fps[fpsindex].md->name);


	EXECUTE_SYSTEM_COMMAND("tmux kill-session -t %s 2> /dev/null", fps[fpsindex].md->name);

    return RETURN_SUCCESS;
}




/** @brief Initialize FPS tmux sesssions
 *
 */
errno_t functionparameter_FPS_tmux_init(
    FUNCTION_PARAMETER_STRUCT *fps,
    int fpsindex
)
{
	int funcstring_maxlen = 2000;
	int argstring_maxlen = 1000;
	
    // terminate tmux sessions
    functionparameter_FPS_tmux_kill(fps, fpsindex);

    EXECUTE_SYSTEM_COMMAND("tmux kill-session -t %s 2> /dev/null", fps[fpsindex].md->name);

    EXECUTE_SYSTEM_COMMAND("tmux new-session -s %s -d", fps[fpsindex].md->name);
    EXECUTE_SYSTEM_COMMAND("tmux rename-window -t %s:0 ctrl", fps[fpsindex].md->name);


    EXECUTE_SYSTEM_COMMAND("tmux new-window -t %s -n conf", fps[fpsindex].md->name);
    EXECUTE_SYSTEM_COMMAND("tmux new-window -t %s -n run", fps[fpsindex].md->name);


    // Write functions to tmux windows
    //
    char functionstring[funcstring_maxlen];
    char argstring[argstring_maxlen];
    char argstringcp[argstring_maxlen];

	if(fps[fpsindex].md->NBnameindex > 0)
	{		
		snprintf(argstring, argstring_maxlen, "%s", fps[fpsindex].md->nameindexW[0]);
	}
	else
	{
		sprintf(argstring, " ");
	}
	
	for(int i=1; i<fps[fpsindex].md->NBnameindex; i++)
	{
		snprintf(argstringcp, argstring_maxlen, "%s %s", argstring, fps[fpsindex].md->nameindexW[i]);
		strcpy(argstring, argstringcp);
	}


	// confstart
	//
    sprintf(functionstring,
            "function fpsconfstart {\n"
            "echo \"STARTING CONF PROCESS\"\n"
            "%s-exec -n %s \\\"%s _CONFSTART_ %s\\\"\n"
            "}\n",
            fps[fpsindex].md->callprogname,
            fps[fpsindex].md->name,
            fps[fpsindex].md->callfuncname,
            argstring
           );

    EXECUTE_SYSTEM_COMMAND("tmux send-keys -t %s:conf \"%s\" C-m", 
		fps[fpsindex].md->name,
		functionstring);

    
    // runstart
    //
    sprintf(functionstring,
            "function fpsrunstart {\n"
            "echo \"STARTING RUN PROCESS\"\n"
            "%s-exec -n %s \\\"%s _RUNSTART_ %s\\\"\n"
            "}\n",
            fps[fpsindex].md->callprogname,
            fps[fpsindex].md->name,
            fps[fpsindex].md->callfuncname,
            argstring
           );

    EXECUTE_SYSTEM_COMMAND("tmux send-keys -t %s:run \"%s\" C-m", 
		fps[fpsindex].md->name,
		functionstring);    

    
    // runstop
    //
    sprintf(functionstring,
            "function fpsrunstop {\n"
            "echo \"STOPPING RUN PROCESS\"\n"
            "%s-exec -n %s \\\"%s _RUNSTOP_ %s\\\"\n"
            "}\n",
            fps[fpsindex].md->callprogname,
            fps[fpsindex].md->name,
            fps[fpsindex].md->callfuncname,
            argstring
           );

    EXECUTE_SYSTEM_COMMAND("tmux send-keys -t %s:run \"%s\" C-m", 
		fps[fpsindex].md->name,
		functionstring);    
    


    return RETURN_SUCCESS;
}
