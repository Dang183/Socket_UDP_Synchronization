#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAXLINE 1024
#define SFN_MAX 1023

void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main() {
    int sockfd;
    unsigned char buffer[MAXLINE];
    struct sockaddr_in servaddr, cliaddr;

    // Tạo socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        error("socket creation failed");
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Điền thông tin máy chủ
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    // Gắn socket với địa chỉ IP và cổng
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        error("bind failed");
    }

    int UE_sfn = 0;
    int is_synced = 0;
    int sync_interval = 80;
    int sync_counter = 0;
    int tmp;
    while (1) {
        
        if(sync_counter%80 == 0 && sync_counter != 0){
            socklen_t len = sizeof(cliaddr);
            int n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *)&cliaddr, &len);
            if (n < 0) {
                error("recvfrom failed");
            }

            // Xử lý và in ra thông tin MIB
            uint8_t message_id = buffer[0];
            uint16_t gNodeB_sfn = (buffer[1] << 8) | buffer[2];
            tmp = gNodeB_sfn % (SFN_MAX + 1);
            // printf("Buffer received: %02X %02X %02X\n", buffer[0], buffer[1], buffer[2]);
            printf("Received MIB: message_id = %d, gNodeB_sfn = %d\n", message_id, tmp);   
        }
        
        // Nhận bản tin MIB từ gNodeB mỗi 80ms
        if (sync_counter >= sync_interval) {
            sync_counter = 0;
            
            if(UE_sfn == tmp){
                is_synced = 1;
            }else{
                is_synced = 0;
            }

            // Đồng bộ UE_sfn với gNodeB_sfn
            if (is_synced) {
                sync_interval = 800;
            } else {
                sync_interval = 80;
            }

            UE_sfn = tmp;
            printf("Update UE_sfn = %d\n", UE_sfn); 
            printf("-----------------\n");
        }

        // Tăng UE_sfn mỗi 10ms
        usleep(10000); // 10ms
        printf("UE_sfn = %d\n", UE_sfn); 
        UE_sfn = (UE_sfn + 1) % (SFN_MAX + 1);
        sync_counter += 10;
    }

    close(sockfd);
    return 0;
}
