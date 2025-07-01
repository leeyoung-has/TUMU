#include "sensor.h"
#include <stdlib.h>
#include <string.h>
#include "stdio.h"
#include "delay.h"
#include "math.h"

/* һ��һ���ݲɼ�ָ��:AA BB 02 40 00 00 C1 2A */
/* һ�Զ����ݲɼ�ָ��:FA 42 F1 73 8E 22 00 00 36 FC*/
/*��ص����ɼ�ָ��:01 03 01 00 00 01 85 F6*/


void Send_Predefined_CMD(const uint8_t *cmd_ptr, uint16_t cmd_len) {
	  GPIO_SetBits(GPIOD, GPIO_Pin_10);//���뷢��ģʽ
	  delay_us(2);
	  //printf("[TX] Sending Command (%d bytes): ", cmd_len);
    for(uint16_t i=0; i<cmd_len; i++) {
			  //printf("%02X ", cmd_ptr[i]);
        USART_SendData(USART3, cmd_ptr[i]);       // ���͵��ֽ�
        while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET); // �ȴ��������
    }
		while (!USART_GetFlagStatus(USART3, USART_FLAG_TC));
		delay_us(200);
		GPIO_ResetBits(GPIOD, GPIO_Pin_10);//���ؽ���ģʽ
		printf("\n");
		
}

void USART3_get_Data(uint8_t *temp_buf, uint16_t *data_len) {
    if (data_len) *data_len = 0; // ��ʼ������

    if (!usart3_dev.data_ready) {
        return; // ����δ������ֱ�ӷ���
    }

    // ������Ч����
    uint16_t valid_len = 0;
    if (usart3_dev.head >= usart3_dev.tail) {
        valid_len = usart3_dev.head - usart3_dev.tail;
    } else {
        valid_len = USART3_RX_BUFFER_SIZE - usart3_dev.tail + usart3_dev.head;
    }

    // �����ȷ��ظ�������
    if (data_len) {
        *data_len = valid_len;
    }
        printf("[RX] Buffer Status: head=%d, tail=%d, valid_len=%d\n", 
        usart3_dev.head, usart3_dev.tail, valid_len);
    // ����ⲿ��������Ч,��������
    if (temp_buf && valid_len > 0) {
        uint16_t bytes_copied = 0;
			 printf("[RX] Copying Data (Hex): ");
        while (bytes_copied < valid_len) {
            temp_buf[bytes_copied] = usart3_dev.buffer[usart3_dev.tail];
            usart3_dev.tail = (usart3_dev.tail + 1) % USART3_RX_BUFFER_SIZE;
					  printf("%02X ", temp_buf[bytes_copied]);
            bytes_copied++;
        }
        usart3_dev.data_ready = 0; // ��������ȡ�������־λ
    }
}

		
//����һ��һ����		
ChannelData* parse_sensor_data(uint8_t *buffer, uint16_t data_len, uint8_t *out_channel_count) {
	if (buffer[0] != 0xCC || buffer[1] != 0xDD) {
		printf("[PARSER] ERROR! Invalid Header: %02X %02X\n", buffer[0], buffer[1]);
        return 0; // У��ʧ��
    }

		//��λ��ͨ����
    uint8_t *ptr = buffer + 34;
    uint8_t channel_count = *ptr++;
    *out_channel_count = channel_count;
    printf("[PARSER] Found %d channels\n", *out_channel_count);
    // ��̬�����ڴ�
    ChannelData *channels = (ChannelData*)malloc(channel_count * sizeof(ChannelData));
    if (!channels) return 0;

		//��ȡ����
    for (int i = 0; i < channel_count; i++) {
        channels[i].channel = *ptr++;
        memcpy(&channels[i].data, ptr, 4);
        ptr += 4;
        memcpy(channels[i].unit, ptr, 8);
        channels[i].unit[8] = '\0';
        ptr += 8;

        // ����λ�ַ���
        for (int j = 0; j < 8; j++) {
            if (channels[i].unit[j] == 0) {
                channels[i].unit[j] = '\0';
                break;
            }
        }
    }

    return channels;
}

float Get_Channel2_Data(void) {
    const uint8_t CMD_GETDATA[] = {0xAA, 0xBB, 0x02, 0x40, 0x00, 0x00, 0xC1, 0x2A};
    
    // ѭ������MAX_RETRY��
    for (int retry = 0; retry < MAX_RETRY; retry++) {
        // 1. ���Ͳɼ�ָ��
        Send_Predefined_CMD(CMD_GETDATA, sizeof(CMD_GETDATA));
        delay_ms(2000);
        // 2. �ȴ����ݾ���
        uint32_t wait_cycles = TIMEOUT_MS / POLL_INTERVAL;
        while (wait_cycles--) {
            if (usart3_dev.data_ready) break;    // ��⵽���ݾ���
            delay_ms(POLL_INTERVAL);             // ��ʱ��������
        }
        
        // 3. ������Ч����
        if (usart3_dev.data_ready) {
            // ����ԭʼ����
            uint8_t temp_buf[256];
            uint16_t data_len = 0;
            USART3_get_Data(temp_buf, &data_len);
            
            // ����ͨ������
            uint8_t channel_count = 0;
            ChannelData *channels = parse_sensor_data(temp_buf, data_len, &channel_count);
            if (!channels) continue;  // ����ʧ������
            
            // ����ͨ��2������
            for (int i = 0; i < channel_count; i++) {
                if (channels[i].channel == 2) {
                    float Fi = channels[i].data;
									printf("Fi:%f",Fi);
								  	float force = (powf(Fi,2) - powf(F0,2)) * K;
                    free(channels);
                    return force;     // �ɹ��򷵻���Чֵ
                }
            }
            free(channels);          // δ�ҵ��ͷ��ڴ�
        }
        
        // 4. ����ǰ��ʱ
        delay_ms(100);
    }
    
    return -1.0f;  // ȫ������ʧ��1
}

// �ֽ���ת������
static int32_t swap_int32(int32_t value) {
    uint8_t bytes[4];
    memcpy(bytes, &value, 4);
    return (int32_t)(
        (bytes[0] << 24) |
        (bytes[1] << 16) |
        (bytes[2] << 8)  |
        bytes[3]
    );
}


//һ�Զ����ݽ�������
SensorStatus ParseSensorData(const uint8_t *raw_data, uint16_t data_len,
                           AngleData *angles, uint8_t *valid_points) 
{
    // Э��ͷУ��
    if(raw_data[0] != 0xFA) return SENSOR_ERR_HEADER;
    if(raw_data[5] != 0x22) return SENSOR_ERR_CMD;

    // CRCУ��
    uint16_t crc = 0xFFFF;
    for(int i=0; i<data_len-2; i++){
        crc ^= raw_data[i];
        for(int j=0; j<8; j++){
            if(crc & 0x0001) 
                crc = (crc >> 1) ^ 0xA001;
            else 
                crc >>= 1;
        }
    }
    uint16_t recv_crc = (raw_data[data_len-1] << 8) | raw_data[data_len-2];
    if(crc != recv_crc) return SENSOR_ERR_CRC;

    // ���ݶδ���
    uint16_t payload_len = (raw_data[6] << 8) | raw_data[7];
    const uint8_t *payload = raw_data+8+4;
    
    // ��ȡ�Ƕȵ�(����ԭ��)
    *valid_points = 0;
    const uint8_t *ptr = payload + 12; // ����12�ֽ�ԭ��
    uint16_t remain = payload_len - 12;
    
    while(remain >= 12 && *valid_points < 8) {
       // ���ת�����ֽ���
        int32_t x, y, z;
        memcpy(&x, ptr, 4);       // X��
        memcpy(&y, ptr + 4, 4);   // Y��
        memcpy(&z, ptr + 8, 4);   // Z��

        // �ֽ���ת��
        x = swap_int32(x);
        y = swap_int32(y);
        z = swap_int32(z);

        // ��λת��
        angles[*valid_points].roll  = x * 0.001f;
        angles[*valid_points].pitch = y * 0.001f;
        angles[*valid_points].yaw   = z * 0.001f;

        ptr += 12;
        remain -= 12;
        (*valid_points)++;
    }
    
    return (*valid_points > 0) ? SENSOR_OK : SENSOR_ERR_NODATA;
}

//һ�Զ����ݻ�ȡ����
SensorStatus GetAngleData(AngleData *angles, uint8_t *valid_points) 
{
    const uint8_t angle_cmd[] = {0xFA,0x42,0xF1,0x73,0x8E,0x22,0x00,0x00,0x36,0xFC};
    uint8_t rx_buf[256];
    // ѭ������MAX_RETRY��
    for (int retry = 0; retry < MAX_RETRY; retry++) {
        // 1. ���Ͳɼ�ָ��
        Send_Predefined_CMD(angle_cmd, sizeof(angle_cmd));
        delay_ms(1000);
        // 2. �ȴ����ݾ���
        uint32_t wait_cycles = TIMEOUT_MS / POLL_INTERVAL;
        while (wait_cycles--) {
            if (usart3_dev.data_ready) break;    // ��⵽���ݾ���
            delay_ms(POLL_INTERVAL);             // ��ʱ��������
        }
				
				    //���ݾ���
            if(usart3_dev.data_ready) {
                uint16_t data_len = 0;
                USART3_get_Data(rx_buf, &data_len);
                
                // ���ý�������
                SensorStatus status = ParseSensorData(rx_buf, data_len, angles, valid_points);
                if(status == SENSOR_OK) 
                    return SENSOR_OK;
                else 
                    break; //����ʧ����������
							}
            delay_ms(100);
				}
    return SENSOR_ERR_TIMEOUT;
}

SensorStatus ParseModbusFrame(const uint8_t *data, uint16_t len, uint8_t *soc) 
{
    // Э��ͷУ��
    if(len < 7) return SENSOR_ERR_NODATA;
    if(data[0] != 0x01) return SENSOR_ERR_HEADER;    // �ӻ���ַ
    if(data[1] != 0x03) return SENSOR_ERR_CMD;      // ������
    
    // CRCУ��
    uint16_t crc = 0xFFFF;
    for(int i=0; i<len-2; i++){
        crc ^= data[i];
        for(int j=0; j<8; j++){
            crc = (crc & 0x0001) ? ((crc >> 1) ^ 0xA001) : (crc >> 1);
        }
    }
    uint16_t recv_crc = (data[len-1] << 8) | data[len-2];
    if(crc != recv_crc) return SENSOR_ERR_CRC;
    
    // ������Ч��
    if(data[2] != 0x02 || len != 7) 
        return SENSOR_ERR_NODATA;
    
    // ����ٷֱ�
    *soc = (uint8_t)( (data[3] << 8) | data[4] );

    return SENSOR_OK;
}

SensorStatus GetBatterySOC(uint8_t *soc) 
{
    const uint8_t cmd[] = {0x01,0x03,0x01,0x00,0x00,0x01,0x85,0xF6}; // Modbus??
    uint8_t rx_buf[256];
    
    for (int retry = 0; retry < MAX_RETRY; retry++) 
    {
        // ����ָ��
        Send_Predefined_CMD(cmd, sizeof(cmd));
        
        // �ȴ���ʱ
        uint32_t wait_cycles = TIMEOUT_MS / POLL_INTERVAL;
        while (wait_cycles--) 
        {
            if (usart3_dev.data_ready) 
            {
                uint16_t data_len = 0;
                USART3_get_Data(rx_buf, &data_len);
                
                // ���ý�������
                const SensorStatus status = ParseModbusFrame(rx_buf, data_len, soc);
                
                // �������
                if(status == SENSOR_OK) {
                    return SENSOR_OK; // �ɹ�ֱ�ӷ���
                } else if(status == SENSOR_ERR_CRC || status == SENSOR_ERR_TIMEOUT) {
                    break; 
                } else {
                    return status; 
                }
            }
            delay_ms(100); 
        }
    }
    return SENSOR_ERR_TIMEOUT; // �������Դ���
}
