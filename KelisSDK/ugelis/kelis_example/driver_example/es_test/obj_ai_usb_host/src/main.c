#include <camera/dcmi.h>
#include <display/fb_display.h>
#include <kernel.h>
#include <stdio.h>
#include <string.h>

#include "ai_common.h"
#include "tinyyolo_main.h"
#include "math.h"
#include "lcd.h"
#include "camera.h"
#include "draw.h"
#include <mem_cfg.h>
#include <gm_hal_mmc.h>
#include <gpio.h>

/*jpg encoder*/
#include <jinclude.h>
#include <jcapi.h>
#include "test_jpg.h"

#include <stdio.h>
#include <stdlib.h>


extern unsigned char RecvBuffer[];
extern unsigned char CMD_BASE[];
extern unsigned char CMD_BASE_BODY[];
extern unsigned char CMD_BASE_GEST[];
extern unsigned char WEI_BASE[];

extern unsigned char UNREG_DUMP_BUFF[];
extern unsigned char REG_DUMP_BUFF[];

extern unsigned short inputImage[];
extern unsigned short OutputImage[];
extern unsigned char MAGFLG_FOR_MODELLOADED[];

#define MAGFLG_DATA 0x5a5aa5a5


#define UGELIS_NEW_VERSION 1

extern struct device *dcmi;
extern struct device *ltdc;

ObjDetData obj_det_data;

node_t *base_head = NULL;
node_t *previousNode = NULL;


#define MAX_PERSON_CNT 20
#define MAX_CMP_CNT 5
#define ONE_PERSON_PICS_CNT 5
#define BUF_SIZE (MAX_PERSON_CNT*ONE_PERSON_PICS_CNT)
#define CAM_PIC_SIZE  (224 * 224 * 3)

#define OBJ_PRINTF printk
//#define OBJ_PRINTF(...)  do{} while(0);

//#define OBJ_DBG_PRINTF printk
#define OBJ_DBG_PRINTF do{} while(0);

uint8_t camvedio_read_done = 0;
uint8_t thread_write_done = 0;

extern uint32_t show_buf_addr;

static int index[BUF_SIZE];

#define MAX_OBJECTCLASS_CNT 20
#define MAX_GESTURES_CNT 2

static char *object_name[MAX_OBJECTCLASS_CNT] =
{
    "aeroplane",
    "bicyle",
    "bird",
    "boat",
    "bottle",
    "bus",
    "car",
    "cat",
    "chair",
    "cow",
    "diningtable",
    "dog",
    "horse",
    "motorbike",
    "person",
    "pottedplant",
    "sheep",
    "sofa",
    "train",
    "tvmonitor"
};

static char *gesture_name[MAX_GESTURES_CNT] =
{
    "fist",
    "palm"
};

static uint32_t object_colordrawn[MAX_OBJECTCLASS_CNT] =
{
    0xFFFF0000,
    0xFF00FF00,
    0xFFFF00FF,
    0xFFFF0040,
    0xFF00FF40,
    0xFFFF0040,
    0xFFFF0080,
    0xFF00FF80,
    0xFFFF0080,
    0xFFFF00C0,
    0xFF00FFC0,
    0xFFFF00C0,
    0xFFFF00FF,
    0xFF00FFFF,
    0xFFFF00FF,
    0xFFFF40FF,
    0xFF40FFFF,
    0xFFFF40FF,
    0xFFFF80FF,
    0xFF80FFFF,
};



/* change this to enable pull-up/pull-down */
#define PULL_UP 0

/* Change this if you have an LED connected to a custom port */
#define PORT     "GPIOB"

#define LED_GPIO_PIN   10   /*PC10*/
#define LED1_GPIO_PIN  11   /*PC11*/
#define SW0_GPIO_PIN   3    /*PB03*/

static int cnt = 0;

struct device *dev;
static struct gpio_callback gpio_cb;


void button_pressed(struct device *gpiob, struct gpio_callback *cb,
                    uint32_t pins)
{
    cnt++;

    if (cnt % 2)
    {
        obj_det_data.det_type_cpy = DET_GEST;
        printf("Swith to Gesture Detection \n");
    }
    else
    {
        obj_det_data.det_type_cpy = DET_BODY;
        printf("Swith to Body Detection \n");
    }
}

void rrggbb_split2rgb(uint8_t *src, uint8_t *dst, int w, int h);

/*Feed rrggbb format to KDP310 for further training */
void rgb_split2rrggbb(uint8_t *src, uint8_t *dst, int w, int h)
{
    uint8_t *dst_b = &(dst[0]);
    uint8_t *dst_g = &(dst[w * h]);
    uint8_t *dst_r = &(dst[w * h * 2]);

    int c = 0, l = 0, i = 0;
    uint8_t *src_rgb = 0;
    for (l = 0; l < h; l++)
    {
        for (c = 0; c < w; c++)
        {
            src_rgb = src + (l * w + c) * 3;
            *(dst_b + i) = *(src_rgb + 2);    //b
            *(dst_g + i) = *(src_rgb + 1);    //g
            *(dst_r + i) = *(src_rgb);        //r
            i++;
        }
    }
}

uint8_t lcd_inited = 0;

/*Called every frame data captured by Camera*/
void camera_isr(struct device *dev, uint32_t line_num)
{
    struct dcmi_buf dis_buf;
    int ret = 0;

    if (!obj_det_data.stop_disp)
    {
        ret = dcmi_ioctl(dcmi, VIDIOC_DQBUF, &dis_buf);

        /*Camera can display Real-Time, we initialize LCD to fix
          snowflake sreen issue when display the first image frame*/
        if (!lcd_inited)
        {
            lcd_config(dis_buf.addr);
            lcd_inited = 1;
        }
        /*Set the address for LCD display, Layer 0*/
        if (!obj_det_data.show_demo_pic)
        {
            if (ret == 0)
            {
                if (!obj_det_data.one_frame_detect_started)
                {
                    /*Captured Picture data*/
                    memcpy(obj_det_data.ex_buf.cam_buf_detected, (void *)dis_buf.addr, CAM_PIC_SIZE);
                    obj_det_data.one_frame_detect_started = 1;
                }
            }

            fb_ioctl(ltdc, 0, FB_X_SETADDR, &(dis_buf.addr));
        }
        /*Release the buffer for next camera capturing*/
        dcmi_ioctl(dcmi, VIDIOC_QBUF, &dis_buf);
    }
}

#ifndef UGELIS_NEW_VERSION
    #define STACKSIZE                4096
    char __noinit __stack stack_area[STACKSIZE];
#else
    #define STACKSIZE                4096

    K_THREAD_STACK_DEFINE(stack_area, STACKSIZE);

    static struct k_thread task_thread;
#endif


static void cambuf_process_thread()
{
    uint32_t demoaddr = 0;

    while (1)
    {
        if (obj_det_data.show_demo_pic)
        {
            demoaddr = (CONFIG_SDRAM_SIZE - CONFIG_KERNEL_MEM_RESVER_SIZE) * 1024 + 0xC0000000;  //0x10400000
            if (obj_det_data.show_demopic_onetime)
            {
                rrggbb_split2rgb((uint8_t *)RecvBuffer, (uint8_t *)demoaddr, 224, 224);
                fb_ioctl(ltdc, 0, FB_X_SETADDR, &demoaddr);
                obj_det_data.show_demopic_onetime = 0;
                k_sem_give(&obj_det_data.validate_lock);
            }
        }
        else
        {
            if (obj_det_data.one_frame_detect_started)
            {
                /*cam_buf_validate now has the KDP 310 format*/
                rgb_split2rrggbb(obj_det_data.ex_buf.cam_buf_detected, obj_det_data.ex_buf.cam_buf_validate, 224, 224);
                k_sem_give(&obj_det_data.validate_lock);
                obj_det_data.one_frame_detect_started = 0;
            }
        }

        k_yield();
    }
}




void init_ai_data()
{
    obj_det_data.ex_buf.cam_buf_detected = malloc_ext(0, 224 * 224 * 3); //malloc
    obj_det_data.ex_buf.cam_buf_validate = malloc_ext(0, TEST_BUFFER_SIZE);//malloc

    printf("cam_buf_detected : 0x%08x cam_buf_validate : 0x%08x \n", obj_det_data.ex_buf.cam_buf_detected, obj_det_data.ex_buf.cam_buf_validate);

    obj_det_data.ex_buf.cam_buf_validate[TEST_BUFFER_SIZE - 1] = 0; /*Specified for CNN, Last*/
    obj_det_data.ex_buf.cam_buf_validate[TEST_BUFFER_SIZE - 2] = 0; /*Specified for CNN, Index*/


    obj_det_data.ex_buf.cam_buf_tmp = malloc_ext(0, 224 * 224 * 4);//malloc     /*A, R, G ,B, it's layer 1, blending on layer 2*/

    printf("cam_buf_tmp : 0x%08x \n", obj_det_data.ex_buf.cam_buf_tmp);

    memset((void *)obj_det_data.ex_buf.cam_buf_tmp, 0x00, 224 * 224 * 4);
    obj_det_data.input_data_addr = obj_det_data.ex_buf.cam_buf_validate;

    obj_det_data.boxcnt = 0;
    obj_det_data.object_name = object_name;
    obj_det_data.stop_disp = 0;                 /*Stop display on LCD*/
    obj_det_data.threshold = 0.6;
    obj_det_data.one_frame_detect_started = 0;  /*Start detect one frame*/

    obj_det_data.show_demo_pic = 0;
    obj_det_data.show_demopic_onetime = 0;
    obj_det_data.cap_show = 1; //0

    obj_det_data.object_name = object_name;

    obj_det_data.unreg_imgcnt = 0;
    obj_det_data.reg_imgcnt = 0;
    obj_det_data.det_type = DET_BODY;      // DET_GEST
    obj_det_data.det_type_cpy = DET_BODY;  // DET_GEST

    obj_det_data.pic_saved = 0;            /*save picsture option*/

    obj_det_data.unregdumpdata = (uint32_t)UNREG_DUMP_BUFF + IMG_COUNT_BYTES;
    obj_det_data.regdumpdata = (uint32_t)REG_DUMP_BUFF + IMG_COUNT_BYTES;

    k_sem_init(&obj_det_data.validate_lock, 0, 1); //UINT_MAX
}


void exit()
{

}

extern uint32_t show_buf_addr;
//extern uint32_t pingpongbuf1;
//extern uint32_t pingpongbuf0;

void rrggbb_split2rgb(uint8_t *src, uint8_t *dst, int w, int h)
{
    int c = 0, l = 0, i = 0;
    uint8_t *src_rgb = src;
    uint8_t *dst_rgb = dst;

    for (i = 0; i < w * h; i ++)
    {
        *dst_rgb = *(src_rgb +  w * h * 2 + i);
        dst_rgb ++;
        *dst_rgb = *(src_rgb + w * h + i);
        dst_rgb ++;
        *dst_rgb = *(src_rgb + i);
        dst_rgb ++;
    }
}

int init()
{
    init_ai_data();
    init_shell(&obj_det_data);
    camera_config(camera_isr);
    printk("Init camera Done \n");

    //lcd_config();
    //printk("Init camera \n");

    return 0;
}

#define BLACK         0x000000
#define BLUE          0x0000FF

//extern unsigned short OutputImage[BUFFERSIZE] ;
detection_type gdet_type = DET_NOTDEF;

#define TEST_BLOCK_NUM              (30)
#define TEST_BLOCK_START            (0)
#define TEST_DATA                   (0xa5)
#define TEST_DATA_SIZE              (512*TEST_BLOCK_NUM)

static uint8_t gbuf[TEST_DATA_SIZE] = {0};
MMC_Handle_T ghMMC;

static uint32_t rec_buff_length;

//unsigned char JPG_enc_buf[7168];  /*buffer to save the encoded jpg pic*/

unsigned char JPG_enc_buf[7168 * 2 * 2]; /*buffer to save the encoded jpg pic*/

jpeg_compress_info info1;

unsigned int pt_buf = 0;

//JQUANT_TBL  JQUANT_TBL_2[2];
JQUANT_TBL  JQUANT_TBL_2[4];

//JHUFF_TBL  JHUFF_TBL_4[4];
JHUFF_TBL  JHUFF_TBL_4[8];

//unsigned char dcttab[3][512];

//volatile unsigned char inbuf_buf[50176];


unsigned char dcttab[3 * 2 * 2][512 * 2 * 2];

volatile unsigned char inbuf_buf[50176 * 2 * 2];

//#define DEMO_USE_SD_MMC  1


int main(void)
{
    uint32_t i = 0;
    uint32_t j = 0;
    uint32_t k = 0;
    uint32_t kk = 0;
    uint32_t t_last, t_now;
    float box_x_min, box_y_min, box_x_max, box_y_max;
    char str[10];

    RET_CODE ret = RET_OK;
    uint32_t blk_cnt = TEST_BLOCK_NUM;
    uint32_t blk_start = TEST_BLOCK_START;
    static uint32_t cnt = 0;
    uint8_t *buf = gbuf;
    MMC_Config_T config;
    uint64_t totalSector = 0;
    MMC_Handle_T *hMMC = &ghMMC;
    struct device *gpiob;

    /*test jpg encoder*/
    int width, height;
    JSAMPLE *image;
    jpeg_compress_info *cinfo;

    /*Reset All IP*/
    //*(volatile uint32_t *)(0x40000070) = 0xFFFFFFFF;
    rec_buff_length = IMG_DUMP_PIXELS;

    *(volatile uint32_t *)REG_DUMP_BUFF = 0;
    *(volatile uint32_t *)UNREG_DUMP_BUFF = 0;

    memset(REG_DUMP_BUFF, 0x00, 4);
    memset(UNREG_DUMP_BUFF, 0x00, 4);

    memset(inputImage, 0x00, 401408);
    memset(OutputImage, 0x00, 401408);

    *(volatile uint32_t *)MAGFLG_FOR_MODELLOADED = 0x00000000;

    /*enable a stable CPU clock*/
    *(volatile uint32_t *)(0x40000090) |= 2 << 0;
    /*power on kdp310 and reset*/
    *(volatile uint32_t *)(0x1FFFF0B0) &= ~(1 << 18);
    *(volatile uint32_t *)(0x40000070) |= 1 << 14 | 1 << 15 | 1 << 16 | 1 << 17;
    *(volatile uint32_t *)(0x40000080) |= 1 << 14 | 1 << 15 | 1 << 16 | 1 << 17;

    /*improve system powerformance*/
    *(volatile uint32_t *)(0x40039288) |= 0x3F << 16;
    *(volatile uint32_t *)(0x4000018C) |= 3;
    *(volatile uint32_t *)(0x4000009c) |= 2 << 16;

    /*print the buffer address*/
    OBJ_PRINTF("inputImage: 0x%08x \n", inputImage);
    OBJ_PRINTF("OutputImage: 0x%08x \n", OutputImage);

    OBJ_PRINTF("CMD_BASE: 0x%08x \n", CMD_BASE);
    OBJ_PRINTF("CMD_BASE_BODY: 0x%08x  \n", CMD_BASE_BODY);
    OBJ_PRINTF("CMD_BASE_GEST: 0x%08x \n", CMD_BASE_GEST);

    OBJ_PRINTF("WEI_BASE: 0x%08x \n", WEI_BASE);

    OBJ_PRINTF("RecvBuffer: 0x%08x \n", RecvBuffer);

    OBJ_PRINTF("UNREG_DUMP_BUFF: 0x%08x \n", UNREG_DUMP_BUFF);
    OBJ_PRINTF("REG_DUMP_BUFF: 0x%08x \n", REG_DUMP_BUFF);

    OBJ_PRINTF("CMD_BASE: 0x%08x, WEI_BASE: 0x%08x, RecvBuffer: 0x%08x \n",  CMD_BASE, WEI_BASE, RecvBuffer);

    /*Init for Object AI*/
    init();

    /*Initial the SRAM specified for KDP310*/
    for (i = 0; i < 0x40000; i += 4)
    {
        *(volatile uint32_t *)(0x20020000 + i) = 0x00000000;
    }

    extern int usb_main(void);
    usb_main();

    OBJ_PRINTF("Wait USB-MSD Plug-in... \n");
    /*waiting for loading model*/
    while (*(volatile uint32_t *)MAGFLG_FOR_MODELLOADED != MAGFLG_DATA)
    {
        k_sleep(1000);
    }
    OBJ_PRINTF("USB-MSD Mounted ... \n");

#ifndef UGELIS_NEW_VERSION
    k_thread_spawn(stack_area, STACKSIZE, cambuf_process_thread, NULL, NULL, NULL,
                   K_PRIO_PREEMPT(1), 0, K_NO_WAIT);
#else
    k_thread_create(&task_thread, stack_area,
                    K_THREAD_STACK_SIZEOF(stack_area),
                    (k_thread_entry_t)cambuf_process_thread,  NULL, NULL, NULL,
                    K_PRIO_PREEMPT(1), 0, K_NO_WAIT);
#endif

    while (1)
    {
        OBJ_DBG_PRINTF("==== \n");
        k_sem_take(&obj_det_data.validate_lock, 3000); //K_FOREVER
        OBJ_DBG_PRINTF("**** \n");

#ifdef CNN_TIME_TEST
        extern uint8_t gGetdata;
        extern uint8_t gInputdata;
        extern uint8_t gPutdata;
        extern uint8_t gMaxPool;
        extern uint32_t gGetdataTime_ms;
        extern uint32_t gPutdata_ms;
        extern uint32_t gMaxPool_ms;

        gGetdata = 0;
        gInputdata = 0;
        gPutdata = 0;
        gMaxPool = 0;
        gGetdataTime_ms = 0;
        gPutdata_ms = 0;
        gMaxPool_ms = 0;
#endif

        extern void suspend_usbref_thread(void);
        extern void resume_usbref_thread(void);

        if (gdet_type != obj_det_data.det_type_cpy)
        {
            OBJ_PRINTF("Load AI Model %s ... \n", obj_det_data.det_type_cpy == 0 ? "Body" : "Gesture");
            if (obj_det_data.det_type_cpy == DET_BODY)
            {
                extern void load_ai_model(uint8_t type);
                /*load the specified model to allocated address space*/
                load_ai_model(obj_det_data.det_type_cpy);

                obj_det_data.det_type = DET_BODY;
            }

            if (obj_det_data.det_type_cpy == DET_GEST)
            {
                extern void load_ai_model(uint8_t type);

                /*load the specified model to allocated address space*/
                load_ai_model(obj_det_data.det_type_cpy);

                obj_det_data.det_type = DET_GEST;
            }

            gdet_type = obj_det_data.det_type_cpy;
            OBJ_PRINTF("AI Model Loaded Done !\n");
        }

        /*in case of stack overflow, it's better to spawn a thread for tiny YOLO*/
        t_last = k_uptime_get_32();

        /*debug use*/
        //      obj_det_data.show_demo_pic = 1;

        if (obj_det_data.show_demo_pic)
        {
            /*for CNN computation*/
            call_cnn(RecvBuffer);
        }
        else
        {
            call_cnn(obj_det_data.ex_buf.cam_buf_validate);
        }

#ifdef CNN_TIME_TEST
        t_last = k_uptime_get_32();
#endif
        PostYolo();
        t_now = k_uptime_get_32();
        OBJ_DBG_PRINTF("t_now- t_last = %d ms \n", t_now - t_last);
        OBJ_DBG_PRINTF("%d Box found\n", obj_det_data.boxcnt);

#ifdef CNN_TIME_TEST
        OBJ_PRINTF("Getdata: %d, PutData:%d, MaxPool:%d ms \n", gGetdataTime_ms, gPutdata_ms, gMaxPool_ms);
#endif

        /*Layer 1*/
        LCD_clear();

        for (i = 0; i < obj_det_data.boxcnt; i ++)
        {
            OBJ_DBG_PRINTF("class: %d\n\r", obj_det_data.box[i].class);

            if (obj_det_data.det_type == DET_BODY)
            {
                OBJ_DBG_PRINTF("class name: person\n\r");
            }

#if defined(TINY_YOLO_FOR_OBJT)
            OBJ_DBG_PRINTF("class name: %s\n\r", object_name[obj_det_data.box[i].class]);
#endif

            if (obj_det_data.det_type == DET_GEST)
            {
                OBJ_DBG_PRINTF("class name: %s\n\r", gesture_name[obj_det_data.box[i].class]);
            }

            OBJ_DBG_PRINTF("score: %lf\n\r", obj_det_data.box[i].score);
            box_y_min = obj_det_data.box[i].box[0] < 0 ? 0 : obj_det_data.box[i].box[0];
            box_x_min = obj_det_data.box[i].box[1] < 0 ? 0 : obj_det_data.box[i].box[1];
            box_y_max = obj_det_data.box[i].box[2] < 0 ? 0 : obj_det_data.box[i].box[2];
            box_x_max = obj_det_data.box[i].box[3] < 0 ? 0 : obj_det_data.box[i].box[3];

            OBJ_DBG_PRINTF("box_y_min: %lf\n\r", box_y_min);
            OBJ_DBG_PRINTF("box_x_min: %lf\n\r", box_x_min);
            OBJ_DBG_PRINTF("box_y_max: %lf\n\r", box_y_max);
            OBJ_DBG_PRINTF("box_x_max: %lf\n\r", box_x_max);

            if (box_x_max > 223)
            {
                box_x_max = 223;
            }

            if (box_y_max > 223)
            {
                box_y_max = 223;
            }

            if (!obj_det_data.cap_show)
            {
                /*This will draw rectangle on Layer 1*/
                LCD_draw_rectangle(box_x_min, box_y_min, box_x_max, box_y_max, object_colordrawn[obj_det_data.box[i].class]);
            }
            else
            {
                if (i == 0)
                {
                    if (obj_det_data.show_demo_pic)
                    {
                        rrggbb_split2rgb((uint8_t *)RecvBuffer, (uint8_t *)obj_det_data.ex_buf.cam_buf_tmp, 224, 224);
                    }
                    else
                    {
                        rrggbb_split2rgb((uint8_t *)obj_det_data.ex_buf.cam_buf_validate, (uint8_t *)obj_det_data.ex_buf.cam_buf_tmp, 224, 224);
                    }
                }

                /*This will show the detected-object on layer 1*/
                LCD_draw_detobjects((uint8_t *)obj_det_data.ex_buf.cam_buf_tmp, box_x_min, box_y_min, box_x_max - box_x_min, box_y_max - box_y_min);
                /*Then draw a color*/
                LCD_draw_rectangle(box_x_min, box_y_min, box_x_max, box_y_max, object_colordrawn[obj_det_data.box[i].class]);
            }

            /*Set the address for LCD display, Layer 1*/
            //fb_ioctl(ltdc, 1, FB_X_SETADDR, &show_buf_addr);
        }

        if (obj_det_data.pic_saved == 0)
        {
            if (obj_det_data.boxcnt == 0) /*record unrecognized pics*/
            {
                sprintf(str, "MS: %d", j + 1);
                LCD_showstring(0, 0, 1000, 24, 24, str, object_colordrawn[0]);

                OBJ_DBG_PRINTF("Ignored the Image-Detection: %d \n", j + 1);
                OBJ_DBG_PRINTF("Recognized pics: %d \n", *(volatile uint32_t *)REG_DUMP_BUFF);
                /*Copy data to dump buffer*/

                memcpy(obj_det_data.unregdumpdata[j % IMG_DUMP_COUNT].vadrgb, (uint8_t *)obj_det_data.ex_buf.cam_buf_validate, IMG_VAD_PIXELS_COUNT_BYTES); //reserved 2 bytes
                rrggbb_split2rgb((uint8_t *)obj_det_data.ex_buf.cam_buf_validate, (uint8_t *)obj_det_data.ex_buf.cam_buf_tmp, 224, 224);
                memcpy(obj_det_data.unregdumpdata[j % IMG_DUMP_COUNT].camrgb, (uint8_t *)obj_det_data.ex_buf.cam_buf_tmp, IMG_CAM_PIXELS_COUNT_BYTES);

                obj_det_data.unreg_imgcnt ++;
                if (obj_det_data.unreg_imgcnt > IMG_DUMP_COUNT)
                {
                    obj_det_data.unreg_imgcnt = IMG_DUMP_COUNT;
                }
                memcpy(UNREG_DUMP_BUFF, (void *)&obj_det_data.unreg_imgcnt, sizeof(uint32_t));
                j ++;
            }
            else /*record recognized pics*/
            {
                /*Copy data to dump buffer*/
                memcpy(obj_det_data.regdumpdata[k % IMG_DUMP_COUNT].vadrgb, (uint8_t *)obj_det_data.ex_buf.cam_buf_validate, IMG_VAD_PIXELS_COUNT_BYTES); //reserved 2 bytes
                rrggbb_split2rgb((uint8_t *)obj_det_data.ex_buf.cam_buf_validate, (uint8_t *)obj_det_data.ex_buf.cam_buf_tmp, 224, 224);

                for (i = 0; i < obj_det_data.boxcnt; i ++)
                {
                    box_y_min = obj_det_data.box[i].box[0] < 0 ? 0 : obj_det_data.box[i].box[0];
                    box_x_min = obj_det_data.box[i].box[1] < 0 ? 0 : obj_det_data.box[i].box[1];
                    box_y_max = obj_det_data.box[i].box[2] < 0 ? 0 : obj_det_data.box[i].box[2];
                    box_x_max = obj_det_data.box[i].box[3] < 0 ? 0 : obj_det_data.box[i].box[3];

                    if (box_x_max > 223)
                    {
                        box_x_max = 223;
                    }

                    if (box_y_max > 223)
                    {
                        box_y_max = 223;
                    }

                    LCD_draw_rectangle2orgpic((uint8_t *)obj_det_data.ex_buf.cam_buf_tmp, box_x_min, box_y_min, box_x_max, box_y_max, object_colordrawn[obj_det_data.box[i].class]);
                }
                memcpy(obj_det_data.regdumpdata[k % IMG_DUMP_COUNT].camrgb, (uint8_t *)obj_det_data.ex_buf.cam_buf_tmp, IMG_CAM_PIXELS_COUNT_BYTES);

                obj_det_data.reg_imgcnt ++;
                if (obj_det_data.reg_imgcnt > IMG_DUMP_COUNT)
                {
                    obj_det_data.reg_imgcnt = IMG_DUMP_COUNT;
                }
                memcpy(REG_DUMP_BUFF, (void *)&obj_det_data.reg_imgcnt, sizeof(uint32_t));

                OBJ_DBG_PRINTF("Recognized pics: %d \n", *(volatile uint32_t *)REG_DUMP_BUFF);
                k ++;
            }
        }
        else
        {
            /*stop here, allow tester to dump data*/
            __asm("bkpt 0");
        }

        /*Set the address for LCD display, Layer 1, after rectangle & object are drawn*/
        fb_ioctl(ltdc, 1, FB_X_SETADDR, &show_buf_addr);

        /*==============jpg encoder=============*/
        thread_write_done = 0;

        if (camvedio_read_done == 1)
        {
            if (obj_det_data.boxcnt == 0)
            {
                image = (uint8_t *)obj_det_data.unregdumpdata[j % IMG_DUMP_COUNT].camrgb;//(uint8_t *)obj_det_data.ex_buf.cam_buf_tmp;
            }
            else
            {
                image = (uint8_t *)obj_det_data.regdumpdata[k % IMG_DUMP_COUNT].camrgb;
            }

            pt_buf = 0;

            cinfo = jpeg_create_compress();
            if (!cinfo)
            {
                printf("error in create cinfo, malloc faild!\n");
            }

            width = 224;
            height = 224;

            cinfo->image_width = width;
            cinfo->image_height = height;
            cinfo->output = JPG_enc_buf;
            jpeg_set_default(cinfo);

            jpeg_start_compress(cinfo);

            while (cinfo->next_line < cinfo->image_height)
            {
                jpeg_write_scanline(cinfo, &image[(cinfo->next_line * cinfo->image_width * 3)]);
            }

            jpeg_finish_compress(cinfo);

#if 0
            for (i = 0; i < 7168; i++)
            {
                g_UsbDeviceVideoMjpegData[i] = JPG_enc_buf[i];
            }
#endif
            thread_write_done = 1;
        }


        /*=====================================*/

        OBJ_DBG_PRINTF("Done!\n");
    }
    exit();
}

