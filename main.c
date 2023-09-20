#include <stdio.h>
#include <hidapi/hidapi.h>
#include <errno.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>

#define MAX_EVENTS   1024
#define MAX_FILEPATH 64
#define EVENT_SIZE   (sizeof(struct inotify_event))
#define BUF_LEN      (MAX_EVENTS * (EVENT_SIZE + MAX_EVENTS))

#define VENDOR_ID 0x0b05
#define PRODUCT_ID 0x19b6
#define SERIAL_NUMBER L""

#define BRIGHTNESS_SRC "/sys/class/leds/asus::kbd_backlight/brightness"

unsigned char AURA_INIT[] = {0x5d,0xb3,0x00,0x00,0xa6,0x00,0x00,0xeb,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
unsigned char LED_SET  [] = {0x5d,0xb5,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
unsigned char LED_APPLY[] = {0x5d,0xb4,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
unsigned char LED_ON   [] = {0x5d,0xbd,0x01,0x0f,0x06,0x03,0x03};
unsigned char LED_OFF  [] = {0x5d,0xbd,0x01,0x00,0x00,0x00,0x00};

#define BRIGHNESS_BIT 4
unsigned char BRIGHTNESS[] = {0x5a,0xba,0xc5,0xc4,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

int inotify_fd;
int inotify_wd;
int hid_init_r;
hid_device* device;

void cleanup(){
    printf("cleaning up...\n");
    if(inotify_fd != -1){
        if(inotify_wd != -1) inotify_rm_watch(inotify_fd, inotify_wd);
        close(inotify_fd);
    }
    if(device != NULL) hid_close(device);
    if(hid_init_r != -1) hid_exit();
}

void exitsignal(int signal){
    cleanup();
    exit(0);
}

/* Sets the backlight level to whatever is provided. */
int set_brighntess(int brightness){
    BRIGHTNESS[BRIGHNESS_BIT] = brightness;
    if(hid_write(device, BRIGHTNESS, sizeof(BRIGHTNESS)) == -1){
        printf("failed to write data to hid: '%ls'\n", hid_error(device));
        return -1;
    }

    return 0;
}

/* Sets the backlight level to the same as the one in the file. */
int synchronize(char* buffer, size_t size){
    FILE* src = fopen(BRIGHTNESS_SRC, "r");
    if(src == NULL){
        printf("failed to read backlight source: '%s'\n", strerror(ferror(src)));
        return -1;
    }
    if(fgets(buffer, size, src) == NULL){
        fclose(src);
        printf("failed to read backlight source: '%s'\n", strerror(ferror(src)));
        return -1;
    }
    fclose(src);

    return set_brighntess(atoi(buffer));
}

int main(int argc, char** argv){
    // cleanup for all the mess we are about to commit
    signal(SIGINT, exitsignal);

    // setup the watcher
    if(access(BRIGHTNESS_SRC, R_OK) == -1){
        printf("can't access brightness source file: '%s'\n", strerror(errno));
        cleanup();
        return 1;
    }
    inotify_fd = inotify_init();
    if(inotify_fd == -1){
        printf("failed to create inotify: '%s'\n", strerror(errno));
        cleanup();
        return 1;
    }
    inotify_wd = inotify_add_watch(inotify_fd, BRIGHTNESS_SRC, IN_MODIFY);
    if(inotify_wd == -1){
        printf("failed to create inotify watch: '%s'\n", strerror(errno));
        cleanup();
        return 1;
    }
    
    // setup the device
    if((hid_init_r = hid_init()) == -1) {
        printf("failed to init hidapi\n");
        cleanup();
        return 1;
    }

    device = hid_open(VENDOR_ID, PRODUCT_ID, SERIAL_NUMBER);
    if(!device) {
        printf("failed to open device: '%ls'\n", hid_error(NULL));
        cleanup();
        return 1;
    }

    // synchornize from the source
    char brightness_buffer[2];
    if(synchronize(brightness_buffer, sizeof(brightness_buffer)) == -1){
        cleanup();
        return 1;
    }

    // monitor for changes
    char buffer[BUF_LEN];
    while(1){
        int i = 0;
        int length = read(inotify_fd, buffer, BUF_LEN);

        while(i < length){
            struct inotify_event* event = (struct inotify_event*)&buffer[i];            
            if(synchronize(brightness_buffer, sizeof(brightness_buffer)) == -1){
                cleanup();
                return 1;
            }
            i += EVENT_SIZE + event->len;
        }
    }
}