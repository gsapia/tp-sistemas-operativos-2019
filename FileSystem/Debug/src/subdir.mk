################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Compactador.c \
../src/FileSystem.c \
../src/Lissandra.c 

OBJS += \
./src/Compactador.o \
./src/FileSystem.o \
./src/Lissandra.o 

C_DEPS += \
./src/Compactador.d \
./src/FileSystem.d \
./src/Lissandra.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


