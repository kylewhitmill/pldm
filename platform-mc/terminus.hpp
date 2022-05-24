#pragma once

#include "libpldm/platform.h"

#include "common/types.hpp"
#include "requester/handler.hpp"

#include <sdbusplus/server/object.hpp>
#include <sdeventplus/event.hpp>
#include <xyz/openbmc_project/Inventory/Item/Board/server.hpp>

#include <algorithm>
#include <bitset>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace pldm
{
namespace platform_mc
{

using SensorId = uint16_t;
using SensorCnt = uint8_t;
using NameLanguageTag = std::string;
using SensorName = std::string;
using SensorAuxiliaryNames = std::tuple<
    SensorId, SensorCnt,
    std::vector<std::vector<std::pair<NameLanguageTag, SensorName>>>>;

/**
 * @brief Terminus
 *
 * Terminus class holds the TID, supported PLDM Type or PDRs which are needed by
 * other manager class for sensor monitoring and control.
 */
class Terminus
{
  public:
    Terminus(pldm_tid_t tid, uint64_t supportedPLDMTypes);

    /** @brief Check if the terminus supports the PLDM type message
     *
     *  @param[in] type - PLDM Type
     *  @return support state - True if support, otherwise False
     */
    bool doesSupportType(uint8_t type);

    /** @brief Check if the terminus supports the PLDM command message
     *
     *  @param[in] type - PLDM Type
     *  @param[in] command - PLDM command
     *  @return support state - True if support, otherwise False
     */
    bool doesSupportCommand(uint8_t type, uint8_t command);

    /** @brief Set the supported PLDM commands for terminus
     *
     *  @param[in] cmds - bit mask of the supported PLDM commands
     *  @return success state - True if success, otherwise False
     */
    bool setSupportedCommands(const std::vector<uint8_t>& cmds)
    {
        const size_t expectedSize = PLDM_MAX_TYPES *
                                    (PLDM_MAX_CMDS_PER_TYPE / 8);
        if (cmds.empty() || cmds.size() != expectedSize)
        {
            lg2::error(
                "setSupportedCommands received invalid bit mask size. Expected: {EXPECTED}, Received: {RECEIVED}",
                "EXPECTED", expectedSize, "RECEIVED", cmds.size());
            return false;
        }

        /* Assign Vector supportedCmds by Vector cmds */
        supportedCmds.resize(cmds.size());
        std::copy(cmds.begin(), cmds.begin() + cmds.size(),
                  supportedCmds.begin());

        return true;
    }

    /** @brief Parse the PDRs stored in the member variable, pdrs.
     */
    void parseTerminusPDRs();

    /** @brief The getter to return terminus's TID */
    pldm_tid_t getTid()
    {
        return tid;
    }

    /** @brief A list of PDRs fetched from Terminus */
    std::vector<std::vector<uint8_t>> pdrs{};

    /** @brief A flag to indicate if terminus has been initialized */
    bool initialized = false;

    /** @brief Get Sensor Auxiliary Names by sensorID
     *
     *  @param[in] id - sensor ID
     *  @return sensor auxiliary names
     */
    std::shared_ptr<SensorAuxiliaryNames> getSensorAuxiliaryNames(SensorId id);

  private:
    /** @brief Parse the numeric sensor PDRs
     *
     *  @param[in] pdrData - the response PDRs from GetPDR command
     *  @return pointer to numeric sensor info struct
     */
    std::shared_ptr<pldm_numeric_sensor_value_pdr>
        parseNumericSensorPDR(const std::vector<uint8_t>& pdrData);

    /** @brief Parse the sensor Auxiliary name PDRs
     *
     *  @param[in] pdrData - the response PDRs from GetPDR command
     *  @return pointer to sensor Auxiliary name info struct
     */
    std::shared_ptr<SensorAuxiliaryNames>
        parseSensorAuxiliaryNamesPDR(const std::vector<uint8_t>& pdrData);

    /** @brief Parse the compact numeric sensor PDRs
     *
     *  @param[in] pdrData - the response PDRs from GetPDR command
     *  @return pointer to compact numeric sensor info struct
     */
    std::shared_ptr<pldm_compact_numeric_sensor_pdr>
        parseCompactNumericSensorPDR(const std::vector<uint8_t>& pdrData);

    /** @brief Parse the sensor Auxiliary name from compact numeric sensor PDRs
     *
     *  @param[in] pdrData - the response PDRs from GetPDR command
     *  @return pointer to sensor Auxiliary name info struct
     */
    std::shared_ptr<SensorAuxiliaryNames>
        parseCompactNumericSensorNames(const std::vector<uint8_t>& pdrData);

    /* @brief The terminus's TID */
    pldm_tid_t tid;

    /* @brief The supported PLDM command types of the terminus */
    std::bitset<64> supportedTypes;

    /** @brief Store supported PLDM commands of a terminus
     *         Maximum number of PLDM Type is PLDM_MAX_TYPES
     *         Maximum number of PLDM command for each type is
     *         PLDM_MAX_CMDS_PER_TYPE.
     *         Each uint8_t can store the supported state of 8 PLDM commands.
     *         Size of supportedCmds will be
     *         PLDM_MAX_TYPES * (PLDM_MAX_CMDS_PER_TYPE / 8).
     */
    std::vector<uint8_t> supportedCmds;

    /* @brief Sensor Auxiliary Name list */
    std::vector<std::shared_ptr<SensorAuxiliaryNames>>
        sensorAuxiliaryNamesTbl{};
};
} // namespace platform_mc
} // namespace pldm
