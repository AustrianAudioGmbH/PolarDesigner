#pragma once

#include "Constants.hpp"
#include "Logging.hpp"

#include <cstddef>
#include <juce_core/juce_core.h>

static float hzFromZeroToOne (size_t nProcessorBands, size_t idx, float val)
{
    switch (nProcessorBands)
    {
        case 1:
            return 0.0f;
        case 2:
            return XOVER_RANGE_START_2B[idx]
                   + val * (XOVER_RANGE_END_2B[idx] - XOVER_RANGE_START_2B[idx]);
        case 3:
            return XOVER_RANGE_START_3B[idx]
                   + val * (XOVER_RANGE_END_3B[idx] - XOVER_RANGE_START_3B[idx]);
        case 4:
            return XOVER_RANGE_START_4B[idx]
                   + val * (XOVER_RANGE_END_4B[idx] - XOVER_RANGE_START_4B[idx]);
        case 5:
            return XOVER_RANGE_START_5B[idx]
                   + val * (XOVER_RANGE_END_5B[idx] - XOVER_RANGE_START_5B[idx]);
        default:
            LOG_ERROR ("Invalid number of bands: " + juce::String (nProcessorBands));
            return 0.0f;
    }
}

static float hzToZeroToOne (size_t nProcessorBands, size_t idx, float hz)
{
    switch (nProcessorBands)
    {
        case 1:
            return 0.0f;
        case 2:
            return (hz - XOVER_RANGE_START_2B[idx])
                   / (XOVER_RANGE_END_2B[idx] - XOVER_RANGE_START_2B[idx]);
        case 3:
            return (hz - XOVER_RANGE_START_3B[idx])
                   / (XOVER_RANGE_END_3B[idx] - XOVER_RANGE_START_3B[idx]);
        case 4:
            return (hz - XOVER_RANGE_START_4B[idx])
                   / (XOVER_RANGE_END_4B[idx] - XOVER_RANGE_START_4B[idx]);
        case 5:
            return (hz - XOVER_RANGE_START_5B[idx])
                   / (XOVER_RANGE_END_5B[idx] - XOVER_RANGE_START_5B[idx]);
        default:
            LOG_ERROR ("Invalid number of bands: " + juce::String (nProcessorBands));
            return 0.0f;
    }
}
