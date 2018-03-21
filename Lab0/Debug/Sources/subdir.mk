################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Sources/Events.c \
../Sources/FIFO.c \
../Sources/main.c 

OBJS += \
./Sources/Events.o \
./Sources/FIFO.o \
./Sources/main.o 

C_DEPS += \
./Sources/Events.d \
./Sources/FIFO.d \
./Sources/main.d 


# Each subdirectory must supply rules for building sources it contributes
Sources/%.o: ../Sources/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -I"C:/Users/13181680/Downloads/Lab0/Static_Code/PDD" -I"C:\Users\13181680\Downloads\Lab0\Library" -I"C:/Users/13181680/Downloads/Lab0/Static_Code/IO_Map" -I"C:/Users/13181680/Downloads/Lab0/Sources" -I"C:/Users/13181680/Downloads/Lab0/Generated_Code" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


