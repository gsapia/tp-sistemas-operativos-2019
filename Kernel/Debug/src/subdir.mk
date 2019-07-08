################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/ApiKernel.c \
../src/IPC.c \
../src/Kernel.c \
../src/Memorias.c \
../src/Metricas.c \
../src/Misc.c 

OBJS += \
./src/ApiKernel.o \
./src/IPC.o \
./src/Kernel.o \
./src/Memorias.o \
./src/Metricas.o \
./src/Misc.o 

C_DEPS += \
./src/ApiKernel.d \
./src/IPC.d \
./src/Kernel.d \
./src/Memorias.d \
./src/Metricas.d \
./src/Misc.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/workspace/tp-2019-1c-UnNombre/serializacion" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


