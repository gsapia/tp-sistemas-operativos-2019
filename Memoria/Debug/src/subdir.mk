################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/API.c \
../src/Consola.c \
../src/IPC.c \
../src/Memoria.c 

OBJS += \
./src/API.o \
./src/Consola.o \
./src/IPC.o \
./src/Memoria.o 

C_DEPS += \
./src/API.d \
./src/Consola.d \
./src/IPC.d \
./src/Memoria.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


