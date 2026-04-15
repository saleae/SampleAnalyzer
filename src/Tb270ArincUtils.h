#ifndef TB270_ARINC_UTILS_H
#define TB270_ARINC_UTILS_H

#include <cstddef>
#include <cstdint>

namespace TB270 {
namespace detail {

// arinc labels are octal (use leading zeros)
enum class Label : uint8_t {
    // clang-format off
    NONE                        = 0,
    BATTERY_DISCRETE_OUTPUTS    = 0271,
    DC_CURRENT                  = 0340,
    DC_VOLTAGE                  = 0341,
    BATTERY_TEMPERATURE         = 0342,
    DELIVERABLE_ENERGY          = 0343,
    CAPACITY                    = 0344,
    STATE_OF_CHARGE             = 0345,
    ESTIMATED_LIFE_REMAINING    = 0346,
    SOFTWARE_VERSION            = 0350,
    OPERATIONAL_STATUS          = 0351,
    FAULT_STATUS                = 0352,
    CONFIGURATION_ID            = 0353,
    EQUIPMENT_ID                = 0377
    // clang-format on
};

struct BnrDescriptor {
    uint8_t sig_bits;  // doesn't include sign bit
    bool is_signed;    // true => explicit sign bit (in addition to magnitude bits)
    float resolution;  // engineering units per LSB
};

struct BnrDescriptors {
    // clang-format off
    static constexpr BnrDescriptor dc_current               {15, true,  0.125f};
    static constexpr BnrDescriptor dc_voltage               {15, false, 0.001953f};
    static constexpr BnrDescriptor battery_temperature      {12, true,  0.125f};
    static constexpr BnrDescriptor deliverable_energy       {13, false, 0.0625f};
    static constexpr BnrDescriptor capacity                 {13, false, 0.0625f};
    static constexpr BnrDescriptor state_of_charge          {11, false, 0.0625f};
    static constexpr BnrDescriptor estimated_life_remaining {11, false, 0.0625f};
    static constexpr BnrDescriptor configuration_id         {16, false, 1.0f};
    // clang-format on
};

struct WordFields {
    uint8_t label;
    uint8_t sdi;
    uint32_t data;
    uint8_t ssm;
    uint8_t parity;
};

WordFields decode_word(uint32_t word);
uint8_t reverse_label_bits(uint8_t label);

float decode_bnr_data(const BnrDescriptor& d, uint32_t data_field);
uint32_t encode_bnr_data(const BnrDescriptor& d, float data_field);

const char* label_name(Label label);
const BnrDescriptor* descriptor_for_label(Label label);

// Formats a parsed ARINC word for display in the analyzer UI.
void format_word_summary(uint32_t word, char* buffer, size_t buffer_size);

}  // namespace detail
}  // namespace TB270

#endif // TB270_ARINC_UTILS_H
