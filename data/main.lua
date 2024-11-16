load_module("utils.lua")
load_module("constants.lua")

local hTextFont = -1
local hIconFont = -1

-------------------------------------------------------------------------------
-- Initialization
-------------------------------------------------------------------------------

---Called once at launch, guaranted to come before everything else, use this to
---initialize stuffs.
function on_initialize()
    print("on_initialize")
    set_update_interval(1000)
    set_widget_size(340, 300)

    -- debug
    debug_set_layout_draw(false)

    hTextFont = load_font("Segoe UI Variable Display Semib", 13)
    hIconFont = load_font("Segoe Fluent Icons", 16)
end

-------------------------------------------------------------------------------
-- Main update and drawing entry point
-------------------------------------------------------------------------------

---Periodically called from the main application, configure the calling rate using
---`ApplicationConfigs.updateInterval` in `configs.lua`.
---@param w integer
---@param h integer
function on_update(w, h)
    local cpuLoad = get_processor_utilization()
    local cpuPkgTemp = get_processor_temperature()
    local ramLoad = get_ram_utilization()
    local netStats = get_network_stats()
    local gpuLoad = get_gpu_utilization()
    local vramLoad = get_vram_utilization()
    local gpuTemp = get_gpu_temperature()

    local cpuLoadStr = string.format("<color='%s'>%.1f%%</>",
        eval_color_from_value(cpuLoad, LoadThresholds), cpuLoad)
    local cpuPkgTempStr = string.format("<color='%s'>%.0f\194\176C</>",
        eval_color_from_value(cpuPkgTemp, TemperatureThresholds), cpuPkgTemp);
    local ramLoadStr = string.format("<color='%s'>%d%%</>, <color='%s'>(%d%%)</>",
        eval_color_from_value(ramLoad.physicalLoad, LoadThresholds), ramLoad.physicalLoad,
        eval_color_from_value(ramLoad.virtualLoad, LoadThresholds), ramLoad.virtualLoad)
    local egressStr = string.format("<color='#b2533e'>%s/s</>", bytes_to_str(netStats.egress))
    local ingressStr = string.format("<color='#186f65'>%s/s</>", bytes_to_str(netStats.ingress))
    local gpuLoadStr = string.format("<color='%s'>%.1f%%</>",
        eval_color_from_value(gpuLoad.graphicsEngineLoad, LoadThresholds), gpuLoad.graphicsEngineLoad)
    local vramLoadStr = string.format("<color='%s'>%d%%</>",
        eval_color_from_value(vramLoad, LoadThresholds), vramLoad)
    local gpuTempStr = string.format("<color='%s'>%.0f\194\176C</>",
        eval_color_from_value(gpuTemp, TemperatureThresholds), gpuTemp);

    local m = h / 2
    draw_text(hIconFont, 0, 0, 25, m, Icon.CPULoad, TextAlignment.Center, LineAlignment.Center)
    draw_text(hTextFont, 25, 0, 40, m, cpuLoadStr, TextAlignment.Left, LineAlignment.Center)
    draw_text(hIconFont, 65, 0, 25, m, Icon.Memory, TextAlignment.Center, LineAlignment.Center)
    draw_text(hTextFont, 90, 0, 80, m, ramLoadStr, TextAlignment.Left, LineAlignment.Center)
    draw_text(hIconFont, 170, 0, 25, m, Icon.Temperature, TextAlignment.Center, LineAlignment.Center)
    draw_text(hTextFont, 195, 0, 40, m, cpuPkgTempStr, TextAlignment.Left, LineAlignment.Center)

    draw_text(hIconFont, 0, m, 25, m, Icon.Graphics, TextAlignment.Center, LineAlignment.Center)
    draw_text(hTextFont, 25, m, 40, m, gpuLoadStr, TextAlignment.Left, LineAlignment.Center)
    draw_text(hIconFont, 65, m, 25, m, Icon.Memory, TextAlignment.Center, LineAlignment.Center)
    draw_text(hTextFont, 90, m, 40, m, vramLoadStr, TextAlignment.Left, LineAlignment.Center)
    draw_text(hIconFont, 130, m, 25, m, Icon.Temperature, TextAlignment.Center, LineAlignment.Center)
    draw_text(hTextFont, 155, m, 40, m, gpuTempStr, TextAlignment.Left, LineAlignment.Center)

    draw_text(hIconFont, 235, 0, 25, m, "<color='#b2533e'>" .. Icon.Upload .. "</>", TextAlignment.Center, LineAlignment.Center)
    draw_text(hTextFont, 260, 0, 80, m, egressStr, TextAlignment.Left, LineAlignment.Center)
    draw_text(hIconFont, 235, m, 25, m, "<color='#186f65'>" .. Icon.Download .. "</>", TextAlignment.Center, LineAlignment.Center)
    draw_text(hTextFont, 260, m, 80, m, ingressStr, TextAlignment.Left, LineAlignment.Center)

end

-------------------------------------------------------------------------------
-- Runtime event callbacks
-------------------------------------------------------------------------------

---Called when user double clicks the application icon in the tray area
function on_mode_switched()
    print("on_mode_switched")
end
