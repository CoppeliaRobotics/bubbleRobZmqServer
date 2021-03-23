#include <stdio.h>
#include <zmq.h>
#include <cstdlib>
#include <string>
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif
#define pi 3.141593f

int main(int argc, char* argv[])
{ // We connect to the port number passed as argument
    int portNb=0;
    if (argv[1]!=NULL)
        portNb=atoi(argv[1]);
    if (portNb==0)
    {
        printf("Indicate the connection port number as argument!\n");
        #ifdef _WIN32
            Sleep(5000);
        #else
            usleep(5000*1000);
        #endif
        return 0;
    }
    std::string add("tcp://*:");
    add+=std::to_string(portNb);

    void* context=zmq_ctx_new();
    void* socket=zmq_socket(context,ZMQ_REP);
    int timeout=1000;
    zmq_setsockopt(socket,ZMQ_RCVTIMEO,&timeout,sizeof(int));
    zmq_setsockopt(socket,ZMQ_LINGER,&timeout,sizeof(int));

    printf("Connecting to client...\n");
    zmq_bind(socket,add.c_str());
    printf("Connected with client.\n");
    float driveBackStartTime=-99.0f;
    float receivedData[2];
    float motorSpeeds[2];
    while (true)
    { // This is the server loop (the robot's control loop):
        int res=zmq_recv(socket,receivedData,2*sizeof(float),0);
        if (res==-1)
            break;
        // We received 2 floats (the sensorDistance and the simulation time):
        float sensorReading=receivedData[0];
        float simulationTime=receivedData[1];

        // We have a very simple control: when we detected something, we simply drive back while slightly turning for 3 seconds
        // If we didn't detect anything, we drive forward:
        if (simulationTime-driveBackStartTime<3.0f)
        { // driving backwards while slightly turning:
            motorSpeeds[0]=-7.0f*0.5f;
            motorSpeeds[1]=-7.0f*0.25f;
        }
        else
        { // going forward:
            motorSpeeds[0]=7.0f;
            motorSpeeds[1]=7.0f;
            if (sensorReading>0.0f)
                driveBackStartTime=simulationTime; // We detected something, and start the backward mode
        }

        // We reply with 2 floats (the left and right motor speed in rad/sec)
        res=zmq_send(socket,motorSpeeds,2*sizeof(float),0);
        if (res==-1)
            break;
    }
    #ifdef _WIN32
        Sleep(5000);
    #else
        usleep(5000*1000);
    #endif
    return 0;
}

