PUNTOMONTAJE=/home/utnso/lfs-compactacion/

cd ./LFS
if [ ! -d "$PUNTOMONTAJE/Metadata/Metadata.bin" ]; then
	mkdir -p "$PUNTOMONTAJE/Metadata"
	cp Metadata.bin "$PUNTOMONTAJE/Metadata"
fi
LD_LIBRARY_PATH=../../../serializacion/Debug ../../../FileSystem/Debug/FileSystem