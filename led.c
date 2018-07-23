#define BYTES_PER_COLOR	3
#define LED_PATTERN_N  60

const u8 LED_pattern_data[][BYTES_PER_COLOR]={
	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},
	//G,R,B
	{0,255,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},
	
	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},
	
	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},

	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},
//16
	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},
	
	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},
	
	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},

	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},
//32
	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},
	
	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},
	
	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},

	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},
//48
	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},
	
	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},
	
	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},

	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},
//64
	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},
	
	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},
	
	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},

	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},
//80
	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},
	
	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},
	
	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},

	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},
//96
	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},
	
	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},
	
	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},

	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},
//112
	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},
	
	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},
	
	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},

	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},
//128
};

#define LED_ARRAY_SIZE		(LED_PATTERN_N*BYTES_PER_COLOR*5*8/(sizeof(uint16_t)*8))

static uint16_t *LED_data;
static unsigned int LED_data_pos = 0, LED_data_end = 0;


void SPI1_IRQHandler()
{
	if(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == SET) {
		if(LED_data_pos < LED_data_end) {
			SPI_I2S_SendData(SPI1, LED_data[LED_data_pos++]);
		}
		else {
			SPI_I2S_SendData(SPI1, 0x8000);
			SPI_ITConfig(SPI1, SPI_I2S_IT_TXE, DISABLE);
		}
	}
}

static void SPI1_SendData(uint16_t hword)
{
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI1, hword);
}

static void set_bits(uint16_t *bits, int offset, int nbits)
{
	while(nbits --) {
		bits[offset/16] |= 1<<(15-(offset%16));
		offset ++;
	}
}

static int get_bit(uint16_t *bits, int offset)
{
	return (bits[offset/16] & (1<<(15-(offset%16)))) ? 1 : 0;
}

static int led_get_bits(uint16_t *bits, uint8_t *color, int n_color)
{
	int i, j, offset = 0;
	uint8_t c;
	int n_sh;

	n_sh = (n_color * BYTES_PER_COLOR * 8 * 5) / (sizeof(uint16_t) * 8);
	memset(bits, 0, n_sh * sizeof(uint16_t));
	for(j=0; j<n_color*BYTES_PER_COLOR; j++) {
		c = color[j];
		for(i=0; i<8; i++) {
			if(c & (1<<(7-i)))
				set_bits(bits, offset, 4);
			else
				set_bits(bits, offset, 1);
			offset += 5;
		}
	}
	//set_bits(bits, offset, 1);
	return offset;
}

static void  cmd_led (struct cmdline* cmd)
{
	int offset, i, j;
	static uint16_t led_spi_data[7][LED_ARRAY_SIZE];

	if(cmd->count == 2 &&  (strcmp(cmd->params[1], "init") == 0)) {
		
	}
	else if(cmd->count == 2 &&  (strcmp(cmd->params[1], "single") == 0)) {
		hal_trace("led single!!\n");
		__delay_ms(10);
		
		offset = led_get_bits(led_spi_data[0], (uint8_t*)LED_pattern_data, LED_PATTERN_N);
#if 0
		LED_data = led_spi_data[0];
		LED_data_pos = 0;
		LED_data_end = ((offset + 15) & (~15)) / (sizeof(uint16_t)*8);
		SPI_ITConfig(SPI1, SPI_I2S_IT_TXE, ENABLE);

		while(LED_data_pos < LED_data_end);
		__delay_us(100);
		SPI_I2S_SendData(SPI1, 0xF000);
#else
		SPI_DMA_init(led_spi_data[0], LED_ARRAY_SIZE);
		while(DMA_GetFlagStatus(DMA2_Stream3, DMA_FLAG_TCIF3) == RESET);
		DMA_Cmd(DMA2_Stream3, DISABLE);
		__delay_us(100);
#endif
	}
	else if(cmd->count == 2 &&  (strcmp(cmd->params[1], "black") == 0)) {
		unsigned char black[LED_PATTERN_N][3];
		hal_trace("led black!!\n");
		__delay_ms(10);

		memset(black, 0, sizeof(black));
		offset = led_get_bits(led_spi_data[0], (uint8_t*)black, LED_PATTERN_N);
#if 0
		LED_data = led_spi_data[0];
		LED_data_pos = 0;
		LED_data_end = ((offset + 15) & (~15)) / (sizeof(uint16_t)*8);
		SPI_ITConfig(SPI1, SPI_I2S_IT_TXE, ENABLE);

		while(LED_data_pos < LED_data_end);
		
#else
		SPI_DMA_init(led_spi_data[0], LED_ARRAY_SIZE);
		while(DMA_GetFlagStatus(DMA2_Stream3, DMA_FLAG_TCIF3) == RESET);
		DMA_Cmd(DMA2_Stream3, DISABLE);
		__delay_us(100);
#endif
	}
	else if((cmd->count == 2) && (strcmp(cmd->params[1], "reset") == 0)) {
		hal_trace("led reset!!\n");
		__delay_ms(10);

		LED_data = led_spi_data[0];
		memset(LED_data, 0, 8*sizeof(uint16_t));
		LED_data[0] = 0xff00;
		LED_data[7] = 0x00ff;
		LED_data_pos = 0;
		LED_data_end = 8;
		SPI_ITConfig(SPI1, SPI_I2S_IT_TXE, ENABLE);
	}
	else if((cmd->count == 2) && (strcmp(cmd->params[1], "run") == 0)) {
		unsigned char single_led[LED_PATTERN_N][3];
		unsigned char colors[7][3] = {
				{0,255,0}, {255,0,0},{0,0,255},{0,255,255},{255,0,255},{255,255,0},{255,255,255},
		};
		
		hal_trace("led run!!\n");
		__delay_ms(10);

		
		while(1) {
			for(i=0; i<7; i++) {
				for(j=0; j<60; j++) {
					memset(single_led, 0, sizeof(single_led));
					single_led[j][0] = colors[i][0];
					single_led[j][1] = colors[i][1];
					single_led[j][2] = colors[i][2];
					led_get_bits(led_spi_data[j&1], (uint8_t*)single_led, LED_PATTERN_N);
					
					SPI_DMA_init(led_spi_data[j&1], LED_ARRAY_SIZE);
					while(DMA_GetFlagStatus(DMA2_Stream3, DMA_FLAG_TCIF3) == RESET);
					DMA_Cmd(DMA2_Stream3, DISABLE);

					__delay_ms(20);
				}
			}
		}
	}
}