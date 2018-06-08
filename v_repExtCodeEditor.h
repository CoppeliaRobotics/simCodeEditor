#pragma once

#ifdef _WIN32
    #define VREP_DLLEXPORT extern "C" __declspec(dllexport)
#endif
#if defined (__linux) || defined (__APPLE__)
    #define VREP_DLLEXPORT extern "C"
#endif


VREP_DLLEXPORT unsigned char v_repStart(void* reservedPointer,int reservedInt);
VREP_DLLEXPORT void v_repEnd();
VREP_DLLEXPORT void* v_repMessage(int message,int* auxiliaryData,void* customData,int* replyData);

VREP_DLLEXPORT char* codeEditor_openModal(const char* initText,const char* properties,int* positionAndSize);
VREP_DLLEXPORT int codeEditor_open(const char* initText,const char* properties);
VREP_DLLEXPORT int codeEditor_setText(int handle,const char* text,int insertMode);
VREP_DLLEXPORT char* codeEditor_getText(int handle);
VREP_DLLEXPORT int codeEditor_show(int handle,int showState);
VREP_DLLEXPORT int codeEditor_close(int handle,int* positionAndSize);
