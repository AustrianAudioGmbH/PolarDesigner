-- Function to find a track by name
function FindTrackByName(name)
  local track_count = reaper.CountTracks(0)
  for i = 0, track_count - 1 do
    local track = reaper.GetTrack(0, i)
    local _, track_name = reaper.GetTrackName(track, "")
    if track_name == name then
      return track, i
    end
  end
  return nil, -1
end

-- Function to find a region by name
function FindRegionByName(name)
  local i = 0
  while true do
    local retval, isrgn, start_time, end_time, rgn_name = reaper.EnumProjectMarkers3(0, i)
    if not retval then break end
    if isrgn and rgn_name == name then
      if start_time < end_time then
        return start_time, end_time
      else
        reaper.ShowConsoleMsg("Error: Invalid region '" .. name .. "' (start_time >= end_time)\n")
        return nil, nil
      end
    end
    i = i + 1
  end
  return nil, nil
end

-- Function to check if a send exists from source to render track
function CheckSendExists(source_track, render_track)
  local send_count = reaper.GetTrackNumSends(source_track, 0)
  for i = 0, send_count - 1 do
    local dest_track = reaper.GetTrackSendInfo_Value(source_track, 0, i, "P_DESTTRACK")
    if dest_track == render_track then
      local send_vol = reaper.GetTrackSendInfo_Value(source_track, 0, i, "D_VOL")
      local send_mode = reaper.GetTrackSendInfo_Value(source_track, 0, i, "I_SENDMODE")
      reaper.ShowConsoleMsg("Send volume: " .. send_vol .. ", Mode: " .. send_mode .. " (0=post-fader, 1=pre-fader, 3=post-FX)\n")
      return send_vol > 0 and (send_mode == 0 or send_mode == 3)
    end
  end
  return false
end

-- Function to count armed tracks for debugging
function CountArmedTracks()
  local count = 0
  local track_count = reaper.CountTracks(0)
  for i = 0, track_count - 1 do
    local track = reaper.GetTrack(0, i)
    if reaper.GetMediaTrackInfo_Value(track, "I_RECARM") == 1 then
      count = count + 1
    end
  end
  return count
end

-- Function to determine if a track is a folder track
function IsFolderTrack(track)
  if not track then return false end
  local folderDepth = reaper.GetMediaTrackInfo_Value(track, "I_FOLDERDEPTH")
  return folderDepth == 1
end

-- Function to expand tracks with "Render" in their name
function ExpandRenderTracks()
  reaper.PreventUIRefresh(1)
  reaper.Undo_BeginBlock2(0)

  local trackCount = reaper.CountTracks(0)
  local firstRenderTrack = nil

  for i = 0, trackCount - 1 do
    local track = reaper.GetTrack(0, i)
    reaper.SetTrackSelected(track, false)
  end

  for i = 0, trackCount - 1 do
    local track = reaper.GetTrack(0, i)
    local _, trackName = reaper.GetTrackName(track, "")
    if string.match(trackName, "Render") then
      reaper.SetTrackSelected(track, true)
      reaper.SetMediaTrackInfo_Value(track, "I_HEIGHTOVERRIDE", 250)
      if not firstRenderTrack then
        firstRenderTrack = track
      end
    else
      reaper.SetMediaTrackInfo_Value(track, "I_HEIGHTOVERRIDE", 24)
    end
  end

  if firstRenderTrack then
    reaper.Main_OnCommand(40913, 0)
  else
    reaper.ShowConsoleMsg("No tracks with 'Render' in the name found.\n")
  end

  reaper.TrackList_AdjustWindows(false)
  reaper.UpdateArrange()
  reaper.Undo_EndBlock2(0, "Expand Tracks with Render in Name", 1)
  reaper.PreventUIRefresh(-1)
end

-- Function to log plugin information for debugging
function LogPluginInfo(track, track_name)
  local fx_count = reaper.TrackFX_GetCount(track)
  reaper.ShowConsoleMsg("Plugins on " .. track_name .. " (" .. fx_count .. "):\n")
  for i = 0, fx_count - 1 do
    local _, fx_name = reaper.TrackFX_GetFXName(track, i, "")
    local enabled = reaper.TrackFX_GetEnabled(track, i) and "Enabled" or "Bypassed"
    reaper.ShowConsoleMsg("  FX " .. i .. ": " .. fx_name .. " (" .. enabled .. ")\n")
  end
end

-- Function to set up a new Reaper project
function SetupNewProject()
  reaper.Main_OnCommand(40023, 0) -- File: New project
  reaper.Undo_BeginBlock()

  -- Create SEND track
  reaper.InsertTrackAtIndex(0, true)
  local send_track = reaper.GetTrack(0, 0)
  reaper.GetSetMediaTrackInfo_String(send_track, "P_NAME", "SEND", true)
  reaper.SetMediaTrackInfo_Value(send_track, "I_NCHAN", 2) -- Stereo track

  -- Add PolarDesigner and PolarDesigner3 plugins
  local pd_idx = reaper.TrackFX_AddByName(send_track, "PolarDesigner", false, -1)
  local pd3_idx = reaper.TrackFX_AddByName(send_track, "PolarDesigner3", false, -1)
  if pd_idx == -1 or pd3_idx == -1 then
    reaper.ShowMessageBox("Error: PolarDesigner or PolarDesigner3 not found!", "Error", 0)
    return false
  end

  -- Create render tracks
  reaper.InsertTrackAtIndex(1, true)
  local render_v2 = reaper.GetTrack(0, 1)
  reaper.GetSetMediaTrackInfo_String(render_v2, "P_NAME", "PolarDesigner_Render", true)
  reaper.SetMediaTrackInfo_Value(render_v2, "I_NCHAN", 2)

  reaper.InsertTrackAtIndex(2, true)
  local render_v3 = reaper.GetTrack(0, 2)
  reaper.GetSetMediaTrackInfo_String(render_v3, "P_NAME", "PolarDesigner3_Render", true)
  reaper.SetMediaTrackInfo_Value(render_v3, "I_NCHAN", 2)

  -- Create sends
  reaper.CreateTrackSend(send_track, render_v2)
  reaper.SetTrackSendInfo_Value(send_track, 0, 0, "D_VOL", 1.0)
  reaper.SetTrackSendInfo_Value(send_track, 0, 0, "I_SENDMODE", 3) -- Post-FX

  reaper.CreateTrackSend(send_track, render_v3)
  reaper.SetTrackSendInfo_Value(send_track, 0, 1, "D_VOL", 1.0)
  reaper.SetTrackSendInfo_Value(send_track, 0, 1, "I_SENDMODE", 3)

  -- Set up recording settings for render tracks
  reaper.SetMediaTrackInfo_Value(render_v2, "I_RECMODE", 2) -- Stereo output
  reaper.SetMediaTrackInfo_Value(render_v2, "I_RECINPUT", -1) -- No input
  reaper.SetMediaTrackInfo_Value(render_v2, "I_RECMON", 1)
  reaper.SetMediaTrackInfo_Value(render_v3, "I_RECMODE", 2)
  reaper.SetMediaTrackInfo_Value(render_v3, "I_RECINPUT", -1)
  reaper.SetMediaTrackInfo_Value(render_v3, "I_RECMON", 1)

  reaper.Undo_EndBlock("Setup New Project", -1)
  return true
end

-- Function to render a track's output with specific plugin enabled
function RenderTrack(source_track, render_track, source_name, render_name, start_time, end_time, enable_polar_designer, callback)
  if not source_track or not render_track or start_time >= end_time then
    reaper.ShowConsoleMsg("Error: Invalid track or region for " .. render_name .. "\n")
    if callback then callback() end
    return false
  end

  -- Set plugin bypass states
  local pd_idx = reaper.TrackFX_GetByName(source_track, "PolarDesigner", false)
  local pd3_idx = reaper.TrackFX_GetByName(source_track, "PolarDesigner3", false)
  reaper.TrackFX_SetEnabled(source_track, pd_idx, enable_polar_designer)
  reaper.TrackFX_SetEnabled(source_track, pd3_idx, not enable_polar_designer)

  LogPluginInfo(source_track, source_name)

  if not CheckSendExists(source_track, render_track) then
    reaper.ShowConsoleMsg("Error: No valid send from " .. source_name .. " to " .. render_name .. "\n")
    if callback then callback() end
    return false
  end

  local source_item_count = reaper.CountTrackMediaItems(source_track)
  reaper.ShowConsoleMsg("Items on source track " .. source_name .. ": " .. source_item_count .. "\n")
  if source_item_count == 0 then
    reaper.ShowConsoleMsg("Warning: No media items on " .. source_name .. " in region!\n")
  end

  local track_states = {}
  for i = 0, reaper.CountTracks(0) - 1 do
    local track = reaper.GetTrack(0, i)
    track_states[i] = {
      mute = reaper.GetMediaTrackInfo_Value(track, "B_MUTE"),
      arm = reaper.GetMediaTrackInfo_Value(track, "I_RECARM"),
      mon = reaper.GetMediaTrackInfo_Value(track, "I_RECMON")
    }
  end

  reaper.PreventUIRefresh(1)
  for i = 0, reaper.CountTracks(0) - 1 do
    local track = reaper.GetTrack(0, i)
    local _, track_name = reaper.GetTrackName(track, "")
    if track_name ~= source_name and track_name ~= render_name and not IsFolderTrack(track) then
      reaper.SetMediaTrackInfo_Value(track, "B_MUTE", 1)
    else
      reaper.SetMediaTrackInfo_Value(track, "B_MUTE", 0)
    end
  end
  reaper.PreventUIRefresh(-1)

  reaper.SetMediaTrackInfo_Value(render_track, "I_RECARM", 1)
  reaper.SetMediaTrackInfo_Value(render_track, "I_RECMON", 1)
  local is_armed = reaper.GetMediaTrackInfo_Value(render_track, "I_RECARM")
  local is_monitored = reaper.GetMediaTrackInfo_Value(render_track, "I_RECMON")
  reaper.ShowConsoleMsg("Render track " .. render_name .. " armed: " .. is_armed .. ", monitored: " .. is_monitored .. "\n")
  reaper.ShowConsoleMsg("Number of armed tracks: " .. CountArmedTracks() .. "\n")

  reaper.SetEditCurPos(start_time, true, false)
  reaper.GetSet_LoopTimeRange(1, 1, start_time, end_time, false)

  reaper.Main_OnCommand(1016, 0) -- Stop any playback/recording
  reaper.Main_OnCommand(40046, 0) -- Start recording

  local timeout = reaper.time_precise() + 60
  local last_play_state = reaper.GetPlayState()
  local function monitorPlayback()
    local play_state = reaper.GetPlayState()
    local current_time = reaper.GetPlayPosition()
    if last_play_state ~= play_state then
      reaper.ShowConsoleMsg("Play state changed: " .. play_state .. " (1=playing, 2=recording, 3=playing+recording), Position: " .. current_time .. "\n")
      last_play_state = play_state
    end

    if play_state & 1 == 1 and current_time < end_time and reaper.time_precise() < timeout then
      reaper.defer(monitorPlayback)
    else
      reaper.Main_OnCommand(40667, 0)
      reaper.SetMediaTrackInfo_Value(render_track, "I_RECARM", 0)
      reaper.SetMediaTrackInfo_Value(render_track, "I_RECMON", 0)

      reaper.PreventUIRefresh(1)
      for i, state in pairs(track_states) do
        local track = reaper.GetTrack(0, i)
        reaper.SetMediaTrackInfo_Value(track, "B_MUTE", state.mute)
        reaper.SetMediaTrackInfo_Value(track, "I_RECARM", state.arm)
        reaper.SetMediaTrackInfo_Value(track, "I_RECMON", state.mon)
      end
      reaper.PreventUIRefresh(-1)

      local item_count = reaper.CountTrackMediaItems(render_track)
      reaper.ShowConsoleMsg("Finished rendering " .. render_name .. ", items recorded: " .. item_count .. "\n")
      if item_count == 0 then
        reaper.ShowConsoleMsg("Error: No media items recorded on " .. render_name .. "\n")
      end

      if callback then callback() end
    end
  end
  reaper.defer(monitorPlayback)
  return true
end

-- Main function with state machine
function Main()
  -- Setup new project
  if not SetupNewProject() then
    reaper.ShowMessageBox("Failed to set up project!", "Error", 0)
    return
  end

  local start_time, end_time = FindRegionByName("TestRegion")
  if not start_time or not end_time then
    reaper.ShowMessageBox("Region named 'TestRegion' not found or invalid!", "Error", 0)
    return
  end

  reaper.ShowConsoleMsg("Region starts: " .. start_time .. ", ends: " .. end_time .. ", duration: " .. (end_time - start_time) .. " seconds\n")

  ExpandRenderTracks()
  reaper.Undo_BeginBlock()

  local track_pairs = {
    {source = "SEND", render = "PolarDesigner_Render", enable_polar_designer = true},
    {source = "SEND", render = "PolarDesigner3_Render", enable_polar_designer = false}
  }

  local state = { current_pair = 1 }
  local function processNextPair()
    if state.current_pair > #track_pairs then
      reaper.PreventUIRefresh(1)
      for i = 0, reaper.CountTracks(0) - 1 do
        local track = reaper.GetTrack(0, i)
        reaper.SetMediaTrackInfo_Value(track, "B_MUTE", 0)
        reaper.SetMediaTrackInfo_Value(track, "I_RECARM", 0)
        reaper.SetMediaTrackInfo_Value(track, "I_RECMON", 0)
      end
      reaper.GetSet_LoopTimeRange(1, 1, 0, 0, false)
      reaper.PreventUIRefresh(-1)
      reaper.ShowConsoleMsg("Rendering complete!\n")
      reaper.Undo_EndBlock("Render Tracks in TestRegion", -1)
      return
    end

    reaper.ShowConsoleMsg("Processing pair " .. state.current_pair .. " of " .. #track_pairs .. "\n")
    local pair = track_pairs[state.current_pair]
    local source_track, source_idx = FindTrackByName(pair.source)
    local render_track, render_idx = FindTrackByName(pair.render)

    if not source_track or not render_track then
      reaper.ShowConsoleMsg("Error: Track not found: " .. pair.source .. " or " .. pair.render .. "\n")
      reaper.Undo_EndBlock("Render Tracks in TestRegion", -1)
      return
    end

    reaper.ShowConsoleMsg("Rendering Plugin track " .. pair.source .. " to Render Track " .. pair.render .. "...\n")
    local success = RenderTrack(source_track, render_track, pair.source, pair.render, start_time, end_time, pair.enable_polar_designer, function()
      state.current_pair = state.current_pair + 1
      reaper.defer(processNextPair)
    end)

    if not success then
      reaper.ShowConsoleMsg("Failed to render " .. pair.render .. ", stopping script.\n")
      reaper.Undo_EndBlock("Render Tracks in TestRegion", -1)
    end
  end

  reaper.defer(processNextPair)
end

-- Run the script
Main()
