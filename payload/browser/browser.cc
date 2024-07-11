#include "Windows.h"
#include "common.h"
#include "config.h"
#include "divert.h"
#include "include/capi/cef_app_capi.h"

typedef int (*TCefInitialize)(const struct _cef_main_args_t* args, const struct _cef_settings_t* settings,
    cef_app_t* application, void* windows_sandbox_info);

divert div_hook_remote;
TCefInitialize fpOriginal;

static decltype(cef_app_t::on_before_command_line_processing) OnBeforeCommandLineProcessing;

static void CEF_CALLBACK Hooked_OnBeforeCommandLineProcessing(struct _cef_app_t* self, const cef_string_t* process_type,
    struct _cef_command_line_t* command_line)
{
    CEF_STR(remote_debugging_cef, remote_debugging, "remote-debugging-port");
    CEF_STR(port_main_cef, port_main, PORT_MAIN_BROWSER);
    CEF_STR(port_offscreen_cef, port_offscreen, PORT_OFFSCREEN_BROWSER);
    CEF_STR(offscreen_cef, offscreen, "offscreen");

    if (command_line->get_switch_value(command_line, &offscreen_cef) == NULL)
    {
        command_line->append_switch_with_value(command_line, &remote_debugging_cef, &port_main_cef);
    }
    else
    {
        // Advertisement background in login page, will be destroyed after login
#ifdef HOOK_OFFSCREEN_BROWSER_PROCESS
        command_line->append_switch_with_value(command_line, &remote_debugging_cef, &port_offscreen_cef);
#endif
    }
}

static int Hooked_CefInitialize(const struct _cef_main_args_t* args, const struct _cef_settings_t* settings,
    cef_app_t* application, void* windows_sandbox_info)
{
    div_hook_remote.unhook();

#ifdef ENABLE_REMOTE_DEBUGGING_PORT
    OnBeforeCommandLineProcessing = application->on_before_command_line_processing;
    application->on_before_command_line_processing = Hooked_OnBeforeCommandLineProcessing;
#endif
    int ret = fpOriginal(args, settings, application, windows_sandbox_info);

    div_hook_remote.hook(fpOriginal, &Hooked_CefInitialize);

    return ret;
}

void HookCefInitialize()
{
    fpOriginal = reinterpret_cast<TCefInitialize>(helper::get_module_export("libcef.dll", "cef_initialize"));
    if (fpOriginal)
    {
        div_hook_remote.hook(fpOriginal, &Hooked_CefInitialize);
    }
}