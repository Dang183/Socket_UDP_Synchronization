#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define SFN_MAX 1023

void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main() {
    int sockfd;
    struct sockaddr_in servaddr;
    int broadcast = 1;

    // Tạo socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        error("socket creation failed");
    }

    // Cho phép gửi dữ liệu broadcast
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0) {
        error("setsockopt (SO_BROADCAST) failed");
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Điền thông tin máy chủ
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = INADDR_BROADCAST; // Sử dụng địa chỉ broadcast

    uint16_t gNodeB_sfn = 0;
    uint8_t message_id = 1; // Định danh bản tin rrc_Paging
    int timeCounter = 0;
    
    while (1) {

        if(timeCounter%80 == 0){
            // Gửi bản tin MIB mỗi 80ms
            unsigned char buffer[3];
            buffer[0] = message_id;
            buffer[1] = (gNodeB_sfn >> 8) & 0xFF;
            buffer[2] = gNodeB_sfn & 0xFF;

            sendto(sockfd, buffer, sizeof(buffer), MSG_CONFIRM, (const struct sockaddr *)&servaddr, sizeof(servaddr));
            printf("Sent MIB: message_id = %d, gNodeB_sfn = %d\n", message_id, gNodeB_sfn);
            // printf("Buffer sent: %02X %02X %02X\n", buffer[0], buffer[1], buffer[2]);
            // Tăng message_id
            message_id++;        
        }
        
    
        // Tăng gNodeB_sfn mỗi 10ms
        usleep(10000); // 10ms
        gNodeB_sfn = (gNodeB_sfn + 1) % (SFN_MAX + 1);
        timeCounter+=10;
        
        if(timeCounter == 8000){
            timeCounter = 0;
        }

    }

    close(sockfd);
    return 0;
}
