#include "iap.h"
#include "stmflash.h"
#include "stdio.h"
#include "string.h"

iapfun jump2app;
uint32_t g_iapbuf[1024];       /* 2K�ֽڻ��� */
extern uint8_t g_usart_tx_buf[100];
uint8_t * p;
/**
 * @brief       IAPд��APP BIN
 * @param       appxaddr : Ӧ�ó������ʼ��ַ
 * @param       appbuf   : Ӧ�ó���CODE
 * @param       appsize  : Ӧ�ó����С(�ֽ�)
 * @retval      ��
 */
 
#if		0
void iap_write_appbin(uint32_t appxaddr, uint8_t *appbuf, uint32_t appsize)
{
    uint16_t t;
    uint16_t i = 0;
    uint16_t temp;
    uint32_t fwaddr = appxaddr; /* ��ǰд��ĵ�ַ */
    uint8_t *dfu = appbuf;

    for (t = 0; t < appsize; t += 2)
    {
        temp = (uint16_t)dfu[1] << 8;
        temp |= (uint16_t)dfu[0];
        dfu += 2;               /* ƫ��2���ֽ� */
        g_iapbuf[i++] = temp;

        if (i == 1024)
        {
            i = 0;
						printf("fwaddr = 0x%x\r\n",fwaddr);
            stmflash_write(fwaddr, g_iapbuf, 1024);
						p = (uint8_t *)g_iapbuf;
						printf("д��flash�ڴ棺0x%x\r\n",fwaddr);
						for(int i = 0;i < 10;i++)
						{
							
							printf("0x%x\r\n",p[i]);
							
						}
						printf("\r\n");
            
						
						
						memset(g_usart_tx_buf,0,100);
						stmflash_read(fwaddr,(uint32_t *)g_usart_tx_buf,20);
						printf("��ȡflash�ڴ棺0x%x\r\n",fwaddr);
						for(int i = 0;i < 10;i++)
						{
							
							printf("0x%x\r\n",g_usart_tx_buf[i]);
							
						}
						printf("\r\n");
						
						fwaddr += 2048;     /* ƫ��2048  16 = 2 * 8  ����Ҫ����2 */
						
        }
    }

    if (i)
    {
        stmflash_write(fwaddr, g_iapbuf, i);  /* ������һЩ�����ֽ�д��ȥ */
    }
}

#else

void iap_write_appbin(uint32_t appxaddr, uint8_t *appbuf, uint32_t appsize)
{
    uint32_t t;
    uint16_t i = 0;
    uint32_t temp;
    uint32_t fwaddr = appxaddr; /* ��ǰд��ĵ�ַ */
    uint8_t *dfu = appbuf;

    for (t = 0; t < appsize; t += 4)
    {
        temp = (uint32_t)dfu[3] << 24;
        temp |= (uint32_t)dfu[2] << 16;
        temp |= (uint32_t)dfu[1] << 8;
        temp |= (uint32_t)dfu[0];
        dfu += 4;               /* ƫ��4���ֽ� */
        g_iapbuf[i++] = temp;

        if (i == 512)
        {
            i = 0;
            stmflash_write(fwaddr, g_iapbuf, 512);
            fwaddr += 2048;     /* ƫ��2048  32 = 4 * 8  ����Ҫ����4 */
        }
    }

    if (i)
    {
        stmflash_write(fwaddr, g_iapbuf, i);  /* ������һЩ�����ֽ�д��ȥ */
    }
}

#endif

/**
 * @brief       ��ת��Ӧ�ó����(ִ��APP)
 * @param       appxaddr : Ӧ�ó������ʼ��ַ

 * @retval      ��
 */
void iap_load_app(uint32_t appxaddr)
{
	
		uint32_t msp = 0;
		//0x08010000 ��ŵ���ջ��ָ��  �����0x2000xxxx  ��ͷ
    if (((*(volatile  uint32_t *)appxaddr) & 0x2FFE0000) == 0x20000000)     /* ���ջ����ַ�Ƿ�Ϸ�.���Է����ڲ�SRAM��64KB(0x20000000) */
    {
        /* �û��������ڶ�����Ϊ����ʼ��ַ(��λ��ַ) */
        jump2app = (iapfun) * (volatile uint32_t *)(appxaddr + 4);
				printf("jump2app = 0x%p\r\n",jump2app);
				printf("appxaddr = 0x%x\r\n",appxaddr);
				msp = *(uint32_t *)appxaddr;
				printf("msp = 0x%x\r\n",msp);
        
        /* ��ʼ��APP��ջָ��(�û��������ĵ�һ�������ڴ��ջ����ַ) */
        sys_msr_msp(*(volatile uint32_t *)appxaddr);
        
        /* ��ת��APP */
        jump2app();
    }
}












