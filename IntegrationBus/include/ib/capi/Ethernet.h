// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once
#include <stdint.h>
#include "ib/capi/IbMacros.h"
#include "ib/capi/Types.h"
#include "ib/capi/InterfaceIdentifiers.h"

#pragma pack(push)
#pragma pack(8)

IB_BEGIN_DECLS

typedef int32_t ib_Ethernet_TransmitStatus;

/*! The message was successfully transmitted on the CAN bus. */
#define ib_Ethernet_TransmitStatus_Transmitted ((ib_Ethernet_TransmitStatus) 0)

/*! The transmit request was rejected, because the Ethernet controller is not active. */
#define ib_Ethernet_TransmitStatus_ControllerInactive ((ib_Ethernet_TransmitStatus) 1)

/*! The transmit request was rejected, because the Ethernet link is down. */
#define ib_Ethernet_TransmitStatus_LinkDown ((ib_Ethernet_TransmitStatus) 2)

/*! The transmit request was dropped, because the transmit queue is full. */
#define ib_Ethernet_TransmitStatus_Dropped ((ib_Ethernet_TransmitStatus) 3)

/*! The given raw Ethernet frame is ill formated (e.g. frame length is too small or too large, etc.). */
#define ib_Ethernet_TransmitStatus_InvalidFrameFormat ((ib_Ethernet_TransmitStatus) 4)

typedef int32_t ib_Ethernet_State;

//!< The Ethernet controller is switched off (default after reset).
#define ib_Ethernet_State_Inactive ((ib_Ethernet_State) 0)

//!< The Ethernet controller is active, but a link to another Ethernet controller in not yet established.
#define ib_Ethernet_State_LinkDown ((ib_Ethernet_State) 1)

//!< The Ethernet controller is active and the link to another Ethernet controller is established.
#define ib_Ethernet_State_LinkUp ((ib_Ethernet_State) 2)

typedef struct 
{
    ib_InterfaceIdentifier interfaceId; //!< The interface id that specifies which version of this struct was obtained
    ib_NanosecondsTime timestamp; //!< Timestamp of the state change event
    ib_Ethernet_State state; //!< New state of the Ethernet controller
} ib_Ethernet_StateChangeEvent;

typedef uint32_t ib_Ethernet_Bitrate; //!< Bitrate in kBit/sec

typedef struct
{
    ib_InterfaceIdentifier interfaceId; //!< The interface id that specifies which version of this struct was obtained
    ib_NanosecondsTime timestamp; //!< Timestamp of the bitrate change event
    ib_Ethernet_Bitrate bitrate; //!< New bitrate in kBit/sec
} ib_Ethernet_BitrateChangeEvent;

//!< A raw Ethernet frame
typedef struct
{
    ib_InterfaceIdentifier interfaceId; //!< The interface id that specifies which version of this struct was obtained
    ib_ByteVector raw;
} ib_Ethernet_Frame;


typedef struct 
{
    ib_InterfaceIdentifier interfaceId; //!< The interface id that specifies which version of this struct was obtained
    ib_NanosecondsTime timestamp; //!< Send time
    ib_Ethernet_Frame* ethernetFrame; //!< The raw Ethernet frame
} ib_Ethernet_FrameEvent;

struct ib_Ethernet_FrameTransmitEvent
{
    ib_InterfaceIdentifier interfaceId; //!< The interface id that specifies which version of this struct was obtained
    void* userContext; //!< Value that was provided by user in corresponding parameter on send of Ethernet frame
    ib_NanosecondsTime timestamp; //!< Reception time
    ib_Ethernet_TransmitStatus status; //!< Status of the EthernetTransmitRequest
};
typedef struct ib_Ethernet_FrameTransmitEvent ib_Ethernet_FrameTransmitEvent;

typedef struct ib_Ethernet_Controller ib_Ethernet_Controller;

/*! Callback type to indicate that a EthernetMessage has been received.
* \param context The context provided by the user upon registration.
* \param controller The Ethernet controller that received the message.
* \param frameEvent Contains the raw frame and the timestamp of the event.
*/
typedef void (*ib_Ethernet_FrameHandler_t)(void* context, ib_Ethernet_Controller* controller, 
  ib_Ethernet_FrameEvent* frameEvent);
    
/*! Callback type to indicate that a EthernetFrame has been sent.
* \param context The by the user provided context on registration.
* \param controller The Ethernet controller that received the acknowledge.
* \param frameTransmitEvent Contains the transmit status and the timestamp of the event.
*/
typedef void (*ib_Ethernet_FrameTransmitHandler_t)(void* context, ib_Ethernet_Controller* controller, 
  ib_Ethernet_FrameTransmitEvent* frameTransmitEvent);
    
/*! Callback type to indicate that the Ethernet controller state has changed.
* \param context The by the user provided context on registration.
* \param controller The Ethernet controller whose state did change.
* \param stateChangeEvent Contains the new state and the timestamp of the event.
*/
typedef void (*ib_Ethernet_StateChangeHandler_t)(void* context, ib_Ethernet_Controller* controller,
  ib_Ethernet_StateChangeEvent* stateChangeEvent);

/*! Callback type to indicate that the link bit rate has changed.
* \param context Context pointer provided by the user on registration.
* \param controller The Ethernet controller that is affected.
* \param bitrateChangeEvent Contains the new bitrate and the timestamp of the event.
*/
typedef void (*ib_Ethernet_BitrateChangeHandler_t)(void* context, ib_Ethernet_Controller* controller,
  ib_Ethernet_BitrateChangeEvent* bitrateChangeEvent);

/*! \brief Create an Ethernet controller at this IB simulation participant.
* 
* \param outController A pointer to a pointer in which the Ethernet controller will be stored (out parameter).
* \param participant The simulation participant for which the Ethernet controller should be created.
* \param name The utf8 encoded name of the new Ethernet controller.
* \result A return code identifying the success/failure of the call.
* ! \note The object returned must not be deallocated using free()!
* 
* \see ib::mw::IParticipant::CreateEthernetController
*/
IntegrationBusAPI ib_ReturnCode ib_Ethernet_Controller_Create(
  ib_Ethernet_Controller** outController,
  ib_Participant* participant, const char* name, const char* network);

typedef ib_ReturnCode(*ib_Ethernet_Controller_Create_t)(
  ib_Ethernet_Controller** outController,
  ib_Participant* participant, 
  const char* name,
  const char* network);

/*! \brief Activates the Ethernet controller
*
* Upon activation of the controller, the controller attempts to
* establish a link. Messages can only be sent once the link has
* been successfully established,
* 
* \param controller The Ethernet controller to be activated.
* \result A return code identifying the success/failure of the call.
* 
* \see ib::sim::eth::IEthernetController::Activate
*/
IntegrationBusAPI ib_ReturnCode ib_Ethernet_Controller_Activate(ib_Ethernet_Controller* controller);

typedef ib_ReturnCode(*ib_Ethernet_Controller_Activate_t)(ib_Ethernet_Controller* controller);

/*! \brief Deactivate the Ethernet controller
*
* Deactivate the controller and shut down the link. The
* controller will no longer receive messages, and it cannot send
* messages anymore.
* 
* \param controller The Ethernet controller to be deactivated.
* \result A return code identifying the success/failure of the call.
* 
* \see ib::sim::eth::IEthernetController::Deactivate
*/
IntegrationBusAPI ib_ReturnCode ib_Ethernet_Controller_Deactivate(ib_Ethernet_Controller* controller);

typedef ib_ReturnCode(*ib_Ethernet_Controller_Deactivate_t)(ib_Ethernet_Controller* controller);
/*! \brief Register a callback for Ethernet message reception
*
* The handler is called when the controller receives a new
* Ethernet message.
* 
* \param controller The Ethernet controller for which the message callback should be registered.
* \param context The user provided context pointer, that is reobtained in the callback.
* \param handler The handler to be called on reception.
* \param outHandlerId The handler identifier that can be used to remove the callback.
* \result A return code identifying the success/failure of the call.
* 
* \see ib::sim::eth::IEthernetController::AddFrameHandler
* 
*/
IntegrationBusAPI ib_ReturnCode ib_Ethernet_Controller_AddFrameHandler(ib_Ethernet_Controller* controller,
                                                                       void* context,
                                                                       ib_Ethernet_FrameHandler_t handler,
                                                                       ib_HandlerId* outHandlerId);

typedef ib_ReturnCode (*ib_Ethernet_Controller_AddFrameHandler_t)(ib_Ethernet_Controller* controller, void* context,
                                                                  ib_Ethernet_FrameHandler_t handler,
                                                                  ib_HandlerId* outHandlerId);

/*! \brief  Remove a \ref ib_Ethernet_FrameHandler_t by ib_HandlerId on this controller 
*
* \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
*/
IntegrationBusAPI ib_ReturnCode ib_Ethernet_Controller_RemoveFrameHandler(ib_Ethernet_Controller* controller,
                                                                          ib_HandlerId handlerId);

typedef ib_ReturnCode (*ib_Ethernet_Controller_RemoveFrameHandler_t)(ib_Ethernet_Controller* controller,
                                                                     ib_HandlerId handlerId);

/*! \brief Register a callback for Ethernet transmit acknowledgments
*
* The handler is called when a previously sent message was
* successfully transmitted or when the transmission has
* failed. The original message is identified by the userContext.
*
* NB: Full support in a detailed simulation. In a simple
* simulation, all messages are immediately positively
* acknowledged by a receiving controller.
* 
* \param controller The Ethernet controller for which the message acknowledge callback should be registered.
* \param context The user provided context pointer, that is reobtained in the callback.
* \param handler The handler to be called on reception.
* \param outHandlerId The handler identifier that can be used to remove the callback.
* \result A return code identifying the success/failure of the call.
* 
* \see ib::sim::eth::IEthernetController::AddFrameTransmitHandler
*/
IntegrationBusAPI ib_ReturnCode
ib_Ethernet_Controller_AddFrameTransmitHandler(ib_Ethernet_Controller* controller, void* context,
                                               ib_Ethernet_FrameTransmitHandler_t handler, ib_HandlerId* outHandlerId);

typedef ib_ReturnCode (*ib_Ethernet_Controller_AddFrameTransmitHandler_t)(ib_Ethernet_Controller* controller,
                                                                          void* context,
                                                                          ib_Ethernet_FrameTransmitHandler_t handler,
                                                                          ib_HandlerId* outHandlerId);

/*! \brief  Remove a \ref ib_Ethernet_FrameTransmitHandler_t by ib_HandlerId on this controller 
*
* \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
*/
IntegrationBusAPI ib_ReturnCode ib_Ethernet_Controller_RemoveFrameTransmitHandler(ib_Ethernet_Controller* controller,
                                                                                  ib_HandlerId handlerId);

typedef ib_ReturnCode (*ib_Ethernet_Controller_RemoveFrameTransmitHandler_t)(ib_Ethernet_Controller* controller,
                                                                             ib_HandlerId handlerId);

/*! \brief Register a callback for changes of the controller state
*
* The handler is called when the state of the controller
* changes. E.g., a call to ib_Ethernet_Controller_Activate() causes the controller to
* change from state ib_Ethernet_State_Inactive to ib_Ethernet_State_LinkDown. Later, when the link
* has been established, the state changes again from ib_Ethernet_State_LinkDown to
* ib_Ethernet_State_LinkUp. Similarly, the status changes back to ib_Ethernet_State_Inactive upon a
* call to ib_Ethernet_Controller_Deactivate().
*
* \param controller The Ethernet controller for which the message acknowledge callback should be registered.
* \param context The user provided context pointer, that is reobtained in the callback.
* \param handler The handler to be called on reception.
* \param outHandlerId The handler identifier that can be used to remove the callback.
* \result A return code identifying the success/failure of the call.
* 
* \see ib::sim::eth::IEthernetController::AddStateChangeHandler
*/
IntegrationBusAPI ib_ReturnCode ib_Ethernet_Controller_AddStateChangeHandler(ib_Ethernet_Controller* controller,
                                                                             void* context,
                                                                             ib_Ethernet_StateChangeHandler_t handler,
                                                                             ib_HandlerId* outHandlerId);

typedef ib_ReturnCode (*ib_Ethernet_Controller_AddStateChangeHandler_t)(ib_Ethernet_Controller* controller,
                                                                        void* context,
                                                                        ib_Ethernet_StateChangeHandler_t handler,
                                                                        ib_HandlerId* outHandlerId);

/*! \brief  Remove a \ref ib_Ethernet_StateChangeHandler_t by ib_HandlerId on this controller 
*
* \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
*/
IntegrationBusAPI ib_ReturnCode ib_Ethernet_Controller_RemoveStateChangeHandler(ib_Ethernet_Controller* controller,
                                                                                ib_HandlerId handlerId);

typedef ib_ReturnCode (*ib_Ethernet_Controller_RemoveStateChangeHandler_t)(ib_Ethernet_Controller* controller,
                                                                           ib_HandlerId handlerId);


/*! \brief Register a callback for changes of the link bit rate
*
* The handler is called when the bit rate of the connected link
* changes. This is typically the case when a link was
* successfully established, or the controller was deactivated.
*
* \param controller The Ethernet controller for which the bitrate change callback should be registered.
* \param context The user provided context pointer, that is reobtained in the callback.
* \param handler The handler to be called on change.
* \param outHandlerId The handler identifier that can be used to remove the callback.
* \result A return code identifying the success/failure of the call.
* 
* \see ib::sim::eth::IEthernetController::AddBitrateChangeHandler
*/
IntegrationBusAPI ib_ReturnCode
ib_Ethernet_Controller_AddBitrateChangeHandler(ib_Ethernet_Controller* controller, void* context,
                                               ib_Ethernet_BitrateChangeHandler_t handler, ib_HandlerId* outHandlerId);

typedef ib_ReturnCode (*ib_Ethernet_Controller_AddBitrateChangeHandler_t)(ib_Ethernet_Controller* controller,
                                                                          void* context,
                                                                          ib_Ethernet_BitrateChangeHandler_t handler,
                                                                          ib_HandlerId* outHandlerId);
/*! \brief  Remove a \ref ib_Ethernet_BitrateChangeHandler_t by ib_HandlerId on this controller 
*
* \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
*/
IntegrationBusAPI ib_ReturnCode ib_Ethernet_Controller_RemoveBitrateChangeHandler(ib_Ethernet_Controller* controller,
                                                                                  ib_HandlerId handlerId);

typedef ib_ReturnCode (*ib_Ethernet_Controller_RemoveBitrateChangeHandler_t)(ib_Ethernet_Controller* controller,
                                                                             ib_HandlerId handlerId);

/*! \brief Send an Ethernet frame
*
* Requires previous activation of the
* controller and a successfully established link. Also, the
* entire EthernetFrame must be valid, e.g., destination and source MAC
* addresses must be valid, ether type and vlan tags must be
* correct, payload size must be valid.
*
* These requirements are not enforced in
* simple simulation. In this case, the message is simply passed
* on to all connected controllers without performing any check. 
* The user must ensure that a valid frame is provided.
* 
* The minimum frame size of 60 bytes must be provided, or 
* ib_ReturnCode_BAD_PARAMETER will be returned.
*
* \param controller The Ethernet controller that should send the frame.
* \param frame The Ethernet frame to be sent.
* \param userContext The user provided context pointer, that is reobtained in the frame ack handler
* \result A return code identifying the success/failure of the call.
* 
* \see ib::sim::eth::IEthernetController::SendFrame
*/
IntegrationBusAPI ib_ReturnCode ib_Ethernet_Controller_SendFrame(
  ib_Ethernet_Controller* controller,
  ib_Ethernet_Frame* frame, 
  void* userContext);

typedef ib_ReturnCode(*ib_Ethernet_Controller_SendFrame_t)(
  ib_Ethernet_Controller* controller,
  ib_Ethernet_Frame* frame,
  void* userContext);

IB_END_DECLS

#pragma pack(pop)
