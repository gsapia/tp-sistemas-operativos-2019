################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Compactador.c \
../src/FileSystem.c \
../src/IPC.c \
../src/LFS.c \
../src/Lissandra.c 

OBJS += \
./src/Compactador.o \
./src/FileSystem.o \
./src/IPC.o \
./src/LFS.o \
./src/Lissandra.o 

C_DEPS += \
./src/Compactador.d \
./src/FileSystem.d \
./src/IPC.d \
./src/LFS.d \
./src/Lissandra.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/workspace/tp-2019-1c-UnNombre/serializacion" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


