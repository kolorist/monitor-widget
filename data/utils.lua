---@param value number value to evaluate
---@param thresholds table color table
function eval_color_from_value(value, thresholds)
    local color = "#ffffff"
    for _, v in ipairs(thresholds) do
        if value >= v.value then
            color = v.color
        end
    end
    return color
end

---@param numBytes number value to evaluate
function bytes_to_str(numBytes)
    if numBytes < 1024 then
        return string.format("%.0f B", numBytes)
    elseif numBytes < 1048576 then
        local v = numBytes / 1024
        return string.format("%.1f KB", v)
    elseif numBytes < 1073741824 then
        local v = numBytes / 1048576
        return string.format("%.1f MB", v)
    else
        local v = numBytes / 1073741824;
        return string.format("%.1f GB", v)
    end
end
