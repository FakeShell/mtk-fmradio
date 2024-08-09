#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "fmradio.h"

static int g_stopscan = 0;
static int scan_req_init_flag = 0;
static struct fm_scan_t scan_req;

int fm_get_seek_space() {
    return FM_SPACE_DEFAULT;
}

void fm_change_string(uint8_t *str, int len) {
    for (int i = 0; i < len; i++) {
        if (str[i] < 0x20 || str[i] > 0x7E)
            str[i] = 0x20;
    }
}

int fm_open_dev(const char *pname, int *fd) {
    int ret = 0;
    int tmp = -1;

    if (!pname || !fd) {
        printf("fm_open_dev: pname or fd is invalid\n");
        return -1;
    }

    tmp = open(pname, O_RDWR);
    if (tmp < 0) {
        printf("fm_open_dev: Open %s failed, %s\n", pname, strerror(errno));
        ret = -1;
    } else {
        *fd = tmp;
        printf("fm_open_dev: [fd=%d] [ret=%d]\n", *fd, ret);
    }
    return ret;
}

int fm_close_dev(int fd) {
    int ret = 0;

    ret = close(fd);
    if (ret)
        printf("fm_close_dev: failed\n");
    else
        printf("fm_close_dev: [ret=%d]\n", ret);

    return ret;
}

int fm_powerup(int fd, int band, int freq) {
    int ret = 0;
    struct fm_tune_parm parm;

    parm.band = band;
    parm.freq = freq;
    parm.hilo = FM_AUTO_HILO_OFF;
    parm.space = FM_SEEK_SPACE;

    ret = ioctl(fd, FM_IOCTL_POWERUP, &parm);
    if (ret < 0)
        perror("FM_IOCTL_POWERUP failed");
    else
        printf("fm_powerup: [ret=%d]\n", ret);

    return ret;
}

int fm_powerdown(int fd, int type) {
    int ret = 0;
    ret = ioctl(fd, FM_IOCTL_POWERDOWN, &type);
    if (ret < 0)
        perror("FM_IOCTL_POWERDOWN failed");
    else
        printf("fm_powerdown: [ret=%d]\n", ret);

    return ret;
}

int fm_tune(int fd, int freq, int band) {
    int ret = 0;
    struct fm_tune_parm parm;

    parm.band = band;
    parm.freq = freq;
    parm.hilo = FM_AUTO_HILO_OFF;
    parm.space = FM_SEEK_SPACE;

    ret = ioctl(fd, FM_IOCTL_TUNE, &parm);
    if (ret < 0)
        perror("FM_IOCTL_TUNE failed");
    else
        printf("fm_tune: [freq=%d] [ret=%d]\n", freq, ret);

    return ret;
}

int fm_seek(int fd, int *freq, int band, int dir, int lev) {
    int ret = 0;
    struct fm_seek_parm parm;

    parm.band = band;
    parm.freq = *freq;
    parm.hilo = FM_AUTO_HILO_OFF;
    parm.space = FM_SEEK_SPACE;
    if (dir == 1)
        parm.seekdir = FM_SEEK_UP;
    else if (dir == 0)
        parm.seekdir = FM_SEEK_DOWN;

    parm.seekth = lev;

    ret = ioctl(fd, FM_IOCTL_SEEK, &parm);
    if (ret < 0)
        perror("FM_IOCTL_SEEK failed");
    else {
        *freq = parm.freq;
        printf("fm_seek: [freq=%d] [ret=%d]\n", *freq, ret);
    }

    return ret;
}

int fm_setvol(int fd, int vol) {
    int ret = 0;

    ret = ioctl(fd, FM_IOCTL_SETVOL, &vol);
    if (ret < 0)
        perror("FM_IOCTL_SETVOL failed");
    else
        printf("fm_setvol: [ret=%d]\n", ret);

    return ret;
}

int fm_getvol(int fd, int *vol) {
    int ret = 0;

    ret = ioctl(fd, FM_IOCTL_GETVOL, vol);
    if (ret < 0)
        perror("FM_IOCTL_GETVOL failed");
    else
        printf("fm_getvol: [vol=%d] [ret=%d]\n", *vol, ret);

    return ret;
}

int fm_mute(int fd, int mute) {
    int ret = 0;
    int tmp = mute;

    ret = ioctl(fd, FM_IOCTL_MUTE, &tmp);
    if (ret < 0)
        perror("FM_IOCTL_MUTE failed");
    else
        printf("fm_mute: [ret=%d]\n", ret);

    return ret;
}

int fm_getrssi(int fd, int *rssi) {
    int ret = 0;

    ret = ioctl(fd, FM_IOCTL_GETRSSI, rssi);
    if (ret < 0)
        perror("FM_IOCTL_GETRSSI failed");
    else
 	printf("fm_getrssi: [rssi=%d] [ret=%d]\n", *rssi, ret);

    return ret;
}

int fm_scan(int fd, struct fm_scan_parm *scan_parm) {
    int ret = 0;

    ret = ioctl(fd, FM_IOCTL_SCAN, scan_parm);
    if (ret < 0)
        perror("FM_IOCTL_SCAN failed");
    else
        printf("fm_scan: [ret=%d]\n", ret);

    return ret;
}

int fm_stop_scan(int fd) {
    int ret = 0;

    ret = ioctl(fd, FM_IOCTL_STOP_SCAN);
    if (ret < 0)
        perror("FM_IOCTL_STOP_SCAN failed");
    else
        printf("fm_stop_scan: [ret=%d]\n", ret);

    return ret;
}

int fm_getchipid(int fd, int *chipid) {
    int ret = 0;
    uint16_t tmp = 0;

    ret = ioctl(fd, FM_IOCTL_GETCHIPID, &tmp);
    *chipid = (int)tmp;
    if (ret < 0)
        perror("FM_IOCTL_GETCHIPID failed");
    else
        printf("fm_getchipid: [chipid=%x] [ret=%d]\n", *chipid, ret);

    return ret;
}

int fm_getcurpamd(int fd, int *pamd) {
    int ret = 0;

    ret = ioctl(fd, FM_IOCTL_GETCURPAMD, pamd);
    if (ret < 0)
        perror("FM_IOCTL_GETCURPAMD failed");
    else
        printf("fm_getcurpamd: [ret=%d]\n", ret);

    return ret;
}

int fm_getgoodbcnt(int fd, int *goodbcnt) {
    int ret = 0;

    ret = ioctl(fd, FM_IOCTL_GETGOODBCNT, goodbcnt);
    if (ret < 0)
        perror("FM_IOCTL_GETGOODBCNT failed");
    else
        printf("fm_getgoodbcnt: [goodbcnt=%d] [ret=%d]\n", *goodbcnt, ret);

    return ret;
}

int fm_getbadbnt(int fd, int *badbnt) {
    int ret = 0;

    ret = ioctl(fd, FM_IOCTL_GETBADBNT, badbnt);
    if (ret < 0)
        perror("FM_IOCTL_GETBADBNT failed");
    else
        printf("fm_getbadbnt: [badbnt=%d] [ret=%d]\n", *badbnt, ret);

    return ret;
}

int fm_getbadratio(int fd, int *badratio) {
    int ret = 0;

    ret = ioctl(fd, FM_IOCTL_GETBLERRATIO, badratio);
    if (ret < 0)
        perror("FM_IOCTL_GETBLERRATIO failed");
    else
        printf("fm_getbadratio: [badratio=%d] [ret=%d]\n", *badratio, ret);

    return ret;
}

int fm_rds_onoff(int fd, int onoff) {
    int ret = 0;
    uint16_t rds_on = -1;

    if (onoff == FMR_RDS_ON) {
        rds_on = 1;
        ret = ioctl(fd, FM_IOCTL_RDS_ONOFF, &rds_on);
        if (ret < 0)
            printf("FM_IOCTL_RDS_ON failed\n");
        else
            printf("Rdsset Success [rds_on=%d] [ret=%d]\n", rds_on, ret);
    } else {
        rds_on = 0;
        ret = ioctl(fd, FM_IOCTL_RDS_ONOFF, &rds_on);
        if (ret < 0)
            printf("FM_IOCTL_RDS_OFF failed\n");
        else
            printf("Rdsset Success [rds_on=%d] [ret=%d]\n", rds_on, ret);
    }
    return ret;
}

int fm_rds_support(int fd, int *support) {
    int ret = 0;

    ret = ioctl(fd, FM_IOCTL_RDS_SUPPORT, &support);
    if (ret < 0)
        perror("FM_IOCTL_RDS_SUPPORT failed");
    else
        printf("fm_rds_support: [support=%d] [ret=%d]\n", support, ret);

    return ret;
}

int fm_pre_search(int fd) {
    int ret = 0;
    ret = ioctl(fd, FM_IOCTL_PRE_SEARCH, 0);

    if (ret < 0)
        perror("FM_IOCTL_PRE_SEARCH failed");
    else
        printf("fm_pre_search: %d [ret=%d]\n",ret);

    return ret;
}

int fm_restore_search(int fd) {
    int ret = 0;

    ret = ioctl(fd, FM_IOCTL_RESTORE_SEARCH, 0);
    if (ret < 0)
        perror("FM_IOCTL_RESTORE_SEARCH failed");
    else
        printf("fm_restore_search: %d [ret=%d]\n",ret);

    return ret;
}

int fm_soft_mute_tune(int fd, int freq) {
    int ret = 0;

    struct fm_softmute_tune_t value;
    value.freq = freq;

    ret = ioctl(fd, FM_IOCTL_SOFT_MUTE_TUNE, &value);
    if (ret < 0)
        perror("FM_IOCTL_SOFT_MUTE_TUNE failed");
    else
        printf("fm_soft_mute_tune: [ret=%d]\n", ret);

    return ret;
}

int fm_get_stereo_mono(int fd, int *stereo) {
    int ret = 0;

    ret = ioctl(fd, FM_IOCTL_GETMONOSTERO, stereo);
    if (ret < 0)
        perror("FM_IOCTL_GETMONOSTERO failed");
    else
        printf("fm_get_stereo_mono: [stereo=%d] [ret=%d]\n", *stereo, ret);

    return ret;
}

int fm_set_stereo_mono(int fd, int stereo) {
    int ret = 0;

    ret = ioctl(fd, FM_IOCTL_SETMONOSTERO, &stereo);
    if (ret < 0)
        perror("FM_IOCTL_SETMONOSTERO failed");
    else
        printf("fm_set_stereo_mono: [ret=%d]\n", ret);

    return ret;
}

int fm_get_caparray(int fd, int *caparray) {
    int ret = 0;

    ret = ioctl(fd, FM_IOCTL_GETCAPARRAY, caparray);
    if (ret < 0)
        perror("FM_IOCTL_GETCAPARRAY failed");
    else
        printf("fm_get_caparray: [caparray=%d] [ret=%d]\n", *caparray, ret);

    return ret;
}

int fm_get_hw_info(int fd, struct fm_hw_info *info) {
    int ret = 0;

    ret = ioctl(fd, FM_IOCTL_GET_HW_INFO, info);

    if (ret < 0)
        perror("FM_IOCTL_GET_HW_INFO failed");
    else
        printf("fm_get_hw_info: [ret=%d]\n", ret);

    return ret;
}

int fm_is_dese_chan(int fd, int freq) {
    int ret = 0;
    int tmp = freq;

    ret = ioctl(fd, FM_IOCTL_IS_DESE_CHAN, &freq);
    if (ret < 0) {
        perror("FM_IOCTL_IS_DESE_CHAN failed");
        return ret;
    } else {
        printf("fm_is_dese_chan: %d --> dese=%d\n", tmp, freq);
        return freq;
    }
}

int fm_desense_check(int fd, int freq, int rssi) {
    int ret = 0;
    fm_desense_check_t parm;

    parm.freq = freq;
    parm.rssi = rssi;
    ret = ioctl(fd, FM_IOCTL_DESENSE_CHECK, &parm);
    if (ret < 0)
        perror("FM_IOCTL_DESENSE_CHECK failed");
    else
        printf("fm_desense_check: %d --> dese=%d\n", freq, ret);

    return ret;
}

int fm_set_search_threshold(int fd, int th_idx, int th_val) {
    int ret = 0;

    struct fm_search_threshold_t th_parm;
    th_parm.th_type = th_idx;
    th_parm.th_val = th_val;
    ret = ioctl(fd, FM_IOCTL_SET_SEARCH_THRESHOLD, &th_parm);
    if (ret < 0)
        perror("FM_IOCTL_SET_SEARCH_THRESHOLD failed");
    else
        printf("fm_set_search_threshold: [ret=%d]\n", ret);

    return ret;
}

int fm_full_cqi_logger(int fd, fm_full_cqi_log_t *log_parm) {
    int ret = 0;

    ret = ioctl(fd, FM_IOCTL_FULL_CQI_LOG, log_parm);
    if (ret < 0)
        perror("FM_IOCTL_FULL_CQI_LOG failed");
    else
        printf("fm_full_cqi_logger: [ret=%d]\n", ret);

    return ret;
}

int fm_ana_switch(int fd, int antenna) {
    int ret = 0;

    ret = ioctl(fd, FM_IOCTL_ANA_SWITCH, &antenna);
    if (ret < 0)
        perror("FM_IOCTL_ANA_SWITCH failed");
    else
        printf("fm_ana_switch: [ret=%d]\n", ret);

    return ret;
}

static int fm_get_af_pi(int fd, uint16_t *pi) {
    struct rds_raw_data rrd;
    uint16_t pi1, pi2;

    if (pi == NULL) {
        fprintf(stderr, "pi is NULL\n");
        return -1;
    }

    memset(&rrd, 0, sizeof(rrd));
    if (ioctl(fd, FM_IOCTL_RDS_GET_LOG, &rrd) < 0) {
        perror("FM_IOCTL_RDS_GET_LOG failed");
        *pi = 0;
        return -1;
    }

    if (rrd.len == 0) {
        fprintf(stderr, "RDS log is empty\n");
        *pi = 0;
        return -1;
    }

    pi1 = rrd.data[4];
    pi1 |= (rrd.data[5] << 8);
    printf("data[4]=%02x, data[5]=%02x, pi1=%04x\n", rrd.data[4], rrd.data[5], pi1);

    pi2 = rrd.data[16];
    pi2 |= (rrd.data[17] << 8);
    printf("data[16]=%02x, data[17]=%02x, pi2=%04x\n", rrd.data[16], rrd.data[17], pi2);

    if (pi1 == pi2) {
        printf("AF PI found\n");
        *pi = pi1;
    } else {
        fprintf(stderr, "AF PI check failed\n");
        *pi = 0;
        return -1;
    }

    return 0;
}

int fm_get_af_list(RDSData_Struct *rds, int16_t **af_list, int *len) {
    int ret = 0;
    AF_Info af_list_temp;

    if (rds == NULL) {
        fprintf(stderr, "Error: rds is NULL\n");
        return -1;
    }

    if (af_list == NULL) {
        fprintf(stderr, "Error: af_list is NULL\n");
        return -1;
    }

    if (!(rds->event_status & RDS_EVENT_AF)) {
        fprintf(stderr, "Get AF list failed: No AF data\n");
        return -ERR_RDS_NO_DATA;
    }

    memset(&af_list_temp, 0, sizeof(af_list_temp));
    memcpy(&af_list_temp, &rds->AF_Data, sizeof(AF_Info));

    af_list_temp.AF_Num = af_list_temp.AF_Num > 25 ? 25 : af_list_temp.AF_Num;
    printf("AF list length: %d\n", af_list_temp.AF_Num);
    *len = af_list_temp.AF_Num;
    *af_list = &af_list_temp.AF[1][0];

    return ret;
}

int fm_get_cqi(int fd, int num, char *buf, int buf_len) {
    int ret;
    struct fm_cqi_req cqi_req;

    num = (num > CQI_CH_NUM_MAX) ? CQI_CH_NUM_MAX : num;
    num = (num < CQI_CH_NUM_MIN) ? CQI_CH_NUM_MIN : num;
    cqi_req.ch_num = (uint16_t)num;
    cqi_req.buf_size = cqi_req.ch_num * sizeof(struct fm_cqi);

    if (!buf || (buf_len < cqi_req.buf_size)) {
        fprintf(stderr, "Error: Invalid buffer\n");
        return -1;
    }

    cqi_req.cqi_buf = buf;

    ret = ioctl(fd, FM_IOCTL_CQI_GET, &cqi_req);
    if (ret < 0) {
        perror("FM_IOCTL_CQI_GET failed");
        return -1;
    }

    return 0;
}

int fm_get_ps(RDSData_Struct *rds, uint8_t **ps, int *ps_len) {
    int ret = 0;
    char tmp_ps[9] = {0};

    if (rds == NULL) {
        fprintf(stderr, "rds is NULL\n");
        return -1;
    }

    if (ps == NULL) {
        fprintf(stderr, "ps is NULL\n");
        return -1;
    }

    if (ps_len == NULL) {
        fprintf(stderr, "ps_len is NULL\n");
        return -1;
    }

    if (rds->event_status & RDS_EVENT_PROGRAMNAME) {
        printf("fm_get_ps: Success: [event_status=%d]\n", rds->event_status);
        *ps = &rds->PS_Data.PS[3][0];
        *ps_len = sizeof(rds->PS_Data.PS[3]);

        fm_change_string(*ps, *ps_len);
        memcpy(tmp_ps, *ps, 8);
        tmp_ps[8] = '\0';
        printf("PS=%s\n", tmp_ps);
    } else {
        fprintf(stderr, "fm_get_ps: Failed: [event_status=%d]\n", rds->event_status);
        *ps = NULL;
        *ps_len = 0;
        ret = -ERR_RDS_NO_DATA;
    }

    return ret;
}

int fm_get_rt(RDSData_Struct *rds, uint8_t **rt, int *rt_len) {
    int ret = 0;
    char tmp_rt[65] = { 0 };

    if (rds == NULL) {
        fprintf(stderr, "rds is NULL\n");
        return -1;
    }

    if (rt == NULL) {
        fprintf(stderr, "rt is NULL\n");
        return -1;
    }

    if (rt_len == NULL) {
        fprintf(stderr, "rt_len is NULL\n");
        return -1;
    }

    if (rds->event_status & RDS_EVENT_LAST_RADIOTEXT) {
        printf("fm_get_rt: Success: [event_status=%d]\n", rds->event_status);
        *rt = &rds->RT_Data.TextData[3][0];
        *rt_len = rds->RT_Data.TextLength;

        fm_change_string(*rt, *rt_len);
        memcpy(tmp_rt, *rt, 64);
        tmp_rt[64] = '\0';
        printf("RT=%s\n", tmp_rt);
    } else {
        fprintf(stderr, "fm_get_rt: Failed: [event_status=%d]\n", rds->event_status);
        *rt = NULL;
        *rt_len = 0;
        ret = -ERR_RDS_NO_DATA;
    }
    return ret;
}

int fm_get_pi(RDSData_Struct *rds, uint16_t *pi) {
    int ret = 0;

    if (rds == NULL) {
        fprintf(stderr, "rds is NULL\n");
        return -1;
    }

    if (pi == NULL) {
        fprintf(stderr, "pi is NULL\n");
        return -1;
    }

    if (rds->event_status & RDS_EVENT_PI_CODE) {
        printf("fm_get_pi: Success: [event_status=%d] [PI=%d]\n", rds->event_status, rds->PI);
        *pi = rds->PI;
    } else {
        printf("fm_get_pi: Failed: there's no PI, [event_status=%d]\n", rds->event_status);
        *pi = (uint16_t)-1;
        ret = -ERR_RDS_NO_DATA;
    }

    return ret;
}

int fm_get_ecc(RDSData_Struct *rds, uint8_t *ecc) {
    int ret = 0;

    if (rds == NULL) {
        fprintf(stderr, "rds is NULL\n");
        return -1;
    }

    if (ecc == NULL) {
        fprintf(stderr, "ecc is NULL\n");
        return -1;
    }

    if (rds->event_status & RDS_EVENT_ECC_CODE) {
        printf("fm_get_ecc: Success: [event_status=%d] [ECC=%d]\n", rds->event_status, rds->Extend_Country_Code);
        *ecc = rds->Extend_Country_Code;
    } else {
        printf("fm_get_ecc: Failed: there's no ECC, [event_status=%d]\n", rds->event_status);
        *ecc = (uint8_t)-1;
        ret = -ERR_RDS_NO_DATA;
    }

    return ret;
}

int fm_get_pty(RDSData_Struct *rds, uint8_t *pty) {
    int ret = 0;

    if (rds == NULL) {
        fprintf(stderr, "rds is NULL\n");
        return -1;
    }

    if (pty == NULL) {
        fprintf(stderr, "pty is NULL\n");
        return -1;
    }

    if (rds->event_status & RDS_EVENT_PTY_CODE) {
        printf("fm_get_pty: Success: [event_status=%d] [PTY=%d]\n", rds->event_status, rds->PTY);
        *pty = rds->PTY;
    } else {
        printf("fm_get_pty: Failed: there's no PTY, [event_status=%d]\n", rds->event_status);
        *pty = (uint8_t)-1;
        ret = -ERR_RDS_NO_DATA;
    }

    return ret;
}

int fm_tx_pwrup(int fd, int band, int freq) {
    int ret = 0;
    struct fm_tune_parm parm_tune;

    parm_tune.band = band;
    parm_tune.freq = freq;
    parm_tune.hilo = FM_AUTO_HILO_OFF;
    parm_tune.space = fm_get_seek_space();

    ret = ioctl(fd, FM_IOCTL_POWERUP_TX, &parm_tune);
    if (ret < 0)
        perror("FM_IOCTL_POWERUP_TX failed");
    else
        printf("fm_tx_pwrup: [freq=%d] [ret=%d]\n", freq, ret);

    return ret;
}

int fm_tx_tune(int fd, int band, int freq) {
    int ret = 0;
    struct fm_tune_parm parm_tune;

    parm_tune.band = band;
    parm_tune.freq = freq;
    parm_tune.hilo = FM_AUTO_HILO_OFF;
    parm_tune.space = fm_get_seek_space();

    ret = ioctl(fd, FM_IOCTL_TUNE_TX, &parm_tune);
    if (ret < 0)
        perror("FM_IOCTL_TUNE_TX failed");
    else
        printf("fm_tx_tune: [freq=%d] [ret=%d]\n", freq, ret);

    return ret;
}

int fm_tx_scan(int fd, int band, int start_freq, int dir, int *num, uint16_t *tbl) {
    int ret = 0;
    struct fm_tx_scan_parm parm;

    memset(&parm, 0, sizeof(struct fm_tx_scan_parm));
    parm.band = band;
    parm.space = fm_get_seek_space();
    parm.hilo = FM_AUTO_HILO_OFF;
    parm.freq = start_freq;
    parm.scandir = dir;
    parm.ScanTBLSize = *num;

    printf("fm_tx_scan: [parm.band=%d] [parm.space=%d] [parm.hilo=%d] [parm.freq=%d]\n",
           parm.band, parm.space, parm.hilo, parm.freq);

    ret = ioctl(fd, FM_IOCTL_TX_SCAN, &parm);
    if (ret < 0) {
        perror("FM_IOCTL_TX_SCAN failed");
        *num = 0;
    } else {
        *num = parm.ScanTBLSize;
        memcpy(tbl, &parm.ScanTBL[0], parm.ScanTBLSize * sizeof(uint16_t));
    }
    printf("fm_tx_scan: [num=%d] [ret=%d]\n", parm.ScanTBLSize, ret);

    return ret;
}

int fm_is_tx_support(int fd, int *supt) {
    int ret = 0;

    ret = ioctl(fd, FM_IOCTL_TX_SUPPORT, supt);
    if (ret < 0) {
        perror("FM_IOCTL_TX_SUPPORT failed");
        *supt = -1;
    }
    printf("fm_is_tx_support: [support=%d] [ret=%d]\n", *supt, ret);
    return ret;
}

int fm_fm_over_bt(int fd, int onoff) {
    int ret = 0;

    ret = ioctl(fd, FM_IOCTL_OVER_BT_ENABLE, &onoff);
    if (ret < 0)
        perror("FM_IOCTL_OVER_BT_ENABLE failed");
    else
        printf("fm_fm_over_bt: [ret=%d]\n", ret);

    return ret;
}

int fm_rdstx_onoff(int fd, int onoff) {
    int ret = 0;

    ret = ioctl(fd, FM_IOCTL_RDSTX_ENABLE, &onoff);
    if (ret < 0)
        perror("FM_IOCTL_RDSTX_ENABLE failed");
    else
        printf("fm_rdstx_onoff: [ret=%d]\n", ret);

    return ret;
}

int fm_tune_new(int fd, int freq, int upper, int lower, int space, void *para) {
    int ret = 0;
    struct fm_tune_t tune_req;

    tune_req.lower = lower;
    tune_req.upper = upper;
    tune_req.space = space;
    tune_req.freq = freq;
    ret = ioctl(fd, FM_IOCTL_TUNE_NEW, &tune_req);
    if (ret < 0)
        perror("FM_IOCTL_TUNE_NEW failed");
    else
        printf("fm_tune_new: freq %d\n", tune_req.freq);

    return ret;
}

int fm_seek_new(int fd, int *freq, int upper, int lower, int space, int dir, int *rssi, void *para) {
    int ret = 0;
    struct fm_seek_t seek_req;

    seek_req.lower = lower;
    seek_req.upper = upper;
    seek_req.space = space;
    seek_req.freq = *freq;
    seek_req.dir = dir;
    seek_req.th = *rssi;
    ret = ioctl(fd, FM_IOCTL_SEEK_NEW, &seek_req);
    if (ret < 0) {
        perror("FM_IOCTL_SEEK_NEW failed");
        return ret;
    }

    *freq = seek_req.freq;
    *rssi = seek_req.th;
    printf("fm_seek_new: freq %d, rssi %d\n", seek_req.freq, seek_req.th);
    return ret;
}

int fm_is_fm_pwrup(int fd, int *pwrup) {
    int ret = 0;

    ret = ioctl(fd, FM_IOCTL_IS_FM_POWERED_UP, pwrup);
    if (ret < 0)
        perror("FM_IOCTL_IS_FM_POWERED_UP failed");
    else
        printf("fm_is_fm_pwrup: [pwrup=%d] [ret=%d]\n", *pwrup, ret);

    return ret;
}

int fm_fm_set_status(int fd, int which, int stat) {
    int ret = 0;
    struct fm_status_t stat_parm;

    memset(&stat_parm, 0, sizeof(struct fm_status_t));
    stat_parm.which = which;
    stat_parm.stat = stat;

    ret = ioctl(fd, FM_IOCTL_FM_SET_STATUS, &stat_parm);
    if (ret < 0)
        perror("FM_IOCTL_FM_SET_STATUS failed");
    else
        printf("fm_fm_set_status: [ret=%d]\n", ret);

    return ret;
}

int fm_fm_get_status(int fd, int which, int *stat) {
    int ret = 0;
    struct fm_status_t stat_parm;

    memset(&stat_parm, 0, sizeof(struct fm_status_t));
    stat_parm.which = which;

    ret = ioctl(fd, FM_IOCTL_FM_GET_STATUS, &stat_parm);
    if (ret < 0)
        perror("FM_IOCTL_FM_GET_STATUS failed");
    else
        printf("fm_fm_get_status: [ret=%d]\n", ret);

    *stat = stat_parm.stat;

    return ret;
}

int fm_read_rds_data(int fd, RDSData_Struct *rds, uint16_t *rds_status) {
    int ret = 0;
    uint16_t event_status;

    if (rds == NULL) {
        fprintf(stderr, "rds is NULL\n");
        return -1;
    }

    if (rds_status == NULL) {
        fprintf(stderr, "rds_status is NULL\n");
        return -1;
    }

    if (read(fd, rds, sizeof(RDSData_Struct)) == sizeof(RDSData_Struct)) {
        event_status = rds->event_status;
        printf("event_status = 0x%x\n", event_status);
        *rds_status = event_status;
        return ret;
    } else {
        fprintf(stderr, "readrds get no event\n");
        ret = -ERR_RDS_NO_DATA;
    }
    return ret;
}

int fm_sw_scan(int fd, uint16_t *scan_tbl, int *max_num, int band, int sort) {
    int ret;
    int chl_cnt = 0;
    uint16_t start_freq = FM_FREQ_MIN;
    struct fm_seek_parm parm;

    g_stopscan = 0;

    do {
        memset(&parm, 0, sizeof(struct fm_seek_parm));
        parm.band = band;
        parm.freq = start_freq;
        parm.hilo = FM_AUTO_HILO_OFF;
        parm.space = fm_get_seek_space();
        parm.seekdir = FM_SEEK_UP;
        parm.seekth = FM_SEEKTH_LEVEL_DEFAULT;

        ret = ioctl(fd, FM_IOCTL_SEEK, &parm);
        if (ret != 0) {
            perror("FM_IOCTL_SEEK failed");
            fprintf(stderr, "FM scan failed, %s, %d\n", strerror(errno), parm.err);
            break;
        }

        if ((parm.err == FM_SUCCESS) && (chl_cnt < *max_num) && (parm.freq > start_freq)) {
            scan_tbl[chl_cnt] = parm.freq;
            chl_cnt++;
        } else {
            break;
        }

        start_freq = parm.freq;
    } while (g_stopscan == 0);

    printf("FM sw scan %d station(s) found\n", chl_cnt);
    return ret;
}

int fm_stop_sw_scan() {
    g_stopscan = 1;

    return 0;
}

int fm_hw_scan_new(int fd, void **ppdst, int upper, int lower, int space, void *para) {
    int ret = 0;
    int tmp = 0;

    if (!scan_req_init_flag) {
        scan_req_init_flag = 1;
        scan_req.sr_size = 0;
        scan_req.sr.ch_rssi_buf = NULL;
    }

    if ((upper - lower) < space) {
        fprintf(stderr, "band parameter error\n");
        return -1;
    }

    tmp = ((upper - lower) / space + 1) * sizeof(struct fm_ch_rssi*);
    if (scan_req.sr_size < tmp) {
        if (scan_req.sr.ch_rssi_buf) {
            free(scan_req.sr.ch_rssi_buf);
            scan_req.sr.ch_rssi_buf = NULL;
        }
        scan_req.sr_size = tmp;
    }

    if (!scan_req.sr.ch_rssi_buf) {
        scan_req.sr.ch_rssi_buf = (struct fm_ch_rssi*)malloc(scan_req.sr_size);
        if (!scan_req.sr.ch_rssi_buf) {
            fprintf(stderr, "scan alloc memory failed\n");
            scan_req.sr_size = 0;
            return -2;
        }
    }

    scan_req.lower = lower;
    scan_req.upper = upper;
    scan_req.space = space;
    scan_req.cmd = FM_SCAN_CMD_START;
    ret = ioctl(fd, FM_IOCTL_SCAN_NEW, &scan_req);
    if (ret < 0) {
        perror("FM_IOCTL_SCAN_NEW (start) failed");
        return ret;
    }

    scan_req.cmd = FM_SCAN_CMD_GET_CH_RSSI;
    ret = ioctl(fd, FM_IOCTL_SCAN_NEW, &scan_req);
    if (ret < 0) {
        perror("FM_IOCTL_SCAN_NEW (get channel info) failed");
        return ret;
    }

    *ppdst = (void*)scan_req.sr.ch_rssi_buf;
    return scan_req.num;
}

int fm_fastget_rssi(int fd, struct fm_rssi_req *rssi_req) {
    int ret = 0;

    if (rssi_req == NULL) {
        fprintf(stderr, "rssi_req is NULL\n");
        return -1;
    }

    if (rssi_req->read_cnt <= 0)
        rssi_req->read_cnt = 1;

    ret = ioctl(fd, FM_IOCTL_SCAN_GETRSSI, rssi_req);
    if (ret < 0)
        perror("FM_IOCTL_SCAN_GETRSSI failed");
    else
        printf("fm_fastget_rssi: [ret=%d]\n", ret);

    return ret;
}

int fm_deactivate_ta(int fd, RDSData_Struct *rds, uint16_t cur_freq, uint16_t *backup_freq, uint16_t *ret_freq) {
    int ret = 0;

    if (rds == NULL) {
        fprintf(stderr, "rds is NULL\n");
        return -1;
    }

    if (backup_freq == NULL) {
        fprintf(stderr, "backup_freq is NULL\n");
        return -1;
    }

    if (ret_freq == NULL) {
        fprintf(stderr, "ret_freq is NULL\n");
        return -1;
    }

    if (rds->event_status & RDS_EVENT_TAON_OFF) {
        uint16_t rds_on = 0;
        struct fm_tune_parm parm;
        parm.band = FM_RAIDO_BAND;
        parm.freq = *backup_freq;
        parm.hilo = FM_AUTO_HILO_OFF;
        parm.space = fm_get_seek_space();

        ioctl(fd, FM_IOCTL_RDS_ONOFF, &rds_on);
        ioctl(fd, FM_IOCTL_TUNE, &parm);
        cur_freq = parm.freq;
        rds_on = 1;
        ioctl(fd, FM_IOCTL_RDS_ONOFF, &rds_on);
    }

    *ret_freq = cur_freq;
    return ret;
}

int fm_active_af(int fd, RDSData_Struct *rds, struct CUST_cfg_ds *cfg_data,
                 uint16_t orig_pi, uint16_t cur_freq, uint16_t *ret_freq) {
    int ret = 0;
    int i = 0, j = 0;
    struct fm_tune_parm parm;
    uint16_t set_freq = 0, sw_freq = 0, org_freq = 0;
    uint16_t PAMD_Value = 0, AF_PAMD_LBound = 0, AF_PAMD_HBound = 0;
    uint16_t PAMD_Level[25];
    uint16_t PI[25];
    uint16_t PAMD_DB_TBL[5] = {8, 12, 15, 18, 20};
    AF_Info af_list_backup;
    AF_Info af_list;

    if (rds == NULL) {
        fprintf(stderr, "rds is NULL\n");
        return -1;
    }
    if (cfg_data == NULL) {
        fprintf(stderr, "cfg_data is NULL\n");
        return -1;
    }

    sw_freq = cur_freq;
    org_freq = cur_freq;
    parm.band = cfg_data->band;
    parm.freq = sw_freq;
    parm.hilo = FM_AUTO_HILO_OFF;
    parm.space = fm_get_seek_space();

    if (!(rds->event_status & RDS_EVENT_AF)) {
        fprintf(stderr, "fm_active_af failed\n");
        *ret_freq = 0;
        ret = -ERR_RDS_NO_DATA;
        return ret;
    }

    memset(&af_list_backup, 0, sizeof(af_list_backup));
    memcpy(&af_list_backup, &rds->AF_Data, sizeof(AF_Info));
    memset(&af_list, 0, sizeof(af_list));

    AF_PAMD_LBound = PAMD_DB_TBL[0]; // 5dB
    AF_PAMD_HBound = PAMD_DB_TBL[1]; // 15dB
    ioctl(fd, FM_IOCTL_GETCURPAMD, &PAMD_Value);
    for (i = 0; i < 3 && (PAMD_Value < AF_PAMD_LBound); i++) {
        usleep(10 * 1000);
        ioctl(fd, FM_IOCTL_GETCURPAMD, &PAMD_Value);
        printf("check PAMD %d time(s), PAMD = %d\n", i + 1, PAMD_Value);
    }
    printf("current_freq=%d, PAMD_Value=%d, orig_pi=%d\n", cur_freq, PAMD_Value, orig_pi);

    /* Start to detect AF channels when original channel turns weak */
    if (PAMD_Value < AF_PAMD_LBound) {
        af_list_backup.AF_Num = af_list_backup.AF_Num > 25 ? 25 : af_list_backup.AF_Num;

        /* Precheck af list */
        for (i = 0, j = 0; i < af_list_backup.AF_Num; i++) {
            set_freq = af_list_backup.AF[1][i];
            if (set_freq < cfg_data->low_band || set_freq > cfg_data->high_band) {
                printf("AF[1][%d]: freq %d out of bandwidth[%d,%d], skip!\n",
                       i, af_list_backup.AF[1][i], cfg_data->low_band, cfg_data->high_band);
                continue;
            }

            /* Using fm_soft_mute_tune to query valid channel */
            if (fm_soft_mute_tune(fd, set_freq) == 0) {
                printf("af list pre-check: freq %d, valid\n", set_freq);
                af_list.AF[1][j] = af_list_backup.AF[1][i];
                j++;
                af_list.AF_Num++;
            } else {
                printf("af list pre-check: freq %d, invalid\n", set_freq);
            }
        }

        /* AF switch process */
        for (i = 0; i < af_list.AF_Num; i++) {
            set_freq = af_list.AF[1][i];
            printf("set_freq[%d] = %d, org_freq = %d\n", i, set_freq, org_freq);

            if (set_freq != org_freq) {
                /* Set mute to check every af channel */
                fm_mute(fd, 1);
                parm.freq = set_freq;
                ioctl(fd, FM_IOCTL_TUNE, &parm);
                usleep(20 * 1000);
                ioctl(fd, FM_IOCTL_GETCURPAMD, &PAMD_Level[i]);

                /* If signal is not good enough, skip */
                if (PAMD_Level[i] < AF_PAMD_HBound) {
                    printf("PAMD_Level[%d] = %d < AF_PAMD_HBound, continue\n", i, PAMD_Level[i]);
                    continue;
                }

                for (j = 0; j < 5; j++) {
                    usleep(200 * 1000);
                    if (fm_get_af_pi(fd, &PI[i])) {
                        if (j == 4)
                            fprintf(stderr, "get af pi fail\n");
                        continue;
                    } else
                        break;
                }

                if (orig_pi != PI[i]) {
                    printf("pi does not match, current pi(%04x), orig pi(%04x)\n", PI[i], orig_pi);
                    continue;
                }
                printf("next_freq=%d, PAMD_Level[%d]=%d\n", parm.freq, i, PAMD_Level[i]);
                if (PAMD_Level[i] > AF_PAMD_HBound) {
                    printf("PAMD_Level[%d] = %d > AF_PAMD_HBound, af switch\n", i, PAMD_Level[i]);
                    sw_freq = set_freq;
                    PAMD_Value = PAMD_Level[i];
                    break;
                }
            }
        }
        printf("AF decide to tune to freq: %d, PAMD_Level: %d\n", sw_freq, PAMD_Value);
        if ((PAMD_Value > AF_PAMD_HBound) && (sw_freq != 0)) {
            parm.freq = sw_freq;
            ioctl(fd, FM_IOCTL_TUNE, &parm);
            cur_freq = parm.freq;
        } else {
            parm.freq = org_freq;
            ioctl(fd, FM_IOCTL_TUNE, &parm);
            cur_freq = parm.freq;
        }
        fm_mute(fd, 0);
    } else {
        printf("RDS_EVENT_AF old freq:%d\n", org_freq);
    }
    *ret_freq = cur_freq;

    return ret;
}

int fm_active_ta(int fd, RDSData_Struct *rds, int band, uint16_t cur_freq, uint16_t *backup_freq, uint16_t *ret_freq) {
    int ret = 0;

    if (rds == NULL) {
        fprintf(stderr, "rds is NULL\n");
        return -1;
    }

    if (backup_freq == NULL) {
        fprintf(stderr, "backup_freq is NULL\n");
        return -1;
    }

    if (ret_freq == NULL) {
        fprintf(stderr, "ret_freq is NULL\n");
        return -1;
    }

    if (rds->event_status & RDS_EVENT_TAON) {
        uint16_t rds_on = 0;
        struct fm_tune_parm parm;
        uint16_t PAMD_Level[25];
        uint16_t PAMD_DB_TBL[5] = {13, 17, 21, 25, 29};
        uint16_t set_freq, sw_freq, org_freq, PAMD_Value, TA_PAMD_Threshold;
        int i = 0;

        rds_on = 0;
        ioctl(fd, FM_IOCTL_RDS_ONOFF, &rds_on);
        TA_PAMD_Threshold = PAMD_DB_TBL[2]; // 15dB
        sw_freq = cur_freq;
        org_freq = cur_freq;
        *backup_freq = org_freq;
        parm.band = band;
        parm.freq = sw_freq;
        parm.hilo = FM_AUTO_HILO_OFF;
        parm.space = fm_get_seek_space();

        ioctl(fd, FM_IOCTL_GETCURPAMD, &PAMD_Value);
        rds->AFON_Data.AF_Num = (rds->AFON_Data.AF_Num > 25) ? 25 : rds->AFON_Data.AF_Num;
        for (i = 0; i < rds->AFON_Data.AF_Num; i++) {
            set_freq = rds->AFON_Data.AF[1][i];
            printf("fm_active_ta: set_freq = 0x%02x, org_freq=0x%02x\n", set_freq, org_freq);
            if (set_freq != org_freq) {
                parm.freq = sw_freq;
                ioctl(fd, FM_IOCTL_TUNE, &parm);
                ioctl(fd, FM_IOCTL_GETCURPAMD, &PAMD_Level[i]);
                if (PAMD_Level[i] > PAMD_Value) {
                    PAMD_Value = PAMD_Level[i];
                    sw_freq = set_freq;
                }
            }
        }

        if ((PAMD_Value > TA_PAMD_Threshold) && (sw_freq != 0)) {
            rds->Switch_TP = 1;
            parm.freq = sw_freq;
            ioctl(fd, FM_IOCTL_TUNE, &parm);
            cur_freq = parm.freq;
        } else {
            parm.freq = org_freq;
            ioctl(fd, FM_IOCTL_TUNE, &parm);
            cur_freq = parm.freq;
        }
        rds_on = 1;
        ioctl(fd, FM_IOCTL_RDS_ONOFF, &rds_on);
    }

    *ret_freq = cur_freq;
    return ret;
}

int fm_hw_scan(int fd, uint16_t *scan_tbl, int *max_num, int band, int sort) {
    struct fm_scan_parm parm;
    uint16_t tmp_val = 0;
    int ret = 0;
    int step = 0;
    int chl_cnt = 0;
    int i, j;
    struct fm_ch_rssi tmp;
    struct fm_rssi_req rssi_req;

    parm.band = band;
    parm.space = fm_get_seek_space();
    parm.hilo = FM_AUTO_HILO_OFF;
    parm.freq = 0;
    parm.ScanTBLSize = sizeof(parm.ScanTBL) / sizeof(uint16_t);

    ret = ioctl(fd, FM_IOCTL_SCAN, &parm);
    if (ret) {
        perror("FM_IOCTL_SCAN failed");
        *max_num = 0;
        return ret;
    }

    memset(&rssi_req, 0, sizeof(struct fm_rssi_req));
    for (int ch_offset = 0; ch_offset < parm.ScanTBLSize; ch_offset++) {
        if (parm.ScanTBL[ch_offset] == 0) {
            continue;
        }
        for (step = 0; step < 16; step++) {
            if (parm.ScanTBL[ch_offset] & (1 << step)) {
                tmp_val = FM_FREQ_MIN + (ch_offset * 16 + step) * (parm.space);
                if (tmp_val <= FM_FREQ_MAX) {
                    rssi_req.cr[chl_cnt].freq = tmp_val;
                    chl_cnt++;
                }
            }
        }
    }

    switch (sort) {
        case FM_SCAN_SORT_NON:
            break;
        case FM_SCAN_SORT_UP:
        case FM_SCAN_SORT_DOWN:
            rssi_req.num = chl_cnt;
            rssi_req.read_cnt = 1;
            ret = ioctl(fd, FM_IOCTL_SCAN_GETRSSI, &rssi_req);
            if (ret) {
                perror("FM_IOCTL_SCAN_GETRSSI failed");
                *max_num = 0;
                return ret;
            }
            for (i = 1; i < chl_cnt; i++) {
                for (j = i; (j > 0) && ((FM_SCAN_SORT_DOWN == sort) ? (rssi_req.cr[j - 1].rssi < rssi_req.cr[j].rssi) : (rssi_req.cr[j - 1].rssi > rssi_req.cr[j].rssi)); j--) {
                    tmp.freq = rssi_req.cr[j].freq;
                    tmp.rssi = rssi_req.cr[j].rssi;
                    rssi_req.cr[j].freq = rssi_req.cr[j - 1].freq;
                    rssi_req.cr[j].rssi = rssi_req.cr[j - 1].rssi;
                    rssi_req.cr[j - 1].freq = tmp.freq;
                    rssi_req.cr[j - 1].rssi = tmp.rssi;
                }
            }
            break;
        default:
            break;
    }

    *max_num = (chl_cnt > *max_num) ? *max_num : chl_cnt;

    printf("Channel list(%d):", chl_cnt);
    for (i = 0; i < *max_num; i++) {
        scan_tbl[i] = rssi_req.cr[i].freq;
        printf("%d(%d dBm) ", (int)scan_tbl[i], rssi_req.cr[i].rssi);
    }

    return ret;
}
