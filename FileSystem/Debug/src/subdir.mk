################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Compactador.c \
../src/Config.c \
../src/FileSystem.c \
../src/IPC.c \
../src/LFS.c \
../src/Lissandra.c \
../src/Shared.c 

OBJS += \
./src/Compactador.o \
./src/Config.o \
./src/FileSystem.o \
./src/IPC.o \
./src/LFS.o \
./src/Lissandra.o \
./src/Shared.o 

C_DEPS += \
./src/Compactador.d \
./src/Config.d \
./src/FileSystem.d \
./src/IPC.d \
./src/LFS.d \
./src/Lissandra.d \
./src/Shared.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/workspace/tp-2019-1c-UnNombre/serializacion" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


