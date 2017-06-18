#include "reaper_plugin.h"
#include "reaper_plugin_functions.h"
#include "VgmstreamDecoder.h"

#define IMPAPI(x) if (!((*((void **)&(x)) = (void *)rec->GetFunc(#x)))) impapierrcnt++;

REAPER_PLUGIN_HINSTANCE g_hInst=0;

PCM_source *(*PCM_Source_CreateFromSimple)(ISimpleMediaDecoder *dec, const char *fn);
void (*format_timestr)(double tpos, char *buf, int buflen);
void (*update_disk_counters)(int read, int write);
void (*ShowConsoleMsg)(const char* msg);

// output diagnostics messages using Reaper's currently available console
#define REAPER_DEBUG_OUTPUT_TRACING

PCM_source *CreateFromType(const char *type, int priority)
{
    if (priority >= 0)
    {
    if (!strcmp(type, "VGMSTREAM"))
      return PCM_Source_CreateFromSimple(new VgmstreamDecoder, 0);
    }

    return NULL;
}

PCM_source *CreateFromFile(const char *filename, int priority)
{
    size_t lfn=strlen(filename);
    if (priority >= 4)
    {
      VgmstreamDecoder* d = new VgmstreamDecoder;
      d->Open(filename, 0, 0, 0);
      if (d->IsOpen()) {
        return PCM_Source_CreateFromSimple(d, 0);
      }
      else {
        delete d;
      }
    }
    return NULL;
}

// this is used for UI only, not so muc
const char *EnumFileExtensions(int i, char **descptr) // call increasing i until returns a string, if descptr's output is NULL, use last description
{
    DEBUGLOGLN("Enumerating extensions");
    if (i == 0)
    {
      // TODO: Enumerate all supported extensions
        if (descptr) *descptr = "ADX Audio";
        return "ADX";
    }
    if (descptr) *descptr=NULL;
    return NULL;
}


pcmsrc_register_t myRegStruct={CreateFromType,CreateFromFile,EnumFileExtensions};

const char *(*GetExePath)();

extern "C"
{

REAPER_PLUGIN_DLL_EXPORT int REAPER_PLUGIN_ENTRYPOINT(REAPER_PLUGIN_HINSTANCE hInstance, reaper_plugin_info_t *rec)
{
    
    g_hInst=hInstance;
    if (rec)
    {
        if (rec->caller_version != REAPER_PLUGIN_VERSION || !rec->GetFunc)
            return 0;
        int impapierrcnt=0;
        IMPAPI(PCM_Source_CreateFromSimple);
        IMPAPI(format_timestr);
        IMPAPI(update_disk_counters);
        IMPAPI(ShowConsoleMsg);
        if (impapierrcnt)
        {
            ShowConsoleMsg("Errors importing Reaper API functions, aborting loading vgmstream wrapper");
            return 0;
        }
        rec->Register("pcmsrc",&myRegStruct);
        return 1;
    }
    return 0;
}
}
