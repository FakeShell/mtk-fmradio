#ifndef FMRADIO_H
#define FMRADIO_H

#include <stdint.h>
#include <sys/ioctl.h>

typedef signed char fm_s8;
typedef signed short fm_s16;
typedef signed int fm_s32;
typedef signed long long fm_s64;
typedef unsigned char fm_u8;
typedef unsigned short fm_u16;
typedef unsigned int fm_u32;
typedef unsigned long long fm_u64;
typedef enum fm_bool {
    fm_false = 0,
    fm_true  = 1
} fm_bool;

// scan sort algorithm
enum {
    FM_SCAN_SORT_NON = 0,
    FM_SCAN_SORT_UP,
    FM_SCAN_SORT_DOWN,
    FM_SCAN_SORT_MAX
};

// scan methods
enum {
    FM_SCAN_SEL_HW = 0, // select hardware scan, advantage: fast
    FM_SCAN_SEL_SW, // select software scan, advantage: more accurate
    FM_SCAN_SEL_MAX
};

#define FM_DEV "/dev/fm"

#define FMR_RSSI_TH_LONG    0x0301 // FM radio long antenna RSSI threshold(11.375dBuV)
#define FMR_RSSI_TH_SHORT   0x02E0 // FM radio short antenna RSSI threshold(-1dBuV)
#define FMR_CQI_TH          0x00E9 // FM radio Channel quality indicator threshold(0x0000~0x00FF)
#define FMR_SEEK_SPACE      1 // FM radio seek space,1:100KHZ; 2:200KHZ
#define FMR_SCAN_CH_SIZE    40 // FM radio scan max channel size
#define FMR_BAND            1 // FM radio band, 1:87.5MHz~108.0MHz; 2:76.0MHz~90.0MHz;
                              // 3:76.0MHz~108.0MHz; 4:special
#define FMR_BAND_FREQ_L     875 // FM radio special band low freq(Default 87.5MHz)
#define FMR_BAND_FREQ_H     1080 // FM radio special band high freq(Default 108.0MHz)
#define FM_SCAN_SORT_SELECT FM_SCAN_SORT_NON
#define FM_SCAN_SELECT      FM_SCAN_SEL_HW
#define FM_SCAN_SOFT_MUTE_GAIN_TH  3 // soft-mute threshold when software scan, rang: 0~3,
                                     // 0 means better audio quality but less channel
#define FM_CHIP_DESE_RSSI_TH (-102) // rang: -102 ~ -72

// errno
#define FM_SUCCESS      0
#define FM_FAILED       1
#define FM_EPARM        2
#define FM_BADSTATUS    3
#define FM_TUNE_FAILED  4
#define FM_SEEK_FAILED  5
#define FM_BUSY         6
#define FM_SCAN_FAILED  7

// band
#define FM_BAND_UNKNOWN 0
#define FM_BAND_UE      1 // US/Europe band  87.5MHz ~ 108MHz (DEFAULT)
#define FM_BAND_JAPAN   2 // Japan band      76MHz   ~ 90MHz
#define FM_BAND_JAPANW  3 // Japan wideband  76MHZ   ~ 108MHz
#define FM_BAND_SPECIAL 4 // special   band  between 76MHZ   and  108MHz
#define FM_BAND_DEFAULT FM_BAND_UE

#define FM_UE_FREQ_MIN  875
#define FM_UE_FREQ_MAX  1080
#define FM_JP_FREQ_MIN  760
#define FM_JP_FREQ_MAX  1080
#define FM_FREQ_MIN  FMR_BAND_FREQ_L
#define FM_FREQ_MAX  FMR_BAND_FREQ_H
#define FM_RAIDO_BAND FM_BAND_UE

// space
#define FM_SPACE_UNKNOWN    0
#define FM_SPACE_100K       1
#define FM_SPACE_200K       2
#define FM_SPACE_50K        5
#define FM_SPACE_DEFAULT    FM_SPACE_100K

#define FM_SEEK_SPACE FMR_SEEK_SPACE

// max scan channel num
#define FM_MAX_CHL_SIZE FMR_SCAN_CH_SIZE
// auto HiLo
#define FM_AUTO_HILO_OFF    0
#define FM_AUTO_HILO_ON     1

// seek direction
#define FM_SEEK_UP          0
#define FM_SEEK_DOWN        1

// seek threshold
#define FM_SEEKTH_LEVEL_DEFAULT 4

#define CQI_CH_NUM_MAX 255
#define CQI_CH_NUM_MIN 0

#define TX_SCAN_MAX 10
#define TX_SCAN_MIN 1

struct fm_tx_scan_parm {
    uint8_t  err;
    uint8_t  band;  // 87.6~108MHz
    uint8_t  space;
    uint8_t  hilo;
    uint16_t freq;  // start freq, if less than band min freq, then will use band min freq
    uint8_t  scandir;
    uint16_t ScanTBL[TX_SCAN_MAX];  // need no less than the chip
    uint16_t ScanTBLSize;  // IN: desired size, OUT: scan result size
};

struct fm_status_t {
    int which;
    int stat;
};

struct fm_fake_channel {
    int freq;
    int rssi_th;
    int reserve;
};

struct fm_fake_channel_t {
    int size;
    struct fm_fake_channel *chan;
};

struct CUST_cfg_ds {
    int16_t chip;
    int32_t band;
    int32_t low_band;
    int32_t high_band;
    int32_t seek_space;
    int32_t max_scan_num;
    int32_t seek_lev;
    int32_t scan_sort;
    int32_t short_ana_sup;
    int32_t rssi_th_l2;
    int32_t low_band_cust;
    int32_t high_band_cust;
    int32_t seek_space_cust;
    struct fm_fake_channel_t *fake_chan;
};

struct fm_tune_parm {
    uint8_t err;
    uint8_t band;
    uint8_t space;
    uint8_t hilo;
    uint16_t freq;
};

struct fm_seek_parm {
    uint8_t err;
    uint8_t band;
    uint8_t space;
    uint8_t hilo;
    uint8_t seekdir;
    uint8_t seekth;
    uint16_t freq;
};

struct fm_scan_parm {
    uint8_t err;
    uint8_t band;
    uint8_t space;
    uint8_t hilo;
    uint16_t freq;
    uint16_t ScanTBL[16];
    uint16_t ScanTBLSize;
};

struct fm_ch_rssi {
    uint16_t freq;
    int rssi;
};

enum fm_scan_cmd_t {
    FM_SCAN_CMD_INIT = 0,
    FM_SCAN_CMD_START,
    FM_SCAN_CMD_GET_NUM,
    FM_SCAN_CMD_GET_CH,
    FM_SCAN_CMD_GET_RSSI,
    FM_SCAN_CMD_GET_CH_RSSI,
    FM_SCAN_CMD_MAX
};

enum fmr_err_em {
    ERR_SUCCESS = 1000, // kernel error begin here
    ERR_INVALID_BUF,
    ERR_INVALID_PARA,
    ERR_STP,
    ERR_GET_MUTEX,
    ERR_FW_NORES,
    ERR_RDS_CRC,
    ERR_INVALID_FD, //  native error begin here
    ERR_UNSUPPORT_CHIP,
    ERR_LD_LIB,
    ERR_FIND_CUST_FNUC,
    ERR_UNINIT,
    ERR_NO_MORE_IDX,
    ERR_RDS_NO_DATA,
    ERR_UNSUPT_SHORTANA,
    ERR_MAX
};

enum fmr_rds_onoff {
    FMR_RDS_ON,
    FMR_RDS_OFF,
    FMR_MAX
};

typedef enum {
    FM_LONG_ANA = 0,
    FM_SHORT_ANA
} fm_antenna_type;

struct fm_scan_t {
    enum fm_scan_cmd_t cmd;
    int ret; // 0, success; else error code
    uint16_t lower; // lower band, Eg, 7600 -> 76.0Mhz
    uint16_t upper; // upper band, Eg, 10800 -> 108.0Mhz
    int space; // 5: 50KHz, 10: 100Khz, 20: 200Khz
    int num; // valid channel number
    void *priv;
    int sr_size; // scan result buffer size in bytes
    union {
        uint16_t *ch_buf; // channel buffer
        int *rssi_buf; // rssi buffer
        struct fm_ch_rssi *ch_rssi_buf; //channel and RSSI buffer
    } sr;
};

struct fm_seek_t {
    int ret; // 0, success; else error code
    uint16_t freq;
    uint16_t lower; // lower band, Eg, 7600 -> 76.0Mhz
    uint16_t upper; // upper band, Eg, 10800 -> 108.0Mhz
    int space; // 5: 50KHz, 10: 100Khz, 20: 200Khz
    int dir; // 0: up; 1: down
    int th; // seek threshold in dbm(Eg, -95dbm)
    void *priv;
};

struct fm_tune_t {
    int ret; // 0, success; else error code
    uint16_t freq;
    uint16_t lower; // lower band, Eg, 7600 -> 76.0Mhz
    uint16_t upper; // upper band, Eg, 10800 -> 108.0Mhz
    int space; // 5: 50KHz, 10: 100Khz, 20: 200Khz
    void *priv;
};

struct fm_softmute_tune_t {
    fm_s32 rssi; // RSSI of current channel
    fm_u16 freq; // current frequency
    fm_bool valid; // current channel is valid(true) or not(false)
};

struct fm_rssi_req {
    uint16_t num;
    uint16_t read_cnt;
    struct fm_ch_rssi cr[16*16];
};

struct fm_hw_info {
    int chip_id;
    int eco_ver;
    int rom_ver;
    int patch_ver;
    int reserve;
};

struct fm_search_threshold_t {
    fm_s32 th_type;// 0, RSSI. 1,desense RSSI. 2,SMG.
    fm_s32 th_val; //threshold value
    fm_s32 reserve;
};

typedef enum {
    FM_I2S_ON = 0,
    FM_I2S_OFF,
    FM_I2S_STATE_ERR
} fm_i2s_state_e;

typedef enum {
    FM_I2S_MASTER = 0,
    FM_I2S_SLAVE,
    FM_I2S_MODE_ERR
} fm_i2s_mode_e;

typedef enum {
    FM_I2S_32K = 0,
    FM_I2S_44K,
    FM_I2S_48K,
    FM_I2S_SR_ERR
} fm_i2s_sample_e;

struct fm_i2s_setting {
    int onoff;
    int mode;
    int sample;
};

typedef enum {
    FM_RX = 0,
    FM_TX = 1
} FM_PWR_T;

typedef struct fm_i2s_info {
    int status; /* 0:FM_I2S_ON, 1:FM_I2S_OFF,2:error */
    int mode; /* 0:FM_I2S_MASTER, 1:FM_I2S_SLAVE,2:error */
    int rate; /* 0:FM_I2S_32K:32000,1:FM_I2S_44K:44100,2:FM_I2S_48K:48000,3:error */
} fm_i2s_info_t;

typedef enum {
    FM_AUD_ANALOG = 0,
    FM_AUD_I2S = 1,
    FM_AUD_MRGIF = 2,
    FM_AUD_ERR
} fm_audio_path_e;

typedef enum {
    FM_I2S_PAD_CONN = 0,
    FM_I2S_PAD_IO = 1,
    FM_I2S_PAD_ERR
} fm_i2s_pad_sel_e;

typedef struct fm_audio_info {
    fm_audio_path_e aud_path;
    fm_i2s_info_t i2s_info;
    fm_i2s_pad_sel_e i2s_pad;
} fm_audio_info_t;

struct fm_cqi {
    int ch;
    int rssi;
    int reserve;
};

struct fm_cqi_req {
    uint16_t ch_num;
    int buf_size;
    char *cqi_buf;
};

typedef struct {
    int freq;
    int rssi;
} fm_desense_check_t;

typedef struct {
    uint16_t lower; // lower band, Eg, 7600 -> 76.0Mhz
    uint16_t upper; // upper band, Eg, 10800 -> 108.0Mhz
    int space; // 0x1: 50KHz, 0x2: 100Khz, 0x4: 200Khz
    int cycle; // repeat times
} fm_full_cqi_log_t;

typedef struct {
    uint8_t TP;
    uint8_t TA;
    uint8_t Music;
    uint8_t Stereo;
    uint8_t Artificial_Head;
    uint8_t Compressed;
    uint8_t Dynamic_PTY;
    uint8_t Text_AB;
    uint32_t flag_status;
} RDSFlag_Struct;

typedef struct {
    uint16_t Month;
    uint16_t Day;
    uint16_t Year;
    uint16_t Hour;
    uint16_t Minute;
    uint8_t Local_Time_offset_signbit;
    uint8_t Local_Time_offset_half_hour;
} CT_Struct;

typedef struct {
    int16_t AF_Num;
    int16_t AF[2][25]; // 100KHz
    uint8_t Addr_Cnt;
    uint8_t isMethod_A;
    uint8_t isAFNum_Get;
} AF_Info;

typedef struct {
    uint8_t PS[4][8];
    uint8_t Addr_Cnt;
} PS_Info;

typedef struct {
    uint8_t TextData[4][64];
    uint8_t GetLength;
    uint8_t isRTDisplay;
    uint8_t TextLength;
    uint8_t isTypeA;
    uint8_t BufCnt;
    uint16_t Addr_Cnt;
} RT_Info;

struct rds_raw_data {
    int dirty; // indicate if the data changed or not
    int len; // the data len form chip
    uint8_t data[148];
};

struct rds_group_cnt {
    unsigned int total;
    unsigned int groupA[16]; // RDS groupA counter
    unsigned int groupB[16]; // RDS groupB counter
};

enum rds_group_cnt_opcode {
    RDS_GROUP_CNT_READ = 0,
    RDS_GROUP_CNT_WRITE,
    RDS_GROUP_CNT_RESET,
    RDS_GROUP_CNT_MAX
};

struct rds_group_cnt_req {
    int err;
    enum rds_group_cnt_opcode op;
    struct rds_group_cnt gc;
};

typedef struct {
    CT_Struct CT;
    RDSFlag_Struct RDSFlag;
    uint16_t PI;
    uint8_t Switch_TP;
    uint8_t PTY;
    AF_Info AF_Data;
    AF_Info AFON_Data;
    uint8_t Radio_Page_Code;
    uint16_t Program_Item_Number_Code;
    uint8_t Extend_Country_Code;
    uint16_t Language_Code;
    PS_Info PS_Data;
    uint8_t PS_ON[8];
    RT_Info RT_Data;
    uint16_t event_status;
    struct rds_group_cnt gc;
} RDSData_Struct;

typedef enum {
    RDS_FLAG_IS_TP              = 0x0001, // Program is a traffic program
    RDS_FLAG_IS_TA              = 0x0002, // Program currently broadcasts a traffic ann.
    RDS_FLAG_IS_MUSIC           = 0x0004, // Program currently broadcasts music
    RDS_FLAG_IS_STEREO          = 0x0008, // Program is transmitted in stereo
    RDS_FLAG_IS_ARTIFICIAL_HEAD = 0x0010, // Program is an artificial head recording
    RDS_FLAG_IS_COMPRESSED      = 0x0020, // Program content is compressed
    RDS_FLAG_IS_DYNAMIC_PTY     = 0x0040, // Program type can change
    RDS_FLAG_TEXT_AB            = 0x0080  // If this flag changes state, a new radio text string begins
} RdsFlag;

typedef enum {
    RDS_EVENT_FLAGS          = 0x0001, // One of the RDS flags has changed state
    RDS_EVENT_PI_CODE        = 0x0002, // The program identification code has changed
    RDS_EVENT_PTY_CODE       = 0x0004, // The program type code has changed
    RDS_EVENT_PROGRAMNAME    = 0x0008, // The program name has changed
    RDS_EVENT_UTCDATETIME    = 0x0010, // A new UTC date/time is available
    RDS_EVENT_LOCDATETIME    = 0x0020, // A new local date/time is available
    RDS_EVENT_LAST_RADIOTEXT = 0x0040, // A radio text string was completed
    RDS_EVENT_AF             = 0x0080, // Current Channel RF signal strength too weak, need do AF switch
    RDS_EVENT_AF_LIST        = 0x0100, // An alternative frequency list is ready
    RDS_EVENT_AFON_LIST      = 0x0200, // An alternative frequency list is ready
    RDS_EVENT_TAON           = 0x0400,  // Other Network traffic announcement start
    RDS_EVENT_TAON_OFF       = 0x0800, // Other Network traffic announcement finished.
    RDS_EVENT_ECC_CODE       = 0x1000, // ECC code
    RDS_EVENT_RDS            = 0x2000, // RDS Interrupt had arrived durint timer period
    RDS_EVENT_NO_RDS         = 0x4000, // RDS Interrupt not arrived durint timer period
    RDS_EVENT_RDS_TIMER      = 0x8000 // Timer for RDS Bler Check. ---- BLER  block error rate
} RdsEvent;

#define FM_IOC_MAGIC        0xf5

#define FM_IOCTL_POWERUP       _IOWR(FM_IOC_MAGIC, 0, struct fm_tune_parm*)
#define FM_IOCTL_POWERDOWN     _IOWR(FM_IOC_MAGIC, 1, int32_t*)
#define FM_IOCTL_TUNE          _IOWR(FM_IOC_MAGIC, 2, struct fm_tune_parm*)
#define FM_IOCTL_SEEK          _IOWR(FM_IOC_MAGIC, 3, struct fm_seek_parm*)
#define FM_IOCTL_SETVOL        _IOWR(FM_IOC_MAGIC, 4, uint32_t*)
#define FM_IOCTL_GETVOL        _IOWR(FM_IOC_MAGIC, 5, uint32_t*)
#define FM_IOCTL_MUTE          _IOWR(FM_IOC_MAGIC, 6, uint32_t*)
#define FM_IOCTL_GETRSSI       _IOWR(FM_IOC_MAGIC, 7, int32_t*)
#define FM_IOCTL_SCAN          _IOWR(FM_IOC_MAGIC, 8, struct fm_scan_parm*)
#define FM_IOCTL_STOP_SCAN     _IO(FM_IOC_MAGIC,   9)

#define FM_IOCTL_GETCHIPID     _IOWR(FM_IOC_MAGIC, 10, uint16_t*)
#define FM_IOCTL_GETMONOSTERO  _IOWR(FM_IOC_MAGIC, 13, uint16_t*)
#define FM_IOCTL_GETCURPAMD    _IOWR(FM_IOC_MAGIC, 14, uint16_t*)
#define FM_IOCTL_GETGOODBCNT   _IOWR(FM_IOC_MAGIC, 15, uint16_t*)
#define FM_IOCTL_GETBADBNT     _IOWR(FM_IOC_MAGIC, 16, uint16_t*)
#define FM_IOCTL_GETBLERRATIO  _IOWR(FM_IOC_MAGIC, 17, uint16_t*)

/* IOCTL for RDS */
#define FM_IOCTL_RDS_ONOFF     _IOWR(FM_IOC_MAGIC, 18, uint16_t*)
#define FM_IOCTL_RDS_SUPPORT   _IOWR(FM_IOC_MAGIC, 19, int32_t*)

#define FM_IOCTL_POWERUP_TX    _IOWR(FM_IOC_MAGIC, 20, struct fm_tune_parm*)
#define FM_IOCTL_TUNE_TX       _IOWR(FM_IOC_MAGIC, 21, struct fm_tune_parm*)
#define FM_IOCTL_RDS_TX        _IOWR(FM_IOC_MAGIC, 22, struct fm_rds_tx_parm*)

#define FM_IOCTL_RDS_SIM_DATA  _IOWR(FM_IOC_MAGIC, 23, uint32_t*)
#define FM_IOCTL_IS_FM_POWERED_UP  _IOWR(FM_IOC_MAGIC, 24, uint32_t*)

/* IOCTL for FM Tx */
#define FM_IOCTL_TX_SUPPORT    _IOWR(FM_IOC_MAGIC, 25, int32_t*)
#define FM_IOCTL_RDSTX_SUPPORT _IOWR(FM_IOC_MAGIC, 26, int32_t*)
#define FM_IOCTL_RDSTX_ENABLE  _IOWR(FM_IOC_MAGIC, 27, int32_t*)
#define FM_IOCTL_TX_SCAN       _IOWR(FM_IOC_MAGIC, 28, struct fm_tx_scan_parm*)

/* IOCTL for FM over BT */
#define FM_IOCTL_OVER_BT_ENABLE  _IOWR(FM_IOC_MAGIC, 29, int32_t*)

/* IOCTL for FM ANTENNA SWITCH */
#define FM_IOCTL_ANA_SWITCH     _IOWR(FM_IOC_MAGIC, 30, int32_t*)
#define FM_IOCTL_GETCAPARRAY    _IOWR(FM_IOC_MAGIC, 31, int32_t*)

/* IOCTL for FM compensation by GPS RTC */
#define FM_IOCTL_GPS_RTC_DRIFT  _IOWR(FM_IOC_MAGIC, 32, struct fm_gps_rtc_info*)

/* IOCTL for FM I2S Setting */
#define FM_IOCTL_I2S_SETTING  _IOWR(FM_IOC_MAGIC, 33, struct fm_i2s_setting*)

#define FM_IOCTL_RDS_GROUPCNT   _IOWR(FM_IOC_MAGIC, 34, struct rds_group_cnt_req*)
#define FM_IOCTL_RDS_GET_LOG    _IOWR(FM_IOC_MAGIC, 35, struct rds_raw_data*)

#define FM_IOCTL_SCAN_GETRSSI   _IOWR(FM_IOC_MAGIC, 36, struct fm_rssi_req*)
#define FM_IOCTL_SETMONOSTERO   _IOWR(FM_IOC_MAGIC, 37, int32_t)

#define FM_IOCTL_RDS_BC_RST     _IOWR(FM_IOC_MAGIC, 38, int32_t*)
#define FM_IOCTL_CQI_GET     _IOWR(FM_IOC_MAGIC, 39, struct fm_cqi_req*)
#define FM_IOCTL_GET_HW_INFO    _IOWR(FM_IOC_MAGIC, 40, struct fm_hw_info*)
#define FM_IOCTL_GET_I2S_INFO   _IOWR(FM_IOC_MAGIC, 41, fm_i2s_info_t*)
#define FM_IOCTL_IS_DESE_CHAN   _IOWR(FM_IOC_MAGIC, 42, int32_t*)

#define FM_IOCTL_TOP_RDWR _IOWR(FM_IOC_MAGIC, 43, struct fm_top_rw_parm*)
#define FM_IOCTL_HOST_RDWR  _IOWR(FM_IOC_MAGIC, 44, struct fm_host_rw_parm*)

#define FM_IOCTL_PRE_SEARCH _IOWR(FM_IOC_MAGIC, 45, int32_t)
#define FM_IOCTL_RESTORE_SEARCH _IOWR(FM_IOC_MAGIC, 46, int32_t)

#define FM_IOCTL_SET_SEARCH_THRESHOLD   _IOWR(FM_IOC_MAGIC, 47, struct fm_search_threshold_t*)

#define FM_IOCTL_GET_AUDIO_INFO _IOWR(FM_IOC_MAGIC, 48, fm_audio_info_t*)
#define FM_IOCTL_FM_SET_STATUS _IOWR(FM_IOC_MAGIC, 49, struct fm_status_t)
#define FM_IOCTL_FM_GET_STATUS _IOWR(FM_IOC_MAGIC, 50, struct fm_status_t)

#define FM_IOCTL_SCAN_NEW       _IOWR(FM_IOC_MAGIC, 60, struct fm_scan_t*)
#define FM_IOCTL_SEEK_NEW       _IOWR(FM_IOC_MAGIC, 61, struct fm_seek_t*)
#define FM_IOCTL_TUNE_NEW       _IOWR(FM_IOC_MAGIC, 62, struct fm_tune_t*)
#define FM_IOCTL_SOFT_MUTE_TUNE _IOWR(FM_IOC_MAGIC, 63, struct fm_softmute_tune_t*)
#define FM_IOCTL_DESENSE_CHECK   _IOWR(FM_IOC_MAGIC, 64, fm_desense_check_t*)

#define COMPAT_FM_IOCTL_GET_AUDIO_INFO _IOWR(FM_IOC_MAGIC, 48, int32_t)

#define FM_IOCTL_PMIC_RDWR  _IOWR(FM_IOC_MAGIC, 65, struct fm_pmic_rw_parm*)

#define FM_IOCTL_FULL_CQI_LOG _IOWR(FM_IOC_MAGIC, 70, fm_full_cqi_log_t *)

#define FM_IOCTL_DUMP_REG   _IO(FM_IOC_MAGIC, 0xFF)

int fm_open_dev(const char *pname, int *fd);
int fm_close_dev(int fd);

int fm_powerup(int fd, int band, int freq);
int fm_powerdown(int fd, int type);
int fm_tune(int fd, int freq, int band);
int fm_seek(int fd, int *freq, int band, int dir, int lev);
int fm_setvol(int fd, int vol);
int fm_getvol(int fd, int *vol);
int fm_mute(int fd, int mute);
int fm_getrssi(int fd, int *rssi);
int fm_scan(int fd, struct fm_scan_parm *scan_parm);
int fm_stop_scan(int fd);
int fm_getchipid(int fd, int *chipid);
int fm_getcurpamd(int fd, int *pamd);
int fm_getgoodbcnt(int fd, int *goodbcnt);
int fm_getbadbnt(int fd, int *badbnt);
int fm_getbadratio(int fd, int *badratio);
int fm_rds_onoff(int fd, int onoff);
int fm_rds_support(int fd, int *support);
int fm_pre_search(int fd);
int fm_restore_search(int fd);
int fm_soft_mute_tune(int fd, int freq);
int fm_get_stereo_mono(int fd, int *stereo);
int fm_set_stereo_mono(int fd, int stereo);
int fm_get_caparray(int fd, int *caparray);
int fm_get_hw_info(int fd, struct fm_hw_info *info);
int fm_is_dese_chan(int fd, int freq);
int fm_desense_check(int fd, int freq, int rssi);
int fm_set_search_threshold(int fd, int th_idx, int th_val);
int fm_full_cqi_logger(int fd, fm_full_cqi_log_t *log_parm);
int fm_ana_switch(int fd, int antenna);
int fm_get_af_list(RDSData_Struct *rds, int16_t **af_list, int *len);
int fm_get_ps(RDSData_Struct *rds, uint8_t **ps, int *ps_len);
int fm_get_rt(RDSData_Struct *rds, uint8_t **rt, int *rt_len);
int fm_get_pi(RDSData_Struct *rds, uint16_t *pi);
int fm_get_ecc(RDSData_Struct *rds, uint8_t *ecc);
int fm_get_pty(RDSData_Struct *rds, uint8_t *pty);
int fm_tx_pwrup(int fd, int band, int freq);
int fm_tx_tune(int fd, int band, int freq);
int fm_tx_scan(int fd, int band, int start_freq, int dir, int *num, uint16_t *tbl);
int fm_is_tx_support(int fd, int *supt);
int fm_fm_over_bt(int fd, int onoff);
int fm_rdstx_onoff(int fd, int onoff);
int fm_tune_new(int fd, int freq, int upper, int lower, int space, void *para);
int fm_seek_new(int fd, int *freq, int upper, int lower, int space, int dir, int *rssi, void *para);
int fm_is_fm_pwrup(int fd, int *pwrup);
int fm_fm_set_status(int fd, int which, int stat);
int fm_fm_get_status(int fd, int which, int *stat);
int fm_read_rds_data(int fd, RDSData_Struct *rds, uint16_t *rds_status);
int fm_sw_scan(int fd, uint16_t *scan_tbl, int *max_num, int band, int sort);
int fm_stop_sw_scan();
int fm_hw_scan_new(int fd, void **ppdst, int upper, int lower, int space, void *para);
int fm_fastget_rssi(int fd, struct fm_rssi_req *rssi_req);
int fm_deactivate_ta(int fd, RDSData_Struct *rds, uint16_t cur_freq, uint16_t *backup_freq, uint16_t *ret_freq);
int fm_active_af(int fd, RDSData_Struct *rds, struct CUST_cfg_ds *cfg_data, uint16_t orig_pi, uint16_t cur_freq, uint16_t *ret_freq);
int fm_active_ta(int fd, RDSData_Struct *rds, int band, uint16_t cur_freq, uint16_t *backup_freq, uint16_t *ret_freq);
int fm_hw_scan(int fd, uint16_t *scan_tbl, int *max_num, int band, int sort);

// Helper functions
int fm_get_seek_space();
void fm_change_string(uint8_t *str, int len);

#endif // FMRADIO_H
