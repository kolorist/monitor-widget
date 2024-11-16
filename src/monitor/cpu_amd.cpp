#include "cpu_amd.h"

#include "km_driver.h"

namespace cpu
{

// ref: Processor Programming Reference (PPR) for AMD Family
constexpr u32 k_THM_TCON_CUR_TMP = 0x00059800;
constexpr u32 k_CUR_TEMP_RANGE_SEL = 0x00080000;

void AMDReadProcessorTemperature(f32* o_packageTemp, f32* o_coreTemps, u32* i_coreIds, u32 i_numCores)
{
    bool needSMNAccess = (o_packageTemp != nullptr) || (o_coreTemps != nullptr);

    if (needSMNAccess && kmdrv::BeginPCI())
    {
        if (o_packageTemp)
        {
            u32 value = kmdrv::ReadSMN(k_THM_TCON_CUR_TMP);
            f32 temperature = f32((value & 0xFFE00000) >> 21); // 10-bit raw value
            if (value & k_CUR_TEMP_RANGE_SEL)                  // reports on range [-49, 206]
            {
                constexpr f32 rangeScale = 255.0f / 2048.0f;
                temperature = temperature * rangeScale - 49.0f;
            }
            else // reports on range [0, 225]
            {
                constexpr f32 rangeScale = 225.0f / 2048.0f;
                temperature = temperature * rangeScale;
            }
            *o_packageTemp = temperature;
        }
        kmdrv::EndPCI();
    }
}

} // namespace cpu
