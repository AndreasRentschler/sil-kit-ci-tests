// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "EthControllerProxy.hpp"


namespace ib {
namespace sim {
namespace eth {

EthControllerProxy::EthControllerProxy(mw::IParticipantInternal* participant,
                                       cfg::EthernetController /*config*/, IEthernetController* facade)
    : _participant(participant)
    , _facade{facade}
{
    if (_facade == nullptr)
    {
        _facade = this;
    }
}

void EthControllerProxy::Activate()
{
    // Check if the Controller has already been activated
    if (_ethState != EthernetState::Inactive)
        return;

    EthernetSetMode msg { EthernetMode::Active };
    _participant->SendIbMessage(this, msg);
}

void EthControllerProxy::Deactivate()
{
    // Check if the Controller has already been deactivated
    if (_ethState == EthernetState::Inactive)
        return;

    EthernetSetMode msg{ EthernetMode::Inactive };
    _participant->SendIbMessage(this, msg);
}

auto EthControllerProxy::SendFrameEvent(EthernetFrameEvent msg) -> EthernetTxId
{
    auto txId = MakeTxId();
    msg.transmitId = txId;

    // we keep a copy until the transmission was acknowledged before tracing the message
    _transmittedMessages[msg.transmitId] = msg.ethFrame;

    _participant->SendIbMessage(this, std::move(msg));

    return txId;
}

auto EthControllerProxy::SendFrame(EthernetFrame frame) -> EthernetTxId
{
    EthernetFrameEvent msg{};
    msg.ethFrame = std::move(frame);
    return SendFrameEvent(std::move(msg));
}

void EthControllerProxy::AddFrameHandler(FrameHandler handler)
{
    RegisterHandler(std::move(handler));
}

void EthControllerProxy::AddFrameTransmitHandler(FrameTransmitHandler handler)
{
    RegisterHandler(std::move(handler));
}

void EthControllerProxy::AddStateChangeHandler(StateChangeHandler handler)
{
    RegisterHandler(std::move(handler));
}

void EthControllerProxy::AddBitrateChangeHandler(BitrateChangeHandler handler)
{
    RegisterHandler(std::move(handler));
}


void EthControllerProxy::ReceiveIbMessage(const IIbServiceEndpoint* /*from*/, const EthernetFrameEvent& msg)
{
    _tracer.Trace(ib::sim::TransmitDirection::RX,
        msg.timestamp, msg.ethFrame);

    CallHandlers(msg);
}

void EthControllerProxy::ReceiveIbMessage(const IIbServiceEndpoint* /*from*/, const EthernetFrameTransmitEvent& msg)
{
    auto transmittedMsg = _transmittedMessages.find(msg.transmitId);
    if (transmittedMsg != _transmittedMessages.end())
    {
        if (msg.status == EthernetTransmitStatus::Transmitted)
        {
            _tracer.Trace(ib::sim::TransmitDirection::TX, msg.timestamp,
                transmittedMsg->second);
        }

        _transmittedMessages.erase(msg.transmitId);
    }

    CallHandlers(msg);
}

void EthControllerProxy::ReceiveIbMessage(const IIbServiceEndpoint* /*from*/, const EthernetStatus& msg)
{
    // In case we are in early startup, ensure we tell our participants the bit rate first
    // and the state later.
    if (msg.bitrate != _ethBitRate)
    {
        _ethBitRate = msg.bitrate;
        CallHandlers(EthernetBitrateChangeEvent{ msg.timestamp, msg.bitrate });
    }

    if (msg.state != _ethState)
    {
        _ethState = msg.state;
        CallHandlers(EthernetStateChangeEvent{ msg.timestamp, msg.state });
    }

}

template<typename MsgT>
void EthControllerProxy::RegisterHandler(CallbackT<MsgT>&& handler)
{
    auto&& handlers = std::get<CallbackVector<MsgT>>(_callbacks);
    handlers.push_back(std::forward<CallbackT<MsgT>>(handler));
}


template<typename MsgT>
void EthControllerProxy::CallHandlers(const MsgT& msg)
{
    auto&& handlers = std::get<CallbackVector<MsgT>>(_callbacks);
    for (auto&& handler : handlers)
    {
        handler(this, msg);
    }
}


} // namespace eth
} // namespace sim
} // namespace ib
