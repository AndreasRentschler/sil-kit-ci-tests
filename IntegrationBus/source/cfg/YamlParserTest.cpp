// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "YamlParser.hpp"

#include "ParticipantConfiguration.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include  <chrono>

namespace {

class YamlParserTest : public testing::Test
{
};

using namespace ib::cfg;
using namespace ib::cfg::v1::datatypes;
using namespace std::chrono_literals;

//!< Yaml config which has almost complete list of config elements.
const auto completeConfiguration= R"raw(
Description: Example configuration to test YAML Parser
CanControllers:
- Name: CAN1
  Replay:
    UseTraceSource: Source1
    Direction: Both
    MdfChannel:
      ChannelName: MyTestChannel1
      ChannelPath: path/to/myTestChannel1
      ChannelSource: MyTestChannel
      GroupName: MyTestGroup
      GroupPath: path/to/myTestGroup1
      GroupSource: MyTestGroup
  UseTraceSinks:
  - Sink1
- Name: MyCAN2
  Network: CAN2
LinControllers:
- Name: SimpleEcu1_LIN1
  Network: LIN1
  Replay:
    UseTraceSource: Source1
    Direction: Both
    MdfChannel:
      ChannelName: MyTestChannel1
      ChannelPath: path/to/myTestChannel1
      ChannelSource: MyTestChannel
      GroupName: MyTestGroup
      GroupPath: path/to/myTestGroup1
      GroupSource: MyTestGroup
  UseTraceSinks:
  - MyTraceSink1
EthernetControllers:
- MacAddress: F6:04:68:71:AA:C2
  Name: ETH0
  Replay:
    UseTraceSource: Source1
    Direction: Receive
    MdfChannel:
      ChannelName: MyTestChannel1
      ChannelPath: path/to/myTestChannel1
      ChannelSource: MyTestChannel
      GroupName: MyTestGroup
      GroupPath: path/to/myTestGroup1
      GroupSource: MyTestGroup
  UseTraceSinks:
  - MyTraceSink1
FlexRayControllers:
- ClusterParameters:
    gColdstartAttempts: 8
    gCycleCountMax: 63
    gListenNoise: 2
    gMacroPerCycle: 3636
    gMaxWithoutClockCorrectionFatal: 2
    gMaxWithoutClockCorrectionPassive: 2
    gNumberOfMiniSlots: 291
    gNumberOfStaticSlots: 70
    gPayloadLengthStatic: 16
    gSyncFrameIDCountMax: 15
    gdActionPointOffset: 2
    gdDynamicSlotIdlePhase: 1
    gdMiniSlot: 5
    gdMiniSlotActionPointOffset: 2
    gdStaticSlot: 31
    gdSymbolWindow: 1
    gdSymbolWindowActionPointOffset: 1
    gdTSSTransmitter: 9
    gdWakeupTxActive: 60
    gdWakeupTxIdle: 180
  Name: FlexRay1
  NodeParameters:
    pAllowHaltDueToClock: 1
    pAllowPassiveToActive: 0
    pChannels: AB
    pClusterDriftDamping: 2
    pKeySlotId: 10
    pKeySlotOnlyEnabled: 0
    pKeySlotUsedForStartup: 1
    pKeySlotUsedForSync: 0
    pLatestTx: 249
    pMacroInitialOffsetA: 3
    pMacroInitialOffsetB: 3
    pMicroInitialOffsetA: 6
    pMicroInitialOffsetB: 6
    pMicroPerCycle: 200000
    pOffsetCorrectionOut: 127
    pOffsetCorrectionStart: 3632
    pRateCorrectionOut: 81
    pSamplesPerMicrotick: 2
    pWakeupChannel: A
    pWakeupPattern: 33
    pdAcceptedStartupRange: 212
    pdListenTimeout: 400162
    pdMicrotick: 25ns
  TxBufferConfigurations:
  - channels: A
    headerCrc: 0
    offset: 0
    PPindicator: false
    repetition: 0
    slotId: 0
    transmissionMode: Continuous
  Replay:
    Direction: Send
    MdfChannel:
      ChannelName: MyTestChannel1
      ChannelPath: path/to/myTestChannel1
      ChannelSource: MyTestChannel
      GroupName: MyTestGroup
      GroupPath: path/to/myTestGroup1
      GroupSource: MyTestGroup
    UseTraceSource: Source1
  UseTraceSinks:
  - Sink1
DataPublishers:
- Name: DataPubSubGroundTruth
  UseTraceSinks:
  - Sink1
Logging:
  Sinks:
  - Type: File
    Level: Critical
    LogName: MyLog1
  FlushLevel: Critical
  LogFromRemotes: false
ParticipantName: Node0
HealthCheck:
  SoftResponseTimeout: 500
  HardResponseTimeout: 5000
Tracing:
  TraceSinks:
  - Name: Sink1
    OutputPath: FlexrayDemo_node0.mf4
    Type: Mdf4File
  TraceSources:
  - Name: Source1
    InputPath: path/to/Source1.mf4
    Type: Mdf4File
Extensions:
  SearchPathHints:
  - path/to/extensions1
  - path/to/extensions2
Middleware:
  Registry:
    Hostname: NotLocalhost
    Logging:
      Sinks:
      - Type: Remote
    Port: 1337
    ConnectAttempts: 9
  TcpNoDelay: true
  TcpQuickAck: true
  EnableDomainSockets: false
  TcpSendBufferSize: 3456
  TcpReceiveBufferSize: 3456
)raw";

TEST_F(YamlParserTest, yaml_complete_configuration)
{
    auto node = YAML::Load(completeConfiguration);
    auto config = node.as<ParticipantConfiguration>();

    EXPECT_TRUE(config.participantName == "Node0");

    EXPECT_TRUE(config.canControllers.size() == 2);
    EXPECT_TRUE(config.canControllers.at(0).name == "CAN1");
    EXPECT_TRUE(config.canControllers.at(0).network.empty());
    EXPECT_TRUE(config.canControllers.at(1).name == "MyCAN2");
    EXPECT_TRUE(config.canControllers.at(1).network == "CAN2");

    EXPECT_TRUE(config.linControllers.size() == 1);
    EXPECT_TRUE(config.linControllers.at(0).name == "SimpleEcu1_LIN1");
    EXPECT_TRUE(config.linControllers.at(0).network == "LIN1");

    EXPECT_TRUE(config.flexRayControllers.size() == 1);
    EXPECT_TRUE(config.flexRayControllers.at(0).name == "FlexRay1");
    EXPECT_TRUE(config.flexRayControllers.at(0).network.empty());

    EXPECT_TRUE(config.dataPublishers.size() == 1);
    EXPECT_TRUE(config.dataPublishers.at(0).name == "DataPubSubGroundTruth");
    
    EXPECT_TRUE(config.logging.sinks.size() == 1);
    EXPECT_TRUE(config.logging.sinks.at(0).type == Sink::Type::File);
    EXPECT_TRUE(config.logging.sinks.at(0).level == ib::mw::logging::Level::Critical);
    EXPECT_TRUE(config.logging.sinks.at(0).logName == "MyLog1");

    EXPECT_TRUE(config.healthCheck.softResponseTimeout.value() == 500ms);
    EXPECT_TRUE(config.healthCheck.hardResponseTimeout.value() == 5000ms);

    EXPECT_TRUE(config.tracing.traceSinks.size() == 1);
    EXPECT_TRUE(config.tracing.traceSinks.at(0).name == "Sink1");
    EXPECT_TRUE(config.tracing.traceSinks.at(0).outputPath == "FlexrayDemo_node0.mf4");
    EXPECT_TRUE(config.tracing.traceSinks.at(0).type == TraceSink::Type::Mdf4File);
    EXPECT_TRUE(config.tracing.traceSources.size() == 1);
    EXPECT_TRUE(config.tracing.traceSources.at(0).name == "Source1");
    EXPECT_TRUE(config.tracing.traceSources.at(0).inputPath == "path/to/Source1.mf4");
    EXPECT_TRUE(config.tracing.traceSources.at(0).type == TraceSource::Type::Mdf4File);

    EXPECT_TRUE(config.extensions.searchPathHints.size() == 2);
    EXPECT_TRUE(config.extensions.searchPathHints.at(0) == "path/to/extensions1");
    EXPECT_TRUE(config.extensions.searchPathHints.at(1) == "path/to/extensions2");
    
    EXPECT_TRUE(config.middleware.registry.connectAttempts == 9);
    EXPECT_TRUE(config.middleware.registry.hostname == "NotLocalhost");
    EXPECT_TRUE(config.middleware.registry.port == 1337);
    EXPECT_TRUE(config.middleware.enableDomainSockets == false);
    EXPECT_TRUE(config.middleware.tcpQuickAck == true);
    EXPECT_TRUE(config.middleware.tcpNoDelay == true);
    EXPECT_TRUE(config.middleware.tcpReceiveBufferSize == 3456);
    EXPECT_TRUE(config.middleware.tcpSendBufferSize == 3456);
}

const auto emptyConfiguration = R"raw(
)raw";

TEST_F(YamlParserTest, yaml_empty_configuration)
{
    auto node = YAML::Load(emptyConfiguration);
    EXPECT_THROW({
        try
        {
            node.as<ParticipantConfiguration>();
        }
        catch (const YAML::TypedBadConversion<ParticipantConfiguration>& e)
        {
            EXPECT_STREQ("bad conversion", e.what());
            throw;
        }
    }, YAML::TypedBadConversion<ParticipantConfiguration>);
}

const auto minimalConfiguration = R"raw(
ParticipantName: Node1
)raw";

TEST_F(YamlParserTest, yaml_minimal_configuration)
{
    auto node = YAML::Load(minimalConfiguration);
    auto config = node.as<ParticipantConfiguration>();
    EXPECT_TRUE(config.participantName == "Node1");
}

TEST_F(YamlParserTest, yaml_native_type_conversions)
{
    {
        uint16_t a{ 0x815 };
        auto node = to_yaml(a);
        uint16_t b = from_yaml<uint16_t>(node);
        EXPECT_TRUE(a == b);
    }
    {
        std::vector<uint32_t> vec{ 0,1,3,4,5 };
        auto node = to_yaml(vec);
        auto vec2 = from_yaml<std::vector<uint32_t>>(node);
        EXPECT_TRUE(vec == vec2);
    }
    {
        MdfChannel mdf;
        mdf.channelName = "channelName";
        mdf.channelPath = "channelPath";
        mdf.channelSource = "channelSource";
        mdf.groupName = "groupName";
        mdf.groupPath = "groupPath";
        mdf.groupSource = "groupSource";
        auto yaml = to_yaml(mdf);
        auto mdf2 = from_yaml<decltype(mdf)>(yaml);
        EXPECT_TRUE(mdf == mdf2);
    }
    {
        Logging logger;
        Sink sink;
        logger.logFromRemotes = true;
        sink.type = Sink::Type::File;
        sink.level = ib::mw::logging::Level::Trace;
        sink.logName = "filename";
        logger.sinks.push_back(sink);
        sink.type=Sink::Type::Stdout;
        sink.logName = "";
        logger.sinks.push_back(sink);
        YAML::Node node;
        node = logger;
        //auto repr = node.as<std::string>();
        auto logger2 = node.as<Logging>();
        EXPECT_TRUE(logger == logger2);
    }
    {
        ParticipantConfiguration config{};
        YAML::Node node;
        node = config;
        auto config2 = node.as<ParticipantConfiguration>();
        EXPECT_TRUE(config == config2);
    }
}

TEST_F(YamlParserTest, middleware_convert)
{
    auto node = YAML::Load(R"(
        {
            "Registry": {
                "Hostname": "not localhost",
                "Port": 1234,
                "Logging": {
                    "Sinks": [
                        {
                            "Type": "Remote"
                        }
                    ]
                },
                "ConnectAttempts": 9
            },
            "TcpNoDelay": true,
            "TcpQuickAck": true,
            "TcpSendBufferSize": 3456,
            "TcpReceiveBufferSize": 3456,
            "EnableDomainSockets": false
        }
    )");
    auto config = node.as<Middleware>();
    EXPECT_EQ(config.registry.connectAttempts, 9);
    EXPECT_EQ(config.registry.logging.sinks.at(0).type, Sink::Type::Remote);
    EXPECT_EQ(config.registry.hostname, "not localhost");
    EXPECT_EQ(config.registry.port, 1234);

    EXPECT_EQ(config.enableDomainSockets, false);
    EXPECT_EQ(config.tcpNoDelay, true);
    EXPECT_EQ(config.tcpQuickAck, true);
    EXPECT_EQ(config.tcpSendBufferSize, 3456);
    EXPECT_EQ(config.tcpReceiveBufferSize, 3456);
}

TEST_F(YamlParserTest, map_serdes)
{
    std::map<std::string, std::string> mapin{
        {"keya", "vala"}, {"keyb", "valb"}, {"keyc", ""}, {"", "vald"}, {"keye\nwithlinebreak", "vale\nwithlinebreak"}};
    auto mapstr = ib::cfg::Serialize<std::map<std::string, std::string>>(mapin);
    auto mapout = ib::cfg::Deserialize<std::map<std::string, std::string>>(mapstr);
    EXPECT_EQ(mapin, mapout);
}

} // anonymous namespace
