#ifndef _MI_EPTZ_H
#define _MI_EPTZ_H

#ifdef __cplusplus
extern "C" {
#endif

//#define MI_EPTZ_DEB_EN
#define MI_EPTZ_MAX_PATH (512) /* max. length of full pathname */
#define MI_EPTZ_MAX_LINE_LEN (512) //for file parse
#define MI_STITCH_FRAME (2)

typedef void* EPTZ_DEV_HANDLE;
typedef void* LDC_BIN_HANDLE;

//=================================================================================
typedef enum _mi_eptz_err
{
    MI_EPTZ_ERR_NONE = 0x200,
    MI_EPTZ_ERR_NOT_INIT,
    MI_EPTZ_ERR_NOT_INIT_LUT,
    MI_EPTZ_ERR_NOT_INIT_WORLD2CAM,
    MI_EPTZ_ERR_NOT_INIT_OUT,
    MI_EPTZ_ERR_INVALID_PARAMETER_EPTZ,
    MI_EPTZ_ERR_INVALID_PARAMETER_DONUT,
    MI_EPTZ_ERR_INVALID_PARAMETER_ERP,
    MI_EPTZ_ERR_INVALID_PARAMETER_MAP_DOWNSAMPLE,
    MI_EPTZ_ERR_OUT_OF_MEMORY,
    MI_EPTZ_ERR_OUT_OF_MEMORY_BIN,
    MI_EPTZ_ERR_IC_CHECK,
    MI_EPTZ_ERR_PARSE_LINE_BUFFER,
    MI_EPTZ_ERR_PARSE_FILE_READ,
    MI_EPTZ_ERR_PARSE_LDC_MODE,
    MI_EPTZ_ERR_PARSE_IMAGE_SIZE,
    MI_EPTZ_ERR_PARSE_IMAGE_CENTER,
    MI_EPTZ_ERR_PARSE_GRIDE_SIZE,
    MI_EPTZ_ERR_PARSE_PAN_LIMIT,
    MI_EPTZ_ERR_PARSE_TILT_LIMIT,
    MI_EPTZ_ERR_PARSE_ZOOM_LIMIT,
    MI_EPTZ_ERR_PARSE_ROTATE_LIMIT,
    MI_EPTZ_ERR_PARSE_RADIUS_LIMIT,
    MI_EPTZ_ERR_PARSE_FILE_IN_NONE,
    MI_EPTZ_ERR_BIN_NOT_DONE,
    MI_EPTZ_ERR_BIN_NOT_READY,
    MI_EPTZ_ERR_PARSE_HW_LIMITATION,
    MI_EPTZ_ERR_PARSE_STITCH_FAIL,
    MI_EPTZ_ERR_MEM_ALLOCATE_FAIL,
    MI_EPTZ_ERR_MEM_FREE_FAIL,
    MI_EPTZ_ERR_LDC_SPLIT_ITERATIVE_FIND,
    MI_EPTZ_ERR_LDC_OVER_MAX_TILE_NUMBER,
    MI_EPTZ_ERR_LDC_BIN_OUT_WRITE,
    MI_EPTZ_ERR_LDC_BIN_READ,
    MI_EPTZ_ERR_INVALID_PARAMETER_CYLINDER,
    MI_EPTZ_ERR_INVALID_PARAMETER_STITCH,
    MI_EPTZ_ERR_FRAME_NUMBER,
    MI_EPTZ_ERR_PARSE_FOV_OUT_1P,
    MI_EPTZ_ERR_PARSE_DISPLACEMENT_MAP_SIZE,
    MI_EPTZ_ERR_PARSE_SQUARE_INFORMATION
}mi_eptz_err;
//=================================================================================
typedef enum _mi_eptz_path
{
    PATH_IN_FOLDER,
    PATH_OUT_FOLDER,
    PATH_LDC_BIN,
    PATH_SRC_IMAGE,   //file name for source image
    PATH_SRC_IMAGE_2,
    PATH_SRC_IMAGE_3,
    PATH_SRC_IMAGE_4,
    PATH_MAP_X,       //for map2bin
    PATH_MAP_Y,
    PATH_MAP_X_2,     //for stereo stitch
    PATH_MAP_Y_2,
    PATH_NUM
}mi_eptz_path;
//=================================================================================
typedef enum _mi_LDC_MODE
{
    LDC_MODE_4R_CM,     //4 EPTZ views with ceiling mount mode
    LDC_MODE_4R_WM,     //4 EPTZ views with wall mount mode
    LDC_MODE_1R,        //1 undistorted view with ceiling/desk mount mode
    LDC_MODE_2P_CM,     //2 panorama views with ceiling mount mode
    LDC_MODE_1P_CM,     //1 panorama view with ceiling mount mode
    LDC_MODE_1P_WM,     //1 panorama views with wall mount mode
    LDC_MODE_1O,        //bypass mode
    LDC_MODE_1R_WM,     //1 undistorted view with ceiling/desk mount mode
    LDC_MODE_2P_DM,     //2 panorama views with desk mount mode
    LDC_MODE_RSC,       //rolling sutter correction
    LDC_MODE_1P_DM,     //1 panorama view with desk mount mode
    LDC_MODE_CYLINDER,  //cylinder projection
    LDC_MODE_STITCH,    //stitch
    LDC_MODE_MAP2BIN,   //Convert map to ldc's bin format.
    LDC_MODE_ST_STITCH  //Stereo stitch
}mi_LDC_MODE;
//=================================================================================
#ifdef  MI_EPTZ_DEB_EN
typedef enum _mi_EPTZ_DEB
{
    MI_EPTZ_DEBUG_NONE = 0x400,
    MI_EPTZ_DEBUG_FILE_PATH_INIT,
    MI_EPTZ_DEBUG_EPTZ_LUT_INIT,
    MI_EPTZ_DEBUG_WORLD2CAM_INIT,
    MI_EPTZ_DEBUG_EPTZ_DST_INIT,
    MI_EPTZ_DEBUG_LDC_SPLIT_INIT,
    MI_EPTZ_DEBUG_EPTZ_INIT_FINISH,
    MI_EPTZ_DEBUG_EPTZ_PARAMETER_SET,
    MI_EPTZ_DEBUG_EPTZ_MAP_GEN,
    MI_EPTZ_DEBUG_EPTZ_RUNTIME_MAP_GEN_FINISH,
    MI_EPTZ_DEBUG_DONUT_PARAMETER_SET,
    MI_EPTZ_DEBUG_DONUT_MAP_GEN,
    MI_EPTZ_DEBUG_DONUT_RUNTIME_MAP_GEN_FINISH,
    MI_EPTZ_DEBUG_ERP_PARAMETER_SET,
    MI_EPTZ_DEBUG_ERP_MAP_GEN,
    MI_EPTZ_DEBUG_ERP_RUNTIME_MAP_GEN_FINISH
}mi_EPTZ_DEB;
#endif
//=================================================================================
typedef struct _point
{
    int x;
    int y;
}point;
//=================================================================================
typedef struct _mi_stitch_param
{
    int width;
    int height;
    int pan;
    int tilt;
    float zoom_h;
    float zoom_v;
    float fc;
    double matrix_buf[9];
    point b_up;
    point b_bottom;
    point i_up;
    point i_bottom;
    int fs_out_width;
    int fs_out_height;
    int s_out_width;
    int s_out_height;
    int s_out_w_tile;
    int s_out_h_tile;
    int fs_out_w_tile;
    int fs_out_h_tile;
}mi_stitch_param;
//=================================================================================
//config parse
typedef struct _mi_eptz_config_param
{
    char path_name[PATH_NUM][MI_EPTZ_MAX_PATH]; /*DO NOT EDIT*/ //-i, -o, -b
    int path_name_len[PATH_NUM];                /*DO NOT EDIT*/
    mi_LDC_MODE ldc_mode;                       /*DO NOT EDIT*/ //-m, (0)4R_CM, (1)4R_WM, (2)1R, (3)2P_CM, (4)1P_CM, (5)1P_WM, (6)1O, (7)1R_WM, (8)2P_DM, (9)RSC, (10)1P_DM,¡@(11)CYLINDER, (12)STITCH, (13)MAP2BIN
    int in_width;                               /*DO NOT EDIT*/ //-s
    int in_height;                              /*DO NOT EDIT*/
    int out_width;                              /*DO NOT EDIT*/
    int out_height;                             /*DO NOT EDIT*/
    int out_width_tile;                         /*DO NOT EDIT*/
    int out_height_tile;                        /*DO NOT EDIT*/
    int in_xc;                                  /*DO NOT EDIT*/ //-c
    int in_yc;                                  /*DO NOT EDIT*/
    int in_fisheye_radius;                      /*DO NOT EDIT*/
    int grid_size;                              /*DO NOT EDIT*/ //-d
    int pan_min;                                /*DO NOT EDIT*/ //-p
    int pan_max;                                /*DO NOT EDIT*/
    int tilt_min;                               /*DO NOT EDIT*/ //-t
    int tilt_max;                               /*DO NOT EDIT*/
    int zoom_min;                               /*DO NOT EDIT*/ //-z
    int zoom_max;                               /*DO NOT EDIT*/
    int rotate_min;                             /*DO NOT EDIT*/ //-r
    int rotate_max;                             /*DO NOT EDIT*/
    int radius_min;                             /*DO NOT EDIT*/ //-a
    int radius_max;                             /*DO NOT EDIT*/
    int hw_mode;        //-h
    int in_xc_offset;   //-co
    int in_yc_offset;
    int frame_cnt;
    mi_stitch_param s_param[MI_STITCH_FRAME];
    int fov_out_1p;     //equirectangular's out FOV, the default setting is 180 degree.
    int map_width;                              // -sm, the map size for RSC mode
    int map_height;
    int map_width_tile;
    int map_height_tile;
    int square_width;                           // -sw
    int square_height;                          // -sh
    int squar_frame_number;                     // -sf
    int square_size;                            // -ss
}mi_eptz_config_param;
//=================================================================================
//parameters
typedef struct _mi_eptz_para
{
    mi_eptz_config_param* ptconfig_para;    /*DO NOT EDIT*/
    int view_index;                         /*DO NOT EDIT*/
    int pan;                                /*DO NOT EDIT*/
    int tilt;                               /*DO NOT EDIT*/
    int rotate;                             /*DO NOT EDIT*/
    float zoom;                             /*DO NOT EDIT*/
    float zoom_h;                           /*DO NOT EDIT*/ //horizental zoom ratio, only for 1P WM mode
    float zoom_v;                           /*DO NOT EDIT*/ //vertical zoom ratio, only for 1P WM mode
    float pi;                               /*DO NOT EDIT*/
    float fc;                               /*DO NOT EDIT*/
    float fov;                              /*DO NOT EDIT*/
    int out_rot;                            /*DO NOT EDIT*/
    int r_inside;                           /*DO NOT EDIT*/
    int r_outside;                          /*DO NOT EDIT*/
    int theta_start;                        /*DO NOT EDIT*/
    int theta_end;                          /*DO NOT EDIT*/
#ifdef  MI_EPTZ_DEB_EN                      /*DO NOT EDIT*/
    mi_EPTZ_DEB deb_state;                  /*DO NOT EDIT*/
#endif                                      /*DO NOT EDIT*/
    float cx;     //the motion compensation coefficient of 3x3 shear transformation for rolling shutter correction
    float cy;
    int fov_out;    //FOV out range for WM/1P.
}mi_eptz_para;
//=================================================================================
//Information
typedef struct _stitch_info
{
    int x_info;
    int y_info;
    int r_width;   //region width
    int r_height;  //region height
}stitch_info;
//=================================================================================
//parameters
typedef struct _ldc_bin_info
{
    LDC_BIN_HANDLE ldc_bin_ptr[3];
    int ldc_bin_size[3];
    stitch_info region_info[3];
}ldc_bin_info;
//=================================================================================
/*
 * mi_eptz_config_parse
 *   Parse configure file.
 *
 *
 * Parameters:
 *  in
 *   pfile_name: configure file path
 *  out
 *   tconfig_para : parsing result
 *
 * Return:
 *   mi_eptz_err error state
 */
mi_eptz_err mi_eptz_config_parse(char* pfile_name, mi_eptz_config_param* tconfig_para);
//=================================================================================
/*
 * mi_eptz_get_buffer_info
 *   Get working buffer length.
 *
 *
 * Parameters:
 *  in
 *   aptconfig_para: Configure setting which invlove image size and other paramters.
 *
 * Return:
 *   working buffer size
 */
int  mi_eptz_get_buffer_info(mi_eptz_config_param* aptconfig_para);
//=================================================================================
/*
* mi_eptz_buffer_free
*   free ldc binary buffer which be allocated by libeptz.a
*
*
* Parameters:
*  in
*   pldc_bin: binary buffer pointer.
*
* Return:
*   mi_eptz_err error state
*/
mi_eptz_err mi_eptz_buffer_free(LDC_BIN_HANDLE pldc_bin);
//=================================================================================
/*
* mi_eptz_runtime_init
*   Generate bin file.
*
*
* Parameters:
*   in
*    pWorkingBuffer: working buffer
*    working_buf_len: working buffer size
*   out
*    apteptz_para : eptz angle parameters
*
* Return:
*   EPTZ_DEV_HANDLE eptz handle pointer
*/
EPTZ_DEV_HANDLE mi_eptz_runtime_init(unsigned char* pWorkingBuffer, int working_buf_len, mi_eptz_para* apteptz_para);
//=================================================================================
/*
 * mi_eptz_runtime_map_gen
 *   Generate bin file.
 *
 *
 * Parameters:
 *  in
 *   apteptz_handle: eptz handle be deacred by mi_eptz_runtime_init()
 *   aptmi_eptz_para : eptz parameters for user assign
 *  out
 *   pldc_bin : out binary buffer pointer.
 *   aplbin_size : out binary size
 *
 * Return:
 *   mi_eptz_err error state
 */
mi_eptz_err mi_eptz_runtime_map_gen(EPTZ_DEV_HANDLE apteptz_handle, mi_eptz_para* aptmi_eptz_para, LDC_BIN_HANDLE* pldc_bin, int* aplbin_size);
//=================================================================================
/*
* mi_donut_runtime_map_gen
*   Generate bin file.
*
*
* Parameters:
*  in
*   apteptz_handle: donut handle be deacred by mi_eptz_runtime_init()
*   aptmi_eptz_para : donut parameters for user assign
*  out
*   pldc_bin : out binary buffer pointer.
*   aplbin_size : out binary size
*
* Return:
*   mi_eptz_err error state
*/
mi_eptz_err mi_donut_runtime_map_gen(EPTZ_DEV_HANDLE apteptz_handle, mi_eptz_para* aptmi_eptz_para, LDC_BIN_HANDLE* pldc_bin, int* aplbin_size);
//=================================================================================
/*
* mi_erp_runtime_map_gen
*   Generate bin file for equirectangular projection.
*
*
* Parameters:
*  in
*   apteptz_handle: eptz handle be deacred by mi_eptz_runtime_init()
*   aptmi_erp_para : equirectangular parameters for user assign
*  out
*   pldc_bin : out binary buffer pointer.
*   aplbin_size : out binary size
*
* Return:
*   mi_eptz_err error state
*/
mi_eptz_err mi_erp_runtime_map_gen(EPTZ_DEV_HANDLE apteptz_handle, mi_eptz_para* aptmi_erp_para, LDC_BIN_HANDLE* pldc_bin, int* aplbin_size);
//=================================================================================
/*
* mi_rsc_runtime_map_gen
*   Generate bin file for rolling shutter correction.
*
*
* Parameters:
*  in
*   apteptz_handle: eptz handle be deacred by mi_eptz_runtime_init()
*   aptmi_rsc_para : rolling shutter correction parameters for user assign
*  out
*   pldc_bin : out binary buffer pointer.
*   aplbin_size : out binary size
*
* Return:
*   mi_eptz_err error state
*/
mi_eptz_err mi_rsc_runtime_map_gen(EPTZ_DEV_HANDLE apteptz_handle, mi_eptz_para* aptmi_rsc_para, LDC_BIN_HANDLE* pldc_bin, int* aplbin_size);
//=================================================================================
/*
* mi_bypass_runtime_map_gen
*   Generate bin file for by pass mode.
*
*
* Parameters:
*  in
*   apteptz_handle: eptz handle be deacred by mi_eptz_runtime_init()
*   aptmi_bypass_para : bypass parameters for user assign
*  out
*   pldc_bin : out binary buffer pointer.
*   aplbin_size : out binary size
*
* Return:
*   mi_eptz_err error state
*/
mi_eptz_err mi_bypass_runtime_map_gen(EPTZ_DEV_HANDLE apteptz_handle, mi_eptz_para* aptmi_bypass_para, LDC_BIN_HANDLE* pldc_bin, int* aplbin_size);
//=================================================================================
/*
* mi_cylinder_runtime_map_gen
*   Generate bin file for cylinder projection.
*
*
* Parameters:
*  in
*   apteptz_handle: eptz handle be deacred by mi_eptz_runtime_init()
*   aptmi_cylinder_para : cylinder parameters for user assign
*  out
*   pldc_bin : out binary buffer pointer.
*   aplbin_size : out binary size
*
* Return:
*   mi_eptz_err error state
*/
mi_eptz_err mi_cylinder_runtime_map_gen(EPTZ_DEV_HANDLE apteptz_handle, mi_eptz_para* aptmi_cylinder_para, LDC_BIN_HANDLE* pldc_bin, int* aplbin_size);
//=================================================================================
/*
* mi_stitch_runtime_map_gen
*   Generate bin file for stitch mode.
*
*
* Parameters:
*  in
*   apteptz_handle: eptz handle be deacred by mi_eptz_runtime_init()
*  out
*   ldc_bin_result : out binary buffer pointer and out binary size.
*
* Return:
*   mi_eptz_err error state
*/
mi_eptz_err mi_stitch_runtime_map_gen(EPTZ_DEV_HANDLE apteptz_handle, ldc_bin_info* ldc_bin_result);
//=================================================================================
/*
* mi_map2bin_runtime_map_gen
*    Convert displacement mapx and mapy to ldc binary format.
*
*
* Parameters:
*  in
*   apteptz_handle: eptz handle be deacred by mi_eptz_runtime_init()
*   aptmi_map2bin_para : map2bin parameters for user assign
*  out
*   pldc_bin : out binary buffer pointer.
*   aplbin_size : out binary size
*
* Return:
*   mi_eptz_err error state
*/
mi_eptz_err mi_map2bin_runtime_map_gen(EPTZ_DEV_HANDLE apteptz_handle, mi_eptz_para* aptmi_map2bin_para, LDC_BIN_HANDLE* pldc_bin, int* aplbin_size);
//=================================================================================
/*
* mi_st_stitch_runtime_map_gen
*    Convert displacement mapx and mapy to ldc binary format which is used to stereo stitch mode
*
* Parameters:
*  in
*   apteptz_handle: eptz handle be deacred by mi_eptz_runtime_init()
*  out
*   ldc_bin_result : out binary buffer pointer and out binary size.
*
* Return:
*   mi_eptz_err error state
*/
mi_eptz_err mi_st_stitch_runtime_map_gen(EPTZ_DEV_HANDLE apteptz_handle, ldc_bin_info* ldc_bin_result);
//=================================================================================
/*
* mi_stitch_buffer_free
*   free ldc binary buffer which be allocated by libeptz.a
*
*
* Parameters:
*  in
*   pldc_bin: binary buffer pointer.
*
* Return:
*   mi_eptz_err error state
*/
mi_eptz_err mi_stitch_buffer_free(ldc_bin_info ldc_bin_result);
#ifdef __cplusplus
}
#endif

#endif//#ifdef _MI_EPTZ_H
