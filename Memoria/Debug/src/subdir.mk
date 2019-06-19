################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/API.c \
../src/Consola.c \
../src/Gossiping.c \
../src/IPC.c \
../src/Memoria.c \
../src/MemoriaPrincipal.c \
../src/Misc.c 

OBJS += \
./src/API.o \
./src/Consola.o \
./src/Gossiping.o \
./src/IPC.o \
./src/Memoria.o \
./src/MemoriaPrincipal.o \
./src/Misc.o 

C_DEPS += \
./src/API.d \
./src/Consola.d \
./src/Gossiping.d \
./src/IPC.d \
./src/Memoria.d \
./src/MemoriaPrincipal.d \
./src/Misc.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/workspace/tp-2019-1c-UnNombre/serializacion" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


