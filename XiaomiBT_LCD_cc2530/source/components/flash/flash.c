//            flash_page_read(0,15,0,fbuf,4);//bank,page,offset b0p2 0x1000
//            flash_page_erase(0,15);
//            flash_page_write(0,15,0,fbuf,4);//bank page off

#include "hal_int.h"
#include "hal_mcu.h"            // Using halMcuWaitUs()
#include "flash.h"
#include <hal_board.h>

#include "util.h"               // Using min()
#include "string.h"
/**************************** Flash defines **************/
#define FLASH_SIZE ((uint32)(256 * 1024))
#define FLASH_BANK_SIZE ((uint32)(32 * 1024))
#define FLASH_PAGE_SIZE ((uint32)(2 * 1024))
#define FLASH_WORD_SIZE ((uint32)(4))
#define FLASH_BANK_PER_FLASH ((uint32)(256 / 32))
#define FLASH_PAGE_PER_BANK ((uint32)(32 / 2))
#define FLASH_BASE (0x8000)
#define FLASH_BANK_BASE(b) ((b) * FLASH_BANK_SIZE)
#define FLASH_PAGE_BASE(p) (FLASH_BASE + (p) * FLASH_PAGE_SIZE)
#define FLASH_PAGE(p) ((uint8*)FLASH_PAGE_BASE((p)))
#define FLASH_PAGE_SEQ_NUM(b, p) (((b) * FLASH_PAGE_PER_BANK) + (p))
#define FLASH_PAGE_ADDR(b, p) (((b) * FLASH_BANK_SIZE) + ((p) * FLASH_PAGE_SIZE))

typedef struct dma_desc_s
{
     uint8 src_addr_h;
     uint8 src_addr_l;
     uint8 dst_addr_h;
     uint8 dst_addr_l;
     uint8 len_h:5;
     uint8 len_v:3;
     uint8 len_l;
     uint8 trig:5;
     uint8 tmode:2;
     uint8 word_size:1;
     uint8 priority:2;
     uint8 m8:1;
     uint8 irq_mask:1;
     uint8 dst_inc:2;
     uint8 src_inc:2;
} dma_desc_t;
/**
 * Reads from the selected flash page of the selected flash bank.
 * bank - the zero-based flash bank number
 * page - the zero-based flash page number
 * buf - the buffer to read to
 * len - the number of bytes to read
 * offset - the offset in a page
 */
void flash_page_read(uint8 bank, uint8 page,  uint16 offset, void *buf,   uint16 len)
{
    uint16 tmp = len;
    uint8 *buf_rd = FLASH_PAGE(page) + offset,
    *buf_wr = (uint8*)buf;
    uint8 memctr = MEMCTR;
    MEMCTR = (MEMCTR & 0xf8) | bank;
    while(tmp--)
        *(buf_wr++) = *(buf_rd++);
    MEMCTR = memctr;
}
void flash_page_erase(uint8 bank, uint8 page)
{
    halIntOff();
    while(FCTL & 0x80);

    FADDRH = FLASH_PAGE_SEQ_NUM(bank, page) << 1;
    FCTL |= 0x01;

    while(FCTL & (0x80));
    halIntOn();
}
void flash_page_write(uint8 bank, uint8 page, uint16 offset, void *buf, uint16 len)
{
    uint32 flash_addr = (FLASH_PAGE_ADDR(bank, page) + offset) / FLASH_WORD_SIZE;
    dma_desc_t dma = 
    {
       .src_addr_h = ((uint16)buf >> 8) & 0x00ff,
       .src_addr_l = (uint16)buf & 0x00ff,
       .dst_addr_h = (((uint16) & FWDATA) >> 8) & 0x00ff,
       .dst_addr_l = ((uint16) & FWDATA) & 0x00ff,
       .len_v      = 0,  /* Use LEN for transfer count */
       .word_size  = 0,  /* Transfer a byte at a time. */
       .tmode      = 0,  /* Transfer a single byte/word after each DMA trigger. */
       .trig       = 18, /* Flash data write complete. */
       .src_inc    = 1,  /* Increment source pointer by 1 bytes/words after each transfer. */
       .dst_inc    = 0,  /* Increment destination pointer by 0 bytes/words after each transfer. */
       .irq_mask   = 0,  /* Disable interrupt generation. */
       .m8         = 0,  /* Use all 8 bits for transfer count. */
       .priority   = 2   /* High, DMA has priority. */
    };
    dma.len_h = (len >> 8) & 0x00ff;
    dma.len_l = len & 0x00ff;

    halIntOff();
    while(FCTL & 0x80);

    FADDRH = (flash_addr >> 8) & 0x00ff;
    FADDRL = (flash_addr) & 0x00ff;
    DMA0CFGH = (((uint16)&dma) >> 8) & 0x00ff;
    DMA0CFGL = ((uint16)&dma) & 0x00ff;
    DMAARM |= 0x01;
    FCTL |= 0x02; 

    while(!(DMAIRQ & 0x01)); //Wait Until Write Complete
    DMAIRQ &= 0xfe; //Clear Any DMA IRQ on Channel 0 - Bit 0

    while(FCTL & (0x80));
    halIntOn();
}