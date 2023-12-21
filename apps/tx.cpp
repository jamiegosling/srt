#include <srt.h>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>

#include "apputil.hpp"

int main() {
    // Initialize SRT library
    if (srt_startup() != 0) {
        std::cerr << "Error: SRT library initialization failed." << std::endl;
        return -1;
    }

    // Create SRT socket
    int srtSocket = srt_create_socket();

    // Set socket options (for example purposes)
    int latency = 120;  // milliseconds
    srt_setsockopt(srtSocket, 0, SRTO_LATENCY, &latency, sizeof(latency));

    // Set target SRT address
    srt::sockaddr_any srtAddress = CreateAddr("127.0.0.1", 1235);
    sockaddr* target = srtAddress.get();

    // Set source UDP address
    // srt::sockaddr_any udpAddr = CreateAddr("0.0.0.0", 1234);
    // sockaddr* psa = udpAddr.get();

    // Create a UDP socket to receive data 
    int udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in udpAddress;
    udpAddress.sin_family = AF_INET;
    udpAddress.sin_addr.s_addr = INADDR_ANY;
    udpAddress.sin_port = htons(1234);  // Replace with the desired local UDP port

    // Bind the socket to a UDP port
    std::cout << "Trying to connect to UDP source..." << std::endl;
    // if (srt_bind(srtSocket, psa, 1234) < 0) {
    //     std::cerr << "Error: Unable to bind to local UDP port." << std::endl;
    //     srt_close(srtSocket);
    //     srt_cleanup();
    //     return -1;
    // }

    if (bind(udpSocket, (struct sockaddr*)&udpAddress, sizeof(udpAddress)) < 0) {
        std::cerr << "Error: Unable to bind to local UDP port for data reception." << std::endl;
        srt_close(srtSocket);
        srt_cleanup();
        close(udpSocket);
        return -1;
    }

    // Connect to the SRT target
    std::cout << "Trying to connect to SRT target..." << std::endl;
    std::cout << target << std::endl;
    while (true) {
        if (srt_connect(srtSocket, target, sizeof(srtAddress)) == 0) {
            std::cout << "SRT connection successful!" << std::endl;
            break;
            // return -1;
        }
    }
    // std::cerr << "Error: Unable to connect to the SRT target." << std::endl;
    // srt_close(srtSocket);
    // srt_cleanup();



    // Receive and transmit data
    const int bufferSize = 4096;
    char buffer[bufferSize];

    std::cout << "Connected to source and target, sending..." << std::endl;
    while (true) {
        // Receive data from the UDP port
        // std::cout << "Recieving..." << std::endl;
        ssize_t bytesRead = recv(udpSocket, buffer, bufferSize, 0);
        if (bytesRead < 0) {
            std::cerr << "Error: Failed to receive data from UDP port." << std::endl;
            break;
        }

        // Send the received data over the SRT socket
        // std::cout << "Sending..." << std::endl;
        int sentBytes = srt_send(srtSocket, buffer, bytesRead);
        if (sentBytes < 0) {
            std::cerr << "Error: Failed to send data over SRT." << std::endl;
            break;
        }

        // Sleep for a short duration as an example
        // std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Close sockets and cleanup
    srt_close(srtSocket);
    srt_cleanup();
    close(udpSocket);

    return 0;
}