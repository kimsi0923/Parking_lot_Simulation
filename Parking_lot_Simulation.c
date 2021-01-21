#include <stdio.h>
#include <apps/shell/tash.h>
#include <fcntl.h>
#include <unistd.h>
#include <tinyara/gpio.h>
#include <tinyara/analog/adc.h>
#include <tinyara/analog/ioctl.h>
#include <tinyara/pwm.h>

#ifdef CONFIG_BUILD_KERNEL
int main(void)
#else
void final_main(void)
#endif
{
	int gpio_trig = 53;
	int gpio_echo = 54;
	int gpio_snd = 55;
	int seat_red= 39;
	int seat_green = 38;
	int grn = 30, ylw = 31, red = 32;	//각각 gpio핀 번호들을 변수로 저장

	char databuf[2], devpath1[16], devpath2[16], devpath3[16];
	char devpath4[16], devpath5[16], devpath6[16];
	char devpath7[16], devpath8[16];	//데이터 버퍼와 gpio 경로를 저장하기 위한 문자 배열 선언

	snprintf(devpath1, 16, "/dev/gpio%d", gpio_trig);
	snprintf(devpath2, 16, "/dev/gpio%d", gpio_echo);
	snprintf(devpath3, 16, "/dev/gpio%d", gpio_snd);
	snprintf(devpath4, 16, "/dev/gpio%d", seat_red);
	snprintf(devpath5, 16, "/dev/gpio%d", seat_green);
	snprintf(devpath6, 16, "/dev/gpio%d", grn);
	snprintf(devpath7, 16, "/dev/gpio%d", ylw);
	snprintf(devpath8, 16, "/dev/gpio%d", red);	//gpio핀 번호에 따라 각각의 경로를 devpath에 저장

	int fd_trig, fd_echo, wsize;
	int fd_snd, fd_adc0, fd_pwm0;
	int fd_seat_red, fd_seat_green;
	int fd_grn, fd_ylw, fd_red;	//각각의 파일 기술자와 데이터의 크기를 저장하기 위한 변수 선언

	struct pwm_info_s pwm_info;		//pwm과 관련된 구조체

	fd_trig = open(devpath1, O_RDWR);
	ioctl(fd_trig, GPIOIOC_SET_DIRECTION, GPIO_DIRECTION_OUT);	
	//초음파 센서 trig사용을 위한 gpio 파일 오픈, OUT으로 방향 설정
	fd_echo = open(devpath2, O_RDWR);
	ioctl(fd_echo, GPIOIOC_SET_DIRECTION, GPIO_DIRECTION_IN);	
	//초음파 센서 echo 사용을 위한 gpio 파일 오픈, IN으로 방향 설정
	fd_snd = open(devpath3, O_RDWR);
	ioctl(fd_snd, GPIOIOC_SET_DIRECTION, GPIO_DIRECTION_OUT);	
	//스피커 사용을 위한 gpio 파일 오픈, OUT으로 방향 설정
	fd_seat_red = open(devpath4, O_RDWR);
	ioctl(fd_seat_red, GPIOIOC_SET_DIRECTION, GPIO_DIRECTION_OUT);	
	//주차 자리 여부 LED(빨강) 사용을 위한 gpio 파일 오픈, OUT으로 방향 설정
	fd_seat_green = open(devpath5, O_RDWR);
	ioctl(fd_seat_green, GPIOIOC_SET_DIRECTION, GPIO_DIRECTION_OUT);	
	//주차 자리 여부 LED(초록) 사용을 위한 gpio 파일 오픈, OUT으로 방향 설정
	fd_grn = open(devpath6, O_RDWR);
	ioctl(fd_grn, GPIOIOC_SET_DIRECTION, GPIO_DIRECTION_OUT);	
	//주차장 진입 정도를 표현하는 LED(초록) 사용을 위한 gpio 파일 오픈, OUT으로 방향 설정
	fd_ylw = open(devpath7, O_RDWR);
	ioctl(fd_ylw, GPIOIOC_SET_DIRECTION, GPIO_DIRECTION_OUT);	
	//주차장 진입 정도를 표현하는 LED(노랑) 사용을 위한 gpio 파일 오픈, OUT으로 방향 설정
	fd_red = open(devpath8, O_RDWR);
	ioctl(fd_red, GPIOIOC_SET_DIRECTION, GPIO_DIRECTION_OUT);	
	//주차장 진입 정도를 표현하는 LED(빨강) 사용을 위한 gpio 파일 오픈, OUT으로 방향 설정
	fd_adc0 = open("/dev/adc0", O_RDONLY);	//adc사용을 위한 파일 오픈 ( 조도 센서에 이용 )

	pwm_info.duty = 0;	//pwm 듀티 값을 0
	int ret0, i = 0;		//adc와 관련된 변수, i는 주차가 되었음을 카운트하기 위한 변수
	double usec_count, distance;	//초음파 센서 거리 계산을 위한 시간, 거리를 저장하는 변수
	struct adc_msg_s adc0_data;	//adc와 관련된 구조체
	size_t readsize0;
	ssize_t nbytes0;		//사이즈를 저장하기 위한 변수들

	for(;;)	//무한반복
	{
		ret0 = ioctl(fd_adc0, ANIOC_TRIGGER, 0);

		readsize0 = sizeof(adc0_data);
		nbytes0 = read(fd_adc0, &adc0_data, readsize0);	
		//fd_adc0가 가리키는 파일에서 adc0_data라는 구조체에 데이터를 저장

		if (adc0_data.am_channel == 0)	//채널이 0일 때
		{
			if (adc0_data.am_data > 500)	//빛의 양이 설정한 기준보다 더 들어오면
			{
				lseek(fd_seat_red, 0, SEEK_SET);
				wsize = snprintf(databuf, sizeof(databuf), "%d", 0);
				write(fd_seat_red, databuf, wsize);  //주차 여부 LED빨강색은 0으로 씀 (OFF)

				lseek(fd_seat_green, 0, SEEK_SET);
				wsize = snprintf(databuf, sizeof(databuf), "%d", 1);
				write(fd_seat_green, databuf, wsize);  //주차 여부 LED초록색은 1로 씀 (ON)

				wsize = snprintf(databuf, sizeof(databuf), "%d", 0);
				lseek(fd_trig, 0, SEEK_SET);
				write(fd_trig, databuf, wsize);	//trig를 0으로 씀
				up_udelay(2);	//아주 약간의 시간 딜레이

				wsize = snprintf(databuf, sizeof(databuf), "%d", 1);
				lseek(fd_trig, 0, SEEK_SET);
				write(fd_trig, databuf, wsize);   //trig를 1로씀 (거리를 재기위한 초음파를 쏨 )
				up_udelay(10);

				wsize = snprintf(databuf, sizeof(databuf), "%d", 0);
				lseek(fd_trig, 0, SEEK_SET);
				write(fd_trig, databuf, wsize);	//trig를 0으로 씀

				while(1)
				{
					lseek(fd_echo, 0, SEEK_SET);
					read(fd_echo, databuf, 2);

					if(databuf[0] == '1')
						break;
				}	//초음파를 받을 때 까지 ( 데이터가 들어올 때 까지 ) 무한반복

				while(1)
				{
					up_udelay(1);
					usec_count += 1.0;

					lseek(fd_echo, 0, SEEK_SET);
					read(fd_echo, databuf, 2);

					usec_count += 5.0;

					if(databuf[0] == '0')
						break;
				}	
			//들어오던 초음파가 끊길 때 까지 ( 데이터가 들어오지 않을 때 까지 ) 무한반복

				distance = (331.5 + (0.60714 * 25.0)) * (usec_count / 2.0) / 10000.0;	
				//임의의 거리 계산 ( 진입 정도의 LED 구역에 맞춤 )

				i = 0;	//카운트를 0으로 설정
				usleep(1000000);
				close(fd_pwm0);		//pwm 파일 닫기 
//( 열지도 않았는데 닫는 이유는 LED 노란색 구역에서 부터 open되는데 초음파가 거리를 받아오기 전에 빠르게 구역을 빠져나오게 되면 pwm이 닫히지 못하고 계속 열려있어 스피커가 꺼지지 않음 )

				if(distance <= 3.0)	//LED 초록색 구역에 진입 했을 때
				{
					lseek(fd_grn, 0, SEEK_SET);
					wsize = snprintf(databuf, sizeof(databuf), "%d", 1);
					write(fd_grn, databuf, wsize);	// LED 초록불 켜짐

					close(fd_pwm0);
		//pwm 파일 닫기 
		//( LED노란색 구역에서 빠져나오면 스피커를 끄기 위해 파일을 닫음 )

					if(distance <= 2.0)	//LED 노란색 구역에 진입 했을 때
					{
						lseek(fd_ylw, 0, SEEK_SET);
						wsize = snprintf(databuf, sizeof(databuf), "%d", 1);
						write(fd_ylw, databuf, wsize);	//LED 노란불 켜짐

						close(fd_pwm0);	
		//pwm 파일 닫기 
		//( LED빨간색 구역에서 빠져나오면 삐- 소리가 나던 스피커를 다시 reset하기 위해 일단 닫음 )

						fd_pwm0 = open("/dev/pwm0", O_RDWR);	//pwm 열기
						usleep(100000);		//이 시간 동안 스피커 울림
						close(fd_pwm0);
					//pwm 파일 닫기, 삐-소리가 계속나는게 아닌 가벼운 경고음이 나온다.

						if(distance <= 1.0)	//LED 빨간색 구역에 진입 했을 때
						{
							lseek(fd_red, 0, SEEK_SET);
							wsize = snprintf(databuf, sizeof(databuf), "%d", 1);
							write(fd_red, databuf, wsize);	//LED 빨간불 켜짐

							fd_pwm0 = open("/dev/pwm0", O_RDWR);								//pwm 파일 열기, 삐-소리가 난다.
						}
						else	//빨간색 구역이 아닌 경우
						{
							lseek(fd_red, 0, SEEK_SET);
							wsize = snprintf(databuf, sizeof(databuf), "%d", 0);
							write(fd_red, databuf, wsize);	//LED 빨간불 끔
						}
					}
					else	//노란색 구역이 아닌 경우
					{
						lseek(fd_red, 0, SEEK_SET);
						wsize = snprintf(databuf, sizeof(databuf), "%d", 0);
						write(fd_red, databuf, wsize);	//LED 빨간불 끔

						lseek(fd_ylw, 0, SEEK_SET);
						wsize = snprintf(databuf, sizeof(databuf), "%d", 0);
						write(fd_ylw, databuf, wsize);	//LED 노란불 끔
					}
				}
				else	//초록색 구역이 아닌 경우
				{
					lseek(fd_red, 0, SEEK_SET);
					wsize = snprintf(databuf, sizeof(databuf), "%d", 0);
					write(fd_red, databuf, wsize);	//LED 빨간불 끔

					lseek(fd_ylw, 0, SEEK_SET);
					wsize = snprintf(databuf, sizeof(databuf), "%d", 0);
					write(fd_ylw, databuf, wsize);	//LED 노란불 끔

					lseek(fd_grn, 0, SEEK_SET);
					wsize = snprintf(databuf, sizeof(databuf), "%d", 0);
					write(fd_grn, databuf, wsize);	//LED 초록불 끔
				}
			}
			else	//빛의 양이 기준보다 들어오지 않는 경우 ( 주차된 경우 )
			{
				usleep(1000000);
				i++;	//1초의 대기 시간을 넣어 1초마다 i가 카운트 됨
				if( i >= 10 )	//10초동안 차량이 주차되어 있으면
				{
					close(fd_pwm0);	// pwm 파일 닫기, 경고음이 꺼짐

					lseek(fd_seat_red, 0, SEEK_SET);
					wsize = snprintf(databuf, sizeof(databuf), "%d", 1);
					write(fd_seat_red, databuf, wsize);	
					//주차 자리 여부 LED 빨간색 켜짐 ( 자리없음 )

					lseek(fd_seat_green, 0, SEEK_SET);
					wsize = snprintf(databuf, sizeof(databuf), "%d", 0);
					write(fd_seat_green, databuf, wsize);	
					//주차 자리 여부 LED 초록색 꺼짐

					lseek(fd_red, 0, SEEK_SET);
					wsize = snprintf(databuf, sizeof(databuf), "%d", 0);
					write(fd_red, databuf, wsize);

					lseek(fd_ylw, 0, SEEK_SET);
					wsize = snprintf(databuf, sizeof(databuf), "%d", 0);
					write(fd_ylw, databuf, wsize);

					lseek(fd_grn, 0, SEEK_SET);
					wsize = snprintf(databuf, sizeof(databuf), "%d", 0);
					write(fd_grn, databuf, wsize);
					//진입 정도를 나타내는 LED 3개 전부 꺼짐

					i--;	//오래 주차하면 i가 int 범위를 넘을 수 있으므로 제어해준다.
				}
			}

		}
	}
	close(fd_trig);
	close(fd_echo);
	close(fd_snd);
	close(fd_seat_red);
	close(fd_seat_green);
	close(fd_grn);
	close(fd_ylw);
	close(fd_red);
	close(fd_adc0);	//시스템이 꺼지면 열려있던 파일을 모두 닫음
}


int main(void){
	tash_cmd_install("final", final_main, TASH_EXECMD_SYNC);	
//tash에 final이라는 명령을 저장한다. 입력시 final_main 함수가 실행된다.
	return 0;
}