#include "v_repExtCodeEditor.h"
#include "scintillaDlg.h"
#include "v_repLib.h"
#include <iostream>

#ifdef _WIN32
    #include <direct.h>
#else
    #include <string.h>
    #define _stricmp(x,y) strcasecmp(x,y)
#endif

#define PLUGIN_VERSION 1

LIBRARY vrepLib;
CScintillaDlg* editor;
bool closeIt=false;

VREP_DLLEXPORT unsigned char v_repStart(void* reservedPointer,int reservedInt)
{
     char curDirAndFile[1024];
 #ifdef _WIN32
     _getcwd(curDirAndFile, sizeof(curDirAndFile));
 #elif defined (__linux) || defined (__APPLE__)
     getcwd(curDirAndFile, sizeof(curDirAndFile));
 #endif
     std::string currentDirAndPath(curDirAndFile);
     std::string temp(currentDirAndPath);
 #ifdef _WIN32
     temp+="/v_rep.dll";
 #elif defined (__linux)
     temp+="/libv_rep.so";
 #elif defined (__APPLE__)
     temp+="/libv_rep.dylib";
 #endif
    vrepLib=loadVrepLibrary(temp.c_str());
    if (vrepLib==NULL)
    {
        std::cout << "Error, could not find or correctly load the V-REP library. Cannot start 'CodeEditor' plugin.\n";
        return(0);
    }
    if (getVrepProcAddresses(vrepLib)==0)
    {
        std::cout << "Error, could not find all required functions in the V-REP library. Cannot start 'CodeEditor' plugin.\n";
        unloadVrepLibrary(vrepLib);
        return(0);
    }

    if (simGetBooleanParameter(sim_boolparam_headless)>0)
    {
        std::cout << "V-REP runs in headless mode. Cannot start 'CodeEditor' plugin.\n";
        unloadVrepLibrary(vrepLib);
        return(0);
    }

    editor=new CScintillaDlg((QWidget*)simGetMainWindow(1));

    simSetModuleInfo("CodeEditor",0,"Dummy Code Editor Plugin",NULL);
    simSetModuleInfo("CodeEditor",1,__DATE__,NULL);

    return(PLUGIN_VERSION);
}

VREP_DLLEXPORT void v_repEnd()
{
    delete editor;
    unloadVrepLibrary(vrepLib);
}

VREP_DLLEXPORT void* v_repMessage(int message,int* auxiliaryData,void* customData,int* replyData)
{
    if (message=sim_message_eventcallback_instancepass)
    {
        if ((editor!=NULL)&&editor->closeRequest)
            simEventNotification("<event origin=\"codeEditor\" msg=\"close\" handle=\"0\"></event>");
    }
    if ( (message=sim_message_eventcallback_guipass)&&closeIt )
    {
        if (editor!=NULL)
        {
            delete editor;
            editor=NULL;
        }
    }

    return(NULL);
}

VREP_DLLEXPORT char* codeEditor_openModal(const char* initText,const char* properties,int* positionAndSize)
{
    printf("codeEditor_openModal\nInitText: %s\nproperties: %s\n",initText,properties);

    // Return values:
    if (positionAndSize!=NULL)
    {
        positionAndSize[0]=0; // x
        positionAndSize[1]=0; // y
        positionAndSize[2]=400; // sx
        positionAndSize[3]=500; // sy
    }
    const char* txt="Hello from modal code editor";
    char* buff=simCreateBuffer(strlen(txt)+1);
    strcpy(buff,txt);
    return(buff);
}
VREP_DLLEXPORT int codeEditor_open(const char* initText,const char* properties)
{
    printf("codeEditor_open\nInitText: %s\nproperties: %s\n",initText,properties);

    return(-1);
}
VREP_DLLEXPORT int codeEditor_setText(int handle,const char* text,int insertMode)
{
    printf("codeEditor_setText\nHandle: %i\nText: %s\ninsertMode: %i\n",handle,text,insertMode);

    return(-1);
}
VREP_DLLEXPORT char* codeEditor_getText(int handle)
{
    printf("codeEditor_getText\nHandle: %i\n",handle);


    // Return values:
    const char* txt="Hello from code editor";
    char* buff=simCreateBuffer(strlen(txt)+1);
    strcpy(buff,txt);
    return(buff);
}
VREP_DLLEXPORT int codeEditor_show(int handle,int showState)
{
    printf("codeEditor_show\nHandle: %i\nshowState %i\n",handle,showState);

    return(-1);
}
VREP_DLLEXPORT int codeEditor_close(int handle,int* positionAndSize)
{
    printf("codeEditor_close\nHandle: %i\n",handle);

    closeIt=true;

    // Return values:
    if (positionAndSize!=NULL)
    {
        positionAndSize[0]=0; // x
        positionAndSize[1]=0; // y
        positionAndSize[2]=400; // sx
        positionAndSize[3]=500; // sy
    }
    return(-1);
}
