// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>
#include <vector>

#include "ib/sim/datatypes.hpp"

namespace ib {
namespace sim {
//! The FlexRay namespace
namespace fr {

using FrMicroTick      = int32_t; //!< FlexRay micro tick
using FrMacroTick      = int32_t; //!< FlexRay macro tick

//! \brief Type and constants for the FlexRay channel parameter A, B, or AB
enum class Channel : uint8_t
{
    None = 0, //!< Invalid Channel
    A    = 1, //!< Channel A
    B    = 2, //!< Channel B
    AB   = 3 //!< Channel AB
};

//! \brief Period of the clock (used for micro tick period and sample clock period).
enum class ClockPeriod : uint8_t
{
    T12_5NS = 1, //!< 12.5ns / 80MHz
    T25NS = 2, //!< 25ns   / 40MHz
    T50NS = 3, //!< 50ns   / 20MHz
};

/*!
 * \brief Protocol relevant global cluster parameters
 *
 *  Cf. 'FlexRay Protocol Specification Version 3.0.1' Appendix B.3.1.1 Parameters.
 */
struct ClusterParameters
{
    //! Number of attempts for a cold start before giving up (range 2-31).
    uint8_t gColdstartAttempts;

    //! Max cycle count value in a given cluster (range 7-63, must be an odd integer).
    uint8_t gCycleCountMax;

    //! Time offset for a static slot in MacroTicks (MT) (range 1-63).
    uint16_t gdActionPointOffset;

    ////! Not used by Network Simulator
    //gdCASRxLowMax   

    //! Duration of the idle phase within a dynamic slot in gdMiniSlots (range 0-2).
    uint16_t gdDynamicSlotIdlePhase;

    ////! Not used by Network Simulator
    //gdIgnoreAfterTx

    //! Duration of a mini slot in MacroTicks (MT) (2-63).
    uint16_t gdMiniSlot;

    //! Time offset for a mini slot in MacroTicks (MT) (range 1-31).
    uint16_t gdMiniSlotActionPointOffset;

    //! Duration of a static slot in MacroTicks (MT) (3-664).
    uint16_t gdStaticSlot;

    //! Duration of the symbol window in MacroTicks (MT) (range 0-162).
    uint16_t gdSymbolWindow;

    //! Time offset for a static symbol windows in MacroTicks (MT) (range 1-63).
    uint16_t gdSymbolWindowActionPointOffset;

    //! Duration of TSS (Transmission Start Sequence) in gdBits (range 1-15).
    uint16_t gdTSSTransmitter;

    ////! Not used by Network Simulator
    //gdWakeupRxIdle

    ////! Not used by Network Simulator
    //gdWakeupRxLow

    ////! Not used by Network Simulator
    //gdWakeupRxWindow

    //! Duration of LOW Phase of a wakeup symbol in gdBit (range 15-60).
    uint16_t gdWakeupTxActive;

    //! Duration of the idle of a wakeup symbol in gdBit (45-180).
    uint16_t gdWakeupTxIdle;

    /*!
     * Upper limit for the startup listen timeout and wakeup listen timeout in the
     * presence of noise. Used as a multiplier of pdListenTimeout (range 2-16).
     */
    uint8_t gListenNoise;

    //! Number of MacroTicks (MT) per cycle, (range 8-16000).
    uint16_t gMacroPerCycle;

    //! Threshold used for testing the vClockCorrectionFailed counter (range 1-15).
    uint8_t gMaxWithoutClockCorrectionFatal;

    //! Threshold used for testing the vClockCorrectionFailed counter (range 1-15).
    uint8_t gMaxWithoutClockCorrectionPassive;

    //! Number of mini slots (range 0-7988).
    uint16_t gNumberOfMiniSlots;

    //! Number of static slots in a cycle (range 2-1023).
    uint16_t gNumberOfStaticSlots;

    //! Length of the payload of a static frame in 16-Bits words (range 0-127).
    uint16_t gPayloadLengthStatic;

    //! Max number of distinct sync frame identifiers present in a given cluster. (range 2-15).
    uint8_t gSyncFrameIDCountMax;
};

/*!
 * \brief Protocol relevant global node parameters
 *
 *  Cf. 'FlexRay Protocol Specification Version 3.0.1' Appendix B.3.2 Parameters.
 */
struct NodeParameters
{
    // ----------------------------------------------------------------------
    // Parameters according to B.3.2.1

    //! Controls the transition to halt state due to clock synchronization errors. (0,1).
    uint8_t pAllowHaltDueToClock;

    //! Required number of consecutive even / odd cycle pairs for normal passive to normal active (range 0-31).
    uint8_t pAllowPassiveToActive;

    //! Channel(s) to which the controller is connected (values Channel::A, Channel::B, Channel::AB).
    Channel pChannels;

    //! Cluster drift damping factor for rate correction in MicroTicks (range 0-10).
    uint8_t pClusterDriftDamping;

    //! Allowed deviation for startup frames during integration in MicroTicks (range 29-2743).
    FrMicroTick pdAcceptedStartupRange;

    ////! Not used by Network Simulator
    //pDecodingCorrection

    ////! Not used by Network Simulator
    //pDelayCompensationA

    ////! Not used by Network Simulator
    //pDelayCompensationB

    //! Duration of listen phase in MicroTicks (range 1926-2567692).
    FrMicroTick pdListenTimeout;

    ////! Not used by Network Simulator
    //pExternalSync

    ////! Not used by Network Simulator
    //pExternOffsetCorrection

    ////! Not used by Network Simulator
    //pExternRateCorrection

    ////! Not used by Network Simulator
    //pFallBackInternal

    //! Slot ID of the key slot (range 0-1023, value 0 means that there is no key slot).
    uint16_t pKeySlotId;

    //! Shall the node enter key slot only mode after startup. (values 0, 1) (AUTOSAR pSingleSlotEnabled).
    uint8_t pKeySlotOnlyEnabled;

    //! Key slot is used for startup (range 0, 1).
    uint8_t pKeySlotUsedForStartup;
    
    //! Key slot is used for sync (range 0, 1).
    uint8_t pKeySlotUsedForSync;

    //! Last mini slot which can be transmitted (range 0-7988).
    uint16_t pLatestTx;

    //! Initial startup offset for frame reference point on channel A (rang 2-68 MacroTicks (MT)).
    uint8_t pMacroInitialOffsetA;

    //! Initial startup offset for frame reference point on channel B (rang 2-68 MacroTicks (MT)).
    uint8_t pMacroInitialOffsetB;

    //! Offset between secondary time reference and MT boundary (range 0-239 MicroTicks).
    FrMicroTick pMicroInitialOffsetA;

    //! Offset between secondary time reference and MT boundary (range 0-239 MicroTicks).
    FrMicroTick pMicroInitialOffsetB;

    //! Nominal number of MicroTicks in the communication cycle (range 960-1280000).
    FrMicroTick pMicroPerCycle;

    //! Maximum permissible offset correction value (range 15-16082 MicroTicks).
    FrMicroTick pOffsetCorrectionOut;

    //! Start of the offset correction phase within the NIT, (7-15999 MT).
    uint16_t pOffsetCorrectionStart;

    //! Maximum permissible rate correction value (range 3-3846 MicroTicks).
    FrMicroTick pRateCorrectionOut;

    ////! Not used by Network Simulator
    //pSecondKeySlotID

    ////! Not used by Network Simulator
    //pTwoKeySlotMode

    //! Channel used by the node to send a wakeup pattern (values Channel::A, Channel::B).
    Channel pWakeupChannel;

    //! Number of repetitions of the wakeup symbol (range 0-63, value 0 or 1 prevents sending of WUP).
    uint8_t pWakeupPattern; 

    // ----------------------------------------------------------------------
    // Parameters according to B.3.2.2

    //! Duration of a FlexRay MicroTick (12.5ns, 25ns or 50ns).
    ClockPeriod pdMicrotick;

    ////! Not used by Network Simulator
    //pNMVectorEarlyUpdate

    ////! Not used by Network Simulator
    //pPayloadLengthDynMax

    //! Number of samples per MicroTick (values 1 or 2).
    uint8_t pSamplesPerMicrotick;
};

//! Transmission mode for FlexRay Tx-Buffer
enum class TransmissionMode : uint8_t
{
    SingleShot = 0, //!< Send TX Buffer only once
    Continuous = 1 //!< Send TX Buffer repeatedly
};

//! Configuration of Tx-Buffer, used in struct ControllerConfig
struct TxBufferConfig
{
    //! (values Channel::A, Channel::B, Channel::AB)
    Channel channels;

    //! The slot Id of frame
    uint16_t slotId;

    //! Base offset for cycle multiplexing (values 0-63).
    uint8_t offset;

    //! Repetition for cycle multiplexing (values 1,2,4,8,16,32,64).
    uint8_t repetition;

    //! Set the PPindicator
    bool hasPayloadPreambleIndicator;

    //! Header CRC, 11 bits
    uint16_t headerCrc;

    //! TransmissionMode::SingleShot or TransmissionMode::Continuous
    TransmissionMode transmissionMode;
};

//! Configure the communication parameters of the FlexRay controller.
struct ControllerConfig
{
    //! FlexRay cluster parameters
    ClusterParameters clusterParams;
    //! FlexRay node parameters
    NodeParameters nodeParams;

    //! FlexRay buffer configs
    std::vector<TxBufferConfig> bufferConfigs;
};

//! Update the configuration of a particular FlexRay TX-Buffer
struct TxBufferConfigUpdate
{
    //! Index of the TX-Buffers according to the configured buffers (cf. ControllerConfig).
    uint16_t txBufferIndex;
    //! The new configuration of the Tx-Buffer
    TxBufferConfig txBufferConfig;
};

//! Update the content of a FlexRay TX-Buffer
struct TxBufferUpdate
{
    //! Index of the TX Buffers according to the configured buffers (cf. ControllerConfig).
    uint16_t txBufferIndex;

    //! Payload data valid flag
    bool payloadDataValid;

    //! Raw payload containing 0 to 254 bytes.
    std::vector<uint8_t> payload;
};

enum class ChiCommand : uint8_t
{
    RUN, //!< ChiCommand RUN
    DEFERRED_HALT, //!< ChiCommand DEFERRED_HALT
    FREEZE, //!< ChiCommand FREEZE
    ALLOW_COLDSTART, //!< ChiCommand ALLOW_COLDSTART
    ALL_SLOTS, //!< ChiCommand ALL_SLOTS
    WAKEUP //!< ChiCommand WAKEUP
};

struct HostCommand
{
    ChiCommand command;
};

struct Header
{
    /*!
     * \brief Flags bit map according to FlagMask. Description:
     *  - [7-5]: unused
     *  - [4]: Reserved bit
     *  - [3]: PPIndicator: 0, regular payload; 1, NM vector or message ID
     *  - [2]: NFIndicator: 0, no valid payload data and PPIndicator = 0; 1, valid payload data
     *  - [1]: SyFIndicator: 0, frame not used for synchronization; 1, frame shall be used for sync
     *  - [0]: SuFIndicator: 0, not a startup frame; 1, a startup frame
     */
    uint8_t flags = 0;
    uint16_t frameId = 0; //!< Slot ID in which the frame was sent: 1 - 2047
    uint8_t payloadLength = 0; //!< Payload length, 7 bits
    uint16_t headerCrc = 0; //!< Header CRC, 11 bits
    uint8_t cycleCount = 0; //!< Cycle in which the frame was sent: 0 - 63


    // --------------------------------------------------------------------------------
    //  Convenience Accessors to Header Flags
    // --------------------------------------------------------------------------------

    //! Flag BitMask definition for helper methods
    enum class Flag : uint8_t
    {
        SuFIndicator = 1 << 0,
        SyFIndicator = 1 << 1,
        NFIndicator = 1 << 2,
        PPIndicator = 1 << 3
    };

    //! Convenience helper to check if a Flag is set
    inline bool IsSet(Flag flag) const;

    //! Convenience helper to set a Flag
    inline void Set(Flag flag);

    //! Convenience helper to clear a Flag
    inline void Clear(Flag flag);

    //! Convenience helper to set or clear a Flag according to a condition
    inline void Set(Flag flag, bool condition);
};

struct Frame
{
    Header header; //!< Header flags, slot, crc, and cycle indidcators
    std::vector<uint8_t> payload; //!< Raw payload containing 0 to 254 bytes
};

// Receive a frame from the Bus.
struct FrMessage
{
    std::chrono::nanoseconds timestamp; //!< Time at end of frame transmission
    Channel channel; //!< FlexRay channel A or B. (Valid values: Channel::A, Channel::B).
    Frame frame; //!< Received FlexRay frame
};

/*!
 * \brief Acknowledge for the transmit on the FlexRay bus
 */
struct FrMessageAck
{
    std::chrono::nanoseconds timestamp; //!< Time at end of frame transmission
    uint16_t txBufferIndex; //!< Tx buffer, that was used for the transmission
    Channel channel; //!< FlexRay channel A or B. (Valid values: Channel::A, Channel::B).
    Frame frame; //!< Copy of the FlexRay frame that was successfully transmitted
};

/*!
 * \brief FlexRay symbols patterns.
 */
enum class SymbolPattern : uint8_t
{
    CasMts, //!< Collision avoidance symbol (CAS) OR media access test symbol (MTS).
    Wus,    //!< Wakeup symbol (WUS).
    Wudop   //!< Wakeup During Operation Pattern (WUDOP).
};

/*!
 * \brief A FlexRay Symbol as received on the FlexRay bus.
 */
struct FrSymbol
{
    std::chrono::nanoseconds timestamp; //!< End time of symbol reception.
    Channel channel; //!< FlexRay channel A or B (values: Channel::A, Channel::B).
    SymbolPattern pattern; //!< The received symbol, e.g. wakeup pattern
};

struct FrSymbolAck : FrSymbol
{
};

/*!
 * \brief Indicate the start of a FlexRay cycle.
 */
struct CycleStart
{
    std::chrono::nanoseconds timestamp; //!< Cycle starting time.
    uint8_t cycleCounter; //!< Counter of FlexRay cycles.
};

/*!
 * \brief Protocol Operation Control (POC) state of the FlexRay communication controller
 * *AUTOSAR Name:* Fr_POCStateType
 */
enum class PocState : uint8_t
{
    DefaultConfig = 0, //!< CC expects configuration. Initial state after reset. 
    Config        = 1, //!< CC is in configuration mode for setting communication parameters
    Ready         = 2, //!< intermediate state for initialization process (after Config).
    Startup       = 3, //!< FlexRay startup phase
    Wakeup        = 4, //!< FlexRay wakeup phase
    NormalActive  = 5, //!< Normal operating mode
    NormalPassive = 6, //!< Operating mode with transient or tolerable errors
    Halt          = 7  //!< CC is halted (caused by the application (ChiCommand::DEFERRED_HALT) or by a fatal error).
};

/*!
 * \brief Status of the simulated FlexRay controller
 * \deprecated ControllerStatus is deprecated in favor of PocStatus which follows the AUTOSAR flexray model more closely.
 */
struct ControllerStatus
{
    std::chrono::nanoseconds timestamp;
    PocState pocState; //!< Status of the Protocol Operation Control (POC).
};

/*!
* \brief Indicates what slot mode the POC is in.
* *AUTOSAR Name:* Fr_SlotModeType
*/
enum class SlotModeType : uint8_t
{
    KeySlot = 0x00,
    AllPending,
    All,
};

/*!
* \brief Indicates what error mode the POC is in.
* *AUTOSAR Name:* Fr_ErrorModeType
*/
enum class ErrorModeType : uint8_t
{
    Active = 0x00,
    Passive,
    CommHalt,
};

/*!
* \brief Indicates the current substate in the startup procedure.
* *AUTOSAR Name:* Fr_StartupStateType
*/

enum class StartupStateType : uint8_t
{
    Undefined = 0x00,
    ColdStartListen,
    IntegrationColdstartCheck,
    ColdStartJoin,
    ColdStartCollisionResolution,
    ColdStartConsistencyCheck,
    IntegrationListen,
    InitializeSchedule,
    IntegrationConsistencyCheck,
    ColdStartGap,
    ExternalStartup,
};

/*!
* \brief Indicates the outcome of the wake-up mechanism.
* *AUTOSAR Name:* Fr_WakeupStateType
*/
enum class WakeupStatusType : uint8_t
{
    Undefined = 0x00,
    ReceivedHeader,
    ReceivedWup,
    CollisionHeader,
    CollisionWup,
    CollisionUnknown,
    Transmitted,
};

/*!
 * \brief Protocol Operation Control status as available in the AUTOSAR
 *        FlexRay driver model.
 *
 * This enhances the deprecated struct ControllerStatus by adding  members
 * that are available through the Controller Host Interface.
 * *AUTOSAR Name:* Fr_POCStatusType
 * 
 */
struct PocStatus
{
    std::chrono::nanoseconds timestamp; //!< IB timestamp

    PocState state; //!< Status of the Protocol Operation Control (POC).
    bool chiHaltRequest; //!< indicates whether a halt request was received from the CHI
    bool coldstartNoise; //!< indicates noisy channel conditions during coldstart 
    bool freeze; //!< indicates that the POC entered a halt state due to an error condition requiring immediate halt.
    bool chiReadyRequest; //!< indicates that the CHI requested to enter ready state at the end of the communication cycle.
    ErrorModeType errorMode; //!< indicates the error mode of the POC
    SlotModeType slotMode; //!< indicates the slot mode of the POC
    StartupStateType startupState; //!< indicates states within the STARTUP mechanism
    WakeupStatusType wakeupStatus; //!< outcome of the execution of the WAKEUP mechanism
};

// ================================================================================
//  Inline Implementations
// ================================================================================
bool Header::IsSet(Flag flag) const
{
    return flags & static_cast<uint8_t>(flag);
}

void Header::Set(Flag flag)
{
    flags |= static_cast<uint8_t>(flag);
}

void Header::Clear(Flag flag)
{
    flags &= ~static_cast<uint8_t>(flag);
}

void Header::Set(Flag flag, bool condition)
{
    if (condition)
        Set(flag);
    else
        Clear(flag);
}

inline bool operator==(const ClusterParameters& lhs, const ClusterParameters& rhs)
{
    return lhs.gColdstartAttempts == rhs.gColdstartAttempts
        && lhs.gCycleCountMax == rhs.gCycleCountMax
        && lhs.gdActionPointOffset == rhs.gdActionPointOffset
        && lhs.gdDynamicSlotIdlePhase == rhs.gdDynamicSlotIdlePhase
        && lhs.gdMiniSlot == rhs.gdMiniSlot
        && lhs.gdMiniSlotActionPointOffset == rhs.gdMiniSlotActionPointOffset
        && lhs.gdStaticSlot == rhs.gdStaticSlot
        && lhs.gdSymbolWindow == rhs.gdSymbolWindow
        && lhs.gdSymbolWindowActionPointOffset == rhs.gdSymbolWindowActionPointOffset
        && lhs.gdTSSTransmitter == rhs.gdTSSTransmitter
        && lhs.gdWakeupTxActive == rhs.gdWakeupTxActive
        && lhs.gdWakeupTxIdle == rhs.gdWakeupTxIdle
        && lhs.gListenNoise == rhs.gListenNoise
        && lhs.gMacroPerCycle == rhs.gMacroPerCycle
        && lhs.gMaxWithoutClockCorrectionFatal == rhs.gMaxWithoutClockCorrectionFatal
        && lhs.gMaxWithoutClockCorrectionPassive == rhs.gMaxWithoutClockCorrectionPassive
        && lhs.gNumberOfMiniSlots == rhs.gNumberOfMiniSlots
        && lhs.gNumberOfStaticSlots == rhs.gNumberOfStaticSlots
        && lhs.gPayloadLengthStatic == rhs.gPayloadLengthStatic
        && lhs.gSyncFrameIDCountMax == rhs.gSyncFrameIDCountMax;
}

inline bool operator==(const NodeParameters& lhs, const NodeParameters& rhs)
{
    return lhs.pAllowHaltDueToClock == rhs.pAllowHaltDueToClock
        && lhs.pAllowPassiveToActive == rhs.pAllowPassiveToActive
        && lhs.pChannels == rhs.pChannels
        && lhs.pClusterDriftDamping == rhs.pClusterDriftDamping
        && lhs.pdAcceptedStartupRange == rhs.pdAcceptedStartupRange
        && lhs.pdListenTimeout == rhs.pdListenTimeout
        && lhs.pKeySlotId == rhs.pKeySlotId
        && lhs.pKeySlotOnlyEnabled == rhs.pKeySlotOnlyEnabled
        && lhs.pKeySlotUsedForStartup == rhs.pKeySlotUsedForStartup
        && lhs.pKeySlotUsedForSync == rhs.pKeySlotUsedForSync
        && lhs.pLatestTx == rhs.pLatestTx
        && lhs.pMacroInitialOffsetA == rhs.pMacroInitialOffsetA
        && lhs.pMacroInitialOffsetB == rhs.pMacroInitialOffsetB
        && lhs.pMicroInitialOffsetA == rhs.pMicroInitialOffsetA
        && lhs.pMicroInitialOffsetB == rhs.pMicroInitialOffsetB
        && lhs.pMicroPerCycle == rhs.pMicroPerCycle
        && lhs.pOffsetCorrectionOut == rhs.pOffsetCorrectionOut
        && lhs.pOffsetCorrectionStart == rhs.pOffsetCorrectionStart
        && lhs.pRateCorrectionOut == rhs.pRateCorrectionOut
        && lhs.pWakeupChannel == rhs.pWakeupChannel
        && lhs.pWakeupPattern == rhs.pWakeupPattern
        && lhs.pdMicrotick == rhs.pdMicrotick
        && lhs.pSamplesPerMicrotick == rhs.pSamplesPerMicrotick;
}

inline bool operator==(const TxBufferConfig& lhs, const TxBufferConfig& rhs)
{
    return lhs.channels == rhs.channels
        && lhs.slotId == rhs.slotId
        && lhs.offset == rhs.offset
        && lhs.repetition == rhs.repetition
        && lhs.hasPayloadPreambleIndicator == rhs.hasPayloadPreambleIndicator
        && lhs.headerCrc == rhs.headerCrc
        && lhs.transmissionMode == rhs.transmissionMode;
}

} // namespace fr
} // namespace sim
} // namespace ib
