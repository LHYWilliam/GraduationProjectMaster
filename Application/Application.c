#include "Application.h"

LCD_t LCD = {
    .RST_Port = LCD_RST_GPIO_Port,
    .RST_Pin = LCD_RST_Pin,
    .BLK_Port = LCD_BLK_GPIO_Port,
    .BLK_Pin = LCD_BLK_Pin,
    .hDMAx = &hdma_memtomem_dma2_stream0,
    .Rotation = Rotation0,
};