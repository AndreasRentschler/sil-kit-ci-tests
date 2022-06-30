/* Copyright (c) Vector Informatik GmbH. All rights reserved. */
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

#include "ib/capi/IntegrationBus.h"

#ifdef WIN32
#pragma warning(disable : 4100 5105)
#include "windows.h"
#define SleepMs(X) Sleep(X)
#else
#include <unistd.h>
#define SleepMs(X) usleep((X)*1000)
#endif

#define UNUSED_ARG(X) (void)(X)

void AbortOnFailedAllocation(const char* failedAllocStrucName)
{
    fprintf(stderr, "Error: Allocation of \"%s\" failed, aborting...", failedAllocStrucName);
    abort();
}

char* LoadFile(char const* path)
{
    size_t length = 0;
    char* result = NULL;
    FILE* f = fopen(path, "rb");

    if (f)
    {
        fseek(f, 0, SEEK_END);
        length = ftell(f);
        fseek(f, 0, SEEK_SET);
        char* buffer = (char*)malloc((length + 1) * sizeof(char));
        if (buffer)
        {
            size_t num = fread(buffer, sizeof(uint8_t), length, f);
            if(num != length)
            {
                printf("Warning: short read on config file: %zu/%zu",
                    num, length);
                exit(1);
            }
            buffer[length] = '\0';
            result = buffer;
        }
        fclose(f);
    }
    return result;
}

const uint64_t ib_NanosecondsTime_Max = ((uint64_t)-1ll);

typedef void (*Timer_Action_t)(ib_NanosecondsTime now);
struct Timer
{
    uint8_t            isActive;
    ib_NanosecondsTime timeOut;
    Timer_Action_t     action;
};
typedef struct Timer Timer;

void Timer_Set(Timer* timer, ib_NanosecondsTime timeOut, Timer_Action_t action)
{
    timer->isActive = 1;
    timer->timeOut = timeOut;
    timer->action = action;
}
void Timer_Clear(Timer* timer)
{
    timer->isActive = 0;
    timer->timeOut = ib_NanosecondsTime_Max;
    timer->action = NULL;
}
void Timer_ExecuteAction(Timer* timer, ib_NanosecondsTime now)
{
    if (!timer->isActive || (now < timer->timeOut))
        return;

    Timer_Action_t action = timer->action;
    Timer_Clear(timer);
    action(now);
}

typedef void (*Task_Action_t)(ib_NanosecondsTime now);
struct Task
{
    ib_NanosecondsTime delay;
    Task_Action_t      action;
};
typedef struct Task Task;

struct Schedule
{
    Timer              timer;
    ib_NanosecondsTime now;
    uint32_t           numTasks;
    uint32_t           nextTaskIndex;
    Task*              schedule;
};
typedef struct Schedule Schedule;

void Schedule_ScheduleNextTask(Schedule* schedule)
{
    uint32_t currentTaskIndex = schedule->nextTaskIndex++;
    if (schedule->nextTaskIndex == schedule->numTasks)
    {
        schedule->nextTaskIndex = 0;
    }
    Timer_Set(&schedule->timer, schedule->now + schedule->schedule[currentTaskIndex].delay,
              schedule->schedule[currentTaskIndex].action);
}

void Schedule_Reset(Schedule* schedule)
{
    schedule->nextTaskIndex = 0;
    Schedule_ScheduleNextTask(schedule);
}

void Schedule_ExecuteTask(Schedule* schedule, ib_NanosecondsTime now)
{
    schedule->now = now;
    Timer_ExecuteAction(&schedule->timer, now);
}

void Schedule_Create(Schedule** outSchedule, Task* tasks, uint32_t numTasks)
{
    Schedule* newSchedule;
    size_t scheduleSize = sizeof(Schedule) + (numTasks * sizeof(Task));
    newSchedule = (Schedule*)malloc(scheduleSize);
    if (newSchedule == NULL)
    {
        AbortOnFailedAllocation("Schedule");
        return;
    }
    newSchedule->numTasks = numTasks;
    newSchedule->nextTaskIndex = 0;
    newSchedule->now = 0;
    newSchedule->timer.action = NULL;
    newSchedule->timer.isActive = 0;
    newSchedule->timer.timeOut = 0;
    memcpy(&newSchedule->schedule, &tasks, numTasks * sizeof(Task));
    Schedule_Reset(newSchedule);
    *outSchedule = newSchedule;
}

void Schedule_Destroy(Schedule* schedule)
{
    if (schedule)
    {
        free(schedule);
    }
}

ib_Participant* participant;
ib_Lin_Controller*         linController;
ib_Lin_ControllerConfig*   controllerConfig;
char*                     participantName;
Schedule*                 masterSchedule;
Timer                     slaveTimer;
ib_NanosecondsTime        slaveNow;

void StopCallback(void* context, ib_Participant* cbParticipant)
{
    UNUSED_ARG(context);
    UNUSED_ARG(cbParticipant);
    printf("Stopping...\n");
}
void ShutdownCallback(void* context, ib_Participant* cbParticipant)
{
    UNUSED_ARG(context);
    UNUSED_ARG(cbParticipant);
    printf("Shutting down...\n");
}

void Master_InitCallback(void* context, ib_Participant* cbParticipant)
{
    UNUSED_ARG(context);
    UNUSED_ARG(cbParticipant);
    printf("Initializing LinMaster\n");
    controllerConfig = (ib_Lin_ControllerConfig*)malloc(sizeof(ib_Lin_ControllerConfig));
    if (controllerConfig == NULL)
    {
        AbortOnFailedAllocation("ib_Lin_ControllerConfig");
        return;
    }
    memset(controllerConfig, 0, sizeof(ib_Lin_ControllerConfig));
    controllerConfig->interfaceId = ib_InterfaceIdentifier_LinControllerConfig;
    controllerConfig->controllerMode = ib_Lin_ControllerMode_Master;
    controllerConfig->baudRate = 20000;
    controllerConfig->numFrameResponses = 0;

    ib_Lin_Controller_Init(linController, controllerConfig);
}

void Master_doAction(ib_NanosecondsTime now)
{

    ib_Lin_ControllerStatus status;
    ib_Lin_Controller_Status(linController, &status);
    if (status != ib_Lin_ControllerStatus_Operational)
        return;
    Schedule_ExecuteTask(masterSchedule, now);
}

void Master_SimTask(void* context, ib_Participant* cbParticipant, ib_NanosecondsTime now)
{
    UNUSED_ARG(context);
    UNUSED_ARG(cbParticipant);
    printf("now%"PRIu64"ms\n", now / 1000000);
    Master_doAction(now);
}

void Master_ReceiveFrameStatus(void* context, ib_Lin_Controller* controller, const ib_Lin_FrameStatusEvent* frameStatusEvent)
{
    UNUSED_ARG(context);
    UNUSED_ARG(controller);

    switch (frameStatusEvent->status)
    {
    case ib_Lin_FrameStatus_LIN_RX_OK:
        break; // good case, no need to warn
    case ib_Lin_FrameStatus_LIN_TX_OK:
        break; // good case, no need to warn
    default:
        printf("WARNING: LIN transmission failed!\n");
    }

    ib_Lin_Frame* frame = frameStatusEvent->frame;
    printf(">> lin::Frame{id=%d, cs=%d, dl=%d, d={%d %d %d %d %d %d %d %d}} status=%d\n", frame->id,
           frame->checksumModel,
           frame->dataLength, frame->data[0], frame->data[1], frame->data[2], frame->data[3], frame->data[4],
           frame->data[5], frame->data[6], frame->data[7], frameStatusEvent->status);

    Schedule_ScheduleNextTask(masterSchedule);
}

void Master_WakeupHandler(void* context, ib_Lin_Controller* controller, const ib_Lin_WakeupEvent* wakeUpEvent)
{
    UNUSED_ARG(context);

    ib_Lin_ControllerStatus status;
    ib_Lin_Controller_Status(controller, &status);

    if ( status != ib_Lin_ControllerStatus_Sleep)
    {
        printf("WARNING: Received Wakeup pulse while ib_Lin_ControllerStatus is %d.\n", status);

    }
    printf(">> Wakeup pulse received @%" PRIu64 "ms; direction=%d\n", wakeUpEvent->timestamp / 1000000,
           wakeUpEvent->direction);
    ib_Lin_Controller_WakeupInternal(controller);
    Schedule_ScheduleNextTask(masterSchedule);
}

void Master_SendFrame_16(ib_NanosecondsTime now)
{
    UNUSED_ARG(now);
    ib_Lin_Frame frame;
    frame.interfaceId = ib_InterfaceIdentifier_LinFrame;
    frame.id = 16;
    frame.checksumModel = ib_Lin_ChecksumModel_Classic;
    frame.dataLength = 6;
    uint8_t tmp[8] = {1, 6, 1, 6, 1, 6, 1, 6};
    memcpy(frame.data, tmp, sizeof(tmp));

    ib_Lin_Controller_SendFrame(linController, &frame, ib_Lin_FrameResponseType_MasterResponse);
    printf("<< LIN Frame sent with ID=%d\n", frame.id);
}

void Master_SendFrame_17(ib_NanosecondsTime now)
{
    UNUSED_ARG(now);
    ib_Lin_Frame frame;
    frame.interfaceId = ib_InterfaceIdentifier_LinFrame;
    frame.id = 17;
    frame.checksumModel = ib_Lin_ChecksumModel_Classic;
    frame.dataLength = 6;
    uint8_t tmp[8] = {1, 7, 1, 7, 1, 7, 1, 7};
    memcpy(frame.data, tmp, sizeof(tmp));

    ib_Lin_Controller_SendFrame(linController, &frame, ib_Lin_FrameResponseType_MasterResponse);
    printf("<< LIN Frame sent with ID=%d\n", frame.id);
}

void Master_SendFrame_18(ib_NanosecondsTime now)
{
    UNUSED_ARG(now);
    ib_Lin_Frame frame;
    frame.interfaceId = ib_InterfaceIdentifier_LinFrame;
    frame.id = 18;
    frame.checksumModel = ib_Lin_ChecksumModel_Enhanced;
    frame.dataLength = 8;
    uint8_t tmp[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    memcpy(frame.data, tmp, sizeof(tmp));

    ib_Lin_Controller_SendFrame(linController, &frame, ib_Lin_FrameResponseType_MasterResponse);
    printf("<< LIN Frame sent with ID=%d\n", frame.id);
}

void Master_SendFrame_19(ib_NanosecondsTime now)
{
    UNUSED_ARG(now);
    ib_Lin_Frame frame;
    frame.interfaceId = ib_InterfaceIdentifier_LinFrame;
    frame.id = 19;
    frame.checksumModel = ib_Lin_ChecksumModel_Classic;
    frame.dataLength = 8;
    uint8_t tmp[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    memcpy(frame.data, tmp, sizeof(tmp));

    ib_Lin_Controller_SendFrame(linController, &frame, ib_Lin_FrameResponseType_MasterResponse);
    printf("<< LIN Frame sent with ID=%d\n", frame.id);
}

void Master_SendFrame_34(ib_NanosecondsTime now)
{
    UNUSED_ARG(now);
    ib_Lin_Frame frame;
    frame.interfaceId = ib_InterfaceIdentifier_LinFrame;
    frame.id = 34;
    frame.checksumModel = ib_Lin_ChecksumModel_Enhanced;
    frame.dataLength = 6;

    ib_Lin_Controller_SendFrame(linController, &frame, ib_Lin_FrameResponseType_SlaveResponse);
    printf("<< LIN Frame Header sent for ID=%d\n", frame.id);
}

void Master_GoToSleep(ib_NanosecondsTime now)
{
    UNUSED_ARG(now);
    printf("<< Sending Go-To-Sleep Command and entering sleep state\n");
    ib_Lin_Controller_GoToSleep(linController);
}

void Slave_InitCallback(void* context, ib_Participant* cbParticipant)
{
    UNUSED_ARG(context);
    UNUSED_ARG(cbParticipant);

    printf("Initializing LinSlave\n");

    // Configure LIN Controller to receive a LinFrameResponse for LIN ID 16
    ib_Lin_FrameResponse response_16;
    response_16.interfaceId = ib_InterfaceIdentifier_LinFrameResponse;
    ib_Lin_Frame frame16;
    frame16.interfaceId = ib_InterfaceIdentifier_LinFrame;
    frame16.id = 16;
    frame16.checksumModel = ib_Lin_ChecksumModel_Classic;
    frame16.dataLength = 6;
    response_16.frame = &frame16;
    response_16.responseMode = ib_Lin_FrameResponseMode_Rx;

    // Configure LIN Controller to receive a LinFrameResponse for LIN ID 17
    //  - This ib_Lin_FrameResponseMode_Unused causes the controller to ignore
    //    this message and not trigger a callback. This is also the default.
    ib_Lin_FrameResponse response_17;
    response_17.interfaceId = ib_InterfaceIdentifier_LinFrameResponse;
    ib_Lin_Frame frame17;
    frame17.interfaceId = ib_InterfaceIdentifier_LinFrame;
    frame17.id = 17;
    frame17.checksumModel = ib_Lin_ChecksumModel_Classic;
    frame17.dataLength = 6;
    response_17.frame = &frame17;
    response_17.responseMode = ib_Lin_FrameResponseMode_Unused;

    // Configure LIN Controller to receive LIN ID 18
    //  - LinChecksumModel does not match with master --> Receive with LIN_RX_ERROR
    ib_Lin_FrameResponse response_18;
    response_18.interfaceId = ib_InterfaceIdentifier_LinFrameResponse;
    ib_Lin_Frame frame18;
    frame18.interfaceId = ib_InterfaceIdentifier_LinFrame;
    frame18.id = 18;
    frame18.checksumModel = ib_Lin_ChecksumModel_Classic;
    frame18.dataLength = 8;
    response_18.frame = &frame18;
    response_18.responseMode = ib_Lin_FrameResponseMode_Rx;

    // Configure LIN Controller to receive LIN ID 19
    //  - dataLength does not match with master --> Receive with LIN_RX_ERROR
    ib_Lin_FrameResponse response_19;
    response_19.interfaceId = ib_InterfaceIdentifier_LinFrameResponse;
    ib_Lin_Frame frame19;
    frame19.interfaceId = ib_InterfaceIdentifier_LinFrame;
    frame19.id = 19;
    frame19.checksumModel = ib_Lin_ChecksumModel_Enhanced;
    frame19.dataLength = 1;
    response_19.frame = &frame19;
    response_19.responseMode = ib_Lin_FrameResponseMode_Rx;

    // Configure LIN Controller to send a LinFrameResponse for LIN ID 34
    ib_Lin_FrameResponse response_34;
    response_34.interfaceId = ib_InterfaceIdentifier_LinFrameResponse;

    ib_Lin_Frame frame34;
    frame34.interfaceId = ib_InterfaceIdentifier_LinFrame;
    frame34.id = 34;
    frame34.checksumModel = ib_Lin_ChecksumModel_Enhanced;
    frame34.dataLength = 6;
    uint8_t tmp[8] = {3, 4, 3, 4, 3, 4, 3, 4};
    memcpy(frame34.data, tmp, sizeof(tmp));
    response_34.frame = &frame34;
    response_34.responseMode = ib_Lin_FrameResponseMode_TxUnconditional;

    const uint32_t numFrameResponses = 5;
    size_t   controllerConfigSize = sizeof(ib_Lin_ControllerConfig);
    controllerConfig = (ib_Lin_ControllerConfig*)malloc(controllerConfigSize);
    if (controllerConfig == NULL)
    {
        AbortOnFailedAllocation("Schedule");
        return;
    }
    controllerConfig->interfaceId = ib_InterfaceIdentifier_LinControllerConfig;
    controllerConfig->controllerMode = ib_Lin_ControllerMode_Slave;
    controllerConfig->baudRate = 20000;
    controllerConfig->numFrameResponses = numFrameResponses;
    controllerConfig->frameResponses = (ib_Lin_FrameResponse*)malloc(numFrameResponses * sizeof(ib_Lin_FrameResponse));
    if (controllerConfig->frameResponses == NULL)
    {
        AbortOnFailedAllocation("ib_Lin_FrameResponse");
        return;
    }
    controllerConfig->frameResponses[0] = response_16;
    controllerConfig->frameResponses[1] = response_17;
    controllerConfig->frameResponses[2] = response_18;
    controllerConfig->frameResponses[3] = response_19;
    controllerConfig->frameResponses[4] = response_34;
    ib_Lin_Controller_Init(linController, controllerConfig);
}

void Slave_DoAction(ib_NanosecondsTime now)
{
    slaveNow = now;
    Timer_ExecuteAction(&slaveTimer, now);
}

void Slave_SimTask(void* context, ib_Participant* cbParticipant, ib_NanosecondsTime now)
{
    UNUSED_ARG(context);
    UNUSED_ARG(cbParticipant);

    printf("now=%"PRIu64"ms\n", now / 1000000);
    Slave_DoAction(now);
    SleepMs(500);
}

void Slave_FrameStatusHandler(void* context, ib_Lin_Controller* controller, const ib_Lin_FrameStatusEvent* frameStatusEvent)
{
    UNUSED_ARG(context);
    UNUSED_ARG(controller);

    ib_Lin_Frame* frame = frameStatusEvent->frame;
    printf(">> lin::Frame{id=%d, cs=%d, dl=%d, d={%d %d %d %d %d %d %d %d}} status=%d timestamp=%"PRIu64"ms\n", frame->id,
           frame->checksumModel, frame->dataLength, frame->data[0], frame->data[1], frame->data[2], frame->data[3],
           frame->data[4], frame->data[5], frame->data[6], frame->data[7], frameStatusEvent->status, frameStatusEvent->timestamp/1000000);
}

void Slave_WakeupPulse(ib_NanosecondsTime now) 
{
    printf("<< Wakeup pulse @%"PRIu64"ms\n", now/1000000);
    ib_Lin_Controller_Wakeup(linController);
}

void Slave_GoToSleepHandler(void* context, ib_Lin_Controller* controller, const ib_Lin_GoToSleepEvent* goToSleepEvent)
{
    UNUSED_ARG(context);
    printf("LIN Slave received go-to-sleep command @%" PRIu64 "ms; entering sleep mode.\n",
           goToSleepEvent->timestamp / 1000000);
    // wakeup in 10 ms
    Timer_Set(&slaveTimer, slaveNow + 10000000, &Slave_WakeupPulse);
    ib_Lin_Controller_GoToSleepInternal(controller);
}

void Slave_WakeupHandler(void* context, ib_Lin_Controller* controller, const ib_Lin_WakeupEvent* wakeUpEvent)
{
    UNUSED_ARG(context);
    printf(">> LIN Slave received wakeup pulse @%" PRIu64 "ms; direction=%d; entering normal operation mode.\n",
           wakeUpEvent->timestamp / 1000000, wakeUpEvent->direction);

    if (wakeUpEvent->direction == ib_Direction_Receive)
    {
        ib_Lin_Controller_WakeupInternal(controller);
    }
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        printf("usage: IbDemoCLin <ConfigJsonFile> <ParticipantName> [<DomainId>]\n");
        return 1;
    }

    char* jsonString = LoadFile(argv[1]);
    if (jsonString == NULL)
    {
        printf("Error: cannot open config file %s\n", argv[1]);
        return 1;
    }
    participantName = argv[2]; 

    const char* registryUri = "vib://localhost:8500";
    if (argc >= 4)
    {
        registryUri = argv[3]; 
    }

    ib_ReturnCode returnCode;
    returnCode = ib_Participant_Create(&participant, jsonString, participantName, registryUri, ib_True);
    if (returnCode) {
        printf("%s\n", ib_GetLastErrorString());
        return 2;
    }
    printf("Creating participant '%s' for simulation '%s'\n", participantName, registryUri);

    const char* controllerName = "LIN1";
    const char* networkName = "LIN1";
    returnCode = ib_Lin_Controller_Create(&linController, participant, controllerName, networkName);
    ib_Participant_SetStopHandler(participant, NULL, &StopCallback);
    ib_Participant_SetShutdownHandler(participant, NULL, &ShutdownCallback);
    ib_Participant_SetPeriod(participant, 1000000);
    
    if (strcmp(participantName, "LinMaster") == 0)
    {
        Task tasks[6] = {{0, &Master_SendFrame_16}, {0, &Master_SendFrame_17}, {0, &Master_SendFrame_18},
                         {0, &Master_SendFrame_19}, {0, &Master_SendFrame_34}, {5000000, &Master_GoToSleep}};
        Schedule_Create(&masterSchedule, tasks, 6);

        ib_Participant_SetCommunicationReadyHandler(participant, NULL, &Master_InitCallback);
        ib_HandlerId frameStatusHandlerId;
        ib_Lin_Controller_AddFrameStatusHandler(linController, NULL, &Master_ReceiveFrameStatus, &frameStatusHandlerId);
        ib_HandlerId wakeupHandlerId;
        ib_Lin_Controller_AddWakeupHandler(linController, NULL, &Master_WakeupHandler, &wakeupHandlerId);
        ib_Participant_SetSimulationTask(participant, NULL, &Master_SimTask);
    }
    else
    {
        ib_Participant_SetCommunicationReadyHandler(participant, NULL, &Slave_InitCallback);
        ib_HandlerId frameStatusHandlerId;
        ib_Lin_Controller_AddFrameStatusHandler(linController, NULL, &Slave_FrameStatusHandler, &frameStatusHandlerId);
        ib_HandlerId goToSleepHandlerId;
        ib_Lin_Controller_AddGoToSleepHandler(linController, NULL, &Slave_GoToSleepHandler, &goToSleepHandlerId);
        ib_HandlerId wakeupHandlerId;
        ib_Lin_Controller_AddWakeupHandler(linController, NULL, &Slave_WakeupHandler, &wakeupHandlerId);
        ib_Participant_SetSimulationTask(participant, NULL, &Slave_SimTask);
    }

    ib_ParticipantState outFinalParticipantState;
    ib_LifecycleConfiguration startConfig;
    startConfig.coordinatedStart = ib_True;
    startConfig.coordinatedStop = ib_True;

    returnCode = ib_Participant_StartLifecycleWithSyncTime(participant, &startConfig);
    if(returnCode != ib_ReturnCode_SUCCESS)
    {
        printf("Error: ib_Participant_StartLifecycleWithSyncTime failed: %s\n", ib_GetLastErrorString());
        exit(1);
    }

    returnCode = ib_Participant_WaitForLifecycleToComplete(participant, &outFinalParticipantState);
    if(returnCode != ib_ReturnCode_SUCCESS)
    {
        printf("Error: ib_Participant_WaitForLifecycleToComplete failed: %s\n", ib_GetLastErrorString());
        exit(1);
    }

    printf("Simulation stopped. Final State:%d\n", outFinalParticipantState);

    ib_Participant_Destroy(participant);
    if (jsonString)
    {
        free(jsonString);
    }
    if (controllerConfig)
    {
        free(controllerConfig);
    }
    Schedule_Destroy(masterSchedule);

    return EXIT_SUCCESS;
}
