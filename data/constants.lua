---@enum ETextAlignment
TextAlignment = {
    Left = 0,
    Center = 1,
    Right = 2,
}

---@enum ELineAlignment
LineAlignment = {
    Top = 0,
    Center = 1,
    Bottom = 2,
}

-------------------------------------------------------------------------------
-- User-specific constants
-------------------------------------------------------------------------------

LoadThresholds = {
    { value = 0,  color = "#0079ff" },
    { value = 25, color = "#00dfa2" },
    { value = 50, color = "#f6fa70" },
    { value = 75, color = "#ff0060" },
}

TemperatureThresholds = {
    { value = 0,  color = "#0079ff" },
    { value = 20, color = "#00dfa2" },
    { value = 35, color = "#f6fa70" },
    { value = 50, color = "#ff0060" },
}

Icon = {
    Memory = "\238\165\144",
    Graphics = "\238\175\146",
    CPULoad = "\238\177\138",
    Temperature = "\238\167\138",
    Frequency = "\238\167\153",
    Video = "\238\158\134",
    Cooling = "\238\158\188",
    Upload = "\238\183\155",
    Download = "\238\183\156",
    Processing = "\238\167\181",
    Cloud = "\238\157\147",

    -- Profiling
    Scripting = "\238\167\185",
    Timing = "\238\167\146",
}
