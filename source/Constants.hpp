/*
 ==============================================================================
 Constants.hpp
 Author: Sebastian Grill

 Copyright (c) 2025 - Austrian Audio GmbH
 www.austrian.audio

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ==============================================================================
 */

#pragma once

static constexpr int PD_DEFAULT_BLOCK_SIZE = 1024;

/* PolarDesigner has a maximum of 5 EQ's .. */
static constexpr unsigned int MAX_NUM_EQS = 5;
/* .. and functions on a maximum of 2 inputs only. */
static constexpr int MAX_NUM_INPUTS = 2;

// initial xover frequencies for several numbers of bands
static constexpr float INIT_XOVER_FREQS_2B[1] = { 1000.0f };
static constexpr float INIT_XOVER_FREQS_3B[2] = { 250.0f, 3000.0f };
static constexpr float INIT_XOVER_FREQS_4B[3] = { 200.0f, 1000.0f, 5000.0f };
static constexpr float INIT_XOVER_FREQS_5B[4] = { 150.0f, 600.0f, 2600.0f, 8000.0f };

// xover ranges for several numbers of bands
static constexpr float XOVER_RANGE_START_2B[1] = { 120.0f };
static constexpr float XOVER_RANGE_END_2B[1] = { 12000.0f };
static constexpr float XOVER_RANGE_START_3B[2] = { 120.0f, 2000.0f };
static constexpr float XOVER_RANGE_END_3B[2] = { 1000.0f, 12000.0f };
static constexpr float XOVER_RANGE_START_4B[3] = { 120.0f, 900.0f, 4000.0f };
static constexpr float XOVER_RANGE_END_4B[3] = { 450.0f, 2500.0f, 12000.0f };
static constexpr float XOVER_RANGE_START_5B[4] = { 120.0f, 500.0f, 2200.0f, 7000.0f };
static constexpr float XOVER_RANGE_END_5B[4] = { 200.0f, 1100.0f, 4000.0f, 12000.0f };

// TODO: check if this is a duplicate of MAX_NUM_INPUTS
static constexpr int N_CH_IN = 2;

static constexpr int FILTER_BANK_NATIVE_SAMPLE_RATE = 48000;
static constexpr int FILTER_BANK_IR_LENGTH_AT_NATIVE_SAMPLE_RATE = 401;

static constexpr int DF_EQ_LEN = 512;
static constexpr int FF_EQ_LEN = 512;
static constexpr int EQ_SAMPLE_RATE = 48000;
