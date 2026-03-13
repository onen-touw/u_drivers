#pragma once

#include "wfap.h"
#include "wfsta.h"
#include "bits/unique_ptr.h"

namespace ufo
{
    namespace drv
    {
        struct wf_t
        {
            using ap_t = ufo::wf_details::wfap_t;
            using sta_t = ufo::wf_details::wfsta_t;
            std::unique_ptr<ap_t> _ap;
            std::unique_ptr<sta_t> _sta;

            // todo 
        };
    } // namespace drv
} // namespace ufo
