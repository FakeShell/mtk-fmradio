#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "fmradio.h"

int main() {
    int ret = 0;
    int fd;
    int freq = 8750;

    ret = fm_open_dev(FM_DEV, &fd);
    if (ret < 0) {
        printf("error opening device: %d\n", ret);
    }

    ret = fm_powerup(fd, FM_BAND_UE, freq);
    if (ret < 0) {
        printf("error powering up: %d\n", ret);
    }

    // just testing stuff
    fm_getrssi(fd, &ret);
    fm_setvol(fd, 10);
    fm_getvol(fd, &ret);
    fm_getchipid(fd, &ret);
    fm_set_stereo_mono(fd, 1);
    fm_get_stereo_mono(fd, &ret);
    fm_is_tx_support(fd, &ret);
    fm_rds_support(fd, &ret);
    fm_getbadratio(fd, &ret);
    fm_getbadbnt(fd, &ret);
    fm_getgoodbcnt(fd, &ret);
    fm_get_caparray(fd, &ret);
    fm_is_fm_pwrup(fd, &ret);

    sleep(50);

    ret = fm_powerdown(fd, 0);
    if (ret < 0) {
        printf("error powering down: %d\n", ret);
    }

    ret = fm_close_dev(fd);
    if (ret < 0) {
        printf("error closing device: %d\n", ret);
    }
}
