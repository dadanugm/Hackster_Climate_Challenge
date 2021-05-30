/*
 * spi_app.c file
 *
 * Author		: dadan
 * Hackster ID	: dadanugm
 * email		: dadanugm07@gmail.com
 * Linkedin 	: https://www.linkedin.com/in/surya-ramadhan-92467736/
 * Scholar		: https://scholar.google.com/citations?user=KaE3Qg8AAAAJ&hl=en&authuser=3
 *
 * Seeking for oportunity as embedded system engineer in Germany or New Zealand
 * or as PhD Student
 *
 */

#include "spi_app.h"

SPI_HandleTypeDef qf_spi;

void spi_init(void)
{
	qf_spi.Init.ucCmdType = CMD_NoResponse;
	qf_spi.Init.ucFreq = SPI_BAUDRATE_1MHZ;
	qf_spi.Init.ulCLKPhase = SPI_PHASE_1EDGE;
	qf_spi.Init.ulCLKPolarity = SPI_POLARITY_LOW;
	qf_spi.Init.ulDataSize = SPI_DATASIZE_8BIT;
	qf_spi.Init.ucSPIInf = SPI_4_WIRE_MODE;
	qf_spi.Init.ulFirstBit = SPI_FIRSTBIT_MSB;
	qf_spi.Init.ucSSn = SPI_SLAVE_2_SELECT;
	qf_spi.ucSPIx = SPI1_MASTER_SEL;

	S3x_Clk_Enable(S3X_A1_CLK);
	S3x_Clk_Enable(S3X_CFG_DMA_A1_CLK);

	if (HAL_SPI_Init(&qf_spi) != HAL_OK){
		spi_err_handler();
	}
	else{
		dbg_str("\r\n INIT SPI OK\r\n");
	}
}


void spi_write(uint8_t data)
{
	HAL_StatusTypeDef Stat;
	HAL_SPI_Transmit(&qf_spi,&data,1,NULL);
	if (Stat != HAL_OK){
		dbg_str("spi fail to transmit\r\n");
	}
	else{
		dbg_str("spi transmit done\r\n");
	}
}

void spi_write_info(void)
{
	dbg_str("callback spi transmit\r\n");
}

void spi_read_data(uint8_t* data)
{

}

void spi_err_handler(void)
{
	dbg_str("\r\n INIT SPI FAIL\r\n");
}

void test_spi_write(uint8_t* data, int len)
{
	HAL_SPI_StateTypeDef status;
	uint8_t tmp = 'T';

	for (int i=0; i<4; i++){
	//	while ((status == HAL_SPI_STATE_TX_BUSY)||(status == HAL_SPI_STATE_RX_BUSY)){
	//	}
		spi_write(tmp);
		HAL_DelayUSec(10000);
	//		data++;
	}
}

//EOF

