/*****************************************************************************
* | File      	:	LCD_Bmp.c
* | Author      :   Waveshare team
* | Function    :	Show SDcard BMP picto LCD 
* | Info        :
*   Image scanning
*      Please use progressive scanning to generate images or fonts
*----------------
* |	This version:   V1.0
* | Date        :   2018-01-11
* | Info        :   Basic version
*
******************************************************************************/
#include "LCD_Bmp.h"
#include "DEV_Config.h"
#include "pico/stdlib.h"
#include "stdio.h"
#include "string.h"
#include "ff.h"
#include "fatfs_storage.h"

char* pDirectoryFiles[ MAX_BMP_FILES ];
uint8_t str[20];
FRESULT f_res;
FATFS microSDFatFs;
unsigned char file_name[25][11];

static const char *fr_str(FRESULT r)
{
	switch (r) {
	case FR_OK: return "FR_OK";
	case FR_DISK_ERR: return "FR_DISK_ERR";
	case FR_INT_ERR: return "FR_INT_ERR";
	case FR_NOT_READY: return "FR_NOT_READY";
	case FR_NO_FILE: return "FR_NO_FILE";
	case FR_NO_PATH: return "FR_NO_PATH";
	case FR_INVALID_NAME: return "FR_INVALID_NAME";
	case FR_DENIED: return "FR_DENIED";
	case FR_EXIST: return "FR_EXIST";
	case FR_INVALID_OBJECT: return "FR_INVALID_OBJECT";
	case FR_WRITE_PROTECTED: return "FR_WRITE_PROTECTED";
	case FR_INVALID_DRIVE: return "FR_INVALID_DRIVE";
	case FR_NOT_ENABLED: return "FR_NOT_ENABLED";
	case FR_NO_FILESYSTEM: return "FR_NO_FILESYSTEM";
	case FR_MKFS_ABORTED: return "FR_MKFS_ABORTED";
	case FR_TIMEOUT: return "FR_TIMEOUT";
	case FR_LOCKED: return "FR_LOCKED";
	case FR_NOT_ENOUGH_CORE: return "FR_NOT_ENOUGH_CORE";
	case FR_TOO_MANY_OPEN_FILES: return "FR_TOO_MANY_OPEN_FILES";
	case FR_INVALID_PARAMETER: return "FR_INVALID_PARAMETER";
	default: return "FR_UNKNOWN";
	}
}

static void print_name_bytes(const char *label, const char *name, int n)
{
	int i;
	printf("%s [", label);
	for (i = 0; i < n && name[i]; i++) {
		printf("%c", (name[i] >= 32 && name[i] < 127) ? name[i] : '.');
	}
	printf("] hex:");
	for (i = 0; i < n; i++) {
		printf(" %02X", (unsigned char)name[i]);
	}
	printf("\r\n");
}

void SD_Init(void){
	
	DEV_Digital_Write(SD_CS_PIN,1);
	DEV_Digital_Write(LCD_CS_PIN,1);
	DEV_Digital_Write(TP_CS_PIN,1);

	int counter = 0;
    //Check the mounted device
	f_res = f_mount(&microSDFatFs,(TCHAR const*)"/",1);
	if(f_res!=FR_OK){
		printf("SD card mount file system failed ,error code :(%d) %s\r\n",
		       f_res, fr_str(f_res));
	}else{
		printf("SD card mount file system success!! fs_type=%u csize=%u\r\n",
		       (unsigned)microSDFatFs.fs_type, (unsigned)microSDFatFs.csize);
		for (counter = 0; counter < MAX_BMP_FILES; counter++){
			pDirectoryFiles[counter] = file_name[counter]; 
		}	
	}
}

/********************************************************************************
function:	Display the BMP picture in the SD card
parameter: 
		Bmp_ScanDir :   Displays the LCD scanning mode of the BMP picture
		Lcd_ScanDir :   LCD normal display scan
********************************************************************************/
void LCD_Show_bmp(LCD_SCAN_DIR Lcd_ScanDir){
	uint32_t bmplen = 0x00;
    uint32_t checkstatus = 0x00;
    uint32_t filesnumbers = 0x00;
    uint32_t bmpcounter = 0x00;
    DIR directory;
    FRESULT res;
	LCD_SCAN_DIR Bmp_ScanDir = Lcd_ScanDir;
	// L2R_U2D  = 0,	//0°
    // D2U_L2R  ,      //90°
    // R2L_D2U  ,      //180°
    // U2D_R2L  ,      //270°  
	if (Lcd_ScanDir == L2R_U2D)
	{
		Bmp_ScanDir = R2L_D2U;
	}
	else if (Lcd_ScanDir == R2L_D2U)
	{
		Bmp_ScanDir = L2R_U2D;
	}
	else if (Lcd_ScanDir == D2U_L2R)
	{
		Bmp_ScanDir = U2D_R2L;
	}
	else
	{
		Bmp_ScanDir = D2U_L2R;
	}
	
    /* Open directory */
	LCD_Clear(LCD_BACKGROUND);
	printf("LCD_Show_bmp: start scan_dir=%d bmp_scan_dir=%d\r\n",
	       (int)Lcd_ScanDir, (int)Bmp_ScanDir);
    res = f_opendir(&directory, "/");
    if((res != FR_OK)){
		printf("LCD_Show_bmp: f_opendir(\"/\") failed %d %s\r\n", res, fr_str(res));
		if(res == FR_NO_FILESYSTEM){
			/* Display message: SD card not FAT formated */
			GUI_DisString_EN(0, 32, "SD_CARD_NOT_FORMATTED", &Font24,LCD_BACKGROUND,BLUE);  
		}else{
			/* Display message: Fail to open directory */
			GUI_DisString_EN(0, 48, "SD_CARD_OPEN_FAIL", &Font24,LCD_BACKGROUND,BLUE);           
		}
    }else{
		FILINFO info;
		unsigned listed = 0;
		printf("LCD_Show_bmp: f_opendir(\"/\") OK — listing all root entries\r\n");
		for (;;) {
			res = f_readdir(&directory, &info);
			if (res != FR_OK) {
				printf("LCD_Show_bmp: f_readdir failed %d %s\r\n", res, fr_str(res));
				break;
			}
			if (info.fname[0] == 0) {
				break;
			}
			listed++;
			printf("  entry %u attr=0x%02X size=%lu ",
			       listed, info.fattrib, (unsigned long)info.fsize);
			print_name_bytes("name", info.fname, (int)strlen(info.fname) + 1);
		}
		printf("LCD_Show_bmp: root entry count=%u\r\n", listed);
		f_closedir(&directory);
	}
	
    /* Get number of bitmap files */
    filesnumbers = Storage_GetDirectoryBitmapFiles ("/", pDirectoryFiles);
	printf("LCD_Show_bmp: Storage_GetDirectoryBitmapFiles count=%lu (max %d)\r\n",
	       (unsigned long)filesnumbers, MAX_BMP_FILES);
	
    /* Set bitmap counter to display first image */
    bmpcounter = 1;

	int showtime;
	if (filesnumbers == 0) {
		printf("LCD_Show_bmp: no BMP names matched filter — skipping slideshow\r\n");
	}
    for(showtime = 0; showtime < filesnumbers ; showtime++){
        sprintf((char*)str, "%-11.11s", pDirectoryFiles[bmpcounter -1]);
		print_name_bytes("LCD_Show_bmp: open candidate", (char *)str, 12);
        
        checkstatus = Storage_CheckBitmapFile((const char*)str, &bmplen);
		printf("LCD_Show_bmp: Storage_CheckBitmapFile -> %lu (0=ok 1=no card else=unsupported)\r\n",
		       (unsigned long)checkstatus);
        
        if(checkstatus == 0){
			uint32_t show_rc;
			BMP_SetGramScanWay(Bmp_ScanDir);
			show_rc = Storage_OpenReadFile(0, 0, (const char*)str);
			printf("LCD_Show_bmp: Storage_OpenReadFile -> %lu (0=not 24bpp, 1=shown or size mismatch)\r\n",
			       (unsigned long)show_rc);
        }else if (checkstatus == 1){
			printf("LCD_Show_bmp: treating as SD_CARD_NOT_FOUND\r\n");
			LCD_SetGramScanWay(Lcd_ScanDir);
			GUI_DisString_EN(0, 64, "SD_CARD_NOT_FOUND", &Font24,LCD_BACKGROUND,BLUE);
        }else {
			printf("LCD_Show_bmp: treating as SD_CARD_FILE_NOT_SUPPORTED\r\n");
			LCD_SetGramScanWay(Lcd_ScanDir);
            GUI_DisString_EN(0, 80, "SD_CARD_FILE_NOT_SUPPORTED", &Font24,LCD_BACKGROUND,BLUE);
        }

        bmpcounter ++;
        if(bmpcounter > filesnumbers){
			bmpcounter = 1;
			break;
        }

	}
	printf("LCD_Show_bmp: done\r\n");
	DEV_Digital_Write(SD_CS_PIN,1);	
}
