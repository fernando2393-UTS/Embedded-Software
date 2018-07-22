################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Sources/Calculations.c \
../Sources/FIFO.c \
../Sources/FTM.c \
../Sources/Flash.c \
../Sources/LEDs.c \
../Sources/PIT.c \
../Sources/Signal.c \
../Sources/UART.c \
../Sources/analogMeasure.c \
../Sources/kiss_fft.c \
../Sources/kiss_fftr.c \
../Sources/main.c \
../Sources/packet.c \
../Sources/voltageRegulator.c 

OBJS += \
./Sources/Calculations.o \
./Sources/FIFO.o \
./Sources/FTM.o \
./Sources/Flash.o \
./Sources/LEDs.o \
./Sources/PIT.o \
./Sources/Signal.o \
./Sources/UART.o \
./Sources/analogMeasure.o \
./Sources/kiss_fft.o \
./Sources/kiss_fftr.o \
./Sources/main.o \
./Sources/packet.o \
./Sources/voltageRegulator.o 

C_DEPS += \
./Sources/Calculations.d \
./Sources/FIFO.d \
./Sources/FTM.d \
./Sources/Flash.d \
./Sources/LEDs.d \
./Sources/PIT.d \
./Sources/Signal.d \
./Sources/UART.d \
./Sources/analogMeasure.d \
./Sources/kiss_fft.d \
./Sources/kiss_fftr.d \
./Sources/main.d \
./Sources/packet.d \
./Sources/voltageRegulator.d 


# Each subdirectory must supply rules for building sources it contributes
Sources/%.o: ../Sources/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -I"C:\Users\12443166\Videos\es18aut06\Project\Project\Library" -I"C:/Users/12443166/Videos/es18aut06/Project/Project/Static_Code/IO_Map" -I"C:/Users/12443166/Videos/es18aut06/Project/Project/Sources" -I"C:/Users/12443166/Videos/es18aut06/Project/Project/Generated_Code" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


