#include "XPlaneProvider.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>

const static char IP[] = "192.168.1.145";
const static int PORT = 49000;

AttitudeProvider::AttitudeProvider(QObject *parent)
    : QObject{parent}
{    
}

bool AttitudeProvider::isStopping() {
    return this->stopping;
}

bool connectAndReceiveFromXPlane(AttitudeProvider* xp, char **msgs){
    sockaddr_in xplaneAddr;

    int fd = socket(AF_INET,SOCK_DGRAM,0);
    if(fd<0){
        perror("cannot open socket");
        return false;
    }

    bzero(&xplaneAddr,sizeof(xplaneAddr));
    xplaneAddr.sin_family = AF_INET;
    xplaneAddr.sin_addr.s_addr = inet_addr(IP);
    xplaneAddr.sin_port = htons(PORT);

    for (int i = 0; i < 2; i++) {

        if (sendto(fd, msgs[i], 413, 0, (sockaddr*)&xplaneAddr, sizeof(xplaneAddr)) < 0){
        std::cout << ("cannot send message") << std::endl;
            close(fd);
            return false;
        }
    }

    int n = 0;
    while (!xp->isStopping()) {
        char InBuf[413];
        n = recvfrom(fd, InBuf, 413, 0, (sockaddr*)&xplaneAddr,  (socklen_t*)sizeof(xplaneAddr));
        emit xp->setPosition(*(float*)&InBuf[9], *(float*)&InBuf[17]);
    }


    close(fd);
    return true;
}

char * buildMessage(int id, const char *ref, int size, bool stop = false) {
    char *drefs = new char[400]();
    memcpy(drefs, ref, size);

    int frq = 100;
    if (stop) {
        frq = 0;
    }

    char *msg = (char *)malloc(sizeof(char) * 413);
    memcpy(&msg[0], "RREF", 4);
    *(int*)&msg[5] = frq;
    *(int*)&msg[9] = id;
    memcpy(&msg[13], drefs, 400);

    return msg;
}

char** prepareMessages(bool stop = false) {
    char pitch[] = "sim/flightmodel/position/true_theta";
    char roll[] = "sim/flightmodel/position/true_phi";
    char* msg[2] = { buildMessage(1, pitch, sizeof(pitch), stop), buildMessage(2, roll, sizeof(roll), stop) };
    char** messages = msg;
    return messages;
}

void AttitudeProvider::subscribePosition() {

    stopping = false;
    connectAndReceiveFromXPlane(this, prepareMessages());
}

void AttitudeProvider::unsubscribePosition() {
    stopping = true;
    connectAndReceiveFromXPlane(this, prepareMessages(true));
}



