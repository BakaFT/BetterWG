#include "Windows.h"
#include "include/capi/cef_app_capi.h"

#include "common.h"
#include "config.h"
#include "divert.h"
#include "filters/teamEloFilter.h"

typedef int (*TCefBrowserHostCreateBrowser)(const cef_window_info_t* windowInfo, struct _cef_client_t* client,
    const cef_string_t* url, const struct _cef_browser_settings_t* settings,
    struct _cef_dictionary_value_t* extra_info,
    struct _cef_request_context_t* request_context);

divert div_hook_filter;
TCefBrowserHostCreateBrowser fpOriginal_filter;

static void HookRequestHandler(cef_client_t* client)
{   
    static auto GetRequestHandler = client->get_request_handler;
    client->get_request_handler = [](cef_client_t* self) -> cef_request_handler_t* {
        auto requestHandler = GetRequestHandler(self);
        static auto GetResourceRequestHandler = requestHandler->get_resource_request_handler;
        requestHandler->get_resource_request_handler =
            [](cef_request_handler_t* self, cef_browser_t* browser, cef_frame_t* frame, cef_request_t* request,
                int is_navigation, int is_download, const cef_string_t* request_initiator,
                int* disable_default_handling) -> cef_resource_request_handler_t* {
                    auto resourceRequestHandler = GetResourceRequestHandler(
                        self, browser, frame, request, is_navigation, is_download, request_initiator, disable_default_handling);

                    static auto GetResourceResponseFilter = resourceRequestHandler->get_resource_response_filter;
                    resourceRequestHandler->get_resource_response_filter =
                        [](cef_resource_request_handler_t* self, cef_browser_t* browser, cef_frame_t* frame,
                            cef_request_t* request, cef_response_t* response) -> cef_response_filter_t* {
                                auto _url = request->get_url(request);
                                std::wstring urlWstr = std::wstring(_url->str, _url->length);
                               
                                // https://wegame.gtimg.com/g.26-r.c2d3c/helper/lol/v2/assets/battle-detail-base.$(6-digithash).js                                                            
                                // If length of url is 89 and [59:77] == "battle-detail-base"
                                if (urlWstr.length() == 89 && urlWstr.substr(59, 18) == L"battle-detail-base")
                                {
                                    auto teamEloFilter = new TeamEloFilter();      
                                    return (cef_response_filter_t*)teamEloFilter;
                                }

                                return GetResourceResponseFilter(self, browser, frame, request, response);
                        };

                    return resourceRequestHandler;
            };

        return requestHandler;
        };
}

static int Hooked_CefBrowserHost_CreateBrowser(const cef_window_info_t* windowInfo, struct _cef_client_t* client,
    const cef_string_t* url, const struct _cef_browser_settings_t* settings,
    struct _cef_dictionary_value_t* extra_info,
    struct _cef_request_context_t* request_context)
{
    div_hook_filter.unhook();
#ifdef ENABLE_REQUEST_INTERCEPTING
    HookRequestHandler(client);
#endif
    int ret = fpOriginal_filter(windowInfo, client, url, settings, extra_info, request_context);

    div_hook_filter.hook(fpOriginal_filter, &Hooked_CefBrowserHost_CreateBrowser);

    return ret;
}

void HookCefBroswserHostCreate()
{
    fpOriginal_filter = reinterpret_cast<TCefBrowserHostCreateBrowser>(
        helper::get_module_export("libcef.dll", "cef_browser_host_create_browser"));
    if (fpOriginal_filter)
    {
        div_hook_filter.hook(fpOriginal_filter, &Hooked_CefBrowserHost_CreateBrowser);
    }
}