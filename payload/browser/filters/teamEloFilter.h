#pragma once
#include "common.h"
#include "include/capi/cef_app_capi.h"

class TeamEloFilter : public CefRefCount<cef_response_filter_t>
{
public:
    TeamEloFilter() : CefRefCount(this)
    {
        cef_response_filter_t::init_filter = _init_filter;
        cef_response_filter_t::filter = _filter;
    }

private:
    static int CEF_CALLBACK _init_filter(struct _cef_response_filter_t* self)
    {
        return 1;
    }

    static cef_response_filter_status_t CEF_CALLBACK _filter(struct _cef_response_filter_t* self, void* data_in,
        size_t data_in_size, size_t* data_in_read, void* data_out,
        size_t data_out_size, size_t* data_out_written)
    {        
        const char* data_in_ptr = static_cast<char*>(data_in);
        std::string src(data_in_ptr, data_in_size);

        const std::string target_str = "!((l=t.battleDetailTeamData)==null?void";
        const std::string insert_str = "Y(P(et),{text:'TeamELO'},{default:Q(()=>[h('div',Bn,['TeamELO:',h('"
            "span',null,w(o.battleDetailTeamData.teamElo),1)])]),_:1}),";

        // Try to find the target string in the data chunk
        size_t pos = src.find(target_str);

        if (pos != std::string::npos)
        {
            // Concatenate the insert string and target string
            std::string fragment = insert_str + target_str;
            // Move the data ( ab + target + cd )
            // 1> before the target string(ab)
            memcpy(data_out, data_in, pos);
            // 2> the concatenated string (target+insert)
            memcpy(static_cast<char*>(data_out) + pos, fragment.c_str(), fragment.length());
            // 3> after the target string (cd)
            memcpy(static_cast<char*>(data_out) + pos + fragment.length(),
                static_cast<char*>(data_in) + pos + target_str.length(), data_in_size - pos - target_str.length());

            *data_out_written = data_in_size + insert_str.length();
            *data_in_read = data_in_size;
        }
        else
        {
            // Injection point not found in this chunk, pass it through
            *data_out_written = data_in_size < data_out_size ? data_in_size : data_out_size;
            if (*data_out_written > 0)
            {
                memcpy(data_out, data_in, *data_out_written);
                *data_in_read = *data_out_written;
            }
        }

        return RESPONSE_FILTER_DONE;
    }
};