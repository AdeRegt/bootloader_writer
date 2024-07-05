#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MBROFFSET 0x1B8

typedef struct{
	uint8_t attributes;
	uint8_t chs_start[3];
	uint8_t type;
	uint8_t chs_end[3];
	uint32_t lba_start;
	uint32_t sectors;
}__attribute__((packed)) Partition;

typedef struct{
	uint32_t unique_disk_id;
	uint16_t reserved;
	Partition partition[4];
	uint16_t signature;
}__attribute__((packed)) MBRTemplate;

int main(int argc,char** argv){
	char* targetdevicepath = NULL;
	char* bootloaderpath = NULL;
	if(argc!=3){
		printf("usage: %s [targetdevice] [bootloaderimage]\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	targetdevicepath = argv[1];
	bootloaderpath = argv[2];
	FILE* reference = fopen(targetdevicepath,"r+");
	if(!reference){
		printf("unable to load device\n");
		exit(EXIT_FAILURE);
	}
	FILE* bootloader = fopen(bootloaderpath,"r");
	if(!bootloader){
		printf("unable to load bootloader\n");
		exit(EXIT_FAILURE);
	}
	// read the data
	fseek(reference,MBROFFSET,SEEK_SET);
	void *bufferdata = calloc(1,sizeof(MBRTemplate));
	fread(bufferdata,sizeof(MBRTemplate),1,reference);
	MBRTemplate* template = (MBRTemplate*) bufferdata;
	for(int i = 0 ; i < 4 ; i++){
		printf("partition %d : type: %d , lba: %d , sectors: %d \n",i,template->partition[i].type,template->partition[i].lba_start,template->partition[i].sectors);
	}
	// write the bootsector
	fseek(bootloader,0,SEEK_END);
	int size = ftell(bootloader);
	fseek(bootloader,0,SEEK_SET);
	printf("bootloader has the size of %d !\n",size);
	// overwrite old bootloader
	void* copyable = (void*) calloc(1,size);
	fread(copyable,size,1,bootloader);
	fseek(reference,0,SEEK_SET);
	fwrite(copyable,size,1,reference);
	fseek(reference,MBROFFSET,SEEK_SET);
	fwrite(bufferdata,sizeof(MBRTemplate),1,reference);
	free(copyable);
	free(bufferdata);
	fclose(reference);
	fclose(bootloader);
	exit(EXIT_SUCCESS);
}
