// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <cstdlib>
#include <thread>
#include <future>

#include "ib/sim/all.hpp"
#include "ib/util/functional.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "NullConnectionComAdapter.hpp"
#include "MockParticipantConfiguration.hpp"

namespace {

using namespace std::chrono_literals;

using testing::_;
using testing::A;
using testing::An;
using testing::InSequence;
using testing::NiceMock;
using testing::Return;

using namespace ib;

class IbConfigExampleITest : public testing::Test
{
protected:
    IbConfigExampleITest() { }

    void VerifyParticipants(const std::vector<cfg::Participant>& participants)
    {
        EXPECT_NE(participants.size(), 0u);

        for (auto&& participant: participants)
        {
            VerifyParticipant(participant.name);
        }
    }

    void VerifyParticipant(const std::string& participantName)
    {
        std::cout << "Verifying participant " << participantName << '\n';
        auto&& participantCfg = cfg::get_by_name(ibConfig.simulationSetup.participants, participantName);

        auto isSynchronized = participantCfg.participantController.value().syncType != ib::cfg::deprecated::SyncType::Unsynchronized;
        auto comAdapter = ib::mw::CreateNullConnectionComAdapterImpl(ib::cfg::MockParticipantConfiguration(), participantName, isSynchronized);

        CreateCanControllers(*comAdapter, participantCfg);
        CreateLinControllers(*comAdapter, participantCfg);
        CreateEthernetControllers(*comAdapter, participantCfg);
        CreateFlexrayControllers(*comAdapter, participantCfg);
        GetParticipantController(*comAdapter, participantCfg);
        GetSystemMonitor(*comAdapter, participantCfg);
        GetSystemController(*comAdapter, participantCfg);
    }
    void CreateCanControllers(mw::IComAdapter& comAdapter, const cfg::Participant& participantCfg)
    {
        for (auto&& controller : participantCfg.canControllers)
        {
            EXPECT_NE(comAdapter.CreateCanController(controller.name, "CAN1"), nullptr);
        }
    }
    void CreateLinControllers(mw::IComAdapter& comAdapter, const cfg::Participant& participantCfg)
    {
        for (auto&& controller : participantCfg.linControllers)
        {
            EXPECT_NE(comAdapter.CreateLinController(controller.name), nullptr);
        }
    }
    void CreateEthernetControllers(mw::IComAdapter& comAdapter, const cfg::Participant& participantCfg)
    {
        for (auto&& controller : participantCfg.ethernetControllers)
        {
            EXPECT_NE(comAdapter.CreateEthController(controller.name), nullptr);
        }
    }
    void CreateFlexrayControllers(mw::IComAdapter& comAdapter, const cfg::Participant& participantCfg)
    {
        for (auto&& controller : participantCfg.flexrayControllers)
        {
            EXPECT_NE(comAdapter.CreateFlexrayController(controller.name), nullptr);
        }
    }
    void GetParticipantController(mw::IComAdapter& comAdapter, const cfg::Participant& /*participantCfg*/)
    {
        EXPECT_NE(comAdapter.GetParticipantController(), nullptr);
        // must be callable repeatedly.
        EXPECT_NE(comAdapter.GetParticipantController(), nullptr);
    }
    void GetSystemMonitor(mw::IComAdapter& comAdapter, const cfg::Participant& /*participantCfg*/)
    {
        EXPECT_NE(comAdapter.GetSystemMonitor(), nullptr);
        // must be callable repeatedly.
        EXPECT_NE(comAdapter.GetSystemMonitor(), nullptr);
    }
    void GetSystemController(mw::IComAdapter& comAdapter, const cfg::Participant& /*participantCfg*/)
    {
        EXPECT_NE(comAdapter.GetSystemController(), nullptr);
        // must be callable repeatedly.
        EXPECT_NE(comAdapter.GetSystemController(), nullptr);
    }


protected:
    ib::cfg::Config ibConfig;
};

TEST_F(IbConfigExampleITest, DISABLED_build_participants_from_IbConfig_Example)
{
    ibConfig = cfg::Config::FromJsonFile("IbConfig_Example.json");
    VerifyParticipants(ibConfig.simulationSetup.participants);
}

TEST_F(IbConfigExampleITest, DISABLED_throw_if_file_logger_without_filename)
{
    EXPECT_THROW(cfg::Config::FromJsonFile("IbConfig_Bad_FileLogger.json"), ib::cfg::deprecated::Misconfiguration);
}

TEST_F(IbConfigExampleITest, DISABLED_build_participants_from_IbConfig_CANoe)
{
    ibConfig = cfg::Config::FromJsonFile("IbConfig_Canoe.json");
    VerifyParticipants(ibConfig.simulationSetup.participants);
}

} // anonymous namespace
