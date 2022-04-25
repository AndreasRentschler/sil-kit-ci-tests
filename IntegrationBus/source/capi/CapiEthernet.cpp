/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

#include "ib/capi/IntegrationBus.h"
#include "ib/IntegrationBus.hpp"
#include "ib/mw/logging/ILogger.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/mw/sync/string_utils.hpp"
#include "ib/sim/eth/all.hpp"
#include "ib/sim/eth/string_utils.hpp"

#include <string>
#include <iostream>
#include <algorithm>
#include <map>
#include <mutex>
#include <cstring>
#include "CapiImpl.h"

std::map<uint32_t, void*> ethernetTransmitContextMap;
std::map<uint32_t, int> ethernetTransmitContextMapCounter;
int transmitAckListeners = 0;

struct PendingEthernetTransmits {
    std::map<uint32_t, void*> userContextById;
    std::map<uint32_t, std::function<void()>> callbacksById;
    std::mutex mutex;
};
PendingEthernetTransmits pendingEthernetTransmits;

#define ETHERNET_MIN_FRAME_SIZE 60

IntegrationBusAPI ib_ReturnCode ib_Ethernet_Controller_Create(ib_Ethernet_Controller** outController, ib_Participant* participant, const char* name, const char* network)
{
  ASSERT_VALID_OUT_PARAMETER(outController);
  ASSERT_VALID_POINTER_PARAMETER(participant);
  ASSERT_VALID_POINTER_PARAMETER(name);
  ASSERT_VALID_POINTER_PARAMETER(network);
  CAPI_ENTER
  {
    std::string strName(name);
    std::string strNetwork(network);
    auto cppParticipant = reinterpret_cast<ib::mw::IParticipant*>(participant);
    auto ethernetController = cppParticipant->CreateEthernetController(strName, strNetwork);
    *outController = reinterpret_cast<ib_Ethernet_Controller*>(ethernetController);
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Ethernet_Controller_Activate(ib_Ethernet_Controller* controller)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  CAPI_ENTER
  {
    auto cppController = reinterpret_cast<ib::sim::eth::IEthernetController*>(controller);
    cppController->Activate();
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Ethernet_Controller_Deactivate(ib_Ethernet_Controller* controller)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  CAPI_ENTER
  {
    auto cppController = reinterpret_cast<ib::sim::eth::IEthernetController*>(controller);
    cppController->Deactivate();
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Ethernet_Controller_AddFrameHandler(ib_Ethernet_Controller* controller, void* context, ib_Ethernet_FrameHandler_t handler)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  ASSERT_VALID_HANDLER_PARAMETER(handler);
  CAPI_ENTER
  {
    auto cppController = reinterpret_cast<ib::sim::eth::IEthernetController*>(controller);
    cppController->AddFrameHandler(
      [handler, context, controller](ib::sim::eth::IEthernetController* /*ctrl*/, const ib::sim::eth::EthernetFrameEvent& cppFrameEvent)
      {
        auto rawFrame = cppFrameEvent.ethFrame.RawFrame();
                
        uint8_t* dataPointer = 0;
        if (rawFrame.size() > 0)
        {
          dataPointer = &(rawFrame[0]);
        }

        ib_Ethernet_FrameEvent frameEvent{};
        ib_Ethernet_Frame frame{ dataPointer, rawFrame.size() };

        frameEvent.interfaceId = ib_InterfaceIdentifier_EthernetFrameEvent;
        frameEvent.ethernetFrame = &frame;
        frameEvent.timestamp = cppFrameEvent.timestamp.count();

        handler(context, controller, &frameEvent);
      });
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Ethernet_Controller_AddFrameTransmitHandler(ib_Ethernet_Controller* controller, void* context, ib_Ethernet_FrameTransmitHandler_t handler)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  ASSERT_VALID_HANDLER_PARAMETER(handler);
  CAPI_ENTER
  {
    auto cppController = reinterpret_cast<ib::sim::eth::IEthernetController*>(controller);
    cppController->AddFrameTransmitHandler(
      [handler, context, controller](ib::sim::eth::IEthernetController* , const ib::sim::eth::EthernetFrameTransmitEvent& ack)
      {
        std::unique_lock<std::mutex> lock(pendingEthernetTransmits.mutex);

        auto transmitContext = pendingEthernetTransmits.userContextById[ack.transmitId];
        if (transmitContext == nullptr)
        {
          pendingEthernetTransmits.callbacksById[ack.transmitId] =
            [handler, context, controller, ack]()
            {
              ib_Ethernet_FrameTransmitEvent eta;
              eta.interfaceId = ib_InterfaceIdentifier_EthernetFrameTransmitEvent;
              eta.status = (ib_Ethernet_TransmitStatus)ack.status;
              eta.timestamp = ack.timestamp.count();

              auto tmpContext = pendingEthernetTransmits.userContextById[ack.transmitId];
              pendingEthernetTransmits.userContextById.erase(ack.transmitId);
              eta.userContext = tmpContext;

              handler(context, controller, &eta);
            };
        }
        else
        {
          ib_Ethernet_FrameTransmitEvent eta;
          eta.interfaceId = ib_InterfaceIdentifier_EthernetFrameTransmitEvent;
          eta.status = (ib_Ethernet_TransmitStatus)ack.status;
          eta.timestamp = ack.timestamp.count();

          auto tmpContext = pendingEthernetTransmits.userContextById[ack.transmitId];
          pendingEthernetTransmits.userContextById.erase(ack.transmitId);
          eta.userContext = tmpContext;

          handler(context, controller, &eta);
        }
      });
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Ethernet_Controller_AddStateChangeHandler(ib_Ethernet_Controller* controller, void* context, ib_Ethernet_StateChangeHandler_t handler)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  ASSERT_VALID_HANDLER_PARAMETER(handler);
  CAPI_ENTER
  {
    auto cppController = reinterpret_cast<ib::sim::eth::IEthernetController*>(controller);
    cppController->AddStateChangeHandler(
      [handler, context, controller](ib::sim::eth::IEthernetController* , const ib::sim::eth::EthernetStateChangeEvent& stateChangeEvent)
      {
        ib_Ethernet_StateChangeEvent cStateChangeEvent;
        cStateChangeEvent.interfaceId = ib_InterfaceIdentifier_EthernetStateChangeEvent;
        cStateChangeEvent.timestamp = stateChangeEvent.timestamp.count();
        cStateChangeEvent.state = (ib_Ethernet_State)stateChangeEvent.state;
        handler(context, controller, cStateChangeEvent);
      });
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Ethernet_Controller_AddBitrateChangeHandler(ib_Ethernet_Controller* controller, void* context, ib_Ethernet_BitrateChangeHandler_t handler)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  ASSERT_VALID_HANDLER_PARAMETER(handler);
  CAPI_ENTER
  {
    auto cppController = reinterpret_cast<ib::sim::eth::IEthernetController*>(controller);
    cppController->AddBitrateChangeHandler(
      [handler, context, controller](ib::sim::eth::IEthernetController* , const ib::sim::eth::EthernetBitrateChangeEvent& bitrateChangeEvent)
      {
            ib_Ethernet_BitrateChangeEvent cBitrateChangeEvent;
            cBitrateChangeEvent.interfaceId = ib_InterfaceIdentifier_EthernetBitrateChangeEvent;
            cBitrateChangeEvent.timestamp = bitrateChangeEvent.timestamp.count();
            cBitrateChangeEvent.bitrate = (ib_Ethernet_Bitrate)bitrateChangeEvent.bitrate;

          handler(context, controller, cBitrateChangeEvent);
      });
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Ethernet_Controller_SendFrame(ib_Ethernet_Controller* controller, ib_Ethernet_Frame* frame, void* userContext)
{
  ASSERT_VALID_POINTER_PARAMETER(controller);
  ASSERT_VALID_POINTER_PARAMETER(frame);
  CAPI_ENTER
  {
    if (frame->size < ETHERNET_MIN_FRAME_SIZE) {
      ib_error_string = "An ethernet frame must be at least 60 bytes in size.";
      return ib_ReturnCode_BADPARAMETER;
    }
    using std::chrono::duration;
    auto cppController = reinterpret_cast<ib::sim::eth::IEthernetController*>(controller);

    ib::sim::eth::EthernetFrame ef;
    std::vector<uint8_t> rawFrame(frame->data, frame->data + frame->size);
    ef.SetRawFrame(rawFrame);
    auto transmitId = cppController->SendFrame(ef);

    std::unique_lock<std::mutex> lock(pendingEthernetTransmits.mutex);
    pendingEthernetTransmits.userContextById[transmitId] = userContext;
    for (auto pendingTransmitId : pendingEthernetTransmits.callbacksById)
    {
      pendingTransmitId.second();
    }
    pendingEthernetTransmits.callbacksById.clear();
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

